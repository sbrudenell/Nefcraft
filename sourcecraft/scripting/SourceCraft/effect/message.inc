/**
 * vim: set ai et ts=4 sw=4 syntax=sourcepawn :
 * File: message.inc
 * Description: User Message stocks
 * Author(s): Naris (Murray Wilson)
 */

#if defined _message_included
#endinput
#endif
#define _message_included

/**
 * Starts a usermessage (network message) that broadcasts to all living clients.
 * @note See StartMessage or StartMessageEx().
 *
 * @param msgname		Message name to start.
 * @param flags			Optional flags to set.
 * @return				A handle to a bf_write bit packing structure, or
 *						INVALID_HANDLE on failure.
 */
stock Handle:StartMessageAllLiving(String:msgname[], flags=0)
{
    new total = 0;
    new clients[MaxClients];
    for (new i=1; i<=MaxClients; i++)
    {
        if (IsClientConnected(i) && IsClientInGame(i) &&
            IsPlayerAlive(i) && !IsFakeClient(i))
        {
            clients[total++] = i;
        }
    }

    if (total > 0)
        return StartMessage(msgname, clients, total, flags);
    else
        return INVALID_HANDLE;
}

/**
 * Starts a usermessage (network message) that broadcasts to all living clients.
 * that don't have blinders or queasiness settings.
 * @note See StartMessage or StartMessageEx().
 *
 * @param msgname		Message name to start.
 * @param flags			Optional flags to set.
 * @return				A handle to a bf_write bit packing structure, or
 *						INVALID_HANDLE on failure.
 */
stock Handle:StartMessageAllAllowed(String:msgname[], flags=0)
{
    new total = 0;
    new clients[MaxClients];
    for (new i=1; i<=MaxClients; i++)
    {
        if (IsClientConnected(i) && IsClientInGame(i) &&
            IsPlayerAlive(i) && !IsFakeClient(i) &&
            !GetSetting(i,Remove_Queasiness) &&
            !GetImmunity(i,Immunity_Drugs))
        {
            clients[total++] = i;
        }
    }

    if (total > 0)
        return StartMessage(msgname, clients, total, flags);
    else
        return INVALID_HANDLE;
}
