/**
 * vim: set ai et ts=4 sw=4 syntax=sourcepawn :
 * File: SteamSprite.inc
 * Description: Define Steam Sprite
 * Author(s): Naris (Murray Wilson)
 */
 
#if defined _SteamSprite_included
 #endinput
#endif
#define _SteamSprite_included

#include <ResourceManager>

#define SetupSteamSprite(%1,%2) SetupModel(g_steamModel, g_steamSprite, false, %1, %2)
#define SteamSprite()           PrepareModel(g_steamModel, g_steamSprite)

stock g_steamSprite;
stock const String:g_steamModel[] = "materials/sprites/steam1.vmt";
