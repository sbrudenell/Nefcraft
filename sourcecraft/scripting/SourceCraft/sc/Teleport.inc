/**
 * vim: set ai et ts=4 sw=4 syntax=sourcepawn :
 * File: Teleport.inc
 * Description: The Teleport Upgrade for SourceCraft.
 * Author(s): Naris (Murray Wilson)
 */
 
#if defined _Teleport_included
 #endinput
#endif
#define _Teleport_included

#include <gametype>
#include <tf2_flag>
#include <TeleportPlayer>

#include "effect/Smoke"
#include "effect/SendEffects"

#define TELEPORT_STUCK_ATTEMPTS 5
#define TELEPORT_MAX_CHARGE_TIME 2.0
#define TELEPORT_MAX_RETRIES 10

#if !defined errorWav
    stock const String:errorWav[] = "sc/perror.mp3";
#endif

#if !defined deniedWav
    stock const String:deniedWav[] = "sc/buzz.wav";
#endif

#if !defined rechargeWav
    stock const String:rechargeWav[] = "sc/transmission.wav";
#endif

enum stuckFlag { notStuck, shortTele, toSpawn };

stock Float:spawnLoc[MAXPLAYERS+1][3];

stock static Float:teleportLoc[MAXPLAYERS+1][3];
stock static Float:m_UltimatePressed[MAXPLAYERS+1];
stock static m_TeleportCount[MAXPLAYERS+1];
stock static m_raceId, m_upgradeId;

stock SetupTeleport(Faction:species, bool:precache, bool:preload=false)
{
    SetupSmokeSprite(precache, preload);

    SetupSound(errorWav, DOWNLOAD, precache, preload);
    SetupSound(deniedWav, DOWNLOAD, precache, preload);
    SetupSound(rechargeWav, DOWNLOAD, precache, preload);
}

stock ResetTeleport(client)
{
    m_TeleportCount[client]=0;
    m_UltimatePressed[client] = 0.0;
}

stock TeleportCommand(client, raceId, upgradeId, level, energy_required, bool:pressed,
                      const Float:teleportDistance[], const String:teleportSnd[]="")
{
    if (pressed)
    {
        m_UltimatePressed[client] = GetGameTime();
        CreateTimer(0.2, UpdateTeleportBar, GetClientUserId(client),
                    TIMER_REPEAT|TIMER_FLAG_NO_MAPCHANGE);
    }
    else
    {
        if (GameType == tf2)
        {
            if (TF2_HasTheFlag(client))
            {
                PrepareSound(deniedWav);
                EmitSoundToClient(client, deniedWav);
                m_UltimatePressed[client] = 0.0;

                decl String:upgradeName[64];
                GetUpgradeName(raceId, upgradeId, upgradeName, sizeof(upgradeName), client);
                DisplayMessage(client, Display_Ultimate, "%t", "NotWithFlag", upgradeName);
                return;
            }
            else
            {
                switch (TF2_GetPlayerClass(client))
                {
                    case TFClass_Spy:
                    {
                        new pcond = TF2_GetPlayerConditionFlags(client);
                        if (TF2_IsCloaked(pcond) || TF2_IsDeadRingered(pcond))
                        {
                            PrepareSound(deniedWav);
                            EmitSoundToClient(client,deniedWav);
                            m_UltimatePressed[client] = 0.0;
                            return;
                        }
                        else if (TF2_IsDisguised(pcond))
                            TF2_RemovePlayerDisguise(client);
                    }
                }
            }
        }

        new stuckFlag:stuck = notStuck;
        new Float:time_pressed = GetGameTime() - m_UltimatePressed[client];
        new energy = GetEnergy(client);

        m_UltimatePressed[client] = 0.0;

        // If the player has already teleported
        if (m_TeleportCount[client] >= 1)
        {
            // Check to see if player got stuck
            new Float:origin[3];
            GetClientAbsOrigin(client, origin);
            if (origin[0] == teleportLoc[client][0] &&
                origin[1] == teleportLoc[client][1] &&
                origin[2] == teleportLoc[client][2])
            {
                // Allow short teleports so players can attempt to get unstuck
                // without having to return to spawn.
                if (m_TeleportCount[client] < TELEPORT_STUCK_ATTEMPTS && time_pressed < 0.3)
                {
                    stuck = shortTele;
                    PrepareSound(errorWav);
                    EmitSoundToClient(client,errorWav);
                    DisplayMessage(client, Display_Ultimate, "%t", "TeleportStuck",
                                   TELEPORT_STUCK_ATTEMPTS-m_TeleportCount[client]);
                }
                else
                {
                    stuck = toSpawn;
                    PrepareSound(errorWav);
                    EmitSoundToClient(client,errorWav);

                    decl String:upgradeName[64];
                    GetUpgradeName(raceId, upgradeId, upgradeName, sizeof(upgradeName), client);
                    DisplayMessage(client, Display_Ultimate, "%t", "TeleportToSpawn", upgradeName);
                }
            }
        }

        // Allow some teleports so players can attempt to get unstuck
        if (!stuck)
        {
            if (energy < energy_required)
            {
                decl String:upgradeName[64];
                GetUpgradeName(raceId, upgradeId, upgradeName, sizeof(upgradeName), client);
                DisplayMessage(client, Display_Energy, "%t", "InsufficientEnergyFor", upgradeName, energy_required);
                EmitEnergySoundToClient(client, GetRaceFaction(raceId));
                return;
            }
            else if (HasCooldownExpired(client, raceId, upgradeId))
                return;
            else if (m_TeleportCount[client] >= 1)
            {
                PrepareSound(deniedWav);
                EmitSoundToClient(client,deniedWav);

                decl String:upgradeName[64];
                GetUpgradeName(raceId, upgradeId, upgradeName, sizeof(upgradeName), client);
                DisplayMessage(client, Display_Ultimate, "%t", "NotRecharged", upgradeName);
                return;
            }
            else if (GetRestriction(client,Restriction_PreventUltimates) ||
                     GetRestriction(client,Restriction_Stunned))
            {
                PrepareSound(deniedWav);
                EmitSoundToClient(client,deniedWav);

                decl String:upgradeName[64];
                GetUpgradeName(raceId, upgradeId, upgradeName, sizeof(upgradeName), client);
                DisplayMessage(client, Display_Ultimate, "%t", "Prevented", upgradeName);
                return;
            }
            else
            {
                m_TeleportCount[client]=0;
            }
        }

        Teleport(client, raceId, upgradeId, level, time_pressed,
                 energy_required, energy, stuck, teleportDistance,
                 teleportSnd);
    }
}

stock Teleport(client, raceId, upgradeId, level, Float:time_pressed,
               energy_required, energy, stuckFlag:stuck,
               const Float:teleportDistance[],
               const String:teleportSnd[]="")
{
    new Float:origin[3];
    GetClientAbsOrigin(client, origin);
    TE_SetupSmoke(origin,SmokeSprite(),40.0,1);
    TE_SendEffectToAll();

    new Float:destloc[3];
    if (stuck == toSpawn)
    {
        energy -= energy_required;
        destloc = spawnLoc[client];
    }
    else
    {
        new Float:clientloc[3],Float:clientang[3];
        GetClientEyePosition(client,clientloc);
        GetClientEyeAngles(client,clientang);

        if (time_pressed > TELEPORT_MAX_CHARGE_TIME || time_pressed <= 0.0)
            time_pressed = TELEPORT_MAX_CHARGE_TIME;

        new Float:percent = time_pressed / TELEPORT_MAX_CHARGE_TIME;
        new Float:range = percent * teleportDistance[level];
        if (range > 0.0)
        {
            new Float:dir[3],Float:endloc[3];
            GetAngleVectors(clientang,dir,NULL_VECTOR,NULL_VECTOR);
            ScaleVector(dir, range);
            AddVectors(clientloc, dir, endloc);
            TR_TraceRayFilter(clientloc, endloc, MASK_PLAYERSOLID,
                              RayType_EndPoint, TraceRayDontHitPlayers,
                              client);
        }
        else
        {
            TR_TraceRayFilter(clientloc, clientang, MASK_PLAYERSOLID,
                              RayType_Infinite, TraceRayDontHitPlayers,
                              client);
        }

        TR_GetEndPosition(destloc);

        if (!stuck)
        {
            new count=0;
            for(;count < TELEPORT_MAX_RETRIES;count++)
            {
                new Float:distance = (GetVectorDistance(clientloc, destloc) - 50.0);
                destloc[1] = (clientloc[1] + (distance * Sine(DegToRad(clientang[1]))));
                destloc[0] = (clientloc[0] + (distance * Cosine(DegToRad(clientang[1]))));

                if (CheckPlayerBox(destloc,clientloc,client))
                    break;
            }

            if (count >= TELEPORT_MAX_RETRIES)
            {
                PrepareSound(deniedWav);
                EmitSoundToClient(client,deniedWav);

                decl String:upgradeName[64];
                GetUpgradeName(raceId, upgradeId, upgradeName, sizeof(upgradeName), client);
                DisplayMessage(client, Display_Ultimate, "%t", "InsufficientRoom", upgradeName);
                return;
            }
        }

        new half = energy_required / 2;
        new amount = RoundToNearest(percent*float(energy_required));
        if (amount < 1)
            amount = 1;
        else if (amount < half)
            amount = half;

        if (energy < amount && energy > 0)
        {
            new Float:p = float(energy) / float(amount);
            percent *= p;
            energy = 0;
        }
        else
            energy -= amount;

        // Save teleport location for stuck comparison later
        teleportLoc[client][0] = destloc[0];
        teleportLoc[client][1] = destloc[1];
        teleportLoc[client][2] = destloc[2];
    }

    SetEnergy(client, energy);
    TeleportPlayer(client,destloc,NULL_VECTOR,NULL_VECTOR);

    if (teleportSnd[0] != '\0')
    {
        PrepareSound(teleportSnd);
        EmitSoundToAll(teleportSnd,client);
    }

    TE_SetupSmoke(destloc,g_smokeSprite,40.0,1);
    TE_SendEffectToAll();

    m_TeleportCount[client]++;

    if (!stuck)
    {
        new Float:cooldown = GetUpgradeCooldown(raceId, upgradeId) * (5-level);
        if (cooldown > 0.0)
        {
            decl String:upgradeName[64];
            GetUpgradeName(raceId, upgradeId, upgradeName, sizeof(upgradeName), client);
            DisplayMessage(client,Display_Ultimate, "%t", "TeleportAgain", cooldown, upgradeName);

            m_raceId = raceId;
            m_upgradeId = upgradeId;
            CreateCooldown(client, raceId, upgradeId,
                           .type=Cooldown_AutoReset
                                |Cooldown_ResetOnChange
                                |Cooldown_ExpireNotify);
        }
    }
}

bool:CheckPlayerBox(Float:bottom[3],Float:start[3],client)
{
    // CSS Player Box = 33 x 73
    // TF2 Player Box = 57 x 95

    new Float:edge[3][8];
    //point A
    edge[2][0]=bottom[2];
    edge[0][0]=bottom[0]+12.5; // 24.5;
    edge[1][0]=bottom[1]+12.5; // 24.5;
    //point B
    edge[2][1]=edge[2][0];
    edge[0][1]=edge[0][0]+25.0; // 37.0;
    edge[1][1]=edge[1][0];
    //point C
    edge[2][2]=edge[2][1];
    edge[0][2]=edge[0][1];
    edge[1][2]=edge[1][1]+25.0; // 37.0;
    //point D
    edge[2][3]=edge[2][2];
    edge[0][3]=edge[0][2]-25.0; // 37.0;
    edge[1][3]=edge[1][2];

    //LogMessage("CheckPlayerBox(%N) bottom={%f,%f,%f},start={%f,%f,%f}",
    //           client, bottom[0], bottom[1], bottom[2],
    //           start[0], start[1], start[2]);

    //other buttons
    for(new i=0;i<4;i++)
    {
        for(new x=0;x<2;x++)
        {
            edge[x][i+4]=edge[x][i];
        }
        edge[2][i+4]=edge[2][i]+67.0; // 89.0;
    }

    for(new i=0;i<4;i++)
    {
        decl Float:point[3];
        point[0]=edge[0][i];
        point[1]=edge[1][i];
        point[2]=edge[2][i];
        decl Float:endpoint[3];
        endpoint[0]=edge[0][i+4];
        endpoint[1]=edge[1][i+4];
        endpoint[2]=edge[2][i+4];
        TR_TraceRayFilter(point, endpoint, MASK_PLAYERSOLID,
                          RayType_EndPoint, TraceRayDontHitSelf,
                          client);
        if(TR_DidHit())
        {
            //LogMessage("CheckPlayerBox(%N) return 1=0, i=%d,point={%f,%f,%f},endpoint={%f,%f,%f}",
            //           client, i, point[0], point[1], point[2], endpoint[0], endpoint[1], endpoint[2]);
            //PrintToChat(client, "CheckPlayerBox() return 1=0, i=%d,point={%f,%f,%f},endpoint={%f,%f,%f}",
            //            i, point[0], point[1], point[2], endpoint[0], endpoint[1], endpoint[2]);
            return false;
        }
    }

    for(new i=0;i<7;i++)
    {
        decl Float:point[3];
        point[0]=edge[0][i];
        point[1]=edge[1][i];
        point[2]=edge[2][i];
        decl Float:endpoint[3];
        endpoint[0]=edge[0][i+1];
        endpoint[1]=edge[1][i+1];
        endpoint[2]=edge[2][i+1];
        TR_TraceRayFilter(point, endpoint, MASK_PLAYERSOLID,
                          RayType_EndPoint, TraceRayDontHitSelf,
                          client);
        if (TR_DidHit())
        {
            //LogMessage("CheckPlayerBox(%N) return 2=0, i=%d,point={%f,%f,%f},endpoint={%f,%f,%f}",
            //           client, i, point[0], point[1], point[2], endpoint[0], endpoint[1], endpoint[2]);
            //PrintToChat(client, "CheckPlayerBox() return 2=0, i=%d,point={%f,%f,%f},endpoint={%f,%f,%f}",
            //            i, point[0], point[1], point[2], endpoint[0], endpoint[1], endpoint[2]);
            return false;
        }
    }

    new Float:top[3];
    top=bottom;
    top[2]+=90; // 112;

    TR_TraceRayFilter(top, bottom, MASK_PLAYERSOLID,
                      RayType_EndPoint, TraceRayDontHitSelf,
                      client);
    if (TR_DidHit())
    {
        //LogMessage("CheckPlayerBox(%N) return 3=0, top={%f,%f,%f},bottom={%f,%f,%f}",
        //           client, top[0], top[1], top[2], bottom[0], bottom[1], bottom[2]);
        //PrintToChat(client, "CheckPlayerBox(%N) return 3=0, top={%f,%f,%f},bottom={%f,%f,%f}",
        //            top[0], top[1], top[2], bottom[0], bottom[1], bottom[2]);
        return false;
    }
    else
    {
        TR_TraceRayFilter(start, top, MASK_PLAYERSOLID,
                          RayType_EndPoint, TraceRayDontHitSelf,
                          client);
        new bool:result = !TR_DidHit();
        //LogMessage("CheckPlayerBox(%N) return 4=%d, start={%f,%f,%f},top={%f,%f,%f}",
        //           client, result, start[0], start[1], start[2], top[0], top[1], top[2]);
        //PrintToChat(client, "CheckPlayerBox() return 4=%d, start={%f,%f,%f},top={%f,%f,%f}",
        //            result, start[0], start[1], start[2], top[0], top[1], top[2]);
        return result; //!TR_DidHit();
    }
}

public OnCooldownExpired(client,race,upgrade,bool:expiredByTime)
{
    if (race == m_raceId && upgrade == m_upgradeId)
    {
        m_TeleportCount[client]=0;
        if (IsClientInGame(client))
        {
            if (GetRace(client) == race && IsPlayerAlive(client) &&
                (GetCooldownType(client,race,upgrade) & Cooldown_ExpireNotify) == Cooldown_None)
            {
                EmitSoundToClient(client, rechargeWav);

                decl String:upgradeName[64];
                GetUpgradeName(race, upgrade, upgradeName, sizeof(upgradeName), client);
                PrintToConsole(client, "%t", "HasRecharged", upgradeName);
            }
        }
    }
}

stock TeleporterAttacked(index, raceId, upgradeId)
{
    new Float:cooldown = GetUpgradeCooldown(raceId, upgradeId);
    if (cooldown > 0.0)
    {
        decl String:upgradeName[64];
        GetUpgradeName(raceId, upgradeId, upgradeName, sizeof(upgradeName), index);
        PrintToConsole(index, "%t", "TeleportWait", cooldown, upgradeName);
        CreateCooldown(index, raceId, upgradeId, .type=Cooldown_AutoReset | Cooldown_ResetOnChange);
        m_TeleportCount[index]=TELEPORT_STUCK_ATTEMPTS;
    }
}

public Action:UpdateTeleportBar(Handle:timer, any:userid)
{
    new client = GetClientOfUserId(userid);
    if (client > 0 && m_UltimatePressed[client] > 0.0 &&
        IsClientInGame(client) && IsPlayerAlive(client))
    {
        new Float:time_pressed = GetGameTime() - m_UltimatePressed[client];
        new String:gauge[30] = "[=====================]";
        new Float:percent = time_pressed/TELEPORT_MAX_CHARGE_TIME;
        if (percent <= 1.0)
        {
            new pos = RoundFloat(percent * 20.0) + 1;
            if (pos < 21)
            {
                if (pos < 1)
                    pos = 1;
                gauge{pos} = ']';
                gauge{pos+1} = 0;
            }
        }
        PrintHintText(client, gauge);
        return Plugin_Continue;
    }
    else
        return Plugin_Stop;
}
