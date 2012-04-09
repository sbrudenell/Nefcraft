/**
 * vim: set ai et ts=4 sw=4 syntax=sourcepawn :
 * File: menumode.inc
 * Description: Define the MenuMode enum for menu and info displays.
 * Author(s): -=|JFH|=-Naris (Murray Wilson)
 */

#if defined _menumode_included
 #endinput
#endif
#define _menumode_included

enum MenuMode (<<= 1)
{
    NoMode = 0,
    InfoMode = 1,
    AdminMode,
    QueryMode,
    UpgradeMode,
    ChangeRaceMode,
    PurchaseItemMode,
    FromAdminMenu,
    FromMainMenu,
    FromShopMenu,
    FromRaceMenu,
    FromFactionMenu,
    FromPlayerInfoMenu,
    FromPlayerInfo
};

#define EnableMenuExitBack (FromAdminMenu|FromMainMenu|FromShopMenu|FromRaceMenu|FromFactionMenu|FromPlayerInfoMenu|FromPlayerInfo)
