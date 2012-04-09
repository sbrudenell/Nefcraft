/**
 * vim: set ai et ts=4 sw=4 syntax=sourcepawn :
 * File: events_cstrike.inc
 * Description: The handler for all the counter-strike events.
 * Author(s): Naris (Murray Wilson)
 * Credits: Anthony Iacono
 */

bool:HookCStrikeEvents()
{
    if (!HookEventEx("round_start",RoundStart,EventHookMode_PostNoCopy))
    {
        LogError("Could not hook the round_start event.");
        return false;
    }

    if (!HookEventEx("round_end",EventRoundWin))
    {
        LogError("Could not hook the round_end event.");
        return false;
    }

    if (!HookEventEx("round_freeze_end",EventRoundActive,EventHookMode_PostNoCopy))
    {
        LogError("Could not hook the round_freeze_end event.");
        return false;
    }

    if (!HookEventEx("bomb_planted",BombPlantedEvent))
    {
        LogError("Could not hook the bomb_planted event.");
        return false;
    }

    if (!HookEventEx("bomb_defused",BombDefusedEvent))
    {
        LogError("Could not hook the bomb_defused event.");
        return false;
    }

    if (!HookEventEx("bomb_exploded",BombExplodedEvent))
    {
        LogError("Could not hook the bomb_exploded event.");
        return false;
    }

    if (!HookEventEx("hostage_rescued",HostageRescuedEvent))
    {
        LogError("Could not hook the hostage_rescued event.");
        return false;
    }

    return true;
}

public HostageRescuedEvent(Handle:event,const String:name[],bool:dontBroadcast)
{
    new index = GetClientOfUserId(GetEventInt(event,"userid"));
    if (index > 0)
    {
        GiveHostageRescuedXP(index);
        GiveHostageCrystalsAndGas(index);
    }
}
