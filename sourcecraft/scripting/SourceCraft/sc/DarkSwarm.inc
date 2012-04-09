/**
 * vim: set ai et ts=4 sw=4 syntax=sourcepawn :
 * File: DarkSwarm.inc
 * Description: The Zerg Dark Swarm upgrade for SourceCraft.
 * Author(s): -=|JFH|=-Naris
 */
 
#if defined darkswarm_included
 #endinput
#endif
#define darkswarm_included

#include <ResourceManager>
#include <entlimit>

#undef REQUIRE_EXTENSIONS
#include <tf2>
#include <tf2_player>
#define REQUIRE_EXTENSIONS

#if !defined errorWav
    stock const String:errorWav[] = "sc/perror.mp3";
#endif

#if !defined deniedWav
    stock const String:deniedWav[] = "sc/buzz.wav";
#endif

#if !defined rechargeWav
    stock const String:rechargeWav[] = "sc/transmission.wav";
#endif

stock const String:darkSwarmWav[] = "sc/zdeblo01.wav";

stock static m_Swarmed[MAXPLAYERS+1][MAXPLAYERS+1];
stock static Float:m_DarkSwarmPos[MAXPLAYERS+1][3];
stock static Handle:m_DarkSwarmTimers[MAXPLAYERS+1];

stock SetupDarkSwarm(Faction:species, bool:precache, bool:preload=false)
{
    SetupSound(darkSwarmWav, true, precache, preload);

    // Also in SetupPlague()
    SetupSound(errorWav, true, precache, preload);
    SetupSound(deniedWav, true, precache, preload);
    SetupSound(rechargeWav, true, precache, preload);
}

stock DarkSwarm(client, raceId, upgradeId, level, Float:swarmRange[])
{
    new energy = GetEnergy(client);
    new amount = GetUpgradeEnergy(raceId,upgradeId);
    if (energy < amount)
    {
        decl String:upgradeName[64];
        GetUpgradeName(raceId, upgradeId, upgradeName, sizeof(upgradeName), client);
        DisplayMessage(client, Display_Energy, "%t", "InsufficientEnergyFor", upgradeName, amount);
        EmitEnergySoundToClient(client,GetRaceFaction(raceId));
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
    }
    else if (!HasCooldownExpired(client, raceId, upgradeId))
        return;
    else if (IsEntLimitReached(.client=client,
                               .message="Unable to spawn anymore dark swarm clouds"))
    {
        decl String:upgradeName[64];
        GetUpgradeName(raceId, upgradeId, upgradeName, sizeof(upgradeName), client);
        DisplayMessage(client, Display_Ultimate, "%t", "NoEntitiesAvailable");
        return;
    }

    new Float:location[3];
    GetClientAbsOrigin(client, location);
    m_DarkSwarmPos[client] = location;

    new String:originData[64];
    Format(originData, sizeof(originData), "%f %f %f",
           location[0], location[1], location[2]);

    // Create the Dark Swarm Cloud
    new String:size[5];
    new String:name[128];
    Format(size, sizeof(size), "%1.0f", swarmRange[level]);
    Format(name, sizeof(name), "DarkSwarm%i", client);
    new cloud = CreateEntityByName("env_smokestack");
    if (cloud > 0 && IsValidEdict(cloud))
    {
        DispatchKeyValue(cloud,"targetname", name);
        DispatchKeyValue(cloud,"Origin", originData);
        DispatchKeyValue(cloud,"BaseSpread", "100");
        DispatchKeyValue(cloud,"SpreadSpeed", "10");
        DispatchKeyValue(cloud,"Speed", "80");
        DispatchKeyValue(cloud,"StartSize", "100");
        DispatchKeyValue(cloud,"EndSize", size);
        DispatchKeyValue(cloud,"Rate", "15");
        DispatchKeyValue(cloud,"JetLength", "400");
        DispatchKeyValue(cloud,"Twist", "4");
        DispatchKeyValue(cloud,"RenderColor", "250 133 0");
        DispatchKeyValue(cloud,"RenderAmt", "100");
        DispatchKeyValue(cloud,"SmokeMaterial", "particle/particle_smokegrenade1.vmt");
        DispatchSpawn(cloud);
        AcceptEntityInput(cloud, "TurnOn");

        PrepareSound(darkSwarmWav);
        EmitSoundToAll(darkSwarmWav,client);

        new Float:length = 3.0 * float(level);
        new Handle:entitypack = CreateDataPack();
        CreateTimer(length, RemoveDarkSwarm, entitypack);
        WritePackCell(entitypack, EntIndexToEntRef(cloud));
        WritePackCell(entitypack, client);
        WritePackFloat(entitypack, length);

        if (m_DarkSwarmTimers[client] == INVALID_HANDLE)
        {
            new Handle:pack;
            m_DarkSwarmTimers[client] = CreateDataTimer(0.2,DarkSwarmTimer,pack,
                                                        TIMER_REPEAT|TIMER_FLAG_NO_MAPCHANGE);
            if (pack != INVALID_HANDLE)
            {
                WritePackCell(pack, client);
                WritePackFloat(pack, swarmRange[level]);
            }
        }

        SetEnergy(client, energy-amount);

        decl String:upgradeName[64];
        GetUpgradeName(raceId, upgradeId, upgradeName, sizeof(upgradeName), client);
        DisplayMessage(client, Display_Ultimate, "%t", "PervadesTheLand", upgradeName);
        CreateCooldown(client, raceId, upgradeId);
    }
    else
        LogError("Unable to create DarkSwarm cloud!");
}

public Action:DarkSwarmTimer(Handle:timer, Handle:pack)
{
    if (pack != INVALID_HANDLE)
    {
        ResetPack(pack);
        new client = ReadPackCell(pack);
        new Float:range = ReadPackFloat(pack);

        new team = GetClientTeam(client);
        for (new index=1;index<=MaxClients;index++)
        {
            if (IsClientInGame(index) && IsPlayerAlive(index) &&
                (index == client || GetClientTeam(index) == team))
            {
                new Float:indexLoc[3];
                GetClientAbsOrigin(index, indexLoc);
                new bool:inRange = (index == client ||
                                    (IsPointInRange(m_DarkSwarmPos[client],indexLoc,range) &&
                                     TraceTargetIndex(client, index, m_DarkSwarmPos[client], indexLoc)));

                if (inRange)
                {
                    SetImmunity(index,Immunity_RangedAttacks,true);
                    if (!m_Swarmed[client][index])
                    {
                        m_Swarmed[client][index] = true;
                        DisplayMessage(index, Display_Misc_Message,
                                       "%t", "YouHaveBeenProtected", client);
                    }
                }
                else
                {
                    SetImmunity(index,Immunity_RangedAttacks,false);
                    if (m_Swarmed[client][index])
                    {
                        m_Swarmed[client][index] = false;
                        DisplayMessage(index, Display_Misc_Message,
                                       "%t", "YouHaveBeenUnprotected");
                    }
                }
            }
        }
    }
    return Plugin_Continue;
}

stock ResetDarkSwarm(client)
{
    new Handle:timer = m_DarkSwarmTimers[client];
    if (timer != INVALID_HANDLE)
    {
        KillTimer(timer, true);
        m_DarkSwarmTimers[client] = INVALID_HANDLE;	
    }

    for (new index=1;index<=MaxClients;index++)
    {
        SetImmunity(index,Immunity_RangedAttacks,false);
        if (m_Swarmed[client][index])
        {
            m_Swarmed[client][index] = false;
            DisplayMessage(index, Display_Misc_Message,
                           "%t", "YouHaveBeenUnprotected");
        }
    }
}

stock ResetProtected(index)
{
    SetImmunity(index,Immunity_RangedAttacks,false);

    for (new client=1;client<=MaxClients;client++)
        m_Swarmed[client][index] = false;
}

public Action:RemoveDarkSwarm(Handle:timer, Handle:entitypack)
{
    ResetPack(entitypack);
    new cloud = EntRefToEntIndex(ReadPackCell(entitypack));
    new client = ReadPackCell(entitypack);
    new Float:length = ReadPackFloat(entitypack);

    ResetDarkSwarm(client);

    if (cloud > 0 && IsValidEntity(cloud))
    {
        AcceptEntityInput(cloud, "TurnOff");
        CreateTimer((length > 5.0) ? length : 5.0, KillDarkSwarm, entitypack);
    }
}

public Action:KillDarkSwarm(Handle:timer, Handle:entitypack)
{
    ResetPack(entitypack);

    new cloud = EntRefToEntIndex(ReadPackCell(entitypack));
    if (cloud > 0 && IsValidEntity(cloud))
        AcceptEntityInput(cloud, "Kill");

    CloseHandle(entitypack);
}

