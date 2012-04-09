/**
 * vim: set ai et ts=4 sw=4 syntax=sourcepawn :
 * File: playertracking.inc
 * Description: More storage for players, temporary and long term.
 * Author(s): -=|JFH|=-Naris (Murray Wilson)
 */

#if defined _player_tracking_included
 #endinput
#endif
#define _player_tracking_included

// Global definition
new Handle:g_OnUpgradeLevelChangedHandle = INVALID_HANDLE;
new Handle:g_OnRaceDeselectedHandle = INVALID_HANDLE;
new Handle:g_OnRaceSelectedHandle = INVALID_HANDLE;

GetOwnsItem(client,item)
{
    TraceInto("PlayerTracking", "GetOwnsItem", "client=%d:%N, item=%d", \
              client, ValidClientIndex(client), item);

    new owns=0;
    if (GetShopItemCount() >= 1)
    {
        new Handle:shopitems=GetShopitems(client);
        if (shopitems != INVALID_HANDLE)
            owns = GetArrayCell(shopitems,item);
    }

    TraceReturn("owns=%d", owns);
    return owns;
}

SetOwnsItem(client,item,owns)
{
    TraceInto("PlayerTracking", "SetOwnsItem", "client=%d:%N, item=%d, owns=%d", \
              client, ValidClientIndex(client), item, owns);

    if (GetShopItemCount() >= 1)
    {
        new Handle:shopitems=GetShopitems(client);
        if (shopitems != INVALID_HANDLE)
            SetArrayCell(shopitems,item,owns);
    }

    TraceReturn();
}

GetXP(client,race)
{
    TraceInto("PlayerTracking", "GetXP", "client=%d:%N, race=%d", \
              client, ValidClientIndex(client), race);

    new xp = 0;
    if (race >= 0 && GetRaceCount() > 1)
    {
        new Handle:raceInfo = GetRaceInfo(client, race);
        if (raceInfo != INVALID_HANDLE)
            xp = GetRaceInfoXP(raceInfo);
    }

    TraceReturn("Get %d:%N's race %d XP=%d", \
                client, ValidClientIndex(client), \
                race, xp);

    return xp;
}

SetXP(client,race,xp)
{
    TraceInto("PlayerTracking", "SetXP", "client=%d:%N, race=%d, xp=%d", \
              client, ValidClientIndex(client), race, xp);

    if (race >= 0 && GetRaceCount() > 1)
    {
        new Handle:raceInfo = GetRaceInfo(client, race);
        if (raceInfo != INVALID_HANDLE)
        {
            Trace("Set %d:%N's race %d XP=%d", \
                  client, ValidClientIndex(client), \
                  race, xp);

            SetRaceInfoXP(raceInfo,xp);
            SetDatabaseSaved(client, false);
        }
    }

    TraceReturn();
}

ResetXP(client,race,xp)
{
    TraceInto("PlayerTracking", "ResetXP", "client=%d:%N, race=%d, xp=%d", \
              client, ValidClientIndex(client), race, xp);

    if (race >= 0 && GetRaceCount() > 1)
    {
        new Handle:raceInfo = GetRaceInfo(client, race);
        if (raceInfo != INVALID_HANDLE)
        {
            Trace("Reset %d:%N's race %d XP=%d", \
                  client, ValidClientIndex(client), \
                  race, xp);

            SetRaceInfoXP(raceInfo,xp);
            SetDatabaseSaved(client, false);

            new Handle:raceHandle=GetRaceHandle(race);
            if (raceHandle != INVALID_HANDLE)
            {
                if (GetRaceInfoLevel(raceInfo) < GetRaceMaxLevel(raceHandle))
                    DoLevelCheck(client);
            }
        }
    }

    TraceReturn();
}

GetLevel(client,race)
{
    TraceInto("PlayerTracking", "GetLevel", "client=%d:%N, race=%d", \
              client, ValidClientIndex(client), race);

    new level = 0;
    if (race >= 0 && GetRaceCount() > 1)
    {
        new Handle:raceInfo = GetRaceInfo(client, race);
        if (raceInfo != INVALID_HANDLE)
            level = GetRaceInfoLevel(raceInfo);
    }

    TraceReturn("Get %d:%N's race %d level=%d", \
                client, ValidClientIndex(client), \
                race, level);

    return level;
}

SetLevel(client,race,level,bool:update=false)
{
    TraceInto("PlayerTracking", "SetLevel", "client=%d:%N, race=%d, level=%d", \
              client, ValidClientIndex(client), race, level);

    new delta = 0;
    if (race >= 0 && GetRaceCount() > 1)
    {
        new Handle:raceInfo = GetRaceInfo(client, race);
        if (raceInfo != INVALID_HANDLE)
        {
            new oldLevel = GetRaceInfoLevel(raceInfo);

            Trace("Set %d:%N's race %d level=%d", \
                  client, ValidClientIndex(client), \
                  race, level);

            delta = level-oldLevel;
            if (delta != 0)
            {
                SetRaceInfoLevel(raceInfo,level);
                SetDatabaseSaved(client, false);

                if (update)
                {
                    new oLevel = GetOverallLevel(client);
                    SetOverallLevel(client,oLevel+delta);

                    new Handle:raceHandle=GetRaceHandle(race);
                    if (raceHandle != INVALID_HANDLE)
                    {
                        new Faction:techId = GetRaceFaction(raceHandle);
                        if (techId > Generic)
                        {
                            IncrementTechLevel(client,techId,delta);
                            if (oldLevel <= 0 && level > 0)
                                IncrementTechCount(client,techId,1);
                            else if (oldLevel > 0 && level <= 0)
                                IncrementTechCount(client,techId,-1);
                        }
                    }
                }
            }
        }
    }

    TraceReturn("delta=%d", delta);
    return delta;
}

ResetLevel(client,race,new_level)
{
    TraceInto("PlayerTracking", "ResetLevel", "client=%d:%N, race=%d, new_level=%d", \
              client, ValidClientIndex(client), race, new_level);

    new delta = SetLevel(client,race,new_level,true);
    if (delta < 0)
    {
        ResetUpgradesNow(client,race);
        if (new_level > 0 && GetDatabaseLoaded(client) != DataLoading)
            UpgradeMenu(client, UpgradeMode);
    }
    else if (delta > 0 && GetDatabaseLoaded(client) != DataLoading)
        UpgradeMenu(client, UpgradeMode);

    TraceReturn();
}

GetAccumulatedEnergy(client,race)
{
    TraceInto("PlayerTracking", "GetAccumulatedEnergy", "client=%d:%N, race=%d", \
              client, ValidClientIndex(client), race);

    new accumulated_energy=0;
    if (race >= 0 && GetRaceCount() > 1)
    {
        new Handle:raceInfo = GetRaceInfo(client, race);
        if (raceInfo != INVALID_HANDLE)
            accumulated_energy = GetRaceInfoAccumulatedEnergy(raceInfo);
    }

    TraceReturn("accumulated_energy=%d", accumulated_energy);
    return accumulated_energy;
}

SetAccumulatedEnergy(client,race,energy)
{
    TraceInto("PlayerTracking", "SetAccumulatedEnergy", "client=%d:%N, race=%d, energy=%d", \
              client, ValidClientIndex(client), race, energy);

    if (race >= 0 && GetRaceCount() > 1)
    {
        new Handle:raceInfo = GetRaceInfo(client, race);
        if (raceInfo != INVALID_HANDLE)
            SetRaceInfoAccumulatedEnergy(raceInfo,energy);
    }

    TraceReturn();
}

IncrementAccumulatedEnergy(client,race,amount)
{
    TraceInto("PlayerTracking", "IncrementAccumulatedEnergy", "client=%d:%N, race=%d, amount=%d", \
              client, ValidClientIndex(client), race, amount);

    if (race >= 0 && GetRaceCount() > 1)
    {
        new Handle:raceInfo = GetRaceInfo(client, race);
        if (raceInfo != INVALID_HANDLE)
        {
            new energy = GetRaceInfoAccumulatedEnergy(raceInfo);
            SetRaceInfoAccumulatedEnergy(raceInfo,energy+amount);
        }
    }

    TraceReturn();
}

GetActiveUltimate(client,race,category)
{
    TraceInto("PlayerTracking", "GetActiveUltimate", "client=%d:%N, race=%d, category=%d", \
              client, ValidClientIndex(client), race, category);

    new upgrade=-1;
    if (category >= 1 && category <= MAX_ULTIMATE_CATEGORY &&
        race >= 0 && GetRaceCount() > 1)
    {
        new Handle:raceInfo = GetRaceInfo(client, race);
        if (raceInfo != INVALID_HANDLE)
        {
            new pos; // = RACEINFO_ACTIVE_ULTIMATE_1+category-1;
            switch (category)
            {
                case 1: pos=RACEINFO_ACTIVE_ULTIMATE_1;
                case 2: pos=RACEINFO_ACTIVE_ULTIMATE_2;
                case 3: pos=RACEINFO_ACTIVE_ULTIMATE_3;
                case 4: pos=RACEINFO_ACTIVE_ULTIMATE_4;
            }
            upgrade = GetArrayCell(raceInfo,pos);
        }
    }

    TraceReturn("upgrade=%d", upgrade);
    return upgrade;
}

SetActiveUltimate(client,race,category,upgrade)
{
    TraceInto("PlayerTracking", "SetActiveUltimate", "client=%d:%N, race=%d, category=%d, upgrade=%d", \
              client, ValidClientIndex(client), race, category, upgrade);

    if (category >= 1 && category <= MAX_ULTIMATE_CATEGORY &&
        race >= 0 && GetRaceCount() > 1)
    {
        new Handle:raceInfo = GetRaceInfo(client, race);
        if (raceInfo != INVALID_HANDLE)
        {
            new pos; // = RACEINFO_ACTIVE_ULTIMATE_1+category-1;
            switch (category)
            {
                case 1: pos=RACEINFO_ACTIVE_ULTIMATE_1;
                case 2: pos=RACEINFO_ACTIVE_ULTIMATE_2;
                case 3: pos=RACEINFO_ACTIVE_ULTIMATE_3;
                case 4: pos=RACEINFO_ACTIVE_ULTIMATE_4;
            }
            SetArrayCell(raceInfo,pos,upgrade);
        }
    }

    TraceReturn();
}

DataStatus:GetRaceLoaded(client,race)
{
    TraceInto("PlayerTracking", "GetRaceLoaded", "client=%d:%N, race=%d", \
              client, ValidClientIndex(client), race);

    new DataStatus:loaded = DataNotLoaded;
    if (race >= 0 && GetRaceCount() > 1)
    {
        new Handle:raceInfo = GetRaceInfo(client, race);
        if (raceInfo != INVALID_HANDLE)
            loaded = GetRaceInfoLoaded(raceInfo);
    }

    TraceReturn("loaded=%d", loaded);
    return loaded;
}

stock SetRaceLoaded(client,race,DataStatus:loaded)
{
    TraceInto("PlayerTracking", "SetRaceLoaded", "client=%d:%N, race=%d, loaded=%d", \
              client, ValidClientIndex(client), race, loaded);

    if (race >= 0 && GetRaceCount() > 1)
    {
        new Handle:raceInfo = GetRaceInfo(client, race);
        if (raceInfo != INVALID_HANDLE)
            SetRaceInfoLoaded(raceInfo,loaded);
    }

    TraceReturn();
}

stock GetSavedXP(client,race)
{
    TraceInto("PlayerTracking", "GetSavedXP", "client=%d:%N, race=%d", \
              client, ValidClientIndex(client), race);

    new xp = 0;
    if (race >= 0 && GetRaceCount() > 1)
    {
        new Handle:raceInfo = GetRaceInfo(client, race);
        if (raceInfo != INVALID_HANDLE)
            xp = GetRaceInfoSavedXP(raceInfo);
    }

    TraceReturn("Get %d:%N's race %d SavedXP=%d", \
                client, ValidClientIndex(client), \
                race, xp);

    return xp;
}

stock SetSavedXP(client,race,xp)
{
    TraceInto("PlayerTracking", "SetSavedXP", "client=%d:%N, race=%d, xp=%d", \
              client, ValidClientIndex(client), race, xp);

    if (race >= 0 && GetRaceCount() > 1)
    {
        new Handle:raceInfo = GetRaceInfo(client, race);
        if (raceInfo != INVALID_HANDLE)
        {
            Trace("Set %d:%N's race %d SavedXP=%d", \
                  client, ValidClientIndex(client), \
                  race, xp);

            SetRaceInfoSavedXP(raceInfo,xp);
        }
    }

    TraceReturn();
}

GetUpgradeLevel(client,race,upgrade)
{
    TraceInto("PlayerTracking", "GetUpgradeLevel", "client=%d:%N, race=%d, upgrade=%d", \
              client, ValidClientIndex(client), race, upgrade);

    new level = 0;
    if (race >= 0 && GetRaceCount() > 1)
    {
        new Handle:raceInfo = GetRaceInfo(client, race);
        if (raceInfo != INVALID_HANDLE)
        {
            new upgradePos = GetRaceInfoUpgradePos(upgrade);
            new upgradeSize = GetArraySize(raceInfo);
            if (upgradePos < upgradeSize)
                level = GetArrayCell(raceInfo,upgradePos); // +UPGRADEINFO_LEVEL
            else
            {
                new Handle:raceHandle=GetRaceHandle(race);
                if (raceHandle != INVALID_HANDLE)
                {
                    new upgradeCount = (upgradeSize-RACEINFO_SIZE)/UPGRADEINFO_SIZE;
                    decl String:rname[64]; GetRaceName(raceHandle,rname,sizeof(rname));
                    LogError("GetUpgradeLevel of Invalid upgrade %d (pos %d) of %d(%d) for race %d(%s)",
                             upgrade, upgradePos, upgradeSize, upgradeCount, race, rname);

                    // For now, Tack on extra upgrade slot
                    PushArrayCell(raceInfo,0); // Upgrade level
                    PushArrayCell(raceInfo,0); // Upgrade level in DB
                }
            }
        }
    }

    TraceReturn("level=%d", level);
    return level;
}

SetUpgradeLevel(client,race,upgrade,level)
{
    TraceInto("PlayerTracking", "SetUpgradeLevel", "client=%d:%N, race=%d, upgrade=%d, level=%d", \
              client, ValidClientIndex(client), race, upgrade, level);

    if (race >= 0 && GetRaceCount() > 1 && IsClientInGame(client))
    {
        new Handle:raceInfo = GetRaceInfo(client, race);
        if (raceInfo != INVALID_HANDLE)
        {
            new old = 0;
            new upgradePos = GetRaceInfoUpgradePos(upgrade);
            new upgradeSize = GetArraySize(raceInfo);
            if (upgradePos < GetArraySize(raceInfo))
            {
                old=GetArrayCell(raceInfo,upgradePos); // +UPGRADEINFO_LEVEL
                if (level != old)
                {
                    SetArrayCell(raceInfo,upgradePos,level);
                    SetDatabaseSaved(client, false);
                }
            }
            else
            {
                LogError("SetUpgradeLevel of Invalid upgrade %d (pos %d) for race %d to %d. size=%d",
                         upgrade, upgradePos, race, level, upgradeSize);

                while (upgradeSize < upgradePos)
                {
                    PushArrayCell(raceInfo,(upgradeSize >= upgradePos - 2) ? level : 0); // Upgrade level
                    PushArrayCell(raceInfo,0); // Upgrade level in DB
                    upgradeSize = GetArraySize(raceInfo);
                }
            }

            if (level != old)
            {
                new result;
                Call_StartForward(g_OnUpgradeLevelChangedHandle);
                Call_PushCell(client);
                Call_PushCell(race);
                Call_PushCell(upgrade);
                Call_PushCell(level);
                Call_Finish(result);
            }
        }
    }

    TraceReturn();
}

bool:IncrementUpgradeLevel(client,race,upgrade,category,max)
{
    TraceInto("PlayerTracking", "IncrementUpgradeLevel", "client=%d:%N, race=%d, upgrade=%d, category=%d", \
              client, ValidClientIndex(client), race, upgrade, category);

    new bool:success=false;
    if (race >= 0 && GetRaceCount() > 1 && IsClientInGame(client))
    {
        new Handle:raceInfo = GetRaceInfo(client, race);
        if (raceInfo != INVALID_HANDLE)
        {
            new upgradePos = GetRaceInfoUpgradePos(upgrade);
            new old = GetArrayCell(raceInfo,upgradePos); // +UPGRADEINFO_LEVEL
            if (old < max)
            {
                new level=old+1;
                SetArrayCell(raceInfo,upgradePos,level);
                SetDatabaseSaved(client, false);
                success = true;

                if (category >= 1 && category <= MAX_ULTIMATE_CATEGORY)
                {
                    new pos; // = RACEINFO_ACTIVE_ULTIMATE_1+category-1;
                    switch (category)
                    {
                        case 1: pos=RACEINFO_ACTIVE_ULTIMATE_1;
                        case 2: pos=RACEINFO_ACTIVE_ULTIMATE_2;
                        case 3: pos=RACEINFO_ACTIVE_ULTIMATE_3;
                        case 4: pos=RACEINFO_ACTIVE_ULTIMATE_4;
                    }
                    SetArrayCell(raceInfo,pos,upgrade);
                }

                new result;
                Call_StartForward(g_OnUpgradeLevelChangedHandle);
                Call_PushCell(client);
                Call_PushCell(race);
                Call_PushCell(upgrade);
                Call_PushCell(level);
                Call_Finish(result);
            }
        }
    }

    TraceReturn("success=%d",success);
    return success;
}

LoadUpgradeLevel(client,race,upgrade,level)
{
    TraceInto("PlayerTracking", "LoadUpgradeLevel", "client=%d:%N, race=%d, upgrade=%d, level=%d", \
              client, ValidClientIndex(client), race, upgrade, level);

    if (race >= 0 && GetRaceCount() > 1 && IsClientInGame(client))
    {
        new Handle:raceInfo = GetRaceInfo(client, race);
        if (raceInfo != INVALID_HANDLE)
        {
            new upgradePos = GetRaceInfoUpgradePos(upgrade);
            if (upgradePos >= GetArraySize(raceInfo))
            {
                new Handle:raceHandle=GetRaceHandle(race);
                if (raceHandle != INVALID_HANDLE)
                {
                    new ident = GetRaceIdent(raceHandle);
                    decl String:name[64]; GetRaceName(raceHandle,name,sizeof(name));
                    LogError("Unable to Set %d:%N's %d-%d:%s Upgrade %d Level=%d, upgradePos=%d of %d == FUBAR!",
                             client, ValidClientIndex(client), race, ident, name,
                             upgrade, level, upgradePos, GetArraySize(raceInfo));
                }
            }
            else
            {
                new old = GetArrayCell(raceInfo,upgradePos); // +UPGRADEINFO_LEVEL
                if (old != level)
                {
                    SetArrayCell(raceInfo,upgradePos,level); // +UPGRADEINFO_LEVEL
                    SetArrayCell(raceInfo,upgradePos+UPGRADEINFO_SAVED_LEVEL,level);

                    new result;
                    Call_StartForward(g_OnUpgradeLevelChangedHandle);
                    Call_PushCell(client);
                    Call_PushCell(race);
                    Call_PushCell(upgrade);
                    Call_PushCell(level);
                    Call_Finish(result);
                }
            }
        }
    }

    TraceReturn();
}

GetUpgradeLevelCount(client,race)
{
    TraceInto("PlayerTracking", "GetUpgradeLevelCount", "client=%d:%N, race=%d", \
              client, ValidClientIndex(client), race);

    new ret = 0;
    if (race >= 0 && GetRaceCount() > 1)
    {
        new Handle:raceInfo = GetRaceInfo(client, race);
        if (raceInfo != INVALID_HANDLE)
        {
            new Handle:raceHandle = GetRaceHandle(race);
            if (raceHandle != INVALID_HANDLE)
            {
                new upgradeCount = GetUpgradeCount(raceHandle);
                new last = GetRaceInfoUpgradePos(upgradeCount);
                for(new x = RACEINFO_SIZE; x < last; x += UPGRADEINFO_SIZE)
                    ret += GetArrayCell(raceInfo,x); // +UPGRADEINFO_LEVEL
            }
        }
    }

    TraceReturn("count=%d",ret);
    return ret;
}

stock GetSavedUpgradeLevel(client,race,upgrade)
{
    TraceInto("PlayerTracking", "GetSavedUpgradeLevel", "client=%d:%N(%d), race=%d, upgrade=%d", \
              ValidClientIndex(client), client, race, upgrade);

    new ret=0;
    if (race >= 0 && GetRaceCount() > 1)
    {
        new Handle:raceInfo = GetRaceInfo(client, race);
        if (raceInfo != INVALID_HANDLE)
        {
            new upgradePos = GetRaceInfoUpgradePos(upgrade);
            ret = GetArrayCell(raceInfo,upgradePos+UPGRADEINFO_SAVED_LEVEL);
        }
    }

    TraceReturn("saved_level=%d",ret);
    return ret;
}

stock SetSavedUpgradeLevel(client,race,upgrade,level)
{
    TraceInto("PlayerTracking", "SetSavedUpgradeLevel", "client=%d:%N, race=%d, upgrade=%d,level=%d", \
              client, ValidClientIndex(client), race, upgrade, level);

    if (race >= 0 && GetRaceCount() > 1)
    {
        new Handle:raceInfo = GetRaceInfo(client, race);
        if (raceInfo != INVALID_HANDLE)
        {
            new upgradePos = GetRaceInfoUpgradePos(upgrade);
            SetArrayCell(raceInfo,upgradePos+UPGRADEINFO_SAVED_LEVEL,level);
            SetDatabaseSaved(client, false);
        }
    }

    TraceReturn();
}

DataStatus:GetTechLoaded(client,Faction:techId)
{
    TraceInto("PlayerTracking", "GetTechLoaded", "client=%d:%N, techId=%d", \
              client, ValidClientIndex(client), techId);

    new DataStatus:loaded = DataNotLoaded;
    if (techId >= Generic)
    {
        new Handle:techInfo=GetTechInfo(client, techId);
        if (techInfo != INVALID_HANDLE)
            loaded = GetTechInfoLoaded(techInfo);
    }

    TraceReturn("loaded=%d", loaded);
    return loaded;
}

stock SetTechLoaded(client,Faction:techId,loaded)
{
    TraceInto("PlayerTracking", "SetTechLoaded", "client=%d:%N, techId=%d, loaded=%d", \
              client, ValidClientIndex(client), techId, loaded);

    if (techId >= Generic)
    {
        new Handle:techInfo=GetTechInfo(client, techId);
        if (techInfo != INVALID_HANDLE)
            SetTechInfoLoaded(techInfo,loaded);
    }

    TraceReturn();
}

stock GetTechCount(client,Faction:techId)
{
    TraceInto("PlayerTracking", "GetTechCount", "client=%d:%N, techId=%d", \
              client, ValidClientIndex(client), techId);

    new count = 0;
    if (techId >= Generic)
    {
        new Handle:techInfo=GetTechInfo(client, techId);
        if (techInfo != INVALID_HANDLE)
            count = GetTechInfoCount(techInfo);
    }

    TraceReturn("Get %d:%N's tech %d count=%d", \
                client, ValidClientIndex(client), \
                techId, count);

    return count;
}

stock SetTechCount(client,Faction:techId, count)
{
    TraceInto("PlayerTracking", "GetTechCount", "client=%d:%N, techId=%d", \
              client, ValidClientIndex(client), techId);

    if (techId >= Generic)
    {
        new Handle:techInfo=GetTechInfo(client, techId);
        if (techInfo != INVALID_HANDLE)
            SetTechInfoCount(techInfo, count);
    }

    TraceReturn();
}

IncrementTechCount(client,Faction:techId, amount)
{
    TraceInto("PlayerTracking", "GetTechCount", "client=%d:%N, techId=%d", \
              client, ValidClientIndex(client), techId);

    if (techId >= Generic)
    {
        new Handle:techInfo=GetTechInfo(client, techId);
        if (techInfo != INVALID_HANDLE)
        {
            new count = GetTechInfoCount(techInfo);
            SetTechInfoCount(techInfo, count+amount);
        }
    }

    TraceReturn();
}

GetTechLevel(client,Faction:techId)
{
    TraceInto("PlayerTracking", "GetTechLevel", "client=%d:%N, techId=%d", \
              client, ValidClientIndex(client), techId);

    new level = 0;
    if (techId >= Generic)
    {
        new Handle:techInfo=GetTechInfo(client, techId);
        if (techInfo != INVALID_HANDLE)
            level = GetTechInfoLevel(techInfo);
    }

    TraceReturn("Get %d:%N's tech %d level=%d", \
                client, ValidClientIndex(client), \
                techId, level);

    return level;
}

stock SetTechLevel(client,Faction:techId, level)
{
    TraceInto("PlayerTracking", "GetTechLevel", "client=%d:%N, techId=%d", \
              client, ValidClientIndex(client), techId);

    if (techId >= Generic)
    {
        new Handle:techInfo=GetTechInfo(client, techId);
        if (techInfo != INVALID_HANDLE)
            SetTechInfoLevel(techInfo, level);
    }

    TraceReturn();
}

IncrementTechLevel(client,Faction:techId, amount)
{
    TraceInto("PlayerTracking", "GetTechLevel", "client=%d:%N, techId=%d", \
              client, ValidClientIndex(client), techId);

    if (techId >= Generic)
    {
        new Handle:techInfo=GetTechInfo(client, techId);
        if (techInfo != INVALID_HANDLE)
        {
            new level = GetTechInfoLevel(techInfo);
            SetTechInfoLevel(techInfo, level+amount);
        }
    }

    TraceReturn();
}

bool:SetRace(client,race)
{
    TraceInto("PlayerTracking", "SetRace", "client=%d:%N, race=%d", \
              client, ValidClientIndex(client), race);

    new oldrace = GetRace(client);
    if (race >= 0 && race != oldrace)
    {
        Trace("Set %d:%N's race=%d", \
              client,ValidClientIndex(client), \
              race);

        new bool:saveEnabled = (g_bSaveXP && GetRaceCount() > 1 &&
                                GetPlayerStatus(client) != PlayerDisabled);

        if (oldrace > 0 && saveEnabled && !GetDatabaseSaved(client) &&
            GetDatabaseLoaded(client) >= DataOK &&
            GetRaceLoaded(client, oldrace) >= DataOK)
        {
            if (g_bDatabaseConnected)
            {
                Trace("Save %d:%N's race=%d Data", \
                      client,ValidClientIndex(client), \
                      race);

                SavePlayerData(client);
            }
            else
            {
                LogError("Database not available to save %N's levels", client);
                PrintHintText(client, "%t", "NoDatabaseForSave");
            }
        }

        SetRaceValue(client, race);
        SetDatabaseSaved(client, false);

        if (saveEnabled && GetRaceLoaded(client,race) < DataOK)
        {
            if (g_bDatabaseConnected)
            {
                Trace("Load %d:%N's race=%d Data", \
                      client, ValidClientIndex(client), \
                      race);

                LoadPlayerRaceData(client, race);
            }
            else
            {
                LogError("Database not available to load %N's levels", client);
                PrintHintText(client, "%t", "NoDatabaseForLoad");
            }
        }

        if (oldrace > 0)
        {
            Trace("%d:%N (client=%d) Deselected/Changing from %d to %d", \
                  client, ValidClientIndex(client), client, oldrace, race);

            Call_StartForward(g_OnRaceDeselectedHandle);
            Call_PushCell(client);
            Call_PushCell(oldrace);
            Call_PushCell(race);

            new Action:result = Plugin_Continue;
            Call_Finish(result);
            if (result >= Plugin_Stop)
            {
                // Someone rejected the change, set the race back.
                SetRaceValue(client, oldrace);
                TraceReturn("%d:%N Deselect failed, Remaining a %d", \
                            client, ValidClientIndex(client), oldrace);
                return false;
            }
        }

        Trace("%d:%N Selected/Changing from %d to %d", \
              client, ValidClientIndex(client), oldrace, race);

        Call_StartForward(g_OnRaceSelectedHandle);
        Call_PushCell(client);
        Call_PushCell(oldrace);
        Call_PushCell(race);

        new Action:result = Plugin_Continue;
        Call_Finish(result);
        if (result >= Plugin_Stop)
        {
            // Someone rejected the change, set the race back.
            SetRaceValue(client, oldrace);
            TraceReturn("%d:%N Select failed, Remaining a %d", \
                        client, ValidClientIndex(client), oldrace);
            return false;
        }

        // Reset accumulated energy when changing races
        new Handle:raceInfo = GetRaceInfo(client, race);
        if (raceInfo != INVALID_HANDLE)
            SetRaceInfoAccumulatedEnergy(raceInfo,0);

        // Set the energy to the initial amount of the new race.
        new initial = GetInitialEnergy(client);

        Trace("%d:%N's initial energy is %d", \
              client, ValidClientIndex(client), initial);

        new Handle:raceHandle=GetRaceHandle(race);
        if (raceHandle != INVALID_HANDLE && initial < 0)
        {
            initial = GetRaceInitialEnergy(raceHandle);

            #if defined _TRACE
                decl String:rname[64]; GetRaceName(GetRaceHandle(race),rname,sizeof(rname));
                Trace("Race %d(0x%08x)'s initial energy is %d", race, raceHandle, initial);
            #endif
        }

        if (initial < 0)
            initial = 0;

        SetEnergy(client, initial);

        Trace("Setting %d:%N's energy to %d", \
              client, ValidClientIndex(client), initial);

        // Setup racial immunities (vulnerabilities)
        new UnitType:type = (raceHandle != INVALID_HANDLE) ? GetRaceType(raceHandle) : GenericUnit;
        SetAttribute(client,Attribute_IsBiological, type == Biological || type == BioMechanical || type == Cybernetic || type == GenericUnit);
        SetAttribute(client,Attribute_IsMechanical, type == Mechanical || type == BioMechanical || type == Cybernetic);
        SetAttribute(client,Attribute_IsRobotic, type == Robotic || type == Cybernetic);
        SetAttribute(client,Attribute_IsEnergy, type == Energy);

        CooldownEvent(client, Cooldown_ResetOnChange);

        #if defined _TRACE
            // Ensure the race data was loaded!
            if (GetRaceLoaded(client,race) < DataOK)
            {
                new ident = GetDatabaseIdent(client);
                if (ident >= 0)
                {
                    Trace("%d:%N was found using ident %d, but their %d race data was either not found or not loaded.", \
                          client, ValidClientIndex(client), race, ident);
                }
                else
                {
                    Trace("%d:%N was not in the Database, so their %d race data was not loaded.", \
                          client, ValidClientIndex(client), race);
                }
            }
        #endif

        TraceReturn("%d:%N Changed from %d to %d", \
                    client, ValidClientIndex(client), oldrace, race);
        return true;
    }

    TraceReturn();
    return false;
}

Handle:GetPropInfo(Handle:propData, Handle:plugin, id)
{
    new size=GetArraySize(propData);
    for (new x=0;x<size;x++)
    {
        new Handle:propInfo=GetArrayCell(propData,x);
        if (GetPropInfoPlugin(propInfo) == plugin &&
            GetPropInfoId(propInfo) == id)
        {
            return propInfo;
        }
    }
    return INVALID_HANDLE;
}

Handle:CreateProperties(client, Handle:plugin, id, Float:forceSpeed=-1.0, Float:forceGravity=-1.0,
                        forceVisibility=-1, Float:speed=-1.0, Float:gravity=-1.0, visibility=-1,
                        VisibilityType:type=VisibilityType:-1, Float:factor1=-1.0, Float:factor2=-1.0,
                        RenderMode:mode=RenderMode:-1, RenderFx:fx=RenderFx:-1, r=-1, g=-1, b=-1,
                        bool:colorWeapons=false, VisibilityPriority:priority=VisibilityPriorityNotSpecified,
                        Float:timestamp=0.0, Immunity:immunities=Immunity_None, MoveType:mt=MOVETYPE_DEFAULT,
                        Float:physicalArmorPercent=0.0, Float:magicalArmorPercent=0.0)
{
    #if defined _TRACE
        SetTraceCategory("Array");
        TraceInto("PlayerTracking", "CreateProperties", "client=%d:%N, plugin=0x%08x,id=%d",
                  client, ValidClientIndex(client), plugin, id);
    #else
        #pragma unused client
    #endif

    new Handle:pHandle=CreateArray();
    PushArrayCell(pHandle, plugin);                 // plugin handle
    PushArrayCell(pHandle, id);                     // identifier
    PushArrayCell(pHandle, forceSpeed);             // force speed?
    PushArrayCell(pHandle, forceGravity);           // force gravity?
    PushArrayCell(pHandle, forceVisibility);        // force visibility?
    PushArrayCell(pHandle, speed);                  // speed
    PushArrayCell(pHandle, gravity);                // gravity
    PushArrayCell(pHandle, visibility);             // visibility 
    PushArrayCell(pHandle, type);                   // visibility type
    PushArrayCell(pHandle, factor1);                // visibility factor1 (melee/delay)
    PushArrayCell(pHandle, factor2);                // visibility factor2 (movement/duration)
    PushArrayCell(pHandle, mode);                   // visibility render mode
    PushArrayCell(pHandle, fx);                     // visibility render fx
    PushArrayCell(pHandle, r);                      // visibility red component
    PushArrayCell(pHandle, g);                      // visibility green component
    PushArrayCell(pHandle, b);                      // visibility blue component
    PushArrayCell(pHandle, colorWeapons);           // visibility color weapons flag
    PushArrayCell(pHandle, priority);               // visibility priority
    PushArrayCell(pHandle, timestamp);              // visibility timestamp
    PushArrayCell(pHandle, immunities);             // immunities
    PushArrayCell(pHandle, mt);                     // move type
    PushArrayCell(pHandle, physicalArmorPercent);   // Physical Armor Percent (War3Source attribute)
    PushArrayCell(pHandle, magicalArmorPercent);    // Magical Armor Percent (War3Source attribute)

    #if defined _TRACE
        TraceReturn("CreateArray playerInfo[%d,0x%08x,%d]=0x%08x", \
                    client, plugin, id, pHandle);
    #endif

    return pHandle;
}

ResetPlayer(client, bool:clear)
{
    TraceInto("PlayerTracking", "ResetPlayer", "client=%d:%N", \
              client, ValidClientIndex(client));

    SetArmor(client, 0);
    SetShields(client, false);
    SetChangingClass(client, false);
    SetSettingsBits(client, NotSet);

    SetImmunityBits(client, Immunity_None);     // Player's immunities
    SetAttributeBits(client, Attribute_None);   // Player's attributes
    SetRaceValue(client, -1);                   // Player race (set to pending value)
    SetEnergy(client, 0);                       // Energy
    SetEnergyRate(client, -1);                  // Energy Rate
    SetEnergyLimit(client, -1);                 // Energy Limit
    SetInitialEnergy(client, -1);               // Initial Energy
    SetCrystalsValue(client, 0);                // Crystals
    SetVespeneValue(client, 0);                 // Vespene
    SetPendingRace(client, -1);                 // Pending race
    SetPendingUpgradeReset(client, 0);          // Pending upgrade reset
    SetOverallLevelValue(client, 0);            // Overall Level
    SetDatabaseIdent(client, -1);               // Database Ident
    SetDatabaseLoaded(client, DataNotLoaded); // Loaded from Database Flag
    SetDatabaseSaved(client, false);            // Saved to Database Flag
    SetDatabaseSaveTime(client, 0.0);           // Time of last Save/Load to/from Database
    SetPlayerStatus(client, PlayerStatus_Unknown);    // Player Status

    // Get The Player's ID (for debugging)
    playerID[client][0] ='\0';
    GetClientAuthString(client,playerID[client],sizeof(playerID[]));

    // Get The Player's Name (for debugging)
    playerName[client][0] ='\0';
    GetClientName(client,playerName[client],sizeof(playerName[]));

    // Reset The Player's Prev Name
    playerPrevName[client][0] ='\0';

    // Reset the Armor Name
    m_ArmorName[client][0] = '\0';

    if (clear)
        ClearPlayer(client);

    // Player's properties
    new Handle:propData=GetPropData(client);
    if (propData == INVALID_HANDLE)
    {
        propData = SetPropData(client, CreateArray()); // Player's properties
        // No need to initialize properties elements.

        TraceCat("Array", "CreateArray playerArray[%d,Properties]=0x%08x", \
                 client, propData);
    }
    else
    {
        new numprops=GetArraySize(propData);
        for (new x=0;x<numprops;x++)
        {
            new Handle:propInfo=GetArrayCell(propData,x);
            if (propInfo != INVALID_HANDLE)
            {
                //SetPropInfoPlugin(propInfo,plugin);                   // plugin handle
                //SetPropInfoId(propInfo,id);                           // identifier
                SetPropInfoOverrideSpeed(propInfo,-1.0);                // force speed?
                SetPropInfoOverrideGravity(propInfo,-1.0);              // force gravity?
                SetPropInfoOverrideVisibility(propInfo,-1);             // force visibility?
                SetPropInfoSpeed(propInfo,-1.0);                        // speed
                SetPropInfoGravity(propInfo,-1.0);                      // gravity
                SetPropInfoVisibility(propInfo,-1);                     // visibility 
                SetPropInfoVisibilityType(propInfo,DefaultVisibility);  // visibility type
                SetPropInfoVisibilityFactor1(propInfo,-1.0);            // visibility factor1 (melee/delay)
                SetPropInfoVisibilityFactor2(propInfo,-1.0);            // visibility factor2 (movement/duration)
                SetPropInfoVisibilityMode(propInfo,-1);                 // visibility render mode
                SetPropInfoVisibilityFx(propInfo,-1);                   // visibility render fx
                SetPropInfoVisibilityR(propInfo,-1);                    // visibility red component
                SetPropInfoVisibilityG(propInfo,-1);                    // visibility green component
                SetPropInfoVisibilityB(propInfo,-1);                    // visibility blue component
                SetPropInfoVisibilityWeapons(propInfo,false);           // visibility color weapons flag
                SetPropInfoVisibilityPriority(propInfo,VisibilityPriorityNotSpecified); // visibility priority
                SetPropInfoVisibilityTime(propInfo,0.0);                // visibility timestamp
                SetPropInfoImmunities(propInfo,0);                      // immunities
                SetPropInfoMoveType(propInfo,MOVETYPE_DEFAULT);         // move type
                SetPropInfoPhysicalArmor(propInfo,0.0);                 // Physical Armor Percent (War3Source attribute)
                SetPropInfoMagicalArmor(propInfo,0.0);                  // Magical Armor Percent (War3Source attribute)
            }
        }
    }

    // Player's shop items
    new shopItemCount;
    new shopVectorCount=GetShopItemCount();
    new Handle:shopitems=GetShopitems(client);
    if (shopitems == INVALID_HANDLE)
    {
        shopitems = SetShopitems(client, CreateArray());

        for(new x=0;x<shopVectorCount;x++)
            PushArrayCell(shopitems,0);     // Owns item x

        shopItemCount=GetArraySize(shopitems);

        TraceCat("Array", "CreateArray playerArray[%d,ShopItems]=0x%08x", \
                 client, shopitems);
    }
    else
    {
        shopItemCount=GetArraySize(shopitems);
        for (new x=0;x<shopItemCount;x++)
            SetArrayCell(shopitems,x,0);     // Owns item x
    }

    // Perform a sanity check.
    if (shopItemCount != shopVectorCount)
    {
        LogError("ERROR: Player %d:%N's shop data got fubared! %d != %d",
                client, ValidClientIndex(client), shopItemCount, shopVectorCount);

        if (!clear)
        {
            ResetPlayer(client, true);
            TraceReturn();
            return;
        }
    }

    // Player's race data
    new raceDataCount;
    new raceCount = GetRaceCount();
    new Handle:raceData=GetRaceData(client);
    if (raceData == INVALID_HANDLE)
    {
        raceData = SetRaceData(client, CreateArray()); // Player's race data

        TraceCat("Array", "CreateArray playerArray[%d,RaceData]=0x%08x", \
                 client, raceData);

        for (new race=0;race<raceCount;race++)
        {
            new Handle:raceInfo=CreateArray(); // Player's races
            PushArrayCell(raceInfo,0); // Race's XP
            PushArrayCell(raceInfo,0); // Race's Level
            PushArrayCell(raceInfo,0); // Race's Accumulated Energy
            PushArrayCell(raceInfo,-1); // Race's Active Ultimate 1 Upgrade 
            PushArrayCell(raceInfo,-1); // Race's Active Ultimate 2 Upgrade 
            PushArrayCell(raceInfo,-1); // Race's Active Ultimate 3 Upgrade 
            PushArrayCell(raceInfo,-1); // Race's Active Ultimate 4 Upgrade 
            PushArrayCell(raceInfo,DataNotLoaded); // Race Not Loaded
            PushArrayCell(raceInfo,0); // Race's Saved XP
            PushArrayCell(raceData,raceInfo);

            TraceCat("Array", "CreateArray playerRaceInfo[%d,%d]=0x%08x", \
                     client, race, raceInfo);

            new Handle:raceHandle=GetRaceHandle(race);
            new upgradeCount = (raceHandle != INVALID_HANDLE)
                               ? GetUpgradeCount(raceHandle) : 0;
            for(new upgrade=0;upgrade<upgradeCount;upgrade++)
            {
                PushArrayCell(raceInfo,0); // Upgrade level
                PushArrayCell(raceInfo,-1); // Saved level in DB
            }

            // Perform a sanity check of the race info.
            if (GetArraySize(raceInfo) != GetRaceInfoUpgradePos(upgradeCount))
            {
                decl String:rname[64]; GetRaceName(GetRaceHandle(race),rname,sizeof(rname));
                LogError("ERROR: Player %d:%N's race %s(%d) data got fubared! %d != %d",
                         client, ValidClientIndex(client), rname, race,
                         GetArraySize(raceInfo), GetRaceInfoUpgradePos(upgradeCount));

                if (!clear)
                {
                    ResetPlayer(client, true);
                    TraceReturn();
                    return;
                }
            }
        }
        raceDataCount=GetArraySize(raceData);
    }
    else
    {
        raceDataCount=GetArraySize(raceData);
        for (new race=0;race<raceDataCount;race++)
        {
            new Handle:raceInfo=GetArrayCell(raceData,race); // Player's races
            if (raceInfo != INVALID_HANDLE)
            {
                SetRaceInfoXP(raceInfo,0); // Race's XP
                SetRaceInfoLevel(raceInfo,0); // Race's Level
                SetRaceInfoAccumulatedEnergy(raceInfo,0); // Race's Accumulated Energy
                SetRaceInfoActiveUltimate1(raceInfo,-1); // Race's Active Ultimate 1 Upgrade 
                SetRaceInfoActiveUltimate2(raceInfo,-1); // Race's Active Ultimate 2 Upgrade 
                SetRaceInfoActiveUltimate3(raceInfo,-1); // Race's Active Ultimate 3 Upgrade 
                SetRaceInfoActiveUltimate4(raceInfo,-1); // Race's Active Ultimate 4 Upgrade 
                SetRaceInfoLoaded(raceInfo,DataNotLoaded); // Race Not Loaded
                SetRaceInfoSavedXP(raceInfo,0); // Race's Saved XP

                new Handle:raceHandle=GetRaceHandle(race);
                new upgradeCount = (raceHandle != INVALID_HANDLE) ? GetUpgradeCount(raceHandle) : 0;
                for (new upgrade=0;upgrade<upgradeCount;upgrade++)
                {
                    new upgradePos = GetRaceInfoUpgradePos(upgrade);
                    SetArrayCell(raceInfo,upgradePos,0); // Upgrade level
                    SetArrayCell(raceInfo,upgradePos+_:UPGRADEINFO_SAVED_LEVEL,-1); // Saved level in DB
                }

                // Perform a sanity check of the race info.
                if (GetArraySize(raceInfo)!=GetRaceInfoUpgradePos(upgradeCount))
                {
                    decl String:rname[64]; GetRaceName(GetRaceHandle(race),rname,sizeof(rname));
                    LogError("ERROR: Player %d:%N's race %d:%s data got fubared! %d != %d",
                             client, ValidClientIndex(client), race, rname,
                             GetArraySize(raceInfo), GetRaceInfoUpgradePos(upgradeCount));

                    if (!clear)
                    {
                        ResetPlayer(client, true);
                        TraceReturn();
                        return;
                    }
                }
            }
        }
    }

    // Perform a sanity check of the race data.
    if (raceDataCount != raceCount)
    {
        LogError("ERROR: Player %d:%N's race data got fubared! %d != %d",
                 client, ValidClientIndex(client), raceDataCount, raceCount);

        if (!clear)
        {
            ResetPlayer(client, true);
            TraceReturn();
            return;
        }
    }

    // Player's tech data

    new techDataCount;
    new Handle:techData=GetTechData(client);
    if (techData == INVALID_HANDLE)
    {
        techData = SetTechData(client, CreateArray()); // Player's tech data

        TraceCat("Array", "CreateArray playerArray[%d,TechData]=0x%08x", \
                 client, techData);

        for (new Faction:techId=Generic;techId<Faction;techId++)
        {
            new Handle:techInfo=CreateArray(); // Player's tech info
            PushArrayCell(techInfo,0); // Tech Count
            PushArrayCell(techInfo,0); // Tech Level
            PushArrayCell(techInfo,DataNotLoaded); // Tech Not Loaded
            PushArrayCell(techData,techInfo);

            TraceCat("Array", "CreateArray playerTechInfo[%d,%d]=0x%08x", \
                     client, techId, techInfo);

            // Perform a sanity check of the tech info.
            if (GetArraySize(techInfo) != TECHINFO_SIZE)
            {
                decl String:tech[NAME_STRING_LENGTH];
                GetFactionName(techId, tech, sizeof(tech));
                LogError("ERROR: Player %d:%N's tech %d:%s data got fubared! %d != %d",
                         client, ValidClientIndex(client), techId, tech,
                         GetArraySize(techInfo), TECHINFO_SIZE);

                if (!clear)
                {
                    ResetPlayer(client, true);
                    TraceReturn();
                    return;
                }
            }
        }
        techDataCount=GetArraySize(techData);
    }
    else
    {
        techDataCount=GetArraySize(techData);
        for (new Faction:techId=Generic;techId<Faction:techDataCount;techId++)
        {
            new Handle:techInfo=GetArrayCell(techData,_:techId); // Player's tech data
            if (techInfo != INVALID_HANDLE)
            {
                SetTechInfoCount(techInfo,0); // Tech Count
                SetTechInfoLevel(techInfo,0); // Tech Level
                SetTechInfoLoaded(techInfo,DataNotLoaded); // Tech Not Loaded

                // Perform a sanity check of the tech info.
                if (GetArraySize(techInfo)!=TECHINFO_SIZE)
                {
                    decl String:tech[NAME_STRING_LENGTH];
                    GetFactionName(techId, tech, sizeof(tech));
                    LogError("ERROR: Player %d:%N's tech %d:%s data got fubared! %d != %d",
                             client, ValidClientIndex(client), techId, tech,
                             GetArraySize(techInfo), TECHINFO_SIZE);

                    if (!clear)
                    {
                        ResetPlayer(client, true);
                        TraceReturn();
                        return;
                    }
                }
            }
        }
    }

    // Perform a sanity check of the tech data.
    if (techDataCount != _:Faction)
    {
        LogError("ERROR: Player %d:%N's tech data got fubared! %d != %d",
                 client, ValidClientIndex(client), techDataCount, Faction);

        if (!clear)
            ResetPlayer(client, true);
    }

    TraceReturn();
}

ClearPlayerArray()
{
    TraceInto("PlayerTracking", "ClearPlayerArray");

    for (new client=0;client<=MaxClients;client++)
        ClearPlayer(client);

    TraceReturn();
}

ClearPlayer(client)
{
    TraceInto("PlayerTracking", "ClearPlayer", "client=%d:%N", \
              client, ValidClientIndex(client));

    new Handle:propData=Handle:GetPropData(client); // Player's properties
    if (propData != INVALID_HANDLE)
    {
        new numprops=GetArraySize(propData);
        for(new x=0;x<numprops;x++)
        {
            new Handle:propInfo=GetArrayCell(propData,x);
            if (propInfo != INVALID_HANDLE)
            {
                #if defined _TRACE
                    new id = GetPropInfoId(propInfo);
                    new Handle:plugin = GetPropInfoPlugin(propInfo);
                    TraceCat("Array", "CloseArray playerInfo[%d,0x%08x,%d,%d]=0x%08x", \
                             client, plugin, id, x, propInfo);
                #endif

                ClearArray(propInfo);
                CloseHandle(propInfo);
            }
        }
        ClearArray(propData);
        CloseHandle(propData);
        SetPropData(client, INVALID_HANDLE);

        TraceCat("Array", "CloseArray playerArray[%d,INFO_PROPERTIES]=0x%08x", \
                 client, propData);
    }

    new Handle:shopitems=Handle:GetShopitems(client); // Player's shop items
    if (shopitems != INVALID_HANDLE)
    {
        ClearArray(shopitems);
        CloseHandle(shopitems);
        SetShopitems(client, INVALID_HANDLE);

        TraceCat("Array", "CloseArray playerArray[%d,INFO_SHOPITEMS]=0x%08x", \
                 client, shopitems);
    }

    new Handle:raceData = GetRaceData(client); // Player's race data
    if (raceData != INVALID_HANDLE)
    {
        new raceDataCount=GetArraySize(raceData);
        for(new race=0;race<raceDataCount;race++)
        {
            new Handle:raceInfo=GetArrayCell(raceData,race); // Player's races
            if (raceInfo != INVALID_HANDLE)
            {
                ClearArray(raceInfo);
                CloseHandle(raceInfo);

                TraceCat("Array", "CloseArray playerRaceInfo[%d,%d]=0x%08x", \
                         client, race, raceInfo);
            }
        }

        ClearArray(raceData);
        CloseHandle(raceData);
        SetRaceData(client, INVALID_HANDLE);

        TraceCat("Array", "CloseArray playerArray[%d,INFO_RACEDATA]=0x%08x", \
                 client, raceData);
    }

    new Handle:techData = GetTechData(client); // Player's tech data
    if (techData != INVALID_HANDLE)
    {
        new techDataCount=GetArraySize(techData);
        for(new techId=0;techId<techDataCount;techId++)
        {
            new Handle:techInfo=GetArrayCell(techData,techId); // Player's tech info
            if (techInfo != INVALID_HANDLE)
            {
                ClearArray(techInfo);
                CloseHandle(techInfo);

                TraceCat("Array", "CloseArray playerTechInfo[%d,%d]=0x%08x", \
                         client, techId, techInfo);
            }
        }

        ClearArray(techData);
        CloseHandle(techData);
        SetTechData(client, INVALID_HANDLE);

        TraceCat("Array", "CloseArray playerArray[%d,INFO_TECHDATA]=0x%08x", \
                 client, techData);
    }

    TraceReturn();
}

Float:SumFloatClientProperty(client, pindex)
{
    new Float:sum = 0.0;
    new Handle:propData=GetPropData(client);
    if (propData == INVALID_HANDLE)
    {
        new numprops=GetArraySize(propData);
        for (new x=0;x<numprops;x++)
        {
            new Handle:propInfo=GetArrayCell(propData,x);
            if (propInfo != INVALID_HANDLE)
                sum += Float:GetArrayCell(propInfo,pindex);
        }
    }
    return sum;
}

