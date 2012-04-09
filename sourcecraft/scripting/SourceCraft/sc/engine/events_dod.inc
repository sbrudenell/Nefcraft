/**
 * vim: set ai et ts=4 sw=4 syntax=sourcepawn :
 * File: events_dod.inc
 * Description: The handler for all the dod events.
 * Author(s):  Naris (Murray Wilson)
 */

bool:HookDodEvents()
{
    if (!HookEventEx("dod_round_start",RoundStart,EventHookMode_PostNoCopy))
    {
        LogError("Could not hook the dod_round_start event.");
        return false;
    }

    if (!HookEventEx("dod_restart_round",RoundStart,EventHookMode_PostNoCopy))
    {
        LogError("Could not hook the dod_restart_round event.");
        return false;
    }

    if (!HookEventEx("dod_round_active",EventRoundActive,EventHookMode_PostNoCopy))
    {
        LogError("Could not hook the dod_round_active event.");
        return false;
    }

    if (!HookEventEx("dod_warmup_ends",EventRoundSetupFinished,EventHookMode_PostNoCopy))
    {
        LogError("Could not hook the dod_warmup_ends event.");
        return false;
    }

    if (!HookEvent("dod_point_captured", EventPointCapture))
    {
        LogError("Could not hook the dod_point_captured event.");
        return false;
    }

    if (!HookEvent("dod_capture_blocked", EventCaptureBlocked))
    {
        LogError("Could not hook the dod_capture_blocked event.");
        return false;
    }

    if (!HookEventEx("dod_round_win",EventRoundWin))
    {
        LogError("Could not hook the dod_round_win event.");
        return false;
    }

    if (!HookEventEx("dod_game_over",EventGameOver,EventHookMode_PostNoCopy))
    {
        LogError("Could not hook the dod_game_over event.");
        return false;
    }

    if (!HookEventEx("dod_bomb_planted",BombPlantedEvent))
    {
        LogError("Could not hook the dod_bomb_planted event.");
        return false;
    }

    if (!HookEventEx("dod_bomb_defused",BombDefusedEvent))
    {
        LogError("Could not hook the dod_bomb_defused event.");
        return false;
    }

    if (!HookEventEx("dod_bomb_exploded",BombExplodedEvent))
    {
        LogError("Could not hook the dod_bomb_exploded event.");
        return false;
    }

    if (!HookEventEx("dod_kill_planter",KillPlanterEvent))
    {
        LogError("Could not hook the dod_kill_planter event.");
        return false;
    }

    if (!HookEventEx("dod_kill_defuser",KillDefuserEvent))
    {
        LogError("Could not hook the dod_kill_defuser event.");
        return false;
    }

    if (!HookEvent("player_class",PlayerChangeClassEvent))
    {
        LogError("Couldn't hook the player_class event.");
        return false;
    }

    return true;
}

public KillPlanterEvent(Handle:event,const String:name[],bool:dontBroadcast)
{
    new index = GetClientOfUserId(GetEventInt(event,"userid"));
    if (index > 0)
    {
        GiveKillPlanterXP(index);
        GiveKillPlanterCrystalsAndGas(index);
    }
}

public KillDefuserEvent(Handle:event,const String:name[],bool:dontBroadcast)
{
    new index = GetClientOfUserId(GetEventInt(event,"userid"));
    if (index > 0)
    {
        GiveKillDefuserXP(index);
        GiveKillDefuserCrystalsAndGas(index);
    }
}

