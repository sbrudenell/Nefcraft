/**
 * vim: set ai et ts=4 sw=4 :
 * File: UndeadScourge.sp
 * Description: The Undead Scourge race for SourceCraft.
 * Author(s): Anthony Iacono 
 * Rewritten by: Naris (Murray Wilson)
 */
 
#pragma semicolon 1

// Pump up the memory!
#pragma dynamic 32767

#include <sourcemod>
#include <sdktools>
#include <raytrace>
#include <range>

#undef REQUIRE_EXTENSIONS
#include <tf2>
#include <tf2_player>
#define REQUIRE_EXTENSIONS

#undef REQUIRE_PLUGIN
#include <hgrsource>
#define REQUIRE_PLUGIN

#include "sc/SourceCraft"
#include "sc/PlagueInfect"
#include "sc/HealthParticle"
#include "sc/Levitation"
#include "sc/SpeedBoost"
#include "sc/maxhealth"

#include "effect/BeamSprite"
#include "effect/HaloSprite"
#include "effect/SendEffects"
#include "effect/FlashScreen"
#include "effect/Shake"

new const String:deniedWav[]        = "sc/buzz.wav";

new Float:g_SpeedLevels[]           = { -1.0, 1.05, 1.10, 1.16, 1.23 };
new Float:g_LevitationLevels[]      = { 1.0, 0.92, 0.733, 0.5466, 0.36 };
new Float:g_VampiricAuraPercent[]   = { 0.0, 0.12, 0.18, 0.24, 0.30 };
new Float:g_BombRadius[]            = { 0.0, 200.0, 250.0, 300.0, 350.0 };

new g_SucideBombDamage      = 300;

new raceID, vampiricID, unholyID, levitationID, suicideID;

// Suicide bomber check
new bool:m_Suicided[MAXPLAYERS+1];
new Float:m_VampiricAuraTime[MAXPLAYERS+1];

public Plugin:myinfo = 
{
    name = "SourceCraft Race - Undead Scourge",
    author = "-=|JFH|=-Naris with credits to PimpinJuice",
    description = "The Undead Scourge race for SourceCraft.",
    version = SOURCECRAFT_VERSION,
    url = "http://www.jigglysfunhouse.net/"
};

// War3Source Functions
public OnPluginStart()
{
    LoadTranslations("sc.common.phrases.txt");
    LoadTranslations("sc.undead.phrases.txt");

    if (GetGameType() == tf2)
    {
        if (!HookEventEx("teamplay_round_win",EventRoundOver,EventHookMode_PostNoCopy))
            SetFailState("Could not hook the teamplay_round_win event.");

        if (!HookEventEx("teamplay_round_stalemate",EventRoundOver,EventHookMode_PostNoCopy))
            SetFailState("Could not hook the teamplay_round_stalemate event.");
    }
    else if (GameType == dod)
    {
        if (!HookEventEx("dod_round_win",EventRoundOver,EventHookMode_PostNoCopy))
            SetFailState("Could not hook the dod_round_win event.");
    }
    else if (GameType == cstrike)
    {
        if (!HookEventEx("round_end",EventRoundOver,EventHookMode_PostNoCopy))
            SetFailState("Could not hook the round_end event.");
    }

    if (IsSourceCraftLoaded())
        OnSourceCraftReady();
}

public OnSourceCraftReady()
{
    raceID       = CreateRace("undead", .faction=UndeadScourge, .type=Undead);
    vampiricID   = AddUpgrade(raceID, "vampiric_aura", .energy=2);
    unholyID     = AddUpgrade(raceID, "unholy_aura");
    levitationID = AddUpgrade(raceID, "levitation");

    // Ultimate 1
    suicideID    = AddUpgrade(raceID, "suicide_bomb", 1, .energy=30);

    // Get Configuration Data
    g_SucideBombDamage = GetConfigNum("damage", g_SucideBombDamage, raceID, suicideID);

    GetConfigFloatArray("speed", g_SpeedLevels, sizeof(g_SpeedLevels),
                        g_SpeedLevels, raceID, unholyID);

    GetConfigFloatArray("gravity", g_LevitationLevels, sizeof(g_LevitationLevels),
                        g_LevitationLevels, raceID, levitationID);

    GetConfigFloatArray("damage_percent", g_VampiricAuraPercent, sizeof(g_VampiricAuraPercent),
                        g_VampiricAuraPercent, raceID, vampiricID);

    GetConfigFloatArray("range", g_BombRadius, sizeof(g_BombRadius),
                        g_BombRadius, raceID, suicideID);
}

public OnMapStart()
{
    SetupBeamSprite(false, false);
    SetupHaloSprite(false, false);
    SetupLevitation(false, false);
    SetupSpeed(false, false);

    SetupSound(deniedWav, true, false, false);
}

public OnPlayerAuthed(client)
{
    m_VampiricAuraTime[client] = 0.0;
}

public OnUltimateCommand(client,race,bool:pressed,arg)
{
    if (pressed)
    {
        if (race == raceID && IsPlayerAlive(client))
        {
            new level = GetUpgradeLevel(client,race,suicideID);
            if (level > 0)
            {
                new energy = GetEnergy(client);
                new amount = GetUpgradeEnergy(raceID,suicideID);
                if (energy < amount)
                {
                    decl String:upgradeName[64];
                    GetUpgradeName(raceID, suicideID, upgradeName, sizeof(upgradeName), client);
                    DisplayMessage(client, Display_Energy, "%t", "InsufficientEnergyFor", upgradeName, amount);
                    EmitEnergySoundToClient(client,UndeadScourge);
                }
                else if (GetRestriction(client,Restriction_PreventUltimates) ||
                         GetRestriction(client,Restriction_Stunned))
                {
                    PrepareSound(deniedWav);
                    EmitSoundToClient(client,deniedWav);

                    decl String:upgradeName[64];
                    GetUpgradeName(raceID, suicideID, upgradeName, sizeof(upgradeName), client);
                    DisplayMessage(client, Display_Ultimate, "%t", "Prevented", upgradeName);
                }
                else
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

                    SetEnergy(client, energy-amount);
                    ExplodePlayer(client, client, GetClientTeam(client),
                                  g_BombRadius[level], g_SucideBombDamage, 0,
                                  RingExplosion, level+5, "sc_suicide_bomb");
                }
            }
        }
    }
}

public OnUpgradeLevelChanged(client,race,upgrade,new_level)
{
    if (race == raceID && GetRace(client) == raceID)
    {
        if (upgrade==unholyID)
            SetSpeedBoost(client, new_level, true, g_SpeedLevels);
        else if (upgrade==levitationID)
            SetLevitation(client, new_level, true, g_LevitationLevels);
    }
}

public OnItemPurchase(client,item)
{
    if (GetRace(client) == raceID && IsPlayerAlive(client))
    {
        if (g_bootsItem < 0)
            g_bootsItem = FindShopItem("boots");

        if (g_sockItem < 0)
            g_sockItem = FindShopItem("sock");

        if (item == g_bootsItem)
        {
            new unholy_level = GetUpgradeLevel(client,raceID,unholyID);
            if (unholy_level > 0)
                SetSpeedBoost(client, unholy_level, true, g_SpeedLevels);
        }
        else if (item == g_sockItem)
        {
            new levitation_level = GetUpgradeLevel(client,raceID,levitationID);
            SetLevitation(client, levitation_level, true, g_LevitationLevels);
        }
    }
}

public Action:OnDropPlayer(client, target)
{
    if (IsValidClient(target) && GetRace(target) == raceID)
    {
        new levitation_level = GetUpgradeLevel(target,raceID,levitationID);
        SetLevitation(target, levitation_level, true, g_LevitationLevels);
    }
    return Plugin_Continue;
}
public Action:OnRaceDeselected(client,oldrace,newrace)
{
    if (oldrace == raceID)
    {
        SetSpeed(client,-1.0);
        SetGravity(client,-1.0);
        ApplyPlayerSettings(client);
        return Plugin_Handled;
    }
    else
        return Plugin_Continue;
}

public Action:OnRaceSelected(client,oldrace,newrace)
{
    if (newrace == raceID)
    {
        new levitation_level = GetUpgradeLevel(client,raceID,levitationID);
        SetLevitation(client, levitation_level, false, g_LevitationLevels);

        new unholy_level = GetUpgradeLevel(client,raceID,unholyID);
        SetSpeedBoost(client, unholy_level, false, g_SpeedLevels);

        if (unholy_level > 0 || levitation_level > 0)
            ApplyPlayerSettings(client);

        return Plugin_Handled;
    }
    else
        return Plugin_Continue;
}

public OnPlayerSpawnEvent(Handle:event, client, race)
{
    if (race==raceID)
    {
        m_VampiricAuraTime[client] = 0.0;
        m_Suicided[client]=false;

        new levitation_level = GetUpgradeLevel(client,raceID,levitationID);
        SetLevitation(client, levitation_level, false, g_LevitationLevels);

        new unholy_level = GetUpgradeLevel(client,raceID,unholyID);
        SetSpeedBoost(client, unholy_level, false, g_SpeedLevels);

        if (unholy_level > 0 || levitation_level > 0)
            ApplyPlayerSettings(client);
    }
}

public OnPlayerDeathEvent(Handle:event, victim_index, victim_race, attacker_index,
                          attacker_race, assister_index, assister_race, damage,
                          const String:weapon[], bool:is_equipment, customkill,
                          bool:headshot, bool:backstab, bool:melee)
{
    if (victim_race == raceID)
    {
        if (!m_Suicided[victim_index] &&
            !IsChangingClass(victim_index))
        {
            new level = GetUpgradeLevel(victim_index,raceID,suicideID);
            if (level > 0)
            {
                ExplodePlayer(victim_index, victim_index, GetClientTeam(victim_index),
                              g_BombRadius[level], g_SucideBombDamage, 0,
                              RingExplosion|OnDeathExplosion, level+5,
                              "sc_suicide_bomb");
            }
        }
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

        if (attacker_race == raceID && attacker_index != victim_index)
            changed |= VampiricAura(damage, attacker_index, victim_index);

        if (assister_race == raceID)
            changed |= VampiricAura(damage, assister_index, victim_index);
    }

    return changed ? Plugin_Changed : Plugin_Continue;
}

bool:VampiricAura(damage, index, victim_index)
{
    new level = GetUpgradeLevel(index,raceID,vampiricID);
    if (level > 0 && GetRandomInt(1,10) <= 6 &&
        IsClientInGame(index) && IsPlayerAlive(index) &&
        !GetRestriction(index, Restriction_PreventUpgrades) &&
        !GetRestriction(index, Restriction_Stunned) &&
        !GetImmunity(victim_index,Immunity_HealthTaking) &&
        !GetImmunity(victim_index,Immunity_Upgrades) &&
        !IsInvulnerable(victim_index))
    {
        new energy = GetEnergy(victim_index);
        new amount = GetUpgradeEnergy(raceID,vampiricID);
        new Float:lastTime = m_VampiricAuraTime[index];
        new Float:interval = GetGameTime() - lastTime;
        if (energy >= amount && (lastTime == 0.0 || interval > 0.25))
        {
            new Float:start[3];
            GetClientAbsOrigin(index, start);
            start[2] += 1620;

            new Float:end[3];
            GetClientAbsOrigin(index, end);
            end[2] += 20;

            static const color[4] = { 255, 10, 25, 255 };
            TE_SetupBeamPoints(start, end, BeamSprite(), HaloSprite(),
                               0, 1, 3.0, 20.0,10.0,5,50.0,color,255);
            TE_SendEffectToAll();
            FlashScreen(index,RGBA_COLOR_GREEN);
            FlashScreen(victim_index,RGBA_COLOR_RED);

            m_VampiricAuraTime[index] = GetGameTime();
            SetEnergy(victim_index, energy-amount);

            new leechhealth=RoundFloat(float(damage)*g_VampiricAuraPercent[level]);
            if (leechhealth <= 0)
                leechhealth = 1;

            new health = GetClientHealth(index) + leechhealth;
            if (health <= GetMaxHealth(index))
            {
                ShowHealthParticle(index);
                SetEntityHealth(index,health);

                decl String:upgradeName[64];
                GetUpgradeName(raceID, vampiricID, upgradeName, sizeof(upgradeName), index);
                DisplayMessage(index, Display_Damage_Done, "%t", "YouHaveLeeched",
                               leechhealth, victim_index, upgradeName);

            }

            new victim_health = GetClientHealth(victim_index);
            if (victim_health <= leechhealth)
                KillPlayer(victim_index, index, "sc_vampiric_aura");
            else
            {
                CreateParticle("blood_impact_red_01_chunk", 0.1, victim_index, Attach, "head");
                SetEntityHealth(victim_index, victim_health-leechhealth);

                decl String:upgradeName[64];
                GetUpgradeName(raceID, vampiricID, upgradeName, sizeof(upgradeName), victim_index);
                DisplayMessage(victim_index, Display_Damage_Taken, "%t", "HasLeeched",
                               index, leechhealth, upgradeName);
            }
            return true;
        }
    }
    return false;
}

public EventRoundOver(Handle:event,const String:name[],bool:dontBroadcast)
{
    for (new index=1;index<=MaxClients;index++)
    {
        if (IsClientInGame(index))
        {
            SetSpeed(index,-1.0);
            SetGravity(index,-1.0);
        }
    }
}
