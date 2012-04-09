// vim: set et ai ts=4 sw=4 :
// SourceMod Script
//
// Developed by <eVa>Dog
// February 2009
// http://www.theville.org
//
// Modified by -=|JFH|=-Naris
// Added Native Interface,
// Merged TF2 variant from <eVa>Dog,
// Numerous other changes

//
// DESCRIPTION:
// This plugin is a port of my TNT plugin
// originally created using EventScripts

#include <sourcemod>
#include <sdktools>

#undef REQUIRE_EXTENSIONS
#include <tf2_stocks>
#define REQUIRE_EXTENSIONS

#define PLUGIN_VERSION "2.5.0"

#define ADMIN_LEVEL ADMFLAG_SLAY

#define MAXTNT 10

new Handle:g_Cvar_tntAmount     = INVALID_HANDLE;
new Handle:g_Cvar_Damage        = INVALID_HANDLE;
new Handle:g_Cvar_Radius        = INVALID_HANDLE;
new Handle:g_Cvar_Admins        = INVALID_HANDLE;
new Handle:g_Cvar_Enable        = INVALID_HANDLE;
new Handle:g_Cvar_Delay         = INVALID_HANDLE;
new Handle:g_Cvar_Restrict      = INVALID_HANDLE;
new Handle:g_Cvar_Mode          = INVALID_HANDLE;
new Handle:g_Cvar_Death         = INVALID_HANDLE;
new Handle:g_Cvar_tntDetDelay   = INVALID_HANDLE;
new Handle:g_Cvar_PlantDelay    = INVALID_HANDLE;
new Handle:g_Cvar_PrimeDelay    = INVALID_HANDLE;
new Handle:g_Cvar_Announce      = INVALID_HANDLE;
new Handle:g_Cvar_FriendlyFire  = INVALID_HANDLE;

new Handle:fwdOnTNTBombed       = INVALID_HANDLE;

new g_tntAmount[MAXPLAYERS+1];
new bool:g_tntEnabled[MAXPLAYERS+1];
new bool:g_can_plant[MAXPLAYERS+1];

new bool:g_tntPrimed[MAXPLAYERS+1][MAXTNT+1];
new g_tntEntity[MAXPLAYERS+1][MAXTNT+1];

new String:g_TNTModel[128];
new String:g_plant_sound[128];
new String:g_primed_sound[128];
new String:g_defused_sound[128];
new bool:g_flipAngles;

new g_Explosion;

new bool:g_NativeControl = false;
new Float:g_tntPrimeDelay[MAXPLAYERS+1];
new Float:g_tntDetDelay[MAXPLAYERS+1];
new bool:g_tntDeath[MAXPLAYERS+1];
new g_tntAllowed[MAXPLAYERS+1];
new g_tntMode[MAXPLAYERS+1];

public Plugin:myinfo = 
{
    name = "Remote IED or TNT",
    author = "<eVa>Dog",
    description = "Plant packs and detonate them remotely or at a distance",
    version = PLUGIN_VERSION,
    url = "http://www.theville.org"
}

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
 * Description: Functions to return infomation about TF2 objects.
 */
#tryinclude <tf2_objects>
#if !defined _tf2_objects_included
    enum TFObjectType
    {
        TFObjectType_Dispenser = 0,
        TFObjectType_Teleporter,
        TFObjectType_Sentrygun,
        TFObjectType_TeleporterEntry,
        TFObjectType_TeleporterExit,
        TFObjectType_Amplifier,
        TFObjectType_Sapper,
        TFObjectType_Unknown
    };

    stock const String:TF2_ObjectClassNames[TFObjectType][] =
    {
        "obj_dispenser",
        "obj_teleporter",
        "obj_sentrygun",
        "obj_teleporter", // _entrance
        "obj_teleporter", // _exit
        "obj_dispenser", // amplifier
        "obj_sapper",
        ""
    };

    stock const String:TF2_ObjectNames[TFObjectType][] =
    {
        "Dispenser",
        "Teleporter",
        "Sentry Gun",
        "Teleporter Entrance",
        "Teleporter Exit",
        "Amplifier",
        "Sapper",
        ""
    };

    stock TF2_ObjectModes[TFObjectType] =
    {
        -1, // dispenser
        -1, // teleporter (either)
        -1, // sentrygun
         0, // telporter_entrance
         1, // teleporter_exit
        -1, // amplifier
        -1, // sapper
        -1  // unknown
    };

    stock TFObjectType:TF2_GetObjectType(entity, bool:specific=false)
    {
        decl String:class[32];
        if (GetEdictClassname(entity,class,sizeof(class)))
            return TF2_GetObjectTypeFromClass(entity, class, specific);
        else
            return TFObjectType_Unknown;
    }

    stock TFObjectType:TF2_GetObjectTypeFromClass(target, const String:class[], bool:specific=false)
    {
        if (StrEqual(class, TF2_ObjectClassNames[TFObjectType_Sentrygun]))
            return TFObjectType_Sentrygun;
        else if (StrEqual(class, TF2_ObjectClassNames[TFObjectType_Dispenser]))
            return TFObjectType_Dispenser;
        else if (StrEqual(class, TF2_ObjectClassNames[TFObjectType_Teleporter]))
        {
            return (specific) ? (TFObjectType_TeleporterEntry + TFObjectType:GetEntProp(target, Prop_Send, "m_iObjectMode"))
                              : TFObjectType_Teleporter;
        }
        else if (StrEqual(class, TF2_ObjectClassNames[TFObjectType_Sapper]))
            return TFObjectType_Sapper;
        else
            return TFObjectType_Unknown;
    }
#endif

public APLRes:AskPluginLoad2(Handle:myself, bool:late, String:error[], err_max)
{
    CreateNative("ControlTNT", Native_ControlTNT);
    CreateNative("SetTNT", Native_SetTNT);
    CreateNative("PlantTNT", Native_PlantTNT);
    CreateNative("DefuseTNT", Native_DefuseTNT);
    CreateNative("DetonateTNT", Native_DetonateTNT);
    CreateNative("TNT", Native_TNT);

    fwdOnTNTBombed=CreateGlobalForward("OnTNTBombed",ET_Hook,Param_Cell,Param_Cell,Param_Cell);

    RegPluginLibrary("sm_tnt");
    return APLRes_Success;
}

public OnPluginStart()
{
    RegConsoleCmd("sm_plant", plant, " -  Plants TNT at coords specified by player's crosshairs");
    RegConsoleCmd("sm_defuse", defuse, " -  Defuses a TNT pack under player's crosshairs ");
    RegConsoleCmd("sm_det", detonate, " -  Detonates a TNT pack under player's crosshairs ");
    RegConsoleCmd("sm_tnt", tnt, " -  Detonates or Defuses a TNT pack under player's crosshairs ");
    
    CreateConVar("sm_tnt_version", PLUGIN_VERSION, "Version of SourceMod TNT on this server", FCVAR_PLUGIN|FCVAR_SPONLY|FCVAR_REPLICATED|FCVAR_NOTIFY);
    g_Cvar_tntAmount    = CreateConVar("sm_tnt_amount", "2", " Number of tnt packs per player at spawn (max 10)", FCVAR_PLUGIN);
    g_Cvar_Damage       = CreateConVar("sm_tnt_damage", "200", " Amount of damage that the tnt does", FCVAR_PLUGIN);
    g_Cvar_Radius       = CreateConVar("sm_tnt_radius", "200.0", " Radius of explosion", FCVAR_PLUGIN);
    g_Cvar_Admins       = CreateConVar("sm_tnt_admins", "0", " Allow Admins only to use tnt", FCVAR_PLUGIN);
    g_Cvar_Enable       = CreateConVar("sm_tnt_enabled", "1", " Enable/Disable the TNT plugin", FCVAR_PLUGIN);
    g_Cvar_Delay        = CreateConVar("sm_tnt_delay", "5.0", " Delay between spawning and making tnt available", FCVAR_PLUGIN);
    g_Cvar_Restrict     = CreateConVar("sm_tnt_restrict", "0", " Class to restrict TNT to (see forum thread)", FCVAR_PLUGIN);
    g_Cvar_Mode         = CreateConVar("sm_tnt_mode", "3", " Detonation mode: 0=radio 1=crosshairs 2=timer 3=timer&crosshairs|radio", FCVAR_PLUGIN);
    g_Cvar_Death        = CreateConVar("sm_tnt_death", "1", " Enable/Disable detonation on owner death/change round", FCVAR_PLUGIN);
    g_Cvar_tntDetDelay  = CreateConVar("sm_tnt_det_delay", "10.0", " Detonation delay", FCVAR_PLUGIN);
    g_Cvar_PlantDelay   = CreateConVar("sm_tnt_plant_delay", "5.0", " Delay between planting TNT", FCVAR_PLUGIN);
    g_Cvar_PrimeDelay   = CreateConVar("sm_tnt_prime_delay", "5.0", " How long it takes TNT to be primed after planting", FCVAR_PLUGIN);
    g_Cvar_Announce     = CreateConVar("sm_tnt_announce", "1", " Announce usage instructions", FCVAR_PLUGIN);

    g_Cvar_FriendlyFire = FindConVar("mp_friendlyfire");
}

public OnMapStart()
{
    if (g_NativeControl || GetConVarInt(g_Cvar_Enable))
    {
        HookEvent("player_spawn", PlayerSpawnEvent);
        HookEvent("player_death", PlayerDeathEvent);
        HookEvent("player_disconnect", PlayerDisconnectEvent);
        
        HookEntityOutput("prop_physics", "OnTakeDamage", TakeDamage);
        HookEntityOutput("prop_physics", "OnBreak", Break);
        
        g_Explosion = PrecacheModel("sprites/sprite_fire01.vmt");

        if (GetGameType() == dod)
        {
            HookEvent("dod_round_start", RoundStartEvent);
            strcopy(g_plant_sound, sizeof(g_plant_sound), "weapons/c4_plant.wav");
            strcopy(g_primed_sound, sizeof(g_primed_sound), "weapons/grenade_string.wav");
            strcopy(g_defused_sound, sizeof(g_plant_sound), "weapons/c4_disarm.wav");
            strcopy(g_TNTModel, sizeof(g_TNTModel), "models/weapons/w_tnt.mdl");
            g_flipAngles = false;
        }
        else
        {
            if (GameType == tf2)
            {
                HookEvent("teamplay_round_start", RoundStartEvent);
                strcopy(g_plant_sound, sizeof(g_plant_sound), "weapons/stickybomblauncher_det.wav");
                strcopy(g_primed_sound, sizeof(g_primed_sound), "weapons/det_pack_timer.wav");
                strcopy(g_defused_sound, sizeof(g_plant_sound), "weapons/sapper_removed.wav");
            }
            else
            {
                HookEvent("round_start", RoundStartEvent);
                strcopy(g_plant_sound, sizeof(g_plant_sound), "weapons/grenade/tick1.wav");
                strcopy(g_primed_sound, sizeof(g_primed_sound), "weapons/slam/mine_mode.wav");
                strcopy(g_defused_sound, sizeof(g_plant_sound), "weapons/ambient/materials/smallwire_pluck3.wav");
            }

            if (FileExists("models/weapons/nades/duke1/w_grenade_mirv.mdl"))
            {
                strcopy(g_TNTModel, sizeof(g_TNTModel), "models/weapons/nades/duke1/w_grenade_mirv.mdl");
                g_flipAngles = false;
            }
            else
            {
                strcopy(g_TNTModel, sizeof(g_TNTModel), "models/items/grenadeAmmo.mdl");
                g_flipAngles = true;
            }
        }

        PrecacheModel(g_TNTModel, true);
        PrecacheSound(g_plant_sound, true);
        PrecacheSound(g_primed_sound, true);
        PrecacheSound(g_defused_sound, true);
    }
}

public OnClientPostAdminCheck(client)
{
    if (GetConVarInt(g_Cvar_Enable) && GetConVarInt(g_Cvar_Announce) &&
        IsClientInGame(client))
    {
        PrintToConsole(client, "This server is running a TNT/IED plugin");
        PrintToConsole(client, "Bind a key to 'sm_plant' and the TNT will be planted where you aim your crosshairs");
        PrintToConsole(client, "Bind a key to 'sm_defuse' and defuse the TNT pack under your crosshairs");
        
        new String:detmsg[64];
        new mode = GetConVarInt(g_Cvar_Mode);
        new Float:detDelay = GetConVarFloat(g_Cvar_tntDetDelay);
        if (mode == 0)
            strcopy(detmsg, sizeof(detmsg), "Bind a key to 'sm_det' to explode all planted packs");
        else if (mode == 1 || mode == 3 || detDelay == 0.0)
            strcopy(detmsg, sizeof(detmsg), "Bind a key to 'sm_det' and aim your crosshairs at the pack");

        if (mode >= 2 && detDelay > 0.0)
            Format(detmsg, sizeof(detmsg), "Once planted, the pack will explode after %f seconds", detDelay);

        PrintToConsole(client, "Detonate: %s ", detmsg);

        PrintToConsole(client, "Each player receives %i TNT packs", GetConVarInt(g_Cvar_tntAmount));
    }
}

public PlayerSpawnEvent(Handle:event, const String:name[], bool:dontBroadcast)
{
    if (g_NativeControl || GetConVarInt(g_Cvar_Enable))
    {
        new client = GetClientOfUserId(GetEventInt(event, "userid"));
        
        if (!g_NativeControl)
        {
            g_tntMode[client] = GetConVarInt(g_Cvar_Mode);
            g_tntDeath[client] = GetConVarBool(g_Cvar_Death);
            g_tntDetDelay[client] = GetConVarFloat(g_Cvar_tntDetDelay);
            g_tntPrimeDelay[client] = GetConVarFloat(g_Cvar_PrimeDelay);

            if (GetConVarInt(g_Cvar_Admins) == 1)
            {
                if (GetUserFlagBits(client) & ADMIN_LEVEL)
                    g_tntAllowed[client] = GetConVarInt(g_Cvar_tntAmount);
                else
                    g_tntAllowed[client] = 0;
            }
            else
            {
                new restrict = GetConVarInt(g_Cvar_Restrict);
                if (restrict > 0)
                {
                    new class = -1;
                    if (GameType == dod)
                        class = GetEntProp(client, Prop_Send, "m_iPlayerClass")+1;
                    else if (GameType == tf2)
                        class = GetEntProp(client, Prop_Send, "m_iClass");

                    if (class == restrict && class >= 0)
                        g_tntAllowed[client] = GetConVarInt(g_Cvar_tntAmount);
                    else
                        g_tntAllowed[client] = 0;
                }
                else
                    g_tntAllowed[client] = GetConVarInt(g_Cvar_tntAmount);
            }
        }
        else
        {
            if (g_tntAllowed[client] < 0)
                g_tntAllowed[client] = GetConVarInt(g_Cvar_tntAmount);

            if (g_tntMode[client] < 0)
                g_tntMode[client] = GetConVarInt(g_Cvar_Mode);

            if (g_tntDetDelay[client] < 0.0)
                g_tntDetDelay[client] = GetConVarFloat(g_Cvar_tntDetDelay);

            if (g_tntPrimeDelay[client] < 0.0)
                g_tntPrimeDelay[client] = GetConVarFloat(g_Cvar_PrimeDelay);
        }

        if (g_tntAllowed[client] > MAXTNT)
            g_tntAllowed[client] = MAXTNT;

        g_tntAmount[client] = g_tntAllowed[client];
        for (new i = g_tntAllowed[client]; i > 0 ; i--)
        {
            g_tntEntity[client][i] = 0;
        }
        
        new Float:delay = GetConVarFloat(g_Cvar_Delay);
        if (delay > 0.0)
        {
            g_tntEnabled[client] = false;
            CreateTimer(delay, SetTNT, client, TIMER_FLAG_NO_MAPCHANGE);
        }
        else
        {
            g_tntEnabled[client] = true;
            g_can_plant[client] = true;
        }
    }
}

public PlayerDeathEvent(Handle:event, const String:name[], bool:dontBroadcast)
{
    if (g_NativeControl || GetConVarInt(g_Cvar_Enable))
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

        g_tntAmount[client] = 0;

        for (new i = g_tntAllowed[client]; i > 0 ; i--)
        {
            new ref = g_tntEntity[client][i];
            if (ref)
            {
                g_tntEntity[client][i] = 0;
                if (EntRefToEntIndex(ref))
                {
                    if (g_tntDeath[client])
                        CreateTimer(0.1, DelayedDetonation, ref, TIMER_FLAG_NO_MAPCHANGE);
                    else
                        CreateTimer(0.1, RemoveTNT, ref, TIMER_FLAG_NO_MAPCHANGE);
                }
            }
        }
    }
}



public PlayerDisconnectEvent(Handle:event, const String:name[], bool:dontBroadcast)
{
    if (g_NativeControl || GetConVarInt(g_Cvar_Enable))
    {
        new client = GetClientOfUserId(GetEventInt(event, "userid"));
        for (new i = g_tntAllowed[client]; i > 0 ; i--)
        {
            new ref = g_tntEntity[client][i];
            if (ref)
            {
                RemoveTNT(INVALID_HANDLE, ref);
                g_tntEntity[client][i] = 0;
            }
        }
    }
}

public RoundStartEvent(Handle:event, const String:name[], bool:dontBroadcast)
{
    if (g_NativeControl || GetConVarInt(g_Cvar_Enable))
    {
        for (new client = 1; client <= MaxClients; client++)
        {
            for (new i = g_tntAllowed[client]; i > 0 ; i--)
            {
                new ref = g_tntEntity[client][i];
                if (ref)
                {
                    g_tntEntity[client][i] = 0;
                    if (EntRefToEntIndex(ref))
                    {
                        if (g_tntDeath[client])
                            CreateTimer(0.1, DelayedDetonation, ref, TIMER_FLAG_NO_MAPCHANGE);
                        else
                            CreateTimer(0.1, RemoveTNT, ref, TIMER_FLAG_NO_MAPCHANGE);
                    }
                }
            }
        }
    }
}

public Action:SetTNT(Handle:timer, any:client)
{
    g_tntEnabled[client] = true;
    g_can_plant[client] = true;
}

public Action:RemoveTNT(Handle:timer, any:ref)
{
    new ent = EntRefToEntIndex(ref);
    if (ent > 0 && IsValidEntity(ent))
    {
        decl Float:tnt_pos[3];
        GetEntPropVector(ent, Prop_Send, "m_vecOrigin", tnt_pos);
        TE_SetupEnergySplash(tnt_pos, NULL_VECTOR, true);
        TE_SendToAll(0.1);

        RemoveEdict(ent);
    }
}


public Action:plant(client, args)
{
    if (g_NativeControl || GetConVarInt(g_Cvar_Enable))
    {
        if (g_tntEnabled[client])
        {   
            if (IsClientInGame(client) && IsPlayerAlive(client))
            { 
                new owner = 0;
                new team = GetClientTeam(client);
                new pack = FindTNT(client, owner);
                if (pack > 0 && owner != 0 && IsClientInGame(owner))
                {
                    if (owner == client)
                        DetonateTNT(client, pack);
                    else if (GetClientTeam(owner) != team)
                        DefuseTNT(client, owner, pack);
                }
                else if (g_can_plant[client] && team > 1)
                {
                    if (g_tntAmount[client] > 0)
                    {
                        if (GameType == tf2)
                        {
                            switch (TF2_GetPlayerClass(client))
                            {
                                case TFClass_Spy:
                                {
                                    new pcond = TF2_GetPlayerConditionFlags(client);
                                    if (TF2_IsCloaked(pcond) || TF2_IsDeadRingered(pcond) || TF2_IsDisguised(pcond))
                                        return Plugin_Handled;
                                }
                                case TFClass_Scout:
                                {
                                    if (TF2_IsPlayerBonked(client))
                                        return Plugin_Handled;
                                }
                            }
                        }

                        decl Float:vOrigin[3];
                        GetClientEyePosition(client,vOrigin);

                        decl Float:vAngles[3];
                        GetClientEyeAngles(client, vAngles);

                        new Handle:trace = TR_TraceRayFilterEx(vOrigin, vAngles, MASK_SHOT, RayType_Infinite, TraceEntityFilterPlayer);
                        if (TR_DidHit(trace))
                        {
                            decl Float:pos[3];
                            TR_GetEndPosition(pos, trace);

                            if (GetVectorDistance(vOrigin, pos) > 200.0)
                            {
                                PrintToChat(client, "[SM] Too far away to plant");
                            }
                            else if (!IsEntLimitReached(.client=client,
                                                        .message="unable to create prop_physics"))
                            {
                                TE_SetupSparks(pos, NULL_VECTOR, 2, 1);
                                TE_SendToAll(0.1);

                                new tntnumber = g_tntAmount[client];
                                new ent = CreateEntityByName("prop_physics_override");
                                if (ent > 0 && IsValidEdict(ent))
                                {
                                    SetEntityModel(ent, g_TNTModel);
                                    if (StrContains(g_TNTModel, "duke") != -1)
                                    {
                                        decl String:skin[4];
                                        Format(skin, sizeof(skin), "%d", team-2);
                                        DispatchKeyValue(ent, "skin", skin);
                                    }

                                    DispatchKeyValue(ent, "StartDisabled", "false");

                                    decl String:string[16];
                                    GetConVarString(g_Cvar_Radius, string, sizeof(string));
                                    DispatchKeyValue(ent, "ExplodeRadius", string);

                                    if (GetConVarBool(g_Cvar_FriendlyFire))
                                        GetConVarString(g_Cvar_Damage, string, sizeof(string));
                                    else
                                        strcopy(string, sizeof(string), "0");

                                    DispatchKeyValue(ent, "ExplodeDamage", string);

                                    DispatchKeyValue(ent, "massScale", "1.0");
                                    DispatchKeyValue(ent, "inertiaScale", "0.1");
                                    DispatchKeyValue(ent, "pressuredelay", "2.0");              

                                    DispatchSpawn(ent);

                                    AcceptEntityInput(ent, "DisableMotion");

                                    new Float:angles[3];
                                    if (pos[2] >= (vOrigin[2] - 50))
                                        angles[0] = g_flipAngles ? 0.0 : 90.0;
                                    else
                                        angles[0] = g_flipAngles ? 90.0 : 0.0;

                                    TeleportEntity(ent, pos, angles, NULL_VECTOR);

                                    new ref = EntIndexToEntRef(ent);
                                    g_tntEntity[client][tntnumber] = ref;
                                    g_tntPrimed[client][tntnumber] = false;

                                    new Float:primeDelay = g_tntPrimeDelay[client];
                                    new Handle:tntpack = CreateDataPack();
                                    WritePackCell(tntpack, client);
                                    WritePackCell(tntpack, tntnumber);
                                    WritePackCell(tntpack, ref);
                                    //CreateTimer(primeDelay, Prime, tntpack, TIMER_DATA_HNDL_CLOSE|TIMER_FLAG_NO_MAPCHANGE);
                                    CreateTimer(primeDelay, Prime, tntpack);

                                    g_tntAmount[client]--;
                                    PrintToChat(client, "[SM] TNT left: %i", g_tntAmount[client]);

                                    EmitSoundToAll(g_plant_sound, ent, _, _, _, 0.8);

                                    if (GameType == dod)
                                        AttachParticle(ent, primeDelay, "grenadetrail");
                                    else if (GameType == tf2)
                                    {
                                        if (team == 2)
                                            AttachParticle(ent, primeDelay, "critical_grenade_red")
                                        else if (team == 3)
                                            AttachParticle(ent, primeDelay, "critical_grenade_blue")
                                    }

                                    CreateTimer(GetConVarFloat(g_Cvar_PlantDelay), AllowPlant, client, TIMER_FLAG_NO_MAPCHANGE);
                                    g_can_plant[client] = false;
                                }
                                else
                                    LogError("Unable to create prop_physics");
                            }
                        }
                        else
                        {
                            PrintToChat(client, "[SM] Too far away to plant");
                        }
                        CloseHandle(trace);
                    }
                    else
                    {
                        PrintToChat(client, "[SM] No TNT left");
                    }
                }
            }
        }
        else
        {
            PrintToChat(client, "[SM] TNT unavailable.  Please wait....");
        }
    }
    return Plugin_Handled;
}

public bool:TraceEntityFilterPlayer(entity, contentsMask)
{
    return !entity || entity > GetMaxClients();
} 

public Action:AllowPlant(Handle:timer, any:client)
{
    g_can_plant[client] = true;
}

public Action:Prime(Handle:timer, Handle:tntpack)
{
    ResetPack(tntpack);
    new owner = ReadPackCell(tntpack);
    new tntnumber = ReadPackCell(tntpack);
    new ref = ReadPackCell(tntpack);
    CloseHandle(tntpack);

    new entity = EntRefToEntIndex(ref);
    if (entity > 0 && IsValidEntity(entity))
    {
        new String:tntname[128];
        Format(tntname, sizeof(tntname), "TNT-%i", entity);
        DispatchKeyValue(entity, "targetname", tntname);

        DispatchKeyValue(entity, "physdamagescale", "9999.0");
        DispatchKeyValue(entity, "spawnflags", "304");
        DispatchKeyValue(entity, "health", "1");
        SetEntProp(entity, Prop_Data, "m_takedamage", 2);

        AcceptEntityInput(entity, "Enable");
        AcceptEntityInput(entity, "TurnOn");
        AcceptEntityInput(entity, "EnableDamageForces");
        g_tntPrimed[owner][tntnumber] = true;

        EmitSoundToAll(g_primed_sound, entity, _, _, _, 0.8);
        CreateTimer(1.0, Fuse, ref, TIMER_REPEAT|TIMER_FLAG_NO_MAPCHANGE);

        if (IsClientInGame(owner))
            PrintToChat(owner, "[SM] TNT primed");

        new Float:delay = g_tntDetDelay[owner];
        if (g_tntMode[owner] >= 2 && delay > 0.0)
            CreateTimer(delay, DelayedDetonation, ref, TIMER_FLAG_NO_MAPCHANGE);
    }
        
    return Plugin_Handled;
}

public Action:DelayedDetonation(Handle:timer, any:ref)
{
    new entity = EntRefToEntIndex(ref);
    if (entity > 0 && IsValidEntity(entity))
        AcceptEntityInput(entity, "break");
}

public Action:Fuse(Handle:timer, any:ref)
{
    new entity = EntRefToEntIndex(ref);
    if (entity > 0 && IsValidEntity(entity))
    {
        new Float:tnt_pos[3];
        GetEntPropVector(entity, Prop_Send, "m_vecOrigin", tnt_pos);
        TE_SetupSparks(tnt_pos, NULL_VECTOR, 2, 1);
        TE_SendToAll(0.1);
        return Plugin_Continue;
    }
    else
        return Plugin_Stop;
}

public TakeDamage(const String:output[], caller, activator, Float:delay)
{   
    for (new client = 1; client <= MaxClients; client++)
    {
        for (new i = g_tntAllowed[client]; i > 0 ; i--)
        {
            if (caller == EntRefToEntIndex(g_tntEntity[client][i]))
            {
                if (activator <= 0 || activator >= MaxClients ||
                    GetClientTeam(client) != GetClientTeam(activator) ||
                    !GetConVarBool(g_Cvar_FriendlyFire))
                {
                    AcceptEntityInput(caller,"break");
                }
                break;
            }
        }
    }
}

public Break(const String:output[], caller, activator, Float:delay)
{
    new owner = 0;
    new bool:primed = false;
    for (new client = 1; client <= MaxClients; client++)
    {
        for (new i = g_tntAllowed[client]; i > 0 ; i--)
        {
            if (caller == EntRefToEntIndex(g_tntEntity[client][i]))
            {
                owner = client;
                primed = g_tntPrimed[owner][i];
                g_tntEntity[client][i] = 0;
                break;
            }
        }
    }

    if (!primed || owner == 0)
        return;
    
    decl Float:tnt_pos[3];
    GetEntPropVector(caller, Prop_Send, "m_vecOrigin", tnt_pos);

    TE_SetupExplosion(tnt_pos, g_Explosion, 10.0, 1, 0, 600, 5000);
    TE_SendToAll();

    new Float:radius = GetConVarFloat(g_Cvar_Radius);
    if (radius > 0.0) // && !GetConVarBool(g_Cvar_FriendlyFire))
    {
        new team = IsClientInGame(owner) ? GetClientTeam(owner) : 0;
        for (new target = 1; target <= MaxClients; target++)
        {
            if (IsClientInGame(target))
            {
                if (IsPlayerAlive(target))
                {
                    if (target == owner || GetClientTeam(target) != team)
                    {
                        new Float:targetVector[3];
                        GetClientEyePosition(target, targetVector);
                                                        
                        new Float:distance = GetVectorDistance(targetVector, tnt_pos)
                        if (distance <= radius && TraceTarget(caller, target, tnt_pos, targetVector))
                        {
                            PushAway(caller, target);

                            if (GetEntProp(target, Prop_Data, "m_takedamage", 1))
                            {
                                if (GameType == tf2)
                                {
                                    if (!TF2_IsPlayerUbercharged(target))
                                    {
                                        new Action:res = Plugin_Continue;
                                        Call_StartForward(fwdOnTNTBombed);
                                        Call_PushCell(caller);
                                        Call_PushCell(owner);
                                        Call_PushCell(target);
                                        Call_Finish(res);

                                        if (res == Plugin_Continue)
                                        {
                                            LogAction(owner, target, "\"%L\" bombed \"%L\"", owner, target);
                                            if (owner != target && IsClientInGame(owner))
                                            {
                                                TF2_IgnitePlayer(target, owner);
                                                CreateTimer(0.1, Explode, target, TIMER_FLAG_NO_MAPCHANGE);
                                            }
                                            else
                                                FakeClientCommand(target, "explode");
                                        }
                                    }
                                }
                                else
                                {
                                    new Action:res = Plugin_Continue;
                                    Call_StartForward(fwdOnTNTBombed);
                                    Call_PushCell(caller);
                                    Call_PushCell(owner);
                                    Call_PushCell(target);
                                    Call_Finish(res);

                                    if (res == Plugin_Continue)
                                    {
                                        LogAction(owner, target, "\"%L\" bombed \"%L\"", owner, target);
                                        ForcePlayerSuicide(target);
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }

        if (GetGameType() == tf2)
        {
            decl Float:pos[3];
            new maxents = GetMaxEntities();
            new damage = GetConVarInt(g_Cvar_Damage);
            for (new ent = MaxClients; ent < maxents; ent++)
            {
                if (IsValidEdict(ent) && IsValidEntity(ent))
                {
                    if (TF2_GetExtObjectTypeFromClass(ent) != TFExtObject_Unknown)
                    {
                        if (GetEntProp(ent, Prop_Send, "m_iTeamNum") != team)
                        {
                            GetEntPropVector(ent, Prop_Send, "m_vecOrigin", pos);
                            pos[2] += 10.0; // Adjust trace position

                            new Float:distance = GetVectorDistance(pos, tnt_pos);
                            if (distance <= radius && TraceTarget(caller, ent, tnt_pos, pos))
                            {
                                SetVariantInt(damage);
                                AcceptEntityInput(ent, "RemoveHealth", owner, owner);
                            }
                        }
                    }
                }
            }
        }
    }
    AcceptEntityInput(caller,"kill");
}

stock bool:TraceTarget(client, target, Float:clientLoc[3], Float:targetLoc[3])
{
    TR_TraceRayFilter(clientLoc, targetLoc, MASK_SOLID,
                      RayType_EndPoint, TraceRayDontHitSelf,
                      client);
    return (TR_GetEntityIndex() == target);
}

public bool:TraceRayDontHitSelf(entity, mask, any:data)
{
    return (entity != data); // Check if the TraceRay hit the owning entity.
}

public Action:Explode(Handle:timer, any:target)
{
    if (IsClientInGame(target) && IsPlayerAlive(target))
        FakeClientCommand(target, "explode");
}

public Action:defuse(client, args)
{
    if (g_NativeControl || GetConVarInt(g_Cvar_Enable))
    {
        new owner = 0;
        new tntpack = FindTNT(client, owner);
        if (tntpack < 0 || owner == 0)
            return Plugin_Handled;
        else
            DefuseTNT(client, owner, tntpack);
    }
    return Plugin_Handled;
}

DefuseTNT(client, owner, tntpack)
{
    new entity = EntRefToEntIndex(g_tntEntity[owner][tntpack]);
    if (entity > 0)
    {
        decl Float:tnt_pos[3];
        GetEntPropVector(entity, Prop_Send, "m_vecOrigin", tnt_pos);

        decl Float:targetVector[3];
        GetClientAbsOrigin(client, targetVector);

        if (GetVectorDistance(targetVector, tnt_pos) > 200)
            PrintToChat(client, "[SM] Too far away to defuse");
        else
        {
            new allowed = g_tntAllowed[owner];
            new amount = g_tntAmount[owner]++;
            if (amount > allowed)
                g_tntAmount[owner] = allowed;

            EmitSoundToAll(g_defused_sound, entity, _, _, _, 0.8);

            PrintToChat(owner, "[SM] TNT pack defused");
            PrintToChat(owner, "[SM] TNT left: %i", g_tntAmount[owner]);

            CreateTimer(0.1, RemoveTNT, entity, TIMER_FLAG_NO_MAPCHANGE);
            g_tntEntity[owner][tntpack] = 0;
        }
    }
    else
    {
        PrintToChat(client, "[SM] TNT pack invalid");
        LogError("%N attempted to defuse an invalid TNT pack %d-%d",
                 client, owner, tntpack);
    }
}

public Action:detonate(client, args)
{
    if (g_NativeControl || GetConVarInt(g_Cvar_Enable))
    {
        if (GameType == tf2)
        {
            if (TF2_GetPlayerClass(client) == TFClass_Spy)
            {
                new pcond = TF2_GetPlayerConditionFlags(client);
                if (TF2_IsCloaked(pcond) || TF2_IsDeadRingered(pcond))
                    return Plugin_Handled;
                else if (TF2_IsDisguised(pcond))
                    TF2_RemovePlayerDisguise(client);
            }
        }

        new mode = g_tntMode[client];
        if (mode == 0)
            DetonateTNT(client, -1);
        else if (mode != 2 || g_tntDetDelay[client] == 0.0)
        {
            new owner = 0;
            new tntpack = FindTNT(client, owner);
            if (owner == 0)
            {
                // Not pointing at a pack.
                if (mode == 3)
                {
                    // Detonate all packs.
                    DetonateTNT(client, -1);
                }
            }
            else if (client != owner)
                PrintToChat(client, "[SM] This is not your TNT pack");
            else
                DetonateTNT(owner, tntpack);
        }
    }
    return Plugin_Handled;
}

public Action:tnt(client, args)
{
    if (g_NativeControl || GetConVarInt(g_Cvar_Enable))
    {
        if (GameType == tf2)
        {
            if (TF2_GetPlayerClass(client) == TFClass_Spy)
            {
                new pcond = TF2_GetPlayerConditionFlags(client);
                if (TF2_IsCloaked(pcond) || TF2_IsDeadRingered(pcond))
                    return Plugin_Handled;
                else if (TF2_IsDisguised(pcond))
                    TF2_RemovePlayerDisguise(client);
            }
        }

        new owner = 0;
        new tntpack = FindTNT(client, owner);
        if (owner != 0 && GetClientTeam(owner) != GetClientTeam(client))
            DefuseTNT(client, owner, tntpack);
        else
        {
            if (owner == 0)
                owner = client;

            new mode = g_tntMode[owner];
            if (mode == 0)
                DetonateTNT(owner, -1);
            else if (mode != 2 || g_tntDetDelay[client] == 0.0)
                DetonateTNT(owner, tntpack);
        }
    }
    return Plugin_Handled;
}

DetonateTNT(owner, tntpack)
{
    if (tntpack <= 0)
    {
        for (new i = g_tntAllowed[owner]; i > 0 ; i--)
        {
            new ref = g_tntEntity[owner][i];
            if (ref)
            {
                new ent = EntRefToEntIndex(ref);
                if (ent > 0 && IsValidEntity(ent))
                    AcceptEntityInput(ent, "break");
            }
        }
    }
    else if (g_tntPrimed[owner][tntpack])
    {
        new ref = g_tntEntity[owner][tntpack];
        if (ref)
        {
            new ent = EntRefToEntIndex(ref);
            if (ent > 0 && IsValidEntity(ent))
                AcceptEntityInput(ent, "break");
        }
    }
}

FindTNT(client, &owner)
{
    new tntpack = -1;
    new aim_entity = GetClientAimTarget(client, false);
    for (new target = 1; target <= MaxClients; target++)
    {
        for (new i = g_tntAllowed[target]; i > 0 ; i--)
        {
            if (aim_entity == EntRefToEntIndex(g_tntEntity[client][i]))
            {
                owner = target;
                tntpack = i;
                break;
            }
        }
    }
    return tntpack;
}

// Greyscale's AntiStick code adapted
PushAway(entity, client)
{
    decl Float:entityloc[3];
    GetEntPropVector(entity, Prop_Send, "m_vecOrigin", entityloc);

    decl Float:clientloc[3];
    GetClientAbsOrigin(client, clientloc);
            
    decl Float:vector[3];
    MakeVectorFromPoints(entityloc, clientloc, vector);
            
    NormalizeVector(vector, vector);
    ScaleVector(vector, 1000.0);
    vector[2]+=200;
            
    TeleportEntity(client, NULL_VECTOR, NULL_VECTOR, vector);
}

// L.Duke's Particle code
AttachParticle(ent, Float:duration, const String:particleType[])
{
    new particle = CreateEntityByName("info_particle_system");
    
    new String:tName[128];
    if (IsValidEdict(particle))
    {
        new Float:pos[3];
        GetEntPropVector(ent, Prop_Send, "m_vecOrigin", pos);
        TeleportEntity(particle, pos, NULL_VECTOR, NULL_VECTOR);
        
        Format(tName, sizeof(tName), "target%i", ent);
        DispatchKeyValue(ent, "targetname", tName);
        
        DispatchKeyValue(particle, "targetname", "tf2particle");
        DispatchKeyValue(particle, "parentname", tName);
        DispatchKeyValue(particle, "effect_name", particleType);
        DispatchSpawn(particle);
        SetVariantString(tName);
        AcceptEntityInput(particle, "SetParent", particle, particle, 0);
        ActivateEntity(particle);
        AcceptEntityInput(particle, "start");
        CreateTimer(duration, DeleteParticles, particle, TIMER_FLAG_NO_MAPCHANGE);
    }
}

public Action:DeleteParticles(Handle:timer, any:particle)
{
    if (IsValidEntity(particle))
    {
        new String:classname[256];
        GetEdictClassname(particle, classname, sizeof(classname));
        if (StrEqual(classname, "info_particle_system", false))
        {
            RemoveEdict(particle);
        }
    }
}

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

public Native_ControlTNT(Handle:plugin, numParams)
{
    g_NativeControl = GetNativeCell(1);
}

public Native_SetTNT(Handle:plugin, numParams)
{
    new client = GetNativeCell(1);
    g_tntMode[client] = GetNativeCell(3);
    g_tntDeath[client] = bool:GetNativeCell(4);
    g_tntDetDelay[client] = Float:GetNativeCell(5);
    g_tntPrimeDelay[client] = Float:GetNativeCell(6);
    g_tntAmount[client] = g_tntAllowed[client] = GetNativeCell(2);
}

public Native_PlantTNT(Handle:plugin, numParams)
{
    plant(GetNativeCell(1), 0);
}

public Native_DefuseTNT(Handle:plugin, numParams)
{
    defuse(GetNativeCell(1), 0);
}

public Native_DetonateTNT(Handle:plugin, numParams)
{
    detonate(GetNativeCell(1), 0);
}

public Native_TNT(Handle:plugin, numParams)
{
    tnt(GetNativeCell(1), 0);
}
