/**
 * vim: set ai et ts=4 sw=4 syntax=sourcepawn :
 * File: sm_flamethrower.inc
 * Description: Native Interface to Flamethrower
 * Author(s): -=|JFH|=-Naris (Murray Wilson)
 */

#if defined _sm_flamethrower_included
 #endinput
#endif
#define _sm_flamethrower_included

/**
 * Assume control of the Flamethrower plugin
 *
 * @param plugin_only 	Set to 1 to assume complete control of the Flamethrower plugin.
 * @return		    	none
 */
native ControlFlamethrower(bool:plugin_only=true);

/**
 * Give a Flamethrower to a player.
 *
 * @param client 	Client index
 * @param amount 	Number of flamethrower cells (fuel) per player at spawn. (-1 = use convar)
 * @param range  	The range of the flamethrower (-1.0 = use convar)
 * @return			none
 */
native GiveFlamethrower(client,amount=-1,Float:range=-1.0);

/**
 * Take the Flamethrower from a player.
 *
 * @param client 	Client index
 * @return			none
 */
native TakeFlamethrower(client);

/**
 * Give a Flamethrower cells (fuel) to a player.
 *
 * @param client 	Client index
 * @param amount 	Number of flamethrower cells (fuel) to give.
 * @return			none
 */
native RefuelFlamethrower(client,amount=1);

/**
 * Returns the amount of remaining fuel in the player's Flamethrower (if any).
 *
 * @param client 	Client index
 * @return			none
 */
native GetFlamethrowerFuel(client);

/**
 * Sets the sound of the flamethrower
 *
 * @param sound 	Sound to use for the flamethrower.
 * @return			none
 */
native SetFlamethrowerSound(const String:sound[]);

/**
 * Use the Flamethrower
 *
 * @param index 	Client index
 * @return			none
 */
native UseFlamethrower(client);

/**
 * Gets called when a player has been flamed with the Flamethrower
 *
 * @param attacker: The index of the attacker.
 * @param victim:   The victim's index.
 * @return		    Plugin_Stop will prevent the target from being bombed.
 */
forward Action:OnPlayerFlamed(attacker,victim);

/* DO NOT EDIT BELOW THIS LINE */

public SharedPlugin:__pl_sm_flamethrower = 
{
	name = "sm_flame",
	file = "sm_flamethrower.smx",
#if defined REQUIRE_PLUGIN
	required = 1,
#else
	required = 0,
#endif
};

#if !defined REQUIRE_PLUGIN
public __pl_sm_flame_SetNTVOptional()
{
	MarkNativeAsOptional("ControlFlamethrower");
	MarkNativeAsOptional("GiveFlamethrower");
	MarkNativeAsOptional("TakeFlamethrower");
	MarkNativeAsOptional("GetFlamethrowerFuel");
	MarkNativeAsOptional("RefuelFlamethrower");
	MarkNativeAsOptional("UseFlamethrower");
}
#endif
