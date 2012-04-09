/**
 * vim: set ai et ts=4 sw=4 syntax=sourcepawn :
 * File: round_state.inc
 * Description: Defines the round states for SourceCraft
 * Author(s): Naris (Murray Wilson)
 */
 
#if defined _round_state_included
 #endinput
#endif
#define _round_state_included

enum RoundStates
{
       	Unknown,
       	RoundStarted,
        RoundFreeze,
       	RoundActive,
        RoundSetupFinished,
       	RoundOvertime,
       	RoundSuddenDeath,
       	RoundOver,
       	GameOver
};

stock String:RoundStateNames[][] = 
{
    "Unknown",
    "RoundStarted",
    "RoundFreeze",
    "RoundActive",
    "RoundSetupFinished",
    "RoundOvertime",
    "RoundSuddenDeath",
    "RoundOver",
    "GameOver"
};
