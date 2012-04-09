/**
 * vim: set ai et ts=4 sw=4 syntax=sourcepawn :
 * File: tf2_flag.inc
 * Description: Defines function to determine if client has the flag
 * Author(s): -=|JFH|=-Naris (Murray Wilson)
 */

#if defined _tf2_flag_included
 #endinput
#endif
#define _tf2_flag_included

stock bool:TF2_HasTheFlag(client)
{
    new ent = -1;
    while ((ent = FindEntityByClassname(ent, "item_teamflag")) != -1)
    {
        if (GetEntPropEnt(ent, Prop_Data, "m_hOwnerEntity")==client)
            return true;
    }
    return false;
}

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
