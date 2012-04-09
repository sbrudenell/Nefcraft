/**
 * vim: set ai et ts=4 sw=4 :
 * File: TerranReaper.sp
 * Description: The Terran Reaper unit for SourceCraft.
 * Author(s): -=|JFH|=-Naris
 */
 
#pragma semicolon 1

#include <sourcemod>
#include <sdktools>

#undef REQUIRE_EXTENSIONS
#include <tf2>
#include <tf2_player>
#define REQUIRE_EXTENSIONS

#undef REQUIRE_PLUGIN
#include <sm_tnt>
#include <jetpack>
#include "sc/RateOfFire"
#define REQUIRE_PLUGIN

#include "sc/SourceCraft"
#include "sc/SpeedBoost"
#include "sc/maxhealth"
#include "sc/weapons"
#include "sc/bunker"
#include "sc/armor"

#include "effect/Lightning"
#include "effect/HaloSprite"
#include "effect/SendEffects"
#include "effect/FlashScreen"

new raceID, weaponsID, armorID, stimpacksID, jetpackID, bunkerID, chargeID;

new const String:g_ArmorName[]  = "Armor";
new Float:g_InitialArmor[]      = { 0.0, 0.25, 0.37, 0.50, 0.75 };
new Float:g_ArmorPercent[][2]   = { {0.00, 0.00},
                                    {0.00, 0.05},
                                    {0.00, 0.15},
                                    {0.05, 0.30},
                                    {0.10, 0.40} };

new g_JetpackFuel[]             = { 0, 40, 50, 70, 90 };
new Float:g_JetpackRefuelTime[] = { 0.0, 45.0, 35.0, 25.0, 15.0 };

new Float:g_WeaponsPercent[]    = { 0.00, 0.10, 0.25, 0.40, 0.50 };
new Float:g_BunkerPercent[]     = { 0.00, 0.50, 0.75, 1.20, 1.60 };
new Float:g_SpeedLevels[]       = { -1.0, 1.10, 1.15, 1.20, 1.25 };

new bool:m_ROFAvailable = false;
new bool:m_TNTAvailable = false;
new bool:m_JetpackAvailable = false;

public Plugin:myinfo = 
{
    name = "SourceCraft Unit - Terran Reaper",
    author = "-=|JFH|=-Naris",
    description = "The Terran Reaper unit for SourceCraft.",
    version = SOURCECRAFT_VERSION,
    url = "http://jigglysfunhouse.net/"
};

public OnPluginStart()
{
    LoadTranslations("sc.common.phrases.txt");
    LoadTranslations("sc.bunker.phrases.txt");
    LoadTranslations("sc.reaper.phrases.txt");
    LoadTranslations("sc.d8charge.phrases.txt");

    if (GetGameType() == tf2)
    {
        if (!HookEventEx("teamplay_round_start",RoundStartEvent,EventHookMode_PostNoCopy))
            SetFailState("Could not hook the teamplay_round_start event.");

        if (!HookEventEx("arena_round_start",RoundStartEvent,EventHookMode_PostNoCopy))
            SetFailState("Could not hook the arena_round_start event.");
    }
    else if (GameType == dod)
    {
        if (!HookEventEx("dod_round_start",RoundStartEvent,EventHookMode_PostNoCopy))
            SetFailState("Could not hook the dod_round_start event.");
    }
    else if (GameType == cstrike)
    {
        if (!HookEventEx("round_start",RoundStartEvent,EventHookMode_PostNoCopy))
            SetFailState("Could not hook the round_start event.");
    }

    if (IsSourceCraftLoaded())
        OnSourceCraftReady();
}

public OnSourceCraftReady()
{
    raceID    = CreateRace("reaper", 48, 0, 24, .faction=Terran,
                           .type=Biological);

    weaponsID = AddUpgrade(raceID, "weapons", .energy=2);
    armorID   = AddUpgrade(raceID, "armor");

    if (m_ROFAvailable || LibraryExists("RateOfFire"))
    {
        stimpacksID = AddUpgrade(raceID, "enhanced_stimpacks", 0, 12,
                                 .recurring_energy=4);

        if (!m_ROFAvailable)
        {
            ControlROF(true);
            m_ROFAvailable = true;
        }
    }
    else
    {
        stimpacksID = AddUpgrade(raceID, "stimpacks", 0, 12);
    }

    // Ultimate 1
    if (m_JetpackAvailable || LibraryExists("jetpack"))
    {
        jetpackID = AddUpgrade(raceID, "jetpack", 1);

        GetConfigArray("fuel", g_JetpackFuel, sizeof(g_JetpackFuel),
                       g_JetpackFuel, raceID, jetpackID);

        GetConfigFloatArray("refuel_time", g_JetpackRefuelTime, sizeof(g_JetpackRefuelTime),
                            g_JetpackRefuelTime, raceID, jetpackID);

        if (!m_JetpackAvailable)
        {
            ControlJetpack(true);
            SetJetpackRefuelingTime(0,30.0);
            SetJetpackFuel(0,100);
            m_JetpackAvailable = true;
        }

    }
    else
    {
        LogError("jetpack is not available");
        jetpackID = AddUpgrade(raceID, "jetpack", 1, 99, 0,
                               .desc="%NotAvailable");
    }

    // Ultimate 2
    bunkerID = AddUpgrade(raceID, "bunker", 2, .energy=30,
                          .cooldown=5.0);

    // Ultimate 3 & 4
    if (m_TNTAvailable || LibraryExists("sm_tnt"))
    {
        chargeID = AddUpgrade(raceID, "d8charge", 3, 16);

        if (!m_TNTAvailable)
        {
            ControlTNT(true);
            m_TNTAvailable = true;
        }
    }
    else
    {
        LogError("sm_tnt is not available");
        chargeID = AddUpgrade(raceID, "d8charge", 3, 99, 0,
                              .desc="%NotAvailable");
    }

    // Get Configuration Data
    GetConfigFloatArray("armor_amount", g_InitialArmor, sizeof(g_InitialArmor),
                        g_InitialArmor, raceID, armorID);

    for (new level=0; level < sizeof(g_ArmorPercent); level++)
    {
        decl String:key[32];
        Format(key, sizeof(key), "armor_percent_level_%d", level);
        GetConfigFloatArray(key, g_ArmorPercent[level], sizeof(g_ArmorPercent[]),
                            g_ArmorPercent[level], raceID, armorID);
    }

    GetConfigFloatArray("bunker_armor", g_BunkerPercent, sizeof(g_BunkerPercent),
                        g_BunkerPercent, raceID, bunkerID);

    GetConfigFloatArray("damage_percent", g_WeaponsPercent, sizeof(g_WeaponsPercent),
                        g_WeaponsPercent, raceID, weaponsID);

    GetConfigFloatArray("speed",  g_SpeedLevels, sizeof(g_SpeedLevels),
                        g_SpeedLevels, raceID, stimpacksID);
}

public OnLibraryAdded(const String:name[])
{
    if (StrEqual(name, "jetpack"))
    {
        if (!m_JetpackAvailable)
        {
            ControlJetpack(true);
            SetJetpackRefuelingTime(0,30.0);
            SetJetpackFuel(0,100);
            m_JetpackAvailable = true;
        }
    }
    else if (StrEqual(name, "sm_tnt"))
    {
        if (!m_TNTAvailable)
        {
            ControlTNT(true);
            m_TNTAvailable = true;
        }
    }
    else if (StrEqual(name, "RateOfFire"))
    {
        if (!m_ROFAvailable)
        {
            ControlROF(true);
            m_ROFAvailable = true;
        }
    }
}

public OnLibraryRemoved(const String:name[])
{
    if (StrEqual(name, "jetpack"))
        m_JetpackAvailable = false;
    else if (StrEqual(name, "sm_tnt"))
        m_TNTAvailable = false;
    else if (StrEqual(name, "RateOfFire"))
        m_ROFAvailable = false;
}

public OnMapStart()
{
    SetupHaloSprite(false, false);
    SetupLightning(false, false);
    SetupBunker(false, false);
    SetupSpeed(false, false);
}

public Action:OnRaceDeselected(client,oldrace,newrace)
{
    if (oldrace == raceID)
    {
        ResetArmor(client);
        SetSpeed(client, -1.0, true);

        if (m_JetpackAvailable)
            TakeJetpack(client);

        if (m_ROFAvailable)
            SetROF(client, 0.0, 0);

        return Plugin_Handled;
    }
    else
        return Plugin_Continue;
}

public Action:OnRaceSelected(client,oldrace,newrace)
{
    if (newrace == raceID)
    {
        new armor_level = GetUpgradeLevel(client,raceID,armorID);
        SetupArmor(client, armor_level, g_InitialArmor,
                   g_ArmorPercent, g_ArmorName);

        new stimpacks_level = GetUpgradeLevel(client,raceID,stimpacksID);
        SetSpeedBoost(client, stimpacks_level, true, g_SpeedLevels);

        new jetpack_level=GetUpgradeLevel(client,raceID,jetpackID);
        SetupJetpack(client, jetpack_level);

        new charge_level=GetUpgradeLevel(client,raceID,chargeID);
        SetupTNT(client, charge_level);

        if (m_ROFAvailable)
        {
            if (stimpacks_level > 0)
            {
                SetROF(client, 2.0/(float(stimpacks_level)),
                       GetUpgradeRecurringEnergy(raceID,stimpacksID));
            }
            else
                SetROF(client, 0.0, 0);
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
        if (upgrade==jetpackID)
            SetupJetpack(client, new_level);
        else if (upgrade==chargeID)
            SetupTNT(client, new_level);
        else if (upgrade==armorID)
        {
            SetupArmor(client, new_level, g_InitialArmor,
                       g_ArmorPercent, g_ArmorName,
                       .upgrade=true);
        }
        else if (upgrade==stimpacksID)
        {
            SetSpeedBoost(client, new_level, true, g_SpeedLevels);

            if (m_ROFAvailable)
            {
                if (new_level > 0)
                {
                    SetROF(client, 2.0/(float(new_level)),
                           GetUpgradeRecurringEnergy(raceID,stimpacksID));
                }
                else
                    SetROF(client, 0.0, 0);
            }
        }
    }
}

public OnItemPurchase(client,item)
{
    if (GetRace(client) == raceID && IsPlayerAlive(client))
    {
        if (g_bootsItem < 0)
            g_bootsItem = FindShopItem("boots");

        if (item == g_bootsItem)
        {
            new level = GetUpgradeLevel(client,raceID,stimpacksID);
            if (level > 0)
                SetSpeedBoost(client, level, true, g_SpeedLevels);
        }
    }
}

public OnUltimateCommand(client,race,bool:pressed,arg)
{
    if (race==raceID && IsPlayerAlive(client))
    {
        switch (arg)
        {
            case 4: // Detonate or Defuse D8 Charge
            {
                new charge_level = GetUpgradeLevel(client,race,chargeID);
                if (charge_level > 0)
                {
                    if (m_TNTAvailable && pressed)
                    {
                        if (GetRestriction(client, Restriction_PreventUltimates) ||
                            GetRestriction(client, Restriction_Stunned))
                        {
                            PrepareSound(deniedWav);
                            EmitSoundToClient(client,deniedWav);
                            DisplayMessage(client, Display_Ultimate,
                                           "%t", "PreventedFromDetonatingD8");
                        }
                        else                            
                        {
                            TNT(client);
                        }
                    }
                }
            }
            case 3: // Plant D8 Charge
            {
                new charge_level = GetUpgradeLevel(client,race,chargeID);
                if (charge_level > 0)
                {
                    if (m_TNTAvailable && pressed)
                    {
                        if (GetRestriction(client, Restriction_PreventUltimates) ||
                            GetRestriction(client, Restriction_Stunned))
                        {
                            PrepareSound(deniedWav);
                            EmitSoundToClient(client,deniedWav);
                            DisplayMessage(client, Display_Ultimate,
                                           "%t", "PreventedFromPlantingD8");
                        }
                        else                            
                        {
                            PlantTNT(client);
                        }
                    }
                }
            }
            case 2: // Enter Bunker
            {
                new bunker_level = GetUpgradeLevel(client,race,bunkerID);
                if (bunker_level > 0)
                {
                    if (pressed)
                    {
                        new armor = RoundToNearest(float(GetPlayerMaxHealth(client))
                                                   * g_BunkerPercent[bunker_level]);

                        EnterBunker(client, armor, raceID, bunkerID);
                    }
                }
                else
                {
                    new jetpack_level = GetUpgradeLevel(client,race,jetpackID);
                    if (jetpack_level > 0)
                        Jetpack(client, pressed);
                }
            }
            default: // Jetpack or Bunker
            {
                new jetpack_level = GetUpgradeLevel(client,race,jetpackID);
                if (jetpack_level > 0)
                    Jetpack(client, pressed);
                else if (pressed)
                {
                    new bunker_level = GetUpgradeLevel(client,race,bunkerID);
                    if (bunker_level > 0)
                    {
                        new armor = RoundToNearest(float(GetPlayerMaxHealth(client))
                                                   * g_BunkerPercent[bunker_level]);

                        EnterBunker(client, armor, raceID, bunkerID);
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
        new armor_level = GetUpgradeLevel(client,raceID,armorID);
        SetupArmor(client, armor_level, g_InitialArmor,
                   g_ArmorPercent, g_ArmorName);

        new stimpacks_level = GetUpgradeLevel(client,raceID,stimpacksID);
        SetSpeedBoost(client, stimpacks_level, true, g_SpeedLevels);

        new jetpack_level=GetUpgradeLevel(client,raceID,jetpackID);
        SetupJetpack(client, jetpack_level);

        new charge_level=GetUpgradeLevel(client,raceID,chargeID);
        SetupTNT(client, charge_level);

        if (m_ROFAvailable)
        {
            if (stimpacks_level > 0)
            {
                SetROF(client, 2.0/(float(stimpacks_level)),
                       GetUpgradeRecurringEnergy(raceID,stimpacksID));
            }
            else
                SetROF(client, 0.0, 0);
        }
    }
}

public Action:OnPlayerHurtEvent(Handle:event, victim_index, victim_race, attacker_index,
                                attacker_race, assister_index, assister_race,
                                damage, absorbed, bool:from_sc)
{
    new bool:changed=false;
    if (!from_sc && victim_index != attacker_index)
    {
        damage += absorbed;

        if (attacker_race == raceID)
        {
            changed |= InfantryWeapons(event, damage, victim_index, attacker_index);
        }

        if (assister_race == raceID)
        {
            changed |= InfantryWeapons(event, damage, victim_index, assister_index);
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
        if (m_ROFAvailable)
            SetROF(victim_race, 0.0, 0);
    }
}

bool:InfantryWeapons(Handle:event, damage, victim_index, index)
{
    new weapons_level = GetUpgradeLevel(index,raceID,weaponsID);
    if (weapons_level > 0)
    {
        if (!GetRestriction(index,Restriction_PreventUpgrades) &&
            !GetRestriction(index,Restriction_Stunned) &&
            !GetImmunity(victim_index,Immunity_HealthTaking) &&
            !GetImmunity(victim_index,Immunity_Upgrades) &&
            !IsInvulnerable(victim_index))
        {
            new energy = GetEnergy(index);
            new amount = GetUpgradeEnergy(raceID,weaponsID);
            if (energy >= amount && GetRandomInt(1,100)<=25)
            {
                decl String:weapon[64];
                new bool:is_equipment=GetWeapon(event,index,weapon,sizeof(weapon));
                if (!IsMelee(weapon, is_equipment,index,victim_index))
                {
                    new health_take = RoundFloat(float(damage)*g_WeaponsPercent[weapons_level]);
                    if (health_take > 0)
                    {
                        new Float:indexLoc[3];
                        GetClientAbsOrigin(index, indexLoc);
                        indexLoc[2] += 50.0;

                        new Float:victimLoc[3];
                        GetClientAbsOrigin(victim_index, victimLoc);
                        victimLoc[2] += 50.0;

                        static const color[4] = { 100, 255, 55, 255 };
                        TE_SetupBeamPoints(indexLoc, victimLoc, Lightning(), HaloSprite(),
                                           0, 50, 1.0, 3.0,6.0,50,50.0,color,255);
                        TE_SendQEffectToAll(index, victim_index);

                        SetEnergy(index, energy-amount);
                        FlashScreen(victim_index,RGBA_COLOR_RED);
                        HurtPlayer(victim_index, health_take, index,
                                   "sc_infantry_weapons", .type=DMG_BULLET,
                                   .in_hurt_event=true);
                        return true;
                    }
                }
            }
        }
    }
    return false;
}

Jetpack(client, bool:pressed)
{
    if (m_JetpackAvailable)
    {
        if (pressed)
        {
            if (InBunker(client) ||
                GetRestriction(client, Restriction_PreventUltimates) ||
                GetRestriction(client, Restriction_Grounded) ||
                GetRestriction(client, Restriction_Stunned))
            {
                PrepareSound(deniedWav);
                EmitSoundToAll(deniedWav, client);
            }
            else
                StartJetpack(client);
        }
        else
            StopJetpack(client);
    }
    else if (pressed)
    {
        PrepareSound(deniedWav);
        EmitSoundToClient(client,deniedWav);

        decl String:upgradeName[64];
        GetUpgradeName(raceID, jetpackID, upgradeName, sizeof(upgradeName), client);
        PrintHintText(client,"%t", "IsNotAvailable", upgradeName);
    }
}

SetupJetpack(client, level)
{
    if (m_JetpackAvailable)
    {
        if (level > 0)
        {
            if (level >= sizeof(g_JetpackFuel))
            {
                LogError("%d:%N has too many levels in TerranReaper::Jetpack level=%d, max=%d",
                         client,ValidClientIndex(client),level,sizeof(g_JetpackFuel));

                level = sizeof(g_JetpackFuel)-1;
            }
            GiveJetpack(client, g_JetpackFuel[level], g_JetpackRefuelTime[level]);
        }
        else
            TakeJetpack(client);
    }
}

SetupTNT(client, level)
{
    if (m_TNTAvailable)
    {
        if (level > 0)
        {
            SetTNT(client, level, (level >= 2) ? 3 : 2, (level >= 4),
                   12.0 - (float(level) * 3.0), 12.0 - (float(level) * 2.0));
        }
        else
            SetTNT(client, 0);
    }
}

public Action:OnTNTBombed(tnt,owner,victim)
{
    if (GetRace(owner) != raceID)
        return Plugin_Continue;
    else
    {
        if (GetImmunity(victim,Immunity_Explosion) ||
            GetImmunity(victim,Immunity_Ultimates))
        {
            return Plugin_Stop;
        }
        else
        {
            DisplayKill(owner, victim, 0, "sc_d8charge");
            return Plugin_Continue;
        }
    }
}

public RoundStartEvent(Handle:event,const String:name[],bool:dontBroadcast)
{
    for (new index=1;index<=MaxClients;index++)
    {
        if (GetRace(index) == raceID && IsClientInGame(index))
        {
            // Reset rate of fire
            CreateTimer(0.1, RoundStartTimer, GetClientUserId(index),
                        TIMER_FLAG_NO_MAPCHANGE);
        }
    }
}

public Action:RoundStartTimer(Handle:timer,any:userid)
{
    new client = GetClientOfUserId(userid);
    if (IsValidClient(client))
    {
        if (GetRace(client) == raceID)
        {
            new stimpacks_level = GetUpgradeLevel(client,raceID,stimpacksID);
            SetSpeedBoost(client, stimpacks_level, true, g_SpeedLevels);

            if (m_ROFAvailable)
            {
                if (stimpacks_level > 0)
                {
                    SetROF(client, 2.0/(float(stimpacks_level)),
                           GetUpgradeRecurringEnergy(raceID,stimpacksID));
                }
                else
                    SetROF(client, 0.0, 0);
            }
        }
    }
}

