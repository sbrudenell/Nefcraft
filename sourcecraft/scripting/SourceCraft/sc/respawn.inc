/**
 * vim: set ai et ts=4 sw=4 syntax=sourcepawn :
 * File: respawn.inc
 * Description: Respawn functions
 * Author(s): Naris (Murray Wilson)
 */
 
#if defined _respawn_included
 #endinput
#endif
#define _respawn_included

#include <trace>
#include <gametype>

#undef REQUIRE_EXTENSIONS
#include <tf2_player>
#define REQUIRE_EXTENSIONS

#include "sc/invuln"

#include "effect/PurpleGlow"
#include "effect/SendEffects"

#define SetupRespawn SetupPurpleGlow

#if !defined _ShopItems
    #include "sc/ShopItems"
#endif    

// Reincarnation variables
stock m_ReincarnationCount[MAXPLAYERS+1];
stock bool:m_IsRespawning[MAXPLAYERS+1];
stock Float:m_DeathLoc[MAXPLAYERS+1][3];
stock Float:m_DeathAng[MAXPLAYERS+1][3];
stock bool:m_WasMole[MAXPLAYERS+1];

#if defined _TRACE
    stock m_SpawnCount[MAXPLAYERS+1]; // For Debugging
#endif    

stock GetNumberSuffix(number, String:suffix[], suffix_size)
{
    switch (number)
    {
        case 1: strcopy(suffix, suffix_size, "st");
        case 2: strcopy(suffix, suffix_size, "nd");
        case 3: strcopy(suffix, suffix_size, "rd");
        default: strcopy(suffix, suffix_size, "th");
    }
}

stock Respawn(client)
{
    TraceInto("Respawn", "Respawn", "client=%d:%N", \
              client,ValidClientIndex(client));

    m_ReincarnationCount[client]++;

    m_IsRespawning[client]=true;

    #if defined _ShopItems
        m_WasMole[client]=m_IsMole[client];
    #else        
        m_WasMole[client]=IsMole(client);
    #endif

    GetClientAbsOrigin(client, m_DeathLoc[client]);
    GetClientAbsAngles(client, m_DeathAng[client]);
    CreateTimer(0.1, RespawnPlayerHandler, GetClientUserId(client),
                TIMER_FLAG_NO_MAPCHANGE);

    TraceReturn("Respawn %N for the %d time", \
                client,m_ReincarnationCount[client]);
}

stock Respawned(client,bool:reposition)
{
    TraceInto("Respawn", "Respawned", "client=%d:%N", \
              client,ValidClientIndex(client));

    if (IsChangingClass(client))
    {
        ResetInvuln(INVALID_HANDLE,client);
        Trace("%N changed class", client);
    }
    else if (m_IsRespawning[client])
    {
        m_IsRespawning[client]=false;

        #if defined _TRACE
            m_SpawnCount[client]++;
        #endif

        if (reposition)
        {
            // Avoid the TF2 Heavy spinning minigun bug.
            if (GetGameType() == tf2 && TF2_IsPlayerSlowed(client) &&
                TF2_GetPlayerClass(client) == TFClass_Heavy)
            {
                // Stun the heavy briefly to stop the minigun from spinning
                TF2_StunPlayer(client, 0.2, 0.0, TF_STUNFLAG_THIRDPERSON);
            }

            CreateTimer(0.1, RespawnedPlayerHandler, GetClientUserId(client),
                        TIMER_FLAG_NO_MAPCHANGE);

            Trace("%N respawned for the %d(%d) time and was teleported", \
                  client, m_ReincarnationCount[client], m_SpawnCount[client]);
        }
        else
        {
            GetClientAbsOrigin(client, m_DeathLoc[client]);
            ResetInvuln(INVALID_HANDLE,client);

            Trace("%N respawned for the %d(%d) time, but was NOT teleported", \
                  client, m_ReincarnationCount[client], m_SpawnCount[client]);
        }

        TE_SetupGlowSprite(m_DeathLoc[client],PurpleGlow(),1.0,3.5,150);
        TE_SendEffectToAll();
    }
    else
    {
        ResetInvuln(INVALID_HANDLE,client);

        Trace("%N spawned, counts=(%d,%d)", \
              client, m_ReincarnationCount[client], m_SpawnCount[client]);
    }

    TraceReturn();
}

public Action:RespawnPlayerHandler(Handle:timer,any:userid)
{
    new client = GetClientOfUserId(userid);
    if (client)
    {
        RespawnPlayer(client);
    }
    return Plugin_Stop;
}

public Action:RespawnedPlayerHandler(Handle:timer,any:userid)
{
    new client = GetClientOfUserId(userid);
    if (IsValidClient(client))
    {
        TeleportEntity(client,m_DeathLoc[client], m_DeathAng[client], NULL_VECTOR);
        SetInvulnerability(client, 0.5);

        if (m_WasMole[client])
        {
            #if defined _ShopItems
                m_IsMole[client] = m_WasMole[client];
            #else
                SetMole(m_WasMole[client]);
            #endif
        }
    }
    return Plugin_Stop;
}
