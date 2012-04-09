/**
 * vim: set ai et ts=4 sw=4 syntax=sourcepawn :
 * File: HealthParticle.inc
 * Description: Functions to show TF2 Health particles
 * Author(s): L. Duke
 */
 
#if defined _HealthParticle_included
 #endinput
#endif
#define _HealthParticle_included

#include <gametype>
#include <particle>
#include <entlimit>

stock g_HealthParticle[MAXPLAYERS+1];

// show a health sign above client's head
stock ShowHealthParticle(client)
{
    if (GetGameType() == tf2 &&
        g_HealthParticle[client] <= 0 &&
        !GetAttribute(client,Attribute_IsBurrowed) &&
        !IsEntLimitReached(.client=client, .message="Unable to spawn anymore health particles") &&
        IsClientInGame(client) && IsPlayerAlive(client))
    {
        switch (GetClientTeam(client))
        {
            case 2:
            {
                g_HealthParticle[client] = CreateParticle("healthgained_red", 1.0,
                                                          client, Attach, "head",
                                                          .deleteFunc=DeleteHealthParticles);
            }
            case 3:
            {
                g_HealthParticle[client] = CreateParticle("healthgained_blu", 1.0,
                                                          client, Attach, "head",
                                                          .deleteFunc=DeleteHealthParticles);
            }                                                              
        }
    }
}

stock DeleteHealthParticle(client)
{
    new particle = g_HealthParticle[client];
	if (particle != 0)
	{
		DeleteParticle(particle);
		g_HealthParticle[client] = 0;
	}
}

public Action:DeleteHealthParticles(Handle:timer, any:particle)
{
    DeleteParticle(particle);

    for (new i = 1; i <= MAXPLAYERS; i++)
    {
        if (g_HealthParticle[i] == particle)
        {
            g_HealthParticle[i] = 0;
            break;
        }
    }
}
