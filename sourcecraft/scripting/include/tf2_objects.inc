/**
 * vim: set ai et ts=4 sw=4 syntax=sourcepawn :
 * File: tf2_objects.inc
 * Description: Functions to return infomation about TF2 objects.
 * Author(s): Naris (Murray Wilson)
 */

#if defined _tf2_objects_included
 #endinput
#endif
#define _tf2_objects_included

#include <tf2>

enum TFExtObjectType
{
    TFExtObject_Unknown = -1,
	TFExtObject_CartDispenser = 0,
    TFExtObject_Dispenser = 0,
    TFExtObject_Teleporter = 1,
    TFExtObject_Sentry = 2,
    TFExtObject_Sapper = 3,
    TFExtObject_TeleporterEntry,
    TFExtObject_TeleporterExit,
    TFExtObject_MiniSentry,
    TFExtObject_Amplifier,
    TFExtObject_RepairNode
};

#define TFObject_Unknown            (TFObjectType:TFExtObject_Unknown)
#define TFObject_TeleporterEntry    (TFObjectType:TFExtObject_TeleporterEntry)
#define TFObject_TeleporterExit     (TFObjectType:TFExtObject_TeleporterExit)
#define TFObject_MiniSentry         (TFObjectType:TFExtObject_MiniSentry)
#define TFObject_Amplifier          (TFObjectType:TFExtObject_Amplifier)
#define TFObject_RepairNode         (TFObjectType:TFExtObject_RepairNode)

stock const String:TF2_ObjectClassNames[TFExtObjectType][] =
{
    "obj_dispenser",
    "obj_teleporter",
    "obj_sentrygun",
    "obj_sapper",
    "obj_teleporter", // _entrance
    "obj_teleporter", // _exit
    "obj_sentrygun",  // minisentry
    "obj_dispenser",  // amplifier
    "obj_dispenser"   // repair_node
};

stock const String:TF2_ObjectNames[TFExtObjectType][] =
{
    "Dispenser",
    "Teleporter",
    "Sentry Gun",
    "Sapper",
    "Teleporter Entrance",
    "Teleporter Exit",
    "Mini Sentry Gun",
    "Amplifier",
    "Repair Node"
};

stock TF2_ObjectModes[TFExtObjectType] =
{
    -1, // dispenser
    -1, // teleporter (either)
    -1, // sentrygun
    -1, // sapper
     0, // telporter_entrance
     1, // teleporter_exit
    -1, // minisentry
    -1, // amplifier
    -1 // repair_node
};

// Max Sentry Ammo for Level:         mini,   1,   2,   3,  max
stock const TF2_MaxSentryShells[]  = { 150, 100, 120, 144,  255 };
stock const TF2_MaxSentryRockets[] = {   0,   0,   0,  20,   63 };
stock const TF2_SentryHealth[]     = { 100, 150, 180, 216, 8191 };

stock const TF2_MaxUpgradeMetal    = 200;
stock const TF2_MaxDispenserMetal  = 400;

stock TFExtObjectType:TF2_GetExtObjectType(entity, bool:specific=false)
{
    new offset = GetEntSendPropOffs(entity, "m_iObjectType");
    if (offset <= 0)
        return TFExtObject_Unknown;
    else
    {
        new TFExtObjectType:type = TFExtObjectType:GetEntData(entity, offset);
        if (specific)
        {
            if (type == TFExtObject_Teleporter)
            {
                type = (TF2_GetObjectMode(entity) == TFObjectMode_Exit)
                       ? TFExtObject_TeleporterExit
                       : TFExtObject_TeleporterEntry;
            }
            else if (type == TFExtObject_Sentry)
            {
                if (GetEntProp(entity, Prop_Send, "m_bMiniBuilding"))
                    type = TFExtObject_MiniSentry;
            }
        }
        return type;
    }
}

stock TFExtObjectType:TF2_GetExtObjectTypeFromClass(entity, bool:specific=false)
{
    decl String:class[32];
    if (GetEdictClassname(entity,class,sizeof(class)))
    {
        if (StrEqual(class, TF2_ObjectClassNames[TFObject_Sentry]))
        {
            return (specific) ? (GetEntProp(entity, Prop_Send, "m_bMiniBuilding")
                                ? TFExtObject_MiniSentry : TFExtObject_Sentry)
                              : TFExtObject_Sentry;
        }
        else if (StrEqual(class, TF2_ObjectClassNames[TFExtObject_Dispenser]))
            return TFExtObject_Dispenser;
        else if (StrEqual(class, TF2_ObjectClassNames[TFExtObject_Teleporter]))
        {
            return (specific) ? (TF2_GetObjectMode(entity) == TFObjectMode_Exit)
                                ? TFExtObject_TeleporterExit : TFExtObject_TeleporterEntry
                              : TFExtObject_Teleporter;
        }
        else if (StrEqual(class, TF2_ObjectClassNames[TFExtObject_Sapper]))
            return TFExtObject_Sapper;
        else
            return TFExtObject_Unknown;
    }
    else
        return TFExtObject_Unknown;
}

