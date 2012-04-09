// vim: set ai et ts=4 sw=4 :
//
// SourceMod Script
//
// Developed by <eVa>Dog
// July 2008
// http://www.theville.org
//

//
// DESCRIPTION:
// This plugin is a port of my Gas plugin
// originally created using EventScripts

#pragma semicolon 1
#include <sourcemod>
#include <sdktools>

#undef REQUIRE_EXTENSIONS
#include <tf2_stocks>

#define PLUGIN_VERSION "1.1.107"

#define ADMIN_LEVEL ADMFLAG_SLAY

#define MORTAR "gas/mortar.mp3"
#define MORTAR_DL "sound/gas/mortar.mp3"

#define HEARTBEAT "player/heartbeat1.wav"

#define SHOOT "sc/zgufir00.wav"
#define SHOOT_DL "sound/sc/zgufir00.wav"

#define GAS_CLOUD "sc/zdeblo00.wav"
#define GAS_CLOUD_DL "sound/sc/zdeblo00.wav"

#define DENIED "sc/buzz.wav"
#define DENIED_DL "sound/sc/buzz.wav"

#define ERROR "sc/perror.mp3"
#define ERROR_DL "sound/sc/perror.mp3"

new gasDamage[MAXPLAYERS+1];
new gasRadius[MAXPLAYERS+1];
new gasAmount[MAXPLAYERS+1];
new gasEveryone[MAXPLAYERS+1];
new gasAllocation[MAXPLAYERS+1];
new bool:gasEnabled[MAXPLAYERS+1];
new Float:g_LastAttack[MAXPLAYERS+1];
new Handle:timer_handle[MAXPLAYERS+1][128];
new Handle:hurtdata[MAXPLAYERS+1][128];
new bool:gNativeControl = false;
new bool:g_roundstart = false;

new Handle:g_Cvar_GasAmount = INVALID_HANDLE;
new Handle:g_Cvar_Red       = INVALID_HANDLE;
new Handle:g_Cvar_Green     = INVALID_HANDLE;
new Handle:g_Cvar_Blue      = INVALID_HANDLE;
new Handle:g_Cvar_Random    = INVALID_HANDLE;
new Handle:g_Cvar_Damage    = INVALID_HANDLE;
new Handle:g_Cvar_Admins    = INVALID_HANDLE;
new Handle:g_Cvar_Time      = INVALID_HANDLE;
new Handle:g_Cvar_Enable    = INVALID_HANDLE;
new Handle:g_Cvar_Delay     = INVALID_HANDLE;
new Handle:g_Cvar_Msg       = INVALID_HANDLE;
new Handle:g_Cvar_Radius    = INVALID_HANDLE;
new Handle:g_Cvar_Whoosh    = INVALID_HANDLE;

public Plugin:myinfo = 
{
    name = "Gas",
    author = "<eVa>Dog",
    description = "Gas plugin",
    version = PLUGIN_VERSION,
    url = "http://www.theville.org"
}

/**
 * Description: Stocks to return information about TF2 player condition, etc.
 */
#tryinclude <tf2_player>
#if !defined _tf2_player_included
    #define TF2_IsDisguised(%1)         (((%1) & TF_CONDFLAG_DISGUISED) != TF_CONDFLAG_NONE)
    #define TF2_IsCloaked(%1)           (((%1) & TF_CONDFLAG_CLOAKED) != TF_CONDFLAG_NONE)
    #define TF2_IsUbercharged(%1)       (((%1) & TF_CONDFLAG_UBERCHARGED) != TF_CONDFLAG_NONE)
    #define TF2_IsBonked(%1)            (((%1) & TF_CONDFLAG_BONKED) != TF_CONDFLAG_NONE)
    #define TF2_IsDeadRingered(%1)      (((%1) & TF_CONDFLAG_DEADRINGERED) != TF_CONDFLAG_NONE)

    #define TF2_IsPlayerUbercharged(%1) TF2_IsUbercharged(TF2_GetPlayerConditionFlags(%1))
    #define TF2_IsPlayerBonked(%1)      TF2_IsBonked(TF2_GetPlayerConditionFlags(%1))
#endif

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
 * Description: Stocks to damage a player or an entity using a point_hurt entity.
 */
#tryinclude <damage>
#if !defined _damage_included
    #define DMG_GENERIC                 0
    #define DMG_NERVEGAS                (1 << 16)

    stock g_damagePointRef = -1;

    stock DamagePlayer(victim,damage,attacker=0,dmg_type=DMG_GENERIC,const String:weapon[]="")
    {
        if (damage>0 && victim>0 && IsClientInGame(victim) && IsPlayerAlive(victim))
        {
            decl String:dmg_str[16];
            IntToString(damage,dmg_str,sizeof(dmg_str));

            decl String:dmg_type_str[32];
            IntToString(dmg_type,dmg_type_str,sizeof(dmg_type_str));

            new pointHurt = EntRefToEntIndex(g_damagePointRef);
            if (pointHurt < 1)
            {
                if (!IsEntLimitReached(.message="Unable to create point_hurt in DamagePlayer()"))
                {
                    pointHurt=CreateEntityByName("point_hurt");
                    if (pointHurt > 0 && IsValidEntity(pointHurt))
                    {
                        //DispatchSpawn(pointHurt);
                        g_damagePointRef = EntIndexToEntRef(pointHurt);
                    }
                    else
                    {
                        LogError("Unable to create point_hurt in DamagePlayer()");
                        return;
                    }
                }
            }

            if (pointHurt > 0 && IsValidEdict(pointHurt))
            {
                decl String:targetname[16];
                Format(targetname,sizeof(targetname), "target%d", victim);

                DispatchKeyValue(victim,"targetname",targetname);
                DispatchKeyValue(pointHurt,"DamageTarget",targetname);
                DispatchKeyValue(pointHurt,"Damage",dmg_str);
                DispatchKeyValue(pointHurt,"DamageType",dmg_type_str);

                if (weapon[0] != '\0')
                    DispatchKeyValue(pointHurt,"classname",weapon);

                DispatchSpawn(pointHurt);

                AcceptEntityInput(pointHurt,"Hurt",(attacker>0)?attacker:-1);
                DispatchKeyValue(pointHurt,"classname","point_hurt");

                IntToString(victim,targetname,sizeof(targetname));
                DispatchKeyValue(victim,"targetname",targetname);
            }
            else
                LogError("Unable to spawn point_hurt in DamagePlayer()");
        }
    }

    stock CleanupDamageEntity()
    {
        new pointHurt = EntRefToEntIndex(g_damagePointRef);
        if (pointHurt > 0)
            RemoveEdict(pointHurt);

        g_damagePointRef = -1;
    }
#endif

public APLRes:AskPluginLoad2(Handle:myself, bool:late, String:error[], err_max)
{
    // Register Natives
    CreateNative("ControlGas",Native_ControlGas);
    CreateNative("GiveGas",Native_GiveGas);
    CreateNative("TakeGas",Native_TakeGas);
    CreateNative("EnableGas",Native_EnableGas);
    CreateNative("IsGasEnabled",Native_IsGasEnabled);
    CreateNative("HasGas",Native_HasGas);
    CreateNative("GasAttack",Native_GasAttack);

    RegPluginLibrary("sm_gas");
    return APLRes_Success;
}

public OnPluginStart()
{
    RegConsoleCmd("sm_gas", Gas, " -  Calls in gas at coords specified by player's crosshairs");

    CreateConVar("sm_gas_version", PLUGIN_VERSION, "Version of SourceMod Gas on this server", FCVAR_PLUGIN|FCVAR_SPONLY|FCVAR_REPLICATED|FCVAR_NOTIFY);
    g_Cvar_GasAmount   = CreateConVar("sm_gas_amount", "1", " Number of gas attacks per player at spawn", FCVAR_PLUGIN);
    g_Cvar_Red         = CreateConVar("sm_gas_red", "180", " Amount of red color in gas", FCVAR_PLUGIN);
    g_Cvar_Green       = CreateConVar("sm_gas_green", "210", " Amount of green color in gas", FCVAR_PLUGIN);
    g_Cvar_Blue        = CreateConVar("sm_gas_blue", "0", " Amount of blue color in gas", FCVAR_PLUGIN);
    g_Cvar_Random      = CreateConVar("sm_gas_random", "0", " Make gas color random <1 to enable>", FCVAR_PLUGIN);
    g_Cvar_Damage      = CreateConVar("sm_gas_damage", "50", " Amount of damage that the gas does", FCVAR_PLUGIN);
    g_Cvar_Admins      = CreateConVar("sm_gas_admins", "0", " Allow Admins only to use Gas", FCVAR_PLUGIN);
    g_Cvar_Time        = CreateConVar("sm_gas_time", "18.0", " Length of time gas should be active", FCVAR_PLUGIN);
    g_Cvar_Enable      = CreateConVar("sm_gas_enabled", "1", " Enable/Disable the Gas plugin", FCVAR_PLUGIN);
    g_Cvar_Delay       = CreateConVar("sm_gas_delay", "20", " Delay between spawning and making gas available", FCVAR_PLUGIN);
    g_Cvar_Msg         = CreateConVar("sm_gas_showmessages", "0", " Show gas messages", FCVAR_PLUGIN);
    g_Cvar_Radius      = CreateConVar("sm_gas_radius", "200", " Radius of the gas cloud", FCVAR_PLUGIN);
    g_Cvar_Whoosh	   = CreateConVar("sm_gas_launchmethod", "0", " 0=Launched by air  1=Instant", FCVAR_PLUGIN);

    HookEvent("player_spawn", PlayerSpawnEvent);
    HookEvent("player_death", PlayerDeathEvent);
    HookEvent("player_disconnect", PlayerDisconnectEvent);

    if (GetGameType() == dod)
        HookEvent("dod_round_start", RoundStartEvent);
    else if (GameType == tf2)
        HookEvent("teamplay_round_start", RoundStartEvent);
    else
        HookEvent("round_start", RoundStartEvent);
}

public OnMapStart()
{
	if (gNativeControl || GetConVarInt(g_Cvar_Enable))
    {
        AddFileToDownloadsTable(MORTAR_DL);
        AddFileToDownloadsTable(GAS_CLOUD_DL);
        AddFileToDownloadsTable(DENIED_DL);
        AddFileToDownloadsTable(ERROR_DL);
        AddFileToDownloadsTable(SHOOT_DL);

        PrecacheSound(MORTAR, true);
        PrecacheSound(HEARTBEAT, true);
        PrecacheSound(GAS_CLOUD, true);
        PrecacheSound(DENIED, true);
        PrecacheSound(ERROR, true);
        PrecacheSound(SHOOT, true);
    }
}

public OnMapEnd()
{
    CleanupDamageEntity();
}

public PlayerSpawnEvent(Handle:event, const String:name[], bool:dontBroadcast)
{
    new client = GetClientOfUserId(GetEventInt(event, "userid"));

    if (gNativeControl)
        gasAmount[client] = gasAllocation[client];
    else if (GetConVarInt(g_Cvar_Enable))
    {
        if (GetConVarInt(g_Cvar_Admins) == 1)
        {
            if (GetUserFlagBits(client) & ADMIN_LEVEL)
                gasAmount[client] = GetConVarInt(g_Cvar_GasAmount);
            else if (GetUserFlagBits(client) & ADMFLAG_ROOT)
                gasAmount[client] = GetConVarInt(g_Cvar_GasAmount);
            else
                gasAmount[client] = 0;
        }
        else
            gasAmount[client] = GetConVarInt(g_Cvar_GasAmount);
    }
    else
        gasAmount[client] = 0;

    gasEnabled[client] = false;

    if (gasAmount[client] != 0)
        CreateTimer(GetConVarFloat(g_Cvar_Delay), SetGas, client);
}

public PlayerDeathEvent(Handle:event, const String:name[], bool:dontBroadcast)
{
	if (gNativeControl || GetConVarInt(g_Cvar_Enable))
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

        new client = GetClientOfUserId(GetEventInt(event, "userid"));
        gasAmount[client] = 0;
    }
}

public PlayerDisconnectEvent(Handle:event, const String:name[], bool:dontBroadcast)
{
	if (gNativeControl || GetConVarInt(g_Cvar_Enable))
	{
		new client = GetClientOfUserId(GetEventInt(event, "userid"));

		for (new i = GetConVarInt(g_Cvar_GasAmount); i > 0 ; i--)
		{
			if (timer_handle[client][i] != INVALID_HANDLE)
			{
				KillTimer(timer_handle[client][i]);
				timer_handle[client][i] = INVALID_HANDLE;
				CloseHandle(hurtdata[client][i]);
			}
		}
	}
}

public RoundStartEvent(Handle:event, const String:name[], bool:dontBroadcast)
{
	if (gNativeControl || GetConVarInt(g_Cvar_Enable))
	{
		g_roundstart = true;
		CreateTimer(1.0, Reset, 0);
		
		for (new klient = 1; klient <= 64; klient++)
		{
			for (new i = GetConVarInt(g_Cvar_GasAmount); i > 0 ; i--)
			{
				if (timer_handle[klient][i] != INVALID_HANDLE)
				{
					KillTimer(timer_handle[klient][i]);
					timer_handle[klient][i] = INVALID_HANDLE;
					CloseHandle(hurtdata[klient][i]);
				}
			}
		}
	}
}

public Action:SetGas(Handle:timer, any:client)
{
	gasEnabled[client] = true;
}

public Action:Reset(Handle:timer, any:client)
{
	g_roundstart = false;
}

public Action:Gas(client, args)
{
    if (gNativeControl || GetConVarInt(g_Cvar_Enable))
    {
        if (client > 0 && IsPlayerAlive(client))
        {
            if (gasEnabled[client] && gasAmount[client] > 0)
            {
                if (GetGameType() == tf2)
                {
                    switch (TF2_GetPlayerClass(client))
                    {
                        case TFClass_Spy:
                        {
                            new pcond = TF2_GetPlayerConditionFlags(client);
                            if (TF2_IsCloaked(pcond) || TF2_IsDeadRingered(pcond))
                            {
                                EmitSoundToClient(client,DENIED);
                                return Plugin_Handled;
                            }
                            else if (TF2_IsDisguised(pcond))
                                TF2_RemovePlayerDisguise(client);
                        }
                        case TFClass_Scout:
                        {
                            if (TF2_IsPlayerBonked(client))
                                return Plugin_Handled;
                        }
                    }
                }

                if (IsEntLimitReached(.client=client,.message="unable to launch gas"))
                    return Plugin_Handled;

                if (gasAmount[client] >= 127)
                    gasAmount[client] = 127;

                new Float:vAngles[3];
                new Float:vOrigin[3];
                new Float:pos[3];

                GetClientEyePosition(client,vOrigin);
                GetClientEyeAngles(client, vAngles);

                new Handle:trace = TR_TraceRayFilterEx(vOrigin, vAngles, MASK_SHOT, RayType_Infinite, TraceEntityFilterPlayer);

                if (TR_DidHit(trace))
                {
                    TR_GetEndPosition(pos, trace);
                    pos[2] += 10.0;
                }
                CloseHandle(trace);

                EmitSoundToAll(SHOOT, client);

                new bool:message = GetConVarBool(g_Cvar_Msg);
                if (message)
                {
                    PrintToChat(client, "[SM] Gas has been called in.  Take cover!");
                    FakeClientCommand(client, "say_team I have called in a gas attack...take cover!");
                }

                TE_SetupSparks(pos, NULL_VECTOR, 2, 1);
                TE_SendToAll(0.1);
                TE_SetupSparks(pos, NULL_VECTOR, 2, 2);
                TE_SendToAll(0.4);
                TE_SetupSparks(pos, NULL_VECTOR, 1, 1);
                TE_SendToAll(1.0);

                new Float:whooshtime;
                if (GetConVarInt(g_Cvar_Whoosh) == 0)
                {
                    CreateTimer(2.5, BigWhoosh, client);
                    whooshtime = 6.0;
                }
                else
                {
                    whooshtime = 0.1;
                }

                new Handle:gasdata = CreateDataPack();
                CreateTimer(whooshtime, CreateGas, gasdata);
                WritePackCell(gasdata, client);
                WritePackFloat(gasdata, pos[0]);
                WritePackFloat(gasdata, pos[1]);
                WritePackFloat(gasdata, pos[2]);
                WritePackCell(gasdata, gasAmount[client]);

                g_LastAttack[client] = GetEngineTime() + whooshtime;

                gasAmount[client]--;
                if (message)
                    PrintToChat(client, "Gas left: %i", gasAmount[client]);
            }
            else
            {
                EmitSoundToClient(client,gasEnabled[client] ? ERROR : DENIED);
                if (GetConVarBool(g_Cvar_Msg))
                    PrintToChat(client, "[SM] Gas unavailable");
            }
        }
    }
    return Plugin_Handled;
}

public bool:TraceEntityFilterPlayer(entity, contentsMask)
{
    return entity > MaxClients || !entity;
} 

public Action:BigWhoosh(Handle:timer, any:client)
{
    EmitSoundToAll(MORTAR, _, _, _, _, 0.8);
}

public Action:CreateGas(Handle:timer, Handle:gasdata)
{
    ResetPack(gasdata);
    new client = ReadPackCell(gasdata);

    if (IsEntLimitReached(.client=client,.message="unable to spawn gas after launch"))
        return Plugin_Stop;

    new Float:location[3];
    location[0] = ReadPackFloat(gasdata);
    location[1] = ReadPackFloat(gasdata);
    location[2] = ReadPackFloat(gasdata);
    new gasNumber = ReadPackCell(gasdata);
    CloseHandle(gasdata);

    new pointHurt = 0;

    new ff_on = gasEveryone[client];
    if (ff_on == -1)
        ff_on = GetConVarInt(FindConVar("mp_friendlyfire"));

    new String:originData[64];
    Format(originData, sizeof(originData), "%f %f %f", location[0], location[1], location[2]);

    new String:radius[64];
    new rad = gasRadius[client];
    Format(radius, sizeof(radius), "%i", (rad > 0) ? rad : GetConVarInt(g_Cvar_Radius));

    // Create the Explosion
    new explosion = CreateEntityByName("env_explosion");
    if (explosion > 0 && IsValidEdict(explosion))
    {
        DispatchKeyValue(explosion,"Origin", originData);
        DispatchKeyValue(explosion,"Magnitude", radius);
        DispatchSpawn(explosion);
        AcceptEntityInput(explosion, "Explode");
        AcceptEntityInput(explosion, "Kill");

        new String:damage[64];
        new dmg = gasDamage[client];
        Format(damage, sizeof(damage), "%i", (dmg > 0) ? dmg : GetConVarInt(g_Cvar_Damage));

        if (ff_on)
        {
            // Create the PointHurt
            pointHurt = CreateEntityByName("point_hurt");
            if (pointHurt > 0 && IsValidEdict(pointHurt))
            {
                DispatchKeyValue(pointHurt,"Origin", originData);
                DispatchKeyValue(pointHurt,"Damage", damage);
                DispatchKeyValue(pointHurt,"DamageRadius", radius);
                DispatchKeyValue(pointHurt,"DamageDelay", "1.0");
                DispatchKeyValue(pointHurt,"DamageType", "65536");
                DispatchSpawn(pointHurt);
                AcceptEntityInput(pointHurt, "TurnOn");

                hurtdata[client][gasNumber] = INVALID_HANDLE;
                timer_handle[client][gasNumber] = INVALID_HANDLE;
            }
        }
        else
        {
            hurtdata[client][gasNumber] = CreateDataPack();
            WritePackCell(hurtdata[client][gasNumber], client);
            WritePackCell(hurtdata[client][gasNumber], gasNumber);
            WritePackFloat(hurtdata[client][gasNumber], location[0]);
            WritePackFloat(hurtdata[client][gasNumber], location[1]);
            WritePackFloat(hurtdata[client][gasNumber], location[2]);
            timer_handle[client][gasNumber] = CreateTimer(1.0, Point_Hurt, hurtdata[client][gasNumber], TIMER_REPEAT);
        }

        new String:colorData[64];
        if (GetConVarInt(g_Cvar_Random) == 0)
        {
            Format(colorData, sizeof(colorData), "%i %i %i", GetConVarInt(g_Cvar_Red),
                   GetConVarInt(g_Cvar_Green), GetConVarInt(g_Cvar_Blue));
        }
        else
        {
            new red = GetRandomInt(1, 255);
            new green = GetRandomInt(1, 255);
            new blue = GetRandomInt(1, 255);
            Format(colorData, sizeof(colorData), "%i %i %i", red, green, blue);
        }

        // Create the Gas Cloud
        new String:gas_name[128];
        Format(gas_name, sizeof(gas_name), "Gas%i", client);
        new gascloud = CreateEntityByName("env_smokestack");
        if (gascloud > 0 && IsValidEdict(gascloud))
        {
            DispatchKeyValue(gascloud,"targetname", gas_name);
            DispatchKeyValue(gascloud,"Origin", originData);
            DispatchKeyValue(gascloud,"BaseSpread", "100");
            DispatchKeyValue(gascloud,"SpreadSpeed", "10");
            DispatchKeyValue(gascloud,"Speed", "80");
            DispatchKeyValue(gascloud,"StartSize", "200");
            DispatchKeyValue(gascloud,"EndSize", "2");
            DispatchKeyValue(gascloud,"Rate", "15");
            DispatchKeyValue(gascloud,"JetLength", "400");
            DispatchKeyValue(gascloud,"Twist", "4");
            DispatchKeyValue(gascloud,"RenderColor", colorData);
            DispatchKeyValue(gascloud,"RenderAmt", "100");
            DispatchKeyValue(gascloud,"SmokeMaterial", "particle/particle_smokegrenade1.vmt");
            DispatchSpawn(gascloud);
            AcceptEntityInput(gascloud, "TurnOn");

            EmitSoundToAll(GAS_CLOUD, _, _, _, _, 0.8);

            new Float:length= GetConVarFloat(g_Cvar_Time);
            if (length <= 8.0)
                length = 8.0;

            g_LastAttack[client] = GetEngineTime() + length;

            new Handle:entitypack = CreateDataPack();
            CreateTimer(length, RemoveGas, entitypack);
            CreateTimer(length + 5.0, KillGas, entitypack);
            WritePackCell(entitypack, EntIndexToEntRef(gascloud));
            WritePackCell(entitypack, EntIndexToEntRef(pointHurt));
            WritePackCell(entitypack, gasNumber);
            WritePackCell(entitypack, client);
        }
        else
            LogError("Unable to create gas cloud!");
    }
    else
        LogError("Unable to create exploson!");

    return Plugin_Stop;
}

public Action:RemoveGas(Handle:timer, Handle:entitypack)
{
    ResetPack(entitypack);
    new gascloud = EntRefToEntIndex(ReadPackCell(entitypack));
    new pointHurt = EntRefToEntIndex(ReadPackCell(entitypack));
    new gasNumber = ReadPackCell(entitypack);
    new client = ReadPackCell(entitypack);

    if (gascloud > 0 && IsValidEntity(gascloud))
        AcceptEntityInput(gascloud, "TurnOff");

    if (pointHurt > 0 && IsValidEntity(pointHurt))
        AcceptEntityInput(pointHurt, "TurnOff");

    if (timer_handle[client][gasNumber] != INVALID_HANDLE)
    {
        KillTimer(timer_handle[client][gasNumber]);
        timer_handle[client][gasNumber] = INVALID_HANDLE;
        CloseHandle(hurtdata[client][gasNumber]);
    }
}

public Action:KillGas(Handle:timer, Handle:entitypack)
{
    ResetPack(entitypack);
    new gascloud = EntRefToEntIndex(ReadPackCell(entitypack));
    new pointHurt = EntRefToEntIndex(ReadPackCell(entitypack));

    if (gascloud > 0 && IsValidEntity(gascloud))
        AcceptEntityInput(gascloud, "Kill");

    if (pointHurt > 0 && IsValidEntity(pointHurt))
        AcceptEntityInput(pointHurt, "Kill");

    CloseHandle(entitypack);
}

public Action:Point_Hurt(Handle:timer, Handle:hurt)
{
	ResetPack(hurt);
	new client = ReadPackCell(hurt);
	new gasNumber = ReadPackCell(hurt);
	new Float:location[3];
	location[0] = ReadPackFloat(hurt);
	location[1] = ReadPackFloat(hurt);
	location[2] = ReadPackFloat(hurt);
	
	if (!g_roundstart)
	{
		for (new target = 1; target <= GetMaxClients(); target++)
		{
			if (IsClientInGame(target))
			{
				if (IsPlayerAlive(target))
				{
					if (GetClientTeam(client) != GetClientTeam(target))
					{
						new Float:targetVector[3];
						GetClientAbsOrigin(target, targetVector);
								
						new Float:distance = GetVectorDistance(targetVector, location);
								
						if (distance < 300)
						{
                            new damage = GetConVarInt(g_Cvar_Damage);
                            #if 1
                            DamagePlayer(target,damage,client,DMG_NERVEGAS,"gas");
                            #else
							new target_health;
							target_health = GetClientHealth(target);
							
							target_health -= damage;
							
							if (target_health <= (GetConVarInt(g_Cvar_Damage) + 1))
							{
								ForcePlayerSuicide(target);
								LogAction(client, target, "\"%L\" gassed \"%L\"", client, target);
							}
							else
								SetEntityHealth(target, target_health);
                            #endif
						}
					}
				}
			}
		}
	}
	else
	{
		KillTimer(timer);
		timer_handle[client][gasNumber] = INVALID_HANDLE;
		CloseHandle(hurt);
	}
}

public Native_ControlGas(Handle:plugin,numParams)
{
    gNativeControl = GetNativeCell(1);
}

public Native_GiveGas(Handle:plugin,numParams)
{
    new client = GetNativeCell(1);
    new amount = GetNativeCell(2);
    gasDamage[client] = GetNativeCell(3);
    gasRadius[client] = GetNativeCell(4);
    gasEveryone[client] = GetNativeCell(5);
    gasAmount[client] = gasAllocation[client] = amount;
    gasEnabled[client] = (amount > 0);
}

public Native_TakeGas(Handle:plugin,numParams)
{
    new client = GetNativeCell(1);
    gasAmount[client] = gasAllocation[client] = gasDamage[client] = gasRadius[client] = 0;
}

public Native_EnableGas(Handle:plugin,numParams)
{
    new client = GetNativeCell(1);
    gasEnabled[client] = bool:GetNativeCell(2);
}

public Native_HasGas(Handle:plugin,numParams)
{
    new client = GetNativeCell(1);
    return GetNativeCell(2) ? gasAllocation[client] : gasAmount[client];
}

public Native_IsGasEnabled(Handle:plugin,numParams)
{
    return gasEnabled[GetNativeCell(1)];
}

public Native_GasAttack(Handle:plugin,numParams)
{
    return (Gas(GetNativeCell(1),0) == Plugin_Handled);
}
