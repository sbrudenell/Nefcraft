// vim: set ai et ts=4 sw=4 :
//////////////////////////////////////////////
//
// SourceMod Script
//
// DoD DropHealthKit Source
//
// Developed by FeuerSturm
//
// - Credits to "monkie" for the request!
//
//////////////////////////////////////////////
//
//
// USAGE:
// ======
//
//
// CVARs:
// ------
//
// dod_drophealthkit_source <1/0>       =   enable/disable dropping a healthkit on players' death
//
// dod_drophealthkit_pickuprule <0/1/2> =   set who can pickup dropped healthkits
//                                          0 = everyone
//                                          1 = only teammates
//                                          2 = only enemies
//
// dod_drophealthkit_addhealth <#>      =   amount of HP to add to a player picking up a healthkit
//
// dod_drophealthkit_maxhealth <#>      =   maximum amount of healthpoints a player can reach
//
// dod_drophealthkit_lifetime <#>       =   number of seconds a dropped healthkit stays on the map
//
// dod_drophealthkit_useteamcolor <1/0> =   use team's color of dropping player to colorize healthkit
//
//
//
//
// CHANGELOG:
// ==========
// 
// - 16 November 2008 - Version 1.0
//   Initial Release
//
// - 18 November 2008 - Version 1.1
//   New Features:
//   * maximum amount of health a player can reach
//     can now be defined
//     (see new cvar "dod_drophealthkit_maxhealth")
//   * healthkit pickup can now be limited to a group
//     of players
//     (see new cvar "dod_drophealthkit_pickuprule")
//
// - 22 November 2008 - Version 1.2
//   New Features:
//   * players can be allowed to drop their healthkit
//     while being alive (command "dropammo" that usually
//     drops an ammobox is used. So if a player has a healthkit
//     on first button press (default is [H]) player's healthkit
//     is dropped if he has one, pressing the button again will
//     drop the ammobox like usually!
//     (see new cvar "dod_drophealthkit_alivedrop")
//   * dropping healthkits on death/alive can be enabled/disabled
//     independently from each other!
//   Bugfixes:
//   * fixed invalid Handle errors
//   General Changes:
//   * renamed cvar "dod_drophealthkit_source" to
//     "dod_drophealthkit_deaddrop"
//
// - 06 December 2009 - Version 2.0 by Naris
//   * Allow players to have more than 1 healthkit
//   * Added native interface
//
// - 06 May 2010 - Version 2.1 by Naris
//   * converted to use sdkhooks instead of dukehacks
//   * check entity limit before creating any new healthkits
//   * validate all healthkits using entrefs
//
//
#include <sourcemod>
#include <sdktools>
#include <sdkhooks>

#define PLUGIN_VERSION "2.1"

public Plugin:myinfo = 
{
    name = "DoD DropHealthKit Source",
    author = "FeuerSturm",
    description = "Players drop a healthkit on death!",
    version = PLUGIN_VERSION,
    url = "http://community.dodsourceplugins.net"
}

#define MAXENTITIES 2048

new const String:g_HealthKit_Model[] = { "models/props_misc/ration_box01.mdl" };
new const String:g_HealthKit_Sound[] = { "object/object_taken.wav" };

new const g_HealthKit_Skin[4] = { 0, 0, 2, 1 };

new g_HasHealthKit[MAXPLAYERS+1];

new g_HealthKitRef[MAXENTITIES+1];
new g_HealthKitOwner[MAXENTITIES+1];
new Handle:HealthKitDropTimer[MAXENTITIES+1] = INVALID_HANDLE;

new Handle:healthkitdeaddrop = INVALID_HANDLE;
new Handle:healthkitrule = INVALID_HANDLE;
new Handle:healthkitdropcmd = INVALID_HANDLE;
new Handle:healthkitmaxhealth = INVALID_HANDLE;
new Handle:healthkithealth = INVALID_HANDLE;
new Handle:healthkitliefetime = INVALID_HANDLE;
new Handle:healthkitteamcolor = INVALID_HANDLE;

new bool:g_NativeControl = false;
new g_NativeHealthkit[MAXPLAYERS+1];
new g_NativeHealthkitRule[MAXPLAYERS+1];
new g_NativeHealthkitCount[MAXPLAYERS+1];

public APLRes:AskPluginLoad2(Handle:myself, bool:late, String:error[], err_max)
{
    CreateNative("ControlHealthkits", Native_ControlHealthkits);
    CreateNative("SetHealthkit", Native_SetHealthkit);
    RegPluginLibrary("healthkit");
    return APLRes_Success;
}

public OnPluginStart()
{   
    CreateConVar("dod_drophealthkit_version", PLUGIN_VERSION, "DoD DropHealthKit Source Version (DO NOT CHANGE!)", FCVAR_DONTRECORD|FCVAR_PLUGIN|FCVAR_SPONLY|FCVAR_REPLICATED|FCVAR_NOTIFY);
    SetConVarString(FindConVar("dod_drophealthkit_version"), PLUGIN_VERSION);
    healthkitdeaddrop = CreateConVar("dod_drophealthkit_deaddrop", "1", "<1/0> = enable/disable dropping a healthkit on players' death", FCVAR_PLUGIN, true, 0.0, true, 1.0);
    healthkitrule = CreateConVar("dod_drophealthkit_pickuprule", "0", "<0/1/2> = set who can pickup dropped healthkits: 0 = everyone, 1 = only teammates, 2 = only enemies", FCVAR_PLUGIN, true, 0.0, true, 2.0);
    healthkitdropcmd = CreateConVar("dod_drophealthkit_alivedrop", "1", "<1/0> = enable/disable allowing alive players to drop their healthkit", FCVAR_PLUGIN, true, 0.0, true, 1.0);
    healthkithealth = CreateConVar("dod_drophealthkit_addhealth", "25", "<#> = amount of HP to add to a player picking up a healthkit", FCVAR_PLUGIN, true, 5.0, true, 95.0);
    healthkitmaxhealth = CreateConVar("dod_drophealthkit_maxhealth", "100", "<#> = maximum amount of healthpoints a player can reach", FCVAR_PLUGIN, true, 50.0, true, 200.0);
    healthkitliefetime = CreateConVar("dod_drophealthkit_lifetime", "30", "<#> = number of seconds a dropped healthkit stays on the map", FCVAR_PLUGIN, true, 5.0, true, 60.0);
    healthkitteamcolor = CreateConVar("dod_drophealthkit_useteamcolor", "0", "<1/0> = use team's color of dropping player to colorize healthkit", FCVAR_PLUGIN, true, 0.0, true, 1.0);
    RegAdminCmd("dropammo", cmdDropHealthKit, 0);
    HookEvent("player_hurt", OnPlayerDeath, EventHookMode_Pre);
    HookEventEx("player_spawn", OnPlayerSpawn, EventHookMode_Post);
    AutoExecConfig(true, "dod_drophealthkit_source", "dod_drophealthkit_source");
}

public Action:cmdDropHealthKit(client, args) 
{
    if (!IsClientInGame(client) || !IsPlayerAlive(client) ||
        g_HasHealthKit[client] <= 0)
    {
        return Plugin_Continue;
    }
    else if (g_NativeControl)
    {
        if (g_NativeHealthkit[client] < 2)
            return Plugin_Continue;
    }
    else if (!GetConVarBool(healthkitdropcmd))
    {
        return Plugin_Continue;
    }
    else if (IsEntLimitReached(.client=client, .message="unable to create healthkit"))
    {
        return Plugin_Continue;
    }

    new Float:origin[3];
    GetClientAbsOrigin(client, origin);
    origin[2] += 55.0;

    new Float:angles[3];
    GetClientEyeAngles(client, angles);

    new Float:velocity[3];
    GetAngleVectors(angles, velocity, NULL_VECTOR, NULL_VECTOR);
    NormalizeVector(velocity,velocity);
    ScaleVector(velocity,350.0);

    CreateHealthkit(client, origin, angles, velocity);
    return Plugin_Handled;
}

public OnMapStart()
{
    PrecacheModel(g_HealthKit_Model,true);
    PrecacheSound(g_HealthKit_Sound, true);
}

public Action:OnPlayerSpawn(Handle:event, const String:name[], bool:dontBroadcast)
{
    new client = GetClientOfUserId(GetEventInt(event, "userid"));
    if (IsClientInGame(client) && IsPlayerAlive(client) && GetClientTeam(client) > 1)
    {
        g_HasHealthKit[client] = g_NativeControl
                                 ? (g_NativeHealthkit[client] != 0)
                                   ? g_NativeHealthkitCount[client] : 0
                                 : 1;
    }
    return Plugin_Continue;
}

public Action:OnPlayerDeath(Handle:event, const String:name[], bool:dontBroadcast)
{
    new client = GetClientOfUserId(GetEventInt(event, "userid"));
    if (GetClientHealth(client) > 0 || !IsClientInGame(client) ||
        g_HasHealthKit[client] <= 0)
    {
        return Plugin_Continue;
    }
    else if (g_NativeControl)
    {
        if (g_NativeHealthkit[client] % 2 == 1)
            return Plugin_Continue;
    }
    else if (!GetConVarBool(healthkitdeaddrop))
    {
        return Plugin_Continue;
    }
    else if (IsEntLimitReached(.client=client, .message="unable to create healthkit"))
    {
        return Plugin_Continue;
    }

    new Float:deathorigin[3];
    GetClientAbsOrigin(client, deathorigin);
    deathorigin[2] += 5.0;

    CreateHealthkit(client, deathorigin);
    return Plugin_Continue;
}

CreateHealthkit(client, const Float:origin[3],
                const Float:angles[3]=NULL_VECTOR,
                const Float:velocity[3]=NULL_VECTOR)
{
    new healthkit = CreateEntityByName("prop_physics_override");
    if (healthkit > 0 && IsValidEntity(healthkit))
    {
        new team = GetClientTeam(client);
        SetEntityModel(healthkit,g_HealthKit_Model);
        SetEntProp(healthkit, Prop_Send, "m_nSkin", g_HealthKit_Skin[team]);
        DispatchSpawn(healthkit);
        TeleportEntity(healthkit, origin, angles, velocity);

        if (GetConVarInt(healthkitteamcolor) == 1)
        {
            SetEntityRenderColor(healthkit, team == 2 ? 0 : 150, team == 2 ? 150 : 0, 0, 50);
        }
        g_HasHealthKit[client]--;
        g_HealthKitOwner[healthkit] = client;
        g_HealthKitRef[healthkit] = EntIndexToEntRef(healthkit);
        SDKHook(healthkit, SDKHook_Touch, OnHealthKitTouched);
        HealthKitDropTimer[healthkit] = CreateTimer(GetConVarFloat(healthkitliefetime),
                                                    RemoveDroppedHealthKit, healthkit,
                                                    TIMER_FLAG_NO_MAPCHANGE);
    }
}

public Action:OnHealthKitTouched(healthkit, client)
{
    if (client > 0 && client <= MaxClients && healthkit > 0 &&
        EntRefToEntIndex(g_HealthKitRef[healthkit]) == healthkit &&
        IsValidEntity(client) && IsClientInGame(client) &&
        IsPlayerAlive(client) && IsValidEdict(healthkit))
    {
        if (g_HealthKitOwner[healthkit] == client)
        {
            if (g_HasHealthKit[client] <= 0)
            {
                KillHealthKitTimer(healthkit);
                EmitSoundToClient(client, g_HealthKit_Sound, .channel=SNDCHAN_WEAPON);
                RemoveEdict(healthkit);
                g_HasHealthKit[client]++;
                g_HealthKitRef[healthkit] = 0;
                g_HealthKitOwner[healthkit] = 0;
                return Plugin_Handled;
            }
            return Plugin_Handled;
        }

        new health = GetClientHealth(client);
        new maxhealth = GetConVarInt(healthkitmaxhealth);
        if (health >= maxhealth)
        {
            return Plugin_Handled;
        }

        new pickuprule = g_NativeControl ? (g_NativeHealthkitRule[client]) : GetConVarInt(healthkitrule);
        new clteam = GetClientTeam(client);
        new kitteam = GetEntProp(healthkit, Prop_Send, "m_nSkin");
        if ((pickuprule == 1 && kitteam != g_HealthKit_Skin[clteam]) ||
            (pickuprule == 2 && kitteam == g_HealthKit_Skin[clteam]))
        {
            return Plugin_Handled;
        }

        new healthkitadd = GetConVarInt(healthkithealth);
        if (health + healthkitadd >= maxhealth)
        {
            SetEntityHealth(client, maxhealth);
        }
        else
        {
            SetEntityHealth(client, health + healthkitadd);
        }

        KillHealthKitTimer(healthkit);
        EmitSoundToClient(client, g_HealthKit_Sound, .channel=SNDCHAN_WEAPON);

        RemoveEdict(healthkit);
        g_HealthKitRef[healthkit] = 0;
        g_HealthKitOwner[healthkit] = 0;
    }
    return Plugin_Handled;
}

public Action:RemoveDroppedHealthKit(Handle:timer, any:healthkit)
{
    HealthKitDropTimer[healthkit] = INVALID_HANDLE;
    if (EntRefToEntIndex(g_HealthKitRef[healthkit]) == healthkit &&
        IsValidEdict(healthkit))
    {
        RemoveEdict(healthkit);
        g_HealthKitRef[healthkit] = 0;
        g_HealthKitOwner[healthkit] = 0;
    }
    return Plugin_Handled;
}

KillHealthKitTimer(healthkit)
{
    new Handle:timer = HealthKitDropTimer[healthkit];
    if (timer != INVALID_HANDLE)
    {
        CloseHandle(HealthKitDropTimer[healthkit]);
        HealthKitDropTimer[healthkit] = INVALID_HANDLE;
    }
}

public Native_ControlHealthkits(Handle:plugin, numParams)
{
    g_NativeControl = GetNativeCell(1);
}

public Native_SetHealthkit(Handle:plugin, numParams)
{
    new client = GetNativeCell(1);
    new enable = GetNativeCell(2);
    new count = GetNativeCell(4);

    g_NativeHealthkit[client] = enable;
    g_NativeHealthkitRule[client] = GetNativeCell(3);
    g_NativeHealthkitCount[client] = count;

    g_HasHealthKit[client] = (enable != 0) ? count : 0;
}

/**
 * Description: Function to check the entity limit.
 *              Use before spawning an entity.
 */
#tryinclude <entlimit>
#if !defined _entlimit_included
    stock IsEntLimitReached(warn=20,critical=16,client=0,const String:message[]="")
    {
        new max = GetMaxEntities();
        new count = GetEntityCount();
        new remaining = max - count;
        if (remaining <= warn)
        {
            if (count <= critical)
            {
                PrintToServer("Warning: Entity limit is nearly reached! Please switch or reload the map!");
                LogError("Entity limit is nearly reached: %d/%d (%d):%s", count, max, remaining, message);

                if (client > 0)
                {
                    PrintToConsole(client, "Entity limit is nearly reached: %d/%d (%d):%s",
                                   count, max, remaining, message);
                }
            }
            else
            {
                PrintToServer("Caution: Entity count is getting high!");
                LogMessage("Entity count is getting high: %d/%d (%d):%s", count, max, remaining, message);

                if (client > 0)
                {
                    PrintToConsole(client, "Entity count is getting high: %d/%d (%d):%s",
                                   count, max, remaining, message);
                }
            }
            return count;
        }
        else
            return 0;
    }
#endif

