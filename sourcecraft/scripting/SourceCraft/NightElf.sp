/**
 * vim: set ai et ts=4 sw=4 :
 * File: NightElf.sp
 * Description: The Night Elf race for SourceCraft.
 * Author(s): Anthony Iacono 
 * Modifications by: Naris (Murray Wilson)
 */

#pragma semicolon 1

#include <sourcemod>
#include <sdktools>
#include <particle>
#include <raytrace>
#include <range>

#undef REQUIRE_EXTENSIONS
#include <tf2>
#include <tf2_player>
#define REQUIRE_EXTENSIONS

#include "sc/SourceCraft"
#include "sc/maxhealth"
#include "sc/freeze"

#include "effect/Lightning"
#include "effect/BeamSprite"
#include "effect/HaloSprite"
#include "effect/SendEffects"
#include "effect/FlashScreen"

new const String:deniedWav[]    = "sc/buzz.wav";
new const String:rechargeWav[]  = "sc/transmission.wav";
new const String:entangleSound[]="sc/entanglingrootsdecay1.wav";

new g_EvasionChance[]           = { 0, 5, 15, 20, 30 };

new g_ThornsChance[]            = { 0, 15, 25, 35, 50 };
new Float:g_ThornsPercent[]     = { 0.0, 0.40, 0.55, 0.70, 0.90 };

new g_TrueshotChance[]          = { 0, 30, 40, 50, 60 };
new Float:g_TrueshotPercent[]   = { 0.0, 0.20, 0.35, 0.60, 0.80 };

new Float:g_RootsRange[]        = { 0.0, 300.0, 450.0, 650.0, 800.0};

new raceID, evasionID, thornsID, trueshotID, rootsID;

new Float:cfgEntangleDuration;

public Plugin:myinfo = 
{
    name = "SourceCraft Race - Night Elf",
    author = "-=|JFH|=-Naris with credits to PimpinJuice",
    description = "The Night Elf race for SourceCraft.",
    version = SOURCECRAFT_VERSION,
    url = "http://www.jigglysfunhouse.net/"
};

public OnPluginStart()
{
    LoadTranslations("sc.common.phrases.txt");
    LoadTranslations("sc.nightelf.phrases.txt");
    GetGameType();

    if (IsSourceCraftLoaded())
        OnSourceCraftReady();
}

public OnSourceCraftReady()
{
    raceID      = CreateRace("nightelf", .initial_energy=30,
                             .energy_limit=150, .faction=NightElf,
                             .type=Biological);

    evasionID   = AddUpgrade(raceID, "evasion", .energy=1);
    thornsID    = AddUpgrade(raceID, "thorns", .energy=2);
    trueshotID  = AddUpgrade(raceID, "trueshot", .energy=1);

    // Ultimate 1
    rootsID     = AddUpgrade(raceID, "roots", 1, .energy=30, .cooldown=2.0);

    // Get Configuration Data
    cfgEntangleDuration = GetConfigFloat("duration", 10.0, raceID, rootsID);

    GetConfigArray("chance", g_EvasionChance, sizeof(g_EvasionChance),
                   g_EvasionChance, raceID, evasionID);

    GetConfigArray("chance", g_ThornsChance, sizeof(g_ThornsChance),
                   g_ThornsChance, raceID, thornsID);

    GetConfigFloatArray("damage_percent",  g_ThornsPercent, sizeof(g_ThornsPercent),
                        g_ThornsPercent, raceID, thornsID);

    GetConfigArray("chance", g_TrueshotChance, sizeof(g_TrueshotChance),
                   g_TrueshotChance, raceID, trueshotID);

    GetConfigFloatArray("damage_percent",  g_TrueshotPercent, sizeof(g_TrueshotPercent),
                        g_TrueshotPercent, raceID, trueshotID);

    GetConfigFloatArray("range",  g_RootsRange, sizeof(g_RootsRange),
                        g_RootsRange, raceID, rootsID);
}

public OnMapStart()
{
    SetupLightning(false, false);
    SetupBeamSprite(false, false);
    SetupHaloSprite(false, false);

    SetupSound(deniedWav, true, false, false);
    SetupSound(rechargeWav, true, false, false);
    SetupSound(entangleSound, true, false, false);
}

public Action:OnPlayerTakeDamage(victim,&attacker,&inflictor,&Float:damage,&damagetype)
{
    if (GetRace(victim) == raceID)
    {
        new evasion_level = GetUpgradeLevel(victim,raceID,evasionID);
        if (evasion_level > 0 &&
            !GetRestriction(victim,Restriction_PreventUpgrades) &&
            !GetRestriction(victim,Restriction_Stunned))
        {
            new energy = GetEnergy(victim);
            new amount = GetUpgradeEnergy(raceID,evasionID);
            if (energy >= amount && GetRandomInt(1,100) <= g_EvasionChance[evasion_level])
            {
                if (attacker > 0 && attacker <= MaxClients && attacker != victim)
                {
                    DisplayMessage(victim,Display_Defense, "%t", "YouEvadedFrom", attacker);
                    DisplayMessage(attacker,Display_Enemy_Defended, "%t", "HasEvaded", victim);
                }
                else
                {
                    DisplayMessage(victim,Display_Defense, "%t", "YouEvaded");
                }

                if (GameType == tf2 && attacker > 0 && attacker <= MaxClients)
                {
                    decl Float:pos[3];
                    GetClientEyePosition(victim, pos);
                    pos[2] += 4.0;
                    TE_SetupParticle("miss_text", pos);
                    TE_SendToClient(attacker);
                }
                return Plugin_Handled;
            }
        }
    }
    return Plugin_Continue;
}

public Action:OnPlayerHurtEvent(Handle:event, victim_index, victim_race, attacker_index,
                                attacker_race, assister_index, assister_race,
                                damage, absorbed, bool:from_sc)
{
    new bool:changed=false;
    if (!from_sc && attacker_index != victim_index)
    {
        damage += absorbed;

        if (victim_race == raceID)
            changed |= ThornsAura(event, damage, victim_index, attacker_index);

        if (attacker_index > 0 && attacker_race == raceID)
            changed |= TrueshotAura(damage, victim_index, attacker_index);

        if (assister_index > 0 && assister_race == raceID)
            changed |= TrueshotAura(damage, victim_index, assister_index);
    }

    return changed ? Plugin_Changed : Plugin_Continue;
}

public bool:ThornsAura(Handle:event, damage, victim_index, index)
{
    new thorns_level = GetUpgradeLevel(victim_index,raceID,thornsID);
    if (thorns_level > 0)
    {
        if (IsValidClient(index) && IsPlayerAlive(index) &&
            !GetRestriction(index, Restriction_PreventUpgrades) &&
            !GetRestriction(index, Restriction_Stunned) &&
            !GetImmunity(index, Immunity_HealthTaking) &&
            !GetImmunity(index, Immunity_Upgrades) &&
            !IsInvulnerable(index))
        {
            new energy = GetEnergy(index);
            new amount = GetUpgradeEnergy(raceID,evasionID);
            new dmgamt = RoundToNearest(damage * GetRandomFloat(0.30,g_ThornsPercent[thorns_level]));
            if (energy >= amount && dmgamt > 0 &&
                GetRandomInt(1,100) <= g_ThornsChance[thorns_level])
            {
                new Float:Origin[3];
                GetClientAbsOrigin(victim_index, Origin);
                Origin[2] += 5;

                FlashScreen(index,RGBA_COLOR_RED);
                TE_SetupSparks(Origin,Origin,255,1);
                TE_SendEffectToAll();

                SetEnergy(index, energy-amount);
                HurtPlayer(index, dmgamt, victim_index,
                           "sc_thorns", .in_hurt_event=true);
                return true;
            }
        }
    }
    return false;
}

public bool:TrueshotAura(damage, victim_index, index)
{
    new trueshot_level = GetUpgradeLevel(index,raceID,trueshotID);
    if (trueshot_level > 0 && !IsInvulnerable(victim_index) &&
        !GetRestriction(index, Restriction_PreventUpgrades) &&
        !GetRestriction(index, Restriction_Stunned) &&
        !GetImmunity(victim_index, Immunity_HealthTaking) &&
        !GetImmunity(victim_index, Immunity_Upgrades))
    {
        if (GetRandomInt(1,100) <= g_TrueshotChance[trueshot_level])
        {
            new energy = GetEnergy(index);
            new amount = GetUpgradeEnergy(raceID,evasionID);
            new dmgamt=RoundFloat(float(damage)*g_TrueshotPercent[trueshot_level]);
            if (energy >= amount && dmgamt > 0)
            {
                new Float:Origin[3];
                GetClientAbsOrigin(victim_index, Origin);
                Origin[2] += 5;

                FlashScreen(victim_index,RGBA_COLOR_RED);
                TE_SetupSparks(Origin,Origin,255,1);
                TE_SendEffectToAll();

                SetEnergy(index, energy-amount);
                HurtPlayer(victim_index, dmgamt, index,
                           "sc_trueshot", .in_hurt_event=true);
                return true;
            }
        }
    }
    return false;
}

public OnUltimateCommand(client,race,bool:pressed,arg)
{
    if (race==raceID && pressed && IsPlayerAlive(client))
    {
        new ult_level=GetUpgradeLevel(client,race,rootsID);
        if (ult_level > 0)
        {
            decl String:upgradeName[64];
            GetUpgradeName(raceID, rootsID, upgradeName, sizeof(upgradeName), client);

            new energy = GetEnergy(client);
            new amount = GetUpgradeEnergy(raceID,rootsID);
            if (energy < amount)
            {
                EmitEnergySoundToClient(client,NightElf);
                DisplayMessage(client, Display_Energy, "%t",
                               "InsufficientEnergyFor", upgradeName, amount);
                return;
            }
            else if (GetRestriction(client,Restriction_PreventUltimates) ||
                     GetRestriction(client,Restriction_Stunned))
            {
                PrepareSound(deniedWav);
                EmitSoundToClient(client,deniedWav);
                DisplayMessage(client, Display_Ultimate, "%t",
                               "Prevented", upgradeName);
                return;
            }
            else if (!HasCooldownExpired(client, raceID, rootsID))
                return;

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

            new Float:indexLoc[3];
            new Float:clientLoc[3];
            GetClientAbsOrigin(client, clientLoc);
            clientLoc[2] += 50.0; // Adjust trace position to the middle of the person instead of the feet.

            SetEnergy(client, energy-amount);

            static const rootsColor[]       = {139, 69, 19, 255};
            static const entangleColor[]    = { 0, 255, 0, 255 };
            static const entangleFlash[]    = { 0, 255, 200, 3 };

            new lightning  = Lightning();
            new beamSprite = BeamSprite();
            new haloSprite = HaloSprite();
            new Float:range = g_RootsRange[ult_level];
            new Float:halfTime = cfgEntangleDuration / 2.0;

            new b_count=0;
            new alt_count=0;
            new list[MaxClients+1];
            new alt_list[MaxClients+1];
            SetupOBeaconLists(list, alt_list, b_count, alt_count, client);

            if (b_count > 0)
            {
                TE_SetupBeamRingPoint(clientLoc, 10.0, range, beamSprite, haloSprite,
                                      0, 15, 0.5, 5.0, 0.0, rootsColor, 10, 0);

                TE_Send(list, b_count, 0.0);
            }

            if (alt_count > 0)
            {
                TE_SetupBeamRingPoint(clientLoc, range-10.0, range, beamSprite, haloSprite,
                                      0, 15, 0.5, 5.0, 0.0, rootsColor, 10, 0);

                TE_Send(alt_list, alt_count, 0.0);
            }


            PrepareSound(entangleSound);

            new count = 0;
            new team  = GetClientTeam(client);
            for (new index=1;index<=MaxClients;index++)
            {
                if (client != index && IsValidClient(index) && IsPlayerAlive(index) &&
                    GetClientTeam(index) != team)
                {
                    if (!GetImmunity(index,Immunity_Ultimates) &&
                        !GetImmunity(index,Immunity_Restore) &&
                        !GetImmunity(index,Immunity_MotionTaking) &&
                        !IsBurrowed(index))
                    {
                        GetClientAbsOrigin(index, indexLoc);
                        indexLoc[2] += 50.0;

                        if (IsPointInRange(clientLoc,indexLoc,range) &&
                            TraceTargetIndex(client, index, clientLoc, indexLoc))
                        {
                            TE_SetupBeamPoints(clientLoc, indexLoc, lightning, haloSprite,
                                               0, 1, halfTime, 10.0,10.0,5,50.0,entangleColor,255);
                            TE_SendQEffectToAll(client,index);

                            indexLoc[2]-= 35.0; // -50.0 + 15.0
                            TE_SetupBeamRingPoint(indexLoc,45.0,44.0,beamSprite,haloSprite,
                                                  0,15,cfgEntangleDuration,5.0,0.0,
                                                  entangleColor,10,0);
                            TE_SendEffectToAll();

                            indexLoc[2]+=15.0;
                            TE_SetupBeamRingPoint(indexLoc,45.0,44.0,beamSprite,haloSprite,
                                                  0,15,cfgEntangleDuration,5.0,0.0,
                                                  entangleColor,10,0);
                            TE_SendEffectToAll();

                            indexLoc[2]+=15.0;
                            TE_SetupBeamRingPoint(indexLoc,45.0,44.0,beamSprite,haloSprite,
                                                  0,15,cfgEntangleDuration,5.0,0.0,
                                                  entangleColor,10,0);
                            TE_SendEffectToAll();
                            FlashScreen(index,entangleFlash);

                            EmitSoundToAll(entangleSound, index);

                            DisplayMessage(index,Display_Enemy_Ultimate, "%t",
                                           "HasEntangled", client);

                            FreezeEntity(index);
                            CreateTimer(cfgEntangleDuration,UnfreezePlayer,GetClientUserId(index),
                                        TIMER_FLAG_NO_MAPCHANGE);
                            count++;
                        }
                    }
                }
            }

            if (count)
            {
                DisplayMessage(client,Display_Ultimate, "%t",
                               "ToEntangleEnemies", count);
            }
            else
            {
                DisplayMessage(client,Display_Ultimate, "%t",
                               "WithoutEffect", upgradeName);
            }

            CreateCooldown(client, raceID, rootsID);
        }
    }
}

