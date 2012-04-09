/**
 * vim: set ai et ts=4 sw=4 syntax=sourcepawn :
 * File: TeleportPlayer.inc
 * Description: Stocks to safely teleport a player, avoiding the heavy spinning minigun bug.
 * Author(s): Naris (Murray Wilson)
 */
 
#if defined _TeleportPlayer_included
 #endinput
#endif
#define _TeleportPlayer_included

#include <sdktools_functions>
#include <gametype>

#if defined(REQUIRE_EXTENSIONS)
    #undef  REQUIRE_EXTENSIONS
#endif    

#include <tf2_player>

/**
 * Teleports a player.
 *
 * @param client		Client index.
 * @param origin		New origin, or NULL_VECTOR for no change.
 * @param angles		New angles, or NULL_VECTOR for no change.
 * @param velocity		New velocity, or NULL_VECTOR for no change.
 * @noreturn
 * @error				Invalid entity or client not in game, or lack of mod support.
 */
stock bool:TeleportPlayer(client, const Float:origin[3], const Float:angles[3], const Float:velocity[3])
{
    if (GetGameType() == tf2)
    {
        if (GetEntProp(client, Prop_Send, "m_iStunFlags") != 0)
        {
            // Don't teleport a stunned player -- it crashes the server!
            return false;
        }
        else
        {
            TeleportEntity(client, origin, angles, velocity);
            if (TF2_GetPlayerClass(client) == TFClass_Heavy &&
                TF2_IsPlayerSlowed(client))
            {
                // Stun the heavy briefly to stop the minigun from spinning
                TF2_StunPlayer(client, 0.2, 0.0, TF_STUNFLAG_NOSOUNDOREFFECT|TF_STUNFLAG_THIRDPERSON);
            }
            return true;
        }
    }
    else
    {
        TeleportEntity(client, origin, angles, velocity);
        return true;
    }
}

stock CrashTeleportPlayer(client, const Float:origin[3], const Float:angles[3], const Float:velocity[3])
{
    if (GetGameType() == tf2 && TF2_IsPlayerSlowed(client) &&
        TF2_GetPlayerClass(client) == TFClass_Heavy)
    {
        // Stun the heavy briefly to stop the minigun from spinning
        TF2_StunPlayer(client, 0.2, 0.0, TF_STUNFLAG_NOSOUNDOREFFECT|TF_STUNFLAG_THIRDPERSON);

        new Handle:pack;
        CreateDataTimer(0.1, TeleportHandler, pack, TIMER_FLAG_NO_MAPCHANGE);
        WritePackCell(pack,GetClientUserId(client));
        WritePackFloat(pack, origin[0]);
        WritePackFloat(pack, origin[1]);
        WritePackFloat(pack, origin[2]);
        WritePackFloat(pack, angles[0]);
        WritePackFloat(pack, angles[1]);
        WritePackFloat(pack, angles[2]);
        WritePackFloat(pack, velocity[0]);
        WritePackFloat(pack, velocity[1]);
        WritePackFloat(pack, velocity[2]);
    }
    else
        TeleportEntity(client, origin, angles, velocity);
}

public Action:TeleportHandler(Handle:timer, Handle:pack)
{
    ResetPack(pack);
    new client = GetClientOfUserId(ReadPackCell(pack));
    if (client > 0)
    {
        new Float:origin[3];
        origin[0] = ReadPackFloat(pack);
        origin[1] = ReadPackFloat(pack);
        origin[2] = ReadPackFloat(pack);

        new Float:angles[3];
        angles[0] = ReadPackFloat(pack);
        angles[1] = ReadPackFloat(pack);
        angles[2] = ReadPackFloat(pack);

        new Float:velocity[3];
        velocity[0] = ReadPackFloat(pack);
        velocity[1] = ReadPackFloat(pack);
        velocity[2] = ReadPackFloat(pack);

        TeleportEntity(client, origin, angles, velocity);
    }
    return Plugin_Stop;
}
