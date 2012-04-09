/**
 * vim: set ai et ts=4 sw=4 syntax=sourcepawn :
 * File: Fade.inc
 * Description: Fade Effects
 * Author(s): Naris (Murray Wilson)
 * Thanks to: V0gelz (Frederik) frederik156@hotmail.com
 *
 */

#if defined _Fade_included
#endinput
#endif
#define _Fade_included

#define FFADE_IN        0x0001  // Fade in
#define FFADE_OUT       0x0002  // Fade out (not in)
#define FFADE_MODULATE  0x0004  // Modulate (don't blend)
#define FFADE_STAYOUT   0x0008  // ignores the duration, stays faded out until new Fade message received
#define FFADE_PURGE     0x0010  // Purges all other fades, replacing them with this one

stock Fade(clients[], numClients, duration,time,const color[4], code=FFADE_IN)
{
    new Handle:hBf=StartMessage("Fade", clients, numClients);
    if(hBf!=INVALID_HANDLE)
    {
        BfWriteShort(hBf,duration); //*400);
        BfWriteShort(hBf,time); //*400);
        BfWriteShort(hBf,code);
        BfWriteByte(hBf,color[0]);
        BfWriteByte(hBf,color[1]);
        BfWriteByte(hBf,color[2]);
        BfWriteByte(hBf,color[3]);
        EndMessage();
    }
}

stock FadeOne(client, duration,time,const color[4], code=FFADE_IN)
{
    new Handle:hBf=StartMessageOne("Fade", client);
    if(hBf!=INVALID_HANDLE)
    {
        BfWriteShort(hBf,duration); //*400);
        BfWriteShort(hBf,time); //*400);
        BfWriteShort(hBf,code);
        BfWriteByte(hBf,color[0]);
        BfWriteByte(hBf,color[1]);
        BfWriteByte(hBf,color[2]);
        BfWriteByte(hBf,color[3]);
        EndMessage();
    }
}

stock FadeAll(duration,time,const color[4], code=FFADE_IN)
{
    new Handle:hBf=StartMessageAllLiving("Fade");
    if(hBf!=INVALID_HANDLE)
    {
        BfWriteShort(hBf,duration); //*400);
        BfWriteShort(hBf,time); //*400);
        BfWriteShort(hBf,code);
        BfWriteByte(hBf,color[0]);
        BfWriteByte(hBf,color[1]);
        BfWriteByte(hBf,color[2]);
        BfWriteByte(hBf,color[3]);
        EndMessage();
    }
}

stock FadeVulerable(duration,time,const color[4], code=FFADE_IN)
{
    new total = 0;
    new altTotal = 0;
    new clients[MaxClients];
    new altClients[MaxClients];
    for (new i=1; i<=MaxClients; i++)
    {
        if (IsClientConnected(i) && IsClientInGame(i) &&
            IsPlayerAlive(i) && !IsFakeClient(i))
        {
            if (!GetImmunity(i,Immunity_Blindness))
            {
                if (!GetSetting(index, Remove_Queasiness) &&
                    !GetSetting(index, Reduce_Queasiness))
                {
                    clients[total++] = i;
                }
                else
                    altClients[altTotal++] = i;
            }
        }
    }

    if (total > 0)
        Fade(clients, total, duration,time,color,code);

    if (altTotal > 0)
    {
        static const black[4]={0,0,0,255};
        Fade(altClients, altTotal, duration,time,black,code);
    }
}
