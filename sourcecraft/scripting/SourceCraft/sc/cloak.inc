/**
 * vim: set ai et ts=4 sw=4 syntax=sourcepawn :
 * File: cloak.inc
 * Description: The Cloak upgrade for SourceCraft.
 * Author(s): -=|JFH|=-Naris
 */
 
#if defined _cloak_included
 #endinput
#endif
#define _cloak_included

#include <sidewinder>

#include "effect/Lightning"
#include "effect/SendEffects"

#define SetupCloak SetupLightning

#if !defined m_SidewinderAvailable
    stock bool:m_SidewinderAvailable  = false;
#endif

stock bool:cfgAllowInvisibility = true;

stock g_cloakItem = -1;

stock Cloak(client, level, bool:apply=false)
{
    static const alphaTable[]          = { 0, 255, 235, 215, 200};
    static const Float:delayTable[]    = { 0.0, 2.0, 1.5, 1.0, 0.5 };
    static const Float:durationTable[] = { 0.0, 5.0, 10.0, 15.0, 20.0 }

    static const RenderFx:fx[] = { RENDERFX_NONE,
                                   RENDERFX_FLICKER_FAST,
                                   RENDERFX_STROBE_FAST,
                                   RENDERFX_FLICKER_SLOW,
                                   RENDERFX_STROBE_SLOW};
    if (level > 0)
    {
        new alpha = alphaTable[level];
        new Float:delay = delayTable[level];
        new Float:duration = durationTable[level];

        /* If the Player also has the Cloak of Shadows,
         * Decrease the delay and Increase the duration.
         */
        if (g_cloakItem < 0)
            g_cloakItem = FindShopItem("cloak");

        if (g_cloakItem != -1 && GetOwnsItem(client,g_cloakItem))
        {
            alpha    *= 0.90;
            delay    *= 0.90;
            duration *= 1.10;
        }

        new Float:start[3];
        GetClientAbsOrigin(client, start);

        new lightning = Lightning();
        static const color[4] = { 0, 255, 50, 128 };
        TE_SetupBeamRingPoint(start, 30.0, 60.0, lightning, lightning,
                              0, 1, 2.0, 10.0, 0.0, color, 10, 0);
        TE_SendEffectToAll();

        if (cfgAllowInvisibility)
        {
            SetVisibility(client, TimedMeleeInvisibility, alpha,
                          delay, duration, RENDER_TRANSCOLOR,
                          fx[level], .apply=apply);
        }
        else if (apply)
            ApplyPlayerSettings(client);

        if (m_SidewinderAvailable)
            SidewinderCloakClient(client, true);
    }
    else
    {
        SetVisibility(client, NormalVisibility, .apply=apply);
        if (m_SidewinderAvailable)
            SidewinderCloakClient(client, false);
    }
}

stock AlphaCloak(client, level, bool:allowInvisibility, bool:apply=false)
{
    static const alphaTable[]        = { 0, 215, 190, 140, 100};
    static const Float:factorTable[] = { 0.0, 0.9, 0.8, 0.7, 0.6 };

    if (level > 0)
    {
        new alpha = alphaTable[level];
        new Float:melee = factorTable[level];
        new Float:movement = factorTable[level];

        /* If the Player also has the Cloak of Shadows,
         * Decrease the delay and Increase the duration.
         */
        new cloak = FindShopItem("cloak");
        if (cloak != -1 && GetOwnsItem(client,cloak))
        {
            alpha    *= 0.90;
            melee    *= 0.90;
            movement *= 1.10;
        }

        new Float:start[3];
        GetClientAbsOrigin(client, start);

        new lightning = Lightning();
        static const color[4] = { 0, 255, 50, 128 };
        TE_SetupBeamRingPoint(start, 30.0, 60.0, lightning, lightning,
                              0, 1, 2.0, 10.0, 0.0, color, 10, 0);
        TE_SendEffectToAll();

        if (cfgAllowInvisibility)
        {
            SetVisibility(client, BasicVisibility, alpha, 
                          melee, movement, RENDER_TRANSCOLOR,
                          .apply=apply);
        }
        else
            SetVisibility(client, NormalVisibility, .apply=apply);

        if (m_SidewinderAvailable)
            SidewinderCloakClient(client, true);
    }
    else
    {
        SetVisibility(client, NormalVisibility, .apply=apply);
        if (m_SidewinderAvailable)
            SidewinderCloakClient(client, false);
    }
}
