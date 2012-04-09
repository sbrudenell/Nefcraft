/**
 * vim: set ai et ts=4 sw=4 syntax=sourcepawn :
 * File: LaserSprite.inc
 * Description: Define Airlock Laser Sprite
 * Author(s): Naris (Murray Wilson)
 */
 
#if defined _LaserSprite_included
 #endinput
#endif
#define _LaserSprite_included

#include <ResourceManager>

#define SetupLaserSprite(%1,%2) SetupModel(g_laserModel, g_laserSprite, false, %1, %2)
#define LaserSprite()           PrepareModel(g_laserModel, g_laserSprite)

stock g_laserSprite;
stock const String:g_laserModel[] = "materials/models/props_lab/airlock_laser.vmt";
