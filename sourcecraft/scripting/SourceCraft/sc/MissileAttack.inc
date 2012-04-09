/**
 * vim: set ai et ts=4 sw=4 syntax=sourcepawn :
 * File: MissileAttack.inc
 * Description: The Missile Attack Upgrade for SourceCraft.
 * Author(s): -=|JFH|=-Naris
 */

#if defined _MissileAttack_included
 #endinput
#endif
#define _MissileAttack_included

#include <trace>

#include "sc/weapons"

#include "effect/CrystalSprite"
#include "effect/SendEffects"
#include "effect/FlashScreen"

stock SetupMissileAttack(const String:missileAttackSound[]="",
                         bool:precache=false, bool:preload=false)
{
    SetupCrystalSprite(precache, preload);

    if (missileAttackSound[0])
        SetupSound(missileAttackSound, true, precache, preload);
}

stock bool:MissileAttack(race, upgrade, level, Handle:event, damage, victim_index, index, sound_source,
                         bool:grenadesOnly, num_levels, const Float:missileAttackPercent[],
                         const missileAttackChance[], const String:missileAttackSound[],
                         const String:missileAttackShort[], const String:missileAttackName[]="")
{
    SetTraceCategory("Damage,Immunity");
    TraceInto("MissileAttack", "MissileAttack", "victim_index=%d:%N, index=%d:%N, damage=%d", \
              victim_index, ValidClientIndex(victim_index), index, ValidClientIndex(index), damage);

    if (!GetRestriction(index, Restriction_PreventUpgrades) &&
        !GetRestriction(index, Restriction_Stunned) &&
        !GetImmunity(victim_index, Immunity_RangedAttacks) &&
        !GetImmunity(victim_index, Immunity_HealthTaking) &&
        !GetImmunity(victim_index, Immunity_Explosion) &&
        !GetImmunity(victim_index, Immunity_Upgrades) &&
        !IsInvulnerable(victim_index))
    {
        decl String:weapon[64];
        new bool:is_equipment=GetWeapon(event,index,weapon,sizeof(weapon));
        if (IsGrenadeOrRocket(weapon, is_equipment) ||
            (!grenadesOnly && (IsNotMelee(weapon, is_equipment, index, victim_index) &&
                               TargetRange(index, victim_index) > 500.0)))
        {
            new energy = GetEnergy(index);
            new amount = GetUpgradeEnergy(race,upgrade);

            if (level >= num_levels)
            {
                decl String:raceName[64];
                GetRaceName(race, raceName, sizeof(raceName));
                LogError("%d:%N has too many levels in %d::%s level=%d, max=%d",
                         index, index, race, raceName, level, num_levels);
                level = num_levels-1;
            }

            if (energy >= amount &&
                GetRandomInt(1,100)<=missileAttackChance[level])
            {
                new Float:percent = missileAttackPercent[level];
                new health_take = RoundFloat(float(damage)*percent);
                if (health_take < 1)
                    health_take = 1;

                new Float:Origin[3];
                GetClientAbsOrigin(victim_index, Origin);
                Origin[2] += 5;

                if (missileAttackSound[0])
                {
                    PrepareSound(missileAttackSound);
                    EmitSoundToAll(missileAttackSound,sound_source);
                }

                TE_SetupGlowSprite(Origin,CrystalSprite(),0.7,3.0,200);
                TE_SendEffectToAll();

                Trace("MissileAttack: victim=%d:%N, attacker=%d:%N, health_take=%d", \
                      victim_index, ValidClientIndex(victim_index), \
                      index, ValidClientIndex(index), health_take);

                SetEnergy(index, energy-amount);
                FlashScreen(victim_index,RGBA_COLOR_RED);
                HurtPlayer(victim_index, health_take, index,
                           missileAttackShort, missileAttackName,
                           .type=DMG_BLAST, .in_hurt_event=true);

                TraceReturn();
                return true;
            }
            else
            {
                TraceReturn("%d:%N failed chance or no energy for %d:%N's Missile Attack, energy=%d,amount=%d,chance=%d", \
                            victim_index, ValidClientIndex(victim_index), index, ValidClientIndex(index), \
                            energy, amount, missileAttackChance[level]);
                return false;
            }
        }
        else
        {
            TraceReturn("%d:%N was attacked by %d:%N with melee(%s) -- No Missile Attack", \
                        victim_index, ValidClientIndex(victim_index), \
                        index, ValidClientIndex(index), weapon);
            return false;
        }
    }
    else
    {
        TraceReturn("%d:%N is immune to %d:%N's Missile Attack, Immunities=%x: Ranged=%d,HealthTaking=%d,Explosion=%d,Upgrades=%d,Invuln=%d", \
                    victim_index, ValidClientIndex(victim_index), index, ValidClientIndex(index), GetImmunityBits(victim_index), \
                    GetImmunity(victim_index, Immunity_RangedAttacks), GetImmunity(victim_index, Immunity_HealthTaking), \
                    GetImmunity(victim_index, Immunity_Explosion), GetImmunity(victim_index, Immunity_Upgrades), \
                    IsInvulnerable(victim_index));
        return false;
    }
}
