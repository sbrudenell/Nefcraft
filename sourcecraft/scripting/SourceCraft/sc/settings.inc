/**
 * vim: set ai et ts=4 sw=4 syntax=sourcepawn :
 * File: settings.inc
 * Description: Defines the settings bits for SourceCraft
 * Author(s): Naris (Murray Wilson)
 */
 
#if defined _settings_included
 #endinput
#endif
#define _settings_included

enum Settings (<<= 1)
{
    NotSet = 0,
    Display_Defense = 1,
    Display_Damage_Done,
    Display_Kills,
    Display_Ultimate,
    Display_Misc_Message,
    Display_Enemy_Defended,
    Display_Damage_Taken,
    Display_Deaths,
    Display_Enemy_Ultimate,
    Display_Enemy_Message,
    Display_XP,
    Display_Crystals,
    Display_Vespene,
    Display_Energy,
    Display_Details,
    Reduce_Queasiness,
    Remove_Queasiness,
    Disable_Beacons,
    Disable_OBeacons,
    Disable_Effects,
    Display_Widescreen,
    Enable_Alternate_HUD,
    Disable_HUD,
    Debug_Condition,
    Debug_Speed
};

enum Settings_Cookies
{
    Display_Defense_Cookie = 0,
    Display_Damage_Done_Cookie,
    Display_Kills_Cookie,
    Display_Ultimate_Cookie,
    Display_Misc_Message_Cookie,
    Display_Enemy_Defended_Cookie,
    Display_Damage_Taken_Cookie,
    Display_Deaths_Cookie,
    Display_Enemy_Ultimate_Cookie,
    Display_Enemy_Message_Cookie,
    Display_XP_Cookie,
    Display_Crystals_Cookie,
    Display_Vespene_Cookie,
    Display_Energy_Cookie,
    Display_Details_Cookie,
    Reduce_Queasiness_Cookie,
    Remove_Queasiness_Cookie,
    Disable_Beacons_Cookie,
    Disable_OBeacons_Cookie,
    Disable_Effects_Cookie,
    Display_Widescreen_Cookie,
    Enable_Alternate_HUD_Cookie,
    Disable_HUD_Cookie,
    Debug_Condition_Cookie,
    Debug_Speed_Cookie
};

stock const String:g_strCookieName[Settings_Cookies][] = {
    "sc_display_defense",
    "sc_display_damage_done",
    "sc_display_kills",
    "sc_display_ultimate",
    "sc_display_misc_message",
    "sc_display_enemy_defended",
    "sc_display_damage_taken",
    "sc_display_deaths",
    "sc_display_enemy_ultimate",
    "sc_display_enemy_message",
    "sc_display_xp",
    "sc_display_crystals",
    "sc_display_vespene",
    "sc_display_energy",
    "sc_display_details",
    "sc_reduce_queasiness",
    "sc_remove_queasiness",
    "sc_disable_beacons",
    "sc_disable_obeacons",
    "sc_disable_effects",
    "sc_display_widescreen",
    "sc_enable_alternate_hud",
    "sc_disable_hud",
    "sc_debug_condition",
    "sc_debug_speed"
};

stock const String:g_strCookiePrompt[Settings_Cookies][] = {
    "DisplayDefense",
    "DisplayDamageDone",
    "DisplayKills",
    "DisplayUltimate",
    "DisplayMiscMessage",
    "DisplayEnemyDefended",
    "DisplayDamageTaken",
    "DisplayDeaths",
    "DisplayEnemyUltimate",
    "DisplayEnemyMessage",
    "DisplayXP",
    "DisplayCrystals",
    "DisplayVespene",
    "DisplayEnergy",
    "DisplayDetails",
    "ReduceQueasiness",
    "RemoveQueasiness",
    "DisableBeacons",
    "DisableOBeacons",
    "DisableEffects",
    "DisplayWidescreen",
    "EnableAltHUD",
    "DisableHUD",
    "DebugCondition",
    "DebugSpeed"
};

stock const bool:g_bCookieYesIsFalse[Settings_Cookies] = {
    false,
    false,
    false,
    false,
    false,
    false,
    false,
    false,
    false,
    false,
    false,
    false,
    false,
    false,
    false,
    true,
    true,
    false,
    false,
    false,
    true,
    true,
    false,
    true,
    true
};

