/**
 * vim: set ai et ts=4 sw=4 syntax=sourcepawn :
 * File: get_damage.inc
 * Description: Damage functions and variables
 * Author(s): Naris (Murray Wilson)
 */
 
#if defined _get_damage_included
 #endinput
#endif
#define _get_damage_included

//#define TRACK_DAMAGE
#if defined TRACK_DAMAGE
#define ResetHealthTimer(%1) g_HealthTimers[%1] = INVALID_HANDLE

stock Handle:g_HealthTimers[MAXPLAYERS+1];
stock g_savedHealth[MAXPLAYERS+1];

stock SaveHealth(client)
{
    if (IsClientInGame(client))
        return g_savedHealth[client] = GetClientHealth(client);
    else
        return 0;
}

stock SetSavedHealth(client, health)
{
    g_savedHealth[client] = health;
}

stock GetSavedHealth(client)
{
    return g_savedHealth[client];
}

stock SaveAllHealth()
{
    for(new x=1;x<=MaxClients;x++)
    {
        if (IsClientInGame(x) && !IsFakeClient(x))
        {
            if (IsPlayerAlive(x))
                g_savedHealth[x] = GetClientHealth(x);
        }
    }
}

stock CreateHealthTimer(client)
{
    if (g_HealthTimers[client] == INVALID_HANDLE)
        g_HealthTimers[client] = CreateTimer(0.1,Timer_SaveHealth,client,
                                             TIMER_REPEAT|TIMER_FLAG_NO_MAPCHANGE);
}

stock KillHealthTimer(client)
{
    new Handle:timer=g_HealthTimers[client];
    if (timer != INVALID_HANDLE)
    {
        KillTimer(timer);
        g_HealthTimers[client] = INVALID_HANDLE;	
    }
}

stock KillAllHealthTimers()
{
    for (new i = 1; i <= MaxClients; i++)
        KillHealthTimer(i);
}

stock ResetAllHealthTimers()
{
    for (new i = 1; i <= MaxClients; i++)
        ResetHealthTimer(i);
}

public Action:Timer_SaveHealth(Handle:timer, any:client)
{
    g_savedHealth[client] = GetClientHealth(client);
}
#endif

stock GetDamage(Handle:event, client=0)
{
    new damage;

    switch (GameType)
    {
        case tf2:
            damage = GetEventInt(event,"damageamount");
        case cstrike:
            damage = GetEventInt(event,"dmg_health");
        case dod:
            damage = GetEventInt(event,"damage");
        default:
        {
            #if defined TRACK_DAMAGE
            if (!client)
                client = GetClientOfUserId(GetEventInt(event,"userid"));

            if (client)
            {
                new prevHealth = g_savedHealth[client]; 
                if (prevHealth)
                {
                    damage = prevHealth - GetEventInt(event,"health");
                    if (damage < 0)
                        damage = 0;
                }
            }
            #else
                damage = 0;
            #endif
        }
    }

    #if defined TRACK_DAMAGE
    if (!client)
        client = GetClientOfUserId(GetEventInt(event,"userid"));

    if (client)
    {
        new prevHealth = g_savedHealth[client]; 
        if (prevHealth)
        {
            new calc_damage = prevHealth - GetEventInt(event,"health");
            if (calc_damage < 0)
                calc_damage = 0;

            if (calc_damage != damage)
            {
                LogMessage("GetDamage(%d:%N)=%d,calc=%d",
                           client, ValidClientIndex(client),
                           damage, calc_damage);
            }
        }
    }
    #endif

    return damage;
}
