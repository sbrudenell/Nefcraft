/**
 * vim: set ai et ts=4 sw=4 :
 * File: ProtossTemplar.sp
 * Description: The Protoss Templar race for SourceCraft.
 * Author(s): Naris (Murray Wilson)
 */

#pragma semicolon 1

#include <sourcemod>
#include <sdktools>
#include <raytrace>
#include <range>

#undef REQUIRE_EXTENSIONS
#include <tf2>
#include <tf2_player>
#include <sidewinder>
#define REQUIRE_EXTENSIONS

#undef REQUIRE_PLUGIN
#include <hgrsource>
#define REQUIRE_PLUGIN

#include "sc/SourceCraft"
#include "sc/Hallucinate"
#include "sc/clienttimer"
#include "sc/Levitation"
#include "sc/maxhealth"
#include "sc/Feedback"
#include "sc/dissolve"
#include "sc/shields"
#include "sc/freeze"

#include "effect/Smoke"
#include "effect/RedGlow"
#include "effect/BlueGlow"
#include "effect/Lightning"
#include "effect/HaloSprite"
#include "effect/SendEffects"
#include "effect/FlashScreen"

new const String:deathWav[]         = "sc/ptedth00.wav";
new const String:spawnWav[]         = "sc/pterdy00.wav";
new const String:deniedWav[]        = "sc/buzz.wav";
new const String:rechargeWav[]      = "sc/transmission.wav";
new const String:psistormWav[]      = "sc/ptesto00.wav";

new const String:g_FeedbackSound[]  = "sc/mind.mp3";

new raceID, immunityID, levitationID, psionicStormID;
new feedbackID, hallucinationID, shieldsID, amuletID;
new archonID;

new g_FeedbackChance[]              = { 0, 15, 25, 35, 50 };
new Float:g_FeedbackPercent[][2]    = { {0.00, 0.00},
                                        {0.10, 0.50},
                                        {0.10, 0.60},
                                        {0.10, 0.75},
                                        {0.10, 0.90} };

new g_HallucinateChance[]           = { 0, 15, 25, 35, 50 };

new Float:g_PsionicStormRange[]     = { 0.0, 250.0, 400.0, 550.0, 650.0 };

new Float:g_LevitationLevels[]      = { 1.0, 0.92, 0.733, 0.5466, 0.36 };

new Float:g_InitialShields[]        = { 0.0, 0.10, 0.25, 0.50, 0.75 };
new Float:g_ShieldsPercent[][2]     = { {0.00, 0.00},
                                        {0.00, 0.10},
                                        {0.00, 0.20},
                                        {0.05, 0.30},
                                        {0.10, 0.50} };

new g_archonRace = -1;

new gPsionicStormDuration[MAXPLAYERS+1];

new bool:m_SidewinderAvailable = false;

public Plugin:myinfo = 
{
    name = "SourceCraft Race - Protoss Templar",
    author = "-=|JFH|=-Naris",
    description = "The Protoss Templar race for SourceCraft.",
    version = SOURCECRAFT_VERSION,
    url = "http://jigglysfunhouse.net/"
};

public OnPluginStart()
{
    LoadTranslations("sc.common.phrases.txt");
    LoadTranslations("sc.templar.phrases.txt");
    LoadTranslations("sc.hallucinate.phrases.txt");

    if (GetGameType() == tf2)
    {
        decl String:error[64];
        m_SidewinderAvailable = (GetExtensionFileStatus("sidewinder.ext", error, sizeof(error)) == 1);
    }

    if (IsSourceCraftLoaded())
        OnSourceCraftReady();
}

public OnSourceCraftReady()
{
    raceID          = CreateRace("templar", 48, 0, 29, .energy_rate=2,
                                 .faction=Protoss, .type=Biological);

    immunityID      = AddUpgrade(raceID, "immunity");
    levitationID    = AddUpgrade(raceID, "levitation");
    feedbackID      = AddUpgrade(raceID, "feedback", .energy=2);
    hallucinationID = AddUpgrade(raceID, "hallucination", .energy=2);

    // Ultimate 1
    psionicStormID = AddUpgrade(raceID, "psistorm", 1,
                                .energy=60, .cooldown=2.0);

    shieldsID       = AddUpgrade(raceID, "shields", .energy=1);
    amuletID        = AddUpgrade(raceID, "amulet");

    // Ultimate 2
    archonID        = AddUpgrade(raceID, "archon", 2, 12,1,
                                 .energy=300, .cooldown=30.0);

    // Get Configuration Data
    GetConfigFloatArray("shields_amount", g_InitialShields, sizeof(g_InitialShields),
                        g_InitialShields, raceID, shieldsID);

    for (new level=0; level < sizeof(g_ShieldsPercent); level++)
    {
        decl String:key[32];
        Format(key, sizeof(key), "shields_percent_level_%d", level);
        GetConfigFloatArray(key, g_ShieldsPercent[level], sizeof(g_ShieldsPercent[]),
                            g_ShieldsPercent[level], raceID, shieldsID);
    }

    GetConfigArray("chance", g_FeedbackChance, sizeof(g_FeedbackChance),
                   g_FeedbackChance, raceID, feedbackID);

    for (new level=0; level < sizeof(g_ShieldsPercent); level++)
    {
        decl String:key[32];
        Format(key, sizeof(key), "damage_percent_level_%d", level);
        GetConfigFloatArray(key, g_FeedbackPercent[level], sizeof(g_FeedbackPercent[]),
                            g_FeedbackPercent[level], raceID, feedbackID);
    }

    GetConfigFloatArray("gravity",  g_LevitationLevels, sizeof(g_LevitationLevels),
                        g_LevitationLevels, raceID, levitationID);

    GetConfigArray("chance", g_HallucinateChance, sizeof(g_HallucinateChance),
                   g_HallucinateChance, raceID, hallucinationID);

    GetConfigFloatArray("range",  g_PsionicStormRange, sizeof(g_PsionicStormRange),
                        g_PsionicStormRange, raceID, psionicStormID);
}

public OnLibraryAdded(const String:name[])
{
    if (strncmp(name, "sidewinder", 10, false) == 0)
        m_SidewinderAvailable = (GetGameType() == tf2);
}

public OnLibraryRemoved(const String:name[])
{
    if (strncmp(name, "sidewinder", 10, false) == 0)
        m_SidewinderAvailable = false;
}

public OnMapStart()
{
    if (GetGameType() == tf2)
    {
        decl String:error[64];
        m_SidewinderAvailable = (GetExtensionFileStatus("sidewinder.ext", error, sizeof(error)) == 1);
    }

    g_archonRace = -1;

    SetupHallucinate(false, false);
    SetupSmokeSprite(false, false);
    SetupHaloSprite(false, false);
    SetupLevitation(false, false);
    SetupLightning(false, false);
    SetupBlueGlow(false, false);
    SetupRedGlow(false, false);

    SetupSound(spawnWav, true, false, false);
    SetupSound(deathWav, true, false, false);
    SetupSound(deniedWav, true, false, false);
    SetupSound(rechargeWav, true, false, false);
    SetupSound(psistormWav, true, false, false);
    SetupSound(g_FeedbackSound, true, false, false);
}

public OnMapEnd()
{
    ResetAllClientTimers();
    ResetAllShieldTimers();
}

public OnClientDisconnect(client)
{
    KillClientTimer(client);
    KillShieldTimer(client);
}

public Action:OnRaceDeselected(client,oldrace,newrace)
{
    if (oldrace == raceID)
    {
        KillClientTimer(client);
        KillShieldTimer(client);

        ResetShields(client);
        SetInitialEnergy(client, -1);
        SetGravity(client,-1.0, true);

        // Turn off Immunities
        new immunity_level=GetUpgradeLevel(client,raceID,immunityID);
        DoImmunity(client, immunity_level, false);

        return Plugin_Handled;
    }
    else
    {
        if (g_archonRace < 0)
            g_archonRace = FindRace("archon");

        if (oldrace == g_archonRace &&
            GetCooldownExpireTime(client, raceID, archonID) <= 0.0)
        {
            CreateCooldown(client, raceID, archonID,
                           .type=Cooldown_CreateNotify
                                |Cooldown_AlwaysNotify);
        }
        return Plugin_Continue;
    }
}

public Action:OnRaceSelected(client,oldrace,newrace)
{
    if (newrace == raceID)
    {
        // Turn on Immunities
        new immunity_level=GetUpgradeLevel(client,raceID,immunityID);
        DoImmunity(client, immunity_level, true);

        new levitation_level = GetUpgradeLevel(client,raceID,levitationID);
        SetLevitation(client, levitation_level, true, g_LevitationLevels);

        new shields_level = GetUpgradeLevel(client,raceID,shieldsID);
        SetupShields(client, shields_level, g_InitialShields, g_ShieldsPercent);

        new amulet_level = GetUpgradeLevel(client,raceID,amuletID);
        if (amulet_level > 0)
            SetInitialEnergy(client, ((amulet_level+1)*30));

        if (IsPlayerAlive(client))
        {
            PrepareSound(spawnWav);
            EmitSoundToAll(spawnWav,client);

            if (shields_level > 0)
            {
                CreateShieldTimer(client);
                CreateClientTimer(client, 1.0, Regeneration,
                                  TIMER_REPEAT|TIMER_FLAG_NO_MAPCHANGE);
            }
        }

        return Plugin_Handled;
    }
    else
        return Plugin_Continue;
}

public OnUpgradeLevelChanged(client,race,upgrade,new_level)
{
    if (race == raceID && GetRace(client) == raceID)
    {
        if (upgrade == immunityID)
            DoImmunity(client, new_level, true);
        else if (upgrade==levitationID)
            SetLevitation(client, new_level, true, g_LevitationLevels);
        else if (upgrade==amuletID)
            SetInitialEnergy(client, ((new_level+1)*30));
        else if (upgrade==shieldsID)
        {
            SetupShields(client, new_level, g_InitialShields,
                         g_ShieldsPercent, .upgrade=true);

            if (new_level > 0)
            {
                if (IsValidClient(client) && IsPlayerAlive(client))
                {
                    CreateShieldTimer(client);
                    CreateClientTimer(client, 1.0, Regeneration,
                                      TIMER_REPEAT|TIMER_FLAG_NO_MAPCHANGE);
                }
            }
            else
            {
                KillClientTimer(client);
                KillShieldTimer(client);
            }
        }
    }
}

public OnItemPurchase(client,item)
{
    if (GetRace(client) == raceID && IsPlayerAlive(client))
    {
        if (g_sockItem < 0)
            g_sockItem = FindShopItem("sock");

        if (item == g_sockItem)
        {
            new levitation_level = GetUpgradeLevel(client,raceID,levitationID);
            SetLevitation(client, levitation_level, true, g_LevitationLevels);
        }
    }
}

public Action:OnDropPlayer(client, target)
{
    if (IsValidClient(target) && GetRace(target) == raceID)
    {
        new levitation_level = GetUpgradeLevel(target,raceID,levitationID);
        SetLevitation(target, levitation_level, true, g_LevitationLevels);
    }
    return Plugin_Continue;
}

public OnUltimateCommand(client,race,bool:pressed,arg)
{
    if (race == raceID && IsPlayerAlive(client))
    {
        if (arg >= 2)
        {
            if (!pressed)
            {
                new archon_level = GetUpgradeLevel(client,race,archonID);
                if (archon_level > 0)
                    SummonArchon(client);
            }
        }
        else
        {
            new ps_level = GetUpgradeLevel(client,race,psionicStormID);
            if (ps_level > 0)
            {
                if (pressed)
                    PsionicStorm(client,ps_level);
            }
            else if (!pressed)
            {
                new archon_level = GetUpgradeLevel(client,race,archonID);
                if (archon_level > 0)
                    SummonArchon(client);
            }
        }
    }
}

public OnPlayerSpawnEvent(Handle:event, client, race)
{
    if (race == raceID)
    {
        PrepareSound(spawnWav);
        EmitSoundToAll(spawnWav,client);

        new immunity_level=GetUpgradeLevel(client,raceID,immunityID);
        DoImmunity(client, immunity_level, true);

        new levitation_level = GetUpgradeLevel(client,raceID,levitationID);
        SetLevitation(client, levitation_level, true, g_LevitationLevels);

        new amulet_level = GetUpgradeLevel(client,raceID,amuletID);
        if (amulet_level > 0)
        {
            new initial = (amulet_level+1) * 30;
            SetInitialEnergy(client, initial);
            if (GetEnergy(client) < initial)
                SetEnergy(client, initial);
        }

        new shields_level = GetUpgradeLevel(client,raceID,shieldsID);
        SetupShields(client, shields_level, g_InitialShields, g_ShieldsPercent);

        if (shields_level > 0)
        {
            CreateShieldTimer(client);
            CreateClientTimer(client, 1.0, Regeneration,
                              TIMER_REPEAT|TIMER_FLAG_NO_MAPCHANGE);
        }
    }
}

public Action:OnPlayerHurtEvent(Handle:event, victim_index, victim_race, attacker_index,
                                attacker_race, assister_index, assister_race,
                                damage, absorbed, bool:from_sc)
{
    new bool:changed=false;

    if (victim_race == raceID)
    {
        if (attacker_index > 0 && attacker_index != victim_index && IsPlayerAlive(attacker_index))
        {
            new feedback_level = GetUpgradeLevel(victim_index,raceID,feedbackID);
            changed |= Feedback(raceID, feedbackID, feedback_level, damage, absorbed, victim_index,
                                attacker_index, assister_index, g_FeedbackPercent, g_FeedbackChance,
                                g_FeedbackSound);
        }
    }

    if (!from_sc && attacker_index != victim_index)
    {
        new amount = GetUpgradeEnergy(raceID,hallucinationID);
        if (attacker_index > 0 && attacker_race == raceID)
        {
            new level = GetUpgradeLevel(attacker_index,raceID,hallucinationID);
            Hallucinate(victim_index, attacker_index, level, amount, g_HallucinateChance);
        }

        if (assister_index > 0 && assister_race == raceID)
        {
            new level = GetUpgradeLevel(assister_index,raceID,hallucinationID);
            Hallucinate(victim_index, assister_index, level, amount, g_HallucinateChance);
        }
    }

    return changed ? Plugin_Changed : Plugin_Continue;
}

public OnPlayerDeathEvent(Handle:event, victim_index, victim_race, attacker_index,
                          attacker_race, assister_index, assister_race, damage,
                          const String:weapon[], bool:is_equipment, customkill,
                          bool:headshot, bool:backstab, bool:melee)
{
    if (victim_race == raceID)
    {
        KillClientTimer(victim_index);
        KillShieldTimer(victim_index);

        PrepareSound(deathWav);
        EmitSoundToAll(deathWav,victim_index);
        DissolveRagdoll(victim_index, 0.2);
    }
    else
    {
        if (g_archonRace < 0)
            g_archonRace = FindRace("archon");

        if (victim_race == g_archonRace &&
            GetCooldownExpireTime(victim_index, raceID, archonID) <= 0.0)
        {
            CreateCooldown(victim_index, raceID, archonID,
                           .type=Cooldown_CreateNotify
                                |Cooldown_AlwaysNotify);
        }
    }
}

public Action:Regeneration(Handle:timer, any:userid)
{
    new client = GetClientOfUserId(userid);
    if (IsValidClient(client) && IsPlayerAlive(client))
    {
        if (GetRace(client) == raceID &&
            !GetRestriction(client,Restriction_PreventUpgrades) &&
            !GetRestriction(client,Restriction_Stunned))
        {
            new shields_level = GetUpgradeLevel(client,raceID,shieldsID);
            new shields_amount = (shields_level+1)*3;
            RegenerateFullShields(client, shields_level, g_InitialShields, shields_amount, shields_amount);
        }
    }
    return Plugin_Continue;
}

DoImmunity(client, level, bool:value)
{
    if (value && level >= 1)
    {
        SetImmunity(client,Immunity_Uncloaking, true);
        SetImmunity(client,Immunity_Detection, true);

        if (m_SidewinderAvailable)
            SidewinderCloakClient(client, true);
    }
    else
    {
        SetImmunity(client,Immunity_Uncloaking, false);

        if (m_SidewinderAvailable &&
            !GetImmunity(client,Immunity_Uncloaking) &&
            !GetImmunity(client,Immunity_Detection))
        {
            SidewinderCloakClient(client, false);
        }
    }

    SetImmunity(client,Immunity_MotionTaking, (value && level >= 2));
    SetImmunity(client,Immunity_Theft, (value && level >= 3));
    SetImmunity(client,Immunity_ShopItems, (value && level >= 4));

    if (value && IsClientInGame(client) && IsPlayerAlive(client))
    {
        new Float:start[3];
        GetClientAbsOrigin(client, start);

        static const color[4] = { 0, 255, 50, 128 };
        TE_SetupBeamRingPoint(start, 30.0, 60.0, Lightning(), HaloSprite(),
                              0, 1, 2.0, 10.0, 0.0 ,color, 10, 0);
        TE_SendEffectToAll();
    }
}

public PsionicStorm(client,ultlevel)
{
    decl String:upgradeName[64];
    GetUpgradeName(raceID, psionicStormID, upgradeName, sizeof(upgradeName), client);

    new energy = GetEnergy(client);
    new amount = GetUpgradeEnergy(raceID,psionicStormID);
    if (energy < amount)
    {
        EmitEnergySoundToClient(client,Protoss);
        DisplayMessage(client, Display_Energy, "%t", "InsufficientEnergyFor",
                       upgradeName, amount);
    }
    else if (GetRestriction(client,Restriction_PreventUltimates) ||
             GetRestriction(client,Restriction_Stunned))
    {
        PrepareSound(deniedWav);
        EmitSoundToClient(client,deniedWav);
        DisplayMessage(client, Display_Ultimate, "%t",
                       "Prevented", upgradeName);
    }
    else if (HasCooldownExpired(client, raceID, psionicStormID))
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

        gPsionicStormDuration[client] = ultlevel*3;

        new Handle:PsionicStormTimer = CreateTimer(0.4, PersistPsionicStorm, GetClientUserId(client),
                                                   TIMER_REPEAT|TIMER_FLAG_NO_MAPCHANGE);
        TriggerTimer(PsionicStormTimer, true);
        SetEnergy(client, energy-amount);

        DisplayMessage(client,Display_Ultimate, "%t", "Invoked", upgradeName);
        CreateCooldown(client, raceID, psionicStormID);
    }
}

public Action:PersistPsionicStorm(Handle:timer,any:userid)
{
    new client = GetClientOfUserId(userid);
    if (IsValidClient(client) && IsPlayerAlive(client) &&
        !GetRestriction(client,Restriction_PreventUltimates) &&
        !GetRestriction(client,Restriction_Stunned))
    {
        new level = GetUpgradeLevel(client,raceID,psionicStormID);
        new Float:range = g_PsionicStormRange[level];

        new Float:lastLoc[3];
        new Float:indexLoc[3];
        new Float:clientLoc[3];
        GetClientAbsOrigin(client, clientLoc);

        new b_count=0;
        new alt_count=0;
        new list[MaxClients+1];
        new alt_list[MaxClients+1];
        SetupOBeaconLists(list, alt_list, b_count, alt_count, client);

        static const psistormColor[4] = { 10, 200, 255, 255 };

        if (b_count > 0)
        {
            TE_SetupBeamRingPoint(clientLoc, 10.0, range, Lightning(), HaloSprite(),
                                  0, 15, 0.5, 5.0, 0.0, psistormColor, 10, 0);

            TE_Send(list, b_count, 0.0);
        }

        if (alt_count > 0)
        {
            TE_SetupBeamRingPoint(clientLoc, range-10.0, range, Lightning(), HaloSprite(),
                                  0, 15, 0.5, 5.0, 0.0, psistormColor, 10, 0);

            TE_Send(alt_list, alt_count, 0.0);
        }
        
        clientLoc[2] += 50.0; // Adjust trace position to the middle of the person instead of the feet.
        lastLoc = clientLoc;

        PrepareSound(psistormWav);
        EmitSoundToAll(psistormWav,client);

        new last=client;
        new minDmg=level;
        new maxDmg=level*5;
        new team=GetClientTeam(client);
        for(new index=1;index<=MaxClients;index++)
        {
            if (client != index && IsClientInGame(index) &&
                IsPlayerAlive(index) && GetClientTeam(index) != team)
            {
                if (!GetImmunity(index,Immunity_Ultimates) &&
                    !GetImmunity(index,Immunity_HealthTaking) &&
                    !IsInvulnerable(index))
                {
                    GetClientAbsOrigin(index, indexLoc);
                    indexLoc[2] += 50.0;

                    if (IsPointInRange(clientLoc,indexLoc,range) &&
                        TraceTargetIndex(client, index, clientLoc, indexLoc))
                    {
                        TE_SetupBeamPoints(lastLoc, indexLoc, Lightning(), HaloSprite(),
                                           0, 1, 10.0, 10.0,10.0,2,50.0,psistormColor,255);
                        TE_SendQEffectToAll(last, index);
                        FlashScreen(index,RGBA_COLOR_RED);

                        new amt=GetRandomInt(minDmg,maxDmg);
                        HurtPlayer(index, amt, client, "sc_psistorm",
                                   .xp=level+5, .type=DMG_ENERGYBEAM);
                        last=index;
                        lastLoc = indexLoc;
                    }
                }
            }
        }

        if (--gPsionicStormDuration[client] > 0)
            return Plugin_Continue;
    }
    return Plugin_Stop;
}

SummonArchon(client)
{
    new energy = GetEnergy(client);
    new amount = GetUpgradeEnergy(raceID,archonID);
    new accumulated = GetAccumulatedEnergy(client, raceID);
    if (accumulated + energy < amount)
    {
        ShowEnergy(client);
        EmitEnergySoundToClient(client,Protoss);

        decl String:upgradeName[64];
        GetUpgradeName(raceID, archonID, upgradeName, sizeof(upgradeName), client);
        DisplayMessage(client, Display_Ultimate, "%t", "InsufficientAccumulatedEnergyFor",
                       upgradeName, amount);
    }
    else if (GetRestriction(client,Restriction_PreventUltimates) ||
             GetRestriction(client,Restriction_Stunned))
    {
        PrepareSound(deniedWav);
        EmitSoundToClient(client,deniedWav);
        DisplayMessage(client, Display_Ultimate,
                       "%t", "PreventedFromSummoningArchon");
    }
    else if (HasCooldownExpired(client, raceID, archonID))
    {
        if (g_archonRace < 0)
            g_archonRace = FindRace("archon");

        if (g_archonRace > 0)
        {
            new Float:clientLoc[3];
            GetClientAbsOrigin(client, clientLoc);
            clientLoc[2] += 40.0; // Adjust position to the middle

            TE_SetupSmoke(clientLoc, SmokeSprite(), 8.0, 2);
            TE_SendEffectToAll();

            TE_SetupGlowSprite(clientLoc,(GetClientTeam(client) == 3) ? BlueGlow() : RedGlow(),
                               5.0,40.0,255);
            TE_SendEffectToAll();

            energy -= amount;
            if (energy < 0)
            {
                SetAccumulatedEnergy(client, raceID, accumulated+energy);
                energy = 0;
            }
            SetEnergy(client, energy);

            ChangeRace(client, g_archonRace, true, false);
        }
        else
        {
            PrepareSound(deniedWav);
            EmitSoundToClient(client,deniedWav);
            LogError("***The Protoss Archon race is not Available!");

            decl String:upgradeName[64];
            GetUpgradeName(raceID, archonID, upgradeName, sizeof(upgradeName), client);
            DisplayMessage(client, Display_Ultimate, "%t", "IsNotAvailable", upgradeName);
        }
    }
}
