/**
 * vim: set ai et ts=4 sw=4 syntax=sourcepawn :
 * File: entlimit.inc
 * Description: Function to check the entity limit.
 *              Use before spawning an entity.
 * Author(s): Marc Hörsken
 */

#if defined _entlimit_included
 #endinput
#endif
#define _entlimit_included

stock bool:IsEntLimitReached(warn=20,critical=16,client=0,const String:message[]="")
{
    return (EntitiesAvailable(warn,critical,client,message) < warn);
}

stock EntitiesAvailable(warn=20,critical=16,client=0,const String:message[]="")
{
    new max = GetMaxEntities();
    new count = GetEntityCount();
    new remaining = max - count;
    if (remaining <= critical)
    {
        PrintToServer("Warning: Entity limit is nearly reached! Please switch or reload the map!");
        LogError("Entity limit is nearly reached: %d/%d (%d):%s", count, max, remaining, message);

        if (client > 0)
        {
            PrintToConsole(client, "Entity limit is nearly reached: %d/%d (%d):%s",
                           count, max, remaining, message);
        }
    }
    else if (remaining <= warn)
    {
        PrintToServer("Caution: Entity count is getting high!");
        LogMessage("Entity count is getting high: %d/%d (%d):%s", count, max, remaining, message);

        if (client > 0)
        {
            PrintToConsole(client, "Entity count is getting high: %d/%d (%d):%s",
                           count, max, remaining, message);
        }
    }
    return remaining;
}
