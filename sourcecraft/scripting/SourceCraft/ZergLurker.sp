/**
 * vim: set ai et ts=4 sw=4 :
 * File: ZergLurker.sp
 * Description: The Zerg Lurker race for SourceCraft.
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

#include "effect/Lightning"
#include "effect/BeamSprite"
#include "effect/HaloSprite"
#include "effect/SendEffects"

new const String:morphWav[]             = "sc/zlurdy00.wav";
new const String:deathWav[]             = "sc/zludth00.wav";
new const String:deniedWav[]            = "sc/buzz.wav";
new const String:rechargeWav[]          = "sc/transmission.wav";
new const String:poisonHitWav[]         = "sc/spifir00.wav";
new const String:poisonReadyWav[]       = "sc/zhyrdy00.wav";
new const String:poisonExpireWav[]      = "sc/zhywht01.wav";
new const String:spineAttackWav[][]     = { "sc/zlrkfir1.wav",  "sc/zlrkfir2.wav" };

new const String:g_MissileAttackSound[] = "sc/zulhit00.wav";

new raceID, carapaceID, regenerationID, warrenID;
new missileID, augmentsID, poisonID, spineID;

new g_MissileAttackChance[]             = { 5, 15, 25, 35, 45 };
new Float:g_MissileAttackPercent[]      = { 0.25, 0.35, 0.60, 0.80, 1.00 };

new Float:g_SpeedLevels[]               = { 0.60, 0.70, 0.80, 0.90, 1.00 };

new Float:g_SpineRange[]                = { 350.0, 400.0, 650.0, 750.0, 900.0 };
new g_SpineDamage[][2]                  = { { 25, 100},
                                            { 50, 125},
                                            {100, 150},
                                            {125, 200},
                                            {150, 250} };

new const String:g_ArmorName[]          = "Carapace";
new Float:g_InitialArmor[]              = { 0.50, 0.75, 1.00, 1.50, 2.0 };
new Float:g_ArmorPercent[][2]           = { {0.10, 0.20},
                                            {0.20, 0.40},
                                            {0.30, 0.60},
                                            {0.40, 0.80},
                                            {0.50, 0.90} };

public Plugin:myinfo = 
{
    name = "SourceCraft Race - Zerg Lurker",
    author = "-=|JFH|=-Naris",
    description = "The Zerg Lurker race for SourceCraft.",
    version = SOURCECRAFT_VERSION,
    url = "http://jigglysfunhouse.net/"
};

public OnPluginStart()
{
    LoadTranslations("sc.common.phrases.txt");
    LoadTranslations("sc.lurker.phrases.txt");

    GetGameType();
    if (IsSourceCraftLoaded())
        OnSourceCraftReady();
}

public OnSourceCraftReady()
{
    raceID          = CreateRace("lurker", -1, -1, 28, 100, -1, 1,
                                 Zerg, Biological, "hydralisk");

    carapaceID      = AddUpgrade(raceID, "armor", 0, 0);
    regenerationID  = AddUpgrade(raceID, "regeneration", 0, 0);
    warrenID        = AddUpgrade(raceID, "deep_warren", 0, 0);
    augmentsID      = AddUpgrade(raceID, "augments", 0, 0);

    missileID       = AddUpgrade(raceID, "missile_attack", 0, 0,
                                 .energy=2);

    poisonID        = AddUpgrade(raceID, "poison_spines", 0, 0,
                                 .energy=2);

    // Ultimate 1
    spineID         = AddUpgrade(raceID, "spine_attack", 1, 0,
                                 .energy=30, .cooldown=5.0);

    // Ultimate 2
    AddBurrowUpgrade(raceID, 2, 0, 2, 2);

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

    for (new level=0; level < sizeof(g_SpineDamage); level++)
    {
        decl String:key[32];
        Format(key, sizeof(key), "damage_level_%d", level);
        GetConfigArray(key, g_SpineDamage[level], sizeof(g_SpineDamage[]),
                       g_SpineDamage[level], raceID, spineID);
    }

    GetConfigFloatArray("range",  g_SpineRange, sizeof(g_SpineRange),
                        g_SpineRange, raceID, spineID);
}

public OnMapStart()
{
    SetupSpeed(false, false);

    SetupLightning(false, false);
    SetupBeamSprite(false, false);
    SetupHaloSprite(false, false);

    SetupSound(morphWav, true, false, false);
    SetupSound(deathWav, true, false, false);
    SetupSound(deniedWav, true, false, false);
    SetupSound(rechargeWav, true, false, false);
    SetupSound(poisonHitWav, true, false, false);
    SetupSound(poisonReadyWav, true, false, false);
    SetupSound(poisonExpireWav, true, false, false);
    SetupMissileAttack(g_MissileAttackSound, false, false);

    for (new i = 0; i < sizeof(spineAttackWav); i++)
        SetupSound(spineAttackWav[i], true, false, false);
}

public OnMapEnd()
{
    ResetAllClientTimers();
}

public OnClientDisconnect(client)
{
    KillClientTimer(client);
}

public Action:OnRaceDeselected(client,oldrace,newrace)
{
    if (oldrace == raceID)
    {
        ResetArmor(client);
        SetSpeed(client,-1.0);
        SetOverrideVisiblity(client, -1);
        KillClientTimer(client);
    }
    return Plugin_Continue;
}

public Action:OnRaceSelected(client,oldrace,newrace)
{
    if (newrace == raceID)
    {
        new carapace_level = GetUpgradeLevel(client,raceID,carapaceID);
        SetupArmor(client, carapace_level, g_InitialArmor,
                   g_ArmorPercent, g_ArmorName);

        new augments_level = GetUpgradeLevel(client,raceID,augmentsID);
        SetSpeedBoost(client, augments_level, true, g_SpeedLevels);

        if (IsPlayerAlive(client))
        {
            PrepareSound(morphWav);
            EmitSoundToAll(morphWav,client);
            CreateClientTimer(client, 1.0, Regeneration, TIMER_REPEAT|TIMER_FLAG_NO_MAPCHANGE);
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
            SetSpeedBoost(client, augments_level, true, g_SpeedLevels);
        }
    }
}

public OnUltimateCommand(client,race,bool:pressed,arg)
{
    if (race==raceID && IsPlayerAlive(client) && pressed)
    {
        switch (arg)
        {
            case 4,3,2:
            {
                Burrow(client, 3);
            }
            default:
            {
                if (IsBurrowed(client) >= BURROWED_COMPLETELY)
                {
                    new spine_level=GetUpgradeLevel(client,race,spineID);
                    SpineAttack(client, spine_level);
                }
                else
                {
                    PrepareSound(deniedWav);
                    EmitSoundToClient(client,deniedWav);
                    DisplayMessage(client, Display_Ultimate,
                                   "%t", "SpineAttackNotBurrowed");
                }
            }
        }
    }
}

// Events
public OnPlayerSpawnEvent(Handle:event, client, race)
{
    // Don't do anything if unburrowing
    if (race == raceID && IsBurrowed(client) <= -1)
    {
        PrepareSound(morphWav);
        EmitSoundToAll(morphWav,client);

        SetOverrideVisiblity(client, -1);

        new carapace_level = GetUpgradeLevel(client,raceID,carapaceID);
        SetupArmor(client, carapace_level, g_InitialArmor,
                   g_ArmorPercent, g_ArmorName);

        new augments_level = GetUpgradeLevel(client,raceID,augmentsID);
        SetSpeedBoost(client, augments_level, true, g_SpeedLevels);

        CreateClientTimer(client, 1.0, Regeneration,
                          TIMER_REPEAT|TIMER_FLAG_NO_MAPCHANGE);
    }
}

public Action:OnPlayerHurtEvent(Handle:event, victim_index, victim_race, attacker_index,
                                attacker_race, assister_index, assister_race,
                                damage, absorbed, bool:from_sc)
{
    new bool:changed=false;
    if (!from_sc)
    {
        damage += absorbed;

        if (attacker_index && attacker_index != victim_index && attacker_race == raceID)
        {
            new poison_level=GetUpgradeLevel(attacker_index,raceID,poisonID);
            if (GetRandomInt(1,100)<=g_MissileAttackChance[poison_level])
            {
                if (!GetRestriction(attacker_index, Restriction_PreventUltimates) &&
                    !GetRestriction(attacker_index, Restriction_Stunned) &&
                    !GetImmunity(victim_index,Immunity_Ultimates) &&
                    !GetImmunity(victim_index,Immunity_HealthTaking) &&
                    !GetImmunity(victim_index,Immunity_Restore) &&
                    !IsInvulnerable(victim_index))
                {
                    new energy = GetEnergy(attacker_index);
                    new amount = GetUpgradeEnergy(raceID,poisonID);
                    if (energy >= amount)
                    {
                        PlagueInfect(attacker_index, victim_index,
                                     poison_level+1, poison_level+2,
                                     ExplosivePlague|FatalPlague|PoisonousPlague,
                                     "sc_lurkerpoison");

                        PrepareSound(poisonHitWav);
                        EmitSoundToClient(attacker_index, poisonHitWav);
                        EmitSoundToAll(poisonHitWav, victim_index);

                        SetEnergy(attacker_index, energy-amount);
                        changed = true;
                    }
                }
            }

            new missile_level=GetUpgradeLevel(attacker_index,raceID,missileID);
            if (MissileAttack(raceID, missileID, missile_level, event, damage, victim_index,
                              attacker_index, victim_index, false, sizeof(g_MissileAttackChance),
                              g_MissileAttackPercent, g_MissileAttackChance,
                              g_MissileAttackSound, "sc_missile_attack"))
            {
                changed = true;
            }
        }

        if (assister_index && assister_race == raceID)
        {
            new poison_level=GetUpgradeLevel(assister_index,raceID,poisonID);
            if (GetRandomInt(1,100)<=g_MissileAttackChance[poison_level])
            {
                if (!GetRestriction(assister_index, Restriction_PreventUltimates) &&
                    !GetRestriction(assister_index, Restriction_Stunned) &&
                    !GetImmunity(victim_index,Immunity_Ultimates) &&
                    !GetImmunity(victim_index,Immunity_HealthTaking) &&
                    !GetImmunity(victim_index,Immunity_Restore) &&
                    !IsInvulnerable(victim_index))
                {
                    new energy = GetEnergy(assister_index);
                    new amount = GetUpgradeEnergy(raceID,poisonID);
                    if (energy >= amount)
                    {
                        PlagueInfect(assister_index, victim_index,
                                     poison_level+1, poison_level+2,
                                     ExplosivePlague|FatalPlague|PoisonousPlague,
                                     "sc_lurkerpoison");

                        PrepareSound(poisonHitWav);
                        EmitSoundToClient(assister_index,poisonHitWav);
                        EmitSoundToAll(poisonHitWav,victim_index);

                        SetEnergy(assister_index, energy-amount);
                        changed = true;
                    }
                }
            }

            new missile_level=GetUpgradeLevel(assister_index,raceID,missileID);
            if (MissileAttack(raceID, missileID, missile_level, event, damage, victim_index,
                              assister_index, victim_index, false, sizeof(g_MissileAttackChance),
                              g_MissileAttackPercent, g_MissileAttackChance,
                              g_MissileAttackSound, "sc_missile_attack"))
            {
                changed = true;
            }
        }
    }

    return changed ? Plugin_Changed : Plugin_Continue;
}

public OnPlayerDeathEvent(Handle:event, victim_index, victim_race, attacker_index,
                          attacker_race, assister_index, assister_race, damage,
                          const String:weapon[], bool:is_equipment, customkill,
                          bool:headshot, bool:backstab, bool:melee)
{
    if (victim_race == raceID)
    {
        PrepareSound(deathWav);
        EmitSoundToAll(deathWav,victim_index);
        SetOverrideVisiblity(victim_index, -1);
        KillClientTimer(victim_index);
    }
}

public Action:Regeneration(Handle:timer, any:userid)
{
    new client = GetClientOfUserId(userid);
    if (IsValidClient(client) && IsPlayerAlive(client))
    {
        if (GetRace(client) == raceID &&
            !GetRestriction(client,Restriction_PreventUpgrades) &&
            !GetRestriction(client,Restriction_Stunned))
        {
            // Double regeneration while burrowed.
            new amount = GetUpgradeLevel(client,raceID,regenerationID)+1;
            if (IsBurrowed(client))
            {
                amount += amount;
                new energy = GetUpgradeLevel(client,raceID,warrenID);
                if (energy > 0)
                    SetEnergy(client,GetEnergy(client) + energy);
            }

            if (amount > 0)
                HealPlayer(client,amount);
        }
    }
    return Plugin_Continue;
}

SpineAttack(client, level)
{
    decl String:upgradeName[64];
    GetUpgradeName(raceID, spineID, upgradeName, sizeof(upgradeName), client);

    new energy = GetEnergy(client);
    new amount = GetUpgradeEnergy(raceID,spineID);
    if (energy < amount)
    {
        EmitEnergySoundToClient(client,Zerg);
        DisplayMessage(client, Display_Energy, "%t", "InsufficientEnergyFor", upgradeName, amount);
    }
    else if (GetRestriction(client,Restriction_PreventUltimates) ||
             GetRestriction(client,Restriction_Stunned))
    {
        PrepareSound(deniedWav);
        EmitSoundToClient(client,deniedWav);
        DisplayMessage(client, Display_Ultimate, "%t", "Prevented", upgradeName);
    }
    else if (HasCooldownExpired(client, raceID, spineID))
    {
        if (GameType == tf2)
        {
            switch (TF2_GetPlayerClass(client))
            {
                case TFClass_Spy:
                {
                    new pcond = TF2_GetPlayerConditionFlags(client);
                    if (TF2_IsCloaked(pcond) || TF2_IsDeadRingered(pcond) || TF2_IsDisguised(pcond))
                    {
                        PrepareSound(deniedWav);
                        EmitSoundToClient(client,deniedWav);
                        return;
                    }
                }
                case TFClass_Scout:
                {
                    if (TF2_IsPlayerBonked(client))
                    {
                        PrepareSound(deniedWav);
                        EmitSoundToClient(client,deniedWav);
                        return;
                    }
                }
            }
        }

        new Float:range = g_SpineRange[level];
        new dmg = GetRandomInt(g_SpineDamage[level][0],
                               g_SpineDamage[level][1]);

        new Float:indexLoc[3];
        new Float:targetLoc[3];
        new Float:clientLoc[3];
        GetClientEyePosition(client, clientLoc);

        new lightning  = Lightning();
        new haloSprite = HaloSprite();
        static const spineColor[4] = {139, 69, 19, 255};

        new count   = 0;
        new plevel  = level+1;
        new dlevel  = plevel*2;
        new xplevel = level+5;
        new team    = GetClientTeam(client);
        new target  = GetClientAimTarget(client);
        if (target > 0)
        {
            GetClientAbsOrigin(target, targetLoc);
            targetLoc[2] += 50.0; // Adjust trace position to the middle of the person instead of the feet.

            if (IsPointInRange(clientLoc, targetLoc, range) &&
                TraceTargetIndex(target, client, targetLoc, clientLoc))
            {
                if (GetClientTeam(target) != team &&
                    !GetImmunity(target,Immunity_HealthTaking) &&
                    !GetImmunity(target,Immunity_Ultimates) &&
                    !IsInvulnerable(target))
                {
                    TE_SetupBeamPoints(clientLoc,targetLoc, lightning, haloSprite,
                                       0, 1, 10.0, 10.0,10.0,2,50.0,spineColor,255);
                    TE_SendQEffectToAll(client,target);

                    new num = GetRandomInt(0,sizeof(spineAttackWav)-1);
                    PrepareSound(spineAttackWav[num]);
                    EmitSoundToAll(spineAttackWav[num], target);

                    PlagueInfect(client, target, dlevel, plevel,
                                 UltimatePlague|EnsnaringPlague|ExplosivePlague|FatalPlague,
                                 "sc_spine_poison");

                    HurtPlayer(target, dmg, client, "sc_spine_attack",
                               .xp=xplevel, .no_suicide=true,
                               .limit=0.0);

                    dmg -= GetRandomInt(10,20);
                    count++;
                }
            }
            else
            {
                target = client;
                targetLoc = clientLoc;
            }
        }
        else
        {
            target = client;
            targetLoc = clientLoc;
        }

        new b_count=0;
        new alt_count=0;
        new list[MaxClients+1];
        new alt_list[MaxClients+1];
        SetupOBeaconLists(list, alt_list, b_count, alt_count, client);

        if (b_count > 0)
        {
            TE_SetupBeamRingPoint(targetLoc, 10.0, range, BeamSprite(), haloSprite,
                                  0, 15, 0.5, 5.0, 0.0, spineColor, 10, 0);

            TE_Send(list, b_count, 0.0);
        }

        if (alt_count > 0)
        {
            TE_SetupBeamRingPoint(targetLoc, range-10.0, range, BeamSprite(), haloSprite,
                                  0, 15, 0.5, 5.0, 0.0, spineColor, 10, 0);

            TE_Send(alt_list, alt_count, 0.0);
        }

        new num = GetRandomInt(0,sizeof(spineAttackWav)-1);
        PrepareSound(spineAttackWav[num]);
        EmitSoundToAll(spineAttackWav[num], client);

        SetOverrideVisiblity(client, 255, true);
        CreateTimer(5.0,ReCloak,GetClientUserId(client),TIMER_FLAG_NO_MAPCHANGE);
        SetEnergy(client, energy-amount);

        targetLoc[2] += 50.0; // Adjust trace position to the middle of the person instead of the feet.

        for (new index=1;index<=MaxClients;index++)
        {
            if (index != client && index != target && IsClientInGame(index) &&
                IsPlayerAlive(index) && GetClientTeam(index) != team)
            {
                if (!GetImmunity(index,Immunity_Ultimates) &&
                    !GetImmunity(index,Immunity_HealthTaking) &&
                    !IsInvulnerable(index))
                {
                    GetClientAbsOrigin(index, indexLoc);
                    indexLoc[2] += 50.0;

                    if (IsPointInRange(targetLoc,indexLoc,range) &&
                        TraceTargetIndex(target, index, targetLoc, indexLoc))
                    {
                        TE_SetupBeamPoints(clientLoc, indexLoc, lightning, haloSprite,
                                           0, 1, 10.0, 10.0,10.0,2,50.0,spineColor,255);
                        TE_SendQEffectToAll(client, index);

                        PlagueInfect(client, index, dlevel, plevel,
                                     UltimatePlague|EnsnaringPlague|ExplosivePlague|FatalPlague,
                                     "sc_spine_poison");

                        HurtPlayer(index, dmg, client, "sc_spine_attack",
                                   .xp=xplevel, .no_suicide=true,
                                   .limit=0.0);

                        num = GetRandomInt(0,sizeof(spineAttackWav)-1);
                        PrepareSound(spineAttackWav[num]);
                        EmitSoundToAll(spineAttackWav[num], index);

                        count++;
                        dmg -= GetRandomInt(10,20);
                        if (dmg <= 0)
                            break;
                    }
                }
            }
        }

        if (count)
        {
            DisplayMessage(client, Display_Ultimate, "%t",
                           "ToDamageEnemies", upgradeName,
                           count);
        }
        else
        {
            DisplayMessage(client,Display_Ultimate, "%t",
                           "WithoutEffect", upgradeName);
        }

        CreateCooldown(client, raceID, spineID);
    }
}

public Action:ReCloak(Handle:timer,any:userid)
{
    new index = GetClientOfUserId(userid);
    if (IsValidClient(index))
    {
        SetOverrideVisiblity(index, -1, IsPlayerAlive(index));
    }                
    return Plugin_Stop;
}
