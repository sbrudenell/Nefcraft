/**
 * vim: set ai et ts=4 sw=4 syntax=sourcepawn :
 * File: armor.inc
 * Description: The Armor Upgrade for SourceCraft.
 * Author(s): -=|JFH|=-Naris
 */
 
#if defined _armor_included
 #endinput
#endif
#define _armor_included

#include "sc/maxhealth"
#include "sc/SourceCraft"

#define ResetArmor(%1) SetArmor(%1,0,.hasShields=false)

stock m_savedArmor[MAXPLAYERS+1];

/**
 * Sets up the amount of armor the client has
 *
 * @param client 	    Client
 * @param level         The upgrade level used to index into the initialArmor[] array
 * @param initialArmor  Array of initial armor amounts for each level (as percent of health).
 * @param armorPercent  Array that holds a range of percent damage the armor absorbs for each level.
 * @param armorName:    What to call the armor in messages. (max 64) [defaults to either Armor or Shields]
 * @param hasShields    Set true if the armor is a shield (regenerates)
 * @param upgrade       Set true if armor is being upgraded
 * @return			    Amount of armor given.
 */
stock SetupArmor(client, level, const Float:initialArmor[], const Float:armorPercent[][2],
                 const String:armorName[]="", bool:hasShields=false, bool:upgrade=false)
{
    new amount = 0;
    ResetMaxHealth(client);
    if (initialArmor[level] > 0.0)
    {
        new maxhp = TF2_GetPlayerMaxHealth(client);
        amount = RoundFloat(float(maxhp)*initialArmor[level]);
    }

    if (upgrade)
    {
        new armor = GetArmor(client);
        new oldArmor = m_savedArmor[client];
        if (level > 0 && armor >= initialArmor[level-1])
            m_savedArmor[client] = amount;
        else if (amount > oldArmor && oldArmor >  0)
            amount = oldArmor;
        else if (armor > oldArmor)
            m_savedArmor[client] = armor;
    }
    else
        m_savedArmor[client] = amount;

    SetArmor(client, amount, armorPercent[level], armorName, hasShields);
    return amount;
}
