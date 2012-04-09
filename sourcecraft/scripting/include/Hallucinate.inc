/**
* vim: set ai et ts=4 sw=4 syntax=sourcepawn :
* =============================================================================
* File: Hallucinate.inc
* Description: Native Interface to Hallucinate
*/

#if defined hallucinate_included
 #endinput
#endif
#define hallucinate_included

#define DRUG_RANDOM     0
#define DRUG_SOURCEMOD  1
#define DRUG_DIZZY      2
#define DRUG_CRAZY      3

native bool:PerformDrug(target, toggle, type=DRUG_RANDOM, Float:magnitude=300.0);
native PerformBlind(target, amount);

/**
 * Do not edit below this line!
 */

public SharedPlugin:__pl_Hallucinate = 
{
	name = "Hallucinate",
	file = "Hallucinate.smx",
#if defined REQUIRE_PLUGIN
	required = 1,
#else
	required = 0,
#endif
};

#if !defined REQUIRE_PLUGIN
public __pl_Hallucinate_SetNTVOptional()
{
	MarkNativeAsOptional("PerformDrug");
	MarkNativeAsOptional("PerformBlind");
}
#endif
