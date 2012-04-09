/**
 * vim: set ai et ts=4 sw=4 syntax=sourcepawn :
 * File: SpeedBoost.inc
 * Description: The Speed Boost Ability for SourceCraft.
 * Author(s): Naris (Murray Wilson)
 */

#if defined _SpeedBoost_included
 #endinput
#endif
#define _SpeedBoost_included

#include "effect/Smoke"
#include "effect/HaloSprite"
#include "effect/SendEffects"

stock g_bootsItem = -1;

stock SetupSpeed(bool:precache, bool:preload=false)
{
    SetupHaloSprite(precache, preload);
    SetupSmokeSprite(precache, preload);
}

stock SpeedBoost(client, Float:speed, bool:apply)
{
    if (speed >= 0.0 && speed != 1.0)
    {
        /* If the Player also has the Boots of Speed,
        * Increase the speed further
        */
        if (g_bootsItem < 0)
            g_bootsItem = FindShopItem("boots");

        if (g_bootsItem != -1 && GetOwnsItem(client,g_bootsItem))
            speed *= 1.1;

        SetSpeed(client,speed,apply);

        if (IsClientInGame(client) && IsPlayerAlive(client))
        {
            new Float:start[3];
            GetClientAbsOrigin(client, start);

            static const color[4] = { 255, 100, 0, 255 };
            TE_SetupBeamRingPoint(start, 20.0, 60.0, SmokeSprite(), HaloSprite(),
                                  0, 1, 1.0, 4.0, 0.0, color, 10, 0);
            TE_SendEffectToAll();
        }
    }
    else
        SetSpeed(client,-1.0,apply);
}

stock LookupSpeedBoost(client, level, bool:apply, const Float:speedLevels[], maxLevel)
{
    // speedLevels is an array of speed % modifiers / level such as:
    // new const Float:speedLevels[] = { 1.0, 1.05, 1.10, 1.15, 1.20 };
    if ( level > maxLevel)
        SpeedBoost(client,speedLevels[maxLevel], apply);
    else if (level >= 0)
        SpeedBoost(client,speedLevels[level], apply);
    else
        SetSpeed(client,-1.0,apply);
}

#define SetSpeedBoost(%1,%2,%3,%4) LookupSpeedBoost(%1,%2,%3,%4,sizeof(%4))
