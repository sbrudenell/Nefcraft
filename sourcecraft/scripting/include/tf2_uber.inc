/**
 * vim: set ai et ts=4 sw=4 syntax=sourcepawn :
 * File: tf2_uber.inc
 * Description: Stocks to return information about TF2 UberCharge.
 * Author(s): Naris (Murray Wilson)
 */
 
#if defined _tf2_uber_included
 #endinput
#endif
#define _tf2_uber_included

#include <tf2_stocks>

stock TF2_IsUberCharge(client)
{
    new index = GetPlayerWeaponSlot(client, 1);
    if (index > 0)
		return GetEntProp(index, Prop_Send, "m_bChargeRelease", 1);
    else
        return 0;
}

stock Float:TF2_GetUberLevel(client)
{
    new index = GetPlayerWeaponSlot(client, 1);
    if (index > 0)
        return GetEntPropFloat(index, Prop_Send, "m_flChargeLevel");
    else
        return 0.0;
}

stock TF2_SetUberLevel(client, Float:uberlevel)
{
    new index = GetPlayerWeaponSlot(client, 1);
    if (index > 0)
		SetEntPropFloat(index, Prop_Send, "m_flChargeLevel", uberlevel);
}

stock TF2_ExIsUberCharge(client)
{
    if (TF2_GetPlayerClass(client) == TFClass_Medic)
    {
        new index = GetPlayerWeaponSlot(client, 1);
        if (index > 0)
        {
            decl String:classname[50];
            if (GetEdictClassname(index, classname, sizeof(classname)) &&
                StrEqual(classname, "tf_weapon_medigun"))
            {
                return GetEntProp(index, Prop_Send, "m_bChargeRelease", 1);
            }
        }
    }
    return 0;
}

stock Float:TF2_ExGetUberLevel(client)
{
    if (TF2_GetPlayerClass(client) == TFClass_Medic)
    {
        new index = GetPlayerWeaponSlot(client, 1);
        if (index > 0)
        {
            decl String:classname[50];
            if (GetEdictClassname(index, classname, sizeof(classname)) &&
                StrEqual(classname, "tf_weapon_medigun"))
            {
                return GetEntPropFloat(index, Prop_Send, "m_flChargeLevel");
            }
        }
    }
    return 0;
}

stock bool:TF2_ExSetUberLevel(client, Float:uberlevel)
{
    if (TF2_GetPlayerClass(client) == TFClass_Medic)
    {
        new index = GetPlayerWeaponSlot(client, 1);
        if (index > 0)
        {
            decl String:classname[50];
            if (GetEdictClassname(index, classname, sizeof(classname)) &&
                StrEqual(classname, "tf_weapon_medigun"))
            {
                SetEntPropFloat(index, Prop_Send, "m_flChargeLevel", uberlevel);
                return true;
            }
        }
    }
    return false;
}

stock TF2_GetNumHealers(client)
{
    return GetEntProp(client, Prop_Send, "m_nNumHealers");
}

/**
 * Get the medic medigun's healing target (if any)
 *
 * @param client 	Client
 * @return			none
 */
stock TF2_GetHealingTarget(client, &bool:ubered = false)
{
    if (GetGameType() == tf2 && IsClientConnected(client) &&
        IsClientInGame(client))
    {
        new weapon = GetEntPropEnt(client, Prop_Send, "m_hActiveWeapon");
        if (weapon > 0 && IsValidEntity(weapon) && IsValidEdict(weapon))
        {
            decl String:classname[64];
            GetEntityNetClass(weapon, classname, sizeof(classname));
            if (!StrEqual(classname, "CWeaponMedigun"))
                return -1;

            ubered = bool:GetEntProp(weapon, Prop_Send, "m_bChargeRelease");
            return GetEntPropEnt(weapon, Prop_Send, "m_hHealingTarget");
        }
    }

    return -1;
}  
