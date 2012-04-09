/*
 * vim: set ai et ts=4 sw=4 syntax=sourcepawn :
 * File: xp.inc
 * Description: Routines to manipulate XP and leveling.
 * Author: -=|JFH|=-Naris (Murray Wilson)
 * Credits: Anthony Iacono  
 */

#include <particle>
#include <entlimit>
#include <tf2_objects>

#define HEADSHOT_XP         10
#define BACKSTAB_XP         10
#define ASSIST_XP           10
#define MELEE_XP            20
#define DOMINATED_XP        15
#define REVENGE_XP          15

#define BUILD_XP            15
#define DESTROY_XP          15
#define UPGRADE_XP           5
#define CHARGE_XP           30

#define PLANT_XP            20
#define DEFUSE_XP           20
#define EXPLODE_XP          20
#define HOSTAGERESCUED_XP   20

#define KILL_DEFUSER_XP     10
#define KILL_PLANTER_XP     10

#define BLOCK_XP            20
#define ESCORT_XP           10
#define CAPTURE_FLAG_XP     10 // Multiplied by KillXP()
#define CAPTURE_POINT_XP     5 // Multiplied by KillXP()

#define ROUND_WIN_XP        20
#define GAME_WIN_XP         10 // Multiplied by score
#define MVP_XP               2 // Multiplied by place * points

#include "effect/SteamSprite"
#include "effect/HaloSprite"

SetupLevelUpEffect(bool:precache, bool:preload=false)
{
    SetupSteamSprite(precache, preload);
    SetupHaloSprite(precache, preload);
}

KillXP(level)
{
    new Handle:temp;
    if (SAVE_ENABLED)
        temp=GetArrayCell(vecLevelConfiguration,1);
    else
        temp=GetArrayCell(vecLevelConfiguration,3);

    new maxLevels = GetArraySize(temp);
    if (level < maxLevels)
        return GetArrayCell(temp,level);
    else
    {
        // Calculate XP given for levels
        // beyond the configuration table.
        new maxAmt = GetArrayCell(temp,maxLevels-1);
        new delta  = maxAmt - GetArrayCell(temp,maxLevels-2);
        return maxAmt + ((level-maxLevels) * delta);
    }
}

ReqLevelXP(level)
{
    new Handle:temp;
    if (SAVE_ENABLED)
        temp=GetArrayCell(vecLevelConfiguration,0);
    else
        temp=GetArrayCell(vecLevelConfiguration,2);

    new maxLevels = GetArraySize(temp);
    if (level < 0)
        return GetArrayCell(temp,0);
    else if (level < maxLevels)
        return GetArrayCell(temp,level);
    else
    {
        // Calculate XP required for levels
        // beyond the configuration table.
        new maxReq = GetArrayCell(temp,maxLevels-1);
        new delta  = maxReq - GetArrayCell(temp,maxLevels-2);
        return maxReq + ((level-maxLevels) * delta);
    }
}

DoLevelCheck(client)
{
    TraceInto("XP", "DoLevelCheck", "Check %d:%N's level", \
              client, ValidClientIndex(client));

    if (GetRaceCount() > 1 &&
        GetDatabaseLoaded(client) != DataLoading)
    {
        new bool:moreLevels=true;
        while (moreLevels)
        {
            new race = GetRace(client);
            new Handle:raceHandle=GetRaceHandle(race);
            new level = GetLevel(client,race);
            if (level < GetRaceMaxLevel(raceHandle))
            {
                new curXP = GetXP(client,race);
                new reqXP = ReqLevelXP(++level);
                if (curXP >= reqXP)
                {
                    new oLevel = GetOverallLevel(client);

                    Trace("Increase %d:%N's race %d level=%d, overall=%d,  xp=%d", \
                          client, ValidClientIndex(client), race, level+1, oLevel+1, curXP-reqXP);

                    SetLevel(client,race,level, true);
                    SetXP(client,race,curXP-reqXP);

                    new Float:vec[3];
                    GetClientAbsOrigin(client, vec);
                    vec[2] += 50;

                    static const color[4] = {255, 200, 0, 200};
                    TE_SetupBeamRingPoint(vec, 20.0, 50.0, SteamSprite(), HaloSprite(),
                                          0, 15, 3.0, 20.0, 0.0, color, 1, 0);
                    TE_SendToAll();

                    if ((GetGameType() == tf2 || GameType == dod) &&
                        !IsEntLimitReached(.client=client,
                                           .message="Unable to spawn anymore achievement particles"))
                    {
                        CreateParticle("Achieved", 5.0, client, Attach, "head");
                    }

                    decl String:raceName[64];
                    GetRaceName(raceHandle,raceName,sizeof(raceName));

                    if (IsFakeClient(client))
                    {
                        LogToGame("[Bot]%N is now a level %d %s (level %d overall).",
                                  client, level, raceName, oLevel);
                    }
                    else
                    {
                        LogToGame("%N is now a level %d %s (level %d overall).",
                                  client, level, raceName, oLevel);

                        EmitLevelSoundToClient(client, GetRaceFactionId(raceHandle));
                        DisplayMessage(client, Display_XP, "%t", "LeveledUp", level, raceName, oLevel);
                        UpgradeMenu(client, UpgradeMode);
                    }
                }
                else
                {
                    moreLevels=false;

                    /*
                    Trace("Check %d:%N's race %d xp=%d(%d required), level=%d, overall=%d", \
                          client, ValidClientIndex(client), race, curXP, reqXP, level, \
                          GetOverallLevel(client));

                    new bool:saveEnabled = (g_bSaveXP && GetRaceCount() > 1 &&
                                            GetPlayerStatus(client) != PlayerDisabled);

                    if (saveEnabled && GetDatabaseLoaded(client) >= DataOK &&
                        GetRaceLoaded(client, race) >= DataOK &&
                        !GetDatabaseSaved(client))
                    {
                        // Save after about 50 kills (or 5 caps) worth of XP gained
                        // and > 1000 XP and more than 10 minutes since last save.
                        new Handle:raceInfo = GetRaceInfo(client, race);
                        if (raceInfo != INVALID_HANDLE)
                        {
                            new unsavedXP = curXP - GetRaceInfoSavedXP(raceInfo);
                            if (unsavedXP > 1000 && unsavedXP > KillXP(level) * 50 &&
                                GetGameTime() - GetDatabaseSaveTime(client) > 600)
                            {
                                SetRaceInfoSavedXP(raceInfo,curXP);
                                SavePlayerData(client);
                            }
                        }
                    }
                    */
                }
            }
            else
                moreLevels=false;
        }
    }
    TraceReturn();
}

ShowXP(client, bool:forceDisplay)
{
    if (GetRaceCount() > 1 && !IsFakeClient(client))
    {
        new race=GetRace(client);
        if (race >= 0)
        {
            new Handle:raceHandle=GetRaceHandle(race);
            if (raceHandle != INVALID_HANDLE)
            {
                decl String:racename[64];
                GetRaceName(raceHandle,racename,sizeof(racename));
                new level=GetLevel(client,race);
                new maxLevel=GetRaceMaxLevel(raceHandle);
                if (level < maxLevel)
                {
                    DisplayMessage(client, forceDisplay ? NotSet : Display_XP, "%t",
                                   "ShowXP", racename, level, maxLevel, GetXP(client,race),
                                   ReqLevelXP(level+1), GetOverallLevel(client));
                }
                else
                {
                    DisplayMessage(client, forceDisplay ? NotSet : Display_XP, "%t",
                                   "ShowMaxXP", racename, level, GetXP(client,race),
                                   GetOverallLevel(client));
                }
            }
        }
    }
}

GiveKillXP(index, victim, bool:headshot, bool:backstab, bool:melee,
           bool:dominated, bool:revenge, bool:assist)
{
    if (GetRaceCount() > 1 && GetClientCount() >= g_iMinPlayers)
    {
        new amount;
        if (victim > 0)
        {
            amount=KillXP(GetLevel(victim,GetRace(victim)));
            new level=GetOverallLevel(index);
            new victim_level=GetOverallLevel(victim);
            if (level < victim_level)
            {
                amount += (victim_level - level);
                new cap = ReqLevelXP(level+1);
                if (amount > cap)
                    amount = cap;
            }
        }
        else
            amount=KillXP(GetLevel(index,GetRace(index)));

        if (headshot)
        {
            amount+=HEADSHOT_XP;
            DisplayMessage(index,Display_Details, "%t", "HeadshotBonus", HEADSHOT_XP);
        }
        else if (backstab)
        {
            amount+=BACKSTAB_XP;
            DisplayMessage(index,Display_Details, "%t", "BackstabBonus", BACKSTAB_XP);
        }
        else if (melee)
        {
            amount+=MELEE_XP;
            DisplayMessage(index,Display_Details, "%t", "MeleeBonus", MELEE_XP);
        }

        if (dominated)
        {
            amount+=DOMINATED_XP;
            DisplayMessage(index,Display_Details, "%t", "DominationBonus", DOMINATED_XP);
        }

        if (revenge)
        {
            amount+=REVENGE_XP;
            DisplayMessage(index,Display_Details, "%t", "RevengeBonus", DOMINATED_XP);
        }

        if (assist)
        {
            amount+=ASSIST_XP;
            DisplayMessage(index,Display_Details, "%t", "AssistBonus", ASSIST_XP);
        }

        GiveXP(index,amount, assist ? "Assist" : "Kill");
    }
}

GiveDefuseXP(client)
{
    // Called when a player defuses the bomb
    GiveXP(client,DEFUSE_XP, "DefuseBomb");
}

GivePlantXP(client)
{
    // Called when a player plants the bomb
    GiveXP(client,PLANT_XP, "PlantBomb");
}

GiveExplodeXP(client)
{
    // Called when the bomb explode
    GiveXP(client,EXPLODE_XP, "ExplodeBomb");
}

GiveKillDefuserXP(client)
{
    // Called when a player defuses the bomb
    GiveXP(client,KILL_DEFUSER_XP, "KillDefuser");
}

GiveKillPlanterXP(client)
{
    // Called when a player plants the bomb
    GiveXP(client,KILL_PLANTER_XP, "KillBomber");
}

GiveHostageRescuedXP(client)
{
    // Called when a player rescues a hostage
    GiveXP(client,HOSTAGERESCUED_XP, "RescueHostage");
}

GiveChargeDeployedXP(client)
{
    // Called when a player deploys thier charge (medic goes uber)
    GiveXP(client,CHARGE_XP, "DeployUber");
}

GiveObjectBuiltXP(client,TFObjectType:object)
{
    // Called when a player builds an object
    GiveXP(client, BUILD_XP, "Building",
           .object=object);
}

GiveObjectUpgradedXP(client,TFObjectType:object, bool:isBuilder)
{
    // Called when a player upgrades an object
    GiveXP(client, isBuilder ? UPGRADE_XP : UPGRADE_XP*2,
           "Upgrading", .object=object);
}

TakeObjectBuiltXP(client,TFObjectType:object)
{
    // Called when a player destroys thier own object
    TakeXP(client,BUILD_XP, "DestroyingOwn",
           .object=object);
}

GiveObjectDestroyedXP(client,TFObjectType:object)
{
    // Called when a player destroys an object
    GiveXP(client,DESTROY_XP, "Destroying",
           .object=object);
}

GiveFlagCapturedXP(client)
{
    // Called when a player captures a flag
    new amount=KillXP(GetLevel(client,GetRace(client))) * CAPTURE_FLAG_XP;
    GiveXP(client,amount, "CapIntel");
}

GivePointCapturedXP(client)
{
    // Called when a player captures a point
    new amount=KillXP(GetLevel(client,GetRace(client))) * CAPTURE_POINT_XP;
    GiveXP(client,amount, "CapPoint");
}

GiveCapBlockedXP(client)
{
    // Called when a player blocks a capture
    GiveXP(client,BLOCK_XP, "BlockPoint");
}

GiveEscortXP(client, points)
{
    // Called when a player escorts the cart
    GiveXP(client,ESCORT_XP*points, "Escort");
}

GiveRoundWinXP(client)
{
    // Called when a team wins a round
    GiveXP(client,ROUND_WIN_XP, "WinRound");
}

GiveGameWinXP(client,score)
{
    // Called when a team wins a round
    GiveXP(client,GAME_WIN_XP*score, "WinGame");
}

GiveMvpXP(client, place, points)
{
    GiveXP(client,points*(7-place)*MVP_XP,
           "BeingMVP", place, points);
}

GiveXP(client, amount, const String:phrase[], place=-1,
       points=-1, TFObjectType:object=TFObject_Unknown)
{
    if (GetRaceCount() > 1 && GetClientCount() >= g_iMinPlayers)
    {
        // Called when a player is given XP
        new Action:result = Plugin_Continue;
        Call_StartForward(g_OnXPGivenHandle);
        Call_PushCell(client);
        Call_PushCellRef(amount);
        Call_PushCell(false);
        Call_Finish(result);

        if (result != Plugin_Stop)
        {
            new String:reason[NAME_STRING_LENGTH];
            if (place >= 0 || points >= 0)
            {
                Format(reason, sizeof(reason), "%T",
                       phrase, client, place, points);
            }
            else if (object != TFObject_Unknown)
            {
                Format(reason, sizeof(reason), "%T",
                       phrase, client, TF2_ObjectNames[object]);
            }
            else
            {
                Format(reason, sizeof(reason), "%T",
                       phrase, client);
            }

            new race=GetRace(client);
            new newxp=GetXP(client,race)+amount;
            SetXP(client,race,newxp);
            LogToGame("%N gained %d experience for %s", client, amount, reason);
            DisplayMessage(client, Display_XP, "%t", "GainedXP", amount, reason);

            new Handle:raceHandle=GetRaceHandle(race);
            if (raceHandle != INVALID_HANDLE &&
                GetLevel(client,race)<GetRaceMaxLevel(GetRaceHandle(race)))
            {
                DoLevelCheck(client);
            }
        }
    }
}

TakeXP(client, amount, const String:phrase[], place=-1,
       points=-1, TFObjectType:object=TFObject_Unknown)
{
    if (GetRaceCount() > 1 && GetClientCount() >= g_iMinPlayers)
    {
        new Action:result = Plugin_Continue;
        Call_StartForward(g_OnXPGivenHandle);
        Call_PushCell(client);
        Call_PushCellRef(amount);
        Call_PushCell(true);
        Call_Finish(result);

        if (result != Plugin_Stop)
        {
            new String:reason[NAME_STRING_LENGTH];
            if (place >= 0 || points >= 0)
            {
                Format(reason, sizeof(reason), "%T",
                       phrase, client, place, points);
            }
            else if (object != TFObject_Unknown)
            {
                Format(reason, sizeof(reason), "%T",
                       phrase, client, TF2_ObjectNames[object]);
            }
            else
            {
                Format(reason, sizeof(reason), "%T",
                       phrase, client);
            }

            new race=GetRace(client);
            new newxp=GetXP(client,race)-amount;
            SetXP(client,race,newxp);
            LogToGame("%N lost %d experience for %s", client, amount, reason);
            DisplayMessage(client, Display_XP, "%t", "LostXP", amount, reason);

            new Handle:raceHandle=GetRaceHandle(race);
            if (raceHandle != INVALID_HANDLE &&
                GetLevel(client,race) < GetRaceMaxLevel(raceHandle))
            {
                DoLevelCheck(client);
            }
        }
    }
}

