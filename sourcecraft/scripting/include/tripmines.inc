/**
 * vim: set ai et ts=4 sw=4 syntax=sourcepawn :
 * File: tripmines.inc
 * Description: Native Interface to tripmines
 * Author(s): -=|JFH|=-Naris (Murray Wilson)
 */

#if defined _tripmines_included
 #endinput
#endif
#define _tripmines_included

/**
 * Assume control of the tripmines plugin
 *
 * @param plugin_only 	Set to 1 to assume complete control of the Tripmines plugin.
 * @return		    	none
 */
native ControlTripmines(bool:plugin_only=true);

/**
 * Give a player one or more tripmines to plant
 *
 * @param index 	Client index
 * @param number    Set to number of tripmines to give the player. (-1 is plugin default)
 * @param per_spawn Set to number of tripmines to give the player on each spawn. (-1 is plugin default)
 * @param maximum   Set to number of tripmines allowed to be active. (-1 is plugin default)
 * @return			none
 */
native GiveTripmines(client,number=-1,per_spawn=-1,maximum=-1);

/**
 * Takes the player's tripmines away
 *
 * @param index 	Client index
 * @return			none
 */
native TakeTripmines(client);

/**
 * Adds one or more tripmines for the player to plant
 *
 * @param index 	Client index
 * @param number    Number of tripmines to add.
 * @return			none
 */
native AddTripmines(client,number=1);

/**
 * Subtracts one or more tripmines for the player to plant
 *
 * @param index 	Client index
 * @param number    Number of tripmines to subtract.
 * @return			none
 */
native SubTripmines(client,number=1);

/**
 * Returns how many tripmines a player has (or is allowed).
 *
 * @param index 	Client index
 * @param allowed   Set to  true to return number allowed, otherwise
 * @return			returns number of remaining tripmines.
 */
native HasTripmines(client,bool:allowed=false);

/**
 * Sets (Plants) a tripmine.
 *
 * @param index 	Client index
 * @return			none
 */
native SetTripmine(client);

/**
 * Counts how many tripmines the player has active.
 *
 * @param index 	Client index
 * @return			none
 */
native CountTripmines(client);

/**
 * Gets called when when a tripmine is set
 * @param client 	Client index of the player setting the tripmine
 */
forward Action:OnSetTripmine(client);

/* DO NOT EDIT BELOW THIS LINE */

public SharedPlugin:__pl_tripmines = 
{
	name = "tripmines",
	file = "tripmines.smx",
#if defined REQUIRE_PLUGIN
	required = 1,
#else
	required = 0,
#endif
};

#if !defined REQUIRE_PLUGIN
public __pl_tripmines_SetNTVOptional()
{
	MarkNativeAsOptional("ControlTripmines");
	MarkNativeAsOptional("GiveTripmines");
	MarkNativeAsOptional("TakeTripmines");
	MarkNativeAsOptional("AddTripmines");
	MarkNativeAsOptional("SubTripmines");
	MarkNativeAsOptional("HasTripmines");
	MarkNativeAsOptional("SetTripmine");
	MarkNativeAsOptional("CountTripmines");
}
#endif
