/**
 * vim: set ai et ts=4 sw=4 :
 * File: AirWeapons.sp
 * Description: The Air Weapons upgrade for SourceCraft.
 * Author(s): -=|JFH|=-Naris
 */

#if defined _AirWeapons_included
 #endinput
#endif
#define _AirWeapons_included

#include "effect/Lightning"
#include "effect/HaloSprite"
#include "effect/SendEffects"
#include "effect/FlashScreen"

stock SetupAirWeapons(bool:precache=false, bool:preload=false)
{
    SetupHaloSprite(precache, preload);
    SetupLightning(precache, preload);
}

bool:AirWeapons(race, upgrade, Handle:event, damage, victim_index, index,
                const Float:weaponsPercent[], const weaponsChance[])
{
    new weapons_level = GetUpgradeLevel(index,race,upgrade);
    if (weapons_level > 0)
    {
        if (!GetRestriction(index, Restriction_PreventUpgrades) &&
            !GetRestriction(index, Restriction_Stunned) &&
            !GetImmunity(victim_index,Immunity_HealthTaking) &&
            !GetImmunity(victim_index,Immunity_Upgrades) &&
            !IsInvulnerable(victim_index))
        {
            new energy = GetEnergy(index);
            new amount = GetUpgradeEnergy(race,upgrade);
            if (energy >= amount && GetRandomInt(1,100)<=weaponsChance[weapons_level])
            {
                decl String:weapon[64];
                new bool:is_equipment=GetWeapon(event,index,weapon,sizeof(weapon));
                if (!IsMelee(weapon, is_equipment,index,victim_index))
                {
                    new health_take = RoundFloat(float(damage)*weaponsPercent[weapons_level]);
                    if (health_take > 0)
                    {
                        new Float:indexLoc[3];
                        GetClientAbsOrigin(index, indexLoc);
                        indexLoc[2] += 50.0;

                        new Float:victimLoc[3];
                        GetClientAbsOrigin(victim_index, victimLoc);
                        victimLoc[2] += 50.0;

                        static const color[4] = { 100, 255, 55, 255 };
                        TE_SetupBeamPoints(indexLoc, victimLoc, Lightning(), HaloSprite(),
                                           0, 50, 1.0, 3.0,6.0,50,50.0,color,255);
                        TE_SendQEffectToAll(index, victim_index);
                        FlashScreen(victim_index,RGBA_COLOR_RED);

                        SetEnergy(index, energy-amount);
                        HurtPlayer(victim_index, health_take, index,
                                   "sc_air_weapons", .type=DMG_BULLET, 
                                   .in_hurt_event=true);
                        return true;
                    }
                }
            }
        }
    }
    return false;
}
