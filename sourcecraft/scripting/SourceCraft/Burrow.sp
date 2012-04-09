/**
 * vim: set ai et ts=4 sw=4 :
 * File: Burrow.sp
 * Description: The Zerg Burrow upgrade for SourceCraft.
 * Author(s): -=|JFH|=-Naris
 */

#pragma semicolon 1

#include <sourcemod>
#include <sdktools>
#include <sdkhooks>
#include <gametype>
#include <ResourceManager>

#undef REQUIRE_EXTENSIONS
#include <tf2>
#include <tf2_uber>
#include <tf2_flag>
#include <tf2_stocks>
#include <tf2_player>
#include <TeleportPlayer>
#define REQUIRE_EXTENSIONS

#undef REQUIRE_PLUGIN
#include <hgrsource>
#include <ztf2grab>
#include <jetpack>
#include <remote>
#define REQUIRE_PLUGIN

// Define _TRACE to enable trace logging for debugging
//#define _TRACE
#include <trace>

#include "sc/SourceCraft"
#include "sc/ShopItems"
#include "sc/maxhealth"
#include "sc/menuitemt"
#include "sc/weapons"

#define MAXENTITIES 2048

new const String:errorWav[] = "sc/perror.mp3";
new const String:buttonWav[] = "buttons/button14.wav";
new const String:deniedWav[] = "sc/buzz.wav";
new const String:lurkerWav[] = "sc/zluburrw.wav";
new const String:burrowUpWav[] = "sc/burrowup.wav";
new const String:burrowDownWav[] = "sc/burrowdn.wav";
new const String:enterBunkerWav[] = "sc/tdrtra00.wav";
new const String:leaveBunkerWav[] = "sc/tdrtra01.wav";
new const String:burrowDeniedWav[] = "sc/zdrerr00.wav"; // "sc/zpwrdown.wav";

new m_BurrowEnergy[MAXPLAYERS+1];
new m_BurrowLevel[MAXPLAYERS+1];
new m_BurrowDepth[MAXPLAYERS+1];
new m_BurrowArmor[MAXPLAYERS+1];
new m_SavedArmor[MAXPLAYERS+1];
new String:m_BurrowName[MAXPLAYERS+1][64];
new String:m_SavedName[MAXPLAYERS+1][64];
new Float:m_SavedPercent[MAXPLAYERS+1][2];

new m_Burrowed[MAXENTITIES+1]; //[MAXPLAYERS+1]; 
new Float:m_BurrowLoc[MAXENTITIES+1][3]; //[MAXPLAYERS+1]; 
new Handle:m_BurrowTimer[MAXENTITIES+1]; //[MAXPLAYERS+1]; 
new Handle:m_UnBurrowTimer[MAXENTITIES+1]; //[MAXPLAYERS+1]; 
new Handle:m_BurrowedTimer[MAXENTITIES+1]; //[MAXPLAYERS+1];

new Handle:m_BurrowedStructures[MAXPLAYERS+1] = { INVALID_HANDLE, ... };

new bool:m_HGRSourceAvailable = false;

public Plugin:myinfo = 
{
    name = "SourceCraft Upgrade - Burrow",
    author = "-=|JFH|=-Naris",
    description = "The Burrow upgrade for SourceCraft.",
    version = SOURCECRAFT_VERSION,
    url = "http://jigglysfunhouse.net/"
};

public APLRes:AskPluginLoad2(Handle:myself, bool:late, String:error[], err_max)
{
    // Register Natives
    CreateNative("Burrow",Native_Burrow);
    CreateNative("UnBurrow",Native_UnBurrow);
    CreateNative("IsBurrowed",Native_IsBurrowed);
    CreateNative("ResetBurrow",Native_ResetBurrow);
    CreateNative("BurrowStructure",Native_BurrowStructure);
    CreateNative("ResetClientStructures",Native_ResetClientStructures);
    RegPluginLibrary("Burrow");

    return APLRes_Success;
}

public OnPluginStart()
{
    LoadTranslations("sc.common.phrases.txt");
    LoadTranslations("sc.burrow.phrases.txt");

    if (!HookEventEx("player_hurt",BurrowPlayerHurtPreEvent, EventHookMode_Pre))
        SetFailState("Couldn't hook the player_hurt pre-event.");

    if (!HookEventEx("player_death",BurrowPlayerDeathPreEvent, EventHookMode_Pre))
        SetFailState("Couldn't hook the player_death pre-event.");

    if (!HookEvent("player_team",PlayerChangeClassEvent))
        SetFailState("Could not hook the player_team event.");

    if (GetGameType() == tf2)
    {
        if (!HookEvent("player_changeclass",PlayerChangeClassEvent,EventHookMode_Post))
            SetFailState("Couldn't hook the player_changeclass event.");

        if (!HookEventEx("teamplay_round_start",EventRoundChange,EventHookMode_PostNoCopy))
            SetFailState("Could not hook the teamplay_round_start event.");

        if (!HookEventEx("teamplay_round_active",EventRoundChange,EventHookMode_PostNoCopy))
            SetFailState("Could not hook the teamplay_round_active event.");

        if (!HookEventEx("teamplay_round_win",EventRoundChange,EventHookMode_PostNoCopy))
            SetFailState("Could not hook the teamplay_round_win event.");

        if (!HookEventEx("teamplay_round_stalemate",EventRoundChange,EventHookMode_PostNoCopy))
            SetFailState("Could not hook the teamplay_round_stalemate event.");

        if (!HookEventEx("tf_game_over",EventRoundChange,EventHookMode_PostNoCopy))
            SetFailState("Could not hook the tf_game_over event.");

        if (!HookEventEx("teamplay_game_over",EventRoundChange,EventHookMode_PostNoCopy))
            SetFailState("Could not hook the teamplay_game_over event.");

        if (!HookEventEx("teamplay_win_panel",EventRoundChange,EventHookMode_PostNoCopy))
            SetFailState("Could not hook the teamplay_win_panel event.");

        if (!HookEventEx("arena_round_start",EventRoundChange,EventHookMode_PostNoCopy))
            SetFailState("Could not hook the arena_round_start event.");

        if (!HookEventEx("arena_win_panel",EventRoundChange,EventHookMode_PostNoCopy))
            SetFailState("Could not hook the arena_win_panel event.");
    }
    else if (GameType == dod)
    {
        if (!HookEvent("dod_round_start",EventRoundChange,EventHookMode_PostNoCopy))
            SetFailState("Couldn't hook the dod_round_start event.");

        if (!HookEventEx("dod_round_active",EventRoundChange,EventHookMode_PostNoCopy))
            SetFailState("Could not hook the dod_round_active event.");

        if (!HookEventEx("dod_round_win",EventRoundChange,EventHookMode_PostNoCopy))
            SetFailState("Could not hook the dod_round_win event.");
    }
    else if (GameType == cstrike)
    {
        if (!HookEvent("round_start",EventRoundChange,EventHookMode_PostNoCopy))
            SetFailState("Couldn't hook the round_start event.");

        if (!HookEventEx("round_active",EventRoundChange,EventHookMode_PostNoCopy))
            SetFailState("Could not hook the round_active event.");

        if (!HookEventEx("round_end",EventRoundChange,EventHookMode_PostNoCopy))
            SetFailState("Could not hook the round_end event.");
    }

    m_HGRSourceAvailable = LibraryExists("hgrsource");
}

public OnLibraryAdded(const String:name[])
{
    if (StrEqual(name, "hgrsource"))
        m_HGRSourceAvailable = true;
}

public OnLibraryRemoved(const String:name[])
{
    if (StrEqual(name, "hgrsource"))
        m_HGRSourceAvailable = false;
}

public OnMapStart()
{
    SetupSound(errorWav, true, false, false);
    //SetupSound(buttonWav, false, false, false);
    SetupSound(deniedWav, true, false, false);
    SetupSound(lurkerWav, true, false, false);
    SetupSound(burrowUpWav, true, false, false);
    SetupSound(burrowDownWav, true, false, false);
    SetupSound(enterBunkerWav, true, false, false);
    SetupSound(leaveBunkerWav, true, false, false);
    SetupSound(burrowDeniedWav, true, false, false);
}

public OnPlayerAuthed(client)
{
    m_Burrowed[client] = 0;
    m_SavedArmor[client] = 0;
    m_BurrowArmor[client] = 0;
    m_BurrowTimer[client] = INVALID_HANDLE;
    m_BurrowedTimer[client] = INVALID_HANDLE;
    m_UnBurrowTimer[client] = INVALID_HANDLE;
    SetAttribute(client, Attribute_IsBurrowed,false);
}

public OnClientDisconnect(client)
{
    ResetBurrow(client, false, false, true);
    ResetClientStructures(client, false);
}

public OnEntityDestroyed(entity)
{
    if (m_Burrowed[entity] != 0)
    {
        new builder = (IsValidEdict(entity) &&
                       TF2_GetExtObjectType(entity) != TFExtObject_Unknown)
                      ? GetEntPropEnt(entity, Prop_Send, "m_hBuilder") : 0;

        ResetBurrowedStructure(builder, entity, false);
    }        
}

public Action:OnGrabPlayer(client, target)
{
    if (m_Burrowed[target] > 0)
    {
        PrepareSound(deniedWav);
        EmitSoundToClient(client,deniedWav);
        return Plugin_Stop;
    }
    return Plugin_Continue;
}

public Action:OnJetpack(client)
{
    if (m_Burrowed[client] > 0)
    {
        PrepareSound(deniedWav);
        EmitSoundToClient(client,deniedWav);
        return Plugin_Stop;
    }
    return Plugin_Continue;
}

public Action:OnPickupObject(client, builder, ent)
{
    if (m_Burrowed[ent] > 0)
    {
        PrepareSound(deniedWav);
        EmitSoundToClient(client,deniedWav);
        return Plugin_Stop;
    }
    return Plugin_Continue;
}

public Action:OnRaceDeselected(client,oldrace,race)
{
    ResetBurrow(client, true, false, false);
    ResetClientStructures(client, false);
    return Plugin_Continue;
}

// Events
public PlayerChangeClassEvent(Handle:event,const String:name[],bool:dontBroadcast)
{
    new client = GetClientOfUserId(GetEventInt(event,"userid"));
    if (IsValidClient(client))
        ResetBurrow(client, true, false, false);
}

public EventRoundChange(Handle:event,const String:name[],bool:dontBroadcast)
{
    for (new index=1;index<=MaxClients;index++)
    {
        ResetBurrow(index, false, false, false);
        ResetClientStructures(index, false);
    }
}

public OnPlayerSpawnEvent(Handle:event, client, race)
{
    if (IsValidClient(client))
        ResetBurrow(client, false, false, false);
}

public Action:BurrowPlayerHurtPreEvent(Handle:event,const String:name[],bool:dontBroadcast)
{
    new victim_index=GetClientOfUserId(GetEventInt(event,"userid"));
    new attacker_index=GetClientOfUserId(GetEventInt(event,"attacker"));
    new assister_index=GetClientOfUserId(GetEventInt(event,"assister"));

    TraceInto("Burrow", "BurrowPlayerHurtPreEvent", "attacker=%d:%N, assister=%d:%N, victim=%d:%N", \
              attacker_index, ValidClientIndex(attacker_index), \
              assister_index, ValidClientIndex(assister_index), \
              victim_index, ValidClientIndex(victim_index));

    if (assister_index > 0)
    {
        new level = m_BurrowLevel[assister_index];
        new burrowed = m_Burrowed[assister_index];
        if (level < 4 && burrowed > 0 &&
            burrowed >= m_BurrowDepth[assister_index])
        {
            Trace("%N assisted wounding %N while burrowed", \
                  assister_index,victim_index);

            if (level >= 2)
                UnBurrow(assister_index);
        }
    }

    if (attacker_index > 0 && attacker_index != victim_index)
    {
        new level = m_BurrowLevel[attacker_index];
        new burrowed = m_Burrowed[attacker_index];
        if (level < 4 && burrowed > 0 &&
            burrowed >= m_BurrowDepth[attacker_index])
        {
            decl String:weapon[64];
            new bool:is_equipment=GetWeapon(event,attacker_index,weapon,sizeof(weapon));
            new bool:melee=IsMelee(weapon,is_equipment,attacker_index,victim_index,200.0);

            Trace("%N wounded %N while burrowed, weapon=%d,melee=%d", \
                  attacker_index,victim_index,weapon,melee);

            if (level >= 2 || melee)
                UnBurrow(attacker_index);

            if (melee && level < 2)
            {
                new health=GetClientHealth(victim_index);
                new damage=GetDamage(event, victim_index);
                if (damage > 0)
                {
                    new newhp=health+damage;
                    new maxhp=GetMaxHealth(victim_index);
                    if (newhp > maxhp)
                        newhp = maxhp;

                    SetEntityHealth(victim_index,newhp);

                    Trace("%N was injured by %N, who was burrowed, restoring %d hp, health=%d,max=%d", \
                          victim_index, attacker_index, damage, newhp, maxhp);
                }
                else
                {
                    Trace("%N was attacked by %N, who was burrowed, without any damage", \
                          victim_index, attacker_index);
                }
            }

            TraceReturn();
            return Plugin_Handled;
        }
    }

    TraceReturn();
    return Plugin_Continue;
}

public BurrowPlayerDeathPreEvent(Handle:event,const String:name[],bool:dontBroadcast)
{
    new victim_index=GetClientOfUserId(GetEventInt(event,"userid"));

    TraceInto("Burrow", "BurrowPlayerDeathPreEvent", "victim=%d:%N", \
              victim_index, ValidClientIndex(victim_index));

    if (GameType == tf2)
    {
        // Skip feigned deaths.
        if (GetEventInt(event, "death_flags") & TF_DEATHFLAG_DEADRINGER)
        {
            TraceReturn("BurrowPlayerDeathPreEvent: %N Deadringer, not resetting burrow", \
                        ValidClientIndex(victim_index));
            return;
        }

        // Skip fishy deaths.
        if (GetEventInt(event, "weaponid") == TF_WEAPON_BAT_FISH &&
            GetEventInt(event, "customkill") != TF_CUSTOM_FISH_KILL)
        {
            TraceReturn("BurrowPlayerDeathPreEvent: %N Fishy, not resetting burrow", \
                        ValidClientIndex(victim_index));
            return;
        }
    }

    // Only ResetBurrow for real deaths, not deadringer or fishy ones.
    Trace("BurrowPlayerDeathPreEvent: %N, Resetting Burrow", \
          ValidClientIndex(victim_index));

    ResetBurrow(victim_index, false, true, false);

    new attacker_index=GetClientOfUserId(GetEventInt(event,"attacker"));
    if (attacker_index > 0 && attacker_index != victim_index)
    {
        new level = m_BurrowLevel[attacker_index];
        new burrowed = m_Burrowed[attacker_index];
        if (level < 4 && burrowed > 0 &&
            burrowed >= m_BurrowDepth[attacker_index])
        {
            decl String:weapon[64];
            new bool:is_equipment=GetWeapon(event,attacker_index,weapon,sizeof(weapon));
            if (level >= 2)
                UnBurrow(attacker_index);
            else if (IsMelee(weapon,is_equipment,attacker_index,victim_index))
            {
                ResetBurrow(attacker_index, false, false, false);
                KillPlayer(attacker_index);
            }
            //else it's probably a sentry
        }
    }

    TraceReturn();
}

public Action:BurrowTimer(Handle:timer,any:client)
{
    TraceInto("Burrow", "BurrowTimer", "client=%d:%N, m_BurrowLevel=%d, m_BurrowDepth=%d, m_Burrowed=%d", \
              client, ValidClientIndex(client), m_BurrowLevel[client], m_BurrowDepth[client], m_Burrowed[client]);

    new RoundStates:roundState = GetRoundState();
    if (roundState >= RoundActive && roundState < RoundOver &&
        IsValidClient(client) && IsPlayerAlive(client))
    {
        new level = m_BurrowLevel[client];

        if (GameType == tf2)
        {
            new bool:ubered = false;
            new pcond = TF2_GetPlayerConditionFlags(client);
            new numHealers = TF2_GetNumHealers(client);
            if (numHealers > 0 || TF2_IsUbercharged(pcond) || TF2_IsKritzkrieged(pcond) ||
                TF2_IsHealing(pcond) || TF2_GetHealingTarget(client, ubered) > 0 || ubered ||
                (m_HGRSourceAvailable && IsGrabbed(client)))
            {
                if (level >= 4)
                {
                    PrepareSound(deniedWav);
                    EmitSoundToAll(deniedWav, client);
                }
                else
                {
                    PrepareSound(burrowDeniedWav);
                    EmitSoundToAll(burrowDeniedWav, client);
                }

                m_BurrowTimer[client] = INVALID_HANDLE;

                if (m_Burrowed[client] > 0)
                    UnBurrow(client);

                TraceReturn("Burrow Stopped for %d:%N, m_Burrowed=%d, NumHealers=%d, IsUbered=%d, IsKritzkrieged=%d, IsHealing=%d, HealingTarget=%d, ubered=%d", \
                            client, ValidClientIndex(client), m_Burrowed[client], numHealers, TF2_IsUbercharged(pcond), TF2_IsKritzkrieged(pcond), \
                            TF2_IsHealing(pcond), TF2_GetHealingTarget(client, ubered), ubered);

                return Plugin_Stop;
            }
        }

        new Float:pos[3];
        GetClientAbsOrigin(client, pos);

        if (level < 4)
        {
            if (level >= 2)
            {
                PrepareSound(lurkerWav);
                EmitSoundToAll(lurkerWav, client);
            }
            else
            {
                PrepareSound(burrowDownWav);
                EmitSoundToAll(burrowDownWav, client);
            }
        }

        SetAttribute(client, Attribute_IsBurrowed,true);

        new depth = ++m_Burrowed[client];
        if (depth < m_BurrowDepth[client])
        {
            // If client just started burrowing
            if (depth <= 1)
            {
                m_BurrowLoc[client] = pos;
                SetOverrideSpeed(client, 0.0);
                SetEntityMoveType(client, MOVETYPE_NONE);

                Trace("%N Started Burrowing, depth=%d, m_BurrowLoc=(%f,%f,%f)", \
                      client, depth, pos[0], pos[1], pos[2]);
            }
            else
            {
                Trace("%N is Burrowing, depth=%d, pos=(%f,%f,%f)", \
                      client, depth, pos[0], pos[1], pos[2]);
            }

            // Client can't attack while burrowing
            SetEntPropFloat(client, Prop_Send, "m_flNextAttack", GetGameTime()+1.0);

            new Float:size[3];
            GetClientMaxs(client, size);

            pos[2] -= size[2] / 4.0;
            if (!TeleportPlayer(client, pos, NULL_VECTOR, NULL_VECTOR))
            {
                m_Burrowed[client]--;
                TraceError("Teleport Failed!");
            }

            TraceReturn("%N is Burrowing, pos=(%f,%f,%f)", \
                        client, pos[0], pos[1], pos[2]);

            return Plugin_Continue;
        }
        else
        {
            // The client has finished burrowing,
            if (depth >= 4)
            {
                // Bury the client up to his eyes (so he can still see).
                new Float:eyes[3];
                GetClientEyePosition(client, eyes);
                new Float:eyeDepth = eyes[2] - pos[2];

                pos = m_BurrowLoc[client];
                pos[2] -= eyeDepth;

                Trace("%N Finished Completely Burrowing, depth=%d, pos=(%f,%f,%f)", \
                      client, depth, pos[0], pos[1], pos[2]);

                if (!TeleportPlayer(client, pos, NULL_VECTOR, NULL_VECTOR))
                {
                    m_Burrowed[client]--;
                    TraceError("Teleport Failed!");
                    return Plugin_Continue;
                }
            }
            else
            {
                new Float:size[3];
                GetClientMaxs(client, size);

                pos[2] -= size[2] / 4.0;
                if (!TeleportPlayer(client, pos, NULL_VECTOR, NULL_VECTOR))
                {
                    m_Burrowed[client]--;
                    TraceError("Teleport Failed!");
                    return Plugin_Continue;
                }

                Trace("%N Finished Burrowing, depth=%d, pos=(%f,%f,%f)", \
                      client, depth, pos[0], pos[1], pos[2]);
            }

            new burrowArmor = m_BurrowArmor[client];
            if (burrowArmor > 0)
            {
                m_SavedArmor[client] = GetArmor(client);
                GetArmorPercent(client, m_SavedPercent[client]);
                GetArmorName(client, m_SavedName[client], sizeof(m_SavedName[]));

                TraceCat("Armor", "BurrowTimer: %N save armor=%d, percent=%1.2f,%1.2f, m_SavedName='%s', m_BurrowName='%s'", \
                         ValidClientIndex(client), m_SavedArmor[client], m_SavedPercent[client][0], \
                         m_SavedPercent[client][1], m_SavedName[client], m_BurrowName[client]);

                static const Float:oneHundredPercent[2] = { 1.0, 1.0 };
                SetArmorPercent(client, oneHundredPercent);
                IncrementArmor(client, burrowArmor);

                if (m_BurrowName[client][0])
                    SetArmorName(client, m_BurrowName[client]);
            }

            if (level < 2)
            {
                // Client can't attack while burrowed for levels < 2
                SetEntPropFloat(client, Prop_Send, "m_flNextAttack", GetGameTime()+1.0);
            }
            else
            {
                // Allow attacks for levels >= 2
                SetEntPropFloat(client, Prop_Send, "m_flNextAttack", GetGameTime());
            }

            if (level < 4)
            {
                Trace("Set InVisibility for %d:%N", \
                      client, ValidClientIndex(client));

                SetVisibility(client, BasicVisibility, 0, 0.1, 0.1,
                              RENDER_TRANSCOLOR, RENDERFX_NONE, .apply=true);

                m_BurrowedTimer[client] = CreateTimer(0.1, BurrowedTimer, client,
                                                      TIMER_REPEAT|TIMER_FLAG_NO_MAPCHANGE);
            }
        }
    }
    else
    {
        m_Burrowed[client] = 0;
        SetAttribute(client,Attribute_IsBurrowed,false);

        Trace("%d:%N attempted to burrow, but failed due to round state or status, m_Burrowed=%d, RoundState=%d, InGame=%d, IsAlive=%d", \
              client, ValidClientIndex(client), m_Burrowed[client], roundState, \
              IsValidClient(client), (IsValidClient(client) && IsPlayerAlive(client)));
    }

    TraceReturn("Burrowing stopped for %d:%N!", \
                client, ValidClientIndex(client));

    m_BurrowTimer[client] = INVALID_HANDLE;
    return Plugin_Stop;
}

public Action:BurrowedTimer(Handle:timer,any:client)
{
    TraceInto("Burrow", "BurrowedTimer", "client=%d:%N", \
              client, ValidClientIndex(client));

    new burrowed = m_Burrowed[client];
    new RoundStates:roundState = GetRoundState();
    if (burrowed > 0 && roundState >= RoundActive && roundState < RoundOver &&
        IsValidClient(client) && IsPlayerAlive(client))
    {
        new level = m_BurrowLevel[client];

        if (GameType == tf2)
        {
            new bool:ubered = false;
            new pcond = TF2_GetPlayerConditionFlags(client);
            if (TF2_IsUbercharged(pcond) || TF2_IsKritzkrieged(pcond) ||
                TF2_IsHealing(pcond) || TF2_GetHealingTarget(client, ubered) > 0 || ubered ||
                (m_HGRSourceAvailable && IsGrabbed(client)))
            {
                if (level >= 4)
                {
                    PrepareSound(deniedWav);
                    EmitSoundToAll(deniedWav, client);
                }
                else
                {
                    PrepareSound(burrowDeniedWav);
                    EmitSoundToAll(burrowDeniedWav, client);
                }

                m_BurrowedTimer[client] = INVALID_HANDLE;
                UnBurrow(client);

                TraceReturn();
                return Plugin_Stop;
            }
        }

        if (burrowed >= m_BurrowDepth[client])
        {
            if (level >= 2)
            {
                if (GetClientButtons(client) & IN_ATTACK)
                    UnBurrow(client);
                else
                {
                    TraceReturn();
                    return Plugin_Continue;
                }
            }
            else
            {
                SetEntPropFloat(client, Prop_Send, "m_flNextAttack", GetGameTime()+1.0);

                TraceReturn();
                return Plugin_Continue;
            }
        }
    }

    m_Burrowed[client] = 0;
    m_BurrowedTimer[client] = INVALID_HANDLE;
    SetAttribute(client,Attribute_IsBurrowed,false);

    TraceReturn();
    return Plugin_Stop;
}

public Action:UnBurrowTimer(Handle:timer,any:client)
{
    TraceInto("Burrow", "UnBurrowTimer", "client=%d:%N", \
              client, ValidClientIndex(client));

    SetAttribute(client,Attribute_IsBurrowed,false);

    new RoundStates:roundState = GetRoundState();
    if (roundState >= RoundActive && roundState < RoundOver &&
        IsValidClient(client) && IsPlayerAlive(client))
    {
        new Float:pos[3];
        GetClientAbsOrigin(client, pos);

        new level = m_BurrowLevel[client];
        if (level < 4)
        {
            if (level >= 2)
            {
                PrepareSound(lurkerWav);
                EmitSoundToAll(lurkerWav, client);
            }
            else
            {
                PrepareSound(burrowUpWav);
                EmitSoundToAll(burrowUpWav, client);
            }
        }

        if (m_BurrowArmor[client] > 0)
        {
            new savedArmor = m_SavedArmor[client];
            new armor = GetArmor(client);
            m_BurrowArmor[client] = 0;
            m_SavedArmor[client] = 0;

            TraceCat("Armor", "UnBurrowTimer: %N restore armor=%d, percent=%1.2f,%1.2f, m_SavedName='%s', m_BurrowName='%s'", \
                     ValidClientIndex(client), savedArmor, m_SavedPercent[client][0], \
                     m_SavedPercent[client][1], m_SavedName[client], m_BurrowName[client]);

            SetArmorName(client, m_SavedName[client]);
            SetArmorPercent(client, m_SavedPercent[client]);

            if (armor > savedArmor)
                SetArmorAmount(client, savedArmor);
        }

        if (--m_Burrowed[client] > 0)
        {
            new Float:angles[3];
            GetClientAbsAngles(client, angles);

            if (m_Burrowed[client] >= 3 || level >= 3)
            {
                Trace("Reset Visibility for %d:%N", \
                      client, ValidClientIndex(client));

                // Reset Visibility
                SetVisibility(client, NormalVisibility);
                SetAttribute(client, Attribute_IsBurrowed,false);
            }

            if (level != 3)
            {
                new Float:size[3];
                GetClientMaxs(client, size);

                pos[2] += size[2] / 4.0;
                if (!TeleportPlayer(client, pos, angles, NULL_VECTOR))
                {
                    m_Burrowed[client]++;
                    TraceError("Teleport Failed!");
                    return Plugin_Continue;
                }

                // Client can't attack while unburrowing
                SetEntPropFloat(client, Prop_Send, "m_flNextAttack", GetGameTime()+1.0);

                TraceReturn("%N is UnBurrowing, pos=(%f,%f,%f)", \
                            client, pos[0], pos[1], pos[2]);

                return Plugin_Continue;
            }
            //else Burst Forth
        }

        Trace("%N is fully UnBurrowed, m_BurrowLoc=(%f,%f,%f)", \
              client, m_BurrowLoc[client][0], m_BurrowLoc[client][1], m_BurrowLoc[client][2]);

        if (!TeleportPlayer(client, m_BurrowLoc[client], NULL_VECTOR, NULL_VECTOR))
        {
            m_Burrowed[client]++;
            TraceError("Teleport Failed!");
            return Plugin_Continue;
        }

        // Allow movement.
        SetEntityMoveType(client, MOVETYPE_WALK);

        // Allow attacks.
        SetEntPropFloat(client, Prop_Send, "m_flNextAttack", GetGameTime());
    }

    m_Burrowed[client] = 0;
    m_UnBurrowTimer[client] = INVALID_HANDLE;

    SetAttribute(client, Attribute_IsBurrowed,false);
    SetVisibility(client, NormalVisibility);
    SetOverrideSpeed(client, -1.0, true);

    TraceReturn("UnBurrowing stopped!");
    return Plugin_Stop;
}

Handle:UnBurrow(client)
{
    TraceInto("Burrow", "UnBurrow", "client=%d:%N", \
              client, ValidClientIndex(client));

    Trace("%d:%N has %s SavedArmor and %d BurrowArmor", \
          client, ValidClientIndex(client), m_SavedArmor[client], \
          m_BurrowArmor[client]);

    new Handle:timer = m_BurrowTimer[client];
    if (timer != INVALID_HANDLE)
    {
        m_BurrowTimer[client] = INVALID_HANDLE;
        KillTimer(timer);

        Trace("Killed %d's BurrowTimer", client);
    }

    timer = m_BurrowedTimer[client];
    if (timer != INVALID_HANDLE)
    {
        m_BurrowedTimer[client] = INVALID_HANDLE;
        KillTimer(timer);

        Trace("Killed %d's BurrowedTimer", client);
    }

    new level = m_BurrowLevel[client];
    if (level >= 4)
    {
        PrepareSound(leaveBunkerWav);
        EmitSoundToAll(leaveBunkerWav, client);
    }
    else if (level >= 2)
    {
        PrepareSound(lurkerWav);
        EmitSoundToAll(lurkerWav, client);
    }
    else
    {
        PrepareSound(burrowUpWav);
        EmitSoundToAll(burrowUpWav, client);
    }

    SetVisibility(client, NormalVisibility, .apply=true);
    SetAttribute(client, Attribute_IsBurrowed,false);

    if (m_BurrowArmor[client] > 0)
    {
        new savedArmor = m_SavedArmor[client];
        new armor = GetArmor(client);
        m_BurrowArmor[client] = 0;
        m_SavedArmor[client] = 0;

        TraceCat("Armor", "UnBurrow: %N's restore armor=%d, percent=%1.2f,%1.2f, m_SavedName='%s', m_BurrowName='%s'", \
                 ValidClientIndex(client), m_SavedArmor[client], m_SavedPercent[client][0], \
                 m_SavedPercent[client][1], m_SavedName[client], m_BurrowName[client]);

        SetArmorName(client, m_SavedName[client]);
        SetArmorPercent(client, m_SavedPercent[client]);

        if (armor > savedArmor)
            SetArmorAmount(client, savedArmor);
    }

    // Reset grabbed players immediately!
    if (m_HGRSourceAvailable && IsGrabbed(client))
        ResetBurrow(client, true, false, false);
    else
    {
        m_UnBurrowTimer[client] = CreateTimer(0.4, UnBurrowTimer, client,
                                              TIMER_REPEAT|TIMER_FLAG_NO_MAPCHANGE);
    }

    TraceReturn();
    return m_UnBurrowTimer[client];
}

ResetBurrow(client, bool:unburrow, bool:death, bool:disconnect)
{
    TraceInto("Burrow", "ResetBurrow", "client=%d:%N", \
              client, ValidClientIndex(client));

    new Handle:timer = m_BurrowTimer[client];
    if (timer != INVALID_HANDLE)
    {
        m_BurrowTimer[client] = INVALID_HANDLE;
        KillTimer(timer);

        Trace("Killed %d's BurrowTimer", client);
    }

    timer = m_BurrowedTimer[client];
    if (timer != INVALID_HANDLE)
    {
        m_BurrowedTimer[client] = INVALID_HANDLE;
        KillTimer(timer);

        Trace("Killed %d's BurrowedTimer", client);
    }

    timer = m_UnBurrowTimer[client];
    if (timer != INVALID_HANDLE)
    {
        m_UnBurrowTimer[client] = INVALID_HANDLE;
        KillTimer(timer);

        Trace("Killed %d's UnBurrowTimer", client);
    }

    if (m_Burrowed[client] != 0)
    {
        if (!disconnect && !death && IsValidClient(client) && IsPlayerAlive(client))
        {
            if (unburrow)
            {
                new RoundStates:roundState = GetRoundState();
                if (roundState >= RoundActive && roundState < RoundOver)
                {
                    TriggerTimer(UnBurrow(client),true);
                    TraceReturn();
                    return;
                }
            }

            new Float:pos[3];
            GetClientAbsOrigin(client, pos);
            if (pos[0] == m_BurrowLoc[client][0] &&
                pos[1] == m_BurrowLoc[client][1])
            {
                if (!TeleportPlayer(client, m_BurrowLoc[client], NULL_VECTOR, NULL_VECTOR))
                {
                    TraceError("Teleport Failed!");
                    TeleportEntity(client, m_BurrowLoc[client], NULL_VECTOR, NULL_VECTOR);
                }
            }
        }

        SetEntityMoveType(client, MOVETYPE_WALK);
        m_Burrowed[client] = 0;
    }

    if (m_BurrowArmor[client] > 0)
    {
        new savedArmor = m_SavedArmor[client];
        new armor = GetArmor(client);
        m_BurrowArmor[client] = 0;
        m_SavedArmor[client] = 0;

        TraceCat("Armor", "ResetBurrow: %N restore armor=%d, percent=%1.2f,%1.2f, m_SavedName='%s', m_BurrowName='%s'", \
                 ValidClientIndex(client), m_SavedArmor[client], m_SavedPercent[client][0], \
                 m_SavedPercent[client][1], savedArmor, m_BurrowName[client]);

        SetArmorName(client, m_SavedName[client]);
        SetArmorPercent(client, m_SavedPercent[client]);

        if (armor > savedArmor)
            SetArmorAmount(client, savedArmor);
    }

    TraceCat("Visibility", "Reset Burrow (Visibility) for %N", \
             ValidClientIndex(client));

    SetAttribute(client,Attribute_IsBurrowed,false);
    SetVisibility(client, NormalVisibility);
    SetOverrideSpeed(client, -1.0);
    ApplyPlayerSettings(client);

    TraceReturn();
}

/**
 * Starts burrowing, or unburrowing, a player depending on
 * if they are already burrowed
 *
 * @param client 	Client
 * @param level:    The level the client has in burrow
 *                  (1=strip weapons, 2=prevent attack, 3=allow attack, 4=unlimited attack - for Bunkers)
 * @param depth:    The maximum depth to burrow to
 *                  (4 is fully burrowed, 2 is 1/2 burrowed - for Bunkers)
 * @param armor:    The amount of additional armor provided (if any) - for Bunkers.
 * @param name:     What to call the additional armor in the HUD and messages - for Bunkers.
 * @return			Retruns true if the player succeeded in burrowing.
 * native bool:Burrow(client, level, depth=4, armor=0, const String:name[]="");
 */

public Native_Burrow(Handle:plugin,numParams)
{
    new client = GetNativeCell(1);
    new level = GetNativeCell(2);
    new depth = GetNativeCell(3);
    new armor = GetNativeCell(4);

    TraceInto("Burrow", "Native_Burrow", "client=%d:%N, level=%d, depth=%d, armor=%d", \
              client, ValidClientIndex(client), level, depth, armor);

    Trace("%d:%N has %s SavedArmor and %d BurrowArmor", \
          client, ValidClientIndex(client), m_SavedArmor[client], \
          m_BurrowArmor[client]);

    if (m_Burrowed[client] >= depth)
    {
        UnBurrow(client);

        Trace("Burrow Denied to %d:%N, m_Burrowed=%d >= depth=%d", \
              client, ValidClientIndex(client), m_Burrowed[client], depth);

        return false;
    }
    else if (m_Burrowed[client] <= 0)
    {
        new RoundStates:roundState = GetRoundState();
        if (roundState < RoundActive || roundState >= RoundOver ||
            !IsValidClient(client) || !IsPlayerAlive(client) ||
            GetRestriction(client,Restriction_PreventUltimates) ||
            GetRestriction(client,Restriction_Stunned) ||
            IsMole(client) || !(GetEntityFlags(client) & FL_ONGROUND) ||
            (m_HGRSourceAvailable && IsGrabbed(client)) ||
            (GameType == tf2 && TF2_HasTheFlag(client)))
        {
            if (level >= 4)
            {
                PrepareSound(deniedWav);
                EmitSoundToAll(deniedWav, client);
            }
            else
            {
                PrepareSound(burrowDeniedWav);
                EmitSoundToAll(burrowDeniedWav, client);
            }

            TraceReturn("Burrow Denied to %d:%N, m_Burrowed=%d, RoundState=%d, InGame=%d, IsAlive=%d, IsMole=%d, HasFlag=%d, OnGround=%d", \
                        client, ValidClientIndex(client), m_Burrowed[client], roundState, \
                        IsValidClient(client), (IsValidClient(client) && IsPlayerAlive(client)), \
                        IsMole(client), TF2_HasTheFlag(client), (GetEntityFlags(client) & FL_ONGROUND));

            return false;
        }
        else
        {
            if (GameType == tf2)
            {
                new bool:ubered = false;
                new pcond = TF2_GetPlayerConditionFlags(client);
                if (TF2_IsUbercharged(pcond) || TF2_IsKritzkrieged(pcond) ||
                    TF2_IsHealing(pcond) || TF2_GetHealingTarget(client, ubered) > 0 || ubered)
                {
                    if (level >= 4)
                    {
                        PrepareSound(deniedWav);
                        EmitSoundToAll(deniedWav, client);
                    }
                    else
                    {
                        PrepareSound(burrowDeniedWav);
                        EmitSoundToAll(burrowDeniedWav, client);
                    }

                    TraceReturn("Burrow Denied to %d:%N, m_Burrowed=%d, IsUbered=%d, IsKritzkrieged=%d, IsHealing=%d, HealingTarget=%d, ubered=%d", \
                                client, ValidClientIndex(client), m_Burrowed[client], TF2_IsUbercharged(pcond), TF2_IsKritzkrieged(pcond), \
                                TF2_IsHealing(pcond), TF2_GetHealingTarget(client, ubered), ubered);

                    return false;
                }
            }

            m_BurrowLevel[client] = level;
            m_BurrowDepth[client] = depth;
            m_BurrowArmor[client] = armor;

            GetNativeString(5,m_BurrowName[client],sizeof(m_BurrowName[]));

            if (level >= 4)
            {
                PrepareSound(enterBunkerWav);
                EmitSoundToAll(enterBunkerWav, client);
            }
            else if (level >= 2)
            {
                PrepareSound(lurkerWav);
                EmitSoundToAll(lurkerWav, client);
            }
            else
            {
                PrepareSound(burrowDownWav);
                EmitSoundToAll(burrowDownWav, client);
            }

            if (!GetRestriction(client,Restriction_PreventBurrow))
            {
                SetAttribute(client,Attribute_IsBurrowed,true);
                m_BurrowTimer[client] = CreateTimer(0.4, BurrowTimer, client,
                                                    TIMER_REPEAT|TIMER_FLAG_NO_MAPCHANGE);
            }

            new Handle:timer = m_UnBurrowTimer[client];
            if (timer != INVALID_HANDLE)
            {
                m_UnBurrowTimer[client] = INVALID_HANDLE;
                KillTimer(timer);

                Trace("Killed %d's UnBurrowTimer", client);
            }
        }
    }

    TraceReturn();
    return true;
}

/**
 * Starts unburrowing, a player if they are burrowed.
 *
 * @param client 	Client
 * @return			none
 * native UnBurrow(client);
 */

public Native_UnBurrow(Handle:plugin,numParams)
{
    new client = GetNativeCell(1);

    TraceInto("Burrow", "Native_UnBurrow", "client=%d:%N", \
              client, ValidClientIndex(client));

    Trace("%d:%N has %s SavedArmor and %d BurrowArmor", \
          client, ValidClientIndex(client), m_SavedArmor[client], \
          m_BurrowArmor[client]);

    if (m_Burrowed[client] > 0)
        UnBurrow(client);

    TraceReturn();
}

/**
 * Returns true if the entity or client is Burrowed.
 *
 * @param entity 	Entity (or client)
 * @return			4 if the entity is burrowed, 1-3 if the entity is burrowing
 *                  and -1 if the entity is being respawned by Burrow()
 * native IsBurrowed(entity);
 */

public Native_IsBurrowed(Handle:plugin,numParams)
{
    new entity = GetNativeCell(1);
    return m_Burrowed[entity];
}

/**
 * Reset a Burrowed Player.
 *
 * @param client 	Client
 * @param unborrow  Starts Unburrowing the player if true, instantly resets burrow if false.
 * @return			none
 * native ResetBurrow(client, bool:unburrow=false);
 */

public Native_ResetBurrow(Handle:plugin,numParams)
{
    new client = GetNativeCell(1);
    new bool:unburrow = bool:GetNativeCell(2);

    TraceInto("Burrow", "Native_ResetBurrow", "client=%d:%N, unburrow=%d", \
              client, ValidClientIndex(client), unburrow);

    ResetBurrow(client, unburrow, false, false);

    TraceReturn();
}

/**
 * Reset a Player's Burrowed Structures.
 *
 * @param client 	Client
 * @param unborrow  Starts Unburrowing the player's structures if true, instantly resets burrow if false.
 * @return			none
 * native ResetClientStructures(client, bool:unburrow=false);
 */

public Native_ResetClientStructures(Handle:plugin,numParams)
{
    new client = GetNativeCell(1);
    new bool:unburrow = bool:GetNativeCell(2);

    TraceInto("Burrow", "Native_ResetClientStructures", "client=%d:%N, unburrow=%d", \
              client, ValidClientIndex(client), unburrow);

    ResetClientStructures(client, unburrow);

    TraceReturn();
}

/**
 * Starts burrowing, or unburrowing, one or more structures that belong
 * to a particular client depending on if they are already burrowed
 *
 * @param client 	Client
 * @param amount 	Amount of energy required to Burrow one Structure.
 * @return			none
 * native BurrowStructure(client, amount);
 */

public Native_BurrowStructure(Handle:plugin,numParams)
{
    new client = GetNativeCell(1);
    new amount = GetNativeCell(2);
    new target = GetClientAimTarget(client, false);

    TraceInto("Burrow", "Native_BurrowStructure", "client=%d:%N, target=%d", \
              client, ValidClientIndex(client), target);

    if (target > 0)
    {
        new TFExtObjectType:type = TF2_GetExtObjectTypeFromClass(target);
        if (type < TFExtObject_Dispenser || type > TFExtObject_Sentry)
            target = 0;
    }

    if (target <= 0 || m_Burrowed[target] <= 0 || !UnBurrowStructure(client, target))
    {
        new energy = GetEnergy(client);

        if (target > 0)
        {
            if (energy < amount)
            {
                EmitEnergySoundToClient(client,Zerg);
                DisplayMessage(client, Display_Energy, "%t",
                               "InsufficientEnergyForBurrowStructure", amount);
            }
            else if (GetRestriction(client,Restriction_PreventUltimates) ||
                     GetRestriction(client,Restriction_Stunned))
            {
                PrepareSound(deniedWav);
                EmitSoundToClient(client,deniedWav);
                DisplayMessage(client, Display_Ultimate, "%t",
                               "PreventedFromBurrowingStructure");
            }
            else
                BurrowStructure(client, target, amount);
        }
        else
        {
            new Handle:menu=CreateMenu(BurrowStructure_Selected);
            SetMenuTitle(menu,"[SC] %T", "BurrowWhich", client);

            new counts[TFExtObjectType];
            new objectCount = AddBuildingsToMenu(menu, client, false, counts, target);

            if (objectCount > 1 && energy >= amount * objectCount)
                AddMenuItemT(menu, "0", "BurrowAll", client);

            // Added burrowed structures (if any)
            new burrowedCount = 0;
            new Handle:array = m_BurrowedStructures[client];
            if (array != INVALID_HANDLE)
            {
                decl String:buf[12], String:item[64];
                new size = GetArraySize(array);
                for (new i = 0; i < size; i++)
                {
                    new ent = GetArrayCell(array,i);
                    if (IsValidEdict(ent) && IsValidEntity(ent))
                    {
                        new TFExtObjectType:type=TF2_GetExtObjectType(ent, true);
                        if (type != TFExtObject_Unknown)
                        {
                            IntToString(EntIndexToEntRef(ent), buf, sizeof(buf));
                            Format(item,sizeof(item), "%t", "UnBurrow",
                                    TF2_ObjectNames[type], ent);

                            AddMenuItem(menu,buf,item);
                            burrowedCount++;
                            target = ent;
                        }
                    }
                }
            }

            if (burrowedCount > 1)
                AddMenuItemT(menu, "-1", "UnBurrowAll", client);

            if (objectCount + burrowedCount > 1)
            {
                m_BurrowEnergy[client] = amount;
                DisplayMenu(menu,client,MENU_TIME_FOREVER);
            }
            else if (burrowedCount == 1)
            {
                CancelMenu(menu);
                UnBurrowStructure(client, target);
            }
            else if (objectCount == 1)
            {
                CancelMenu(menu);
                BurrowStructure(client, target, amount);
            }
            else
            {
                CancelMenu(menu);
                PrepareSound(errorWav);
                EmitSoundToClient(client,errorWav);
                DisplayMessage(client, Display_Ultimate,
                               "%t", "NoStructuresToBurrow");
            }
        }
    }

    TraceReturn();
}

public BurrowStructure_Selected(Handle:menu,MenuAction:action,client,selection)
{
    if (action == MenuAction_Select)
    {
        PrepareSound(buttonWav);
        EmitSoundToClient(client,buttonWav);
        
        decl String:SelectionInfo[12];
        GetMenuItem(menu,selection,SelectionInfo,sizeof(SelectionInfo));

        new targetRef = StringToInt(SelectionInfo);
        if (targetRef == 0)
        {
            new amount = m_BurrowEnergy[client];
            new energy = GetEnergy(client);
            if (energy < amount)
            {
                EmitEnergySoundToClient(client,Zerg);
                DisplayMessage(client, Display_Energy, "%t",
                               "InsufficientEnergyForBurrowStructure",
                               amount);
            }
            else if (GetRestriction(client,Restriction_PreventUltimates) ||
                     GetRestriction(client,Restriction_Stunned))
            {
                PrepareSound(deniedWav);
                EmitSoundToClient(client,deniedWav);
                DisplayMessage(client, Display_Ultimate, "%t",
                               "PreventedFromBurrowingStructure");
            }
            else
            {
                BurrowObjects(client, amount, "obj_sentrygun");
                BurrowObjects(client, amount, "obj_dispenser");
                BurrowObjects(client, amount, "obj_teleporter");
            }
        }
        else if (targetRef == -1)
        {
            new Handle:array = m_BurrowedStructures[client];
            if (array != INVALID_HANDLE)
            {
                new size = GetArraySize(array);
                for (new i = 0; i < size; i++)
                {
                    UnBurrowStructure(client, GetArrayCell(array,i));
                }
            }
        }
        else
        {
            new targetEnt = EntRefToEntIndex(targetRef);
            if (targetEnt > 0 && IsValidEntity(targetEnt))
            {
                new Handle:array = m_BurrowedStructures[client];
                if (array != INVALID_HANDLE && FindValueInArray(array,targetEnt) > 0)
                    UnBurrowStructure(client, targetEnt);
                else
                    BurrowStructure(client, targetEnt, m_BurrowEnergy[client]);
            }
        }
    }
    else if (action == MenuAction_End)
        CloseHandle(menu);
}

BurrowObjects(client, amount, const String:ClassName[])
{
    new ent = -1;
    new Handle:array = m_BurrowedStructures[client];
    while ((ent = FindEntityByClassname(ent, ClassName)) != -1)
    {
        if (GetEntPropEnt(ent, Prop_Send, "m_hBuilder") == client &&
            GetEntPropFloat(ent, Prop_Send, "m_flPercentageConstructed") >= 1.0 &&
            !GetEntProp(ent, Prop_Send, "m_bHasSapper") &&
            !GetEntProp(ent, Prop_Send, "m_bDisabled"))
        {
            if (array == INVALID_HANDLE || FindValueInArray(array,ent) < 0)
                BurrowStructure(client, ent, amount);
        }
    }
}

ResetClientStructures(client, bool:unburrow)
{
    new Handle:array = m_BurrowedStructures[client];
    if (array != INVALID_HANDLE)
    {
        while (GetArraySize(array) > 0)
        {
            new ent = GetArrayCell(array,0);
            ResetBurrowedStructure(client, ent, unburrow);
        }
    }
}

ResetBurrowedStructure(client, target, bool:unburrow)
{
    TraceInto("Burrow", "ResetBurrowedStructure", "client=%d:%N, target=%d", \
              client, ValidClientIndex(client), target);

    new Handle:timer = m_BurrowTimer[target];
    if (timer != INVALID_HANDLE)
    {
        m_BurrowTimer[target] = INVALID_HANDLE;
        KillTimer(timer);

        Trace("Killed %d's BurrowStructureTimer", client);
    }

    timer = m_BurrowedTimer[target];
    if (timer != INVALID_HANDLE)
    {
        m_BurrowedTimer[target] = INVALID_HANDLE;
        KillTimer(timer);

        Trace("Killed %d's BurrowedStructureTimer", client);
    }

    if (m_Burrowed[target] != 0)
    {
        if (IsValidEntity(target) &&
            TF2_GetExtObjectType(target) != TFExtObject_Unknown)
        {
            new RoundStates:roundState = GetRoundState();
            if (roundState >= RoundActive && roundState < RoundOver &&
                IsValidClient(client))
            {
                if (unburrow)
                {
                    TriggerTimer(UnBurrowStructure(client,target),true);

                    TraceReturn();
                    return;
                }
                else
                    SetEntProp(target, Prop_Send, "m_bDisabled", 0); // Enable target.
            }
            TeleportEntity(target, m_BurrowLoc[target], NULL_VECTOR, NULL_VECTOR);
        }
        m_Burrowed[target] = 0;
    }

    new Handle:array = m_BurrowedStructures[client];
    if (array != INVALID_HANDLE)
    {
        new index = FindValueInArray(array,target);
        if (index >= 0)
            RemoveFromArray(array,index);
    }

    TraceReturn();
}

BurrowStructure(client, target, amount)
{
    if (target > 0 && IsValidEdict(target) && IsValidEntity(target))
    {
        if (TF2_GetExtObjectType(target) == TFExtObject_Unknown ||
            GetEntPropEnt(target, Prop_Send, "m_hBuilder") == client ||
            GetEntPropFloat(target, Prop_Send, "m_flPercentageConstructed") < 1.0 ||
            GetEntProp(target, Prop_Send, "m_bHasSapper") ||
            GetEntProp(target, Prop_Send, "m_bDisabled"))
        {
            PrepareSound(deniedWav);
            EmitSoundToClient(client,deniedWav);
            DisplayMessage(client, Display_Ultimate, "%t",
                           "InvalidTargetForBurrowStructure");
        }
        else if (GetRestriction(client,Restriction_PreventUltimates) ||
                 GetRestriction(client,Restriction_Stunned))
        {
            PrepareSound(deniedWav);
            EmitSoundToClient(client,deniedWav);
            DisplayMessage(client, Display_Ultimate, "%t",
                           "PreventedFromBurrowingStructure");
        }
        else
        {
            new energy = GetEnergy(client);
            if (energy < amount)
            {
                EmitEnergySoundToClient(client,Zerg);
                DisplayMessage(client, Display_Energy, "%t",
                               "InsufficientEnergyForBurrowStructure",
                               amount);
            }
            else
            {
                SetEnergy(client, energy-amount);

                new Handle:timer = m_BurrowedTimer[target];
                if (timer != INVALID_HANDLE)
                {
                    m_BurrowedTimer[target] = INVALID_HANDLE;
                    KillTimer(timer);
                }

                timer = m_UnBurrowTimer[target];
                if (timer != INVALID_HANDLE)
                {
                    m_UnBurrowTimer[target] = INVALID_HANDLE;
                    KillTimer(timer);
                }

                new Handle:array = m_BurrowedStructures[client];
                if (array == INVALID_HANDLE)
                    m_BurrowedStructures[client] = array = CreateArray();

                m_Burrowed[target] = 0;
                PushArrayCell(array,target);

                new Handle:pack;
                m_BurrowTimer[target] = CreateDataTimer(0.4, BurrowStructureTimer, pack,
                                                        TIMER_REPEAT|TIMER_FLAG_NO_MAPCHANGE);
                if (pack != INVALID_HANDLE)
                {
                    WritePackCell(pack, client);
                    WritePackCell(pack, target);
                    WritePackCell(pack, EntIndexToEntRef(target));
                }
            }
        }
    }
}

Handle:UnBurrowStructure(client, target)
{
    if (target > 0 && target <= GetMaxEntities() && m_Burrowed[target] > 0)
    {
        if (IsValidEdict(target) && IsValidEntity(target))
        {
            if (TF2_GetExtObjectType(target) != TFExtObject_Unknown)
            {
                new Handle:timer = m_BurrowTimer[target];
                if (timer != INVALID_HANDLE)
                {
                    m_BurrowTimer[target] = INVALID_HANDLE;
                    KillTimer(timer);
                }

                timer = m_BurrowedTimer[target];
                if (timer != INVALID_HANDLE)
                {
                    m_BurrowedTimer[target] = INVALID_HANDLE;
                    KillTimer(timer);

                    Trace("Killed %d's BurrowedStructureTimer", client);
                }

                new Handle:pack;
                m_UnBurrowTimer[target] = CreateDataTimer(0.4, UnBurrowStructureTimer, pack,
                                                          TIMER_REPEAT|TIMER_FLAG_NO_MAPCHANGE);
                if (pack != INVALID_HANDLE)
                {
                    WritePackCell(pack, client);
                    WritePackCell(pack, target);
                    WritePackCell(pack, EntIndexToEntRef(target));
                }
                return m_UnBurrowTimer[target];
            }
        }
    }
    return INVALID_HANDLE;
}

public Action:BurrowStructureTimer(Handle:timer,any:pack)
{
    if (pack != INVALID_HANDLE)
    {
        ResetPack(pack);
        new client=ReadPackCell(pack);
        new target=ReadPackCell(pack);
        new ref=ReadPackCell(pack);

        TraceInto("Burrow", "BurrowStructureTimer", "client=%d:%N, target=%d, ref=%s, m_Burrowed=%d", \
                  client, ValidClientIndex(client), target, ref, m_Burrowed[target]);

        new RoundStates:roundState = GetRoundState();
        if (roundState >= RoundActive && roundState < RoundOver &&
            IsValidClient(client) && EntRefToEntIndex(ref) == target)
        {
            if (!GetEntProp(target, Prop_Send, "m_bHasSapper"))
            {
                new Float:pos[3];
                GetEntPropVector(target, Prop_Send, "m_vecOrigin", pos);

                PrepareSound(burrowDownWav);
                EmitSoundToAll(burrowDownWav,target);

                new Float:size[3];
                GetEntPropVector(target, Prop_Send, "m_vecBuildMaxs", size);

                if ( ++m_Burrowed[target] < 4)
                {
                    // If target just started burrowing
                    if (m_Burrowed[target] <= 1)
                        m_BurrowLoc[target] = pos;

                    pos[2] -= size[2] / 4.0;
                    TeleportEntity(target, pos, NULL_VECTOR, NULL_VECTOR);

                    TraceReturn("%N is Burrowing, pos=(%f,%f,%f)", \
                                client, pos[0], pos[1], pos[2]);

                    return Plugin_Continue;
                }
                else
                {
                    m_BurrowTimer[target] = INVALID_HANDLE;
                    pos = m_BurrowLoc[target];
                    pos[2] -= size[2];
                    TeleportEntity(target, pos, NULL_VECTOR, NULL_VECTOR);
                    SetEntProp(target, Prop_Send, "m_bDisabled", 1); // Disable target.
                    SetEntityRenderMode(target, RENDER_NONE); // Make target invisible

                    new Handle:newpack;
                    m_BurrowTimer[target] = CreateDataTimer(0.1, BurrowedStructureTimer, newpack,
                                                            TIMER_REPEAT|TIMER_FLAG_NO_MAPCHANGE);
                    if (newpack != INVALID_HANDLE)
                    {
                        WritePackCell(newpack, client);
                        WritePackCell(newpack, target);
                        WritePackCell(newpack, ref);
                    }

                    TraceReturn("Burrow Structure Finished for %d:%N, target=%d, m_Burrowed=%d", \
                                client, ValidClientIndex(client), target, m_Burrowed[target]);

                    return Plugin_Stop;
                }
            }
        }

        m_BurrowTimer[target] = INVALID_HANDLE;
        ResetBurrowedStructure(client,target, true);

        TraceReturn("Burrowing Structure stopped for %d:%N, target=%d!", \
                    client, ValidClientIndex(client), target);
    }

    return Plugin_Stop;
}

public Action:BurrowedStructureTimer(Handle:timer,any:pack)
{
    if (pack != INVALID_HANDLE)
    {
        ResetPack(pack);
        new client=ReadPackCell(pack);
        new target=ReadPackCell(pack);
        new ref=ReadPackCell(pack);

        TraceInto("Burrow", "BurrowedStructureTimer", "client=%d:%N, target=%d, ref=%x", \
                  client, ValidClientIndex(client), target, ref);

        new RoundStates:roundState = GetRoundState();
        if (roundState >= RoundActive && roundState < RoundOver &&
            m_Burrowed[target] > 0 && EntRefToEntIndex(ref) == target &&
            IsValidClient(client))
        {
            // Make sure target remains disabled!
            SetEntProp(target, Prop_Send, "m_bDisabled", 1);

            TraceReturn();
            return Plugin_Continue;
        }

        m_BurrowedTimer[target] = INVALID_HANDLE;

        TraceReturn();
    }

    return Plugin_Stop;
}

public Action:UnBurrowStructureTimer(Handle:timer,any:pack)
{
    if (pack != INVALID_HANDLE)
    {
        ResetPack(pack);
        new client=ReadPackCell(pack);
        new target=ReadPackCell(pack);
        new ref=ReadPackCell(pack);

        new RoundStates:roundState = GetRoundState();
        if (roundState >= RoundActive && roundState < RoundOver &&
            m_Burrowed[target] > 0 && IsValidClient(client) &&
            EntRefToEntIndex(ref) == target)
        {
            if (GetEntPropEnt(target, Prop_Send, "m_hBuilder") == client &&
                GetEntPropFloat(target, Prop_Send, "m_flPercentageConstructed") >= 1.0)
            {
                PrepareSound(burrowUpWav);
                EmitSoundToAll(burrowUpWav,target);

                if (--m_Burrowed[target] > 0)
                {
                    if (m_Burrowed[target] >= 3)
                        SetEntityRenderMode(target, RENDER_NORMAL); // Make target visible

                    new Float:pos[3];
                    GetEntPropVector(target, Prop_Send, "m_vecOrigin", pos);

                    new Float:size[3];
                    GetEntPropVector(target, Prop_Send, "m_vecBuildMaxs", size);

                    pos[2] += size[2] / 4.0;
                    TeleportEntity(target, pos, NULL_VECTOR, NULL_VECTOR);
                    return Plugin_Continue;
                }
                else
                {
                    SetEntProp(target, Prop_Send, "m_bDisabled", 0); // Enable target.
                    TeleportEntity(target, m_BurrowLoc[target], NULL_VECTOR, NULL_VECTOR);
                }
            }
        }

        m_UnBurrowTimer[target] = INVALID_HANDLE;
        ResetBurrowedStructure(client,target, false);
    }
    return Plugin_Stop;
}
