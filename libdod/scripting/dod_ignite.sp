// vim: set ai et ts=4 sw=4 :
///////////////////////////////////////////////////////////////////////////////////////
//
//  File:   dod_ignite.sp (was FireLoopFix.sp)
//  Author: Daedilus
//  Date:   2009-10-18
//
//  License:
//
//  This program is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program.  If not, see <http://www.gnu.org/licenses/>.
//
///////////////////////////////////////////////////////////////////////////////////////

#pragma semicolon 1

#include <sourcemod>
#include <sdktools>
#include <sdktools_sound>

#define FIRE_SMALL_LOOP2    "ambient/fire/fire_small_loop2.wav"
#define MAXENTITIES         2048

///////////////////////////////////////////////////////////////////////////////////////
// Plugin Info

public Plugin:myinfo =
{
    name = "dod_ignite",
    author = "Daedilus/-=|JFH|=-Naris",
    description = "Fixes the fire sound that loops for IgniteEntity in DoD by turning it off",
    version = "2.0.0",
    url = "http://www.budznetwork.com"
};

new Handle:cvarBurnTime;
new Float:flBurnTime[MAXENTITIES+1];

///////////////////////////////////////////////////////////////////////////////////////
// AskPluginLoad

public APLRes:AskPluginLoad2(Handle:myself, bool:late, String:error[], err_max)
{
    // Register Natives
    CreateNative("DOD_IgniteEntity",Native_DOD_IgniteEntity);
    RegPluginLibrary("dod_ignite");
    return APLRes_Success;
}

///////////////////////////////////////////////////////////////////////////////////////
// OnPluginStart

public OnPluginStart()
{
    cvarBurnTime = CreateConVar("sm_dod_burn_sound_time", "2.0", "Default amount of time to allow burning sound");

    // Hook the sounds being emitted to the client
    AddNormalSoundHook(NormalSoundHook);

    // Hook ConVar Changes
    HookConVarChange(cvarBurnTime, OnConVarChange);
}


///////////////////////////////////////////////////////////////////////////////////////
// OnConfigsExecuted

public OnConfigsExecuted()
{
    flBurnTime[0] = GetConVarFloat(cvarBurnTime);
}

///////////////////////////////////////////////////////////////////////////////////////
// OnConVarChange

public OnConVarChange(Handle:hHandle, String:strOldVal[], String:strNewVal[])
{
    if (hHandle == cvarBurnTime)
        flBurnTime[0] = StringToFloat(strNewVal);
}

///////////////////////////////////////////////////////////////////////////////////////
// NormalSoundHook

public Action:NormalSoundHook(clients[MAXPLAYERS], &client_count, String:sample[PLATFORM_MAX_PATH],
                              &entity, &channel, &Float:volume, &level, &pitch, &flags)
{
    if (strcmp(FIRE_SMALL_LOOP2, sample, false) == 0)
    {
        new Float:time = flBurnTime[entity];
        if (time <= 0.0)
            time = flBurnTime[0];
        if (time > 0.0)
        {
            new Handle:pack = CreateDataPack();
            WritePackCell(pack,entity);
            WritePackCell(pack,channel);
            CreateTimer(time, KillSound, pack);
            flBurnTime[entity] = 0.0;
            return Plugin_Continue;
        }
        else
            return Plugin_Stop;
    }
    return Plugin_Continue;
}

///////////////////////////////////////////////////////////////////////////////////////
// KillSound

public Action:KillSound(Handle:timer, Handle:pack)
{
    if (pack != INVALID_HANDLE)
    {
        ResetPack(pack);
        new entity = ReadPackCell(pack);
        if ((entity > MaxClients) ? IsValidEntity(entity)
            : (IsClientConnected(entity) && IsClientInGame(entity)))
        {            
            new channel = ReadPackCell(pack);
            StopSound(entity,channel,FIRE_SMALL_LOOP2);
        }
        CloseHandle(pack);
    }
    return Plugin_Stop;
}

///////////////////////////////////////////////////////////////////////////////////////
// Native_DOD_Ignite

public Native_DOD_IgniteEntity(Handle:plugin,numParams)
{
    new entity = GetNativeCell(1);
    new Float:time = Float:GetNativeCell(2);
    flBurnTime[entity] = time;
    IgniteEntity(entity, time, bool:GetNativeCell(3),
                 Float:GetNativeCell(4), bool:GetNativeCell(5));
}
