/**
 * vim: set ai et ts=4 sw=4 syntax=sourcepawn :
 * File: ubercharger.inc
 * Description: Native Interface to the Medic Uber Charger for TF2
 * Author(s): -=|JFH|=-Naris (Murray Wilson)
 */

#if defined _ubercharger_included
 #endinput
#endif
#define _ubercharger_included

/**
 * Assume control of the Uber Charger plugin
 *
 * @param plugin_only 	Set to 1 to assume complete control of the Uber Charger plugin.
 * @return		    	none
 */
native ControlUberCharger(bool:plugin_only=true);

/**
 * Give an uber charger to the player
 *
 * @param index 	Client index
 * @param arm       Set to true to enable Uber Charger for Client when a Medic.
 * @param amount    Set to amount of uber per heartbeat, 0.0 = default amount from convar.
 * @return			none
 */
native SetUberCharger(client,bool:arm=true,Float:amount=0.0);

/* DO NOT EDIT BELOW THIS LINE */

public SharedPlugin:__pl_ubercharger = 
{
	name = "ubercharger",
	file = "ubercharger.smx",
#if defined REQUIRE_PLUGIN
	required = 1,
#else
	required = 0,
#endif
};

#if !defined REQUIRE_PLUGIN
public __pl_ubercharger_SetNTVOptional()
{
	MarkNativeAsOptional("ControlUberCharger");
	MarkNativeAsOptional("SetUberCharger");
}
#endif
