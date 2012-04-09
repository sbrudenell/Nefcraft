/**
 * vim: set ai et ts=4 sw=4 syntax=sourcepawn :
 * File: tf2_weapon.inc
 * Description: Functions to return information about TF2 weapons.
 * Author(s): Naris (Murray Wilson)
 */
 
#if defined _tf2_weapon_included
 #endinput
#endif
#define _tf2_weapon_included

#include <tf2_stocks>

stock Handle:TF2WeaponConf = INVALID_HANDLE;
stock Handle:hTF2GiveNamedItem = INVALID_HANDLE;
stock Handle:hTF2WeaponEquip = INVALID_HANDLE;

stock LoadWeaponConf()
{
    TF2WeaponConf = LoadGameConfigFile("givenameditem.games");

    StartPrepSDKCall(SDKCall_Player);
    //PrepSDKCall_SetFromConf(TF2WeaponConf, SDKConf_Virtual, "GiveNamedItem");
    PrepSDKCall_SetFromConf(TF2WeaponConf, SDKConf_Signature, "GiveNamedItem");
    PrepSDKCall_AddParameter(SDKType_String, SDKPass_Pointer);
    PrepSDKCall_AddParameter(SDKType_PlainOldData, SDKPass_Plain);
    PrepSDKCall_AddParameter(SDKType_PlainOldData, SDKPass_Plain);
    PrepSDKCall_SetReturnInfo(SDKType_CBaseEntity, SDKPass_Plain);
    hTF2GiveNamedItem = EndPrepSDKCall();

    StartPrepSDKCall(SDKCall_Player);
    //PrepSDKCall_SetFromConf(TF2WeaponConf, SDKConf_Virtual, "WeaponEquip");
    PrepSDKCall_SetFromConf(TF2WeaponConf, SDKConf_Signature, "WeaponEquip");
    PrepSDKCall_AddParameter(SDKType_CBaseEntity, SDKPass_Pointer);
    hTF2WeaponEquip = EndPrepSDKCall();
}

stock bool:TF2_GiveNamedItem(client, const String:item[])
{
    if (!hTF2GiveNamedItem)
        LoadWeaponConf();

    return SDKCall(hTF2GiveNamedItem, client, item, 0, 0);
}

stock bool:TF2_WeaponEquip(client, entity)
{
    if (!hTF2WeaponEquip)
        LoadWeaponConf();

    return SDKCall(hTF2WeaponEquip, client, entity);
}

/**
 * Removes all actual weapons from a client
 *  Leave the spys pda and engineers destroy devices
 *
 * @param client        Player's index.
 * @noreturn
 */
stock TF2_RemoveOnlyWeapons(client)
{
    new lastSlot;
    new TFClassType:class = TF2_GetPlayerClass(client);
    if (class == TFClass_Spy)
    {
        ClientCommand(client, "slot4");
        lastSlot = 2;
    }
    else if (class == TFClass_Engineer)
    {
        ClientCommand(client, "slot5");
        lastSlot = 3;
    }
    else
        lastSlot = 5;

    for (new i = 0; i <= lastSlot; i++)
    {
        TF2_RemoveWeaponSlot(client, i);
    }
}

/**
 * Removes all non-melee weapons from a client
 *  Leave the spys pda and engineers destroy devices, if pda is true
 *
 * @param client        Player's index.
 * @param pda           Set true to include spy's pda & engies build & destroy.
 * @noreturn
 */
stock TF2_RemoveNonMelee(client, bool:pda=false)
{
    for (new i = 0; i <= 1; i++)
    {
        TF2_RemoveWeaponSlot(client, i);
    }

    // slot3 (#2) is the melee weapon
    ClientCommand(client, "slot3");

    if (!pda)
    {
        for (new i = 3; i <= 5; i++)
        {
            TF2_RemoveWeaponSlot(client, i);
        }
    }
}

/**
 * Gives a named weapon to a client
 *
 * @param client		Player's index.
 * @param weapon		Weapon name
 * @return				False if weapon could not be created, true on success
 * @error				Invalid client index or lack of mod support
 */
stock bool:TF2_GivePlayerWeapon(client, const String:weapon[])
{
    //new weaponIndex = GivePlayerItem(client, weapon);
    new weaponIndex = TF2_GiveNamedItem(client, weapon);
    if (weaponIndex == -1)
        return false;
    else
    {
        EquipPlayerWeapon(client, weaponIndex);
        return true;
    }
}

/**
 * Equips a client with a class's weapons. This does not remove existing weapons.
 *
 * Note: 	Some class specific items such tf_weapon_pda_engineer_build are only given 
 *			if the client is the correct class.
 *
 * @param client		Player's index.
 * @param class			TFClasssType class symbol.
 * @noreturn
 */
stock TF2_EquipPlayerClassWeapons(client, TFClassType:class, bool:onlyWeapons=false)
{
    switch(class)
    {
        case TFClass_Scout:
        {
            TF2_GivePlayerWeapon(client, "tf_weapon_scattergun");
            TF2_GivePlayerWeapon(client, "tf_weapon_pistol_scout");
            TF2_GivePlayerWeapon(client, "tf_weapon_bat");
        }
        case TFClass_Sniper:
        {
            TF2_GivePlayerWeapon(client, "tf_weapon_sniperrifle");
            TF2_GivePlayerWeapon(client, "tf_weapon_smg");
            TF2_GivePlayerWeapon(client, "tf_weapon_club");
        }
        case TFClass_Soldier:
        {
            TF2_GivePlayerWeapon(client, "tf_weapon_rocketlauncher");
            TF2_GivePlayerWeapon(client, "tf_weapon_shotgun_soldier");
            TF2_GivePlayerWeapon(client, "tf_weapon_shovel");
        }
        case TFClass_DemoMan:
        {
            TF2_GivePlayerWeapon(client, "tf_weapon_pipebomblauncher");
            TF2_GivePlayerWeapon(client, "tf_weapon_grenadelauncher");
            TF2_GivePlayerWeapon(client, "tf_weapon_bottle");
        }
        case TFClass_Medic:
        {
            TF2_GivePlayerWeapon(client, "tf_weapon_syringegun_medic");
            TF2_GivePlayerWeapon(client, "tf_weapon_medigun");
            TF2_GivePlayerWeapon(client, "tf_weapon_bonesaw");
        }
        case TFClass_Heavy:
        {
            TF2_GivePlayerWeapon(client, "tf_weapon_minigun");
            TF2_GivePlayerWeapon(client, "tf_weapon_shotgun_hwg");
            TF2_GivePlayerWeapon(client, "tf_weapon_fists");
        }
        case TFClass_Pyro:
        {
            TF2_GivePlayerWeapon(client, "tf_weapon_flamethrower");
            TF2_GivePlayerWeapon(client, "tf_weapon_shotgun_pyro");
            TF2_GivePlayerWeapon(client, "tf_weapon_fireaxe");
        }
        case TFClass_Spy:
        {
            TF2_GivePlayerWeapon(client, "tf_weapon_revolver");
            TF2_GivePlayerWeapon(client, "tf_weapon_knife");

            if (onlyWeapons || TF2_GetPlayerClass(client) != TFClass_Spy)
                return;

            TF2_GivePlayerWeapon(client, "tf_weapon_pda_spy");
        }
        case TFClass_Engineer:
        {
            TF2_GivePlayerWeapon(client, "tf_weapon_shotgun_primary");
            TF2_GivePlayerWeapon(client, "tf_weapon_pistol");
            TF2_GivePlayerWeapon(client, "tf_weapon_wrench");

            if (onlyWeapons || TF2_GetPlayerClass(client) != TFClass_Engineer)
                return;

            TF2_GivePlayerWeapon(client, "tf_weapon_pda_engineer_build");
            TF2_GivePlayerWeapon(client, "tf_weapon_pda_engineer_destroy");
        }
    }
}
