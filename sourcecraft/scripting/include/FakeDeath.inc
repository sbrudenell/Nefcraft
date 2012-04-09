/**
 * vim: set ai et ts=4 sw=4 syntax=sourcepawn :
 * File: FakeDeath.inc
 * Description: Native Interface to FakeDeath
 * Author(s): -=|JFH|=-Naris (Murray Wilson)
 */

#if defined _FakeDeath_included
 #endinput
#endif
#define _FakeDeath_included

/**
 * Assume control of the FakeDeath plugin
 *
 * @param plugin_only 	Set to 1 to assume complete control of the Firemines plugin.
 * @return		    	none
 */
native ControlDeath(bool:plugin_only=true);

/**
 * Give a player one or more Fake Deaths to perpetrate
 *
 * @param index 	Client index
 * @param number    Set to number of Fake Deaths to allow the player.
 * @param dissolve 	Dissolve ragdolls (0 = disabled | 1 = enabled)
 * @return			none
 */
native GiveDeath(client,number=1, bool:dissolve=false);

/**
 * Removes a player's ability to Fake Deaths.
 *
 * @param index 	Client index
 * @return			none
 */
native TakeDeath(client);

/**
 * Fakes a player's Death (spawns a ragdoll)
 *
 * @param index 	Client index
 * @return			none
 */
native FakeDeath(client);

/* DO NOT EDIT BELOW THIS LINE */

public SharedPlugin:__pl_FakeDeath = 
{
	name = "FakeDeath",
	file = "FakeDeath.smx",
#if defined REQUIRE_PLUGIN
	required = 1,
#else
	required = 0,
#endif
};

#if !defined REQUIRE_PLUGIN
public __pl_FakeDeath_SetNTVOptional()
{
	MarkNativeAsOptional("ControlDeath");
	MarkNativeAsOptional("GiveDeath");
	MarkNativeAsOptional("TakeDeath");
	MarkNativeAsOptional("FakeDeath");
}
#endif
