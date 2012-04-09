/**
 * vim: set ai et ts=4 sw=4 syntax=sourcepawn :
 * File: BeamSprite.inc
 * Description: Define Beam Sprite
 * Author(s): Naris (Murray Wilson)
 */
 
#if defined _BeamSprite_included
 #endinput
#endif
#define _BeamSprite_included

#include <ResourceManager>

#define SetupBeamSprite(%1,%2)  SetupModel(g_beamModel, g_beamSprite, false, %1, %2)
#define BeamSprite()            PrepareModel(g_beamModel, g_beamSprite)

stock g_beamSprite;
stock const String:g_beamModel[] = "materials/sprites/laser.vmt";
