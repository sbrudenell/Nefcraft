/**
 * vim: set ai et ts=4 sw=4 syntax=sourcepawn :
 * File: client.inc
 * Description: Client functions
 * Author(s): Naris (Murray Wilson)
 */
 
#if defined _client_included
 #endinput
#endif
#define _client_included

#define IsValidClient(%1) (%1 > 0 && %1 <= MaxClients && IsClientInGame(%1))
#define ValidClientIndex(%1) (IsValidClient(%1) ? %1 : 0)

