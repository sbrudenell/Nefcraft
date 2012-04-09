/**
 * vim: set ai et ts=4 sw=4 syntax=sourcepawn :
 * File: SendEffects.inc
 * Description: Stocks to TE_Send() tempents based on settings.
 * Author(s): Naris (Murray Wilson)
 */
 
#if defined _SendEffects_included
 #endinput
#endif
#define _SendEffects_included

#include <ResourceManager>

/**
 * Sets up the list[] and alt_list[] for beacon effects.
 */
stock SetupBeaconLists(list[], alt_list[], &count, &alt_count)
{
    count = alt_count = 0;
    new maxClients = GetMaxClients();
    for (new i=1; i<=maxClients; i++)
    {
        if (IsClientInGame(i) && !IsFakeClient(i))
        {
            if (!GetSetting(i, Remove_Queasiness) &&
                !GetSetting(i, Disable_Beacons))
            {
                if (GetSetting(i, Reduce_Queasiness))
                    alt_list[alt_count++] = i;
                else
                    list[count++] = i;
            }
        }
    }
}

/**
 * Sets up the list[] and alt_list[] for offensive beacon effects.
 */
stock SetupOBeaconLists(list[], alt_list[], &count, &alt_count, client=0)
{
    count = alt_count = 0;
    new maxClients = GetMaxClients();
    for (new i=1; i<=maxClients; i++)
    {
        if (IsClientInGame(i) && !IsFakeClient(i))
        {
            if (!GetSetting(i, Remove_Queasiness))
            {
                if (i == client)
                {
                    if (!GetSetting(i, Disable_Beacons))
                    {
                        if (GetSetting(i, Reduce_Queasiness))
                            alt_list[alt_count++] = i;
                        else
                            list[count++] = i;
                    }
                }
                else if (!GetSetting(i, Disable_OBeacons))
                {
                    if (GetSetting(i, Reduce_Queasiness))
                        alt_list[alt_count++] = i;
                    else
                        list[count++] = i;
                }
            }
        }
    }
}



/**
 * Broadcasts the current beacon temp entity to all clients
 * that don't have Disable_Beacons set.
 * @note See TE_Start().
 *
 * @param delay		Delay in seconds to send the TE.
 * @noreturn
 */
stock TE_SendBeaconToAll(Float:delay=0.0)
{
	new maxClients = GetMaxClients();
	new total = 0;
	new clients[maxClients];
	for (new i=1; i<=maxClients; i++)
	{
		if (IsClientInGame(i) && !IsFakeClient(i) &&
            !GetSetting(i, Disable_Beacons))
		{
			clients[total++] = i;
		}
	}
	return TE_Send(clients, total, delay);
}

/**
 * Broadcasts the current beacon temp entity to all clients
 * that don't have Disable_Beacons set.
 * @note See TE_Start().
 *
 * @param delay		Delay in seconds to send the TE.
 * @noreturn
 */
stock TE_SendOBeaconToAll(client=0, Float:delay=0.0)
{
    new maxClients = GetMaxClients();
    new total = 0;
    new clients[maxClients];
    for (new i=1; i<=maxClients; i++)
    {
        if (IsClientInGame(i) && !IsFakeClient(i))
        {
            if (i == client)
            {
                if (!GetSetting(i, Disable_Beacons))
                    clients[total++] = i;
            }
            else if (!GetSetting(i, Disable_OBeacons))
            {
                clients[total++] = i;
            }
        }
    }
    return TE_Send(clients, total, delay);
}

/**
 * Broadcasts the current (posssibly queasy Effect) temp entity to all clients
 * that don't have Disable_Beacons set.
 * @note See TE_Start().
 *
 * @param delay		Delay in seconds to send the TE.
 * @noreturn
 */
stock TE_SendQEffectToAll(client=0, victim=0, Float:delay=0.0)
{
    new maxClients = GetMaxClients();
    new total = 0;
    new clients[maxClients];
    for (new i=1; i<=maxClients; i++)
    {
        if (IsClientInGame(i) && !IsFakeClient(i))
        {
            if (!GetSetting(i, Disable_Effects) &&
                ((i != client && i != victim) ||
                 (!GetSetting(i, Reduce_Queasiness) &&
                  !GetSetting(i, Remove_Queasiness))))
            {
                clients[total++] = i;
            }
        }
    }
    return TE_Send(clients, total, delay);
}

/**
 * Broadcasts the current (Effect) temp entity to all clients
 * that don't have Disable_Beacons set.
 * @note See TE_Start().
 *
 * @param delay		Delay in seconds to send the TE.
 * @noreturn
 */
stock TE_SendEffectToAll(Float:delay=0.0)
{
	new maxClients = GetMaxClients();
	new total = 0;
	new clients[maxClients];
	for (new i=1; i<=maxClients; i++)
	{
		if (IsClientInGame(i) && !IsFakeClient(i) &&
            !GetSetting(i, Disable_Effects))
		{
			clients[total++] = i;
		}
	}
	return TE_Send(clients, total, delay);
}

/**
 * Broadcasts the current (posssibly queasy Effect) temp entity to all clients
 * that don't have Disable_Beacons set.
 * @note See TE_Start().
 *
 * @param delay		Delay in seconds to send the TE.
 * @noreturn
 */
stock TE_SendEffectToOthers(client=0, victim=0, Float:delay=0.0)
{
    new maxClients = GetMaxClients();
    new total = 0;
    new clients[maxClients];
    for (new i=1; i<=maxClients; i++)
    {
        if (IsClientInGame(i) && !IsFakeClient(i))
        {
            if ((i != client && i != victim) &&
                !GetSetting(i, Disable_Effects))
            {
                clients[total++] = i;
            }
        }
    }
    return TE_Send(clients, total, delay);
}
