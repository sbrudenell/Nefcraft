/**
 * vim: set ai et ts=4 sw=4 :
 * File: cooldown.inc
 * Description: Cooldown Type bits
 * Author(s): -=|JFH|=-Naris
 */

#if defined _cooldowntype_included
 #endinput
#endif
#define _cooldowntype_included

enum CooldownType (<<= 1)
{
    Cooldown_None = 0,
    Cooldown_TimerOnly = 0,
    Cooldown_AutoReset = 1,
    Cooldown_ResetOnChange,
    Cooldown_ResetOnDeath,
    Cooldown_ResetOnSpawn,
    Cooldown_CreateNotify,
    Cooldown_ExpireNotify,
    Cooldown_AlwaysNotify,
    Cooldown_CreateOnSpawn,
    Cooldown_SpecifiesBaseValue
};
