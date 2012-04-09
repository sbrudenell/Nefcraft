/**
 * vim: set ai et ts=4 sw=4 syntax=sourcepawn :
 * File: PlagueInfect.sp
 * Description: PlagueInfect for the Zerg Plague upgrade for SourceCraft.
 * Author(s): -=|JFH|=-Naris
 */
 
#pragma semicolon 1

#include <sourcemod>
#include <sdktools>
#include <raytrace>
#include <range>

#undef REQUIRE_EXTENSIONS
#include <tf2>
#include <tf2_stocks>
#define REQUIRE_EXTENSIONS

#undef REQUIRE_PLUGIN
#include <dod_ignite>
#define REQUIRE_PLUGIN

// Define _TRACE to enable trace logging for debugging
//#define _TRACE
#include <trace>

#include "sc/SourceCraft"
#include "sc/PlagueType"

#include "effect/Lightning"
#include "effect/HaloSprite"
#include "effect/Explosion"
#include "effect/SendEffects"
#include "effect/FlashScreen"
#include "effect/Shake"

new const String:explodeWav[] = "weapons/explode5.wav";

new const String:HurtSound[][] = { "player/pain.wav",     "player/pl_pain5.wav",
                                   "player/pl_pain6.wav", "player/pl_pain7.wav" };

new m_PlagueDuration[MAXPLAYERS+1];
new m_PlagueAmount[MAXPLAYERS+1];
new m_PlagueInflicter[MAXPLAYERS+1];
new bool:m_HasExploded[MAXPLAYERS+1];
new PlagueType:m_PlagueType[MAXPLAYERS+1];
new Handle:m_PlagueVictimTimers[MAXPLAYERS+1];
new String:m_PlagueShort[MAXPLAYERS+1][32];
new String:m_PlagueName[MAXPLAYERS+1][64];

new Handle:m_TransmitTimer = INVALID_HANDLE;

public Plugin:myinfo = 
{
    name = "SourceCraft Upgrade - Plague Infect",
    author = "-=|JFH|=-Naris",
    description = "The Plague Infect upgrade for SourceCraft.",
    version = SOURCECRAFT_VERSION,
    url = "http://jigglysfunhouse.net/"
};

public APLRes:AskPluginLoad2(Handle:myself, bool:late, String:error[], err_max)
{
    // Register Natives
    CreateNative("PlagueInfect",Native_PlagueInfect);
    CreateNative("ExplodePlayer",Native_ExplodePlayer);
    CreateNative("HasPlayerExploded",Native_HasPlayerExploded);
    RegPluginLibrary("PlagueInfect");

    return APLRes_Success;
}

public OnPluginStart()
{
    GetGameType();
}

public OnMapStart()
{
    SetupLightning(false, false);
    SetupHaloSprite(false, false);

    if (SetupBigExplosion(false, false))
        SetupExplosion(false, false);

    SetupSound(explodeWav, false, false, false);

    for (new i = 0; i < sizeof(HurtSound); i++)
        SetupSound(HurtSound[i], false, false, false);
}

public OnMapEnd()
{
    for (new index=1;index<=MaxClients;index++)
        ResetPlagueVictim(index);

    if (m_TransmitTimer != INVALID_HANDLE)
    {
        KillTimer(m_TransmitTimer);
        m_TransmitTimer = INVALID_HANDLE;
    }
}

public OnClientConnected(client)
{
    ResetPlagueVictim(client);
    m_HasExploded[client]=false;
}

public OnClientDisconnect(client)
{
    ResetPlagueVictim(client);
    m_HasExploded[client]=false;
}

public Action:OnPlayerRestored(client)
{
    ResetPlagueVictim(client);
    return Plugin_Continue;
}

public OnPlayerSpawnEvent(Handle:event, client, race)
{
    if (client > 0)
    {
        ResetPlagueVictim(client);
        m_HasExploded[client]=false;
    }
}

public OnPlayerDeathEvent(Handle:event, victim_index, victim_race, attacker_index,
                          attacker_race, assister_index, assister_race, damage,
                          const String:weapon[], bool:is_equipment, customkill,
                          bool:headshot, bool:backstab, bool:melee)
{
    ResetPlagueVictim(victim_index);
}

ResetPlagueVictim(client)
{
    m_PlagueType[client] = NormalPlague;
    m_PlagueAmount[client] = 0;
    m_PlagueDuration[client] = 0;
    m_PlagueInflicter[client] = 0;

    if (m_PlagueVictimTimers[client] != INVALID_HANDLE)
    {
        KillTimer(m_PlagueVictimTimers[client]);
        m_PlagueVictimTimers[client] = INVALID_HANDLE;	
    }

    SetOverrideSpeed(client,-1.0);
    SetVisibility(client, NormalVisibility);
}

public Action:PlagueVictimTimer(Handle:timer, any:client)
{
    SetTraceCategory("Damage,Immunity");
    TraceInto("PlagueInfect", "PlagueVictimTimer", "client=%d:%N", \
              client, ValidClientIndex(client));

    if (IsClientInGame(client))
    {
        if (IsPlayerAlive(client) &&
            IsClientInGame(m_PlagueInflicter[client]))
        {
            new PlagueType:plagueType = m_PlagueType[client];
            new Immunity:immunity_flag = ((plagueType & UltimatePlague) == UltimatePlague)
                                         ? Immunity_Upgrades : Immunity_Ultimates;

            if (GetImmunity(client,immunity_flag))
            {
                // Unit is immune.
            }
            else if (GetImmunity(client,Immunity_Restore))
            {
                // Unit is immune.
            }
            else if ((plagueType & IrradiatePlague) &&
                     (GetImmunity(client,Immunity_Radiation) ||
                      !GetAttribute(client,Attribute_IsBiological)))
            {
                // Unit is immune.
            }
            else if ((plagueType & InfectiousPlague) &&
                     (GetImmunity(client,Immunity_Infection)))
            {
                // Unit is immune.
            }
            else if ((plagueType & PoisonousPlague) &&
                     (GetImmunity(client,Immunity_Poison)))
            {
                // Unit is immune.
            }
            else
            {
                new duration = m_PlagueDuration[client];
                new amount = (duration > 0) ? duration * m_PlagueAmount[client]
                                            : m_PlagueAmount[client];
                if (amount < 1)
                    amount = 1;

                new health = GetClientHealth(client);
                if (!IsInvulnerable(client))
                {
                    if (!GetImmunity(client,Immunity_HealthTaking))
                    {
                        if (health > amount)
                        {
                            new num = GetRandomInt(0,sizeof(HurtSound)-1);
                            PrepareSound(HurtSound[num]);
                            EmitSoundToAll(HurtSound[num], client);

                            if (plagueType & IrradiatePlague)
                            {
                                Trace("PlagueVictimTimer: client=%d:%N, attacker=%d:%N, health_take=%d", \
                                        client, ValidClientIndex(client), m_PlagueInflicter[client], \
                                        ValidClientIndex(m_PlagueInflicter[client]), amount);

                                FlashScreen(client,RGBA_COLOR_GOLD);
                                HurtPlayer(client, amount, m_PlagueInflicter[client],
                                           m_PlagueShort[client], m_PlagueName[client],
                                           .ignore_armor=true, .type=DMG_RADIATION);
                            }
                            else
                            {
                                if (plagueType & PoisonousPlague)
                                    FlashScreen(client,RGBA_COLOR_BROWN);
                                else if (plagueType & InfectiousPlague)
                                    FlashScreen(client,RGBA_COLOR_ORANGE);
                                else
                                    FlashScreen(client,RGBA_COLOR_RED);

                                Trace("PlagueVictimTimer: client=%d:%N, attacker=%d:%N, health_take=%d", \
                                      client,ValidClientIndex(client), m_PlagueInflicter[client], \
                                      ValidClientIndex(m_PlagueInflicter[client]), amount);

                                health = HurtPlayer(client, amount, m_PlagueInflicter[client],
                                                    m_PlagueShort[client], m_PlagueName[client],
                                                    .ignore_armor=true, .type= DMG_POISON);
                            }
                        }
                        else
                        {
                            m_PlagueDuration[client] = 0;
                            if (m_PlagueType[client] & ExplosivePlague)
                            {
                                new inflicter = m_PlagueInflicter[client];
                                new team = (inflicter > 0 && client != inflicter) ? GetClientTeam(inflicter) : 0;
                                ExplodePlayer(client, inflicter, team, 500.0, 800, 800, NormalExplosion,
                                              10, "sc_explode");
                                health = 0;
                            }
                            else if (m_PlagueType[client] & FatalPlague)
                            {
                                Trace("PlagueVictimTimer: client=%d:%N, attacker=%d:%N, health_take=%d", \
                                        client,ValidClientIndex(client), m_PlagueInflicter[client], \
                                        ValidClientIndex(m_PlagueInflicter[client]), amount);

                                health = HurtPlayer(client, amount, m_PlagueInflicter[client],
                                                    m_PlagueShort[client], m_PlagueName[client],
                                                    .ignore_armor=true,
                                                    .type=(plagueType & IrradiatePlague)
                                                          ? DMG_RADIATION : DMG_POISON);
                            }
                        }
                    }

                    if ((plagueType & FlamingPlague) && health > 0 &&
                        !GetImmunity(client,Immunity_Burning))
                    {
                        if (GameType == tf2)
                            TF2_IgnitePlayer(client, client);
                        else if (GameType == dod)
                            DOD_IgniteEntity(client, 1.0);
                        else
                            IgniteEntity(client, 1.0);
                    }
                }
            }

            if (--m_PlagueDuration[client] > 0)
            {
                TraceReturn("Infection continues for %d:%N!", \
                            client, ValidClientIndex(client));

                return Plugin_Continue;
            }
        }

        SetOverrideSpeed(client,-1.0);
        SetVisibility(client, NormalVisibility);
    }

    m_PlagueVictimTimers[client] = INVALID_HANDLE;	
    m_PlagueInflicter[client] = 0;
    m_PlagueDuration[client] = 0;
    m_PlagueAmount[client] = 0;
    m_PlagueType[client] = NormalPlague;

    TraceReturn("Infection stopped for %d:%N!", \
                client, ValidClientIndex(client));

    return Plugin_Stop;
}

ExplodePlayer(client, inflicter=0, team=0, Float:radius=500.0, damage=800, building=800,
              ExplosionType:type=NormalExplosion, xp=10,
              const String:weapon_name[]="sc_explode",
              const String:weapon_desc[]="")
{
    new clientTeam = GetClientTeam(client);
    if (clientTeam >= 2 && !m_HasExploded[client])
    {
        if ((type & NonFatalExplosion) != NonFatalExplosion)
        {
            m_HasExploded[client]=true;

            if ((type & OnDeathExplosion) != OnDeathExplosion || IsPlayerAlive(client))
                KillPlayer(client, inflicter, weapon_name, weapon_desc, .explode=true);
        }

        new Float:client_location[3];
        GetClientAbsOrigin(client,client_location);
        client_location[2] += 50.0; // Adjust trace position to the middle of the person instead of the feet.

        PrepareSound(explodeWav);
        EmitSoundToAll(explodeWav,client);

        TE_SetupExplosion(client_location,
                          ((type & SmallExplosion) == SmallExplosion) ? Explosion() : BigExplosion(),
                          10.0, 30, 0, RoundToNearest(radius), 20);
        TE_SendEffectToAll();

        if ((type & RingExplosion) == RingExplosion)
        {
            TE_SetupBeamRingPoint(client_location, 10.0, radius, Lightning(), HaloSprite(),
                                  0, 15, 0.5, 10.0, 10.0, {255,255,255,33}, 120, 0);
            TE_SendEffectToAll();

            new beamcolor[]={0,200,255,255}; //blue //secondary ring
            if (clientTeam == 2)
            { //TERRORISTS/RED in TF
                beamcolor[0]=255;
                beamcolor[1]=0;
                beamcolor[2]=0;
            }
            
            TE_SetupBeamRingPoint(client_location, 20.0, radius+10.0, Lightning(), HaloSprite(),
                                  0, 15, 0.5, 10.0, 10.0, beamcolor, 120, 0);
            TE_SendEffectToAll();
        }

        new Immunity:immunity_flag = ((type & SkillExplosion) == SkillExplosion)
                                     ? Immunity_Upgrades : Immunity_Ultimates;

        new bool:flaming = ((type & FlamingExplosion) == FlamingExplosion);
        for (new index=1;index<=MaxClients;index++)
        {
            if (index != client && IsClientInGame(index) &&
                IsPlayerAlive(index) && GetClientTeam(index) != team)
            {
                if (!GetImmunity(index,Immunity_Explosion) &&
                    !GetImmunity(index,immunity_flag) &&
                    !IsInvulnerable(index))
                {
                    new Float:check_location[3];
                    GetClientAbsOrigin(index,check_location);

                    new Float:distance;
                    new dmg = PowerOfRange(client_location, radius, check_location,
                                           damage, .distance=distance);
                    if (dmg > 0 || flaming)
                    {
                        if (TraceTargetIndex(client, index, client_location, check_location))
                        {
                            if (dmg > 0 && !GetImmunity(index,Immunity_HealthTaking))
                            {
                                new hp = HurtPlayer(index, dmg, inflicter, weapon_name, weapon_desc,
                                                    xp, .explode=true, .type=DMG_BLAST|DMG_ALWAYSGIB);
                                if (hp > 0)
                                {
                                    /*
                                    if (!IsFakeClient(client))
                                    {
                                        new Float:factor = (radius-distance)/radius;
                                        if (factor > 0.0 &&
                                            !GetSetting(client,Remove_Queasiness) &&
                                            !GetImmunity(client,Immunity_Drugs))
                                        {
                                            ShakeScreen(index,3.0*factor,250.0*factor,30.0);
                                        }
                                        else
                                            FlashScreen(index,RGBA_COLOR_RED);
                                    }
                                    */

                                    if (flaming && !GetImmunity(index,Immunity_Burning))
                                    {
                                        if (GameType == tf2)
                                            TF2_IgnitePlayer(index, client);
                                        else if (GameType == dod)
                                            DOD_IgniteEntity(index, 10.0);
                                        else
                                            IgniteEntity(index, 10.0);
                                    }
                                }
                            }
                            else
                            {
                                /*
                                if (!IsFakeClient(client))
                                {
                                    new Float:factor = (radius-distance)/radius;
                                    if (factor > 0.0 &&
                                        !GetSetting(client,Remove_Queasiness) &&
                                        !GetImmunity(client,Immunity_Drugs))
                                    {
                                        ShakeScreen(index,3.0*factor,250.0*factor,30.0);
                                    }
                                }
                                */

                                if (flaming && !GetImmunity(index,Immunity_Burning))
                                {
                                    if (GameType == tf2)
                                        TF2_IgnitePlayer(index, client);
                                    else if (GameType == dod)
                                        DOD_IgniteEntity(index, 10.0);
                                    else
                                        IgniteEntity(index, 10.0);
                                }
                            }
                        }
                    }
                }
            }
        }

        if (building > 0 && GetGameType() == tf2)
        {
            new Float:pos[3];
            new maxents = GetMaxEntities();

            for (new ent = MaxClients; ent < maxents; ent++)
            {
                if (IsValidEdict(ent) && IsValidEntity(ent))
                {
                    if (TF2_GetExtObjectTypeFromClass(ent, false) != TFExtObject_Unknown)
                    {
                        if (GetEntProp(ent, Prop_Send, "m_iTeamNum") != team)
                        {
                            new builder = GetEntPropEnt(ent, Prop_Send, "m_hBuilder");
                            if (builder <= 0 || (!GetImmunity(builder,Immunity_Explosion) &&
                                                 !GetImmunity(builder,immunity_flag)))
                            {
                                GetEntPropVector(ent, Prop_Send, "m_vecOrigin", pos);
                                new dmg = PowerOfRange(client_location, radius, pos, building);
                                if (dmg > 0)
                                {
                                    if (TraceTargetEntity(client, ent, client_location, pos))
                                    {
                                        SetVariantInt(dmg);
                                        AcceptEntityInput(ent, "RemoveHealth", client, client);
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}

public Action:TransmitPlague(Handle:timer)
{
    static Float:InfectedVec[MAXPLAYERS + 1][3];
    static Float:NotInfectedVec[MAXPLAYERS + 1][3];
    static InfectedPlayerVec[MAXPLAYERS + 1];
    static NotInfectedPlayerVec[MAXPLAYERS + 1];

    new InfectedCount = 0, NotInfectedCount = 0;

    for (new client = 1; client <= MaxClients; client++)
    {
        if (IsClientInGame(client) && IsPlayerAlive(client))
        {
            new PlagueType:plagueType = m_PlagueType[client];
            if (plagueType)
            {
                if (plagueType & ContagiousPlague)
                {
                    GetClientAbsOrigin(client, InfectedVec[InfectedCount]);
                    InfectedPlayerVec[InfectedCount] = client;
                    InfectedCount++;
                }
            }
            else
            {
                GetClientAbsOrigin(client, NotInfectedVec[NotInfectedCount]);
                NotInfectedPlayerVec[NotInfectedCount] = client;
                NotInfectedCount++;
            }
        }
    }

    if (NotInfectedCount == 0 || InfectedCount == 0)
    {
        m_TransmitTimer = INVALID_HANDLE;
        return Plugin_Stop;
    }

    new check;
    new Float:distance = 2000.0; // GetConVarFloat(Cvar_SpreadDistance);
    for (new infected = 0; infected < InfectedCount; infected++)
    {
        for (check = 0; check < NotInfectedCount; check++)
        {
            if (GetVectorDistance(InfectedVec[infected], NotInfectedVec[check], true) < distance )
            {
                TransmitInfection(NotInfectedPlayerVec[check],InfectedPlayerVec[infected]);
            }
        }
    }
    return Plugin_Continue;
}

stock TransmitInfection(to,from)
{
    new team = GetClientTeam(to);
    if (GetClientTeam(from) == team)
    {
        m_PlagueType[to] = m_PlagueType[from];
        m_PlagueInflicter[to] = m_PlagueInflicter[from];
        m_PlagueDuration[to] = m_PlagueDuration[from];
        m_PlagueAmount[to] = m_PlagueAmount[from];

        strcopy(m_PlagueShort[to], sizeof(m_PlagueShort[]), m_PlagueShort[from]);
        strcopy(m_PlagueName[to], sizeof(m_PlagueName[]), m_PlagueName[from]);

        new r,g,b;
        if (TFTeam:team == TFTeam_Red)
        {
            r = 255;
            g = 60;
            b = 100;
        }
        else
        {
            r = 0;
            g = 100;
            b = 255;
        }
        SetVisibility(to, BasicVisibility, 
                      .mode=RENDER_GLOW,
                      .r=r, .g=g, .b=b);

        if (m_PlagueType[to] & EnsnaringPlague)
            SetOverrideSpeed(to, 0.75);

        if (m_PlagueVictimTimers[to] == INVALID_HANDLE)
            m_PlagueVictimTimers[to] = CreateTimer(1.0,PlagueVictimTimer,to,TIMER_REPEAT);
    }
}

/**
 * Inflicts the plague on someone
 *
 * @param inflicter	   The client doing the inflicting.
 * @param index:       The client getting inflicted.
 * @param duration:    The number of increments to do damage.
 * @param amount:      The amount of damage (* increment) to do per increment.
 * @param type:        The type of plague
 * @param weapon_name: The name of the weapon used to infect the client.
 * @param weapon_desc: The description of the weapon used to infect the client.
 * @noreturn
 * native PlagueInfect(inflicter, index, duration=1, amount=1,
 *                     PlagueType:type=NormalPlague,
 *                     const String:weapon_name[]="plague",
 *                     const String:weapon_desc[]="Plague");
 */
public Native_PlagueInfect(Handle:plugin,numParams)
{
    new inflicter = GetNativeCell(1);
    new index = GetNativeCell(2);
    if (m_PlagueInflicter[index] != inflicter)
    {
        new bool:immune = GetImmunity(index,Immunity_Restore) ||
                          IsInvulnerable(index);

        new PlagueType:plagueType = PlagueType:GetNativeCell(5);
        if ((plagueType & IrradiatePlague) &&
            (GetImmunity(index,Immunity_Radiation) ||
             !GetAttribute(index,Attribute_IsBiological)))
        {
            immune = true;
        }
        else if ((plagueType & InfectiousPlague) &&
                 (GetImmunity(index,Immunity_Infection)))
        {
            immune = true;
        }
        else if ((plagueType & PoisonousPlague) &&
                 (GetImmunity(index,Immunity_Poison)))
        {
            immune = true;
        }
        else if (GetImmunity(index,Immunity_HealthTaking))
        {
            immune = true;
        }

        if ((plagueType & EnsnaringPlague) &&
            !GetImmunity(index,Immunity_MotionTaking) &&
            !GetImmunity(index,Immunity_Restore))
        {
            SetOverrideSpeed(index, 0.75);
            immune = false;
        }

        if (plagueType & ContagiousPlague)
        {
            if (m_TransmitTimer != INVALID_HANDLE)
            {
                m_TransmitTimer = CreateTimer(1.0,TransmitPlague,0,TIMER_REPEAT|TIMER_FLAG_NO_MAPCHANGE);
                immune = false;
            }
        }

        if (!immune)
        {
            m_PlagueType[index] = plagueType;
            m_PlagueInflicter[index] = inflicter;
            m_PlagueDuration[index] = GetNativeCell(3);
            m_PlagueAmount[index] = GetNativeCell(4);

            GetNativeString(6,m_PlagueShort[index],sizeof(m_PlagueShort[]));
            GetNativeString(7,m_PlagueName[index],sizeof(m_PlagueName[]));

            new r,b,g;
            if (TFTeam:GetClientTeam(index) == TFTeam_Red)
            {
                r = 255;
                b = 100;
                g = 60;
            }
            else
            {
                r = 0;
                b = 255;
                g = 100;
            }
            SetVisibility(index, BasicVisibility, 
                          .mode=RENDER_GLOW,
                          .r=r, .b=b, .g=g);

            if (m_PlagueVictimTimers[index] == INVALID_HANDLE)
            {
                m_PlagueVictimTimers[index] = CreateTimer(1.0, PlagueVictimTimer,
                                                          index, TIMER_REPEAT);
            }
        }
    }
}

/**
 * Explode a Player
 *
 * @param client:      The client to explode
 * @param inflicter	   The client causing the explode (if any)
 * @param team:        The team that is NOT affected by the explosion (if any)
 * @param radius:      Radius of the explosion.
 * @param damage:      Damage caused at the center of the explosion
 * @param building:    Damage caused to buildings at the center of the explosion
 * @param type:        Bits to determine what type of explosion
 * @param xp:          Amount of extra xp for a kill (if any)
 * @param weapon_name: The name of the weapon used for the explosion.
 * @param weapon_desc: The description of the weapon used for the explosion.
 * @noreturn
 * native ExplodePlayer(client, inflicter=0, team=0, Float:radius=500.0, damage=800,
 *                      building=800, ExplosionType:type=NormalExplosion, xp=10,
 *                      const String:weapon_name[]="sc_explode",
 *                      const String:weapon_desc[]="");
 */
public Native_ExplodePlayer(Handle:plugin,numParams)
{
    decl String:short[sizeof(m_PlagueShort[])];
    decl String:name[sizeof(m_PlagueName[])];

    GetNativeString(9,short,sizeof(short));
    GetNativeString(10,name,sizeof(name));

    ExplodePlayer(GetNativeCell(1), GetNativeCell(2), GetNativeCell(3),
                  Float:GetNativeCell(4), GetNativeCell(5), GetNativeCell(6),
                  ExplosionType:GetNativeCell(7), GetNativeCell(8),
                  short, name);
}

/**
 * Returns true if the given client has been exploded
 *
 * @param client:      The client to check
 * @return             Returns true if the client has exploded.
 * native bool:HasPlayerExploded(client);
 */
public Native_HasPlayerExploded(Handle:plugin,numParams)
{
    return m_HasExploded[GetNativeCell(1)];
}

