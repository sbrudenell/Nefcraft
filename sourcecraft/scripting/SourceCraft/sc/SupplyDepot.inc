/**
 * vim: set ai et ts=4 sw=4 syntax=sourcepawn :
 * File: SupplyDepot.inc
 * Description: The Terran Supply Depot upgrade for SourceCraft.
 * Author(s): -=|JFH|=-Naris
 */

#if defined _SupplyDepot_included
 #endinput
#endif
#define _SupplyDepot_included

#include <weapons>

#undef REQUIRE_EXTENSIONS
#include <tf2_ammo>
#include <dod>
#define REQUIRE_EXTENSIONS

enum SupplyTypes ( <<= 1) { SupplyDefault = 1, SupplyPrimary, SupplySecondary, SupplyMetal, SupplyGrenade, SupplySmoke };

stock SupplyAmmo(client, amount, const String:source[],
                 SupplyTypes:types=SupplyDefault, heavyFactor=10)
{
    new ammoSupplied = false;
    if (amount > 0)
    {
        if (GameType == tf2)
        {
            switch (TF2_GetPlayerClass(client))
            {
                case TFClass_Scout: 
                {
                    if (!types || (types & SupplyDefault) || (types & SupplyPrimary))
                    {
                        new ammo = TF2_GetAmmoAmount(client, Primary);
                        if (ammo < 64.0)
                        {
                            TF2_SetAmmoAmount(client, ammo + amount, Primary);
                            ammoSupplied = true;
                        }
                        else if (types & SupplyDefault)
                            types |= SupplySecondary;
                    }

                    if (types & SupplySecondary)
                    {
                        new ammo = TF2_GetAmmoAmount(client, Secondary);
                        if (ammo < 72.0)
                        {
                            TF2_SetAmmoAmount(client, ammo + amount, Secondary);
                            ammoSupplied = true;
                        }
                        else if (types & SupplyDefault)
                            types |= SupplySecondary;
                    }
                }
                case TFClass_Sniper: 
                {
                    if (!types || (types & SupplyDefault) || (types & SupplyPrimary))
                    {
                        new ammo = TF2_GetAmmoAmount(client, Primary);
                        if (ammo < 50.0)
                        {
                            TF2_SetAmmoAmount(client, ammo + amount, Primary);
                            ammoSupplied = true;
                        }
                        else if (types & SupplyDefault)
                            types |= SupplySecondary;
                    }

                    if (types & SupplySecondary)
                    {
                        new ammo = TF2_GetAmmoAmount(client, Secondary);
                        if (ammo < 150.0)
                        {
                            TF2_SetAmmoAmount(client, ammo + amount, Secondary);
                            ammoSupplied = true;
                        }
                    }
                }
                case TFClass_Soldier: 
                {
                    if (!types || (types & SupplyDefault) || (types & SupplyPrimary))
                    {
                        new ammo = TF2_GetAmmoAmount(client, Primary);
                        if (ammo < 40.0)
                        {
                            TF2_SetAmmoAmount(client, ammo + amount, Primary);
                            ammoSupplied = true;
                        }
                        else if (types & SupplyDefault)
                            types |= SupplySecondary;
                    }

                    if (types & SupplySecondary)
                    {
                        new ammo = TF2_GetAmmoAmount(client, Secondary);
                        if (ammo < 64.0)
                        {
                            TF2_SetAmmoAmount(client, ammo + amount, Secondary);
                            ammoSupplied = true;
                        }
                    }
                }
                case TFClass_DemoMan: 
                {
                    if (!types || (types & SupplyDefault) || (types & SupplyPrimary))
                    {
                        new ammo = TF2_GetAmmoAmount(client, Primary);
                        if (ammo < 60.0)
                        {
                            TF2_SetAmmoAmount(client, ammo + amount, Primary);
                            ammoSupplied = true;
                        }
                        else if (types & SupplyDefault)
                            types |= SupplySecondary;
                    }

                    if (types & SupplySecondary)
                    {
                        new ammo = TF2_GetAmmoAmount(client, Secondary);
                        if (ammo < 80.0)
                        {
                            TF2_SetAmmoAmount(client, ammo + amount, Secondary);
                            ammoSupplied = true;
                        }
                    }
                }
                case TFClass_Medic: 
                {
                    if (!types || (types & SupplyDefault) || (types & SupplyPrimary))
                    {
                        new ammo = TF2_GetAmmoAmount(client, Primary);
                        if (ammo < 300.0)
                        {
                            TF2_SetAmmoAmount(client, ammo + (amount * heavyFactor), Primary);
                            ammoSupplied = true;
                        }
                    }
                }
                case TFClass_Heavy: 
                {
                    if (!types || (types & SupplyDefault) || (types & SupplyPrimary))
                    {
                        new ammo = TF2_GetAmmoAmount(client, Primary);
                        if (ammo < 400.0)
                        {
                            TF2_SetAmmoAmount(client, ammo + (amount * heavyFactor), Primary);
                            ammoSupplied = true;
                        }
                        else if (types & SupplyDefault)
                            types |= SupplySecondary;
                    }

                    if (types & SupplySecondary)
                    {
                        new ammo = TF2_GetAmmoAmount(client, Secondary);
                        if (ammo < 64.0)
                        {
                            TF2_SetAmmoAmount(client, ammo + amount, Secondary);
                            ammoSupplied = true;
                        }
                    }

                }
                case TFClass_Pyro: 
                {
                    if (!types || (types & SupplyDefault) || (types & SupplyPrimary))
                    {
                        new ammo = TF2_GetAmmoAmount(client, Primary);
                        if (ammo < 400.0)
                        {
                            TF2_SetAmmoAmount(client, ammo + (amount * heavyFactor), Primary);
                            ammoSupplied = true;
                        }
                        else if (types & SupplyDefault)
                            types |= SupplySecondary;
                    }

                    if (types & SupplySecondary)
                    {
                        new ammo = TF2_GetAmmoAmount(client, Secondary);
                        if (ammo < 64.0)
                        {
                            TF2_SetAmmoAmount(client, ammo + amount, Secondary);
                            ammoSupplied = true;
                        }
                    }
                }
                case TFClass_Spy: 
                {
                    if (!types || (types & SupplyDefault) || (types & SupplyPrimary))
                    {
                        new ammo = TF2_GetAmmoAmount(client, Primary);
                        if (ammo < 48.0)
                        {
                            TF2_SetAmmoAmount(client, ammo + amount, Primary);
                            ammoSupplied = true;
                        }
                    }
                }
                case TFClass_Engineer: // Gets Metal instead of Ammo
                {
                    if (!types || (types & SupplyDefault) || (types & SupplyMetal))
                    {
                        new ammo = TF2_GetAmmoAmount(client, Metal);
                        if (ammo < 400.0)
                        {
                            TF2_SetAmmoAmount(client, ammo + (amount * heavyFactor), Metal);
                            ammoSupplied = true;
                        }
                        else if (types & SupplyDefault)
                            types |= SupplyPrimary;
                    }

                    if (types & SupplyPrimary)
                    {
                        new ammo = TF2_GetAmmoAmount(client, Primary);
                        if (ammo < 64.0)
                        {
                            TF2_SetAmmoAmount(client, ammo + amount, Primary);
                            ammoSupplied = true;
                        }
                        else if (types & SupplyDefault)
                            types |= SupplySecondary;
                    }

                    if (types & SupplySecondary)
                    {
                        new ammo = TF2_GetAmmoAmount(client, Secondary);
                        if (ammo < 200.0)
                        {
                            TF2_SetAmmoAmount(client, ammo + amount, Secondary);
                            ammoSupplied = true;
                        }
                    }
                }
                default:
                {
                    if (!types || (types & SupplyDefault) || (types & SupplyPrimary))
                    {
                        new ammo = TF2_GetAmmoAmount(client, Primary);
                        if (ammo < 100.0)
                        {
                            TF2_SetAmmoAmount(client, ammo + amount, Primary);
                            ammoSupplied = true;
                        }
                        else if (types & SupplyDefault)
                            types |= SupplySecondary;
                    }

                    if (types & SupplySecondary)
                    {
                        new ammo = TF2_GetAmmoAmount(client, Secondary);
                        if (ammo < 100.0)
                        {
                            TF2_SetAmmoAmount(client, ammo + amount, Secondary);
                            ammoSupplied = true;
                        }
                    }
                }
            }
        }
        else if (GameType == dod)
        {
            new Max[4];
            new DODWeapon:Slot[4];
            new DODTeam:team = DODTeam:GetClientTeam(client);
            switch (DOD_GetPlayerClass(client))
            {
                case DODClass_Rifleman:
                {
                    if (team == DODTeam_Allies) 
                    {
                        Slot[0] = DODWeapon_Garand;
                        Max[0]  = 99;
                        Slot[1] = DODWeapon_Colt;
                        Max[1]  = 99;
                        Slot[2] = DODWeapon_USRifleGrenade;
                        Max[2]  = 4;
                        Slot[3] = DODWeapon_None;
                        Max[3]  = 0;
                    }
                    else
                    {
                        Slot[0] = DODWeapon_K98;
                        Max[0]  = 99;
                        Slot[1] = DODWeapon_P38;
                        Max[1]  = 99;
                        Slot[2] = DODWeapon_GermanRifleGrenade;
                        Max[2]  = 4;
                        Slot[3] = DODWeapon_None;
                        Max[3]  = 0;
                    }
                }
                case DODClass_Assault:
                {
                    if (team == DODTeam_Allies) 
                    {
                        Slot[0] = DODWeapon_Thompson;
                        Max[0]  = 999;
                        Slot[1] = DODWeapon_Colt;
                        Max[1]  = 99;
                        Slot[2] = DODWeapon_FragGrenade;
                        Max[2]  = 4;
                        Slot[3] = DODWeapon_USSmokeGrenade;
                        Max[3]  = 4;
                    }
                    else
                    {
                        Slot[0] = DODWeapon_MP40;
                        Max[0]  = 999;
                        Slot[1] = DODWeapon_P38;
                        Max[1]  = 99;
                        Slot[2] = DODWeapon_StickGrenade;
                        Max[2]  = 4;
                        Slot[3] = DODWeapon_GermanSmokeGrenade;
                        Max[3]  = 4;
                    }
                }
                case DODClass_Support:
                {
                    if (team == DODTeam_Allies) 
                    {
                        Slot[0] = DODWeapon_BAR;
                        Max[0]  = 999;
                        Slot[1] = DODWeapon_Colt;
                        Max[1]  = 99;
                        Slot[2] = DODWeapon_FragGrenade;
                        Max[2]  = 4;
                        Slot[3] = DODWeapon_None;
                        Max[3]  = 0;
                    }
                    else
                    {
                        Slot[0] = DODWeapon_MP44;
                        Max[0]  = 999;
                        Slot[1] = DODWeapon_P38;
                        Max[1]  = 99;
                        Slot[2] = DODWeapon_StickGrenade;
                        Max[2]  = 4;
                        Slot[3] = DODWeapon_None;
                        Max[3]  = 0;
                    }
                }
                case DODClass_Sniper:
                {
                    if (team == DODTeam_Allies) 
                    {
                        Slot[0] = DODWeapon_Springfield;
                        Max[0]  = 99;
                        Slot[1] = DODWeapon_Colt;
                        Max[1]  = 99;
                        Slot[2] = DODWeapon_None;
                        Max[2]  = 0;
                        Slot[3] = DODWeapon_None;
                        Max[3]  = 0;
                    }
                    else
                    {
                        Slot[0] = DODWeapon_ScopedK98;
                        Max[0]  = 99;
                        Slot[1] = DODWeapon_P38;
                        Max[1]  = 99;
                        Slot[2] = DODWeapon_None;
                        Max[2]  = 0;
                        Slot[3] = DODWeapon_None;
                        Max[3]  = 0;
                    }
                }
                case DODClass_MachineGunner:
                {
                    if (team == DODTeam_Allies) 
                    {
                        Slot[0] = DODWeapon_30CalMG;
                        Max[0]  = 999;
                        Slot[1] = DODWeapon_Colt;
                        Max[1]  = 99;
                        Slot[2] = DODWeapon_None;
                        Max[2]  = 0;
                        Slot[3] = DODWeapon_None;
                        Max[3]  = 0;
                    }
                    else
                    {
                        Slot[0] = DODWeapon_MG42;
                        Max[0]  = 999;
                        Slot[1] = DODWeapon_P38;
                        Max[1]  = 999;
                        Slot[2] = DODWeapon_None;
                        Max[2]  = 0;
                        Slot[3] = DODWeapon_None;
                        Max[3]  = 0;
                    }
                }
                case DODClass_Rocketman:
                {
                    if (team == DODTeam_Allies) 
                    {
                        Slot[0] = DODWeapon_Bazooka;
                        Max[0]  = 99;
                        Slot[1] = DODWeapon_M1Carbine;
                        Max[1]  = 999;
                        Slot[2] = DODWeapon_None;
                        Max[2]  = 0;
                        Slot[3] = DODWeapon_None;
                        Max[3]  = 0;
                    }
                    else
                    {
                        Slot[0] = DODWeapon_Panzerschrek;
                        Max[0]  = 99;
                        Slot[1] = DODWeapon_C96;
                        Max[1]  = 999;
                        Slot[2] = DODWeapon_None;
                        Max[2]  = 0;
                        Slot[3] = DODWeapon_None;
                        Max[3]  = 0;
                    }
                }
            }

            new m_iAmmo = FindSendPropOffs("CDODPlayer", "m_iAmmo");
            for (new i=0; i < sizeof(Slot); i++)
            {
                if (i == 0 && !(!types || (types & SupplyDefault) || (types & SupplyPrimary)))
                    continue;
                else if (i == 1 && !(types & SupplySecondary))
                    continue;
                else if (i == 2 && !(types & SupplyGrenade))
                    continue;
                else if (i == 3 && !(types & SupplySmoke))
                    continue;
                else
                {
                    new DODWeapon:weapon = Slot[i];
                    if (weapon > DODWeapon_None)
                    {
                        new offset = m_iAmmo + DOD_AmmoOffset[weapon];
                        new ammo = GetEntData(client, offset);
                        if (ammo < Max[i])
                        {
                            SetEntData(client, offset, ammo + amount, 4, true);
                            ammoSupplied = true;
                        }
                    }
                }
            }
        }
        else
        {
            new curWeapon = GetActiveWeapon(client);
            if (curWeapon > 0)
            {
                SetClip(curWeapon, 5);
                ammoSupplied = true;
            }
        }
    }

    if (ammoSupplied)
    {
        DisplayMessage(client,Display_Misc_Message,
                       "You have received ammo from the {blue}%s{default}.",
                       source);
    }
}
