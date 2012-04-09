/**
 * vim: set ai et ts=4 sw=4 :
 * File: Baneling.sp
 * Description: The Baneling race for SourceCraft.
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
#include "sc/PlagueInfect"
#include "sc/MeleeAttack"
#include "sc/SpeedBoost"
#include "sc/maxhealth"
#include "sc/weapons"
#include "sc/burrow"

#include "effect/Smoke"
#include "effect/HaloSprite"
#include "effect/SendEffects"
//#include "effect/FlashScreen"
#include "effect/Shake"

new raceID, boostID, rollID, meleeID, explodeID;

new Float:g_SpeedLevels[]           = { 0.60, 0.70, 0.80, 1.00, 1.10 };
new Float:g_AdrenalGlandsPercent[]  = { 0.15, 0.35, 0.55, 0.75, 0.95 };
new Float:g_ExplodeRadius[]         = { 300.0, 450.0, 500.0, 650.0, 800.0 };

new const String:spawnWav[] = "sc/zzeyes02.wav";  // Spawn sound
new const String:deathWav[] = "sc/zbghit00.wav";  // Death sound
new const String:errorWav[] = "sc/perror.mp3";
new const String:deniedWav[] = "sc/buzz.wav";
new const String:rechargeWav[] = "sc/transmission.wav";
new const String:g_AdrenalGlandsSound[] = "sc/zulhit00.wav";

public Plugin:myinfo = 
{
    name = "SourceCraft Race - Baneling",
    author = "-=|JFH|=-Naris",
    description = "The Baneling race for SourceCraft.",
    version = SOURCECRAFT_VERSION,
    url = "http://jigglysfunhouse.net/"
};

public OnPluginStart()
{
    LoadTranslations("sc.common.phrases.txt");
    LoadTranslations("sc.explode.phrases.txt");
    LoadTranslations("sc.baneling.phrases.txt");

    GetGameType();
    if (IsSourceCraftLoaded())
        OnSourceCraftReady();
}

public OnSourceCraftReady()
{
    raceID   = CreateRace("baneling", -1, -1, 17, .faction=Zerg,
                          .type=Biological, .parent="zergling");

    boostID  = AddUpgrade(raceID, "hooks", 0, 0);
    meleeID  = AddUpgrade(raceID, "adrenal_glands", 0, 0, .energy=2);

    // Ultimate 1
    rollID   = AddUpgrade(raceID, "roll", 1, 0, .energy=20);

    // Ultimate 2
    AddBurrowUpgrade(raceID, 2, 0, 1, 1);

    // Ultimate 3
    explodeID = AddUpgrade(raceID, "explode", 3, 0);

    // Get Configuration Data
    GetConfigFloatArray("speed", g_SpeedLevels, sizeof(g_SpeedLevels),
                        g_SpeedLevels, raceID, boostID);

    GetConfigFloatArray("damage_percent", g_AdrenalGlandsPercent, sizeof(g_AdrenalGlandsPercent),
                        g_AdrenalGlandsPercent, raceID, meleeID);

    GetConfigFloatArray("range", g_ExplodeRadius, sizeof(g_ExplodeRadius),
                        g_ExplodeRadius, raceID, explodeID);
}

public OnMapStart()
{
    SetupSpeed(false, false);
    SetupSmokeSprite(false, false);

    SetupSound(spawnWav, true, false, false);
    SetupSound(deathWav, true, false, false);
    SetupSound(errorWav, true, false, false);
    SetupSound(deniedWav, true, false, false);
    SetupSound(rechargeWav, true, false, false);
    SetupSound(g_AdrenalGlandsSound, true, false, false);
}

public Action:OnRaceDeselected(client,oldrace,newrace)
{
    if (oldrace == raceID)
    {
        SetSpeed(client,-1.0);
        SetVisibility(client, NormalVisibility);
        ApplyPlayerSettings(client);
    }
    return Plugin_Continue;
}

public Action:OnRaceSelected(client,oldrace,newrace)
{
    if (newrace == raceID)
    {
        //Set Baneling Color
        new r,g,b;
        if (TFTeam:GetClientTeam(client) == TFTeam_Red)
        { r = 255; g = 165; b = 0; }
        else
        { r = 0; g = 224; b = 208; }
        SetVisibility(client, BasicVisibility,
                      .mode=RENDER_GLOW,
                      .fx=RENDERFX_GLOWSHELL,
                      .r=r, .g=g, .b=b);

        new boost_level = GetUpgradeLevel(client,raceID,boostID);
        SetSpeedBoost(client, boost_level, true, g_SpeedLevels);

        if (IsPlayerAlive(client))
        {
            PrepareSound(spawnWav);
            EmitSoundToAll(spawnWav,client);
        }
    }
    return Plugin_Continue;
}

public OnUpgradeLevelChanged(client,race,upgrade,new_level)
{
    if (race == raceID && GetRace(client) == raceID)
    {
        if (upgrade==boostID)
            SetSpeedBoost(client, new_level, true, g_SpeedLevels);
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
            new boost_level = GetUpgradeLevel(client,race,boostID);
            SetSpeedBoost(client, boost_level, true, g_SpeedLevels);
        }
    }
}

public OnUltimateCommand(client,race,bool:pressed,arg)
{
    if (race==raceID && IsPlayerAlive(client) && pressed)
    {
        switch (arg)
        {
            case 4,3:
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

                if (GetRestriction(client,Restriction_PreventUltimates) ||
                    GetRestriction(client,Restriction_Stunned))
                {
                    PrepareSound(deniedWav);
                    EmitSoundToClient(client,deniedWav);
                    DisplayMessage(client, Display_Ultimate,
                                   "%t", "PreventedFromExploding");
                }
                else
                {
                    new level=GetUpgradeLevel(client,race,explodeID);
                    ExplodePlayer(client, client, GetClientTeam(client),
                                  g_ExplodeRadius[level], 800, 800,
                                  NormalExplosion, level+5);
                }
            }
            case 2:
            {
                new burrow_level=GetUpgradeLevel(client,race,burrowID);
                Burrow(client, burrow_level+1);
            }
            default:
            {
                new roll_level=GetUpgradeLevel(client,race,rollID);
                Roll(client, roll_level);
            }
        }
    }
}

// Events
public OnPlayerSpawnEvent(Handle:event, client, race)
{
    if (race == raceID)
    {
        PrepareSound(spawnWav);
        EmitSoundToAll(spawnWav,client);

        //Set Baneling Color
        new r,g,b;
        if (TFTeam:GetClientTeam(client) == TFTeam_Red)
        { r = 255; g = 165; b = 0; }
        else
        { r = 0; g = 224; b = 208; }

        SetVisibility(client, BasicVisibility,
                      .mode=RENDER_GLOW,
                      .fx=RENDERFX_GLOWSHELL,
                      .r=r, .g=g, .b=b);

        new boost_level = GetUpgradeLevel(client,raceID,boostID);
        SetSpeedBoost(client, boost_level, true, g_SpeedLevels);
    }
}

public Action:OnPlayerHurtEvent(Handle:event, victim_index, victim_race, attacker_index,
                                attacker_race, assister_index, assister_race,
                                damage, absorbed, bool:from_sc)
{
    new bool:changed=false;
    if (attacker_index > 0 && attacker_index != victim_index &&
        attacker_race == raceID && !from_sc)
    {
        new adrenal_glands_level = GetUpgradeLevel(attacker_index,raceID,meleeID);
        if (adrenal_glands_level > 0)
        {
            changed |= MeleeAttack(raceID, meleeID, adrenal_glands_level, event, damage+absorbed,
                                   victim_index, attacker_index, g_AdrenalGlandsPercent,
                                   g_AdrenalGlandsSound, "sc_adrenal_glands");
        }
    }
    return changed ? Plugin_Changed : Plugin_Continue;
}

public OnPlayerDeathEvent(Handle:event, victim_index, victim_race, attacker_index,
                          attacker_race, assister_index, assister_race, damage,
                          const String:weapon[], bool:is_equipment, customkill,
                          bool:headshot, bool:backstab, bool:melee)
{
    if (victim_race == raceID && !IsChangingClass(victim_index))
    {
        if (GetRestriction(victim_index,Restriction_PreventUpgrades) ||
            GetRestriction(victim_index,Restriction_Stunned))
        {
            PrepareSound(deniedWav);
            EmitSoundToClient(victim_index,deniedWav);
            DisplayMessage(victim_index, Display_Misc_Message,
                           "%t", "PreventedFromExploding");
        }
        else
        {
            PrepareSound(deathWav);
            EmitSoundToAll(deathWav,victim_index);

            new level=GetUpgradeLevel(victim_index,raceID,explodeID);
            ExplodePlayer(victim_index, victim_index, GetClientTeam(victim_index),
                          g_ExplodeRadius[level], 800, 800, OnDeathExplosion,
                          level+5);
        }
    }
}

Roll(client, level)
{
    if (IsPlayerAlive(client))
    {
        new energy = GetEnergy(client);
        new amount = GetUpgradeEnergy(raceID,rollID);
        if (energy < amount)
        {
            decl String:upgradeName[64];
            GetUpgradeName(raceID, rollID, upgradeName, sizeof(upgradeName), client);
            DisplayMessage(client, Display_Energy, "%t", "InsufficientEnergyFor", upgradeName, amount);
            EmitEnergySoundToClient(client,Zerg);
        }
        else if (GetRestriction(client,Restriction_PreventUltimates) ||
                 GetRestriction(client,Restriction_Stunned))
        {
            PrepareSound(deniedWav);
            EmitSoundToClient(client,deniedWav);
            DisplayMessage(client, Display_Ultimate,
                           "%t", "PreventedFromRolling");
        }
        else
        {
            new Float:speed=1.10 + (float(level)*0.15);

            /* If the Player also has the Boots of Speed,
             * Increase the speed further
             */
            if (g_bootsItem < 0)
                g_bootsItem = FindShopItem("boots");

            if (g_bootsItem != -1 && GetOwnsItem(client,g_bootsItem))
            {
                speed *= 1.1;
            }

            SetSpeed(client,speed);
            SetEnergy(client, energy-amount);

            new Float:start[3];
            GetClientAbsOrigin(client, start);

            static const color[4] = { 255, 100, 100, 255 };
            TE_SetupBeamRingPoint(start, 20.0, 60.0, SmokeSprite(), HaloSprite(),
                                  0, 1, 1.0, 4.0, 0.0 ,color, 10, 0);
            TE_SendEffectToAll();

            CreateTimer(10.0, EndRoll, GetClientUserId(client),TIMER_FLAG_NO_MAPCHANGE);
        }
    }
}

public Action:EndRoll(Handle:timer,any:userid)
{
    new index = GetClientOfUserId(userid);
    if (IsValidClient(index) && IsPlayerAlive(index))
    {
        if (GetRace(index) == raceID)
        {
            new boost_level = GetUpgradeLevel(index,raceID,boostID);
            SetSpeedBoost(index, boost_level, true, g_SpeedLevels);
        }
        else
            SetSpeed(index,-1.0,true);
    }
}
