/**
 * vim: set ai et ts=4 sw=4 syntax=sourcepawn :
 * File: RecallStructure.inc
 * Description: SourceCraft Upgrade to recall buildings.
 * Author(s): -=|JFH|=-Naris
 */
 
#if defined _RecallStructure_included
 #endinput
#endif
#define _RecallStructure_included

#include <gametype>
#include <tf2_flag>

#undef REQUIRE_PLUGIN
#include <remote>
#include <ztf2grab>
#define REQUIRE_PLUGIN

#include "sc/RecallSounds"
#include "sc/ShopItems"

#if !defined errorWav
    stock const String:errorWav[] = "sc/perror.mp3";
#endif

#if !defined m_GravgunAvailable
    stock bool:m_GravgunAvailable = false;
#endif

stock static m_raceId, m_upgradeId;
stock static bool:m_RecallStructureDelay[MAXPLAYERS+1];

stock RecallStructure(client, raceId, upgradeId, bool:disable, bool:engineersOnly)
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
    else if (GetRestriction(client,Restriction_PreventUltimates) ||
             GetRestriction(client,Restriction_Stunned))
    {
        PrepareSound(deniedWav);
        EmitSoundToClient(client,deniedWav);

        decl String:upgradeName[64];
        GetUpgradeName(raceId, upgradeId, upgradeName, sizeof(upgradeName), client);
        DisplayMessage(client, Display_Ultimate, "%t", "Prevented", upgradeName);
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
    else if (HasCooldownExpired(client, raceId, m_upgradeId))
    {
        if (engineersOnly)
        {
            if (TF2_GetPlayerClass(client) != TFClass_Engineer)
            {
                PrepareSound(deniedWav);
                EmitSoundToClient(client,deniedWav);

                decl String:upgradeName[64];
                GetUpgradeName(raceId, upgradeId, upgradeName, sizeof(upgradeName), client);
                DisplayMessage(client, Display_Ultimate, "%t", "EngineersOnly", upgradeName);
                return;
            }
        }
        else
        {
            switch (TF2_GetPlayerClass(client))
            {
                case TFClass_Spy:
                {
                    new pcond = TF2_GetPlayerConditionFlags(client);
                    if (TF2_IsCloaked(pcond) || TF2_IsDeadRingered(pcond) || TF2_IsDisguised(pcond))
                    {
                        PrepareSound(deniedWav);
                        EmitSoundToClient(client,deniedWav);
                        return;
                    }
                }
                case TFClass_Scout:
                {
                    if (TF2_IsPlayerBonked(client))
                    {
                        PrepareSound(deniedWav);
                        EmitSoundToClient(client,deniedWav);
                        return;
                    }
                }
            }
        }

        new target = GetClientAimTarget(client);
        if (target > 0) 
            RecallIt(client, raceId, upgradeId, target, disable);
        else
        {
            new Handle:menu=CreateMenu(RecallStructure_Selected);
            SetMenuTitle(menu, "[SC] %T", "RecallStructureTitle", client);

            new counts[TFExtObjectType];
            new sum = AddBuildingsToMenu(menu, client, false, counts, target);
            if (sum == 1)
                RecallIt(client, raceId, upgradeId, target, disable);
            else if (sum > 0)
            {
                m_raceId = raceId;
                m_upgradeId = upgradeId;
                m_RecallStructureDelay[client] = disable;
                DisplayMenu(menu,client,MENU_TIME_FOREVER);
            }
            else
            {
                CancelMenu(menu);
                PrepareSound(errorWav);
                EmitSoundToClient(client,errorWav);
                DisplayMessage(client, Display_Ultimate,
                               "%t", "NoStructuresToRecall");
            }
        }
    }
}

public RecallStructure_Selected(Handle:menu,MenuAction:action,client,selection)
{
    if (action == MenuAction_Select)
    {
        PrepareSound(buttonWav);
        EmitSoundToClient(client,buttonWav);
        
        if (GetRace(client) == m_raceId)
        {
            decl String:SelectionInfo[12];
            GetMenuItem(menu,selection,SelectionInfo,sizeof(SelectionInfo));

            new ref = StringToInt(SelectionInfo);
            if (ref != 0)
            {
                new ent = EntRefToEntIndex(ref);
                if (ent > 0 && IsValidEntity(ent))
                    RecallIt(client, m_raceId, m_upgradeId, ent, m_RecallStructureDelay[client]);
            }
        }
    }
    else if (action == MenuAction_End)
        CloseHandle(menu);
}

stock RecallIt(client, raceId, upgradeId, target, bool:disable)
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
        switch (TF2_GetPlayerClass(client))
        {
            case TFClass_Spy:
            {
                new pcond = TF2_GetPlayerConditionFlags(client);
                if (TF2_IsCloaked(pcond) || TF2_IsDeadRingered(pcond) || TF2_IsDisguised(pcond))
                {
                    PrepareSound(deniedWav);
                    EmitSoundToClient(client,deniedWav);
                    return;
                }
            }
            case TFClass_Scout:
            {
                if (TF2_IsPlayerBonked(client))
                {
                    PrepareSound(deniedWav);
                    EmitSoundToClient(client,deniedWav);
                    return;
                }
            }
        }

        if (IsValidEdict(target) && IsValidEntity(target))
        {
            new TFExtObjectType:type=TF2_GetExtObjectType(target);
            if (type != TFExtObject_Unknown)
            {
                if (GetEntPropEnt(target, Prop_Send, "m_hBuilder") == client &&
                    !GetEntProp(target, Prop_Send, "m_bHasSapper") &&
                    !GetEntProp(target, Prop_Send, "m_bDisabled"))
                {
                    new Float:src[3];
                    GetEntPropVector(target, Prop_Send, "m_vecOrigin", src);

                    PrepareSound(recallSrcWav);
                    EmitAmbientSound(recallSrcWav,src);

                    new Float:size[3];
                    GetEntPropVector(target, Prop_Send, "m_vecBuildMaxs", size);

                    new Float:pos[3];
                    GetClientAbsOrigin(client, pos);

                    new Float:angles[3];
                    GetClientAbsAngles(client, angles);

                    if (type == TFExtObject_Teleporter ||
                        type == TFExtObject_TeleporterEntry ||
                        type == TFExtObject_TeleporterExit)
                    {
                        // Move player up
                        new Float:newPos[3];
                        newPos = pos;
                        newPos[2] += size[2] * 1.1;

                        TeleportEntity(client, newPos, NULL_VECTOR, NULL_VECTOR);
                    }
                    else
                    {
                        // Make it noclip so you don't get stuck
                        SetEntProp(target, Prop_Send, "m_CollisionGroup", 5);
                        CreateTimer(2.0, ResetCollisionGroup, target, TIMER_FLAG_NO_MAPCHANGE);
                    }

                    TeleportEntity(target, pos, angles, NULL_VECTOR); // Teleport building

                    PrepareSound(recallDstWav);
                    EmitAmbientSound(recallDstWav,pos,target);

                    if (m_GravgunAvailable)
                        DropEntity(target);

                    if (disable)
                    {
                        new recall_structure_level = GetUpgradeLevel(client,raceId,upgradeId);
                        new Float:delay = float(5-recall_structure_level) * 0.5;
                        if (type == TFExtObject_Sentry &&
                            GetEntProp(target, Prop_Send, "m_bMiniBuilding") == 0)
                        {
                            delay *= float(GetEntProp(target, Prop_Send, "m_iUpgradeLevel")) * 0.25;
                        }

                        if (delay > 0.0)
                        {
                            SetEntProp(target, Prop_Send, "m_bDisabled", 1);
                            CreateTimer(delay, Activate, EntIndexToEntRef(target), TIMER_FLAG_NO_MAPCHANGE);
                        }
                    }

                    SetEnergy(client, energy-energy_cost);
                    SetVespene(client, vespene-vespene_cost);
                    CreateCooldown(client, raceId, upgradeId);
                }
                else
                {
                    PrepareSound(deniedWav);
                    EmitSoundToClient(client,deniedWav);
                    DisplayMessage(client, Display_Ultimate,
                                   "%t", "TargetInvalid");
                }
            }
            else
            {
                PrepareSound(deniedWav);
                EmitSoundToClient(client,deniedWav);
                DisplayMessage(client, Display_Ultimate,
                               "%t", "TargetInvalid");
            }
        }
        else
        {
            PrepareSound(deniedWav);
            EmitSoundToClient(client,deniedWav);
            DisplayMessage(client, Display_Ultimate,
                           "%t", "TargetInvalid");
        }
    }
}

public Action:Activate(Handle:timer,any:ref)
{
    new object = EntRefToEntIndex(ref);
    if (object > 0 && IsValidEdict(object) && IsValidEntity(object))
    {
        if (!GetEntProp(object, Prop_Send, "m_bHasSapper"))
            SetEntProp(object, Prop_Send, "m_bDisabled", 0);
        else
            CreateTimer(0.2, Activate, ref, TIMER_FLAG_NO_MAPCHANGE);
    }
    return Plugin_Stop;
}

public Action:ResetCollisionGroup(Handle:timer,any:object)
{
    if (IsValidEdict(object) && IsValidEntity(object))
    {
        if (TF2_GetExtObjectType(object) != TFExtObject_Unknown)
        {
            if (GetEntProp(object, Prop_Send, "m_CollisionGroup") != 0)
            {
                new builder = GetEntPropEnt(object, Prop_Send, "m_hBuilder");
                if (IsValidClient(builder) && IsPlayerAlive(builder))
                {
                    decl Float:playerPos[3];
                    GetClientAbsOrigin(builder, playerPos);

                    decl Float:objectPos[3];
                    GetEntPropVector(object, Prop_Send, "m_vecOrigin", objectPos);

                    decl Float:size[3];
                    GetEntPropVector(object, Prop_Send, "m_vecBuildMaxs", size);

                    new Float:distance = GetVectorDistance(objectPos, playerPos);
                    if (distance < size[0] * -1.1 || distance > size[0] * 1.1)
                        SetEntProp(object, Prop_Send, "m_CollisionGroup", 0);
                    else
                        CreateTimer(2.0, ResetCollisionGroup, object, TIMER_FLAG_NO_MAPCHANGE);
                }
                else
                    SetEntProp(object, Prop_Send, "m_CollisionGroup", 0);
            }
        }
    }
    return Plugin_Stop;
}
