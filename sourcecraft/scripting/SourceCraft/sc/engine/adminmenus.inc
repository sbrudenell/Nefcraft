/*
 * vim: set ai et ts=4 sw=4 syntax=sourcepawn :
 * File: adminmenus.inc
 * Description: This file handles the admin menu.
 * Author(s): Naris (Murray Wilson)
 */

new Handle:hTopAdminMenu = INVALID_HANDLE;

InitAdminMenu()
{
    /* Account for late loading */
    new Handle:topmenu;
    if (LibraryExists("adminmenu") && ((topmenu = GetAdminTopMenu()) != INVALID_HANDLE))
    {
        OnAdminMenuReady(topmenu);
    }
    return true;
}

public OnLibraryRemoved(const String:name[])
{
    if (StrEqual(name, "adminmenu"))
        hTopAdminMenu = INVALID_HANDLE;
}

public OnAdminMenuReady(Handle:topmenu)
{
    /*************************************************************/
    /* Add Administer SourceCraft option to SourceMod Admin Menu */
    /*************************************************************/

    /* Block us from being called twice */
    if (topmenu != hTopAdminMenu)
    {
        /* Save the Handle */
        hTopAdminMenu = topmenu;

        /* Find the "Server Commands" category */
        new TopMenuObject:server_commands = FindTopMenuCategory(hTopAdminMenu, ADMINMENU_SERVERCOMMANDS);
        if (server_commands != INVALID_TOPMENUOBJECT)
        {
            AddToTopMenu(hTopAdminMenu, "sc_admin", TopMenuObject_Item, AdminTopMenu,
                         server_commands, "sc_admin", ADMFLAG_SOURCECRAFT);
        }
    }
}

public AdminTopMenu(Handle:topmenu, TopMenuAction:action, TopMenuObject:object_id, param, String:buffer[], maxlength)
{
    if (action == TopMenuAction_DisplayOption)
    {
        Format(buffer, maxlength, "%T", "AdministerMenu", param);
    }
    else if (action == TopMenuAction_SelectOption)
    {
        AdminMenu(param,0);
    }
}

public Action:AdminMenu(client,args)
{
    new Handle:adminMenu=CreateMenu(Admin_Selected);
    SetMenuExitButton(adminMenu,true);
    SetMenuTitle(adminMenu,"[SC] %T", "SelectPlayer", client);
    AddTargetsToMenu(adminMenu, client, true, false);
    DisplayMenu(adminMenu,client,MENU_TIME_FOREVER);
    return Plugin_Handled;
}

public Admin_Selected(Handle:menu,MenuAction:action,client,selection)
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
        else if (!CanUserTarget(client, target))
            CPrintToChat(client, "{green}[SC]{default} %t", "NoTarget");
        else
            Admin_Player(client,target);
    }
    else if (action == MenuAction_End)
        CloseHandle(menu);
}

Admin_Player(client,target)
{
    new Handle:adminMenu_Player=CreateMenu(Admin_Player_Select);
    SetMenuExitButton(adminMenu_Player,true);
    SetMenuExitBackButton(adminMenu_Player,true);
    SetMenuTitle(adminMenu_Player,"[SC] %T", "AdministerPlayer", client, target);

    decl String:buf[SMALL_STRING_LENGTH];
    Format(buf,sizeof(buf),"%d",target);

    new race_style = (GetRaceCount() >= 1) ? ITEMDRAW_DEFAULT : ITEMDRAW_DISABLED;
    new shop_style = (GetShopItemCount() >= 1) ? ITEMDRAW_DEFAULT : ITEMDRAW_DISABLED;
    AddMenuItemT(adminMenu_Player,buf,"ViewInfo", client);
    AddMenuItemT(adminMenu_Player,buf,"Reset", client, race_style);
    AddMenuItemT(adminMenu_Player,buf,"SetRace", client, race_style);
    AddMenuItemT(adminMenu_Player,buf,"GiveShopItem", client, shop_style);
    AddMenuItemT(adminMenu_Player,buf,"ChangeXP", client, race_style);
    AddMenuItemT(adminMenu_Player,buf,"ChangeLevel", client, race_style);
    AddMenuItemT(adminMenu_Player,buf,"ChangeCrystals", client);
    AddMenuItemT(adminMenu_Player,buf,"ChangeVespene", client);
    AddMenuItemT(adminMenu_Player,buf,"ChangeEnergy", client);

    if (GetPlayerStatus(target) == PlayerDisabled)
        AddMenuItemT(adminMenu_Player,buf,"EnablePlayer", client);
    else
        AddMenuItemT(adminMenu_Player,buf,"DisablePlayer", client);

    AddMenuItemT(adminMenu_Player,buf,"DeletePlayer", client);
    DisplayMenu(adminMenu_Player,client,MENU_TIME_FOREVER);
}

public Admin_Player_Select(Handle:menu,MenuAction:action,client,selection)
{
    if (action == MenuAction_Select)
    {
        EmitSoundToClient(client,buttonWav);

        decl String:SelectionInfo[SMALL_STRING_LENGTH];
        GetMenuItem(menu,selection,SelectionInfo,sizeof(SelectionInfo));

        new target = StringToInt(SelectionInfo);
        if (IsValidClient(target))
        {
            // What do they want to do with the player?
            switch(selection)
            {
                case 0: PlayerInfo(client,target, AdminMode|FromAdminMenu);
                case 1: Admin_Reset(client,target);
                case 2: Admin_SetRace(client,target);
                case 3: Admin_GiveShopItem(client,target);
                case 4: Admin_XP(client,target);
                case 5: Admin_Level(client,target);
                case 6: Admin_Crystals(client,target);
                case 7: Admin_Vespene(client,target);
                case 8: Admin_Energy(client,target);
                case 9: Admin_Disable(client,target);
                case 10: Admin_Delete(client,target);
            }
        }
        else
        {
            CPrintToChat(client, "{green}[SC]{default} %t", "NoPlayer");
            AdminMenu(client,0);
        }
    }
    else if (action == MenuAction_Cancel &&
             selection == MenuCancel_ExitBack)
    {
        EmitSoundToClient(client,buttonWav);
        AdminMenu(client,0);
    }
    else if (action == MenuAction_End)
        CloseHandle(menu);
}

Admin_Reset(client,target)
{
    // Reset upgrades
    new race=GetRace(target);
    new upgradeCount=GetUpgradeCount(GetRaceHandle(race));
    for (new y=0;y<upgradeCount;y++)
            SetUpgradeLevel(target,race,y,0);

    if (GetDatabaseLoaded(target) != DataLoading &&
        GetUpgradeLevelCount(target, race) < GetLevel(target,race))
    {
        UpgradeMenu(target, UpgradeMode);
    }

    LogAction(client, target, "[SC] %L reset %L's upgrades", client, target);
    CPrintToChat(target, "{green}[SC]{default} %t", "AdminResetPlayer", client);
    CPrintToChat(client, "{green}[SC]{default} %t", "AdminResetConfirm", target);

    Admin_Player(client,target);
}

Admin_Return(Handle:menu,client)
{
    EmitSoundToClient(client,buttonWav);

    decl String:SelectionInfo[SMALL_STRING_LENGTH];
    GetMenuItem(menu,1,SelectionInfo,sizeof(SelectionInfo));

    new target = StringToInt(SelectionInfo);
    if (IsValidClient(target))
        Admin_Player(client,target);
    else
    {
        CPrintToChat(client, "{green}[SC]{default} %t", "NoPlayer");
        AdminMenu(client,0);
    }
}

Admin_XP(client,target)
{
    if (IsValidClient(target))
    {
        new Handle:menu=CreateMenu(Admin_XP_Select);
        SetMenuExitButton(menu,true);
        SetMenuExitBackButton(menu,true);
        SetMenuTitle(menu, "[SC] %T", "AdminOption", client, target);

        decl String:buf[SMALL_STRING_LENGTH];
        Format(buf,sizeof(buf),"%d",target);

        AddMenuItemT(menu,buf,"Give100XP",client);
        AddMenuItemT(menu,buf,"Give1000XP",client);
        AddMenuItemT(menu,buf,"Give10000XP",client);
        AddMenuItemT(menu,buf,"Remove 100 XP",client);
        AddMenuItemT(menu,buf,"Remove 1000 XP",client);
        AddMenuItemT(menu,buf,"Remove 10000 XP",client);
        DisplayMenu(menu,client,MENU_TIME_FOREVER);
    }
    else
    {
        CPrintToChat(client, "{green}[SC]{default} %t", "NoPlayer");
        AdminMenu(client,0);
    }
}

public Admin_XP_Select(Handle:menu,MenuAction:action,client,selection)
{
    if (action == MenuAction_Select)
    {
        EmitSoundToClient(client,buttonWav);

        decl String:SelectionInfo[SMALL_STRING_LENGTH];
        GetMenuItem(menu,selection,SelectionInfo,sizeof(SelectionInfo));

        new target = StringToInt(SelectionInfo);
        if (IsValidClient(target))
        {
            new race=GetRace(target);
            if (selection < 3) // Give XP
            {
                new xpadd;
                switch (selection)
                {
                    case 0: xpadd=100;
                    case 1: xpadd=1000;
                    case 2: xpadd=10000;
                }
                new newxp=GetXP(target,race)+xpadd;
                SetXP(target,race,newxp);

                LogAction(client, target, "[SC] %L gave %L %d XP", client, target, xpadd);
                CPrintToChat(client, "{green}[SC]{default} %t", "AdminGaveXP", target, xpadd);
                CPrintToChat(target, "{green}[SC]{default} %t", "PlayerReceivedXP", client, xpadd);

                DoLevelCheck(target);
            }
            else
            {
                new xprem;
                switch (selection)
                {
                    case 3: xprem=100;
                    case 4: xprem=1000;
                    case 5: xprem=10000;
                }
                new newxp=GetXP(target,race)-xprem;
                if (newxp<0)
                    newxp=0;

                SetXP(target,race,newxp);

                LogAction(client, target, "[SC] %L removed %d XP from %L", client, xprem, target);
                CPrintToChat(client, "{green}[SC]{default} %t", "AdminRemovedXP", xprem, target);
                CPrintToChat(target, "{green}[SC]{default} %t", "PlayerForfeitedXP", client, xprem);
            }
            Admin_XP(client,target);
        }
        else
        {
            CPrintToChat(client, "{green}[SC]{default} %t", "NoPlayer");
            AdminMenu(client,0);
        }
    }
    else if (action == MenuAction_Cancel)
    {
        if (selection == MenuCancel_ExitBack)
            Admin_Return(menu,client);
    }
    else if (action == MenuAction_End)
        CloseHandle(menu);
}

Admin_GiveShopItem(client,target)
{
    if (IsValidClient(target))
    {
        new Handle:menu=CreateMenu(Admin_Category_Selected);
        SetMenuTitle(menu,"[SC] %T", "SelectItem", client, target);
        SetMenuExitBackButton(menu,true);
        SetMenuExitButton(menu,true);

        decl String:lastCategory[NAME_STRING_LENGTH];
        lastCategory[0] = '\0';

        new count = 0;
        new shopItemCount = GetShopIndexCount();
        for (new x=0;x<shopItemCount;x++)
        {
            new Handle:itemHandle=GetItemIndexHandle(x);
            decl String:itemCategory[NAME_STRING_LENGTH];
            GetItemCategory(itemHandle, itemCategory, sizeof(itemCategory),client);

            if (strcmp(itemCategory, lastCategory, false) != 0)
            {
                if (itemCategory[0] == '\0')
                {
                    Format(itemCategory,sizeof(itemCategory),
                           "%T", "MiscellaneousItems", client);
                }

                decl String:itembuf[SMALL_STRING_LENGTH];
                Format(itembuf,sizeof(itembuf),"%d,%d",target,x);
                AddMenuItem(menu,itembuf,itemCategory);
                strcopy(lastCategory, sizeof(lastCategory), itemCategory);
                count++;
            }
        }

        if (count > 0)
            DisplayMenu(menu,client,MENU_TIME_FOREVER);
        else
        {
            CancelMenu(menu);
            CPrintToChat(client, "{green}[SC]{default} %t", "NoItems");
            Admin_Return(menu,client);
        }
    }
    else
    {
        CPrintToChat(client, "{green}[SC]{default} %t", "NoPlayer");
    }
}

public Admin_Category_Selected(Handle:menu,MenuAction:action,client,selection)
{
    if (action == MenuAction_Select)
    {
        if (IsValidClient(client))
        {
            EmitSoundToClient(client,buttonWav);

            decl String:SelectionInfo[SMALL_STRING_LENGTH];
            GetMenuItem(menu,selection,SelectionInfo,sizeof(SelectionInfo));

            decl String:SelectionData[2][5];
            ExplodeString(SelectionInfo,",",SelectionData, 2, 5);
            new target = StringToInt(SelectionData[0]);
            new index  = StringToInt(SelectionData[1]);

            new shopItemCount = GetShopIndexCount();
            if (index >= 0 && index < shopItemCount)
            {
                decl String:theCategory[NAME_STRING_LENGTH];
                decl String:itemCategory[NAME_STRING_LENGTH];

                new Handle:itemMenu = CreateMenu(Admin_Item_Selected);
                SetMenuTitle(itemMenu,"[SC] %T", "SelectItem", client, target);
                SetMenuExitBackButton(itemMenu,true);
                SetMenuExitButton(itemMenu,true);

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
                        decl String:itembuf[SMALL_STRING_LENGTH];
                        Format(itembuf,sizeof(itembuf),"%d,%d",
                               target,GetShopId(itemHandle));

                        decl String:itemName[NAME_STRING_LENGTH];
                        GetItemName(itemHandle, itemName, sizeof(itemName),client);
                        AddMenuItem(itemMenu,itembuf,itemName);
                        count++;
                    }
                }

                if (count > 0)
                    DisplayMenu(itemMenu,client,MENU_TIME_FOREVER);
                else
                {
                    CancelMenu(itemMenu);
                    CPrintToChat(client, "{green}[SC]{default} %t", "NoItems");
                    Admin_Return(menu,client);
                }
            }
            else
            {
                CPrintToChat(client, "{green}[SC]{default} %t", "NoItems");
                Admin_Return(menu,client);
            }
        }
    }
    else if (action == MenuAction_Cancel)
    {
        if (selection == MenuCancel_ExitBack)
            Admin_Return(menu,client);
    }
    else if (action == MenuAction_End)
        CloseHandle(menu);
}

public Admin_Item_Selected(Handle:menu,MenuAction:action,client,selection)
{
    if (action == MenuAction_Select)
    {
        EmitSoundToClient(client,buttonWav);

        decl String:SelectionInfo[SMALL_STRING_LENGTH];
        GetMenuItem(menu,selection,SelectionInfo,sizeof(SelectionInfo));

        decl String:SelectionData[2][5];
        ExplodeString(SelectionInfo,",",SelectionData, 2, 5);
        new target = StringToInt(SelectionData[0]);
        if (IsValidClient(target))
        {
            new item=StringToInt(SelectionData[1]);
            if(!GetOwnsItem(target,item))
            {
                SetOwnsItem(target,item,1);

                new result;
                Call_StartForward(g_OnItemPurchaseHandle);
                Call_PushCell(target);
                Call_PushCell(item);
                Call_Finish(result);

                new Handle:itemHandle=GetShopItemHandle(item);
                decl String:itemname[NAME_STRING_LENGTH];
                GetItemName(itemHandle, itemname, sizeof(itemname),client);

                LogAction(client, target, "[SC] %L gave %L a %s", client, target, itemname);
                CPrintToChat(client, "{green}[SC]{default} %t", "AdminGaveItem", target, itemname);
                CPrintToChat(target, "{green}[SC]{default} %t", "PlayerReceivedItem", itemname, client);
            }
            else
            {
                CPrintToChat(client, "{green}[SC]{default} %t", "PlayerOwnsItem");
            }
            Admin_Player(client,target);
        }
        else
        {
            CPrintToChat(client, "{green}[SC]{default} %t", "NoPlayer");
            AdminMenu(client,0);
        }
    }
    else if (action == MenuAction_Cancel)
    {
        if (selection == MenuCancel_ExitBack)
        {
            EmitSoundToClient(client,buttonWav);

            decl String:SelectionInfo2[SMALL_STRING_LENGTH];
            GetMenuItem(menu,1,SelectionInfo2,sizeof(SelectionInfo2));
            Admin_GiveShopItem(client,StringToInt(SelectionInfo2));
        }
    }
    else if (action == MenuAction_End)
        CloseHandle(menu);
}

Admin_SetRace(client,target)
{
    if (IsValidClient(target))
    {
        new Handle:menu=CreateMenu(Admin_Faction_Selected);
        SetMenuTitle(menu,"[SC] %T", "SelectRace", client, "", target);
        SetMenuExitBackButton(menu,true);
        SetMenuExitButton(menu,true);

        new indexCount = GetRaceIndexCount();
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
                        GetRaceMenuItem(item, sizeof(item), client, raceHandle, 0, false);
                        if (item[0] != 0)
                        {
                            decl String:rbuf[SMALL_STRING_LENGTH];
                            Format(rbuf,sizeof(rbuf),"%d,%d",target,GetRaceId(raceHandle));
                            AddMenuItem(menu,rbuf,item,ITEMDRAW_DEFAULT);
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
                Format(rbuf,sizeof(rbuf),"%d,%d",target,-index);
                AddMenuItem(menu,rbuf,item,ITEMDRAW_DEFAULT);
            }
        }
        DisplayMenu(menu,client,MENU_TIME_FOREVER);
    }
    else
    {
        CPrintToChat(client, "{green}[SC]{default} %t", "NoPlayer");
        AdminMenu(client,0);
    }
}

public Admin_Faction_Selected(Handle:menu,MenuAction:action,client,selection)
{
    if (action == MenuAction_Select)
    {
        decl String:SelectionInfo[SMALL_STRING_LENGTH];
        GetMenuItem(menu,selection,SelectionInfo,sizeof(SelectionInfo));

        decl String:SelectionData[2][5];
        ExplodeString(SelectionInfo,",",SelectionData, 2, 5);
        new target = StringToInt(SelectionData[0]);
        new index  = StringToInt(SelectionData[1]);
        if (index < 0)
            AdminFactionMenu(client, target, -index);
        else            
            Admin_SetRace_Select(menu,MenuAction:action,client,selection);
    }
    else if (action == MenuAction_Cancel)
    {
        if (selection == MenuCancel_ExitBack)
            Admin_Return(menu,client);
    }
    else if (action == MenuAction_End)
        CloseHandle(menu);
}

public Admin_SetRace_Select(Handle:menu,MenuAction:action,client,selection)
{
    if (action == MenuAction_Select)
    {
        EmitSoundToClient(client,buttonWav);

        decl String:SelectionInfo[SMALL_STRING_LENGTH];
        GetMenuItem(menu,selection,SelectionInfo,sizeof(SelectionInfo));

        decl String:SelectionData[2][5];
        ExplodeString(SelectionInfo,",",SelectionData, 2, 5);

        new target = StringToInt(SelectionData[0]);
        if (IsValidClient(target))
        {
            new race=StringToInt(SelectionData[1]);
            SetRace(target,race);
            decl String:racename[NAME_STRING_LENGTH];
            GetRaceName(GetRaceHandle(race),racename,sizeof(racename),client);

            LogAction(client, target, "[SC] %L set %L's race to %s", client, target, racename);
            CPrintToChat(client, "{green}[SC]{default} %t", "AdminSetRace", target, racename);
            CPrintToChat(target, "{green}[SC]{default} %t", "PlayersRaceWasSet", client, racename);
        }
        else
        {
            CPrintToChat(client, "{green}[SC]{default} %t", "NoPlayer");
            AdminMenu(client,0);
        }
    }
    else if (action == MenuAction_Cancel)
    {
        if (selection == MenuCancel_ExitBack)
        {
            EmitSoundToClient(client,buttonWav);

            decl String:SelectionInfo2[SMALL_STRING_LENGTH];
            GetMenuItem(menu,1,SelectionInfo2,sizeof(SelectionInfo2));
            Admin_SetRace(client,StringToInt(SelectionInfo2));
        }
    }
    else if (action == MenuAction_End)
        CloseHandle(menu);
}

AdminFactionMenu(client, target, start)
{
    new indexCount = GetRaceIndexCount();
    if (start > 0 && start < indexCount)
    {
        new Handle:menu      = CreateMenu(Admin_SetRace_Select);
        new Faction:theFaction = GetRaceFaction(GetRaceIndexHandle(start));

        decl String:factionName[NAME_STRING_LENGTH];
        GetFactionName(theFaction,factionName,sizeof(factionName), client);

        SetMenuTitle(menu,"[SC] %T", "SelectRace", client, factionName, target);
        SetMenuExitBackButton(menu,true);
        SetMenuExitButton(menu,true);

        for(new index = start; index < indexCount; index++)
        {
            new Handle:raceHandle=GetRaceIndexHandle(index);
            if (GetRaceFaction(raceHandle) != theFaction)
                break;
            else
            {
                decl String:item[ITEM_STRING_LENGTH];
                GetRaceMenuItem(item, sizeof(item), client, raceHandle, 0, false);
                if (item[0] != 0)
                {
                    decl String:rbuf[SMALL_STRING_LENGTH];
                    Format(rbuf,sizeof(rbuf),"%d,%d",target,GetRaceId(raceHandle));
                    AddMenuItem(menu,rbuf,item,ITEMDRAW_DEFAULT);
                }
            }
        }
        DisplayMenu(menu,client,MENU_TIME_FOREVER);
    }
}

Admin_Level(client,target)
{
    if (IsValidClient(target))
    {
        new Handle:menu=CreateMenu(Admin_Level_Select);
        SetMenuExitButton(menu,true);
        SetMenuExitBackButton(menu,true);

        SetMenuTitle(menu, "[SC] %T", "AdminOption", client, target);

        decl String:buf[SMALL_STRING_LENGTH];
        Format(buf,sizeof(buf),"%d",target);

        AddMenuItemT(menu,buf,"GiveLevel",client);
        AddMenuItemT(menu,buf,"RemoveLevel",client);
        DisplayMenu(menu,client,MENU_TIME_FOREVER);
    }
    else
    {
        CPrintToChat(client, "{green}[SC]{default} %t", "NoPlayer");
        AdminMenu(client,0);
    }
}

public Admin_Level_Select(Handle:menu,MenuAction:action,client,selection)
{
    if (action == MenuAction_Select)
    {
        EmitSoundToClient(client,buttonWav);

        decl String:SelectionInfo[SMALL_STRING_LENGTH];
        GetMenuItem(menu,selection,SelectionInfo,sizeof(SelectionInfo));

        new target = StringToInt(SelectionInfo);
        if (IsValidClient(target))
        {
            new race=GetRace(target);
            if (selection==0)
            {
                // Give a level
                new newlevel = GetLevel(target,race)+1;
                if (newlevel > GetRaceMaxLevel(GetRaceHandle(race)))
                {
                    CPrintToChat(client, "{green}[SC]{default} %t", "PlayerAtMaxLevel", target);
                }
                else
                {
                    LogAction(client, target, "[SC] %L gave %L a level", client, target);
                    CPrintToChat(client, "{green}[SC]{default} %t", "AdminGaveLevel", target);
                    CPrintToChat(target, "{green}[SC]{default} %t", "PlayerReceivedLevel", client);

                    ResetLevel(target,race,newlevel);
                }
            }
            else
            {
                // Remove a level
                new newlevel = GetLevel(target,race)-1;
                if (newlevel < 0)
                {
                    CPrintToChat(client, "{green}[SC]{default} %t", "PlayerHasNoLevels", target);
                }
                else
                {
                    LogAction(client, target, "[SC] %L removed a level from %L", client, target);
                    CPrintToChat(client, "{green}[SC]{default} %t", "AdminRemovedLevel", target);
                    CPrintToChat(target, "{green}[SC]{default} %t", "PlayerForfeitedLevel", client);

                    ResetLevel(target,race,newlevel);
                }
            }
            Admin_Level(client,target);
        }
        else
        {
            CPrintToChat(client, "{green}[SC]{default} %t", "NoPlayer");
            AdminMenu(client,0);
        }
    }
    else if (action == MenuAction_Cancel)
    {
        if (selection == MenuCancel_ExitBack)
            Admin_Return(menu,client);
    }
    else if (action == MenuAction_End)
        CloseHandle(menu);
}

Admin_Crystals(client,target)
{
    if (IsValidClient(target))
    {
        new Handle:menu=CreateMenu(Admin_Crystals_Select);
        SetMenuExitButton(menu,true);
        SetMenuExitBackButton(menu,true);

        SetMenuTitle(menu, "[SC] %T", "AdminOption", client, target);

        decl String:buf[SMALL_STRING_LENGTH];
        Format(buf,sizeof(buf),"%d",target);

        AddMenuItemT(menu,buf,"Give1Crystal",client);
        AddMenuItemT(menu,buf,"Give5Crystals",client);
        AddMenuItemT(menu,buf,"Give10Crystals",client);
        AddMenuItemT(menu,buf,"Give25Crystals",client);
        AddMenuItemT(menu,buf,"Remove1Crystal",client);
        AddMenuItemT(menu,buf,"Remove5Crystals",client);
        AddMenuItemT(menu,buf,"Remove10Crystals",client);
        AddMenuItemT(menu,buf,"Remove25Crystals",client);

        DisplayMenu(menu,client,MENU_TIME_FOREVER);
    }
    else
    {
        CPrintToChat(client, "{green}[SC]{default} %t", "NoPlayer");
        AdminMenu(client,0);
    }
}

public Admin_Crystals_Select(Handle:menu,MenuAction:action,client,selection)
{
    if (action == MenuAction_Select)
    {
        EmitSoundToClient(client,buttonWav);

        decl String:SelectionInfo[SMALL_STRING_LENGTH];
        GetMenuItem(menu,selection,SelectionInfo,sizeof(SelectionInfo));

        new target = StringToInt(SelectionInfo);
        if (IsValidClient(target))
        {
            if (selection < 4) // Give crystals
            {
                new credadd;
                switch (selection)
                {
                    case 0: credadd=1;
                    case 1: credadd=5;
                    case 2: credadd=10;
                    case 3: credadd=25;
                }
                new newcred = GetCrystals(target)+credadd;
                if (newcred > g_iMaxCrystals && g_iMaxCrystals > 0)
                    newcred = g_iMaxCrystals;
                SetCrystals(target,newcred);

                LogAction(client, target, "[SC] %L gave %L %d crystals", client, target, credadd);
                CPrintToChat(client, "{green}[SC]{default} %t", "AdminGaveCrystals", target, credadd);
                CPrintToChat(target, "{green}[SC]{default} %t", "PlayerReceivedCrystals", credadd, client);

                Admin_Crystals(client,target);
            }
            else
            {
                new credrem;
                switch (selection)
                {
                    case 4: credrem=1;
                    case 5: credrem=5;
                    case 6: credrem=10;
                    case 7: credrem=25;
                }
                new newcred = GetCrystals(target)-credrem;
                if (newcred < 0)
                    newcred = 0;
                SetCrystals(target,newcred);

                LogAction(client, target, "[SC] %L removed %d crystals from %L", client, credrem, target);
                CPrintToChat(client, "{green}[SC]{default} %t", "AdminRemovedCrystals", credrem, target);
                CPrintToChat(target, "{green}[SC]{default} %t", "PlayerForfeitedCrystals", client, credrem);

                Admin_Crystals(client,target);
            }
        }
        else
        {
            CPrintToChat(client, "{green}[SC]{default} %t", "NoPlayer");
            AdminMenu(client,0);
        }
    }
    else if (action == MenuAction_Cancel)
    {
        if (selection == MenuCancel_ExitBack)
            Admin_Return(menu,client);
    }
    else if (action == MenuAction_End)
        CloseHandle(menu);
}

Admin_Vespene(client,target)
{
    if (IsValidClient(target))
    {
        new Handle:menu=CreateMenu(Admin_Vespene_Select);
        SetMenuExitButton(menu,true);
        SetMenuExitBackButton(menu,true);

        SetMenuTitle(menu, "[SC] %T", "AdminOption", client, target);

        decl String:buf[SMALL_STRING_LENGTH];
        Format(buf,sizeof(buf),"%d",target);

        AddMenuItemT(menu,buf,"Give1Vespene",client);
        AddMenuItemT(menu,buf,"Give5Vespene",client);
        AddMenuItemT(menu,buf,"Give10Vespene",client);
        AddMenuItemT(menu,buf,"Give50Vespene",client);
        AddMenuItemT(menu,buf,"Give100Vespene",client);
        AddMenuItemT(menu,buf,"Remove1Vespene",client);
        AddMenuItemT(menu,buf,"Remove5Vespene",client);
        AddMenuItemT(menu,buf,"Remove50Vespene",client);
        AddMenuItemT(menu,buf,"Remove100Vespene",client);

        DisplayMenu(menu,client,MENU_TIME_FOREVER);
    }
    else
    {
        CPrintToChat(client, "{green}[SC]{default} %t", "NoPlayer");
        AdminMenu(client,0);
    }
}

public Admin_Vespene_Select(Handle:menu,MenuAction:action,client,selection)
{
    if (action == MenuAction_Select)
    {
        EmitSoundToClient(client,buttonWav);

        decl String:SelectionInfo[SMALL_STRING_LENGTH];
        GetMenuItem(menu,selection,SelectionInfo,sizeof(SelectionInfo));

        new target = StringToInt(SelectionInfo);
        if (IsValidClient(target))
        {
            if (selection < 5) // Give vespene
            {
                new gasadd;
                switch(selection)
                {
                    case 0: gasadd=1;
                    case 1: gasadd=5;
                    case 2: gasadd=10;
                    case 3: gasadd=50;
                    case 4: gasadd=100;
                }
                new newgas=GetVespene(target)+gasadd;
                if (newgas > g_iMaxVespene && g_iMaxVespene > 0)
                    newgas = g_iMaxVespene;
                SetVespene(target,newgas);

                LogAction(client, target, "[SC] %L gave %L %d vespene", client, target, gasadd);
                CPrintToChat(client, "{green}[SC]{default} %t", "AdminGaveVespene", target, gasadd);
                CPrintToChat(target, "{green}[SC]{default} %t", "PlayerReceivedVespene", gasadd, client);

                Admin_Vespene(client,target);
            }
            else
            {
                new gasrem;
                switch(selection)
                {
                    case 5: gasrem=1;
                    case 6: gasrem=5;
                    case 7: gasrem=10;
                    case 8: gasrem=50;
                    case 9: gasrem=100;
                }
                new newgas = GetVespene(target)-gasrem;
                if (newgas < 0)
                    newgas = 0;
                SetVespene(target,newgas);

                LogAction(client, target, "[SC] %L removed %d vespene from %L", client, gasrem, target);
                CPrintToChat(client, "{green}[SC]{default} %t", "AdminRemovedVespene", gasrem, target);
                CPrintToChat(target, "{green}[SC]{default} %t", "PlayerForfeitedVespene", client, gasrem);

                Admin_Vespene(client,target);
            }
        }
        else
        {
            CPrintToChat(client, "{green}[SC]{default} %t", "NoPlayer");
            AdminMenu(client,0);
        }
    }
    else if (action == MenuAction_Cancel)
    {
        if (selection == MenuCancel_ExitBack)
            Admin_Return(menu,client);
    }
    else if (action == MenuAction_End)
        CloseHandle(menu);
}

Admin_Energy(client,target)
{
    if (IsValidClient(target))
    {
        new Handle:menu=CreateMenu(Admin_Energy_Select);
        SetMenuExitButton(menu,true);
        SetMenuExitBackButton(menu,true);

        SetMenuTitle(menu, "[SC] %T", "AdminOption", client, target);

        decl String:buf[SMALL_STRING_LENGTH];
        Format(buf,sizeof(buf),"%d",target);

        AddMenuItemT(menu,buf,"Give1Energy",client);
        AddMenuItemT(menu,buf,"Give5Energy",client);
        AddMenuItemT(menu,buf,"Give10Energy",client);
        AddMenuItemT(menu,buf,"Give50Energy",client);
        AddMenuItemT(menu,buf,"Give100Energy",client);
        AddMenuItemT(menu,buf,"Remove1Energy",client);
        AddMenuItemT(menu,buf,"Remove5Energy",client);
        AddMenuItemT(menu,buf,"Remove10Energy",client);
        AddMenuItemT(menu,buf,"Remove50Energy",client);
        AddMenuItemT(menu,buf,"Remove100Energy",client);

        DisplayMenu(menu,client,MENU_TIME_FOREVER);
    }
    else
    {
        CPrintToChat(client, "{green}[SC]{default} %t", "NoPlayer");
        AdminMenu(client,0);
    }
}

public Admin_Energy_Select(Handle:menu,MenuAction:action,client,selection)
{
    if (action == MenuAction_Select)
    {
        EmitSoundToClient(client,buttonWav);

        decl String:SelectionInfo[SMALL_STRING_LENGTH];
        GetMenuItem(menu,selection,SelectionInfo,sizeof(SelectionInfo));

        new target = StringToInt(SelectionInfo);
        if (IsValidClient(target))
        {
            if (selection < 5) // Give energy
            {
                new add;
                switch(selection)
                {
                    case 0: add=1;
                    case 1: add=5;
                    case 2: add=10;
                    case 3: add=50;
                    case 4: add=100;
                }
                new energy=GetEnergy(target)+add;
                SetEnergy(target,energy);

                LogAction(client, target, "[SC] %L gave %L %d energy", client, target, add);
                CPrintToChat(client, "{green}[SC]{default} %t", "AdminGaveEnergy", target, add);
                CPrintToChat(target, "{green}[SC]{default} %t", "PlayerReceivedEnergy", add, client);

                Admin_Energy(client,target);
            }
            else
            {
                new rem;
                switch(selection)
                {
                    case 5: rem=1;
                    case 6: rem=5;
                    case 7: rem=10;
                    case 8: rem=50;
                    case 9: rem=100;
                }
                new energy = GetEnergy(target)-rem;
                if (energy < 0)
                    energy = 0;
                SetEnergy(target,energy);

                LogAction(client, target, "[SC] %L removed %d energy from %L", client, rem, target);
                CPrintToChat(client, "{green}[SC]{default} %t", "AdminRemovedEnergy", rem, target);
                CPrintToChat(target, "{green}[SC]{default} %t", "PlayerForfeitedEnergy", client, rem);

                Admin_Energy(client,target);
            }
        }
        else
        {
            CPrintToChat(client, "{green}[SC]{default} %t", "NoPlayer");
            AdminMenu(client,0);
        }
    }
    else if (action == MenuAction_Cancel)
    {
        if (selection == MenuCancel_ExitBack)
            Admin_Return(menu,client);
    }
    else if (action == MenuAction_End)
        CloseHandle(menu);
}

Admin_Disable(client,target)
{
    if (IsValidClient(target))
    {
        new Handle:menu=CreateMenu(Admin_Disable_Confirm);
        SetMenuExitButton(menu,true);
        SetMenuExitBackButton(menu,true);

        decl String:buf[SMALL_STRING_LENGTH];
        Format(buf,sizeof(buf),"%d",target);

        if (GetPlayerStatus(target) == PlayerDisabled)
        {
            SetMenuTitle(menu, "[SC] %T", "ConfirmEnable", client, target);
            AddMenuItemT(menu, buf, "EnablePlayer", client);
        }
        else
        {
            SetMenuTitle(menu, "[SC] %T", "ConfirmDisable", client, target);
            AddMenuItemT(menu, buf, "DisablePlayer", client);
        }

        AddMenuItemT(menu, buf, "No", client);

        DisplayMenu(menu,client,MENU_TIME_FOREVER);
    }
    else
    {
        CPrintToChat(client, "{green}[SC]{default} %t", "NoPlayer");
        AdminMenu(client,0);
    }
}

public Admin_Disable_Confirm(Handle:menu,MenuAction:action,client,selection)
{
    if (action == MenuAction_Select)
    {
        EmitSoundToClient(client,buttonWav);

        decl String:SelectionInfo[SMALL_STRING_LENGTH];
        GetMenuItem(menu,selection,SelectionInfo,sizeof(SelectionInfo));

        new target = StringToInt(SelectionInfo);
        if (IsValidClient(target))
        {
            switch (selection)
            {
                case 0: // Yes
                {
                    if (GetPlayerStatus(target) == PlayerDisabled)
                    {
                        EnablePlayer(client, target);
                        ResetPlayer(target, false);
                        LoadPlayerData(target); // Reload levels

                        LogAction(client, target, "[SC] %L re-enabled %L", client, target);
                        CPrintToChat(client, "{green}[SC]{default} %t", "AdminEnabled", target);
                        CPrintToChat(target, "{green}[SC]{default} %t", "PlayerEnabled", client);
                    }
                    else
                    {
                        DisablePlayer(client, target);
                        ResetPlayer(target, false); // Reset target

                        new race = FindRace("human"); // Make target a human
                        SetRace(target, (race >= 0) ? race : 0);

                        LogAction(client, target, "[SC] %L disabled %L", client, target);
                        CPrintToChat(client, "{green}[SC]{default} %t", "AdminDisabled", target);
                        CPrintToChat(target, "{green}[SC]{default} %t", "PlayerDisabled", client);
                    }

                    Admin_Return(menu,client);
                }
            }
        }
        else
        {
            CPrintToChat(client, "{green}[SC]{default} %t", "NoPlayer");
            AdminMenu(client,0);
        }
    }
    else if (action == MenuAction_Cancel)
    {
        if (selection == MenuCancel_ExitBack)
            Admin_Return(menu,client);
    }
    else if (action == MenuAction_End)
        CloseHandle(menu);
}

Admin_Delete(client,target)
{
    if (IsValidClient(target))
    {
        new Handle:menu=CreateMenu(Admin_Delete_Confirm);
        SetMenuExitButton(menu,true);
        SetMenuExitBackButton(menu,true);

        SetMenuTitle(menu,"[SC] %T", "ConfirmDelete", client, target);

        decl String:buf[SMALL_STRING_LENGTH];
        Format(buf,sizeof(buf),"%d",target);

        AddMenuItemT(menu, buf, "DeletePlayer", client);
        AddMenuItemT(menu, buf, "No", client);

        DisplayMenu(menu,client,MENU_TIME_FOREVER);
    }
    else
    {
        CPrintToChat(client, "{green}[SC]{default} %t", "NoPlayer");
        AdminMenu(client,0);
    }
}

public Admin_Delete_Confirm(Handle:menu,MenuAction:action,client,selection)
{
    if (action == MenuAction_Select)
    {
        EmitSoundToClient(client,buttonWav);

        decl String:SelectionInfo[SMALL_STRING_LENGTH];
        GetMenuItem(menu,selection,SelectionInfo,sizeof(SelectionInfo));

        new target = StringToInt(SelectionInfo);
        if (IsValidClient(target))
        {
            switch (selection)
            {
                case 0: // Yes
                {
                    DeletePlayer(client, target);
                    ResetPlayer(target, false); // Reset target

                    new race = FindRace("human"); // Make target a human
                    SetRace(target, (race >= 0) ? race : 0);

                    LogAction(client, target, "[SC] %L deleted %L from the database", client, target);
                    CPrintToChat(client, "{green}[SC]{default}", "AdminDeleted", target);
                    CPrintToChat(target, "{green}[SC]{default}", "PlayerDeleted", client);

                    Admin_Return(menu,client);
                }
            }
        }
        else
        {
            CPrintToChat(client, "{green}[SC]{default} %t", "NoPlayer");
            AdminMenu(client,0);
        }
    }
    else if (action == MenuAction_Cancel)
    {
        if (selection == MenuCancel_ExitBack)
            Admin_Return(menu,client);
    }
    else if (action == MenuAction_End)
        CloseHandle(menu);
}
