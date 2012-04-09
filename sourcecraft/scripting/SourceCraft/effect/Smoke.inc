/**
 * vim: set ai et ts=4 sw=4 syntax=sourcepawn :
 * File: smoke.inc
 * Description: Declares the smoke sprite for SourceCraft.
 * Author(s): Naris (Murray Wilson)
 */

#if defined _smoke_included
 #endinput
#endif
#define _smoke_included

#include <ResourceManager>

stock g_smokeSprite;

stock const String:g_smokeModel[] = "materials/sprites/smoke.vmt";

#define SetupSmokeSprite(%1,%2)    SetupModel(g_smokeModel, g_smokeSprite, false, %1, %2)
#define SmokeSprite()           PrepareModel(g_smokeModel, g_smokeSprite)
