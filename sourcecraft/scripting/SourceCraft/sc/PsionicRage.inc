/**
 * vim: set ai et ts=4 sw=4 :
 * File: PsionicRage.inc
 * Description: The Psionic Rage Upgrade for SourceCraft.
 * Author(s): Naris (Murray Wilson)
 */

#if defined _PsionicRage_included
 #endinput
#endif
#define _PsionicRage_included

#include <ResourceManager>
#include "sc/shields"

#if !defined deniedWav
    stock const String:deniedWav[] = "sc/buzz.wav";
#endif

stock static m_raceId, m_upgradeId;
stock static String:m_RageExpireSound[PLATFORM_MAX_PATH];
stock bool:m_RageActive[MAXPLAYERS+1];

PsionicRage(client, raceId, upgradeId, level,
            const String:activeSound[],
            const String:expireSound[])
{
    if (level > 0)
    {
        decl String:upgradeName[64];
        GetUpgradeName(raceId, upgradeId, upgradeName, sizeof(upgradeName), client);

        new energy = GetEnergy(client);
        new vespene = GetVespene(client);
        new energy_cost = GetUpgradeEnergy(raceId,upgradeId);
        new vespene_cost = GetUpgradeVespene(raceId,upgradeId);
        if (energy < energy_cost)
        {
            EmitEnergySoundToClient(client,Protoss);
            DisplayMessage(client, Display_Energy, "%t", "InsufficientEnergyFor",
                           upgradeName, energy_cost);
        }
        else if (vespene < vespene_cost)
        {
            EmitVespeneSoundToClient(client,Protoss);
            DisplayMessage(client, Display_Energy, "%t", "InsufficientVespene",
                           upgradeName, vespene_cost);
        }
        else if (GetRestriction(client,Restriction_PreventUltimates) ||
                 GetRestriction(client,Restriction_Stunned))
        {
            PrepareSound(deniedWav);
            EmitSoundToClient(client,deniedWav);
            DisplayMessage(client, Display_Ultimate, "%t",
                           "Prevented", upgradeName);
        }
        else if (HasCooldownExpired(client, raceId, upgradeId))
        {
            SetVespene(client, vespene-vespene_cost);
            SetShields(client, 0);
            SetEnergy(client, 0);

            m_raceId = raceId;
            m_upgradeId = upgradeId;
            m_RageActive[client] = true;
            TF2_SetPlayerPowerPlay(client, true);
            strcopy(m_RageExpireSound, sizeof(m_RageExpireSound), expireSound);
            CreateTimer(4.0 * float(level), EndRage, GetClientUserId(client),TIMER_FLAG_NO_MAPCHANGE);

            HudMessage(client, "%t", "RageHud");
            PrintHintText(client, "%t", "YouAreRaging");

            PrepareSound(activeSound);
            EmitSoundToAll(activeSound,client);
        }
    }
}

public Action:EndRage(Handle:timer,any:userid)
{
    new client = GetClientOfUserId(userid);
    if (IsValidClient(client) && m_RageActive[client])
    {
        m_RageActive[client]=false;
        TF2_SetPlayerPowerPlay(client, false);
        SetShields(client, 0);
        SetEnergy(client, 0);

        if (IsPlayerAlive(client))
        {
            PrepareSound(m_RageExpireSound);
            EmitSoundToAll(m_RageExpireSound,client);
            PrintHintText(client, "%t", "YouStoppedRaging");
        }

        decl String:message[64];
        Format(message, sizeof(message), "%T", "RageHud", client);
        ReplaceString(message, sizeof(message), "*", "");
        ReplaceString(message, sizeof(message), " ", "");
        ClearHud(client, message);

        CreateCooldown(client, m_raceId, m_upgradeId,
                       .type=Cooldown_CreateNotify
                            |Cooldown_AlwaysNotify);
    }
}

