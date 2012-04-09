/**
 * vim: set ai et ts=4 sw=4 syntax=sourcepawn :
 * File: MindControl.inc
 * Description: Native Interface to MindControl
 * Author(s): -=|JFH|=-Naris (Murray Wilson)
 */

#if defined _mindcontrol_included
 #endinput
#endif
#define _mindcontrol_included

#include <tf2_objects>

/**
 * MindControl a Targetted Object.
 *
 * @param client 	Client index
 * @param range     Range of MindControl
 * @param percent   Percent change of success.
 * @param builder   Set to builder of object controlled.
 * @param type      Set to type of object controlled.
 * @return			returns true for success.
 */
native bool:MindControl(client, Float:range, percent, &builder=0, &TFObjectType:type=TFObject_Unknown);

/**
 * MindControl a Specified Object.
 *
 * @param client 	Client index
 * @param target    Enitity ID of object to control.
 * @param builder   Set to builder of object controlled.
 * @param type      Set to type of object controlled.
 * @return			returns true for sucess.
 */
native bool:ControlObject(client, target, &builder=0, &TFObjectType:type=TFObject_Unknown);

/**
 * Reset Mind Controlled Objects.
 *
 * @param client 	Client
 * @param remove    Set to true to remove all object controlled by client.
 * @return			none
 */
native ResetMindControlledObjects(client, bool:remove);

/* DO NOT EDIT BELOW THIS LINE */

public SharedPlugin:__pl_MindControl = 
{
	name = "MindControl",
	file = "MindControl.smx",
#if defined REQUIRE_PLUGIN
	required = 1,
#else
	required = 0,
#endif
};

#if !defined REQUIRE_PLUGIN
public __pl_MindControl_SetNTVOptional()
{
	MarkNativeAsOptional("MindControl");
	MarkNativeAsOptional("ControlObject");
	MarkNativeAsOptional("ResetMindControlledObjects");
}
#endif
