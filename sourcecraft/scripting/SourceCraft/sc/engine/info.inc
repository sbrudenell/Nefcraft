/**
 * vim: set ai et ts=4 sw=4 syntax=sourcepawn :
 * File: info.inc
 * Description: Display information about a player
 * Author(s): -=|JFH|=-Naris (Murray Wilson)
 */

#if defined _info_included
 #endinput
#endif
#define _info_included

#include "sc/engine/menumode"

new MenuMode:gPlayerInfoMenuMode[MAXPLAYERS+1];
new MenuMode:gUpgradesMenuMode[MAXPLAYERS+1];
new gUpgradesInfoTarget[MAXPLAYERS+1];
new gRaceForUpgrades[MAXPLAYERS+1];

PlayerInfoMenu(client,MenuMode:mode)
{
    if (IsValidClient(client) && !IsFakeClient(client))
    {
        new Handle:playerMenu=CreateMenu(View_Selected);
        SetMenuTitle(playerMenu,"[SC] %T", "ViewPlayer", client);
        SetMenuExitButton(playerMenu,true);

        if (mode & EnableMenuExitBack)
            SetMenuExitBackButton(playerMenu,true);

        //AddTargetsToMenu(playerMenu, client, true, false);
        AddTargetsToMenu2(playerMenu, client, COMMAND_FILTER_NO_IMMUNITY);
        gPlayerInfoMenuMode[client] = mode|InfoMode;
        DisplayMenu(playerMenu,client,MENU_TIME_FOREVER);
    }
}

public View_Selected(Handle:menu,MenuAction:action,client,selection)
{
    if (action == MenuAction_Select)
    {
        new userid, target;
        decl String:SelectionInfo[SMALL_STRING_LENGTH];
        EmitSoundToClient(client,buttonWav);
        GetMenuItem(menu,selection,SelectionInfo,sizeof(SelectionInfo));
        userid=StringToInt(SelectionInfo);
        if ((target = GetClientOfUserId(userid)) == 0)
            CPrintToChat(client, "{green}[SC]{default} %t", "NoPlayer");
        else
            PlayerInfo(client,target,gPlayerInfoMenuMode[client]);
    }
    else if (action == MenuAction_Cancel)
    {
        if (selection == MenuCancel_ExitBack)
            PlayerInfoMenu(client,gPlayerInfoMenuMode[client]);
    }
    else if (action == MenuAction_End)
        CloseHandle(menu);
}

public PlayerInfo(client,target,MenuMode:mode)
{
    if (IsValidClient(client) && !IsFakeClient(client) &&
        IsValidClient(target))
    {
        decl String:title[LONG_STRING_LENGTH];
        new race = GetRace(target);
        if (race >= 0)
        {
            new Handle:raceHandle=GetRaceHandle(race);

            decl String:raceName[NAME_STRING_LENGTH];
            GetRaceName(raceHandle, raceName, sizeof(raceName));

            new Faction:techId = GetRaceFactionId(raceHandle);
            new UnitType:typeId = GetRaceType(raceHandle);
            if (techId > Generic)
            {
                decl String:techName[NAME_STRING_LENGTH];
                GetFactionName(techId, techName, sizeof(techName));

                decl String:unitType[NAME_STRING_LENGTH];
                GetUnitTypeName(typeId, unitType, sizeof(unitType));
                Format(title,sizeof(title),"%s: %s {%s}",
                       techName, raceName, unitType);

                new len=strlen(title);
                Format(title[len], sizeof(title)-len,"\n%T", "PlayerTechLevel", client,
                       techName, GetTechLevel(target,techId));
            }
            else if (typeId > GenericUnit)
            {
                decl String:unit[NAME_STRING_LENGTH];
                GetUnitTypeName(typeId, unit, sizeof(unit));
                Format(title,sizeof(title),"%s {%s}",
                       raceName, unit);
            }
            else
            {
                strcopy(title, sizeof(title), raceName);
            }

            new len      = strlen(title);
            new level    = GetLevel(target,race);
            new maxLevel = GetRaceMaxLevel(raceHandle);
            Format(title[len], sizeof(title)-len, "\n%T", "PlayerLevel", client,
                   level, maxLevel, GetOverallLevel(target));

            if (level<maxLevel)
            {
                len = strlen(title);
                Format(title[len], sizeof(title)-len, "\n%T", "PlayerExperience",
                       client, GetXP(target,race), ReqLevelXP(level+1));
            }
            else
            {
                len = strlen(title);
                Format(title[len], sizeof(title)-len, "\n%T", "PlayerExperienceMaxed",
                       client, GetXP(target,race));
            }

            len = strlen(title);
            Format(title[len], sizeof(title)-len, "\n%T", "PlayerAccumulatedEnergy",
                   client, GetEnergy(target), GetAccumulatedEnergy(target,race));
        }
        else
        {
            Format(title, sizeof(title), "\n%T", "PlayerEnergy",
                   client, GetEnergy(target));
        }

        new len = strlen(title);
        Format(title[len], sizeof(title)-len, "\n%T", "PlayerCrystals",
               client, GetCrystals(target));

        len = strlen(title);
        Format(title[len], sizeof(title)-len, "\n%T", "PlayerVespene",
               client, GetVespene(target));

        new Handle:playerInfo=CreateMenu(PlayerInfo_Select);
        SetMenuTitle(playerInfo, "[SC] %T", "PlayerInfo", client,
                     IsFakeClient(target) ? "[BOT] " : "",
                     target, title);

        SetMenuExitButton(playerInfo,true);

        if (mode & EnableMenuExitBack)
            SetMenuExitBackButton(playerInfo,true);

        decl String:buf[SMALL_STRING_LENGTH];
        Format(buf,sizeof(buf),"%d",target);

        if (client == target)
        {
            if (race > 0)
                AddMenuItemT(playerInfo,buf,"ViewUpgradeLevels", client);

            AddMenuItemT(playerInfo,buf,"ViewShopItems", client);
        }
        else
        {
            if (race > 0)
                AddMenuItemT(playerInfo,buf,"ViewPlayersUpgradeLevels", client);

            if (mode & AdminMode)
                AddMenuItemT(playerInfo,buf,"ViewPlayersShopItems", client);
        }

        AddMenuItem(playerInfo,"","",ITEMDRAW_SPACER);
        AddMenuItemT(playerInfo,buf,"BackToOptions", client);

        gPlayerInfoMenuMode[client] = mode;
        DisplayMenu(playerInfo,client,MENU_TIME_FOREVER);
    }
    else
    {
        CPrintToChat(client, "{green}[SC]{default} %t", "NoPlayer");

        if (mode & FromAdminMenu)
            AdminMenu(client,0);
        else if (mode & FromPlayerInfoMenu)
            PlayerInfoMenu(client,gPlayerInfoMenuMode[client]);
        else if (mode & FromMainMenu)
            MainMenu(client);
    }
}

public PlayerInfo_Select(Handle:menu,MenuAction:action,client,selection)
{
    if (action == MenuAction_Select)
    {
        decl String:SelectionInfo[SMALL_STRING_LENGTH];
        EmitSoundToClient(client,buttonWav);
        GetMenuItem(menu,selection,SelectionInfo,sizeof(SelectionInfo));
        new target=StringToInt(SelectionInfo);
        if (IsValidClient(target))
        {
            if (target == client || (gPlayerInfoMenuMode[client] & AdminMode))
            {
                switch(selection)
                {
                    case 0: // upgrade levels
                    {
                        UpgradesInfo(client,target,GetRace(target),
                                     gPlayerInfoMenuMode[client]|FromPlayerInfo);
                    }
                    case 1: // shop items
                    {
                        ShopMenu(client,target,
                                 gPlayerInfoMenuMode[client]|InfoMode|FromPlayerInfo);
                    }
                    default:
                    {
                        new MenuMode:mode = gPlayerInfoMenuMode[client];
                        if (mode & FromAdminMenu)
                            Admin_Player(client,target);
                        else if (mode & FromMainMenu)
                            MainMenu(client);
                    }
                }
            }
            else // Only option (for other players) is Upgrade Info for non-admins
            {
                new race = GetRace(target);
                if (race > 0)
                {
                    UpgradesInfo(client, target, race,
                                 gPlayerInfoMenuMode[client]|FromPlayerInfo);
                }
            }
        }
        else
        {
            CPrintToChat(client, "{green}[SC]{default} %t", "NoPlayer");

            new MenuMode:mode = gPlayerInfoMenuMode[client];
            if (mode & FromAdminMenu)
                AdminMenu(client,0);
            else if (mode & FromMainMenu)
                MainMenu(client);
        }
    }
    else if (action == MenuAction_Cancel)
    {
        if (selection == MenuCancel_ExitBack)
        {
            new MenuMode:mode = gPlayerInfoMenuMode[client];
            if (mode & FromAdminMenu)
                Admin_Return(menu,client);
            else if (mode & FromMainMenu)
                MainMenu(client);
        }
    }
    else if (action == MenuAction_End)
        CloseHandle(menu);
}

UpgradesInfo(client,target,race,MenuMode:mode)
{
    TraceInto("Info", "UpgradesInfo", "client=%d:%N, target=%d, race=%d, mode=%d", \
              client, client, target, race, mode);

    if (IsValidClient(client) && !IsFakeClient(client))
    {
        decl String:item[ITEM_STRING_LENGTH];
        decl String:buf[SMALL_STRING_LENGTH];

        new Handle:menu=CreateMenu(UpgradesInfo_Selected);
        SetMenuExitButton(menu,true);

        if (mode & EnableMenuExitBack)
            SetMenuExitBackButton(menu,true);

        new Handle:raceHandle=GetRaceHandle(race);

        decl String:raceName[NAME_STRING_LENGTH];
        GetRaceName(raceHandle, raceName, sizeof(raceName));

        if (client == target)
        {
            decl String:short[SHORT_STRING_LENGTH];
            GetRaceShortName(raceHandle, short, sizeof(short));

            decl String:unitType[NAME_STRING_LENGTH];
            GetUnitTypeName(GetRaceType(raceHandle), unitType,
                            sizeof(unitType), client);

            decl String:factionName[NAME_STRING_LENGTH];
            GetFactionName(GetRaceFaction(raceHandle), factionName,
                           sizeof(factionName), client);

            SetMenuTitle(menu,"[SC] %T", "RaceInfo", client, raceName, short,
                         GetRaceMaxLevel(raceHandle), factionName, unitType);
        }
        else
        {
            SetMenuTitle(menu,"[SC] %T", "RaceInfoFor", client,
                         raceName, IsFakeClient(target) ? "[BOT] " : "", target);
        }

        if (mode & ChangeRaceMode)
        {
            AddMenuItemT(menu,"-1","ChangeToRace",client);
            AddMenuItem(menu,"","",ITEMDRAW_SPACER);
        }

        new maxLevel = GetRaceMaxLevel(raceHandle);
        new upgradeCount = GetUpgradeCount(raceHandle);
        for(new upgrade=0;upgrade<upgradeCount;upgrade++)
        {
            new upgradeCategory = GetUpgradeCategory(raceHandle, upgrade);
            if (upgradeCategory > 1)
            {
                Format(item, sizeof(item), "%T:", "UltimateN",
                       client, upgradeCategory);
            }
            else if (upgradeCategory == 1)
                Format(item, sizeof(item), "%T:", "Ultimate", client);
            else
                item[0] = 0;

            decl String:upgradeName[NAME_STRING_LENGTH];
            GetUpgradeName(raceHandle, upgrade, upgradeName, sizeof(upgradeName), client);
            StrCat(item, sizeof(item), upgradeName);

            new maxUpgradeLevel = GetUpgradeMaxLevel(raceHandle,upgrade);
            new requiredLevel = GetUpgradeRequiredLevel(raceHandle,upgrade);
            if (requiredLevel > maxLevel || maxUpgradeLevel <= 0)
            {
                new len=strlen(item);
                Format(item[len], sizeof(item)-len," [%T]", "NotAvailable", client);
            }
            else if (requiredLevel < 1)
            {
                new len=strlen(item);
                Format(item[len], sizeof(item)-len," [%T]", "AlwaysAvailable", client);
            }
            else if (requiredLevel > 1)
            {
                new len=strlen(item);
                Format(item[len], sizeof(item)-len," [%T]", "MinimumLevel", client, requiredLevel);
            }

            new upgradeLevel;
            if (mode & (InfoMode|QueryMode))
            {
                upgradeLevel=GetUpgradeLevel(target,race,upgrade);
                if (upgradeLevel > 0)
                {
                    new len=strlen(item);
                    Format(item[len], sizeof(item)-len," (%T)", "IsLevel", client, upgradeLevel);
                }
            }
            else
                upgradeLevel = 0;

            if ((mode & QueryMode) == NoMode || upgradeLevel > 0 ||
                requiredLevel < 1 || upgradeCount < 8)
            {
                Format(buf,sizeof(buf),"%d",upgrade);
                AddMenuItem(menu,buf,item);
            }
        }

        gRaceForUpgrades[client] = race;
        gUpgradesMenuMode[client] = mode;
        gUpgradesInfoTarget[client]=target;
        DisplayMenu(menu,client,MENU_TIME_FOREVER);
    }

    TraceReturn();
}

public UpgradesInfo_Selected(Handle:menu,MenuAction:action,client,selection)
{
    if(action==MenuAction_Select)
    {
        EmitSoundToClient(client,buttonWav);
        new target = gUpgradesInfoTarget[client];
        if (IsValidClient(target))
        {
            decl String:SelectionInfo[SMALL_STRING_LENGTH];
            EmitSoundToClient(client,buttonWav);
            GetMenuItem(menu,selection,SelectionInfo,sizeof(SelectionInfo));
            if (SelectionInfo[0])
            {
                new race=gRaceForUpgrades[client];
                new Handle:raceHandle=GetRaceHandle(race);
                new upgrade=StringToInt(SelectionInfo);
                if (upgrade < 0) // They decided to change to this race.
                {
                    ChangeRace(target, race, raceHandle, false, true);
                    if (gUpgradesMenuMode[client] & FromMainMenu)
                        MainMenu(client);
                }
                else
                    UpgradesInfo_Upgrade(client,raceHandle,upgrade);
            }
        }
        else if (target != client)
        {
            CPrintToChat(client, "{green}[SC]{default} %t", "NoPlayer");
        }
    }
    else if (action == MenuAction_Cancel)
    {
        if (selection == MenuCancel_ExitBack)
        {
            new MenuMode:mode = gUpgradesMenuMode[client];
            if (mode & FromPlayerInfo)
                PlayerInfo(client,gUpgradesInfoTarget[client], mode);
            else if (mode & FromAdminMenu)
                AdminMenu(client,gUpgradesInfoTarget[client]);
            else if (mode & FromFactionMenu)
                FactionMenu(client);
            else if (mode & FromRaceMenu)
                RaceMenu(client,mode);
            else if (mode & FromMainMenu)
                MainMenu(client);
        }
    }
    else if (action == MenuAction_End)
        CloseHandle(menu);
}

UpgradesInfo_Upgrade(client,Handle:raceHandle,upgrade)
{
    decl String:name[NAME_STRING_LENGTH];
    decl String:desc[LONG_STRING_LENGTH];
    decl String:item[LONG_STRING_LENGTH];

    new Handle:menu=CreateMenu(UpgradesInfo_Back);
    SetMenuExitButton(menu,true);
    SetMenuExitBackButton(menu,true);

    GetUpgradeNameAndDesc(raceHandle, upgrade, name, sizeof(name), desc, sizeof(desc), client);

    SetMenuTitle(menu,"[SC] %T", "UpgradeInfo", client, name);
    AddMenuItem(menu,"",desc,ITEMDRAW_DISABLED);

    new energy = GetUpgradeEnergy(raceHandle, upgrade);
    new recurring_energy = GetUpgradeRecurringEnergy(raceHandle, upgrade);
    if (recurring_energy > 0)
    {
        if (energy > 0)
        {
            Format(item, sizeof(item), "%T", "UsesEnergyToInvoke", client, energy);
            AddMenuItem(menu,"",item,ITEMDRAW_DISABLED);

            Format(item, sizeof(item), "%T", "AndEnergyPerUse", client, recurring_energy);
            AddMenuItem(menu,"",item,ITEMDRAW_DISABLED);
        }
        else
        {
            Format(item,sizeof(item), "%T", "EnergyPerUse", client, recurring_energy);
            AddMenuItem(menu,"",item,ITEMDRAW_DISABLED);
        }
    }
    else if (energy > 0)
    {
        Format(item,sizeof(item), "%T", "UsesEnergy", client, energy);
        AddMenuItem(menu,"",item,ITEMDRAW_DISABLED);
    }

    new cost = GetUpgradeCostFlag(raceHandle, upgrade);
    new vespene = GetUpgradeVespene(raceHandle, upgrade);
    if (vespene > 0)
    {
        switch (cost)
        {
            case COST_PER_USE:     Format(item, sizeof(item), "%T", "CostsVespenePerUse", client, vespene);
            case COST_TO_PURCHASE: Format(item, sizeof(item), "%T", "CostsVespeneToUpgrade", client, vespene);
            default:               Format(item, sizeof(item), "%T", "CostsVespeneToUpgradeRefundable", client, vespene);
        }
        AddMenuItem(menu,"",item,ITEMDRAW_DISABLED);
    }

    new crystals = GetUpgradeCrystals(raceHandle, upgrade);
    if (crystals > 0)
    {
        switch (cost)
        {
            case COST_PER_USE:     Format(item,sizeof(item), "%T", "CostsCrystalsPerUse", crystals);
            case COST_TO_PURCHASE: Format(item,sizeof(item), "%T", "CostsCrystalsToUpgrade", client, crystals);
            default:               Format(item,sizeof(item), "%T", "CostsCrystalsToUpgradeRefundable", crystals);
        }
        AddMenuItem(menu,"",item,ITEMDRAW_DISABLED);
    }

    new category = GetUpgradeCategory(raceHandle, upgrade);
    if (category == 1)
        AddMenuItemT(menu, "", "InvokedWithUltimate", client, ITEMDRAW_DISABLED);
    else if (category > 1)
    {
        Format(item,sizeof(item), "%T", "InvokedWithUltimateN", client, category);
        AddMenuItem(menu,"",item,ITEMDRAW_DISABLED);
    }

    if (GetUpgradeRequiredLevel(raceHandle,upgrade) == 0)
        AddMenuItemT(menu, "", "AlwaysAvailable", client, ITEMDRAW_DISABLED);

    AddMenuItem(menu, "", "", ITEMDRAW_SPACER);
    AddMenuItemT(menu, "", "BackToUpgrades", client);
    DisplayMenu(menu,client,MENU_TIME_FOREVER);
}

public UpgradesInfo_Back(Handle:menu,MenuAction:action,client,selection)
{
    if(action==MenuAction_Select)
    {
        EmitSoundToClient(client,buttonWav);
        UpgradesInfo(client,gUpgradesInfoTarget[client],
                     gRaceForUpgrades[client],
                     gUpgradesMenuMode[client]);
    }
    else if (action == MenuAction_Cancel)
    {
        if (selection == MenuCancel_ExitBack)
        {
            EmitSoundToClient(client,buttonWav);
            UpgradesInfo(client,gUpgradesInfoTarget[client],
                         gRaceForUpgrades[client],
                         gUpgradesMenuMode[client]);
        }
    }
    else if (action == MenuAction_End)
        CloseHandle(menu);
}
