/**
 * vim: set ai et ts=4 sw=4 :
 * File: cooldown.inc
 * Description: Cooldown processing
 * Author(s): -=|JFH|=-Naris
 */

#include "sc/cooldown"

new const String:deniedWav[] = "sc/buzz.wav";
new const String:rechargeWav[] = "sc/transmission.wav";

// Handles
new Handle:g_OnCooldownExpiredHandle = INVALID_HANDLE;
new Handle:m_PlayerCooldownTrie[MAXPLAYERS+1];
new Handle:m_PlayerCooldownArray[MAXPLAYERS+1];

// Place to stash the last cooldown type
new m_CooldownExpireRace;
new m_CooldownExpireClient;
new m_CooldownExpireUpgrade;
new CooldownType:m_CooldownExpireType;

Float:GetCooldownExpireTime(client, race, upgrade, &Handle:pack=INVALID_HANDLE)
{
    new Handle:trie = m_PlayerCooldownTrie[client];
    if (trie != INVALID_HANDLE)
    {
        decl String:key[SHORT_STRING_LENGTH];
        Format(key, sizeof(key), "%d-%d", race, upgrade);

        if (GetTrieValue(trie, key, pack))
        {
            ResetPack(pack);
            new index  = ReadPackCell(pack);
            new userid = ReadPackCell(pack);
            if (GetClientOfUserId(userid) == index)
            {
                ReadPackCell(pack); // race
                ReadPackCell(pack); // upgrade
                return ReadPackFloat(pack); // expireTime
            }
        }
    }
    return 0.0;
}

CooldownType:GetCooldownType(client, race, upgrade)
{
    if (client == m_CooldownExpireClient &&
        race == m_CooldownExpireRace &&
        upgrade == m_CooldownExpireUpgrade)
    {
        return m_CooldownExpireType;
    }
    else
    {
        new Handle:pack = INVALID_HANDLE;
        new Float:cooldownExpires = GetCooldownExpireTime(client, race, upgrade, pack);
        if (pack != INVALID_HANDLE && cooldownExpires != 0.0)
            return CooldownType:ReadPackCell(pack); // CooldownType:type
        else        
            return Cooldown_None;
    }
}

bool:HasCooldownExpired(client, race, upgrade, bool:notify=true)
{
    new Handle:pack = INVALID_HANDLE;
    new Float:currentTime = GetGameTime();
    new Float:cooldownExpires = GetCooldownExpireTime(client, race, upgrade, pack);
    if (cooldownExpires > currentTime)
    {
        if (notify)
        {
            ReadPackCell(pack); // CooldownType:type
            ReadPackCell(pack); // Handle:timerHandle

            decl String:name[NAME_STRING_LENGTH];
            ReadPackString(pack, name, sizeof(name));

            if (!name[0])
            {
                new Handle:raceHandle=GetRaceHandle(race);
                new upgradeCategory = GetUpgradeCategory(raceHandle, upgrade);
                if (upgradeCategory > 1)
                    Format(name,sizeof(name),"Ultimate%d",upgradeCategory);
                else if (upgradeCategory == 1)
                    strcopy(name, sizeof(name), "Ultimate");
                else
                    strcopy(name, sizeof(name), "Upgrade");
            }

            PrepareSound(deniedWav);
            EmitSoundToClient(client,deniedWav);

            new Float:remaining = cooldownExpires - currentTime;
            new Float:minutes = remaining / 60.0;
            if (minutes > 1.0)
            {
                new seconds = RoundToFloor(FloatFraction(minutes) * 60.0);
                DisplayMessage(client, Display_Ultimate, "%t", "WontBeReadyForMinutes",
                               name, RoundToFloor(minutes), seconds);
            }
            else
            {
                DisplayMessage(client, Display_Ultimate, "%t", "WontBeReadyForSeconds",
                               name, remaining);
            }
        }
        return false;
    }
    else
        return true;
}

CreateCooldown(client, race, upgrade, Float:cooldown, const String:name[],
               CooldownType:type, const String:expireSound[])
{
    TraceInto("Cooldown", "CreateCooldown", "client=%d:%N, race=%d, upgrade=%d", \
              client, ValidClientIndex(client), race, upgrade);

    if (cooldown > 0.0)
    {
        if (name[0] && (type & Cooldown_CreateNotify) == Cooldown_CreateNotify)
        {
            new Float:minutes = cooldown / 60.0;
            if (minutes > 1.0)
            {
                new seconds = RoundToFloor(FloatFraction(minutes) * 60.0);
                DisplayMessage(client, Display_Ultimate, "%t", "MustWaitMinutes",
                               RoundToFloor(minutes), seconds, name);
            }
            else
            {
                DisplayMessage(client, Display_Ultimate, "%t", "MustWaitSeconds",
                               cooldown, name);
            }
        }

        if ((type & Cooldown_AutoReset) == Cooldown_AutoReset)
        {
            if (cooldown > 30.0)
            {
                type &= ~(Cooldown_ResetOnDeath  |
                          Cooldown_ResetOnSpawn);

                if (cooldown > 60.0)
                    type &= ~Cooldown_ResetOnChange;
                else                    
                    type |= Cooldown_ResetOnChange;
            }
            else
            {
                type |= (Cooldown_ResetOnChange |
                         Cooldown_ResetOnDeath);
            }
        }            

        new Handle:pack;
        new Handle:timer = CreateDataTimer(cooldown,CooldownExpireTimer,pack,TIMER_FLAG_NO_MAPCHANGE);

        TraceCat("Array", "CreateDataTimer Cooldown[%d,%d,%d]=0x%08x; pack=0x%08x", \
                 client, race, upgrade, timer, pack);

        if (pack != INVALID_HANDLE)
        {
            WritePackCell(pack, client);
            WritePackCell(pack, GetClientUserId(client));
            WritePackCell(pack, race);
            WritePackCell(pack, upgrade);
            WritePackFloat(pack, GetGameTime() + cooldown);
            WritePackCell(pack, _:type);
            WritePackCell(pack, _:timer);
            WritePackString(pack, name);
            WritePackString(pack, expireSound);

            new Handle:array = m_PlayerCooldownArray[client];
            if (array == INVALID_HANDLE)
            {
                array = m_PlayerCooldownArray[client] = CreateArray();

                TraceCat("Array", "CreateArray m_PlayerCooldownArray[%d]=0x%08x", \
                             client,array);
            }

            new Handle:trie = m_PlayerCooldownTrie[client];
            if (trie == INVALID_HANDLE)
            {
                trie = m_PlayerCooldownTrie[client] = CreateTrie();

                TraceCat("Array", "CreateTrie m_PlayerCooldownTrie[%d]=0x%08x", \
                             client,trie);
            }

            decl String:key[SHORT_STRING_LENGTH];
            Format(key, sizeof(key), "%d-%d", race, upgrade);

            // Check if there is already a cooldown for this client/race/upgrade.
            new Handle:oldPack;
            if (GetTrieValue(trie, key, oldPack))
            {
                // If there is, delete the old one
                // and replace it with the new one.

                ResetPack(oldPack);
                ReadPackCell(oldPack); // index
                ReadPackCell(oldPack); // userid
                ReadPackCell(oldPack); // race
                ReadPackCell(oldPack); // upgrade
                ReadPackFloat(oldPack); // expireTime
                ReadPackCell(oldPack); // CooldownType:type
                new Handle:oldTimerHandle = Handle:ReadPackCell(oldPack);

                new item = FindValueInArray(array, oldPack);
                if (item >= 0)
                    SetArrayCell(array, item, pack);
                else
                    PushArrayCell(array, pack);

                if (oldTimerHandle != INVALID_HANDLE)
                {
                    KillTimer(oldTimerHandle, true);

                    TraceCat("Array", "CloseDataTimer Cooldown[%d,%d,%d]=0x%08x; pack=0x%08x", \
                             client, race, upgrade, oldTimerHandle, oldPack);
                }
                else
                {
                    CloseHandle(oldPack);

                    TraceCat("Array", "CloseDataPack Cooldown[%d,%d,%d]=0x%08x; pack=0x%08x", \
                             client, race, upgrade, oldTimerHandle, oldPack);
                }
            }
            else
            {
                PushArrayCell(array, pack);
            }

            SetTrieValue(trie, key, pack, true);
        }
    }

    TraceReturn();
}

public Action:CooldownExpireTimer(Handle:timer,any:pack)
{
    ResetPack(pack);
    new index  = ReadPackCell(pack);
    new userid = ReadPackCell(pack);
    new race = ReadPackCell(pack);
    new upgrade = ReadPackCell(pack);

    TraceInto("Cooldown", "CooldownExpireTimer", \
              "timer=0x%08x, pack=0x%08x, userid=%d, index=%d:%N, race=%d, upgrade=%d", \
              timer, pack, userid, index, ValidClientIndex(index), race, upgrade);

    new Handle:trie = m_PlayerCooldownTrie[index];
    if (trie != INVALID_HANDLE)
    {
        decl String:key[SHORT_STRING_LENGTH];
        Format(key, sizeof(key), "%d-%d", race, upgrade);
        RemoveFromTrie(trie, key);
    }

    new Handle:array = m_PlayerCooldownArray[index];
    if (array != INVALID_HANDLE)
    {
        new item = FindValueInArray(array, pack);
        if (item >= 0)
            RemoveFromArray(array, item);
    }

    if (GetClientOfUserId(userid) == index)
    {
        ReadPackFloat(pack); // expireTime
        new CooldownType:type = CooldownType:ReadPackCell(pack);
        ReadPackCell(pack); // Handle:timerHandle

        // Stash the cooldown type
        m_CooldownExpireRace    = race;
        m_CooldownExpireClient  = index;
        m_CooldownExpireUpgrade = upgrade;
        m_CooldownExpireType    = type;

        new Action:result = Plugin_Continue;
        Call_StartForward(g_OnCooldownExpiredHandle);
        Call_PushCell(index);
        Call_PushCell(race);
        Call_PushCell(upgrade);
        Call_PushCell(true);
        Call_Finish(result);

        if ((type & (Cooldown_ExpireNotify|Cooldown_AlwaysNotify)) != Cooldown_TimerOnly &&
            result == Plugin_Continue)
        {
            if (IsClientInGame(index) && IsPlayerAlive(index))
            {
                if ((type & Cooldown_AlwaysNotify) != Cooldown_TimerOnly ||
                    GetRace(index) == race)
                {
                    decl String:name[NAME_STRING_LENGTH];
                    ReadPackString(pack, name, sizeof(name));

                    decl String:expireSound[LONG_STRING_LENGTH];
                    ReadPackString(pack, expireSound, sizeof(expireSound));

                    if (!name[0])
                    {
                        new Handle:raceHandle=GetRaceHandle(race);
                        new upgradeCategory = GetUpgradeCategory(raceHandle, upgrade);
                        if (upgradeCategory > 1)
                            Format(name,sizeof(name),"Ultimate%d",upgradeCategory);
                        else if (upgradeCategory == 1)
                            strcopy(name, sizeof(name), "Ultimate");
                        else
                            strcopy(name, sizeof(name), "Upgrade");
                    }

                    if (expireSound[0])
                    {
                        PrepareSound(expireSound);
                        EmitSoundToClient(index, expireSound);
                    }
                    else
                    {
                        PrepareSound(rechargeWav);
                        EmitSoundToClient(index, rechargeWav);
                    }

                    DisplayMessage(index, Display_Ultimate,
                                   "%t", "IsReady", name);
                }
            }
        }
    }

    SetTraceCategory("Array");
    TraceReturn("EndDataTimer Cooldown[%d,%d,%d]=0x%08x; pack=0x%08x", \
                index, race, upgrade, timer, pack);

    return Plugin_Stop;
}

CooldownSpawn(client)
{
    new race = GetRace(client);
    new Handle:raceHandle = GetRaceHandle(race);
    new upgradeCount=GetUpgradeCount(raceHandle);
    for (new upgrade=0;upgrade<upgradeCount;upgrade++)
    {
        new CooldownType:type = GetUpgradeCooldownType(raceHandle,upgrade);
        if ((type & Cooldown_CreateOnSpawn) == Cooldown_CreateOnSpawn)
        {
            new Float:cooldown = GetUpgradeCooldownDuration(raceHandle,upgrade);
            if (cooldown > 0.0)
            {
                decl String:name[64];
                GetUpgradeName(raceHandle, upgrade, name, sizeof(name), client);
                CreateCooldown(client, race, upgrade, cooldown, name, type, "");
            }
        }
    }
}

CooldownEvent(client, CooldownType:resetBits)
{
    TraceInto("Cooldown", "CooldownEvent", "client=%d:%N, resetBits=0x%02x", \
              client, ValidClientIndex(client), resetBits);

    new Handle:array = m_PlayerCooldownArray[client];
    if (array != INVALID_HANDLE)
    {
        // Iterate backwards since we will be removing elements.
        for (new i = GetArraySize(array)-1; i >= 0 ; i--)
        {
            new Handle:pack = Handle:GetArrayCell(array, i);
            ResetPack(pack);

            ReadPackCell(pack); // index
            new userid = ReadPackCell(pack);
            new race = ReadPackCell(pack);
            new upgrade = ReadPackCell(pack);
            new Float:expireTime = ReadPackFloat(pack);
            new CooldownType:type = CooldownType:ReadPackCell(pack);
            new Handle:timerHandle = Handle:ReadPackCell(pack);

            new bool:remove = (GetClientOfUserId(userid) != client);
            if (remove || ((type & resetBits) != Cooldown_TimerOnly &&
                           expireTime > GetGameTime()))
            {
                new Action:result = Plugin_Continue;
                Call_StartForward(g_OnCooldownExpiredHandle);
                Call_PushCell(client);
                Call_PushCell(race);
                Call_PushCell(upgrade);
                Call_PushCell(false);
                Call_Finish(result);

                if (remove || result == Plugin_Continue)
                {
                    new Handle:trie = m_PlayerCooldownTrie[client];
                    if (trie != INVALID_HANDLE)
                    {
                        decl String:key[SHORT_STRING_LENGTH];
                        Format(key, sizeof(key), "%d-%d", race, upgrade);
                        RemoveFromTrie(trie, key);
                    }

                    RemoveFromArray(array, i);

                    if (timerHandle != INVALID_HANDLE)
                    {
                        KillTimer(timerHandle, true);

                        TraceCat("Array", "CloseDataTimer Cooldown[%d,%d,%d]=0x%08x; pack=0x%08x", \
                                 client, race, upgrade, timerHandle, pack);
                    }
                    else
                    {
                        CloseHandle(pack);

                        TraceCat("Array", "CloseDataPack Cooldown[%d,%d,%d]=0x%08x; pack=0x%08x", \
                                 client, race, upgrade, timerHandle, pack);
                    }
                }
            }
        }
    }

    TraceReturn();
}

CooldownDisconnect(client)
{
    TraceInto("Cooldown", "CooldownDisconnect", "client=%d:%N", \
              client, ValidClientIndex(client));

    new Handle:trie = m_PlayerCooldownTrie[client];
    if (trie != INVALID_HANDLE)
    {
        ClearTrie(trie);
        CloseHandle(trie);
        m_PlayerCooldownTrie[client] = INVALID_HANDLE;

        TraceCat("Array", "CloseTrie m_PlayerCooldownTrie[%d]=0x%08x", \
                 client,trie);
    }

    new Handle:array = m_PlayerCooldownArray[client];
    if (array != INVALID_HANDLE)
    {
        // Iterate backwards for consistency.
        for (new i = GetArraySize(array)-1; i >= 0 ; i--)
        {
            new Handle:pack = Handle:GetArrayCell(array, i);
            ResetPack(pack);
            ReadPackCell(pack); // index
            ReadPackCell(pack); // userid
            new race = ReadPackCell(pack); // race
            new upgrade = ReadPackCell(pack); // upgrade
            ReadPackFloat(pack); // expireTime
            ReadPackCell(pack); // CooldownType:type
            new Handle:timerHandle = Handle:ReadPackCell(pack);

            #if !defined _TRACE
                #pragma unused race, upgrade
            #endif

            if (timerHandle != INVALID_HANDLE)
            {
                KillTimer(timerHandle, true);

                TraceCat("Array", "CloseDataTimer Cooldown[%d,%d,%d]=0x%08x; pack=0x%08x", \
                         client, race, upgrade, timerHandle, pack);
            }
            else
            {
                CloseHandle(pack);

                TraceCat("Array", "CloseDataPack Cooldown[%d,%d,%d]=0x%08x; pack=0x%08x", \
                         client, race, upgrade, timerHandle, pack);
            }
        }

        ClearArray(array);
        CloseHandle(array);
        m_PlayerCooldownArray[client] = INVALID_HANDLE;

        TraceCat("Array", "CloseArray m_PlayerCooldownArray[%d]=0x%08x", \
                 client,array);
    }

    TraceReturn();
}

ResetAllCooldowns()
{
    for (new i = 1; i <= MaxClients; i++)
        CooldownDisconnect(i);
}

ResetCooldown(client, race, upgrade, bool:notify)
{
    TraceInto("Cooldown", "ResetCooldown", "client=%d:%N, race=$d, upgrade=%d, notify=%d", \
              client, ValidClientIndex(client), race, upgrade, notify);

    new Handle:trie = m_PlayerCooldownTrie[client];
    if (trie != INVALID_HANDLE)
    {
        decl String:key[SHORT_STRING_LENGTH];
        Format(key, sizeof(key), "%d-%d", race, upgrade);

        new Handle:pack;
        if (GetTrieValue(trie, key, pack))
        {
            ResetPack(pack);
            ReadPackCell(pack); // index
            ReadPackCell(pack); // userid
            ReadPackCell(pack); // race
            ReadPackCell(pack); // upgrade
            ReadPackFloat(pack); // expireTime
            new CooldownType:type = CooldownType:ReadPackCell(pack);
            new Handle:timerHandle = Handle:ReadPackCell(pack);

            new Action:result = Plugin_Continue;
            Call_StartForward(g_OnCooldownExpiredHandle);
            Call_PushCell(client);
            Call_PushCell(race);
            Call_PushCell(upgrade);
            Call_PushCell(false);
            Call_Finish(result);

            if (result == Plugin_Continue)
            {
                if ((type & (Cooldown_ExpireNotify|Cooldown_AlwaysNotify)) != Cooldown_TimerOnly)
                {
                    if (IsClientInGame(client) && IsPlayerAlive(client))
                    {
                        if (notify || (type & Cooldown_AlwaysNotify) != Cooldown_TimerOnly)
                        {
                            decl String:name[NAME_STRING_LENGTH];
                            ReadPackString(pack, name, sizeof(name));

                            decl String:expireSound[LONG_STRING_LENGTH];
                            ReadPackString(pack, expireSound, sizeof(expireSound));

                            if (!name[0])
                            {
                                new Handle:raceHandle=GetRaceHandle(race);
                                new upgradeCategory = GetUpgradeCategory(raceHandle, upgrade);
                                if (upgradeCategory > 1)
                                    Format(name,sizeof(name),"Ultimate%d",upgradeCategory);
                                else if (upgradeCategory == 1)
                                    strcopy(name, sizeof(name), "Ultimate");
                                else
                                    strcopy(name, sizeof(name), "Upgrade");
                            }

                            if (expireSound[0])
                            {
                                PrepareSound(expireSound);
                                EmitSoundToClient(client, expireSound);
                            }
                            else
                            {
                                PrepareSound(rechargeWav);
                                EmitSoundToClient(client, rechargeWav);
                            }

                            DisplayMessage(client, Display_Ultimate,
                                           "%t", "IsReady", name);
                        }
                    }
                }

                RemoveFromTrie(trie, key);

                new Handle:array = m_PlayerCooldownArray[client];
                if (array != INVALID_HANDLE)
                {
                    new item = FindValueInArray(array, pack);
                    if (item >= 0)
                        RemoveFromArray(array, item);
                }

                if (timerHandle != INVALID_HANDLE)
                {
                    KillTimer(timerHandle, true);

                    TraceCat("Array", "CloseDataTimer Cooldown[%d,%d,%d]=0x%08x; pack=0x%08x", \
                             client, race, upgrade, timerHandle, pack);
                }
                else
                {
                    CloseHandle(pack);

                    TraceCat("Array", "CloseDataPack Cooldown[%d,%d,%d]=0x%08x; pack=0x%08x", \
                             client, race, upgrade, timerHandle, pack);
                }
            }
        }
    }

    TraceReturn();
}
