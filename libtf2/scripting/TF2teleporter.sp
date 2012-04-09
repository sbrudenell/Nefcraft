/**
 * vim: set ai et ts=4 sw=4 syntax=sourcepawn :
 * File: tf2teleporter.sp
 * Description: Decrease teleporter time in TF2
 * Author(s): Nican132
 */

#include <sourcemod>
#include <sdktools>

#define PL_VERSION "4.0"

public Plugin:myinfo = 
{
    name = "Teleport Tools",
    author = "Naris,Nican132&kim_perm",
    description = "Change the time teleporters take to recharge",
    version = PL_VERSION,
    url = "http://sourcemod.net/"
};       

#define LIST_OBJECT 0
#define LIST_TEAM 1

new bool:NativeControl = false;
new Float:TeleporterTime[ MAXPLAYERS+1 ] = { 0.0, ...};

new TeleporterList[ MAXPLAYERS+1 ][ 2 ];

new Handle:g_cvarEnabled = INVALID_HANDLE;
new Handle:g_cvarBlueTime = INVALID_HANDLE;
new Handle:g_cvarRedTime = INVALID_HANDLE;
new Handle:g_cvarTime = INVALID_HANDLE;

new g_Enabled;         // plugin status enabled/disabled
new Float:g_BlueTime;  // blue team recharge time
new Float:g_RedTime;   // red team recharge time
new Float:g_Time;      // global recharge time

public APLRes:AskPluginLoad2(Handle:myself, bool:late, String:error[], err_max)
{
    // Register Natives
    CreateNative("ControlTeleporter",Native_ControlTeleporter);
    CreateNative("SetTeleporter",Native_SetTeleporter);
    RegPluginLibrary("tf2teleporter");
    return APLRes_Success;
}


public OnPluginStart()
{
    CreateConVar("sm_tf_teletools", PL_VERSION, "Teleport Tools", FCVAR_PLUGIN|FCVAR_SPONLY);

    g_cvarEnabled = CreateConVar("sm_tele_on","1","Enable/Disable teleport manager");
    g_cvarBlueTime = CreateConVar("sm_teleblue_time","0.6","Amount of time for blue tele to recharg, 0.0=disable");
    g_cvarRedTime = CreateConVar("sm_telered_time","0.6","Amount of time for red tele to recharg, 0.0=disable");
    g_cvarTime = CreateConVar("sm_tele_time","0.0","Amount of time for the recharge timer tick, 0.0=auto");

    HookEvent("player_builtobject", Event_player_builtobject);
    HookEvent("player_teleported", event_player_teleported);
}

public OnConfigsExecuted()
{
    g_Enabled = GetConVarInt(g_cvarEnabled);
    g_BlueTime = GetConVarFloat(g_cvarBlueTime);
    g_RedTime = GetConVarFloat(g_cvarRedTime);
    g_Time = GetConVarFloat(g_cvarRedTime);

    HookConVarChange(g_cvarEnabled,  TF2ConfigsChanged );
    HookConVarChange(g_cvarBlueTime, TF2ConfigsChanged ); 
    HookConVarChange(g_cvarRedTime,  TF2ConfigsChanged );
    HookConVarChange(g_cvarTime,  TF2ConfigsChanged );
}

public TF2ConfigsChanged(Handle:convar, const String:oldValue[], const String:newValue[])
{
    if (convar == g_cvarTime)
        g_Time = StringToFloat(newValue);
    else if (convar == g_cvarBlueTime)
        g_BlueTime = StringToFloat(newValue);
    else if (convar == g_cvarRedTime)
        g_RedTime = StringToFloat(newValue);
    else if (convar == g_cvarEnabled)
    {
        g_Enabled = StringToInt(newValue);
        if (StrEqual(oldValue, "0") && g_Enabled > 0)
        {
            //plugin change status disabled -> enabled
            //must collect all existing teleports
            new owner;
            decl String:classname[19];

            for (new i = GetMaxClients() + 1; i <= GetMaxEntities(); i++)
            {
                if (IsValidEntity(i))
                {
                    GetEntityNetClass(i, classname, sizeof(classname));
                    if (StrEqual(classname, "CObjectTeleporter"))
                    {
                        if (GetEntProp(i, Prop_Send, "m_iObjectMode") == 0)
                        {
                            owner = GetEntPropEnt(i, Prop_Send, "m_hBuilder");
                            TeleporterList[owner][ LIST_TEAM ] = GetEntProp(i, Prop_Send, "m_iTeamNum");
                            TeleporterList[owner][ LIST_OBJECT ] = i;
                        }   
                    }
                }
            }
        }
    }
}

public Action:Event_player_builtobject(Handle:event, const String:name[], bool:dontBroadcast)
{
    if ( GetEventInt(event, "object") != 1)
        return Plugin_Continue;

    new entity = GetEventInt(event, "index");
    if (IsValidEntity(entity))
    {
        new owner = GetClientOfUserId(GetEventInt(event, "userid"));

        //check for entrance (0 = entrance, 1 = exit)
        if (GetEntProp(entity, Prop_Send, "m_iObjectMode") == 0)
        {
            TeleporterList[owner][ LIST_TEAM ] = GetEntProp(entity, Prop_Send, "m_iTeamNum");
            TeleporterList[owner][ LIST_OBJECT ] = entity;
        }
    }
    return Plugin_Continue;
}

public Action:event_player_teleported(Handle:event, const String:name[], bool:dontBroadcast)
{
    if(g_Enabled)
    {
        new owner = GetClientOfUserId(GetEventInt(event, "builderid"));
        new entity = TeleporterList[owner][ LIST_OBJECT ];
        if(IsValidEntity(entity))
        {
            new Float:time;
            if (NativeControl)
                time = TeleporterTime[owner];
            else if( TeleporterList[owner][LIST_TEAM] == 3)
                time = (g_Time > 0.0) ? g_Time : g_BlueTime;
            else if( TeleporterList[owner][LIST_TEAM] == 2)
                time = (g_Time > 0.0) ? g_Time : g_RedTime;
            else // Unknown Team!
                time = g_Time;

            if (time != 0.0)
                SetEntPropFloat(entity, Prop_Send, "m_flRechargeTime", GetGameTime() + time);
        }
    }
    return Plugin_Continue;
}

public Native_ControlTeleporter(Handle:plugin,numParams)
{
    NativeControl = GetNativeCell(1);
}

public Native_SetTeleporter(Handle:plugin,numParams)
{
    TeleporterTime[GetNativeCell(1)] = Float:GetNativeCell(2);
}
