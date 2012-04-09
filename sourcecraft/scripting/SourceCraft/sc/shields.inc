/**
 * vim: set ai et ts=4 sw=4 syntax=sourcepawn :
 * File: shields.inc
 * Description: The Shields Upgrade for SourceCraft.
 * Author(s): -=|JFH|=-Naris
 */
 
#if defined _shields_included
 #endinput
#endif
#define _shields_included

#include <entlimit>
#include "sc/armor"

//Uncomment this to enable shield particles effects.
//#define ENABLE_SHIELD_PARTICLES

// Define Shield aliases for Armor functions
#define SetShields          SetArmorAmount
#define GetShields          GetArmor
#define ResetShields        ResetArmor
#define SetShieldsPercent   SetArmorPercent

/**
 * Sets up the amount of shields the client has, an alias for SetupArmor()
 *
 * @param client 	    Client
 * @param level         The upgrade level used to index into the initialArmor[] array
 * @param initialArmor  Array of initial armor amounts for each level (as percent of health).
 * @param armorPercent  Array that holds a range of percent damage the armor absorbs for each level.
 * @param armorName:    What to call the armor in messages. (max 64) [defaults to Shields]
 * @param upgrade       Set true if shields are being upgraded
 * @return			    Amount of armor given.
 */
stock SetupShields(client, level, const Float:initialArmor[],
                   const Float:armorPercent[][2],
                   bool:upgrade=false)
{
    return SetupArmor(client, level, initialArmor, armorPercent,
                      "Shields", true, upgrade);
}

/**
 * Sets up the amount of armor the client has when they also have shields
 *
 * @param client 	    Client
 * @param armor_level   The armor level used to index into the initialArmor[] array
 * @param shields_level The shields level used to index into the maxShields[] array
 * @param initialArmor  Array of initial armor amounts for each level (as percent of health).
 * @param maxShields    Array of maximum shield amounts for each level (as percent of health).
 * @param armorPercent  Array that holds a range of percent damage the armor absorbs for each level.
 * @param armorName:    What to call the armor in messages. (max 64) [defaults to Shields]
 * @param upgrade       Set true when upgrading
 * @return			    Amount of armor given.
 */
stock SetupArmorAndShields(client, armor_level, shields_level, const Float:initialArmor[],
                           const Float:armorPercent[][2], const Float:maxShields[],
                           bool:upgrade=false)
{
    new amount = 0;
    ResetMaxHealth(client);
    if (armor_level > 0 || shields_level > 0)
    {
        new maxhp = GetPlayerMaxHealth(client);
        amount = RoundFloat(float(maxhp)*(initialArmor[armor_level]+maxShields[shields_level]));
    }

    if (upgrade)
    {
        new oldArmor = m_savedArmor[client];
        if (amount > oldArmor)
            amount = oldArmor;
        else
        {
            new armor = GetArmor(client);
            if (armor > m_savedArmor[client])
                m_savedArmor[client] = armor;
        }
    }
    else
        m_savedArmor[client] = amount;

    new bool:hasShields = (shields_level > 0);
    new level = (armor_level > shields_level) ? armor_level : shields_level;
    SetArmor(client, amount, armorPercent[level], (hasShields) ? "Shields" : "Armor", hasShields);
    return amount;
}

stock RegenerateShields(client, level, const Float:maxShields[], amount=1, power=1)
{
    new maxhp = GetPlayerMaxHealth(client);
    new shields = RoundToNearest(float(maxhp) * maxShields[level]);
    if (shields > maxhp)
        shields = maxhp;

    new armor = GetArmor(client);
    if (armor < shields)
    {
        new energy = GetEnergy(client);
        if (energy >= power)
        {
            if (amount > energy)
                amount = energy;
            if (amount > 0)
            {
                if (power > amount)
                    power = amount;

                armor += amount;
                if (armor > shields)
                    armor = shields;

                SetArmorAmount(client, armor);
                SetEnergy(client, energy-power);
            }
        }
    }
}

stock RegenerateFullShields(client, level, const Float:maxShields[], amount=1, power=1)
{
    new maxhp = GetPlayerMaxHealth(client);
    new shields = RoundToNearest(float(maxhp) * maxShields[level]);
    new armor = GetArmor(client);
    if (armor < shields)
    {
        new energy = GetEnergy(client);
        if (energy >= power)
        {
            if (amount > energy)
                amount = energy;
            if (amount > 0)
            {
                if (power > amount)
                    power = amount;

                armor += amount;
                if (armor > shields)
                    armor = shields;

                SetArmorAmount(client, armor);
                SetEnergy(client, energy-power);
            }
        }
    }
}

#define ResetShieldTimer(%1) g_ShieldTimers[%1] = INVALID_HANDLE

stock Handle:g_ShieldTimers[MAXPLAYERS+1];
stock g_ShieldParticle[MAXPLAYERS+1][2];

stock CreateShieldTimer(client)
{
    #if defined ENABLE_SHIELD_PARTICLES
        if (GameType == tf2 && g_ShieldTimers[client] == INVALID_HANDLE &&
            !IsEntLimitReached(.client=client,
                               .message="Unable to spawn anymore shield particles"))
        {
            g_ShieldTimers[client] = CreateTimer(0.5,ShieldEffects,client,
                                             TIMER_REPEAT|TIMER_FLAG_NO_MAPCHANGE);
        }
    #endif
}

stock KillShieldTimer(client)
{
    new Handle:timer=g_ShieldTimers[client];
    if (timer != INVALID_HANDLE)
    {
        g_ShieldTimers[client] = INVALID_HANDLE;	
        KillTimer(timer);
    }

    // Also kill any shield particles.
    DeleteShieldParticles(client);
}

stock KillAllShieldTimers()
{
    for (new i = 1; i <= MaxClients; i++)
        KillShieldTimer(i);
}

stock ResetAllShieldTimers()
{
    for (new i = 1; i <= MaxClients; i++)
        ResetShieldTimer(i);
}

public Action:ShieldEffects(Handle:timer, any:client)
{
    new armor = GetArmor(client);
    if (armor > 25)
    {
        new Float:pos[3];
        pos[2] = 70.0;
        new Float:ang[3];
        ang[2] = 180.0;

        new team = GetClientTeam(client);
        if (TF2_GetPlayerClass(client) == TFClass_Spy)
        {
            new pcond = TF2_GetPlayerConditionFlags(client);
            if (TF2_IsCloaked(pcond))
            {
                DeleteShieldParticles(client);
                return;
            }
            else if (TF2_IsDisguised(pcond))
                team = (team == 2) ? 3 : 2;
        }

        if( team == 2 )
        {
            if (g_ShieldParticle[client][0] <= 0)
                g_ShieldParticle[client][0] = CreateParticle("teleporter_red_exit", 0.0, client, Attach, "", pos, ang);

            if (g_ShieldParticle[client][1] <= 0)
                g_ShieldParticle[client][1] = CreateParticle("teleporter_red_entrance", 0.0, client, Attach, "");
        }
        else if( team == 3 )
        {
            if (g_ShieldParticle[client][0] <= 0)
                g_ShieldParticle[client][0] = CreateParticle("teleporter_blue_exit", 0.0, client, Attach, "", pos, ang);

            if (g_ShieldParticle[client][1] <= 0)
                g_ShieldParticle[client][1] = CreateParticle("teleporter_blue_entrance", 0.0, client, Attach, "");
        }	
    }
    else
        DeleteShieldParticles(client);
}

stock DeleteShieldParticles(client)
{
    if (IsClientInGame(client))
    {
        new particle = g_ShieldParticle[client][0];
        if (particle != 0)
        {
            DeleteParticle(particle);
            g_ShieldParticle[client][0] = 0;
        }

        particle = g_ShieldParticle[client][1];
        if (particle != 0)
        {
            DeleteParticle(particle);
            g_ShieldParticle[client][1] = 0;
        }
    }
}
