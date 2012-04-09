/**
 * vim: set ai et ts=4 sw=4 syntax=sourcepawn :
 * File: tf2_meter.inc
 * Description: Functions to return information about TF2 cloak, rage and energy drink meters.
 * Author(s): Naris (Murray Wilson)
 */
 
#if defined _tf2_meter_included
 #endinput
#endif
#define _tf2_meter_included

#include <sdktools>

stock Float:TF2_GetCloakMeter(client)
{
    return GetEntPropFloat(client, Prop_Send, "m_flCloakMeter");
}

stock TF2_SetCloakMeter(client,Float:cloakMeter)
{
    SetEntPropFloat(client, Prop_Send, "m_flCloakMeter", cloakMeter);
}

stock Float:TF2_GetRageMeter(client)
{
    return GetEntPropFloat(client, Prop_Send, "m_flRageMeter");
}

stock TF2_SetRageMeter(client,Float:rageMeter)
{
    SetEntPropFloat(client, Prop_Send, "m_flRageMeter", rageMeter);
}

stock Float:TF2_GetEnergyDrinkMeter(client)
{
    return GetEntPropFloat(client, Prop_Send, "m_flEnergyDrinkMeter");
}

stock TF2_SetEnergyDrinkMeter(client,Float:energyDrinkMeter)
{
    SetEntPropFloat(client, Prop_Send, "m_flEnergyDrinkMeter", energyDrinkMeter);
}
