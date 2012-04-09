/**
 * vim: set ai et ts=4 sw=4 syntax=sourcepawn :
 * File: sm_gas.inc
 * Description: Native Interface to sm_gas
 * Author(s): -=|JFH|=-Naris (Murray Wilson)
 */

#if defined _sm_gas_included
 #endinput
#endif
#define _sm_gas_included

/**
 * Assume control of the sm_gas plugin
 *
 * @param plugin_only 	Set to 1 to assume complete control of the Gas plugin.
 * @return		    	none
 */
native ControlGas(bool:plugin_only=true);

/**
 * Give a player access to Gas attacks
 *
 * @param index 	Client index
 * @param number    Set to number of gas attacks, -1 is unlimited
 * @param damage    Set to damage inflicted by gas attacks, -1 to use convar value
 * @param radius    Set to radius of the gas cloud, -1 to use convar value
 * @param everyone  Set to 1 to gas everyone, 0 to only gas enemies and -1 to use the mp_friendlyfire value
 * @return			none
 */
native GiveGas(client,number=1,damage=-1,radius=-1,everyone=-1);

/**
 * Take a player's access to Gas attacks away
 *
 * @param index 	Client index
 * @return			none
 */
native TakeGas(client);

/**
 * Enable/Disable a player's access to Gas attacks
 *
 * @param index 	Client index
 * @param enable    Set to true to enable gas attacks, false to disable them
 * @return			none
 */
native EnableGas(client,bool:enable=false);

/**
 * Returns how many gas attacks a player has (or is allowed).
 *
 * @param index 	Client index
 * @param allowed   Set to  true to return number allowed, otherwise
 * @return			returns number of remaining gas attacks.
 */
native HasGas(client,bool:allowed=false);

/**
 * Returns if gas is enabled or disabled
 *
 * @param index 	Client index
 * @return			returns true if Gas Attacks are enabled
 */
native IsGasEnabled(client);

/**
 * Initiate a gas attack
 *
 * @param index 	Client index
 * @return			none
 */
native GasAttack(client);

/* DO NOT EDIT BELOW THIS LINE */

public SharedPlugin:__pl_sm_gas = 
{
	name = "sm_gas",
	file = "sm_gas.smx",
#if defined REQUIRE_PLUGIN
	required = 1,
#else
	required = 0,
#endif
};

#if !defined REQUIRE_PLUGIN
public __pl_sm_gas_SetNTVOptional()
{
	MarkNativeAsOptional("ControlGas");
	MarkNativeAsOptional("GiveGas");
	MarkNativeAsOptional("TakeGas");
	MarkNativeAsOptional("EnableGas");
	MarkNativeAsOptional("HasGas");
	MarkNativeAsOptional("IsGasEnabled");
	MarkNativeAsOptional("GasAttack");
}
#endif
