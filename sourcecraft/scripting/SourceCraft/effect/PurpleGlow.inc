/**
 * vim: set ai et ts=4 sw=4 syntax=sourcepawn :
 * File: PurpleGlow.inc
 * Description: Define Purple Glow
 * Author(s): Naris (Murray Wilson)
 */
 
#if defined _PurpleGlow_included
 #endinput
#endif
#define _PurpleGlow_included

#include <ResourceManager>

#define SetupPurpleGlow(%1,%2)  SetupModel(g_purpleModel, g_purpleGlow, false, %1, %2)
#define PurpleGlow()            PrepareModel(g_purpleModel, g_purpleGlow)

stock g_purpleGlow;
stock const String:g_purpleModel[] = "materials/sprites/purpleglow1.vmt";
