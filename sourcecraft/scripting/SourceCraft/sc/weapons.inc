/**
 * vim: set ai et ts=4 sw=4 syntax=sourcepawn :
 * File: weapons.inc
 * Description: Weapons related functions.
 * Author(s): -=|JFH|=-Naris (Murray Wilson)
 */

#if defined _scweapons_included
 #endinput
#endif
#define _scweapons_included

#include "range"

stock bool:GetWeapon(Handle:event, index,
                     String:buffer[], buffersize)
{
    new bool:is_equipment;

    buffer[0] = 0;
    GetEventString(event, "weapon", buffer, buffersize);

    if (buffer[0] == '\0' && index && IsPlayerAlive(index))
    {
        is_equipment = true;
        GetClientWeapon(index, buffer, buffersize);
    }
    else
        is_equipment = false;

    return is_equipment;
}

stock bool:IsEquipmentMelee(const String:weapon[])
{
    switch (GameType)
    {
        case cstrike:
        {
            return StrEqual(weapon,"weapon_knife");
        }
        case dod:
        {
            return (StrEqual(weapon,"weapon_amerknife") ||
                    StrEqual(weapon,"weapon_spade"));
        }
        case tf2:
        {
            return (StrEqual(weapon,"tf_weapon_knife") ||
                    StrEqual(weapon,"tf_weapon_shovel") ||
                    StrEqual(weapon,"tf_weapon_wrench") ||
                    StrEqual(weapon,"tf_weapon_bat") ||
                    StrEqual(weapon,"tf_weapon_bat_wood") ||
                    StrEqual(weapon,"tf_weapon_bonesaw") ||
                    StrEqual(weapon,"tf_weapon_bottle") ||
                    StrEqual(weapon,"tf_weapon_club") ||
                    StrEqual(weapon,"tf_weapon_fireaxe") ||
                    StrEqual(weapon,"tf_weapon_fists") ||
                    StrEqual(weapon,"tf_weapon_sword") ||
			        StrEqual(weapon,"tf_weapon_bat_fish") ||
			        StrEqual(weapon,"tf_weapon_robot_arm") ||
			        StrEqual(weapon,"tf_weapon_stickbomb") ||
			        StrEqual(weapon,"tf_wearable_item_demoshield"));
        }
    }
    return false;
}

stock bool:IsDamageFromMelee(const String:weapon[])
{
    switch (GameType)
    {
        case cstrike:
        {
            return StrEqual(weapon,"weapon_knife");
        }
        case dod:
        {
            return (StrEqual(weapon,"amerknife") ||
                    StrEqual(weapon,"spade") ||
                    StrEqual(weapon,"punch"));
        }
        case tf2:
        {
            return (StrEqual(weapon,"knife") ||
                    StrEqual(weapon,"shovel") ||
                    StrEqual(weapon,"wrench") ||
                    StrEqual(weapon,"bat") ||
                    StrEqual(weapon,"bonesaw") ||
                    StrEqual(weapon,"bottle") ||
                    StrEqual(weapon,"club") ||
                    StrEqual(weapon,"fireaxe") ||
                    StrEqual(weapon,"axtinguisher") ||
                    StrEqual(weapon,"fists") ||
                    StrEqual(weapon,"sandman") ||
                    StrEqual(weapon,"pickaxe") ||
                    StrEqual(weapon,"sword") ||
                    StrEqual(weapon,"demoshield") ||
                    StrEqual(weapon,"bear_claws") ||
                    StrEqual(weapon,"steel_fists") ||
                    StrEqual(weapon,"ullapool_caber") ||
                    StrEqual(weapon,"amputator") ||
                    StrEqual(weapon,"candy_cane") ||
                    StrEqual(weapon,"boston_basher")   ||
                    StrEqual(weapon,"back_scratcher") ||
                    StrEqual(weapon,"wrench_jag") ||
                    StrEqual(weapon,"taunt_scout") ||
                    StrEqual(weapon,"taunt_sniper") ||
                    StrEqual(weapon,"taunt_pyro") ||
                    StrEqual(weapon,"taunt_demoman") ||
                    StrEqual(weapon,"taunt_heavy") ||
                    StrEqual(weapon,"taunt_spy") ||
                    StrEqual(weapon,"taunt_soldier"));
        }
    }
    return false;
}

stock bool:IsEquipmentGrenadeOrRocket(const String:weapon[])
{
    switch (GameType)
    {
        case cstrike:
        {
            return StrEqual(weapon,"hegrenade");
        }
        case dod:
        {
            return (StrEqual(weapon,"weapon_frag_us") ||
                    StrEqual(weapon,"weapon_frag_us",false) ||
                    StrEqual(weapon,"weapon_frag_ger",false) ||
                    StrEqual(weapon,"weapon_riflegren_us") ||
                    StrEqual(weapon,"weapon_riflegren_ger") ||
                    StrEqual(weapon,"weapon_bazooka") ||
                    StrEqual(weapon,"weapon_pschreck"));
        }
        case tf2:
        {
            return (StrEqual(weapon,"tf_weapon_rocketlauncher") ||
                    StrEqual(weapon,"tf_weapon_rocketlauncher_directhit") ||
                    StrEqual(weapon,"tf_weapon_grenadelauncher") ||
                    StrEqual(weapon,"tf_weapon_pipebomblauncher"));
        }
    }
    return false;
}

stock bool:IsDamageFromGrenadeOrRocket(const String:weapon[])
{
    switch (GameType)
    {
        case cstrike:
        {
            return StrEqual(weapon,"hegrenade");
        }
        case dod:
        {
            return (StrEqual(weapon,"weapon_frag_us") ||
                    StrEqual(weapon,"weapon_frag_us",false) ||
                    StrEqual(weapon,"weapon_frag_ger",false) ||
                    StrEqual(weapon,"weapon_riflegren_us") ||
                    StrEqual(weapon,"weapon_riflegren_ger") ||
                    StrEqual(weapon,"weapon_bazooka") ||
                    StrEqual(weapon,"weapon_pschreck"));
        }
        case tf2:
        {
            return (StrEqual(weapon,"tf_projectile_pipe") ||
                    StrEqual(weapon,"tf_projectile_pipe_remote") ||
                    StrEqual(weapon,"tf_projectile_rocket") ||
                    StrEqual(weapon,"tf_weapon_rocketlauncher") ||
                    StrEqual(weapon,"rocketlauncher_directhit") ||
                    StrEqual(weapon,"sticky_resistance") ||
                    StrEqual(weapon,"deflect_rocket") ||
                    StrEqual(weapon,"deflect_sticky") ||
                    StrEqual(weapon,"deflect_promode") ||
                    StrEqual(weapon,"taunt_soldier") ||
                    StrEqual(weapon,"tf_pumpkin_bomb"));
        }
    }
    return false;
}

stock bool:IsMelee(const String:weapon[], bool:is_equipment, index, victim, Float:range=100.0)
{
    if (is_equipment)
    {
        if (IsEquipmentMelee(weapon))
            return IsInRange(index,victim,range);
        else
            return false;
    }
    else
        return IsDamageFromMelee(weapon);
}

stock bool:IsNotMelee(const String:weapon[], bool:is_equipment, index, victim, Float:range=100.0)
{
    if (is_equipment)
    {
        if (IsEquipmentMelee(weapon))
            return false;
        else
            return !IsInRange(index,victim,range);
    }
    else
        return !IsDamageFromMelee(weapon);
}

stock bool:IsGrenadeOrRocket(const String:weapon[], bool:is_equipment)
{
    return (is_equipment) ? IsEquipmentGrenadeOrRocket(weapon)
                          : IsDamageFromGrenadeOrRocket(weapon);
}

