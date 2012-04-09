/**
 * vim: set ai et ts=4 sw=4 syntax=sourcepawn :
 * File: defines.inc
 * Description: Define various symbols used by SourceCraft primitives
 * Author(s): -=|JFH|=-Naris (Murray Wilson)
 */

#if defined _defines_included
 #endinput
#endif
#define _defines_included

// Define cost_flags used by AddUpgrade() native
#define COST_PER_USE        0
#define COST_TO_PURCHASE    1
#define COST_IS_REFUNDABLE  2

// Define fileIds used by Config natives
#define MAIN               -1
#define SHOPITEM            0

// Define nodes used by Config natives
#define NONE               -1

enum Translation
{
    Translation_Default = -1,
    Translation_None = 0,
    Translation_Auto,
    Translation_Custom,
    Translation_SC,
    Translation_W3S
};
