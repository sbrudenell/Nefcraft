/**
 * vim: set ai et ts=4 sw=4 :
 * File: Al-Qaeda.sp
 * Description: The Al-Qaeda race for SourceCraft.
 * Author(s): -=|JFH|=-Naris (Murray Wilson) 
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
#include "sc/PlagueInfect"
#include "sc/ShopItems"
#include "sc/clienttimer"
#include "sc/respawn"

#include "effect/Lightning"
#include "effect/BeamSprite"
#include "effect/HaloSprite"
#include "effect/PurpleGlow"
#include "effect/SendEffects"
#include "effect/FlashScreen"
#include "effect/Shake"

new const String:allahWav[] = "sc/allahuakbar.wav";
new const String:kaboomWav[] = "sc/iraqi_engaging.wav";
new const String:deniedWav[] = "sc/buzz.wav";

new g_ReincarnationChance[] = { 0, 9, 22, 36, 53 };

new Float:g_WrathRange[]    = { 0.0, 300.0, 450.0, 650.0, 800.0 };

new g_BomberChance[]        = { 0, 75, 60, 40, 20 };
new g_BomberDamage[]        = { 0, 25, 50, 70, 80 };
new Float:g_BomberRadius[]  = { 0.0, 100.0, 200.0, 250.0, 300.0 };

new g_SucideBombDamage      = 300;

new raceID, reincarnationID, wrathID, suicideID, bomberID;

new cfgMaxRespawns;

new bool:m_Suicided[MAXPLAYERS+1];
new Float:m_BomberTime[MAXPLAYERS+1];

public Plugin:myinfo = 
{
    name = "SourceCraft Race - Al-Qaeda",
    author = "-=|JFH|=-Naris (Murray Wilson)",
    description = "The Al-Qaeda race for SourceCraft.",
    version = SOURCECRAFT_VERSION,
    url = "http://jigglysfunhouse.net/"
};

public OnPluginStart()
{
    LoadTranslations("sc.reincarnate.phrases.txt");
    LoadTranslations("sc.alqaeda.phrases.txt");
    LoadTranslations("sc.explode.phrases.txt");
    LoadTranslations("sc.common.phrases.txt");

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
    raceID = CreateRace("alqaeda", 16, .faction=OrcishHoard, .type=Biological);

    cfgMaxRespawns = GetConfigNum("max_respawns", 4);
    if (cfgMaxRespawns > 1)
    {
        reincarnationID  = AddUpgrade(raceID, "reincarnation", .max_level=cfgMaxRespawns, .energy=5);

        GetConfigArray("chance", g_ReincarnationChance, sizeof(g_ReincarnationChance),
                       g_ReincarnationChance, raceID, reincarnationID);
    }
    else
    {
        reincarnationID  = AddUpgrade(raceID, "reincarnation", 0, 99, 0, .desc="%NotAvailable");
        LogMessage("Disabling Al-Qaeda:Reincarnation due to configuration: sc_maxrespawns=%d",
                   cfgMaxRespawns);
    }

    wrathID     = AddUpgrade(raceID, "wrath");
    suicideID   = AddUpgrade(raceID, "suicide_bomb");

    // Ultimate 1
    bomberID    = AddUpgrade(raceID, "mad_bomber", 1, .energy=2);

    // Get Configuration Data
    g_SucideBombDamage = GetConfigNum("damage", g_SucideBombDamage, raceID, suicideID);

    GetConfigFloatArray("range", g_WrathRange, sizeof(g_WrathRange),
                        g_WrathRange, raceID, wrathID);

    GetConfigArray("chance", g_BomberChance, sizeof(g_BomberChance),
                   g_BomberChance, raceID, bomberID);

    GetConfigArray("damage", g_BomberDamage, sizeof(g_BomberDamage),
                   g_BomberDamage, raceID, bomberID);

    GetConfigFloatArray("range", g_BomberRadius, sizeof(g_BomberRadius),
                        g_BomberRadius, raceID, bomberID);
}

public OnMapStart()
{
    SetupRespawn(false, false);
    SetupLightning(false, false);
    SetupPurpleGlow(false, false);
    SetupBeamSprite(false, false);
    SetupHaloSprite(false, false);

    SetupSound(allahWav, true, false, false);
    SetupSound(kaboomWav, true, false, false);
    SetupSound(deniedWav, true, false, false);
}

public OnMapEnd()
{
    ResetAllClientTimers();
}

public OnPlayerAuthed(client)
{
    m_ReincarnationCount[client]=0;
    m_IsRespawning[client]=false;
    m_BomberTime[client] = 0.0;
    m_Suicided[client]=false;

    #if defined _TRACE
        m_SpawnCount[client]=0;
    #endif
}

public OnClientDisconnect(client)
{
    KillClientTimer(client);
}

public Action:OnRaceDeselected(client,oldrace,newrace)
{
    if (oldrace == raceID)
    {
        TraceInto("Al-Qaeda", "OnRaceDeselected", "client=%d:%N, oldrace=%d, newrace=%d", \
                  client,ValidClientIndex(client), oldrace, newrace);

        m_Suicided[client]=false;
        m_BomberTime[client] = 0.0;
        m_IsRespawning[client]=false;
        m_ReincarnationCount[client]=0;
        KillClientTimer(client);

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
        TraceInto("Al-Qaeda", "OnRaceSelected", "client=%d:%N, oldrace=%d, newrace=%d", \
                  client,ValidClientIndex(client), oldrace, newrace);

        #if defined _TRACE
            m_SpawnCount[client]=0;
        #endif

        m_Suicided[client]=false;
        m_BomberTime[client] = 0.0;
        m_IsRespawning[client]=false;
        m_ReincarnationCount[client]=0;

        if (IsPlayerAlive(client))
        {
            new flaming_wrath_level=GetUpgradeLevel(client,raceID,wrathID);
            if (flaming_wrath_level > 0)
            {
                CreateClientTimer(client, 3.0, FlamingWrath,
                                  TIMER_REPEAT|TIMER_FLAG_NO_MAPCHANGE);
            }
        }

        TraceReturn();
        return Plugin_Handled;
    }
    else
        return Plugin_Continue;
}

public OnUpgradeLevelChanged(client,race,upgrade,new_level)
{
    if (race == raceID && GetRace(client) == raceID)
    {
        if (upgrade==wrathID)
        {
            if (new_level > 0)
            {
                if (IsValidClient(client) && IsPlayerAlive(client))
                {
                    CreateClientTimer(client, 3.0, FlamingWrath,
                                      TIMER_REPEAT|TIMER_FLAG_NO_MAPCHANGE);
                }
            }
            else
                KillClientTimer(client);
        }
    }
}

public OnUltimateCommand(client,race,bool:pressed,arg)
{
    if (pressed)
    {
        if (race == raceID && IsPlayerAlive(client))
        {
            TraceInto("Al-Qaeda", "OnUltimateCommand", "client=%d:%N, race=%d, pressed=%d, arg=%d", \
                      client,ValidClientIndex(client), race, pressed, arg);

            new level = GetUpgradeLevel(client,race,bomberID);
            if (level > 0)
            {
                new energy = GetEnergy(client);
                new amount = GetUpgradeEnergy(raceID,bomberID);
                if (energy < amount)
                {
                    decl String:upgradeName[64];
                    GetUpgradeName(raceID, bomberID, upgradeName, sizeof(upgradeName), client);
                    DisplayMessage(client, Display_Energy, "%t", "InsufficientEnergyFor", upgradeName, amount);
                    EmitEnergySoundToClient(client,OrcishHoard);
                }
                else if (GetRestriction(client,Restriction_PreventUltimates) ||
                         GetRestriction(client,Restriction_Stunned))
                {
                    PrepareSound(deniedWav);
                    EmitSoundToClient(client,deniedWav);

                    decl String:upgradeName[64];
                    GetUpgradeName(raceID, bomberID, upgradeName, sizeof(upgradeName), client);
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

                    PrepareSound(allahWav);
                    EmitSoundToAll(allahWav,client);
                    SetEnergy(client, energy-amount);
                    CreateTimer(0.5, MadBomber, GetClientUserId(client),TIMER_FLAG_NO_MAPCHANGE);
                }
            }

            TraceReturn();
        }
    }
}

public RoundEndEvent(Handle:event,const String:name[],bool:dontBroadcast)
{
    for (new index=1;index<=MaxClients;index++)
    {
        m_ReincarnationCount[index]=0;
        m_IsRespawning[index]=false;
        m_BomberTime[index] = 0.0;
        m_Suicided[index]=false;

        #if defined _TRACE
            m_SpawnCount[index]=0;
        #endif
    }
}

public OnPlayerSpawnEvent(Handle:event, client, race)
{
    if (race == raceID)
    {
        TraceInto("Al-Qaeda", "OnPlayerSpawnEvent", "client=%d:%N, raceID=%d", \
                  client,ValidClientIndex(client), raceID);

        m_BomberTime[client] = 0.0;
        m_Suicided[client]=false;

        Respawned(client,true);

        new flaming_wrath_level = GetUpgradeLevel(client,raceID,wrathID);
        if (flaming_wrath_level > 0)
            CreateClientTimer(client, 3.0, FlamingWrath, TIMER_REPEAT|TIMER_FLAG_NO_MAPCHANGE);

        TraceReturn();
    }
}

public OnPlayerDeathEvent(Handle:event,victim_index,victim_race, attacker_index,
                          attacker_race,assister_index,assister_race,
                          damage,const String:weapon[], bool:is_equipment,
                          customkill,bool:headshot,bool:backstab,bool:melee)
{
    if (victim_index && victim_race == raceID)
    {
        TraceInto("Al-Qaeda", "OnPlayerDeathEvent", "victim_index=%d:%N, victim_race=%d, attacker_index=%d:%N, attacker_race=%d", \
                  victim_index, ValidClientIndex(victim_index), victim_race, \
                  attacker_index, ValidClientIndex(attacker_index), attacker_race);

        KillClientTimer(victim_index);

        if (m_IsRespawning[victim_index])
        {
            TraceReturn("%N died again while respawning", victim_index);
            return;
        }
        else if (IsChangingClass(victim_index))
        {
            m_ReincarnationCount[victim_index] = 0;

            #if defined _TRACE
                m_SpawnCount[victim_index]=0;
            #endif

            TraceReturn("%N changed class", victim_index);
            return;
        }
        else if (m_Suicided[victim_index])
        {
            m_Suicided[victim_index]=false;
            m_ReincarnationCount[victim_index] = 0;
            CreateTimer(0.4, Kaboom, GetClientUserId(victim_index),TIMER_FLAG_NO_MAPCHANGE);

            #if defined _TRACE
                m_SpawnCount[victim_index]=0;
            #endif

            TraceReturn("%N suicided", victim_index);
            return;
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
        else if (IsValidClient(assister_index) && GetImmunity(assister_index,Immunity_Silver))
        {
            PrepareSound(deniedWav);
            EmitSoundToClient(victim_index,deniedWav);
            m_ReincarnationCount[victim_index] = 0;

            if (attacker_index > 0)
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
            new reincarnation_level =GetUpgradeLevel(victim_index,victim_race,reincarnationID);
            if (reincarnation_level > 0 && count < cfgMaxRespawns && count < reincarnation_level)
            {
                if (GetRandomInt(1,100)<=g_ReincarnationChance[reincarnation_level])
                {
                    new energy = GetEnergy(victim_index);
                    new amount = GetUpgradeEnergy(raceID, reincarnationID);
                    //new accumulated = GetAccumulatedEnergy(victim_index, raceID);
                    //if (accumulated  + energy >= amount)
                    if (energy >= amount)
                    {
                        energy -= amount;
                        //if (energy < 0)
                        //{
                        //    SetAccumulatedEnergy(victim_index, raceID, accumulated+energy);
                        //    energy = 0;
                        //}
                        SetEnergy(victim_index, energy);

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

                            if (IsValidClient(attacker_index) && attacker_index != victim_index)
                            {
                                DisplayMessage(attacker_index,Display_Enemy_Message,"%t",
                                               "IsReincarnating",  victim_index, count, suffix);
                            }
                        }
                        return; // No Suicide bombing when reincarnating!
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

            new suicide_level=GetUpgradeLevel(victim_index,victim_race,suicideID);
            if (suicide_level > 0)
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
                    PrepareSound(kaboomWav);
                    EmitSoundToAll(kaboomWav,victim_index);
                    CreateTimer(0.4, Kaboom, GetClientUserId(victim_index),TIMER_FLAG_NO_MAPCHANGE);
                }
            }
        }

        TraceReturn();
    }
}

public Action:MadBomber(Handle:timer,any:userid)
{
    new client = GetClientOfUserId(userid);
    if (client > 0)
    {
        new ult_level=GetUpgradeLevel(client,raceID,bomberID);
        if (ult_level > 0)
        {
            new Float:interval = GetGameTime() - m_BomberTime[client];
            m_BomberTime[client] = GetGameTime();
            if (interval < 0.18 || GetRandomInt(1,100)<=g_BomberChance[ult_level])
            {
                m_Suicided[client]=true;
                PrepareSound(kaboomWav);
                EmitSoundToAll(kaboomWav,client);
                KillPlayer(client, client, "sc_suicide_bomb",
                           .explode=true);
            }
            else
                Bomber(client,ult_level,false);
        }
    }
    return Plugin_Stop;
}

public Action:Kaboom(Handle:timer,any:userid)
{
    new client = GetClientOfUserId(userid);
    if (client > 0)
    {
        new suicide_level=GetUpgradeLevel(client,raceID,suicideID);
        Bomber(client,suicide_level,true);
    }
    return Plugin_Stop;
}

public Bomber(client,level,bool:ondeath)
{
    if (ondeath)
    {
        ExplodePlayer(client, client, GetClientTeam(client), g_BomberRadius[level],
                      g_SucideBombDamage, 0, OnDeathExplosion | SkillExplosion | RingExplosion,
                      level+5, "sc_suicide_bomb");
    }
    else
    {
        ExplodePlayer(client, client, GetClientTeam(client), g_BomberRadius[level],
                      g_BomberDamage[level], 0, RingExplosion | NonFatalExplosion,
                      level+5, "sc_mad_bomber");
    }
}

public Action:FlamingWrath(Handle:timer, any:userid)
{
    new client = GetClientOfUserId(userid);
    if (IsValidClient(client) && IsPlayerAlive(client))
    {
        if (GetRace(client) == raceID)
        {
            static const wrathColor[4] = {255, 10, 55, 255};
            new flaming_wrath_level=GetUpgradeLevel(client,raceID,wrathID);
            if (flaming_wrath_level > 0 && 
                !(GetRestriction(client,Restriction_PreventUpgrades) ||
                  GetRestriction(client,Restriction_Stunned)))
            {
                new Float:range=g_WrathRange[flaming_wrath_level];

                new Float:indexLoc[3];
                new Float:clientLoc[3];
                GetClientAbsOrigin(client, clientLoc);
                clientLoc[2] += 50.0; // Adjust trace position to the middle of the person instead of the feet.

                new count = 0;
                new alt_count=0;
                new list[MaxClients+1];
                new alt_list[MaxClients+1];
                new team=GetClientTeam(client);
                for (new index=1;index<=MaxClients;index++)
                {
                    if (index != client && IsValidClient(index)
                                        && IsPlayerAlive(index))
                    {
                        if (GetClientTeam(index) != team)
                        {
                            if (!GetImmunity(index, Immunity_HealthTaking) &&
                                !GetImmunity(index, Immunity_Upgrades) &&
                                !IsInvulnerable(index))
                            {
                                GetClientAbsOrigin(index, indexLoc);
                                indexLoc[2] += 50.0;

                                if (IsPointInRange(clientLoc,indexLoc,range) &&
                                    TraceTargetIndex(client, index, clientLoc, indexLoc))
                                {
                                    TE_SetupBeamPoints(clientLoc,indexLoc, Lightning(), HaloSprite(),
                                                      0, 1, 3.0, 10.0, 10.0, 5, 50.0, wrathColor, 255);
                                    TE_SendQEffectToAll(client,index);
                                    FlashScreen(index,RGBA_COLOR_RED);

                                    HurtPlayer(index,flaming_wrath_level, client,
                                               "sc_flaming_wrath", .type=DMG_SLOWBURN);

                                    if (!GetSetting(index, Disable_Beacons) &&
                                        !GetSetting(index, Remove_Queasiness))
                                    {
                                        if (GetSetting(index, Reduce_Queasiness))
                                            alt_list[alt_count++] = index;
                                        else
                                            list[count++] = index;
                                    }
                                }
                            }
                        }
                    }
                }

                if (!GetSetting(client, Disable_Beacons) &&
                    !GetSetting(client, Remove_Queasiness))
                {
                    if (GetSetting(client, Reduce_Queasiness))
                        alt_list[alt_count++] = client;
                    else
                        list[count++] = client;
                }

                clientLoc[2] -= 50.0; // Adjust position back to the feet.

                if (count > 0)
                {
                    TE_SetupBeamRingPoint(clientLoc, 10.0, range, BeamSprite(), HaloSprite(),
                                          0, 10, 0.6, 10.0, 0.5, wrathColor, 10, 0);
                    TE_Send(list, count, 0.0);
                }

                if (alt_count > 0)
                {
                    TE_SetupBeamRingPoint(clientLoc, range-10.0, range, BeamSprite(), HaloSprite(),
                                          0, 10, 0.6, 10.0, 0.5, wrathColor, 10, 0);
                    TE_Send(alt_list, alt_count, 0.0);
                }
            }
        }
    }
    return Plugin_Continue;
}
