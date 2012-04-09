/**
 * vim: set ai et ts=4 sw=4 syntax=sourcepawn :
 * File: sidewinder.inc
 * Description: Native Interface to the sidewinder seeking rockets for TF2
 * Author(s): -=|JFH|=-Naris (Murray Wilson)
 */

#if defined _sidewinder_included
 #endinput
#endif
#define _sidewinder_included

enum SidewinderEnableFlags (<<= 1)
{
    SidewinderDisabled = 0,     // Completly disable sidewinder
    SidewinderSentry = 1,       // Enable sidewinder sentry rockets
    SidewinderRocket,           // Enable sidewinder soldier rockets
    SidewinderArrow,            // Enable sidewinder huntsman arrows
    SidewinderFlare,            // Enable sidewinder pyro flares
    SidewinderPipe,             // Enable sidewinder demo pipe bombs
    SidewinderSyringe           // Enable sidewinder medic syringes
};

enum SidewinderClientFlags (<<= 1)
{
    DefaultRockets = 0,	        // Check convars for rocket type
    NormalSentryRockets = 1,    // Sentry can fire normal rockets
    TrackingSentryRockets,      // Sentry can fire tracking rockets
    CritSentryRockets,          // Sentry can fire crit rockets
    CritTrackerSentryRockets,   // Only crit sentry rockets track.
    NormalRockets,              // Soldier can fire normal rockets
    TrackingRockets,            // Soldier can fire tracking rockets
    CritTrackerRockets,         // Only crit rockets track.
    NormalArrows,               // Huntsman can fire normal arrows
    TrackingArrows,             // Huntsman can fire tracking arrows
    CritTrackerArrows,          // Only crit arrows track.
    NormalFlares,               // Pyro can fire normal flares
    TrackingFlares,             // Pyro can fire tracking flares
    CritTrackerFlares,          // Only crit flares track.
    NormalPipes,                // Demo can fire normal pipe bombs
    TrackingPipes,              // Demo can fire tracking pipe bombs
    CritTrackerPipes,           // Only crit pipe bombs track.
    NormalSyringe,              // Medic can fire normal syringes
    TrackingSyringe,            // Medic can fire tracking syringes
    CritTrackerSyringe,         // Only crit syringes track.
    ClientIsDesignated,         // Client is "designated" as a target!
    ClientIsCloaked,			// Client is "cloaked" and can not be seen by rockets.
    ClientIsDetected			// Client is "detected" and can be seen even if "cloaked".
};

enum TrackType { NoTrack=0, TrackAll, TrackCrits };

/**
 * Control the Sidewinder extension (disable global settings).
 *
 * @param control	            Set true to disable global settings and control the sidewinder extension.
 * @param enable                If control is false, value for sm_sidewinder_enabled
 * @param sentry_crit           If control is false, value for sm_sentryrocket_crit
 * @param sentry_track_crit     If control is false, value for sm_sentryrocket_crit_tracker
 * @param rocket_track_crit     If control is false, value for sm_rocket_crit_tracker
 * @param arrow_track_crit      If control is false, value for sm_arrow_crit_tracker
 * @param flare_track_crit      If control is false, value for sm_flare_crit_tracker
 * @param pipe_track_crit       If control is false, value for sm_pipe_crit_tracker
 * @param syringe_track_crit    If control is false, value for sm_syringe_crit_tracker
 * @noreturn
 */
native SidewinderControl(bool:control=true,
                         bool:sentry_crit=false,
                         SidewinderEnableFlags:enable=SidewinderDisabled,
                         SidewinderEnableFlags:track_crit=SidewinderDisabled);

/**
 * Enable's Sidewinder rockets for the given client
 *
 * @param client	        The client's index you want.
 * @param flags		        Sidewinder flags to use for this client.
 * @param excludeStatusBits	Set true to leave the status bits unchanged.
 *                          status bits == (cloaked, detected, designated)
 * @noreturn
 */
native SidewinderFlags(client, SidewinderClientFlags:flags, bool:excludeStatusBits=true);

/**
 * Sets chance that a rocket will track targets
 *
 * @param client	The client's index you want.
 * @param chance    Change each rocket will track targets.
 * @noreturn
 */
native SidewinderTrackChance(client, chance);

/**
 * Sets Sidewinder sentry rocket crit chance for the given client
 *
 * @param client	The client's index you want.
 * @param chance    Change each sentry rocket will be a crit rocket.
 * @noreturn
 */
native SidewinderSentryCritChance(client, chance);

/**
 * Designate priority targets for guided rockets
 *
 * @param client	The client's index you want.
 * @param value     Set to true to designate the client as a priority target.
 * @noreturn
 */
native SidewinderDesignateClient(client, bool:value);

/**
 * Cloaks or uncloaks clients from guided rockets
 *
 * @param client	The client's index you want.
 * @param value     Set to true to cloak the target from rockets.
 * @noreturn
 */
native SidewinderCloakClient(client, bool:value);

/**
 * Detects or undetects clients for guided rockets
 *
 * @param client	The client's index you want.
 * @param value     Set to true to detect the target for rockets.
 * @noreturn
 */
native SidewinderDetectClient(client, bool:value);

/**
 * @brief Called whenever Sidewinder needs to query if a shot should be made a seeker
 *
 * @param client	    Index of the client firing the weapon.
 * @param projectile	What kind of projectile is being fired.
 * @param critical      True if the projectile being fired is critical.
 * @param value		    Flag that indicate to track, not track or only track crits.
 * @return			    Plugin_Handled to block tracking.
 */
forward Action:OnSidewinderSeek(client, SidewinderEnableFlags:projectile, bool:critical, &TrackType:value);

/**
 * Do not edit below this line!
 */
public Extension:__ext_Sidewinder = 
{
	name = "Sidewinder",
	file = "sidewinder.ext",
#if defined AUTOLOAD_EXTENSIONS
	autoload = 1,
#else
	autoload = 0,
#endif
#if defined REQUIRE_EXTENSIONS
	required = 1,
#else
	required = 0,
#endif
};


#if !defined REQUIRE_EXTENSIONS
public __ext_Sidewinder_SetNTVOptional()
{
	MarkNativeAsOptional("SidewinderControl");
	MarkNativeAsOptional("SidewinderFlags");
	MarkNativeAsOptional("SidewinderTrackChance");
	MarkNativeAsOptional("SidewinderSentryCritChance");
	MarkNativeAsOptional("SidewinderDesignateClient");
	MarkNativeAsOptional("SidewinderCloakClient");
	MarkNativeAsOptional("SidewinderDetectClient");
}
#endif
