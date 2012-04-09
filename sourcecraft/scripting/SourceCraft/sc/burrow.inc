/**
 * vim: set ai et ts=4 sw=4 syntax=sourcepawn :
 * File: burrow.inc
 * Description: The Zerg Burrow upgrade for SourceCraft.
 * Author(s): -=|JFH|=-Naris
 */
 
#if defined _burrow_included
 #endinput
#endif
#define _burrow_included

#define BURROWED_COMPLETELY 4

#if !defined burrowID
    stock burrowID;
#endif

stock AddBurrowUpgrade(raceId, category=2, required_level=6, max_level=1, start_level=0)
{
    AdditionalTranslations("sc.burrow.phrases.txt");

    decl String:desc[32];
    if (max_level > start_level && start_level > 0)
        Format(desc, sizeof(desc), "%%burrow_%d.%d", start_level, max_level);
    else
        Format(desc, sizeof(desc), "%%burrow_%d", max_level);

    burrowID = AddUpgrade(raceId, "burrow", category, required_level, max_level-start_level,
                          .name="%Burrow", .desc=desc);
}

/**
 * Starts burrowing, or unburrowing, a player depending on
 * if they are already burrowed
 *
 * @param client 	Client
 * @param level:    The level the client has in burrow
 *                  (1=strip weapons, 2=prevent attack, 3=allow attack, 4=unlimited attack - for Bunkers)
 * @param depth:    The maximum depth to burrow to
 *                  (4 is fully burrowed, 2 is 1/2 burrowed - for Bunkers)
 * @param armor:    The amount of additional armor provided (if any) - for Bunkers.
 * @param name:     What to call the additional armor in the HUD and messages - for Bunkers.
 * @return			Retruns true if the player succeeded in burrowing.
 */
native bool:Burrow(client, level, depth=4, armor=0, const String:name[]="");

/**
 * Starts unburrowing, a player if they are burrowed.
 *
 * @param client 	Client
 * @return			none
 */
native UnBurrow(client);

/**
 * Returns true if the entity or client is Burrowed.
 *
 * @param entity 	Entity (or client)
 * @return			4 if the entity is burrowed, 1-3 if the entity is burrowing
 *                  and -1 if the entity is being respawned by Burrow()
 */
native IsBurrowed(entity);

/**
 * Reset a Burrowed Player.
 *
 * @param client 	Client
 * @param unborrow  Starts Unburrowing the player if true, instantly resets burrow if false.
 * @return			none
 */
native ResetBurrow(client, bool:unburrow=false);

/**
 * Starts burrowing, or unburrowing, one or more structures that belong
 * to a particular client depending on if they are already burrowed
 *
 * @param client 	Client
 * @param amount 	Amount of energy required to Burrow one Structure.
 * @return			none
 */
native BurrowStructure(client, amount);

/* DO NOT EDIT BELOW THIS LINE */

public SharedPlugin:__pl_Burrow = 
{
	name = "Burrow",
	file = "Burrow.smx",
#if defined REQUIRE_PLUGIN
	required = 1,
#else
	required = 0,
#endif
};

#if !defined REQUIRE_PLUGIN
public __pl_Burrow_SetNTVOptional()
{
	MarkNativeAsOptional("Burrow");
	MarkNativeAsOptional("UnBurrow");
	MarkNativeAsOptional("IsBurrowed");
	MarkNativeAsOptional("ResetBurrow");
	MarkNativeAsOptional("BurrowStructure");
}
#endif
