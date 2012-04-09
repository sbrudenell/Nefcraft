/**
 * vim: set ai et ts=4 sw=4 syntax=sourcepawn :
 * File: dod_ignite.inc
 * Description: Provide a DOD_IgniteEntity() function that fixes the dod looping burning sound bug
 * Author(s): -=|JFH|=-Naris (Murray Wilson)
 */

#if defined _dod_ignite_included
 #endinput
#endif
#define _dod_ignite_included

/**
 * Ignites an entity on fire and stops the sound when the time expires.
 *
 * @param entity		Entity index.
 * @param time			Number of seconds to set on fire.
 * @param npc			True to only affect NPCs.
 * @param size			Unknown.
 * @param level			Unknown.
 * @noreturn
 * @error				Invalid entity or client not in game, or lack of mod support.
 */
native DOD_IgniteEntity(entity, Float:time, bool:npc=false, Float:size=0.0, bool:level=false);

/* DO NOT EDIT BELOW THIS LINE */

public SharedPlugin:__pl_dod_ignite = 
{
	name = "dod_ignite",
	file = "dod_ignite.smx",
#if defined REQUIRE_PLUGIN
	required = 1,
#else
	required = 0,
#endif
};

#if !defined REQUIRE_PLUGIN
public __pl_dod_ignite_SetNTVOptional()
{
	MarkNativeAsOptional("DOD_IgniteEntity");
}
#endif
