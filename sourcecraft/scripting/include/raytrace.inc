/**
 * vim: set ai et ts=4 sw=4 syntax=sourcepawn :
 * File: raytrace.inc
 * Description: Ray Trace functions and variables
 * Author(s): Naris (Murray Wilson)
 */
 
#if defined _raytrace_included
 #endinput
#endif
#define _raytrace_included

#include <entity_flags>

stock bool:TraceTarget(client, target, Float:clientLoc[3], Float:targetLoc[3])
{
    TR_TraceRayFilter(clientLoc, targetLoc, MASK_SOLID,
                      RayType_EndPoint, TraceRayDontHitSelf,
                      client);

    return (!TR_DidHit() || TR_GetEntityIndex() == target);
}

stock bool:TraceTargetIndex(client, target, Float:clientLoc[3], Float:targetLoc[3])
{
    targetLoc[2] += 50.0; // Adjust trace position of target
    TR_TraceRayFilter(clientLoc, targetLoc, MASK_SOLID,
                      RayType_EndPoint, TraceRayDontHitSelf,
                      client);

    return (!TR_DidHit() || TR_GetEntityIndex() == target);
}

stock bool:TraceTargetEntity(client, target, Float:clientLoc[3], Float:targetLoc[3])
{
    targetLoc[2] += 10.0; // Adjust trace position of target
    TR_TraceRayFilter(clientLoc, targetLoc, MASK_SOLID,
                      RayType_EndPoint, TraceRayDontHitSelf,
                      client);

    return (!TR_DidHit() || TR_GetEntityIndex() == target);
}

stock bool:TraceTargetClients(client, target, Float:clientLoc[3], Float:targetLoc[3])
{
    clientLoc[2] += 50.0; // Adjust trace position to the middle
    targetLoc[2] += 50.0; // of the person instead of the feet.
    TR_TraceRayFilter(clientLoc, targetLoc, MASK_SOLID,
                      RayType_EndPoint, TraceRayDontHitSelf,
                      client);

    return (!TR_DidHit() || TR_GetEntityIndex() == target);
}

stock TraceAimTarget(client)
{
    new Float:clientloc[3],Float:clientang[3];
    GetClientEyePosition(client,clientloc);
    GetClientEyeAngles(client,clientang);
    TR_TraceRayFilter(clientloc, clientang, MASK_SOLID,
                      RayType_Infinite, TraceRayDontHitSelf,
                      client);

    return TR_GetEntityIndex();
}

stock bool:TraceAimPosition(client, Float:destLoc[3], bool:hitPlayers)
{
    new Float:clientloc[3],Float:clientang[3];
    GetClientEyePosition(client,clientloc);
    GetClientEyeAngles(client,clientang);

    if (hitPlayers)
    {
        TR_TraceRayFilter(clientloc, clientang, MASK_SOLID,
                          RayType_Infinite, TraceRayDontHitSelf,
                          client);
    }
    else
    {
        TR_TraceRayFilter(clientloc, clientang, MASK_SOLID,
                          RayType_Infinite, TraceRayDontHitPlayers,
                          client);
    }

    TR_GetEndPosition(destLoc);
    return TR_DidHit();
}

stock IsPlayerStuck(client)
{
    decl Float:vecMin[3], Float:vecMax[3], Float:vecOrigin[3];
    GetClientMins(client, vecMin);
    GetClientMaxs(client, vecMax);
    GetClientAbsOrigin(client, vecOrigin);
    TR_TraceHullFilter(vecOrigin, vecOrigin, vecMin, vecMax,
                       MASK_PLAYERSOLID, TraceRayHitCollidable,
                       client);
    return TR_GetEntityIndex();
}

/***************
 *Trace Filters*
****************/

public bool:TraceRayDontHitPlayers(entity,mask)
{
  // Check if the beam hit a player and tell it to keep tracing if it did
  return (entity <= 0 || entity > MaxClients);
}

public bool:TraceRayDontHitSelf(entity, mask, any:data)
{
    return (entity != data); // Check if the TraceRay hit the owning entity.
}

public bool:TraceRayHitEntities(entity, mask)
{
	return (entity > MaxClients);
}

public bool:TraceRayHitCollidable(entity, mask)
{
    if (entity > MaxClients)
    {
        new m_CollisionGroup = GetEntProp(entity, Prop_Send, "m_CollisionGroup");
        return (m_CollisionGroup != _:COLLISION_GROUP_DEBRIS &&
                m_CollisionGroup != _:COLLISION_GROUP_DEBRIS_TRIGGER);
    }
    else
        return false;
}
