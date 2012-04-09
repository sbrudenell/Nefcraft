/**
 * vim: set ai et ts=4 sw=4 syntax=sourcepawn :
 * File: UltimateFeedback.inc
 * Description: The Ultimate Feedback Upgrade  for SourceCraft.
 * Author(s): -=|JFH|=-Naris
 */

#if defined _UltimateFeedback_included
 #endinput
#endif
#define _UltimateFeedback_included

#include "effect/Lightning"
#include "effect/BeamSprite"
#include "effect/PlasmaHaloSprite"

#include "effect/SendEffects"
#include "effect/FlashScreen"

#if !defined deniedWav
    stock const String:deniedWav[] = "sc/buzz.wav";
#endif

#if !defined ultimateFeedbackSound
    stock const String:ultimateFeedbackSound[] = "sc/feedback.mp3";
#endif

stock SetupUltimateFeedback(bool:precache, bool:preload=false)
{
    SetupLightning(precache, preload);
    SetupBeamSprite(precache, preload);
    SetupPlasmaHaloSprite(precache, preload);

    SetupSound(ultimateFeedbackSound, true, precache, preload);
}

stock UltimateFeedback(client, raceId, upgradeId, level, Float:feedbackRange[])
{
    decl String:upgradeName[64];
    GetUpgradeName(raceId, upgradeId, upgradeName, sizeof(upgradeName), client);

    new energy = GetEnergy(client);
    new amount = GetUpgradeEnergy(raceId,upgradeId);
    if (energy < amount)
    {
        EmitEnergySoundToClient(client,GetRaceFaction(raceId));
        DisplayMessage(client, Display_Energy, "%t", "InsufficientEnergyFor", upgradeName, amount);
    }
    else if (GetRestriction(client,Restriction_PreventUltimates) ||
             GetRestriction(client,Restriction_Stunned))
    {
        PrepareSound(deniedWav);
        EmitSoundToClient(client,deniedWav);
        DisplayMessage(client, Display_Ultimate, "%t",
                       "Prevented", upgradeName);
    }
    else if (HasCooldownExpired(client, raceId, upgradeId))
    {
        if (GameType == tf2)
        {
            switch (TF2_GetPlayerClass(client))
            {
                case TFClass_Spy:
                {
                    new pcond = TF2_GetPlayerConditionFlags(client);
                    if (TF2_IsCloaked(pcond) || TF2_IsDeadRingered(pcond))
                    {
                        PrepareSound(deniedWav);
                        EmitSoundToClient(client,deniedWav);
                        return;
                    }
                    else if (TF2_IsDisguised(pcond))
                        TF2_RemovePlayerDisguise(client);
                }
                case TFClass_Scout:
                {
                    if (TF2_IsPlayerBonked(client))
                    {
                        PrepareSound(deniedWav);
                        EmitSoundToClient(client,deniedWav);
                        return;
                    }
                }
            }
        }

        static const lightningColor[4] = { 10, 200, 255, 255 };

        new Float:range = feedbackRange[level];

        new Float:lastLoc[3];
        new Float:indexLoc[3];
        new Float:targetLoc[3];
        GetClientAbsOrigin(client, lastLoc);
        lastLoc[2] += 50.0; // Adjust trace position to the middle of the person instead of the feet.

        new last;
        new count=0;
        new team=GetClientTeam(client);
        new target = GetClientAimTarget(client);
        if (target > 0) 
        {
            last=target;
            GetClientAbsOrigin(target, targetLoc);
            targetLoc[2] += 50.0; // Adjust trace position to the middle of the person instead of the feet.

            TE_SetupBeamPoints(lastLoc, targetLoc, Lightning(), PlasmaHaloSprite(),
                               0, 1, 10.0, 10.0,10.0,2,50.0,lightningColor,255);
            TE_SendQEffectToAll(client, target);

            if (GetClientTeam(target) != team &&
                !GetImmunity(target,Immunity_Ultimates) &&
                !GetImmunity(target,Immunity_Restore) &&
                !IsInvulnerable(target))
            {
                new dmg = GetEnergy(target);
                if (dmg > 0)
                {
                    count++;
                    SetEnergy(target, 0);

                    PrepareSound(ultimateFeedbackSound);
                    EmitSoundToAll(ultimateFeedbackSound,target);

                    if (GetImmunity(target,Immunity_HealthTaking))
                        FlashScreen(target,RGBA_COLOR_WHITE);
                    else
                    {
                        FlashScreen(target,RGBA_COLOR_PINK);
                        HurtPlayer(target, dmg, client, "sc_ultimate_feedback",
                                   .xp=5+level, .ignore_armor=true, .limit=0.0,
                                   .type=DMG_ENERGYBEAM);
                    }
                }
            }
        }
        else
        {
            last = client;
            targetLoc = lastLoc;
        }

        PrepareSound(ultimateFeedbackSound);
        EmitSoundToAll(ultimateFeedbackSound,client);
        
        new lightning  = Lightning();
        new beamSprite = BeamSprite();
        new haloSprite = PlasmaHaloSprite();

        new b_count=0;
        new alt_count=0;
        new list[MaxClients+1];
        new alt_list[MaxClients+1];
        SetupOBeaconLists(list, alt_list, b_count, alt_count, client);

        if (b_count > 0)
        {
            TE_SetupBeamRingPoint(targetLoc, 10.0, range, beamSprite, haloSprite,
                                  0, 15, 0.5, 5.0, 0.0, lightningColor, 10, 0);

            TE_Send(list, b_count, 0.0);
        }

        if (alt_count > 0)
        {
            TE_SetupBeamRingPoint(targetLoc, range-10.0, range, beamSprite, haloSprite,
                                  0, 15, 0.5, 5.0, 0.0, lightningColor, 10, 0);

            TE_Send(alt_list, alt_count, 0.0);
        }
        
        SetEnergy(client, energy-amount);

        for (new index=1;index<=MaxClients;index++)
        {
            if (client != index && client != target && IsClientInGame(index) &&
                IsPlayerAlive(index) && GetClientTeam(index) != team)
            {
                if (!GetImmunity(index,Immunity_Ultimates) &&
                    !GetImmunity(index,Immunity_HealthTaking) &&
                    !GetImmunity(index,Immunity_Restore) &&
                    !IsInvulnerable(index))
                {
                    GetClientAbsOrigin(index, indexLoc);
                    indexLoc[2] += 50.0;

                    if (IsPointInRange(targetLoc,indexLoc,range) &&
                        TraceTargetIndex(client, index, targetLoc, indexLoc))
                    {
                        new dmg = GetEnergy(index);
                        if (dmg > 0)
                        {
                            TE_SetupBeamPoints(lastLoc, indexLoc, lightning, haloSprite,
                                               0, 1, 10.0, 10.0,10.0,2,50.0,lightningColor,255);
                            TE_SendQEffectToAll(last, index);

                            PrepareSound(ultimateFeedbackSound);
                            EmitSoundToAll(ultimateFeedbackSound,index);

                            count++;
                            last = index;
                            lastLoc = indexLoc;
                            SetEnergy(index, 0);

                            if (!GetImmunity(target,Immunity_HealthTaking))
                            {
                                HurtPlayer(index, dmg, client, "sc_ultimate_feedback",
                                           .xp=5+level, .ignore_armor=true, .limit=0.0,
                                           .type=DMG_PLASMA);
                            }
                        }
                    }
                }
            }
        }

        if (count)
        {
            DisplayMessage(client, Display_Ultimate, "%t",
                           "ToDamageEnemies", upgradeName,
                           count);
        }
        else
        {
            DisplayMessage(client,Display_Ultimate, "%t",
                           "WithoutEffect", upgradeName);
        }

        CreateCooldown(client, raceId, upgradeId);
    }
}
