/*
 * vim: set ai et ts=4 sw=4 syntax=sourcepawn :
 * File: changesettings.inc
 * Description: This file handles player settings.
 * Author(s): Naris (Murray Wilson)
 */

#include <clientprefs>
#include "sc/settings"

//#define USE_CLIENTPREFS
#if defined USE_CLIENTPREFS
new Handle:g_hCookie[Settings_Cookies];
new g_bCookieSelected[MAXPLAYERS];

InitCookies()
{
    for (new Settings_Cookies:i = Display_Defense_Cookie; i < Settings_Cookies; i++)
    {
        g_hCookie[i] = RegClientCookie(g_strCookieName[i], g_strCookiePrompt[i], CookieAccess_Public);
        SetCookiePrefabMenu(g_hCookie[i],  CookieMenu_YesNo_Int, g_strCookiePrompt[i], CookieSelected, _:i);
    }

    // Hook the TextMsg displayed when the user changes a cookie.
    HookUserMessage(GetUserMessageId("TextMsg"), CookieTextHook);
}

public OnClientCookiesCached(client)
{
    // SourceCraft DataBase info takes precedence over clientprefs
    if (GetDatabaseLoaded(client) < DataLoaded)
    {
        decl String:strCookie[2];
        for (new Settings_Cookies:i = Display_Defense_Cookie; i < Settings_Cookies; i++)
        {
            GetClientCookie(client, g_hCookie[i], strCookie, sizeof(strCookie));
            if (g_bCookieYesIsFalse[i])
            {
                if (strCookie[0] != '1')
                    g_ClientSettingsBits[client] |= Settings:(1 << _:i);
            }
            else
            {
                if (strCookie[0] == '1')
                    g_ClientSettingsBits[client] |= Settings:(1 << _:i);
            }
        }
    }
}

public CookieSelected(client, CookieMenuAction:action, any:info, String:buffer[], maxlen)
{
    if (action == CookieMenuAction_DisplayOption )
    {
        if (info >= 0 && info < Settings_Cookies)
        {
            // Translate the buffer
            Format(buffer, maxlen, "%T", g_strCookiePrompt[info], client);
        }
        else
            LogError("Invalid cookie %d requested", info);
    }
    else if (client > 0 && action == CookieMenuAction_SelectOption)
        g_bCookieSelected[client] = info+1; // We need 0 to be not selected.
}

public Action:CookieTextHook(UserMsg:msg_id, Handle:bf, const players[], playersNum, bool:reliable, bool:init)
{
    if (playersNum == 1)
    {
        new client = players[0];
        if (client > 0)
        {
            new info = g_bCookieSelected[client]-1;
            if (info >= 0)
            {
                if (info < _:Settings_Cookies)
                {
                    new String:text[LONG_STRING_LENGTH];
                    BfReadByte(bf);
                    BfReadString(bf, text, sizeof(text));
                    if (strncmp(text, "[SM]", 4) == 0 && StrContains(text, "\"sc_") >= 0)
                    {
                        decl String:strCookie[2];
                        GetClientCookie(client, g_hCookie[info], strCookie, sizeof(strCookie));
                        LogMessage("%N's Cookie %d was set to %s", client, info, strCookie);

                        if (g_bCookieYesIsFalse[info])
                        {
                            if (strCookie[0] != '1')
                                g_ClientSettingsBits[client] |= Settings:(1 << info);
                            else
                                g_ClientSettingsBits[client] &= Settings:(~(1 << info));
                        }
                        else
                        {
                            if (strCookie[0] == '1')
                                g_ClientSettingsBits[client] |= Settings:(1 << info);
                            else
                                g_ClientSettingsBits[client] &= Settings:(~(1 << info));
                        }
                    }
                }
                g_bCookieSelected[client] = 0;
            }
        }
    }
    return Plugin_Continue;
}
#else
InitCookies()
{
    // Don't use clientprefs until the SourceMod team can fix the crashing.
}
#endif

ChangeSettings(client, bool:fromMain)
{
    new Handle:SettingsMenu=CreateMenu(Settings_Select);
    SetMenuTitle(SettingsMenu,"[SC] %T", "ChooseSetting", client);
    SetMenuExitButton(SettingsMenu,true);
    if (fromMain)
        SetMenuExitBackButton(SettingsMenu,true);

    decl String:buf[12];
    new flags = _:g_ClientSettingsBits[client];
    for (new Settings_Cookies:i = Display_Defense_Cookie; i < Settings_Cookies; i++)
    {
        IntToString(_:i, buf, sizeof(buf));
        new bool:value = ((flags & (1 << _:i)) == _:NotSet);
        AddSetting(client, SettingsMenu, buf, g_strCookiePrompt[i],
                   g_bCookieYesIsFalse[i] ? !value : value);
    }

    DisplayMenu(SettingsMenu,client,MENU_TIME_FOREVER);
}

public Settings_Select(Handle:menu,MenuAction:action,client,selection)
{
    if (action == MenuAction_Select)
    {
        decl String:SelectionInfo[SMALL_STRING_LENGTH];
        GetMenuItem(menu,selection,SelectionInfo,sizeof(SelectionInfo));

        // Update the settings bits
        new info = StringToInt(SelectionInfo);
        new Settings:bit = Settings:(1 << info);
        g_ClientSettingsBits[client] ^= bit;

        // Set the cookie
        #if defined USE_CLIENTPREFS
            SetClientCookie(client, g_hCookie[Settings_Cookies:info],
                            (g_ClientSettingsBits[client] & bit) ? "1" : "0");
        #endif                        

        EmitSoundToClient(client,buttonWav);
        ChangeSettings(client,false);
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
}

AddSetting(client, Handle:SettingsMenu, const String:info[], const String:setting[], bool:value)
{
    decl String:buffer[ITEM_STRING_LENGTH];
    Format(buffer, sizeof(buffer), "[%s] %T", value ? "Y" : "N", setting, client);
    AddMenuItem(SettingsMenu,info,buffer);
}
