/**
 * vim: set ai et ts=4 sw=4 :
 * File: ZergHydralisk.sp
 * Description: The Zerg Hydralisk race for SourceCraft.
 * Author(s): -=|JFH|=-Naris
 */
 
#pragma semicolon 1

#include <sourcemod>
#include <sdktools>
#include <raytrace>
#include <range>

#undef REQUIRE_EXTENSIONS
#include <tf2>
#include <tf2_player>
#define REQUIRE_EXTENSIONS

#include "sc/SourceCraft"
#include "sc/MissileAttack"
#include "sc/PlagueInfect"
#include "sc/clienttimer"
#include "sc/SpeedBoost"
#include "sc/maxhealth"
#include "sc/weapons"
#include "sc/burrow"
#include "sc/armor"

#include "effect/Smoke"
#include "effect/RedGlow"
#include "effect/BlueGlow"
#include "effect/SendEffects"
#include "effect/FlashScreen"

new const String:errorWav[]              = "sc/perror.mp3";
new const String:deniedWav[]             = "sc/buzz.wav";
new const String:rechargeWav[]           = "sc/transmission.wav";
new const String:poisonHitWav[]          = "sc/spifir00.wav";
new const String:poisonReadyWav[]        = "sc/zhyrdy00.wav";
new const String:poisonExpireWav[]       = "sc/zhywht01.wav";

new const String:g_MissileAttackSound[]  = "sc/spooghit.wav";
new g_MissileAttackChance[]              = { 0, 5, 15, 25, 35 };
new Float:g_MissileAttackPercent[]       = { 0.0, 0.35, 0.60, 0.80, 1.00 };

new const String:g_ArmorName[]           = "Carapace";
new Float:g_InitialArmor[]               = { 0.0, 0.50, 0.75, 1.0, 1.50 };
new Float:g_ArmorPercent[][2]            = { {0.00, 0.00},
                                             {0.00, 0.10},
                                             {0.00, 0.30},
                                             {0.10, 0.60},
                                             {0.20, 0.80} };

new Float:g_SpeedLevels[]                = { -1.0, 1.10, 1.15, 1.20, 1.25 };
//new Float:g_SpeedLevels[]              = { -1.0, 1.20, 1.28, 1.36, 1.50 };

new g_lurkerRace = -1;

new raceID, carapaceID, regenerationID, augmentsID;
new missileID, spinesID, poisonID, lurkerID;

new bool:m_PoisonActive[MAXPLAYERS+1];

public Plugin:myinfo = 
{
    name = "SourceCraft Race - Zerg Hydralisk",
    author = "-=|JFH|=-Naris",
    description = "The Zerg Hydralisk race for SourceCraft.",
    version = SOURCECRAFT_VERSION,
    url = "http://jigglysfunhouse.net/"
};

public OnPluginStart()
{
    LoadTranslations("sc.common.phrases.txt");
    LoadTranslations("sc.hydralisk.phrases.txt");

    GetGameType();
    if (IsSourceCraftLoaded())
        OnSourceCraftReady();
}

public OnSourceCraftReady()
{
    raceID          = CreateRace("hydralisk", 48, 0, 26, .faction=Zerg,
                                 .type=Biological);

    carapaceID      = AddUpgrade(raceID, "armor");
    regenerationID  = AddUpgrade(raceID, "regeneration");
    augmentsID      = AddUpgrade(raceID, "augments");
    missileID       = AddUpgrade(raceID, "missile_attack", .energy=2);
    spinesID        = AddUpgrade(raceID, "grooved_spines", .energy=2);

    // Ultimate 1
    poisonID        = AddUpgrade(raceID, "poison_spines", 1,
                                 .energy=20, .cooldown=2.0);

    // Ultimate 2
    AddBurrowUpgrade(raceID, 2, 6, 1);

    // Ultimate 3
    lurkerID        = AddUpgrade(raceID, "lurker", 3, 12, 1,
                                 .energy=120, .cooldown=60.0);

    // Get Configuration Data
    GetConfigFloatArray("armor_amount", g_InitialArmor, sizeof(g_InitialArmor),
                        g_InitialArmor, raceID, carapaceID);

    for (new level=0; level < sizeof(g_ArmorPercent); level++)
    {
        decl String:key[32];
        Format(key, sizeof(key), "armor_percent_level_%d", level);
        GetConfigFloatArray(key, g_ArmorPercent[level], sizeof(g_ArmorPercent[]),
                            g_ArmorPercent[level], raceID, carapaceID);
    }

    GetConfigFloatArray("speed", g_SpeedLevels, sizeof(g_SpeedLevels),
                        g_SpeedLevels, raceID, augmentsID);

    GetConfigArray("chance", g_MissileAttackChance, sizeof(g_MissileAttackChance),
                   g_MissileAttackChance, raceID, missileID);

    GetConfigFloatArray("damage_percent", g_MissileAttackPercent, sizeof(g_MissileAttackPercent),
                        g_MissileAttackPercent, raceID, missileID);
}

public OnMapStart()
{
    SetupSpeed(false, false);
    SetupRedGlow(false, false);
    SetupBlueGlow(false, false);
    SetupSmokeSprite(false, false);

    SetupSound(errorWav, true, false, false);
    SetupSound(deniedWav, true, false, false);
    SetupSound(rechargeWav, true, false, false);
    SetupSound(poisonHitWav, true, false, false);
    SetupSound(poisonReadyWav, true, false, false);
    SetupSound(poisonExpireWav, true, false, false);
    SetupMissileAttack(g_MissileAttackSound, false, false);
}

public OnMapEnd()
{
    ResetAllClientTimers();
}

public OnPlayerAuthed(client)
{
    m_PoisonActive[client] = false;
}

public OnClientDisconnect(client)
{
    m_PoisonActive[client] = false;
    KillClientTimer(client);
}

public Action:OnRaceDeselected(client,oldrace,newrace)
{
    if (oldrace == raceID)
    {
        KillClientTimer(client);
        SetSpeed(client,-1.0);
        ResetArmor(client);

        if (m_PoisonActive[client])
            EndPoison(INVALID_HANDLE, GetClientUserId(client));

        return Plugin_Handled;
    }
    else
    {
        if (g_lurkerRace < 0)
            g_lurkerRace = FindRace("lurker");

        if (oldrace == g_lurkerRace &&
            GetCooldownExpireTime(client, raceID, lurkerID) <= 0.0)
        {
            CreateCooldown(client, raceID, lurkerID,
                           .type=Cooldown_CreateNotify
                                |Cooldown_AlwaysNotify);
        }
        return Plugin_Continue;
    }
}

public Action:OnRaceSelected(client,oldrace,newrace)
{
    if (newrace == raceID)
    {
        m_PoisonActive[client] = false;

        new carapace_level = GetUpgradeLevel(client,raceID,carapaceID);
        SetupArmor(client, carapace_level, g_InitialArmor,
                   g_ArmorPercent, g_ArmorName);

        new augments_level = GetUpgradeLevel(client,raceID,augmentsID);
        SetSpeedBoost(client, augments_level, true, g_SpeedLevels);

        new regeneration_level=GetUpgradeLevel(client,raceID,regenerationID);
        if (regeneration_level && IsPlayerAlive(client))
        {
            CreateClientTimer(client, 1.0, Regeneration,
                              TIMER_REPEAT|TIMER_FLAG_NO_MAPCHANGE);
        }
        return Plugin_Handled;
    }
    else
        return Plugin_Continue;
}

public OnUpgradeLevelChanged(client,race,upgrade,new_level)
{
    if (race == raceID && GetRace(client) == raceID)
    {
        if (upgrade==augmentsID)
            SetSpeedBoost(client, new_level, true, g_SpeedLevels);
        else if (upgrade==carapaceID)
        {
            SetupArmor(client, new_level, g_InitialArmor,
                       g_ArmorPercent, g_ArmorName,
                       .upgrade=true);
        }
        else if (upgrade==burrowID)
        {
            if (new_level <= 0)
                ResetBurrow(client, true);
        }
        else if (upgrade==regenerationID)
        {
            if (new_level > 0)
            {
                if (IsClientInGame(client) && IsPlayerAlive(client))
                {
                    CreateClientTimer(client, 1.0, Regeneration,
                                      TIMER_REPEAT|TIMER_FLAG_NO_MAPCHANGE);
                }
            }
            else
                KillClientTimer(client);
        }
    }
}

public OnItemPurchase(client,item)
{
    new race=GetRace(client);
    if (race == raceID && IsPlayerAlive(client))
    {
        if (g_bootsItem < 0)
            g_bootsItem = FindShopItem("boots");

        if (item == g_bootsItem)
        {
            new augments_level = GetUpgradeLevel(client,race,augmentsID);
            if (augments_level > 0)
                SetSpeedBoost(client, augments_level, true, g_SpeedLevels);
        }
    }
}

public OnUltimateCommand(client,race,bool:pressed,arg)
{
    if (race==raceID && IsPlayerAlive(client))
    {
        switch (arg)
        {
            case 4,3:
            {
                new lurker_level=GetUpgradeLevel(client,race,lurkerID);
                if (lurker_level > 0)
                {
                    if (!pressed)
                        LurkerAspect(client);
                }
                else if (pressed)
                {
                    new burrow_level=GetUpgradeLevel(client,race,burrowID);
                    if (burrow_level > 0)
                        Burrow(client, burrow_level);
                    else
                    {
                        new poison_level=GetUpgradeLevel(client,race,poisonID);
                        if (poison_level > 0)
                            Poison(client, poison_level);
                    }
                }
            }
            case 2:
            {
                new burrow_level=GetUpgradeLevel(client,race,burrowID);
                if (burrow_level > 0)
                {
                    if (pressed)
                        Burrow(client, burrow_level);
                }
                else
                {
                    new poison_level=GetUpgradeLevel(client,race,poisonID);
                    if (poison_level > 0)
                    {
                        if (pressed)
                            Poison(client, poison_level);
                    }
                    else if (!pressed)
                    {
                        new lurker_level=GetUpgradeLevel(client,race,lurkerID);
                        if (lurker_level > 0)
                            LurkerAspect(client);
                    }
                }
            }
            default:
            {
                new poison_level=GetUpgradeLevel(client,race,poisonID);
                if (poison_level > 0)
                {
                    if (pressed)
                        Poison(client, poison_level);
                }
                else
                {
                    new burrow_level=GetUpgradeLevel(client,race,burrowID);
                    if (burrow_level > 0)
                    {
                        if (pressed)
                            Burrow(client, burrow_level);
                    }
                    else if (!pressed)
                    {
                        new lurker_level=GetUpgradeLevel(client,race,lurkerID);
                        if (lurker_level > 0)
                            LurkerAspect(client);
                    }
                }
            }
        }
    }
}

// Events
public OnPlayerSpawnEvent(Handle:event, client, race)
{
    if (race == raceID)
    {
        m_PoisonActive[client] = false;

        new carapace_level = GetUpgradeLevel(client,raceID,carapaceID);
        SetupArmor(client, carapace_level, g_InitialArmor,
                   g_ArmorPercent, g_ArmorName);

        new augments_level = GetUpgradeLevel(client,raceID,augmentsID);
        SetSpeedBoost(client, augments_level, true, g_SpeedLevels);

        new regeneration_level=GetUpgradeLevel(client,raceID,regenerationID);
        if (regeneration_level > 0)
        {
            CreateClientTimer(client, 1.0, Regeneration,
                              TIMER_REPEAT|TIMER_FLAG_NO_MAPCHANGE);
        }
    }
}

public OnPlayerDeathEvent(Handle:event, victim_index, victim_race, attacker_index,
                          attacker_race, assister_index, assister_race, damage,
                          const String:weapon[], bool:is_equipment, customkill,
                          bool:headshot, bool:backstab, bool:melee)
{
    if (victim_race == raceID)
    {
        KillClientTimer(victim_index);
        if (m_PoisonActive[victim_index])
            EndPoison(INVALID_HANDLE, GetClientUserId(victim_index));
    }
    else
    {
        if (g_lurkerRace < 0)
            g_lurkerRace = FindRace("lurker");

        if (victim_race == g_lurkerRace &&
            GetCooldownExpireTime(victim_index, raceID, lurkerID) <= 0.0)
        {
            CreateCooldown(victim_index, raceID, lurkerID,
                           .type=Cooldown_CreateNotify
                                |Cooldown_AlwaysNotify);
        }
    }
}

public Action:OnPlayerHurtEvent(Handle:event, victim_index, victim_race, attacker_index,
                                attacker_race, assister_index, assister_race,
                                damage, absorbed, bool:from_sc)
{
    new bool:changed=false;
    if (!from_sc && attacker_index != victim_index)
    {
        damage += absorbed;

        if (attacker_index > 0 && attacker_race == raceID)
        {
            new bool:used=false;
            if (m_PoisonActive[attacker_index])
            {
                new poison_level=GetUpgradeLevel(attacker_index,raceID,poisonID);
                if (poison_level > 0)
                {
                    if (!GetRestriction(attacker_index, Restriction_PreventUltimates) &&
                        !GetRestriction(attacker_index, Restriction_Stunned) &&
                        !GetImmunity(victim_index,Immunity_Ultimates) &&
                        !GetImmunity(victim_index,Immunity_HealthTaking) &&
                        !GetImmunity(victim_index,Immunity_Restore) &&
                        !IsInvulnerable(victim_index))
                    {
                        PlagueInfect(attacker_index, victim_index,
                                     poison_level, poison_level,
                                     UltimatePlague|FatalPlague|PoisonousPlague,
                                     "sc_poison_spines");

                        PrepareSound(poisonHitWav);
                        EmitSoundToClient(attacker_index,poisonHitWav);
                        EmitSoundToAll(poisonHitWav,victim_index);
                        changed = used = true;
                    }
                }
            }

            if (!used)
            {
                new spines_level=GetUpgradeLevel(attacker_index,raceID,spinesID);
                new missile_level=GetUpgradeLevel(attacker_index,raceID,missileID);
                if (missile_level > 0)
                {
                    changed |= (used = MissileAttack(raceID, missileID, missile_level, event,
                                                     damage, victim_index, attacker_index,
                                                     victim_index, (spines_level > 0),
                                                     sizeof(g_MissileAttackChance),
                                                     g_MissileAttackPercent,
                                                     g_MissileAttackChance,
                                                     g_MissileAttackSound,
                                                     "sc_missile_attack"));
                }

                if (spines_level && !used)
                    changed |= GroovedSpines(damage, victim_index,
                                             attacker_index, spines_level);
            }
        }

        if (assister_index > 0 && assister_race == raceID)
        {
            new bool:used=false;
            if (m_PoisonActive[assister_index])
            {
                new poison_level=GetUpgradeLevel(assister_index,raceID,poisonID);
                if (poison_level > 0)
                {
                    if (!GetRestriction(assister_index, Restriction_PreventUltimates) &&
                        !GetRestriction(assister_index, Restriction_Stunned) &&
                        !GetImmunity(victim_index,Immunity_Ultimates) &&
                        !GetImmunity(victim_index,Immunity_HealthTaking) &&
                        !GetImmunity(victim_index,Immunity_Restore) &&
                        !IsInvulnerable(victim_index))
                    {
                        PlagueInfect(assister_index, victim_index,
                                     poison_level, poison_level,
                                     UltimatePlague|FatalPlague|PoisonousPlague,
                                     "sc_poison_spines");

                        PrepareSound(poisonHitWav);
                        EmitSoundToClient(assister_index,poisonHitWav);
                        EmitSoundToAll(poisonHitWav,victim_index);
                        changed = used = true;
                    }
                }
            }

            if (!used)
            {
                new spines_level=GetUpgradeLevel(assister_index,raceID,spinesID);
                new missile_level=GetUpgradeLevel(assister_index,raceID,missileID);
                if (missile_level > 0)
                {
                    changed |= (used = MissileAttack(raceID, missileID, missile_level, event,
                                                     damage, victim_index, assister_index,
                                                     victim_index, (spines_level > 0),
                                                     sizeof(g_MissileAttackChance),
                                                     g_MissileAttackPercent,
                                                     g_MissileAttackChance,
                                                     g_MissileAttackSound,
                                                     "sc_missile_attack"));
                }

                if (spines_level && !used)
                {
                    changed |= GroovedSpines(damage, victim_index,
                                             assister_index, spines_level);
                }
            }
        }
    }

    return changed ? Plugin_Changed : Plugin_Continue;
}

public Action:Regeneration(Handle:timer, any:userid)
{
    new client = GetClientOfUserId(userid);
    if (IsValidClient(client) && IsPlayerAlive(client) && GetRace(client) == raceID &&
        !GetRestriction(client,Restriction_PreventUpgrades) &&
        !GetRestriction(client,Restriction_Stunned))
    {
        new amount = GetUpgradeLevel(client,raceID,regenerationID);
        if (amount > 0)
            HealPlayer(client,amount);
    }
    return Plugin_Continue;
}

bool:GroovedSpines(damage, victim_index, index, level)
{
    if (!GetRestriction(index, Restriction_PreventUpgrades) &&
        !GetRestriction(index, Restriction_Stunned) &&
        !GetImmunity(victim_index,Immunity_HealthTaking) &&
        !GetImmunity(victim_index,Immunity_Upgrades) &&
        !IsInvulnerable(victim_index))
    {
        new Float:percent;
        new Float:distance = TargetRange(index, victim_index);
        if (distance > 1000.0)
        {
            if (GameType == tf2)
            {
                switch (TF2_GetPlayerClass(index))
                {
                    case TFClass_Scout:     percent = 0.10 * float(level);
                    case TFClass_Sniper:    percent = 0.02 * float(level);
                    case TFClass_Soldier:   percent = 0.10 * float(level);
                    case TFClass_DemoMan:   percent = 0.10 * float(level);
                    case TFClass_Medic:     percent = 0.10 * float(level);
                    case TFClass_Heavy:     percent = 0.50 * float(level);
                    case TFClass_Pyro:      percent = 0.15 * float(level);
                    case TFClass_Spy:       percent = 0.10 * float(level);
                    case TFClass_Engineer:  percent = 0.10 * float(level);
                }
            }
            else
                percent = 0.10 * float(level);
        }
        else if (distance > 500.0)
        {
            if (GameType == tf2)
            {
                switch (TF2_GetPlayerClass(index))
                {
                    case TFClass_Scout:     percent = 0.05 * float(level);
                    case TFClass_Sniper:    percent = 0.01 * float(level);
                    case TFClass_Soldier:   percent = 0.05 * float(level);
                    case TFClass_DemoMan:   percent = 0.05 * float(level);
                    case TFClass_Medic:     percent = 0.05 * float(level);
                    case TFClass_Heavy:     percent = 0.25 * float(level);
                    case TFClass_Pyro:      percent = 0.07 * float(level);
                    case TFClass_Spy:       percent = 0.05 * float(level);
                    case TFClass_Engineer:  percent = 0.05 * float(level);
                }
            }
            else
                percent = 0.05 * float(level);
        }
        else
            percent = 0.0;

        if (percent > 0.0)
        {
            new energy = GetEnergy(index);
            new amount = GetUpgradeEnergy(raceID,spinesID);
            new health_take=RoundFloat(float(damage)*percent);
            if (energy >= amount && health_take > 0)
            {
                SetEnergy(index, energy-amount);
                FlashScreen(victim_index,RGBA_COLOR_RED);
                HurtPlayer(victim_index, health_take, index,
                           "sc_grooved_spines", .type=DMG_SLASH,
                           .in_hurt_event=true);

                return true;
            }
        }
    }
    return false;
}

LurkerAspect(client)
{
    new energy = GetEnergy(client);
    new amount = GetUpgradeEnergy(raceID,lurkerID);
    new accumulated = GetAccumulatedEnergy(client, raceID);
    if (accumulated + energy < amount)
    {
        ShowEnergy(client);
        EmitEnergySoundToClient(client,Zerg);

        decl String:upgradeName[64];
        GetUpgradeName(raceID, lurkerID, upgradeName, sizeof(upgradeName), client);
        DisplayMessage(client, Display_Ultimate, "%t", "InsufficientAccumulatedEnergyFor",
                       upgradeName, amount);
    }
    else if (GetRestriction(client,Restriction_PreventUltimates) ||
             GetRestriction(client,Restriction_Stunned))
    {
        PrepareSound(deniedWav);
        EmitSoundToClient(client,deniedWav);
        DisplayMessage(client, Display_Ultimate,
                       "%t", "PreventedFromLurker");
    }
    else if (HasCooldownExpired(client, raceID, lurkerID))
    {
        if (g_lurkerRace < 0)
            g_lurkerRace = FindRace("lurker");

        if (g_lurkerRace > 0)
        {
            new Float:clientLoc[3];
            GetClientAbsOrigin(client, clientLoc);
            clientLoc[2] += 40.0; // Adjust position to the middle

            TE_SetupSmoke(clientLoc, SmokeSprite(), 8.0, 2);
            TE_SendEffectToAll();

            TE_SetupGlowSprite(clientLoc,(GetClientTeam(client) == 3) ? BlueGlow() : RedGlow(),
                               5.0, 40.0, 255);
            TE_SendEffectToAll();

            accumulated -= amount;
            if (accumulated < 0)
            {
                SetEnergy(client, energy+accumulated);
                accumulated = 0;
            }
            SetAccumulatedEnergy(client, raceID, accumulated);
            ChangeRace(client, g_lurkerRace, true, false);
        }
        else
        {
            PrepareSound(deniedWav);
            EmitSoundToClient(client,deniedWav);
            LogError("***The Zerg Lurker race is not Available!");

            decl String:upgradeName[64];
            GetUpgradeName(raceID, lurkerID, upgradeName, sizeof(upgradeName), client);
            DisplayMessage(client, Display_Ultimate, "%t", "IsNotAvailable", upgradeName);
        }
    }
}

Poison(client, level)
{
    if (level > 0)
    {
        new energy = GetEnergy(client);
        new amount = GetUpgradeEnergy(raceID,poisonID);
        if (energy < amount)
        {
            decl String:upgradeName[64];
            GetUpgradeName(raceID, poisonID, upgradeName, sizeof(upgradeName), client);
            DisplayMessage(client, Display_Energy, "%t", "InsufficientEnergyFor", upgradeName, amount);
            EmitEnergySoundToClient(client,Zerg);
        }
        else if (GetRestriction(client,Restriction_PreventUltimates) ||
                 GetRestriction(client,Restriction_Stunned))
        {
            PrepareSound(deniedWav);
            EmitSoundToClient(client,deniedWav);

            decl String:upgradeName[64];
            GetUpgradeName(raceID, poisonID, upgradeName, sizeof(upgradeName), client);
            DisplayMessage(client, Display_Ultimate, "%t", "Prevented", upgradeName);
        }
        else if (HasCooldownExpired(client, raceID, poisonID))
        {
            m_PoisonActive[client] = true;
            SetEnergy(client, energy-amount);
            CreateTimer(5.0 * float(level), EndPoison, GetClientUserId(client),TIMER_FLAG_NO_MAPCHANGE);

            PrintHintText(client, "%t", "PoisonActive");
            HudMessage(client, "%t", "PoisonHud");
            
            PrepareSound(poisonReadyWav);
            EmitSoundToAll(poisonReadyWav,client);
        }
    }
}

public Action:EndPoison(Handle:timer,any:userid)
{
    new client = GetClientOfUserId(userid);
    if (client > 0 && m_PoisonActive[client])
    {
        if (IsClientInGame(client) && IsPlayerAlive(client))
        {
            PrepareSound(poisonExpireWav);
            EmitSoundToAll(poisonExpireWav,client);

            PrintHintText(client, "%t", "PoisonEnded");
        }

        ClearHud(client, "%t", "PoisonHud");
        m_PoisonActive[client]=false;
        CreateCooldown(client, raceID, poisonID);
    }
}

