/*
 * vim: set ai et ts=4 sw=4 syntax=sourcepawn :
 * File: menus.inc
 * Description: This file handles most of the menus.
 * Author: Naris (Murray Wilson) 
 * Credits: Anthony Iacono 
 */

new MenuMode:gShopMenuMode[MAXPLAYERS+1];
new MenuMode:gRaceMenuMode[MAXPLAYERS+1];
new MenuMode:gUpgradeMenuMode[MAXPLAYERS+1];
new gFactionMenuRace[MAXPLAYERS+1];
new gUpgradeMenuRace[MAXPLAYERS+1];
new gShopMenuTarget[MAXPLAYERS+1];

IntroMenu(client)
{
    if (IsValidClient(client) && !IsFakeClient(client))
    {
        new Handle:introMenu=CreateMenu(IntroMenu_Action);
        SetMenuExitButton(introMenu,false);
        if (GetRaceCount() > 1)
        {
            if (GetShopItemCount() >= 1)
            {
                SetMenuTitle(introMenu,"%T", "WelcomeToSourceCraft", client, SOURCECRAFT_VERSION);
            }
            else
            {
                SetMenuTitle(introMenu,"%T", "WelcomeToSourceCraftNoShop", client, SOURCECRAFT_VERSION);
            }
        }
        else
        {
            SetMenuTitle(introMenu,"%T", "WelcomeToShopMenu", client, SOURCECRAFT_VERSION);
        }

        SetMenuExitButton(introMenu,true);
        AddMenuItem(introMenu,"","Exit");

        DisplayMenu(introMenu,client, 20);
    }
}

public IntroMenu_Action(Handle:menu,MenuAction:action,client,param)
{
    if (action == MenuAction_Select ||
       (action == MenuAction_Cancel
        && (param == MenuCancel_Timeout ||
            param == MenuCancel_Exit)))
    {
        if (GetRaceCount() > 1 &&
            GetDatabaseLoaded(client) == DataNotFound)
        {
            RaceMenu(client, ChangeRaceMode);
        }

        m_FirstSpawn[client]=false;
    }
    else if (action == MenuAction_End)
        CloseHandle(menu);
}

MainMenu(client)
{
    if (IsValidClient(client) && !IsFakeClient(client))
    {
        new Handle:mainMenu=CreateMenu(Menu_Select);
        SetMenuExitButton(mainMenu,true);
        SetMenuTitle(mainMenu,"[SC] %T", "ChooseTask", client);

        new bool:shopAvailable = (GetShopItemCount() >= 1);
        if (shopAvailable)
        {
            AddMenuItemT(mainMenu,"0","ShowShopMenu",client);
        }

        if (GetRaceCount() > 1)
        {
            AddMenuItemT(mainMenu,"1","ChangeRace",client);
            AddMenuItemT(mainMenu,"2","ShowUpgradeLevels",client);
            AddMenuItemT(mainMenu,"3","ResetUpgrades",client);
            AddMenuItemT(mainMenu,"4","SpendPoints",client);
            AddMenuItemT(mainMenu,"5","ViewUpgrades",client);
        }

        AddMenuItemT(mainMenu,"6","ChangeSettings",client);

        if (shopAvailable)
        {
            AddMenuItemT(mainMenu,"7","ShowItems",client);
        }

        AddMenuItemT(mainMenu,"8","ViewCommands",client);
        AddMenuItemT(mainMenu,"9","ViewOtherPlayer",client);
        AddMenuItemT(mainMenu,"10","ViewSCInfo",client);
        AddMenuItemT(mainMenu,"11","ViewWiki",client);
        AddMenuItemT(mainMenu,"12","ViewUpdates",client);

        DisplayMenu(mainMenu,client,MENU_TIME_FOREVER);
    }
}

public Menu_Select(Handle:menu,MenuAction:action,client,selection)
{
    TraceInto("Menus", "Menu_Select", "client=%d:%N, menu=%d, action=%d, selection=%d", \
              client, ValidClientIndex(client), menu, action, selection);

    if (action == MenuAction_Select)
    {
        EmitSoundToClient(client,buttonWav);

        decl String:SelectionInfo[SMALL_STRING_LENGTH];
        GetMenuItem(menu,selection,SelectionInfo,sizeof(SelectionInfo));
        switch(StringToInt(SelectionInfo))
        {
            case 0: ShopMenu(client,client,PurchaseItemMode|FromMainMenu);
            case 1: RaceMenu(client,ChangeRaceMode|FromMainMenu);
            case 2: UpgradesInfo(client,client,GetRace(client),InfoMode|FromMainMenu); // showupgrades
            case 3: // reset
            {
                ResetUpgrades(client);
                MainMenu(client);
            }
            case 4: // upgrade
            {
                new race=GetRace(client);
                if (GetUpgradeLevelCount(client,race)<GetLevel(client,race))
                    UpgradeMenu(client, UpgradeMode|FromMainMenu);
                else
                {
                    CPrintToChat(client, "{green}[SC]{default} %t", "NoPoints");
                    MainMenu(client);
                }
            }
            case 5: UpgradesInfo(client,client,GetRace(client),FromMainMenu); // upgradeinfo
            case 6: ChangeSettings(client,true);
            case 7: ShopMenu(client,client,InfoMode|FromMainMenu);
            case 8: Help(client);
            case 9: PlayerInfoMenu(client,FromMainMenu);
            case 10: ShowInfo(client);
            case 11: ShowWiki(client);
            case 12: ShowUpdates(client);
        }
    }
    else if (action == MenuAction_End)
        CloseHandle(menu);

    TraceReturn();
}

RaceMenu(client,MenuMode:mode)
{
    if (IsValidClient(client) && !IsFakeClient(client))
    {
        new indexCount = GetRaceIndexCount();
        if (indexCount > 1)
        {
            new bool:change   = bool:(mode & ChangeRaceMode);
            new Handle:crMenu = CreateMenu(Faction_Selected);

            if (change)
                SetMenuTitle(crMenu, "[SC] %T", "ChooseRace", client, "");
            else
                SetMenuTitle(crMenu, "[SC] %T", "ViewRace", client, "");

            SetMenuExitButton(crMenu,true);
            if (mode & EnableMenuExitBack)
                SetMenuExitBackButton(crMenu,true);

            new level = GetOverallLevel(client);
            new Faction:lastFaction = Unassigned;
            for(new index=1; index < indexCount; index++)
            {
                new Handle:raceHandle = GetRaceIndexHandle(index);
                new Faction:faction   = GetRaceFaction(raceHandle);
                if (faction != lastFaction)
                {
                    lastFaction = faction;
                    if (index < indexCount)
                    {
                        new Handle:nextHandle   = GetRaceIndexHandle(index+1);
                        new Faction:nextFaction = GetRaceFaction(nextHandle);
                        if (nextFaction != faction)
                        {
                            decl String:item[ITEM_STRING_LENGTH];
                            new reqLevel = GetRaceRequiredLevel(raceHandle);
                            new bool:disable = (level < reqLevel || reqLevel < 0);
                            GetRaceMenuItem(item, sizeof(item), client, raceHandle, reqLevel, disable);
                            if (item[0] != 0)
                            {
                                decl String:rbuf[SMALL_STRING_LENGTH];
                                Format(rbuf,sizeof(rbuf),"%d",GetRaceId(raceHandle));
                                new style = (change && disable) ? ITEMDRAW_DISABLED : ITEMDRAW_DEFAULT;
                                AddMenuItem(crMenu,rbuf,item,style);
                                continue;
                            }
                        }
                    }

                    decl String:name[NAME_STRING_LENGTH];
                    GetFactionName(faction,name,sizeof(name), client);
                    if (name[0] == '\0')
                        Format(name, sizeof(name), "%T", "Miscellaneous", client);

                    decl String:item[ITEM_STRING_LENGTH];
                    Format(item, sizeof(item), "%T", "FactionItem", client, name);

                    decl String:rbuf[SMALL_STRING_LENGTH];
                    Format(rbuf,sizeof(rbuf),"%d",-index);
                    AddMenuItem(crMenu,rbuf,item,ITEMDRAW_DEFAULT);
                }
            }
            gRaceMenuMode[client]=mode;
            DisplayMenu(crMenu,client,MENU_TIME_FOREVER);
        }
        else
        {
            CPrintToChat(client, "{green}[SC]{default} %t", "NoRaces");

            if (mode & FromMainMenu)
                MainMenu(client);
        }
    }
}

public Faction_Selected(Handle:menu,MenuAction:action,client,selection)
{
    if (action == MenuAction_Select)
    {
        decl String:SelectionInfo[SMALL_STRING_LENGTH];
        GetMenuItem(menu,selection,SelectionInfo,sizeof(SelectionInfo));
        new sel = StringToInt(SelectionInfo);
        if (sel < 0)
            FactionMenu(client, -sel);
        else            
            UpgradesInfo(client,client,sel,gRaceMenuMode[client]|FromRaceMenu);
    }
    else if (action == MenuAction_Cancel)
    {
        if (selection == MenuCancel_ExitBack)
        {
            EmitSoundToClient(client,buttonWav);
            RaceMenu(client,gRaceMenuMode[client]);
        }
    }
    else if (action == MenuAction_End)
        CloseHandle(menu);
}

FactionMenu(client, start=-1)
{
    if (start < 0)
        start = gFactionMenuRace[client];
    else
        gFactionMenuRace[client]=start;

    new indexCount = GetRaceIndexCount();
    if (start > 0 && start < indexCount)
    {
        new bool:change        = bool:(gRaceMenuMode[client] & ChangeRaceMode);
        new Handle:crMenu      = CreateMenu(RaceMenu_Selected);
        new Faction:theFaction = GetRaceFaction(GetRaceIndexHandle(start));

        decl String:factionName[NAME_STRING_LENGTH];
        GetFactionName(theFaction,factionName,sizeof(factionName), client);

        if (change)
            SetMenuTitle(crMenu, "[SC] %T", "ChooseRace", client, factionName);
        else
            SetMenuTitle(crMenu, "[SC] %T", "ViewRace", client, factionName);

        SetMenuExitButton(crMenu,true);
        SetMenuExitBackButton(crMenu,true);

        new level = GetOverallLevel(client);
        for(new index = start; index < indexCount; index++)
        {
            new Handle:raceHandle=GetRaceIndexHandle(index);
            if (GetRaceFaction(raceHandle) != theFaction)
                break;
            else
            {
                decl String:item[ITEM_STRING_LENGTH];
                new reqLevel = GetRaceRequiredLevel(raceHandle);
                new bool:disable = (level < reqLevel || reqLevel < 0);
                GetRaceMenuItem(item, sizeof(item), client, raceHandle, reqLevel, disable);
                if (item[0] != 0)
                {
                    decl String:rbuf[SMALL_STRING_LENGTH];
                    Format(rbuf,sizeof(rbuf),"%d",GetRaceId(raceHandle));
                    new style = (change && disable) ? ITEMDRAW_DISABLED : ITEMDRAW_DEFAULT;
                    AddMenuItem(crMenu,rbuf,item,style);
                }
            }
        }
        DisplayMenu(crMenu,client,MENU_TIME_FOREVER);
    }
}

GetRaceMenuItem(String:item[], maxlength, client, Handle:raceHandle, reqLevel, bool:disable)
{
    new  String:flag[3] = " ";
    new id = GetRaceId(raceHandle);
    if (id == GetRace(client))
        flag[0] = '>';
    else if (id == GetPendingRace(client))
        flag[0] = '<';

    decl String:name[NAME_STRING_LENGTH];
    GetRaceName(raceHandle,name,sizeof(name), client);
    if (name[0] == '\0')
        item[0] = '\0';
    else if (reqLevel > 0)
    {
        Format(item, maxlength, "%s%T", flag,
               "RaceRequiredLevel", client, name, reqLevel);
    }
    else
    {
        decl String:parent[NAME_STRING_LENGTH];
        GetRaceParentName(raceHandle,parent,sizeof(parent));
        if (parent[0] != '\0')
        {
            new pRace = FindRace(parent);
            if (pRace >= 0)
            {
                new Handle:pHandle = GetRaceHandle(pRace);
                if (pHandle != INVALID_HANDLE)
                    GetRaceName(pHandle,parent,sizeof(parent),client);
            }

            Format(item, maxlength, "%s%T", flag,
                   "RaceSummoned", client, name, parent);
        }
        else if (disable)
        {
            Format(item, maxlength, "%s%T", flag,
                   "RaceLocked", client, name);
        }
        else
        {
            Format(item, maxlength, "%s%s", flag, name);
        }
    }
}

public RaceMenu_Selected(Handle:menu,MenuAction:action,client,selection)
{
    if (action == MenuAction_Select)
    {
        decl String:SelectionInfo[SMALL_STRING_LENGTH];
        GetMenuItem(menu,selection,SelectionInfo,sizeof(SelectionInfo));
        if (SelectionInfo[0])
        {
            EmitSoundToClient(client,buttonWav);
            UpgradesInfo(client,client,StringToInt(SelectionInfo),gRaceMenuMode[client]|FromFactionMenu);
        }
    }
    else if (action == MenuAction_Cancel)
    {
        if (selection == MenuCancel_ExitBack)
        {
            EmitSoundToClient(client,buttonWav);
            RaceMenu(client,gRaceMenuMode[client]);
        }
    }
    else if (action == MenuAction_End)
        CloseHandle(menu);
}

UpgradeMenu(client, MenuMode:mode)
{
    TraceInto("Menus", "UpgradeMenu", "client=%d:%N, mode=%d", \
              client, ValidClientIndex(client), mode);

    if (IsValidClient(client) && !IsFakeClient(client))
    {
        new bool:fromMain = bool:(mode & FromMainMenu);
        if (GetRaceCount() <= 1)
        {
            CPrintToChat(client, "{green}[SC]{default} %t", "NoRacesOrUpgrades");

            if (fromMain)
                MainMenu(client);
        }
        else if (GetDatabaseLoaded(client) == DataLoading)
        {
            CPrintToChat(client, "{green}[SC]{default} %t", "StillLoadingUnableToUpgrade");

            if (fromMain)
                MainMenu(client);
        }
        else
        {
            new Handle:sMenu=CreateMenu(UpgradeMenu_Selected);
            new race=GetRace(client);
            new Handle:raceHandle=GetRaceHandle(race);
            new upgradecount=GetUpgradeLevelCount(client,race);
            new level=GetLevel(client,race);
            new vespene=GetVespene(client);
            new crystals=GetCrystals(client);
            SetMenuTitle(sMenu, "[SC] %T", "ChooseUpgrade", client,
                         upgradecount, level, crystals, vespene);

            SetMenuExitButton(sMenu,true);
            if (mode & EnableMenuExitBack)
                SetMenuExitBackButton(sMenu,true);

            new style, cost, vespene_cost;
            decl String:sbuf[SMALL_STRING_LENGTH];
            decl String:item[ITEM_STRING_LENGTH];
            new maxLevel = GetRaceMaxLevel(raceHandle);
            new raceLevel = GetLevel(client,race);

            new activeUltimate[MAX_ULTIMATE_CATEGORY+1];
            for (new i=1;i<sizeof(activeUltimate);i++)
                activeUltimate[i] = GetActiveUltimate(client,race,i);

            new activeItems = 0;
            new upgradeCount = GetUpgradeCount(raceHandle);
            for(new upgrade=0;upgrade<upgradeCount;upgrade++)
            {
                new upgradeCategory = GetUpgradeCategory(raceHandle, upgrade);
                if (upgradeCategory >= 1)
                {
                    if (upgradeCategory == 1)
                    {
                        Format(item, sizeof(item), "%T:", "Ultimate", client);
                    }
                    else
                    {
                        Format(item, sizeof(item), "%T:", "UltimateN",
                               client, upgradeCategory);
                    }

                    if (upgradeCategory <= MAX_ULTIMATE_CATEGORY &&
                        activeUltimate[upgradeCategory] >= 0 &&
                        activeUltimate[upgradeCategory] != upgrade)
                    {
                        style = ITEMDRAW_DISABLED;
                    }
                }
                else
                    item[0] = 0;

                decl String:upgradeName[NAME_STRING_LENGTH];
                GetUpgradeName(raceHandle, upgrade, upgradeName, sizeof(upgradeName), client);
                StrCat(item, sizeof(item), upgradeName);

                new maxUpgradeLevel=GetUpgradeMaxLevel(raceHandle,upgrade);
                new requiredLevel=GetUpgradeRequiredLevel(raceHandle,upgrade);
                new upgradeLevel=GetUpgradeLevel(client,race,upgrade);

                if (requiredLevel > maxLevel || maxUpgradeLevel <= 0)
                {
                    new len=strlen(item);
                    Format(item[len], sizeof(item)-len," [%T]", "NotAvailable", client);
                    style = ITEMDRAW_DISABLED;
                }
                else if (raceLevel < requiredLevel)
                {
                    new len=strlen(item);
                    Format(item[len], sizeof(item)-len," [%T]", "MinimumLevel", client, requiredLevel);
                    style = ITEMDRAW_DISABLED;
                }
                else if (upgradeLevel >= maxUpgradeLevel)
                    style = ITEMDRAW_DISABLED;
                else
                    style = ITEMDRAW_DEFAULT;

                if (GetUpgradeCostFlag(raceHandle,upgrade) >= COST_TO_PURCHASE)
                {
                    cost = GetUpgradeCrystals(raceHandle,upgrade);
                    vespene_cost = GetUpgradeVespene(raceHandle,upgrade);
                }
                else
                    cost = vespene_cost = 0;

                if (cost > 0)
                {
                    if (vespene_cost > 0)
                    {
                        new len=strlen(item);
                        Format(item[len], sizeof(item)-len, " {%T}",
                               "CostVespeneAndCrystals", client,
                               vespene_cost, cost);

                        if (vespene_cost > vespene)
                            style = ITEMDRAW_DISABLED;
                    }
                    else
                    {
                        new len=strlen(item);
                        Format(item[len], sizeof(item)-len, " {%T}",
                               "CostCrystals", client, cost);
                    }

                    if (cost > crystals)
                        style = ITEMDRAW_DISABLED;
                }
                else if (vespene_cost > 0)
                {
                    new len=strlen(item);
                    Format(item[len], sizeof(item)-len, " {%T}",
                           "CostVespene", client, vespene_cost);

                    if (vespene_cost > vespene)
                        style = ITEMDRAW_DISABLED;
                }

                if (style == ITEMDRAW_DEFAULT)
                {
                    activeItems++;
                    new len=strlen(item);
                    Format(item[len], sizeof(item)-len,
                           " (%T)", "UpgradeLevel",
                           client, upgradeLevel+1);
                }

                Format(sbuf,sizeof(sbuf),"%d",upgrade);
                AddMenuItem(sMenu,sbuf,item,style);
            }

            if (activeItems > 0)
            {
                gUpgradeMenuRace[client]=race;
                gUpgradeMenuMode[client]=mode;
                DisplayMenu(sMenu,client,MENU_TIME_FOREVER);
            }
            else
            {
                CPrintToChat(client, "{green}[SC]{default} %t", "NoUpgrades");
            }
        }
    }

    TraceReturn();
}

public UpgradeMenu_Selected(Handle:menu,MenuAction:action,client,selection)
{
    TraceInto("Menus", "UpgradeMenu_Selected", "client=%d, menu=%d, action=%d, selection=%d", \
              client, menu, action, selection);

    if (action == MenuAction_Select)
    {
        EmitSoundToClient(client,buttonWav);
        if (IsValidClient(client))
        {
            new race = gUpgradeMenuRace[client]; //GetRace(client);
            new Handle:raceHandle=GetRaceHandle(race);
            new upgradeCount=GetUpgradeCount(raceHandle);

            decl String:SelectionInfo[SMALL_STRING_LENGTH];
            GetMenuItem(menu,selection,SelectionInfo,sizeof(SelectionInfo));
            new upgrade=StringToInt(SelectionInfo);
            if (upgrade >= 0 && upgrade < upgradeCount)
            {
                EmitSoundToClient(client,buttonWav);
                // Okay, they selected a valid upgrade
                new level = GetLevel(client,race);
                new vespene=GetVespene(client);
                new crystals=GetCrystals(client);

                new vespene_cost, cost;
                if (GetUpgradeCostFlag(raceHandle,upgrade) >= COST_TO_PURCHASE)
                {
                    cost = GetUpgradeCrystals(raceHandle,upgrade);
                    vespene_cost = GetUpgradeVespene(raceHandle,upgrade);
                }
                else
                    cost = vespene_cost = 0;

                if (GetUpgradeLevelCount(client,race)>=level)
                {
                    CPrintToChat(client, "{green}[SC]{default} %t", "NeedMoreLevels");
                }
                else if (cost > 0 && crystals < cost)
                {
                    EmitCrystalSoundToClient(client, GetRaceFactionId(raceHandle));
                    CPrintToChat(client, "{green}[SC]{default} %t", "InsufficientCrystals", cost);
                }
                else if (vespene_cost > 0 && vespene < vespene_cost)
                {
                    EmitVespeneSoundToClient(client, GetRaceFactionId(raceHandle));
                    CPrintToChat(client, "{green}[SC]{default} %t", "InsufficientVespene", vespene_cost);
                }
                else
                {
                    new minLevel = GetUpgradeRequiredLevel(raceHandle,upgrade);
                    if (level < minLevel)
                    {
                        decl String:upgradeName[NAME_STRING_LENGTH];
                        GetUpgradeName(raceHandle, upgrade, upgradeName, sizeof(upgradeName), client);
                        CPrintToChat(client, "{green}[SC]{default} %t", "InsufficientLevels",
                                    minLevel, upgradeName);
                    }
                    else
                    {
                        new upgradeCategory = GetUpgradeCategory(raceHandle, upgrade);
                        new activeUltimate = GetActiveUltimate(client,race,upgradeCategory);
                        if (activeUltimate >= 0 && activeUltimate != upgrade)
                        {
                            decl String:activename[NAME_STRING_LENGTH];
                            GetUpgradeName(raceHandle, activeUltimate, activename, sizeof(activename), client);
                            CPrintToChat(client, "{green}[SC]{default} %t", "UltimateTaken", activename, upgradeCategory);
                        }
                        else
                        {
                            new maxLevel = GetUpgradeMaxLevel(raceHandle,upgrade);
                            if (!IncrementUpgradeLevel(client,race,upgrade,upgradeCategory,maxLevel))
                            {
                                decl String:upgradeName[NAME_STRING_LENGTH];
                                GetUpgradeName(raceHandle, upgrade, upgradeName, sizeof(upgradeName), client);
                                CPrintToChat(client, "{green}[SC]{default} %t", "UpgradeMaxed", upgradeName, maxLevel);
                            }
                            else
                            {
                                if (cost > 0)
                                    SetCrystals(client,crystals-cost);

                                if (vespene_cost > 0)
                                    SetVespene(client,vespene-vespene_cost);

                                decl String:upgradeName[NAME_STRING_LENGTH];
                                GetUpgradeName(raceHandle, upgrade, upgradeName, sizeof(upgradeName), client);
                                CPrintToChat(client, "{green}[SC]{default} %s is now level %d.",
                                             upgradeName, GetUpgradeLevel(client,race,upgrade));
                            }
                        }
                    }

                    if (GetUpgradeLevelCount(client,race)<GetLevel(client,race))
                        UpgradeMenu(client, gUpgradeMenuMode[client]);
                    else if (g_bSaveXP && !GetDatabaseSaved(client) &&
                             GetPlayerStatus(client) != PlayerDisabled &&
                             GetDatabaseLoaded(client) >= DataOK &&
                             GetRaceLoaded(client, race) >= DataOK)
                    {
                        if (g_bDatabaseConnected)
                            SavePlayerData(client);
                        else
                        {
                            LogError("Database not available to save %N's levels", client);
                            PrintHintText(client, "%t", "NoDatabaseForSave");
                        }
                    }
                }
            }
        }
    }
    else if (action == MenuAction_Cancel)
    {
        if (selection == MenuCancel_ExitBack)
        {
            EmitSoundToClient(client,buttonWav);
            MainMenu(client);
        }
    }
    else if (action == MenuAction_End)
        CloseHandle(menu);

    TraceReturn();
}

ShopMenu(client, target, MenuMode:mode)
{
    if (IsValidClient(client) && !IsFakeClient(client))
    {
        new shopItemCount = GetShopItemCount();
        if (shopItemCount >= 1)
        {
            new count = 0;
            new Handle:shopMenu;
            new Handle:itemHandle;
            decl String:itembuf[SMALL_STRING_LENGTH];

            if (client != target || (mode & InfoMode))
            {
                decl id;
                decl String:itemName[NAME_STRING_LENGTH];

                shopMenu = CreateMenu(ShopMenu_Selected);
                SetMenuTitle(shopMenu, "[SC] %T", "ListItems", client, target);

                for (new x=0;x<shopItemCount;x++)
                {
                    itemHandle=GetItemIndexHandle(x);
                    id=GetShopId(itemHandle);
                    if (GetOwnsItem(client,id))
                    {
                        count++;
                        Format(itembuf,sizeof(itembuf),"%d",id);
                        GetItemName(itemHandle, itemName, sizeof(itemName),client);
                        AddMenuItem(shopMenu,itembuf,itemName);
                    }
                }
            }
            else
            {
                decl String:lastCategory[NAME_STRING_LENGTH];
                decl String:itemCategory[NAME_STRING_LENGTH];

                mode    |= FromShopMenu;
                shopMenu = CreateMenu(Category_Selected);
                SetMenuTitle(shopMenu, "[SC] %T", (mode & PurchaseItemMode) ? "ShopMenu" : "ShopItems",
                             client, GetCrystals(client), GetVespene(client));

                lastCategory[0] = '\0';
                for (new x=0;x<shopItemCount;x++)
                {
                    itemHandle=GetItemIndexHandle(x);
                    GetItemCategory(itemHandle, itemCategory, sizeof(itemCategory),client);

                    if (strcmp(itemCategory, lastCategory, false) != 0)
                    {
                        if (itemCategory[0] == '\0')
                        {
                            Format(itemCategory,sizeof(itemCategory),
                                   "%T", "MiscellaneousItems", client);
                        }

                        Format(itembuf,sizeof(itembuf),"%d",x);
                        AddMenuItem(shopMenu,itembuf,itemCategory);
                        strcopy(lastCategory, sizeof(lastCategory), itemCategory);
                        count++;
                    }
                }
            }

            if (count == 0)
            {
                AddMenuItemT(shopMenu,"","NoInventory",client,ITEMDRAW_DISABLED);
            }

            SetMenuExitButton(shopMenu,true);
            if (mode & EnableMenuExitBack)
                SetMenuExitBackButton(shopMenu,true);

            gShopMenuMode[client]   = mode;
            gShopMenuTarget[client] = target;
            DisplayMenu(shopMenu,client,MENU_TIME_FOREVER);
        }
        else
        {
            CPrintToChat(client, "{green}[SC]{default} %t", "NoItems");
        }
    }
}

public Category_Selected(Handle:menu,MenuAction:action,client,selection)
{
    if (action == MenuAction_Select)
    {
        if (IsValidClient(client))
        {
            EmitSoundToClient(client,buttonWav);

            decl String:SelectionInfo[SMALL_STRING_LENGTH];
            GetMenuItem(menu,selection,SelectionInfo,sizeof(SelectionInfo));
            new index=StringToInt(SelectionInfo);
            new shopItemCount = GetShopIndexCount();
            if (index >= 0 && index < shopItemCount)
            {
                decl String:item[ITEM_STRING_LENGTH];
                decl String:theCategory[NAME_STRING_LENGTH];
                decl String:itemCategory[NAME_STRING_LENGTH];

                new MenuMode:mode     = gShopMenuMode[client];
                new Handle:shopMenu   = CreateMenu(ShopMenu_Selected);
                SetMenuTitle(shopMenu, "[SC] %T", (mode & PurchaseItemMode) ? "ShopMenu" : "ShopItems",
                             client, GetCrystals(client), GetVespene(client));

                GetItemCategory(GetItemIndexHandle(index), theCategory, sizeof(theCategory), client);

                new count = 0;
                for (new x=index;x<shopItemCount;x++)
                {
                    new Handle:itemHandle=GetItemIndexHandle(x);
                    GetItemCategory(itemHandle, itemCategory, sizeof(itemCategory),client);
                    if (strcmp(itemCategory,theCategory,false) != 0)
                        break;
                    else
                    {
                        new id=GetShopId(itemHandle);
                        decl String:itembuf[SMALL_STRING_LENGTH];
                        Format(itembuf,sizeof(itembuf),"%d",id);

                        decl String:itemName[NAME_STRING_LENGTH];
                        GetItemName(itemHandle, itemName, sizeof(itemName),client);
                        count++;

                        decl String:shortName[SHORT_STRING_LENGTH];
                        GetItemShortName(itemHandle, shortName, sizeof(shortName));

                        new cost = GetItemCrystals(itemHandle);
                        new vespene_cost = GetItemVespene(itemHandle);
                        if (vespene_cost)
                        {
                            Format(item,sizeof(item),"%s (%s) - %T", itemName, shortName,
                                   "CostCrystalsAndVespene", client, cost, vespene_cost);
                        }
                        else
                        {
                            Format(item,sizeof(item),"%s (%s) - %T", itemName, shortName,
                                   "CostCrystals", client, cost);
                        }
                        AddMenuItem(shopMenu,itembuf,item);
                    }
                }

                if (count > 0)
                {
                    SetMenuExitButton(shopMenu,true);
                    SetMenuExitBackButton(shopMenu,true);
                    DisplayMenu(shopMenu,client,MENU_TIME_FOREVER);
                }
                else
                {
                    CancelMenu(shopMenu);
                    CPrintToChat(client, "{green}[SC]{default} %t", "NoItems");
                    ShopMenu(client, gShopMenuTarget[client], gShopMenuMode[client]);
                }
            }
            else
            {
                CPrintToChat(client, "{green}[SC]{default} %t", "NoItems");
                ShopMenu(client, gShopMenuTarget[client], gShopMenuMode[client]);
            }
        }
    }
    else if (action == MenuAction_Cancel)
    {
        if (selection == MenuCancel_ExitBack)
        {
            EmitSoundToClient(client,buttonWav);
            new MenuMode:mode = gShopMenuMode[client];
            if (mode & FromPlayerInfo)
            {
                PlayerInfo(client,gShopMenuTarget[client],
                           gShopMenuMode[client]);
            }
            else if (mode & FromAdminMenu)
                AdminMenu(client,gShopMenuTarget[client]);
            else if (mode & FromMainMenu)
                MainMenu(client);
        }
    }
    else if (action == MenuAction_End)
        CloseHandle(menu);
}

public ShopMenu_Selected(Handle:menu,MenuAction:action,client,selection)
{
    if (action == MenuAction_Select)
    {
        if (IsValidClient(client))
        {
            EmitSoundToClient(client,buttonWav);

            decl String:SelectionInfo[SMALL_STRING_LENGTH];
            GetMenuItem(menu,selection,SelectionInfo,sizeof(SelectionInfo));

            new item=StringToInt(SelectionInfo);
            if (item >= 0 && item < GetShopItemCount())
            {
                EmitSoundToClient(client,buttonWav);
                ShopMenu_Item(client,item);
            }
        }
    }
    else if (action == MenuAction_Cancel)
    {
        if (selection == MenuCancel_ExitBack)
        {
            EmitSoundToClient(client,buttonWav);
            new MenuMode:mode = gShopMenuMode[client];
            if (mode & FromPlayerInfo)
            {
                PlayerInfo(client,gShopMenuTarget[client],
                           gShopMenuMode[client]);
            }
            else if (mode & FromShopMenu)
            {
                ShopMenu(client, gShopMenuTarget[client],
                         gShopMenuMode[client]);
            }
            else if (mode & FromAdminMenu)
                AdminMenu(client,gShopMenuTarget[client]);
            else if (mode & FromMainMenu)
                MainMenu(client);
        }
    }
    else if (action == MenuAction_End)
        CloseHandle(menu);
}

public ShopMenu_Item(client,item)
{
    new vespene=GetVespene(client);
    new crystals=GetCrystals(client);
    new Handle:shopMenu_Item=CreateMenu(ShopMenu_Item_Select);
    SetMenuExitButton(shopMenu_Item,true);
    SetMenuExitBackButton(shopMenu_Item,true);

    new Handle:itemHandle=GetShopItemHandle(item);

    decl String:itemName[NAME_STRING_LENGTH];
    GetItemName(itemHandle, itemName, sizeof(itemName),client);

    decl String:desc[LONG_STRING_LENGTH];
    GetItemDescription(itemHandle, desc, sizeof(desc));

    new cost = GetItemCrystals(itemHandle);
    new vespene_cost = GetItemVespene(itemHandle);
    SetMenuTitle(shopMenu_Item,"[SC] %T", "ShopItemTitle", client,
                 itemName, crystals, vespene, desc, cost, vespene_cost);

    if (gShopMenuMode[client] & PurchaseItemMode)
    {
        decl String:buf[SMALL_STRING_LENGTH];
        Format(buf,sizeof(buf),"%d",item);
        AddMenuItemT(shopMenu_Item,buf,"Purchase", client);
    }

    AddMenuItemT(shopMenu_Item,"","BackToItems", client);
    DisplayMenu(shopMenu_Item,client,MENU_TIME_FOREVER);
}

public ShopMenu_Item_Select(Handle:menu,MenuAction:action,client,selection)
{
    if (action == MenuAction_Select)
    {
        if (IsValidClient(client))
        {
            EmitSoundToClient(client,buttonWav);
            if (selection==0 && client == gShopMenuTarget[client] &&
                (gShopMenuMode[client] & PurchaseItemMode))
            {
                decl String:SelectionInfo[SMALL_STRING_LENGTH];
                GetMenuItem(menu,selection,SelectionInfo,sizeof(SelectionInfo));

                if (SelectionInfo[0] &&
                    PurchaseShopItem(client, StringToInt(SelectionInfo)))
                {
                    if (gShopMenuMode[client] & FromMainMenu)
                        MainMenu(client);
                }
                else
                    ShopMenu(client, gShopMenuTarget[client], gShopMenuMode[client]);
            }
            else
                ShopMenu(client, gShopMenuTarget[client], gShopMenuMode[client]);
        }
    }
    else if (action == MenuAction_Cancel)
    {
        if (selection == MenuCancel_ExitBack)
        {
            EmitSoundToClient(client,buttonWav);
            ShopMenu(client, gShopMenuTarget[client], gShopMenuMode[client]);
        }
    }
    else if (action == MenuAction_End)
        CloseHandle(menu);
}
