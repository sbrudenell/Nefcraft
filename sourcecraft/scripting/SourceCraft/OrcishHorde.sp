/**
 * vim: set ai et ts=4 sw=4 :
 * File: OrcishHorde.sp
 * Description: The Orcish Horde race for SourceCraft.
 * Author(s): Anthony Iacono 
 * Modifications by: Naris (Murray Wilson)
 */
 
#pragma semicolon 1

#include <sourcemod>
#include <sdktools>
#include <raytrace>
#include <range>

#undef REQUIRE_EXTENSIONS
#include <tf2>
#include <tf2_player>
#include <cstrike>
#define REQUIRE_EXTENSIONS

// Define _TRACE to enable trace logging for debugging
//#define _TRACE
#include <trace>

#include "sc/SourceCraft"
#include "sc/MissileAttack"
#include "sc/ShopItems"
#include "sc/respawn"
#include "sc/weapons"

#include "effect/Lightning"
#include "effect/HaloSprite"
#include "effect/PurpleGlow"
#include "effect/SendEffects"
#include "effect/FlashScreen"

new raceID, strikeID, missileID, reincarnationID, lightningID;

new g_ReincarnationChance[]         = { 0, 15, 37, 59, 80 };

new Float:g_StrikePercent[]         = { 0.0, 0.40, 0.60, 0.90, 1.20 };

new Float:g_ChainRange[]            = { 0.0, 300.0, 450.0, 650.0, 800.0 };

new g_MissileAttackChance[]         = { 0, 15, 15, 15, 15 };
new Float:g_MissileAttackPercent[]  = { 0.0, 0.35, 0.60, 0.80, 1.00 };


new const String:deniedWav[]        = "sc/buzz.wav";
new const String:rechargeWav[]      = "sc/transmission.wav";
new const String:thunderWav[]       = "sc/thunder1long.mp3";

new bool:m_HasRespawned[MAXPLAYERS+1];

new cfgMaxRespawns;

public Plugin:myinfo = 
{
    name = "SourceCraft Race - Orcish Horde",
    author = "-=|JFH|=-Naris with credits to PimpinJuice",
    description = "The Orcish Horde race for SourceCraft.",
    version = SOURCECRAFT_VERSION,
    url = "http://www.jigglysfunhouse.net/"
};

public OnPluginStart()
{
    LoadTranslations("sc.reincarnate.phrases.txt");
    LoadTranslations("sc.common.phrases.txt");
    LoadTranslations("sc.orc.phrases.txt");

    if (GetGameType() == tf2)
    {
        if (!HookEvent("teamplay_round_start",RoundEndEvent,EventHookMode_PostNoCopy))
            SetFailState("Couldn't hook the teamplay_round_start event.");

        if (!HookEventEx("teamplay_round_active",RoundEndEvent,EventHookMode_PostNoCopy))
            SetFailState("Could not hook the teamplay_round_active event.");

        if(!HookEventEx("teamplay_round_win",RoundEndEvent, EventHookMode_PostNoCopy))
            SetFailState("Couldn't hook the teamplay_round_win event.");

        if(!HookEventEx("teamplay_round_stalemate",RoundEndEvent, EventHookMode_PostNoCopy))
            SetFailState("Couldn't hook the teamplay_round_stalemate event.");

        if (!HookEventEx("arena_round_start",RoundEndEvent,EventHookMode_PostNoCopy))
            SetFailState("Could not hook the arena_round_start event.");

        if (!HookEventEx("arena_win_panel",RoundEndEvent,EventHookMode_PostNoCopy))
            SetFailState("Could not hook the arena_win_panel event.");

        if (!HookEvent("teamplay_suddendeath_begin",RoundEndEvent,EventHookMode_PostNoCopy))
            SetFailState("Couldn't hook the teamplay_suddendeath_begin event.");
    }
    else if (GameType == dod)
    {
        if (!HookEvent("dod_round_start",RoundEndEvent,EventHookMode_PostNoCopy))
            SetFailState("Couldn't hook the dod_round_start event.");

        if (!HookEventEx("dod_round_active",RoundEndEvent,EventHookMode_PostNoCopy))
            SetFailState("Could not hook the dod_round_active event.");

        if (!HookEvent("dod_round_win",RoundEndEvent,EventHookMode_PostNoCopy))
            SetFailState("Couldn't hook the dod_round_start event.");
    }
    else if (GameType == cstrike)
    {
        if (!HookEvent("round_start",RoundEndEvent,EventHookMode_PostNoCopy))
            SetFailState("Couldn't hook the round_start event.");

        if (!HookEventEx("round_active",RoundEndEvent,EventHookMode_PostNoCopy))
            SetFailState("Could not hook the round_active event.");

        if (!HookEvent("round_end",RoundEndEvent,EventHookMode_PostNoCopy))
            SetFailState("Couldn't hook the round_end event.");
    }

    if (IsSourceCraftLoaded())
        OnSourceCraftReady();
}

public OnSourceCraftReady()
{
    raceID      = CreateRace("orc", .faction=OrcishHoard, .type=Biological);
    strikeID    = AddUpgrade(raceID, "acute_strike", .energy=2);
    missileID   = AddUpgrade(raceID, "acute_grenade", .energy=2);

    cfgMaxRespawns = GetConfigNum("max_respawns", 4);
    if (cfgMaxRespawns > 1)
    {
        reincarnationID = AddUpgrade(raceID, "reincarnation", .max_level=cfgMaxRespawns, .energy=10);
    }
    else
    {
        reincarnationID  = AddUpgrade(raceID, "reincarnation", 0, 99, 0, .desc="%NotAvailable");
        LogMessage("Disabling Orcish Horde:Reincarnation due to configuration: sc_maxrespawns=%d",
                   cfgMaxRespawns);
    }

    // Ultimate 1
    lightningID     = AddUpgrade(raceID, "lightning", 1, .energy=30, .cooldown=2.0);

    // Get Configuration Data
    GetConfigArray("chance", g_ReincarnationChance, sizeof(g_ReincarnationChance),
                   g_ReincarnationChance, raceID, reincarnationID);

    GetConfigArray("chance", g_MissileAttackChance, sizeof(g_MissileAttackChance),
                   g_MissileAttackChance, raceID, missileID);

    GetConfigFloatArray("damage_percent",  g_MissileAttackPercent, sizeof(g_MissileAttackPercent),
                        g_MissileAttackPercent, raceID, missileID);

    GetConfigFloatArray("damage_percent",  g_StrikePercent, sizeof(g_StrikePercent),
                        g_StrikePercent, raceID, strikeID);

    GetConfigFloatArray("range",  g_ChainRange, sizeof(g_ChainRange),
                        g_ChainRange, raceID, lightningID);
}

public OnMapStart()
{
    SetupRespawn(false, false);
    SetupLightning(false, false);
    SetupHaloSprite(false, false);
    SetupPurpleGlow(false, false);
    SetupMissileAttack("", false, false);

    SetupSound(deniedWav, true, false, false);
    SetupSound(thunderWav, true, false, false);
    SetupSound(rechargeWav, true, false, false);
}

public OnPlayerAuthed(client)
{
    m_HasRespawned[client]=false;
    m_IsRespawning[client]=false;
    m_ReincarnationCount[client]=0;

    #if defined _TRACE
        m_SpawnCount[client]=0;
    #endif
}

public Action:OnRaceDeselected(client,oldrace,newrace)
{
    if (oldrace == raceID)
    {
        TraceInto("OrcishHorde", "OnRaceDeselected", "client=%d:%N, oldrace=%d, newrace=%d", \
                  client,ValidClientIndex(client), oldrace, newrace);

        m_IsRespawning[client]=false;
        m_ReincarnationCount[client]=0;

        #if defined _TRACE
            m_SpawnCount[client]=0;
        #endif

        TraceReturn();
        return Plugin_Handled;
    }
    else
        return Plugin_Continue;
}

public Action:OnRaceSelected(client,oldrace,newrace)
{
    if (newrace == raceID)
    {
        TraceInto("OrcishHorde", "OnRaceSelected", "client=%d:%N, oldrace=%d, newrace=%d", \
                  client, ValidClientIndex(client), oldrace, newrace);

        m_HasRespawned[client]=false;
        m_IsRespawning[client]=false;
        m_ReincarnationCount[client]=0;

        #if defined _TRACE
            m_SpawnCount[client]=0;
        #endif

        TraceReturn();
        return Plugin_Handled;
    }
    else
        return Plugin_Continue;
}

public OnUltimateCommand(client,race,bool:pressed,arg)
{
    if (pressed && race == raceID && IsPlayerAlive(client))
    {
        TraceInto("OrcishHorde", "OnUltimateCommand", "client=%d:%N, race=%d, pressed=%d, arg=%d", \
                  client, ValidClientIndex(client), race, pressed, arg);

        new lightning_level = GetUpgradeLevel(client,race,lightningID);
        if (lightning_level > 0)
        {
            decl String:upgradeName[64];
            GetUpgradeName(raceID, lightningID, upgradeName, sizeof(upgradeName), client);

            new energy = GetEnergy(client);
            new amount = GetUpgradeEnergy(raceID,lightningID);
            if (energy < amount)
            {
                EmitEnergySoundToClient(client,OrcishHoard);
                DisplayMessage(client, Display_Energy, "%t", "InsufficientEnergyFor", upgradeName, amount);
            }
            else if (GetRestriction(client,Restriction_PreventUltimates) ||
                     GetRestriction(client,Restriction_Stunned))
            {
                PrepareSound(deniedWav);
                EmitSoundToClient(client,deniedWav);
                DisplayMessage(client, Display_Ultimate, "%t", "Prevented", upgradeName);
            }
            else if (HasCooldownExpired(client, raceID, lightningID))
            {
                if (GameType == tf2)
                {
                    switch (TF2_GetPlayerClass(client))
                    {
                        case TFClass_Spy:
                        {
                            new pcond = TF2_GetPlayerConditionFlags(client);
                            if (TF2_IsCloaked(pcond) || TF2_IsDeadRingered(pcond))
                            {
                                PrepareSound(deniedWav);
                                EmitSoundToClient(client,deniedWav);
                                return;
                            }
                            else if (TF2_IsDisguised(pcond))
                                TF2_RemovePlayerDisguise(client);
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
                ChainLightning(client,lightning_level);
            }
        }

        TraceReturn();
    }
}

// Events
public RoundEndEvent(Handle:event,const String:name[],bool:dontBroadcast)
{
    for (new index=1;index<=MaxClients;index++)
    {
        m_HasRespawned[index]=false;
        m_IsRespawning[index]=false;
        m_ReincarnationCount[index]=0;

        #if defined _TRACE
            m_SpawnCount[index]=0;
        #endif
    }
}

public OnPlayerSpawnEvent(Handle:event, client, race)
{
    if (race == raceID)
    {
        TraceInto("OrcishHorde", "OnPlayerSpawnEvent", "client=%d:%N, raceID=%d", \
                  client, ValidClientIndex(client), raceID);

        Respawned(client,true);

        TraceReturn();
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

        if (attacker_index > 0 && attacker_race == raceID)
        {
            new missile_level = GetUpgradeLevel(attacker_index,raceID,missileID);
            if (missile_level > 0)
            {
                changed = MissileAttack(raceID, missileID, missile_level, event, damage, victim_index,
                                        attacker_index, victim_index, true, sizeof(g_MissileAttackPercent),
                                        g_MissileAttackPercent, g_MissileAttackChance, "",
                                        "sc_acute_grenade");
            }

            if (!changed)
            {
                decl String:weapon[64];
                new bool:is_equipment=GetWeapon(event,attacker_index,weapon,sizeof(weapon));
                if (!IsGrenadeOrRocket(weapon, is_equipment))
                {
                    changed |= AcuteStrike(damage, victim_index, attacker_index);
                }
            }
        }

        if (assister_index > 0 && assister_race == raceID)
        {
            changed |= AcuteStrike(damage, victim_index, assister_index);
        }
    }

    return changed ? Plugin_Changed : Plugin_Continue;
}

public OnPlayerDeathEvent(Handle:event, victim_index, victim_race, attacker_index,
                          attacker_race, assister_index, assister_race, damage,
                          const String:weapon[], bool:is_equipment, customkill,
                          bool:headshot, bool:backstab, bool:melee)
{
    if (victim_race==raceID && (GameType != cstrike || !m_HasRespawned[victim_index]))
    {
        TraceInto("OrcishHorde", "OnPlayerDeathEvent", "victim_index=%d:%N, victim_race=%d, attacker_index=%d:%N, attacker_race=%d", \
                  victim_index, ValidClientIndex(victim_index), victim_race, \
                  attacker_index, ValidClientIndex(attacker_index), attacker_race);

        if (m_IsRespawning[victim_index])
        {
            Trace("%N died again while respawning", victim_index);
        }
        else if (IsChangingClass(victim_index))
        {
            m_ReincarnationCount[victim_index] = 0;

            #if defined _TRACE
                m_SpawnCount[victim_index]=0;
            #endif

            Trace("%N changed class", victim_index);
        }
        else if (IsMole(victim_index))
        {
            PrepareSound(deniedWav);
            EmitSoundToClient(victim_index,deniedWav);

            decl String:upgradeName[64];
            GetUpgradeName(raceID, reincarnationID, upgradeName, sizeof(upgradeName), victim_index);
            DisplayMessage(victim_index, Display_Misc_Message, "%t", "NotAsMole", upgradeName);
            m_ReincarnationCount[victim_index] = 0;

            #if defined _TRACE
                m_SpawnCount[victim_index]=0;
            #endif

            Trace("%N died while a mole", \
                  ValidClientIndex(victim_index));
        }
        else if (GetImmunity(attacker_index,Immunity_Silver))
        {
            PrepareSound(deniedWav);
            EmitSoundToClient(victim_index,deniedWav);
            m_ReincarnationCount[victim_index] = 0;

            if (attacker_index != victim_index && IsValidClient(attacker_index))
            {
                DisplayMessage(victim_index, Display_Misc_Message, "%t", "PreventedFromReincarnatingBySilver", attacker_index);
                DisplayMessage(attacker_index, Display_Enemy_Message, "%t", "ReincarnateWasPreventedBySilver", victim_index);
            }
            else
            {
                DisplayMessage(victim_index, Display_Misc_Message, "%t", "ReincarnatePreventedBySilver");
            }

            #if defined _TRACE
                m_SpawnCount[victim_index]=0;
            #endif

            Trace("%d:%N died due to %d:%N's silver!", \
                  victim_index, ValidClientIndex(victim_index), \
                  attacker_index, ValidClientIndex(attacker_index));
        }
        else if (assister_index > 0 && GetImmunity(assister_index,Immunity_Silver))
        {
            PrepareSound(deniedWav);
            EmitSoundToClient(victim_index,deniedWav);
            m_ReincarnationCount[victim_index] = 0;

            if (attacker_index > 0 && IsValidClient(assister_index))
            {
                DisplayMessage(victim_index, Display_Misc_Message, "%t", "PreventedFromReincarnatingBySilver", assister_index);
                DisplayMessage(assister_index, Display_Enemy_Message, "%t", "ReincarnateWasPreventedBySilver", victim_index);
            }
            else
            {
                DisplayMessage(victim_index, Display_Misc_Message, "%t", "ReincarnatePreventedBySilver");
            }

            #if defined _TRACE
                m_SpawnCount[victim_index]=0;
            #endif

            Trace("%d:%N died due to %d:%N's silver!", \
                  victim_index, ValidClientIndex(victim_index), \
                  assister_index, ValidClientIndex(assister_index));
        }
        else if (GetRestriction(victim_index,Restriction_PreventRespawn) ||
                 GetRestriction(victim_index,Restriction_PreventUpgrades) ||
                 GetRestriction(victim_index,Restriction_Stunned))
        {
            PrepareSound(deniedWav);
            EmitSoundToClient(victim_index,deniedWav);
            DisplayMessage(victim_index, Display_Misc_Message, "%t", "ReincarnatePrevented");
            m_ReincarnationCount[victim_index] = 0;

            #if defined _TRACE
                m_SpawnCount[victim_index]=0;
            #endif

            Trace("%d:%N died due to restrictions!", \
                  victim_index, ValidClientIndex(victim_index));
        }
        else
        {
            new count = m_ReincarnationCount[victim_index];
            new reincarnation_level=GetUpgradeLevel(victim_index,victim_race,reincarnationID);
            if (reincarnation_level > 0 && count < cfgMaxRespawns && count < reincarnation_level)
            {
                if (GetRandomInt(1,100)<=g_ReincarnationChance[reincarnation_level])
                {
                    new energy = GetEnergy(victim_index);
                    new amount = GetUpgradeEnergy(raceID, reincarnationID);
                    //new accumulated = GetAccumulatedEnergy(victim_index, raceID);
                    //if (accumulated + energy >= amount)
                    if (energy >= amount)
                    {
                        energy -= amount;
                        //if (energy < 0)
                        //{
                        //    SetAccumulatedEnergy(victim_index, raceID, accumulated+energy);
                        //    energy = 0;
                        //}
                        SetEnergy(victim_index, energy);

                        m_HasRespawned[victim_index]=true;
                        Respawn(victim_index);

                        decl String:suffix[3];
                        count = m_ReincarnationCount[victim_index];
                        GetNumberSuffix(count, suffix, sizeof(suffix));

                        if (GameType == dod)
                        {
                            DisplayMessage(victim_index, Display_Misc_Message, "%t",
                                           "WillReincarnate", count, suffix);
                        }
                        else
                        {
                            TE_SetupGlowSprite(m_DeathLoc[victim_index],PurpleGlow(),1.0,3.5,150);
                            TE_SendEffectToAll();

                            DisplayMessage(victim_index, Display_Misc_Message,"%t",
                                           "YouAreReincarnating",  count, suffix);
                            if (attacker_index != victim_index && IsValidClient(attacker_index))
                            {
                                DisplayMessage(attacker_index,Display_Enemy_Message,"%t",
                                               "IsReincarnating",  victim_index, count, suffix);
                            }
                        }
                    }
                    else
                    {
                        m_ReincarnationCount[victim_index] = 0;

                        #if defined _TRACE
                            m_SpawnCount[victim_index]=0;
                        #endif

                        Trace("%N died due to lack of energy", victim_index);
                    }
                }
                else
                {
                    m_ReincarnationCount[victim_index] = 0;

                    #if defined _TRACE
                        m_SpawnCount[victim_index]=0;
                    #endif

                    Trace("%N died due to fate", victim_index);
                }
            }
            else
            {
                m_ReincarnationCount[victim_index] = 0;

                #if defined _TRACE
                    m_SpawnCount[victim_index]=0;
                #endif

                Trace("%N died due to lack of levels(=%d, count=%d)", \
                      victim_index, reincarnation_level, count);
            }
        }

        TraceReturn();
    }
}

bool:AcuteStrike(damage, victim_index, index)
{
    new strike_level = GetUpgradeLevel(index,raceID,strikeID);
    if (strike_level > 0 && !GetRestriction(index,Restriction_PreventUpgrades) &&
        !GetRestriction(index,Restriction_Stunned) &&
        !GetImmunity(victim_index,Immunity_HealthTaking) &&
        !GetImmunity(victim_index,Immunity_Upgrades) &&
        !IsInvulnerable(victim_index))
    {
        new energy = GetEnergy(index);
        new amount = GetUpgradeEnergy(raceID,strikeID);
        if (energy >= amount && GetRandomInt(1,100)<=25)
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
            TE_SendQEffectToAll(index,victim_index);
            FlashScreen(victim_index,RGBA_COLOR_RED);

            new health_take = RoundFloat(float(damage)*g_StrikePercent[strike_level]);
            if (health_take < 1)
                health_take = 1;

            SetEnergy(index, energy-amount);
            HurtPlayer(victim_index, health_take, index, "sc_acute_strike",
                       .type=DMG_BULLET, .in_hurt_event=true);
            return true;
        }
    }
    return false;
}

ChainLightning(client,ultlevel)
{
    new factor = ultlevel * 10;
    new dmg=GetRandomInt(10+factor,30+(factor*2));
    new Float:range = g_ChainRange[ultlevel];

    new Float:lastLoc[3];
    new Float:indexLoc[3];
    new Float:clientLoc[3];
    GetClientAbsOrigin(client, clientLoc);
    clientLoc[2] += 50.0; // Adjust trace position to the middle of the person instead of the feet.
    lastLoc = clientLoc;

    new lightning  = Lightning();
    new haloSprite = HaloSprite();
    static const lightningColor[4] = { 10, 200, 255, 255 };

    new b_count=0;
    new alt_count=0;
    new list[MaxClients+1];
    new alt_list[MaxClients+1];
    SetupOBeaconLists(list, alt_list, b_count, alt_count, client);

    if (b_count > 0)
    {
        TE_SetupBeamRingPoint(clientLoc, 10.0, range, lightning, haloSprite,
                              0, 15, 0.5, 5.0, 0.0, lightningColor, 10, 0);

        TE_Send(list, b_count, 0.0);
    }

    if (alt_count > 0)
    {
        TE_SetupBeamRingPoint(clientLoc, range-10.0, range, lightning, haloSprite,
                              0, 15, 0.5, 5.0, 0.0, lightningColor, 10, 0);

        TE_Send(alt_list, alt_count, 0.0);
    }
    
    PrepareSound(thunderWav);
    EmitSoundToAll(thunderWav,client);

    new count=0;
    new last=client;
    new team=GetClientTeam(client);
    new bool:hitVector[MAXPLAYERS] = {false, ...};

    do
    {
        new index = FindClosestTarget(client, team, range, clientLoc, indexLoc, hitVector);
        if (index == 0)
            break;
        else
        {
            TE_SetupBeamPoints(lastLoc, indexLoc, lightning, haloSprite,
                               0, 15, 1.0, 40.0,40.0,0,40.0,lightningColor,40);
            TE_SendQEffectToAll(last,index);
            FlashScreen(index,RGBA_COLOR_RED);

            HurtPlayer(index, dmg, client, "sc_chain_lightning",
                       .xp=5+ultlevel, .type=DMG_ENERGYBEAM);

            hitVector[index] = true;
            lastLoc = indexLoc;
            last = index;
            count++;
        }
    } while (dmg > 0);

    decl String:upgradeName[64];
    GetUpgradeName(raceID, lightningID, upgradeName, sizeof(upgradeName), client);

    if (count)
    {
        DisplayMessage(client, Display_Ultimate, "%t",
                       "ToDamageEnemies", upgradeName, count);
    }
    else
    {
        DisplayMessage(client, Display_Ultimate, "%t",
                       "WithoutEffect", upgradeName);
    }

    CreateCooldown(client, raceID, lightningID);
}

FindClosestTarget(client, team, Float:range, Float:clientLoc[3],
                  Float:indexLoc[3], bool:hitVector[MAXPLAYERS])
{
    new target = 0;
    for(new index=1;index<=MaxClients;index++)
    {
        if (client != index && !hitVector[index] &&
            IsClientInGame(index) && IsPlayerAlive(index) &&
            GetClientTeam(index) != team)
        {
            if (!GetImmunity(index,Immunity_Ultimates) &&
                !GetImmunity(index,Immunity_HealthTaking) &&
                !IsInvulnerable(index))
            {
                GetClientAbsOrigin(index, indexLoc);
                indexLoc[2] += 50.0;

                new Float:distance = GetVectorDistance(clientLoc,indexLoc);
                if (distance <= range &&
                    TraceTargetIndex(client, index, clientLoc, indexLoc))
                {
                    target = index;
                    range = distance;
                }
            }
        }
    }
    return target;
}

