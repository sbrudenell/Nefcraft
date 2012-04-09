/**
 * vim: set ai et ts=4 sw=4 syntax=sourcepawn :
 * File: MeleeAttack.inc
 * Description: The Melee Attack (Adrenal Glands) Upgrade  for SourceCraft.
 * Author(s): -=|JFH|=-Naris
 */

#if defined _MeleeAttack_included
 #endinput
#endif
#define _MeleeAttack_included

#include "sc/weapons"

#include "effect/SendEffects"
#include "effect/FlashScreen"

stock bool:MeleeAttack(race, upgrade, level, Handle:event, damage, victim_index, index,
                       const Float:meleeAttackPercent[], const String:meleeAttackSound[],
                       const String:meleeAttackShort[], const String:meleeAttackName[]="")
{
    if (!GetRestriction(index, Restriction_PreventUpgrades) &&
        !GetRestriction(index, Restriction_Stunned) &&
        !GetImmunity(victim_index, Immunity_HealthTaking) &&
        !GetImmunity(victim_index, Immunity_Upgrades) &&
        !IsInvulnerable(victim_index))
    {
        decl String:weapon[64];
        new bool:is_equipment=GetWeapon(event,index,weapon,sizeof(weapon));
        if (IsMelee(weapon, is_equipment, index, victim_index))
        {
            new energy = GetEnergy(index);
            new amount = GetUpgradeEnergy(race,upgrade);
            if (energy >= amount)
            {
                new dmgamt = RoundFloat(float(damage)*meleeAttackPercent[level]);
                if (dmgamt > 0)
                {
                    new Float:Origin[3];
                    GetClientAbsOrigin(victim_index, Origin);
                    Origin[2] += 5;

                    PrepareSound(meleeAttackSound);
                    EmitSoundToAll(meleeAttackSound,victim_index);
                    TE_SetupSparks(Origin,Origin,255,1);
                    TE_SendEffectToAll();

                    SetEnergy(index, energy-amount);
                    FlashScreen(victim_index,RGBA_COLOR_RED);
                    HurtPlayer(victim_index, dmgamt, index,
                               meleeAttackShort, meleeAttackName,
                               .type=DMG_SLASH, .in_hurt_event=true);
                    return true;
                }
            }
        }
    }
    return false;
}

stock bool:UberMeleeAttack(race, upgrade, level, Handle:event, damage, victim_index, index,
                           const Float:meleeAttackPercent[], const String:meleeAttackSound[],
                           const String:meleeAttackShort[], const String:meleeAttackName[]="")
{
    decl String:weapon[64];
    new bool:is_equipment=GetWeapon(event,index,weapon,sizeof(weapon));
    if (IsMelee(weapon, is_equipment, index, victim_index))
    {
        new bool:isUber = IsInvulnerable(victim_index);
        if (!GetRestriction(index, Restriction_PreventUpgrades) &&
            !GetRestriction(index, Restriction_Stunned) &&
            (level >= 3 ||
             (!GetImmunity(victim_index,Immunity_HealthTaking) &&
              !GetImmunity(index, Immunity_Upgrades))) &&
            (level >= 4 || !isUber))
        {
            new energy = GetEnergy(index);
            new amount = GetUpgradeEnergy(race,upgrade);
            if (energy >= amount)
            {
                new dmgamt=RoundFloat(float(damage)*meleeAttackPercent[level]);
                if (isUber)
                    dmgamt /= 2;

                if (dmgamt > 0)
                {
                    new Float:Origin[3];
                    GetClientAbsOrigin(victim_index, Origin);
                    Origin[2] += 5;

                    PrepareSound(meleeAttackSound);
                    EmitSoundToAll(meleeAttackSound,victim_index);
                    TE_SetupSparks(Origin,Origin,255,1);
                    TE_SendEffectToAll();

                    SetEnergy(index, energy-amount);
                    FlashScreen(victim_index,RGBA_COLOR_RED);
                    HurtPlayer(victim_index, dmgamt, index,
                               meleeAttackShort, meleeAttackName,
                               .type=DMG_SLASH, .in_hurt_event=true);
                    return true;
                }
            }
        }
    }
    return false;
}
