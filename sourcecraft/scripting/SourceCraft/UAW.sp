/**
 * vim: set ai et ts=4 sw=4 :
 * File: UAW.sp
 * Description: The UAW race for SourceCraft.
 * Author(s): -=|JFH|=-Naris (Murray Wilson) 
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

#undef REQUIRE_PLUGIN
#include <hgrsource>
#define REQUIRE_PLUGIN

// Define _TRACE to enable trace logging for debugging
//#define _TRACE
#include <trace>

#include "sc/SourceCraft"
#include "sc/ShopItems"
#include "sc/clienttimer"
#include "sc/maxhealth"
#include "sc/respawn"

#include "effect/Explosion"
#include "effect/PurpleGlow"
#include "effect/SendEffects"

new const String:deniedWav[] = "sc/buzz.wav";
new const String:explodeWav[] = "weapons/explode5.wav";

new g_BuyoutChance[]        = { 0, 9, 22, 50, 63 };
new g_JobsBankChance[]      = { 0, 7, 15, 30, 50 };
new g_ShelteredChance[]     = { 0, 5, 10, 20, 35 };

new g_HookDuration[]        = { 0,    10,    20,    40,   -1  };
new Float:g_HookCooldown[]  = { 0.0,  20.0,  15.0,  10.0, 5.0 };
new Float:g_HookRange[]     = { 0.0, 150.0, 300.0, 450.0, 0.0 };

new g_RopeDuration[]        = { 0,     5,    10,    20,   -1  };
new Float:g_RopeCooldown[]  = { 0.0,  20.0,  15.0,  10.0, 0.0 };
new Float:g_RopeRange[]     = { 0.0, 150.0, 300.0, 450.0, 0.0 };

new raceID, wageID, seniorityID, negotiationID, rulesID, hookID, ropeID;

// Reincarnation variables
new bool:m_JobsBank[MAXPLAYERS+1];
new bool:m_TeleportOnSpawn[MAXPLAYERS+1];
new Float:m_SpawnLoc[MAXPLAYERS+1][3];

new bool:m_HGRSourceAvailable = false;

public Plugin:myinfo = 
{
    name = "SourceCraft Race - UAW",
    author = "-=|JFH|=-Naris (Murray Wilson)",
    description = "The UAW race for War3Source.",
    version = SOURCECRAFT_VERSION,
    url = "http://jigglysfunhouse.net/"
};

public OnPluginStart()
{
    LoadTranslations("sc.common.phrases.txt");
    LoadTranslations("sc.uaw.phrases.txt");

    GetGameType();
    if (IsSourceCraftLoaded())
        OnSourceCraftReady();
}

public OnSourceCraftReady()
{
    raceID        = CreateRace("uaw", 16, 0, 24, .faction=UndeadScourge, .type=Undead);

    wageID        = AddUpgrade(raceID, "wages");
    seniorityID   = AddUpgrade(raceID, "seniority");
    negotiationID = AddUpgrade(raceID, "negotiations", 0, 0);
    rulesID       = AddUpgrade(raceID, "rules");

    if (m_HGRSourceAvailable || LibraryExists("hgrsource"))
    {
        // Ultimate 1
        hookID    = AddUpgrade(raceID, "hook", 1, .energy=1,
                               .recurring_energy=1);

        // Ultimate 2
        ropeID    = AddUpgrade(raceID, "rope", 2);

        if (!m_HGRSourceAvailable)
        {
            m_HGRSourceAvailable = true;
            ControlHookGrabRope(true);
        }
    }
    else
    {
        LogError("HGR:Source is not available");

        // Ultimate 1
        hookID    = AddUpgrade(raceID, "hook", 1, 99, 0,
                               .desc="%NotAvailable");

        // Ultimate 2
        ropeID    = AddUpgrade(raceID, "rope", 2, 99, 0,
                               .desc="%NotAvailable");
    }

    // Get Configuration Data
    GetConfigArray("buyout_chance", g_BuyoutChance, sizeof(g_BuyoutChance),
                   g_BuyoutChance, raceID, seniorityID);

    GetConfigArray("jobs_bank_chance", g_JobsBankChance, sizeof(g_JobsBankChance),
                   g_JobsBankChance, raceID, seniorityID);

    GetConfigArray("sheltered_chance", g_ShelteredChance, sizeof(g_ShelteredChance),
                   g_ShelteredChance, raceID, seniorityID);

    GetConfigArray("duration", g_HookDuration, sizeof(g_HookDuration),
                   g_HookDuration, raceID, hookID);

    GetConfigFloatArray("range", g_HookRange, sizeof(g_HookRange),
                        g_HookRange, raceID, hookID);

    GetConfigFloatArray("cooldown_per_level", g_HookCooldown,
                        sizeof(g_HookCooldown), g_HookCooldown,
                        raceID, hookID);

    GetConfigArray("duration", g_RopeDuration, sizeof(g_RopeDuration),
                   g_RopeDuration, raceID, ropeID);

    GetConfigFloatArray("range", g_RopeRange, sizeof(g_RopeRange),
                        g_RopeRange, raceID, ropeID);

    GetConfigFloatArray("cooldown_per_level", g_RopeCooldown,
                        sizeof(g_RopeCooldown), g_RopeCooldown,
                        raceID, ropeID);
}

public OnLibraryAdded(const String:name[])
{
    if (StrEqual(name, "hgrsource"))
    {
        if (!m_HGRSourceAvailable)
        {
            m_HGRSourceAvailable = true;
            ControlHookGrabRope(true);
        }
    }
}

public OnLibraryRemoved(const String:name[])
{
    if (StrEqual(name, "hgrsource"))
        m_HGRSourceAvailable = false;
}

public OnMapStart()
{
    SetupPurpleGlow(false, false);
    SetupBigExplosion(false, false);

    SetupSound(deniedWav, true, false, false);
    SetupSound(explodeWav, true, false, false);
}

public OnMapEnd()
{
    ResetAllClientTimers();
}

public OnClientDisconnect(client)
{
    KillClientTimer(client);
}

public Action:OnRaceDeselected(client,oldrace,newrace)
{
    if (oldrace == raceID)
    {
        SetEnergyRate(client, -1);
        KillClientTimer(client);
        if (m_HGRSourceAvailable)
        {
            TakeHook(client);
            TakeRope(client);
        }

        new maxCrystals = GetMaxCrystals();
        if (GetCrystals(client) > maxCrystals)
        {
            SetCrystals(client, maxCrystals);
            DisplayMessage(client, Display_Crystals, "%t",
                           "CrystalsReduced", maxCrystals);
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
        m_JobsBank[client]=false;
        m_TeleportOnSpawn[client]=false;

        new rules_level = GetUpgradeLevel(client,raceID,rulesID);
        SetEnergyRate(client, (rules_level > 0) ? rules_level : -1);

        new hook_level=GetUpgradeLevel(client,raceID,hookID);
        SetupHook(client, hook_level);

        new rope_level=GetUpgradeLevel(client,raceID,ropeID);
        SetupRope(client, rope_level);

        if (IsPlayerAlive(client))
        {
            CreateClientTimer(client, 10.0, Negotiations,
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
        if (upgrade==hookID)
            SetupHook(client, new_level);
        else if (upgrade==ropeID)
            SetupRope(client, new_level);
        else if (upgrade==rulesID)
            SetEnergyRate(client, (new_level > 0) ? new_level : -1);
    }
}

public OnUltimateCommand(client,race,bool:pressed,arg)
{
    if (race==raceID && IsPlayerAlive(client) && m_HGRSourceAvailable)
    {
        switch (arg)
        {
            case 4,3,2:
            {
                TraceInto("UAW", "OnUltimateCommand", "%N Roping/Detaching", client);
                new rope_level = GetUpgradeLevel(client,raceID,ropeID);
                if (rope_level > 0)
                {
                    if (m_HGRSourceAvailable)
                    {
                        if (pressed)
                        {
                            if (GetRestriction(client, Restriction_PreventUltimates) ||
                                GetRestriction(client, Restriction_Grounded) ||
                                GetRestriction(client, Restriction_Stunned))
                            {
                                PrepareSound(deniedWav);
                                EmitSoundToClient(client,deniedWav);
                                DisplayMessage(client, Display_Ultimate, "%t", "PreventedFromSwinging");
                            }
                            else
                                Rope(client);
                        }
                        else
                            Detach(client);
                    }
                    else if (pressed)
                    {
                        decl String:upgradeName[64];
                        GetUpgradeName(raceID, ropeID, upgradeName, sizeof(upgradeName), client);
                        PrintHintText(client,"%t", "IsNotAvailable", upgradeName);
                    }
                }

                TraceReturn();
            }
            default:
            {
                TraceInto("UAW", "OnUltimateCommand", "%N Hooking/Unhooking", client);

                new hook_level = GetUpgradeLevel(client,raceID,hookID);
                if (hook_level > 0)
                {
                    if (m_HGRSourceAvailable)
                    {
                        if (pressed)
                        {
                            new energy = GetEnergy(client);
                            new amount = GetUpgradeEnergy(raceID,hookID);
                            if (energy < amount)
                            {
                                EmitEnergySoundToClient(client,UndeadScourge);
                                DisplayMessage(client, Display_Energy, "%t", "InsufficientEnergyToHook",
                                               amount);
                            }
                            else if (GetRestriction(client, Restriction_PreventUltimates) ||
                                     GetRestriction(client, Restriction_Grounded) ||
                                     GetRestriction(client, Restriction_Stunned))
                            {
                                PrepareSound(deniedWav);
                                EmitSoundToClient(client,deniedWav);
                                DisplayMessage(client, Display_Ultimate, "%t", "PreventedFromHooking");
                            }
                            else
                                Hook(client);
                        }
                        else
                            UnHook(client);
                    }
                    else if (pressed)
                    {
                        decl String:upgradeName[64];
                        GetUpgradeName(raceID, hookID, upgradeName, sizeof(upgradeName), client);
                        PrintHintText(client,"%t", "IsNotAvailable", upgradeName);
                    }
                }

                TraceReturn();
            }
        }
    }
}

public RoundStartEvent(Handle:event,const String:name[],bool:dontBroadcast)
{
    for (new x=1;x<=MaxClients;x++)
    {
        m_TeleportOnSpawn[x]=false;
        m_JobsBank[x]=false;
    }
}

public OnPlayerSpawnEvent(Handle:event, client, race)
{
    if (race == raceID)
    {
        CreateClientTimer(client, 10.0, Negotiations,
                          TIMER_REPEAT|TIMER_FLAG_NO_MAPCHANGE);

        new rules_level = GetUpgradeLevel(client,raceID,rulesID);
        SetEnergyRate(client, (rules_level > 0) ? rules_level : -1);

        new hook_level=GetUpgradeLevel(client,raceID,hookID);
        SetupHook(client, hook_level);

        new rope_level=GetUpgradeLevel(client,raceID,ropeID);
        SetupRope(client, rope_level);

        if (m_TeleportOnSpawn[client])
        {
            m_TeleportOnSpawn[client]=false;

            // Avoid the TF2 Heavy spinning minigun bug.
            if (GetGameType() == tf2 && TF2_IsPlayerSlowed(client) &&
                TF2_GetPlayerClass(client) == TFClass_Heavy)
            {
                // Stun the heavy briefly to stop the minigun from spinning
                TF2_StunPlayer(client, 0.2, 0.0, TF_STUNFLAG_THIRDPERSON);
            }

            CreateTimer(0.1, TeleportOnSpawn, GetClientUserId(client),
                        TIMER_FLAG_NO_MAPCHANGE);
        }
        else
        {
            GetClientAbsOrigin(client,m_SpawnLoc[client]);

            if (m_JobsBank[client])
            {
                m_JobsBank[client]=false;
                TE_SetupGlowSprite(m_SpawnLoc[client], PurpleGlow(), 1.0, 3.5, 150);
                TE_SendEffectToAll();
                DisplayMessage(client,Display_Misc_Message,
                               "%t", "JobsBank");
            }
        }
    }
}

public OnPlayerDeathEvent(Handle:event, victim_index, victim_race, attacker_index,
                          attacker_race, assister_index, assister_race, damage,
                          const String:weapon[], bool:is_equipment, customkill,
                          bool:headshot, bool:backstab, bool:melee)
{
    KillClientTimer(victim_index);

    if (victim_race == raceID)
    {
        new seniority_level=GetUpgradeLevel(victim_index,raceID,seniorityID);
        if (seniority_level > 0 && !GetRestriction(victim_index, Restriction_PreventUpgrades) &&
            !GetRestriction(victim_index, Restriction_Stunned))
        {
            new chance = GetRandomInt(1,100);
            if (chance<=g_ShelteredChance[seniority_level] &&
                !IsMole(victim_index))
            {
                DisplayMessage(victim_index,Display_Misc_Message,
                               "%t", "YouWereSheltered");

                if (attacker_index != victim_index)
                {
                    if (attacker_index > 0)
                    {
                        DisplayMessage(attacker_index,Display_Enemy_Message,
                                       "%t", "WasSheltered", victim_index);
                    }

                    if (assister_index > 0)
                    {
                        DisplayMessage(assister_index,Display_Enemy_Message,
                                       "%t", "WasSheltered", victim_index);
                    }
                }

                m_TeleportOnSpawn[victim_index]=true;
                GetClientAbsOrigin(victim_index,m_SpawnLoc[victim_index]);
                CreateTimer(0.1,RespawnPlayerHandler,
                            GetClientUserId(victim_index),
                            TIMER_FLAG_NO_MAPCHANGE);
            }
            else if (chance<=g_JobsBankChance[seniority_level])
            {
                m_JobsBank[victim_index]=true;
                CreateTimer(0.1,RespawnPlayerHandler,
                            GetClientUserId(victim_index),
                            TIMER_FLAG_NO_MAPCHANGE);
            }
            else if (chance<=g_BuyoutChance[seniority_level])
            {
                // No monetary limit on UAW Buyout offers!
                new amount = GetRandomInt(1,100);
                SetCrystals(victim_index, GetCrystals(victim_index)+amount,false);
                DisplayMessage(victim_index,Display_Misc_Message, "%t", "Buyout", amount);
            }
        }
    }
}

public Action:TeleportOnSpawn(Handle:timer,any:userid)
{
    new client = GetClientOfUserId(userid);
    if (IsValidClient(client))
    {
        TeleportEntity(client,m_SpawnLoc[client], NULL_VECTOR, NULL_VECTOR);
        TE_SetupGlowSprite(m_SpawnLoc[client], PurpleGlow(), 1.0, 3.5, 150);
        TE_SendEffectToAll();
    }
    return Plugin_Stop;
}

public Action:OnXPGiven(client,&amount,bool:taken)
{
    if (GetRace(client)==raceID && IsPlayerAlive(client) &&
        !GetRestriction(client, Restriction_PreventUpgrades) &&
        !GetRestriction(client, Restriction_Stunned))
    {
        new inflated_wages_level=GetUpgradeLevel(client,raceID,wageID);
        if (inflated_wages_level > 0)
        {
            amount=RoundToNearest(float(amount)*(1.0 + (float(inflated_wages_level)*0.5)));
            new cap = GetLevelXP(client, raceID);
            if (amount > cap)
                amount = cap;
            return Plugin_Changed;
        }
        else
            return Plugin_Handled;
    }
    else
        return Plugin_Continue;
}

public Action:OnCrystalsGiven(client,&amount,bool:taken)
{
    if (GetRace(client)==raceID && IsPlayerAlive(client) &&
        !GetRestriction(client, Restriction_PreventUpgrades) &&
        !GetRestriction(client, Restriction_Stunned))
    {
        new inflated_wages_level=GetUpgradeLevel(client,raceID,wageID);
        if (inflated_wages_level > 0)
        {
            amount *= (inflated_wages_level+1);
            return Plugin_Changed;
        }
        else
            return Plugin_Handled;
    }
    else
        return Plugin_Continue;
}

public SetupHook(client, level)
{
    if (m_HGRSourceAvailable)
    {
        if (level > 0)
            GiveHook(client,g_HookDuration[level],g_HookRange[level],g_HookCooldown[level],0);
        else
            TakeHook(client);
    }
}

public SetupRope(client, level)
{
    if (m_HGRSourceAvailable)
    {
        if (level > 0)
            GiveRope(client,g_RopeDuration[level],g_RopeRange[level],g_RopeCooldown[level],0);
        else
            TakeRope(client);
    }
}

public Action:OnHook(client)
{
    if (GetRestriction(client,Restriction_PreventUltimates) ||
        GetRestriction(client,Restriction_Grounded) ||
        GetRestriction(client,Restriction_Stunned))
    {
        PrepareSound(deniedWav);
        EmitSoundToClient(client,deniedWav);
        DisplayMessage(client, Display_Ultimate, "%t", "PreventedFromHooking");
        return Plugin_Stop;
    }
    else
    {
        new energy = GetEnergy(client);
        new amount = GetUpgradeRecurringEnergy(raceID,hookID);
        if (energy < amount)
        {
            EmitEnergySoundToClient(client,UndeadScourge);
            DisplayMessage(client, Display_Energy, "%t", "InsufficientEnergyToHook", amount);
            return Plugin_Stop;
        }
        else
        {
            SetEnergy(client, energy-amount);
            return Plugin_Continue;
        }
    }
}

public Action:OnRope(client)
{
    if (GetRestriction(client,Restriction_PreventUltimates) ||
        GetRestriction(client,Restriction_Grounded) ||
        GetRestriction(client,Restriction_Stunned))
    {
        PrepareSound(deniedWav);
        EmitSoundToClient(client,deniedWav);
        DisplayMessage(client, Display_Ultimate, "%t", "PreventedFromSwinging");
        return Plugin_Stop;
    }
    else
        return Plugin_Continue;
}

public Action:Negotiations(Handle:timer, any:userid)
{
    new client = GetClientOfUserId(userid);
    if (IsValidClient(client) && IsPlayerAlive(client) &&
        !GetRestriction(client, Restriction_PreventUpgrades) &&
        !GetRestriction(client, Restriction_Stunned))
    {
        if (GetRace(client) == raceID)
        {
            static percent[] = { 1, 10, 25, 35, 45, 55 };
            new negotiations_level=GetUpgradeLevel(client,raceID,negotiationID)+1;
            if (GetRandomInt(1,100) <= percent[negotiations_level])
            {
                /*Negotiations:
                 * Overtime/Premium Pay/Shift differential/COLA/Profit Sharing/Bonus - Get Money
                 * Grievance/Shop Steward/Arbitration/Collective Bargaining/Fringe Benefits - Get XP
                 * Boycott/Strike/Picketing (you teleport back to spawn)
                 * Lockout/Workforce Reduction (you die)
                 * Forced Buyout (you die and get money for XP and Level)
                 * Bankruptcy (you die & lose level & XP)
                 */
                // No monetary limit on UAW Money!

                new option;
                if (GetRandomInt(1,100) <= percent[negotiations_level])
                {
                    option = GetRandomInt(1,21);
                    if (option > 12)
                        return Plugin_Continue;
                }
                else
                    option = GetRandomInt(1,41) % 30;

                switch(option)
                {
                    case 1: // Overtime
                        AddCrystals(client, GetRandomInt(negotiations_level,10*negotiations_level), "Overtime");
                    case 2: // Premium Pay
                        AddCrystals(client, GetRandomInt(negotiations_level,10*negotiations_level), "PremiumPay");
                    case 3: // Shift Differential 
                        AddCrystals(client, GetRandomInt(negotiations_level,10*negotiations_level), "ShiftDifferentialPay");
                    case 4: // COLA 
                        AddCrystals(client, GetRandomInt(negotiations_level,10*negotiations_level), "COLA");
                    case 5: // Profit Sharing 
                        AddCrystals(client, GetRandomInt(negotiations_level,10*negotiations_level), "ProfitSharing");
                    case 6: // Bonus
                        AddCrystals(client, GetRandomInt(negotiations_level,10*negotiations_level), "Bonus");
                    case 7: // Grievance
                    {
                        new amount = GetRandomInt(negotiations_level,10*negotiations_level);
                        ResetXP(client, raceID, GetXP(client, raceID)+amount);
                        DisplayMessage(client,Display_Misc_Message, "%t", "Grievance", amount);
                    }
                    case 8: // Shop Steward
                    {
                        new amount = GetRandomInt(negotiations_level,10*negotiations_level);
                        ResetXP(client, raceID, GetXP(client, raceID)+amount);
                        DisplayMessage(client,Display_Misc_Message, "%t", "ShopSteward", amount);
                    }
                    case 9: // Arbitration
                    {
                        new amount = GetRandomInt(negotiations_level,10*negotiations_level);
                        ResetXP(client, raceID, GetXP(client, raceID)+amount);
                        DisplayMessage(client,Display_Misc_Message, "%t", "Arbitration",amount);
                    }
                    case 10: // Collective Bargaining
                    {
                        new amount = GetRandomInt(negotiations_level,10*negotiations_level);
                        ResetXP(client, raceID, GetXP(client, raceID)+amount);
                        DisplayMessage(client,Display_Misc_Message, "%t", "Bargaining", amount);
                    }
                    case 11: // Fringe Benefits
                    {
                        new amount = GetRandomInt(negotiations_level,10*negotiations_level);
                        ResetXP(client, raceID, GetXP(client, raceID)+amount);
                        DisplayMessage(client,Display_Misc_Message, "%t", "Benefits",amount);
                    }
                    case 12: // Boycott
                    {
                        DisplayMessage(client,Display_Misc_Message, "%t", "Boycott");
                        CreateTimer(0.1,RespawnPlayerHandler, userid,TIMER_FLAG_NO_MAPCHANGE);
                    }
                    case 13: // Strike
                    {
                        DisplayMessage(client,Display_Misc_Message, "%t", "Strike");
                        CreateTimer(0.1,RespawnPlayerHandler, userid,TIMER_FLAG_NO_MAPCHANGE);
                    }
                    case 14: // Picketing
                    {
                        DisplayMessage(client,Display_Misc_Message, "%t", "Picketing");
                        CreateTimer(0.1,RespawnPlayerHandler, userid,TIMER_FLAG_NO_MAPCHANGE);
                    }
                    case 15: // Lockout
                    {
                        if (GetRandomInt(1,100) > 20+negotiations_level)
                            TriggerTimer(timer); // Get a different Negotiation
                        else
                        {
                            DisplayMessage(client,Display_Deaths, "%t", "Lockout");

                            new Float:location[3];
                            GetClientAbsOrigin(client,location);
                            TE_SetupExplosion(location, BigExplosion(), 10.0, 30, 0, 50, 20);
                            TE_SendEffectToAll();

                            PrepareSound(explodeWav);
                            EmitSoundToAll(explodeWav,client);
                            KillPlayer(client, client, "sc_lockout", .explode=true);
                        }
                    }
                    case 16: // Workforce Reduction
                    {
                        if (GetRandomInt(1,100) > 20+negotiations_level)
                            TriggerTimer(timer); // Get a different Negotiation
                        else
                        {
                            DisplayMessage(client,Display_Deaths, "%t", "Reduction");

                            new Float:location[3];
                            GetClientAbsOrigin(client,location);
                            TE_SetupExplosion(location, BigExplosion(), 10.0, 30, 0, 50, 20);
                            TE_SendEffectToAll();

                            PrepareSound(explodeWav);
                            EmitSoundToAll(explodeWav,client);
                            KillPlayer(client, client, "sc_reduction", .explode=true);
                        }
                    }
                    case 17: // Forced Buyout
                    {
                        new level = GetLevel(client, raceID);
                        if (level < 8 || GetRandomInt(1,100) > 9-negotiations_level)
                            TriggerTimer(timer); // Get a different Negotiation
                        else
                        {
                            new amount = GetRandomInt(100,200 * negotiations_level);
                            SetCrystals(client, GetCrystals(client)+amount,false);

                            new reduction = GetRandomInt(0,level) - (level/4);
                            if (reduction > 0)
                            {
                                DisplayMessage(client,Display_Deaths,"%t", "BuyoutReduction", amount, reduction);
                            }
                            else
                            {
                                DisplayMessage(client,Display_Deaths,"%t", "Buyout", amount);
                            }

                            new Float:location[3];
                            GetClientAbsOrigin(client,location);
                            TE_SetupExplosion(location, BigExplosion(), 10.0, 30, 0, 50, 20);
                            TE_SendEffectToAll();

                            PrepareSound(explodeWav);
                            EmitSoundToAll(explodeWav,client);
                            KillPlayer(client, client, "sc_buyout", .explode=true);

                            if (reduction > 0)
                                ResetLevel(client, raceID, level-reduction);
                        }
                    }
                    case 18: // Bankruptcy
                    {
                        new level = GetLevel(client, raceID);
                        if (level < 8 || GetRandomInt(1,100) > 9-negotiations_level)
                            TriggerTimer(timer); // Get a different Negotiation
                        else
                        {
                            new reduction = GetRandomInt(0,level) - (level/2);
                            if (reduction > 0)
                            {
                                DisplayMessage(client,Display_Deaths,"%t", "BankruptcyReduction", reduction);
                            }
                            else
                            {
                                DisplayMessage(client,Display_Deaths,"%t", "Bankruptcy");
                            }

                            new Float:location[3];
                            GetClientAbsOrigin(client,location);
                            TE_SetupExplosion(location, BigExplosion(), 10.0, 30, 0, 50, 20);
                            TE_SendEffectToAll();

                            PrepareSound(explodeWav);
                            EmitSoundToAll(explodeWav,client);
                            KillPlayer(client, client, "sc_bankruptcy", .explode=true);

                            if (reduction > 0)
                                ResetLevel(client, raceID, level-reduction);
                        }
                    }
                    case 19: // Union Dues
                    {
                        new balance = GetCrystals(client);
                        if (balance > 0)
                        {
                            new amount = GetRandomInt(1,balance-(negotiations_level*2));
                            SetCrystals(client, balance-amount,false);
                            DisplayMessage(client,Display_Misc_Message,
                                           "%t", "UnionDues", amount);
                        }
                    }
                    case 20: // OSHA
                    {
                        DisplayMessage(client,Display_Deaths, "%t", "OSHA");

                        new Float:location[3];
                        GetClientAbsOrigin(client,location);
                        TE_SetupExplosion(location, BigExplosion(), 10.0, 30, 0, 50, 20);
                        TE_SendEffectToAll();

                        PrepareSound(explodeWav);
                        EmitSoundToAll(explodeWav,client);
                        KillPlayer(client, client, "sc_osha", .explode=true);
                    }
                }
            }
        }
    }
    return Plugin_Continue;
}

AddCrystals(client, amount, const String:reason[])
{
    SetCrystals(client, GetCrystals(client)+amount, false);
    DisplayMessage(client,Display_Misc_Message, "%t", reason, amount);
}

