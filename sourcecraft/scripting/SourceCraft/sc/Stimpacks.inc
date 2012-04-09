/**
 * vim: set ai et ts=4 sw=4 syntax=sourcepawn :
 * File: Stimpacks.inc
 * Description: The Terran Stimpacks Upgrade for SourceCraft.
 * Author(s): -=|JFH|=-Naris
 */

#if defined _Stimpacks_included
 #endinput
#endif
#define _Stimpacks_included

#undef REQUIRE_PLUGIN
#include "sc/RateOfFire"
#define REQUIRE_PLUGIN

stock const String:errorWav[] = "sc/perror.mp3";
stock const String:stimpacksExpireWav[] = "sc/tmapss01.wav";
stock const String:stimpacksWav[][] = { "sc/tmasti00.mp3",  // Stimpack sounds
                                        "sc/tmasti01.mp3" };

stock stimpacksID;
stock bool:m_ROFAvailable = false;

stock bool:m_StimpacksActive[MAXPLAYERS+1];

stock SetupStimpacks(bool:precache, bool:preload=false)
{
    SetupSound(errorWav, true, false, false);
    for (new i = 0; i < sizeof(stimpacksWav); i++)
        SetupSound(stimpacksWav[i], true, precache, preload);
}

stock Stimpacks(client, level, raceId, upgradeId)
{
    if (level > 0)
    {
        new energy = GetEnergy(client);
        new amount = GetUpgradeEnergy(raceId,upgradeId);
        if (energy < amount)
        {
            decl String:upgradeName[64];
            GetUpgradeName(raceID, upgradeId, upgradeName, sizeof(upgradeName), client);
            DisplayMessage(client, Display_Energy, "%t", "InsufficientEnergyFor", upgradeName, amount);
            EmitEnergySoundToClient(client,GetRaceFaction(raceId));
        }
        else if (!m_ROFAvailable)
        {
            PrepareSound(deniedWav);
            EmitSoundToClient(client,deniedWav);

            decl String:upgradeName[64];
            GetUpgradeName(raceID, upgradeId, upgradeName, sizeof(upgradeName), client);
            PrintHintText(client,"%t", "AreNotAvailable", upgradeName);
        }
        else if (GetRestriction(client,Restriction_PreventUltimates) ||
                 GetRestriction(client,Restriction_Stunned))
        {
            PrepareSound(deniedWav);
            EmitSoundToClient(client,deniedWav);

            decl String:upgradeName[64];
            GetUpgradeName(raceID, upgradeId, upgradeName, sizeof(upgradeName), client);
            DisplayMessage(client, Display_Ultimate, "%t", "Prevented", upgradeName);
        }
        else if (HasCooldownExpired(client, raceId, upgradeId))
        {
            new hurt = level*10;
            new health = GetClientHealth(client);
            if (health <= hurt+10)
            {
                PrepareSound(errorWav);
                EmitSoundToClient(client,errorWav);
                DisplayMessage(client, Display_Ultimate, "%t",
                               "InsufficientHealthForStimpacks");
            }
            else
            {
                SetEnergy(client, energy-amount);
                SetEntityHealth(client, health - hurt);

                SetROF(client, 2.0/float(level),
                GetUpgradeRecurringEnergy(raceId,upgradeId));

                HudMessage(client, "%t", "StimpackHud");
                PrintHintText(client, "%t", "StimpacksActive");

                new num = GetRandomInt(0,sizeof(stimpacksWav)-1);
                PrepareSound(stimpacksWav[num]);
                EmitSoundToAll(stimpacksWav[num],client);

                CreateTimer(2.0 * float(level), EndStimpack,
                            GetClientUserId(client),
                            TIMER_FLAG_NO_MAPCHANGE);
            }
        }
    }
}

public Action:EndStimpack(Handle:timer,any:userid)
{
    new client = GetClientOfUserId(userid);
    if (client > 0)
    {
        if (IsClientInGame(client) && IsPlayerAlive(client))
        {
            PrepareSound(stimpacksExpireWav);
            EmitSoundToAll(stimpacksExpireWav,client);

            decl String:message[64];
            Format(message, sizeof(message), "%T", "StimpackHud", client);
            ReplaceString(message, sizeof(message), "*", "");
            ReplaceString(message, sizeof(message), " ", "");
            ClearHud(client, message);
            PrintHintText(client, "%t", "StimpacksDissipated");
        }

        m_StimpacksActive[client]=false;
        if (m_ROFAvailable)
            SetROF(client, 0.0, 0);

        CreateCooldown(client, raceID, stimpacksID);
    }
}

