/**
* vim: set ai et ts=4 sw=4 syntax=sourcepawn :
* =============================================================================
* File: Hallucinate.sp
* Description: The Hallucinate upgrade for SourceCraft, based on:
*              SourceMod Basefuncommands Plugin
*              to Provide drug functionality
*
* SourceMod (C)2004-2007 AlliedModders LLC.  All rights reserved.
* =============================================================================
*
* This program is free software; you can redistribute it and/or modify it under
* the terms of the GNU General Public License, version 3.0, as published by the
* Free Software Foundation.
* 
* This program is distributed in the hope that it will be useful, but WITHOUT
* ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
* FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
* details.
*
* You should have received a copy of the GNU General Public License along with
* this program.  If not, see <http://www.gnu.org/licenses/>.
*
* As a special exception, AlliedModders LLC gives you permission to link the
* code of this program (as well as its derivative works) to "Half-Life 2," the
* "Source Engine," the "SourcePawn JIT," and any Game MODs that run on software
* by the Valve Corporation.  You must obey the GNU General Public License in
* all respects for all other code used.  Additionally, AlliedModders LLC grants
* this exception to all derivative works.  AlliedModders LLC defines further
* exceptions, found in LICENSE.txt (as of this writing, version JULY-31-2007),
* or <http://www.sourcemod.net/license.php>.
*
* Version: $Id: drug.sp 1833 2007-12-28 16:46:42Z ferret $
*/

#pragma semicolon 1

#include <sourcemod>
#include <sdktools>
#include <gametype>

#undef REQUIRE_EXTENSIONS
#include <tf2_stocks>
#define REQUIRE_EXTENSIONS

#define DRUG_RANDOM     0
#define DRUG_SOURCEMOD  1
#define DRUG_DIZZY      2
#define DRUG_CRAZY      3

new g_DrugType[MAXPLAYERS+1];
new Float:g_flMagnitude[MAXPLAYERS+1];
new Handle:g_DrugTimers[MAXPLAYERS+1];
new const Float:g_DrugAngles[20] = {0.0, 5.0, 10.0, 15.0, 20.0, 25.0,
                                    20.0, 15.0, 10.0, 5.0, 0.0, -5.0,
                                    -10.0, -15.0, -20.0, -25.0, -20.0,
                                    -15.0, -10.0, -5.0};

// UserMessageId for Fade.
new UserMsg:g_FadeUserMsgId;

// Offset for m_vecPunchAngle
new g_offsPunchAngle;

public Plugin:myinfo = 
{
    name = "SourceCraft Upgrade - Hallucinate",
    author = "-=|JFH|=- Naris with code from the SourceMod Team",
    description = "Native interface to the Hallucinate upgrade for SourceCraft.",
    version = "2.0",
    url = "http://sourcemod.net/"
};

public APLRes:AskPluginLoad2(Handle:myself, bool:late, String:error[], err_max)
{
    CreateNative("PerformDrug",Native_PerformDrug);
    CreateNative("PerformBlind",Native_PerformBlind);
    RegPluginLibrary("Hallucinate");

    return APLRes_Success;
}

public OnPluginStart()
{
    g_FadeUserMsgId = GetUserMessageId("Fade");

    g_offsPunchAngle = FindSendPropInfo("CBasePlayer", "m_vecPunchAngle");
    if (g_offsPunchAngle == -1)
        SetFailState("Couldn't find \"m_vecPunchAngle\"!");

    if(!HookEventEx("player_death",Event_PlayerDeath))
        SetFailState("Could not hook the player_death event.");

    if (GetGameType() == tf2)
    {
        if (!HookEventEx("teamplay_round_start",Event_RoundEnd,EventHookMode_PostNoCopy))
            SetFailState("Could not hook the teamplay_round_start event.");

        if (!HookEventEx("teamplay_round_active",Event_RoundEnd,EventHookMode_PostNoCopy))
            SetFailState("Could not hook the teamplay_round_active event.");

        if(!HookEventEx("teamplay_round_win",Event_RoundEnd, EventHookMode_PostNoCopy))
            SetFailState("Couldn't hook the teamplay_round_win event.");

        if(!HookEventEx("teamplay_round_stalemate",Event_RoundEnd, EventHookMode_PostNoCopy))
            SetFailState("Couldn't hook the teamplay_round_stalemate event.");

        if (!HookEventEx("arena_round_start",Event_RoundEnd,EventHookMode_PostNoCopy))
            SetFailState("Could not hook the arena_round_start event.");

        if (!HookEventEx("arena_win_panel",Event_RoundEnd,EventHookMode_PostNoCopy))
            SetFailState("Could not hook the arena_win_panel event.");
    }
    else if (GameType == dod)
    {
        if (!HookEventEx("dod_round_start",Event_RoundEnd,EventHookMode_PostNoCopy))
            SetFailState("Could not hook the dod_round_start event.");

        if (!HookEventEx("dod_round_active",Event_RoundEnd,EventHookMode_PostNoCopy))
            SetFailState("Could not hook the dod_round_active event.");

        if (!HookEventEx("dod_round_win",Event_RoundEnd,EventHookMode_PostNoCopy))
            SetFailState("Could not hook the dod_round_win event.");
    }
    else if (GameType == cstrike)
    {
        if (!HookEventEx("round_start",Event_RoundEnd,EventHookMode_PostNoCopy))
            SetFailState("Could not hook the round_start event.");

        if (!HookEventEx("round_active",Event_RoundEnd,EventHookMode_PostNoCopy))
            SetFailState("Could not hook the round_active event.");

        if (!HookEvent("round_end", Event_RoundEnd, EventHookMode_PostNoCopy))
            LogError("Couldn't hook the round_end event.");
    }
}

public OnMapEnd()
{
    KillAllDrugs();
}

public OnClientDisconnect(client)
{
    PerformDrug(client, 0, 0, 0.0);
}

public Action:Event_PlayerDeath(Handle:event,const String:name[],bool:dontBroadcast)
{
    if (GameType == tf2)
    {
        // Skip feigned deaths.
        if (GetEventInt(event, "death_flags") & TF_DEATHFLAG_DEADRINGER)
            return Plugin_Handled;

        // Skip fishy deaths.
        if (GetEventInt(event, "weaponid") == TF_WEAPON_BAT_FISH &&
            GetEventInt(event, "customkill") != TF_CUSTOM_FISH_KILL)
        {
            return Plugin_Handled;
        }
    }

    new client=GetClientOfUserId(GetEventInt(event,"userid"));
    KillDrug(client);
    return Plugin_Handled;
}

public Action:Event_RoundEnd(Handle:event,const String:name[],bool:dontBroadcast)
{
    KillAllDrugs();
    return Plugin_Handled;
}

CreateDrug(client, type, Float:magnitude)
{
    g_DrugType[client] = type;
    g_flMagnitude[client] = magnitude;
    g_DrugTimers[client] = CreateTimer(1.0, Timer_Drug, client, TIMER_REPEAT);	
}

KillDrug(client)
{
    KillDrugTimer(client);

    new Float:pos[3];
    GetClientAbsOrigin(client, pos);
    new Float:angs[3];
    GetClientEyeAngles(client, angs);

    angs[2] = 0.0;

    TeleportEntity(client, pos, angs, NULL_VECTOR);	

    new clients[2];
    clients[0] = client;	

    new Handle:message = StartMessageEx(g_FadeUserMsgId, clients, 1);
    BfWriteShort(message, 1536);
    BfWriteShort(message, 1536);
    BfWriteShort(message, (0x0001 | 0x0010));
    BfWriteByte(message, 0);
    BfWriteByte(message, 0);
    BfWriteByte(message, 0);
    BfWriteByte(message, 0);
    EndMessage();	

    ClientCommand(client, "r_screenoverlay 0");
}

KillDrugTimer(client)
{
    if (g_DrugTimers[client] != INVALID_HANDLE)
    {
        KillTimer(g_DrugTimers[client]);
        g_DrugTimers[client] = INVALID_HANDLE;	
    }
}

KillAllDrugs()
{
    for (new i = 1; i <= MaxClients; i++)
    {
        if(IsClientInGame(i))
            KillDrug(i);
        else
            KillDrugTimer(i);
    }
}

bool:PerformDrug(target, toggle, type, Float:magnitude)
{
    switch (toggle)
    {
        case (2):
        {
            if (g_DrugTimers[target] == INVALID_HANDLE)
            {
                CreateDrug(target, type, magnitude);
                return true;
            }
            else
                KillDrug(target);
        }

        case (1):
        {
            if (g_DrugTimers[target] == INVALID_HANDLE)
            {
                CreateDrug(target, type, magnitude);
                return true;
            }
            else
            {
                g_DrugType[target] = type;
                g_flMagnitude[target] = magnitude;
            }
        }

        case (0):
        {
            if (g_DrugTimers[target] != INVALID_HANDLE)
                KillDrug(target);
        }
    }
    return false;
}

PerformBlind(target, amount)
{
	new targets[2];
	targets[0] = target;
	
	new Handle:message = StartMessageEx(g_FadeUserMsgId, targets, 1);
	BfWriteShort(message, 1536);
	BfWriteShort(message, 1536);
	
	if (amount == 0)
		BfWriteShort(message, (0x0001 | 0x0010));
	else
		BfWriteShort(message, (0x0002 | 0x0008));
	
	BfWriteByte(message, 0);
	BfWriteByte(message, 0);
	BfWriteByte(message, 0);
	BfWriteByte(message, amount);
	
	EndMessage();
}

public Action:Timer_Drug(Handle:timer, any:client)
{
    if (!IsClientInGame(client))
    {
        KillDrugTimer(client);
        return Plugin_Handled;
    }
    else if (!IsPlayerAlive(client))
    {
        KillDrug(client);
        return Plugin_Handled;
    }

    new type = g_DrugType[client];
    if (type == DRUG_RANDOM)
        type = GetRandomInt(DRUG_SOURCEMOD,DRUG_CRAZY);

    switch (type)
    {
        case DRUG_SOURCEMOD:
        {
            new Float:pos[3];
            GetClientAbsOrigin(client, pos);

            new Float:angs[3];
            GetClientEyeAngles(client, angs);

            angs[2] = g_DrugAngles[GetRandomInt(0,100) % 20];

            TeleportEntity(client, pos, angs, NULL_VECTOR);

            new clients[2];
            clients[0] = client;	

            new Handle:message = StartMessageEx(g_FadeUserMsgId, clients, 1);
            BfWriteShort(message, 255);
            BfWriteShort(message, 255);
            BfWriteShort(message, (0x0002));
            BfWriteByte(message, GetRandomInt(0,255));
            BfWriteByte(message, GetRandomInt(0,255));
            BfWriteByte(message, GetRandomInt(0,255));
            BfWriteByte(message, 128);
            EndMessage();
        }
        case DRUG_DIZZY:
        {
            new Float:vecPunch[3];
            vecPunch[0] = GetRandomFloat(g_flMagnitude[client] * -1, g_flMagnitude[client]);
            vecPunch[1] = GetRandomFloat(g_flMagnitude[client] * -1, g_flMagnitude[client]);
            vecPunch[2] = GetRandomFloat(g_flMagnitude[client] * -1, g_flMagnitude[client]);
            SetEntDataVector(client, g_offsPunchAngle, vecPunch);
        }
        case DRUG_CRAZY:
        {
            ClientCommand(client, "r_screenoverlay effects/tp_eyefx/tp_eyefx.vmt");
        }
    }

    return Plugin_Handled;
}

public Native_PerformDrug(Handle:plugin,numParams)
{
    return PerformDrug(GetNativeCell(1),GetNativeCell(2),
                       GetNativeCell(3),Float:GetNativeCell(4));
}

public Native_PerformBlind(Handle:plugin,numParams)
{
    PerformBlind(GetNativeCell(1),GetNativeCell(2));
}
