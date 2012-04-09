/**
 * vim: set ai et ts=4 sw=4 syntax=sourcepawn :
 * File: ammopacks.inc
 * Description: Native Interface to the TF2 Ammopacks
 * Author(s): -=|JFH|=-Naris (Murray Wilson)
 */

#if defined _ammopacks_included
 #endinput
#endif
#define _ammopacks_included

/**
 * Assume control of the TF2 Ammopacks plugin
 *
 * @param plugin_only 	Set to 1 to assume complete control of the Medipacks plugin.
 * @return		    	none
 */
native ControlAmmopacks(bool:plugin_only=true);

/**
 * Enable Ammopacks for the player
 *
 * @param index 	Client index
 * @param enable    Enable/Disable ammopacks for client (0 = disabled   | 1 = on death |
 *                                                       2 = on command | 3 = on death and command)
 * @return			none
 */
native SetAmmopack(client,enable=3);

/**
 * Drop a Ammopack.
 *
 * @param index 	Client index
 * @param charge    Set to amount of metal to use for ammopack (-1=use actual metal level).
 * @return			true if a ammopack was successfully dropped
 */
native bool:DropAmmopack(client,charge=-1);

/* DO NOT EDIT BELOW THIS LINE */

public SharedPlugin:__pl_ammopacks = 
{
	name = "ammopacks",
	file = "ammopacks.smx",
#if defined REQUIRE_PLUGIN
	required = 1,
#else
	required = 0,
#endif
};

#if !defined REQUIRE_PLUGIN
public __pl_ammopacks_SetNTVOptional()
{
	MarkNativeAsOptional("ControlAmmopacks");
	MarkNativeAsOptional("SetAmmopack");
	MarkNativeAsOptional("DropAmmopack");
}
#endif
