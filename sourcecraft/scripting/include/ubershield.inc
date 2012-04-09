/**
 * vim: set ai et ts=4 sw=4 syntax=sourcepawn :
 * File: ubershield.inc
 * Description: Native Interface to the Medic Uber Shield for TF2
 * Author(s): -=|JFH|=-Naris (Murray Wilson)
 */

#if defined _ubershield_included
 #endinput
#endif
#define _ubershield_included

enum ShieldFlags (<<= 1)
{
    Shield_None = 0,            // No shield allowed
    Shield_Normal = 1,          // Shield stops bullets but not players
    Shield_Immobilize,          // Shield immobilizes everything
	Shield_Team_Specific,	    // Shield is team specific
    Shield_Mobile,              // Shield is mobile (parented to target)
    Shield_Target_Self,         // Sheild can target self
	Shield_Target_Team,	        // Shield can target teammates
    Shield_Target_Enemy,        // Shield can target enemies
    Shield_Target_Location,     // Shield can target locations (position)
    Shield_With_Medigun,        // Shield can be invoked with Medigun
    Shield_With_Kritzkrieg,     // Shield can be invoked with Kritzkrieg
    Shield_Reload_Normal,       // Reload Shield stops bullets but not players
    Shield_Reload_Immobilize,   // Reload Shield immobilizes everything
	Shield_Reload_Team_Specific,// Reload Shield is team specific
    Shield_Reload_Mobile,       // Reload Shield is mobile (parented to target)
    Shield_Reload_Self,         // Reload Sheild can target self
	Shield_Reload_Team,	        // Reload Shield can target teammates
    Shield_Reload_Enemy,        // Reload Shield can target enemies
    Shield_Reload_Location,     // Reload Shield can target locations (position)
    Shield_UseAlternateSounds,  // Use the Alternate Shield Sounds
    Shield_DisableStartSound,   // Disable the Start Shield Sound
    Shield_DisableActiveSound,  // Disable the Active Shield Sound
    Shield_DisableStopSound     // Disable the Stop Shield Sound
};

#define ShieldNormalMask    (Shield_Normal|Shield_Immobilize|Shield_Team_Specific|Shield_Mobile|Shield_Target_Self|Shield_Target_Team|Shield_Target_Enemy|Shield_Target_Location)
#define ShieldReloadMask    (Shield_Reload_Normal|Shield_Reload_Immobilize|Shield_Reload_Team_Specific|Shield_Reload_Mobile|Shield_Reload_Self|Shield_Reload_Team|Shield_Reload_Location)
#define ShieldDefault       (ShieldNormalMask|ShieldReloadMask)

/**
 * Assume control of the Uber Shield plugin
 *
 * @param plugin_only 	Set to 1 to assume complete control of the Uber Shield plugin.
 * @return		    	none
 */
native ControlUberShield(bool:plugin_only=true);

/**
 * Give a player one or more uber shields to invoke
 *
 * @param index 	Client index
 * @param number    Set to number of uber shields to give the player. (-1 is unlimited)
 * @param per_spawn Set to number of uber shields to give the player on each spawn. (-1 is unlimited)
 * @param flags     The default ShieldFlags to use for the shield for this player.
 * @return			none
 */
native GiveUberShield(client,number=-1,per_spawn=-1,ShieldFlags:flags=ShieldDefault);

/**
 * Takes the player's uber shields away
 *
 * @param index 	Client index
 * @return			none
 */
native TakeUberShield(client);

/**
 * Invoke the Uber Shield on the Target entity the client is aiming at
 * (or the client if there is no aim target)
 *
 * @param client 	Client index
 * @param flags     The ShieldFlags to use for the shield.
 * @param duration  The duration of the shield (in seconds, 0 = infinite, < 0 to use convar).
 * @return			none
 */
native UberShieldTarget(client,Float:duration=-1.0,
                        ShieldFlags:flags=ShieldDefault);

/**
 * Invoke the Uber Shield on the location the client is looking at
 *
 * @param client 	Client index
 * @param flags     The ShieldFlags to use for the shield.
 * @param duration  The duration of the shield (in seconds, 0 = infinite, < 0 to use convar).
 * @return			none
 */
native UberShieldLocation(client,Float:duration=-1.0,
                          ShieldFlags:flags=ShieldDefault);

/**
 * Invoke the Uber Shield on either the target, position or where the client is looking,
 * depending on what is passed in.
 *
 * @param client 	Client index
 * @param target 	Target index (set to client to shield self)
 * @param pos       The position to shield (if any, NULL_VECTOR for none).
 * @param flags     The ShieldFlags to use for the shield.
 * @param duration  The duration of the shield (in seconds, 0 = infinite, < 0 to use convar).
 * @return			none
 */
native UberShield(client,target=0,const Float:pos[3]=NULL_VECTOR,
                  Float:duration=-1.0,
                  ShieldFlags:flags=ShieldDefault);

/**
 * Set Alternate Sound for the Shield
 *
 * @param startSound    The Alternate Shield Start Sound
 * @param activeSound   The Alternate Shield Active (Loop) Sound
 * @param stopSound     The Alternate Shield End Sound
 * @param startTime     The duration of the Alternate Shield Start Sound
 * @param stopTime      The duration of the Alternate Shield Stop Sound
 * @return		        none
 */
native SetAlternateShieldSound(const String:startSound[] = "",
                               const String:activeSound[] = "",
                               const String:stopSound[] = "",
                               Float:startTime=2.0,
                               Float:stopTime=2.0);

/**
 * Gets called when an ubershield is deployed
 * @param client:   The index of the client that deployed the ubershield.
 * @param target 	Target index (set to client to shield self, set to 0 for a location)
 */
forward Action:OnDeployUberShield(client, target);

/* DO NOT EDIT BELOW THIS LINE */

public SharedPlugin:__pl_ubershield = 
{
	name = "ubershield",
	file = "ubershield.smx",
#if defined REQUIRE_PLUGIN
	required = 1,
#else
	required = 0,
#endif
};

#if !defined REQUIRE_PLUGIN
public __pl_ubershield_SetNTVOptional()
{
	MarkNativeAsOptional("SetAlternateShieldSound");
	MarkNativeAsOptional("UberShieldLocation");
	MarkNativeAsOptional("ControlUberShield");
	MarkNativeAsOptional("UberShieldTarget");
	MarkNativeAsOptional("GiveUberShield");
	MarkNativeAsOptional("TakeUberShield");
	MarkNativeAsOptional("UberShield");
}
#endif
