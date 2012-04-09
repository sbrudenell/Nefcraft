/**
 * vim: set ai et ts=4 sw=4 syntax=sourcepawn :
 * File: help.inc
 * Description: Holds all the help functions.
 * Author(s): Naris (Murray Wilson)
 * Credits: Anthony Iacono
 */

#define HELPCOMMAND_COUNT GetArraySize(vecHelpCommands)

new Handle:vecHelpCommands = INVALID_HANDLE;

bool:InitHelpVector()
{
    TraceInto("Help", "InitHelpVector");

    if (vecHelpCommands == INVALID_HANDLE)
        vecHelpCommands=CreateArray();

    TraceReturn("vecHelpCommands=0x%08x", vecHelpCommands);
    return (vecHelpCommands != INVALID_HANDLE);
}

ClearHelpVector()
{
    TraceInto("Help", "ClearHelpVector");

    if (vecHelpCommands != INVALID_HANDLE)
    {
        TraceCat("Array", "CloseArray vecHelpCommands=0x%08x", vecHelpCommands);

        ClearArray(vecHelpCommands);
        CloseHandle(vecHelpCommands);
        vecHelpCommands = INVALID_HANDLE;
    }

    TraceReturn();
}

Handle:HelpItem(command)
{
    return GetArrayCell(vecHelpCommands,command);
}

NewHelpCommand(String:name[],String:info[])
{
    new Handle:vec=CreateArray(ByteCountToCells(256));
    PushArrayString(vec,name);
    PushArrayString(vec,info);
    return PushArrayCell(vecHelpCommands,vec);
}

InitHelpCommands()
{
    NewHelpCommand("say menu",          "HelpMenu");
    NewHelpCommand("bind",              "HelpBind");
    NewHelpCommand("say info",          "HelpInfo");
    NewHelpCommand("say inv",           "HelpInv");
    NewHelpCommand("say shopmenu",      "HelpShop");
    NewHelpCommand("say changerace",    "HelpChangeRace");
    NewHelpCommand("say reset",         "HelpReset");
    NewHelpCommand("say raceinfo",      "HelpRaceInfo");
    NewHelpCommand("say upgradeinfo",   "HelpUpgradeInfo");
    NewHelpCommand("say upgrade",       "HelpUpgrade");
    NewHelpCommand("say showxp",        "HelpShowXP");
    NewHelpCommand("say showupgrades",  "HelpShowUpgrades");
    NewHelpCommand("say crystals",      "HelpCrystals");
    NewHelpCommand("say vespene",       "HelpVespene");
    NewHelpCommand("say energy",        "HelpEnergy");
    NewHelpCommand("say playerinfo",    "HelpPlayerInfo");
    NewHelpCommand("say settings",      "HelpSettings");
    NewHelpCommand("+ultimate",         "HelpUltimate");
    NewHelpCommand("+ultimate 1",       "HelpUltimate1");
    NewHelpCommand("+ultimate 2",       "HelpUltimate2");
    NewHelpCommand("+ultimate 3",       "HelpUltimate3");
    NewHelpCommand("+ultimate 4",       "HelpUltimate4");
    NewHelpCommand("+item",             "HelpItem0");
    NewHelpCommand("+item 1",           "HelpItem1");
    NewHelpCommand("+item 2",           "HelpItem2");
    NewHelpCommand("+item 3",           "HelpItem3");
    NewHelpCommand("+item 4",           "HelpItem4");
    NewHelpCommand("sc_admin",          "HelpAdmin");
}

Help(client)
{
    new Handle:helpMenu=CreateMenu(HelpMenu_Selected);
    SetMenuExitButton(helpMenu,true);
    SetMenuTitle(helpMenu,"[SC] %T", "HelpTitle", client);
    decl String:commandname[64];
    decl String:helpbuf[4];
    new Handle:commandHandle;
    for(new x=0;x<HELPCOMMAND_COUNT;x++)
    {
        Format(helpbuf,3,"%d",x);
        commandHandle=HelpItem(x);
        GetArrayString(commandHandle,0,commandname,sizeof(commandname));
        AddMenuItem(helpMenu,helpbuf,commandname);
    }
    DisplayMenu(helpMenu,client,MENU_TIME_FOREVER);
}

public HelpMenu_Selected(Handle:menu,MenuAction:action,client,selection)
{
    if(action==MenuAction_Select)
    {
        EmitSoundToClient(client,buttonWav);

        decl String:SelectionInfo[SMALL_STRING_LENGTH];
        GetMenuItem(menu,selection,SelectionInfo,sizeof(SelectionInfo));

        new command=StringToInt(SelectionInfo);
        if (command >= 0 && command < HELPCOMMAND_COUNT)
            HelpMenu_Command(client,command);
    }
    else if (action == MenuAction_End)
        CloseHandle(menu);
}

public HelpMenu_Command(client,command)
{
    new Handle:helpCommandHandle=HelpItem(command);

    decl String:name[NAME_STRING_LENGTH];
    GetArrayString(helpCommandHandle,0,name,sizeof(name));

    decl String:phrase[300];
    GetArrayString(helpCommandHandle,1,phrase,sizeof(phrase));

    decl String:desc[300];
    Format(desc, sizeof(desc), "%T", phrase, client);

    new Handle:helpMenu_Command=CreateMenu(HM_Command_Select);
    SetMenuExitButton(helpMenu_Command,true);
    SetMenuExitBackButton(helpMenu_Command,true);
    SetMenuTitle(helpMenu_Command, "%T", "HelpItemTitle", client, name, desc);

    AddMenuItemT(helpMenu_Command,"backtohelp","BackToHelp", client);
    DisplayMenu(helpMenu_Command,client,MENU_TIME_FOREVER);
}

public HM_Command_Select(Handle:menu,MenuAction:action,client,selection)
{
    if (action == MenuAction_Select)
    {
        EmitSoundToClient(client,buttonWav);
        Help(client);
    }
    else if (action == MenuAction_End)
    {
        CloseHandle(menu);
        if (selection == MenuEnd_ExitBack)
        {
            EmitSoundToClient(client,buttonWav);
            Help(client);
        }
    }
    else if (action == MenuAction_End)
        CloseHandle(menu);
}

