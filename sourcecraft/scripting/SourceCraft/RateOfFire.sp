/**
 * vim: set ai et ts=4 sw=4 :
 * File: RateOfFire.sp
 * Description: SourceCraft/TF2 Rate of Fire plugin
 * Author(s): -=|JFH|=-Naris (Murray Wilson)
 */

#pragma semicolon 1
#include <sourcemod>
#include <sdktools>
#include <gametype>

#undef REQUIRE_EXTENSIONS
#include <tf2_stocks>
#include <tf2_player>
#define REQUIRE_EXTENSIONS

#include "weapons"

#include "sc/SourceCraft"
#include "sc/weapons"

public Plugin:myinfo = 
{
    name = "SourceCraft Rate of Fire plugin",
    author = "-=|JFH|=-Naris",
    description = "Exports natives to change the Rate of Fire",
    version = SOURCECRAFT_VERSION,
    url = "http://www.jigglysfunhouse.net/"
}

new m_WeaponRateQueueLen;
new m_WeaponRateQueue[MAXPLAYERS+1];
new Float:m_WeaponRateMult[MAXPLAYERS+1];
new Float:m_ClientRateMult[MAXPLAYERS+1];
new m_EnergyAmount[MAXPLAYERS+1];

new bool:g_NativeControl = false;
new Handle:g_hROF = INVALID_HANDLE;
new Float:g_mult = 1.0;

public APLRes:AskPluginLoad2(Handle:myself, bool:late, String:error[], err_max)
{
    switch (GetGameType())
    {
        case cstrike:
        {
            if (!HookEvent("weapon_fire",WeaponFireEvent, EventHookMode_Pre))
            {
                strcopy(error, err_max, "Failed to hook the weapon_fire event");
                return APLRes_SilentFailure;
            }

            if (!HookEventEx("round_start",ResetClientForEvent,EventHookMode_PostNoCopy))
            {
                strcopy(error, err_max, "Failed to hook the round_start event");
                return APLRes_SilentFailure;
            }

            if (!HookEventEx("round_end",ResetClientForEvent,EventHookMode_PostNoCopy))
            {
                strcopy(error, err_max, "Failed to hook the round_end event");
                return APLRes_SilentFailure;
            }
        }
        case dod:
        {
            if (!HookEvent("dod_stats_weapon_attack",WeaponFireEvent, EventHookMode_Pre))
            {
                strcopy(error, err_max, "Failed to hook the dod_stats_weapon_attack event");
                return APLRes_SilentFailure;
            }
            if (!HookEventEx("dod_round_start",ResetClientForEvent,EventHookMode_PostNoCopy))
            {
                strcopy(error, err_max, "Failed to hook the dod_round_start event");
                return APLRes_SilentFailure;
            }

            if (!HookEventEx("dod_restart_round",ResetClientForEvent,EventHookMode_PostNoCopy))
            {
                strcopy(error, err_max, "Failed to hook the dod_restart_round event");
                return APLRes_SilentFailure;
            }

            if (!HookEventEx("dod_round_win",ResetClientForEvent,EventHookMode_PostNoCopy))
            {
                strcopy(error, err_max, "Failed to hook the dod_round_win event");
                return APLRes_SilentFailure;
            }
        }
        case tf2:
        {
            if (!HookEventEx("teamplay_round_start",ResetClientForEvent,EventHookMode_PostNoCopy))
            {
                strcopy(error, err_max, "Failed to hook the teamplay_round_start event");
                return APLRes_SilentFailure;
            }

            if (!HookEventEx("arena_round_start",ResetClientForEvent,EventHookMode_PostNoCopy))
            {
                strcopy(error, err_max, "Failed to hook the arena_round_start event");
                return APLRes_SilentFailure;
            }
        }
        default:
        {
            if (!HookEvent("player_shoot",WeaponFireEvent, EventHookMode_Pre))
            {
                strcopy(error, err_max, "Failed to hook the player_shoot event");
                return APLRes_SilentFailure;
            }
        }
    }

    CreateNative("ControlROF", Native_ControlROF);
    CreateNative("SetROF", Native_SetROF);
    RegPluginLibrary("RateOfFire");
    return  APLRes_Success;
}

public OnPluginStart()
{
    g_hROF = CreateConVar("sm_rof", "1.0", "Rate Of Fire multiplier.", FCVAR_PLUGIN|FCVAR_NOTIFY);
    HookConVarChange(g_hROF, Cvar_rof);
}

public OnConfigsExecuted()
{
    g_mult = 1.0/GetConVarFloat(g_hROF);
}

public bool:OnClientConnect(client, String:rejectmsg[], maxlen)
{
    m_ClientRateMult[client] = 0.0;
    return true;
}

public OnClientDisconnect(client)
{
    m_ClientRateMult[client] = 0.0;
}

public ResetClientForEvent(Handle:event,const String:name[],bool:dontBroadcast)
{
    for(new client=1;client<=MaxClients;client++)
    {
        m_ClientRateMult[client] = 0.0;
    }
}

public OnGameFrame()
{
    if (m_WeaponRateQueueLen)
    {
        new Float:enginetime = GetGameTime();
        for (new i=0;i<m_WeaponRateQueueLen;i++)
        {
            new ent = m_WeaponRateQueue[i];
            if (IsValidEntity(ent))
            {
                new Float:rofmult = m_WeaponRateMult[i];
                if (rofmult != 1.0)
                {
                    new Float:time = (GetEntPropFloat(ent, Prop_Send, "m_flNextPrimaryAttack")-enginetime)*rofmult;
                    SetEntPropFloat(ent, Prop_Send, "m_flNextPrimaryAttack", time+enginetime);

                    time = (GetEntPropFloat(ent, Prop_Send, "m_flNextSecondaryAttack")-enginetime)*rofmult;
                    SetEntPropFloat(ent, Prop_Send, "m_flNextSecondaryAttack", time+enginetime);
                }
            }
        }
        m_WeaponRateQueueLen = 0;
    }
}

public Cvar_rof(Handle:convar, const String:oldValue[], const String:newValue[])
{
    g_mult = 1.0/GetConVarFloat(g_hROF);
}

public Action:TF2_CalcIsAttackCritical(client, weapon, String:weaponname[], &bool:result)
{
    if (client > 0 && weapon > 0)
    {
        new Float:rate = g_NativeControl ? m_ClientRateMult[client] : g_mult;
        if (rate < 0.0)
            rate = g_mult;

        if (rate != 0.0 && rate != 1.0)
        {
            new energy = GetEnergy(client);
            new amount = m_EnergyAmount[client];
            if (energy >= amount)
            {
                // Don't enable rapid fire for Melee Weapons or the Pyro's Flamethrower
                // (since it's not very useful and eats energy)
                // Also don't allow zoomed snipers to have rapid fire!
                if (!IsEquipmentMelee(weaponname) &&
                     ((!StrEqual(weaponname, "tf_weapon_flamethrower") &&
                      (TF2_GetPlayerClass(client) != TFClass_Sniper ||
                       !TF2_IsPlayerZoomed(client)))))
                {
                    m_WeaponRateQueue[m_WeaponRateQueueLen] = weapon;
                    m_WeaponRateMult[m_WeaponRateQueueLen] = rate;
                    m_WeaponRateQueueLen++;

                    if (amount > 0)
                        SetEnergy(client, energy-amount);
                }
            }
        }
    }
    return Plugin_Continue;
}

public WeaponFireEvent(Handle:event,const String:name[],bool:dontBroadcast)
{ 
    new client = (GameType == dod) ? GetEventInt(event,"attacker")
                                   : GetClientOfUserId(GetEventInt(event,"userid"));

    new weapon = GetActiveWeapon(client);
    if (weapon > 0)
    {
        new Float:rate = g_NativeControl ? m_ClientRateMult[client] : g_mult;
        if (rate < 0.0)
            rate = g_mult;

        if (rate != 0.0 && rate != 1.0)
        {
            new energy = GetEnergy(client);
            new amount = m_EnergyAmount[client];
            if (energy >= amount)
            {
                m_WeaponRateQueue[m_WeaponRateQueueLen] = weapon;
                m_WeaponRateMult[m_WeaponRateQueueLen] = rate;
                m_WeaponRateQueueLen++;

                if (amount > 0)
                    SetEnergy(client, energy-amount);
            }
        }
    } 
}

public Native_ControlROF(Handle:plugin, numParams)
{
    g_NativeControl = GetNativeCell(1);
}

public Native_SetROF(Handle:plugin, numParams)
{
    new client = GetNativeCell(1);
    m_ClientRateMult[client] = Float:GetNativeCell(2);
    m_EnergyAmount[client] = GetNativeCell(3);
}
