/**
 * vim: set ai et ts=4 sw=4 :
 * File: ZergInfestor.sp
 * Description: The Zerg Infestor race for SourceCraft.
 * Author(s): -=|JFH|=-Naris
 */
 
#pragma semicolon 1

#include <sourcemod>
#include <sdktools>
#include <raytrace>
#include <range>

#undef REQUIRE_EXTENSIONS
#include <tf2_player>
#define REQUIRE_EXTENSIONS

#undef REQUIRE_PLUGIN
#include <sm_gas>

#include "sc/MindControl"
#define REQUIRE_PLUGIN

#include "sc/SourceCraft"
#include "sc/HealthParticle"
#include "sc/clienttimer"
#include "sc/maxhealth"
#include "sc/DarkSwarm"
#include "sc/weapons"
#include "sc/burrow"
#include "sc/Plague"
#include "sc/armor"

#include "effect/BeamSprite"
#include "effect/HaloSprite"
#include "effect/SendEffects"
#include "effect/FlashScreen"

new const String:g_PlagueSound[] = "sc/zdeblo01.wav";

new const String:g_ArmorName[]  = "Carapace";
new Float:g_InitialArmor[]      = { 0.25, 0.33, 0.50, 1.0, 1.25 };
new Float:g_ArmorPercent[][2]   = { {0.00, 0.10},
                                    {0.00, 0.20},
                                    {0.10, 0.30},
                                    {0.20, 0.50},
                                    {0.30, 0.60} };

new Float:g_ConsumePercent[]    = { 0.10, 0.18, 0.28, 0.38, 0.48 };
new Float:g_PlagueRange[]       = { 300.0, 400.0, 550.0, 700.0, 900.0 };
new Float:g_DarkSwarmRange[]    = { 300.0, 400.0, 600.0, 800.0, 1000.0 };
new Float:g_CorruptionRange[]   = { 250.0, 500.0, 700.0, 1000.0, 1500.0 };

new raceID, carapaceID, regenerationID, consumeID, darkSwarmID, plagueID, swarmID, corruptionID;

new m_gasAllocation[MAXPLAYERS+1];
new Float:m_ConsumeEnemyTime[MAXPLAYERS+1];

new bool:m_GasAvailable = false;
new bool:m_MindControlAvailable = false;

public Plugin:myinfo = 
{
    name = "SourceCraft Race - Zerg Infestor",
    author = "-=|JFH|=-Naris",
    description = "The Zerg race Infestor for SourceCraft.",
    version = SOURCECRAFT_VERSION,
    url = "http://jigglysfunhouse.net/"
};

public OnPluginStart()
{
    LoadTranslations("sc.common.phrases.txt");
    LoadTranslations("sc.infestor.phrases.txt");
    LoadTranslations("sc.protector.phrases.txt");

    GetGameType();
    if (IsSourceCraftLoaded())
        OnSourceCraftReady();
}

public OnSourceCraftReady()
{
    raceID          = CreateRace("infestor", -1, -1, 29, 100, -1, 1,
                                 Zerg, Biological, "defiler");

    regenerationID  = AddUpgrade(raceID, "regeneration", 0, 0);
    carapaceID      = AddUpgrade(raceID, "armor", 0, 0);
    consumeID       = AddUpgrade(raceID, "consume", 0, 0, .energy=1);

    // Ultimate 2
    AddBurrowUpgrade(raceID, 2, 0, 1, 1);

    // Ultimate 1
    darkSwarmID = AddUpgrade(raceID, "dark_swarm", 1, 0,
                             .energy=90, .cooldown=10.0);

    // Ultimate 1
    plagueID = AddUpgrade(raceID, "disease", 1, 1,
                          .energy=90, .cooldown=10.0);

    // Ultimate 3
    if (m_GasAvailable || LibraryExists("sm_gas"))
    {
        swarmID = AddUpgrade(raceID, "swarm", 3, 0,
                             .cooldown=2.0);

        if (!m_GasAvailable)
        {
            ControlGas(true);
            m_GasAvailable = true;
        }
    }
    else
    {
        AddUpgrade(raceID, "swarm", 3, 99, 0,
                   .name="Swarm Infestation",
                   .desc="Not Available");
    }

    // Ultimate 4
    if (GetGameType() == tf2)
    {
        if (m_MindControlAvailable || LibraryExists("MindControl"))
        {
            m_MindControlAvailable = true;
            corruptionID = AddUpgrade(raceID, "corruption", 4, 4,
                                      .energy=10);
        }
        else
        {
            corruptionID = AddUpgrade(raceID, "corruption", 4, 4, .energy=10,
                                      .desc="%infestor_corruption_nocontrol_desc");
        }
    }
    else
    {
        corruptionID = AddUpgrade(raceID, "corruption", 4, 99, 0,
                                  .desc="%NotAvailable");
    }

    // Get Configuration Data
    GetConfigFloatArray("armor_amount", g_InitialArmor, sizeof(g_InitialArmor),
                        g_InitialArmor, raceID, carapaceID);

    for (new level=0; level < sizeof(g_ArmorPercent); level++)
    {
        decl String:key[32];
        Format(key, sizeof(key), "armor_percent_level_%d", level);
        GetConfigFloatArray(key, g_ArmorPercent[level], sizeof(g_ArmorPercent[]),
                            g_ArmorPercent[level], raceID, carapaceID);
    }

    GetConfigFloatArray("damage_percent", g_ConsumePercent, sizeof(g_ConsumePercent),
                        g_ConsumePercent, raceID, consumeID);

    GetConfigFloatArray("range", g_DarkSwarmRange, sizeof(g_DarkSwarmRange),
                        g_DarkSwarmRange, raceID, darkSwarmID);

    GetConfigFloatArray("range", g_PlagueRange, sizeof(g_PlagueRange),
                        g_PlagueRange, raceID, plagueID);

    GetConfigFloatArray("range", g_CorruptionRange, sizeof(g_CorruptionRange),
                        g_CorruptionRange, raceID, corruptionID);
}

public OnLibraryAdded(const String:name[])
{
    if (StrEqual(name, "sm_gas"))
    {
        if (!m_GasAvailable)
        {
            ControlGas(true);
            m_GasAvailable = true;
        }
    }
    else if (StrEqual(name, "MindControl"))
        m_MindControlAvailable = (GetGameType() == tf2);
}

public OnLibraryRemoved(const String:name[])
{
    if (StrEqual(name, "sm_gas"))
        m_GasAvailable = false;
    else if (StrEqual(name, "MindControl"))
        m_MindControlAvailable = false;
}

public OnMapStart()
{
    SetupBeamSprite(false, false);
    SetupHaloSprite(false, false);

    SetupPlague(Zerg, g_PlagueSound, false, false);
    SetupDarkSwarm(Zerg, false, false);
}

public OnMapEnd()
{
    for (new index=1;index<=MaxClients;index++)
    {
        ResetPlague(index);
        ResetDarkSwarm(index);
        ResetClientTimer(index);
    }
}

public OnPlayerAuthed(client)
{
    m_gasAllocation[client] = 0;
    m_ConsumeEnemyTime[client] = 0.0;
}

public OnClientDisconnect(client)
{
    ResetPlague(client);
    ResetDarkSwarm(client);
    ResetProtected(client);
    KillClientTimer(client);
}

public Action:OnRaceDeselected(client,oldrace,newrace)
{
    if (oldrace == raceID)
    {
        if (m_GasAvailable)
            TakeGas(client);

        ResetArmor(client);
        ResetDarkSwarm(client);
        KillClientTimer(client);
    }
    return Plugin_Continue;
}

public Action:OnRaceSelected(client,oldrace,newrace)
{
    if (newrace == raceID)
    {
        m_gasAllocation[client] = 0;

        new swarm_level=GetUpgradeLevel(client,raceID,swarmID);
        SetupSwarmInfestation(client, swarm_level);

        new carapace_level = GetUpgradeLevel(client,raceID,carapaceID);
        SetupArmor(client, carapace_level, g_InitialArmor,
                   g_ArmorPercent, g_ArmorName);

        if (IsPlayerAlive(client))
        {
            CreateClientTimer(client, 1.0, Regeneration,
                              TIMER_REPEAT|TIMER_FLAG_NO_MAPCHANGE);
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
        if (upgrade==carapaceID)
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
        else if (upgrade==swarmID)
            SetupSwarmInfestation(client, new_level);
    }
}

public OnUltimateCommand(client,race,bool:pressed,arg)
{
    if (race==raceID && IsPlayerAlive(client) && pressed)
    {
        switch (arg)
        {
            case 4:
            {
                new corruption_level=GetUpgradeLevel(client,race,corruptionID);
                if (GetGameType() == tf2 && corruption_level > 0)
                    Corruption(client, corruption_level);
            }
            case 3:
            {
                SwarmInfestation(client);
            }
            case 2:
            {
                new burrow_level=GetUpgradeLevel(client,race,burrowID);
                Burrow(client, burrow_level+1);
            }
            default:
            {
                new plague_level=GetUpgradeLevel(client,race,plagueID);
                if (plague_level > 0)
                {
                    Plague(client, race, plagueID, plague_level,
                           UltimatePlague|EnsnaringPlague|ExplosivePlague|FatalPlague|InfectiousPlague,
                           false, g_PlagueRange, g_PlagueSound, "sc_disease");
                }
                else
                {
                    new dark_swarm_level=GetUpgradeLevel(client,race,darkSwarmID);
                    DarkSwarm(client, race, darkSwarmID, dark_swarm_level, g_DarkSwarmRange);
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
        m_gasAllocation[client] = 0;
        m_ConsumeEnemyTime[client] = 0.0;

        new swarm_level=GetUpgradeLevel(client,raceID,swarmID);
        SetupSwarmInfestation(client, swarm_level);

        new carapace_level = GetUpgradeLevel(client,raceID,carapaceID);
        SetupArmor(client, carapace_level, g_InitialArmor,
                   g_ArmorPercent, g_ArmorName);

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
    if (!from_sc && attacker_index != victim_index)
    {
        damage += absorbed;

        if (attacker_index > 0 && attacker_race == raceID)
            changed |= ConsumeEnemy(damage, attacker_index, victim_index);

        if (assister_index > 0 && assister_race == raceID)
            changed |= ConsumeEnemy(damage, assister_index, victim_index);
    }

    return changed ? Plugin_Changed : Plugin_Continue;
}

public OnPlayerDeathEvent(Handle:event, victim_index, victim_race, attacker_index,
                          attacker_race, assister_index, assister_race, damage,
                          const String:weapon[], bool:is_equipment, customkill,
                          bool:headshot, bool:backstab, bool:melee)
{
    KillClientTimer(victim_index);
    ResetProtected(victim_index);
    m_gasAllocation[victim_index] = 0;
}

bool:ConsumeEnemy(damage, index, victim_index)
{
    new level = GetUpgradeLevel(index,raceID,consumeID);
    if (level > 0 && GetRandomInt(1,10) <= 9 &&
        !GetRestriction(index, Restriction_PreventUpgrades) &&
        !GetRestriction(index, Restriction_Stunned) &&
        !GetImmunity(victim_index,Immunity_HealthTaking) &&
        !GetImmunity(victim_index,Immunity_Upgrades) &&
        !IsInvulnerable(victim_index))
    {
        new energy = GetEnergy(index);
        new amount = GetUpgradeEnergy(raceID,consumeID);
        new Float:lastTime = m_ConsumeEnemyTime[index];
        new Float:interval = GetGameTime() - lastTime;
        if (energy >= amount && (lastTime == 0.0 || interval > 0.25))
        {
            new Float:start[3];
            GetClientAbsOrigin(index, start);
            start[2] += 1620;

            new Float:end[3];
            GetClientAbsOrigin(index, end);
            end[2] += 20;

            static const color[4] = { 255, 10, 25, 255 };
            TE_SetupBeamPoints(start, end, BeamSprite(), HaloSprite(),
                               0, 1, 3.0, 20.0,10.0,5,50.0,color,255);
            TE_SendEffectToAll();
            FlashScreen(index,RGBA_COLOR_GREEN);
            FlashScreen(victim_index,RGBA_COLOR_RED);

            SetEnergy(index, energy-amount);
            m_ConsumeEnemyTime[index] = GetGameTime();

            new leechhealth = RoundFloat(float(damage)*g_ConsumePercent[level]);
            if (leechhealth <= 0)
                leechhealth = 1;

            if (IsClientInGame(index) && IsPlayerAlive(index))
            {
                new health = GetClientHealth(index) + leechhealth;
                if (health <= GetMaxHealth(index))
                {
                    ShowHealthParticle(index);
                    SetEntityHealth(index,health);

                    decl String:upgradeName[64];
                    GetUpgradeName(raceID, consumeID, upgradeName, sizeof(upgradeName), index);
                    DisplayMessage(index, Display_Damage_Done, "%t", "YouHaveLeeched",
                                   leechhealth, victim_index, upgradeName);
                }
            }

            new victim_health = GetClientHealth(victim_index);
            if (victim_health <= leechhealth)
                KillPlayer(victim_index, index, "sc_consume");
            else
            {
                CreateParticle("blood_impact_red_01_chunk", 0.1, victim_index, Attach, "head");
                SetEntityHealth(victim_index, victim_health - leechhealth);

                decl String:upgradeName[64];
                GetUpgradeName(raceID, consumeID, upgradeName, sizeof(upgradeName), victim_index);
                DisplayMessage(victim_index, Display_Damage_Taken, "%t", "HasLeeched",
                               index, leechhealth, upgradeName);
            }
            return true;
        }
    }
    return false;
}

public Action:Regeneration(Handle:timer, any:userid)
{
    new client = GetClientOfUserId(userid);
    if (IsValidClient(client) && IsPlayerAlive(client) && GetRace(client) == raceID &&
        !GetRestriction(client,Restriction_PreventUpgrades) &&
        !GetRestriction(client,Restriction_Stunned))
    {
        new regeneration_level=GetUpgradeLevel(client,raceID,regenerationID);
        HealPlayer(client,regeneration_level+1);
    }
    return Plugin_Continue;
}

SetupSwarmInfestation(client, level)
{
    if (m_GasAvailable)
    {
        new amount = ((level+1)*2) - m_gasAllocation[client];
        if (amount > 0)
        {
            m_gasAllocation[client] += amount;
            GiveGas(client, amount, .everyone=1);
        }
    }
}

SwarmInfestation(client)
{
    if (m_GasAvailable)
    {
        new energy = GetEnergy(client);
        new amount = GetUpgradeEnergy(raceID,swarmID);
        if (energy < amount)
        {
            decl String:upgradeName[64];
            GetUpgradeName(raceID, swarmID, upgradeName, sizeof(upgradeName), client);
            DisplayMessage(client, Display_Energy, "%t", "InsufficientEnergyFor", upgradeName, amount);
            EmitEnergySoundToClient(client,Zerg);
        }
        else if (GetRestriction(client,Restriction_PreventUltimates) ||
                 GetRestriction(client,Restriction_Stunned))
        {
            PrepareSound(deniedWav);
            EmitSoundToClient(client,deniedWav);

            decl String:upgradeName[64];
            GetUpgradeName(raceID, swarmID, upgradeName, sizeof(upgradeName), client);
            DisplayMessage(client, Display_Ultimate, "%t", "Prevented", upgradeName);
        }
        else
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

            if (GasAttack(client))
            {
                SetEnergy(client, energy-amount);
                new remaining = HasGas(client);
                if (remaining == 0)
                {
                    DisplayMessage(client,Display_Ultimate, "%t",
                                   "UsedLastSwarmInfestation");
                }
                else
                {
                    new Float:cooldown = GetUpgradeCooldown(raceID, swarmID);
                    if (cooldown > 0.0)
                    {
                        DisplayMessage(client, Display_Ultimate, "%t", "UsedSwarmInfestationMustWait", remaining, cooldown);
                        CreateTimer(cooldown,AllowSwarmInfestation,GetClientUserId(client),TIMER_FLAG_NO_MAPCHANGE);
                        EnableGas(client, false);
                    }
                    else
                    {
                        DisplayMessage(client, Display_Ultimate, "%t", "UsedSwarmInfestation", remaining);
                    }
                }
            }
            else if (HasGas(client) == 0)
            {
                DisplayMessage(client ,Display_Ultimate, "%t",
                               "SwarmInfestationDepleted");
            }
            else // if (!IsGasEnabled(client))
            {
                PrepareSound(deniedWav);
                EmitSoundToClient(client,deniedWav);

                decl String:upgradeName[64];
                GetUpgradeName(raceID, swarmID, upgradeName, sizeof(upgradeName), client);
                DisplayMessage(client, Display_Ultimate, "%t", "IsNotReady", upgradeName);
            }
        }
    }
    else
    {
        PrepareSound(deniedWav);
        EmitSoundToClient(client,deniedWav);

        decl String:upgradeName[64];
        GetUpgradeName(raceID, swarmID, upgradeName, sizeof(upgradeName), client);
        PrintHintText(client, "%t", "IsNotAvailable", upgradeName);
    }
}

public Action:AllowSwarmInfestation(Handle:timer,any:userid)
{
    new index = GetClientOfUserId(userid);
    if (index > 0 && m_GasAvailable)
    {
        EnableGas(index, true);
        if (IsClientInGame(index) && IsPlayerAlive(index))
        {
            if (GetRace(index) == raceID)
            {
                PrepareSound(rechargeWav);
                EmitSoundToClient(index, rechargeWav);

                decl String:upgradeName[64];
                GetUpgradeName(raceID, swarmID, upgradeName, sizeof(upgradeName), index);
                DisplayMessage(index, Display_Ultimate, "%t", "IsReady", upgradeName);
                PrintHintText(index, "%t", "IsReady", upgradeName);
            }
        }                
    }                
    return Plugin_Stop;
}

Corruption(client, level)
{
    new energy = GetEnergy(client);
    new amount = GetUpgradeEnergy(raceID,corruptionID);
    if (energy < amount)
    {
        decl String:upgradeName[64];
        GetUpgradeName(raceID, corruptionID, upgradeName, sizeof(upgradeName), client);
        DisplayMessage(client, Display_Energy, "%t", "InsufficientEnergyFor", upgradeName, amount);
        EmitEnergySoundToClient(client,Zerg);
        return;
    }
    else if (GetRestriction(client,Restriction_PreventUltimates) ||
             GetRestriction(client,Restriction_Stunned))
    {
        PrepareSound(deniedWav);
        EmitSoundToClient(client,deniedWav);

        decl String:upgradeName[64];
        GetUpgradeName(raceID, corruptionID, upgradeName, sizeof(upgradeName), client);
        DisplayMessage(client, Display_Ultimate, "%t", "Prevented", upgradeName);
        return;
    }

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

    new Float:range = g_CorruptionRange[level];
    new target = TraceAimTarget(client);
    if (target >= 0)
    {
        new Float:clientLoc[3];
        GetClientAbsOrigin(client, clientLoc);

        new Float:targetLoc[3];
        TR_GetEndPosition(targetLoc);

        if (IsPointInRange(clientLoc,targetLoc,range))
        {
            if (IsValidEdict(target) && IsValidEntity(target) &&
                TF2_GetExtObjectTypeFromClass(target) != TFExtObject_Unknown)
            {
                new builder = GetEntPropEnt(target, Prop_Send, "m_hBuilder");
                if (builder > 0 && !GetImmunity(builder,Immunity_Ultimates))
                {
                    new Handle:pack;
                    if (CreateDataTimer(0.5, CorruptionTimer, pack, TIMER_REPEAT|TIMER_FLAG_NO_MAPCHANGE) &&
                        pack != INVALID_HANDLE)
                    {
                        WritePackCell(pack, EntIndexToEntRef(target));
                        WritePackCell(pack, client);
                        WritePackCell(pack, level);
                        WritePackFloat(pack, GetEngineTime() + (float(level) * 5.0));

                        SetEnergy(client, energy-amount);

                        new targetTeam = GetEntProp(target, Prop_Send, "m_iTeamNum");
                        if (targetTeam != GetClientTeam(client))
                        {
                            new r,b,g;
                            if (TFTeam:targetTeam == TFTeam_Red)
                            {
                                r = 255;
                                b = 100;
                                g = 60;
                            }
                            else
                            {
                                r = 0;
                                b = 255;
                                g = 100;
                            }
                            SetEntityRenderColor(target, r, b, g, 255);
                        }
                    }
                }
                else
                {
                    PrepareSound(deniedWav);
                    EmitSoundToClient(client,deniedWav);
                    DisplayMessage(client, Display_Ultimate,
                                   "%t", "TargetIsImmune");
                }
            }
            else
            {
                PrepareSound(deniedWav);
                EmitSoundToClient(client,deniedWav);
                DisplayMessage(client, Display_Ultimate,
                               "%t", "TargetInvalid");
            }
        }
        else
        {
            PrepareSound(errorWav);
            EmitSoundToClient(client,errorWav);
            DisplayMessage(client, Display_Ultimate,
                           "%t", "TargetIsTooFar");
        }
    }
    else
    {
        PrepareSound(deniedWav);
        EmitSoundToClient(client,deniedWav);
    }
}

public Action:CorruptionTimer(Handle:timer,Handle:pack)
{
    if (pack != INVALID_HANDLE)
    {
        ResetPack(pack);
        new ref=ReadPackCell(pack);
        new client=ReadPackCell(pack);
        new level=ReadPackCell(pack);
        new Float:endTime=ReadPackFloat(pack);

        new target = EntRefToEntIndex(ref);
        if (target > 0 && IsValidEntity(target) && IsValidEdict(target))
        {
            new health = GetEntProp(target, Prop_Send, "m_iHealth");
            new team = GetEntProp(target, Prop_Send, "m_iTeamNum");
            if (team == GetClientTeam(client))
            {
                new max_health = GetEntProp(target, Prop_Send, "m_iMaxHealth");
                if (health < max_health)
                {
                    health += 10 * level;
                    if (health > max_health)
                        health = max_health;

                    SetEntProp(target, Prop_Send, "m_iHealth", health);
                    if (GetEngineTime() < endTime)
                        return Plugin_Continue;
                }
            }
            else
            {
                new amount = 10 * level;
                health -= amount;
                if (health > 50 + amount)
                {
                    SetEntProp(target, Prop_Send, "m_iHealth", health);
                    if (GetEngineTime() < endTime)
                        return Plugin_Continue;
                    else
                        SetEntityRenderColor(target, 255, 255, 255, 255);
                }
                else
                {
                    SetEntityRenderColor(target, 255, 255, 255, 255);
                    if (m_MindControlAvailable)
                        ControlObject(client, target);
                }
            }
        }
    }
    return Plugin_Stop;
}
