/**
 * vim: set ai et ts=4 sw=4 syntax=sourcepawn :
 * File: playerproperties.inc
 * Description: Functions that alter the player's properties (visibility, speed, gravity).
 * Author(s): -=|JFH|=-Naris (Murray Wilson)
 * Credit(s): PimpinJuice (Anthony Iacono)
 */

#if defined _player_properties_included
 #endinput
#endif
#define _player_properties_included

#if defined _TRACE
    new m_BaseHealth[MAXPLAYERS+1];
    new Float:m_BaseSpeed[MAXPLAYERS+1];

    new m_SpawnHealth[MAXPLAYERS+1];
    new Float:m_SpawnSpeed[MAXPLAYERS+1];
#endif

// Global definition
new g_iRoundNumber = 0;
new RoundStates:g_RoundState = Unknown;

new Float:m_StoppedMoving[MAXPLAYERS+1];
new Float:m_CloakTime[MAXPLAYERS+1];
new bool:m_SpeedAltered[MAXPLAYERS+1];
new bool:m_GravityAltered[MAXPLAYERS+1];
new bool:m_ColorAltered[MAXPLAYERS+1];
new bool:m_WeaponsAltered[MAXPLAYERS+1];
new bool:m_VisibilityAltered[MAXPLAYERS+1];
new bool:m_MoveTypeAltered[MAXPLAYERS+1];
new Handle:m_MonitorMotion[MAXPLAYERS+1];
new bool:m_IsMoving[MAXPLAYERS+1];
new bool:g_Killed[MAXPLAYERS+1] = { false, ...};
new bool:g_Dead[MAXPLAYERS+1] = { false, ...};
new g_InjuredBy[MAXPLAYERS+1] = { 0, ...};
new g_KilledBy[MAXPLAYERS+1] = { 0, ...};

new String:m_HudMessage[MAXPLAYERS+1][64];
new Handle:m_HudSynch[6] = { INVALID_HANDLE, ... };

new Handle:g_HUDTimers[MAXPLAYERS+1];
new Handle:g_PropertyTimers[MAXPLAYERS+1];

#define ResetHUDTimer(%1)           g_HUDTimers[%1]      = INVALID_HANDLE
#define ResetPropertyTimer(%1)      g_PropertyTimers[%1] = INVALID_HANDLE

#define UpdatePlayerProperties(%1)  \
    if (g_PropertyTimers[%1] != INVALID_HANDLE) \
        TriggerTimer(g_PropertyTimers[%1], false)
                    

enum HudAspect { Normal, WideScreen };
enum HudBlocks { Block1, Block2 };
//enum Position { XPos, YPos };
#define XPos    0
#define YPos    1

//                                                        Normal        Widescreen
//new const Float:HudPos[HudBlocks][HudAspect][Position]
new const Float:HudPos[HudBlocks][HudAspect][2]    = { { {0.20, 0.80}, {0.18, 0.84} },    // Block 1
                                                       { {0.64, 0.80}, {0.68, 0.87} } };  // Block 2

CreateHUDTimer(client)
{
    if (g_HUDTimers[client] == INVALID_HANDLE)
    {
        g_HUDTimers[client] = CreateTimer(1.0,PlayerHUD,GetClientUserId(client),
                                          TIMER_REPEAT|TIMER_FLAG_NO_MAPCHANGE);
    }
}

KillHUDTimer(client)
{
    new Handle:timer=g_HUDTimers[client];
    if (timer != INVALID_HANDLE)
    {
        ResetHUDTimer(client);	
        KillTimer(timer);
    }
}

stock KillAllHUDTimers()
{
    for (new i = 1; i <= MaxClients; i++)
        KillHUDTimer(i);
}

ResetAllHUDTimers()
{
    for (new i = 1; i <= MaxClients; i++)
        ResetHUDTimer(i);
}

CreatePropertyTimer(client)
{
    if (g_PropertyTimers[client] == INVALID_HANDLE)
    {
        g_PropertyTimers[client] = CreateTimer(1.0,PlayerProperties,GetClientUserId(client),
                                               TIMER_REPEAT|TIMER_FLAG_NO_MAPCHANGE);
    }
}

KillPropertyTimer(client)
{
    new Handle:timer=g_PropertyTimers[client];
    if (timer != INVALID_HANDLE)
    {
        ResetPropertyTimer(client);	
        KillTimer(timer);
    }
}

stock KillAllPropertyTimers()
{
    for (new i = 1; i <= MaxClients; i++)
        KillPropertyTimer(i);
}

ResetAllPropertyTimers()
{
    for (new i = 1; i <= MaxClients; i++)
        ResetPropertyTimer(i);
}

InitHud()
{
    if (GameType != dod)
    {
        for (new h=0; h < sizeof(m_HudSynch); h++)
            m_HudSynch[h] = CreateHudSynchronizer();
    }
}

CloseHud()
{
    if (GameType == tf2 || GameType == hl2mp)
    {
        for (new h=0; h < sizeof(m_HudSynch); h++)
        {
            CloseHandle(m_HudSynch[h]);
            m_HudSynch[h] = INVALID_HANDLE;
        }
    }
}

public Action:PlayerHUD(Handle:timer, any:userid)
{
    new client = GetClientOfUserId(userid);

    TraceInto("PlayerProperties", "PlayerHUD", "userid=%d, client=%d:%L", \
              userid, client, ValidClientIndex(client));

    if (client > 0 && IsClientInGame(client) && !IsFakeClient(client))
    {
        if (g_RoundState >= RoundActive && g_RoundState < RoundOver &&
            IsPlayerAlive(client))
        {
            new rate = GetEnergyRate(client);
            new limit = GetEnergyLimit(client);

            new race = GetRace(client);
            if (race > 0)
            {
                new Handle:raceHandle = GetRaceHandle(race);

                if (rate < 0)
                    rate = GetRaceEnergyRate(raceHandle);

                if (limit < 0)
                    limit = GetRaceEnergyLimit(raceHandle);
            }

            if (rate < 0)
                rate = 1;

            // Increment Player's Energy
            new energy = GetEnergy(client);
            if (rate != 0 && (energy < limit || limit < 0))
                SetEnergy(client, energy += rate);

            if (GetDatabaseLoaded(client) >= DataOK)
            {
                if ((GameType == tf2 || GameType == hl2mp) &&
                    !GetSetting(client, Disable_HUD))
                {
                    // Update Player's HUD
                    new HudAspect:ws = GetSetting(client, Display_Widescreen) ? WideScreen : Normal;

                    // Block 1
                    new Float:x = HudPos[Block1][ws][XPos]; // 0.18
                    new Float:y = HudPos[Block1][ws][YPos]; // 0.84

                    if (limit != 0)
                    {
                        SetHudTextParams(x, y, 1.0, 255, 192, 192, 255);
                        ShowSyncHudText(client, m_HudSynch[0], "%t", "PlayerEnergy", energy);
                    }

                    //x = 0.18;
                    y += 0.03; //y = 0.87;
                    new armor = GetArmor(client);
                    if (armor > 0 || m_ArmorName[client][0] != '\0')
                    {
                        SetHudTextParams(x, y, 1.0, 255, 153, 128, 255);

                        if (HasShields(client))
                        {
                            ShowSyncHudText(client, m_HudSynch[1], "%t",
                                            "PlayerShields", armor);
                        }
                        else if (m_ArmorName[client][0] == '\0' ||
                                 StrEqual(m_ArmorName[client], "Armor", false))
                        {
                            ShowSyncHudText(client, m_HudSynch[1], "%t",
                                            "PlayerArmor", armor);
                        }
                        else
                        {
                            ShowSyncHudText(client, m_HudSynch[1], "%s: %d",
                                            m_ArmorName[client], armor);
                        }
                    }

                    new health = GetClientHealth(client);
                    if (health > ((GameType == tf2) ? 500 : 100))
                    {
                        //x = 0.18;
                        y += 0.03; //y = 0.90;
                        SetHudTextParams(x, y, 1.0, 128, 255, 128, 255);
                        ShowSyncHudText(client, m_HudSynch[2], "%t", "PlayerHealth", health);
                    }

                    // Block 2
                    x = HudPos[Block2][ws][XPos]; // 0.68
                    y = HudPos[Block2][ws][YPos]; // 0.87

                    SetHudTextParams(x, y, 1.0, 128, 128, 255, 255);
                    ShowSyncHudText(client, m_HudSynch[4], "%t", "PlayerCrystals", GetCrystals(client));

                    //x = 0.68;
                    y += 0.03; //y = 0.91;
                    SetHudTextParams(x, y, 1.0, 255, 255, 128, 255);
                    ShowSyncHudText(client, m_HudSynch[5], "%t", "PlayerVespene", GetVespene(client));

                    if (m_HudMessage[client][0] != '\0')
                    {
                        //x = 0.68;
                        y += 0.03; //y = 0.94;
                        SetHudTextParams(x, y, 1.0, 255, 128, 128, 255);
                        ShowSyncHudText(client, m_HudSynch[3], m_HudMessage[client]);
                    }
                }
                else if (GetSetting(client, Enable_Alternate_HUD))
                {
                    new armor = GetArmor(client);
                    if (armor > 0)
                    {
                        PrintHintText(client, "%t",
                                      HasShields(client) ? "AltHudShields" : "AltHudArmor",
                                      GetArmor(client), energy, m_HudMessage[client]);
                    }
                    else
                    {
                        PrintHintText(client, "%t", "AltHudEnergy",
                                      energy, m_HudMessage[client]);
                    }
                }
            }
        }
    }
    else
    {
        Trace("Skipping PlayerHUD for %L, Alive=%d, RoundState=%d(%s)", \
              client, IsPlayerAlive(client), g_RoundState, RoundStateNames[g_RoundState]);
    }

    TraceReturn();
    return Plugin_Continue;
}

public Action:PlayerProperties(Handle:timer, any:userid)
{
    new client = GetClientOfUserId(userid);

    TraceInto("PlayerProperties", "PlayerProperties", "userid=%d, client=%d:%L", \
              userid, client, ValidClientIndex(client));

    if (client > 0 && IsClientInGame(client))
    {
        if (g_RoundState >= RoundActive && g_RoundState < RoundOver &&
            IsPlayerAlive(client))
        {
            new bool:overridePlayerVisibility=false;
            new bool:overridePlayerGravity=false;
            new bool:overridePlayerSpeed=false;

            new visibility=-1,r=-1,g=-1,b=-1;
            new Float:gravity=-1.0,Float:speed=-1.0;
            new VisibilityType:vistype=NormalVisibility;
            new RenderMode:mode=RenderMode:-1;
            new RenderFx:fx=RenderFx:-1;
            new visibilityRank=-1;
            new bool:colorWeapons=false;
            new Float:time=0.0, Float:colorTime=0.0;
            new VisibilityPriority:priority=VisibilityPriorityNotSpecified;
            new VisibilityPriority:colorPty=VisibilityPriorityNotSpecified;
            new MoveType:mt = MOVETYPE_DEFAULT;

            decl String:wepName[128];
            GetClientWeapon(client, wepName, sizeof(wepName));

            new bool:isStill = !m_IsMoving[client] && IsStill(client);
            m_IsMoving[client] = false;

            // Process Player's Properties (Speed, Gravity & Visibility)
            new Handle:propData=GetPropData(client); // Player's properties
            new size = GetArraySize(propData);
            for(new k=0;k<size;k++)
            {
                // check which plugin is the highest/lowest bidder, but first, override?
                new Handle:propInfo             = GetArrayCell(propData,k);
                new Float:overridespeed         = GetPropInfoOverrideSpeed(propInfo);
                new Float:overridegravity       = GetPropInfoOverrideGravity(propInfo);
                new overridevisibility          = GetPropInfoOverrideVisibility(propInfo);
                new Float:checkSpeed            = GetPropInfoSpeed(propInfo);
                new Float:checkGravity          = GetPropInfoGravity(propInfo);
                new checkVisibility             = GetPropInfoVisibility(propInfo);
                new VisibilityType:type         = GetPropInfoVisibilityType(propInfo);
                new Float:factor1               = GetPropInfoVisibilityFactor1(propInfo);
                new Float:factor2               = GetPropInfoVisibilityFactor2(propInfo);
                new RenderFx:checkFx            = GetPropInfoVisibilityFx(propInfo);
                new RenderMode:checkMode        = GetPropInfoVisibilityMode(propInfo);
                new checkR                      = GetPropInfoVisibilityR(propInfo);
                new checkG                      = GetPropInfoVisibilityG(propInfo);
                new checkB                      = GetPropInfoVisibilityB(propInfo);
                new bool:checkWeapons           = GetPropInfoVisibilityWeapons(propInfo);
                new VisibilityPriority:checkPty = GetPropInfoVisibilityPriority(propInfo);
                new Float:checkTime             = GetPropInfoVisibilityTime(propInfo);
                new MoveType:checkMt            = GetPropInfoMoveType(propInfo);

                #if defined _TRACE
                    new id = GetPropInfoId(propInfo);
                    new Handle:plugin = GetPropInfoPlugin(propInfo);
                    decl String:pluginName[CLASS_STRING_LENGTH];
                    GetPluginFilename(plugin, pluginName, sizeof(pluginName));
                #endif

                if (mt != MOVETYPE_NONE && (mt == MOVETYPE_NONE || checkMt > mt))
                    mt = checkMt;

                if (overridePlayerSpeed)
                {
                    if (overridespeed >= 0.0 &&
                        overridespeed < speed)
                    {
                        speed=overridespeed;
                    }
                }
                else
                {
                    if (overridespeed >= 0.0)
                    {
                        speed=overridespeed;
                        overridePlayerSpeed=true;
                    }
                    else if(checkSpeed > speed)
                        speed=checkSpeed;
                }

                if (overridePlayerGravity)
                {
                    if (overridegravity >= 0.0 &&
                        overridegravity > gravity)
                    {
                        gravity=overridegravity;
                    }
                }
                else
                {
                    if (overridegravity >= 0.0)
                    {
                        gravity=overridegravity;
                        overridePlayerGravity=true;
                        m_GravityAltered[client]=true;
                    }
                    else if(checkGravity >= 0 &&
                            (gravity < 0.0 || checkGravity < gravity))
                    {
                        gravity=checkGravity;
                        m_GravityAltered[client]=true;
                    }
                }

                if (!overridePlayerVisibility)
                {
                    if (overridevisibility >= 0)
                    {
                        #if defined _TRACE
                            TraceCat("Visibility", "Assign %L.%d's %s Override Visibility=%d", \
                                     client, pluginName, id, overridevisibility);
                        #endif

                        time=checkTime;
                        vistype=NormalVisibility;
                        visibility=overridevisibility;
                        priority=VisibilityPriorityOverride;
                        overridePlayerVisibility=true;
                        m_VisibilityAltered[client]=true;
                    }
                    else if (m_VisibilityAltered[client] || type > DefaultVisibility ||
                             checkVisibility >= 0 || _:checkFx >= 0 || _:checkMode >= 0)
                    {
                        TraceCat("Visibility", "Check %L.%d's %s Visibility=%d,type=%d,fx=%d,mode=%d,still=%d, priority=%d, time=%f", \
                                 client,pluginName,id,checkVisibility,type,checkFx,checkMode, isStill, priority, time);

                        new checkRank = CalculateVisibility(client, wepName, isStill, checkVisibility,
                                                            type, factor1, factor2, checkMode, checkFx);

                        new bool:hasPriority = (checkPty > priority ||
                                                (checkPty == priority && checkTime > time &&
                                                 priority > VisibilityPriorityNotSpecified));

                        if (checkRank >= 0 && (hasPriority || visibilityRank < 0 || checkRank < visibilityRank))
                        {
                            TraceCat("Visibility", "Assign %L.%d's %s checkRank=%d (Visibility=%d,type=%d,fx=%d,mode=%d, priority=%d, time=%f)", \
                                     client,pluginName,id,checkRank,checkVisibility,type,checkFx,checkMode, priority, time);

                            m_VisibilityAltered[client]=true;
                            visibility=checkVisibility;
                            visibilityRank=checkRank;
                            priority=checkPty;
                            time=checkTime;
                            mode=checkMode;
                            vistype=type;
                            fx=checkFx;
                        }
                        else
                        {
                            TraceCat("Visibility", "Reject %L.%d's %s visibilityRank=%d, checkRank=%d (Visibility=%d,type=%d,fx=%d,mode=%d, priority=%d, time=%f)", \
                                     client,pluginName,id,visibilityRank,checkRank,checkVisibility,type,checkFx,checkMode, priority, time);
                        }
                    }
                    else
                    {
                        TraceCat("Visibility", "Don't check %L.%d's %s (Visibility=%d,type=%d,fx=%d,mode=%d, priority=%d, time=%f)", \
                                 client,pluginName,id,checkVisibility,type,checkFx,checkMode, priority, time);
                    }
                }

                if ((checkR >= 0 && checkR < 255) ||
                    (checkG >= 0 && checkG < 255) ||
                    (checkB >= 0 && checkB < 255))
                {
                    TraceCat("Color", "Check %L.%d's %s color=%d,%d,%d, priority=%d, time=%f", \
                             client,pluginName,id,checkR,checkG,checkB, priority, time);

                    new bool:colorPriority = (checkPty > colorPty ||
                                              (priority == colorPty && time < colorTime &&
                                               priority > VisibilityPriorityNotSpecified));

                    if (colorPriority || (r < 0 || r >= 255) &&
                                         (g < 0 || g >= 255) &&
                                         (b < 0 || b >= 255))
                    {
                        m_ColorAltered[client]=true;
                        colorWeapons=checkWeapons;
                        colorTime=checkTime;
                        colorPty=checkPty;
                        r=checkR;
                        g=checkG;
                        b=checkB;

                        if (colorWeapons)
                            m_WeaponsAltered[client]=true;

                        TraceCat("Color", "Use %L.%d's %s Colors=(%d,%d,%d), weapons=%d, priority=%d, time=%f", \
                                 client,pluginName,id,r,g,b,colorWeapons, priority, time);
                    }
                    else
                    {
                        TraceCat("Color", "Don't use %L.%d's %s Colors=(%d,%d,%d), weapons=%d, priority=%d, time=%f", \
                                 client,pluginName,id,r,g,b,colorWeapons, priority, time);
                    }
                }
            }

            if (m_MoveTypeAltered[client] || mt > MOVETYPE_DEFAULT)
            {
                if (mt == MOVETYPE_DEFAULT || mt == MOVETYPE_WALK)
                {
                    if (m_MoveTypeAltered[client] &&
                        GetEntityMoveType(client) != MOVETYPE_WALK)
                    {
                        SetEntityMoveType(client, MOVETYPE_WALK);
                    }
                    m_MoveTypeAltered[client] = false;
                }
                else
                {
                    SetEntityMoveType(client, mt);
                    m_MoveTypeAltered[client] = true;
                }
            }

            if (m_SpeedAltered[client] || overridePlayerSpeed || speed >= 0.0)
            {
                if (speed < 0.0)
                    speed = 1.0;

                if (GameType == tf2)
                {
                    new pcond = TF2_GetPlayerConditionFlags(client);
                    if (!TF2_IsSlowed(pcond) && !TF2_IsZoomed(pcond) &&
                        !TF2_IsCharging(pcond))
                    {
                        new Float:classSpeed = TF2_GetPlayerSpeed(client);
                        new Float:curSpeed = GetEntPropFloat(client, Prop_Send, "m_flMaxspeed");
                        //new Float:curSpeed = GetEntDataFloat(client,FindSendPropInfo("CBasePlayer", "m_flMaxspeed"));
                        new Float:newSpeed = classSpeed * speed;
                        if (curSpeed != newSpeed)
                        {
                            SetEntPropFloat(client, Prop_Send, "m_flMaxspeed", newSpeed);
                            //SetEntDataFloat(client,FindSendPropInfo("CBasePlayer", "m_flMaxspeed"), newSpeed);
                            if (newSpeed == 0.0)
                                SetEntityMoveType(client, MOVETYPE_NONE);
                            else if (GetEntityMoveType(client) == MOVETYPE_NONE)
                                SetEntityMoveType(client, MOVETYPE_WALK);
                        }

                        m_SpeedAltered[client] = (speed != 1.0);

                        TraceCat("Speed", "Set %L's Speed=%4.2f, actual=%3.0f, previous=%3.0f, class=%3.0f, base=%3.0f, spawn=%3.0f, altered=%d, pcond=%X", \
                                 client, speed, newSpeed, curSpeed, classSpeed, m_BaseSpeed[client], m_SpawnSpeed[client], m_SpeedAltered[client], pcond);
                    }
                }
                else // not tf2
                {
                    SetEntPropFloat(client, Prop_Send, "m_flLaggedMovementValue", speed);
                    m_SpeedAltered[client] = (speed != 1.0);
                }
            }

            if (m_GravityAltered[client] || overridePlayerGravity || gravity >= 0.0)
            {
                if (gravity < 0.0)
                    gravity = 1.0;

                SetEntityGravity(client, gravity);
                m_GravityAltered[client] = (gravity != 1.0);

                TraceCat("Gravity", "Set %L's Gravity=%4.2f, altered=%d", \
                         client, gravity, m_GravityAltered[client]);
            }

            if (m_VisibilityAltered[client] || m_ColorAltered[client] ||
                m_WeaponsAltered[client] || overridePlayerVisibility ||
                visibility >= 0 || vistype == TimedMeleeInvisibility)
            {
                if (mode < RENDER_NORMAL)
                    mode = RENDER_NORMAL;

                if (fx < RENDERFX_NONE)
                    fx = RENDERFX_NONE;

                if (visibility < 0 || visibility > 255)
                    visibility = 255;

                if (r < 0 || r > 255)
                    r = 255;

                if (g < 0 || g > 255)
                    g = 255;

                if (b < 0 || b > 255)
                    b = 255;

                if (visibility >= 255 && r >= 255 && g >= 255 && b >= 255 &&
                    mode == RENDER_NORMAL && fx == RENDERFX_NONE)
                {
                    TraceCat("Visibility", "Reset %L's visibility=%d",client,visibility);

                    ResetVisibility(client);

                    // Color and Visibility are back to normal, Reset the flags
                    m_ColorAltered[client]=m_WeaponsAltered[client]=m_VisibilityAltered[client]=false;
                }
                else
                    SetVisibility(client, visibility, mode, fx, r, g, b, colorWeapons, vistype);
            }
            else
            {
                TraceCat("Visibility", "Don't change %L's visibility=%d", client, visibility);
            }

            if (GameType == tf2)
            {
                new bool:debugSpeed = GetSetting(client, Debug_Speed);
                new bool:debugCondition = GetSetting(client, Debug_Condition);
                if (debugCondition)
                {
                    new String:condFlags[] = "                                ";
                    new pcond = TF2_GetPlayerConditionFlags(client);
                    if (TF2_IsSlowed(pcond))
                        condFlags[0] = 'S';
                    if (TF2_IsZoomed(pcond))
                        condFlags[1] = 'Z';
                    if (TF2_IsDisguising(pcond))
                        condFlags[2] = 'd';
                    if (TF2_IsDisguised(pcond))
                        condFlags[3] = 'D';
                    if (TF2_IsCloaked(pcond))
                        condFlags[4] = 'C';
                    if (TF2_IsUbercharged(pcond))
                        condFlags[5] = 'U';
                    if (TF2_IsTeleportedGlow(pcond))
                        condFlags[6] = 'g';
                    if (TF2_IsTaunting(pcond))
                        condFlags[7] = 'T';
                    if (TF2_IsUberchargeFading(pcond))
                        condFlags[8] = 'f';
                    if (TF2_IsCloakFlicker(pcond))
                        condFlags[9] = 'c';
                    if (TF2_IsTeleporting(pcond))
                        condFlags[10] = 'p';
                    if (TF2_IsKritzkrieged(pcond))
                        condFlags[11] = 'K';
                    if (pcond & (1 << 12)) // TFCond_Unknown2
                        condFlags[12] = '2';
                    if (TF2_IsDeadRingered(pcond))
                        condFlags[13] = 'R';
                    if (TF2_IsBonked(pcond))
                        condFlags[14] = 'b';
                    if (TF2_IsDazed(pcond))
                        condFlags[15] = 'A';
                    if (TF2_IsBuffed(pcond))
                        condFlags[16] = 'B';
                    if (TF2_IsCharging(pcond))
                        condFlags[17] = '-';
                    if (TF2_IsDemoBuff(pcond))
                        condFlags[18] = '>';
                    if (TF2_IsCritCola(pcond))
                        condFlags[19] = 'r';
                    if (TF2_IsHealing(pcond))
                        condFlags[20] = 'H';
                    if (pcond & (1 << 21)) // Unknown3
                        condFlags[21] = '3';
                    if (TF2_IsOnFire(pcond))
                        condFlags[22] = 'F';
                    if (TF2_IsOverhealed(pcond))
                        condFlags[23] = 'O';
                    if (TF2_IsJarated(pcond))
                        condFlags[24] = 'J';
                    if (TF2_IsBleeding(pcond))
                        condFlags[25] = 'L';
                    if (TF2_IsDefenseBuffed(pcond))
                        condFlags[26] = 'E';
                    if (TF2_IsMilked(pcond))
                        condFlags[27] = 'M';
                    if (pcond & (1 << 28)) // Unknown4
                        condFlags[28] = '4';
                    if (pcond & (1 << 29)) // Unknown5
                        condFlags[29] = '5';
                    if (pcond & (1 << 30)) // Unknown6
                        condFlags[30] = '6';
                    if (pcond & (1 << 31)) // Unknown7
                        condFlags[31] = '7';
                    if (pcond & (1 << 32)) // Unknown8
                        condFlags[32] = '8';

                    if (debugSpeed)
                    {
                        new Float:maxSpeed = GetEntPropFloat(client, Prop_Send, "m_flMaxspeed");
                        LogMessage("%Ns pcond=[%s]-(0x%08x) Speed=%4.2f:%3.0f", client, condFlags, pcond, speed, maxSpeed);
                        CPrintToChat(client, "pcond=[%s]-(0x%08x) Speed=%4.2f:%3.0f", condFlags, pcond, speed, maxSpeed);
                    }
                    else
                    {
                        LogMessage("%Ns pcond=[%s]-(0x%08x)", client, condFlags, pcond);
                        CPrintToChat(client, "pcond=[%s]-(0x%08x)", condFlags, pcond);
                    }
                }
                else if (debugSpeed)
                {
                    new Float:maxSpeed = GetEntPropFloat(client, Prop_Send, "m_flMaxspeed");
                    LogMessage("%Ns Speed=%4.2f:%3.0f", client, speed, maxSpeed);
                    CPrintToChat(client, "Speed=%4.2f:%3.0f", speed, maxSpeed);
                }
            }
        }
    }
    else
    {
        Trace("Skipping PlayerProperties for %L, Alive=%d, RoundState=%d(%s)", \
              client, IsPlayerAlive(client), g_RoundState, RoundStateNames[g_RoundState]);
    }

    TraceReturn();
    return Plugin_Continue;
}

bool:IsStill(client)
{
    SetTraceCategory("Visibility");
    TraceInto("PlayerProperties", "IsStill", "client=%d:%L", \
              client, ValidClientIndex(client));

    new Float:vecVelocity[3];
    //GetEntPropVector(client, Prop_Send, "m_vecVelocity[0]", vecVelocity);
    GetEntDataVector(client, FindSendPropInfo("CBasePlayer", "m_vecVelocity[0]"), vecVelocity);
    if (vecVelocity[0] == 0.0 && vecVelocity[1] == 0.0 && vecVelocity[2] == 0.0)
    {
        if (!(GetClientButtons(client) & (IN_FORWARD   |IN_BACK    | IN_MOVELEFT |
                                          IN_MOVERIGHT |IN_SPEED   | IN_JUMP |
                                          IN_ATTACK    |IN_ATTACK2 | IN_ZOOM)))
        {
            if (GameType == tf2)
            {
                new playerCond = TF2_GetPlayerConditionFlags(client);
                if (TF2_IsSlowed(playerCond) || TF2_IsZoomed(playerCond))
                {
                    m_StoppedMoving[client] = 0.0;

                    TraceReturn("still=false");
                    return false;
                }
            }

            if (m_StoppedMoving[client] == 0.0)
                m_StoppedMoving[client] = GetGameTime();

            TraceReturn("still=true");
            return true;
        }
    }

    m_StoppedMoving[client] = 0.0;

    TraceReturn("still=false");
    return false;
}

CalculateVisibility(client, const String:wepName[], bool:isStill, &visibility,
                    VisibilityType:type, Float:factor1, Float:factor2,
                    &RenderMode:mode, &RenderFx:fx)
{
    SetTraceCategory("Visibility");
    TraceInto("PlayerProperties", "CalculateVisibility", "client=%d:%L,wepName=%s,isStill=%d,visibility=%d,type=%d,factor1=%f,factor2=%f,mode=%d,fx=%d", \
              client, ValidClientIndex(client), wepName, isStill, visibility, type, factor1, factor2, mode, fx);

    new original_visibility = visibility;

    if (type == TimedInvisibility || type == TimedMeleeInvisibility || type == GraduatedEffects)
    {
        if (!isStill)
        {
            visibility = 255;
            Trace("Uncloaking %L (motion)", client);
        }
        else if (m_CloakTime[client] != 0.0) // Currently Cloaked
        {
            // factor2 == duration
            if (factor2 > 0.0 && (GetGameTime() - m_CloakTime[client]) >= factor2)
            {
                visibility = 255;
                Trace("Uncloaking %L (duration)", client);
            }
            else if (type != TimedMeleeInvisibility || IsEquipmentMelee(wepName))
            {
                visibility = 0;
                Trace("Cloaking %L", client);
            }
            else
            {
                visibility = 255;
                Trace("Uncloaking %L (other)", client);
            }
        }
        else // Not Cloaked
        {
            // factor1 == delay
            if (factor1 <= 0.0 || (GetGameTime() - m_StoppedMoving[client]) >= factor1)
            {
                if (type != TimedMeleeInvisibility || IsEquipmentMelee(wepName))
                {
                    visibility = 0;
                    Trace("Cloaking %L", client);
                }
                else
                {
                    visibility = 255;
                    Trace("Uncloaking %L (equip)", client);
                }
            }
            else
            {
                visibility = 255;
                Trace("Uncloaking %L (delay)", client);
            }
        }

        if (visibility != 0)
        {
            if (mode == RENDER_NONE)
                mode = RENDER_TRANSCOLOR;
            if (fx == RENDERFX_FADE_SLOW || fx == RENDERFX_FADE_FAST)
                fx = RENDERFX_NONE;
        }

        if (type == GraduatedEffects && visibility == 255)
        {
            fx = RenderFx:((original_visibility / 14) + 1);
            if (fx == RENDERFX_EXPLODE)
                fx = RENDERFX_PULSE_FAST_WIDER;
            else if (fx == RENDERFX_GLOWSHELL)
                fx = RENDERFX_MAX;

            mode = RENDER_TRANSCOLOR;
        }
        else if (visibility == 0)
            fx = RENDERFX_NONE;
    }
    else //if (type == BasicVisibility || type == NormalVisibility || type == DefaultVisibility)
    {
        if (visibility < 0 || visibility > 255)
            visibility = 255;

        if (factor1 >= 0.0 && factor1 != 1.0) // factor1 == melee factor
        {
            if (IsEquipmentMelee(wepName))
                visibility = RoundToNearest(float(visibility) * factor1);
        }

        if (factor2 >= 0.0 && factor2 != 1.0) // factor2 == movement factor
        {
            if (isStill)
                visibility = RoundToNearest(float(visibility) * factor2);
        }
    }

    new rank;
    if (mode == RENDER_NONE)
        rank = 0;
    else
    {
        rank = (visibility >= 0) ? visibility : 255;

        if (mode > RENDER_NORMAL && rank > 0)
            rank--;

        if (fx > RENDERFX_NONE && rank > _:fx)
            rank -= _:fx;

        if (rank >= 255 && original_visibility < 0)
            rank = original_visibility;
    }

    TraceReturn("rank=%d, visibility=%d, mode=%d, fx=%d", \
                rank, visibility, mode, fx);

    return rank;
}

SetVisibility(client, visibility, RenderMode:mode, RenderFx:fx,
              r, g, b, bool:colorWeapons, VisibilityType:type)
{
    SetTraceCategory("Visibility");
    TraceInto("PlayerProperties", "SetVisibility", \
              "client=%d:%L,visibility=%d,mode=%d,fx=%d,r=%d,g=%d,b=%d", \
              client, ValidClientIndex(client), visibility, mode, fx, r, g, b);

    // Change RENDER_NONE into RENDER_TRANSCOLOR with 0 alpha
    if (mode == RENDER_NONE)
    {
        visibility = 0;
        mode = RENDER_TRANSCOLOR;
    }
    else if (mode <= RENDER_NORMAL &&
             (visibility < 255 || r < 255 || g < 255 || b < 255))
    {
       mode = RENDER_TRANSCOLOR;
    }

    new bool:setMode = (GetEntityRenderMode(client) != mode);
    if (setMode)
    {
        SetEntityRenderMode(client,mode);
        m_VisibilityAltered[client]=true;
    }

    new bool:setFx = (GetEntityRenderFx(client) != fx);
    if (setFx)
    {
        SetEntityRenderFx(client,fx);
        m_VisibilityAltered[client]=true;
    }

    new bool:setAlpha = (GetEntityRenderAlpha(client) != visibility);
    new bool:setColor = (GetEntityRenderRed(client) != r ||
                         GetEntityRenderGreen(client) != g ||
                         GetEntityRenderBlue(client) != b);
    if (setAlpha || setColor)
    {
        SetEntityRenderColor(client,r,g,b,visibility);

        if (setColor)
            m_ColorAltered[client]=true;
            
        if (setAlpha)
            m_VisibilityAltered[client]=true;
    }

    if (setMode || setFx || setAlpha || setColor || colorWeapons ||
        m_VisibilityAltered[client] || m_ColorAltered[client] ||
        m_WeaponsAltered[client])
    {
        new r2, g2, b2;
        if (colorWeapons)
        {
            r2 = r;
            g2 = g;
            b2 = b;
            m_WeaponsAltered[client]=true;
        }
        else
        {
            r2 = 255;
            g2 = 255;
            b2 = 255;
        }

        new weapon = GetEntPropEnt(client, Prop_Send, "m_hActiveWeapon");
        if (weapon > 0 && IsValidEdict(weapon))
        {
            decl String:classname[CLASS_STRING_LENGTH];
            if (GetEdictClassname(weapon, classname, sizeof(classname)) &&
                StrContains(classname, "weapon") == 0)
            {
                // Set the visibility of the player's active weapon
                if (GetEntSendPropOffs(weapon, "m_nRenderFX") > -1 &&
                    GetEntityRenderFx(weapon) != fx)
                {
                    SetEntityRenderFx(weapon,fx);
                }

                if (GetEntSendPropOffs(weapon, "m_nRenderMode") > -1 &&
                    GetEntityRenderMode(weapon) != mode)
                {
                    SetEntityRenderMode(weapon,mode);
                }

                if (GetEntSendPropOffs(weapon, "m_clrRender") > -1 &&
                    GetEntityRenderAlpha(weapon) != visibility ||
                    GetEntityRenderRed(weapon) != r2 ||
                    GetEntityRenderGreen(weapon) != g2 ||
                    GetEntityRenderBlue(weapon) != b2)
                {
                    #if defined _TRACE
                        decl String:netclass[CLASS_STRING_LENGTH]; netclass[0] = '\0';
                        GetEntityNetClass(weapon, netclass, sizeof(netclass));
                        decl String:classname[CLASS_STRING_LENGTH]; classname[0] = '\0';
                        GetEdictClassname(weapon, classname, sizeof(classname));
                        TraceCat("Color","Setting %d:%L's weapon (entity=%d is a %s/%s) to color %d,%d,%d", \
                                 client, ValidClientIndex(client), weapon, classname, netclass, r2, b2, g2);
                    #endif

                    SetEntityRenderColor(weapon,r2,g2,b2,visibility);
                }
            }
        }
        else if (m_hMyWeapons)
        {
            // Set the visibility of ALL the player's weapons
            for (new i=0;i<47;i += 4)
            {
                weapon=GetEntDataEnt2(client,m_hMyWeapons + i);
                if (weapon > 0 && IsValidEdict(weapon))
                {
                    decl String:classname[CLASS_STRING_LENGTH];
                    if (GetEdictClassname(weapon, classname, sizeof(classname)) &&
                        StrContains(classname, "weapon") == 0)
                    {
                        if (GetEntSendPropOffs(weapon, "m_nRenderFX") > -1 &&
                            GetEntityRenderFx(weapon) != fx)
                        {
                            SetEntityRenderFx(weapon,fx);
                        }

                        if (GetEntSendPropOffs(weapon, "m_nRenderMode") > -1 &&
                            GetEntityRenderMode(weapon) != mode)
                        {
                            SetEntityRenderMode(weapon,mode);
                        }

                        if (GetEntSendPropOffs(weapon, "m_clrRender") > -1 &&
                            GetEntityRenderAlpha(weapon) != visibility ||
                            GetEntityRenderRed(weapon) != r2 ||
                            GetEntityRenderGreen(weapon) != g2 ||
                            GetEntityRenderBlue(weapon) != b2)
                        {
                            SetEntityRenderColor(weapon,r2,g2,b2,visibility);
                        }

                        #if defined _TRACE
                            decl String:netclass[CLASS_STRING_LENGTH]; netclass[0] = '\0';
                            GetEntityNetClass(weapon, netclass, sizeof(netclass));
                            TraceCat("Color","Setting %d:%L's %d weapon (entity=%d is a %s/%s) to color %d,%d,%d", \
                                     client, ValidClientIndex(client), i, weapon, classname, netclass, r2,g2,b2);
                        #endif
                    }
                    else
                    {
                        #if defined _TRACE
                            decl String:netclass[CLASS_STRING_LENGTH]; netclass[0] = '\0';
                            GetEntityNetClass(weapon, netclass, sizeof(netclass));
                            TraceCat("Color","%d:%L's %d weapon (entity=%d) is a %s/%s", \
                                     client, ValidClientIndex(client), i, weapon, classname, netclass);
                        #endif
                    }
                }
            }
        }

        if (GameType == tf2)
        {
            // Set the visibility of the Chargin Targe
            if (TF2_IsPlayerCharging(client))
            {
                weapon = GetPlayerWeaponSlot(client, 2);
                if (weapon > 0 && IsValidEdict(weapon))
                {
                    if (GetEntityRenderFx(weapon) != fx)
                        SetEntityRenderFx(weapon,fx);

                    if (GetEntityRenderMode(weapon) != mode)
                        SetEntityRenderMode(weapon,mode);

                    if (GetEntityRenderAlpha(weapon) != visibility ||
                        GetEntityRenderRed(weapon) != r2 ||
                        GetEntityRenderGreen(weapon) != g2 ||
                        GetEntityRenderBlue(weapon) != b2)
                    {
                        #if defined _TRACE
                            decl String:netclass[CLASS_STRING_LENGTH]; netclass[0] = '\0';
                            GetEntityNetClass(weapon, netclass, sizeof(netclass));
                            decl String:classname[CLASS_STRING_LENGTH]; classname[0] = '\0';
                            GetEdictClassname(weapon, classname, sizeof(classname));
                            TraceCat("Color","Setting %d:%L's targe (entity=%d is a %s/%s) to color %d,%d,%d", \
                                     client, ValidClientIndex(client), weapon, classname, netclass, r2, b2, g2);
                        #endif

                        SetEntityRenderColor(weapon,r2,g2,b2,visibility);
                    }
                }
            }

            SetWearablesVisibility(client,  "tf_wearable_item",
                                   visibility, mode, fx, r2, g2, b2);

            SetWearablesVisibility(client,  "tf_wearable_item_demoshield",
                                   visibility, mode, fx, r2, g2, b2);
        }
    }

    if (visibility == 0)
    {
        if (m_CloakTime[client] == 0.0)
            m_CloakTime[client] = GetGameTime(); // Start cloak timer

        if (g_RoundState >= RoundActive && g_RoundState < RoundOver &&
            (type == TimedMeleeInvisibility || type == TimedInvisibility) &&
            m_MonitorMotion[client] == INVALID_HANDLE)
        {
            m_MonitorMotion[client] = CreateTimer(0.1, CheckPlayerMotion, client,
                                                  TIMER_REPEAT);
        }
    }
    else
    {
        if (m_MonitorMotion[client] != INVALID_HANDLE)
        {
            KillTimer(m_MonitorMotion[client]);
            m_MonitorMotion[client] = INVALID_HANDLE;
        }

        if (m_CloakTime[client] > 0.0)
        {
            m_CloakTime[client] = 0.0;
            m_StoppedMoving[client] = 0.0; // Reset moving timer when cloak expires.
        }
    }

    TraceReturn();
}

SetWearablesVisibility(client,  const String:class[], visibility,
                       RenderMode:mode, RenderFx:fx, r, g, b)
{
    SetTraceCategory("Visibility");
    TraceInto("PlayerProperties", "SetWearablesVisibility", \
              "client=%d:%L,class=%s,visibility=%d,mode=%d,fx=%d,r=%d,g=%d,b=%d", \
              client, ValidClientIndex(client), class, visibility, mode, fx, r, g, b);

	new ent = -1;
	while( (ent = FindEntityByClassname(ent, class)) != -1 )
	{
        if (GetEntPropEnt(ent, Prop_Data, "m_hOwnerEntity") == client)
        {
            if (GetEntityRenderFx(ent) != fx)
                SetEntityRenderFx(ent, fx);

            if (GetEntityRenderMode(ent) != mode)
                SetEntityRenderMode(ent, mode);

            if (GetEntityRenderAlpha(ent) != visibility ||
                GetEntityRenderRed(ent) != r ||
                GetEntityRenderGreen(ent) != g ||
                GetEntityRenderBlue(ent) != b)
            {
                SetEntityRenderColor(ent, r,g,b,visibility);

                #if defined _TRACE
                    decl String:netclass[CLASS_STRING_LENGTH];
                    GetEntityNetClass(weapon, netclass, sizeof(netclass));
                    TraceCat("Color","Setting %d:%L's item (entity=%d is a %s/%s) to color %d,%d,%d", \
                             client, ValidClientIndex(client), ent, class, netclass, r2, b2, g2);
                #endif
            }
        }
	}
    TraceReturn();
}

public Action:CheckPlayerMotion(Handle:timer, any:client)
{
    TraceInto("PlayerProperties", "CheckPlayerMotion", "client=%d:%L", \
              client, ValidClientIndex(client));

    if (client > 0 && IsClientInGame(client) && IsPlayerAlive(client))
    {
        if ((GetClientButtons(client) & (IN_FORWARD   | IN_BACK    | IN_MOVELEFT |
                                         IN_MOVERIGHT | IN_SPEED   | IN_JUMP |
                                         IN_ATTACK    | IN_ATTACK2 | IN_ZOOM)))
        {
            m_IsMoving[client] = true;
            m_StoppedMoving[client] = 0.0;
            m_MonitorMotion[client] = INVALID_HANDLE;

            if (g_RoundState < RoundOver)
            {
                UpdatePlayerProperties(client);
            }

            TraceReturn("%L IS moving", client);
            return Plugin_Stop;
        }

        TraceReturn("%L is not moving");
        return Plugin_Continue;
    }

    m_MonitorMotion[client] = INVALID_HANDLE;

    TraceReturn();
    return Plugin_Stop;
}

ResetVisibility(client)
{
    SetTraceCategory("Visibility");
    TraceInto("PlayerProperties", "ResetVisibility", "client=%d:%L", \
              client, ValidClientIndex(client));

    if (client>0)
    {
        Trace("Reset %d:%L's Visibility", \
              client, ValidClientIndex(client));

        if (GetEntityRenderFx(client) != RENDERFX_NONE)
            SetEntityRenderFx(client, RENDERFX_NONE);

        if (GetEntityRenderMode(client) != RENDER_NORMAL)
            SetEntityRenderMode(client, RENDER_NORMAL);

        if (GetEntityRenderAlpha(client) != 255 ||
            GetEntityRenderRed(client) != 255 ||
            GetEntityRenderGreen(client) != 255 ||
            GetEntityRenderBlue(client) != 255)
        {
            SetEntityRenderColor(client,255,255,255,255);
        }

        // Reset the visibility of players active weapon
        new weapon = GetEntPropEnt(client, Prop_Send, "m_hActiveWeapon");
        if (weapon > 0 && IsValidEdict(weapon))
        {
            decl String:classname[CLASS_STRING_LENGTH];
            if (GetEdictClassname(weapon, classname, sizeof(classname)) &&
                StrContains(classname, "weapon") == 0)
            {
                if (GetEntSendPropOffs(weapon, "m_nRenderFX") > -1 &&
                    GetEntityRenderFx(weapon) != RENDERFX_NONE)
                {
                    SetEntityRenderFx(weapon, RENDERFX_NONE);
                }

                if (GetEntSendPropOffs(weapon, "m_nRenderMode") > -1 &&
                    GetEntityRenderMode(weapon) != RENDER_NORMAL)
                {
                    SetEntityRenderMode(weapon, RENDER_NORMAL);
                }

                if (GetEntSendPropOffs(weapon, "m_clrRender") > -1 &&
                    GetEntityRenderAlpha(weapon) != 255 ||
                    GetEntityRenderRed(weapon) != 255 ||
                    GetEntityRenderGreen(weapon) != 255 ||
                    GetEntityRenderBlue(weapon) != 255)
                {
                    SetEntityRenderColor(weapon,255,255,255,255);
                }
            }
        }

        if (m_hMyWeapons)
        {
            // Just to be sure, reset the visibility of ALL of the players weapons
            for (new i=0;i<47;i += 4)
            {
                weapon=GetEntDataEnt2(client,m_hMyWeapons + i);
                if (weapon > 0 && IsValidEdict(weapon))
                {
                    decl String:classname[CLASS_STRING_LENGTH];
                    if (GetEdictClassname(weapon, classname, sizeof(classname)) &&
                        StrContains(classname, "weapon") == 0)
                    {
                        if (GetEntSendPropOffs(weapon, "m_nRenderFX") > -1 &&
                            GetEntityRenderFx(weapon) != RENDERFX_NONE)
                        {
                            SetEntityRenderFx(weapon, RENDERFX_NONE);
                        }

                        if (GetEntSendPropOffs(weapon, "m_nRenderMode") > -1 &&
                            GetEntityRenderMode(weapon) != RENDER_NORMAL)
                        {
                            SetEntityRenderMode(weapon, RENDER_NORMAL);
                        }

                        if (GetEntSendPropOffs(weapon, "m_clrRender") > -1 &&
                            GetEntityRenderAlpha(weapon) != 255 ||
                            GetEntityRenderRed(weapon) != 255 ||
                            GetEntityRenderGreen(weapon) != 255 ||
                            GetEntityRenderBlue(weapon) != 255)
                        {
                            SetEntityRenderColor(weapon,255,255,255,255);
                        }

                        #if defined _TRACE
                            decl String:netclass[CLASS_STRING_LENGTH]; netclass[0] = '\0';
                            GetEntityNetClass(weapon, netclass, sizeof(netclass));
                            Trace("%d:%L's %d weapon (entity=%d) is a %s/%s", \
                                  client, ValidClientIndex(client), i, weapon, classname, netclass);
                        #endif
                    }
                    else
                    {
                        #if defined _TRACE
                            decl String:netclass[CLASS_STRING_LENGTH]; netclass[0] = '\0';
                            GetEntityNetClass(weapon, netclass, sizeof(netclass));
                            Trace("%d:%L's %d weapon (entity=%d) is a %s/%s", \
                                  client, ValidClientIndex(client), i, weapon, classname, netclass);
                        #endif
                    }
                }
            }
        }

        if (GetGameType() == tf2)
        {
            // Reset the visibility of the Chargin Targe
            if (TF2_IsPlayerCharging(client))
            {
                weapon = GetPlayerWeaponSlot(client, 2);
                if (weapon > 0 && IsValidEdict(weapon))
                {
                    if (GetEntityRenderFx(weapon) != RENDERFX_NONE)
                        SetEntityRenderFx(weapon, RENDERFX_NONE);

                    if (GetEntityRenderMode(weapon) != RENDER_NORMAL)
                        SetEntityRenderMode(weapon, RENDER_NORMAL);

                    if (GetEntityRenderAlpha(weapon) != 255 ||
                        GetEntityRenderRed(weapon) != 255 ||
                        GetEntityRenderGreen(weapon) != 255 ||
                        GetEntityRenderBlue(weapon) != 255)
                    {
                        SetEntityRenderColor(weapon,255,255,255,255);
                    }
                }
            }

            // Reset the visibility of any wearable items
            SetWearablesVisibility(client,  "tf_wearable_item",
                                   255, RENDER_NORMAL, RENDERFX_NONE, 255, 255, 255);

            SetWearablesVisibility(client,  "tf_wearable_item_demoshield",
                                   255, RENDER_NORMAL, RENDERFX_NONE, 255, 255, 255);
        }
    }

    TraceReturn();
}
