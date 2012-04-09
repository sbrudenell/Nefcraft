/**
 * vim: set ai et ts=4 sw=4 syntax=sourcepawn :
 * File: events_tf2.inc
 * Description: The handler for all the Team Fortress event based stuff.
 * Author(s): Naris (Murray Wilson)
 */

#define MAXENTITIES 2048

new m_ObjectBuilt[MAXENTITIES+1];

bool:HookTFEvents()
{
    if (!HookEventEx("teamplay_round_start",RoundStart,EventHookMode_PostNoCopy))
    {
        LogError("Could not hook the teamplay_round_start event.");
        return false;
    }

    if (!HookEventEx("teamplay_round_active",EventRoundActive,EventHookMode_PostNoCopy))
    {
        LogError("Could not hook the teamplay_round_active event.");
        return false;
    }

    if (!HookEventEx("teamplay_setup_finished",EventRoundSetupFinished,EventHookMode_PostNoCopy))
    {
        LogError("Could not hook the teamplay_setup_finished event.");
        return false;
    }

    if (!HookEventEx("teamplay_round_win",EventRoundWin))
    {
        LogError("Could not hook the teamplay_round_win event.");
        return false;
    }

    if (!HookEventEx("teamplay_round_stalemate",EventRoundSuddenDeath,EventHookMode_PostNoCopy))
    {
        LogError("Could not hook the teamplay_round_stalemate event.");
        return false;
    }

    if (!HookEventEx("teamplay_win_panel",EventScorePanel))
    {
        LogError("Could not hook the teamplay_win_panel event.");
        return false;
    }

    if (!HookEventEx("tf_game_over",EventGameOver,EventHookMode_PostNoCopy))
    {
        LogError("Could not hook the tf_game_over event.");
        return false;
    }

    if (!HookEventEx("teamplay_game_over",EventGameOver,EventHookMode_PostNoCopy))
    {
        LogError("Could not hook the teamplay_game_over event.");
        return false;
    }

    if (!HookEventEx("teamplay_broadcast_audio",EventBroadcast))
    {
        LogError("Could not hook the teamplay_broadcast_audio event.");
    }

    if (!HookEvent("teamplay_flag_event", EventFlagEvent))
    {
        LogError("Could not hook the teamplay_flag_event.");
        return false;
    }

    if (!HookEvent("teamplay_point_captured", EventPointCapture))
    {
        LogError("Could not hook the teampoint_point_captured event.");
        return false;
    }

    if (!HookEvent("teamplay_point_startcapture", EventPointCapture))
    {
        LogError("Could not hook the teampoint_point_captured event.");
        return false;
    }

    if (!HookEvent("teamplay_capture_blocked", EventCaptureBlocked))
    {
        LogError("Could not hook the teadmplay_capture_blocked event.");
        return false;
    }

    if (!HookEventEx("player_escort_score",EventEscortScore))
    {
        LogError("Could not hook the player_escort_score event.");
        return false;
    }

    if (!HookEvent("player_changeclass",PlayerChangeClassEvent))
    {
        LogError("Couldn't hook the player_changeclass event.");
        return false;
    }

    if (!HookEvent("player_team",PlayerChangeClassEvent))
    {
        LogError("Couldn't hook the player_team event.");
        return false;
    }

    if (!HookEvent("teamplay_teambalanced_player",PlayerChangeClassEvent))
    {
        LogError("Couldn't hook the teamplay_teambalanced_player event.");
        return false;
    }

    if (!HookEvent("player_chargedeployed", PlayerChargeDeployed))
    {
        LogError("Could not hook the player_chargedeployed event.");
        return false;
    }

    if (!HookEvent("player_builtobject", PlayerBuiltObject))
    {
        LogError("Could not hook the player_builtobject event.");
        return false;
    }

    if (!HookEvent("object_destroyed", EventObjectDestroyed))
    {
        LogError("Could not hook the object_destroyed event.");
        return false;
    }

    if (!HookEvent("object_removed", EventObjectRemoved))
    {
        LogError("Could not hook the object_removed event.");
        return false;
    }

    if (!HookEvent("player_upgradedobject", EventObjectUpgraded))
    {
        LogError("Could not hook the player_upgradedobject event.");
        return false;
    }

    if (!HookEventEx("arena_round_start",EventRoundActive,EventHookMode_PostNoCopy))
    {
        LogError("Could not hook the arena_round_start event.");
        return false;
    }

    if (!HookEventEx("arena_win_panel",EventScorePanel))
    {
        LogError("Could not hook the arena_win_panel event.");
        return false;
    }

    #if defined _TRACE
        if (!HookEventEx("pumpkin_lord_summoned",LogEvent))
        {
            LogError("Could not hook the pumpkin_lord_summoned event.");
        }

        if (!HookEventEx("pumpkin_lord_killed",LogEvent))
        {
            LogError("Could not hook the pumpkin_lord_killed event.");
        }

        if (!HookEventEx("fish_notice",LogEvent))
        {
            LogError("Could not hook the fish_notice event.");
        }

        if (!HookEventEx("duel_status",LogEvent))
        {
            LogError("Could not hook the duel_status event.");
        }

        if (!HookEventEx("player_damaged",LogEvent))
        {
            LogError("Could not hook the player_damaged event.");
        }

        if (!HookEventEx("player_stunned",LogEvent))
        {
            LogError("Could not hook the player_stunned event.");
        }

        if (!HookEventEx("player_jarated",LogEvent))
        {
            LogError("Could not hook the player_stunned event.");
        }

        if (!HookEventEx("player_sapped_object",LogEvent))
        {
            LogError("Could not hook the player_stunned event.");
        }

        if (!HookEventEx("achievement_earned",LogEvent))
        {
            LogError("Could not hook the achievement_earned event.");
        }

        if (!HookEventEx("item_found",LogEvent))
        {
            LogError("Could not hook the item_found event.");
        }

        if (!HookEventEx("escort_speed",LogEvent))
        {
            LogError("Could not hook the escort_speed event.");
        }

        if (!HookEventEx("escort_recede",LogEvent))
        {
            LogError("Could not hook the escort_recede event.");
        }

        if (!HookEventEx("escort_progress",LogEvent))
        {
            LogError("Could not hook the escort_progress event.");
        }

        if (!HookEventEx("building_info_changed",LogEvent))
        {
            LogError("Could not hook the building_info_changed event.");
        }

        if (!HookEventEx("teamplay_overtime_begin",LogEvent))
        {
            LogError("Could not hook the teamplay_overtime_begin event.");
        }

        if (!HookEventEx("teamplay_overtime_end",LogEvent))
        {
            LogError("Could not hook the teamplay_overtime_end event.");
        }

        if (!HookEventEx("teamplay_suddendeath_begin",LogEvent))
        {
            LogError("Could not hook the teamplay_suddendeath_begin event.");
        }

        if (!HookEventEx("teamplay_suddendeath_end",LogEvent))
        {
            LogError("Could not hook the teamplay_suddendeath_end event.");
        }

        if (!HookEventEx("teamplay_waiting_begins",LogEvent))
        {
            LogError("Could not hook the teamplay_waiting_begins event.");
        }

        if (!HookEventEx("player_invulned",LogEvent))
        {
            LogError("Could not hook the player_invulned event.");
        }

        if (!HookEventEx("player_ignited_inv",LogEvent))
        {
            LogError("Could not hook the player_ignited_inv event.");
        }

        if (!HookEventEx("player_ignited",LogEvent))
        {
            LogError("Could not hook the player_ignited event.");
        }

        if (!HookEventEx("player_extinguished",LogEvent))
        {
            LogError("Could not hook the player_extinguished event.");
        }

        if (!HookEventEx("player_teleported",LogEvent))
        {
            LogError("Could not hook the player_teleported event.");
        }

        if (!HookEventEx("player_calledformedic",LogEvent))
        {
            LogError("Could not hook the player_calledformedic event.");
        }

        if (!HookEventEx("player_healedmediccall",LogEvent))
        {
            LogError("Could not hook the player_healedmediccall event.");
        }

        if (!HookEventEx("player_invulned",LogEvent))
        {
            LogError("Could not hook the player_invulned event.");
        }

        if (!HookEventEx("client_loadout_changed",LogEvent))
        {
            LogError("Could not hook the client_loadout_changed event.");
        }

        if (!HookEventEx("ctf_flag_captured",LogEvent))
        {
            LogError("Could not hook the ctf_flag_captured event.");
        }

        if (!HookEventEx("teamplay_point_locked",LogEvent))
        {
            LogError("Could not hook the teamplay_point_locked event.");
        }

        if (!HookEventEx("teamplay_point_unlocked",LogEvent))
        {
            LogError("Could not hook the teamplay_point_unlocked event.");
        }

        if (!HookEventEx("teamplay_capture_broken",LogEvent))
        {
            LogError("Could not hook the teamplay_capture_broken event.");
        }

        if (!HookEventEx("teamplay_capture_blocked",LogEvent))
        {
            LogError("Could not hook the teamplay_capture_blocked event.");
        }

        if (!HookEventEx("air_dash",LogEvent))
        {
            LogError("Could not hook the air_dash event.");
        }

        if (!HookEventEx("player_damage_dodged",LogEvent))
        {
            LogError("Could not hook the player_damage_dodged event.");
        }

        if (!HookEventEx("player_stunned",LogEvent))
        {
            LogError("Could not hook the player_stunned event.");
        }

        if (!HookEventEx("inventory_updated",LogEvent))
        {
            LogError("Could not hook the inventory_updated event.");
        }

        if (!HookEventEx("arena_player_notification",LogEvent))
        {
            LogError("Could not hook the arena_player_notification event.");
        }

        if (!HookEventEx("arena_match_maxstreak",LogEvent))
        {
            LogError("Could not hook the arena_match_maxstreak event.");
        }

        if (!HookEventEx("deploy_buff_banner",LogEvent))
        {
            LogError("Could not hook the deploy_buff_banner event.");
        }

        if (!HookEventEx("player_buff",LogEvent))
        {
            LogError("Could not hook the player_buff event.");
        }

        if (!HookEventEx("item_found",LogEvent))
        {
            LogError("Could not hook the item_found event.");
        }

        if (!HookEventEx("player_pinned",LogEvent))
        {
            LogError("Could not hook the player_pinned event.");
        }

        if (!HookEventEx("halloween_pumpkin_grab",LogEvent))
        {
            LogError("Could not hook the halloween_pumpkin_grab  event.");
        }

        if (!HookEventEx("rocket_jump",LogEvent))
        {
            LogError("Could not hook the rocket_jump event.");
        }

        if (!HookEventEx("rocket_jump_landed",LogEvent))
        {
            LogError("Could not hook the rocket_jump_landed event.");
        }

        if (!HookEventEx("sticky_jump",LogEvent))
        {
            LogError("Could not hook the sticky_jump event.");
        }

        if (!HookEventEx("sticky_jump_landed",LogEvent))
        {
            LogError("Could not hook the sticky_jump_landed event.");
        }

        if (!HookEventEx("player_destroyed_pipebomb",LogEvent))
        {
            LogError("Could not hook the player_destroyed_pipebomb event.");
        }

        if (!HookEventEx("object_deflected",LogEvent))
        {
            LogError("Could not hook the object_deflected event.");
        }

        if (!HookEventEx("medic_defended",LogEvent))
        {
            LogError("Could not hook the medic_defended event.");
        }

        if (!HookEventEx("player_mvp",LogEvent))
        {
            LogError("Could not hook the player_mvp event.");
        }

        if (!HookEventEx("building_info_changed",LogBuildingEvent))
        {
            LogError("Could not hook the building_info_changed event.");
        }

        if (!HookEventEx("update_status_item",LogBuildingEvent))
        {
            LogError("Could not hook the update_status_item event.");
        }
    #endif

    HookEntityOutput("item_healthkit_small",  "OnPlayerTouch",    EntityOutput_HealthKit);
    HookEntityOutput("item_healthkit_medium", "OnPlayerTouch",    EntityOutput_HealthKit);
    HookEntityOutput("item_healthkit_full",   "OnPlayerTouch",    EntityOutput_HealthKit);

    HookEntityOutput("prop_dynamic",          "OnAnimationBegun", EntityOutput_OnAnimationBegun);

    AddGameLogHook(LogScanner);

    return true;
}

public PlayerChargeDeployed(Handle:event,const String:name[],bool:dontBroadcast)
{
    new userid = GetEventInt(event,"userid");
    if (userid > 0)
    {
        new index = GetClientOfUserId(userid);
        if (IsValidClient(index))
        {
            GiveChargeDeployedXP(index);
            GiveChargeDeployedCrystals(index);
        }
    }
}

public PlayerBuiltObject(Handle:event,const String:name[],bool:dontBroadcast)
{
    if (g_RoundState >= RoundActive && g_RoundState < RoundOver)
    {
        new userid = GetEventInt(event,"userid");
        if (userid > 0)
        {
            new index = GetClientOfUserId(userid);
            if (IsValidClient(index))
            {
                new object = GetEventInt(event,"index");
                new TFObjectType:type = TFObjectType:GetEventInt(event,"object");

                TraceInto("Events_TF2", "PlayerBuiltObject", "object=%d,type=%d,userid=%d,%D:%N", \
                          object, type, userid,index, ValidClientIndex(index));

                if (m_ObjectBuilt[object] != index)
                {
                    GiveObjectBuiltXP(index, type);
                    m_ObjectBuilt[object] = index;

                    new Action:result = Plugin_Continue;
                    Call_StartForward(g_OnPlayerBuiltObject);
                    Call_PushCell(event);
                    Call_PushCell(index);
                    Call_PushCell(object);
                    Call_PushCell(type);
                    Call_Finish(result);
                }

                TraceReturn();
            }
        }
    }
}

public EventObjectDestroyed(Handle:event,const String:name[],bool:dontBroadcast)
{
    new object = GetEventInt(event,"index");
    if (object > 0)
        m_ObjectBuilt[object] = 0;

    if (g_RoundState >= RoundActive && g_RoundState < RoundOver)
    {
        new userid = GetEventInt(event,"userid");
        new index  = GetClientOfUserId(userid);
        new TFObjectType:type = TFObjectType:GetEventInt(event,"objecttype");

        new attacker = GetEventInt(event,"attacker"); // user ID who killed
        new attacker_index = GetClientOfUserId(attacker);

        new assister = GetEventInt(event,"assister"); // user ID of assister
        new assister_index = GetClientOfUserId(assister);

        TraceInto("Events_TF2", "EventObjectDestroyed", "object=%d,type=%d,userid=%d,%D:%N, attacker=%d,%d:%N, assister=%d,%d:%N", \
                  object, type, userid,index, ValidClientIndex(index), \
                  attacker,attacker_index, ValidClientIndex(attacker_index), \
                  assister,assister_index, ValidClientIndex(assister_index));

        if (attacker > 0 && attacker != userid)
        {
            //new attacker_index = GetClientOfUserId(attacker);
            if (attacker_index > 0)
                GiveObjectDestroyedXP(attacker_index,type);
        }
        else
        {
            TakeObjectBuiltXP(index,type);
        }

        if (assister> 0 && assister != userid)
        {
            //new assister_index = GetClientOfUserId(assister);
            if (assister_index > 0)
                GiveObjectDestroyedXP(assister_index,type);
        }

        TraceReturn();
    }
}

public EventObjectRemoved(Handle:event,const String:name[],bool:dontBroadcast)
{
    new object = GetEventInt(event,"index");
    if (object > 0)
        m_ObjectBuilt[object] = 0;

    if (g_RoundState >= RoundActive && g_RoundState < RoundOver)
    {
        new userid = GetEventInt(event,"userid");
        new index  = GetClientOfUserId(userid);
        if (index > 0)
        {
            new TFObjectType:type = TFObjectType:GetEventInt(event,"objecttype");

            TraceInto("Events_TF2", "EventObjectRemoved", "object=%d,type=%d,userid=%d,%D:%N", \
                    object, type, userid,index, ValidClientIndex(index));

            TakeObjectBuiltXP(index,type);
        }

        TraceReturn();
    }
}

public EventObjectUpgraded(Handle:event,const String:name[],bool:dontBroadcast)
{
    if (g_RoundState >= RoundActive && g_RoundState < RoundOver)
    {
        new userid = GetEventInt(event,"userid");
        new index  = GetClientOfUserId(userid);
        new TFObjectType:type = TFObjectType:GetEventInt(event,"object");
        new bool:isBuilder = GetEventBool(event,"isbuilder");

        #if defined _TRACE
            new object = GetEventInt(event,"index");
            TraceInto("Events_TF2", "EventObjectUpgraded", "object=%d,type=%d,userid=%d,%D:%N",
                      object, type, userid,index, ValidClientIndex(index));
        #endif

        GiveObjectUpgradedXP(index, type, isBuilder);

        #if defined _TRACE
            TraceReturn();
        #endif
    }
}

public EventFlagEvent(Handle:event,const String:name[],bool:dontBroadcast)
{
    new player = GetEventInt(event,"player");

    #if defined _TRACE
        new index = GetClientOfUserId(player); // Is player a client or a userid?
        TraceInto("Events_TF2", "EventFlagEvent", "player=%d,%d:%N, index=%d:%N, eventtype=%d",
                  player, ValidClientIndex(player), index, ValidClientIndex(index),
                  GetEventInt(event,"eventtype"));
    #endif

    if (IsValidClient(player))
    {
        new eventtype = GetEventInt(event,"eventtype");
        if (eventtype == TF_FLAGEVENT_CAPTURED)
        {
            GiveFlagCapturedXP(player);
            GiveFlagCapturedCrystalsAndGas(player);
        }
        else if (eventtype == TF_FLAGEVENT_DEFENDED)
            GiveFlagDefendedCrystalsAndGas(player);
        else if (eventtype == TF_FLAGEVENT_PICKEDUP)
            GivePickedUpFlagCrystalsAndGas(player);
        else if (eventtype == TF_FLAGEVENT_DROPPED)
            TakeDroppedFlagCrystalsAndGas(player);
    }

    #if defined _TRACE
        TraceReturn();
    #endif
}

public EventPointCapture(Handle:event,const String:name[],bool:dontBroadcast)
{
    decl String:cappers[MAXPLAYERS+1];
    if (GetEventString(event,"cappers", cappers, MAXPLAYERS)>0)
    {
        new bool:startcap = StrEqual(name, "teamplay_point_startcapture");
        new len = strlen(cappers);
        for(new i=0;i<len;i++)
        {
            new index = cappers{i};
            if (IsValidClient(index))
            {
                if (startcap)
                    GivePointTouchedCrystals(index);
                else
                {
                    GivePointCapturedXP(index);
                    GivePointCapturedCrystalsAndGas(index);
                }
            }
        }
    }
}

public EventCaptureBlocked(Handle:event,const String:name[],bool:dontBroadcast)
{
    new blocker=GetEventInt(event,"blocker");

    #if defined _TRACE
        new index = GetClientOfUserId(blocker); // Is blocker a client or a userid?
        TraceInto("Events_TF2", "EventCaptureBlocked", "blocker=%d:%N, index=%d:%N",
                  blocker, ValidClientIndex(blocker), index, ValidClientIndex(index));
    #endif

    if (IsValidClient(blocker))
    {
        GiveCapBlockedXP(blocker);
        GiveCapBlockedCrystalsAndGas(blocker);
    }

    #if defined _TRACE
        TraceReturn();
    #endif
}

public EventEscortScore(Handle:event,const String:name[],bool:dontBroadcast)
{
    new player = GetEventInt(event,"player");

    #if defined _TRACE
        new index = GetClientOfUserId(player); // Is player a client or a userid?
        TraceInto("Events_TF2", "EventEscortScore", "player=%d,%d:%N, index=%d:%N, points=%d",
                  player, ValidClientIndex(player), index, ValidClientIndex(index),
                  GetEventInt(event,"points"));
    #endif

    if (IsValidClient(player))
    {
        new points = GetEventInt(event,"points");
        GiveEscortXP(player, points);
        GiveEscortCrystalsAndGas(player, points);
    }

    #if defined _TRACE
        TraceReturn();
    #endif
}

public EventRoundSetupFinished(Handle:event,const String:name[],bool:dontBroadcast)
{
    g_RoundState = RoundSetupFinished;

    #if defined _TRACE
        LogRoundEvent(event,name,dontBroadcast);
    #endif
}

public EventScorePanel(Handle:event,const String:name[],bool:dontBroadcast)
{
    new team  = GetEventInt(event,"winning_team");
    new score = (team == 2) ? GetEventInt(event,"blue_score")
                            : GetEventInt(event,"red_score");

    SetTraceCategory("Event,Round");
    TraceInto("Events_TF2", "EventScorePanel", "team=%d", team);

    #if defined _TRACE
        LogRoundEvent(event,name,dontBroadcast);
    #endif

    for(new index=1;index<=MaxClients;index++)
    {
        if (IsClientInGame(index))
        {
            ResetInvulnerability(index);
            if (GetClientTeam(index) == team)
            {
                GiveGameWinCrystals(index,score);
                GiveGameWinXP(index,score);
            }
        }
    }

    new player = GetEventInt(event, "player_1");
    if (IsValidClient(player))
    {
        new points = GetEventInt(event,"player_1_points");
        if (points == 0)
            points = GetEventInt(event,"player_1_damage") / 10;
        if (points == 0)
            points = 1;

        GiveMvpCrystals(player,1,points);
        GiveMvpXP(player,1,points);
    }

    #if defined _TRACE
        new index = GetClientOfUserId(player);
        Trace("EventScorePanel; player_1=%d:%N, index=%d:%N", \
              player, ValidClientIndex(player), \
              index, ValidClientIndex(index));
    #endif

    player = GetEventInt(event, "player_2");
    if (IsValidClient(player))
    {
        new points = GetEventInt(event,"player_2_points");
        if (points == 0)
            points = GetEventInt(event,"player_2_damage") / 10;
        if (points == 0)
            points = 1;

        GiveMvpCrystals(player,2,points);
        GiveMvpXP(player,2,points);
    }

    #if defined _TRACE
        index = GetClientOfUserId(player);
        Trace("EventScorePanel; player_2=%d:%N, index=%d:%N", \
              player, ValidClientIndex(player), \
              index, ValidClientIndex(index));
    #endif

    player = GetEventInt(event, "player_3");
    if (IsValidClient(player))
    {
        new points = GetEventInt(event,"player_3_points");
        if (points == 0)
            points = GetEventInt(event,"player_3_damage") / 10;
        if (points == 0)
            points = 1;

        GiveMvpCrystals(player,3,points);
        GiveMvpXP(player,3,points);
    }

    #if defined _TRACE
        index = GetClientOfUserId(player);
        Trace("EventScorePanel; player_3=%d:%N, index=%d:%N", \
              player, ValidClientIndex(player), \
              index, ValidClientIndex(index));
    #endif

    player = GetEventInt(event, "player_4");
    if (IsValidClient(player))
    {
        new points = GetEventInt(event,"player_4_points");
        if (points == 0)
            points = GetEventInt(event,"player_4_damage") / 10;
        if (points == 0)
            points = 1;

        GiveMvpCrystals(player,4,points);
        GiveMvpXP(player,4,points);
    }

    #if defined _TRACE
        index = GetClientOfUserId(player);
        Trace("EventScorePanel; player_4=%d:%N, index=%d:%N", \
              player, ValidClientIndex(player), \
              index, ValidClientIndex(index));
    #endif

    player = GetEventInt(event, "player_5");
    if (IsValidClient(player))
    {
        new points = GetEventInt(event,"player_5_points");
        if (points == 0)
            points = GetEventInt(event,"player_5_damage") / 10;
        if (points == 0)
            points = 1;

        GiveMvpCrystals(player,5,points);
        GiveMvpXP(player,5,points);
    }

    #if defined _TRACE
        index = GetClientOfUserId(player);
        Trace("EventScorePanel; player_5=%d:%N, index=%d:%N", \
              player, ValidClientIndex(player), \
              index, ValidClientIndex(index));
    #endif

    player = GetEventInt(event, "player_6");
    if (IsValidClient(player))
    {
        new points = GetEventInt(event,"player_6_points");
        if (points == 0)
            points = GetEventInt(event,"player_6_damage") / 10;
        if (points == 0)
            points = 1;

        GiveMvpCrystals(player,6,points);
        GiveMvpXP(player,6,points);
    }

    #if defined _TRACE
        index = GetClientOfUserId(player);
        Trace("EventScorePanel; player_6=%d:%N, index=%d:%N", \
              player, ValidClientIndex(player), \
              index, ValidClientIndex(index));
    #endif

    // Give Mvp awards to the losing team.
    GiveTeamMvpAwards((team == 2) ? 3 : 2);
    TraceReturn();
}

GiveTeamMvpAwards(team)
{
    new ScoreOffset = FindSendPropOffs("CTFPlayerResource", "m_iTotalScore");
    new EntPlayerManager = FindEntityByClassname(-1, "tf_player_manager");
    if (EntPlayerManager > 0 && ScoreOffset > 0)
    {
        new Scores[MaxClients][2];
        new client;

        // For sorting purpose, fill Scores[][] array from zero index
        //
        for (new i = 0; i < MaxClients; i++)
        {
            client = i + 1;
            Scores[i][0] = client;
            if (IsClientInGame(client) && GetClientTeam(client) == team)
                Scores[i][1] = GetEntData(EntPlayerManager, ScoreOffset + (client * 4), 4);
            else
                Scores[i][1] = -1;
        }

        SortCustom2D(Scores, MaxClients, SortScoreDesc);

        for (new n = 0; n <= 5; n++)
        {
            if (Scores[n][0] > 0  && Scores[n][1] > 0)
            {
                GiveMvpCrystals(Scores[n][0],n+1,Scores[n][1]);
                GiveMvpEnergy(Scores[n][0],n+1,Scores[n][1]);
            }
        }
    }
}

public SortScoreDesc(x[], y[], array[][], Handle:data)
{
    if (x[1] > y[1])
        return -1;
    else if (x[1] < y[1])
        return 1;
    else
        return 0;
}

public EventBroadcast(Handle:event,const String:name[],bool:dontBroadcast)
{
    decl String:sound[64];
    GetEventString(event, "sound", sound, sizeof(sound));

    TraceInto("Events_TF2", "EventBroadcast", "sound=%s", sound);

    if (StrEqual(sound,"Game.SuddenDeath"))
        g_RoundState = RoundSuddenDeath;

    TraceReturn();
}

public Action:LogScanner(const String:message[])
{
    if (StrContains(message, "changed map", false) >= 0 ||
        StrContains(message, "changing map", false) >= 0)
    {
        g_MapChanging = true;
        LogMessage("LogScanner() found a Map Change:%s", message);
    }
    return Plugin_Continue;
}

public Action:OnLogAction(Handle:source, Identity:ident, client, target, const String:message[])
{
    if (StrContains(message, "changed map", false) >= 0 ||
        StrContains(message, "changing map", false) >= 0)
    {
        g_MapChanging = true;
        LogMessage("OnLogAction() found a Map Change:%s", message);
    }

    return Plugin_Continue;
}

public EntityOutput_HealthKit(const String:output[], caller, activator, Float:delay)
{
    if (IsValidClient(activator) && IsPlayerAlive(activator))
    {
        #if defined TRACK_DAMAGE
            SetSavedHealth(activator, GetClientHealth(activator));
        #endif

        new Action:result = Plugin_Continue;
        Call_StartForward(g_OnHealthUsedHandle);
        Call_PushCell(activator);
        Call_PushCell(caller);
        Call_Finish(result);
    }
}

public EntityOutput_OnAnimationBegun(const String:output[], caller, activator, Float:delay)
{
	if (IsValidEntity(caller))
	{
		new String:modelname[128];
		GetEntPropString(caller, Prop_Data, "m_ModelName", modelname, 128);
		if (StrEqual(modelname, "models/props_gameplay/resupply_locker.mdl"))
		{
			decl Float:pos[3], Float:orig[3];
			GetEntPropVector(caller, Prop_Send, "m_vecOrigin", pos);

			for (new index=1;index<=MaxClients;index++)
			{
                if (IsClientInGame(index) && IsPlayerAlive(index))
				{
                    GetClientAbsOrigin(index, orig);
                    if (IsPointInRange(pos, orig, 128.0))
                    {
                        #if defined TRACK_DAMAGE
                            SetSavedHealth(index, GetClientHealth(index));
                        #endif

                        new Action:result = Plugin_Continue;
                        Call_StartForward(g_OnCabinetUsedHandle);
                        Call_PushCell(index);
                        Call_PushCell(caller);
                        Call_Finish(result);
                    }
                }
            }
        }
    }
}
