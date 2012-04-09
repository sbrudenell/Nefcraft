/**
 * vim: set ai et ts=4 sw=4 syntax=sourcepawn :
 * File: freeze.inc
 * Description: Freeze functions and variables
 * Author(s): Naris (Murray Wilson)
 */
 
#if defined _freeze_included
 #endinput
#endif
#define _freeze_included

#include "sc/burrow"

stock FreezeEntity(entity)
{
    SetEntityMoveType(entity,MOVETYPE_NONE); // Freeze client
}

stock UnFreezeEntity(entity)
{
    SetEntityMoveType(entity,MOVETYPE_WALK); // Unfreeze client
}

public Action:UnfreezePlayer(Handle:timer,any:userid)
{
    new client = GetClientOfUserId(userid);
    if (client && !IsBurrowed(client))
        UnFreezeEntity(client);

    return Plugin_Stop;
}
