/**
 * vim: set ai et ts=4 sw=4 syntax=sourcepawn :
 * File: Shake.inc
 * Description: Shake Effects
 * Author(s): Naris (Murray Wilson)
 * Thanks to: V0gelz (Frederik) frederik156@hotmail.com
 */

#if defined _Shake_included
#endinput
#endif
#define _Shake_included

#include <Hallucinate>
#include "effect/message"

stock Shake(const clients[],numClients,on_off=0,Float:amplitude=30.0,Float:duration=1.0,Float:frequency=30.0)
{
    new Handle:hBf=StartMessage("Shake", clients, numClients);
    if(hBf!=INVALID_HANDLE)
    {
        BfWriteByte(hBf, on_off);
        BfWriteFloat(hBf,amplitude);
        BfWriteFloat(hBf,frequency);
        BfWriteFloat(hBf,duration);
        EndMessage();
    }
}

stock ShakeOne(client,on_off=0,Float:amplitude=30.0,Float:duration=1.0,Float:frequency=30.0)
{
    new Handle:hBf=StartMessageOne("Shake", client);
    if(hBf!=INVALID_HANDLE)
    {
        BfWriteByte(hBf, on_off);
        BfWriteFloat(hBf,amplitude);
        BfWriteFloat(hBf,frequency);
        BfWriteFloat(hBf,duration);
        EndMessage();
    }
}

stock ShakeAll(on_off,Float:amplitude=30.0,Float:duration=1.0,Float:frequency=30.0)
{
    new Handle:hBf=(on_off) ? StartMessageAll("Shake")
                            : StartMessageAllLiving("Shake");
    if(hBf!=INVALID_HANDLE)
    {
        BfWriteByte(hBf, on_off);
        BfWriteFloat(hBf,amplitude);
        BfWriteFloat(hBf,frequency);
        BfWriteFloat(hBf,duration);
        EndMessage();
    }
}

stock ShakeOneAllowed(client,on_off=0,Float:amplitude=30.0,Float:duration=1.0,Float:frequency=30.0)
{
    if (!IsFakeClient(client) &&
        !GetSetting(client,Remove_Queasiness) &&
        !GetImmunity(client,Immunity_Drugs))
    {
        new Handle:hBf=StartMessageOne("Shake", client);
        if(hBf!=INVALID_HANDLE)
        {
            BfWriteByte(hBf, on_off);
            BfWriteFloat(hBf,amplitude);
            BfWriteFloat(hBf,frequency);
            BfWriteFloat(hBf,duration);
            EndMessage();
        }
    }
}

stock ShakeAllAllowed(on_off,Float:amplitude=30.0,Float:duration=1.0,Float:frequency=30.0)
{
    new Handle:hBf=(on_off) ? StartMessageAll("Shake")
                            : StartMessageAllAllowed("Shake");
    if(hBf!=INVALID_HANDLE)
    {
        BfWriteByte(hBf, on_off);
        BfWriteFloat(hBf,amplitude);
        BfWriteFloat(hBf,frequency);
        BfWriteFloat(hBf,duration);
        EndMessage();
    }
}

/**
 * Shakes a player's screen at a magnitude and noise for a duration.
 * @param client: Client's index.
 * @param duration: Optional, how long to shake.
 * @param magnitude: Optional magnitude value.
 * @param noise: Optional noise value.
 * @noreturn
 */
stock ShakeScreen(client,Float:duration=1.0,Float:magnitude=40.0,Float:noise=30.0)
{
    if (!IsFakeClient(client))
    {
        if (GetSetting(client,Remove_Queasiness) ||
            GetImmunity(client,Immunity_Drugs))
        {
            PerformBlind(client, 225);
            CreateTimer(duration, RemoveBlind,
                        GetClientUserId(client),
                        TIMER_FLAG_NO_MAPCHANGE);
        }
        else
        {
            ShakeOne(client, 0, magnitude, duration, noise);
        }
    }
}

public Action:RemoveBlind(Handle:timer,any:userid)
{
    new client = GetClientOfUserId(userid);
    if (client > 0 && IsValidClient(client))
    {
        PerformBlind(client, 0);
    }
    return Plugin_Stop;
}

