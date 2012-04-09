/**
 * vim: set ai et ts=4 sw=4 syntax=sourcepawn :
 * File: Feedback.inc
 * Description: The Feedback Upgrade  for SourceCraft.
 * Author(s): -=|JFH|=-Naris
 */

#if defined _Feedback_included
 #endinput
#endif
#define _Feedback_included

#include "effect/SendEffects"
#include "effect/FlashScreen"

bool:Feedback(race, upgrade, level, damage, absorbed, victim_index, attacker_index, assister_index,
              const Float:feedbackPercent[][2], const feedbackChance[], const String:feedbackSound[]="")
{
    if (GetRandomInt(1,100) <= feedbackChance[level])
    {
        // Feedback is invoked by the victim!
        new energy = GetEnergy(victim_index);
        new amount = GetUpgradeEnergy(race,upgrade);
        if (energy >= amount)
        {
            new Float:percent = GetRandomFloat(feedbackPercent[level][0],
                                               feedbackPercent[level][1]);

            new dmgamt=RoundToNearest(float(damage+absorbed)*percent);
            if (dmgamt > absorbed)
            {
                new newhp=GetClientHealth(victim_index)+dmgamt-absorbed;
                new maxhp=GetMaxHealth(victim_index);
                if (newhp > maxhp)
                    newhp = maxhp;

                SetEntityHealth(victim_index,newhp);
            }

            SetEnergy(victim_index, energy-amount);

            LogToGame("[SC] Feedback prevented damage to %N from %N!\n",
                      victim_index, attacker_index);

            if (attacker_index > 0 && attacker_index != victim_index &&
                IsClientInGame(attacker_index) && IsPlayerAlive(attacker_index) &&
                !GetImmunity(attacker_index,Immunity_HealthTaking) &&
                !GetImmunity(attacker_index,Immunity_Upgrades) &&
                !IsInvulnerable(attacker_index))
            {
                new Float:Origin[3];
                GetClientAbsOrigin(attacker_index, Origin);
                Origin[2] += 5;

                TE_SetupSparks(Origin,Origin,255,1);
                TE_SendEffectToAll();

                if (feedbackSound[0] != '\0')
                {
                    PrepareSound(feedbackSound);
                    EmitSoundToAll(feedbackSound,victim_index);
                }
                
                new health=GetClientHealth(attacker_index);
                if (dmgamt >= health)
                {
                    if (percent > 0.95)
                        percent = 0.95;

                    dmgamt = RoundToCeil(float(health) * percent)+1;
                }

                FlashScreen(attacker_index,RGBA_COLOR_RED);
                HurtPlayer(attacker_index, dmgamt, victim_index,
                           "sc_feedback", .in_hurt_event=true);
            }
            else if (attacker_index  > 0 && attacker_index != victim_index)
            {
                DisplayMessage(victim_index,Display_Defense,
                               "%t", "YouEvadedFrom", attacker_index);

                DisplayMessage(attacker_index,Display_Enemy_Defended,
                               "%t", "HasEvaded", victim_index);
            }
            else
            {
                DisplayMessage(victim_index,Display_Defense,
                               "%t", "YouEvaded");
            }

            if (assister_index > 0)
            {
                DisplayMessage(assister_index, Display_Enemy_Defended,
                               "%t", "HasEvaded", victim_index);
            }
            return true;
        }
    }
    return false;
}


