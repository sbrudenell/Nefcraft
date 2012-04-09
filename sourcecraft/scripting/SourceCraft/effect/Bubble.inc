/**
 * vim: set ai et ts=4 sw=4 syntax=sourcepawn :
 * File: bubble.inc
 * Description: Declares the bubble sprite for SourceCraft.
 * Author(s): Naris (Murray Wilson)
 */

#if defined _bubble_included
 #endinput
#endif
#define _bubble_included

#include <ResourceManager>

stock g_bubbleModel;

stock const String:g_bubbleModelName[] = "materials/effects/bubble.vmt";

#define SetupBubbleModel(%1,%2) SetupModel(g_bubbleModelName, g_bubbleModel, false, %1, %2)
#define BubbleModel()           PrepareModel(g_bubbleModelName, g_bubbleModel)
