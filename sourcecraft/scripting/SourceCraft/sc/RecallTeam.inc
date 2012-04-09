/**
 * vim: set ai et ts=4 sw=4 syntax=sourcepawn :
 * File: RecallTeam.inc
 * Description: SourceCraft Upgrade to recall team mates.
 * Author(s): -=|JFH|=-Naris
 */
 
#if defined _RecallTeam_included
 #endinput
#endif
#define _RecallTeam_included

#include <gametype>
#include <tf2_flag>
#include <TeleportPlayer>

#include "sc/RecallSounds"
#include "sc/ShopItems"
#include "sc/burrow"

stock RecallTeam(client, raceId, upgradeId)
{
    new energy = GetEnergy(client);
    new vespene = GetVespene(client);
    new energy_cost = GetUpgradeEnergy(raceId,upgradeId);
    new vespene_cost = GetUpgradeVespene(raceId,upgradeId);

    if (!IsPlayerAlive(client))
    {
        PrepareSound(recallErrWav);
        EmitSoundToClient(client,recallErrWav);

        decl String:upgradeName[64];
        GetUpgradeName(raceId, upgradeId, upgradeName, sizeof(upgradeName), client);
        DisplayMessage(client, Display_Ultimate, "%t", "YouHaveDied", upgradeName);
    }
    else if (energy < energy_cost)
    {
        decl String:upgradeName[64];
        GetUpgradeName(raceId, upgradeId, upgradeName, sizeof(upgradeName), client);
        DisplayMessage(client, Display_Energy, "%t", "InsufficientEnergyFor", upgradeName, energy_cost);
        EmitEnergySoundToClient(client,GetRaceFaction(raceId));
    }
    else if (vespene < vespene_cost)
    {
        decl String:upgradeName[64];
        GetUpgradeName(raceId, upgradeId, upgradeName, sizeof(upgradeName), client);
        DisplayMessage(client, Display_Energy, "%t", "InsufficientVespeneFor", upgradeName, vespene_cost);
        EmitVespeneSoundToClient(client,GetRaceFaction(raceId));
    }
    else if (IsMole(client))
    {
        PrepareSound(recallErrWav);
        EmitSoundToClient(client,recallErrWav);

        decl String:upgradeName[64];
        GetUpgradeName(raceId, upgradeId, upgradeName, sizeof(upgradeName), client);
        DisplayMessage(client, Display_Ultimate, "%t", "NotAsMole", upgradeName);
    }
    else if (GetGameType() == tf2 && TF2_HasTheFlag(client))
    {
        PrepareSound(deniedWav);
        EmitSoundToClient(client,deniedWav);

        decl String:upgradeName[64];
        GetUpgradeName(raceId, upgradeId, upgradeName, sizeof(upgradeName), client);
        DisplayMessage(client, Display_Ultimate, "%t", "NotWithFlag", upgradeName);
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
    else if (HasCooldownExpired(client, raceId, upgradeId))
    {
        if (GameType == tf2)
        {
            switch (TF2_GetPlayerClass(client))
            {
                case TFClass_Spy:
                {
                    new pcond = TF2_GetPlayerConditionFlags(client);
                    if (TF2_IsCloaked(pcond) || TF2_IsDeadRingered(pcond) || TF2_IsDisguised(pcond))
                    {
                        PrepareSound(recallErrWav);
                        EmitSoundToClient(client,recallErrWav);
                        return;
                    }
                }
                case TFClass_Scout:
                {
                    if (TF2_IsPlayerBonked(client))
                    {
                        PrepareSound(recallErrWav);
                        EmitSoundToClient(client,recallErrWav);
                        return;
                    }
                }
            }
        }

        SetEnergy(client, energy-energy_cost);
        SetVespene(client, vespene-vespene_cost);

        new team = GetClientTeam(client);
        new flagcarrier = TF2_GetFlagCarrier(team);
        for (new index=1;index<=MaxClients;index++)
        {
            if (IsClientInGame(index) && IsPlayerAlive(index))
            {
                if (index != client && GetClientTeam(index) == team && index != flagcarrier)
                {
                    decl String:buf[12];
                    IntToString(client, buf, sizeof(buf));

                    new Handle:menu=CreateMenu(Recall_Selected);
                    SetMenuTitle(menu, "[SC] %T", "RecallTeamTitle", index, client);
                    AddMenuItem(menu,buf,"Yes");
                    AddMenuItem(menu,"0","No");
                    DisplayMenu(menu,index,MENU_TIME_FOREVER);

                    new Float:pos[3];
                    GetClientAbsOrigin(index, pos);

                    PrepareSound(recallQryWav);
                    EmitSoundToClient(index,recallQryWav);
                }
            }
        }

        CreateCooldown(client, raceId, upgradeId);
    }
}

public Recall_Selected(Handle:menu,MenuAction:action,index,selection)
{
    if (action == MenuAction_Select)
    {
        PrepareSound(buttonWav);
        EmitSoundToClient(index,buttonWav);
        
        decl String:SelectionInfo[12];
        GetMenuItem(menu,selection,SelectionInfo,sizeof(SelectionInfo));

        new id = StringToInt(SelectionInfo);
        if (IsValidClient(id) && IsPlayerAlive(id) &&
            GetClientTeam(id) == GetClientTeam(index))
        {
            if (IsBurrowed(index))
                ResetBurrow(index);

            new Float:src[3];
            GetClientAbsOrigin(index, src);

            PrepareSound(recallSrcWav);
            EmitAmbientSound(recallSrcWav,src);

            new Float:pos[3];
            GetClientAbsOrigin(id, pos);

            new Float:angles[3];
            GetClientAbsAngles(id, angles);

            TeleportPlayer(index, pos, angles, NULL_VECTOR); // Teleport to id's position

            PrepareSound(recallDstWav);
            EmitAmbientSound(recallDstWav,pos,id);
        }
    }
    else if (action == MenuAction_End)
        CloseHandle(menu);
}
