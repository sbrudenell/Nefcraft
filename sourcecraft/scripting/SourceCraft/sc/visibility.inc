/*
File: visibility.inc
Description: All the visibility types.
Author(s): Naris
*/

// The types of visibility.

#if defined _Visibility_included
 #endinput
#endif
#define _Visibility_included

enum VisibilityType
{
	DefaultVisibility=-1,	// Indicates no visibility effects have been altered, should not be used outside of the engine
	NormalVisibility=0,	// Same as BasicVisibility. Mostly used to indicate normal, fully visibile with no effects
	BasicVisibility,	// Multiplies visibility*factor1 for melee weapons and factor2 for standing still
	TimedInvisibility,	// Sets invisibility after standing still for factor1 seconds, lasts for factor2 seconds
	TimedMeleeInvisibility,	// Same as TimedInvisibility, but must also have melee weapon equipped
	GraduatedEffects,	// visibility is used to choose a RenderFx instead of an alpha value
	AdditiveVisibility	// Used to add (or change) only certain attributes to what has already been set
};

enum VisibilityPriority
{
	VisibilityPriorityNotSpecified=-1,
	VisibilityPriorityNone=0,
	VisibilityPriorityBase,
	VisibilityPriorityDefault,
	VisibilityPrioritySkill,
	VisibilityPriorityUltimate,
	VisibilityPriorityOverride=255
}

// Additional MoveType used by playertacking
#define MOVETYPE_DEFAULT (MoveType:-1)
