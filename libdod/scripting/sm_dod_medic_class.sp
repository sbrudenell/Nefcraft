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
// For Day of Defeat Source only
// This plugin is a port of the Medic Class plugin for DoDS
// originally created by me in EventScripts
// Additional testing and coding by Lebson
//
//
// CHANGELOG:
// See http://forums.alliedmods.net/showthread.php?t=73997

#include <sourcemod>
#include <sdktools>

#define PLUGIN_VERSION "2.0.111"

enum MedicState { NotMedic=0, IsMedic, NotMedicPending, IsMedicPending }

new const String:g_classlist[][] = { "",
    "Rifleman",
    "Assault",
    "Support",
    "Sniper",
    "MG",
    "Rocket" };

new const String:g_model[][] = { "", "",
    "models/player/american_medic.mdl",
    "models/player/german_medic.mdl" };

new const String:g_model_files[][]  = {
    "models/player/american_medic.dx80.vtx",
    "models/player/american_medic.dx90.vtx",
    "models/player/american_medic.mdl",
    "models/player/american_medic.phy",
    "models/player/american_medic.sw.vtx",
    "models/player/american_medic.vvd",
    "models/player/german_medic.dx80.vtx",
    "models/player/german_medic.dx90.vtx",
    "models/player/german_medic.mdl",
    "models/player/german_medic.phy",
    "models/player/german_medic.sw.vtx",
    "models/player/german_medic.vvd",
    "materials/models/player/american/allis_mc_body.vmt",
    "materials/models/player/american/allis_mc_body.vtf",
    "materials/models/player/german/axs_mc_body.vmt",
    "materials/models/player/german/axs_mc_body.vtf" };


new Handle:g_Cvar_MedicEnable;
new Handle:g_Cvar_MedicWeapon;
new Handle:g_Cvar_MedicNades;
new Handle:g_Cvar_MedicNadeAmmo;
new Handle:g_Cvar_MedicAmmo;
new Handle:g_Cvar_MedicSpeed;
new Handle:g_Cvar_MedicWeight;
new Handle:g_Cvar_MedicHealth;
new Handle:g_Cvar_MedicMaxHeal;
new Handle:g_Cvar_MedicMax;
new Handle:g_Cvar_MedicPacks;
new Handle:g_Cvar_MedicMessages;
new Handle:g_Cvar_MedicRestrict;
new Handle:g_Cvar_MedicPickup;
new Handle:g_Cvar_MedicKeepWeapon;
new Handle:g_Cvar_MedicMinPlayers;
new Handle:g_Cvar_MedicMaxHealth;
new Handle:g_Cvar_MedicSelf;

new Handle:g_OnMedicHealedHandle = INVALID_HANDLE;
new Handle:g_PistolPluginHandle = INVALID_HANDLE;

new Handle:g_WeaponTimer[MAXPLAYERS+1];

new g_medic_master[4][17];

new MedicState:g_IsMedic[MAXPLAYERS+1];
new bool:g_MedicKeepWeapon[MAXPLAYERS+1];
new bool:g_MedicPickup[MAXPLAYERS+1];
new Float:g_MedicWeight[MAXPLAYERS+1];
new Float:g_MedicSpeed[MAXPLAYERS+1];
new g_MedicPacksLeft[MAXPLAYERS+1];
new g_NumMedicPacks[MAXPLAYERS+1];
new g_MedicNadeAmmo[MAXPLAYERS+1];
new g_MaxMedicHeal[MAXPLAYERS+1];
new g_MedicHealth[MAXPLAYERS+1];
new g_MedicWeapon[MAXPLAYERS+1];
new g_MedicNades[MAXPLAYERS+1];
new g_MedicAmmo[MAXPLAYERS+1];

new bool:swap[MAXPLAYERS+1];
new bool:yell[MAXPLAYERS+1];

new g_MaxMedics;
new g_MinPlayers;
new g_SelfHealth;
new g_MedicRestrict;
new g_MaxMedicHealth;
new bool:g_MedicEnabled;
new bool:g_MedicMessages;
new bool:g_NativeOverride = false;

new ammo_offset;

public Plugin:myinfo = 
{
    name = "Medic Class for DoDS",
    author = "<eVa>Dog",
    description = "Medic Class plugin for Day of Defeat Source",
    version = PLUGIN_VERSION,
    url = "http://www.theville.org"
}

public APLRes:AskPluginLoad2(Handle:myself, bool:late, String:error[], err_max)
{
    // Register Natives
    CreateNative("ControlMedic",Native_ControlMedic);
    CreateNative("AssignMedic",Native_AssignMedic);
    CreateNative("UnassignMedic",Native_UnassignMedic);
    CreateNative("GetMedicSpeed",Native_GetMedicSpeed);
    CreateNative("GetMedicWeight",Native_GetMedicWeight);
    CreateNative("MedicHeal",Native_MedicHeal);

    g_OnMedicHealedHandle=CreateGlobalForward("OnMedicHealed",ET_Ignore,Param_Cell,Param_Cell,Param_Cell);

    RegPluginLibrary("medic_class");
    return APLRes_Success;
}

public OnPluginStart()
{
    CreateConVar("sm_dod_medic_version", PLUGIN_VERSION, "Medic Class for DoDS version", FCVAR_PLUGIN|FCVAR_SPONLY|FCVAR_REPLICATED|FCVAR_NOTIFY);
    g_Cvar_MedicWeapon     = CreateConVar("sm_dod_medic_weapon", "0", "The secondary weapon to give to the Medic <0=Pistols,1=Carbine/C96>", FCVAR_PLUGIN);
    g_Cvar_MedicAmmo       = CreateConVar("sm_dod_medic_ammo", "14", "The amount of ammo to give to the Medic", FCVAR_PLUGIN);
    g_Cvar_MedicNades      = CreateConVar("sm_dod_medic_nades", "2", "Enable nades for the Medic <0=Disable,1=Smoke,2=Nades>", FCVAR_PLUGIN);
    g_Cvar_MedicNadeAmmo   = CreateConVar("sm_dod_medic_nades_ammo", "2", "The amount of nades to give to the Medic", FCVAR_PLUGIN);
    g_Cvar_MedicSpeed      = CreateConVar("sm_dod_medic_speed", "1.1", "Sets the speed of the medic", FCVAR_PLUGIN);
    g_Cvar_MedicWeight     = CreateConVar("sm_dod_medic_weight", "0.9", "Sets the weight (gravity) of the medic", FCVAR_PLUGIN);
    g_Cvar_MedicHealth     = CreateConVar("sm_dod_medic_health", "80", "Sets the HP of the medic", FCVAR_PLUGIN);
    g_Cvar_MedicMaxHeal    = CreateConVar("sm_dod_medic_maxhealing", "50", "Maximum amount of health to heal", FCVAR_PLUGIN);
    g_Cvar_MedicMaxHealth  = CreateConVar("sm_dod_medic_maxhealth", "100", "Maximum health a player be healed to (100 = full health)", FCVAR_PLUGIN);
    g_Cvar_MedicMax        = CreateConVar("sm_dod_medic_max", "2", "Maximum number of Medics per team (0 = unlimited)", FCVAR_PLUGIN);
    g_Cvar_MedicPacks      = CreateConVar("sm_dod_medic_packs", "20", "Number of Medic Packs the Medic carries", FCVAR_PLUGIN);
    g_Cvar_MedicMessages   = CreateConVar("sm_dod_medic_messages", "1", "Message the Medic/Patient on events", FCVAR_PLUGIN);
    g_Cvar_MedicRestrict   = CreateConVar("sm_dod_medic_restrict", "0", "Class to restrict Medic to (see forum thread)", FCVAR_PLUGIN);
    g_Cvar_MedicPickup     = CreateConVar("sm_dod_medic_useweapons", "0", "Allow Medics to pickup and use dropped weapons", FCVAR_PLUGIN);
    g_Cvar_MedicKeepWeapon = CreateConVar("sm_dod_medic_keepweapons", "0", "Allow Medics to keep thier original classes' weapons", FCVAR_PLUGIN);
    g_Cvar_MedicMinPlayers = CreateConVar("sm_dod_medic_minplayers", "0", "Minimum number of players before Medic class available", FCVAR_PLUGIN);
    g_Cvar_MedicSelf       = CreateConVar("sm_dod_medic_minhealth", "20", "Minimum hp before a player can self heal", FCVAR_PLUGIN);
    g_Cvar_MedicEnable     = CreateConVar("sm_dod_medic_enable", "1", "Enables/Disables Medic Class", FCVAR_PLUGIN);
    
    RegConsoleCmd("sm_class_medic", beMedic, " - Change class to a Medic");
    RegConsoleCmd("sm_heal", Heal, " - Heal a player");
    RegConsoleCmd("sm_medic", Yell, " - Call for a medic");
    RegConsoleCmd("voice_medic", Call, " -  Voice Call for a medic");
    RegConsoleCmd("sm_medic_who", Who, " - Display the Medics on your team");

    HookEvent("player_spawn", PlayerSpawnEvent);
    HookEvent("player_changeclass", PlayerChangeClassEvent);
    HookEvent("player_team", PlayerChangeTeamEvent); //added by psychocoder 
}

public OnConfigsExecuted()
{
    g_MedicEnabled = GetConVarBool(g_Cvar_MedicEnable);
    g_MedicMessages = GetConVarBool(g_Cvar_MedicMessages);
    g_MaxMedicHealth = GetConVarInt(g_Cvar_MedicMaxHealth);
    g_MedicRestrict = GetConVarInt(g_Cvar_MedicRestrict);
    g_SelfHealth = GetConVarInt(g_Cvar_MedicSelf);
    g_MinPlayers = GetConVarInt(g_Cvar_MedicMinPlayers);
    g_MaxMedics = GetConVarInt(g_Cvar_MedicMax);
}

public OnMapStart()
{
    // Added custom models to download table
    for (new i = 0; i < sizeof(g_model_files); i++)
        AddFileToDownloadsTable(g_model_files[i]);
    
    AddFileToDownloadsTable("sound/bandage/bandage.mp3")
    
    PrecacheModel(g_model[2], true)
    PrecacheModel(g_model[3], true)
    
    PrecacheSound("bandage/bandage.mp3", true)
    PrecacheSound("common/weapon_denyselect.wav", true)
    PrecacheSound("common/weapon_select.wav", true)
    
    for (new i = 1; i < sizeof(g_medic_master[]); i++)
    {
        g_medic_master[2][i] = 0
        g_medic_master[3][i] = 0
    }
    
    for (new i = 0; i < sizeof(g_IsMedic); i++)
    {
        g_IsMedic[i] = NotMedic
        swap[i]=false; 
        yell[i]=false; 
    }
    
    g_PistolPluginHandle = FindConVar("sm_dod_pistols_version")
    
    ammo_offset = FindSendPropOffs("CDODPlayer", "m_iAmmo")
}

public bool:OnClientConnect(client, String:rejectmsg[], maxlen)
{
    g_MedicPickup[client] = GetConVarBool(g_Cvar_MedicPickup);
    g_MedicKeepWeapon[client] = GetConVarBool(g_Cvar_MedicKeepWeapon);
    g_NumMedicPacks[client] = GetConVarInt(g_Cvar_MedicPacks);
    g_MedicNadeAmmo[client] = GetConVarInt(g_Cvar_MedicNadeAmmo);
    g_MaxMedicHeal[client] = GetConVarInt(g_Cvar_MedicMaxHeal);
    g_MedicWeapon[client] = GetConVarInt(g_Cvar_MedicWeapon);
    g_MedicHealth[client] = GetConVarInt(g_Cvar_MedicHealth);
    g_MedicWeight[client] = GetConVarFloat(g_Cvar_MedicWeight);
    g_MedicSpeed[client] = GetConVarFloat(g_Cvar_MedicSpeed);
    g_MedicNades[client] = GetConVarInt(g_Cvar_MedicNades);
    g_MedicAmmo[client] = GetConVarInt(g_Cvar_MedicAmmo);
    return true;
}

public OnClientDisconnect(client)
{
    g_IsMedic[client] = NotMedic
    swap[client]=false; 
    yell[client]=false; 

    if (g_NativeOverride || g_MedicEnabled)
    {
        if (g_MaxMedics > 0)
        {
            for (new i = 1; i < sizeof(g_medic_master[]); i++)
            {
                for (new team = 2; team <= 3; team++)
                {
                    if (g_medic_master[team][i] == client)
                    {
                        g_medic_master[team][i] = 0
                        break;
                    }
                }
            }
        }
    }
}

public PlayerSpawnEvent(Handle:event, const String:name[], bool:dontBroadcast)
{
    if (g_NativeOverride || g_MedicEnabled)
    {
        new client = GetClientOfUserId(GetEventInt(event, "userid"));
        new team = GetClientTeam(client);
        
        //added by psychocoder
        if (swap[client])
        {
            if (g_NativeOverride)
                AssignMedic(client);
            else
                beMedic(client,0);

            swap[client] = false; 
        }
        //end add 
        
        if (g_IsMedic[client] == NotMedicPending)
        {
            if (g_MaxMedics > 0)
            {
                new otherTeam = (team == 2) ? 3 : 2;
                for (new i = 1; i <= g_MaxMedics; i++)
                {
                    if (g_medic_master[team][i] == client)
                        g_medic_master[team][i] = 0;
                
                    //added by psychocoder, shows if medic client is in the wrong team
                    if (g_medic_master[otherTeam][i] == client) 
                        g_medic_master[otherTeam][i] = 0;
                    //end add 
                }
            }
            g_IsMedic[client] = NotMedic;
        }
        else if (g_IsMedic[client] == IsMedicPending)
            g_IsMedic[client] = IsMedic;
            
        if (g_IsMedic[client] == IsMedic)
        {
            SetEntityGravity(client, g_MedicWeight[client]);
            SetEntityHealth(client, g_MedicHealth[client]);
            SetEntityModel(client, g_model[team]);
            g_MedicPacksLeft[client] = g_NumMedicPacks[client]; 
          
            // For some reason, setting m_flLaggedMovementValue when used with SourceCraft
            // causes weird teleportation to nowhere after spawn?
            if (!g_NativeOverride)
                SetEntPropFloat(client, Prop_Send, "m_flLaggedMovementValue", g_MedicSpeed[client]);

            if (!g_MedicKeepWeapon[client])
            {
                if (!g_MedicPickup[client] && !g_WeaponTimer[client])
                    g_WeaponTimer[client] = CreateTimer(0.1, WeaponsCheck, client, TIMER_REPEAT);

                if (g_PistolPluginHandle == INVALID_HANDLE)
                    CreateTimer(0.1, GiveClientWeapon, client);
                else
                    CreateTimer(0.2, GiveClientWeapon, client);
            }
        }
        else
        {
            yell[client] = false;
            g_MedicPacksLeft[client] = 0;
        }
    }
}

//added by psychocoder, if a swap plugin swap medic 
public PlayerChangeTeamEvent(Handle:event, const String:name[], bool:dontBroadcast)
{
    if (g_NativeOverride || g_MedicEnabled)
    {
        new client = GetClientOfUserId(GetEventInt(event, "userid"));
        if (g_IsMedic[client] >= IsMedic && g_IsMedic[client] <= NotMedicPending)
        { 
            new team = GetEventInt(event, "team"); 
            new oldteam = GetEventInt(event, "oldteam");
            if (team != oldteam) 
            {
                g_IsMedic[client] = NotMedic 
                if (!GetEventBool(event, "disconnect"))
                    swap[client]=true;

                if (g_MaxMedics > 0)
                {
                    for (new i = 1; i < sizeof(g_medic_master[]); i++)
                    { 
                        if (g_medic_master[oldteam][i] == client) 
                        {
                            g_medic_master[oldteam][i] = 0
                            break;
                        }
                    } 
                } 
            }
        }
    }
}
//end add 

public PlayerChangeClassEvent(Handle:event, const String:name[], bool:dontBroadcast)
{
    new client = GetClientOfUserId(GetEventInt(event, "userid"))
    if (g_NativeOverride)
    {
        if (g_IsMedic[client] == IsMedic ||
            g_IsMedic[client] == IsMedicPending &&
            IsPlayerAlive(client))
        {
            AssignMedic(client);
            return;
        }
    }
    else if (g_MedicEnabled)
    {
        if (IsPlayerAlive(client))
        {
            g_IsMedic[client] = NotMedicPending
            PrintToChat(client, "*You will no longer spawn as Medic")
            return;
        }
        else
        {
            g_IsMedic[client] = NotMedic
            PrintToChat(client, "*You are no longer a Medic")
        }
    }

    if (g_MaxMedics > 0)
    {
        for (new i = 1; i < sizeof(g_medic_master[]); i++)
        {
            for (new team = 2; team <= 3; team++)
            {
                if (g_medic_master[team][i] == client)
                {
                    g_medic_master[team][i] = 0;
                    break;
                }
            }
        }
    }
}

public Action:WeaponsCheck(Handle:timer, any:client)
{
    if (!g_IsMedic[client]      || g_MedicPickup[client] ||
        !IsClientInGame(client) || !IsPlayerAlive(client))
    {
        g_WeaponTimer[client] = INVALID_HANDLE;
        return Plugin_Stop;
    }
    
    new weaponindex = GetPlayerWeaponSlot(client, 0)
    if (weaponindex != -1)
    {   
        RemovePlayerItem(client, weaponindex)
        
        //Added by Lebson
        RemoveEdict(weaponindex);
        if ((weaponindex = GetPlayerWeaponSlot(client, 1)) != -1)
            EquipPlayerWeapon(client, weaponindex);

        PrintToChat(client, "[SM] Medics not permitted to use primary weapons");
    }
    
    return Plugin_Continue;
}

public Action:beMedic(client, args)
{
    if (g_MedicEnabled || g_NativeOverride)
    {
        if (client > 0)
        {
            new currentplayers = GetClientCount(false);
            if (currentplayers >= g_MinPlayers)
            {
                if (g_IsMedic[client] == IsMedic)
                {
                    g_IsMedic[client] = NotMedicPending;
                    PrintToChat(client, "*You will no longer spawn as Medic");
                }
                else
                {
                    //added by psychocoder
                    new team = GetClientTeam(client);
                    if (team != 2 && team != 3)
                        return Plugin_Handled; 

                    //Check player's class, if enabled
                    if (g_MedicRestrict > 0)
                    {
                        new class = GetEntProp(client, Prop_Send, "m_iPlayerClass");
                        if (class+1 != g_MedicRestrict)
                        {
                            PrintToChat(client, "[SM] Medic restricted to %s", g_classlist[g_MedicRestrict]);
                            return Plugin_Handled;
                        }
                    }

                    if (g_MaxMedics > 0)
                    {
                        new slot_available = 0;
                        for (new i = 1; i <= g_MaxMedics; i++)
                        {
                            new occupant = g_medic_master[team][i];
                            if (occupant == client)
                            {
                                PrintToChat(client, "*You will spawn as Medic");
                                return Plugin_Handled;
                            }
                            else if (occupant == 0 && slot_available == 0)
                            {
                                slot_available = i;
                            }
                        }

                        if (slot_available != 0)
                        {
                            //Medic slot available
                            g_medic_master[team][slot_available] = client;
                            g_IsMedic[client] = IsMedicPending;
                            PrintToChat(client, "*You will spawn as Medic");
                        }
                        else
                        {
                            PrintToChat(client, "[SM] Medic class is full");
                        }
                    }
                    else
                    {
                        g_IsMedic[client] = IsMedicPending;
                        PrintToChat(client, "*You will spawn as Medic");
                    }
                }
            }
            else
            {
                PrintToChat(client, "[SM] Not enough players to enable Medic Class");
            }
        }
    }
    return Plugin_Handled;
}

public Action:Yell(client, args)
{
    if (g_NativeOverride || g_MedicEnabled)
    {
        if (!yell[client])
        {
            yell[client] = true;
            ClientCommand(client, "voice_medic");
            
            new currentplayers = GetClientCount(false);
            if (currentplayers < g_MinPlayers)
            {
                if (client > 0 && IsPlayerAlive(client))
                {
                    new health = GetClientHealth(client);
                    if (health < g_SelfHealth)
                    {
                        health += GetRandomInt(10, 50);
                        SetEntityHealth(client, health);
                        EmitSoundToClient(client, "bandage/bandage.mp3", _, _, _, _, 0.8);
                    }
                    else if (health < g_MaxMedicHealth)
                    {
                        if (g_MedicMessages)
                            PrintToChat(client, "[SM] You have called the medic!");
                    }
                    else
                    {
                        PrintToChat(client, "[SM] Get up on your feet, soldier! Get back in there and fight");
                        yell[client] = false;
                    }
                }
            }
            
            CreateTimer(2.0, ResetYell, client)
        }
    }       
    return Plugin_Handled
}

public Action:Call(client, args)
{
    if (g_NativeOverride || g_MedicEnabled)
    {
        if (!yell[client])
        {
            yell[client] = true;

            new currentplayers = GetClientCount(false);
            if (currentplayers < g_MinPlayers)
            {
                if (IsPlayerAlive(client) && (client > 0))
                {
                    new health = GetClientHealth(client);
                    if (health < g_SelfHealth)
                    {
                        health += GetRandomInt(10, 50);
                        SetEntityHealth(client, health);
                        EmitSoundToClient(client, "bandage/bandage.mp3", _, _, _, _, 0.8);
                    }
                    else if (health < g_MaxMedicHealth)
                    {
                        if (g_MedicMessages)
                            PrintToChat(client, "[SM] You have called the medic!");
                    }
                    else
                    {
                        PrintToChat(client, "[SM] Get up on your feet, soldier! Get back in there and fight");
                        yell[client] = false;
                    }
                }
            }

            CreateTimer(2.0, ResetYell, client);
        }
    }       
    return Plugin_Handled
}

public Action:ResetYell(Handle:timer, any:client)
{
    yell[client] = false;
}

public Action:Heal(client, args)
{
    if (g_NativeOverride || g_MedicEnabled)
    {
        if (g_IsMedic[client] >= IsMedic && g_IsMedic[client] <= NotMedicPending)
        {
            new Float:medicVector[3];
            new Float:patientVector[3];

            if (IsPlayerAlive(client))
            {
                new patient = GetClientAimTarget(client, true);
                if (patient > 0)
                {
                    if (IsPlayerAlive(patient))
                    {
                        new client_team = GetClientTeam(client);
                        new patient_team = GetClientTeam(patient);

                        if (client_team == patient_team)
                        {
                            if (g_MedicPacksLeft[client] > 0)
                            {
                                GetClientAbsOrigin(client, medicVector);
                                GetClientAbsOrigin(patient, patientVector);

                                new Float:distance = GetVectorDistance(medicVector, patientVector);

                                if (distance > 100)
                                {
                                    EmitSoundToClient(client, "common/weapon_denyselect.wav", _, _, _, _, 0.8);

                                    if (g_MedicMessages)
                                        PrintToChat(client, "[SM] Too far away to heal this patient");
                                }
                                else
                                {
                                    //Perform healing
                                    new String:patientName[128];
                                    GetClientName(patient, patientName, sizeof(patientName));
                                    new String:medicName[128];
                                    GetClientName(client, medicName, sizeof(medicName));

                                    new patienthealth = GetClientHealth(patient);
                                    if (patienthealth < g_MaxMedicHealth)
                                    {
                                        new amount = GetRandomInt(20, g_MaxMedicHeal[client]);
                                        patienthealth += amount;

                                        if (patienthealth > g_MaxMedicHealth)
                                        {
                                            amount -= patienthealth - g_MaxMedicHealth;
                                            patienthealth = g_MaxMedicHealth;
                                        }

                                        if ((g_IsMedic[patient] >= IsMedic && g_IsMedic[patient] <= NotMedicPending) &&
                                                (patienthealth >= g_MedicHealth[client]))
                                        {
                                            amount -= patienthealth - g_MedicHealth[client];
                                            patienthealth = g_MedicHealth[client];
                                        }

                                        SetEntityHealth(patient, patienthealth);

                                        g_MedicPacksLeft[client]--;
                                        EmitSoundToClient(client, "bandage/bandage.mp3", _, _, _, _, 0.8);
                                        EmitSoundToClient(patient, "bandage/bandage.mp3", _, _, _, _, 0.8);

                                        LogToGame("\"%L\" triggered \"medic_heal\"", client);

                                        if (g_MedicMessages)
                                        {
                                            PrintToChat(client, "[SM] Healed %s with %ihp", patientName, amount);
                                            PrintToChat(patient, "[SM] %s healed you with %ihp", medicName, amount);
                                        }

                                        new Action:result=Plugin_Continue;
                                        Call_StartForward(g_OnMedicHealedHandle);
                                        Call_PushCell(client);
                                        Call_PushCell(patient);
                                        Call_PushCell(amount);
                                        Call_Finish(result);
                                    }
                                    else
                                    {
                                        EmitSoundToClient(client, "common/weapon_denyselect.wav", _, _, _, _, 0.8);
                                    }
                                }
                            }
                            else
                            {
                                EmitSoundToClient(client, "common/weapon_select.wav", _, _, _, _, 0.8);

                                if (g_MedicMessages)
                                    PrintToChat(client, "[SM] You have no more Medic Packs left");
                            }
                        }
                    }
                }
            }
        }
        else
        {
            PrintToChat(client, "[SM] You need to be a Medic to use this command");
        }
    }

    return Plugin_Handled;
}

public Action:GiveClientWeapon(Handle:timer, any:client)
{
    if (IsClientInGame(client))
    {
        new team = GetClientTeam(client);
        new weaponslot;

        // Strip the weapons
        for(new slot = 0; slot < 5; slot++)
        {
            weaponslot = GetPlayerWeaponSlot(client, slot);
            if(weaponslot != -1) 
            {
                RemovePlayerItem(client, weaponslot);
            }
        }

        if (team == 2) 
        {
            GivePlayerItem(client, "weapon_amerknife");

            if (g_MedicWeapon[client] == 0)
            {
                GivePlayerItem(client, "weapon_colt");
                SetEntData(client, ammo_offset+4, g_MedicAmmo[client], 4, true);
            }
            else 
            {
                GivePlayerItem(client, "weapon_m1carbine");
                SetEntData(client, ammo_offset+24, g_MedicAmmo[client], 4, true);
            }

            if (g_MedicNades[client] == 1)
            {
                GivePlayerItem(client, "weapon_smoke_us");
                SetEntData(client, ammo_offset+68, g_MedicNadeAmmo[client], 4, true);
            }
            else if (g_MedicNades[client] == 2)
            {
                GivePlayerItem(client, "weapon_frag_us");
                SetEntData(client, ammo_offset+52, g_MedicNadeAmmo[client], 4, true);
            }
        }

        if (team == 3) 
        {
            GivePlayerItem(client, "weapon_spade");

            if (g_MedicWeapon[client] == 0)
            {
                GivePlayerItem(client, "weapon_p38");
                SetEntData(client, ammo_offset+8, g_MedicAmmo[client], 4, true);
            }
            else 
            {
                GivePlayerItem(client, "weapon_c96");
                SetEntData(client, ammo_offset+12, g_MedicAmmo[client], 4, true);
            }

            if (g_MedicNades[client] == 1)
            {
                GivePlayerItem(client, "weapon_smoke_ger");
                SetEntData(client, ammo_offset+72, g_MedicNadeAmmo[client], 4, true);
            }
            else if (g_MedicNades[client] == 2)
            {
                GivePlayerItem(client, "weapon_frag_ger");
                SetEntData(client, ammo_offset+56, g_MedicNadeAmmo[client], 4, true);
            }
        }
    }
    return Plugin_Handled;
}

 // Added by Lebson506th
public Action:Who(client, args)
{
    if (g_NativeOverride || g_MedicEnabled)
    {
        new ctr = 0;
        if (client == 0) // Console
        {
            for (new i = 1; i < sizeof(g_medic_master[]); i++)
            {
                if (g_medic_master[3][i] != 0)
                {
                    new String:playerName[128];
                    GetClientName(g_medic_master[3][i], playerName, sizeof(playerName));
                    PrintToServer("Axis Medic #%i: %s", i, playerName);
                    ctr++;
                }
            }
            for (new i = 1; i < sizeof(g_medic_master[]); i++)
            {
                if (g_medic_master[2][i] != 0)
                {
                    new String:playerName[128];
                    GetClientName(g_medic_master[2][i], playerName, sizeof(playerName));
                    PrintToServer("Allies Medic #%i: %s", i, playerName);
                    ctr++;
                }
            }
            if (ctr == 0)
                PrintToServer("[SM] The medic class is not being used");
        }
        else
        {
            new team = GetClientTeam(client);
            for (new i = 1; i < sizeof(g_medic_master[]); i++)
            {
                if (g_medic_master[team][i] != 0)
                {
                    new String:playerName[128];
                    GetClientName(g_medic_master[team][i], playerName, sizeof(playerName));

                    if (IsClientInGame(client))
                        PrintToChat(client, "Medic #%i: %s", i, playerName);

                    ctr++;
                }
            }
            if (ctr == 0)
            {
                if (IsClientInGame(client))
                    PrintToChat(client, "[SM] There are no medics on your team");
            }
        }
    }
    return Plugin_Handled
} 

public Native_ControlMedic(Handle:plugin,numParams)
{
    g_NativeOverride = GetNativeCell(1);
    g_MaxMedicHealth = GetNativeCell(2);
    g_SelfHealth = GetNativeCell(3);
    g_MedicRestrict = GetNativeCell(4);
    g_MinPlayers = GetNativeCell(5);
    g_MaxMedics = GetNativeCell(6);

    if (g_MaxMedicHealth < 0)
        g_MaxMedicHealth = GetConVarInt(g_Cvar_MedicMaxHealth);

    if (g_MedicRestrict < 0)
        g_MedicRestrict = GetConVarInt(g_Cvar_MedicRestrict);

    if (g_MinPlayers < 0)
        g_MinPlayers = GetConVarInt(g_Cvar_MedicMinPlayers);

    if (g_MaxMedics < 0)
        g_MaxMedics = GetConVarInt(g_Cvar_MedicMax);

    if (g_SelfHealth < 0)
        g_SelfHealth = GetConVarInt(g_Cvar_MedicSelf);
}

public Native_UnassignMedic(Handle:plugin,numParams)
{
    new client = GetNativeCell(1);
    if (IsPlayerAlive(client))
    {
        g_IsMedic[client] = NotMedicPending
        PrintToChat(client, "*You will no longer spawn as Medic")
    }
    else
    {
        g_IsMedic[client] = NotMedic
        PrintToChat(client, "*You are no longer a Medic")

        if (g_MaxMedics > 0)
        {
            for (new i = 1; i < sizeof(g_medic_master[]); i++)
            {
                for (new team = 2; team <= 3; team++)
                {
                    if (g_medic_master[team][i] == client)
                    {
                        g_medic_master[team][i] = 0
                        break;
                    }
                }
            }
        }
    }
}

public Native_AssignMedic(Handle:plugin,numParams)
{
    new client = GetNativeCell(1);
    if (client > 0)
    {
        g_MedicKeepWeapon[client] = bool:GetNativeCell(2);
        g_MedicPickup[client] = bool:GetNativeCell(3);
        g_MedicSpeed[client] = Float:GetNativeCell(4);
        g_MedicWeight[client] = Float:GetNativeCell(5);
        g_MedicHealth[client] = GetNativeCell(6);
        g_MaxMedicHeal[client] = GetNativeCell(7);
        g_NumMedicPacks[client] = GetNativeCell(8);
        g_MedicWeapon[client] = GetNativeCell(9);
        g_MedicAmmo[client] = GetNativeCell(10);
        g_MedicNades[client] = GetNativeCell(11);
        g_MedicNadeAmmo[client] = GetNativeCell(12);

        if (g_NumMedicPacks[client] < 0)
            g_NumMedicPacks[client] = GetConVarInt(g_Cvar_MedicPacks);

        if (g_MedicNadeAmmo[client] < 0)
            g_MedicNadeAmmo[client] = GetConVarInt(g_Cvar_MedicNadeAmmo);

        if (g_MaxMedicHeal[client] < 0)
            g_MaxMedicHeal[client] = GetConVarInt(g_Cvar_MedicMaxHeal);

        if (g_MedicWeapon[client] < 0)
            g_MedicWeapon[client] = GetConVarInt(g_Cvar_MedicWeapon);

        if (g_MedicHealth[client] < 0)
            g_MedicHealth[client] = GetConVarInt(g_Cvar_MedicHealth);

        if (g_MedicWeight[client] < 0.0)
            g_MedicWeight[client] = GetConVarFloat(g_Cvar_MedicWeight);

        if (g_MedicSpeed[client] < 0.0)
            g_MedicSpeed[client] = GetConVarFloat(g_Cvar_MedicSpeed);

        if (g_MedicNades[client] < 0)
            g_MedicNades[client] = GetConVarInt(g_Cvar_MedicNades);

        if (g_MedicAmmo[client] < 0)
            g_MedicAmmo[client] = GetConVarInt(g_Cvar_MedicAmmo);

        AssignMedic(client);
    }
}    

public AssignMedic(client)
{
    if (IsPlayerAlive(client) && g_IsMedic[client] != IsMedic)
    {
        g_IsMedic[client] = IsMedic;

        SetEntityGravity(client, g_MedicWeight[client]);
        SetEntityHealth(client, g_MedicHealth[client]);
        SetEntityModel(client, g_model[GetClientTeam(client)]);
        g_MedicPacksLeft[client] = g_NumMedicPacks[client]; 

        // For some reason, setting m_flLaggedMovementValue when used with SourceCraft
        // causes weird teleportation to nowhere after spawn?
        if (!g_NativeOverride)
            SetEntPropFloat(client, Prop_Send, "m_flLaggedMovementValue", g_MedicSpeed[client]);

        if (!g_MedicKeepWeapon[client])
        {
            if (!g_MedicPickup[client])
                g_WeaponTimer[client] = CreateTimer(0.1, WeaponsCheck, client, TIMER_REPEAT);

            CreateTimer(0.1, GiveClientWeapon, client);
        }

        if (g_MaxMedics > 0)
        {
            new slot_available = 0;
            new team = GetClientTeam(client);
            for (new i = 1; i < sizeof(g_medic_master[]); i++)
            {
                new occupant = g_medic_master[team][i];
                if (occupant == client)
                {
                    slot_available = 0;
                    break;
                }
                else if (occupant == 0 && slot_available == 0)
                {
                    slot_available = i;
                }
            }

            if (slot_available != 0)
                g_medic_master[team][slot_available] = client;
        }
    }
    else
    {
        g_IsMedic[client] = IsMedicPending;
    }
}

public Native_GetMedicSpeed(Handle:plugin,numParams)
{
    new Float:speed = g_MedicSpeed[GetNativeCell(1)]
    return (speed < 0.0) ? (_:GetConVarFloat(g_Cvar_MedicSpeed)) : (_:speed);
}

public Native_GetMedicWeight(Handle:plugin,numParams)
{
    new Float:weight = g_MedicWeight[GetNativeCell(1)]
    return (weight < 0.0) ? (_:GetConVarFloat(g_Cvar_MedicWeight)) : (_:weight);
}

public Native_MedicHeal(Handle:plugin,numParams)
{
    Heal(GetNativeCell(1), 0);
}
