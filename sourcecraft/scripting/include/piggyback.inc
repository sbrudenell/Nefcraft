/**
 * vim: set ai et ts=4 sw=4 syntax=sourcepawn :
 * File: piggyback.inc
 * Description: Native Interface to Piggyback
 * Author(s): -=|JFH|=-Naris (Murray Wilson)
 */

#if defined _piggyback_included
 #endinput
#endif
#define _piggyback_included

// Define the PiggyMethod bits
enum PiggyMethod (<<= 1)
{
    PiggyMethod_Default = -1,
    PiggyMethod_None = 0,
    PiggyMethod_ForceView = 1,
    PiggyMethod_DisableAttack,
    PiggyMethod_AllowSpys,
    PiggyMethod_SharedFate,
    PiggyMethod_Pickup,
    PiggyMethod_Enable
}

/**
 * Assume control of the Piggyback plugin
 *
 * @param plugin_only 	Set to 1 to assume complete control of the Piggyback plugin.
 * @return		    	none
 */
native ControlPiggyback(bool:plugin_only=true);

/**
 * Allows a player to "piggyback" other players
 *
 * @param index 	Client index
 * @param method    The PiggyMethod to use for this player.
 * @param distance  The distance from which someone can be picked up. (-1 to use convar)
 * @return			none
 */
native GivePiggyback(client,PiggyMethod:method=PiggyMethod_Default,Float:distance=-1.0);

/**
 * Takes the player's "piggyback" away
 *
 * @param index 	Client index
 * @return			none
 */
native TakePiggyback(client);

/**
 * Invoke the Piggybak to pick up either the target or the player the client is looking at
 * depending on what is passed in.
 *
 * @param client 	Client index
 * @param target 	Target index
 * @return			none
 */
native Piggyback(client,target=0);

/**
 * Gets called when an piggyback is deployed
 * @param rider:    The index of the client that  is the rider
 * @param carrier 	the index of the client that is the carrier
 * @param distance 	The distance between client and target
 * @param pickedup 	Tre if the target was picked up or false if he jumped off
 */
forward Action:OnPlayerPiggyback(rider, carrier, bool:pickedup, Float:distance);

/* DO NOT EDIT BELOW THIS LINE */

public SharedPlugin:__pl_piggyback = 
{
	name = "piggyback",
	file = "piggyback.smx",
#if defined REQUIRE_PLUGIN
	required = 1,
#else
	required = 0,
#endif
};

#if !defined REQUIRE_PLUGIN
public __pl_piggyback_SetNTVOptional()
{
	MarkNativeAsOptional("ControlPiggyback");
	MarkNativeAsOptional("GivePiggyback");
	MarkNativeAsOptional("TakePiggyback");
	MarkNativeAsOptional("Piggyback");
}
#endif
