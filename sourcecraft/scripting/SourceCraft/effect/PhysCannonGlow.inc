/**
 * vim: set ai et ts=4 sw=4 syntax=sourcepawn :
 * File: PhysCannonGlow.inc
 * Description: Define PhysCannon Glow
 * Author(s): Naris (Murray Wilson)
 */
 
#if defined _PhysCannonGlow_included
 #endinput
#endif
#define _PhysCannonGlow_included

#include <ResourceManager>

#define SetupPhysCannonGlow(%1,%2)  SetupModel(g_physCannonModel, g_physCannonGlow, false, %1, %2)
#define PhysCannonGlow()            PrepareModel(g_physCannonModel, g_physCannonGlow)

stock g_physCannonGlow;
stock const String:g_physCannonModel[] = "materials/sprites/physcannon_blueglow.vmt";
