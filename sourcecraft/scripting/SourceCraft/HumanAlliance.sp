/**
 * vim: set ai et ts=4 sw=4 :
 * File: HumanAlliance.sp
 * Description: The Human Alliance race for SourceCraft.
 * Author(s): Anthony Iacono 
 * Refactored by: -=|JFH|=-Naris (Murray Wilson)
 */
 
#pragma semicolon 1

#include <sourcemod>
#include <sdktools>
#include <raytrace>
#include <range>

#include "sc/SourceCraft"
#include "sc/maxhealth"
#include "sc/Teleport"
#include "sc/freeze"
#include "sc/burrow"

#include "effect/Lightning"
#include "effect/HaloSprite"
#include "effect/BeamSprite"
#include "effect/PhysCannonGlow"
#include "effect/SendEffects"
#include "effect/FlashScreen"

new const String:teleportWav[]      = "war3source/blinkarrival.wav";
//                                    "ambient/machines/teleport1.wav";
//                                    "beams/beamstart5.wav";
//                                    "sc/MassTeleportTarget.wav";

new raceID, immunityID, devotionID, bashID, teleportID;

new g_BashChance[]                  = { 0, 15, 21, 27, 32 };

new Float:g_DevotionHealthPercent[] = { 0.0, 0.15, 0.26, 0.38, 0.50 };

new Float:g_TeleportDistance[]      = { 0.0, 300.0, 500.0, 800.0, 1500.0 };

new Float:gBashTime[MAXPLAYERS+1];

new bool:cfgAllowTeleport;

public Plugin:myinfo = 
{
    name = "SourceCraft Race - Human Alliance",
    author = "-=|JFH|=-Naris with credits to PimpinJuice",
    description = "The Human Alliance race for SourceCraft.",
    version = SOURCECRAFT_VERSION,
    url = "http://www.jigglysfunhouse.net/"
};

public OnPluginStart()
{
    LoadTranslations("sc.human.phrases.txt");
    LoadTranslations("sc.common.phrases.txt");
    LoadTranslations("sc.teleport.phrases.txt");
    GetGameType();

    if (IsSourceCraftLoaded())
        OnSourceCraftReady();
}

public OnSourceCraftReady()
{
    raceID     = CreateRace("human", .faction=HumanAlliance, .type=Biological);
    immunityID = AddUpgrade(raceID, "immunity");
    devotionID = AddUpgrade(raceID, "devotion");
    bashID     = AddUpgrade(raceID, "bash", .energy=2);

    cfgAllowTeleport = bool:GetConfigNum("allow_teleport", true);
    if (cfgAllowTeleport)
    {
        // Ultimate 1
        teleportID = AddUpgrade(raceID, "teleport", 1, .energy=20, .cooldown=2.0);

        GetConfigFloatArray("range",  g_TeleportDistance, sizeof(g_TeleportDistance),
                            g_TeleportDistance, raceID, teleportID);
    }
    else
    {
        // Ultimate 1
        teleportID = AddUpgrade(raceID, "teleport", 1, 99, 0);
        LogMessage("Disabling Human Alliance:Teleport due to configuration: sc_allow_teleport=%d",
                   cfgAllowTeleport);
    }

    // Get Configuration Data
    GetConfigArray("chance", g_BashChance, sizeof(g_BashChance),
                   g_BashChance, raceID, bashID);

    GetConfigFloatArray("percent_health",  g_DevotionHealthPercent, sizeof(g_DevotionHealthPercent),
                        g_DevotionHealthPercent, raceID, devotionID);
}

public OnMapStart()
{
    SetupLightning(false, false);
    SetupHaloSprite(false, false);
    SetupPhysCannonGlow(false, false);
    SetupTeleport(HumanAlliance, false, false);
    SetupSound(teleportWav, DOWNLOAD, false, false);
}

public OnPlayerAuthed(client)
{
    ResetTeleport(client);
    gBashTime[client] = 0.0;
}

public Action:OnRaceDeselected(client,oldrace,newrace)
{
    if (oldrace == raceID)
    {
        gBashTime[client] = 0.0;
        ResetMaxHealth(client);
        ResetTeleport(client);

        #if defined HEALTH_ADDS_ARMOR
            SetArmor(client, 0);
        #endif

        // Turn off Immunities
        new immunity_level=GetUpgradeLevel(client,raceID,immunityID);
        DoImmunity(client, immunity_level, false);

        return Plugin_Handled;
    }
    else
        return Plugin_Continue;
}

public Action:OnRaceSelected(client,oldrace,newrace)
{
    if (newrace == raceID)
    {
        gBashTime[client] = 0.0;

        // Turn on Immunities
        new immunity_level=GetUpgradeLevel(client,raceID,immunityID);
        DoImmunity(client, immunity_level, true);

        new devotion_level=GetUpgradeLevel(client,raceID,devotionID);
        DevotionAura(client, devotion_level);

        return Plugin_Handled;
    }
    else
        return Plugin_Continue;
}

public OnUpgradeLevelChanged(client,race,upgrade,new_level)
{
    if (race == raceID && GetRace(client) == raceID)
    {
        if (upgrade == immunityID)
            DoImmunity(client, new_level, true);
        else if (upgrade == devotionID)
            DevotionAura(client, new_level);
    }
}

public OnUltimateCommand(client,race,bool:pressed,arg)
{
    if (race==raceID && IsPlayerAlive(client))
    {
        new level=GetUpgradeLevel(client,race,teleportID);
        if (level && cfgAllowTeleport)
        {
            new energy=GetUpgradeEnergy(raceID,teleportID) * (5-level);
            TeleportCommand(client, race, teleportID, level, energy,
                            pressed, g_TeleportDistance, teleportWav);
        }
    }
}

// Events
public OnPlayerSpawnEvent(Handle:event, client, race)
{
    if (race == raceID)
    {
        GetClientAbsOrigin(client,spawnLoc[client]);
        gBashTime[client] = 0.0;
        ResetTeleport(client);

        new immunity_level=GetUpgradeLevel(client,raceID,immunityID);
        DoImmunity(client, immunity_level, true);

        new devotion_level=GetUpgradeLevel(client,raceID,devotionID);
        if (devotion_level > 0)
        {
            CreateTimer(0.1,DoDevotionAura,GetClientUserId(client),
                        TIMER_FLAG_NO_MAPCHANGE);
        }
    }
}

public Action:DoDevotionAura(Handle:timer,any:userid)
{
    new client = GetClientOfUserId(userid);
    if (client > 0)
    {
        if (GetRace(client) == raceID)
        {
            new devotion_level=GetUpgradeLevel(client,raceID,devotionID);
            DevotionAura(client, devotion_level);
        }
    }
    return Plugin_Stop;
}

public OnPlayerDeathEvent(Handle:event, victim_index, victim_race, attacker_index,
                          attacker_race, assister_index, assister_race, damage,
                          const String:weapon[], bool:is_equipment, customkill,
                          bool:headshot, bool:backstab, bool:melee)
{
    // Reset MaxHealth back to normal
    if (victim_race == raceID)
    {
        ResetMaxHealth(victim_index);

        #if defined HEALTH_ADDS_ARMOR
            SetArmor(victim_index, 0);
        #endif
    }
}

public Action:OnPlayerHurtEvent(Handle:event, victim_index, victim_race, attacker_index,
                                attacker_race, assister_index, assister_race,
                                damage, absorbed, bool:from_sc)
{
    new bool:changed=false;

    #if defined HEALTH_ADDS_ARMOR
        if (victim_race == raceID && absorbed > 0 &&
            GetUpgradeLevel(victim_index,raceID,devotionID) > 0)
        {
            ShowHealthParticle(victim_index);
        }
    #endif

    if (!from_sc && attacker_index != victim_index)
    {
        if (attacker_index > 0 && attacker_race == raceID)
        {
            Bash(victim_index, attacker_index);

            // Don't allow teleporting immediately after an attack!
            new level = GetUpgradeLevel(attacker_index,raceID,teleportID);
            if (level && cfgAllowTeleport)
                TeleporterAttacked(attacker_index,raceID,teleportID);
        }

        if (assister_index > 0 && assister_race == raceID)
            Bash(victim_index, assister_index);
    }

    return changed ? Plugin_Changed : Plugin_Continue;
}

DoImmunity(client, level, bool:value)
{
    SetImmunity(client,Immunity_ShopItems, (value && level >= 1));
    SetImmunity(client,Immunity_Explosion, (value && level >= 2));
    SetImmunity(client,Immunity_HealthTaking, (value && level >= 3));
    SetImmunity(client,Immunity_Ultimates, (value && level >= 4));

    if (value && IsClientInGame(client) && IsPlayerAlive(client))
    {
        new Float:start[3];
        GetClientAbsOrigin(client, start);

        static const color[4] = { 0, 255, 50, 128 };
        TE_SetupBeamRingPoint(start, 30.0, 60.0, Lightning(), HaloSprite(),
                              0, 1, 2.0, 10.0, 0.0 ,color, 10, 0);
        TE_SendEffectToAll();
    }
}

DevotionAura(client, level)
{
    if (level > 0 && IsValidClient(client) &&
        !GetRestriction(client, Restriction_PreventUpgrades) &&
        !GetRestriction(client, Restriction_Stunned))
    {
        new classmax = GetPlayerMaxHealth(client);
        new maxhp = GetMaxHealth(client);
        if (maxhp > classmax)
            maxhp = classmax;

        new hpadd=RoundFloat(float(maxhp)*g_DevotionHealthPercent[level]);
        if (GetClientHealth(client) < classmax + hpadd)
        {
            decl String:upgradeName[64];
            GetUpgradeName(raceID, devotionID, upgradeName, sizeof(upgradeName), client);
            DisplayMessage(client, Display_Misc_Message, "%t", "ReceivedHP", hpadd, upgradeName);
            SetIncreasedHealth(client, hpadd, 0, "Devotion");

            if (IsPlayerAlive(client))
            {
                if (GetGameType() != tf2)
                {
                    new haloSprite = HaloSprite();

                    new Float:start[3];
                    GetClientAbsOrigin(client, start);

                    decl ringColor[4];
                    if (GetClientTeam(client)==2)
                        ringColor={255,0,0,255};
                    else
                        ringColor={0,0,255,255};

                    start[2]+=25.0;
                    TE_SetupBeamRingPoint(start,40.0,10.0,BeamSprite(),haloSprite,
                                          0,15,1.0,15.0,0.0,ringColor,10,0);
                    TE_SendEffectToAll();

                    new Float:end[3];
                    end[0] = start[0];
                    end[1] = start[1];
                    end[2] = start[2] + 150;

                    static const color[4] = { 200, 255, 205, 255 };
                    TE_SetupBeamPoints(start, end, Lightning(), haloSprite,
                                       0, 1, 2.0, 40.0, 10.0 ,5,50.0,color,255);
                    TE_SendEffectToAll();
                }
            }
        }
    }
    else
    {
        #if defined HEALTH_ADDS_ARMOR
            SetArmor(client, 0);
        #endif
    }
}

Bash(victim_index, index)
{
    if (!GetRestriction(index, Restriction_PreventUpgrades) &&
        !GetRestriction(index, Restriction_Stunned) &&
        !GetImmunity(victim_index, Immunity_MotionTaking) &&
        !GetImmunity(victim_index, Immunity_Upgrades) &&
        !GetImmunity(victim_index, Immunity_Restore) &&
        !IsBurrowed(victim_index))
    {
        new bash_level=GetUpgradeLevel(index,raceID,bashID);
        if (bash_level > 0)
        {
            if (GetRandomInt(1,100) <= g_BashChance[bash_level] &&
                (gBashTime[victim_index] == 0.0 ||
                 GetGameTime() - gBashTime[victim_index] > 2.0))
            {
                new energy = GetEnergy(index);
                new amount = GetUpgradeEnergy(raceID,bashID);
                if (energy >= amount)
                {
                    new Float:Origin[3];
                    GetClientAbsOrigin(victim_index, Origin);
                    TE_SetupGlowSprite(Origin, PhysCannonGlow(), 1.0, 2.3, 90);
                    FlashScreen(victim_index,RGBA_COLOR_BLUE);

                    DisplayMessage(victim_index, Display_Enemy_Ultimate, "%t",
                                   "WasBashed", index);

                    FreezeEntity(victim_index);
                    SetEnergy(index, energy-amount);
                    gBashTime[victim_index] = GetGameTime();
                    CreateTimer(1.0,UnfreezePlayer,GetClientUserId(victim_index),TIMER_FLAG_NO_MAPCHANGE);
                }
            }
        }
    }
}

