/*
 * vim: set ai et ts=4 sw=4 syntax=sourcepawn :
 * File: hooks.inc
 * Description: Handles hooking functions, will be used to get chat commands.
 * Author(s): Naris (Murray Wilson)
 * Credits: Anthony Iacono
 */

InitHooks()
{
    AddCommandListener(SayCommand, "say");
    AddCommandListener(SayCommand, "say_team");
    AddCommandListener(ChangelevelCommand, "changelevel");

    RegConsoleCmd("sc_menu",SCMenuCommand,"use SourceCraft menu to change race, reset upgrades, etc...");
    RegConsoleCmd("sm_menu",SCMenuCommand,"use SourceCraft menu to change race, reset upgrades, etc...");
    RegConsoleCmd("sc_buy",BuyCommand,"Purchase a SourceCraft Item");
    RegConsoleCmd("sm_buy",BuyCommand,"Purchase a SourceCraft Item");
    RegConsoleCmd("shopmenu",BuyCommand,"use SourceCraft shopmenu to purchase shop items");
    RegConsoleCmd("changeunit",ChangeRaceCommand,"SourceCraft command to change unit.");
    RegConsoleCmd("changerace",ChangeRaceCommand,"SourceCraft command to change race.");
    RegConsoleCmd("sc_settings",ChangeSettingsCommand,"SourceCraft command to change settings.");
    RegConsoleCmd("sc_info",InfoCommand,"View SourceCraft Information and lookup players");
    RegConsoleCmd("sm_info",InfoCommand,"View SourceCraft Information and lookup players");
    RegConsoleCmd("sc_updates",UpdatesCommand,"View SourceCraft Updates");
    RegConsoleCmd("sm_updates",UpdatesCommand,"View SourceCraft Updates");
    RegConsoleCmd("sc_guiki",WikiCommand,"View SourceCraft [Gu]Wiki");
    RegConsoleCmd("sm_guiki",WikiCommand,"View SourceCraft [Gu]Wiki");
    RegConsoleCmd("sc_wiki",WikiCommand,"View SourceCraft [Gu]Wiki");
    RegConsoleCmd("sm_wiki",WikiCommand,"View SourceCraft [Gu]Wiki");
    RegConsoleCmd("sc_bug",BugCommand,"Report Bugs");
    RegConsoleCmd("sm_bug",BugCommand,"Report Bugs");

    RegConsoleCmd("+ultimate",UltimateCommand,"use SourceCraft ultimate ability (keydown)",FCVAR_GAMEDLL);
    RegConsoleCmd("-ultimate",UltimateCommand,"use SourceCraft ultimate ability (keyup)",FCVAR_GAMEDLL);

    RegConsoleCmd("+ultimate1",UltimateCommand,"use SourceCraft ultimate 1 ability (keydown)",FCVAR_GAMEDLL);
    RegConsoleCmd("-ultimate1",UltimateCommand,"use SourceCraft ultimate 1 ability (keyup)",FCVAR_GAMEDLL);

    RegConsoleCmd("+ultimate2",UltimateCommand,"use SourceCraft ultimate 2 ability (keydown)",FCVAR_GAMEDLL);
    RegConsoleCmd("-ultimate2",UltimateCommand,"use SourceCraft ultimate 2 ability (keyup)",FCVAR_GAMEDLL);

    RegConsoleCmd("+ultimate3",UltimateCommand,"use SourceCraft ultimate 3 ability (keydown)",FCVAR_GAMEDLL);
    RegConsoleCmd("-ultimate3",UltimateCommand,"use SourceCraft ultimate 3 ability (keyup)",FCVAR_GAMEDLL);

    RegConsoleCmd("+ultimate4",UltimateCommand,"use SourceCraft ultimate 4 ability (keydown)",FCVAR_GAMEDLL);
    RegConsoleCmd("-ultimate4",UltimateCommand,"use SourceCraft ultimate 4 ability (keyup)",FCVAR_GAMEDLL);

    RegConsoleCmd("+ultimate5",UltimateCommand,"use SourceCraft ultimate 5 ability (keydown)",FCVAR_GAMEDLL);
    RegConsoleCmd("-ultimate5",UltimateCommand,"use SourceCraft ultimate 5 ability (keyup)",FCVAR_GAMEDLL);
}

public Action:SayCommand(client, const String:command[], argc)
{
    TraceInto("Hooks", "SayCommand", "client=%d:%L, argc=%d, command=%s", \
              client, ValidClientIndex(client), argc, command);

    if (IsValidClient(client))
    {
        decl String:text[128];
        GetCmdArg(1,text,sizeof(text));

        decl String:arg[2][64];
        new argCount = ExplodeString(text, " ", arg, 2, 64);

        new String:firstChar[] = " ";
        firstChar{0} = arg[0]{0};
        if (StrContains("!/\\",firstChar) >= 0)
            strcopy(arg[0], sizeof(arg[]), arg[0]{1});

        if (StrEqual(arg[0],"menu") ||
            StrEqual(arg[0],"scmenu") ||
            StrEqual(arg[0],"wcsmenu"))
        {
            MainMenu(client);
            return Plugin_Handled;
        }
        else if (StrEqual(arg[0],"shopmenu") ||
                 StrEqual(arg[0],"scbuy") ||
                 StrEqual(arg[0],"buy"))
        {
            if (argCount > 1 && strlen(arg[1]) > 0)
            {
                new item = FindShopItem(arg[1]);
                if (item >= 0)
                    PurchaseShopItem(client, item);
                else
                    ShopMenu(client,client,PurchaseItemMode);
            }
            else
                ShopMenu(client,client,PurchaseItemMode);
            return Plugin_Handled;
        }
        else if (StrEqual(arg[0],"changeunit") ||
                 StrEqual(arg[0],"changerace"))
        {
            if (argCount > 1 && strlen(arg[1]) > 0)
            {
                new race = FindRace(arg[1]);
                if (race >= 0)
                {
                    new Handle:raceHandle=GetRaceHandle(race);
                    if (raceHandle != INVALID_HANDLE)
                        ChangeRace(client, race, raceHandle, false, true);
                    else
                        RaceMenu(client,ChangeRaceMode);
                }
                else
                    RaceMenu(client,ChangeRaceMode);
            }
            else
                RaceMenu(client,ChangeRaceMode);

            return Plugin_Handled;
        }
        else if (StrEqual(arg[0],"unitinfo") ||
                 StrEqual(arg[0],"raceinfo"))
        {
            RaceMenu(client,InfoMode);
            return Plugin_Handled;
        }
        else if (StrEqual(arg[0],"reset") ||
                 StrEqual(arg[0],"resetupgrades") ||
                 StrEqual(arg[0],"resetskills"))
        {
            ResetUpgrades(client);
            return Plugin_Handled;
        }
        else if (StrEqual(arg[0],"upgrade") ||
                 StrEqual(arg[0],"spendup") ||
                 StrEqual(arg[0],"spendupgrades") ||
                 StrEqual(arg[0],"spendskills"))
        {
            new race=GetRace(client);
            if (GetUpgradeLevelCount(client, race) < GetLevel(client, race))
                UpgradeMenu(client, UpgradeMode);
            else
            {
                CPrintToChat(client,"{green}[SC]{default} %t", "NoPoints");
            }
            return Plugin_Handled;
        }
        else if (StrEqual(arg[0],"schelp") ||
                 StrEqual(arg[0],"help"))
        {
            Help(client);
            return Plugin_Handled;
        }
        else if(StrEqual(arg[0],"scsettings"))
        {
            ChangeSettings(client, false);
            return Plugin_Handled;
        }
        else if (StrEqual(arg[0],"info") ||
                 StrEqual(arg[0],"show"))
        {
            PlayerInfo(client,client,InfoMode|QueryMode);
            return Plugin_Handled;
        }
        else if (StrEqual(arg[0],"playerinfo") ||
                 StrEqual(arg[0],"showplayer"))
        {
            PlayerInfoMenu(client,InfoMode);
            return Plugin_Handled;
        }
        else if (StrEqual(arg[0],"showup") ||
                 StrEqual(arg[0],"upinfo") ||
                 StrEqual(arg[0],"showupgrades") ||
                 StrEqual(arg[0],"upgradeinfo") ||
                 StrEqual(arg[0],"showskills") ||
                 StrEqual(arg[0],"skillsinfo"))
        {
            UpgradesInfo(client,client,GetRace(client),InfoMode);
            return Plugin_Handled;
        }
        else if (StrEqual(arg[0],"showitems") ||
                 StrEqual(arg[0],"inventory") ||
                 StrEqual(arg[0],"inv"))
        {
            ShopMenu(client,client,InfoMode);
            return Plugin_Handled;
        }
        else if (StrEqual(arg[0],"xp") ||
                 StrEqual(arg[0],"showxp") ||
                 StrEqual(arg[0],"showexp") ||
                 StrEqual(arg[0],"showexperience"))
        {
            ShowXP(client,true);
            return Plugin_Handled;
        }
        else if (StrEqual(arg[0],"crystals") || 
                 StrEqual(arg[0],"showcrystals") ||
                 StrEqual(arg[0],"showc"))
        {
            ShowCrystals(client,true);
            return Plugin_Handled;
        }
        else if (StrEqual(arg[0],"vespene") || 
                 StrEqual(arg[0],"showvespene") ||
                 StrEqual(arg[0],"showv"))
        {
            ShowVespene(client,true);
            return Plugin_Handled;
        }
        else if (StrEqual(arg[0],"energy") || 
                 StrEqual(arg[0],"showenergy") ||
                 StrEqual(arg[0],"showe"))
        {
            ShowEnergy(client,true);
            return Plugin_Handled;
        }
        else if (StrEqual(arg[0],"scinfo") || 
                 StrEqual(arg[0],"details") || 
                 StrEqual(arg[0],"lookup"))
        {
            ShowInfo(client);
            return Plugin_Handled;
        }
        else if (StrEqual(arg[0],"wiki") || 
                 StrEqual(arg[0],"guiki"))
        {
            ShowWiki(client);
            return Plugin_Handled;
        }
        else if (StrEqual(arg[0],"scinfo") || 
                 StrEqual(arg[0],"updates"))
        {
            ShowUpdates(client);
            return Plugin_Handled;
        }
        else if (StrEqual(arg[0],"scbug") || 
                 StrEqual(arg[0],"bug"))
        {
            ReportBugs(client);
            return Plugin_Handled;
        }
        else if (StrEqual(arg[0],"killme"))
        {
            KillPlayer(client, client, "sc_suicide", "Suicide");
        }
        else if (StrEqual(arg[0],"entities"))
        {
            new max = GetMaxEntities();
            new count = GetEntityCount();
            new remaining = max - count;

            LogMessage("There are %d of %d entities available, %d are used.",
                       remaining,max,count);

            DisplayMessage(client, Display_Misc_Message,
                           "There are {green}%d of %d{default} entities available, %c%d%c are used.",
                           remaining,max,count);
        }
    }
    else
    {
        TraceCat("Connect", "Command issued from Invalid client=%d", client);
    }

    TraceReturn();
    return Plugin_Continue;
}

public Action:UltimateCommand(client,args)
{
    TraceInto("Hooks", "UltimateCommand");

    if (IsValidClient(client))
    {
        new result;
        new race=GetRace(client);

        decl String:command[CLASS_STRING_LENGTH];
        GetCmdArg(0,command,sizeof(command));
        new bool:pressed=(!strcmp(command,"+ultimate"));

        new arg;
        if (IsCharNumeric(command[9]))
        {
            arg = StringToInt(command[9]);

            Trace("%N issuing +ultimate%d, pressed=%d, args=%d, CmdArgs=%d", \
                  client, arg, pressed, args, GetCmdArgs());
        }
        else if (args > 1) // (GetCmdArgs() > 1)
        {
            decl String:argString[16];
            GetCmdArg(1,argString,sizeof(argString));
            arg = StringToInt(argString);

            Trace("%N issuing +ultimate %s(%d), pressed=%d, args=%d, CmdArgs=%d", \
                  client, argString, arg, pressed, args, GetCmdArgs());
        }
        else
        {
            arg = 1;

            Trace("%N issuing +ultimate, pressed=%d, args=%d, CmdArgs=%d", \
                  client, pressed, args, GetCmdArgs());
        }

        Call_StartForward(g_OnUltimateCommandHandle);
        Call_PushCell(client);
        Call_PushCell(race);
        Call_PushCell(pressed);
        Call_PushCell(arg);
        Call_Finish(result);

        Trace("%N completed +ultimate %s, pressed=%d", client, arg, pressed);
    }

    TraceReturn();
    return Plugin_Handled;
}

public Action:SCMenuCommand(client,args)
{
    MainMenu(client);
    return Plugin_Handled;
}

public Action:BuyCommand(client,args)
{
    if (IsValidClient(client))
    {
        decl String:arg[64];
        if (GetCmdArg(1,arg,sizeof(arg)) > 0)
        {
            new item = FindShopItem(arg);
            if (item >= 0)
                PurchaseShopItem(client, item);
            else
                ShopMenu(client,client,PurchaseItemMode);
        }
        else
            ShopMenu(client,client,PurchaseItemMode);
    }
    return Plugin_Handled;
}

public Action:ChangeRaceCommand(client,args)
{
    if (IsValidClient(client))
    {
        decl String:arg[64];
        if (GetCmdArg(1,arg,sizeof(arg)) > 0)
        {
            new race = FindRace(arg);
            if (race >= 0)
            {
                new Handle:raceHandle=GetRaceHandle(race);
                if (raceHandle != INVALID_HANDLE)
                    ChangeRace(client, race, raceHandle, false, true);
                else
                    RaceMenu(client,ChangeRaceMode);
            }
            else
                RaceMenu(client,ChangeRaceMode);
        }
        else
            RaceMenu(client,ChangeRaceMode);
    }
    return Plugin_Handled;
}

public Action:ChangeSettingsCommand(client,args)
{
    ChangeSettings(client, false);
    return Plugin_Handled;
}

public Action:InfoCommand(client,args)
{
    ShowInfo(client);
    return Plugin_Handled;
}

ShowInfo(client)
{
    decl String:url[128];
    decl String:steamid[CLASS_STRING_LENGTH];
    if (GetClientAuthString(client,steamid,sizeof(steamid)))
        Format(url, sizeof(url), g_InfoURL, steamid);
    else
        strcopy(url, sizeof(url), g_InfoBaseURL);

    ShowMOTDPanel(client, "SourceCraft Information", url, MOTDPANEL_TYPE_URL);
}

public Action:WikiCommand(client,args)
{
    ShowWiki(client);
    return Plugin_Handled;
}

ShowWiki(client)
{
    ShowMOTDPanel(client, "SourceCraft [Gu]Wiki", g_WikiURL, MOTDPANEL_TYPE_URL);
}

public Action:UpdatesCommand(client,args)
{
    ShowUpdates(client);
    return Plugin_Handled;
}

ShowUpdates(client)
{
    ShowMOTDPanel(client, "SourceCraft Updates", g_UpdateURL, MOTDPANEL_TYPE_URL);
}

public Action:BugCommand(client,args)
{
    ReportBugs(client);
    return Plugin_Handled;
}

ReportBugs(client)
{
    ShowMOTDPanel(client, "SourceCraft Bugs", g_BugURL, MOTDPANEL_TYPE_URL);
}

public Action:ChangelevelCommand(client, const String:command[], argc)
{
    g_MapChanging = true;
    return Plugin_Continue;
}

