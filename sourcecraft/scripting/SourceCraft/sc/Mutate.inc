/**
 * vim: set ai et ts=4 sw=4 syntax=sourcepawn :
 * File: Mutate.inc
 * Description: SourceCraft Upgrade to Mutate into buildings.
 * Author(s): -=|JFH|=-Naris
 */
 
#if defined _Mutate_included
 #endinput
#endif
#define _Mutate_included

#include <ResourceManager>

#undef REQUIRE_PLUGIN
#include <remote>
#include <amp_node>
#include <ztf2grab>
#define REQUIRE_PLUGIN

#include "sc/burrow"
#include "sc/menuitemt"
#include "sc/ShopItems"

#if !defined m_GravgunAvailable
    stock bool:m_GravgunAvailable = false;
#endif

#if !defined m_BuildAvailable
    stock bool:m_BuildAvailable = false;
#endif

#if !defined m_AmpNodeAvailable
    stock bool:m_AmpNodeAvailable = false;
#endif

#if !defined buttonWav
    stock const String:buttonWav[] = "buttons/button14.wav";
#endif

#if !defined errorWav
    stock const String:errorWav[] = "sc/perror.mp3";
#endif

stock const String:mutateWav[] = "sc/zchrdy00.wav";
stock const String:mutateErr[] = "sc/zzdmin00.wav";

stock Float:m_MutateAmpRange[][] =
{
    { 0.0,    0.0,   0.0,   0.0 },
    { 0.0 , 100.0, 150.0, 200.0 },
    { 0.0 , 150.0, 200.0, 250.0 },
    { 0.0 , 200.0, 250.0, 300.0 },
    { 0.0 , 250.0, 300.0, 350.0 }
};

stock Float:m_MutateNodeRange[][] =
{
    { 0.0,    0.0,   0.0,   0.0 },
    { 0.0 , 100.0, 150.0, 200.0 },
    { 0.0 , 150.0, 250.0, 300.0 },
    { 0.0 , 200.0, 300.0, 400.0 },
    { 0.0 , 250.0, 350.0, 500.0 }
};

stock m_MutateNodeRegen[][] =
{
    { 0,  0,  0,  0 },
    { 0, 10, 15, 20 },
    { 0, 15, 20, 25 },
    { 0, 20, 25, 30 },
    { 0, 25, 30, 40 }
};

stock m_MutateNodeShells[][] =
{
    { 0,  0,  0,  0 },
    { 0,  0,  0,  0 },
    { 0,  0,  5, 10 },
    { 0,  5, 10, 15 },
    { 0, 10, 15, 20 }
};

stock m_MutateNodeRockets[] = { 0,  0,  0,  2,  4 };

stock static m_raceId, m_upgradeId;
stock static m_maxUpgradeID, m_maxObjects;

stock SetupMutate(bool:precache, bool:preload=false)
{
    SetupSound(errorWav, true, precache, preload);
    SetupSound(buttonWav, false, precache, preload);
}

stock Mutate(client, level, raceId, upgradeId, maxUpgradeID=-1,
             maxObjects=5, bool:engineersOnly=false)
{
    if (GetGameType() != tf2 || !m_BuildAvailable)
    {
        PrepareSound(mutateErr);
        EmitSoundToClient(client,mutateErr);

        decl String:upgradeName[64];
        GetUpgradeName(raceId, upgradeId, upgradeName, sizeof(upgradeName), client);
        PrintHintText(client,"%t", "IsNotAvailable", upgradeName);
    }
    else if (!IsPlayerAlive(client))
    {
        PrepareSound(mutateErr);
        EmitSoundToClient(client,mutateErr);

        decl String:upgradeName[64];
        GetUpgradeName(raceId, upgradeId, upgradeName, sizeof(upgradeName), client);
        DisplayMessage(client, Display_Ultimate, "%t", "YouHaveDied", upgradeName);
    }
    else if (GetRestriction(client,Restriction_PreventUltimates) ||
             GetRestriction(client,Restriction_Stunned))
    {
        PrepareSound(mutateErr);
        EmitSoundToClient(client,mutateErr);

        decl String:upgradeName[64];
        GetUpgradeName(raceId, upgradeId, upgradeName, sizeof(upgradeName), client);
        DisplayMessage(client, Display_Ultimate, "%t", "Prevented", upgradeName);
    }
    else if (IsBurrowed(client))
    {
        PrepareSound(mutateErr);
        EmitSoundToClient(client,mutateErr);

        decl String:upgradeName[64];
        GetUpgradeName(raceId, upgradeId, upgradeName, sizeof(upgradeName), client);
        DisplayMessage(client, Display_Ultimate, "%t", "NotWhileBurrowed", upgradeName);
    }
    else if (IsMole(client))
    {
        PrepareSound(mutateErr);
        EmitSoundToClient(client,mutateErr);

        decl String:upgradeName[64];
        GetUpgradeName(raceId, upgradeId, upgradeName, sizeof(upgradeName), client);
        DisplayMessage(client, Display_Ultimate, "%t", "NotAsMole", upgradeName);
    }
    else if (HasCooldownExpired(client, raceId, upgradeId))
    {
        new TFClassType:class = TF2_GetPlayerClass(client);
        if (engineersOnly && class != TFClass_Engineer)
        {
            PrepareSound(mutateErr);
            EmitSoundToClient(client,mutateErr);

            decl String:upgradeName[64];
            GetUpgradeName(raceId, upgradeId, upgradeName, sizeof(upgradeName), client);
            DisplayMessage(client, Display_Ultimate, "%t", "EngineersOnly", upgradeName);
            return;
        }
        else
        {
            switch (class)
            {
                case TFClass_Spy:
                {
                    new pcond = TF2_GetPlayerConditionFlags(client);
                    if (TF2_IsCloaked(pcond) || TF2_IsDeadRingered(pcond) || TF2_IsDisguised(pcond))
                    {
                        PrepareSound(mutateErr);
                        EmitSoundToClient(client,mutateErr);
                        return;
                    }
                }
                case TFClass_Scout:
                {
                    if (TF2_IsPlayerBonked(client))
                    {
                        PrepareSound(mutateErr);
                        EmitSoundToClient(client,mutateErr);
                        return;
                    }
                }
            }
        }

        new Handle:menu=CreateMenu(Mutate_Selected);
        SetMenuTitle(menu, "[SC] %T", "MutateTitle", client);

        new maxCount=(maxUpgradeID >= 0) ? GetUpgradeLevel(client,raceId,maxUpgradeID) + 1 : 1;
        if (maxCount > maxObjects)
            maxCount = maxObjects;

        new energy = GetEnergy(client);
        new vespene = GetVespene(client);
        new energy_cost = GetUpgradeEnergy(raceId, upgradeId);
        new vespene_cost = GetUpgradeVespene(raceId, upgradeId);

        new bool:ok=(energy >= energy_cost && vespene >= vespene_cost);

        new counts[TFExtObjectType];
        new sum = CountBuildings(client, counts);

        AddMenuItemT(menu,"6","MiniSentry", (ok && counts[TFObject_Sentry] < maxCount)
                                           ? ITEMDRAW_DEFAULT : ITEMDRAW_DISABLED);

        if (level >= 2)
            AddMenuItemT(menu,"2","Sentry", (ok && counts[TFObject_Sentry] < maxCount)
                                               ? ITEMDRAW_DEFAULT : ITEMDRAW_DISABLED);
        else
            AddMenuItemT(menu,"2","SentryRequiresLevel2", ITEMDRAW_DISABLED);

        if (m_AmpNodeAvailable)
        {
            AddMenuItemT(menu,"7","Amplifier", (ok && counts[TFObject_Dispenser] < maxCount)
                                              ? ITEMDRAW_DEFAULT : ITEMDRAW_DISABLED);

            AddMenuItemT(menu,"8","RepairNode", (ok && counts[TFObject_Dispenser] < maxCount)
                                               ? ITEMDRAW_DEFAULT : ITEMDRAW_DISABLED);
        }
        else
        {
            AddMenuItemT(menu,"0","Dispenser", (ok && counts[TFObject_Dispenser] < maxCount)
                                              ? ITEMDRAW_DEFAULT : ITEMDRAW_DISABLED);
        }

        m_raceId = raceId;
        m_upgradeId = upgradeId;
        m_maxUpgradeID = maxUpgradeID;
        AddMenuItemT(menu,"12","DestroyStructure", (sum > 0) ? ITEMDRAW_DEFAULT : ITEMDRAW_DISABLED);
        DisplayMenu(menu,client,MENU_TIME_FOREVER);
    }
}

public Mutate_Selected(Handle:menu,MenuAction:action,client,selection)
{
    if (action == MenuAction_Select)
    {
        PrepareSound(buttonWav);
        EmitSoundToClient(client,buttonWav);

        if (GetRace(client) == m_raceId)
        {
            decl String:SelectionInfo[12];
            GetMenuItem(menu,selection,SelectionInfo,sizeof(SelectionInfo));

            new item = StringToInt(SelectionInfo);
            if (item == 12)
            {
                if (!DestroyBuildingMenu(client))
                {
                    EmitSoundToClient(client,errorWav);
                    DisplayMessage(client, Display_Ultimate,
                                   "%t", "NoStructuresToDestroy");
                }
            }
            else
            {
                new mutate_level=GetUpgradeLevel(client,m_raceId,m_upgradeId);
                MutateInto(client, m_raceId, m_upgradeId, m_maxUpgradeID, m_maxObjects,
                           TFExtObjectType:item, (mutate_level > 1) ? mutate_level - 1 : 1);
            }
        }
    }
    else if (action == MenuAction_End)
        CloseHandle(menu);
}

stock MutateInto(client, raceId, upgradeId, maxUpgradeID, maxObjects=5,
                 TFExtObjectType:type, level=1, bool:bMini=false,
                 bool:bShielded=false)
{
    new energy = GetEnergy(client);
    new vespene = GetVespene(client);
    new energy_cost = GetUpgradeEnergy(raceId,upgradeId);
    new vespene_cost = GetUpgradeVespene(raceId,upgradeId);

    if (!IsPlayerAlive(client))
    {
        PrepareSound(mutateErr);
        EmitSoundToClient(client,mutateErr);

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
        PrepareSound(mutateErr);
        EmitSoundToClient(client,mutateErr);

        decl String:upgradeName[64];
        GetUpgradeName(raceId, upgradeId, upgradeName, sizeof(upgradeName), client);
        DisplayMessage(client, Display_Ultimate, "%t", "Prevented", upgradeName);
    }
    else if (IsBurrowed(client))
    {
        PrepareSound(mutateErr);
        EmitSoundToClient(client,mutateErr);

        decl String:upgradeName[64];
        GetUpgradeName(raceId, upgradeId, upgradeName, sizeof(upgradeName), client);
        DisplayMessage(client, Display_Ultimate, "%t", "NotWhileBurrowed", upgradeName);
    }
    else if (IsMole(client))
    {
        PrepareSound(mutateErr);
        EmitSoundToClient(client,mutateErr);

        decl String:upgradeName[64];
        GetUpgradeName(raceId, upgradeId, upgradeName, sizeof(upgradeName), client);
        DisplayMessage(client, Display_Ultimate, "%t", "NotAsMole", upgradeName);
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
                        PrepareSound(mutateErr);
                        EmitSoundToClient(client,mutateErr);
                        return;
                    }
                }
                case TFClass_Scout:
                {
                    if (TF2_IsPlayerBonked(client))
                    {
                        PrepareSound(mutateErr);
                        EmitSoundToClient(client,mutateErr);
                        return;
                    }
                }
            }
        }

        new maxCount = (maxUpgradeID >= 0) ? GetUpgradeLevel(client,raceId,maxUpgradeID) + 1 : 1;
        if (maxCount > maxObjects)
            maxCount = maxObjects;

        new numBuilt = CountObjects(client, TF2_ObjectClassNames[type]);
        if (numBuilt >= maxCount)
        {
            PrepareSound(mutateErr);
            EmitSoundToClient(client,mutateErr);
            DisplayMessage(client, Display_Ultimate,
                           "%t", "MutateLimitReached",
                           TF2_ObjectNames[type], numBuilt);
            return;
        }

        new Float:pos[3];
        new ent = BuildObject(client, type, (bMini || type == TFExtObject_MiniSentry) ? 1 : level, bMini,
                              bShielded, .iHealth=GetClientHealth(client), .pos=pos, .check=false, .bDisabled=false);
        if (ent > 0)
        {
            SetEnergy(client, energy-energy_cost);
            SetVespene(client, vespene-vespene_cost);

            KillPlayer(client, .silent=true);

            PrepareSound(mutateWav);
            EmitAmbientSound(mutateWav,pos,ent);

            DisplayMessage(client, Display_Ultimate, "%t",
                           "Mutated", TF2_ObjectNames[type]);

            new Float:cooldown = GetUpgradeCooldown(raceId, upgradeId) * float((numBuilt+1) * 2);
            CreateCooldown(client, raceId, upgradeId, cooldown);

            if (m_AmpNodeAvailable)
            {
                if (type == TFExtObject_Amplifier)
                {
                    ConvertToAmplifier(ent, client, TFCond_Buffed, m_MutateAmpRange[level]);
                }
                else if (type == TFExtObject_RepairNode)
                {
                    ConvertToRepairNode(ent, client, m_MutateNodeRange[level],
                                        m_MutateNodeRegen[level], m_MutateNodeShells[level],
                                        m_MutateNodeRockets[level], .team=true);
                }
            }
        }
        else
        {
            PrepareSound(mutateErr);
            EmitSoundToClient(client,mutateErr);
            DisplayMessage(client, Display_Ultimate,
                           "%t", "UnableToMutate");
        }
    }
}
