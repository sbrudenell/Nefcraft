/**
 * vim: set ai et ts=4 sw=4 :
 * File: ZergUltralisk.sp
 * Description: The Zerg Ultralisk unit for SourceCraft.
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

// Define _TRACE to enable trace logging for debugging
//#define _TRACE
#include <trace>

#include "sc/SourceCraft"
#include "sc/MeleeAttack"
#include "sc/clienttimer"
#include "sc/maxhealth"
#include "sc/weapons"
#include "sc/burrow"
#include "sc/armor"

#include "effect/Lightning"
#include "effect/HaloSprite"
#include "effect/SendEffects"

new const String:deathWav[] = "sc/zuldth00.wav";
new const String:evolveWav[] = "sc/zulrdy00.wav";
new const String:cleaveWav[] = "sc/zulror00.wav";
new const String:deniedWav[] = "sc/buzz.wav";
new const String:rechargeWav[] = "sc/transmission.wav";

new raceID, armorID, speedID, regenerationID, meleeID, cleaveID, chargeID;

new const String:g_UberKaiserBladesSound[] = "sc/zulhit01.wav";
new Float:g_UberKaiserBladesPercent[] = { 0.15, 0.40, 0.65, 0.85, 1.00 };

new const String:g_ChargeSound[] = "sc/zulror00.wav";
new const String:g_ChargeAttackSound[][] = { "sc/zulatt00.wav" ,
                                             "sc/zulatt01.wav" ,
                                             "sc/zulatt02.wav" };
new Float:g_ChargePercent[] = { 0.15, 0.40, 0.65, 0.85, 1.00 };
#include "sc/Charge"

new const String:g_ArmorName[] = "Plating";
new Float:g_InitialArmor[]     = { 0.50, 1.00, 1.25, 1.50, 2.00 };
new Float:g_ArmorPercent[][2]  = { {0.00, 0.10},
                                   {0.10, 0.30},
                                   {0.30, 0.50},
                                   {0.40, 0.60},
                                   {0.60, 0.90} };

new Float:g_Gravity[]          = { 1.0, 2.0, 4.0, 6.0, 10.0 };

//new Float:speed=g_Speed[armor_level][speed_level];
new Float:g_Speed[][]          = { { 0.85, 0.90, 0.93, 0.97, 1.00 },
                                { 0.80, 0.85, 0.90, 0.95, 0.99 },
                                { 0.75, 0.80, 0.85, 0.90, 0.98 },
                                { 0.70, 0.80, 0.85, 0.90, 0.97 },
                                { 0.60, 0.70, 0.80, 0.90, 0.95 } };
//new Float:g_Speed[][]        = { { 0.85, 0.88, 0.93, 0.97, 1.00 },
//                                { 0.78, 0.84, 0.88, 0.93, 0.97 },
//                                { 0.75, 0.80, 0.85, 0.90, 0.95 },
//                                { 0.70, 0.75, 0.80, 0.85, 0.90 },
//                                { 0.65, 0.70, 0.75, 0.80, 0.85 } };
//new Float:g_Speed[][]        = { { 0.80, 0.85, 0.90, 0.95, 1.00 },
//                                { 0.75, 0.80, 0.85, 0.90, 0.95 },
//                                { 0.70, 0.75, 0.80, 0.85, 0.90 },
//                                { 0.65, 0.70, 0.75, 0.80, 0.85 },
//                                { 0.60, 0.65, 0.70, 0.75, 0.80 } };
//new Float:g_Speed[][]        = { { 1.00, 1.05, 1.10, 1.15, 1.20 },
//                                { 0.90, 1.00, 1.05, 1.10, 1.15 },
//                                { 0.80, 0.90, 1.00, 1.05, 1.10 },
//                                { 0.70, 0.80, 0.90, 1.00, 1.05 },
//                                { 0.60, 0.70, 0.80, 0.90, 1.00 } };

new g_sockItem = -1;
new g_bootsItem = -1;

new bool:m_HasAttacked[MAXPLAYERS+1];

public Plugin:myinfo = 
{
    name = "SourceCraft Unit - Zerg Ultralisk",
    author = "-=|JFH|=-Naris",
    description = "The Zerg Ultralisk unit for SourceCraft.",
    version = SOURCECRAFT_VERSION,
    url = "http://jigglysfunhouse.net/"
};

public OnPluginStart()
{
    LoadTranslations("sc.common.phrases.txt");
    LoadTranslations("sc.charge.phrases.txt");
    LoadTranslations("sc.ultralisk.phrases.txt");

    GetGameType();
    if (IsSourceCraftLoaded())
        OnSourceCraftReady();
}

public OnSourceCraftReady()
{
    raceID          = CreateRace("ultralisk", -1, -1, 24, .faction=Zerg,
                                 .type=Biological, .parent="omegalisk");

    armorID         = AddUpgrade(raceID, "armor", 0, 0);
    speedID         = AddUpgrade(raceID, "speed", 0, 0);
    meleeID         = AddUpgrade(raceID, "uber_blades", 0, 0, .energy=2);
    regenerationID  = AddUpgrade(raceID, "regeneration", 0, 0);

    // Ultimate 1
    cleaveID        = AddUpgrade(raceID, "cleave", 1, 0,
                                 .energy=45, .cooldown=2.0);

    // Ultimate 2
    AddBurrowUpgrade(raceID, 2, 0, 3, 3);

    // Ultimate 3
    chargeID        = AddUpgrade(raceID, "charge", 3, 4,
                                 .energy=200, .cooldown=20.0);

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

    GetConfigFloatArray("gravity", g_Gravity, sizeof(g_Gravity),
                        g_Gravity, raceID, armorID);

    for (new level=0; level < sizeof(g_Speed); level++)
    {
        decl String:key[32];
        Format(key, sizeof(key), "armor_level_%d", level);
        GetConfigFloatArray(key, g_Speed[level], sizeof(g_Speed[]),
                            g_Speed[level], raceID, speedID);
    }

    GetConfigFloatArray("damage_percent", g_UberKaiserBladesPercent, sizeof(g_UberKaiserBladesPercent),
                        g_UberKaiserBladesPercent, raceID, meleeID);

    GetConfigFloatArray("damage_percent", g_ChargePercent, sizeof(g_ChargePercent),
                        g_ChargePercent, raceID, chargeID);
}

public OnMapStart()
{
    SetupLightning(false, false);
    SetupHaloSprite(false, false);

    SetupSound(deathWav, true, false, false);
    SetupSound(evolveWav, true, false, false);
    SetupSound(cleaveWav, true, false, false);
    SetupSound(deniedWav, true, false, false);
    SetupSound(rechargeWav, true, false, false);
    SetupSound(g_UberKaiserBladesSound, true, false, false);

    SetupSound(g_ChargeSound, true, false, false);
    for (new i = 0; i < sizeof(g_ChargeAttackSound); i++)
        SetupSound(g_ChargeAttackSound[i], true, false, false);
}

public OnMapEnd()
{
    KillAllClientTimers();
}

public OnPlayerAuthed(client)
{
    m_HasAttacked[client] = false;
    m_ChargeActive[client] = false;
}

public OnClientDisconnect(client)
{
    m_HasAttacked[client] = false;
    m_ChargeActive[client] = false;
    KillClientTimer(client);
}

public Action:OnRaceDeselected(client,oldrace,newrace)
{
    if (oldrace == raceID)
    {
        TraceInto("ZergUltralisk", "OnRaceDeselected", "%N Changing from Ultralisk", client);

        SetOverrideSpeed(client,-1.0);
        SetOverrideGravity(client,-1.0);
        KillClientTimer(client);
        ResetArmor(client);

        TraceReturn("Set %N's speed=-1.0(default), gravity=-1.0(default)", client);
    }
    return Plugin_Continue;
}

public Action:OnRaceSelected(client,oldrace,newrace)
{
    if (newrace == raceID)
    {
        TraceInto("ZergUltralisk", "OnRaceSelected", "%N Changing to Ultralisk", client);

        m_HasAttacked[client] = false;
        m_ChargeActive[client] = false;

        new armor_level = GetUpgradeLevel(client,raceID,armorID);
        SetupArmor(client, armor_level, g_InitialArmor,
                   g_ArmorPercent, g_ArmorName);

        new speed_level = GetUpgradeLevel(client,raceID,speedID);
        SetupSpeedAndGravity(client, armor_level, speed_level);

        if (IsPlayerAlive(client))
        {
            PrepareSound(evolveWav);
            EmitSoundToAll(evolveWav,client);
            CreateClientTimer(client, 1.0, Regeneration, TIMER_REPEAT);
        }

        TraceReturn();
        return Plugin_Handled;
    }
    else
        return Plugin_Continue;
}

public OnUpgradeLevelChanged(client,race,upgrade,new_level)
{
    if (race == raceID && GetRace(client) == raceID)
    {
        if (upgrade==armorID)
        {
            new speed_level = GetUpgradeLevel(client,raceID,speedID);
            SetupSpeedAndGravity(client, new_level, speed_level);
            SetupArmor(client, new_level, g_InitialArmor,
                       g_ArmorPercent, g_ArmorName,
                       .upgrade=true);
        }
        else if (upgrade==speedID)
        {
            new armor_level = GetUpgradeLevel(client,raceID,armorID);
            SetupSpeedAndGravity(client, armor_level, new_level);
        }
    }
}

public OnUltimateCommand(client,race,bool:pressed,arg)
{
    if (race==raceID && IsPlayerAlive(client) && pressed)
    {
        switch (arg)
        {
            case 3:
            {
                TraceInto("ZergUltralisk", "OnUltimateCommand", "%N Charging", client);

                new charge_level=GetUpgradeLevel(client,race,chargeID);
                Charge(client, race, chargeID, charge_level, charge_level,
                       40, 150.0, 100.0, Zerg);

                TraceReturn();
            }
            case 2:
            {
                TraceInto("ZergUltralisk", "OnUltimateCommand", "%N Burrowing", client);

                Burrow(client, 3);

                TraceReturn();
            }
            default:
            {
                TraceInto("ZergUltralisk", "OnUltimateCommand", "%N Cleaving", client);

                new cleave_level=GetUpgradeLevel(client,race,cleaveID);
                Cleave(client, cleave_level);

                TraceReturn();
            }
        }
    }
}

// Events

public OnPlayerSpawnEvent(Handle:event, client, race)
{
    if (race == raceID)
    {
        m_HasAttacked[client] = false;
        m_ChargeActive[client] = false;

        PrepareSound(evolveWav);
        EmitSoundToAll(evolveWav,client);

        new armor_level = GetUpgradeLevel(client,raceID,armorID);
        SetupArmor(client, armor_level, g_InitialArmor,
                   g_ArmorPercent, g_ArmorName);

        new speed_level = GetUpgradeLevel(client,raceID,speedID);
        SetupSpeedAndGravity(client, armor_level, speed_level);

        CreateClientTimer(client, 1.0, Regeneration, TIMER_REPEAT);
    }
}

public Action:OnPlayerHurtEvent(Handle:event,victim_index,victim_race, attacker_index,
                                attacker_race,assister_index,assister_race,
                                damage, absorbed, bool:from_sc)
{
    new bool:changed=false;
    if (attacker_index && attacker_index != victim_index &&
        attacker_race == raceID && !GetEventBool(event, "sourcecraft"))
    {
        new kaiser_blades_level=GetUpgradeLevel(attacker_index,raceID,meleeID);
        if (kaiser_blades_level > 0)
        {
            changed |= UberMeleeAttack(raceID, meleeID, kaiser_blades_level, event, damage+absorbed,
                                       victim_index, attacker_index, g_UberKaiserBladesPercent,
                                       g_UberKaiserBladesSound, "sc_uber_blades");
        }
    }
    return changed ? Plugin_Changed : Plugin_Continue;
}

public OnPlayerDeathEvent(Handle:event,victim_index,victim_race, attacker_index,
                          attacker_race,assister_index,assister_race, damage,
                          const String:weapon[],bool:is_equipment,customkill,
                          bool:headshot,bool:backstab,bool:melee)
{
    if (victim_race == raceID)
    {
        TraceInto("ZergUltralisk", "OnPlayerDeathEvent", "victim_index=%d:%N, victim_race=%d, attacker_index=%d:%N, attacker_race=%d", \
                  victim_index, ValidClientIndex(victim_index), victim_race, \
                  attacker_index, ValidClientIndex(attacker_index), attacker_race);

        PrepareSound(deathWav);
        EmitSoundToAll(deathWav,victim_index);
        SetOverrideGravity(victim_index,-1.0);
        SetOverrideSpeed(victim_index,-1.0);
        KillClientTimer(victim_index);

        TraceReturn("Set %d:%N's speed=-1.0(default), gravity=-1.0(default)", \
                    victim_index, ValidClientIndex(victim_index));
    }
}

public Action:OnPlayerRunCmd(client, &buttons, &impulse, Float:vel[3], Float:angles[3], &weapon)
{
    if (buttons & (IN_ATTACK|IN_ATTACK2))
    {
        m_HasAttacked[client] = true;
    }
    return Plugin_Continue;
}

public Action:Regeneration(Handle:timer, any:userid)
{
    new client = GetClientOfUserId(userid);
    if (IsValidClient(client) && IsPlayerAlive(client) && GetRace(client) == raceID &&
        !GetRestriction(client,Restriction_PreventUpgrades) &&
        !GetRestriction(client,Restriction_Stunned))
    {
        new maxhp = GetMaxHealth(client);
        new level = GetUpgradeLevel(client,raceID,regenerationID)+1;
        new amount = RoundToNearest(float(level)*0.04*float(maxhp));
        new minimun = level*2;
        if (amount < minimun)
            amount = minimun;

        if (m_HasAttacked[client])            
            m_HasAttacked[client] = false;
        else            
            amount *= 2;

        HealPlayer(client, amount);
    }
    return Plugin_Continue;
}

SetupSpeedAndGravity(client, armor_level, speed_level, bool:apply=false)
{
    TraceInto("ZergUltralisk", "SetupSpeedAndGravity");

    new Float:speed=g_Speed[armor_level][speed_level];
    if (speed >= 0.0 && speed != 1.0)
    {
        /* If the Player also has the Boots of Speed,
         * Increase the speed further
         */
        if (g_bootsItem < 0)
            g_bootsItem = FindShopItem("boots");

        if (g_bootsItem != -1 && GetOwnsItem(client,g_bootsItem))
            speed *= 1.1;

        #if defined _TRACE
            new Float:calc = speed;
        #endif

        // Make slowest base speed 200 (Heavy == 230)
        new Float:limit = (TF2_IsPlayerSlowed(client) ? 80.0 : 200.0)
                          / TF2_GetClassSpeed(TF2_GetPlayerClass(client));
        if (speed < limit)
            speed = limit;

        SetOverrideSpeed(client,speed,apply);

        #if defined _TRACE
            Trace("Set %d:%N's speed=%4.2f, limit=%4.2f, calc=%4.2f", \
                  client, ValidClientIndex(client), speed, limit, calc);
        #endif
    }
    else
    {
        SetOverrideSpeed(client,-1.0,apply);
        Trace("Set %d:%N's speed=%4.2f(default)", \
              client, ValidClientIndex(client), -1.0);
    }

    new Float:gravity = g_Gravity[armor_level];
    if (gravity >= 0.0 && gravity != 1.0)
    {
        /* If the Player also has the Sock of the Feather,
         * Decrease the gravity further.
         */
        if (g_sockItem < 0)
            g_sockItem = FindShopItem("sock");

        if (g_sockItem != -1 && GetOwnsItem(client,g_sockItem))
            gravity *= 0.8;

        SetOverrideGravity(client,gravity,apply);

        Trace("Set %d:%N's gravity=%4.2f", \
              client, ValidClientIndex(client), gravity);
    }
    else
    {
        SetOverrideGravity(client,-1.0,apply);
        Trace("Set %d:%N's gravity=%4.2f(default)", \
              client, ValidClientIndex(client), -1.0);
    }

    Trace("Set %d:%N's speed=%4.2f, gravity=%4.2f", \
          client, ValidClientIndex(client), speed, gravity);

    if (IsPlayerAlive(client))
    {
        new Float:start[3];
        GetClientAbsOrigin(client, start);

        static const color[4] = { 100, 20, 100, 255 };
        TE_SetupBeamRingPoint(start,20.0,50.0, Lightning(), HaloSprite(),
                              0, 1, 2.0, 60.0, 0.8 ,color, 10, 1);
        TE_SendEffectToAll();
    }

    TraceReturn();
}

Cleave(client, level)
{
    new energy = GetEnergy(client);
    new amount = GetUpgradeEnergy(raceID,cleaveID);
    if (energy < amount)
    {
        decl String:upgradeName[64];
        GetUpgradeName(raceID, cleaveID, upgradeName, sizeof(upgradeName), client);
        DisplayMessage(client, Display_Energy, "%t", "InsufficientEnergyFor", upgradeName, amount);
        EmitEnergySoundToClient(client,Zerg);
    }
    else if (GetRestriction(client,Restriction_PreventUltimates) ||
             GetRestriction(client,Restriction_Stunned))
    {
        PrepareSound(deniedWav);
        EmitSoundToClient(client,deniedWav);

        decl String:upgradeName[64];
        GetUpgradeName(raceID, cleaveID, upgradeName, sizeof(upgradeName), client);
        DisplayMessage(client, Display_Ultimate, "%t", "Prevented", upgradeName);
    }
    else if (HasCooldownExpired(client, raceID, cleaveID))
    {
        new TFClassType:class;
        if (GameType == tf2)
        {
            class = TF2_GetPlayerClass(client);
            switch (class)
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
        else
            class = TFClass_Unknown;

        new lightning  = Lightning();
        new haloSprite = HaloSprite();
        static const cleaveColor[4] = {139, 69, 19, 255};
        new Float:range = 50.0 + float(level)*50.0;

        new dmg = (GameType == tf2 && (class == TFClass_Scout || class == TFClass_Spy))
                  ? GetRandomInt(1+(level*5),5+(level*10))
                  : GetRandomInt(1+(level*10),5+(level*20));

        PrepareSound(cleaveWav);
        EmitSoundToAll(cleaveWav,client);
        SetEnergy(client, energy-amount);

        new Float:indexLoc[3];
        new Float:clientLoc[3];
        GetClientAbsOrigin(client, clientLoc);
        clientLoc[2] += 50.0; // Adjust trace position to the middle of the person instead of the feet.

        new count = 0;
        new Float:force = float(level) * -75.0;
        new Float:vertForce = force * -2.0;
        if (vertForce < 300.0)
            vertForce = 300.0;

        new team = GetClientTeam(client);
        for (new index=1;index<=MaxClients;index++)
        {
            if (client != index && IsClientInGame(index) &&
                IsPlayerAlive(index) && GetClientTeam(index) != team)
            {
                GetClientAbsOrigin(index, indexLoc);
                indexLoc[2] += 50.0;

                if (IsPointInRange(clientLoc, indexLoc, range) &&
                    TraceTargetIndex(client, index, clientLoc, indexLoc))
                {
                    // Knockback isn't effected by immunities & uber!
                    Push(index, indexLoc, clientLoc, force, vertForce);

                    new bool:isUber = IsInvulnerable(index);
                    if ((level >= 2 || !GetImmunity(index,Immunity_HealthTaking)) &&
                        (level >= 3 || !GetImmunity(index,Immunity_Ultimates)) &&
                        (level >= 4 || !isUber))
                    {
                        TE_SetupBeamPoints(clientLoc,indexLoc, lightning, haloSprite,
                                           0, 1, 10.0, 10.0,10.0,2,50.0,cleaveColor,255);
                        TE_SendQEffectToAll(client,index);

                        new Float:Origin[3];
                        GetClientAbsOrigin(index, Origin);
                        Origin[2] += 5;

                        PrepareSound(g_UberKaiserBladesSound);
                        EmitSoundToAll(g_UberKaiserBladesSound,index);
                        TE_SetupSparks(Origin,Origin,255,1);
                        TE_SendEffectToAll();

                        new dmgamt=RoundFloat(float(dmg)*(1.0+g_UberKaiserBladesPercent[level]));
                        if (isUber)
                            dmgamt /= 2;

                        FlashScreen(index,RGBA_COLOR_RED);
                        HurtPlayer(index, dmgamt, client, "sc_cleave",
                                   .type=DMG_SLASH);

                        if (++count > level && level < 4)
                            break;
                    }
                }
            }
        }

        if (GetGameType() == tf2)
        {
            // Damage Structures
            new Float:pos[3];
            new maxents = GetMaxEntities();
            for (new ent = MaxClients; ent < maxents; ent++)
            {
                if (IsValidEdict(ent) && IsValidEntity(ent))
                {
                    if (TF2_GetExtObjectTypeFromClass(ent) != TFExtObject_Unknown)
                    {
                        if (GetEntProp(ent, Prop_Send, "m_iTeamNum") != team)
                        {
                            GetEntPropVector(ent, Prop_Send, "m_vecOrigin", pos);
                            if (IsPointInRange(clientLoc, pos, range) &&
                                TraceTargetEntity(client, ent, clientLoc, pos))
                            {
                                SetVariantInt(dmg);
                                AcceptEntityInput(ent, "RemoveHealth", client, client);
                            }
                        }
                    }
                }
            }
        }

        if (count)
        {
            DisplayMessage(client, Display_Ultimate,
                           "%t", "CleavedEnemies",
                           count);
        }
        else
        {
            DisplayMessage(client, Display_Ultimate,
                           "%t", "CleavedNothing");
        }
        CreateCooldown(client, raceID, cleaveID);
    }
}

