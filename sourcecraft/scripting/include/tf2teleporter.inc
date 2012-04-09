/**
 * vim: set ai et ts=4 sw=4 syntax=sourcepawn :
 * File: tf2teleporter.inc
 * Description: Native Interface to the TF2 Teleporter Tools
 * Author(s): -=|JFH|=-Naris (Murray Wilson)
 */

#if defined _tf2teleporter_included
 #endinput
#endif
#define _tf2teleporter_included

/**
 * Assume control of the TF2 Teleporter plugin
 *
 * @param plugin_only 	Set to 1 to assume complete control of the Teleporter plugin.
 * @return		    	none
 */
native ControlTeleporter(bool:plugin_only=true);

/**
 * Enable Teleporter Recharger for a player.
 *
 * @param index 	Client index
 * @param recharge  Recharge time for teleporters built by client.
 * @return			none
 */
native SetTeleporter(client,Float:recharge);

/* DO NOT EDIT BELOW THIS LINE */

public SharedPlugin:__pl_tf2tele = 
{
	name = "tf2tele",
	file = "tf2teleporter.smx",
#if defined REQUIRE_PLUGIN
	required = 1,
#else
	required = 0,
#endif
};

#if !defined REQUIRE_PLUGIN
public __pl_tf2tele_SetNTVOptional()
{
	MarkNativeAsOptional("ControlTeleporter");
	MarkNativeAsOptional("SetTeleporter");
}
#endif
