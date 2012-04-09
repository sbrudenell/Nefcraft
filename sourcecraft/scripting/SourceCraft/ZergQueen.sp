/*
 * vim: set ai et ts=4 sw=4 :
 * File: ZergQueen.sp
 * Description: The Zerg Queen race for SourceCraft.
 * Author(s): -=|JFH|=-Naris
 */
 
#pragma semicolon 1

#include <sourcemod>
#include <sdktools>
#include <raytrace>
#include <range>

#undef REQUIRE_EXTENSIONS
#include <tf2_meter>
#include <tf2_player>
#include <sidewinder>
#define REQUIRE_EXTENSIONS

#undef REQUIRE_PLUGIN
#include <jetpack>
#define REQUIRE_PLUGIN

#include "sc/SourceCraft"
#include "sc/clienttimer"
#include "sc/SpeedBoost"
#include "sc/maxhealth"
#include "sc/Detector"
#include "sc/weapons"
#include "sc/armor"

#include "effect/BeamSprite"
#include "effect/HaloSprite"

new const String:deniedWav[]    = "sc/buzz.wav";
new const String:rechargeWav[]  = "sc/transmission.wav";
new const String:queenFireWav[] = "sc/zqufir00.wav";
new const String:ensnareHitWav[] = "sc/zquens00.wav";
new const String:infestedHitWav[] = "sc/zquhit02.wav";
new const String:parasiteHitWav[] = "sc/zqutag01.wav";
new const String:parasiteFireWav[] = "sc/zqutag00.wav";
new const String:broodlingHitWav[] = "sc/zqutag01.wav";

new const String:g_ArmorName[]  = "Carapace";
new Float:g_InitialArmor[]      = { 0.0, 0.10, 0.25, 0.35, 0.50 };
new Float:g_ArmorPercent[][2]   = { {0.00, 0.00},
                                    {0.00, 0.05},
                                    {0.00, 0.10},
                                    {0.05, 0.20},
                                    {0.10, 0.40} };

new g_JetpackFuel[]             = { 0,     40,   50,   70, 90 };
new Float:g_JetpackRefuelTime[] = { 0.0, 45.0, 35.0, 25.0, 15.0 };

new Float:g_BroodlingRange[]    = { 350.0, 400.0, 650.0, 750.0, 900.0 };
new Float:g_InfestRange[]       = { 350.0, 400.0, 650.0, 750.0, 900.0 };
new Float:g_SpeedLevels[]       = { -1.0, 1.10, 1.15, 1.20, 1.25 };

new Float:g_EnsnareSpeed[]      = { 0.95, 0.90, 0.80, 0.70, 0.60 };
new g_EnsnareChance[]           = {    5,   15,   25,   35, 45 };

new raceID, armorID, regenerationID, pneumatizedID, parasiteID, ensnareID;
new jetpackID, meiosisID, broodlingID, infestID;

new g_broodlingRace = -1;
new g_infestedRace = -1;

new bool:m_JetpackAvailable = false;

new bool:m_Ensnared[MAXPLAYERS+1];
new Handle:m_ParasiteTimer[MAXPLAYERS+1];
new m_ParasiteCount[MAXPLAYERS+1];

public Plugin:myinfo = 
{
    name = "SourceCraft Race - Zerg Queen",
    author = "-=|JFH|=-Naris",
    description = "The Zerg Queen race for SourceCraft.",
    version = SOURCECRAFT_VERSION,
    url = "http://jigglysfunhouse.net/"
};

public OnPluginStart()
{
    GetGameType();
    if (IsSourceCraftLoaded())
        OnSourceCraftReady();
}

public OnSourceCraftReady()
{
    LoadTranslations("sc.detector.phrases.txt");
    LoadTranslations("sc.common.phrases.txt");
    LoadTranslations("sc.queen.phrases.txt");

    if (GetGameType() == tf2)
    {
        decl String:error[64];
        m_SidewinderAvailable = (GetExtensionFileStatus("sidewinder.ext", error, sizeof(error)) == 1);
    }

    raceID          = CreateRace("queen", 100, 0, 36, 120, 1000, 1,
                                 Zerg, Biological);

    armorID         = AddUpgrade(raceID, "armor");
    pneumatizedID   = AddUpgrade(raceID, "pneumatized");
    regenerationID  = AddUpgrade(raceID, "regeneration");
    parasiteID      = AddUpgrade(raceID, "parasite", .energy=1);
    ensnareID       = AddUpgrade(raceID, "ensnare", .energy=3);

    // Ultimate 1
    if (m_JetpackAvailable || LibraryExists("jetpack"))
    {
        jetpackID   = AddUpgrade(raceID, "flyer", 1);

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

    meiosisID       = AddUpgrade(raceID, "meiosis");

    // Ultimate 2
    broodlingID     = AddUpgrade(raceID, "broodling", 2, 10,
                                 .energy=90, .cooldown=5.0);

    // Ultimate 3
    infestID        = AddUpgrade(raceID, "infest", 3, 12,
                                 .energy=180, .cooldown=5.0);

    // Get Configuration Data
    GetConfigFloatArray("armor_amount", g_InitialArmor, sizeof(g_InitialArmor),
                        g_InitialArmor, raceID, armorID);

    for (new level=0; level < sizeof(g_ArmorPercent); level++)
    {
        decl String:key[32];
        Format(key, sizeof(key), "armor_percent_level_%d", level);
        GetConfigFloatArray(key, g_ArmorPercent[level], sizeof(g_ArmorPercent[]),
                            g_ArmorPercent[level], raceID, armorID);
    }

    GetConfigFloatArray("speed", g_SpeedLevels, sizeof(g_SpeedLevels),
                        g_SpeedLevels, raceID, pneumatizedID);

    GetConfigArray("chance", g_EnsnareChance, sizeof(g_EnsnareChance),
                   g_EnsnareChance, raceID, ensnareID);

    GetConfigFloatArray("speed", g_EnsnareSpeed, sizeof(g_EnsnareSpeed),
                        g_EnsnareSpeed, raceID, ensnareID);

    GetConfigFloatArray("range",  g_BroodlingRange, sizeof(g_BroodlingRange),
                        g_BroodlingRange, raceID, broodlingID);

    GetConfigFloatArray("range",  g_InfestRange, sizeof(g_InfestRange),
                        g_InfestRange, raceID, infestID);
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
    else if (strncmp(name, "sidewinder", 10, false) == 0)
        m_SidewinderAvailable = (GetGameType() == tf2);
}

public OnLibraryRemoved(const String:name[])
{
    if (StrEqual(name, "jetpack"))
        m_JetpackAvailable = false;
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

    SetupBeamSprite(false, false);
    SetupHaloSprite(false, false);

    SetupSpeed(false, false);

    SetupSound(deniedWav, true, false, false);
    SetupSound(rechargeWav, true, false, false);
    SetupSound(queenFireWav, true, false, false);
    SetupSound(ensnareHitWav, true, false, false);
    SetupSound(infestedHitWav, true, false, false);
    SetupSound(parasiteHitWav, true, false, false);
    SetupSound(parasiteFireWav, true, false, false);
    SetupSound(broodlingHitWav, true, false, false);
}

public OnMapEnd()
{
    for (new index=1;index<=MaxClients;index++)
    {
        m_ParasiteTimer[index] = INVALID_HANDLE;	
        ResetParasite(index);
        ResetDetected(index);
        ResetEnsnared(index);
        ResetClientTimer(index);
    }
}

public OnClientDisconnect(client)
{
    ResetParasite(client);
    ResetDetected(client);
    ResetEnsnared(client);
    KillClientTimer(client);
}

public Action:OnRaceDeselected(client,oldrace,newrace)
{
    if (oldrace == raceID)
    {
        ResetArmor(client);
        SetSpeed(client, -1.0, true);
        SetInitialEnergy(client, -1);
        KillClientTimer(client);

        if (m_JetpackAvailable)
            TakeJetpack(client);

        return Plugin_Handled;
    }
    else
        return Plugin_Continue;
}

public Action:OnRaceSelected(client,oldrace,newrace)
{
    if (newrace == raceID)
    {
        new armor_level = GetUpgradeLevel(client,raceID,armorID);
        SetupArmor(client, armor_level, g_InitialArmor,
                   g_ArmorPercent, g_ArmorName);

        new pneumatized_level = GetUpgradeLevel(client,raceID,pneumatizedID);
        SetSpeedBoost(client, pneumatized_level, true, g_SpeedLevels);

        new flyer_level=GetUpgradeLevel(client,raceID,jetpackID);
        SetupJetpack(client, flyer_level);

        new meiosis_level = GetUpgradeLevel(client,raceID,meiosisID);
        SetInitialEnergy(client, 120 + (meiosis_level*30));

        if (IsPlayerAlive(client))
        {
            new regeneration_level=GetUpgradeLevel(client,raceID,regenerationID);
            if (regeneration_level > 0)
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
        if (upgrade==jetpackID)
            SetupJetpack(client, new_level);
        else if (upgrade==meiosisID)
            SetInitialEnergy(client, 120 + (new_level*30));
        else if (upgrade==pneumatizedID)
            SetSpeedBoost(client, new_level, true, g_SpeedLevels);
        else if (upgrade==armorID)
        {
            SetupArmor(client, new_level, g_InitialArmor,
                       g_ArmorPercent, g_ArmorName,
                       .upgrade=true);
        }
        else if (upgrade==regenerationID)
        {
            if (new_level > 0)
            {
                if (IsClientInGame(client) && IsPlayerAlive(client))
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

public OnItemPurchase(client,item)
{
    new race=GetRace(client);
    if (race == raceID && IsPlayerAlive(client))
    {
        if (g_bootsItem < 0)
            g_bootsItem = FindShopItem("boots");

        if (item == g_bootsItem)
        {
            new level = GetUpgradeLevel(client,race,pneumatizedID);
            if (level > 0)
                SetSpeedBoost(client, level, true, g_SpeedLevels);
        }
    }
}

public OnUltimateCommand(client,race,bool:pressed,arg)
{
    if (race==raceID && IsPlayerAlive(client))
    {
        switch (arg)
        {
            case 4,3:
            {
                if (pressed)
                    InfestEnemy(client);
            }
            case 2:
            {
                if (pressed)
                    SpawnBroodling(client);
            }
            default:
            {
                new flyer_level=GetUpgradeLevel(client,raceID,jetpackID);
                if (flyer_level > 0)
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
}

// Events
public OnPlayerSpawnEvent(Handle:event, client, race)
{
    if (race == raceID)
    {
        new armor_level = GetUpgradeLevel(client,raceID,armorID);
        SetupArmor(client, armor_level, g_InitialArmor,
                   g_ArmorPercent, g_ArmorName);

        new pneumatized_level = GetUpgradeLevel(client,raceID,pneumatizedID);
        SetSpeedBoost(client, pneumatized_level, true, g_SpeedLevels);

        new flyer_level=GetUpgradeLevel(client,raceID,jetpackID);
        SetupJetpack(client, flyer_level);

        new meiosis_level = GetUpgradeLevel(client,raceID,meiosisID);
        new initial = 120 + (meiosis_level*30);
        SetInitialEnergy(client, initial);
        if (GetEnergy(client) < initial)
            SetEnergy(client, initial);

        new regeneration_level=GetUpgradeLevel(client,raceID,regenerationID);
        if (regeneration_level > 0)
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
    if (!from_sc)
    {
        if (attacker_index && attacker_index != victim_index &&
            attacker_race == raceID)
        {
            new ensnare_level=GetUpgradeLevel(attacker_index,raceID,ensnareID);
            if (GetRandomInt(1,100)<=g_EnsnareChance[ensnare_level])
            {
                if (!m_Ensnared[victim_index] &&
                    !GetRestriction(attacker_index, Restriction_PreventUpgrades) &&
                    !GetRestriction(attacker_index, Restriction_Stunned) &&
                    !GetImmunity(victim_index,Immunity_MotionTaking) &&
                    !GetImmunity(victim_index,Immunity_Upgrades) &&
                    !GetImmunity(victim_index,Immunity_Restore) &&
                    !IsInvulnerable(victim_index))
                {
                    new energy = GetEnergy(attacker_index);
                    new amount = GetUpgradeEnergy(raceID,ensnareID);
                    if (energy >= amount)
                    {
                        PrepareSound(ensnareHitWav);
                        EmitSoundToAll(ensnareHitWav,victim_index);
                        EmitSoundToClient(attacker_index, ensnareHitWav);

                        m_Ensnared[victim_index] = true;
                        SetEnergy(attacker_index, energy-amount);

                        SetOverrideGravity(victim_index, 1.0);
                        SetOverrideSpeed(victim_index, g_EnsnareSpeed[ensnare_level]);
                        SetRestriction(victim_index, Restriction_Grounded, true);

                        new victim_energy=GetEnergy(victim_index)-ensnare_level*5;
                        SetEnergy(victim_index, (victim_energy > 0) ? victim_energy : 0);
                        CreateTimer(5.0,EnsnareExpire, GetClientUserId(victim_index),TIMER_FLAG_NO_MAPCHANGE);
                    }
                }
            }
            else
            {
                new parasite_level=GetUpgradeLevel(attacker_index,raceID,parasiteID);
                if (GetRandomInt(1,100)<=g_EnsnareChance[parasite_level])
                {
                    if (m_ParasiteTimer[victim_index] == INVALID_HANDLE &&
                        !GetRestriction(attacker_index, Restriction_PreventUpgrades) &&
                        !GetRestriction(attacker_index, Restriction_Stunned) &&
                        !GetImmunity(victim_index,Immunity_Upgrades) &&
                        !GetImmunity(victim_index,Immunity_Restore) &&
                        !IsInvulnerable(victim_index))
                    {
                        new energy = GetEnergy(attacker_index);
                        new amount = GetUpgradeEnergy(raceID,parasiteID);
                        if (energy >= amount)
                        {
                            PrepareSound(parasiteHitWav);
                            PrepareSound(parasiteFireWav);
                            EmitSoundToAll(parasiteFireWav,attacker_index);
                            EmitSoundToAll(parasiteHitWav,victim_index);
                            
                            SetOverrideVisiblity(victim_index, 255);
                            SetEnergy(attacker_index, energy-amount);
                            HudMessage(victim_index, "%t", "ParasiteHud");
                            DisplayMessage(victim_index,Display_Enemy_Ultimate, "%t",
                                           "InfestedWithParasite", attacker_index);

                            m_ParasiteCount[victim_index] = parasite_level*15;
                            m_ParasiteTimer[victim_index] = CreateTimer(1.0,Parasite,victim_index,
                                                                        TIMER_REPEAT|TIMER_FLAG_NO_MAPCHANGE);
                        }
                    }
                }
            }
        }

        if (assister_index && assister_race == raceID)
        {
            new ensnare_level=GetUpgradeLevel(assister_index,raceID,ensnareID);
            if (GetRandomInt(1,100)<=g_EnsnareChance[ensnare_level])
            {
                if (!m_Ensnared[victim_index] &&
                    !GetRestriction(assister_index, Restriction_PreventUpgrades) &&
                    !GetRestriction(assister_index, Restriction_Stunned) &&
                    !GetImmunity(victim_index,Immunity_MotionTaking) &&
                    !GetImmunity(victim_index,Immunity_Upgrades) &&
                    !GetImmunity(victim_index,Immunity_Restore) &&
                    !IsInvulnerable(victim_index))
                {
                    new energy = GetEnergy(assister_index);
                    new amount = GetUpgradeEnergy(raceID,ensnareID);
                    if (energy >= amount)
                    {
                        PrepareSound(ensnareHitWav);
                        EmitSoundToAll(ensnareHitWav,victim_index);
                        EmitSoundToClient(assister_index, ensnareHitWav);
                        
                        SetEnergy(assister_index, energy-amount);

                        m_Ensnared[victim_index] = true;
                        SetOverrideGravity(victim_index, 1.0);
                        SetOverrideSpeed(victim_index, g_EnsnareSpeed[ensnare_level]);
                        SetRestriction(victim_index, Restriction_Grounded, true);

                        new victim_energy=GetEnergy(victim_index)-ensnare_level*5;
                        SetEnergy(victim_index, (victim_energy > 0) ? victim_energy : 0);
                        CreateTimer(5.0,EnsnareExpire, GetClientUserId(victim_index),TIMER_FLAG_NO_MAPCHANGE);
                    }
                }
            }
            else
            {
                new parasite_level=GetUpgradeLevel(assister_index,raceID,parasiteID);
                if (GetRandomInt(1,100)<=g_EnsnareChance[parasite_level])
                {
                    if (m_ParasiteTimer[victim_index] == INVALID_HANDLE &&
                        !GetRestriction(assister_index, Restriction_PreventUpgrades) &&
                        !GetRestriction(assister_index, Restriction_Stunned) &&
                        !GetImmunity(victim_index,Immunity_Upgrades) &&
                        !GetImmunity(victim_index,Immunity_Restore) &&
                        !IsInvulnerable(victim_index))
                    {
                        new energy = GetEnergy(assister_index);
                        new amount = GetUpgradeEnergy(raceID,parasiteID);
                        if (energy >= amount)
                        {
                            PrepareSound(parasiteHitWav);
                            PrepareSound(parasiteFireWav);
                            EmitSoundToAll(parasiteFireWav,assister_index);
                            EmitSoundToAll(parasiteHitWav,victim_index);
                            
                            SetOverrideVisiblity(victim_index, 255);
                            SetEnergy(assister_index, energy-amount);
                            HudMessage(victim_index, "%t", "ParasiteHud");
                            DisplayMessage(victim_index,Display_Enemy_Ultimate, "%t",
                                           "InfestedWithParasite", assister_index);

                            m_ParasiteCount[victim_index] = parasite_level*15;
                            m_ParasiteTimer[victim_index] = CreateTimer(1.0,Parasite,victim_index,
                                                                        TIMER_REPEAT|TIMER_FLAG_NO_MAPCHANGE);
                        }
                    }
                }
            }
        }
    }

    return changed ? Plugin_Changed : Plugin_Continue;
}

public Action:OnJetpack(client)
{
    if (m_Ensnared[client])
    {
        PrepareSound(deniedWav);
        EmitSoundToClient(client,deniedWav);
        return Plugin_Stop;
    }
    return Plugin_Continue;
}

public Action:EnsnareExpire(Handle:timer,any:userid)
{
    new client = GetClientOfUserId(userid);
    if (client > 0)
        ResetEnsnared(client);

    return Plugin_Stop;
}

public OnPlayerDeathEvent(Handle:event, victim_index, victim_race, attacker_index,
                          attacker_race, assister_index, assister_race, damage,
                          const String:weapon[], bool:is_equipment, customkill,
                          bool:headshot, bool:backstab, bool:melee)
{
    if (victim_index > 0)
    {
        ResetParasite(victim_index);
        ResetDetected(victim_index);
        ResetEnsnared(victim_index);
        KillClientTimer(victim_index);
    }
}

public Action:OnPlayerRestored(client)
{
    ResetParasite(client);
    ResetDetected(client);
    ResetEnsnared(client);
    return Plugin_Continue;
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
            new regeneration_level=GetUpgradeLevel(client,raceID,regenerationID);
            if (regeneration_level > 0)
                HealPlayer(client,regeneration_level);
        }
    }
    return Plugin_Continue;
}

public Action:Parasite(Handle:timer, any:client)
{
    if (IsClientInGame(client) && IsPlayerAlive(client))
    {
        new Float:clientLoc[3];
        GetClientAbsOrigin(client, clientLoc);
        clientLoc[2] += 50.0; // Adjust trace position to the middle of the person instead of the feet.

        new bool:decloaked=false;
        if (GetGameType() == tf2 &&
            !GetImmunity(client,Immunity_Uncloaking) &&
            !GetImmunity(client,Immunity_Upgrades) &&
            TF2_GetPlayerClass(client) == TFClass_Spy)
        {
            new Float:meter = TF2_GetCloakMeter(client);
            if (meter > 0.0 && meter <= 100.0)
                TF2_SetCloakMeter(client, 0.0);

            meter = TF2_GetRageMeter(client);
            if (meter > 0.0 && meter <= 100.0)
                TF2_SetRageMeter(client, 0.0);

            meter = TF2_GetEnergyDrinkMeter(client);
            if (meter > 0.0 && meter <= 100.0)
                TF2_SetEnergyDrinkMeter(client, 0.0);

            new pcond = TF2_GetPlayerConditionFlags(client);
            if (TF2_IsDisguised(pcond))
            {
                TF2_RemovePlayerDisguise(client);
                decloaked=true;
            }

            if (TF2_IsCloaked(pcond) || TF2_IsDeadRingered(pcond))
            {
                TF2_RemoveCondition(client,TFCond_Cloaked);
                decloaked=true;
            }

            if (decloaked)
            {
                DisplayMessage(client,Display_Enemy_Ultimate,
                               "%t", "ParasiteUncloakedYou");
            }
        }

        if (!GetImmunity(client,Immunity_Detection) &&
            !GetImmunity(client,Immunity_Upgrades))
        {
            SetOverrideVisiblity(client, 255);
            if (m_SidewinderAvailable)
                SidewinderDetectClient(client, true);
        }

        HudMessage(client, "%t", "ParasiteHud");

        new count = 0;
        new alt_count=0;
        new list[MaxClients+1];
        new alt_list[MaxClients+1];
        new energy = GetEnergy(client);
        new amount = GetRandomInt(0,3)-2;
        new team = GetClientTeam(client);
        for (new index=1;index<=MaxClients;index++)
        {
            if (index != client && IsClientInGame(index))
            {
                if (GetClientTeam(index) == team)
                {
                    new bool:detect = !GetImmunity(index,Immunity_Detection) &&
                                      !GetImmunity(index,Immunity_Upgrades) &&
                                      IsPlayerAlive(index) &&
                                      IsInRange(client,index,500.0);
                    if (detect)
                    {
                        new Float:indexLoc[3];
                        GetClientAbsOrigin(index, indexLoc);
                        detect = TraceTargetIndex(client, index, clientLoc, indexLoc);
                    }
                    if (detect)
                    {
                        amount++;
                        SetOverrideVisiblity(index, 255);
                        if (m_SidewinderAvailable)
                            SidewinderDetectClient(index, true);

                        if (!m_Detected[client][index])
                        {
                            m_Detected[client][index] = true;
                            ApplyPlayerSettings(index);
                        }

                        HudMessage(index, "%t", "DetectedHud");
                        DisplayMessage(index,Display_Enemy_Ultimate, "%t",
                                       "DetectedByParasite", client);
                    }
                    else // undetect
                    {
                        SetOverrideVisiblity(index, -1);
                        if (m_SidewinderAvailable)
                            SidewinderDetectClient(index, false);

                        if (m_Detected[client][index])
                        {
                            m_Detected[client][index] = false;
                            ClearHud(index, "%t", "DetectedHud");
                            ApplyPlayerSettings(index);
                        }
                    }
                }
                else
                {
                    if (!GetSetting(index, Disable_OBeacons) &&
                        !GetSetting(index, Remove_Queasiness))
                    {
                        if (GetSetting(index, Reduce_Queasiness))
                            alt_list[alt_count++] = index;
                        else
                            list[count++] = index;
                    }
                }
            }
        }

        if (amount > 0)
        {
            energy -= amount;
            SetEnergy(client, (energy > 0) ? energy : 0);
        }            

        if (GetRandomInt(1,100)<50)
        {
            PrepareSound(parasiteHitWav);
            EmitSoundToAll(parasiteHitWav,client);
        }

        static const parasiteColor[4] = {255, 182, 193, 255};
        clientLoc[2] -= 50.0; // Adjust position back to the feet.

        if (count > 0)
        {
            TE_SetupBeamRingPoint(clientLoc, 10.0, 500.0, BeamSprite(), HaloSprite(),
                                  0, 10, 0.6, 10.0, 0.5, parasiteColor, 10, 0);
            TE_Send(list, count, 0.0);
        }

        if (alt_count > 0)
        {
            TE_SetupBeamRingPoint(clientLoc, 490.0, 500.0, BeamSprite(), HaloSprite(),
                                  0, 10, 0.6, 10.0, 0.5, parasiteColor, 10, 0);
            TE_Send(alt_list, alt_count, 0.0);
        }

        if (--m_ParasiteCount[client] > 0)
            return Plugin_Continue;
    }

    m_ParasiteTimer[client] = INVALID_HANDLE;
    ResetParasite(client);

    return Plugin_Stop;
}

ResetParasite(client)
{
    new Handle:timer = m_ParasiteTimer[client];
    if (timer != INVALID_HANDLE)
    {
        m_ParasiteTimer[client] = INVALID_HANDLE;	
        KillTimer(timer);
    }

    ClearHud(client, "%t", "ParasiteHud");
    ResetDetection(client);
    ResetDetected(client);
}

ResetEnsnared(client)
{
    if (m_Ensnared[client])
    {
        m_Ensnared[client] = false;
        SetOverrideSpeed(client, -1.0);
        SetOverrideGravity(client, -1.0);
        SetRestriction(client, Restriction_Grounded, false);
    }
}

SetupJetpack(client, level)
{
    if (m_JetpackAvailable)
    {
        if (level > 0)
        {
            if (level >= sizeof(g_JetpackFuel))
            {
                LogError("%d:%N has too many levels in ZergQueen::Flyer level=%d, max=%d",
                         client,ValidClientIndex(client),level,sizeof(g_JetpackFuel));

                level = sizeof(g_JetpackFuel)-1;
            }
            GiveJetpack(client, g_JetpackFuel[level], g_JetpackRefuelTime[level]);
        }
        else
            TakeJetpack(client);
    }
}

GetTarget(client, level, const Float:range[])
{
    new target = GetClientAimTarget(client);
    if (target > 0 &&
        GetClientTeam(target) != GetClientTeam(client)) 
    {
        new Float:clientLoc[3];
        GetClientAbsOrigin(client, clientLoc);

        new Float:targetLoc[3];
        GetClientAbsOrigin(target, targetLoc);

        if (IsPointInRange(clientLoc,targetLoc,range[level]) &&
            TraceTargetClients(client, target, clientLoc, targetLoc))
        {
            if (!GetImmunity(target,Immunity_Ultimates) &&
                !IsInvulnerable(target))
            {
                return target;
            }
        }
    }
    return 0;
}

SpawnBroodling(client)
{
    new broodling_level = GetUpgradeLevel(client,raceID,broodlingID);
    if (broodling_level > 0)
    {
        new energy = GetEnergy(client);
        new amount = GetUpgradeEnergy(raceID,broodlingID);
        new accumulated = GetAccumulatedEnergy(client, raceID);
        if (accumulated + energy < amount)
        {
            ShowEnergy(client);
            EmitEnergySoundToClient(client,Zerg);

            decl String:upgradeName[64];
            GetUpgradeName(raceID, broodlingID, upgradeName, sizeof(upgradeName), client);
            DisplayMessage(client, Display_Ultimate, "%t", "InsufficientAccumulatedEnergyFor",
                           upgradeName, amount);
        }
        else if (GetRestriction(client,Restriction_PreventUltimates) ||
                 GetRestriction(client,Restriction_Stunned))
        {
            PrepareSound(deniedWav);
            EmitSoundToClient(client,deniedWav);
            DisplayMessage(client, Display_Ultimate,
                           "%t", "PreventedFromBroodling");
        }
        else if (HasCooldownExpired(client, raceID, broodlingID))
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

            new target = GetTarget(client, broodling_level, g_BroodlingRange);
            if (target > 0)
            {
                if ((GetAttribute(target,Attribute_IsBiological) ||
                     GetAttribute(target,Attribute_IsMechanical)) &&
                    !GetImmunity(target,Immunity_Ultimates) &&
                    !IsInvulnerable(target))
                {
                    if (g_broodlingRace < 0)
                        g_broodlingRace = FindRace("broodling");

                    energy -= amount;
                    if (energy < 0)
                    {
                        SetAccumulatedEnergy(client, raceID, accumulated+energy);
                        energy = 0;
                    }
                    SetEnergy(client, energy);

                    PrepareSound(queenFireWav);
                    PrepareSound(broodlingHitWav);
                    EmitSoundToAll(queenFireWav,client);
                    EmitSoundToAll(broodlingHitWav,target);

                    ChangeRace(target, g_broodlingRace, true, false);

                    DisplayMessage(client,Display_Ultimate, "%t",
                                   "BroodlingSpawned", target);

                    CreateCooldown(client, raceID, broodlingID);
                }
                else
                {
                    PrepareSound(deniedWav);
                    EmitSoundToClient(client,deniedWav);
                    DisplayMessage(client, Display_Ultimate,
                                   "%t", "TargetIsInvulerable");
                }
            }
            else
            {
                PrepareSound(deniedWav);
                EmitSoundToClient(client,deniedWav);
                DisplayMessage(client, Display_Ultimate,
                               " No Targets in Range!");
            }
        }
    }
}

InfestEnemy(client)
{
    new infest_level = GetUpgradeLevel(client,raceID,infestID);
    if (infest_level > 0)
    {
        new energy = GetEnergy(client);
        new amount = GetUpgradeEnergy(raceID,infestID);
        new accumulated = GetAccumulatedEnergy(client, raceID);
        if (accumulated + energy < amount)
        {
            ShowEnergy(client);
            EmitEnergySoundToClient(client,Zerg);

            decl String:upgradeName[64];
            GetUpgradeName(raceID, infestID, upgradeName, sizeof(upgradeName), client);
            DisplayMessage(client, Display_Ultimate, "%t", "InsufficientAccumulatedEnergyFor",
                           upgradeName, amount);
        }
        else if (GetRestriction(client,Restriction_PreventUltimates) ||
                 GetRestriction(client,Restriction_Stunned))
        {
            PrepareSound(deniedWav);
            EmitSoundToClient(client,deniedWav);
            DisplayMessage(client, Display_Ultimate,
                           "%t", "PreventedFromInfesting");
        }
        else if (HasCooldownExpired(client, raceID, infestID))
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

            new target = GetTarget(client, infest_level, g_InfestRange);
            if (target > 0)
            {
                if (!GetImmunity(target,Immunity_Ultimates) &&
                    !IsInvulnerable(target))
                {
                    if (g_infestedRace < 0)
                        g_infestedRace = FindRace("infested");

                    energy -= amount;
                    if (energy < 0)
                    {
                        SetAccumulatedEnergy(client, raceID, accumulated+energy);
                        energy = 0;
                    }
                    SetEnergy(client, energy);

                    PrepareSound(queenFireWav);
                    PrepareSound(infestedHitWav);
                    EmitSoundToAll(queenFireWav,client);
                    EmitSoundToAll(infestedHitWav,target);

                    ChangeRace(target, g_infestedRace, true, false);

                    DisplayMessage(client,Display_Ultimate, "%t",
                                   "YouHaveInfested", target);

                    CreateCooldown(client, raceID, infestID);
                }
                else
                {
                    PrepareSound(deniedWav);
                    EmitSoundToClient(client,deniedWav);
                    DisplayMessage(client, Display_Ultimate,
                                   " Target is Immune or Invulerable!");
                }
            }
            else
            {
                PrepareSound(deniedWav);
                EmitSoundToClient(client,deniedWav);
                DisplayMessage(client, Display_Ultimate,
                               " No Targets in Range!");
            }
        }
    }
}
