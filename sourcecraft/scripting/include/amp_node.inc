/**
 * vim: set ai et ts=4 sw=4 syntax=sourcepawn :
 * File: amp_node.inc
 * Description: Native Interface to Amplifier
 * Author(s): -=|JFH|=-Naris (Murray Wilson)
 */

#if defined _amp_node_included
 #endinput
#endif
#define _amp_node_included

#include <tf2_objects>

/**
 * Assume control of the Amplifier plugin
 *
 * @param plugin_only 	Set to 1 to assume complete control of Amplifier.
 * @return		    	none
 */
native ControlAmpNode(bool:plugin_only=true);

/**
 * Set the Building Type flag to Repair Node
 *
 * @param client 	Client index
 * @param type      Type of building to use when client builds Dispensers.
 * @return		    none
 */
native SetBuildingType(client, TFExtObjectType:type);

/**
 * Set the Building Type flag to Amplifier
 *
 * @param client 	Client index
 * @param range     Range of the amplifier (-1.0=use convar)
 * @param condition Condition to set players within range of the amplifier (-1=use convar)
 * @param percent   Percent (0-100) chance of applying the condition (-1=use convar)
 * @param enable    Set true to enable the player to build Amplifiers.
 * @param select    Set true to set player's BuildingType to Amplifier
 * @return		    none
 */
native SetAmplifier(client, TFCond:condition=TFCond:-1,
                    const Float:range[4]={-1.0,-1.0,-1.0,-1.0},
                    percent=-1, bool:enable=true, bool:select=false);

/**
 * Set the Building Type flag to Repair Node
 *
 * @param client 	Client index
 * @param range     Range of the repair node for each level (-1.0=use convar).
 * @param regen     Regen rate of the repair node for each level (-1=use convar).
 * @param shells    Rate the repair node replenishes sentry shells for each level. (-1=use convar).
 * @param rockets   Rate the repair node replenishes sentry rockets for level 3 sentries. (-1=use convar).
 * @param team      Flag to allow the repair node to repair team mates buildings. (0=no,1=yes,-1=use convar).
 * @param mini      Flag to allow the repair node to repair mini buildings. (0=no,1=yes,-1=use convar).
 * @param percent   Percent (0-100) chance of repairing the building (-1=use convar)
 * @param enable    Set true to enable the client to build Repair Nodes
 * @param select    Set true to set player's BuildingType to Repair Node
 * @return		    none
 */
native SetRepairNode(client, const Float:range[4]={-1.0,-1.0,-1.0,-1.0},
                     const regen[4]={-1,-1,-1,-1},  const shells[4]={-1,-1,-1,-1},
                     const rockets=-1, team=-1, mini=-1, percent=-1,
                     bool:enable=true, bool:select=false);

/**
 * Count how many Buildings of a given type the client has constructed.
 *
 * @param index 	Client index
 * @param type      Type of building to count.
 * @return			The number of buildings of the given type the client has constructed.
 */
native CountConvertedBuildings(client, TFExtObjectType:type);

/**
 * Converts a dispenser into an Amplifier
 *
 * @param entity    Dispenser Entity to convert.
 * @param client 	Client index of the owner.
 * @param range     Range of the amplifier (-1.0=use convar)
 * @param condition Condition to set players within range of the amplifier (-1=use convar)
 * @param percent   Percent (0-100) chance of applying the condition (-1=use convar)
 * @return			none
 */
native ConvertToAmplifier(entity, client, TFCond:condition=TFCond:-1,
                          const Float:range[4]={-1.0,-1.0,-1.0,-1.0},
                          percent=-1);

/**
 * Converts a dispenser into a Repair Node
 *
 * @param entity    Dispenser Entity to convert.
 * @param client 	Client index of the owner.
 * @param range     Range of the repair node for each level (-1.0=use convar).
 * @param regen     Regen rate of the repair node for each level (-1=use convar).
 * @param shells    Rate the repair node replenishes sentry shells for each level (-1=use convar).
 * @param rockets   Rate the repair node replenishes sentry rockets for level 3 sentries. (-1=use convar).
 * @param team      Flag to allow the repair node to repair team mates buildings. (0=no,1=yes,-1=use convar).
 * @param mini      Flag to allow the repair node to repair mini buildings. (0=no,1=yes,-1=use convar).
 * @param percent   Percent (0-100) chance of repairing the building (-1=use convar)
 * @return			none
 */
native ConvertToRepairNode(entity, client, const Float:range[4]={-1.0,-1.0,-1.0,-1.0},
                           const regen[4]={-1,-1,-1,-1}, const shells[4]={-1,-1,-1,-1},
                           const rockets=-1, team=-1, mini=-1, percent=-1);

/**
 * Gets called when the amplifier applys a condition to someone
 *
 * @param builder:  The index of the client that built the amplifier.
 * @param client:   The index of the player that is being afected.
 * @param condition The condition beign applied to the client.
 * @return		    Plugin_Stop will prevent the client from being affected.
 */
forward Action:OnAmplify(builder,client,TFCond:condition);

/* DO NOT EDIT BELOW THIS LINE */

public SharedPlugin:__pl_amp_node = 
{
	name = "amp_node",
	file = "amp_node.smx",
#if defined REQUIRE_PLUGIN
	required = 1,
#else
	required = 0,
#endif
};

#if !defined REQUIRE_PLUGIN
public __pl_amp_node_SetNTVOptional()
{
	MarkNativeAsOptional("ControlAmpNode");
	MarkNativeAsOptional("SetBuildingType");
	MarkNativeAsOptional("SetAmplifier");
	MarkNativeAsOptional("SetRepairNode");
	MarkNativeAsOptional("CountConvertedBuildings");
	MarkNativeAsOptional("ConvertToAmplifier");
	MarkNativeAsOptional("ConvertToRepairNode");
}
#endif

