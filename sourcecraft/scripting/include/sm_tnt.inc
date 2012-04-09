/**
 * vim: set ai et ts=4 sw=4 syntax=sourcepawn :
 * File: sm_tnt.inc
 * Description: Native Interface to TNT
 * Author(s): -=|JFH|=-Naris (Murray Wilson)
 */

#if defined _sm_tnt_included
 #endinput
#endif
#define _sm_tnt_included

/**
 * Assume control of the TNT plugin
 *
 * @param plugin_only 	Set to 1 to assume complete control of the TNT plugin.
 * @return		    	none
 */
native ControlTNT(bool:plugin_only=true);

/**
 * Enable TNT for a player.
 *
 * @param index 	Client index
 * @param count 	Number of TNT packs client given per spawn. (-1 = use convar)
 * @param mode 	    TNT Detonation mode: -1=use convar 0=radio 1=crosshairs 2=timer 3=timer&crosshairs|radio.
 * @param death 	Enable/Disable detonation on owner death/change round.
 * @param detTime 	TNT Detonation delay (-1.0 = use convar)
 * @param primeTime How long it takes TNT to be primed after planting (-1.0 = use convar)
 * @return			none
 */
native SetTNT(client,count=-1,mode=-1, bool:death=false, Float:detTime=-1.0, Float:primeTime=-1.0);

/**
 * Plant a TNT pack
 *
 * @param index 	Client index
 * @return			none
 */
native PlantTNT(client);

/**
 * Defuse a TNT pack
 *
 * @param index 	Client index
 * @return			none
 */
native DefuseTNT(client);

/**
 * Detonate a TNT pack
 *
 * @param index 	Client index
 * @return			none
 */
native DetonateTNT(client);

/**
 * Detonate your or Defuse some else's TNT pack
 *
 * @param index 	Client index
 * @return			none
 */
native TNT(client);

/**
 * Gets called when a player has been bombed with TNT
 *
 * @param tnt:      The entity index of the tnt.
 * @param owner:    The index of the owner of the tnt.
 * @param victim:   The victim's index.
 * @return		    Plugin_Stop will prevent the target from being bombed.
 */
forward Action:OnTNTBombed(tnt,owner,victim);

/* DO NOT EDIT BELOW THIS LINE */

public SharedPlugin:__pl_sm_tnt = 
{
	name = "sm_tnt",
	file = "sm_tnt.smx",
#if defined REQUIRE_PLUGIN
	required = 1,
#else
	required = 0,
#endif
};

#if !defined REQUIRE_PLUGIN
public __pl_sm_tnt_SetNTVOptional()
{
	MarkNativeAsOptional("ControlTNT");
	MarkNativeAsOptional("SetTNT");
	MarkNativeAsOptional("PlantTNT");
	MarkNativeAsOptional("DefuseTNT");
	MarkNativeAsOptional("DetonateTNT");
	MarkNativeAsOptional("TNT");
}
#endif
