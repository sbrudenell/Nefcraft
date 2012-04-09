/**
 * vim: set ai et ts=4 sw=4 syntax=sourcepawn :
 * File: events.inc
 * Description: The handler for all the source engine events.
 * Author(s): Naris (Murray Wilson)
 * Credits: Anthony Iacono  
 */

#include <particle>
#include <tf2_uber>

bool:HookEvents()
{
    if (!HookEventEx("player_spawn",PlayerSpawnEvent))
    {
        LogError("Could not hook the player_spawn event.");
        return false;
    }

    if (!HookEventEx("player_hurt",PlayerHurtPreEvent, EventHookMode_Pre))
    {
        LogError("Could not hook the player_hurt pre-event.");
        return false;
    }

    if (!HookEventEx("player_hurt",PlayerHurtEvent))
    {
        LogError("Could not hook the player_hurt event.");
        return false;
    }

    if (!HookEventEx("player_death",CorrectDeathEvent, EventHookMode_Pre))
    {
        LogError("Could not hook the player_death pre event.");
        return false;
    }

    if (!HookEventEx("player_death",PlayerDeathEvent))
    {
        LogError("Could not hook the player_death event.");
        return false;
    }

    return true;
}

public PlayerSpawnEvent(Handle:event,const String:name[],bool:dontBroadcast)
{
    new client = GetClientOfUserId(GetEventInt(event,"userid"));
    if (client > 0)
    {
        TraceInto("Events", "PlayerSpawnEvent", "client=%d:%L", \
                  client, ValidClientIndex(client));

        PendingCheck(client);

        new race=GetRace(client);
        new Handle:raceHandle = GetRaceHandle(race);

        if (IsFakeClient(client))
        {
            // Level up the bot?
            new max = GetUpgradeLevelCount(client,race);
            new level = GetLevel(client,race);
            if (level < max)
            {
                new count = GetUpgradeCount(raceHandle)-1;
                if (count >= 0 )
                {
                    // Count the current upgrade levels
                    for(new upgrade=0;upgrade<count;upgrade++)
                    {
                        level -= GetUpgradeLevel(client, race, upgrade);
                    }

                    // And pick which ones to, err, upgrade (if any)...
                    while (level > 0)
                    {
                        new upgrade = GetRandomInt(0, count);
                        new ulevel = GetUpgradeLevel(client, race, upgrade);
                        new maxLevel = GetUpgradeMaxLevel(raceHandle,upgrade);
                        if (ulevel < maxLevel)
                        {
                            SetUpgradeLevel(client, race, upgrade, ++ulevel);
                        }
                        level--;
                    }
                }
            }
        }
        else
        {
            if (m_FirstSpawn[client])
            {
                CPrintToChat(client, "%t", "Welcome");
                CPrintToChat(client, "%t", "ReportBugs", g_BugURL);

                // Only show them the Intro if they
                // haven't played before
                // (Not found in DB)
                if (GetDatabaseLoaded(client) == DataNotFound)
                    IntroMenu(client);
                else
                    m_FirstSpawn[client]=false;
            }
            else if (GetDatabaseLoaded(client) != DataLoading &&
                     GetUpgradeLevelCount(client,race) < GetLevel(client,race))
            {
                UpgradeMenu(client, UpgradeMode);
            }
        }

        new initial = GetInitialEnergy(client);

        Trace("%d:%N's initial energy is %d", \
              client, ValidClientIndex(client), initial);

        if (raceHandle && initial < 0)
        {
            initial = GetRaceInitialEnergy(raceHandle);

            #if defined _TRACE
                decl String:rname[64]; GetRaceName(GetRaceHandle(race),rname,sizeof(rname));
                Trace("Race %d:%s's initial energy is %d", race, rname, initial);
            #endif
        }

        if (initial < 0)
            initial = 0;

        SetEnergy(client, initial);

        Trace("Setting %d:%N's energy to %d", \
              client, ValidClientIndex(client), initial);

        if (GetDatabaseLoaded(client) >= DataOK)
        {
            ShowXP(client,false);
            ShowCrystals(client,false);
            ShowVespene(client,false);
            ShowEnergy(client,false);
        }

        #if defined _TRACE
            if (GameType == tf2)
            {
                new hp = m_BaseHealth[client] = m_SpawnHealth[client] = GetClientHealth(client);
                new Float:speed = m_BaseSpeed[client] = m_SpawnSpeed[client] = GetEntPropFloat(client, Prop_Send, "m_flMaxspeed");
                TraceCat("Speed", "%d:%N spawned with %d hp and speed=%1.0f", client, ValidClientIndex(client), hp, speed);
            }
        #endif

        g_InjuredBy[client] = 0;
        g_KilledBy[client] = 0;
        g_Killed[client] = false;
        g_Dead[client] = false;

        if (m_IsChangingClass[client])
            m_IsChangingClass[client]=false;
        else
            CooldownEvent(client, Cooldown_ResetOnSpawn);

        CooldownSpawn(client);

        new Action:result;
        Call_StartForward(g_OnPlayerSpawnEventHandle);
        Call_PushCell(event);
        Call_PushCell(client);
        Call_PushCell(race);
        Call_Finish(result);

        CreateHUDTimer(client);
        CreatePropertyTimer(client);

        #if defined TRACK_DAMAGE
            CreateHealthTimer(client);
        #endif

        TraceReturn();
    }
}

public Action:OnWeaponSwitch(client, weapon)
{
    #if defined _TRACE
        decl String:sWeapon[32];
        GetEdictClassname(weapon, sWeapon, sizeof(sWeapon));

        TraceInto("Events", "OnWeaponSwitch", "client=%d:%L, weapon=%d:%s", \
                  client, ValidClientIndex(client), weapon, sWeapon);

        new Float:speed = m_BaseSpeed[client] = GetEntPropFloat(client, Prop_Send, "m_flMaxspeed");
        TraceCat("Speed", "%d:%N's speed=%1.0f", client, ValidClientIndex(client), speed);
    #endif

    UpdatePlayerProperties(client);

    #if defined _TRACE
        TraceReturn();                  
    #endif

    return Plugin_Continue;
}

public Action:OnTakeDamage(victim,&attacker,&inflictor,&Float:damage,&damagetype)
{
    TraceInto("Events", "OnTakeDamage", "victim=%d:%L, attacker=%d:%L, inflictor=%d, damage=%f, damagetype=%d", \
              victim, ValidClientIndex(victim), attacker, ValidClientIndex(attacker), inflictor, damage, damagetype);

    static bool:inTakeDamage = false;

    if (victim > 0 && victim <= MaxClients)
    {
        if (attacker > 0 && attacker <= MaxClients && attacker != victim &&
            GetImmunity(victim, Immunity_RangedAttacks) &&
            !IsInRange(attacker,victim,300.0))
        {
            Trace("%N was protected by Immunity to Ranged Attacks in OnTakeDamage, damage was %f", victim, damage);

            DisplayMessage(victim, Display_Defense, "%t", "YouWereProtected", attacker);
            DisplayMessage(attacker, Display_Enemy_Defended, "%t", "TargetWasProtected", victim);

            if (GameType == tf2)
            {
                decl Float:pos[3];
                GetClientEyePosition(victim, pos);
                pos[2] += 4.0;
                TE_SetupParticle("miss_text", pos);
                TE_SendToClient(attacker);
            }

            TraceReturn("return=Plugin_Handled");
            return Plugin_Handled;
        }

        new Action:result = Plugin_Continue;
        if (g_bSupressOnPlayerTakeDamage)
        {
            Trace("Supressing TakeDamage Forward: %d:%N was damaged by %d:%N for %f", \
                  victim, ValidClientIndex(victim), attacker, ValidClientIndex(attacker), \
                  damage);

            g_bSupressOnPlayerTakeDamage = false;
        }
        else if (!inTakeDamage)
        {
            Trace("OnPlayerTakeDamage: %d:%N was damaged by %d:%N for %f", \
                  victim, ValidClientIndex(victim), \
                  attacker, ValidClientIndex(attacker), \
                  damage);

            inTakeDamage = true;
            Call_StartForward(g_OnPlayerTakeDamageHandle);
            Call_PushCell(victim);
            Call_PushCellRef(attacker);
            Call_PushCellRef(inflictor);
            Call_PushFloatRef(damage);
            Call_PushCellRef(damagetype);
            Call_Finish(result);
            inTakeDamage = false;
        }

        TraceReturn("result=%d, attacker=%d:%L, inflictor=%d, damage=%f, damagetype=%d", \
                    result, attacker, ValidClientIndex(attacker), inflictor, damage, damagetype);

        return result;
    }
    else
    {
        TraceReturn("return=Plugin_Continue");
        return Plugin_Continue;
    }
}

public Action:PlayerHurtPreEvent(Handle:event,const String:name[],bool:dontBroadcast)
{
    new victim_index = GetClientOfUserId(GetEventInt(event,"userid"));
    if (victim_index > 0)
    {
        TraceInto("Events", "PlayerHurtPreEvent", "victim_index=%d:%L", \
                  victim_index, ValidClientIndex(victim_index));

        // Don't damage dead people!
        if (g_Killed[victim_index] || g_Dead[victim_index])
        {
            TraceReturn("return=Plugin_Handled");
            return Plugin_Handled;
        }

        new attacker_index = GetClientOfUserId(GetEventInt(event,"attacker"));
        if (attacker_index > 0 && attacker_index != victim_index &&
            GetImmunity(victim_index, Immunity_RangedAttacks) &&
            !IsInRange(attacker_index,victim_index,300.0))
        {
            #if defined _TRACE
                new health = GetClientHealth(victim_index);
                new damage = GetDamage(event, victim_index);
                Trace("%N was protected by Immunity to Ranged Attacks in PlayerHurtPreEvent, health was left at %d, damage was %d", victim_index, health, damage);
            #endif

            DisplayMessage(victim_index, Display_Defense, "%t", "YouWereProtected", attacker_index);
            DisplayMessage(attacker_index, Display_Enemy_Defended, "%t", "TargetWasProtected", victim_index);

            new assister_index = GetClientOfUserId(GetEventInt(event,"assister"));
            if (assister_index > 0)
            {
                DisplayMessage(assister_index, Display_Enemy_Defended, "%t",
                               "TargetWasProtected", victim_index);
            }

            TraceReturn("return=Plugin_Handled");
            return Plugin_Handled;
        }
    }

    return Plugin_Continue;
}

public Action:PlayerHurtEvent(Handle:event,const String:name[],bool:dontBroadcast)
{
    new Action:result = Plugin_Continue;
    new victim_index = GetClientOfUserId(GetEventInt(event,"userid"));
    if (victim_index > 0)
    {
        new damage = GetDamage(event, victim_index);
        new attacker_index = GetClientOfUserId(GetEventInt(event,"attacker"));

        TraceInto("Events", "PlayerHurtEvent", "victim_index=%d:%L, attacker_index=%d:%L", \
                  victim_index, ValidClientIndex(victim_index), attacker_index, ValidClientIndex(attacker_index));

        new injured_by = g_InjuredBy[victim_index];
        if (injured_by == attacker_index)
            g_InjuredBy[victim_index] = 0;

        if ((attacker_index > 0 && attacker_index != victim_index &&
             GetImmunity(victim_index, Immunity_RangedAttacks) &&
             !IsInRange(attacker_index,victim_index,300.0)))
        {
            new health = GetClientHealth(victim_index) + damage;
            SetEntityHealth(victim_index, health);
            SetEventInt(event, "health", health);
            result = Plugin_Changed;

            #if defined TRACK_DAMAGE
                SetSavedHealth(victim_index, health);
            #endif

            Trace("%N was protected by Immunity to Ranged Attacks in PlayerHurtEvent, health was restored to %d, damage was %d", victim_index, health, damage);

            if (attacker_index && attacker_index != victim_index)
            {
                DisplayMessage(victim_index, Display_Defense, "%t", "YouWereProtected", attacker_index);
                DisplayMessage(attacker_index, Display_Enemy_Defended, "%t", "TargetWasProtected", victim_index);
            }
            else
            {
                DisplayMessage(victim_index, Display_Defense, "%t", "ProtectedFromDamage");
            }
        }
        else
        {
            new amount = 0;
            new absorbed = GetEventInt(event, "absorbed");
            new armor = GetArmor(victim_index);
            if (armor > 0)
            {
                new all_damage = damage+absorbed;
                amount = RoundFloat(float(all_damage)
                         * GetRandomFloat(m_ArmorPercent[victim_index][0],
                                          m_ArmorPercent[victim_index][1]));

                if (amount > 0)
                {
                    if (amount > armor)
                        amount = armor;

                    if (absorbed < amount)
                    {
                        new delta = amount-absorbed;
                        new newhp = GetClientHealth(victim_index)+delta;
                        new maxhp = GetMaxHealth(victim_index);
                        if (newhp > maxhp)
                            newhp = maxhp;

                        damage -= delta;
                        absorbed += delta;
                        m_Armor[victim_index] -= delta;

                        SetEntityHealth(victim_index,newhp);
                    }

                    result = Plugin_Changed;

                    TraceCat("Armor", "%d:%N's armor absorbed %d/%d of %d/%d, burrowed=%d, m_ArmorPercent=%1.2f,%1.2f, m_ArmorName='%s'", \
                             victim_index,ValidClientIndex(victim_index), amount, absorbed, all_damage, damage, \
                             GetAttribute(victim_index, Attribute_IsBurrowed), \
                             m_ArmorPercent[victim_index][0], \
                             m_ArmorPercent[victim_index][1], \
                             m_ArmorName[victim_index]);

                    DisplayMessage(victim_index, Display_Defense, "%t", "ArmorAbsorbed",
                                   m_ArmorName[victim_index], (amount > absorbed) ? amount : absorbed);
                }
            }

            new attacker_race, assister_index, assister_race;
            new victim_race = GetRace(victim_index);

            if (attacker_index > 0)
            {
                attacker_race = GetRace(attacker_index);

                if (GameType == tf2)
                {
                    // Check for an "assister" (Medic ubering/criting/healing)
                    new pcond = TF2_GetPlayerConditionFlags(attacker_index);
                    new numHealers = TF2_GetNumHealers(attacker_index);

                    Trace("numHealers=%d, pcond=%08x", numHealers, pcond);

                    if (numHealers > 0 || TF2_IsUbercharged(pcond) ||
                        TF2_IsKritzkrieged(pcond) || TF2_IsHealing(pcond))
                    {
                        for(new index=1;index<=MaxClients;index++)
                        {
                            if (IsClientInGame(index))
                            {
                                if (TF2_GetHealingTarget(index) == attacker_index)
                                {
                                    assister_index = index;
                                    assister_race = GetRace(index);

                                    Trace("assister_index=%d:%L", \
                                          assister_index, ValidClientIndex(assister_index));
                                    break;
                                }
                            }
                        }
                    }
                }
            }


            new bool:from_sc = (injured_by == attacker_index) ||
                               GetEventBool(event, "sourcecraft");

            Call_StartForward(g_OnPlayerHurtEventHandle);
            Call_PushCell(event);
            Call_PushCell(victim_index);
            Call_PushCell(victim_race);
            Call_PushCell(attacker_index);
            Call_PushCell(attacker_race);
            Call_PushCell(assister_index);
            Call_PushCell(assister_race);
            Call_PushCell(damage);
            Call_PushCell(absorbed);
            Call_PushCell(from_sc);
            Call_Finish(result);

            if (amount > 0)
                result = Plugin_Changed;

            #if defined TRACK_DAMAGE
                if (result >= Plugin_Changed)
                {
                    if (attacker_index)
                        SaveHealth(attacker_index);

                    if (assister_index)
                        SaveHealth(assister_index);
                }

                new health = SaveHealth(victim_index);
                if (result == Plugin_Changed)
                    SetEventInt(event, "health", health);
            #else
                if (result == Plugin_Changed)
                    SetEventInt(event, "health", GetClientHealth(victim_index));
            #endif
        }

        TraceReturn("result=%d", result);
    }

    g_bDamageFromPlayerHurt = false;
    return result;
}

public PlayerDeathEvent(Handle:event,const String:name[],bool:dontBroadcast)
{
    new victim_index = GetClientOfUserId(GetEventInt(event,"userid"));
    if (victim_index > 0 && !g_Dead[victim_index])
    {
        new customkill;
        new deathFlags;
        new bool:backstab;
        new bool:headshot;

        g_InjuredBy[victim_index] = 0;
        g_KilledBy[victim_index] = 0;

        if (GameType == cstrike)
        {
            backstab   = false;
            headshot   = GetEventBool(event,"headshot");
            customkill = headshot ? TF_CUSTOM_HEADSHOT : 0;
            deathFlags = 0;
        }
        else if (GameType == dod)
        {
            backstab   = false;
            headshot   = GetEventBool(event,"headshot");
            customkill = headshot ? TF_CUSTOM_HEADSHOT : 0;
            deathFlags = 0;
        }
        else if (GameType == tf2)
        {
            deathFlags = GetEventInt(event, "death_flags");
            if (deathFlags & TF_DEATHFLAG_DEADRINGER)
            {
                g_Dead[victim_index] = false;
                g_Killed[victim_index] = false;
                return;			//skip dead ringer / FakeDeath
            }
            else
            {
                customkill = GetEventInt(event,"customkill");
                if (customkill != TF_CUSTOM_FISH_KILL &&
                    GetEventInt(event, "weaponid") == TF_WEAPON_BAT_FISH)
                {
                    g_Dead[victim_index] = false;
                    g_Killed[victim_index] = false;
                    return; // skip fishy kill
                }
                else
                {
                    headshot = (customkill == TF_CUSTOM_HEADSHOT);
                    backstab = (customkill == TF_CUSTOM_BACKSTAB);
                }
            }
        }

        new victim_race=(victim_index > 0) ? GetRace(victim_index) : -1;

        new attacker_index=GetClientOfUserId(GetEventInt(event,"attacker"));
        new attacker_race=(attacker_index > 0) ? GetRace(attacker_index) : -1;

        new assister_index=GetClientOfUserId(GetEventInt(event,"assister"));
        new assister_race=(assister_index > 0) ? GetRace(assister_index) : -1;

        decl String:weapon[64];
        new bool:is_equipment = GetWeapon(event, attacker_index,
                                          weapon, sizeof(weapon));

        KillHUDTimer(victim_index);
        KillPropertyTimer(victim_index);

        #if defined TRACK_DAMAGE
            KillHealthTimer(victim_index);
        #endif

        ResetInvulnerability(victim_index);
        SetImmunity(victim_index,Immunity_RangedAttacks,false);

        // Clear HUD Message
        m_HudMessage[victim_index][0] = '\0';
        ClearSyncHud(victim_index, m_HudSynch[3]);

        new damage     = GetDamage(event, victim_index);
        new bool:melee = backstab || IsMelee(weapon, is_equipment,
                                             attacker_index,
                                             victim_index);

        new Action:result;
        Call_StartForward(g_OnPlayerDeathEventHandle);
        Call_PushCell(event);
        Call_PushCell(victim_index);
        Call_PushCell(victim_race);
        Call_PushCell(attacker_index);
        Call_PushCell(attacker_race);
        Call_PushCell(assister_index);
        Call_PushCell(assister_race);
        Call_PushCell(damage);
        Call_PushString(weapon);
        Call_PushCell(is_equipment);
        Call_PushCell(customkill);
        Call_PushCell(headshot);
        Call_PushCell(backstab);
        Call_PushCell(melee);
        Call_Finish(result);

        if (attacker_index != victim_index)
        {
            if (attacker_index > 0)
            {
                if (GetClientTeam(attacker_index) != GetClientTeam(victim_index))
                {
                    GiveKillCrystals(attacker_index);
                    GiveKillXP(attacker_index, victim_index,
                               headshot, backstab, melee,
                               ((deathFlags & TF_DEATHFLAG_KILLERDOMINATION) != 0),
                               ((deathFlags & TF_DEATHFLAG_KILLERREVENGE) != 0),
                               false);
                }
            }

            #if defined TRACK_DAMAGE
                SaveHealth(attacker_index);
            #endif

            if (assister_index > 0)
            {
                if (GetClientTeam(assister_index) != GetClientTeam(victim_index))
                {
                    GiveKillCrystals(assister_index);
                    GiveKillXP(assister_index, victim_index,
                               headshot, backstab, melee,
                               ((deathFlags & TF_DEATHFLAG_ASSISTERDOMINATION) != 0),
                               ((deathFlags & TF_DEATHFLAG_ASSISTERREVENGE) != 0),
                               true);
                }
            }

            #if defined TRACK_DAMAGE
                SaveHealth(assister_index);
            #endif
        }

        if (victim_index > 0)
        {
            new initial = GetInitialEnergy(victim_index);
            if (initial < 0 && victim_race > 0)
                initial = GetRaceInitialEnergy(GetRaceHandle(victim_race));

            // Accumulate energy and zero the players active energy.
            new energy = GetEnergy(victim_index) - initial;
            if (energy > 0 && victim_race > 0)
                IncrementAccumulatedEnergy(victim_index, victim_race, energy);

            SetEnergy(victim_index, 0);

            // Check for pending race change.
            new race;
            if (PendingCheck(victim_index) < 0)
            {
                race = GetRace(victim_index);
                new Handle:raceHandle = GetRaceHandle(race);
                new parentRace = GetRaceParentId(raceHandle);
                if (parentRace > 0)
                {
                    // Revert back to Parent race after death as a Summoned race.
                    // (Set pending race to the parent)
                    ChangeRace(victim_index, parentRace, GetRaceHandle(parentRace), false, false);
                    race = 0; // and skip upgrade check below.
                }
            }
            else
                race=GetRace(victim_index);

            if (race > 0 && GetDatabaseLoaded(victim_index) != DataLoading &&
                GetUpgradeLevelCount(victim_index,race) < GetLevel(victim_index,race))
            {
                UpgradeMenu(victim_index, UpgradeMode);
            }
        }

        #if defined TRACK_DAMAGE
            SaveHealth(victim_index);
        #endif

        g_Dead[victim_index] = true;

        CooldownEvent(victim_index, Cooldown_ResetOnDeath);

        if (IsFakeClient(victim_index))
        {
            // Should the Bot buy a mole?
            new crystals = GetCrystals(victim_index);
            if (GetRandomInt(0,100) >= ((crystals >= 100) ? 10 : 50))
            {
                new mole = FindShopItem("mole");
                if (mole >= 0)
                {
                    new Handle:moleHandle = GetShopItemHandle(mole);
                    if (moleHandle && crystals >= GetItemCrystals(moleHandle))
                        PurchaseShopItem(victim_index, mole);
                }
            }
        }
    }

    g_bDamageFromPlayerHurt = false;
}

public Action:CorrectDeathEvent(Handle:event,const String:name[],bool:dontBroadcast)
{
    new victim_index=GetClientOfUserId(GetEventInt(event,"userid"));
    if (victim_index > 0)
    {
        // Don't allow double kills/deaths!
        if (g_Dead[victim_index])
            return Plugin_Handled;
        else
        {
            new attacker_index=GetClientOfUserId(GetEventInt(event,"attacker"));
            new killed_by = g_KilledBy[victim_index];
            if (killed_by != attacker_index && IsValidClient(killed_by) &&
                !GetEventBool(event,"fake"))
            {
                SetEventInt(event,"attacker",GetClientUserId(killed_by));
                g_KilledBy[victim_index] = 0;
                return Plugin_Changed;
            }
        }
    }
    return Plugin_Continue;
}

public PlayerChangeClassEvent(Handle:event,const String:name[],bool:dontBroadcast)
{
    new userid = GetEventInt(event,"userid");
    if (userid > 0)
    {
        new index = GetClientOfUserId(userid);
        if (IsValidClient(index))
        {
            TraceInto("Events", "PlayerChangeClassEvent", "index=%d:%L", \
                      index, ValidClientIndex(index));

            if (IsPlayerAlive(index))
                m_IsChangingClass[index] = true;

            // Reset energy when changing class
            new initial = GetInitialEnergy(index);

            Trace("%d:%N's initial energy is %d", \
                  index, ValidClientIndex(index), initial);

            new race = GetRace(index);
            if (race > 0)
            {
                new Handle:raceHandle = GetRaceHandle(race);
                if (raceHandle && initial < 0)
                {
                    initial = GetRaceInitialEnergy(raceHandle);

                    #if defined _TRACE
                        decl String:rname[64]; GetRaceName(GetRaceHandle(race),rname,sizeof(rname));
                        Trace("Race %d:%s's initial energy is %d", race, rname, initial);
                    #endif
                }
            }

            if (initial < 0)
                initial = 0;

            SetEnergy(index,initial);

            TraceReturn("Setting %d:%N's energy to %d", \
                        index, ValidClientIndex(index), \
                        initial);
        }
    }
}

public RoundStart(Handle:event,const String:name[],bool:dontBroadcast)
{
    g_RoundState = RoundStarted;
    g_iRoundNumber++;

    if (GameType == cstrike)
    {
        new Handle:freezeTimeCvar=FindConVar("mp_freezetime");
        if (freezeTimeCvar)
        {
            new Float:fFreezeTime=GetConVarFloat(freezeTimeCvar);
            if (fFreezeTime>0.0)
            {
                CreateTimer(fFreezeTime,EndFreezeTime,g_iRoundNumber);
                g_RoundState = RoundFreeze;
            }
        }
    }

    #if defined _TRACE
        LogRoundEvent(event,name,dontBroadcast);
    #endif

    for(new index=1;index<=MaxClients;index++)
    {
        if (IsClientInGame(index))
            ResetInvulnerability(index);
    }
}

public Action:EndFreezeTime(Handle:timer,any:roundNumber)
{
	if (roundNumber==g_iRoundNumber)
        g_RoundState = RoundActive;
}

public EventRoundActive(Handle:event,const String:name[],bool:dontBroadcast)
{
    g_RoundState = RoundActive;

    #if defined _TRACE
        LogRoundEvent(event,name,dontBroadcast);
    #endif

    for(new index=1;index<=MaxClients;index++)
    {
        if (IsClientInGame(index))
            ResetInvulnerability(index);
    }
}

public EventRoundSuddenDeath(Handle:event,const String:name[],bool:dontBroadcast)
{
    g_RoundState = RoundSuddenDeath;

    #if defined _TRACE
        LogRoundEvent(event,name,dontBroadcast);
    #endif

    for(new index=1;index<=MaxClients;index++)
    {
        m_IsChangingClass[index]=false;
        if (IsClientInGame(index))
            ResetInvulnerability(index);
    }
}

public EventRoundOver(Handle:event,const String:name[],bool:dontBroadcast)
{
    g_RoundState = RoundOver;

    #if defined _TRACE
        LogRoundEvent(event,name,dontBroadcast);
    #endif

    for(new index=1;index<=MaxClients;index++)
    {
        m_IsChangingClass[index]=false;
        if (IsClientInGame(index))
            ResetInvulnerability(index);
    }
}

public EventRoundWin(Handle:event,const String:name[],bool:dontBroadcast)
{
    g_RoundState = RoundOver;

    #if defined _TRACE
        LogRoundEvent(event,name,dontBroadcast);
    #endif

    new team  = GetEventInt(event,"team");
    for(new index=1;index<=MaxClients;index++)
    {
        m_IsChangingClass[index]=false;
        if (IsClientInGame(index))
        {
            ResetInvulnerability(index);
            if (GetClientTeam(index) == team)
            {
                GiveRoundWinCrystalsAndGas(index);
                GiveRoundWinXP(index);
            }
        }
    }

    //if (SAVE_ENABLED)
    //    SaveAllPlayersData();
}

public EventGameOver(Handle:event,const String:name[],bool:dontBroadcast)
{
    g_RoundState = GameOver;
    g_MapChanging = true;

    #if defined _TRACE
        LogRoundEvent(event,name,dontBroadcast);
    #endif
}

public BombPlantedEvent(Handle:event,const String:name[],bool:dontBroadcast)
{
    new index = GetClientOfUserId(GetEventInt(event,"userid"));
    if (index > 0)
    {
        GivePlantXP(index);
        GivePlantCrystalsAndGas(index);
    }
}

public BombDefusedEvent(Handle:event,const String:name[],bool:dontBroadcast)
{
    new index = GetClientOfUserId(GetEventInt(event,"userid"));
    if (index > 0)
    {
        GiveDefuseXP(index);
        GiveDefuseCrystalsAndGas(index);
    }
}

public BombExplodedEvent(Handle:event,const String:name[],bool:dontBroadcast)
{
    new index = GetClientOfUserId(GetEventInt(event,"userid"));
    if (index > 0)
    {
        GiveExplodeXP(index);
        GiveExplodeCrystalsAndGas(index);
    }
}

#if defined _TRACE
    public LogEvent(Handle:event,const String:name[],bool:dontBroadcast)
    {
        SetTraceCategory("Event");
        TraceInto("Events_TF2", "LogEvent");

        if (event == INVALID_HANDLE)
            TraceError("event=%s", name);
        else
        {
            new userid=GetEventInt(event,"userid");
            new index=GetClientOfUserId(userid);
            new player=GetEventInt(event,"player");
            new pindex=GetClientOfUserId(player);
            new team=GetEventInt(event,"team");
            new amount=GetEventInt(event,"amount");
            new type=GetEventInt(event,"type");
            new short=GetEventInt(event,"short");
            new medic=GetEventInt(event,"medic");
            new ownerid=GetEventInt(event,"ownerid");
            new weaponid=GetEventInt(event,"weaponid");
            new buff_type=GetEventInt(event,"buff_type");
            new buff_owner =GetEventInt(event,"buff_owner");
            Trace("event=%s, userid=%d(client=%d:%N), player=%d(client=%d:%N), team=%d, amount=%d, type=%d, short=%d, medic=%d:%N, ownerid=%d:%N, weaponid=%d,buff_type=%d, buff_owner=%d:%N", \
                  name, userid, index, ValidClientIndex(index), player, pindex, ValidClientIndex(pindex), \
                  team, amount, type, short, medic, ValidClientIndex(medic), ownerid, ValidClientIndex(ownerid), \
                  weaponid, buff_type, buff_owner, ValidClientIndex(buff_owner));
        }

        TraceReturn();
    }

    public LogRoundEvent(Handle:event,const String:name[],bool:dontBroadcast)
    {
        SetTraceCategory("Event");
        TraceInto("Events_TF2", "LogRoundEvent");

        if (event == INVALID_HANDLE)
            TraceError("event=%s, RoundState=%d(%s)", name, g_RoundState, RoundStateNames[g_RoundState]);
        else
        {
            new team=GetEventInt(event,"team");
            new amount=GetEventInt(event,"amount");
            new type=GetEventInt(event,"type");
            new short=GetEventInt(event,"short");
            Trace("event=%s, RoundState=%d(%s), team=%d, amount=%d, type=%d, short=%d", \
                  name, g_RoundState, RoundStateNames[g_RoundState], team, amount, type, short);
        }

        TraceReturn();
    }

    public LogBuildingEvent(Handle:event,const String:name[],bool:dontBroadcast)
    {
        SetTraceCategory("Event");
        TraceInto("Events_TF2", "LogBuildingEvent");

        if (event == INVALID_HANDLE)
            TraceError("event=%s", name);
        else
        {
            Trace("event=%s, building_type=%d, object_mode=%d, remove=%d, object=%d, index=%d", name, \
                  GetEventInt(event,"building_type"), GetEventInt(event,"object_mode"), \
                  GetEventInt(event,"remove"), GetEventInt(event,"object"), \
                  GetEventInt(event,"index"));
        }

        TraceReturn();
    }
#endif
