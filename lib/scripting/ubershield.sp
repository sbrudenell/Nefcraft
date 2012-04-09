/*
 * vim: set ai et ts=4 sw=4 :
 * File: ubershield.sp
 * Description: Uber Bubble Shield for TF2
 * Author(s): Naris
 */

#pragma semicolon 1

#include <sourcemod>
#include <sdktools>

#undef REQUIRE_EXTENSIONS
#include <tf2_stocks>
#define REQUIRE_EXTENSIONS

// Colors
#define COLOR_DEFAULT           0x01
#define COLOR_TEAM              0x03
#define COLOR_GREEN             0x04 // Actually red for DOD

#define MDL_REDSHIELD           "models/shield/redshield.mdl"
#define MDL_BLUESHIELD          "models/shield/blueshield.mdl"

#define SND_ERROR               "common/wpn_denyselect.wav"
#define SND_ACTIVE              "ambient/machines/thumper_amb.wav"
#define SND_START               "ambient/machines/thumper_startup1.wav"
#define SND_STOP                "ambient/machines/thumper_shutdown1.wav"

#define SND_START_TIME          "2.0"
#define SND_STOP_TIME           "2.0"

new const String:ShieldModelFiles[][]  = {
    "models/shield/redshield.mdl",
    "models/shield/redshield.phy",
    "models/shield/redshield.vvd",
    "models/shield/redshield.sw.vtx",
    "models/shield/redshield.dx80.vtx",
    "models/shield/redshield.dx90.vtx",
    "materials/models/shield/redshield.vmt",

    "models/shield/blueshield.mdl",
    "models/shield/blueshield.phy",
    "models/shield/blueshield.vvd",
    "models/shield/blueshield.sw.vtx",
    "models/shield/blueshield.dx80.vtx",
    "models/shield/blueshield.dx90.vtx",
    "materials/models/shield/blueshield.vmt" };

enum ShieldFlags (<<= 1)
{
    Shield_None = 0,            // No shield allowed
    Shield_Normal = 1,          // Shield stops bullets but not players
    Shield_Immobilize,          // Shield immobilizes everything
	Shield_Team_Specific,	    // Shield is team specific
    Shield_Mobile,              // Shield is mobile (parented to target)
    Shield_Target_Self,         // Sheild can target self
	Shield_Target_Team,	        // Shield can target teammates
    Shield_Target_Enemy,        // Shield can target enemies
    Shield_Target_Location,     // Shield can target locations (position)
    Shield_With_Medigun,        // Shield can be invoked with Medigun
    Shield_With_Kritzkrieg,     // Shield can be invoked with Kritzkrieg
    Shield_Reload_Normal,       // Reload Shield stops bullets but not players
    Shield_Reload_Immobilize,   // Reload Shield immobilizes everything
	Shield_Reload_Team_Specific,// Reload Shield is team specific
    Shield_Reload_Mobile,       // Reload Shield is mobile (parented to target)
    Shield_Reload_Self,         // Reload Sheild can target self
	Shield_Reload_Team,	        // Reload Shield can target teammates
    Shield_Reload_Enemy,        // Reload Shield can target enemies
    Shield_Reload_Location,     // Reload Shield can target locations (position)
    Shield_UseAlternateSounds,  // Use the Alternate Shield Sounds
    Shield_DisableStartSound,   // Disable the Start Shield Sound
    Shield_DisableActiveSound,  // Disable the Active Shield Sound
    Shield_DisableStopSound     // Disable the Stop Shield Sound
};

#define ShieldNormalMask    (Shield_Normal|Shield_Immobilize|Shield_Team_Specific|Shield_Mobile|Shield_Target_Self|Shield_Target_Team|Shield_Target_Enemy|Shield_Target_Location)
#define ShieldReloadMask    (Shield_Reload_Normal|Shield_Reload_Immobilize|Shield_Reload_Team_Specific|Shield_Reload_Mobile|Shield_Reload_Self|Shield_Reload_Team|Shield_Reload_Location)
#define ShieldDefault       (ShieldNormalMask|ShieldReloadMask)

new gVelocityOffset;
new gShieldRedModelIndex;
new gShieldBlueModelIndex;
new Float:gAlternateStartTime;
new Float:gAlternateStopTime;
new Float:gStartTime;
new Float:gStopTime;

new String:mdlRedShield[256];
new String:mdlBlueShield[256];

new String:gAlternateActiveSound[PLATFORM_MAX_PATH];
new String:gAlternateStartSound[PLATFORM_MAX_PATH];
new String:gAlternateStopSound[PLATFORM_MAX_PATH];
new String:gStartSound[PLATFORM_MAX_PATH];
new String:gActiveSound[PLATFORM_MAX_PATH];
new String:gStopSound[PLATFORM_MAX_PATH];

new gField[MAXPLAYERS+1];	            // EntRef for the player's field
new gShield[MAXPLAYERS+1];	            // EntRef for the player's shield
new gParent[MAXPLAYERS+1];	            // EntRef for entity to "parent" to the player
new gAllowed[MAXPLAYERS+1];             // how many shields player allowed
new gRemaining[MAXPLAYERS+1];           // how many shields player has this spawn
new ShieldFlags:gFlags[MAXPLAYERS+1];   // Shield type/permissions flags for each player
new bool:m_ShieldCharged[MAXPLAYERS+1]; // Shield charged flags for each player
new Handle:gObjectTimers[MAXPLAYERS+1] = { INVALID_HANDLE, ... };

new Handle:cvRedModel = INVALID_HANDLE;
new Handle:cvBlueModel = INVALID_HANDLE;

new Handle:cvActiveSound = INVALID_HANDLE;
new Handle:cvStartSound = INVALID_HANDLE;
new Handle:cvStartTime = INVALID_HANDLE;
new Handle:cvStopSound = INVALID_HANDLE;
new Handle:cvStopTime = INVALID_HANDLE;

new Handle:cvPerLife = INVALID_HANDLE;
new Handle:cvMinUber = INVALID_HANDLE;
new Handle:cvMaxUber = INVALID_HANDLE;
new Handle:cvKritzkrieg = INVALID_HANDLE;
new Handle:cvMaxDuration = INVALID_HANDLE;
new Handle:cvRechargeTime = INVALID_HANDLE;
new Handle:cvMediShieldFlags = INVALID_HANDLE;
new Handle:cvMediReloadShieldFlags = INVALID_HANDLE;

new bool:gNativeControl = false;

// forwards
new Handle:fwdOnDeployUberShield;

public Plugin:myinfo = 
{
    name = "Uber Shield",
    author = "-=|JFH|=-Naris",
    description = "Allows medics to generate an uber bubble shield",
    version = "1.2",
    url = "http://www.jigglysfunhouse.net/"
};

/**
 * Description: Function to determine game/mod type
 */
#tryinclude <gametype>
#if !defined _gametype_included
    enum Game { undetected, tf2, cstrike, dod, hl2mp, insurgency, zps, l4d, l4d2, other_game };
    stock Game:GameType = undetected;

    stock Game:GetGameType()
    {
        if (GameType == undetected)
        {
            new String:modname[30];
            GetGameFolderName(modname, sizeof(modname));
            if (StrEqual(modname,"cstrike",false))
                GameType=cstrike;
            else if (StrEqual(modname,"tf",false)) 
                GameType=tf2;
            else if (StrEqual(modname,"dod",false)) 
                GameType=dod;
            else if (StrEqual(modname,"hl2mp",false)) 
                GameType=hl2mp;
            else if (StrEqual(modname,"Insurgency",false)) 
                GameType=insurgency;
            else if (StrEqual(modname,"left4dead", false)) 
                GameType=l4d;
            else if (StrEqual(modname,"left4dead2", false)) 
                GameType=l4d2;
            else if (StrEqual(modname,"zps",false)) 
                GameType=zps;
            else
                GameType=other_game;
        }
        return GameType;
    }
#endif

/**
 * Description: Function to check the entity limit.
 *              Use before spawning an entity.
 */
#tryinclude <entlimit>
#if !defined _entlimit_included
    stock IsEntLimitReached(warn=20,critical=16,client=0,const String:message[]="")
    {
        new max = GetMaxEntities();
        new count = GetEntityCount();
        new remaining = max - count;
        if (remaining <= warn)
        {
            if (count <= critical)
            {
                PrintToServer("Warning: Entity limit is nearly reached! Please switch or reload the map!");
                LogError("Entity limit is nearly reached: %d/%d (%d):%s", count, max, remaining, message);

                if (client > 0)
                {
                    PrintToConsole(client, "Entity limit is nearly reached: %d/%d (%d):%s",
                                   count, max, remaining, message);
                }
            }
            else
            {
                PrintToServer("Caution: Entity count is getting high!");
                LogMessage("Entity count is getting high: %d/%d (%d):%s", count, max, remaining, message);

                if (client > 0)
                {
                    PrintToConsole(client, "Entity count is getting high: %d/%d (%d):%s",
                                   count, max, remaining, message);
                }
            }
            return count;
        }
        else
            return 0;
    }
#endif

/**
 * Description: Stocks to return information about TF2 UberCharge.
 */
#tryinclude <tf2_uber>
#if !defined _tf2_uber_included
    stock TF2_IsUberCharge(client)
    {
        new index = GetPlayerWeaponSlot(client, 1);
        if (index > 0)
            return GetEntProp(index, Prop_Send, "m_bChargeRelease", 1);
        else
            return 0;
    }

    stock Float:TF2_GetUberLevel(client)
    {
        new index = GetPlayerWeaponSlot(client, 1);
        if (index > 0)
            return GetEntPropFloat(index, Prop_Send, "m_flChargeLevel");
        else
            return 0.0;
    }

    stock TF2_SetUberLevel(client, Float:uberlevel)
    {
        new index = GetPlayerWeaponSlot(client, 1);
        if (index > 0)
            SetEntPropFloat(index, Prop_Send, "m_flChargeLevel", uberlevel);
    }
#endif

/**
 * Description: Ray Trace functions and variables
 */
#tryinclude <raytrace>
#if !defined _raytrace_included
    stock bool:TraceAimPosition(client, Float:destLoc[3], bool:hitPlayers)
    {
        new Float:clientloc[3],Float:clientang[3];
        GetClientEyePosition(client,clientloc);
        GetClientEyeAngles(client,clientang);

        if (hitPlayers)
        {
            TR_TraceRayFilter(clientloc, clientang, MASK_SOLID,
                              RayType_Infinite, TraceRayDontHitSelf,
                              client);
        }
        else
        {
            TR_TraceRayFilter(clientloc, clientang, MASK_SOLID,
                              RayType_Infinite, TraceRayDontHitPlayers,
                              client);
        }

        TR_GetEndPosition(destLoc);
        return TR_DidHit();
    }

    /***************
     *Trace Filters*
    ****************/

    public bool:TraceRayDontHitPlayers(entity,mask)
    {
      // Check if the beam hit a player and tell it to keep tracing if it did
      return (entity <= 0 || entity > MaxClients);
    }

    public bool:TraceRayDontHitSelf(entity, mask, any:data)
    {
        return (entity != data); // Check if the TraceRay hit the owning entity.
    }
#endif

/**
 * Description: Manage precaching resources.
 */
#tryinclude "ResourceManager"
#if !defined _ResourceManager_included
	// Trie to hold precache status of sounds
	new Handle:g_precacheTrie = INVALID_HANDLE;

	stock PrepareSound(const String:sound[], bool:preload=false)
	{
    		// If the sound hasn't been played yet, precache it first
    		// :( IsSoundPrecached() doesn't work ):
    		//if (!IsSoundPrecached(sound))
    		new bool:value;
    		if (!GetTrieValue(g_precacheTrie, sound, value))
    		{
			    PrecacheSound(sound,preload);
			    SetTrieValue(g_precacheTrie, sound, true);
    		}
	}

	stock SetupSound(const String:sound[], download=1,
	                  bool:precache=false, bool:preload=false)
	{
		decl String:dl[PLATFORM_MAX_PATH+1];
		Format(dl, sizeof(dl), "sound/%s", sound);

		if (download && FileExists(dl))
			AddFileToDownloadsTable(dl);

		if (precache)
			PrecacheSound(sound, preload);
	}

    stock SetupModel(const String:model[], &index, bool:download,
                     bool:precache, bool:preload=false)
    {
        if (download && FileExists(model))
            AddFileToDownloadsTable(model);

        if (precache)
            index = PrecacheModel(model,preload);
        else
            index = 0;
    }

    stock PrepareModel(const String:model[], &index, bool:preload=false)
    {
        if (index <= 0)
            index = PrecacheModel(model,preload);

        return index;
    }
#endif

public APLRes:AskPluginLoad2(Handle:myself, bool:late, String:error[], err_max)
{
    // Register Natives
    CreateNative("SetAlternateShieldSound",Native_SetAlternateShieldSound);
    CreateNative("UberShieldLocation",Native_UberShieldLocation);
    CreateNative("ControlUberShield",Native_ControlUberShield);
    CreateNative("UberShieldTarget",Native_UberShieldTarget);
    CreateNative("GiveUberShield",Native_GiveUberShield);
    CreateNative("TakeUberShield",Native_TakeUberShield);
    CreateNative("UberShield",Native_UberShield);

    // Register Forwards
    fwdOnDeployUberShield=CreateGlobalForward("OnDeployUberShield",ET_Hook,Param_Cell,Param_Cell);

    RegPluginLibrary("ubershield");
    return APLRes_Success;
}

public OnPluginStart()
{
    HookEvent("player_team", PlayerDeathEvent);
    HookEvent("player_death", PlayerDeathEvent);
    HookEvent("player_spawn", PlayerSpawnEvent);

    if (GetGameType() == tf2)
    {
        HookEvent("player_changeclass", PlayerDeathEvent);
        HookEvent("teamplay_flag_event", EventFlagEvent);

        cvRedModel = CreateConVar("sm_shield_red_model", MDL_REDSHIELD, "Red Shield model");
        cvBlueModel = CreateConVar("sm_shield_blue_model", MDL_BLUESHIELD, "Blue Shield model");

        cvMinUber = CreateConVar("sm_shield_min_uber", "0.30", "Minimum uber required for shield");
        cvMaxUber = CreateConVar("sm_shield_max_uber", "0.90", "Maximum uber for shield to deploy with Alt. Attack");
        cvRechargeTime = CreateConVar("sm_shield_recharge", "0.0", "Time it takes the shield to recharge after use");

        cvKritzkrieg = CreateConVar("sm_shield_kritkrieg", "1", "Shield is invoked with the Kritzkrieg");
        cvMediShieldFlags = CreateConVar("sm_shield_medi_type", "4", "Flags for shield medigun creates (0=none,1=normal,2=immobilize,4=team specific,8=shield moves,16=target self,32=target team,64=target enemy,128=target location)");
        //cvMediReloadShieldFlags = CreateConVar("sm_shield_medi_reload_type", "148");
        cvMediReloadShieldFlags = CreateConVar("sm_shield_medi_reload_type", "4", "Flags for shield medigun reload creates (0=none,1=normal,2=immobilize,4=team specific,8=shield moves,16=target self,32=target team,64=target enemy,128=target location)");
    }
    else
    {
        if (GameType == dod)
        {
            cvRedModel = CreateConVar("sm_shield_axis_model", MDL_REDSHIELD, "Axis Shield model");
            cvBlueModel = CreateConVar("sm_shield_allies_model", MDL_BLUESHIELD, "Allies Shield model");
        }
        else if (GameType == cstrike)
        {
            cvRedModel = CreateConVar("sm_shield_t_model", MDL_REDSHIELD, "Terrorists Shield model");
            cvBlueModel = CreateConVar("sm_shield_ct_model", MDL_BLUESHIELD, "Counter-Terrorists Shield model");
        }
        else
        {
            cvRedModel = CreateConVar("sm_shield_model_1", MDL_REDSHIELD, "Team 1's Shield model (Actually Team 2)");
            cvBlueModel = CreateConVar("sm_shield_model_2", MDL_BLUESHIELD, "Team 2's Shield model (Actually Team 3)");
        }

        cvRechargeTime = CreateConVar("sm_shield_recharge", "60.0", "Time it takes the shield to recharge after use");
        cvMediShieldFlags = CreateConVar("sm_shield_type", "4", "Flags for shield (0=none,1=normal,2=immobilize,4=team specific,8=shield moves,16=target self,32=target team,64=target enemy,128=target location)");
    }

    cvMaxDuration = CreateConVar("sm_shield_max_duration", "10.0", "Maximum duration of the shield");
    cvPerLife = CreateConVar("sm_shield_per_life", "-1", "Number of shields allowed per life (-1 = unlimited)");

    cvActiveSound = CreateConVar("sm_shield_active_sound", SND_ACTIVE, "Shield Start Sound");
    cvStartSound = CreateConVar("sm_shield_start_sound", SND_START, "Shield Start Sound");
    cvStartTime = CreateConVar("sm_shield_start_time", SND_START_TIME, "Shield Start Sound Duration");
    cvStopSound = CreateConVar("sm_shield_stop_sound", SND_STOP, "Shield Stop Sound");
    cvStopTime = CreateConVar("sm_shield_stop_time", SND_STOP_TIME, "Shield Stop Sound Duration");

    gVelocityOffset = FindSendPropInfo("CBasePlayer", "m_vecVelocity[0]");

    RegConsoleCmd("shield", ShieldCommand, "Deploy the shield");
}

public OnConfigsExecuted()
{
    // set models based on cvar
    GetConVarString(cvRedModel, mdlRedShield, sizeof(mdlRedShield));
    GetConVarString(cvBlueModel, mdlBlueShield, sizeof(mdlBlueShield));

    // set sounds based on cvar
    GetConVarString(cvStartSound, gStartSound, sizeof(gStartSound));
    GetConVarString(cvActiveSound, gActiveSound, sizeof(gActiveSound));
    GetConVarString(cvStopSound, gStopSound, sizeof(gStopSound));

    gStartTime = GetConVarFloat(cvStartTime);
    gStopTime = GetConVarFloat(cvStopTime);

    #if !defined _ResourceManager_included
        // Setup trie to keep track of precached sounds
        if (g_precacheTrie == INVALID_HANDLE)
                g_precacheTrie = CreateTrie();
        else
                ClearTrie(g_precacheTrie);
    #endif

    gShieldRedModelIndex = 0;
    gShieldBlueModelIndex = 0;

    if (gStopSound[0])
        SetupSound(gStartSound,true,false);

    if (gActiveSound[0])
        SetupSound(gActiveSound,true,false);

    if (gStopSound[0])
        SetupSound(gStopSound,true,false);

    if (gAlternateStartSound[0])
        SetupSound(gAlternateStartSound,true,false);

    if (gAlternateStopSound[0])
        SetupSound(gAlternateStopSound,true,false);

    if (gAlternateActiveSound[0])
        SetupSound(gAlternateActiveSound,true,false);

    // Added custom models to download table
    for (new i = 0; i < sizeof(ShieldModelFiles); i++)
        AddFileToDownloadsTable(ShieldModelFiles[i]);
}

public OnClientPutInServer(client)
{
    gField[client] = 0;
    gShield[client] = 0;
    gParent[client] = 0;
    m_ShieldCharged[client]=true;

    if (!gNativeControl)
        CreateTimer(45.0, Timer_Advert, client);
}

public OnClientDisconnect(client)
{
    RemoveShield(client);

    new Handle:timer=gObjectTimers[client];
    if (timer != INVALID_HANDLE)
    {
        gObjectTimers[client] = INVALID_HANDLE;
        KillTimer(timer, true);
    }
}

// Hack around new limitation of effectivly no longer being
// able to parent entities to players, so fake it.
public OnGameFrame()
{
    for(new i = 1; i <= MaxClients; i++)
    {
        new ref = gParent[i];
        if (ref != 0)
        {
            new ent = EntRefToEntIndex(ref);
            if (ent > 0)
            {
                new Float:pos[3];
                GetClientAbsOrigin(i, pos);

                new Float:vel[3];
                GetEntDataVector(i, gVelocityOffset, vel);

                TeleportEntity(ent, pos, NULL_VECTOR, vel);
            }
        }
    }
}

public PlayerSpawnEvent(Handle:event,const String:name[],bool:dontBroadcast)
{
    new client=GetClientOfUserId(GetEventInt(event,"userid")); // Get clients index
    new TFClassType:class = (GameType == tf2) ? TF2_GetPlayerClass(client) : TFClass_Unknown;

    if (gNativeControl)
    {
        gRemaining[client] = gAllowed[client];
    }
    else if (GameType == tf2)
    {
        gRemaining[client] = (class == TFClass_Medic) ? GetConVarInt(cvPerLife) : 0;
        gFlags[client] = (GetConVarBool(cvKritzkrieg) ? Shield_With_Kritzkrieg
                          : Shield_With_Medigun | ShieldFlags:GetConVarInt(cvMediShieldFlags) |
                            ShieldFlags:(GetConVarInt(cvMediReloadShieldFlags) >> 10));
    }
    else
    {
        gRemaining[client] = 0;
        gFlags[client] = Shield_None;
    }
}

public PlayerDeathEvent(Handle:event,const String:name[],bool:dontBroadcast)
{
    if (GameType == tf2)
    {
        // Skip feigned deaths.
        if (GetEventInt(event, "death_flags") & TF_DEATHFLAG_DEADRINGER)
            return;

        // Skip fishy deaths.
        if (GetEventInt(event, "weaponid") == TF_WEAPON_BAT_FISH &&
            GetEventInt(event, "customkill") != TF_CUSTOM_FISH_KILL)
        {
            return;
        }
    }

    new client=GetClientOfUserId(GetEventInt(event,"userid")); // Get clients index

    RemoveShield(client);

    new Handle:timer=gObjectTimers[client];
    if (timer != INVALID_HANDLE)
    {
        gObjectTimers[client] = INVALID_HANDLE;
        KillTimer(timer, true);
    }
}

public EventFlagEvent(Handle:event,const String:name[],bool:dontBroadcast)
{
    new player = GetEventInt(event,"player");
    if (player > 0 && IsClientInGame(player))
    {
        new eventtype = GetEventInt(event,"eventtype");
        if (eventtype == 1) // Flag Picked Up
        {
            // Remove the UberShield and it's timer
            new Handle:timer=gObjectTimers[player];
            if (timer != INVALID_HANDLE)
            {
                gObjectTimers[player] = INVALID_HANDLE;
                KillTimer(timer, true);
            }

            RemoveShield(player);
        }
    }
}

public Action:Timer_Advert(Handle:timer, any:client)
{
    if (IsClientConnected(client) && IsClientInGame(client))
    {
        if (GameType == tf2)
        {
            PrintToChat(client, "\x01\x04[SM]\x01 Medics are able to invoke a Uber Shield by using their %s secondary fire or Reload buttons",
                        GetConVarBool(cvKritzkrieg) ? "Kritzkrieg's" : "Medipack's");
        }
        else
        {
            PrintToChat(client, "\x01\x04[SM]\x01 Bind a key to shield to invoke the shield");
        }
    }
}

public Action:ShieldCommand(client, args)
{
    new ShieldFlags:flags = gFlags[client];
    new Float:duration    = GetConVarFloat(cvMaxDuration);

    if (GetGameType() == tf2)
    {
        if (TF2_GetPlayerClass(client) == TFClass_Medic)
        {
            new Float:UberCharge = TF2_GetUberLevel(client);
            if (UberCharge >= GetConVarFloat(cvMinUber))
            {
                duration *= UberCharge;
                TF2_SetUberLevel(client, 0.0);
            }
            else
            {
                PrepareSound(SND_ERROR);
                EmitSoundToClient(client, SND_ERROR);
            }
        }
        else
        {
            PrepareSound(SND_ERROR);
            EmitSoundToClient(client, SND_ERROR);
        }
    }
    else
    {
        if (flags & Shield_Target_Location)
            ShieldLocation(client, flags, duration);
        else
            ShieldTarget(client, flags, duration);
    }

    return Plugin_Handled;
}

public Action:OnPlayerRunCmd(client, &buttons, &impulse, Float:vel[3], Float:angles[3], &weapon)
{
    if (buttons & (IN_RELOAD|IN_ATTACK2) != 0 && gRemaining[client] != 0 &&
        TF2_GetPlayerClass(client) == TFClass_Medic && TF2_IsUberCharge(client) == 0) 
    {
        new weaponent = GetEntPropEnt(client, Prop_Send, "m_hActiveWeapon");
        if (weaponent > 0)
        {
            decl String:classname[32];
            if (GetEdictClassname(weaponent, classname , sizeof(classname)) &&
                StrEqual(classname, "tf_weapon_medigun") )
            {
                new ShieldFlags:flags = gFlags[client];
                if (GetEntProp(weaponent, Prop_Send, "m_iItemDefinitionIndex") == 35) // Kritzkrieg
                {
                    if (_:(flags & Shield_With_Kritzkrieg) == 0)
                        return Plugin_Continue;
                }
                else // Medigun
                {
                    if (_:(flags & Shield_With_Medigun) == 0)
                        return Plugin_Continue;
                }

                new Float:UberCharge = TF2_GetUberLevel(client);
                if (buttons & IN_ATTACK2)
                {
                    if (UberCharge >= GetConVarFloat(cvMinUber) &&
                        UberCharge < GetConVarFloat(cvMaxUber))
                    {
                        new Float:duration = GetConVarFloat(cvMaxDuration)*UberCharge;
                        if (ShieldTarget(client, flags, duration))
                            TF2_SetUberLevel(client, 0.0);
                    }
                    else
                    {
                        PrepareSound(SND_ERROR);
                        EmitSoundToClient(client, SND_ERROR);
                    }
                }
                else if (buttons & IN_RELOAD)
                {
                    if (UberCharge >= GetConVarFloat(cvMinUber))
                    {
                        new Float:duration = GetConVarFloat(cvMaxDuration)*UberCharge;
                        if (flags & Shield_Reload_Location)
                        {
                            // Copy the Reload bits into the "normal" bits location
                            new ShieldFlags:reloadFlags = ((flags & ShieldReloadMask) >> ShieldFlags:10);
                            flags &= ~ShieldNormalMask; // Mask off the old bits
                            flags |= reloadFlags; // or in the reload ones.
                            if (ShieldLocation(client, flags, duration))
                                TF2_SetUberLevel(client, 0.0);
                        }
                        else
                        {
                            // Copy the Reload bits into the "normal" bits location
                            new ShieldFlags:reloadFlags = ((flags & ShieldReloadMask) >> ShieldFlags:10);
                            flags &= ~ShieldNormalMask; // Mask off the old bits
                            flags |= reloadFlags; // or in the reload ones.
                            if (ShieldTarget(client, flags, duration))
                                TF2_SetUberLevel(client, 0.0);
                        }
                    }
                    else
                    {
                        PrepareSound(SND_ERROR);
                        EmitSoundToClient(client, SND_ERROR);
                    }
                }
            }
        }
    }
    return Plugin_Continue;
}

bool:ShieldTarget(client, ShieldFlags:flags, Float:duration)
{
    new target = 0;
    new Float:targetLoc[3];
    if (flags & (Shield_Target_Self|Shield_Target_Team|Shield_Target_Enemy))
    {
        target = GetClientAimTarget(client);
        if (target > 0 && target != client)
        {
            if (GetClientTeam(target) != GetClientTeam(client))
            {
                if ( !(flags & Shield_Target_Enemy))
                {
                    PrepareSound(SND_ERROR);
                    EmitSoundToClient(client, SND_ERROR);
                    return false;
                }
            }
            else
            {
                if ( !(flags & Shield_Target_Team))
                {
                    PrepareSound(SND_ERROR);
                    EmitSoundToClient(client, SND_ERROR);
                    return false;
                }
            }
        }
        else if (flags & Shield_Target_Self)
            target = client;

        if (target > 0)
            GetClientEyePosition(target, targetLoc);
        else if (flags & Shield_Target_Location)
            TraceAimPosition(client, targetLoc, true);
        else
            GetClientEyePosition(client, targetLoc);
    }
    else
        GetClientAbsOrigin(client, targetLoc);

    return (CreateShield(client, target, targetLoc, flags, duration) != 0);
}

bool:ShieldLocation(client, ShieldFlags:flags, Float:duration)
{
    if (flags & Shield_Target_Location)
    {
        new Float:targetLoc[3];
        TraceAimPosition(client, targetLoc, true);
        return (CreateShield(client, 0, targetLoc, flags, duration) != 0);
    }
    else
    {
        PrepareSound(SND_ERROR);
        EmitSoundToClient(client, SND_ERROR);
        return false;
    }
}

CreateShield(client, target, const Float:pos[3], ShieldFlags:flags, Float:Duration)
{
    // Check for limit or shield not charged.
    if (gRemaining[client] == 0 || !m_ShieldCharged[client] ||
        IsEntLimitReached(.client=client,.message="unable to create shield"))
    {
        PrepareSound(SND_ERROR);
        EmitSoundToClient(client, SND_ERROR);
        return 0;
    }

    // Check with other plugins/forward (if any)
    new Action:res = Plugin_Continue;
    Call_StartForward(fwdOnDeployUberShield);
    Call_PushCell(client);
    Call_PushCell(target);
    Call_Finish(res);

    if (res != Plugin_Continue)
        return 0;

    // Only 1 shield allowed per client
    RemoveShield(client);

    new shield = CreateEntityByName("prop_dynamic_override"); // ("prop_prop_override");

    if (shield > 0 && IsValidEdict(shield))
    {
        new team = GetClientTeam(client);
        switch (team)
        {
            case 2:
            {
                if (GameType == dod)
                {
                    PrepareModel(mdlBlueShield, gShieldBlueModelIndex);
                    SetEntityModel(shield,mdlBlueShield);
                }
                else
                {
                    PrepareModel(mdlRedShield, gShieldRedModelIndex);
                    SetEntityModel(shield,mdlRedShield);
                }
            }
            case 3:
            {
                if (GameType == dod)
                {
                    PrepareModel(mdlRedShield, gShieldRedModelIndex);
                    SetEntityModel(shield,mdlRedShield);
                }
                else
                {
                    PrepareModel(mdlBlueShield, gShieldBlueModelIndex);
                    SetEntityModel(shield,mdlBlueShield);
                }
            }
        }

        new String:tmp[16];
        Format(tmp, sizeof(tmp), "shield%d", shield);
        DispatchKeyValue(shield, "targetname", tmp);

        SetEntProp(shield, Prop_Send, "m_iTeamNum", team, 4);

        DispatchSpawn(shield);
        DispatchKeyValue(shield, "solid", "6");
        SetEntProp(shield, Prop_Data, "m_CollisionGroup", (flags & Shield_Immobilize) ? 0 : 2);

        TeleportEntity(shield, pos, NULL_VECTOR, NULL_VECTOR);

        new field;
        if (flags & Shield_Team_Specific)
        {
            if (GameType == tf2)
                field = CreateEntityByName("func_respawnroomvisualizer");
            else if (GameType == dod)
                field = CreateEntityByName("func_team_wall");
            else
                field = 0;

            if (field > 0 && IsValidEdict(field))
            {
                switch (team)
                {
                    case 2: SetEntityModel(field,(GameType == dod) ? mdlBlueShield : mdlRedShield);
                    case 3: SetEntityModel(field,(GameType == dod) ? mdlRedShield : mdlBlueShield);
                }

                Format(tmp, sizeof(tmp), "field%d", shield);
                // DispatchKeyValue(field, "targetname", tmp);
                DispatchKeyValue(field, "Name", tmp);

                DispatchSpawn(field);

                if (GameType == tf2)
                {
                    SetEntityRenderMode(field, RENDER_NONE); // Not a brush, so don't render!
                    SetEntProp(field, Prop_Send, "m_iTeamNum", team, 4);
                    DispatchKeyValue(field, "solid", "6");
                    SetEntProp(field, Prop_Data, "m_CollisionGroup", 0);
                }
                else
                {
                    SetEntProp(field, Prop_Data, "m_CollisionGroup", 2);
                    DispatchKeyValue(field, "block_team", (team == 2) ? "3" : "2");
                    //SetEntProp(field, Prop_Send, "m_iTeamNum", (team == 2) ? 3 : 2, 4);
                }

                TeleportEntity(field, pos, NULL_VECTOR, NULL_VECTOR);
                gField[client] = EntIndexToEntRef(field);
            }
            else
                gField[client] = field = 0;
        }
        else
            gField[client] = field = 0;

        new shieldRef = gShield[client] = EntIndexToEntRef(shield);

        if (target > 0 && (flags & Shield_Mobile))
        {
            new String:strTargetName[64];
            Format(strTargetName, sizeof(strTargetName), "target%i", target);
            DispatchKeyValue(target, "targetname", strTargetName);

            SetEntityMoveType(shield, MOVETYPE_NOCLIP);

            // Entities parented to players are no longer visible :(
            //SetVariantString(strTargetName);
            //AcceptEntityInput(shield, "SetParent", -1, -1, 0);
            gParent[target] = shieldRef;

            if (field > 0)
            {
                SetEntityMoveType(field, MOVETYPE_NOCLIP);
                SetVariantString(strTargetName);
                AcceptEntityInput(field, "SetParent", -1, -1, 0);
            }

            /*
            if (GetClientTeam(target) == team)
                SetEntPropEnt(shield, Prop_Send, "m_hOwnerEntity", target);
            else
                SetEntPropEnt(shield, Prop_Send, "m_hOwnerEntity", client);
             */
        }
        else
        {
            //SetEntPropEnt(shield, Prop_Send, "m_hOwnerEntity", client);
            SetEntityMoveType(shield, MOVETYPE_NONE); // MOVETYPE_VPHYSICS);
            gParent[client] = 0;
            target = client;
        }

        new Float:startTime = 0.0;
        new bool:useAlt = ((flags & Shield_UseAlternateSounds) != Shield_None);
        if (!(flags & Shield_DisableStartSound))
        {
            if (useAlt && gAlternateStartSound[0])
            {
                PrepareSound(gAlternateStartSound);
                EmitSoundToAll(gAlternateStartSound, shield);
                startTime = gAlternateStartTime;
            }
            else if (gStartSound[0])
            {
                PrepareSound(gStartSound);
                EmitSoundToAll(gStartSound, shield);
                startTime = gStartTime;
            }
        }

        new bool:longEnough = (Duration > startTime || Duration <= 0.0);
        if (longEnough && !(flags & Shield_DisableActiveSound))
        {
            if (useAlt && gAlternateActiveSound[0])
            {
                new Handle:pack;
                CreateDataTimer(startTime, PlaySound, pack, TIMER_FLAG_NO_MAPCHANGE);
                WritePackCell(pack, shieldRef);
                WritePackCell(pack, false);
                WritePackString(pack, gAlternateActiveSound);
            }
            else if (gActiveSound[0])
            {
                new Handle:pack;
                CreateDataTimer(startTime, PlaySound, pack, TIMER_FLAG_NO_MAPCHANGE);
                WritePackCell(pack, shieldRef);
                WritePackCell(pack, false);
                WritePackString(pack, gActiveSound);
            }
        }

        if (Duration > 0.0)
        {
            if (longEnough && !(flags & Shield_DisableStopSound))
            {
                new Float:activeTime = Duration - ((useAlt) ? gAlternateStopTime : gStopTime);
                if (activeTime > 0.0)
                {
                    if (useAlt && gAlternateStopSound[0])
                    {
                        new Handle:pack;
                        CreateDataTimer(activeTime, PlaySound, pack, TIMER_FLAG_NO_MAPCHANGE);
                        WritePackCell(pack, shieldRef);
                        WritePackCell(pack, true); // Stop active sound
                        WritePackString(pack, gAlternateStopSound);
                    }
                    else if (gStopSound[0])
                    {
                        new Handle:pack;
                        CreateDataTimer(activeTime, PlaySound, pack, TIMER_FLAG_NO_MAPCHANGE);
                        WritePackCell(pack, shieldRef);
                        WritePackCell(pack, true); // Stop active sound
                        WritePackString(pack, gStopSound);
                    }
                }
            }

            new Handle:pack;
            gObjectTimers[client] = CreateDataTimer(Duration, ShieldExpired, pack, TIMER_FLAG_NO_MAPCHANGE);
            WritePackCell(pack, client);
        }

        if (gRemaining[client] > 0)
            gRemaining[client]--;

        new Float:time = GetConVarFloat(cvRechargeTime);
        if (time > 0.0)
        {
            m_ShieldCharged[client]=false;
            CreateTimer(time,RechargeShield,client,TIMER_FLAG_NO_MAPCHANGE);
        }
    }
    else
    {
        gField[client] = gShield[client] = shield = 0;
    }

    return shield;
}

public Action:PlaySound(Handle:timer, Handle:pack)
{
    if (pack != INVALID_HANDLE)
    {
        ResetPack(pack);
        new shield = EntRefToEntIndex(ReadPackCell(pack));
        if (shield > 0 && IsValidEntity(shield))
        {
            // Check if we need to stop active sound
            if (ReadPackCell(pack))
            {
                if (gActiveSound[0])
                    StopSound(shield, SNDCHAN_AUTO, gActiveSound);

                if (gAlternateActiveSound[0])
                    StopSound(shield, SNDCHAN_AUTO, gAlternateActiveSound);
            }

            decl String:sound[PLATFORM_MAX_PATH+1];
            ReadPackString(pack, sound, sizeof(sound));
            if (sound[0])
            {
                PrepareSound(sound);
                EmitSoundToAll(sound, shield);
            }
        }
    }
}

public Action:ShieldExpired(Handle:timer, Handle:pack)
{
    if (pack != INVALID_HANDLE)
    {
        ResetPack(pack);
        new target = ReadPackCell(pack);

        gObjectTimers[target] = INVALID_HANDLE;
        RemoveShield(target);
    }
}

public Action:RechargeShield(Handle:timer,any:index)
{
    m_ShieldCharged[index]=true;
    return Plugin_Stop;
}

RemoveShield(client)
{
    new shield = EntRefToEntIndex(gShield[client]);
    if (shield > 0 && IsValidEntity(shield))
    {
        if (gStartSound[0])
            StopSound(shield, SNDCHAN_AUTO, gStartSound);

        if (gActiveSound[0])
            StopSound(shield, SNDCHAN_AUTO, gActiveSound);

        if (gAlternateStartSound[0])
            StopSound(shield, SNDCHAN_AUTO, gAlternateStartSound);

        if (gAlternateActiveSound[0])
            StopSound(shield, SNDCHAN_AUTO, gAlternateActiveSound);

        AcceptEntityInput(shield, "Kill");
    }

    new field = EntRefToEntIndex(gField[client]);
    if (field > 0 && IsValidEntity(shield))
        AcceptEntityInput(field, "Kill");

    gParent[client] = 0;
    gShield[client] = 0;
    gField[client] = 0;
}

public Native_ControlUberShield(Handle:plugin,numParams)
{
    gNativeControl = (numParams >= 1) ? (bool:GetNativeCell(1)) : true;
}

public Native_GiveUberShield(Handle:plugin,numParams)
{
    if (numParams >= 1)
    {
        new client = GetNativeCell(1);
        gRemaining[client] = (numParams >= 2) ? GetNativeCell(2) : -1;
        gAllowed[client] = (numParams >= 3) ? GetNativeCell(3) : -1;
        gFlags[client] = (numParams >= 4) ? (ShieldFlags:GetNativeCell(4)) : ShieldDefault;

        if (gFlags[client] == ShieldDefault)
            gFlags[client] = ShieldFlags:GetConVarInt(cvMediShieldFlags);
    }
}

public Native_TakeUberShield(Handle:plugin,numParams)
{
    if (numParams >= 1)
    {
        new client = GetNativeCell(1);
        gRemaining[client] = gAllowed[client] = 0;
    }
}

public Native_UberShieldTarget(Handle:plugin,numParams)
{
    if (numParams >= 1)
    {
        new client = GetNativeCell(1);

        new Float:duration = (numParams >= 2) ? (Float:GetNativeCell(2)) : -1.0;
        if (duration < 0.0)
            duration = GetConVarFloat(cvMaxDuration);

        new ShieldFlags:flags = (numParams >= 3) ? (ShieldFlags:GetNativeCell(3)) : ShieldDefault;
        if (flags == ShieldDefault)
            flags = gFlags[client];

        return ShieldTarget(client, flags, duration);
    }
    else
        return false;
}

public Native_UberShieldLocation(Handle:plugin,numParams)
{
    if (numParams >= 1)
    {
        new client = GetNativeCell(1);

        new Float:duration = (numParams >= 2) ? (Float:GetNativeCell(2)) : -1.0;
        if (duration < 0.0)
            duration = GetConVarFloat(cvMaxDuration);

        new ShieldFlags:flags = (numParams >= 3) ? (ShieldFlags:GetNativeCell(3)) : ShieldDefault;
        if (flags == ShieldDefault)
            flags = gFlags[client];

        return ShieldLocation(client, flags, duration);
    }
    else
        return false;
}

public Native_UberShield(Handle:plugin,numParams)
{
    if (numParams >= 1)
    {
        new client = GetNativeCell(1);
        new target = (numParams >= 2) ? GetNativeCell(2) : 0;

        decl Float:pos[3];
        if (numParams >= 3)
            GetNativeArray(3, pos, sizeof(pos));
        else
            pos[0] = pos[1] = pos[2] = 0.0;

        new Float:duration = (numParams >= 4) ? (Float:GetNativeCell(4)) : -1.0;
        if (duration < 0.0)
            duration = GetConVarFloat(cvMaxDuration);

        new ShieldFlags:flags = (numParams >= 5) ? (ShieldFlags:GetNativeCell(5)) : ShieldDefault;
        if (flags == ShieldDefault)
            flags = gFlags[client];

        if (pos[0] == 0.0 && pos[1] == 0.0 && pos[2] == 0.0)
        {
            if (target > 0)
                GetClientEyePosition(target, pos);
            else
                return ShieldLocation(client, flags, duration);
        }

        return CreateShield(client, target, pos, flags, duration);
    }
    else
        return 0;
}

public Native_SetAlternateShieldSound(Handle:plugin,numParams)
{
    if (numParams >= 5)
    {
        GetNativeString(1, gAlternateStartSound, sizeof(gAlternateStartSound));
        GetNativeString(2, gAlternateActiveSound, sizeof(gAlternateActiveSound));
        GetNativeString(3, gAlternateStopSound, sizeof(gAlternateStopSound));
        gAlternateStartTime = Float:GetNativeCell(4);
        gAlternateStopTime = Float:GetNativeCell(5);
    }
}

