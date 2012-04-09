/**
 * vim: set ai et ts=4 sw=4 :
 * File: ProtossProbe.sp
 * Description: The Protoss Probe race for SourceCraft.
 * Author(s): -=|JFH|=-Naris
 */
 
#pragma semicolon 1

#include <sourcemod>
#include <sdktools>
#include <raytrace>
#include <range>

#undef REQUIRE_EXTENSIONS
#include <tf2>
#include <tf2_objects>
#include <tf2_player>
#define REQUIRE_EXTENSIONS

#undef REQUIRE_PLUGIN
#include <remote>
#include <amp_node>
#include <ztf2grab>
#include <tf2teleporter>
#define REQUIRE_PLUGIN

#include "sc/SourceCraft"
#include "sc/RecallStructure"
#include "sc/clienttimer"
#include "sc/SupplyDepot"
#include "sc/maxhealth"
#include "sc/menuitemt"
#include "sc/weapons"
#include "sc/shields"

#include "effect/Smoke"
#include "effect/RedGlow"
#include "effect/BlueGlow"
#include "effect/Explosion"
#include "effect/SendEffects"
#include "effect/FlashScreen"

#define MAXENTITIES 2048

new const String:summonWav[]    = "sc/pprrdy00.wav";
new const String:deathWav[]     = "sc/pprdth00.wav";
new const String:pylonWav[]     = "sc/ppywht00.wav";
new const String:resetWav[]     = "sc/unrwht00.wav";
new const String:forgeWav[]     = "sc/pfowht00.wav";
new const String:cannonWav[]    = "sc/phohit00.wav";
new const String:rechargeWav[]  = "sc/transmission.wav";
//new const String:buttonWav[]  = "buttons/button14.wav";
//new const String:deniedWav[]  = "sc/buzz.wav";
//new const String:errorWav[]   = "sc/perror.mp3";

new Float:g_InitialShields[]    = { 0.0, 0.25, 0.50, 0.75, 1.00 };
new Float:g_ShieldsPercent[][2] = { {0.00, 0.00},
                                    {0.00, 0.10},
                                    {0.00, 0.30},
                                    {0.10, 0.50},
                                    {0.20, 0.60} };

new Float:g_ForgeFactor[]       = { 1.0, 1.10, 1.20, 1.40, 1.60 };

new Float:g_WarpGateRate[]      = { 0.0, 8.0, 6.0, 3.0, 1.0 };

new g_CannonChance[]            = { 0, 20, 40, 60, 90 };
new Float:g_CannonPercent[]     = { 0.0, 0.14, 0.27, 0.43, 0.63 };

new m_DarkPylonAlpha[]          = { 255, 150, 100, 50, 10, 0 };

new Float:g_AmpRange[][][] =
{
    {   // Slow, Undisguise, Decloak, Really Slow
        { 0.0,    0.0,   0.0,   0.0 },
        { 0.0 , 130.0, 150.0, 170.0 },
        { 0.0 , 150.0, 170.0, 190.0 },
        { 0.0 , 170.0, 190.0, 210.0 },
        { 0.0 , 190.0, 210.0, 230.0 }
    },
    {   // Taunt
        { 0.0,    0.0,   0.0,   0.0 },
        { 0.0 , 100.0, 150.0, 200.0 },
        { 0.0 , 150.0, 200.0, 250.0 },
        { 0.0 , 200.0, 250.0, 300.0 },
        { 0.0 , 250.0, 300.0, 350.0 }
    },
    {   // Krit, Uber
        { 0.0,    0.0,   0.0,   0.0 },
        { 0.0 , 120.0, 150.0, 180.0 },
        { 0.0 , 150.0, 180.0, 210.0 },
        { 0.0 , 180.0, 210.0, 240.0 },
        { 0.0 , 210.0, 240.0, 280.0 }
    },
    {   // Buff, Jar, Fire, Milk, Defense
        { 0.0,    0.0,   0.0,   0.0 },
        { 0.0 , 100.0, 150.0, 200.0 },
        { 0.0 , 150.0, 200.0, 250.0 },
        { 0.0 , 200.0, 250.0, 300.0 },
        { 0.0 , 250.0, 300.0, 350.0 }
    }
};

new raceID, shieldsID, batteriesID, forgeID, warpGateID, cannonID;
new recallStructureID, pylonID, amplifierID, phasePrismID;

new g_phasePrismRace = -1;

new cfgAllowSentries;
new bool:cfgAllowTeleport;
new bool:cfgAllowInvisibility;

new bool:m_BuildAvailable = false;
new bool:m_AmpNodeAvailable = false;
new bool:m_TeleporterAvailable = false;
//stock bool:m_GravgunAvailable = false;

new bool:m_IsDarkPylon[MAXPLAYERS+1];
new Float:m_CannonTime[MAXPLAYERS+1];

public Plugin:myinfo = 
{
    name = "SourceCraft Race - Protoss Probe",
    author = "-=|JFH|=-Naris",
    description = "The Protoss Probe race for SourceCraft.",
    version = SOURCECRAFT_VERSION,
    url = "http://jigglysfunhouse.net/"
};

public OnPluginStart()
{
    LoadTranslations("sc.objects.phrases.txt");
    LoadTranslations("sc.common.phrases.txt");
    LoadTranslations("sc.recall.phrases.txt");
    LoadTranslations("sc.probe.phrases.txt");

    if (GetGameType() == tf2)
    {
        if (!HookEvent("player_upgradedobject", PlayerUpgradedObject))
            SetFailState("Could not hook the player_builtobject event.");

        if (!HookEventEx("teamplay_round_win",EventRoundOver,EventHookMode_PostNoCopy))
            SetFailState("Could not hook the teamplay_round_win event.");

        if (!HookEventEx("teamplay_round_stalemate",EventRoundOver,EventHookMode_PostNoCopy))
            SetFailState("Could not hook the teamplay_round_stalemate event.");
    }
    else if (GameType == dod)
    {
        if (!HookEventEx("dod_round_win",EventRoundOver,EventHookMode_PostNoCopy))
            SetFailState("Could not hook the dod_round_win event.");

        if (!HookEventEx("dod_game_over",EventRoundOver,EventHookMode_PostNoCopy))
            SetFailState("Could not hook the dod_game_over event.");
    }
    else if (GameType == cstrike)
    {
        if (!HookEventEx("end",EventRoundOver,EventHookMode_PostNoCopy))
            SetFailState("Could not hook the round_end event.");
    }

    if (IsSourceCraftLoaded())
        OnSourceCraftReady();
}

public OnSourceCraftReady()
{
    raceID          = CreateRace("probe", 80, 0, 33, .energy_rate=2,
                                 .faction=Protoss, .type=Robotic);

    shieldsID       = AddUpgrade(raceID, "shields", .energy=1);

    if (GameType == tf2)
    {
        batteriesID = AddUpgrade(raceID, "batteries");

        cfgAllowSentries = GetConfigNum("allow_sentries", 2);
        if (cfgAllowSentries >= 2)
        {
            forgeID = AddUpgrade(raceID, "forge");

            GetConfigFloatArray("factor", g_ForgeFactor, sizeof(g_ForgeFactor),
                                g_ForgeFactor, raceID, forgeID);
        }
        else
        {
            forgeID = AddUpgrade(raceID, "forge", 0, 99,0, .desc="%NotAllowed");
            LogMessage("Disabling Protoss Probe:Shield Batteries and Forge due to configuration: sc_allow_sentries=%d",
                       cfgAllowSentries);
        }

        if (m_TeleporterAvailable || LibraryExists("tf2teleporter"))
        {
            warpGateID = AddUpgrade(raceID, "warp_gate");

            GetConfigFloatArray("rate", g_WarpGateRate, sizeof(g_WarpGateRate),
                                g_WarpGateRate, raceID, warpGateID);

            if (!m_TeleporterAvailable)
            {
                ControlTeleporter(true);
                m_TeleporterAvailable = true;
            }
        }
        else
        {
            LogMessage("tf2teleporter is not available");
            warpGateID = AddUpgrade(raceID, "warp_gate", 0, 99, 0,
                                    .desc="%NotAvailable");
        }
    }
    else
    {
        batteriesID = AddUpgrade(raceID, "batteries", 0, 0, .desc="%NotApplicable");
        forgeID     = AddUpgrade(raceID, "forge", 0, 99,0, .desc="%NotApplicable");
        warpGateID  = AddUpgrade(raceID, "warp_gate", 0, 99, 0, .desc="%NotApplicable");
    }

    cannonID = AddUpgrade(raceID, "cannon", 0, 8, .energy=2);

    if (GameType == tf2)
    {
        // Ultimate 1
        cfgAllowTeleport = bool:GetConfigNum("allow_teleport", true);
        if (cfgAllowSentries >= 1 && cfgAllowTeleport)
        {
            recallStructureID = AddUpgrade(raceID, "recall_structure", 1, 8, .energy=30,
                                           .vespene=5, .cooldown=5.0);

            if (!m_GravgunAvailable && LibraryExists("ztf2grab"))
            {
                ControlZtf2grab(true);
                m_GravgunAvailable = true;
            }
        }
        else
        {
            recallStructureID = AddUpgrade(raceID, "recall_structure", 1, 99,0, .desc="%NotAllowed");
            LogMessage("Disabling Protoss Probe:Recall Structure due to configuration: sc_allow_sentries=%d, sc_allow_teleport=%d",
                        cfgAllowSentries, cfgAllowTeleport);
        }

        // Ultimate 2
        cfgAllowInvisibility = bool:GetConfigNum("allow_invisibility", true);
        if (cfgAllowInvisibility)
        {
            pylonID = AddUpgrade(raceID, "pylon", 2, 10, .energy=30, .cooldown=2.0);

            GetConfigArray("alpha", m_DarkPylonAlpha, sizeof(m_DarkPylonAlpha),
                           m_DarkPylonAlpha, raceID, pylonID);
        }
        else
        {
            pylonID = AddUpgrade(raceID, "pylon", 2, 99, 2, .desc="%Not Allowed");
            LogMessage("Disabling Protoss Probe:Dark Pylon due to configuration: sc_allow_invisibility=%d",
                        cfgAllowInvisibility);
        }

        // Ultimate 3
        if ((m_AmpNodeAvailable || LibraryExists("amp_node")) &&
            (m_BuildAvailable || LibraryExists("remote")))
        {
            amplifierID = AddUpgrade(raceID, "amplifier", 3, 8, .energy=30,
                                     .vespene=5, .cooldown=10.0);

            for (new type=0; type < sizeof(g_AmpRange); type++)
            {
                decl String:section[32];
                Format(section, sizeof(section), "amplifier_type_%d", type);

                for (new level=0; level < sizeof(g_AmpRange[]); level++)
                {
                    decl String:key[32];
                    Format(key, sizeof(key), "range_level_%d", level);
                    GetConfigFloatArray(key, g_AmpRange[type][level], sizeof(g_AmpRange[][]),
                                        g_AmpRange[type][level], raceID, amplifierID, section);
                }
            }

            if (!m_AmpNodeAvailable)
            {
                ControlAmpNode(true);
                m_AmpNodeAvailable = true;
            }

            if (!m_BuildAvailable)
            {
                ControlBuild(true);
                m_BuildAvailable = true;
            }
        }
        else
        {
            LogMessage("amp_node and/or remote are not available");
            amplifierID = AddUpgrade(raceID, "amplifier", 3, 99, 0,
                                     .desc="%NotAvailable");
        }
    }
    else
    {
        // Ultimate 1
        recallStructureID = AddUpgrade(raceID, "recall_structure", 1, 99,0, .desc="%NotApplicable");

        // Ultimate 2
        pylonID = AddUpgrade(raceID, "pylon", 2, 99, 2, .desc="%NotApplicable");

        // Ultimate 3
        amplifierID = AddUpgrade(raceID, "amplifier", 3, 99, 0, .desc="%NotApplicable");
        //LogMessage("sentries, teleporters and/or dispensers are not supported by this mod");
    }

    // Ultimate 4
    phasePrismID = AddUpgrade(raceID, "prism", 4, 14, 1,
                              .energy=300, .cooldown=60.0);

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

    GetConfigArray("chance", g_CannonChance, sizeof(g_CannonChance),
                   g_CannonChance, raceID, cannonID);

    GetConfigFloatArray("damage_percent", g_CannonPercent, sizeof(g_CannonPercent),
                        g_CannonPercent, raceID, cannonID);
}

public OnLibraryAdded(const String:name[])
{
    if (StrEqual(name, "tf2teleporter"))
    {
        if (GetGameType() == tf2 && !m_TeleporterAvailable)
        {
            m_TeleporterAvailable = true;
            ControlTeleporter(true);
        }
    }
    else if (StrEqual(name, "remote"))
    {
        if (GetGameType() == tf2 && !m_BuildAvailable)
        {
            ControlBuild(true);
            m_BuildAvailable = true;
        }
    }
    else if (StrEqual(name, "amp_node"))
    {
        if (GetGameType() == tf2 && !m_AmpNodeAvailable)
        {
            ControlAmpNode(true);
            m_AmpNodeAvailable = true;
        }
    }
    else if (StrEqual(name, "ztf2grab"))
    {
        if (!m_GravgunAvailable)
        {
            ControlZtf2grab(true);
            m_GravgunAvailable = true;
        }
    }
}

public OnLibraryRemoved(const String:name[])
{
    if (StrEqual(name, "tf2teleporter"))
        m_TeleporterAvailable = false;
    else if (StrEqual(name, "ztf2grab"))
        m_GravgunAvailable = false;
    else if (StrEqual(name, "remote"))
        m_BuildAvailable = false;
    else if (StrEqual(name, "amp_node"))
        m_AmpNodeAvailable = false;
}

public OnMapStart()
{
    SetupExplosion(false, false);
    SetupSmokeSprite(false, false);
    SetupBlueGlow(false, false);
    SetupRedGlow(false, false);

    SetupRecallSounds(false, false);

    //SetupSound(buttonWav, false, false, false);
    SetupSound(summonWav, true, false, false);
    SetupSound(pylonWav, true, false, false);
    SetupSound(resetWav, true, false, false);
    SetupSound(forgeWav, true, false, false);
    SetupSound(deathWav, true, false, false);
    SetupSound(errorWav, true, false, false);
    SetupSound(deniedWav, true, false, false);
    SetupSound(cannonWav, true, false, false);
    SetupSound(rechargeWav, true, false, false);
}

public OnMapEnd()
{
    ResetAllClientTimers();
    ResetAllShieldTimers();
}

public OnPlayerAuthed(client)
{
    m_IsDarkPylon[client] = false;
    m_CannonTime[client] = 0.0;
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
        ResetShields(client);
        KillClientTimer(client);
        KillShieldTimer(client);

        if (m_TeleporterAvailable)
            SetTeleporter(client, 0.0);

        if (m_BuildAvailable)
            DestroyBuildings(client, false);

        return Plugin_Handled;
    }
    else
    {
        if (g_phasePrismRace < 0)
            g_phasePrismRace = FindRace("prism");

        if (oldrace == g_phasePrismRace &&
            GetCooldownExpireTime(client, raceID, phasePrismID) <= 0.0)
        {
            CreateCooldown(client, raceID, phasePrismID,
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
        m_CannonTime[client] = 0.0;
        m_IsDarkPylon[client] = false;

        new warp_gate_level = GetUpgradeLevel(client,raceID,warpGateID);
        if (warp_gate_level > 0)
            SetupTeleporter(client, warp_gate_level);

        new shields_level = GetUpgradeLevel(client,raceID,shieldsID);
        SetupShields(client, shields_level, g_InitialShields, g_ShieldsPercent);

        if (shields_level > 0)
            CreateShieldTimer(client);

        if (IsPlayerAlive(client))
        {
            new battery_level = GetUpgradeLevel(client,raceID,batteriesID);
            if (shields_level ||
                (battery_level && GameType == tf2 &&
                 TF2_GetPlayerClass(client) == TFClass_Engineer))
            {
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
        if (upgrade==warpGateID)
            SetupTeleporter(client, new_level);
        else if (upgrade==shieldsID)
        {
            SetupShields(client, new_level, g_InitialShields,
                         g_ShieldsPercent, .upgrade=true);

            if (new_level ||
                (GetUpgradeLevel(client,raceID,batteriesID) &&
                 GameType == tf2 && TF2_GetPlayerClass(client) == TFClass_Engineer))
            {
                if (IsValidClient(client) && IsPlayerAlive(client))
                {
                    CreateClientTimer(client, 1.0, Regeneration,
                                      TIMER_REPEAT|TIMER_FLAG_NO_MAPCHANGE);

                    if (new_level > 0)
                        CreateShieldTimer(client);
                }
            }
            else
                KillClientTimer(client);
        }
        else if (upgrade==batteriesID)
        {
            if (GetUpgradeLevel(client,raceID,shieldsID) ||
                (new_level && GameType == tf2 && TF2_GetPlayerClass(client) == TFClass_Engineer))
            {
                if (IsPlayerAlive(client))
                {
                    CreateClientTimer(client, 1.0, Regeneration,
                                      TIMER_REPEAT|TIMER_FLAG_NO_MAPCHANGE);
                }
            }
            else
                KillClientTimer(client);
        }
    }
}

public OnUltimateCommand(client,race,bool:pressed,arg)
{
    if (race==raceID)
    {
        switch (arg)
        {
            case 4:
            {
                new phase_prism_level=GetUpgradeLevel(client,race,phasePrismID);
                if (phase_prism_level > 0)
                {
                    if (!pressed)
                        SummonPhasePrism(client);
                }
                else
                {
                    new amplifier_level = GetUpgradeLevel(client,raceID,amplifierID);
                    if (amplifier_level > 0)
                    {
                        if (!pressed)
                            WarpInAmplifier(client, amplifier_level);
                    }
                    else
                    {
                        new pylon_level=GetUpgradeLevel(client,race,pylonID);
                        if (GameType == tf2 && pylon_level > 0 && cfgAllowInvisibility)
                        {
                            if (pressed)
                                DarkPylon(client, pylon_level);
                        }
                    }
                }
            }
            case 3:
            {
                new amplifier_level = GetUpgradeLevel(client,raceID,amplifierID);
                if (amplifier_level > 0)
                {
                    if (!pressed)
                        WarpInAmplifier(client, amplifier_level);
                }
                else
                {
                    new pylon_level=GetUpgradeLevel(client,race,pylonID);
                    if (GameType == tf2 && pylon_level > 0 && cfgAllowInvisibility)
                    {
                        if (pressed)
                            DarkPylon(client, pylon_level);
                    }
                    else if (!pressed)
                    {
                        new phase_prism_level=GetUpgradeLevel(client,race,phasePrismID);
                        if (phase_prism_level > 0)
                            SummonPhasePrism(client);
                    }
                }
            }
            case 2:
            {
                new pylon_level=GetUpgradeLevel(client,race,pylonID);
                if (GameType == tf2 && pylon_level > 0 && cfgAllowInvisibility)
                {
                    if (pressed)
                        DarkPylon(client, pylon_level);
                }
                else if (!pressed)
                {
                    new phase_prism_level=GetUpgradeLevel(client,race,phasePrismID);
                    if (phase_prism_level > 0)
                        SummonPhasePrism(client);
                }
            }
            default:
            {
                new recall_structure_level = GetUpgradeLevel(client,race,recallStructureID);
                if (GameType == tf2 && recall_structure_level > 0 && cfgAllowTeleport &&
                    (cfgAllowSentries >= 2) || (cfgAllowSentries >= 1 && GameType == tf2 &&
                                                TF2_GetPlayerClass(client) != TFClass_Engineer))
                {
                    if (pressed)
                        RecallStructure(client,race,recallStructureID, true, cfgAllowSentries == 1);
                }
                else
                {
                    new pylon_level=GetUpgradeLevel(client,race,pylonID);
                    if (GameType == tf2 && pylon_level > 0 && cfgAllowInvisibility)
                    {
                        if (pressed)
                            DarkPylon(client, pylon_level);
                    }
                    else if (!pressed)
                    {
                        new phase_prism_level=GetUpgradeLevel(client,race,phasePrismID);
                        if (phase_prism_level > 0)
                            SummonPhasePrism(client);
                    }
                }
            }
        }
    }
}

// Events
public OnPlayerSpawnEvent(Handle:event, client, race)
{
    if (race == raceID)
    {
        m_CannonTime[client] = 0.0;

        PrepareSound(summonWav);
        EmitSoundToAll(summonWav,client);

        new shields_level = GetUpgradeLevel(client,raceID,shieldsID);
        SetupShields(client, shields_level, g_InitialShields, g_ShieldsPercent);

        if (shields_level > 0)
            CreateShieldTimer(client);

        new battery_level=GetUpgradeLevel(client,raceID,batteriesID);
        if (shields_level ||
            (battery_level && GameType == tf2 &&
             TF2_GetPlayerClass(client) == TFClass_Engineer))
        {
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
    if (!from_sc && victim_index != attacker_index)
    {
        damage += absorbed;

        if (attacker_race == raceID)
        {
            changed |= PhotonCannon(damage, victim_index, attacker_index);
        }

        if (assister_race == raceID)
        {
            changed |= PhotonCannon(damage, victim_index, assister_index);
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
    }
    else
    {
        if (g_phasePrismRace < 0)
            g_phasePrismRace = FindRace("prism");

        if (victim_race == g_phasePrismRace &&
            GetCooldownExpireTime(victim_index, raceID, phasePrismID) <= 0.0)
        {
            CreateCooldown(victim_index, raceID, phasePrismID,
                           .type=Cooldown_CreateNotify
                                |Cooldown_AlwaysNotify);
        }
    }
}

public EventRoundOver(Handle:event,const String:name[],bool:dontBroadcast)
{
    for (new index=1;index<=MaxClients;index++)
    {
        m_IsDarkPylon[index] = false;
    }
}

public OnPlayerBuiltObject(Handle:event, client, object, TFObjectType:type)
{
    if (object > 0 && type != TFObject_Sapper)
    {
        if (IsValidClient(client) && GetRace(client) == raceID &&
            !GetRestriction(client, Restriction_PreventUpgrades) &&
            !GetRestriction(client, Restriction_Stunned))
        {
            if (cfgAllowSentries >= 1 && GetUpgradeLevel(client,raceID,forgeID) > 0)
            {
                new Float:time = (GetEntPropFloat(object, Prop_Send, "m_flPercentageConstructed") >= 1.0) ? 0.1 : 10.0;
                CreateTimer(time, ForgeTimer, EntIndexToEntRef(object), TIMER_FLAG_NO_MAPCHANGE);
            }

            new pylon_level = cfgAllowInvisibility && m_IsDarkPylon[client] ? GetUpgradeLevel(client,raceID,pylonID) : 0;
            if (pylon_level > 0)
            {
                SetEntityRenderColor(object, 255, 255, 255, m_DarkPylonAlpha[pylon_level]);
                SetEntityRenderMode(object,RENDER_TRANSCOLOR);
            }
        }
    }
}

public PlayerUpgradedObject(Handle:event,const String:name[],bool:dontBroadcast)
{
    new object = GetEventInt(event,"index");
    if (object > 0)
    {
        new client = GetClientOfUserId(GetEventInt(event,"userid"));
        if (IsValidClient(client) && GetRace(client) == raceID &&
            !GetRestriction(client, Restriction_PreventUpgrades) &&
            !GetRestriction(client, Restriction_Stunned))
        {
            if (cfgAllowSentries >= 1 && GetUpgradeLevel(client,raceID,forgeID) > 0)
                CreateTimer(0.1, ForgeTimer, EntIndexToEntRef(object), TIMER_FLAG_NO_MAPCHANGE);

            new pylon_level = m_IsDarkPylon[client] ? GetUpgradeLevel(client,raceID,pylonID) : 0;
            if (pylon_level > 0 && cfgAllowInvisibility)
            {
                SetEntityRenderColor(object, 255, 255, 255, m_DarkPylonAlpha[pylon_level]);
                SetEntityRenderMode(object,RENDER_TRANSCOLOR);
            }
        }
    }
}

public Action:ForgeTimer(Handle:timer,any:ref)
{
    new object = EntRefToEntIndex(ref);
    if (object > 0 && IsValidEntity(object) && IsValidEdict(object))
    {
        new builder = GetEntPropEnt(object, Prop_Send, "m_hBuilder");
        if (builder > 0 && GetRace(builder) == raceID &&
            !GetRestriction(builder, Restriction_PreventUpgrades) &&
            !GetRestriction(builder, Restriction_Stunned))
        {
            if (GetEntPropFloat(object, Prop_Send, "m_flPercentageConstructed") >= 1.0)
            {
                new build_level = GetUpgradeLevel(builder,raceID,forgeID);
                if (build_level > 0 && cfgAllowSentries >= 1)
                {
                    new iLevel = GetEntProp(object, Prop_Send, "m_bMiniBuilding") ? 0 : 
                                 GetEntProp(object, Prop_Send, "m_iUpgradeLevel");

                    //new health = GetEntProp(object, Prop_Send, "m_iHealth");
                    new health = RoundToNearest(float(TF2_SentryHealth[iLevel]) * g_ForgeFactor[build_level]);

                    new maxHealth = TF2_SentryHealth[4]; //[iLevel+1];
                    if (health > maxHealth)
                        health = maxHealth;

                    if (health > GetEntProp(object, Prop_Send, "m_iMaxHealth"))
                        SetEntProp(object, Prop_Send, "m_iMaxHealth", health);

                    SetEntProp(object, Prop_Send, "m_iHealth", health);

                    if (TF2_GetObjectType(object) == TFObject_Sentry)
                    {
                        new maxShells = TF2_MaxSentryShells[4]; //[iLevel+1];
                        //new iShells = GetEntProp(object, Prop_Send, "m_iAmmoShells");
                        new iShells = RoundToNearest(float(TF2_MaxSentryShells[iLevel]) *g_ForgeFactor[build_level]);
                        if (iShells > maxShells)
                            iShells = maxShells;

                        SetEntProp(object, Prop_Send, "m_iAmmoShells", iShells);

                        if (iLevel > 2)
                        {
                            new maxRockets = TF2_MaxSentryRockets[4]; //[iLevel+1];
                            //new iRockets = GetEntProp(object, Prop_Send, "m_iAmmoRockets");
                            new iRockets = RoundToNearest(float(TF2_MaxSentryRockets[iLevel]) *g_ForgeFactor[build_level]);
                            if (iRockets > maxRockets)
                                iRockets = maxRockets;

                            SetEntProp(object, Prop_Send, "m_iAmmoRockets", iRockets);
                        }
                    }

                    PrepareSound(forgeWav);
                    EmitSoundToAll(forgeWav,object);
                }

                new pylon_level = m_IsDarkPylon[builder] ? GetUpgradeLevel(builder,raceID,pylonID) : 0;
                if (pylon_level > 0 && cfgAllowInvisibility)
                {
                    SetEntityRenderColor(object, 255, 255, 255, m_DarkPylonAlpha[pylon_level]);
                    SetEntityRenderMode(object,RENDER_TRANSCOLOR);
                }
            }
            else
                CreateTimer(1.0, ForgeTimer, ref, TIMER_FLAG_NO_MAPCHANGE);
        }
    }
    return Plugin_Stop;
}

public Action:Regeneration(Handle:timer, any:userid)
{
    new client = GetClientOfUserId(userid);
    if (IsValidClient(client) && GetRace(client) == raceID &&
        !GetRestriction(client,Restriction_PreventUpgrades) &&
        !GetRestriction(client,Restriction_Stunned))
    {
        new shields_level = GetUpgradeLevel(client,raceID,shieldsID);
        if (shields_level > 0)
            RegenerateShields(client, shields_level, g_InitialShields);

        if (GetGameType() != tf2)
            return Plugin_Continue;

        new battery_level = GetUpgradeLevel(client,raceID,batteriesID);
        if (battery_level > 0 && cfgAllowSentries >= 1 && GameType == tf2 &&
            TF2_GetPlayerClass(client) == TFClass_Engineer)
        {
            new maxentities = GetMaxEntities();
            for (new i = MaxClients + 1; i <= maxentities; i++)
            {
                if (IsValidEntity(i) && IsValidEdict(i))
                {
                    new TFExtObjectType:type=TF2_GetExtObjectTypeFromClass(i);
                    if (type != TFExtObject_Unknown)
                    {
                        if (GetEntPropEnt(i, Prop_Send, "m_hBuilder") == client &&
                            GetEntPropFloat(i, Prop_Send, "m_flPercentageConstructed") >= 1.0)
                        {
                            new iLevel = GetEntProp(i, Prop_Send, "m_iUpgradeLevel");
                            if (iLevel < 3)
                            {
                                new iUpgrade = GetEntProp(i, Prop_Send, "m_iUpgradeMetal");
                                if (iUpgrade < TF2_MaxUpgradeMetal)
                                {
                                    iUpgrade += battery_level*2;
                                    if (iUpgrade > TF2_MaxUpgradeMetal)
                                        iUpgrade = TF2_MaxUpgradeMetal;
                                    SetEntProp(i, Prop_Send, "m_iUpgradeMetal", iUpgrade);
                                }
                            }
                            else
                            {
                                switch (type)
                                {
                                    case TFExtObject_Dispenser:
                                    {
                                        new iMetal = GetEntProp(i, Prop_Send, "m_iAmmoMetal");
                                        if (iMetal < TF2_MaxDispenserMetal)
                                        {
                                            iMetal += battery_level*2;
                                            if (iMetal > TF2_MaxDispenserMetal)
                                                iMetal = TF2_MaxDispenserMetal;
                                            SetEntProp(i, Prop_Send, "m_iAmmoMetal", iMetal);
                                        }
                                    }
                                    case TFExtObject_Sentry:
                                    {
                                        new maxShells = TF2_MaxSentryShells[iLevel];
                                        new iShells = GetEntProp(i, Prop_Send, "m_iAmmoShells");
                                        if (iShells < maxShells)
                                        {
                                            iShells += battery_level*2;
                                            if (iShells > maxShells)
                                                iShells = maxShells;
                                            SetEntProp(i, Prop_Send, "m_iAmmoShells", iShells);
                                        }

                                        new maxRockets = TF2_MaxSentryRockets[iLevel];
                                        new iRockets = GetEntProp(i, Prop_Send, "m_iAmmoRockets");
                                        if (iRockets < maxRockets)
                                        {
                                            iRockets += battery_level;
                                            if (iRockets > maxRockets)
                                                iRockets = maxRockets;
                                            SetEntProp(i, Prop_Send, "m_iAmmoRockets", iRockets);
                                        }
                                    }
                                }
                            }

                            new maxHealth = GetEntProp(i, Prop_Send, "m_iMaxHealth");
                            new health = GetEntProp(i, Prop_Send, "m_iHealth");
                            if (health < maxHealth)
                            {
                                health += (battery_level*2);
                                if (health > maxHealth)
                                    health = maxHealth;

                                SetEntProp(i, Prop_Send, "m_iHealth", health);
                            }
                        }
                    }
                }
            }
        }
    }
    return Plugin_Continue;
}

SetupTeleporter(client, level)
{
    if (m_TeleporterAvailable)
        SetTeleporter(client, g_WarpGateRate[level]);
}

bool:PhotonCannon(damage, victim_index, index)
{
    new cannon_level = GetUpgradeLevel(index,raceID,cannonID);
    if (cannon_level > 0)
    {
        if (!GetRestriction(index, Restriction_PreventUpgrades) &&
            !GetRestriction(index, Restriction_Stunned) &&
            !GetImmunity(victim_index,Immunity_HealthTaking) &&
            !GetImmunity(victim_index,Immunity_Upgrades) &&
            !IsInvulnerable(victim_index))
        {
            new Float:lastTime = m_CannonTime[index];
            new Float:interval = GetGameTime() - lastTime;
            if (lastTime == 0.0 || interval > 0.25)
            {
                if (GetRandomInt(1,100) <= g_CannonChance[cannon_level])
                {
                    new health_take = RoundToFloor(float(damage)*g_CannonPercent[cannon_level]);
                    if (health_take > 0)
                    {
                        new energy = GetEnergy(index);
                        new amount = GetUpgradeEnergy(raceID,cannonID);
                        if (energy >= amount)
                        {
                            if (interval == 0.0 || interval >= 2.0)
                            {
                                new Float:Origin[3];
                                GetClientAbsOrigin(victim_index, Origin);
                                Origin[2] += 5;

                                TE_SetupExplosion(Origin, Explosion(), 5.0, 1,0, 5, 10);
                                TE_SendEffectToAll();
                            }

                            PrepareSound(cannonWav);
                            EmitSoundToAll(cannonWav,victim_index);
                            FlashScreen(victim_index,RGBA_COLOR_RED);

                            SetEnergy(index, energy-amount);
                            m_CannonTime[index] = GetGameTime();
                            HurtPlayer(victim_index, health_take, index,
                                       "sc_photon_cannon", .type=DMG_ENERGYBEAM,
                                       .in_hurt_event=true);
                            return true;
                        }
                    }
                }
            }
        }
    }
    return false;
}

DarkPylon(client, level)
{
    if (m_IsDarkPylon[client])
    {
        PrepareSound(deniedWav);
        EmitSoundToClient(client,deniedWav);
        DisplayMessage(client,Display_Ultimate,
                       "%t", "PylonAlreadyActive");
    }
    else
    {
        new energy = GetEnergy(client);
        new amount = GetUpgradeEnergy(raceID,pylonID);
        if (energy < amount)
        {
            decl String:upgradeName[64];
            GetUpgradeName(raceID, pylonID, upgradeName, sizeof(upgradeName), client);
            DisplayMessage(client, Display_Energy, "%t", "InsufficientEnergyFor", upgradeName, amount);
            EmitEnergySoundToClient(client,Protoss);
        }
        else if (GetRestriction(client,Restriction_PreventUltimates) ||
                 GetRestriction(client,Restriction_Stunned))
        {
            PrepareSound(deniedWav);
            EmitSoundToClient(client,deniedWav);

            decl String:upgradeName[64];
            GetUpgradeName(raceID, pylonID, upgradeName, sizeof(upgradeName), client);
            DisplayMessage(client, Display_Ultimate, "%t", "Prevented", upgradeName);
        }
        else if (HasCooldownExpired(client, raceID, pylonID))
        {
            new count = 0;
            new maxentities = GetMaxEntities();
            for (new i = MaxClients + 1; i <= maxentities; i++)
            {
                if (IsValidEntity(i) && IsValidEdict(i))
                {
                    if (TF2_GetExtObjectTypeFromClass(i) != TFExtObject_Unknown)
                    {
                        if (GetEntPropEnt(i, Prop_Send, "m_hBuilder") == client)
                        {
                            count++;
                            SetEntityRenderColor(i, 255, 255, 255, m_DarkPylonAlpha[level]);
                            SetEntityRenderMode(i,RENDER_TRANSCOLOR);

                            PrepareSound(pylonWav);
                            EmitSoundToAll(pylonWav,i);
                        }
                    }
                }
            }

            if (count > 0)
            {
                m_IsDarkPylon[client] = true;
                SetEnergy(client, energy-amount);

                new Float:time = float(level)*2.0;
                DisplayMessage(client,Display_Ultimate, "%t", "PylonInvoked", time);
                CreateTimer(time, ResetDarkPylon, GetClientUserId(client),TIMER_FLAG_NO_MAPCHANGE);
                CreateCooldown(client, raceID, pylonID);
            }
            else
            {
                PrepareSound(errorWav);
                EmitSoundToClient(client,errorWav);
                DisplayMessage(client,Display_Ultimate,
                               "%t", "PylonFoundNothing");
            }
        }
    }
}

public Action:ResetDarkPylon(Handle:timer,any:userid)
{
    new client = GetClientOfUserId(userid);
    if (client > 0)
    {
        new maxentities = GetMaxEntities();
        for (new i = MaxClients + 1; i <= maxentities; i++)
        {
            if (IsValidEntity(i) && IsValidEdict(i))
            {
                if (TF2_GetExtObjectTypeFromClass(i) != TFExtObject_Unknown)
                {
                    if (GetEntPropEnt(i, Prop_Send, "m_hBuilder") == client)
                    {
                        PrepareSound(resetWav);
                        EmitSoundToAll(resetWav,i);

                        SetEntityRenderColor(i, 255, 255, 255, 255);
                        SetEntityRenderMode(i,RENDER_NORMAL);
                    }
                }
            }
        }

        m_IsDarkPylon[client] = false;
        DisplayMessage(client,Display_Ultimate,
                       "%t", "PylonExpired");
    }
}

SummonPhasePrism(client)
{
    new energy = GetEnergy(client);
    new amount = GetUpgradeEnergy(raceID,phasePrismID);
    new accumulated = GetAccumulatedEnergy(client, raceID);
    if (accumulated + energy < amount)
    {
        ShowEnergy(client);
        EmitEnergySoundToClient(client,Protoss);

        decl String:upgradeName[64];
        GetUpgradeName(raceID, phasePrismID, upgradeName, sizeof(upgradeName), client);
        DisplayMessage(client, Display_Ultimate, "%t", "InsufficientAccumulatedEnergyFor",
                       upgradeName, amount);
    }
    else if (GetRestriction(client,Restriction_PreventUltimates) ||
             GetRestriction(client,Restriction_Stunned))
    {
        PrepareSound(deniedWav);
        EmitSoundToClient(client,deniedWav);
        DisplayMessage(client, Display_Ultimate,
                       "%t", "PreventedFromSummoningPrism");
    }
    else if (HasCooldownExpired(client, raceID, phasePrismID))
    {
        if (g_phasePrismRace < 0)
            g_phasePrismRace = FindRace("prism");

        if (g_phasePrismRace > 0)
        {
            new Float:clientLoc[3];
            GetClientAbsOrigin(client, clientLoc);
            clientLoc[2] += 40.0; // Adjust position to the middle

            TE_SetupSmoke(clientLoc, SmokeSprite(), 8.0, 2);
            TE_SendEffectToAll();

            TE_SetupGlowSprite(clientLoc,(GetClientTeam(client) == 3) ? BlueGlow() : RedGlow(),
                               5.0, 40.0, 255);
            TE_SendEffectToAll();

            energy -= amount;
            if (energy < 0)
            {
                SetAccumulatedEnergy(client, raceID, accumulated+energy);
                energy = 0;
            }
            SetEnergy(client, energy);

            ChangeRace(client, g_phasePrismRace, true, false);
        }
        else
        {
            PrepareSound(deniedWav);
            EmitSoundToClient(client,deniedWav);
            LogError("***The Phase Prism race is not Available!");

            decl String:upgradeName[64];
            GetUpgradeName(raceID, phasePrismID, upgradeName, sizeof(upgradeName), client);
            DisplayMessage(client, Display_Ultimate, "%t", "IsNotAvailable", upgradeName);
        }
    }
}

WarpInAmplifier(client, amp_level)
{
    if (m_BuildAvailable)
    {
        new energy = GetEnergy(client);
        new vespene = GetVespene(client);
        new energy_cost = GetUpgradeEnergy(raceID,amplifierID);
        new vespene_cost = GetUpgradeVespene(raceID,amplifierID);
        if (energy < energy_cost)
        {
            decl String:upgradeName[64];
            GetUpgradeName(raceID, amplifierID, upgradeName, sizeof(upgradeName), client);
            DisplayMessage(client, Display_Energy, "%t", "InsufficientEnergyFor", upgradeName, energy_cost);
            EmitEnergySoundToClient(client,Protoss);
        }
        else if (vespene < vespene_cost)
        {
            decl String:upgradeName[64];
            GetUpgradeName(raceID, amplifierID, upgradeName, sizeof(upgradeName), client);
            DisplayMessage(client, Display_Energy, "%t", "InsufficientVespeneFor", upgradeName, vespene_cost);
            EmitVespeneSoundToClient(client,Protoss);
        }
        else if (IsMole(client))
        {
            PrepareSound(deniedWav);
            EmitSoundToClient(client,deniedWav);

            decl String:upgradeName[64];
            GetUpgradeName(raceID, amplifierID, upgradeName, sizeof(upgradeName), client);
            DisplayMessage(client, Display_Ultimate, "%t", "NotAsMole", upgradeName);
        }
        else if (GetRestriction(client,Restriction_PreventUltimates) ||
                 GetRestriction(client,Restriction_Stunned))
        {
            PrepareSound(deniedWav);
            EmitSoundToClient(client,deniedWav);
            DisplayMessage(client, Display_Ultimate,
                           "%t", "PreventedFromWarpingInAmplifier");
        }
        else if (HasCooldownExpired(client, raceID, amplifierID))
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

            new Handle:menu=CreateMenu(Amplifier_Selected);
            SetMenuTitle(menu,"[SC] %T", "ChooseAmplifier", client);

            new count = CountConvertedBuildings(client, TFExtObject_Amplifier);
            new level = GetLevel(client, raceID);
            AddMenuItemT(menu,"0","SlowAmplifier",client, (count < amp_level && amp_level >= 3) ? ITEMDRAW_DEFAULT : ITEMDRAW_DISABLED);
            AddMenuItemT(menu,"1","JarAmplifier",client, (count < amp_level && amp_level >= 2) ? ITEMDRAW_DEFAULT : ITEMDRAW_DISABLED);
            AddMenuItemT(menu,"9","MilkAmplifier",client, (count < amp_level && amp_level >= 2) ? ITEMDRAW_DEFAULT : ITEMDRAW_DISABLED);
            AddMenuItemT(menu,"2","FireAmplifier",client, (count < amp_level && amp_level >= 4) ? ITEMDRAW_DEFAULT : ITEMDRAW_DISABLED);
            AddMenuItemT(menu,"11","BleedAmplifier",client, (count < amp_level && amp_level >= 4) ? ITEMDRAW_DEFAULT : ITEMDRAW_DISABLED);
            AddMenuItemT(menu,"3","TauntAmplifier",client, (count < amp_level && amp_level >= 4 && level >= 16) ? ITEMDRAW_DEFAULT : ITEMDRAW_DISABLED);
            AddMenuItemT(menu,"12","StunAmplifier",client, (count < amp_level && amp_level >= 4 && level >= 16) ? ITEMDRAW_DEFAULT : ITEMDRAW_DISABLED);
            AddMenuItemT(menu,"4","UndisguiseAmplifier",client, (count < amp_level && amp_level >= 4) ? ITEMDRAW_DEFAULT : ITEMDRAW_DISABLED);
            AddMenuItemT(menu,"5","DecloakAmplifier",client, (count < amp_level && amp_level >= 4) ? ITEMDRAW_DEFAULT : ITEMDRAW_DISABLED);
            AddMenuItemT(menu,"6","BuffAmplifier",client, (count < amp_level) ? ITEMDRAW_DEFAULT : ITEMDRAW_DISABLED);
            AddMenuItemT(menu,"10","DefenseAmplifier",client, (count < amp_level) ? ITEMDRAW_DEFAULT : ITEMDRAW_DISABLED);
            AddMenuItemT(menu,"7","KritAmplifier",client, (count < amp_level && amp_level >= 3 && level >= 10) ? ITEMDRAW_DEFAULT : ITEMDRAW_DISABLED);
            AddMenuItemT(menu,"8","UberAmplifier",client, (count < amp_level && amp_level >= 4 && level >= 16) ? ITEMDRAW_DEFAULT : ITEMDRAW_DISABLED);
            AddMenuItemT(menu,"13","DestroyStructure",client, (count > 0) ? ITEMDRAW_DEFAULT : ITEMDRAW_DISABLED);
            DisplayMenu(menu,client,MENU_TIME_FOREVER);
        }
    }
}

public Amplifier_Selected(Handle:menu,MenuAction:action,client,selection)
{
    static const String:typeName[][] = { "SlowAmplifier", "JarAmplifier", "FireAmplifier", "TauntAmplifier", "UndisguiseAmplifier",
                                         "DecloakAmplifier", "BuffAmplifier", "KritAmplifier", "UberAmplifier", "MilkAmplifier",
                                         "DefenseAmplifier", "BleedAmplifier", "StunAmplifier", "SlowAmplifier" };

    static const TFCond:condition[] = { TFCond_Slowed,
                                        TFCond_Jarated,
                                        TFCond_OnFire,
                                        TFCond_Taunting,
                                        TFCond_Disguised,
                                        TFCond_Cloaked,
                                        TFCond_Buffed,
                                        TFCond_Kritzkrieged,
                                        TFCond_Ubercharged,
                                        TFCond_Milked,
	                                    TFCond_DefenseBuffed,
                                        TFCond_Bleeding,
                                        TFCond_Dazed,
                                        TFCond_Zoomed // Really Slow
                                      };

    if (action == MenuAction_Select)
    {
        PrepareSound(buttonWav);
        EmitSoundToClient(client,buttonWav);
        
        if (GetRace(client) == raceID)
        {
            decl String:SelectionInfo[12];
            GetMenuItem(menu,selection,SelectionInfo,sizeof(SelectionInfo));

            new item = StringToInt(SelectionInfo);
            if (item == 13)
            {
                if (!DestroyBuildingMenu(client))
                {
                    PrepareSound(errorWav);
                    EmitSoundToClient(client,errorWav);
                    DisplayMessage(client, Display_Ultimate,
                                   "%t", "NoStructuresToDestroy");
                }
            }
            else
            {
                new energy = GetEnergy(client);
                new vespene = GetVespene(client);
                new energy_cost = GetUpgradeEnergy(raceID,amplifierID);
                new vespene_cost = GetUpgradeVespene(raceID,amplifierID);
                if (!IsPlayerAlive(client))
                {
                    PrepareSound(deniedWav);
                    EmitSoundToClient(client,deniedWav);

                    decl String:upgradeName[64];
                    GetUpgradeName(raceID, amplifierID, upgradeName, sizeof(upgradeName), client);
                    DisplayMessage(client, Display_Ultimate,
                                   "%t", "YouHaveDied", upgradeName);
                }
                else if (energy < energy_cost)
                {
                    decl String:upgradeName[64];
                    GetUpgradeName(raceID, amplifierID, upgradeName, sizeof(upgradeName), client);
                    DisplayMessage(client, Display_Energy, "%t", "InsufficientEnergyFor", upgradeName, energy_cost);
                    EmitEnergySoundToClient(client,Protoss);
                }
                else if (vespene < vespene_cost)
                {
                    decl String:upgradeName[64];
                    GetUpgradeName(raceID, amplifierID, upgradeName, sizeof(upgradeName), client);
                    DisplayMessage(client, Display_Energy, "%t", "InsufficientVespeneFor", upgradeName, vespene_cost);
                    EmitVespeneSoundToClient(client,Protoss);
                }
                else if (IsMole(client))
                {
                    PrepareSound(deniedWav);
                    EmitSoundToClient(client,deniedWav);

                    decl String:upgradeName[64];
                    GetUpgradeName(raceID, amplifierID, upgradeName, sizeof(upgradeName), client);
                    DisplayMessage(client, Display_Ultimate, "%t", "NotAsMole", upgradeName);
                }
                else if (GetRestriction(client,Restriction_PreventUltimates) ||
                         GetRestriction(client,Restriction_Stunned))
                {
                    PrepareSound(deniedWav);
                    EmitSoundToClient(client,deniedWav);
                    DisplayMessage(client, Display_Ultimate,
                                   "%t", "PreventedFromWarpingInAmplifier");
                }
                else
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

                    SetEnergy(client, energy-energy_cost);
                    SetVespene(client, vespene-vespene_cost);

                    new object = BuildObject(client, TFExtObject_Dispenser, 1, .check=false);
                    if (m_AmpNodeAvailable)
                    {
                        new percent, ri;
                        new level = GetUpgradeLevel(client,raceID,amplifierID);
                        switch (item)
                        {
                            case 0,13: // Slow, Really Slow
                            {
                                ri = 0;
                                percent = level * 25;
                                if (level > 2)
                                    item = 13; // for TFCond_Zoomed
                            }
                            case 3, 12: // Taunt, Stun
                            {
                                ri = 1;
                                percent = level * 5;
                            }
                            case 4,5: // Undisguise, Decloak
                            {
                                ri = 0;
                                percent = level * 5;
                            }
                            case 7,8: // Krit, Uber
                            {
                                ri = 2;
                                percent = level * 15;
                            }
                            default: // Buff, Jar, Fire, Milk, Defense, Bleed
                            {
                                ri = 3;
                                percent = level * 25;
                            }
                        }
                        ConvertToAmplifier(object, client, condition[item], g_AmpRange[ri][level], percent);
                    }

                    PrepareSound(recallDstWav);
                    EmitSoundToAll(recallDstWav,client);

                    decl String:ampName[64];
                    Format(ampName, sizeof(ampName), "%T", typeName[item], client);
                    DisplayMessage(client,Display_Ultimate, "%t", "WarpedIn", ampName);
                    CreateCooldown(client, raceID, amplifierID);
                }
            }
        }
    }
    else if (action == MenuAction_End)
        CloseHandle(menu);
}

public Action:OnAmplify(builder,client,TFCond:condition)
{
    new amount, from;
    switch (condition)
    {
        case TFCond_Slowed, TFCond_Zoomed:
        {
            if (GetImmunity(client,Immunity_MotionTaking) ||
                GetImmunity(client,Immunity_Restore))
            {
                return Plugin_Stop;
            }
            else
            {
                from  = builder;
                amount = 2;
            }                
        }
        case TFCond_Taunting, TFCond_Dazed:
        {
            if (GetImmunity(client,Immunity_MotionTaking) ||
                GetImmunity(client,Immunity_Restore))
            {
                return Plugin_Stop;
            }
            else
            {
                from  = builder;
                amount = 10;
            }                
        }
        case TFCond_Disguised, TFCond_Cloaked:
        {
            if (GetImmunity(client,Immunity_Uncloaking))
                return Plugin_Stop;
            else
            {
                from  = builder;
                amount = 10;
            }                
        }
        case TFCond_OnFire:
        {
            if (GetImmunity(client,Immunity_Burning) ||
                GetImmunity(client,Immunity_Restore))
            {
                return Plugin_Stop;
            }
            else
            {
                from  = builder;
                amount = 1;
            }
        }
        case TFCond_Bleeding:
        {
            if (GetImmunity(client,Immunity_HealthTaking) ||
                GetImmunity(client,Immunity_Restore))
            {
                return Plugin_Stop;
            }
            else
            {
                from  = builder;
                amount = 1;
            }
        }
        case TFCond_Jarated, TFCond_Milked:
        {
            if (GetImmunity(client,Immunity_Poison) ||
                GetImmunity(client,Immunity_Restore))
            {
                return Plugin_Stop;
            }
            else
            {
                from  = builder;
                amount = 1;
            }
        }
        case TFCond_Buffed, TFCond_DefenseBuffed:
        {
            from  = client;
            amount = 1;
        }
        case TFCond_Kritzkrieged, TFCond_Ubercharged:
        {
            from  = client;
            amount = 4;
        }
        default:
            amount = from = 0;
    }

    if (from > 0 && amount > 0 &&
        builder > 0 && GetRace(builder) == raceID)
    {
        new energy = GetEnergy(from);
        if (energy < amount)
            return Plugin_Stop;
        else
            SetEnergy(from, energy-amount);
    }

    return Plugin_Continue;
}

