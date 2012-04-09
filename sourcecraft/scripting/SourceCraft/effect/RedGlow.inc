/**
 * vim: set ai et ts=4 sw=4 syntax=sourcepawn :
 * File: RedGlow.inc
 * Description: Define Red Glow
 * Author(s): Naris (Murray Wilson)
 */
 
#if defined _RedGlow_included
 #endinput
#endif
#define _RedGlow_included

#include <ResourceManager>

#define SetupRedGlow(%1,%2) SetupModel(g_redModel, g_redGlow, false, %1, %2)
#define RedGlow()           PrepareModel(g_redModel, g_redGlow)

stock g_redGlow;
stock const String:g_redModel[] = "materials/sprites/redglow1.vmt";
