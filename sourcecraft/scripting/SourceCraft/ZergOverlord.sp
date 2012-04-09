/**
 * vim: set ai et ts=4 sw=4 :
 * File: Zerg.sp
 * Description: The Zerg Overlord race for SourceCraft.
 * Author(s): -=|JFH|=-Naris
 */
 
#pragma semicolon 1

#include <sourcemod>
#include <sdktools>
#include <raytrace>
#include <range>

#undef REQUIRE_EXTENSIONS
#include <tf2>
#include <tf2_flag>
#include <tf2_meter>
#include <tf2_player>
#include <sidewinder>
#define REQUIRE_EXTENSIONS

#undef REQUIRE_PLUGIN
#include <jetpack>
#include <piggyback>
#define REQUIRE_PLUGIN

#include "sc/SourceCraft"
#include "sc/SupplyDepot"
#include "sc/clienttimer"
#include "sc/Levitation"
#include "sc/SpeedBoost"
#include "sc/maxhealth"
#include "sc/Detector"
#include "sc/weapons"
#include "sc/freeze"

#include "effect/Smoke"
#include "effect/RedGlow"
#include "effect/BlueGlow"
#include "effect/Lightning"
#include "effect/BeamSprite"
#include "effect/HaloSprite"
#include "effect/SendEffects"
#include "effect/FlashScreen"

new const String:errorWav[]     = "sc/perror.mp3";
new const String:deniedWav[]    = "sc/buzz.wav";
new const String:rechargeWav[]  = "sc/transmission.wav";

new raceID, pneumatizedID, boostID, regenerationID, healingID;
new transfusionID, detectorID, jetpackID, excreteID, sacsID, overseerID;

new g_JetpackFuel[]             = { 0,     20,   25,   35,   45 };
new Float:g_JetpackRefuelTime[] = { 0.0, 45.0, 35.0, 25.0, 15.0 };

new Float:g_ExcreteCreepRange[] = { 350.0, 400.0, 650.0, 750.0, 900.0 };
new Float:g_TransfusionRange[]  = { 0.0, 300.0, 450.0, 650.0, 800.0 };
new Float:g_DetectingRange[]    = { 0.0, 300.0, 450.0, 650.0, 800.0 };
new Float:g_HealingRange[]      = { 0.0, 300.0, 450.0, 650.0, 800.0 };

new Float:g_SpeedLevels[]       = { 1.0, 1.05, 1.10, 1.15, 1.20 };
new Float:g_LevitationLevels[]  = { 1.0, 0.92, 0.733, 0.5466, 0.36 };

new Float:g_PiggybackRange[]    = { 0.0, 350.0, 400.0, 650.0, 750.0 };
new PiggyMethod:g_PiggybackMethod[] =
{
    PiggyMethod_None,
    PiggyMethod_Enable | PiggyMethod_Pickup | PiggyMethod_DisableAttack | PiggyMethod_SharedFate | PiggyMethod_ForceView,
    PiggyMethod_Enable | PiggyMethod_Pickup | PiggyMethod_DisableAttack | PiggyMethod_SharedFate,
    PiggyMethod_Enable | PiggyMethod_Pickup | PiggyMethod_DisableAttack | PiggyMethod_AllowSpys,
    PiggyMethod_Enable | PiggyMethod_Pickup | PiggyMethod_DisableAttack | PiggyMethod_AllowSpys
};

new bool:m_JetpackAvailable     = false;
new bool:m_PiggybackAvailable   = false;

new g_overseerRace              = -1;

public Plugin:myinfo = 
{
    name = "SourceCraft Race - Zerg Overlord",
    author = "-=|JFH|=-Naris",
    description = "The Zerg Overlord race for SourceCraft.",
    version = SOURCECRAFT_VERSION,
    url = "http://jigglysfunhouse.net/"
};

public OnPluginStart()
{
    LoadTranslations("sc.common.phrases.txt");
    LoadTranslations("sc.detector.phrases.txt");
    LoadTranslations("sc.overlord.phrases.txt");

    if (IsSourceCraftLoaded())
        OnSourceCraftReady();
}

public OnSourceCraftReady()
{
    if (GetGameType() == tf2)
    {
        decl String:error[64];
        m_SidewinderAvailable = (GetExtensionFileStatus("sidewinder.ext", error, sizeof(error)) == 1);
    }

    raceID          = CreateRace("overlord", 32, 0, 37, 45, 150, 1,
                                 Zerg, Biological);

    pneumatizedID   = AddUpgrade(raceID, "pneumatized");
    boostID         = AddUpgrade(raceID, "boost");
    regenerationID  = AddUpgrade(raceID, "regeneration");
    healingID       = AddUpgrade(raceID, "healing");
    transfusionID   = AddUpgrade(raceID, "transfusion");
    detectorID      = AddUpgrade(raceID, "antennae");

    // Ultimate 1
    if (m_JetpackAvailable || LibraryExists("jetpack"))
    {
        jetpackID   = AddUpgrade(raceID, "flyer", 1, 0);

        GetConfigArray("fuel", g_JetpackFuel, sizeof(g_JetpackFuel),
                       g_JetpackFuel, raceID, jetpackID);

        GetConfigFloatArray("refuel_time", g_JetpackRefuelTime, sizeof(g_JetpackRefuelTime),
                            g_JetpackRefuelTime, raceID, jetpackID);

        if (!m_JetpackAvailable)
        {
            ControlJetpack(true);
            SetJetpackRefuelingTime(0,30.0);
            SetJetpackFuel(0,100);
            m_JetpackAvailable = true;
        }
    }
    else
    {
        jetpackID   = AddUpgrade(raceID, "flyer", 1, 99, 0,
                                 .desc="%NotAvailable");
    }

    // Ultimate 2
    excreteID       = AddUpgrade(raceID, "excrete", 2, .energy=30,
                                 .cooldown=2.0);

    // Ultimate 3
    if (m_PiggybackAvailable || LibraryExists("piggyback"))
    {
        sacsID      = AddUpgrade(raceID, "sacs", 3, .energy=30);

        GetConfigArray("method", g_PiggybackMethod, sizeof(g_PiggybackMethod),
                       g_PiggybackMethod, raceID, sacsID);

        GetConfigFloatArray("range", g_PiggybackRange, sizeof(g_PiggybackRange),
                            g_PiggybackRange, raceID, sacsID);

        if (!m_PiggybackAvailable)
        {
            ControlPiggyback(true);
            m_PiggybackAvailable = true;
        }
    }
    else
    {
        sacsID      = AddUpgrade(raceID, "sacs", 3, 99, 0,
                                 .desc="%NotAvailable");
    }

    // Ultimate 4
    overseerID      = AddUpgrade(raceID, "overseer", 4, 12, 1,
                                 .energy=120, .cooldown=60.0);

    // Get Configuration Data
    GetConfigFloatArray("range",  g_HealingRange, sizeof(g_HealingRange),
                        g_HealingRange, raceID, healingID);

    GetConfigFloatArray("range",  g_DetectingRange, sizeof(g_DetectingRange),
                        g_DetectingRange, raceID, detectorID);

    GetConfigFloatArray("range",  g_TransfusionRange, sizeof(g_TransfusionRange),
                        g_TransfusionRange, raceID, transfusionID);

    GetConfigFloatArray("range",  g_ExcreteCreepRange, sizeof(g_ExcreteCreepRange),
                        g_ExcreteCreepRange, raceID, excreteID);

    GetConfigFloatArray("speed", g_SpeedLevels, sizeof(g_SpeedLevels),
                        g_SpeedLevels, raceID, boostID);

    GetConfigFloatArray("gravity", g_LevitationLevels, sizeof(g_LevitationLevels),
                        g_LevitationLevels, raceID, pneumatizedID);
}

public OnLibraryAdded(const String:name[])
{
    if (StrEqual(name, "jetpack"))
    {
        if (!m_JetpackAvailable)
        {
            ControlJetpack(true);
            SetJetpackRefuelingTime(0,30.0);
            SetJetpackFuel(0,100);
            m_JetpackAvailable = true;
        }
    }
    else if (StrEqual(name, "piggyback"))
    {
        if (!m_PiggybackAvailable)
        {
            ControlPiggyback(true);
            m_PiggybackAvailable = true;
        }
    }
    else if (strncmp(name, "sidewinder", 10, false) == 0)
        m_SidewinderAvailable = (GetGameType() == tf2);
}

public OnLibraryRemoved(const String:name[])
{
    if (StrEqual(name, "jetpack"))
        m_JetpackAvailable = false;
    else if (StrEqual(name, "piggyback"))
        m_PiggybackAvailable = false;
    else if (strncmp(name, "sidewinder", 10, false) == 0)
        m_SidewinderAvailable = false;
}

public OnMapStart()
{
    if (GetGameType() == tf2)
    {
        decl String:error[64];
        m_SidewinderAvailable = (GetExtensionFileStatus("sidewinder.ext", error, sizeof(error)) == 1);
    }

    SetupRedGlow(false, false);
    SetupBlueGlow(false, false);
    SetupSmokeSprite(false, false);
    SetupBeamSprite(false, false);
    SetupHaloSprite(false, false);
    SetupLevitation(false, false);
    SetupLightning(false, false);
    SetupSpeed(false, false);

    SetupSound(errorWav, true, false, false);
    SetupSound(deniedWav, true, false, false);
    SetupSound(rechargeWav, true, false, false);
}

public OnMapEnd()
{
    ResetAllClientTimers();
}

public OnClientDisconnect(client)
{
    KillClientTimer(client);
    ResetDetection(client);
    ResetDetected(client);
}

public Action:OnRaceDeselected(client,oldrace,newrace)
{
    if (oldrace == raceID)
    {
        if (m_JetpackAvailable)
            TakeJetpack(client);

        if (m_PiggybackAvailable)
            TakePiggyback(client);

        SetSpeed(client,-1.0);
        SetGravity(client,-1.0);
        ApplyPlayerSettings(client);

        KillClientTimer(client);
        ResetDetection(client);
        return Plugin_Handled;
    }
    else
    {
        if (g_overseerRace < 0)
            g_overseerRace = FindRace("overseer");

        if (oldrace == g_overseerRace &&
            GetCooldownExpireTime(client, raceID, overseerID) <= 0.0)
        {
            CreateCooldown(client, raceID, overseerID,
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
        new flyer_level=GetUpgradeLevel(client,raceID,jetpackID);
        SetupJetpack(client, flyer_level);

        new sacs_level=GetUpgradeLevel(client,raceID,sacsID);
        SetupPiggyback(client, sacs_level);

        new pneumatized_level = GetUpgradeLevel(client,raceID,pneumatizedID);
        SetLevitation(client, pneumatized_level, false, g_LevitationLevels);

        new boost_level = GetUpgradeLevel(client,raceID,boostID);
        SetSpeedBoost(client, boost_level, false, g_SpeedLevels);

        if (IsPlayerAlive(client))
        {
            new healing_aura_level=GetUpgradeLevel(client,raceID,healingID);
            new regeneration_level=GetUpgradeLevel(client,raceID,regenerationID);
            new transfusion_level=GetUpgradeLevel(client,raceID,transfusionID);
            if (healing_aura_level > 0 || regeneration_level > 0 ||
                transfusion_level > 0)
            {
                CreateClientTimer(client, 1.0, Regeneration,
                                  TIMER_REPEAT|TIMER_FLAG_NO_MAPCHANGE);
            }

            if (boost_level > 0 || pneumatized_level > 0)
                ApplyPlayerSettings(client);
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
        if (upgrade==boostID)
            SetSpeedBoost(client, new_level, true, g_SpeedLevels);
        else if (upgrade==pneumatizedID)
            SetLevitation(client, new_level, true, g_LevitationLevels);
        else if (upgrade==jetpackID)
            SetupJetpack(client, new_level);
        else if (upgrade==sacsID)
            SetupPiggyback(client, new_level);
        else if (upgrade==healingID)
        {
            if (new_level || GetUpgradeLevel(client,raceID,regenerationID)
                          || GetUpgradeLevel(client,raceID,transfusionID))
            {
                if (IsClientInGame(client) && IsPlayerAlive(client))
                {
                    CreateClientTimer(client, 1.0, Regeneration,
                                      TIMER_REPEAT|TIMER_FLAG_NO_MAPCHANGE);
                }
            }
            else
            {
                KillClientTimer(client);
                ResetDetection(client);
            }
        }
        else if (upgrade==regenerationID)
        {
            if (new_level || GetUpgradeLevel(client,raceID,healingID)
                          || GetUpgradeLevel(client,raceID,transfusionID))
            {
                if (IsClientInGame(client) && IsPlayerAlive(client))
                {
                    CreateClientTimer(client, 1.0, Regeneration,
                                      TIMER_REPEAT|TIMER_FLAG_NO_MAPCHANGE);
                }
            }
            else
            {
                KillClientTimer(client);
                ResetDetection(client);
            }
        }
        else if (upgrade==transfusionID)
        {
            if (new_level || GetUpgradeLevel(client,raceID,healingID)
                          || GetUpgradeLevel(client,raceID,regenerationID))
            {
                if (IsClientInGame(client) && IsPlayerAlive(client))
                {
                    CreateClientTimer(client, 1.0, Regeneration,
                                      TIMER_REPEAT|TIMER_FLAG_NO_MAPCHANGE);
                }
            }
            else
            {
                KillClientTimer(client);
                ResetDetection(client);
            }
        }
    }
}

public OnItemPurchase(client,item)
{
    if (GetRace(client) == raceID && IsPlayerAlive(client))
    {
        if (g_bootsItem < 0)
            g_bootsItem = FindShopItem("boots");

        if (g_sockItem < 0)
            g_sockItem = FindShopItem("sock");

        if (item == g_bootsItem)
        {
            new boost_level = GetUpgradeLevel(client,raceID,boostID);
            SetSpeedBoost(client, boost_level, true, g_SpeedLevels);
        }
        else if (item == g_sockItem)
        {
            new pneumatized_level = GetUpgradeLevel(client,raceID,pneumatizedID);
            SetLevitation(client, pneumatized_level, true, g_LevitationLevels);
        }
    }
}

public Action:OnDropPlayer(client, target)
{
    if (IsValidClient(target) && GetRace(target) == raceID)
    {
        new pneumatized_level = GetUpgradeLevel(target,raceID,pneumatizedID);
        SetLevitation(target, pneumatized_level, true, g_LevitationLevels);
    }
    return Plugin_Continue;
}

public OnUltimateCommand(client,race,bool:pressed,arg)
{
    if (race==raceID && IsPlayerAlive(client))
    {
        switch (arg)
        {
            case 4:
            {
                new overseer_level=GetUpgradeLevel(client,race,overseerID);
                if (overseer_level > 0)
                {
                    if (pressed)
                        OverseerMorph(client);
                }
            }
            case 3:
            {
                if (pressed)
                {
                    if (m_PiggybackAvailable &&
                        GetUpgradeLevel(client,race,sacsID) > 0 &&
                        !GetRestriction(client,Restriction_PreventUltimates) &&
                        !GetRestriction(client,Restriction_Stunned))
                    {
                        Piggyback(client);
                    }
                }
            }
            case 2:
            {
                if (pressed)
                {
                    new excrete_level = GetUpgradeLevel(client,race,excreteID);
                    if (excrete_level > 0)
                        ExcreteCreep(client,excrete_level);
                    else if (m_PiggybackAvailable &&
                             GetUpgradeLevel(client,race,sacsID) > 0 &&
                             !GetRestriction(client,Restriction_PreventUltimates) &&
                             !GetRestriction(client,Restriction_Stunned))
                    {
                        Piggyback(client);
                    }
                }
            }
            default:
            {
                if (m_JetpackAvailable)
                {
                    if (pressed)
                    {
                        if (GetRestriction(client, Restriction_PreventUltimates) ||
                            GetRestriction(client, Restriction_Grounded) ||
                            GetRestriction(client, Restriction_Stunned))
                        {
                            PrepareSound(deniedWav);
                            EmitSoundToAll(deniedWav, client);
                        }
                        else
                            StartJetpack(client);
                    }
                    else
                        StopJetpack(client);
                }
                else if (pressed)
                {
                    decl String:upgradeName[64];
                    GetUpgradeName(raceID, jetpackID, upgradeName, sizeof(upgradeName), client);
                    PrintHintText(client,"%t", "IsNotAvailable", upgradeName);
                }
            }
        }
    }
}

// Events
public OnPlayerSpawnEvent(Handle:event, client, race)
{
    if (race==raceID)
    {
        new flyer_level=GetUpgradeLevel(client,raceID,jetpackID);
        SetupJetpack(client, flyer_level);

        new sacs_level=GetUpgradeLevel(client,raceID,sacsID);
        SetupPiggyback(client, sacs_level);

        new pneumatized_level = GetUpgradeLevel(client,raceID,pneumatizedID);
        SetLevitation(client, pneumatized_level, true, g_LevitationLevels);

        new boost_level = GetUpgradeLevel(client,raceID,boostID);
        SetSpeedBoost(client, boost_level, false, g_SpeedLevels);

        if (boost_level > 0 || pneumatized_level > 0)
            ApplyPlayerSettings(client);

        new healing_aura_level=GetUpgradeLevel(client,raceID,healingID);
        new regeneration_level=GetUpgradeLevel(client,raceID,regenerationID);
        new transfusion_level=GetUpgradeLevel(client,raceID,transfusionID);
        if (healing_aura_level > 0 || regeneration_level > 0 ||
            transfusion_level > 0)
        {
            CreateClientTimer(client, 1.0, Regeneration,
                              TIMER_REPEAT|TIMER_FLAG_NO_MAPCHANGE);
        }
    }
}

public OnPlayerDeathEvent(Handle:event, victim_index, victim_race, attacker_index,
                          attacker_race, assister_index, assister_race, damage,
                          const String:weapon[], bool:is_equipment, customkill,
                          bool:headshot, bool:backstab, bool:melee)
{
    ResetDetected(victim_index);

    if (victim_race == raceID)
    {
        KillClientTimer(victim_index);
        ResetDetection(victim_index);
    }
    else
    {
        if (g_overseerRace < 0)
            g_overseerRace = FindRace("overseer");

        if (victim_race == g_overseerRace &&
            GetCooldownExpireTime(victim_index, raceID, overseerID) <= 0.0)
        {
            CreateCooldown(victim_index, raceID, overseerID,
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
        if (GetRace(client) == raceID)
        {
            new regeneration_level = GetUpgradeLevel(client,raceID,regenerationID);
            if (regeneration_level > 0 &&
                !GetRestriction(client,Restriction_PreventUpgrades) &&
                !GetRestriction(client,Restriction_Stunned))
            {
                HealPlayer(client,regeneration_level);
            }

            new healing_aura_level = GetUpgradeLevel(client,raceID,healingID);
            new Float:healing_range = g_HealingRange[healing_aura_level];

            new transfusion_level = GetUpgradeLevel(client,raceID,transfusionID);
            new Float:transfusion_range = g_TransfusionRange[transfusion_level];

            new detecting_level = GetUpgradeLevel(client,raceID,detectorID);
            new Float:detecting_range = g_DetectingRange[detecting_level];

            if ((healing_aura_level <= 0 && transfusion_level <= 0 && detecting_level <= 0) ||
                GetRestriction(client, Restriction_PreventUpgrades) ||
                GetRestriction(client, Restriction_Stunned))
            {
                ResetDetection(client);
            }
            else
            {
                static const healingColor[4] = {0, 255, 0, 255};
                new Float:indexLoc[3];
                new Float:clientLoc[3];
                GetClientAbsOrigin(client, clientLoc);
                clientLoc[2] += 50.0; // Adjust trace position to the middle of the person instead of the feet.

                decl String:upgradeName[64];
                GetUpgradeName(raceID, detectorID, upgradeName, sizeof(upgradeName), client);

                new count=0;
                new alt_count=0;
                new list[MaxClients+1];
                new alt_list[MaxClients+1];
                new team=GetClientTeam(client);
                new auraAmount = healing_aura_level*5;
                for (new index=1;index<=MaxClients;index++)
                {
                    if (index != client && IsClientInGame(index))
                    {
                        new bool:alive = IsPlayerAlive(index);
                        GetClientAbsOrigin(index, indexLoc);

                        if (GetClientTeam(index) == team)
                        {
                            if (!GetSetting(index, Disable_Beacons) &&
                                !GetSetting(index, Remove_Queasiness))
                            {
                                if (GetSetting(index, Reduce_Queasiness))
                                    alt_list[alt_count++] = index;
                                else
                                    list[count++] = index;
                            }

                            if (alive && (transfusion_level > 0 || healing_aura_level > 0) &&
                                TraceTargetIndex(client, index, clientLoc, indexLoc))
                            {
                                if (transfusion_level > 0)
                                {
                                    if (IsPointInRange(clientLoc,indexLoc,transfusion_range))
                                    {
                                        new SupplyTypes:type;
                                        if (GameType == dod)
                                        {
                                            new pick = GetRandomInt(0,10);
                                            type = (pick > 6) ? SupplyDefault :
                                                   (pick > 3) ? SupplySecondary
                                                              : (SupplyGrenade|SupplySmoke);
                                        }
                                        else
                                        {
                                            type = (GetRandomInt(0,10) > 5) ? SupplyDefault : SupplySecondary;
                                        }

                                        SupplyAmmo(index, transfusion_level, "Transfusion", type);
                                    }
                                }

                                if (healing_aura_level > 0)
                                {
                                    if (IsPointInRange(clientLoc,indexLoc,healing_range))
                                    {
                                        new health=GetClientHealth(index);
                                        new max=GetMaxHealth(index);
                                        if (health < max)
                                            HealPlayer(index,auraAmount,health,max);
                                    }
                                }
                            }
                        }
                        else
                        {
                            if (detecting_level > 0)
                            {
                                if (alive && IsPointInRange(clientLoc,indexLoc,detecting_range) &&
                                    TraceTargetIndex(client, index, clientLoc, indexLoc))
                                {
                                    new bool:uncloaked = false;
                                    if (GetGameType() == tf2 &&
                                        !GetImmunity(index,Immunity_Uncloaking) &&
                                        TF2_GetPlayerClass(index) == TFClass_Spy)
                                    {
                                        TF2_RemoveCondition(index, TFCond_Cloaked);

                                        uncloaked = true;
                                        HudMessage(index, "%t", "UncloakedHud");
                                        DisplayMessage(index, Display_Enemy_Message, "%t",
                                                       "HasUncloaked", client, upgradeName);
                                    }

                                    if (!GetImmunity(index,Immunity_Detection))
                                    {
                                        SetOverrideVisiblity(index, 255);
                                        if (m_SidewinderAvailable)
                                        {
                                            SidewinderDetectClient(index, true);
                                            HudMessage(index, "%t", "DetectedHud");
                                        }

                                        if (!m_Detected[client][index])
                                        {
                                            m_Detected[client][index] = true;
                                            ApplyPlayerSettings(index);
                                        }

                                        if (!uncloaked)
                                        {
                                            HudMessage(index, "%t", "DetectedHud");
                                            DisplayMessage(index, Display_Enemy_Message, "%t",
                                                           "HasDetected", client, upgradeName);
                                        }
                                    }
                                }
                                else // undetect
                                {
                                    SetOverrideVisiblity(index, -1);
                                    if (m_SidewinderAvailable)
                                        SidewinderDetectClient(index, false);

                                    if (m_Detected[client][index])
                                    {
                                        m_Detected[client][index] = false;
                                        ApplyPlayerSettings(index);
                                        ClearDetectedHud(index);
                                    }
                                }
                            }
                        }
                    }
                }

                if (!GetSetting(client, Disable_Beacons) &&
                    !GetSetting(client, Remove_Queasiness))
                {
                    if (GetSetting(client, Reduce_Queasiness))
                        alt_list[alt_count++] = client;
                    else
                        list[count++] = client;
                }

                static const transfusionColor[4] = {255, 225, 0, 255};
                static const detectColor[4] = {202, 225, 255, 255};
                clientLoc[2] -= 50.0; // Adjust position back to the feet.

                if (count > 0)
                {
                    if (transfusion_level > 0)
                    {
                        TE_SetupBeamRingPoint(clientLoc, 10.0, transfusion_range, BeamSprite(), HaloSprite(),
                                              0, 15, 0.5, 5.0, 0.0, transfusionColor, 10, 0);
                        TE_Send(list, count, 0.0);
                    }

                    if (detecting_level > 0)
                    {
                        TE_SetupBeamRingPoint(clientLoc, 10.0, detecting_range, BeamSprite(), HaloSprite(),
                                              0, 10, 0.6, 10.0, 0.5, detectColor, 10, 0);
                        TE_Send(list, count, 0.0);
                    }

                    if (healing_aura_level > 0)
                    {
                        TE_SetupBeamRingPoint(clientLoc, 10.0, healing_range, BeamSprite(), HaloSprite(),
                                              0, 5, 0.7, 15.0, 1.0, healingColor, 10, 0);
                        TE_Send(list, count, 0.0);
                    }
                }

                if (alt_count > 0)
                {
                    if (transfusion_level > 0)
                    {
                        TE_SetupBeamRingPoint(clientLoc, transfusion_range-10.0, transfusion_range, BeamSprite(), HaloSprite(),
                                              0, 15, 0.5, 5.0, 0.0, transfusionColor, 10, 0);
                        TE_Send(alt_list, alt_count, 0.0);
                    }

                    if (detecting_level > 0)
                    {
                        TE_SetupBeamRingPoint(clientLoc, detecting_range-10.0, detecting_range, BeamSprite(), HaloSprite(),
                                              0, 10, 0.6, 10.0, 0.5, detectColor, 10, 0);
                        TE_Send(alt_list, alt_count, 0.0);
                    }

                    if (healing_aura_level > 0)
                    {
                        TE_SetupBeamRingPoint(clientLoc, healing_range-10.0, healing_range, BeamSprite(), HaloSprite(),
                                              0, 5, 0.7, 15.0, 1.0, healingColor, 10, 0);
                        TE_Send(alt_list, alt_count, 0.0);
                    }
                }
            }
        }
    }
    return Plugin_Continue;
}

SetupJetpack(client, level)
{
    if (m_JetpackAvailable)
    {
        if (level >= sizeof(g_JetpackFuel))
        {
            LogError("%d:%N has too many levels in ZergOverlord::Flyer level=%d, max=%d",
                     client,ValidClientIndex(client),level,sizeof(g_JetpackFuel));

            level = sizeof(g_JetpackFuel)-1;
        }
        GiveJetpack(client, g_JetpackFuel[level], g_JetpackRefuelTime[level]);
    }
}

ExcreteCreep(client, level)
{
    decl String:upgradeName[64];
    GetUpgradeName(raceID, excreteID, upgradeName, sizeof(upgradeName), client);

    new energy = GetEnergy(client);
    new amount = GetUpgradeEnergy(raceID,excreteID);
    if (energy < amount)
    {
        EmitEnergySoundToClient(client,Zerg);
        DisplayMessage(client, Display_Energy, "%t", "InsufficientEnergyFor", upgradeName, amount);
    }
    else if (GetRestriction(client,Restriction_PreventUltimates) ||
             GetRestriction(client,Restriction_Stunned))
    {
        PrepareSound(deniedWav);
        EmitSoundToClient(client,deniedWav);
        DisplayMessage(client, Display_Ultimate, "%t", "Prevented", upgradeName);
    }
    else if (HasCooldownExpired(client, raceID, excreteID))
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

        new lightning  = Lightning();
        new haloSprite = HaloSprite();
        static const color[4] = { 0, 255, 0, 255 };

        new Float:indexLoc[3];
        new Float:clientLoc[3];
        GetClientAbsOrigin(client, clientLoc);
        clientLoc[2] += 50.0; // Adjust trace position to the middle of the person instead of the feet.

        new count = 0;
        new team  = GetClientTeam(client);
        new Float:range = g_ExcreteCreepRange[level];
        for (new index=1;index<=MaxClients;index++)
        {
            if (client != index && IsClientInGame(index) &&
                IsPlayerAlive(index) && GetClientTeam(index) != team)
            {
                if (!GetImmunity(index,Immunity_Ultimates) &&
                    !GetImmunity(index,Immunity_Restore))
                {
                    GetClientAbsOrigin(index, indexLoc);
                    indexLoc[2] += 50.0;

                    if (IsPointInRange(clientLoc,indexLoc,range) &&
                        TraceTargetIndex(client, index, clientLoc, indexLoc))
                    {
                        if (!GetImmunity(index,Immunity_Detection))
                        {
                            SetOverrideVisiblity(index, 255);
                            if (m_SidewinderAvailable)
                                SidewinderDetectClient(index, true);

                            CreateTimer(10.0,RecloakPlayer,GetClientUserId(index),TIMER_FLAG_NO_MAPCHANGE);
                        }

                        if (GetGameType() == tf2 &&
                            !GetImmunity(index,Immunity_Uncloaking) &&
                            TF2_GetPlayerClass(index) == TFClass_Spy)
                        {
                            TF2_RemovePlayerDisguise(index);
                            TF2_RemoveCondition(index, TFCond_Cloaked);

                            new Float:cloakMeter = TF2_GetCloakMeter(index);
                            if (cloakMeter > 0.0 && cloakMeter <= 100.0)
                                TF2_SetCloakMeter(index, 0.0);

                            decl String:creepName[64];
                            Format(creepName,sizeof(creepName), "%T", "Creep", index);
                            DisplayMessage(index, Display_Enemy_Message, "%t", "HasUncloaked", client, creepName);
                        }

                        if (!GetImmunity(index,Immunity_MotionTaking) &&
                            !GetImmunity(index,Immunity_Restore) &&
                            !IsBurrowed(index))
                        {
                            TE_SetupBeamPoints(clientLoc,indexLoc, lightning, haloSprite,
                                               0, 1, 3.0, 10.0,10.0,5,50.0,color,255);
                            TE_SendQEffectToAll(client,index);
                            FlashScreen(index,RGBA_COLOR_BLUE);

                            decl String:creepName[64];
                            Format(creepName,sizeof(creepName), "%T", "Creep", index);
                            DisplayMessage(index, Display_Enemy_Message, "%t", "HasEnsnared", client, creepName);

                            FreezeEntity(index);
                            CreateTimer(10.0,UnfreezePlayer,GetClientUserId(index),TIMER_FLAG_NO_MAPCHANGE);
                            count++;
                        }
                    }
                }
            }
        }

        SetEnergy(client, energy-amount);

        if (count)
        {
            DisplayMessage(client, Display_Ultimate, "%t",
                           "ToEnsnareEnemies", upgradeName,
                           count);
        }
        else
        {
            DisplayMessage(client,Display_Ultimate, "%t",
                           "WithoutEffect", upgradeName);
        }

        CreateCooldown(client, raceID, excreteID);
    }
}

public Action:RecloakPlayer(Handle:timer,any:userid)
{
    new client = GetClientOfUserId(userid);
    if (client > 0)
    {
        SetOverrideVisiblity(client, -1);
        if (m_SidewinderAvailable)
            SidewinderDetectClient(client, false);
    }
    return Plugin_Stop;
}

SetupPiggyback(client, level)
{
    if (m_PiggybackAvailable)
    {
        if (level > 0)
            GivePiggyback(client, g_PiggybackMethod[level], g_PiggybackRange[level]);
        else
            TakePiggyback(client);
    }
}

public Action:OnPlayerPiggyback(rider, carrier, bool:pickedup, Float:distance)
{
    if (pickedup)
    {
        if (GetRace(carrier) == raceID)
        {
            new energy = GetEnergy(carrier);
            new amount = GetUpgradeEnergy(raceID,sacsID);
            if (energy < amount)
            {
                decl String:upgradeName[64];
                GetUpgradeName(raceID, sacsID, upgradeName, sizeof(upgradeName), carrier);
                DisplayMessage(carrier, Display_Energy, "%t", "InsufficientEnergyFor", upgradeName, amount);
                EmitEnergySoundToClient(carrier,Zerg);
                return Plugin_Stop;
            }
            else if (GetRestriction(carrier,Restriction_PreventUltimates) ||
                     GetRestriction(carrier,Restriction_Stunned))
            {
                PrepareSound(deniedWav);
                EmitSoundToClient(carrier,deniedWav);

                decl String:upgradeName[64];
                GetUpgradeName(raceID, sacsID, upgradeName, sizeof(upgradeName), carrier);
                DisplayMessage(carrier, Display_Ultimate, "%t", "Prevented", upgradeName);
            }
            else if (IsBurrowed(rider))
            {
                PrepareSound(errorWav);
                EmitSoundToClient(carrier,errorWav);
                DisplayMessage(carrier, Display_Ultimate, "%t", "TargetIsBurrowed");
            }
            else if (GameType == tf2 && TF2_HasTheFlag(rider))
            {
                PrepareSound(deniedWav);
                EmitSoundToClient(carrier,deniedWav);

                decl String:upgradeName[64];
                GetUpgradeName(raceID, sacsID, upgradeName, sizeof(upgradeName), carrier);
                DisplayMessage(carrier, Display_Ultimate, "%t", "CantUseOnFlagCarrier", upgradeName);
            }
            else
            {
                SetEnergy(carrier, energy-amount);
                if (GetUpgradeLevel(carrier,raceID,sacsID) >= 4)
                {
                    SetVisibility(rider, BasicVisibility, 0, 
                                  .mode=RENDER_TRANSCOLOR,
                                  .fx=RENDERFX_NONE);
                }
            }
        }
    }
    else
        SetVisibility(rider, NormalVisibility);

    return Plugin_Continue;
}

OverseerMorph(client)
{
    new energy = GetEnergy(client);
    new amount = GetUpgradeEnergy(raceID,overseerID);
    new accumulated = GetAccumulatedEnergy(client, raceID);
    if (accumulated + energy < amount)
    {
        ShowEnergy(client);
        EmitEnergySoundToClient(client,Zerg);

        decl String:upgradeName[64];
        GetUpgradeName(raceID, overseerID, upgradeName, sizeof(upgradeName), client);
        DisplayMessage(client, Display_Ultimate, "%t", "InsufficientAccumulatedEnergyFor",
                       upgradeName, amount);
    }
    else if (GetRestriction(client,Restriction_PreventUltimates) ||
             GetRestriction(client,Restriction_Stunned))
    {
        PrepareSound(deniedWav);
        EmitSoundToClient(client,deniedWav);
        DisplayMessage(client, Display_Ultimate,
                       "%t", "PreventedFromOverseer");
    }
    else if (HasCooldownExpired(client, raceID, overseerID))
    {
        if (g_overseerRace < 0)
            g_overseerRace = FindRace("overseer");

        if (g_overseerRace > 0)
        {
            new Float:clientLoc[3];
            GetClientAbsOrigin(client, clientLoc);
            clientLoc[2] += 40.0; // Adjust position to the middle

            TE_SetupSmoke(clientLoc, SmokeSprite(), 8.0, 2);
            TE_SendEffectToAll();

            TE_SetupGlowSprite(clientLoc,(GetClientTeam(client) == 3) ? BlueGlow() : RedGlow(),
                               5.0, 40.0, 255);
            TE_SendEffectToAll();

            accumulated -= amount;
            if (accumulated < 0)
            {
                SetEnergy(client, energy+accumulated);
                accumulated = 0;
            }
            SetAccumulatedEnergy(client, raceID, accumulated);

            DisplayMessage(client,Display_Ultimate,
                           " You have emerged as an {blue}Overseer{default}.");

            ChangeRace(client, g_overseerRace, true, false);
        }
        else
        {
            PrepareSound(deniedWav);
            EmitSoundToClient(client,deniedWav);
            LogError("***The Zerg Overseer race is not Available!");

            decl String:upgradeName[64];
            GetUpgradeName(raceID, overseerID, upgradeName, sizeof(upgradeName), client);
            DisplayMessage(client, Display_Ultimate, "%t", "IsNotAvailable", upgradeName);
        }
    }
}
