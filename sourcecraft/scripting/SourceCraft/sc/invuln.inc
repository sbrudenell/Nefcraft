/**
 * vim: set ai et ts=4 sw=4 syntax=sourcepawn :
 * File: invuld.inc
 * Description: Invulnerability functions
 * Author(s): Naris (Murray Wilson)
 */
 
#if defined _invuln_included
 #endinput
#endif
#define _invuln_included

#undef REQUIRE_EXTENSIONS
#include <tf2>
#include <tf2_player>
#define REQUIRE_EXTENSIONS

// settings for m_takedamage
#define	DAMAGE_NO				0
#define DAMAGE_EVENTS_ONLY		1		// Call damage functions, but don't modify health
#define	DAMAGE_YES				2
#define	DAMAGE_AIM				3

/**
 * Makes a player Invulnerable
 * @param client: The client's index you want.
 * @noreturn
 */
stock SetInvulnerability(client, Float:duration)
{
    if (GameType == tf2)
        TF2_AddCondition(client, TFCond_Ubercharged, duration);
    else
    {
        SetEntProp(client, Prop_Data, "m_takedamage", DAMAGE_NO, 1);
        if (duration > 0.0)
            CreateTimer(duration, ResetInvuln, GetClientUserId(client), TIMER_FLAG_NO_MAPCHANGE);
    }
}

public Action:ResetInvuln(Handle:timer,any:userid)
{
    new client = GetClientOfUserId(userid);
    if (IsValidClient(client))
        ResetInvulnerability(client);

    return Plugin_Stop;
}

/**
 * Makes a player NOT Invulnerable anymore
 * @param client: The client's index you want.
 * @noreturn
 */
stock ResetInvulnerability(client)
{
    if (GameType == tf2)
        TF2_RemoveCondition(client, TFCond_Ubercharged);
    else        
        SetEntProp(client, Prop_Data, "m_takedamage", DAMAGE_YES, 1);
}

/**
 * Returns true if the player is Invulnerable
 * @param client: The client's index you want.
 * @return The return value will be true if they are invulnerable or false if they aren't.
 */
stock bool:IsInvulnerable(client)
{
    if (GetGameType() == tf2 && TF2_IsPlayerUbercharged(client))
        return true;
    else
        return (GetEntProp(client, Prop_Data, "m_takedamage", 1) == 0);
}
