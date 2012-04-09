/**
 * vim: set ai et ts=4 sw=4 syntax=sourcepawn :
 * File: dod_drophealthkit_source.inc
 * Description: Native Interface to the TF2 Healthkits
 * Author(s): -=|JFH|=-Naris (Murray Wilson)
 */

#if defined _healthkit_included
 #endinput
#endif
#define _healthkit_included

/**
 * Assume control of the DOD Healthkits plugin
 *
 * @param plugin_only 	Set to 1 to assume complete control of the Healthkits plugin.
 * @return		    	none
 */
native ControlHealthkits(bool:plugin_only=true);

/**
 * Enable Healthkits for a player.
 *
 * @param index 	Client index
 * @param enable    Enable/Disable healthkits for client (0 = disabled   | 1 = on death |
 *                                                        2 = on command | 3 = on death and command)
 * @param rule      Who can pickup the healthkits (0 = everyone | 1 = only teammates | 2 = only enemies)
 * @param count 	Number of ammopacks client given per spawn.
 * @return			none
 */
native SetHealthkit(client,enable=3,rule=0,count=1);

/* DO NOT EDIT BELOW THIS LINE */

public SharedPlugin:__pl_healthkit = 
{
	name = "healthkit",
	file = "dod_drophealthkit_source.smx",
#if defined REQUIRE_PLUGIN
	required = 1,
#else
	required = 0,
#endif
};

#if !defined REQUIRE_PLUGIN
public __pl_healthkit_SetNTVOptional()
{
	MarkNativeAsOptional("ControlHealthkits");
	MarkNativeAsOptional("SetHealthkit");
}
#endif
