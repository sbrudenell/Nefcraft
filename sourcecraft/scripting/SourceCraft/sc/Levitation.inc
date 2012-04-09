/**
 * vim: set ai et ts=4 sw=4 syntax=sourcepawn :
 * File: Levitation.inc
 * Description: The Levitation Ability for SourceCraft.
 * Author(s): Naris (Murray Wilson)
 */

#if defined _Levitation_included
 #endinput
#endif
#define _Levitation_included

#include "effect/Lightning"
#include "effect/HaloSprite"
#include "effect/SendEffects"

stock g_sockItem = -1;

stock SetupLevitation(bool:precache, bool:preload=false)
{
    SetupLightning(precache, preload);
    SetupHaloSprite(precache, preload);
}

stock Levitation(client, Float:gravity, bool:apply=false)
{
    if (gravity >= 0.0 && gravity != 1.0)
    {
        /* If the Player also has the Sock of the Feather,
         * Decrease the gravity further.
         */
        if (g_sockItem < 0)
            g_sockItem = FindShopItem("sock");

        if (g_sockItem != -1 && GetOwnsItem(client,g_sockItem))
            gravity *= 0.8;

        SetGravity(client,gravity,apply);

        if (IsPlayerAlive(client))
        {
            new Float:start[3];
            GetClientAbsOrigin(client, start);

            static const color[4] = { 0, 20, 100, 255 };
            TE_SetupBeamRingPoint(start, 20.0, 50.0, Lightning(), HaloSprite(),
                                  0, 1, 2.0, 60.0, 0.8 ,color, 10, 1);
            TE_SendEffectToAll();
        }
    }
    else
        SetGravity(client,-1.0,apply);
}

stock LookupLevitation(client, level, bool:apply, const Float:levitationLevels[], maxLevel)
{
    // speedLevels is an array of gravity % modifiers / level such as:
    // static const Float:levitationLevels[] = { 0.92, 0.733, 0.5466, 0.36, 0.26 };
    if (level > maxLevel)
        Levitation(client,levitationLevels[maxLevel], apply);
    else if (level >= 0)
        Levitation(client,levitationLevels[level], apply);
    else
        Levitation(client,-1.0,apply);
}

#define SetLevitation(%1,%2,%3,%4) LookupLevitation(%1,%2,%3,%4,sizeof(%4))
