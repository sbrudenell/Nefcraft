 /**
 * vim: set ai et ts=4 sw=4 :
 * File: DarkTemplar.sp
 * Description: The Protoss Dark Templar race for SourceCraft.
 * Author(s): Naris (Murray Wilson)
 */

#pragma semicolon 1

#include <sourcemod>
#include <sdktools>
#include <raytrace>
#include <range>

#undef REQUIRE_EXTENSIONS
#include <tf2>
#include <tf2_meter>
#include <tf2_player>
#include <sidewinder>
#define REQUIRE_EXTENSIONS

#undef REQUIRE_PLUGIN
#include "FakeDeath"
#define REQUIRE_PLUGIN

#include "sc/SourceCraft"
#include "sc/MeleeAttack"
#include "sc/clienttimer"
#include "sc/SpeedBoost"
#include "sc/maxhealth"
#include "sc/Teleport"
#include "sc/dissolve"
#include "sc/shields"
#include "sc/freeze"
#include "sc/cloak"

#include "effect/Smoke"
#include "effect/RedGlow"
#include "effect/BlueGlow"
#include "effect/Lightning"
#include "effect/HaloSprite"
#include "effect/SendEffects"

new const String:spawnWav[]         = "sc/pdtrdy00.wav";
new const String:deathWav[]         = "sc/pdtdth00.wav";
new const String:teleportWav[]      = "sc/ptemov00.wav";
new const String:g_PsiBladesSound[] = "sc/uzefir00.wav";

new raceID, immunityID, legID, shieldsID, cloakID, regenID;
new meleeID, teleportID, darkArchonID, deathID;

new Float:g_InitialShields[]        = { 0.0, 0.25, 0.50, 0.75, 1.00 };
new Float:g_ShieldsPercent[][2]     = { {0.00, 0.00},
                                        {0.00, 0.05},
                                        {0.02, 0.10},
                                        {0.05, 0.15},
                                        {0.08, 0.20} };

new Float:g_SpeedLevels[]           = { -1.0, 1.05, 1.10, 1.15, 1.20 };
new Float:g_PsiBladesPercent[]      = { 0.0, 0.15, 0.35, 0.55, 0.75 };

new Float:g_TeleportDistance[]      = { 0.0, 300.0, 500.0, 800.0, 1500.0 };

new g_darkArchonRace = -1;

new Float:m_CloakRegenTime[MAXPLAYERS+1];

new bool:cfgAllowTeleport;

new bool:m_FakeDeathAvailable = false;

public Plugin:myinfo = 
{
    name = "SourceCraft Race - Protoss Dark Templar",
    author = "-=|JFH|=-Naris",
    description = "The Protoss Dark Templar race for SourceCraft.",
    version = SOURCECRAFT_VERSION,
    url = "http://jigglysfunhouse.net/"
};

public OnPluginStart()
{
    LoadTranslations("sc.common.phrases.txt");
    LoadTranslations("sc.teleport.phrases.txt");
    LoadTranslations("sc.dark_templar.phrases.txt");

    if (GetGameType() == tf2)
    {
        if (!HookEventEx("teamplay_round_win",EventRoundOver,EventHookMode_PostNoCopy))
            SetFailState("Could not hook the teamplay_round_win event.");

        if (!HookEventEx("teamplay_round_stalemate",EventRoundOver,EventHookMode_PostNoCopy))
            SetFailState("Could not hook the teamplay_round_stalemate event.");
    }
    else if (GameType == dod)
    {
        if (!HookEventEx("dod_round_win",EventRoundOver,EventHookMode_PostNoCopy))
            SetFailState("Could not hook the dod_round_win event.");
    }
    else if (GameType == cstrike)
    {
        if (!HookEventEx("round_end",EventRoundOver,EventHookMode_PostNoCopy))
            SetFailState("Could not hook the round_end event.");
    }

    if (IsSourceCraftLoaded())
        OnSourceCraftReady();
}

public OnSourceCraftReady()
{
    if (GameType == tf2)
    {
        decl String:error[64];
        m_SidewinderAvailable = (GetExtensionFileStatus("sidewinder.ext", error, sizeof(error)) == 1);
    }

    raceID      = CreateRace("dark_templar", 64, 0, 30, .energy_rate=2, .faction=Protoss);

    immunityID  = AddUpgrade(raceID, "immunity");
    legID       = AddUpgrade(raceID, "leg");
    shieldsID   = AddUpgrade(raceID, "shields", .energy=1);

    cfgAllowInvisibility = bool:GetConfigNum("allow_invisibility", true);
    if (cfgAllowInvisibility)
    {
        cloakID     = AddUpgrade(raceID, "cloak");
    }
    else
    {
        cloakID     = AddUpgrade(raceID, "cloak", 0, 0, 1,
                                 .desc="%dark_templar_cloak_noinvis_desc");

        LogMessage("Reducing Protoss Dark Templar:Cloaking due to configuration: sc_allow_invisibility=%d",
                   cfgAllowInvisibility);
    }

    if (GameType == tf2 && (cfgAllowInvisibility))
    {
        regenID     = AddUpgrade(raceID, "cloak_regen");
    }
    else
    {
        regenID     = AddUpgrade(raceID, "cloak_regen", 0, 99,
                                 .desc="%NotAvailable");

        LogMessage("Disabling Protoss Dark Templar: Cloak Regeneration due to configuration: sc_allow_invisibility=%d",
                   cfgAllowInvisibility);
    }

    meleeID    = AddUpgrade(raceID, "blades", .energy=5);

    // Ultimate 1
    cfgAllowTeleport = bool:GetConfigNum("allow_teleport", true);
    if (cfgAllowTeleport)
    {
        teleportID = AddUpgrade(raceID, "blink", 1,
                                .energy=30, .cooldown=2.0);

        GetConfigFloatArray("range",  g_TeleportDistance, sizeof(g_TeleportDistance),
                            g_TeleportDistance, raceID, teleportID);
    }
    else
    {
        teleportID = AddUpgrade(raceID, "blink", 1, 99, 0,
                                .desc="%NotAllowed");

        LogMessage("Disabling Protoss Dark Templar:Blink due to configuration: sc_allow_teleport=%d",
                   cfgAllowTeleport);
    }

    // Ultimate 3
    darkArchonID = AddUpgrade(raceID, "dark_archon", 3, 10,1,
                              .energy=300, .cooldown=30.0);

    // Ultimate 2
    if ((GameType == tf2) && (m_FakeDeathAvailable || LibraryExists("FakeDeath")))
    {
        deathID = AddUpgrade(raceID, "death_shadow", 2, 12, .energy=10);
        if (!m_FakeDeathAvailable)
        {
            ControlDeath(true);
            m_FakeDeathAvailable = true;
        }
    }
    else
    {
        LogMessage("FakeDeath is not available");
        deathID = AddUpgrade(raceID, "death_shadow", 2, 99,
                             .desc="%NotAvailable");
    }

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

    GetConfigFloatArray("damage_percent",  g_PsiBladesPercent, sizeof(g_PsiBladesPercent),
                        g_PsiBladesPercent, raceID, meleeID);

    GetConfigFloatArray("speed",  g_SpeedLevels, sizeof(g_SpeedLevels),
                        g_SpeedLevels, raceID, legID);
}

public OnLibraryAdded(const String:name[])
{
    if (StrEqual(name, "FakeDeath"))
    {
        if (GetGameType() == tf2 && !m_FakeDeathAvailable)
        {
            ControlDeath(true);                         
            m_FakeDeathAvailable = true;
        }
    }
    else if (strncmp(name, "sidewinder", 10, false) == 0)
        m_SidewinderAvailable = (GetGameType() == tf2);
}

public OnLibraryRemoved(const String:name[])
{
    if (StrEqual(name, "FakeDeath"))
        m_FakeDeathAvailable = false;
    else if (strncmp(name, "sidewinder", 10, false) == 0)
        m_SidewinderAvailable = false;
}

public OnMapStart()
{
    if (GameType == tf2)
    {
        decl String:error[64];
        m_SidewinderAvailable = (GetExtensionFileStatus("sidewinder.ext", error, sizeof(error)) == 1);
    }

    SetupSmokeSprite(false, false);
    SetupHaloSprite(false, false);
    SetupLightning(false, false);
    SetupBlueGlow(false, false);
    SetupRedGlow(false, false);

    SetupSpeed(false, false);
    SetupCloak(false, false);
    SetupTeleport(Protoss, false, false);

    SetupSound(deathWav, DOWNLOAD, false, false);
    SetupSound(spawnWav, DOWNLOAD, false, false);
    SetupSound(teleportWav, DOWNLOAD, false, false);
    SetupSound(g_PsiBladesSound, DOWNLOAD, false, false);
}

public OnMapEnd()
{
    ResetAllClientTimers();
    ResetAllShieldTimers();
}

public OnPlayerAuthed(client)
{
    m_CloakRegenTime[client] = 0.0;
    ResetTeleport(client);
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
        ResetTeleport(client);
        SetSpeed(client,-1.0);
        SetOverrideVisiblity(client, -1);
        SetVisibility(client, NormalVisibility);
        ApplyPlayerSettings(client);

        if (m_FakeDeathAvailable)
            TakeDeath(client);

        // Turn off Immunities
        new immunity_level=GetUpgradeLevel(client,raceID,immunityID);
        DoImmunity(client, immunity_level, false);

        return Plugin_Handled;
    }
    else
    {
        if (g_darkArchonRace < 0)
            g_darkArchonRace = FindRace("dark_archon");

        if (oldrace == g_darkArchonRace &&
            GetCooldownExpireTime(client, raceID, darkArchonID) <= 0.0)
        {
            CreateCooldown(client, raceID, darkArchonID,
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
        m_CloakRegenTime[client] = 0.0;

        new immunity_level=GetUpgradeLevel(client,raceID,immunityID);
        DoImmunity(client, immunity_level, true);

        new cloak_level=GetUpgradeLevel(client,raceID,cloakID);
        AlphaCloak(client, cloak_level, false);

        new leg_level = GetUpgradeLevel(client,raceID,legID);
        SetSpeedBoost(client, leg_level, false, g_SpeedLevels);

        new shields_level = GetUpgradeLevel(client,raceID,shieldsID);
        SetupShields(client, shields_level, g_InitialShields, g_ShieldsPercent);

        if (IsPlayerAlive(client))
        {
            PrepareSound(spawnWav);
            EmitSoundToAll(spawnWav, client);

            ApplyPlayerSettings(client);

            if (m_FakeDeathAvailable)
            {
                new death_level = GetUpgradeLevel(client,raceID,deathID);
                GiveDeath(client, death_level, true);
            }

            new regen_level = GetUpgradeLevel(client,raceID,regenID);
            if (shields_level > 0 || (regen_level > 0 && GameType == tf2))
            {
                CreateClientTimer(client, 1.0, Regeneration, TIMER_REPEAT|TIMER_FLAG_NO_MAPCHANGE);
                if (shields_level > 0)
                    CreateShieldTimer(client);
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
        else if (upgrade==legID)
            SetSpeedBoost(client, new_level, true, g_SpeedLevels);
        else if (upgrade == cloakID)
                AlphaCloak(client, new_level, true);
        else if (upgrade==deathID)
        {
            if (m_FakeDeathAvailable)
                GiveDeath(client, new_level, true);
        }
        else if (upgrade==shieldsID)
        {
            SetupShields(client, new_level, g_InitialShields,
                         g_ShieldsPercent, .upgrade=true);

            new regen_level = GetUpgradeLevel(client,raceID,regenID);
            if (new_level > 0 || (regen_level > 0 && GameType == tf2
                                  && cfgAllowInvisibility))
            {
                if (IsValidClient(client) && IsPlayerAlive(client))
                {
                    CreateClientTimer(client, 1.0, Regeneration, TIMER_REPEAT|TIMER_FLAG_NO_MAPCHANGE);
                    if (new_level > 0)
                        CreateShieldTimer(client);
                    else
                        KillShieldTimer(client);
                }
            }
            else
            {
                KillClientTimer(client);
                KillShieldTimer(client);
            }
        }
        else if (upgrade==regenID)
        {
            new shields_level = GetUpgradeLevel(client,raceID,shieldsID);
            if (shields_level > 0 || (new_level > 0 && GameType == tf2
                                      && cfgAllowInvisibility))
            {
                if (IsValidClient(client) && IsPlayerAlive(client))
                    CreateClientTimer(client, 1.0, Regeneration, TIMER_REPEAT|TIMER_FLAG_NO_MAPCHANGE);
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

        if (g_cloakItem < 0)
            g_cloakItem = FindShopItem("cloak");

        if (item == g_bootsItem)
        {
            new leg_level = GetUpgradeLevel(client,race,legID);
            if (leg_level > 0)
                SetSpeedBoost(client, leg_level, true, g_SpeedLevels);
        }
        else if (item == g_cloakItem)
        {
            new cloak_level=GetUpgradeLevel(client,race,cloakID);
            AlphaCloak(client, cloak_level, true);
        }
    }
}

public OnUltimateCommand(client,race,bool:pressed,arg)
{
    if (race == raceID && IsPlayerAlive(client))
    {
        switch (arg)
        {
            case 4,3:
            {
                if (pressed)
                {
                    new darkArchon_level = GetUpgradeLevel(client,race,darkArchonID);
                    if (darkArchon_level > 0)
                        SummonDarkArchon(client);
                }
            }
            case 2:
            {
                if (pressed)
                {
                    if (m_FakeDeathAvailable)
                    {
                        decl String:upgradeName[64];
                        GetUpgradeName(raceID, deathID, upgradeName, sizeof(upgradeName), client);

                        new energy = GetEnergy(client);
                        new amount = GetUpgradeEnergy(raceID,deathID);
                        if (energy < amount)
                        {
                            EmitEnergySoundToClient(client,Protoss);
                            DisplayMessage(client, Display_Energy, "%t",
                                           "InsufficientEnergyFor", upgradeName, amount);
                        }
                        else if (GetRestriction(client,Restriction_PreventUltimates) ||
                                 GetRestriction(client,Restriction_Stunned))
                        {
                            PrepareSound(deniedWav);
                            EmitSoundToClient(client,deniedWav);
                            DisplayMessage(client, Display_Ultimate, "%t",
                                           "Prevented", upgradeName);
                        }
                        else
                        {
                            SetEnergy(client, energy-amount);
                            FakeDeath(client);
                        }
                    }
                    else
                        PrintHintText(client,"%t", "NoDeathShadow");
                }
            }
            default:
            {
                new blink_level = GetUpgradeLevel(client,race,teleportID);
                if (blink_level && cfgAllowTeleport)
                {
                    SetVisibility(client, NormalVisibility);
                    SetOverrideVisiblity(client, 255, true);
                    if (m_SidewinderAvailable)
                        SidewinderCloakClient(client, false);

                    CreateTimer(5.0,RecloakPlayer,GetClientUserId(client),TIMER_FLAG_NO_MAPCHANGE);
                    new blink_energy=GetUpgradeEnergy(raceID,teleportID) * (5-blink_level);
                    TeleportCommand(client, race, teleportID, blink_level, blink_energy,
                                    pressed, g_TeleportDistance, teleportWav);
                }
                else
                {
                    if (pressed)
                    {
                        new darkArchon_level = GetUpgradeLevel(client,race,darkArchonID);
                        if (darkArchon_level > 0)
                            SummonDarkArchon(client);
                    }
                }
            }
        }
    }
}

public Action:RecloakPlayer(Handle:timer,any:userid)
{
    new client = GetClientOfUserId(userid);
    if (client)
    {
        SetOverrideVisiblity(client, -1);

        new cloak_level=GetUpgradeLevel(client,raceID,cloakID);
        AlphaCloak(client, cloak_level, true);
    }
    return Plugin_Stop;
}

public OnPlayerSpawnEvent(Handle:event, client, race)
{
    if (race == raceID)
    {
        m_CloakRegenTime[client] = 0.0;

        PrepareSound(spawnWav);
        EmitSoundToAll(spawnWav, client);

        GetClientAbsOrigin(client,spawnLoc[client]);
        ResetTeleport(client);

        new immunity_level=GetUpgradeLevel(client,raceID,immunityID);
        DoImmunity(client, immunity_level, true);

        new cloak_level=GetUpgradeLevel(client,raceID,cloakID);
        AlphaCloak(client, cloak_level, false);

        new leg_level = GetUpgradeLevel(client,raceID,legID);
        SetSpeedBoost(client, leg_level, false, g_SpeedLevels);

        ApplyPlayerSettings(client);

        new shields_level = GetUpgradeLevel(client,raceID,shieldsID);
        SetupShields(client, shields_level, g_InitialShields, g_ShieldsPercent);

        if (shields_level > 0)
            CreateShieldTimer(client);

        new regen_level = GetUpgradeLevel(client,raceID,regenID);
        if (shields_level > 0 || (regen_level > 0 && GameType == tf2))
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
    if (attacker_index && attacker_index != victim_index &&
        !from_sc)
    {
        if (attacker_race == raceID)
        {
            new blades_level=GetUpgradeLevel(attacker_index,raceID,meleeID);
            if (blades_level > 0)
            {
                changed |= MeleeAttack(raceID, meleeID, blades_level, event, damage+absorbed,
                                       victim_index, attacker_index, g_PsiBladesPercent,
                                       g_PsiBladesSound, "sc_blades");
            }

            new blink_level = GetUpgradeLevel(attacker_index,raceID,teleportID);
            if (blink_level && cfgAllowTeleport)
                TeleporterAttacked(attacker_index,raceID,teleportID);
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

        SetOverrideVisiblity(victim_index, -1);
        SetVisibility(victim_index, NormalVisibility);
        
        PrepareSound(deathWav);
        EmitSoundToAll(deathWav,victim_index);
        DissolveRagdoll(victim_index, 0.1);
    }
    else
    {
        if (g_darkArchonRace < 0)
            g_darkArchonRace = FindRace("dark_archon");

        if (victim_race == g_darkArchonRace &&
            GetCooldownExpireTime(victim_index, raceID, darkArchonID) <= 0.0)
        {
            CreateCooldown(victim_index, raceID, darkArchonID,
                           .type=Cooldown_CreateNotify
                                |Cooldown_AlwaysNotify);
        }
    }
}

public EventRoundOver(Handle:event,const String:name[],bool:dontBroadcast)
{
    for(new index=1;index<=MaxClients;index++)
    {
        if (IsClientInGame(index))
        {
            if (GetRace(index) == raceID)
            {
                SetOverrideVisiblity(index, -1);
                SetVisibility(index, NormalVisibility);
            }
        }
    }
}

DoImmunity(client, level, bool:value)
{
    if (value && level >= 1)
    {
        SetImmunity(client,Immunity_Uncloaking, true);

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

    SetImmunity(client,Immunity_HealthTaking, (value && level >= 2));
    SetImmunity(client,Immunity_Ultimates, (value && level >= 3));
    SetImmunity(client,Immunity_ShopItems, (value && level >= 4));

    if (value && IsClientInGame(client) && IsPlayerAlive(client))
    {
        new Float:start[3];
        GetClientAbsOrigin(client, start);

        static const color[4] = { 0, 255, 50, 128 };
        TE_SetupBeamRingPoint(start,30.0,60.0,Lightning(),HaloSprite(),
                              0, 1, 2.0, 10.0, 0.0 ,color, 10, 0);
        TE_SendEffectToAll();
    }
}

SummonDarkArchon(client)
{
    new energy = GetEnergy(client);
    new amount = GetUpgradeEnergy(raceID,darkArchonID);
    new accumulated = GetAccumulatedEnergy(client, raceID);
    if (accumulated + energy < amount)
    {
        ShowEnergy(client);
        EmitEnergySoundToClient(client,Protoss);

        decl String:upgradeName[64];
        GetUpgradeName(raceID, darkArchonID, upgradeName, sizeof(upgradeName), client);
        DisplayMessage(client, Display_Ultimate, "%t", "InsufficientAccumulatedEnergyFor",
                       upgradeName, amount);
    }
    else if (GetRestriction(client,Restriction_PreventUltimates) ||
             GetRestriction(client,Restriction_Stunned))
    {
        PrepareSound(deniedWav);
        EmitSoundToClient(client,deniedWav);
        DisplayMessage(client, Display_Ultimate, "%t",
                       "PreventedFromSummoningDarkArchon");
    }
    else if (HasCooldownExpired(client, raceID, darkArchonID))
    {
        if (g_darkArchonRace < 0)
            g_darkArchonRace = FindRace("dark_archon");

        if (g_darkArchonRace > 0)
        {
            new Float:clientLoc[3];
            GetClientAbsOrigin(client, clientLoc);
            clientLoc[2] += 40.0; // Adjust position to the middle

            TE_SetupSmoke(clientLoc,SmokeSprite(),8.0,2);
            TE_SendEffectToAll();

            TE_SetupGlowSprite(clientLoc,
                               (GetClientTeam(client) == 3) ? BlueGlow() : RedGlow(),
                               5.0,40.0,255);
            TE_SendEffectToAll();

            accumulated -= amount;
            if (accumulated < 0)
            {
                SetEnergy(client, energy+accumulated);
                accumulated = 0;
            }
            SetAccumulatedEnergy(client, raceID, accumulated);

            ChangeRace(client, g_darkArchonRace, true, false);
        }
        else
        {
            PrepareSound(deniedWav);
            EmitSoundToClient(client,deniedWav);
            LogError("***The Dark Archon race is not Available!");

            decl String:upgradeName[64];
            GetUpgradeName(raceID, darkArchonID, upgradeName, sizeof(upgradeName), client);
            DisplayMessage(client, Display_Ultimate, "%t", "IsNotAvailable", upgradeName);
        }
    }
}

public Action:Regeneration(Handle:timer, any:userid)
{
    new client = GetClientOfUserId(userid);
    if (IsValidClient(client) && IsPlayerAlive(client) && GetRace(client) == raceID &&
        !GetRestriction(client,Restriction_PreventUpgrades) &&
        !GetRestriction(client,Restriction_Stunned))
    {
        new shields_level = GetUpgradeLevel(client,raceID,shieldsID);
        if (shields_level > 0)
            RegenerateShields(client, shields_level, g_InitialShields);

        new regen_level = GetUpgradeLevel(client,raceID,regenID);
        if (regen_level > 0 && GameType == tf2 && cfgAllowInvisibility &&
            TF2_GetPlayerClass(client) == TFClass_Spy)
        {
            // Check for the Dead Ringer in slot 4
            new weapon = GetPlayerWeaponSlot(client, 4);
            if (weapon > 0 && IsValidEdict(weapon) && IsValidEntity(weapon))
            {
                if (GetEntProp(weapon, Prop_Send, "m_iItemDefinitionIndex") == 59)
                {
                    //Player has a Dead Ringer!
                    return Plugin_Continue;
                }
            }

            new Float:cloak = TF2_GetCloakMeter(client);
            if (cloak < 100.0)
            {
                new energy = GetEnergy(client);
                new amt = regen_level * 2;
                if (amt > energy)
                    amt = energy;

                if (amt > 0)
                {
                    new Float:lastTime = m_CloakRegenTime[client];
                    new Float:interval = GetGameTime() - lastTime;
                    if (lastTime == 0.0 || interval >= 2.00)
                    {
                        SetEnergy(client, --energy);
                        m_CloakRegenTime[client] = GetGameTime();
                    }

                    TF2_SetCloakMeter(client,cloak + float(amt));
                    DisplayMessage(client,Display_Misc_Message,
                                   "%t", "ReceivedCloak", amt);
                }
            }
        }
    }
    return Plugin_Continue;
}

