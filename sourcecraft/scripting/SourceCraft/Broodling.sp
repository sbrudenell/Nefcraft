/**
 * vim: set ai et ts=4 sw=4 :
 * File: Broodling.sp
 * Description: The Broodling race for SourceCraft.
 * Author(s): -=|JFH|=-Naris
 */
 
#pragma semicolon 1

#include <sourcemod>
#include <sdktools>
#include <raytrace>
#include <range>

#undef REQUIRE_EXTENSIONS
#include <tf2>
#include <tf2_player>
#define REQUIRE_EXTENSIONS

#include "sc/SourceCraft"
#include "sc/MeleeAttack"
#include "sc/clienttimer"
#include "sc/maxhealth"
#include "sc/weapons"
#include "sc/respawn"
#include "sc/burrow"

#include "effect/Lightning"
#include "effect/BeamSprite"
#include "effect/HaloSprite"
#include "effect/PurpleGlow"
#include "effect/SendEffects"
#include "effect/FlashScreen"

#define ResetExclaimTimer(%1) m_ExclaimTimers[%1] = INVALID_HANDLE

new raceID, degenerationID, meleeID, attackID;

new Float:g_BroodlingAttackRange[]      = { 1000.0, 800.0, 800.0, 800.0, 800.0 };
new Float:g_AdrenalGlandsPercent[]      = { 0.0, 0.15, 0.35, 0.55, 0.75 };

new const String:g_AdrenalGlandsSound[] = "sc/zbratt00.wav";

new const String:spawnWav[]             = "sc/zbrrdy00.wav";
new const String:deathWav[]             = "sc/zbrdth00.wav";
new const String:deniedWav[]            = "sc/buzz.wav";

new const String:broodlingWav[][]       = { "sc/zbrwht00.wav" ,
                                            "sc/zbrwht01.wav" ,
                                            "sc/zbrwht02.wav" ,
                                            "sc/zbrwht03.wav" ,
                                            "sc/zbrpss00.wav" ,
                                            "sc/zbrpss01.wav" ,
                                            "sc/zbrpss02.wav" ,
                                            "sc/zbrpss03.wav" };

new m_LastRace[MAXPLAYERS+1];
new m_Countdown[MAXPLAYERS+1];
new Handle:m_ExclaimTimers[MAXPLAYERS+1];

public Plugin:myinfo = 
{
    name = "SourceCraft Race - Broodling",
    author = "-=|JFH|=-Naris",
    description = "The Broodling race for SourceCraft.",
    version = SOURCECRAFT_VERSION,
    url = "http://jigglysfunhouse.net/"
};

public OnPluginStart()
{
    LoadTranslations("sc.broodling.phrases.txt");

    if (IsSourceCraftLoaded())
        OnSourceCraftReady();
}

public OnSourceCraftReady()
{
    raceID          = CreateRace("broodling", -1, -1, 17, .faction=Zerg, .type=Biological);

    degenerationID  = AddUpgrade(raceID, "degeneration", 0, 0);
    attackID        = AddUpgrade(raceID, "broodling_attack", 0, 0);
    meleeID         = AddUpgrade(raceID, "adrenal_glands", .energy=2);

    AddUpgrade(raceID, "spawning", 0, 0);

    // Ultimate 2
    AddBurrowUpgrade(raceID, 2, 6, 1);

    // Get Configuration Data
    GetConfigFloatArray("range", g_BroodlingAttackRange, sizeof(g_BroodlingAttackRange),
                        g_BroodlingAttackRange, raceID, attackID);

    GetConfigFloatArray("damage_percent", g_AdrenalGlandsPercent, sizeof(g_AdrenalGlandsPercent),
                        g_AdrenalGlandsPercent, raceID, meleeID);
}

public OnMapStart()
{
    SetupRespawn(false, false);
    SetupLightning(false, false);
    SetupBeamSprite(false, false);
    SetupHaloSprite(false, false);
    SetupPurpleGlow(false, false);

    SetupSound(spawnWav, true, false, false);
    SetupSound(deathWav, true, false, false);
    SetupSound(deniedWav, true, false, false);
    SetupSound(g_AdrenalGlandsSound, true, false, false);
    
    for (new i = 0; i < sizeof(broodlingWav); i++)
        SetupSound(broodlingWav[i], true, false, false);
}

public OnMapEnd()
{
    for (new i = 1; i <= MaxClients; i++)
    {
        ResetClientTimer(i);
        ResetExclaimTimer(i);
    }
}

public OnClientDisconnect(client)
{
    KillClientTimer(client);
    KillExclaimTimer(client);
}

public Action:OnRaceDeselected(client,oldrace,newrace)
{
    if (oldrace == raceID)
    {
        if (m_ReincarnationCount[client] < 1)
        {
            PrepareSound(deniedWav);
            EmitSoundToClient(client,deniedWav);
            DisplayMessage(client,Display_Misc_Message,
                           "%t", "BroodlingChange");
            return Plugin_Stop;
        }
        else
        {
            SetSpeed(client,-1.0);

            if (IsPlayerAlive(client))
            {
                TF2_RemoveCondition(client, TFCond_Jarated);
                KillExclaimTimer(client);
                KillClientTimer(client);
            }
        }
        return Plugin_Handled;
    }
    else
        return Plugin_Continue;
}   

public Action:OnRaceSelected(client,oldrace,newrace)
{
    if (newrace == raceID)
    {
        m_LastRace[client] = oldrace;
        m_Countdown[client] = 20;
        m_ReincarnationCount[client] = 0;

        if (IsPlayerAlive(client))
        {
            HudMessage(client, "%t", "BroodlingHud");

            PrepareSound(spawnWav);
            EmitSoundToAll(spawnWav, client);
            TF2_AddCondition(client, TFCond_Jarated, 1.0);
            CreateClientTimer(client, 1.0, Degeneration, TIMER_REPEAT|TIMER_FLAG_NO_MAPCHANGE);
            CreateExclaimTimer(client, 1.5, Exclaimation, TIMER_REPEAT|TIMER_FLAG_NO_MAPCHANGE);
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
        if (upgrade==burrowID)
        {
            if (new_level <= 0)
                ResetBurrow(client, true);
        }
    }
}

public OnUltimateCommand(client,race,bool:pressed,arg)
{
    if (race==raceID && IsPlayerAlive(client) && pressed)
    {
        new burrow_level = GetUpgradeLevel(client,race,burrowID);
        if (burrow_level > 0)
            Burrow(client, burrow_level);
    }
}

// Events
public OnPlayerSpawnEvent(Handle:event, client, race)
{
    if (race == raceID)
    {
        HudMessage(client, "%t", "BroodlingHud");

        m_Countdown[client] = 20;
        Respawned(client,false);
        PrepareSound(spawnWav);
        EmitSoundToAll(spawnWav, client);
        TF2_AddCondition(client, TFCond_Jarated, 1.0);
        CreateClientTimer(client, 1.0, Degeneration, TIMER_REPEAT|TIMER_FLAG_NO_MAPCHANGE);
        CreateExclaimTimer(client, 1.5, Exclaimation, TIMER_REPEAT|TIMER_FLAG_NO_MAPCHANGE);
    }
}

public Action:OnPlayerHurtEvent(Handle:event, victim_index, victim_race, attacker_index,
                                attacker_race, assister_index, assister_race,
                                damage, absorbed, bool:from_sc)
{
    new bool:changed=false;
    if (attacker_index && attacker_index != victim_index &&
        attacker_race == raceID && !from_sc)
    {
        new adrenal_glands_level = GetUpgradeLevel(attacker_index,raceID,meleeID);
        if (adrenal_glands_level > 0)
        {
            changed |= MeleeAttack(raceID, meleeID, adrenal_glands_level, event, damage+absorbed,
                                   victim_index, attacker_index, g_AdrenalGlandsPercent,
                                   g_AdrenalGlandsSound, "sc_adrenal_glands");
        }
    }
    return changed ? Plugin_Changed : Plugin_Continue;
}

public OnPlayerDeathEvent(Handle:event, victim_index, victim_race, attacker_index,
                          attacker_race, assister_index, assister_race, damage,
                          const String:weapon[], bool:is_equipment, customkill,
                          bool:headshot, bool:backstab, bool:melee)
{
    KillClientTimer(victim_index);
    KillExclaimTimer(victim_index);

    if (victim_race==raceID)
    {
        PrepareSound(deathWav);
        EmitSoundToAll(deathWav, victim_index);
        if (m_ReincarnationCount[victim_index] < 1)
        {
            Respawn(victim_index);
            TE_SetupGlowSprite(m_DeathLoc[victim_index],PurpleGlow(),1.0,3.5,150);
            TE_SendEffectToAll();

            DisplayMessage(victim_index,Display_Misc_Message,"%t", "BroodlingSpawn");
            if (attacker_index != victim_index && IsValidClient(attacker_index))
            {
                DisplayMessage(attacker_index,Display_Enemy_Message,"%t",
                               "BroodlingSpawned", victim_index);
            }
        }
        else
        {
            ClearHud(victim_index, "%t", "BroodlingHud");

            // Default race to human for new players.
            new race = m_LastRace[victim_index];
            if (race <= 0)
                race = FindRace("human");

            // Revert back to previous race upon death as an Broodling.
            ChangeRace(victim_index, race, true, true);
        }
    }
}

public Action:Degeneration(Handle:timer, any:userid)
{
    new client = GetClientOfUserId(userid);
    if (IsValidClient(client) && IsPlayerAlive(client) &&
        GetRace(client) == raceID)
    {
        new Float:clientLoc[3];
        GetClientAbsOrigin(client, clientLoc);
        clientLoc[2] += 50.0; // Adjust trace position to the middle of the person instead of the feet.

        new degeneration_level=GetUpgradeLevel(client,raceID,degenerationID);
        HurtPlayer(client,8-degeneration_level,client,"sc_degeneration",
                   .type=DMG_POISON);

        FlashScreen(client,RGBA_COLOR_RED);
        TF2_AddCondition(client, TFCond_Jarated, 1.0);

        new attack_level=GetUpgradeLevel(client,raceID,attackID);
        if (attack_level > 0)
        {
            new Float:indexLoc[3];
            new Float:range=g_BroodlingAttackRange[attack_level];

            new lightning  = Lightning();
            new haloSprite = HaloSprite();
            static const attackColor[4] = {255, 10, 55, 255};

            new count=0;
            new alt_count=0;
            new list[MaxClients+1];
            new alt_list[MaxClients+1];
            new team=GetClientTeam(client);
            for (new index=1;index<=MaxClients;index++)
            {
                if (index != client && IsValidClient(index) &&
                    IsPlayerAlive(index) && GetClientTeam(index) == team)
                {
                    if (!GetImmunity(index, Immunity_HealthTaking) &&
                        !GetImmunity(index, Immunity_Upgrades) &&
                        !IsInvulnerable(index))
                    {
                        GetClientAbsOrigin(index, indexLoc);
                        indexLoc[2] += 50.0;

                        if (IsPointInRange(clientLoc,indexLoc,range) &&
                            TraceTargetIndex(client, index, clientLoc, indexLoc))
                        {
                            TE_SetupBeamPoints(clientLoc,indexLoc, lightning, haloSprite,
                                              0, 1, 3.0, 10.0,10.0,5,50.0,attackColor,255);
                            TE_SendQEffectToAll(client,index);
                            FlashScreen(index,RGBA_COLOR_RED);

                            EmitSoundToAll(g_AdrenalGlandsSound,index);
                            HurtPlayer(index,10-attack_level,client,
                                       "sc_broodling_attack",
                                       .type=DMG_SLASH);

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
            }

            if (!GetSetting(client, Disable_Beacons) &&
                !GetSetting(client, Remove_Queasiness))
            {
                if (GetSetting(client, Reduce_Queasiness))
                    alt_list[alt_count++] = client;
                else
                    list[count++] = client;
            }

            clientLoc[2] -= 50.0; // Adjust position back to the feet.

            if (count > 0)
            {
                TE_SetupBeamRingPoint(clientLoc, 10.0, range, BeamSprite(), haloSprite,
                                      0, 10, 0.6, 10.0, 0.5, attackColor, 10, 0);

                TE_Send(list, count, 0.0);
            }

            if (alt_count > 0)
            {
                TE_SetupBeamRingPoint(clientLoc, range-10.0, range, BeamSprite(), haloSprite,
                                      0, 10, 0.6, 10.0, 0.5, attackColor, 10, 0);

                TE_Send(alt_list, alt_count, 0.0);
            }
        }
        return Plugin_Continue;
    }
    return Plugin_Stop;
}

public Action:Exclaimation(Handle:timer, any:client)
{
    if (IsValidClient(client) && IsPlayerAlive(client))
    {
        if (GetRace(client) == raceID)
        {
            new Float:clientLoc[3];
            GetClientAbsOrigin(client, clientLoc);

            new num = GetRandomInt(0,sizeof(broodlingWav)-1);
            PrepareSound(broodlingWav[num]);
            EmitAmbientSound(broodlingWav[num], clientLoc, client);

            if (--m_Countdown[client] > 0)
                return Plugin_Continue;
            else
                KillPlayer(client, client, "sc_expire");
        }
    }
    m_ExclaimTimers[client] = INVALID_HANDLE;	
    return Plugin_Stop;
}

stock CreateExclaimTimer(client, Float:interval, Timer:func,
                         flags=TIMER_REPEAT|TIMER_FLAG_NO_MAPCHANGE)
{
    if (m_ExclaimTimers[client] == INVALID_HANDLE)
        m_ExclaimTimers[client] = CreateTimer(interval,func,client,flags);
}

stock KillExclaimTimer(client)
{
    new Handle:timer=m_ExclaimTimers[client];
    if (timer != INVALID_HANDLE)
    {
        m_ExclaimTimers[client] = INVALID_HANDLE;	
        KillTimer(timer);
    }
}
