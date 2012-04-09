/**
 * vim: set ai et ts=4 sw=4 :
 * File: piggyback.sp
 * Description: Allows players to piggyback another player!
 * Author(s): Mecha the Slag
 */

#pragma semicolon 1

//Includes:
#include <sourcemod>
#include <sdktools>
#include <sdkhooks>
#include <colors>

#define PLUGIN_VERSION "2.4"

#undef REQUIRE_EXTENSIONS
#include <tf2>
#include <tf2_stocks>

// Define the PiggyMethod bits
enum PiggyMethod (<<= 1)
{
    PiggyMethod_Default = -1,
    PiggyMethod_None = 0,
    PiggyMethod_ForceView = 1,
    PiggyMethod_DisableAttack,
    PiggyMethod_AllowSpys,
    PiggyMethod_SharedFate,
    PiggyMethod_Pickup,
    PiggyMethod_Enable
}

new g_piggy[MAXPLAYERS+1];
new Float:g_distance[MAXPLAYERS+1];
new PiggyMethod:g_method[MAXPLAYERS+1];

new String:infoText[512];

new bool:IsTF2 = false;
new bool:gNativeControl = false;
new gVelocityOffset;

// convars
new Handle:pb_distance;
new Handle:pb_method;
new Handle:pb_enable;
new Handle:pb_info;

// forwards
new Handle:fwdOnPiggyback;

static const String:TF2Weapons[][]={"tf_weapon_fists", "tf_weapon_shovel", "tf_weapon_bat",
                                    "tf_weapon_fireaxe", "tf_weapon_bonesaw", "tf_weapon_bottle",
                                    "tf_weapon_sword", "tf_weapon_club", "tf_weapon_wrench"};

public Plugin:myinfo = {
    name = "Piggyback",
    author = "Mecha the Slag",
    description = "Allows players to piggyback another player!",
    version = PLUGIN_VERSION,
    url = "http://mechaware.net/"
};

public APLRes:AskPluginLoad2(Handle:myself, bool:late, String:error[], err_max)
{
    // Register Natives
    CreateNative("ControlPiggyback",Native_ControlPiggyback);
    CreateNative("GivePiggyback",Native_GivePiggyback);
    CreateNative("TakePiggyback",Native_TakePiggyback);
    CreateNative("Piggyback",Native_Piggyback);

    // Register Forwards
    fwdOnPiggyback=CreateGlobalForward("OnPlayerPiggyback",ET_Hook,Param_Cell,Param_Cell,Param_Cell,Param_Float);

    RegPluginLibrary("piggyback");
    return APLRes_Success;
}

public OnPluginStart()
{
    // G A M E  C H E C K //
    decl String:game[32];
    GetGameFolderName(game, sizeof(game));
    if (StrEqual(game, "tf"))
    {
        IsTF2 = true;
        strcopy(infoText, sizeof(infoText), "{green}You can piggyback teammates by right-clicking them with your melee out!{default}");

        HookEvent("teamplay_flag_event", Flag_Event);
    }
    else
    {
        strcopy(infoText, sizeof(infoText), "{green}You can piggyback teammates by right-clicking them!{default}");
    }

    HookEvent("player_spawn", Player_Spawn);
    HookEvent("player_death", Player_Death);

    pb_distance = CreateConVar("pb_distance", "142.0", "Distance from which someone can be piggybacked", FCVAR_PLUGIN);
    pb_method = CreateConVar("pb_method", "6", "Method to handle a piggybacking player. 1=force view, 2=disable shooting, 3=force view & disable shooting, 4=allow spays, 8=passenger dies with carrier, 16=client picks up target instead of jumps on target, 0=do nothing (inaccurate aim)", FCVAR_PLUGIN);
    pb_enable = CreateConVar("pb_enable", "1", "Enable piggybacking", FCVAR_PLUGIN);
    pb_info = CreateConVar("pb_info", "120.0", "Time interval in seconds between notifications (0 for none)", FCVAR_PLUGIN);

    CreateConVar("pb_version", PLUGIN_VERSION, "Piggyback Version", FCVAR_DONTRECORD|FCVAR_PLUGIN|FCVAR_NOTIFY);

    if (GetConVarFloat(pb_info) > 0.0)
        CreateTimer(GetConVarFloat(pb_info), Notification);

    for (new i = 0; i <= MaxClients; i++)
    {
        g_piggy[i] = -1;
    }

    gVelocityOffset = FindSendPropInfo("CBasePlayer", "m_vecVelocity[0]");
}

public Action:Notification(Handle:hTimer)
{
    if (GetConVarFloat(pb_info) > 0.0)
    {
        if (!gNativeControl && GetConVarBool(pb_enable))
            CPrintToChatAll(infoText); 

        CreateTimer(GetConVarFloat(pb_info), Notification);
    }
    return Plugin_Stop;
}

public Player_Spawn(Handle:event, const String:name[], bool:dontBroadcast)
{
    new client = GetClientOfUserId(GetEventInt(event, "userid"));
    RemovePiggy(client, true);

    for (new i = 1; i <= MaxClients; i++)
    {
        if (g_piggy[i] == client)
            RemovePiggy(i, true);
    }

    if (!gNativeControl)
    {
        if (GetConVarBool(pb_enable))
        {
            g_distance[client] = GetConVarFloat(pb_distance);
            g_method[client]  = PiggyMethod:GetConVarInt(pb_method);
            g_method[client] |= PiggyMethod_Enable;
        }
        else
        {
            g_distance[client] = 0.0;
            g_method[client] = PiggyMethod_None;
        }
    }
}

public Player_Death(Handle:event, const String:name[], bool:dontBroadcast)
{
    new client = GetClientOfUserId(GetEventInt(event, "userid"));
    RemovePiggy(client, true);

    for (new i = 1; i <= MaxClients; i++)
    {
        if (g_piggy[i] == client)
        {
            RemovePiggy(i, true);
            if (g_method[client] & PiggyMethod_SharedFate)
                ForcePlayerSuicide(i);
        }
    }
}

public Flag_Event(Handle:event,const String:name[],bool:dontBroadcast)
{
    new player = GetEventInt(event,"player");
    if (player > 0 && IsClientInGame(player))
    {
        if (GetEventInt(event,"eventtype") == 1)
        {
            // Flag Picked Up
            RemovePiggy(player, true);

            for (new i = 1; i <= MaxClients; i++)
            {
                if (g_piggy[i] == player)
                    RemovePiggy(i, true);
            }
        }
    }
}

public OnClientPutInServer(client)
{
    if (!(IsFakeClient(client)))
        SDKHook(client, SDKHook_PreThink, OnPreThink);

    g_piggy[client] = -1;
}

public OnClientDisconnect(client)
{
    if (!(IsFakeClient(client)))
        SDKUnhook(client, SDKHook_PreThink, OnPreThink);

    RemovePiggy(client, true);
    for (new i = 1; i <= MaxClients; i++)
    {
        if (g_piggy[i] == client)
            RemovePiggy(i, true);
    }
}

public OnPreThink(client)
{
    new iButtons = GetClientButtons(client);
    
    if ((iButtons & IN_ATTACK2) &&
        (g_method[client] & PiggyMethod_Enable))
    {
        new bool:go = false;

        if (IsTF2)
        {
            decl String:Weapon[128];
            GetClientWeapon(client, Weapon, sizeof(Weapon));
            for (new i = 0; i < sizeof(TF2Weapons); i++)
            {
                if (StrEqual(Weapon,TF2Weapons[i],false))
                {
                    go = true;
                    break;
                }
            }
        }
        else
            go = true;

        if (go)
            TraceTarget(client);

        //if (!(go)) PrintToChatAll("Weapon: %s", Weapon);
    }
    
    new parent = g_piggy[client];
    if (parent > -1)
    {
        if (iButtons & IN_JUMP)
            RemovePiggy(client, false);
        else
        {
            new PiggyMethod:method = g_method[parent];
            if (!IsTF2)
            {
                if (method & PiggyMethod_ForceView)
                {
                    decl Float:vecClientEyeAng[3];
                    GetClientEyeAngles(g_piggy[client], vecClientEyeAng); // Get the angle the player is looking
                    TeleportEntity(client, NULL_VECTOR, vecClientEyeAng, NULL_VECTOR);
                }
            }

            if (method & PiggyMethod_DisableAttack)
            {
                iButtons &= ~(IN_ATTACK|IN_ATTACK2);
                SetEntProp(client, Prop_Data, "m_nButtons", iButtons);
            }
        }
    }
}

// terribad (patent pending)
// Hack around new limitation of effectivly no longer being
// able to parent entities to players, so fake it.
public OnGameFrame()
{
    if (IsTF2)
    {
        for(new client = 1; client <= MaxClients; client++)
        {
            new parent = g_piggy[client];
            if (parent > 0)
            {
                decl Float:vecClientEyePos[3];
                GetClientEyePosition(parent, vecClientEyePos); // Get the player's location

                decl Float:vecVelocity[3];
                GetEntDataVector(parent, gVelocityOffset, vecVelocity);

                if (g_method[parent] & PiggyMethod_ForceView)
                {
                    decl Float:vecClientEyeAng[3];
                    GetClientEyeAngles(parent, vecClientEyeAng); // Get the angle the player is looking
                    TeleportEntity(client, vecClientEyePos, vecClientEyeAng, vecVelocity);
                }
                else
                {
                    TeleportEntity(client, vecClientEyePos, NULL_VECTOR, vecVelocity);
                }
            }
        }
    }
}

public Piggy(entity, other)
{
    if (entity != other && entity <= MaxClients && other <= MaxClients &&
        g_piggy[entity] <= -1 && g_piggy[other] <= -1 && g_piggy[other] != entity &&
        IsPlayerAlive(entity) && IsPlayerAlive(other))
    {
        new team = GetClientTeam(entity);
        if (GetClientTeam(other) != team)
            return;

        if ((g_method[entity] & PiggyMethod_Pickup))
        {
            // entity is actually attempting to pickup other
            new temp = entity;
            entity = other;
            other = temp;
        }

        if (IsTF2)
        {
            if (!(g_method[other] & PiggyMethod_AllowSpys) &&
                TF2_GetPlayerClass(other) == TFClass_Spy)
            {
                return;
            }
            else
            {
                // Don't allow flag carrier to be involved.
                new flagCarrier = TF2_GetFlagCarrier(team);
                if (flagCarrier == entity || flagCarrier == other)
                    return;
            }
        }

        decl Float:PlayerVec[3];
        GetClientAbsOrigin(other, PlayerVec);

        decl Float:PlayerVec2[3];
        GetClientAbsOrigin(entity, PlayerVec2);

        new Float:distance = GetVectorDistance(PlayerVec2, PlayerVec);
        if (distance <= g_distance[other])
        {
            // Check with other plugins/forward (if any)
            new Action:res = Plugin_Continue;
            Call_StartForward(fwdOnPiggyback);
            Call_PushCell(entity);
            Call_PushCell(other);
            Call_PushCell(true);
            Call_PushFloat(distance);
            Call_Finish(res);

            if (res == Plugin_Continue)
            {
                static const Float:vecClientVel[3] = {0.0, 0.0, 0.0 };

                decl Float:vecClientEyeAng[3];
                GetClientEyeAngles(other, vecClientEyeAng); // Get the angle the player is looking

                CPrintToChatEx(other, entity, "{teamcolor}%N{default} is on your back", entity);
                CPrintToChatEx(entity, other, "You are piggybacking {teamcolor}%N{default}, hit the {green}JUMP{default} key (space) to jump off!", other);

                PlayerVec[2] -= 60;
                TeleportEntity(entity, PlayerVec, vecClientEyeAng, vecClientVel);

                g_piggy[entity] = other;

                if (!IsTF2)
                {
                    decl String:tName[32];
                    GetEntPropString(other, Prop_Data, "m_iName", tName, sizeof(tName));
                    DispatchKeyValue(entity, "parentname", tName);

                    SetVariantString("!activator");
                    AcceptEntityInput(entity, "SetParent", other, other, 0);

                    if (IsTF2)
                        SetVariantString("flag");
                    else
                        SetVariantString("forward");

                    AcceptEntityInput(entity, "SetParentAttachment", other, other, 0);
                }
            }
        }
    }
}

public RemovePiggy(entity, bool:force)
{
    if (entity > 0 && entity <= MaxClients)
    {
        new other = g_piggy[entity];
        if (other > 0)
        {
            // Check with other plugins/forward (if any)
            new Action:res = Plugin_Continue;
            Call_StartForward(fwdOnPiggyback);
            Call_PushCell(entity);
            Call_PushCell(other);
            Call_PushCell(false);
            Call_PushFloat(0.0);
            Call_Finish(res);

            if (force || res == Plugin_Continue)
            {
                if (IsPlayerAlive(other))
                    CPrintToChatEx(other, entity, "{teamcolor}%N{default} jumped off your back", entity);

                if (!IsTF2)
                    AcceptEntityInput(entity, "SetParent", -1, -1, 0);

                SetEntityMoveType(entity, MOVETYPE_WALK);

                g_piggy[entity] = -1;

                if (IsPlayerAlive(entity))
                {
                    static const Float:vecClientVel[3] = {0.0, 0.0, 0.0};
                    decl Float:PlayerVec[3];
                    decl Float:vecClientEyeAng[3];
                    GetClientAbsOrigin(other, PlayerVec);
                    GetClientEyeAngles(other, vecClientEyeAng); // Get the angle the player is looking
                    TeleportEntity(entity, PlayerVec, NULL_VECTOR, vecClientVel);
                }
            }
        }
    }
}

public TraceTarget(client)
{
    decl Float:vecClientEyePos[3];
    decl Float:vecClientEyeAng[3];
    GetClientEyePosition(client, vecClientEyePos); // Get the position of the player's eyes
    GetClientEyeAngles(client, vecClientEyeAng); // Get the angle the player is looking

    //Check for colliding entities
    TR_TraceRayFilter(vecClientEyePos, vecClientEyeAng, MASK_PLAYERSOLID,
                      RayType_Infinite, TraceRayDontHitSelf, client);

    if (TR_DidHit(INVALID_HANDLE))
    {
        new TRIndex = TR_GetEntityIndex(INVALID_HANDLE);
        if (TRIndex > 0 && TRIndex <= MaxClients)
            Piggy(client, TRIndex);
    }
}

public bool:TraceRayDontHitSelf(entity, mask, any:data)
{
    return (entity != data); // Check if the TraceRay hit the entity.
}

public Native_ControlPiggyback(Handle:plugin,numParams)
{
    gNativeControl = bool:GetNativeCell(1);
}

public Native_GivePiggyback(Handle:plugin,numParams)
{
    new client = GetNativeCell(1);

    new PiggyMethod:method = PiggyMethod:GetNativeCell(2);
    if (method > PiggyMethod_Default)
        g_method[client] = method;
    else if (GetConVarBool(pb_enable))
    {
        g_method[client]  = PiggyMethod:GetConVarInt(pb_method);
        g_method[client] |= PiggyMethod_Enable;
    }
    else
        g_method[client] = PiggyMethod_None;

    new Float:distance = Float:GetNativeCell(3);
    if (distance >= 0.0)
        g_distance[client] = distance;
    else
        g_distance[client] = GetConVarFloat(pb_distance);
}

public Native_TakePiggyback(Handle:plugin,numParams)
{
    new client = GetNativeCell(1);
    g_method[client] = PiggyMethod_None;
    RemovePiggy(client, true);

    for (new i = 1; i <= MaxClients; i++)
    {
        if (g_piggy[i] == client)
            RemovePiggy(i, true);
    }
}

public Native_Piggyback(Handle:plugin,numParams)
{
    new client = GetNativeCell(1);
    new target = GetNativeCell(2);

    for (new i = 1; i <= MaxClients; i++)
    {
        if (g_piggy[i] == client)
        {
            RemovePiggy(i, false);
            return;
        }
    }

    if (target > 0)
        Piggy(client, target);
    else
        TraceTarget(client);
}

/**
 * Determine if client has the flag
 */
#tryinclude <tf2_flag>
#if !defined _tf2_flag_included
    stock TF2_GetFlagCarrier(team)
    {
        new ent = -1;
        while ((ent = FindEntityByClassname(ent, "item_teamflag")) != -1)
        {
            new owner = GetEntPropEnt(ent, Prop_Data, "m_hOwnerEntity");
            if (owner > 0)
            {
                if (GetClientTeam(owner) == team)
                    return owner;
            }
        }
        return 0;
    }
#endif
