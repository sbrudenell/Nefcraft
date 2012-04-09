/**
 * vim: set ai et ts=4 sw=4 syntax=sourcepawn :
 * File: ztf2grab.inc
 * Description: Native Interface to ztf2grab (gravgun)
 * Author(s): -=|JFH|=-Naris (Murray Wilson)
 */

#if defined _ztf2grab_included
 #endinput
#endif
#define _ztf2grab_included

// These define the permissions
#define HAS_GRABBER		            (1 << 0)
#define CAN_STEAL		            (1 << 1)
#define CAN_GRAB_PROPS		        (1 << 2)
#define CAN_GRAB_BUILDINGS		    (1 << 3)
#define CAN_GRAB_OTHER_BUILDINGS    (1 << 4)
#define CAN_THROW_BUILDINGS         (1 << 5)
#define CAN_HOLD_ENABLED_BUILDINGS  (1 << 6)
#define CAN_THROW_ENABLED_BUILDINGS (1 << 7)
#define CAN_JUMP_WHILE_HOLDING      (1 << 8)
#define DISABLE_OPPONENT_BUILDINGS  (1 << 9)
#define CAN_REPAIR_WHILE_HOLDING    (1 << 10)
#define CAN_HOLD_WHILE_SAPPED       (1 << 11)

#if defined _ztf2grab_plugin
 #endinput
#endif

/**
 * Assume control of the ztf2grab plugin
 *
 * @param plugin_only 	Set to 1 to assume complete control of ztf2grab.
 * @return		    	none
 */
native ControlZtf2grab(bool:plugin_only=true);

/**
 * Give a gravgun to the player
 *
 * @param index 	    Client index
 * @param duration      How long (in seconds) player can hold objects, 0.0 is unlimited (default=-1 to use ConVars)
 * @param throw_speed   Speed to throw objects at. (default=-1 to use ConVars)
 * @param throw_gravity Gravity to use for thrown objects (default=-1 to use ConVars)
 * @param permissions   Permissions to use (default=-1 to use ConVars)
 * @return		    	none
 */
native GiveGravgun(client, Float:duration=-1.0, Float:throw_speed=-1.0, Float:throw_gravity=-1.0, permissions=-1);

/**
 * Take the gravgun from the player
 *
 * @param index 	Client index
 * @return			none
 */
native TakeGravgun(client);

/**
 * Pickup an object using the gravgun
 *
 * @param index 	Client index
 * @return			none
 */
native PickupObject(client);

/**
 * Drop an object using the gravgun
 *
 * @param index 	Client index
 * @return			none
 */
native DropObject(client);

/**
 * Start to throw an object using the gravgun
 * (call when key pressed down)
 *
 * @param index 	Client index
 * @return			none
 */
native StartThrowObject(client);

/**
 * Throw an object using the gravgun
 * (call when key is released)
 *
 * @param index 	Client index
 * @return			none
 */
native ThrowObject(client);

/**
 * Rotate an object using the gravgun
 *
 * @param index 	Client index
 * @return			none
 */
native RotateObject(client);

/**
 * Drop an entity so it descends until it hit something.
 *
 * @param entity 	entity index to drop.
 * @param speed 	vertical speed to impart when dropping (should be negative).
 * @param gravity 	gravity to set object to (-1.0 to leave unchanged)
 * @return			none
 */
native DropEntity(entity, Float:speed=-1.0, Float:gravity=10.0);

/**
 * Gets called an object is picked up
 *
 * @param client:   The index of the client that used the gravgun.
 * @param builder:  The index of the builder of the object that was grabbed
 * @param ent:      The index of the object that was grabbed
 * @return		    Plugin_Stop will prevent the pickup.
 */
forward Action:OnPickupObject(client,builder,ent);

/**
 * Gets called when an object is thrown
 *
 * @param client:   The index of the client that used the gravgun.
 * @param ent:      The index of the object that was grabbed
 * @return		    Plugin_Stop will prevent the throw.
 */
forward Action:OnThrowObject(client,ent);

/**
 * Gets called while a grabbed object is carried
 *
 * @param client:   The index of the client that used the gravgun.
 * @param ent:      The index of the object that was grabbed.
 * @param time:     The EngineTime() when the object was picked up.
 * @return		    Plugin_Stop will cause the object to be dropped.
 */
forward Action:OnCarryObject(client,ent,Float:time);

/**
 * Gets called when an object is dropped
 *
 * @param client:   The index of the client that used the gravgun.
 * @param ent:      The index of the object that was grabbed
 * @return			none
 */
forward OnDropObject(client,ent);

/**
 * Gets called when when a thrown or dropped object stops
 *
 * @param ent:      The index of the object that was grabbed
 * @return			none
 */
forward OnObjectStop(ent);

/* DO NOT EDIT BELOW THIS LINE */

public SharedPlugin:__pl_ztf2grab = 
{
	name = "ztf2grab",
	file = "ztf2grab.smx",
#if defined REQUIRE_PLUGIN
	required = 1,
#else
	required = 0,
#endif
};

#if !defined REQUIRE_PLUGIN
public __pl_ztf2grab_SetNTVOptional()
{
	MarkNativeAsOptional("ControlZtf2grab");
	MarkNativeAsOptional("StartThrowObject");
	MarkNativeAsOptional("RotateObject");
	MarkNativeAsOptional("PickupObject");
	MarkNativeAsOptional("GiveGravgun");
	MarkNativeAsOptional("TakeGravgun");
	MarkNativeAsOptional("ThrowObject");
	MarkNativeAsOptional("DropEntity");
	MarkNativeAsOptional("DropObject");
}
#endif
