/**
 * vim: set ai et ts=4 sw=4 syntax=sourcepawn :
 * File: clienttimer.inc
 * Description: ClientTimer functions
 * Author(s): Naris (Murray Wilson)
 */
 
#if defined _clienttimer_included
 #endinput
#endif
#define _clienttimer_included

#define ResetClientTimer(%1) g_ClientTimers[%1] = INVALID_HANDLE

stock Handle:g_ClientTimers[MAXPLAYERS+1];

stock CreateClientTimer(client, Float:interval, Timer:func,
                        flags=TIMER_REPEAT|TIMER_FLAG_NO_MAPCHANGE)
{
    if (g_ClientTimers[client] == INVALID_HANDLE)
        g_ClientTimers[client] = CreateTimer(interval,func,GetClientUserId(client),flags);
}

stock KillClientTimer(client)
{
    new Handle:timer=g_ClientTimers[client];
    if (timer != INVALID_HANDLE)
    {
        ResetClientTimer(client);	
        KillTimer(timer);
    }
}

stock KillAllClientTimers()
{
    for (new i = 1; i <= MaxClients; i++)
        KillClientTimer(i);
}

stock ResetAllClientTimers()
{
    for (new i = 1; i <= MaxClients; i++)
        ResetClientTimer(i);
}
