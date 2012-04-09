// vim: set ai et ts=4 sw=4 :
//////////////////////////////////////////////
//
// SourceMod Script
//
// DoD Ammo
//
// Developed by -=|JFH|=- Naris
//
// - Credits to "FeuerSturm" for the DoD Restock Source 
// - Credits to "FeuerSturm" for the DoD DropAmmoPack Source
//
//////////////////////////////////////////////
//
//
// USAGE:
// ======
//
//
// CVARs:
// ------
//
// dod_ammo_restock <1/0>       =   enable/disable players being able to ammo
//
// dod_ammo_restock_perlife <#/0>   =   number of restocks per life a player can use
//                      0 = no limit
//
// dod_ammo_restock_delay <#/0>     =   number of seconds until restock can be used again
//                      0 = no limit
//
// dod_ammo_restock_check <1/0>     =   allow/disallow restock if player has
//                      more than half of the set restock ammo
//
// dod_ammo_announce <1/2/0>        =   set announcement
//                      1 = only on first spawn
//                      2 = every spawn until it is used
//                      0 = no announcements at all
//
// dod_ammo_pack_drop <1/0>     =   allow/disallow players to drop their ammopack on command
//
// dod_ammo_pack_drop_ondeath <1/0> =   enable/disable dropping a ammopack on players' death
//
// dod_ammo_pack_pickuprule <0/1/2> =   set who can pickup dropped ammopacks
//                      0 = everyone
//                      1 = only teammates
//                      2 = only enemies
//
// dod_ammo_pack_lifetime <#>       =   number of seconds a dropped ammopack stays on the map
//
//
//
// CHANGELOG:
// ==========
// 
// - 05 December 2009 - Version 1.0
//   Initial Release
//
// - 06 May 2010 - Version 2.1
//   * converted to use sdkhooks instead of dukehacks
//   * check entity limit before creating any new ammopacks
//   * validate all ammopacks using entrefs
//
//
#include <sourcemod>
#include <sdktools>
#include <sdkhooks>

#define PLUGIN_VERSION "2.1"

#define MAXENTITIES 2048

public Plugin:myinfo = 
{
    name = "DoD Ammo",
    author = "-=|JFH|=-Naris,FeuerSturm",
    description = "Players drop ammopacks and can restock the current weapon!",
    version = PLUGIN_VERSION,
    url = "http://www.jigglysfunhouse.net"
}

#define MAXWEAPONS 24

new const String:g_TakenSound[]       = "object/object_taken.wav";
new const String:g_DeniedSound[]      = "common/weapon_denyselect.wav";
new const String:g_NeedAmmoSound[4][] = { "", "", "player/american/us_needammo2.wav", "player/german/ger_needammo2.wav" };
new const String:g_AmmoPackModel[]    = { "", "", "models/ammo/ammo_us.mdl", "models/ammo/ammo_axis.mdl" };

new const String:g_Weapon[MAXWEAPONS][] =
{
    "weapon_amerknife", "weapon_spade", "weapon_colt", "weapon_p38", "weapon_m1carbine", "weapon_c96",
    "weapon_garand", "weapon_k98", "weapon_thompson", "weapon_mp40", "weapon_bar", "weapon_mp44",
    "weapon_spring", "weapon_k98_scoped", "weapon_30cal", "weapon_mg42", "weapon_bazooka", "weapon_pschreck",
    "weapon_riflegren_us", "weapon_riflegren_ger", "weapon_frag_us", "weapon_frag_ger", "weapon_smoke_us", "weapon_smoke_ger"
}
 
new const g_AmmoOffs[MAXWEAPONS] =
{
    0, 0, 4, 8, 24, 12, 16, 20, 32, 32, 36, 32, 28, 20, 40, 44, 48, 48, 84, 88, 52, 56, 68, 72
}

new const g_AmmoAmmo[MAXWEAPONS] =
{
    0, 0, 14, 16, 30, 40, 80, 60, 180, 180, 240, 180, 50, 60, 300, 250, 4, 4, 2, 2, 2, 2, 0, 0
}

new g_iAmmo;

new Float:g_LastAmmo[MAXPLAYERS+1]
new bool:g_UsedAmmo[MAXPLAYERS+1]
new g_RestockCount[MAXPLAYERS+1]

new Handle:RestockEnabled = INVALID_HANDLE
new Handle:RestockCount = INVALID_HANDLE
new Handle:RestockDelay = INVALID_HANDLE
new Handle:RestockCheck = INVALID_HANDLE

new Handle:AmmoPackLifetime = INVALID_HANDLE;
new Handle:AmmoPackOnDeath = INVALID_HANDLE;
new Handle:AmmoPackDrop = INVALID_HANDLE;
new Handle:AmmoPackRule = INVALID_HANDLE;

new Handle:AmmoAnnounce = INVALID_HANDLE
new g_HasAmmoPack[MAXPLAYERS+1];

new g_AmmoPackRef[MAXENTITIES+1];
new g_AmmoPackOwner[MAXENTITIES+1];
new Handle:AmmoPackTimer[MAXENTITIES+1] = INVALID_HANDLE;

new bool:g_NativeControl = false;
new g_NativeAmmoPack[MAXPLAYERS+1];
new g_NativeAmmoPackRule[MAXPLAYERS+1];
new g_NativeAmmoPackCount[MAXPLAYERS+1];

public APLRes:AskPluginLoad2(Handle:myself, bool:late, String:error[], err_max)
{
    CreateNative("ControlDodAmmo", Native_ControlDodAmmo);
    CreateNative("SetDodAmmo", Native_SetDodAmmo);
    CreateNative("Restock", Native_Restock);
    RegPluginLibrary("dod_ammo");
    return APLRes_Success;
}

public OnPluginStart()
{
    CreateConVar("dod_ammo_version", PLUGIN_VERSION, "DoD Ammo Version (DO NOT CHANGE!)", FCVAR_DONTRECORD|FCVAR_PLUGIN|FCVAR_SPONLY|FCVAR_REPLICATED|FCVAR_NOTIFY)

    AmmoPackDrop = CreateConVar("dod_ammo_pack_drop", "1", "<1/0> = allow/disallow players to drop their ammopack on command", FCVAR_PLUGIN, true, 0.0, true, 1.0);
    AmmoPackOnDeath = CreateConVar("dod_ammo_pack_ondeath", "1", "<1/0> = enable/disable dropping a ammopack on players' death", FCVAR_PLUGIN, true, 0.0, true, 1.0);
    AmmoPackLifetime = CreateConVar("dod_ammopack_lifetime", "30", "<#> = number of seconds a dropped ammopack stays on the map", FCVAR_PLUGIN, true, 5.0, true, 60.0);
    AmmoPackRule = CreateConVar("dod_ammopack_pickuprule", "0", "<0/1/2> = set who can pickup dropped ammopacks: 0 = everyone, 1 = only teammates, 2 = only enemies", FCVAR_PLUGIN, true, 0.0, true, 2.0);

    RestockEnabled = CreateConVar("dod_ammo_restock", "1", "<1/0> = enable/disable players being able to restock", FCVAR_PLUGIN)
    RestockCount = CreateConVar("dod_ammo_restock_perlife", "3", "<#/0> = number of restocks per life a player can use  -  0=no limit", FCVAR_PLUGIN)
    RestockDelay = CreateConVar("dod_ammo_restock_delay", "30", "<#/0> = number of seconds after ammoing can be used again  -  0=no limit", FCVAR_PLUGIN)
    AmmoAnnounce = CreateConVar("dod_ammo_announce", "1", "<1/2/0> = set announcement  -  1=only on first spawn  -  2=every spawn until it is used  -  0=no announcements", FCVAR_PLUGIN)
    RestockCheck = CreateConVar("dod_ammo_restock_check", "1", "<1/0> = enable/disable disallowing to ammo if player has more than half of the set ammo ammo", FCVAR_PLUGIN)

    decl String:ConVarName[256]
    decl String:ConVarValue[256]
    decl String:ConVarDescription[256]
    for(new i = 2; i < MAXWEAPONS; i++)
    {
        Format(ConVarName, sizeof(ConVarName), "dod_ammo_%s",g_Weapon[i])
        IntToString(g_AmmoAmmo[i], ConVarValue, sizeof(ConVarValue))
        Format(ConVarDescription, sizeof(ConVarDescription), "<#> set amount of Ammo to ammo for %s", g_Weapon[i])
        CreateConVar(ConVarName, ConVarValue, ConVarDescription,FCVAR_PLUGIN)
    }

    RegConsoleCmd("say", SayAmmo)
    RegConsoleCmd("say_team", SayAmmo)
    HookEventEx("player_death", OnPlayerDeath, EventHookMode_Post)
    HookEventEx("player_spawn", OnPlayerSpawn, EventHookMode_Post)
    AutoExecConfig(true, "dod_ammo", "dod_ammo")
    LoadTranslations("dod_restock_source.txt")
}

public OnMapStart()
{
    g_iAmmo = FindSendPropOffs("CDODPlayer", "m_iAmmo")

    PrecacheSound(g_TakenSound)
    PrecacheSound(g_DeniedSound)
    PrecacheSound(g_NeedAmmoSound[2])
    PrecacheSound(g_NeedAmmoSound[3])

    PrecacheModel(g_AmmoPackModel[2],true);
    PrecacheModel(g_AmmoPackModel[3],true);
}

public OnClientPostAdminCheck(client)
{
    g_LastAmmo[client] = 0.0
    g_RestockCount[client] = 0
    g_UsedAmmo[client] = false
}

public OnClientDisconnect(client)
{
    g_LastAmmo[client] = 0.0
    g_RestockCount[client] = 0
    g_UsedAmmo[client] = false
}

public Action:OnPlayerSpawn(Handle:event, const String:name[], bool:dontBroadcast)
{
    new client = GetClientOfUserId(GetEventInt(event, "userid"))
    if (IsClientInGame(client) && IsPlayerAlive(client) && GetClientTeam(client) > 1)
    {
        g_HasAmmoPack[client] = g_NativeControl
                                ? (g_NativeAmmoPack[client] != 0)
                                  ? g_NativeAmmoPackCount[client] : 0
                                : 1;

        new announce = GetConVarInt(AmmoAnnounce)
        if (announce != 0 && !g_UsedAmmo[client] && GetConVarInt(RestockEnabled) == 1)
        {
            decl String:ammo[32];
            Format(ammo, sizeof(ammo), "\x04!ammo\x01");
            PrintToChat(client, "\x04[DoD Ammo] \x01%T", "AnnounceRestock", client, ammo);
            if (announce == 1)
                g_UsedAmmo[client] = true;
        }
    }
    return Plugin_Continue
}

public Action:OnPlayerDeath(Handle:event, const String:name[], bool:dontBroadcast)
{
    new client = GetClientOfUserId(GetEventInt(event, "userid"));
    g_LastAmmo[client] = 0.0;
    g_RestockCount[client] = 0;

    if (GetClientHealth(client) > 0 || !IsClientInGame(client) ||
        g_HasAmmoPack[client] <= 0)
    {
        return Plugin_Continue;
    }
    else if (g_NativeControl)
    {
        if (g_NativeAmmoPack[client] % 2 == 1)
            return Plugin_Continue;
    }
    else if (!GetConVarBool(AmmoPackOnDeath))
    {
        return Plugin_Continue;
    }
    else if (IsEntLimitReached(.client=client, .message="unable to create ammopack"))
    {
        return Plugin_Continue;
    }

    new Float:deathorigin[3];
    GetClientAbsOrigin(client,deathorigin);
    deathorigin[2] += 5.0;

    CreateAmmoPack(client, deathorigin, NULL_VECTOR, NULL_VECTOR);
    return Plugin_Continue
}

public Action:SayAmmo(client, args)
{
    decl String:AmmoCmd[9]
    GetCmdArg(1, AmmoCmd, sizeof(AmmoCmd))
    if(strcmp(AmmoCmd, "!ammo", false) == 0)
    {
        if (g_NativeControl || GetConVarInt(RestockEnabled) == 0)
        {
            PrintToChat(client, "\x04[DoD Ammo] \x01%T", "PluginDisabled", client)
            return Plugin_Handled
        }
        else
        {
            if (!g_UsedAmmo[client])
            {
                g_UsedAmmo[client] = true
            }
            cmdAmmo(client, args)
            return Plugin_Handled
        }
    }
    return Plugin_Continue
}

public Action:cmdAmmo(client, args)
{
    if(IsClientInGame(client) && IsPlayerAlive(client))
    {
        decl String:Weapon[32]
        GetClientWeapon(client, Weapon, sizeof(Weapon))
        new WeaponID = -1
        for(new i = 0; i < MAXWEAPONS; i++)
        {
            if(strcmp(Weapon,g_Weapon[i]) == 0)
            {
                WeaponID = i
            }
        }
        if(WeaponID != -1)
        {
            ReplaceString(Weapon, sizeof(Weapon), "weapon_", "")
            if(WeaponID == 0 || WeaponID == 1)
            {
                PrintToChat(client, "\x04[DoD Ammo] \x01%T", "NoMeleeRestock", client, Weapon)
                EmitSoundToClient(client, g_DeniedSound, .channel=SNDCHAN_WEAPON)
                return Plugin_Handled
            }
            else
            {
                decl String:AmmoConVar[256]
                Format(AmmoConVar, sizeof(AmmoConVar), "dod_ammo_weapon_%s", Weapon) 
                new AmmoAmmo = GetConVarInt(FindConVar(AmmoConVar))
                if(AmmoAmmo == 0)
                {
                    PrintToChat(client, "\x04[DoD Ammo] \x01%T", "WeaponDisabled", client, Weapon)
                    EmitSoundToClient(client, g_DeniedSound, .channel=SNDCHAN_WEAPON)
                    return Plugin_Handled
                }               
                new delay = GetConVarInt(RestockDelay)
                new maxammos = GetConVarInt(RestockCount)
                if((GetGameTime() < g_LastAmmo[client] + delay) && g_RestockCount[client] != 0 && g_RestockCount[client] < maxammos && delay != 0)
                {
                    EmitSoundToClient(client, g_DeniedSound, .channel=SNDCHAN_WEAPON)
                    PrintToChat(client, "\x04[DoD Ammo] \x01%T", "RecentlyRestocked", client, RoundToCeil(g_LastAmmo[client] + delay - GetGameTime()))
                    return Plugin_Handled
                }
                if(g_RestockCount[client] >= maxammos && maxammos != 0)
                {
                    EmitSoundToClient(client, g_DeniedSound, .channel=SNDCHAN_WEAPON)
                    PrintToChat(client, "\x04[DoD Ammo] \x01%T", "RestockLimit", client,  g_RestockCount[client])
                    return Plugin_Handled
                }
                new WeaponAmmo = g_iAmmo + g_AmmoOffs[WeaponID]
                if(GetConVarInt(RestockCheck) == 1)
                {
                    new currammo = GetEntData(client, WeaponAmmo)
                    if((StrContains(Weapon, "riflegren") != -1 && currammo > RoundToCeil(float(AmmoAmmo) / 2.0)-1) || (StrContains(Weapon, "riflegren") == -1 && currammo > RoundToCeil(float(AmmoAmmo) / 2.0)))
                    {
                        EmitSoundToClient(client, g_DeniedSound, .channel=SNDCHAN_WEAPON)
                        PrintToChat(client, "\x04[DoD Ammo] \x01%T", "RestockEnoughAmmo", client, Weapon)
                        return Plugin_Handled
                    }
                }
                new team = GetClientTeam(client)
                EmitSoundToClient(client, g_NeedAmmoSound[team], SOUND_FROM_PLAYER,SNDCHAN_VOICE,SNDLEVEL_NORMAL, SND_NOFLAGS, SNDVOL_NORMAL, SNDPITCH_NORMAL)
                if(StrContains(Weapon, "riflegren") != -1)
                {
                    AmmoAmmo--
                }
                SetEntData(client, WeaponAmmo, AmmoAmmo, 4, true)
                EmitSoundToClient(client, g_TakenSound, .channel=SNDCHAN_WEAPON)
                PrintToChat(client, "\x04[DoD Ammo] \x01%T", "RestockSuccess", client, Weapon)
                g_LastAmmo[client] = GetGameTime()
                g_RestockCount[client]++
                return Plugin_Handled
            }
        }
    }
    else
    {
        PrintToChat(client, "\x04[DoD Ammo] \x01%T", "DeadRestock", client)
        return Plugin_Handled
    }
    return Plugin_Handled
}

public Action:cmdAmmopack(client, args) 
{
    if (!IsClientInGame(client) || !IsPlayerAlive(client) ||
        g_HasAmmoPack[client] <= 0)
    {
        return Plugin_Continue;
    }
    else if (g_NativeControl)
    {
        if (g_NativeAmmoPack[client] < 2)
            return Plugin_Continue;
    }
    else if (!GetConVarBool(AmmoPackDrop))
    {
        return Plugin_Continue;
    }
    else if (IsEntLimitReached(.client=client, .message="unable to create ammopack"))
    {
        return Plugin_Continue;
    }

    new Float:origin[3];
    GetClientAbsOrigin(client, origin);
    origin[2] += 55.0;

    new Float:angles[3];
    GetClientEyeAngles(client, angles);

    new Float:velocity[3];
    GetAngleVectors(angles, velocity, NULL_VECTOR, NULL_VECTOR);
    NormalizeVector(velocity,velocity);
    ScaleVector(velocity,350.0);

    CreateAmmoPack(client, origin, angles, velocity);
    return Plugin_Handled;
}

CreateAmmoPack(client, const Float:origin[3],
               const Float:angles[3]=NULL_VECTOR,
               const Float:velocity[3]=NULL_VECTOR)
{
    new ammopack = CreateEntityByName("prop_physics_override");
    if (ammopack > 0 && IsValidEntity(ammopack))
    {
        new team = GetClientTeam(client);
        SetEntityModel(ammopack,g_AmmoPackModel[team]);
        DispatchSpawn(ammopack);
        TeleportEntity(ammopack, origin, angles, velocity);

        g_HasAmmoPack[client]--;
        g_AmmoPackOwner[ammopack] = client;
        g_AmmoPackRef[ammopack] = EntIndexToEntRef(ammopack);
        SDKHook(ammopack, SDKHook_Touch, OnAmmoPackTouched);
        AmmoPackTimer[ammopack] = CreateTimer(GetConVarFloat(AmmoPackLifetime),
                                              RemoveDroppedAmmoPack, ammopack,
                                              TIMER_FLAG_NO_MAPCHANGE);
    }
}

public Action:OnAmmoPackTouched(ammopack, client)
{
    if (client > 0 && client <= GetMaxClients() && ammopack > 0 &&
        EntRefToEntIndex(g_AmmoPackRef[ammopack]) == ammopack &&
        IsClientInGame(client) && IsPlayerAlive(client) &&
        IsValidEdict(ammopack))
    {
        if (g_AmmoPackOwner[ammopack] == client)
        {
            if (g_HasAmmoPack[client] <= 0)
            {
                KillAmmoPackTimer(ammopack);
                EmitSoundToClient(client, g_TakenSound, .channel=SNDCHAN_WEAPON);
                RemoveEdict(ammopack);
                g_HasAmmoPack[client]++;
                g_AmmoPackRef[ammopack] = 0;
                g_AmmoPackOwner[ammopack] = 0;
                return Plugin_Handled;
            }
            return Plugin_Handled;
        }

        new pickuprule = g_NativeControl ? (g_NativeAmmoPackRule[client]) : GetConVarInt(AmmoPackRule);
        new clteam = GetClientTeam(client);
        new kitteam = GetClientTeam(g_AmmoPackOwner[ammopack]);
        if ((pickuprule == 1 && kitteam != clteam) || (pickuprule == 2 && kitteam == clteam))
        {
            return Plugin_Handled;
        }

        cmdAmmo(client, 0);

        KillAmmoPackTimer(ammopack);
        EmitSoundToClient(client, g_TakenSound, .channel=SNDCHAN_WEAPON);
        RemoveEdict(ammopack);
        g_AmmoPackRef[ammopack] = 0;
        g_AmmoPackOwner[ammopack] = 0;
    }
    return Plugin_Handled;
}

public Action:RemoveDroppedAmmoPack(Handle:timer, any:ammopack)
{
    AmmoPackTimer[ammopack] = INVALID_HANDLE;
    if (EntRefToEntIndex(g_AmmoPackRef[ammopack]) == ammopack &&
        IsValidEdict(ammopack))
    {
        RemoveEdict(ammopack);
        g_AmmoPackRef[ammopack] = 0;
        g_AmmoPackOwner[ammopack] = 0;
    }
    return Plugin_Handled;
}

KillAmmoPackTimer(ammopack)
{
	new Handle:timer = AmmoPackTimer[ammopack];
	if (timer != INVALID_HANDLE)
	{
		CloseHandle(AmmoPackTimer[ammopack]);
		AmmoPackTimer[ammopack] = INVALID_HANDLE;
	}
}

public Native_ControlDodAmmo(Handle:plugin, numParams)
{
	g_NativeControl = GetNativeCell(1);
}

public Native_SetDodAmmo(Handle:plugin, numParams)
{
	new client = GetNativeCell(1);
	new enable = GetNativeCell(2);
	new count = GetNativeCell(4);

	g_NativeAmmoPack[client] = enable;
	g_NativeAmmoPackRule[client] = GetNativeCell(3);
	g_NativeAmmoPackCount[client] = count;

	g_HasAmmoPack[client] = (enable != 0) ? count : 0;
}

public Native_Restock(Handle:plugin, numParams)
{
    cmdAmmo(GetNativeCell(1), 0);
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

