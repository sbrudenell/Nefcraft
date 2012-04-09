/**
 * vim: set ai et ts=4 sw=4 syntax=sourcepawn :
 * File: bunker.inc
 * Description: The Terran Bunker upgrade for SourceCraft.
 * Author(s): -=|JFH|=-Naris
 */
 
#if defined _bunker_included
 #endinput
#endif
#define _bunker_included

#include <ResourceManager>
#include <gametype>
#include <tf2_flag>

#include "sc/ShopItems"
#include "sc/burrow"
#include "sc/armor"

#define LeaveBunker         UnBurrow
#define InBunker            IsBurrowed
#define ResetBunker         ResetBurrow

stock const String:deniedWav[] = "sc/buzz.wav";
stock const String:rechargeWav[] = "sc/transmission.wav";

stock SetupBunker(bool:precache, bool:preload=false)
{
    SetupSound(deniedWav, true, false, false);
    SetupSound(rechargeWav, true, false, false);
}

/**
 * The client Enters (or Leaves) the Bunker,
 * depending on if they are already in the Bunker.
 *
 * @param client 	Client
 * @param armor:    The amount of additional armor provided by the Bunker.
 * @return			none
 */
stock EnterBunker(client, armor, raceId, upgradeId)
{
    if (InBunker(client))
    {
        Burrow(client, 4, 2, armor, "Bunker");
        DisplayMessage(client,Display_Ultimate,
                       "%t", "LeftBunker");

        CreateCooldown(client, raceId, upgradeId);
    }
    else if (IsMole(client))
    {
        PrepareSound(deniedWav);
        EmitSoundToClient(client,deniedWav);
        DisplayMessage(client, Display_Ultimate,
                       "%t", "CantEnterBunkerAsMole");
    }
    else if (GetGameType() == tf2 && TF2_HasTheFlag(client))
    {
        PrepareSound(deniedWav);
        EmitSoundToClient(client,deniedWav);
        DisplayMessage(client, Display_Ultimate,
                       "%t", "CantEnterBunkerWithFlag");
    }
    else if (GetRestriction(client,Restriction_PreventUltimates) ||
             GetRestriction(client,Restriction_Stunned))
    {
        PrepareSound(deniedWav);
        EmitSoundToClient(client,deniedWav);
        DisplayMessage(client, Display_Ultimate,
                       "%t", "PreventedFromEnteringBunker");
    }
    else if (HasCooldownExpired(client, raceId, upgradeId))
    {
        new energy = GetEnergy(client);
        new energy_cost = GetUpgradeEnergy(raceId,upgradeId);
        if (energy < energy_cost)
        {
            EmitEnergySoundToClient(client,GetRaceFaction(raceId));
            DisplayMessage(client, Display_Energy, "%t",
                           "InsufficientEnergyToEnterBunker",
                           energy_cost);
        }
        else
        {
            if (Burrow(client, 4, 2, armor, "Bunker"))
            {
                SetEnergy(client, energy-energy_cost);
                CreateCooldown(client, raceId, upgradeId);
                DisplayMessage(client,Display_Ultimate,
                               "%t", "EnteredBunker");
            }
        }
    }
}
