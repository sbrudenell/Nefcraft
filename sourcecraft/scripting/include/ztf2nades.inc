/**
 * vim: set ai et ts=4 sw=4 syntax=sourcepawn :
 * File: ztf2nades.inc
 * Description: Native Interface to ztf2nades
 * Author(s): -=|JFH|=-Naris (Murray Wilson)
 */

#if defined _ztf2nades_included
 #endinput
#endif
#define _ztf2nades_included


// *************************************************
// defines
// *************************************************

// Nade Types
enum NadeType
{
	DefaultNade = 0, // use class for nade type
	ConcNade,
	BearTrap,
	NailNade,
	MirvNade,
	HealthNade,
	HeavyNade,
	NapalmNade,
	HallucNade,
	EmpNade,
	Bomblet,
	SmokeNade,
	GasNade,
	TargetingDrone,
	FragNade
};

#if defined _ztf2nades_plugin
 #endinput
#endif

/**
 * Assume control of the ztf2nades plugin
 *
 * @param plugin_only 	Set to 1 to assume complete control of ztf2nades.
 * @param targetting 	Set to 1 to assume control of targetting.
 * @return		    	none
 */
native ControlNades(bool:plugin_only=true, bool:targetting=false);

/**
 * Give nades to the player
 *
 * @param client 	        Client index
 * @param frag              How many frag nades to give to the player
 * @param frag_per_spawn    How many frag nades to give to the player on each spawn
 * @param special           How many special nades to give to the player
 * @param special_per_spawn How many special nades to give to the player on each spawn
 * @param can_restock       Can the player restock at cabinets?
 * @param special_type      What type of nade is the special (default is use class to determine)
 * @return		    	    none
 */
native GiveNades(client, frag=-1, frag_per_spawn=-1, special=-1, special_per_spawn=-1,
                 can_restock=false, NadeType:special_type=DefaultNade);

/**
 * Take the nades from the player
 *
 * @param client 	Client index
 * @return			none
 */
native TakeNades(client);

/**
 * Adds one or more frag nades for the player to use
 *
 * @param client 	Client index
 * @param number    Number of frag nades to add.
 * @return			none
 */
native AddFragNades(client,number=1);

/**
 * Subtracts one or more frag nades for the player to use
 *
 * @param client 	Client index
 * @param number    Number of frag nades to add.
 * @return			none
 */
native SubFragNades(client,number=1);

/**
 * Returns how many fraqg nadea a player has (or is allowed).
 *
 * @param index 	Client index
 * @param allowed   Set to  true to return number allowed, otherwise
 * @return			returns number of remaining frag nades.
 */
native HasFragNades(client,bool:allowed=false);

/**
 * Adds one or more special nades for the player to use
 *
 * @param client 	Client index
 * @param number    Number of special nades to add.
 * @return			none
 */
native AddSpecialNades(client,number=1);

/**
 * Subtracts one or more special nades for the player to use
 *
 * @param client 	Client index
 * @param number    Number of special nades to add.
 * @return			none
 */
native SubSpecialNades(client,number=1);

/**
 * Returns how many special nades a player has (or is allowed).
 *
 * @param index 	Client index
 * @param allowed   Set to  true to return number allowed, otherwise
 * @return			returns number of remaining special nades.
 */
native HasSpecialNades(client,bool:allowed=false);

/**
 * Throw a Frag nade
 *
 * @param client 	Client index
 * @param pressed   Set true when pressed, false when released
 * @return			none
 */
native ThrowFragNade(client, bool:pressed);

/**
 * Throw a Special nade
 *
 * @param client 	Client index
 * @param pressed   Set true when pressed, false when released
 * @return			none
 */
native ThrowSpecialNade(client, bool:pressed);

/**
 * Damage Buildings in range of an attack
 *
 * @param attacker 	Client index of attacker
 * @param start     Starting Location
 * @param start     radius of damage
 * @param damage    amount of damage
 * @param ent       Entity doing damage
 * @param trace     Set true to check using trace
 * @return			none
 */
native DamageBuildings(attacker, Float:start[3], Float:radius, damage, ent, bool:trace);

/**
 * Always throw specified type of nade, does NOT alter remaining count.
 *
 * @param client 	Client index
 * @param pressed   Set true when pressed, false when released
 * @param type      Type of nade to throw
 * @return			none
 */
native ThrowNade(client, bool:pressed, NadeType:type=DefaultNade);

/**
 * Returns if a player has been targetted by the targetting drone
 *
 * @param index 	Client index
 * @return			returns true if the client has been targetted
 */
native IsTargeted(client);

/**
 * Gets called when when a grenade explodes
 * @param client 	Client index of the player that threw the nade
 * @param team 	    Team of the player that threw the nade
 * @param type      The type of nade
 * @param nade      The entity index of the nade exploding
 * @param pos 	    The location of the nade
 * @param inRange 	Array used determine which players are in range, a value of > 0.0 means that client is in range.
 * @param count     Count of players in the inRange array.
 */
forward Action:OnNadeExplode(client, team, NadeType:type, nade,
                             const Float:pos[3], const Float:inRange[], count);

/* DO NOT EDIT BELOW THIS LINE */

public SharedPlugin:__pl_ztf2nades = 
{
	name = "ztf2nades",
	file = "ztf2nades.smx",
#if defined REQUIRE_PLUGIN
	required = 1,
#else
	required = 0,
#endif
};

#if !defined REQUIRE_PLUGIN
public __pl_ztf2nades_SetNTVOptional()
{
	MarkNativeAsOptional("ControlNades");
	MarkNativeAsOptional("AddFragNades");
	MarkNativeAsOptional("SubFragNades");
	MarkNativeAsOptional("HasFragNades");
	MarkNativeAsOptional("ThrowFragNade");
	MarkNativeAsOptional("AddSpecialNades");
	MarkNativeAsOptional("SubSpecialNades");
	MarkNativeAsOptional("HasSpecialNades");
	MarkNativeAsOptional("ThrowSpecialNade");
	MarkNativeAsOptional("DamageBuildings");
	MarkNativeAsOptional("IsTargeted");
	MarkNativeAsOptional("GiveNades");
	MarkNativeAsOptional("TakeNades");
	MarkNativeAsOptional("ThrowNade");
}
#endif
