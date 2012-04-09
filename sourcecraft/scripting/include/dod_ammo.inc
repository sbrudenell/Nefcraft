/**
 * vim: set ai et ts=4 sw=4 syntax=sourcepawn :
 * File: dod_ammo.inc
 * Description: Native Interface to DoD Ammo
 * Author(s): -=|JFH|=-Naris (Murray Wilson)
 */

#if defined _dod_ammo_included
 #endinput
#endif
#define _dod_ammo_included

/**
 * Assume control of the DOD Ammo plugin
 *
 * @param plugin_only 	Set to 1 to assume complete control of the DOD Ammo plugin.
 * @return		    	none
 */
native ControlDodAmmo(bool:plugin_only=true);

/**
 * Enable DOD Ammo Packs for a player.
 *
 * @param index 	Client index
 * @param enable    Enable/Disable ammopacks for client (0 = disabled   | 1 = on death |
 *                                                       2 = on command | 3 = on death and command)
 * @param rule      Who can pickup the ammopacks (0 = everyone | 1 = only teammates | 2 = only enemies)
 * @param count 	Number of ammopacks client given per spawn.
 * @return			none
 */
native SetDodAmmo(client,enable=3,rule=0,count=1);

/**
 * Restock a player's ammo
 *
 * @param index 	Client index
 * @return			none
 */
native Restock(client);

/* DO NOT EDIT BELOW THIS LINE */

public SharedPlugin:__pl_dod_ammo = 
{
	name = "dod_ammo",
	file = "dod_ammo.smx",
#if defined REQUIRE_PLUGIN
	required = 1,
#else
	required = 0,
#endif
};

#if !defined REQUIRE_PLUGIN
public __pl_dod_ammo_SetNTVOptional()
{
	MarkNativeAsOptional("ControlDodAmmo");
	MarkNativeAsOptional("SetDodAmmo");
	MarkNativeAsOptional("Restock");
}
#endif
