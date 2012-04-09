/**
 * vim: set ai et ts=4 sw=4 syntax=sourcepawn :
 * File: ShopItems.inc
 * Description: The shop items that come with SourceCraft.
 * Author(s): -=|JFH|=-Naris
 */
 
#if defined _ShopItems_included
 #endinput
#endif
#define _ShopItems_included

/**
 * Returns if the client is a mole.
 *
 * @param client 	Client to query
 * @return			true if the player is a mole.
 */
native bool:IsMole(client);

/**
 * Sets the client's "is a mole" flag.
 *
 * @param client 	Client to query
 * @param value 	Value to set the IsMole flag.
 * @noreturn
 */
native SetMole(client, bool:value=true);

/**
 * Transform a player into a mole by teleporting him to the enemies spawn.
 *
 * @param client 	Client to query
 * @return			true if the player is a mole.
 */
native InvokeMole(client);

/* DO NOT EDIT BELOW THIS LINE */

public SharedPlugin:__pl_ShopItems = 
{
	name = "ShopItems",
	file = "ShopItems.smx",
#if defined REQUIRE_PLUGIN
	required = 1,
#else
	required = 0,
#endif
};

#if !defined REQUIRE_PLUGIN
public __pl_ShopItems_SetNTVOptional()
{
	MarkNativeAsOptional("IsMole");
	MarkNativeAsOptional("SetMole");
	MarkNativeAsOptional("InvokeMole");
}
#endif
