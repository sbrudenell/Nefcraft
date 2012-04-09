/**
 * vim: set ai et ts=4 sw=4 syntax=sourcepawn :
 * File: hgrsource.inc
 * Description: Native Interface to HookGrabRope for Source
 * Author(s): -=|JFH|=-Naris (Murray Wilson)
 */

#if defined _hgrsource_included
 #endinput
#endif
#define _hgrsource_included

#define ACTION_HOOK 0
#define ACTION_GRAB 1
#define ACTION_ROPE 2

/**
 * Assume control of the HookGrabRope plugin
 *
 * @param plugin_only 	Set to 1 to assume complete control of the HGR plugin.
 * @return		    	none
 */
native ControlHookGrabRope(bool:plugin_only=true);

/**
 * Give a hook to the player
 *
 * @param client 	Client index
 * @param duration  How long (in seconds) player can hook, <=0 is unlimited (default 0)
 * @param range     Range of the hook, <=0.0 is unlimited (default 0.0)
 * @param cooldown  How long (in seconds) before player can hook again, <=0 is unlimited (default 0)
 * @param flags     Not used yet.
 * @return			none
 */
native GiveHook(client,duration=0,Float:range=0.0,Float:cooldown=0.0,flags=0);

/**
 * Take the hook from the player
 *
 * @param client 	Client index
 * @return			none
 */
native TakeHook(client);

/**
 * Give a grabber to the player
 *
 * @param client 	Client index
 * @param duration  How long (in seconds) player can grab, <=0 is unlimited
 * @param range     Range of the grab, <=0.0 is unlimited (default 0.0)
 * @param cooldown  How long (in seconds) before player can grab again, <=0 is unlimited (default 0)
 * @param flags     1==Puller (drag victim closer), 0==Grabber (keep distance constant {more or less}) (default 0)
 * @return			none
 */
native GiveGrab(client,duration=0,Float:range=0.0,Float:cooldown=0.0,flags=0);

/**
 * Take the grabber from the player
 *
 * @param client 	Client index
 * @return			none
 */
native TakeGrab(client);

/**
 * Give a rope to the player
 *
 * @param client 	Client index
 * @param duration  How long (in seconds) player can rope, <=0 is unlimited
 * @param range     Range of the rope, <=0.0 is unlimited (default 0.0)
 * @param cooldown  How long (in seconds) before player can rope again, <=0 is unlimited (default 0)
 * @param flags     Not used yet.
 * @return			none
 */
native GiveRope(client,duration=0,Float:range=0.0,Float:cooldown=0.0,flags=0);

/**
 * Take the rope from the player
 *
 * @param client 	Client index
 * @return			none
 */
native TakeRope(client);

/**
 * Use the hook (bind to a keydown)
 *
 * @param client 	Client index
 * @return			none
 */
native Hook(client);

/**
 * Stop using the hook (bind to a keyup)
 *
 * @param client 	Client index
 * @return			none
 */
native UnHook(client);

/**
 * Toggle the state of the hook
 *
 * @param client 	Client index
 * @return			none
 */
native ToggleHook(client);

/**
 * Use the grabber (bind to a keydown)
 *
 * @param client 	Client index
 * @return			none
 */
native Grab(client);

/**
 * Stop using the grabber (bind to a keyup)
 *
 * @param client 	Client index
 * @return			none
 */
native Drop(client);

/**
 * Toggle the state of the grabber
 *
 * @param client 	Client index
 * @return			none
 */
native ToggleGrab(client);

/**
 * Use the rope (bind to a keydown)
 *
 * @param client 	Client index
 * @return			none
 */
native Rope(client);

/**
 * Stop using the rope (bind to a keyup)
 *
 * @param client 	Client index
 * @return			none
 */
native Detach(client);

/**
 * Toggle the state of the rope
 *
 * @param client 	Client index
 * @return			none
 */
native ToggleRope(client);

/**
 * Check if a player is grabbing, hoocking or roping.
 *
 * @param client 	Client index
 * @param action	action to check (ACTION_HOOK,ACTION_GRAB or ACTION_ROPE)
 * @return			returns true is client is performing the given action.
 */
native bool:HGRState(client,action);

/**
 * Check if a player has been grabbed.
 *
 * @param client 	Client index
 * @return			returns true is client is grabbed.
 */
native bool:IsGrabbed(client);

/**
 * Check who a player is grabbing.
 *
 * @param client 	Client index
 * @return			returns the target that client is grabbing (if any).
 */
native GrabTarget(client);

/**
 * Gets called when the grabber grabs someone
 *
 * @param client: The index of the client that used the grabber.
 * @param target: The index of the target player that was grabbed
 * @return		  Plugin_Stop will prevent the target from being grabbed.
 */
forward Action:OnGrabPlayer(client,target);

/**
 * Gets called when the grabber drags someone
 *
 * @param client: The index of the client that used the grabber.
 * @param target: The index of the target player that was grabbed
 * @return		  Plugin_Stop will cause the target player to be dropped.
 */
forward Action:OnDragPlayer(client,target);

/**
 * Gets called when the grabber drops someone
 *
 * @param client: The index of the client that used the grabber.
 * @param target: The index of the target player that was grabbed
 * @return		  none
 */
forward Action:OnDropPlayer(client,target);

/**
 * Gets called when hooking
 *
 * @param client: The index of the client that used the hook.
 */
forward Action:OnHook(client);

/**
 * Gets called when hooking
 *
 * @param client: The index of the client that used the hook.
 */
forward Action:OnRope(client);

/* DO NOT EDIT BELOW THIS LINE */

public SharedPlugin:__pl_hgrsource = 
{
	name = "hgrsource",
	file = "hgrsource.smx",
#if defined REQUIRE_PLUGIN
	required = 1,
#else
	required = 0,
#endif
};

#if !defined REQUIRE_PLUGIN
public __pl_hgrsource_SetNTVOptional()
{
	MarkNativeAsOptional("ControlHookGrabRope");
	MarkNativeAsOptional("GiveHook");
	MarkNativeAsOptional("TakeHook");
	MarkNativeAsOptional("GiveGrab");
	MarkNativeAsOptional("TakeGrab");
	MarkNativeAsOptional("GiveRope");
	MarkNativeAsOptional("TakeRope");
	MarkNativeAsOptional("Hook");
	MarkNativeAsOptional("UnHook");
	MarkNativeAsOptional("ToggleHook");
	MarkNativeAsOptional("Grab");
	MarkNativeAsOptional("Drop");
	MarkNativeAsOptional("ToggleGrab");
	MarkNativeAsOptional("Rope");
	MarkNativeAsOptional("Detach");
	MarkNativeAsOptional("ToggleRope");
	MarkNativeAsOptional("HGRState");
	MarkNativeAsOptional("IsGrabbed");
	MarkNativeAsOptional("GrabTarget");
}
#endif
