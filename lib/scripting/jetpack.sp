/**
 * vim: set ai et! ts=4 sw=4 :
 * File: jetpack.sp
 * Description: Jetpack for source.
 * Author(s): Knagg0
 * Modified by: -=|JFH|=-Naris (Murray Wilson)
 *              -- Added Fuel & Refueling Time
 *              -- Added AdminOnly
 *              -- Added Give/Take Jetpack 
 *              -- Added Admin Interface
 *              -- Added Native Interface
 *              -- Added sm_jetpack_team
 *              -- Added sm_jetpack_max_refuels
 *              -- Added sm_jetpack_noflag
 *
 * Fixed by: iggythepop/-SinCO-
 *           -- Fixed jetpack sticking to the ground
 *
 * Added by: Grrrrrrrrrrrrrrrrrrr
 *           -- Added Flame Effect
 */

#pragma semicolon 1

#include <sourcemod>
#include <sdktools>

#undef REQUIRE_PLUGIN
#include <adminmenu>
#define REQUIRE_PLUGIN

#undef REQUIRE_EXTENSIONS
#include <tf2_stocks>
#define REQUIRE_EXTENSIONS

#define PLUGIN_VERSION "2.2"

#define MOVECOLLIDE_DEFAULT	0
#define MOVECOLLIDE_FLY_BOUNCE	1

#define LIFE_ALIVE	0

#define COLOR_DEFAULT 0x01
#define COLOR_GREEN 0x04

//#define ADMFLAG_JETPACK ADMFLAG_GENERIC
#define ADMFLAG_JETPACK ADMFLAG_CUSTOM2

#define START_SOUND     ""
#define START_SOUND_TF2 "weapons/flame_thrower_airblast.wav"

#define STOP_SOUND      ""
#define STOP_SOUND_TF2  "weapons/flame_thrower_end.wav"

#define LOOP_SOUND      "vehicles/airboat/fan_blade_fullthrottle_loop1.wav"
#define LOOP_SOUND_TF2  "weapons/flame_thrower_loop.wav"

//Use SourceCraft sounds if it is present
#tryinclude <SourceCraft/sc/SourceCraft>
#if defined SOURCECRAFT
	#define EMPTY_SOUND  	 "sc/outofgas.wav"
	#define EMPTY_SOUND_TF2  "sc/outofgas.wav"

	#define REFUEL_SOUND     "sc/transmission.wav"
	#define REFUEL_SOUND_TF2 "sc/transmission.wav"
#else
	#define EMPTY_SOUND      "common/bugreporter_failed.wav"
	#define EMPTY_SOUND_TF2  "weapons/syringegun_reload_air2.wav"

	#define REFUEL_SOUND     "hl1/fvox/activated.wav"
	#define REFUEL_SOUND_TF2 "hl1/fvox/activated.wav"
#endif

// TF2 Classes

#define SCOUT 1
#define SNIPER 2
#define SOLDIER 3
#define DEMO 4
#define MEDIC 5
#define HEAVY 6
#define PYRO 7
#define SPY 8
#define ENGIE 9
#define CLS_MAX 10

// DOD Classes

#define RIFLEMAN 0
#define ASSAULT 1
#define SUPPORT 2
#define DODSNIPER 3
#define MG 4
#define ROCKETMAN 5

// ConVars
new Handle:sm_jetpack		        = INVALID_HANDLE;
new Handle:sm_jetpack_start_sound	= INVALID_HANDLE;
new Handle:sm_jetpack_stop_sound	= INVALID_HANDLE;
new Handle:sm_jetpack_loop_sound	= INVALID_HANDLE;
new Handle:sm_jetpack_empty_sound	= INVALID_HANDLE;
new Handle:sm_jetpack_refuel_sound	= INVALID_HANDLE;
new Handle:sm_jetpack_speed	        = INVALID_HANDLE;
new Handle:sm_jetpack_volume        = INVALID_HANDLE;
new Handle:sm_jetpack_fuel	        = INVALID_HANDLE;
new Handle:sm_jetpack_team          = INVALID_HANDLE;
new Handle:sm_jetpack_onspawn	    = INVALID_HANDLE;
new Handle:sm_jetpack_announce	    = INVALID_HANDLE;
new Handle:sm_jetpack_adminonly	    = INVALID_HANDLE;
new Handle:sm_jetpack_refueling_time= INVALID_HANDLE;
new Handle:sm_jetpack_max_refuels   = INVALID_HANDLE;
new Handle:sm_jetpack_noflag        = INVALID_HANDLE;
new Handle:sm_jetpack_gravity       = INVALID_HANDLE;
new Handle:sm_jetpack_rate[CLS_MAX] = { INVALID_HANDLE, ...};

new Handle:hAdminMenu = INVALID_HANDLE;
new TopMenuObject:oGiveJetpack = INVALID_TOPMENUOBJECT;
new TopMenuObject:oTakeJetpack = INVALID_TOPMENUOBJECT;

// SendProp Offsets
new g_iMoveCollide	= -1;
new g_iVelocity		= -1;

// Soundfiles
new String:g_StartSound[PLATFORM_MAX_PATH];
new String:g_StopSound[PLATFORM_MAX_PATH];
new String:g_LoopSound[PLATFORM_MAX_PATH];
new String:g_EmptySound[PLATFORM_MAX_PATH];
new String:g_RefuelSound[PLATFORM_MAX_PATH];

// Is Jetpack Enabled
new bool:g_bHasJetpack[MAXPLAYERS + 1];
new bool:g_bFromNative[MAXPLAYERS + 1];
new bool:g_bJetpackOn[MAXPLAYERS + 1];

// Fuel for the Jetpacks
new g_iFuel[MAXPLAYERS + 1];
new g_iRate[MAXPLAYERS + 1];
new g_iMaxRefuels[MAXPLAYERS + 1];
new g_iRefuelCount[MAXPLAYERS + 1];
new g_iRefuelAmount[MAXPLAYERS + 1];
new Float:g_fRefuelingTime[MAXPLAYERS + 1];
new g_JetpackParticle[MAXPLAYERS + 1][2];

// Timer For GameFrame
new Float:g_fTimer	= 0.0;

// Native interface settings
new g_iNativeRate[MAXPLAYERS + 1];
new bool:g_bNativeOverride = false;
new g_iNativeJetpacks      = 0;

// Forward handles
new Handle:fwdOnJetpack;

public Plugin:myinfo =
{
    name = "Jetpack",
    author = "Knagg0",
    description = "Adds a jetpack to fly around the map with",
    version = PLUGIN_VERSION,
    url = "http://www.mfzb.de"
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
 * Description: Stocks for DoD
 */
#tryinclude <dod>
#if !defined _dod_included
    enum DODClassType
    {
        DODClass_Unassigned = -1,
        DODClass_Rifleman = 0,
        DODClass_Assault,
        DODClass_Support,
        DODClass_Sniper,
        DODClass_MachineGunner,
        DODClass_Rocketman
    };

    /**
     * Get's a Clients current class.
     *
     * @param client		Player's index.
     * @return				Current DODClassType of player.
     * @error				Invalid client index.
     */
    stock DODClassType:DOD_GetPlayerClass(client)
    {
        return DODClassType:GetEntProp(client, Prop_Send, "m_iPlayerClass");
    }
#endif

/**
 * Description: stock for SendTopMessage
 */
#tryinclude <topmessage>
#if !defined _topmessage_included
	stock SendTopMessage(client, level, time, r, g, b, a, String:text[], any:...)
	{
		new String:message[100];
		VFormat(message,sizeof(message),text, 9);
		
		new Handle:kv = CreateKeyValues("message", "title", message);
		KvSetColor(kv, "color", r, g, b, a);
		KvSetNum(kv, "level", level);
		KvSetNum(kv, "time", time);

		CreateDialog(client, kv, DialogType_Msg);

		CloseHandle(kv);
	}
#endif

/**
 * Description: Functions to show TF2 particles
 */
#tryinclude "particle"
#if !defined _particle_included
	// Particle Attachment Types  -------------------------------------------------
	enum ParticleAttachmentType
	{
		NoAttach = 0,
		Attach,
		AttachMaintainOffset
	};

	// Particles ------------------------------------------------------------------

	/* CreateParticle()
	**
	** Creates a particle at an entity's position. Attach determines the attachment
	** type (0 = not attached, 1 = normal attachment, 2 = head attachment). Allows
	** offsets from the entity's position.
	** ------------------------------------------------------------------------- */
	stock CreateParticle(const String:particleType[], Float:time=5.0, entity=0,
						 ParticleAttachmentType:attach=Attach,
						 const String:attachToBone[]="head",
						 const Float:offsetPos[3]=NULL_VECTOR,
						 const Float:offsetAng[3]=NULL_VECTOR,
						 Timer:deleteFunc=Timer:0,
						 &Handle:timerHandle=INVALID_HANDLE)
	{
		new particle = CreateEntityByName("info_particle_system");
		if (particle > 0 && IsValidEdict(particle))
		{
			decl Float:pos[3], Float:ang[3];
			if (entity > 0)
			{
				GetEntPropVector(entity, Prop_Send, "m_vecOrigin", pos);
				AddVectors(pos, offsetPos, pos);

				GetEntPropVector(entity, Prop_Send, "m_angRotation", ang);
				AddVectors(ang, offsetAng, ang);
			}
			else
			{
				pos[0] = offsetPos[0];
				pos[1] = offsetPos[1];
				pos[2] = offsetPos[2];

				ang[0] = offsetAng[0];
				ang[1] = offsetAng[1];
				ang[2] = offsetAng[2];
			}

			TeleportEntity(particle, pos, ang, NULL_VECTOR);

			decl String:tName[32];
			Format(tName, sizeof(tName), "target%i", entity);
			DispatchKeyValue(entity, "targetname", tName);

			DispatchKeyValue(particle, "targetname", "sc2particle");
			DispatchKeyValue(particle, "parentname", tName);
			DispatchKeyValue(particle, "effect_name", particleType);

			if (attach > NoAttach)
			{
				SetVariantString("!activator");
				AcceptEntityInput(particle, "SetParent", entity, particle, 0);

				if (attachToBone[0] != '\0')
				{
					SetVariantString(attachToBone);
					AcceptEntityInput(particle, (attach >= AttachMaintainOffset)
												? "SetParentAttachmentMaintainOffset"
												: "SetParentAttachment",
									  particle, particle, 0);
				}
			}

			DispatchSpawn(particle);
			ActivateEntity(particle);
			AcceptEntityInput(particle, "start");

			if (time > 0.0)
			{
				timerHandle = CreateTimer(time, deleteFunc ? deleteFunc : DeleteParticles,
										  EntIndexToEntRef(particle));
			}
		}
		else
			LogError("CreateParticle: could not create info_particle_system");

		return particle;
	}

	stock DeleteParticle(particleRef)
	{
		new particle = EntRefToEntIndex(particleRef);
		if (particle > 0 && IsValidEntity(particle))
		{
			new String:classname[32];
			GetEdictClassname(particle, classname, sizeof(classname));
			if (StrEqual(classname, "info_particle_system", false))
			{
				AcceptEntityInput(particle, "stop");
				RemoveEdict(particle);
			}
		}
	}

	public Action:DeleteParticles(Handle:timer, any:particleRef)
	{
		DeleteParticle(particleRef);
		return Plugin_Stop;
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
/*****************************************************************/

public APLRes:AskPluginLoad2(Handle:myself, bool:late, String:error[], err_max)
{
    // Register Natives
    CreateNative("ControlJetpack",Native_ControlJetpack);
    CreateNative("GetJetpack",Native_GetJetpack);
    CreateNative("GetJetpackFuel",Native_GetJetpackFuel);
    CreateNative("GetJetpackRate",Native_GetJetpackRate);
    CreateNative("GetJetpackRefuelingTime",Native_GetJetpackRefuelingTime);
    CreateNative("SetJetpackFuel",Native_SetJetpackFuel);
    CreateNative("SetJetpackRate",Native_SetJetpackRate);
    CreateNative("SetJetpackRefuelingTime",Native_SetJetpackRefuelingTime);
    CreateNative("GiveJetpack",Native_GiveJetpack);
    CreateNative("TakeJetpack",Native_TakeJetpack);
    CreateNative("GiveJetpackFuel",Native_GiveJetpackFuel);
    CreateNative("TakeJetpackFuel",Native_TakeJetpackFuel);
    CreateNative("StartJetpack",Native_StartJetpack);
    CreateNative("StopJetpack",Native_StopJetpack);

    fwdOnJetpack=CreateGlobalForward("OnJetpack",ET_Hook,Param_Cell);

    RegPluginLibrary("jetpack");

    return APLRes_Success;
}

public OnPluginStart()
{
    // Create ConCommands
    RegConsoleCmd("+jetpack", JetpackPressed, "use jetpack (keydown)", FCVAR_GAMEDLL);
    RegConsoleCmd("-jetpack", JetpackReleased, "use jetpack (keyup)", FCVAR_GAMEDLL);

    // Register admin cmds
    RegAdminCmd("sm_jetpack_give",Command_GiveJetpack,ADMFLAG_JETPACK,"","give a jetpack to a player");
    RegAdminCmd("sm_jetpack_take",Command_TakeJetpack,ADMFLAG_JETPACK,"","take the jetpack from a player");

    // Hook events
    HookEvent("player_spawn",PlayerSpawnEvent);

    // Find SendProp Offsets
    if((g_iMoveCollide = FindSendPropOffs("CBaseEntity", "movecollide")) == -1)
        LogError("Could not find offset for CBaseEntity::movecollide");

    if((g_iVelocity = FindSendPropOffs("CBasePlayer", "m_vecVelocity[0]")) == -1)
        LogError("Could not find offset for CBasePlayer::m_vecVelocity[0]");

    // Create ConVars
    sm_jetpack = CreateConVar("sm_jetpack", "0", "enable jetpacks on the server", FCVAR_PLUGIN);
    sm_jetpack_speed = CreateConVar("sm_jetpack_speed", "100", "speed of the jetpack", FCVAR_PLUGIN);
    sm_jetpack_volume = CreateConVar("sm_jetpack_volume", "0.5", "volume of the jetpack sound", FCVAR_PLUGIN);
    sm_jetpack_fuel = CreateConVar("sm_jetpack_fuel", "-1", "amount of fuel to start with (-1 == unlimited)", FCVAR_PLUGIN);
    sm_jetpack_max_refuels = CreateConVar("sm_jetpack_max_refuels", "-1", "number of times the jetpack can be refueled (-1 == unlimited)", FCVAR_PLUGIN);
    sm_jetpack_refueling_time = CreateConVar("sm_jetpack_refueling_time", "30.0", "amount of time to wait before refueling", FCVAR_PLUGIN);
    sm_jetpack_onspawn = CreateConVar("sm_jetpack_onspawn", "0", "enable giving players a jetpack when they spawn", FCVAR_PLUGIN);
    sm_jetpack_team = CreateConVar("sm_jetpack_team", "0", "team restriction (0=all use, 2 or 3 to only allowed specified team to have a jetpack", FCVAR_PLUGIN);
    sm_jetpack_gravity = CreateConVar("sm_jetpack_gravity", "1", "Set to 1 to have gravity affect the jetpack (MOVETYPE_FLYGRAVITY), 0 for no gravity (MOVETYPE_FLY).", FCVAR_PLUGIN);
    sm_jetpack_announce = CreateConVar("sm_jetpack_announce","0","This will enable announcements that jetpacks are available", FCVAR_PLUGIN);
    sm_jetpack_adminonly = CreateConVar("sm_jetpack_adminonly", "0", "only allows admins to have jetpacks when set to 1", FCVAR_PLUGIN);

    // Disable noflag if the game isn't TF2.
    if (GetGameType() == tf2)
    {
        sm_jetpack_start_sound = CreateConVar("sm_jetpack_start_sound", START_SOUND_TF2, "the jetpack start sound", FCVAR_PLUGIN);
        sm_jetpack_stop_sound = CreateConVar("sm_jetpack_stop_sound", STOP_SOUND_TF2, "the jetpack stop sound", FCVAR_PLUGIN);
        sm_jetpack_loop_sound = CreateConVar("sm_jetpack_loop_sound", LOOP_SOUND_TF2, "the jetpack loop sound", FCVAR_PLUGIN);
        sm_jetpack_empty_sound = CreateConVar("sm_jetpack_empty_sound", EMPTY_SOUND_TF2, "the jetpack empty sound", FCVAR_PLUGIN);
        sm_jetpack_refuel_sound = CreateConVar("sm_jetpack_refuel_sound", REFUEL_SOUND_TF2, "the jetpack refuel sound", FCVAR_PLUGIN);
        sm_jetpack_noflag = CreateConVar("sm_jetpack_noflag", "1", "When enabled, prevents TF2 flag carrier from using the jetpack", FCVAR_PLUGIN);

        sm_jetpack_rate[SCOUT] = CreateConVar("sm_jetpack_rate_scout", "1", "rate at which the jetpack consumes fuel for scouts");
        sm_jetpack_rate[SNIPER] = CreateConVar("sm_jetpack_rate_sniper", "1", "rate at which the jetpack consumes fuel for snipers");
        sm_jetpack_rate[SOLDIER] = CreateConVar("sm_jetpack_rate_soldier", "1", "rate at which the jetpack consumes fuel for soldiers");
        sm_jetpack_rate[DEMO] = CreateConVar("sm_jetpack_rate_demo", "1", "rate at which the jetpack consumes fuel for demo men");
        sm_jetpack_rate[MEDIC] = CreateConVar("sm_jetpack_rate_medic", "1", "rate at which the jetpack consumes fuel for medics");
        sm_jetpack_rate[HEAVY] = CreateConVar("sm_jetpack_rate_heavy", "2", "rate at which the jetpack consumes fuel for heavys");
        sm_jetpack_rate[PYRO] = CreateConVar("sm_jetpack_rate_pyro", "1", "rate at which the jetpack consumes fuel for pyros");
        sm_jetpack_rate[SPY] = CreateConVar("sm_jetpack_rate_spy", "1", "rate at which the jetpack consumes fuel for spys");
        sm_jetpack_rate[ENGIE] = CreateConVar("sm_jetpack_rate_engineer", "1", "rate at which the jetpack consumes fuel for engineers");
    }
    else
    {
        sm_jetpack_start_sound = CreateConVar("sm_jetpack_start_sound", START_SOUND, "the jetpack start sound", FCVAR_PLUGIN);
        sm_jetpack_stop_sound = CreateConVar("sm_jetpack_stop_sound", STOP_SOUND, "the jetpack stop sound", FCVAR_PLUGIN);
        sm_jetpack_loop_sound = CreateConVar("sm_jetpack_loop_sound", LOOP_SOUND, "the jetpack loop sound", FCVAR_PLUGIN);
        sm_jetpack_empty_sound = CreateConVar("sm_jetpack_empty_sound", EMPTY_SOUND, "the jetpack empty sound", FCVAR_PLUGIN);
        sm_jetpack_refuel_sound = CreateConVar("sm_jetpack_refuel_sound", REFUEL_SOUND, "the jetpack refuel sound", FCVAR_PLUGIN);

        if (GameType == dod)
        {
            sm_jetpack_rate[RIFLEMAN] = CreateConVar("sm_jetpack_rate_rifleman", "1", "rate at which the jetpack consumes fuel for riflemen");
            sm_jetpack_rate[ASSAULT] = CreateConVar("sm_jetpack_rate_assault", "1", "rate at which the jetpack consumes fuel for assault");
            sm_jetpack_rate[SUPPORT] = CreateConVar("sm_jetpack_rate_support", "1", "rate at which the jetpack consumes fuel for support");
            sm_jetpack_rate[DODSNIPER] = CreateConVar("sm_jetpack_rate_sniper", "1", "rate at which the jetpack consumes fuel for snipers");
            sm_jetpack_rate[MG] = CreateConVar("sm_jetpack_rate_mg_type", "2", "rate at which the jetpack consumes fuel for machine gunners");
            sm_jetpack_rate[ROCKETMAN] = CreateConVar("sm_jetpack_rate_rocket", "1", "rate at which the jetpack consumes fuel for rocket men");
        }
        else
            sm_jetpack_rate[0] = CreateConVar("sm_jetpack_rate", "1", "rate at which the jetpack consumes fuel");
    }

    AutoExecConfig();

    CreateConVar("sm_jetpack_version", PLUGIN_VERSION, "", FCVAR_PLUGIN | FCVAR_REPLICATED | FCVAR_NOTIFY);

    /* Account for late loading */
    new Handle:topmenu;
    if (LibraryExists("adminmenu") && ((topmenu = GetAdminTopMenu()) != INVALID_HANDLE))
    {
        OnAdminMenuReady(topmenu);
    }
}

stock bool:SetupSound(const String:wav[], bool:preload=false)
{
    if (wav[0])
    {
        decl String:file[PLATFORM_MAX_PATH+1];
        Format(file, PLATFORM_MAX_PATH, "sound/%s", wav);

        if(FileExists(file))
            AddFileToDownloadsTable(file);

        return PrecacheSound(wav,preload);
    }
    else
        return false;
}

public OnMapStart()
{
    g_fTimer = 0.0;
}

public OnConfigsExecuted()
{
    GetConVarString(sm_jetpack_start_sound, g_StartSound, sizeof(g_LoopSound));
    SetupSound(g_StartSound,true);

    GetConVarString(sm_jetpack_stop_sound, g_StopSound, sizeof(g_LoopSound));
    SetupSound(g_StopSound,true);

    GetConVarString(sm_jetpack_loop_sound, g_LoopSound, sizeof(g_LoopSound));
    SetupSound(g_LoopSound,true);

    GetConVarString(sm_jetpack_empty_sound, g_EmptySound, sizeof(g_EmptySound));
    SetupSound(g_EmptySound,true);

    GetConVarString(sm_jetpack_refuel_sound, g_RefuelSound, sizeof(g_RefuelSound));
    SetupSound(g_RefuelSound,true);
}

public PlayerSpawnEvent(Handle:event,const String:name[],bool:dontBroadcast)
{
    new index=GetClientOfUserId(GetEventInt(event,"userid")); // Get clients index

    if (g_bHasJetpack[index])
    {
        g_iRefuelCount[index] = 0;
        g_iFuel[index] = g_iRefuelAmount[index];

        if (g_iNativeRate[index] < 0)
        {
            new class = 0;
            switch (GameType)
            {
                case tf2: class = _:TF2_GetPlayerClass(index);
                case dod: class = _:DOD_GetPlayerClass(index); 
            }
            g_iRate[index] = GetConVarInt(sm_jetpack_rate[class]);
        }
        else
            g_iRate[index] = g_iNativeRate[index];
    }
    else if (GetConVarBool(sm_jetpack) && GetConVarBool(sm_jetpack_onspawn))
    {
        // Check for Admin Only
        if (GetConVarBool(sm_jetpack_adminonly))
        {
            new AdminId:aid = GetUserAdmin(index);
            if (aid == INVALID_ADMIN_ID || !GetAdminFlag(aid, Admin_Generic, Access_Effective))
                return;
        }

        // Check for allowed teams.
        new team = GetConVarInt(sm_jetpack_team);
        if (team > 0 && team != GetClientTeam(index))
            return;

        new class = 0;
        switch (GameType)
        {
            case tf2: class = _:TF2_GetPlayerClass(index);
            case dod: class = _:DOD_GetPlayerClass(index); 
        }

        g_iRate[index] = GetConVarInt(sm_jetpack_rate[class]);
        g_iFuel[index] = g_iRefuelAmount[index] = GetConVarInt(sm_jetpack_fuel);
        g_fRefuelingTime[index] = GetConVarFloat(sm_jetpack_refueling_time);
        g_iMaxRefuels[index] = GetConVarInt(sm_jetpack_max_refuels);
        g_bHasJetpack[index] = true;
        g_iRefuelCount[index] = 0;

        if (GetConVarBool(sm_jetpack_announce))
        {
            PrintToChat(index,"%c[Jetpack] %cIs enabled, valid commands are: [%c+jetpack%c] [%c-jetpack%c]",
                        COLOR_GREEN,COLOR_DEFAULT,COLOR_GREEN,COLOR_DEFAULT,COLOR_GREEN,COLOR_DEFAULT);
        }
    }
}

public OnGameFrame()
{
    if ((g_iNativeJetpacks > 0 || GetConVarBool(sm_jetpack)) && g_fTimer < GetGameTime() - 0.075)
    {
        g_fTimer = GetGameTime();

        for(new i = 1; i <= MaxClients; i++)
        {
            if(g_bJetpackOn[i])
            {
                if(!IsPlayerAlive(i))
                    StopJetpack(i);
                else
                {
                    if (g_iFuel[i] == 0)
                    {
                        StopJetpack(i);
                        SendTopMessage(i, 1, 1, 255,0,0,128, "[] Your jetpack has run out of fuel");
                        PrintToChat(i,"%c[Jetpack] %cYour jetpack has run out of fuel",
                                	COLOR_GREEN,COLOR_DEFAULT);

                        EmptyEffect(i);

                        if (g_EmptySound[0])
                            EmitSoundToClient(i, g_EmptySound);

                        new refuels = g_iMaxRefuels[i];
                        if (refuels < 0 || g_iRefuelCount[i] < refuels)
                            CreateTimer(g_fRefuelingTime[i],RefuelJetpack,i);
                    }
                    else
                    {
                        if (sm_jetpack_noflag && GetConVarBool(sm_jetpack_noflag) && TF2_HasTheFlag(i))
                        {
                            StopJetpack(i);
                            return;
                        }
                        else if (g_iFuel[i] > 0 && g_iFuel[i] < 25)
                        {
                            // Low on Fuel, Make it sputter.
                            StopJetpackSound(i);
                            DeleteParticle(g_JetpackParticle[i][0]);
                            DeleteParticle(g_JetpackParticle[i][1]);
                            g_JetpackParticle[i][0] = 0;
                            g_JetpackParticle[i][1] = 0;

                            if (g_iFuel[i] % 2)
                                SetMoveType(i, MOVETYPE_WALK, MOVECOLLIDE_DEFAULT);
                            else
                            {
                                EmitJetpackSound(INVALID_HANDLE, i);

                                SetMoveType(i, (GetConVarInt(sm_jetpack_gravity)) ? MOVETYPE_FLYGRAVITY : MOVETYPE_FLY, MOVECOLLIDE_FLY_BOUNCE);
                                AddVelocity(i, GetConVarFloat(sm_jetpack_speed));
                                AddFireEffect(i);
                            }
                        }
                        else
                        {
                            AddVelocity(i, GetConVarFloat(sm_jetpack_speed));
                            AddFireEffect(i);
                        }

                        if (g_iFuel[i] > 0)
                        {
                            g_iFuel[i] -= g_iRate[i];
                            if (g_iFuel[i] < 0)
                                g_iFuel[i] = 0;

                            /* Display the Fuel Gauge */
                            new String:gauge[30] = "[====+=====|=====+====]";
                            new Float:percent = float(g_iFuel[i]) / float(g_iRefuelAmount[i]);
                            new pos = RoundFloat(percent * 20.0)+1;
                            if (pos < 21)
                            {
                                gauge{pos} = ']';
                                gauge{pos+1} = 0;
                            }

                            new r,g,b;
                            if (percent <= 0.25 || g_iFuel[i] < 25)
                            {
                                r = 255;
                                g = 0;
                                b = 0;
                            }
                            else if (percent >= 0.50)
                            {
                                r = 0;
                                g = 255;
                                b = 0;
                            }
                            else
                            {
                                r = 255;
                                g = 255;
                                b = 0;
                            }
                            SendTopMessage(i, pos+2, 1, r,g,b,255, gauge);
                        }
                    }
                }
            }
        }
    }
}

public Action:RefuelJetpack(Handle:timer,any:client)
{
    if (client && g_bHasJetpack[client] && IsClientConnected(client) && IsPlayerAlive(client))
    {
        new refuels = g_iMaxRefuels[client];
        if (refuels < 0 || g_iRefuelCount[client] < refuels)
        {
            new tank_size = g_iRefuelAmount[client];
            if (g_iFuel[client] < tank_size)
            {
                g_iRefuelCount[client]++;
                g_iFuel[client] = tank_size;

                SendTopMessage(client, 30, 2, 0,255,0,128, "[====+=====|=====+====]");
                PrintToChat(client,"%c[Jetpack] %cYour jetpack has been refueled",
                            COLOR_GREEN,COLOR_DEFAULT);

                if (g_RefuelSound[0])
                    EmitSoundToClient(client, g_RefuelSound);
            }
        }
    }
    return Plugin_Handled;
}

public OnClientPutInServer(client)
{
    g_bHasJetpack[client] = false;
    if (g_bFromNative[client])
    {
        g_bFromNative[client] = false;
        g_iNativeJetpacks--;
    }
}

public OnClientDisconnect(client)
{
    StopJetpack(client);
    g_bHasJetpack[client] = false;
    if (g_bFromNative[client])
    {
        g_bFromNative[client] = false;
        g_iNativeJetpacks--;
    }
}

public Action:JetpackPressed(client, args)
{
    if (g_iNativeJetpacks > 0 || GetConVarBool(sm_jetpack))
        StartJetpack(client);

    return Plugin_Continue;
}

public Action:JetpackReleased(client, args)
{
    StopJetpack(client);
    return Plugin_Continue;
}

StartJetpack(client)
{
    if (g_bHasJetpack[client] && !g_bJetpackOn[client] && g_iFuel[client] != 0 && IsPlayerAlive(client) &&
        !(sm_jetpack_noflag && GetConVarBool(sm_jetpack_noflag) && TF2_HasTheFlag(client)))
    {
        new Action:res = Plugin_Continue;
        Call_StartForward(fwdOnJetpack);
        Call_PushCell(client);
        Call_Finish(res);

        if (res == Plugin_Continue)
        {
            SetMoveType(client, (GetConVarInt(sm_jetpack_gravity)) ? MOVETYPE_FLYGRAVITY : MOVETYPE_FLY, MOVECOLLIDE_FLY_BOUNCE);
            g_bJetpackOn[client] = true;

            if (g_StartSound[0])
            {
                decl Float:vecPos[3];
                GetClientAbsOrigin(client, vecPos);
                EmitSoundToAll(g_StartSound, client, SNDCHAN_AUTO, SNDLEVEL_NORMAL, SND_NOFLAGS,
                               GetConVarFloat(sm_jetpack_volume), SNDPITCH_NORMAL, -1,
                               vecPos, NULL_VECTOR, true, 0.0);

                if (g_LoopSound[0])
                    CreateTimer(0.02, EmitJetpackSound, client);
            }
            else if (g_LoopSound[0])
                EmitJetpackSound(INVALID_HANDLE, client);

            if (GameType == tf2 && !IsEntLimitReached(.client=client,.message="unable to create flame particles"))
            {
                static const Float:ang[3] = { -25.0, 90.0, 0.0 };
                static const Float:pos[3] = {   0.0, 10.0, 1.0 };

                CreateParticle("muzzle_minigun", 0.15, client, Attach, "flag", pos, ang);

                if (g_JetpackParticle[client][0] == 0)
                {
                    if (TFTeam:GetClientTeam(client) == TFTeam_Red)
                    {
                        g_JetpackParticle[client][0] = EntIndexToEntRef(CreateParticle("flamethrower_crit_red", 0.0,
                                                                                        client, Attach, "flag",
                                                                                        NULL_VECTOR, ang));
                    }
                    else
                    {
                        g_JetpackParticle[client][0] = EntIndexToEntRef(CreateParticle("flamethrower_crit_blue", 0.0,
                                                                                        client, Attach, "flag",
                                                                                        NULL_VECTOR, ang));
                    }
                }

                if (g_JetpackParticle[client][1] == 0)
                {
                    g_JetpackParticle[client][1] = EntIndexToEntRef(CreateParticle("rockettrail_!", 0.0,
                                                                                    client, Attach, "flag",
                                                                                    NULL_VECTOR, ang));
                }
            }
        }
    }
}

StopJetpack(client)
{
    StopJetpackSound(client);
    DeleteParticle(g_JetpackParticle[client][0]);
    DeleteParticle(g_JetpackParticle[client][1]);
    g_JetpackParticle[client][0] = 0;
    g_JetpackParticle[client][1] = 0;

    if (g_bJetpackOn[client])
    {
        g_bJetpackOn[client] = false;
        if(IsPlayerAlive(client))
            SetMoveType(client, MOVETYPE_WALK, MOVECOLLIDE_DEFAULT);
    }
}

public Action:EmitJetpackSound(Handle:timer, any:client)
{
    if (g_bJetpackOn[client] && g_LoopSound[0])
    {
        decl Float:vecPos[3];
        GetClientAbsOrigin(client, vecPos);
        EmitSoundToAll(g_LoopSound, client, SNDCHAN_AUTO, SNDLEVEL_NORMAL, SND_NOFLAGS,
                       GetConVarFloat(sm_jetpack_volume), SNDPITCH_NORMAL, -1,
                       vecPos, NULL_VECTOR, true, 0.0);
    }
}

StopJetpackSound(client)
{
    if (g_StartSound[0])
        StopSound(client, SNDCHAN_AUTO, g_StartSound);

    if (g_LoopSound[0])
        StopSound(client, SNDCHAN_AUTO, g_LoopSound);

    if (g_EmptySound[0])
        StopSound(client, SNDCHAN_AUTO, g_EmptySound);
}

SetMoveType(client, MoveType:movetype, movecollide)
{
    SetEntityMoveType(client,movetype);
    if(g_iMoveCollide != -1)
        SetEntData(client, g_iMoveCollide, movecollide);
}

AddVelocity(client, Float:speed)
{
    if (g_iVelocity == -1) return;

    decl Float:vecVelocity[3];
    GetEntDataVector(client, g_iVelocity, vecVelocity);

    vecVelocity[2] += speed;

    //give the player a little push if they're on the ground
    //(fixes stuck issue from pyro/medic updates)
    if (GetEntityFlags(client) & FL_ONGROUND)
    {
        decl Float:vecOrigin[3];
        GetClientAbsOrigin(client, vecOrigin);
        vecOrigin[2] += 1; //gets player off the ground if they're not in the air
        TeleportEntity(client, vecOrigin, NULL_VECTOR, vecVelocity);
    }
    else
        TeleportEntity(client, NULL_VECTOR, NULL_VECTOR, vecVelocity);
}

// Updated by Grrrrrrrrrrrrrrrrrrr
AddFireEffect(client)
{
    if (GameType != tf2 && GameType != l4d && GameType != l4d2)
    {
        decl Float:vecPos[3],Float:vecDir[3];
        GetClientAbsOrigin(client, vecPos);
        GetClientEyePosition(client,vecDir);

        vecDir[0] = 80.0;
        if(vecDir[1]==0.0)
            vecDir[1] = 179.8;
        else if(vecDir[1]==90.0||vecDir[1]==-90.0)
            vecDir[1] = (vecDir[1]*-1.0);
        else if(vecDir[1]>90.0)
            vecDir[1] = ((vecDir[1]-90.0)*-1.0);
        else if(vecDir[1]<-90.0)
            vecDir[1] = ((vecDir[1]+90.0)*-1.0);
        else if(vecDir[1]<90.0&&vecDir[1]>0.0)
            vecDir[1] = ((vecDir[1]+90.0)*-1.0);
        else if(vecDir[1]<0.0&&vecDir[1]>-90.0)
            vecDir[1] = ((vecDir[1]-90.0)*-1.0);

        TE_SetupEnergySplash(vecPos, vecDir, false);
        TE_SendToAll();
    }
}

EmptyEffect(client)
{
	if (GameType == tf2 && !IsEntLimitReached(.client=client,.message="unable to create empty particle"))
	{
		static const Float:ang[3] = { -25.0, 90.0, 0.0 };
		static const Float:pos[3] = {   0.0, 10.0, 1.0 };
		CreateParticle("muzzle_minigun", 0.15, client, Attach, "flag", pos, ang);	
	}
	else
	{
		decl Float:vecPos[3],Float:vecDir[3];
		GetClientAbsOrigin(client, vecPos);
		GetClientEyePosition(client,vecDir);

		vecDir[0] = 80.0;
		if(vecDir[1]==0.0)
			vecDir[1] = 179.8;
		else if(vecDir[1]==90.0||vecDir[1]==-90.0)
			vecDir[1] = (vecDir[1]*-1.0);
		else if(vecDir[1]>90.0)
			vecDir[1] = ((vecDir[1]-90.0)*-1.0);
		else if(vecDir[1]<-90.0)
			vecDir[1] = ((vecDir[1]+90.0)*-1.0);
		else if(vecDir[1]<90.0&&vecDir[1]>0.0)
			vecDir[1] = ((vecDir[1]+90.0)*-1.0);
		else if(vecDir[1]<0.0&&vecDir[1]>-90.0)
			vecDir[1] = ((vecDir[1]-90.0)*-1.0);

		TE_SetupDust(vecPos,vecDir,15.0,100.0);
		TE_SendToAll();
	}
}

public Native_StartJetpack(Handle:plugin,numParams)
{
    new client = GetNativeCell(1);
    if (client > 0 && client <= MAXPLAYERS+1)
        StartJetpack(client);
}

public Native_StopJetpack(Handle:plugin,numParams)
{
    new client = GetNativeCell(1);
    if (client > 0 && client <= MAXPLAYERS+1)
        StopJetpack(client);
}

public Native_GiveJetpack(Handle:plugin,numParams)
{
    new client = GetNativeCell(1);
    if (client > 0 && client <= MAXPLAYERS+1)
    {
        g_iNativeJetpacks++;
        g_bHasJetpack[client] = true;
        g_bFromNative[client] = true;
        g_iRefuelCount[client] = 0;

        g_iFuel[client] = g_iRefuelAmount[client] = GetNativeCell(2);
        g_fRefuelingTime[client] = GetNativeCell(3);
        g_iMaxRefuels[client] = GetNativeCell(4);
        g_iNativeRate[client] = GetNativeCell(5);
        if (g_iNativeRate[client] < 0)
        {
            new class = 0;
            switch (GameType)
            {
                case tf2: class = _:TF2_GetPlayerClass(client);
                case dod: class = _:DOD_GetPlayerClass(client); 
            }
            g_iRate[client] = GetConVarInt(sm_jetpack_rate[class]);
        }
        else
            g_iRate[client] = g_iNativeRate[client];

        return g_iFuel[client];
    }
    return -1;
}

public Native_TakeJetpack(Handle:plugin,numParams)
{
    new client = GetNativeCell(1);
    if (client > 0 && client <= MAXPLAYERS+1)
    {
        StopJetpack(client);
        g_bHasJetpack[client] = false;
        if (g_bFromNative[client])
        {
            g_bFromNative[client] = false;
            g_iNativeJetpacks--;
        }
    }
}

public Native_GiveJetpackFuel(Handle:plugin,numParams)
{
    new client = GetNativeCell(1);
    if (client > 0 && client <= MAXPLAYERS+1)
    {
        new amount = GetNativeCell(2);
        if (amount >= 0)
            g_iFuel[client] += amount;
        else
            g_iFuel[client] = amount;

        new refuels = GetNativeCell(3);
        if (refuels >= 0)
            g_iMaxRefuels[client] += refuels;
        else
            g_iMaxRefuels[client] = refuels;

        return g_iFuel[client];
    }
    return -1;
}

public Native_TakeJetpackFuel(Handle:plugin,numParams)
{
    new client = GetNativeCell(1);
    if (client > 0 && client <= MAXPLAYERS+1)
    {
        new amount = GetNativeCell(2);
        if (amount >= 0)
        {
            g_iFuel[client] -= amount;
            if (g_iFuel[client] < 0)
                g_iFuel[client] = 0;
        }
        else
            g_iFuel[client] = 0;

        new refuels = GetNativeCell(3);
        if (refuels >= 0)
        {
            g_iMaxRefuels[client] -= refuels;
            if (g_iMaxRefuels[client] < 0)
                g_iMaxRefuels[client] = 0;
        }
        else
            g_iMaxRefuels[client] = 0;

        return g_iFuel[client];
    }
    return -1;
}

public Native_SetJetpackFuel(Handle:plugin,numParams)
{
    new client = GetNativeCell(1);
    if (client > 0 && client <= MAXPLAYERS+1)
    {
        g_iFuel[client] = g_iRefuelAmount[client] = GetNativeCell(2);
        g_iMaxRefuels[client] = GetNativeCell(3);
    }
    else
    {
        SetConVarInt(sm_jetpack_fuel, GetNativeCell(2));
        SetConVarInt(sm_jetpack_max_refuels, GetNativeCell(3));
    }
}

public Native_SetJetpackRate(Handle:plugin,numParams)
{
    new client = GetNativeCell(1);
    if (client > 0 && client <= MAXPLAYERS+1)
    {
        g_iNativeRate[client] = GetNativeCell(2);
        if (g_iNativeRate[client] < 0)
        {
            new class = 0;
            switch (GameType)
            {
                case tf2: class = _:TF2_GetPlayerClass(client);
                case dod: class = _:DOD_GetPlayerClass(client); 
            }
            g_iRate[client] = GetConVarInt(sm_jetpack_rate[class]);
        }
        else
            g_iRate[client] = g_iNativeRate[client];
    }
}

public Native_SetJetpackRefuelingTime(Handle:plugin,numParams)
{
    new client = GetNativeCell(1);
    if (client > 0 && client <= MAXPLAYERS+1)
        g_fRefuelingTime[client] =  Float:GetNativeCell(2);
    else
        SetConVarFloat(sm_jetpack_refueling_time, Float:GetNativeCell(2));
}

public Native_GetJetpackFuel(Handle:plugin,numParams)
{
    return g_iFuel[GetNativeCell(1)];
}

public Native_GetJetpackRate(Handle:plugin,numParams)
{
    return g_iRate[GetNativeCell(1)];
}

public Native_GetJetpackRefuelingTime(Handle:plugin,numParams)
{
    return _:(g_fRefuelingTime[GetNativeCell(1)]);
}

public Native_GetJetpack(Handle:plugin,numParams)
{
    return g_bHasJetpack[GetNativeCell(1)];
}

public Native_ControlJetpack(Handle:plugin,numParams)
{
    g_bNativeOverride = GetNativeCell(1);
}

public Action:Command_GiveJetpack(client,argc)
{
    if (argc>=1)
    {
        decl String:target[64];
        GetCmdArg(1,target,64);
        new count = SetJetpack(client,target,true);
        if (!count)
            ReplyToTargetError(client, count);
    }
    else
    {
        ReplyToCommand(client,"%c[Jetpack] Usage: %csm_jetpack_give <@userid/partial name>",
                       COLOR_GREEN,COLOR_DEFAULT);
    }
    return Plugin_Handled;
}

public Action:Command_TakeJetpack(client,argc)
{
    if (argc>=1)
    {
        decl String:target[64];
        GetCmdArg(1,target,64);

        new count=SetJetpack(client,target,false);
        if (!count)
            ReplyToTargetError(client, count);
    }
    else
    {
        ReplyToCommand(client,"%c[Jetpack] Usage: %csm_jetpack_take <@userid/partial name>",
                       COLOR_GREEN,COLOR_DEFAULT);
    }
    return Plugin_Handled;
}

public SetJetpack(client,const String:target[],bool:enable)
{
    decl bool:isml, String:name[64], clients[MAXPLAYERS+1];
    new count=ProcessTargetString(target,client,clients,MAXPLAYERS+1,COMMAND_FILTER_NO_BOTS,
                                  name,sizeof(name),isml);
    if (count)
    {
        for(new x=0;x<count;x++)
        {
            new index = clients[x];
            switch (PerformJetpack(client, index, enable))
            {
                case 0:
                {
                    if (enable)
                        ReplyToCommand(client, "Gave a jetpack to %N", index);
                    else
                        ReplyToCommand(client, "Removed the jetpack from %N", index);
                }
                case 1: ReplyToCommand(client,"%N already has a jetpack", index);
                case 2: ReplyToCommand(client,"Unable to remove the jetpack");
            }
        }
    }
    return count;
}

public PerformJetpack(client, target, bool:enable)
{
    if (enable)
    {
        if (!g_bHasJetpack[target])
        {
            new class = 0;
            switch (GameType)
            {
                case tf2: class = _:TF2_GetPlayerClass(client);
                case dod: class = _:DOD_GetPlayerClass(client); 
            }

            g_iRate[target] = GetConVarInt(sm_jetpack_rate[class]);
            g_iFuel[target] = g_iRefuelAmount[target] = GetConVarInt(sm_jetpack_fuel);
            g_fRefuelingTime[target] = GetConVarFloat(sm_jetpack_refueling_time);
            g_iMaxRefuels[target] = GetConVarInt(sm_jetpack_max_refuels);
            g_bHasJetpack[target] = true;
            g_iRefuelCount[target] = 0;

            if(GetConVarBool(sm_jetpack_announce))
            {
                PrintToChat(target,"%c[Jetpack] %cIs enabled, valid commands are: [%c+jetpack%c] [%c-jetpack%c]",
                            COLOR_GREEN,COLOR_DEFAULT,COLOR_GREEN,COLOR_DEFAULT,COLOR_GREEN,COLOR_DEFAULT);
            }
            LogAction(client, target, "\"%L\" gave a jetpack to \"%L\"", client, target);
            return 0;
        }
        else
            return 1;
    }
    else
    {
        if (!g_bFromNative[target])
        {
            StopJetpack(target);
            g_bHasJetpack[target] = false;
            LogAction(client, target, "\"%L\" took the jetpack from \"%L\"", client, target);
            return 0;
        }
        else
            return 2;
    }
}

public OnLibraryRemoved(const String:name[])
{
    if (StrEqual(name, "adminmenu"))
        hAdminMenu = INVALID_HANDLE;
}

public OnAdminMenuReady(Handle:topmenu)
{
    /* Block us from being called twice */
    if (topmenu != hAdminMenu)
    {
        /* Save the Handle */
        hAdminMenu = topmenu;

        if (!g_bNativeOverride)
        {
            new TopMenuObject:server_commands = FindTopMenuCategory(hAdminMenu, ADMINMENU_PLAYERCOMMANDS);
            oGiveJetpack = AddToTopMenu(hAdminMenu, "sm_give_jetpack", TopMenuObject_Item, AdminMenu,
                                        server_commands, "sm_give_jetpack", ADMFLAG_JETPACK);
            oTakeJetpack = AddToTopMenu(hAdminMenu, "sm_take_jetpack", TopMenuObject_Item, AdminMenu,
                                        server_commands, "sm_take_jetpack", ADMFLAG_JETPACK);
        }
    }
}

public AdminMenu(Handle:topmenu, TopMenuAction:action, TopMenuObject:object_id, param, String:buffer[], maxlength)
{
    if (action == TopMenuAction_DisplayOption)
    {
        if (object_id == oGiveJetpack)
            Format(buffer, maxlength, "Give Jetpack");
        else if (object_id == oTakeJetpack)
            Format(buffer, maxlength, "Take Jetpack");
    }
    else if (action == TopMenuAction_SelectOption)
    {
        JetpackMenu(param, object_id);
    }
}

JetpackMenu(client, TopMenuObject:object_id)
{
    new Handle:menu = CreateMenu(MenuHandler_Jetpack);

    SetMenuTitle(menu, (object_id == oGiveJetpack)
                       ? "Give a Jetpack to"
                       : "Take the Jetpack from");

    SetMenuExitBackButton(menu, true);

    AddTargetsToMenu(menu, client, true, true);

    DisplayMenu(menu, client, MENU_TIME_FOREVER);
}

public MenuHandler_Jetpack(Handle:menu, MenuAction:action, param1, param2)
{
    decl String:title[32];
    GetMenuTitle(menu,title,sizeof(title));

    if (action == MenuAction_End)
    {
        CloseHandle(menu);
    }
    else if (action == MenuAction_Cancel)
    {
        if (param2 == MenuCancel_ExitBack && hAdminMenu != INVALID_HANDLE)
        {
            DisplayTopMenu(hAdminMenu, param1, TopMenuPosition_LastCategory);
        }
    }
    else if (action == MenuAction_Select)
    {
        decl String:info[32];
        new userid, target;

        GetMenuItem(menu, param2, info, sizeof(info));
        userid = StringToInt(info);

        if ((target = GetClientOfUserId(userid)) == 0)
        {
            PrintToChat(param1, "[SM] Player no longer available");
        }
        else if (!CanUserTarget(param1, target))
        {
            PrintToChat(param1, "[SM] Unable to target");
        }
        else
        {
            new String:name[32];
            GetClientName(target, name, sizeof(name));

            if (StrContains(title, "Give") != -1)
            {
                PerformJetpack(param1, target, true);
                ShowActivity2(param1, "[SM] ", "Gave %s a jetpack", name);
            }
            else
            {
                PerformJetpack(param1, target, false);
                ShowActivity2(param1, "[SM] ", "Took the jetpack from %s", name);
            }
        }

        /* Re-draw the menu if they're still valid */
        if (IsClientInGame(param1) && !IsClientInKickQueue(param1))
        {
            if (StrContains(title, "Give") != -1)
            {
                JetpackMenu(param1, oGiveJetpack);
            }
            else
            {
                JetpackMenu(param1, oTakeJetpack);
            }
        }
    }
}

/**
 * Determine if client has the flag
 */
#tryinclude <tf2_flag>
#if !defined _tf2_flag_included
    stock bool:TF2_HasTheFlag(client)
    {
        new ent = -1;
        while ((ent = FindEntityByClassname(ent, "item_teamflag")) != -1)
        {
            if (GetEntPropEnt(ent, Prop_Data, "m_hOwnerEntity")==client)
                return true;
        }
        return false;
    }
#endif
