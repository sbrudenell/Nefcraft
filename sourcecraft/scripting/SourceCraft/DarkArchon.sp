/**
 * vim: set ai et ts=4 sw=4 :
 * File: DarkArchon.sp
 * Description: The Protoss Dark Archon race for SourceCraft.
 * Author(s): Naris (Murray Wilson)
 */

#pragma semicolon 1

#include <sourcemod>
#include <sdktools>
#include <raytrace>

#undef REQUIRE_EXTENSIONS
#include <tf2>
#include <tf2_meter>
#include <tf2_player>
#include <sidewinder>
#define REQUIRE_EXTENSIONS

#undef REQUIRE_PLUGIN
#include "sc/MindControl"
#define REQUIRE_PLUGIN

#include "sc/SourceCraft"
#include "sc/PsionicRage"
#include "sc/UltimateFeedback"
#include "sc/MeleeAttack"
#include "sc/clienttimer"
#include "sc/maxhealth"
#include "sc/dissolve"
#include "sc/shields"
#include "sc/freeze"
#include "sc/burrow"

#include "effect/Lightning"
#include "effect/HaloSprite"
#include "effect/FlashScreen"

//new const String:deniedWav[]   = "sc/buzz.wav";
new const String:errorWav[]      = "sc/perror.mp3";
new const String:deathWav[]      = "sc/pardth00.wav";
new const String:summonWav[]     = "sc/pdardy00.wav";
new const String:rechargeWav[]   = "sc/transmission.wav";
new const String:rageReadyWav[]  = "sc/pdapss03.wav";
new const String:rageExpireWav[] = "sc/pdawht00.wav";

new const String:archonWav[][]   = { "sc/pdapss00.wav" ,
                                     "sc/pdapss01.wav" ,
                                     "sc/pdapss02.wav" ,
                                     "sc/pdayes01.wav" ,
                                     "sc/pdayes03.wav" ,
                                     "sc/pdawht00.wav" ,
                                     "sc/pdawht02.wav" ,
                                     "sc/pdawht03.wav" };

new const String:g_PsiBladesSound[] = "sc/uzefir00.wav";

new raceID, shockwaveID, shieldsID, meleeID, rageID;
new maelstormID, controlID, ultimateFeedbackID;

new g_MindControlChance[]       = { 30, 50, 70, 90, 95 };
new Float:g_MindControlRange[]  = { 150.0, 300.0, 450.0, 650.0, 800.0 };
new Float:g_MaelstormRange[]    = { 350.0, 400.0, 650.0, 750.0, 900.0 };
new Float:g_PsiBladesPercent[]  = { 0.15, 0.35, 0.55, 0.75, 0.95 };

new Float:g_FeedbackRange[]     = { 350.0, 400.0, 650.0, 750.0, 900.0 };

//new Float:g_InitialShields[]  = { 0.25, 0.50, 1.0, 1.50, 2.0 };
new Float:g_InitialShields[]    = { 0.20, 0.50, 0.75, 1.0, 1.5 };
new Float:g_ShieldsPercent[][2] = { {0.10, 0.40},
                                    {0.15, 0.50},
                                    {0.20, 0.60},
                                    {0.25, 0.70},
                                    {0.30, 0.80} };

new bool:m_MindControlAvailable = false;
new bool:m_SidewinderAvailable = false;

public Plugin:myinfo = 
{
    name = "SourceCraft Race - Protoss Archon",
    author = "-=|JFH|=-Naris",
    description = "The Protoss Archon race for SourceCraft.",
    version = SOURCECRAFT_VERSION,
    url = "http://jigglysfunhouse.net/"
};

public OnPluginStart()
{
    LoadTranslations("sc.common.phrases.txt");
    LoadTranslations("sc.detector.phrases.txt");
    LoadTranslations("sc.dark_archon.phrases.txt");
    LoadTranslations("sc.mind_control.phrases.txt");
    LoadTranslations("sc.psionic_rage.phrases.txt");

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

    raceID = CreateRace("dark_archon", -1, -1, 28, 45, 100, 2,
                        Protoss, Energy, "dark_templar");

    shockwaveID = AddUpgrade(raceID, "shockwave", 0, 0, .energy=2);
    shieldsID   = AddUpgrade(raceID, "shields", 0, 0, .energy=1);
    meleeID     = AddUpgrade(raceID, "blades", 0, 0, .energy=2);

    // Ultimate 1
    maelstormID = AddUpgrade(raceID, "maelstorm", 1, 0, .energy=45, .cooldown=2.0);

    // Ultimate 2
    if (GameType == tf2 && (m_MindControlAvailable || LibraryExists("MindControl")))
    {
        m_MindControlAvailable = true;
        controlID = AddUpgrade(raceID, "mind_control", 2, .energy=45, .cooldown=2.0);
    }
    else
    {
        LogMessage("MindControl is not available");
        controlID = AddUpgrade(raceID, "mind_control", 2, 99, 0, .desc="%NotAvailable");
    }

    // Ultimate 3
    ultimateFeedbackID = AddUpgrade(raceID, "ultimate_feedback", 3, 8, .energy=30, .cooldown=3.0);

    // Ultimate 4
    rageID = AddUpgrade(raceID, "rage", 4, 12, .energy=180, .vespene=20, .cooldown=100.0);

    // Get Configuration Data
    GetConfigFloatArray("shields_amount",  g_InitialShields, sizeof(g_InitialShields),
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

    GetConfigFloatArray("range",  g_MaelstormRange, sizeof(g_MaelstormRange),
                        g_MaelstormRange, raceID, maelstormID);

    GetConfigArray("chance", g_MindControlChance, sizeof(g_MindControlChance),
                   g_MindControlChance, raceID, controlID);

    GetConfigFloatArray("range",  g_MindControlRange, sizeof(g_MindControlRange),
                        g_MindControlRange, raceID, controlID);

    GetConfigFloatArray("range",  g_FeedbackRange, sizeof(g_FeedbackRange),
                        g_FeedbackRange, raceID, ultimateFeedbackID);

}

public OnLibraryAdded(const String:name[])
{
    if (StrEqual(name, "MindControl"))
        m_MindControlAvailable = (GetGameType() == tf2);
    else if (strncmp(name, "sidewinder", 10, false) == 0)
        m_SidewinderAvailable = (GetGameType() == tf2);
}

public OnLibraryRemoved(const String:name[])
{
    if (StrEqual(name, "MindControl"))
        m_MindControlAvailable = false;
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

    SetupLightning(false, false);
    SetupHaloSprite(false, false);
    SetupUltimateFeedback(false, false);

    SetupSound(errorWav, true, false, false);
    SetupSound(deathWav, true, false, false);
    SetupSound(summonWav, true, false, false);
    SetupSound(deniedWav, true, false, false);
    SetupSound(rechargeWav, true, false, false);
    SetupSound(rageReadyWav, true, false, false);
    SetupSound(rageExpireWav, true, false, false);
    SetupSound(g_PsiBladesSound, true, false, false);

    for (new i = 0; i < sizeof(archonWav); i++)
        SetupSound(archonWav[i], true, false, false);
}

public OnMapEnd()
{
    ResetAllClientTimers();
    ResetAllShieldTimers();
}

public OnPlayerAuthed(client)
{
    m_RageActive[client] = false;
}

public OnClientDisconnect(client)
{
    m_RageActive[client] = false;
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
        SetVisibility(client, NormalVisibility);

        if (m_RageActive[client])
            EndRage(INVALID_HANDLE, GetClientUserId(client));

        if (m_MindControlAvailable)
            ResetMindControlledObjects(client, false);
    }
    return Plugin_Continue;
}

public Action:OnRaceSelected(client,oldrace,newrace)
{
    if (newrace == raceID)
    {
        m_RageActive[client] = false;

        //Set Archon Color
        new r,g,b;
        if (TFTeam:GetClientTeam(client) == TFTeam_Red)
        { r = 255; g = 0; b = 0; }
        else
        { r = 0; g = 0; b = 255; }
        SetVisibility(client, BasicVisibility, 
                      .mode=RENDER_GLOW,
                      .fx=RENDERFX_GLOWSHELL,
                      .r=r, .g=g, .b=b);

        new shields_level = GetUpgradeLevel(client,raceID,shieldsID);
        SetupShields(client, shields_level, g_InitialShields, g_ShieldsPercent);

        if (IsPlayerAlive(client))
        {
            PrepareSound(summonWav);
            EmitSoundToAll(summonWav, client);

            CreateClientTimer(client, 3.0, Regeneration, TIMER_REPEAT|TIMER_FLAG_NO_MAPCHANGE);
            CreateShieldTimer(client);
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
        if (upgrade == shieldsID)
        {
            SetupShields(client, new_level, g_InitialShields,
                         g_ShieldsPercent, .upgrade=true);
        }
    }
}

public OnUltimateCommand(client,race,bool:pressed,arg)
{
    if (pressed && race==raceID && IsPlayerAlive(client))
    {
        switch (arg)
        {
            case 4:
            {
                new rage_level = GetUpgradeLevel(client,race,rageID);
                if (rage_level > 0)
                {
                    PsionicRage(client, race, rageID, rage_level,
                                rageReadyWav, rageExpireWav);
                }
                else
                {
                    new ultimate_feedback_level = GetUpgradeLevel(client,race,ultimateFeedbackID);
                    if (ultimate_feedback_level > 0)
                    {
                        UltimateFeedback(client, raceID, ultimateFeedbackID,
                                         ultimate_feedback_level, g_FeedbackRange);
                    }
                    else
                    {
                        new maelstorm_level=GetUpgradeLevel(client,race,maelstormID);
                        if (maelstorm_level > 0)
                            Maelstorm(client,maelstorm_level);
                        else
                        {
                            new control_level=GetUpgradeLevel(client,race,controlID);
                            DoMindControl(client,control_level);
                        }
                    }
                }
            }
            case 3:
            {
                new ultimate_feedback_level = GetUpgradeLevel(client,race,ultimateFeedbackID);
                if (ultimate_feedback_level > 0)
                {
                    UltimateFeedback(client, raceID, ultimateFeedbackID,
                                     ultimate_feedback_level, g_FeedbackRange);
                }
                else
                {
                    new maelstorm_level=GetUpgradeLevel(client,race,maelstormID);
                    if (maelstorm_level > 0)
                        Maelstorm(client,maelstorm_level);
                    else
                    {
                        new control_level=GetUpgradeLevel(client,race,controlID);
                        DoMindControl(client,control_level);
                    }
                }
            }
            case 2:
            {
                new control_level=GetUpgradeLevel(client,race,controlID);
                DoMindControl(client,control_level);
            }
            default:
            {
                new maelstorm_level=GetUpgradeLevel(client,race,maelstormID);
                Maelstorm(client,maelstorm_level);
            }
        }
    }
}

public OnPlayerSpawnEvent(Handle:event, client, race)
{
    if (race == raceID)
    {
        m_RageActive[client] = false;

        PrepareSound(summonWav);
        EmitSoundToAll(summonWav, client);

        // Adjust Health to offset shields
        new shields_level = GetUpgradeLevel(client,raceID,shieldsID);
        new shield_amount = SetupShields(client, shields_level, g_InitialShields,
                                         g_ShieldsPercent);

        new health = GetClientHealth(client)-shield_amount;
        if (health <= 0)
            health = GetMaxHealth(client) / 2;

        SetEntityHealth(client, health);

        //Set Archon Color
        new r,g,b;
        if (TFTeam:GetClientTeam(client) == TFTeam_Red)
        { r = 255; g = 0; b = 0; }
        else
        { r = 0; g = 0; b = 255; }
        SetVisibility(client, BasicVisibility, 
                      .mode=RENDER_GLOW,
                      .fx=RENDERFX_GLOWSHELL,
                      .r=r, .g=g, .b=b);

        CreateClientTimer(client, 3.0, Regeneration, TIMER_REPEAT|TIMER_FLAG_NO_MAPCHANGE);
        CreateShieldTimer(client);
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
        {
            new blades_level=GetUpgradeLevel(attacker_index,raceID,meleeID);
            changed |= MeleeAttack(raceID, meleeID, blades_level, event, damage,
                                   victim_index, attacker_index, g_PsiBladesPercent,
                                   g_PsiBladesSound, "sc_blades");

            changed |= PsionicShockwave(damage, victim_index, attacker_index);
        }

        if (assister_index > 0 && assister_race == raceID)
            changed |= PsionicShockwave(damage, victim_index, assister_index);
    }

    return changed ? Plugin_Changed : Plugin_Continue;
}

public OnPlayerDeathEvent(Handle:event, victim_index, victim_race, attacker_index,
                          attacker_race, assister_index, assister_race, damage,
                          const String:weapon[], bool:is_equipment, customkill,
                          bool:headshot,bool:backstab,bool:melee)
{
    if (victim_race == raceID)
    {
        if (m_RageActive[victim_index])
            EndRage(INVALID_HANDLE, GetClientUserId(victim_index));

        PrepareSound(deathWav);
        EmitSoundToAll(deathWav, victim_index);

        DissolveRagdoll(victim_index, 0.1);        
        KillClientTimer(victim_index);
        KillShieldTimer(victim_index);
    }
}

public bool:PsionicShockwave(damage, victim_index, index)
{
    if (!GetRestriction(index, Restriction_PreventUpgrades) &&
        !GetRestriction(index, Restriction_Stunned) &&
        !GetImmunity(victim_index,Immunity_HealthTaking) &&
        !GetImmunity(index, Immunity_Upgrades) &&
        !IsInvulnerable(victim_index))
    {
        new shockwave_level=GetUpgradeLevel(index,raceID,shockwaveID);
        new adj = shockwave_level*10;
        if (GetRandomInt(1,100) <= GetRandomInt(10+adj,100-adj))
        {
            new energy = GetEnergy(index);
            new amount = GetUpgradeEnergy(raceID,shockwaveID);
            if (energy >= amount)
            {
                new dmgamt;
                switch(shockwave_level)
                {
                    case 0: dmgamt=damage / 2;
                    case 1: dmgamt=damage;
                    case 2: dmgamt=RoundFloat(float(damage)*1.50);
                    case 3: dmgamt=damage * 2;
                    case 4: dmgamt=RoundFloat(float(damage)*2.50);
                }
                if (dmgamt > 0)
                {
                    new Float:Origin[3];
                    GetClientAbsOrigin(victim_index, Origin);
                    Origin[2] += 5;

                    FlashScreen(victim_index,RGBA_COLOR_RED);
                    TE_SetupSparks(Origin,Origin,255,1);
                    TE_SendEffectToAll();

                    SetEnergy(index, energy-amount);
                    HurtPlayer(victim_index, dmgamt, index,
                               "sc_shockwave", "Psionic Shockwave",
                               .in_hurt_event=true, .type=DMG_SHOCK);
                    return true;
                }
            }
        }
    }
    return false;
}

public Maelstorm(client, level)
{
    decl String:upgradeName[64];
    GetUpgradeName(raceID, maelstormID, upgradeName, sizeof(upgradeName), client);

    new energy = GetEnergy(client);
    new amount = GetUpgradeEnergy(raceID,maelstormID);
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
    else if (HasCooldownExpired(client, raceID, maelstormID))
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

        new Float:range = g_MaelstormRange[level];

        new count=0;
        new Float:indexLoc[3];
        new Float:clientLoc[3];
        GetClientAbsOrigin(client, clientLoc);
        clientLoc[2] += 50.0; // Adjust trace position to the middle of the person instead of the feet.

        new lightning  = Lightning();
        new haloSprite = HaloSprite();
        static const color[4] = { 0, 255, 0, 255 };

        new team=GetClientTeam(client);
        for (new index=1;index<=MaxClients;index++)
        {
            if (client != index && IsValidClient(index) &&
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

                        if (GameType == tf2 &&
                            !GetImmunity(index,Immunity_Uncloaking) &&
                            TF2_GetPlayerClass(index) == TFClass_Spy)
                        {
                            TF2_RemovePlayerDisguise(index);
                            TF2_RemoveCondition(client,TFCond_Cloaked);

                            new Float:cloakMeter = TF2_GetCloakMeter(index);
                            if (cloakMeter > 0.0 && cloakMeter <= 100.0)
                                TF2_SetCloakMeter(index, 0.0);

                            DisplayMessage(index,Display_Enemy_Ultimate, "%t",
                                           "HasUndisguised", client, upgradeName);
                        }

                        if (!GetImmunity(index,Immunity_MotionTaking) &&
                            !GetImmunity(index,Immunity_Restore) &&
                            !IsBurrowed(index))
                        {
                            TE_SetupBeamPoints(clientLoc,indexLoc, lightning, haloSprite,
                                              0, 1, 3.0, 10.0,10.0,5,50.0,color,255);
                            TE_SendEffectToAll();

                            DisplayMessage(index,Display_Enemy_Ultimate, "%t",
                                           "HasEnsnared", client, upgradeName);

                            FreezeEntity(index);
                            CreateTimer(20.0,UnfreezePlayer,GetClientUserId(index),TIMER_FLAG_NO_MAPCHANGE);
                            count++;
                        }
                    }
                }
            }
        }

        if (count)
        {
            SetEnergy(client, energy-amount);
            DisplayMessage(client, Display_Ultimate, "%t",
                           "ToEnsnareEnemies", upgradeName,
                           count);
        }
        else
        {
            DisplayMessage(client,Display_Ultimate, "%t",
                           "WithoutEffect", upgradeName);
        }

        CreateCooldown(client, raceID, maelstormID);
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

DoMindControl(client,level)
{
    decl String:upgradeName[64];
    GetUpgradeName(raceID, controlID, upgradeName, sizeof(upgradeName), client);

    if (!m_MindControlAvailable)
    {
        PrepareSound(deniedWav);
        EmitSoundToClient(client,deniedWav);
        PrintHintText(client, "%t", "IsNotAvailable", upgradeName);
        return;
    }
    else
    {
        new energy = GetEnergy(client);
        new amount = GetUpgradeEnergy(raceID,controlID);
        if (energy < amount)
        {
            EmitEnergySoundToClient(client,Protoss);
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
        }
        else if (HasCooldownExpired(client, raceID, controlID))
        {
            new builder;
            new TFObjectType:type;
            if (MindControl(client, g_MindControlRange[level],
                            g_MindControlChance[level],
                            builder, type))
            {
                if (IsValidClient(builder))
                {
                    DisplayMessage(builder, Display_Enemy_Ultimate,
                                   "%t", "HasControlled", client,
                                   TF2_ObjectNames[type]);

                    DisplayMessage(client, Display_Ultimate, "%t", 
                                   "YouHaveControlled", builder,
                                   TF2_ObjectNames[type]);
                }

                SetEnergy(client, energy-amount);
                CreateCooldown(client, raceID, controlID);
            }
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
            new Float:vec[3];
            GetClientEyePosition(client, vec);
            
            new num = GetRandomInt(0,sizeof(archonWav)-1);
            PrepareSound(archonWav[num]);
            EmitAmbientSound(archonWav[num], vec, client);

            if (!GetRestriction(client,Restriction_PreventUpgrades) &&
                !GetRestriction(client,Restriction_Stunned))
            {
                new shields_level  = GetUpgradeLevel(client,raceID,shieldsID);
                new shields_amount = (shields_level+1)*3;
                RegenerateFullShields(client, shields_level, g_InitialShields,
                                      shields_amount, shields_amount);
            }
        }
    }
    return Plugin_Continue;
}

