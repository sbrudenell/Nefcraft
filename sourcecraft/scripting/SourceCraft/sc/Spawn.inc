/**
 * vim: set ai et ts=4 sw=4 syntax=sourcepawn :
 * File: Spawn.inc
 * Description: SourceCraft Upgrade to spawn buildings.
 * Author(s): -=|JFH|=-Naris
 */
 
#if defined _Spawn_included
 #endinput
#endif
#define _Spawn_included

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

#if !defined m_AmpNodeAvailable
    stock bool:m_AmpNodeAvailable = false;
#endif

#if !defined errorWav
    stock const String:errorWav[] = "sc/perror.mp3";
#endif

#if !defined buttonWav
    stock const String:buttonWav[] = "buttons/button14.wav";
#endif

#if !defined deniedWav
    stock const String:deniedWav[] = "sc/buzz.wav";
#endif

stock Float:m_SpawnAmpRange[][] =
{
    { 0.0,    0.0,   0.0,   0.0 },
    { 0.0 , 100.0, 150.0, 200.0 },
    { 0.0 , 150.0, 200.0, 250.0 },
    { 0.0 , 200.0, 250.0, 300.0 },
    { 0.0 , 250.0, 300.0, 350.0 }
};

stock Float:m_SpawnNodeRange[][] =
{
    { 0.0,    0.0,   0.0,   0.0 },
    { 0.0 , 100.0, 150.0, 200.0 },
    { 0.0 , 150.0, 250.0, 300.0 },
    { 0.0 , 200.0, 300.0, 400.0 },
    { 0.0 , 250.0, 350.0, 500.0 }
};

stock m_SpawnNodeRegen[][] =
{
    { 0,  0,  0,  0 },
    { 0, 10, 15, 20 },
    { 0, 15, 20, 25 },
    { 0, 20, 25, 30 },
    { 0, 25, 30, 40 }
};

stock m_SpawnNodeShells[][] =
{
    { 0,  0,  0,  0 },
    { 0,  0,  0,  0 },
    { 0,  0,  5, 10 },
    { 0,  5, 10, 15 },
    { 0, 10, 15, 20 }
};

stock m_SpawnNodeRockets[] = { 0,  0,  0,  2,  4 };

stock static m_raceId, m_upgradeId;
stock static String:m_SpawnSound[PLATFORM_MAX_PATH];

stock Spawn(client, level, raceId, upgradeId, maxObjects, bool:engineersOnly,
            const String:spawnSound[], const String:menuTitle[])
{
    if (!IsPlayerAlive(client))
    {
        PrepareSound(deniedWav);
        EmitSoundToClient(client,deniedWav);

        decl String:upgradeName[64];
        GetUpgradeName(raceId, upgradeId, upgradeName, sizeof(upgradeName), client);
        DisplayMessage(client, Display_Ultimate, "%t", "YouHaveDied", upgradeName);
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
    else if (IsBurrowed(client))
    {
        PrepareSound(deniedWav);
        EmitSoundToClient(client,deniedWav);

        decl String:upgradeName[64];
        GetUpgradeName(raceId, upgradeId, upgradeName, sizeof(upgradeName), client);
        DisplayMessage(client, Display_Ultimate, "%t", "NotWhileBurrowed", upgradeName);
    }
    else if (IsMole(client))
    {
        PrepareSound(deniedWav);
        EmitSoundToClient(client,deniedWav);

        decl String:upgradeName[64];
        GetUpgradeName(raceId, upgradeId, upgradeName, sizeof(upgradeName), client);
        DisplayMessage(client, Display_Ultimate, "%t", "NotAsMole", upgradeName);
    }
    else if (HasCooldownExpired(client, raceId, upgradeId))
    {
        new TFClassType:class = TF2_GetPlayerClass(client);
        if (engineersOnly && class != TFClass_Engineer)
        {
            PrepareSound(deniedWav);
            EmitSoundToClient(client,deniedWav);

            decl String:upgradeName[64];
            GetUpgradeName(raceId, upgradeId, upgradeName, sizeof(upgradeName), client);
            DisplayMessage(client, Display_Ultimate, "%t", "EngineersOnly", upgradeName);

        }
        else
        {
            switch (class)
            {
                case TFClass_Spy:
                {
                    new pcond = TF2_GetPlayerConditionFlags(client);
                    if (TF2_IsCloaked(pcond) || TF2_IsDeadRingered(pcond))
                    {
                        PrepareSound(deniedWav);
                        EmitSoundToClient(client,deniedWav);
                        return;
                    }
                    else if (TF2_IsDisguised(pcond))
                        TF2_RemovePlayerDisguise(client);
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

        decl String:title[64];
        Format(title, sizeof(title), "[SC] %T", menuTitle, client);

        new Handle:menu=CreateMenu(Spawn_Selected);
        SetMenuTitle(menu,title);

        new energy = GetEnergy(client);
        new vespene = GetVespene(client);
        new energy_cost = GetUpgradeEnergy(raceId,upgradeId);
        new vespene_cost = GetUpgradeVespene(raceId,upgradeId);
        new bool:ok=(energy >= energy_cost && vespene >= vespene_cost);

        new max_count = (level > maxObjects) ? maxObjects : level;
        new max_dispensers  = (max_count > 1) ? 1 : max_count;

        new counts[TFExtObjectType];
        new sum = CountBuildings(client, counts);

        AddMenuItemT(menu,"6","MiniSentry", (ok && counts[TFObject_Sentry] < max_count) ? ITEMDRAW_DEFAULT : ITEMDRAW_DISABLED);

        AddMenuItemT(menu,"2","Sentry", (ok && counts[TFObject_Sentry] < max_count) ? ITEMDRAW_DEFAULT : ITEMDRAW_DISABLED);
        AddMenuItemT(menu,"0","Dispenser", (ok && counts[TFObject_Dispenser] < max_dispensers) ? ITEMDRAW_DEFAULT : ITEMDRAW_DISABLED);
        AddMenuItemT(menu,"7","Amplifier", (ok && counts[TFObject_Dispenser] < max_count) ? ITEMDRAW_DEFAULT : ITEMDRAW_DISABLED);
        AddMenuItemT(menu,"8","RepairNode", (ok && counts[TFObject_Dispenser] < max_count) ? ITEMDRAW_DEFAULT : ITEMDRAW_DISABLED);
        AddMenuItemT(menu,"12","DestroyStructure", sum > 0 ? ITEMDRAW_DEFAULT : ITEMDRAW_DISABLED);

        m_raceId = raceId;
        m_upgradeId = upgradeId;
        strcopy(m_SpawnSound, sizeof(m_SpawnSound), spawnSound);
        DisplayMenu(menu,client,MENU_TIME_FOREVER);
    }
}

public Spawn_Selected(Handle:menu,MenuAction:action,client,selection)
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
                    PrepareSound(errorWav);
                    EmitSoundToClient(client,errorWav);
                    DisplayMessage(client, Display_Ultimate,
                                   "%t", "NoStructuresToDestroy");
                }
            }
            else
            {
                if (TF2_GetPlayerClass(client) == TFClass_Engineer &&
                    CountObjects(client,TF2_ObjectClassNames[item]) <= 0)
                {
                    new TFObjectType:object;
                    if (item == _:TFExtObject_MiniSentry)
                        object = TFObject_Sentry;
                    else if (item == _:TFExtObject_Amplifier)
                    {
                        object = TFObject_Dispenser;
                        if (m_AmpNodeAvailable)
                        {
                            SetAmplifier(client, .enable=true);
                        }
                    }
                    else if (item == _:TFExtObject_RepairNode)
                    {
                        object = TFObject_Dispenser;
                        if (m_AmpNodeAvailable)
                        {
                            new spawn_level=GetUpgradeLevel(client,m_raceId,m_upgradeId);
                            SetRepairNode(client, .enable=true,
                                          .team=(spawn_level>2));
                        }
                    }
                    else
                        object = TFObjectType:item;

                    ClientCommand(client, "build %d", object);
                }
                else
                {
                    new spawn_level=GetUpgradeLevel(client,m_raceId,m_upgradeId);
                    SpawnIt(client, TFExtObjectType:item, m_raceId, m_upgradeId,
                            spawn_level, false, false, m_SpawnSound);
                }
            }
        }
    }
    else if (action == MenuAction_End)
        CloseHandle(menu);
}

stock SpawnIt(client, TFExtObjectType:type, raceId, upgradeId, level,
              bool:bMini, bool:bShielded, const String:spawnSound[])
{
    new count;
    new energy = GetEnergy(client);
    new vespene = GetVespene(client);
    new energy_cost = GetUpgradeEnergy(raceId,upgradeId);
    new vespene_cost = GetUpgradeVespene(raceId,upgradeId);

    if (!IsPlayerAlive(client))
    {
        PrepareSound(deniedWav);
        EmitSoundToClient(client,deniedWav);

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
    else if (IsBurrowed(client))
    {
        PrepareSound(deniedWav);
        EmitSoundToClient(client,deniedWav);

        decl String:upgradeName[64];
        GetUpgradeName(raceId, upgradeId, upgradeName, sizeof(upgradeName), client);
        DisplayMessage(client, Display_Ultimate, "%t", "NotWhileBurrowed", upgradeName);
    }
    else if (IsMole(client))
    {
        PrepareSound(deniedWav);
        EmitSoundToClient(client,deniedWav);

        decl String:upgradeName[64];
        GetUpgradeName(raceId, upgradeId, upgradeName, sizeof(upgradeName), client);
        DisplayMessage(client, Display_Ultimate, "%t", "NotAsMole", upgradeName);
    }
    else if (!CheckBuild(client, TFObjectType:type, -1, count))
    {
        PrepareSound(deniedWav);
        EmitSoundToClient(client,deniedWav);
        DisplayMessage(client, Display_Ultimate,
                       "%t", "StructureLimitReached",
                       count, TF2_ObjectNames[type]);
    }
    else if (HasCooldownExpired(client, raceId, upgradeId))
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
                    return;
                }
                else if (TF2_IsDisguised(pcond))
                    TF2_RemovePlayerDisguise(client);
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

        m_raceId = raceId;
        m_upgradeId = upgradeId;
        strcopy(m_SpawnSound, sizeof(m_SpawnSound), spawnSound);
        new object = BuildObject(client, type, (bMini || type == TFExtObject_MiniSentry) ? 1 : level,
                                 bMini, bShielded, .bDisabled=true, .check=false);
        if (object > 0)
        {
            //if (m_GravgunAvailable)
            //    DropEntity(object);

            PrepareSound(m_SpawnSound);
            EmitSoundToAll(m_SpawnSound,client);

            SetEnergy(client, energy-energy_cost);
            SetVespene(client, vespene-vespene_cost);

            new spawn_level=GetUpgradeLevel(client,raceId,upgradeId);
            new num_built = CountObjects(client, TF2_ObjectClassNames[type]);
            new Float:cooldown = GetUpgradeCooldown(raceId, upgradeId) * float(4-spawn_level+((num_built > 1) ? num_built * 2 : 1));
            CreateCooldown(client, raceId, upgradeId, cooldown);

            if (m_AmpNodeAvailable)
            {
                if (type == TFExtObject_Amplifier)
                {
                    ConvertToAmplifier(object, client, TFCond_Buffed, m_SpawnAmpRange[level]);
                }
                else if (type == TFExtObject_RepairNode)
                {
                    ConvertToRepairNode(object, client, m_SpawnNodeRange[level],
                                        m_SpawnNodeRegen[level], m_SpawnNodeShells[level],
                                        m_SpawnNodeRockets[level], .team=(spawn_level>2));
                }
            }
        }
    }
}

public Action:OnBuildCommand(client,TFObjectType:type,mode,count)
{
    if (GetRace(client) == m_raceId && IsClientInGame(client) &&
        TF2_GetPlayerClass(client) == TFClass_Engineer)
    {
        // always allow 2 teleporters or 1 of everything else without cost
        new max = (type == TFObject_Teleporter) ? 1 : 0;
        if (count > max)
        {
            new energy = GetEnergy(client);
            new vespene = GetVespene(client);
            new energy_cost = GetUpgradeEnergy(m_raceId,m_upgradeId);
            new vespene_cost = GetUpgradeVespene(m_raceId,m_upgradeId);
            if (energy < energy_cost)
            {
                decl String:upgradeName[64];
                GetUpgradeName(m_raceId, m_upgradeId, upgradeName, sizeof(upgradeName), client);
                DisplayMessage(client, Display_Energy, "%t", "InsufficientEnergyFor", upgradeName, energy_cost);
                EmitEnergySoundToClient(client,GetRaceFaction(m_raceId));
                return Plugin_Handled;
            }
            else if (vespene < vespene_cost)
            {
                decl String:upgradeName[64];
                GetUpgradeName(m_raceId, m_upgradeId, upgradeName, sizeof(upgradeName), client);
                DisplayMessage(client, Display_Energy, "%t", "InsufficientVespeneFor", upgradeName, vespene_cost);
                EmitVespeneSoundToClient(client,GetRaceFaction(m_raceId));
                return Plugin_Handled;
            }
            else if (!HasCooldownExpired(client, m_raceId, m_upgradeId))
                return Plugin_Handled;
            else
            {
                SetEnergy(client, energy-energy_cost);
                SetVespene(client, vespene-vespene_cost);

                PrepareSound(m_SpawnSound);
                EmitSoundToAll(m_SpawnSound,client);

                CreateCooldown(client, m_raceId, m_upgradeId);
            }
        }

        // Make sure to turn off Amplifiers after (possibly) building one!
        if (m_AmpNodeAvailable && type == TFObject_Dispenser)
        {
            CreateTimer(5.0,ResetAmplifier,GetClientUserId(client),
                        TIMER_FLAG_NO_MAPCHANGE);
        }
    }

    return Plugin_Continue;
}

public Action:ResetAmplifier(Handle:timer,any:userid)
{
    new client = GetClientOfUserId(userid);
    if (client > 0 && m_AmpNodeAvailable)
        SetAmplifier(client, .enable=false);

    return Plugin_Stop;
}

public Action:OnAmplify(builder,client,TFCond:condition)
{
    if (condition == TFCond_Buffed && builder > 0 && GetRace(builder) == m_raceId)
    {
        new energy = GetEnergy(client);
        if (energy < 4)
            return Plugin_Stop;
        else
            SetEnergy(client, energy-4);
    }

    return Plugin_Continue;
}
