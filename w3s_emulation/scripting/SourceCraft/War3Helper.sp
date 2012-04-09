/**
 * vim: set ai et ts=4 sw=4 :
 * File: War3Helper.sp
 * Description: Helper natives for War3Source compatability.
 * Author(s): PimpinJuice(Anthony Iacono) and Ownz
 * Adapted to SourceCraft by: -=|JFH|=-Naris
 */

#pragma semicolon 1

#include <sourcemod>
#include <gametype>
#include <ResourceManager>

// Define _TRACE to enable trace logging for debugging
#define _TRACE
#include <trace>

#include "sc/SourceCraft"
#include "sc/maxhealth"
#include "sc/client"

#include "W3SIncs/constants"
//#include "W3SIncs/War3Source_Interface"

#define VERSION_NUM "1.1.6B6"
#define REVISION_NUM 1163 //increment every release

#define g_Game                      GameType
#define War3_GetGame()              GameType

#define War3_GetMaxHP(%1)           g_iMaxXP[%1]

#define W3GetDamageType()           g_Lastdamagetype
#define W3GetDamageInflictor()      g_Lastinflictor

#define W3HasImmunity               GetImmunity
#define W3GetDamageIsBullet()       NW3GetDamageIsBullet(INVALID_HANDLE,0)
#define W3ForceDamageIsBullet()     NW3ForceDamageIsBullet(INVALID_HANDLE,0)

new const String:abilityReadySound[]="war3source/ability_refresh.mp3";
new const String:ultimateReadySound[]="war3source/ult_ready.wav";

// Forwards
new Handle:g_OnWar3RaceSelectedHandle;
new Handle:g_OnWar3UltimateCommandHandle;
new Handle:g_OnAbilityCommandHandle;
new Handle:g_OnWar3PluginReadyHandle; //loadin default races in order
new Handle:g_OnWar3PluginReadyHandle2; //other races
new Handle:g_OnWar3PluginReadyHandle3; //other races backwards compatable
new Handle:g_War3InterfaceExecFH;
new Handle:g_OnWar3EventSpawnFH;
new Handle:g_OnWar3EventDeathFH;
new Handle:g_War3GlobalEventFH; 
//new Handle:g_OnWar3EventRoundStartFH;
//new Handle:g_OnWar3EventRoundEndFH;

// Forwards from Engine6
new Handle:FHOnW3TakeDmgAll;
new Handle:FHOnW3TakeDmgBullet;
new Handle:g_OnWar3EventPostHurtFH;

// Convars
new Handle:ChanceModifierPlasma;
new Handle:ChanceModifierBurn;
new Handle:ChanceModifierHeavy;
new Handle:ChanceModifierSentry;
new Handle:ChanceModifierSentryRocket;
new Handle:ChanceModifierMedic;
new Handle:ChanceModifierSMGSniper;
new Handle:hUseMetric;

// Offsets
new MyWeaponsOffset;
//new DuckedOffset;
new Clip1Offset;
new AmmoOffset;

// GameFrame tracking definitions
new Float:fAngle[65][3];
new Float:fPos[65][3];
new bool:bDucking[65];
new iWeapon[65][10][2]; // [client][iterator][0-ent/1-clip1]
new iAmmo[65][32];
new iDeadAmmo[65][32];
new iDeadClip1[65][10];
new String:sWepName[65][10][64];
new bool:bIgnoreTrackGF[65];

// MaxHP
new g_iMaxXP[MAXPLAYERS+1]; // kinda hacky
new W3VarArr[W3Var];

// Damage
new g_Lastdamagetype;
new g_Lastinflictor; //variables from sdkhooks, natives retrieve them if needed
new g_LastDamageIsWarcraft; //for this damage only
new g_ForceDamageIsBullet; //for this damage only
new actualdamagedealt = 0;
new bool:nextDamageIsWarcraftDamage; //dealdamage tells hook that the damage he hooked is warcraft damage
new bool:nextDamageIsTrueDamage;
new Float:damageModifierPercent=1.0; //use -1.0 to 1.0

new DamageStack[255];
new DamageStackVictim[255];
new DamageStackLen=-1;

// Expire Timer stuff
#define MAXTHREADS 2000
new Float:expireTime[MAXTHREADS];
new threadsLoaded;

public Plugin:myinfo= 
{
	name="War3Helper",
	author="Naris, PimpinJuice and Ownz",
	description="War3Source compatibility for SourceCraft.",
	version=SOURCECRAFT_VERSION,
	url="http://war3source.com/"
};

public APLRes:AskPluginLoad2(Handle:myself, bool:late, String:error[], err_max)
{
    if(!War3Source_InitNatives())
    {
        PrintToServer("[War3Source] There was a failure in creating the native based functions, definately halting.");
        return APLRes_Failure;
    }
    else if(!War3Source_InitForwards())
    {
        PrintToServer("[War3Source] There was a failure in creating the forward based functions, definately halting.");
        return APLRes_Failure;
    }
    else
        return APLRes_Success;
}

public OnPluginStart()
{
    GetGameType();

    if(!War3Source_InitCVars())
        SetFailState("[War3Source] There was a failure in initiating console variables.");

    if(!War3Source_InitHooks())
        SetFailState("[War3Source] There was a failure in initiating the hooks.");

    if(!War3Source_InitOffset())
        SetFailState("[War3Source] There was a failure in finding the offsets required.");

    // Start cheesy damage reset timer :/
    CreateTimer(1.0,SecondTimer,_,TIMER_REPEAT);
}

public OnSourceCraftReady()
{
    //ordered loads
    new res;
    for(new i;i<=MAXRACES*10;i++){
        Call_StartForward(g_OnWar3PluginReadyHandle);
        Call_PushCell(i);
        Call_Finish(res);
    }

    //orderd loads 2
    for(new i;i<=MAXRACES*10;i++){
        Call_StartForward(g_OnWar3PluginReadyHandle2);
        Call_PushCell(i);
        Call_Finish(res);
    }

    //unorderd loads
    Call_StartForward(g_OnWar3PluginReadyHandle3);
    Call_Finish(res);
}

public OnMapStart()
{
    new dummyreturn;
    Call_StartForward(g_War3InterfaceExecFH);
    Call_Finish(dummyreturn);

    for(new i=0;i<sizeof(expireTime);i++){
        expireTime[i]=0.0;
    }

    SetupSound(abilityReadySound, ALWAYS_DOWNLOAD, false, false);
    SetupSound(ultimateReadySound, ALWAYS_DOWNLOAD, false, false);
}

public OnClientPutInServer(client)
{
    DoFwd_War3_Event(ClearPlayerVariables,client);
}

public OnClientDisconnect(client)
{
    DoFwd_War3_Event(ClearPlayerVariables,client);
}

new ignoreClient;
public NWar3_GetAimEndPoint(Handle:plugin,numParams)
{
	new client=GetNativeCell(1);
	new Float:angle[3];
	GetClientEyeAngles(client,angle);
	new Float:endpos[3];
	new Float:startpos[3];
	GetClientEyePosition(client,startpos);
	
	ignoreClient=client;
	TR_TraceRayFilter(startpos,angle,MASK_ALL,RayType_Infinite,AimTargetFilter);
	TR_GetEndPosition(endpos);
	
	SetNativeArray(2,endpos,3);
}
public NWar3_GetAimTraceMaxLen(Handle:plugin,numParams)
{
	new client=GetNativeCell(1);
	new Float:angle[3];
	GetClientEyeAngles(client,angle);
	new Float:endpos[3];
	new Float:startpos[3];
	GetClientEyePosition(client,startpos);
	new Float:dir[3];
	GetAngleVectors(angle, dir, NULL_VECTOR, NULL_VECTOR);
	
	ScaleVector(dir, GetNativeCell(3));
	AddVectors(startpos, dir, endpos);
	
	ignoreClient=client;
	TR_TraceRayFilter(startpos,endpos,MASK_ALL,RayType_EndPoint,AimTargetFilter);
	
	TR_GetEndPosition(endpos); //overwrites to actual end pos
	
	SetNativeArray(2,endpos,3);
}
public bool:AimTargetFilter(entity,mask)
{
	return !(entity==ignoreClient);
}
public bool:CanHitThis(entityhit, mask, any:data)
{
	if(entityhit == data )
	{// Check if the TraceRay hit the itself.
		return false; // Don't allow self to be hit, skip this result
	}
	if(ValidPlayer(entityhit)&&ValidPlayer(data)&&War3_GetGame()==Game_TF&&GetClientTeam(entityhit)==GetClientTeam(data)){
		return false; //skip result, prend this space is not taken cuz they on same team
	}
	return true; // It didn't hit itself
}

public Native_War3_GetTargetInViewCone(Handle:plugin,numParams)
{
	if(numParams==5)
	{
		new client=GetNativeCell(1);
		if (IsClientInGame(client) && IsPlayerAlive(client))
		{
			new Float:max_distance=GetNativeCell(2);
			new bool:include_friendlys=GetNativeCell(3);
			new Float:cone_angle=GetNativeCell(4);
			new Function:FilterFunction=GetNativeCell(5);
			if(max_distance<0.0)	max_distance=0.0;
			if(cone_angle<0.0)	cone_angle=0.0;
			
			new Float:PlayerEyePos[3];
			new Float:PlayerAimAngles[3];
			new Float:PlayerToTargetVec[3];
			new Float:OtherPlayerPos[3];
			GetClientEyePosition(client,PlayerEyePos);
			GetClientEyeAngles(client,PlayerAimAngles);
			new Float:ThisAngle;
			new Float:playerDistance;
			new Float:PlayerAimVector[3];
			GetAngleVectors(PlayerAimAngles,PlayerAimVector,NULL_VECTOR,NULL_VECTOR);
			new bestTarget=0;
			new Float:bestTargetDistance;
			for(new i=1;i<=MaxClients;i++)
			{
				if(cone_angle<=0.0)	break;
				if(IsClientConnected(i)&&IsClientInGame(i)&&IsPlayerAlive(i)&& client!=i)
				{
					if(FilterFunction!=INVALID_FUNCTION)
					{
						Call_StartFunction(plugin,FilterFunction);
						Call_PushCell(i);
						new result;
						if(Call_Finish(result)>SP_ERROR_NONE)
						{
							result=1; // bad callback, lets return 1 to be safe
							new String:plugin_name[256];
							GetPluginFilename(plugin,plugin_name,256);
							PrintToServer("[War3Source] ERROR in plugin \"%s\" traced to War3_GetTargetInViewCone(), bad filter function provided.",plugin_name);
						}
						if(result==0)
						{
							continue;
						}
					}
					if(!include_friendlys && GetClientTeam(client) == GetClientTeam(i))
					{
						continue;
					}
					GetClientEyePosition(i,OtherPlayerPos);
					playerDistance = GetVectorDistance(PlayerEyePos,OtherPlayerPos);
					if(max_distance>0.0 && playerDistance>max_distance)
					{
						continue;
					}
					SubtractVectors(OtherPlayerPos,PlayerEyePos,PlayerToTargetVec);
					ThisAngle=ArcCosine(GetVectorDotProduct(PlayerAimVector,PlayerToTargetVec)/(GetVectorLength(PlayerAimVector)*GetVectorLength(PlayerToTargetVec)));
					ThisAngle=ThisAngle*360/2/3.14159265;
					if(ThisAngle<=cone_angle)
					{
						ignoreClient=client;
						TR_TraceRayFilter(PlayerEyePos,OtherPlayerPos,MASK_ALL,RayType_EndPoint,AimTargetFilter);
						if(TR_DidHit())
						{
							new entity=TR_GetEntityIndex();
							if(entity!=i)
							{
								continue;
							}
						}
						if(bestTarget>0)
						{
							if(playerDistance<bestTargetDistance)
							{
								bestTarget=i;
								bestTargetDistance=playerDistance;
							}
						}
						else
						{
							bestTarget=i;
							bestTargetDistance=playerDistance;
						}
					}
				}
			}
			if(bestTarget==0)
			{
				new Float:endpos[3];
				if(max_distance>0.0)
					ScaleVector(PlayerAimVector,max_distance);
				else
					ScaleVector(PlayerAimVector,56756.0);
				AddVectors(PlayerEyePos,PlayerAimVector,endpos);
				TR_TraceRayFilter(PlayerEyePos,endpos,MASK_ALL,RayType_EndPoint,AimTargetFilter);
				if(TR_DidHit())
				{
					new entity=TR_GetEntityIndex();
					if(entity>0 && entity<=MaxClients && IsClientConnected(entity) && IsPlayerAlive(entity) && GetClientTeam(client)!=GetClientTeam(entity) )
					{
						new result=1;
						if(FilterFunction!=INVALID_FUNCTION)
						{
							Call_StartFunction(plugin,FilterFunction);
							Call_PushCell(entity);
							if(Call_Finish(result)>SP_ERROR_NONE)
							{
								result=1; // bad callback, return 1 to be safe
								new String:plugin_name[256];
								GetPluginFilename(plugin,plugin_name,256);
								PrintToServer("[War3Source] ERROR in plugin \"%s\" traced to War3_GetTargetInViewCone(), bad filter function provided.",plugin_name);
							}
						}
						if(result!=0)
						{
							bestTarget=entity;
						}
					}
				}
			}
			return bestTarget;
		}
	}
	return 0;
}

public NW3LOS(Handle:plugin,numParams)
{
	new client=GetNativeCell(1);
	new target=GetNativeCell(2);
	if(ValidPlayer(client,true)&&ValidPlayer(target,true))
	{
		new Float:PlayerEyePos[3];
		new Float:OtherPlayerPos[3];
		GetClientEyePosition(client,PlayerEyePos);
		GetClientEyePosition(target,OtherPlayerPos);
		ignoreClient=client;
		TR_TraceRayFilter(PlayerEyePos,OtherPlayerPos,MASK_ALL,RayType_EndPoint,AimTargetFilter);
		if(TR_DidHit())
		{
			new entity=TR_GetEntityIndex();
			if(entity==target)
			{
				return true;
			}
		}
	}
	return false;
}

public Native_War3_CachedAngle(Handle:plugin,numParams)
{
	if(numParams==2)
	{
		new client=GetNativeCell(1);
		SetNativeArray(2,fAngle[client],3);
	}
}

public Native_War3_CachedPosition(Handle:plugin,numParams)
{
	if(numParams==2)
	{
		new client=GetNativeCell(1);
		SetNativeArray(2,fPos[client],3);
	}
}

public Native_War3_CachedDucking(Handle:plugin,numParams)
{
	if(numParams==1)
	{
		new client=GetNativeCell(1);
		return (bDucking[client])?1:0;
	}
	return 0;
}

public Native_War3_CachedWeapon(Handle:plugin,numParams)
{
	if(numParams==2)
	{
		new client=GetNativeCell(1);
		new iter=GetNativeCell(2);
		if (iter>=0 && iter<10)
		{
			return iWeapon[client][iter][0];
		}
	}
	return 0;
}

public Native_War3_CachedClip1(Handle:plugin,numParams)
{
	if(numParams==2)
	{
		new client=GetNativeCell(1);
		new iter=GetNativeCell(2);
		if (iter>=0 && iter<10)
		{
			return iWeapon[client][iter][1];
		}
	}
	return 0;
}

public Native_War3_CachedAmmo(Handle:plugin,numParams)
{
	if(numParams==2)
	{
		new client=GetNativeCell(1);
		new id=GetNativeCell(2);
		if (id>=0 && id<32)
		{
			return iAmmo[client][id];
		}
	}
	return 0;
}

public Native_War3_CachedDeadClip1(Handle:plugin,numParams)
{
	if(numParams==2)
	{
		new client=GetNativeCell(1);
		new iter=GetNativeCell(2);
		if (iter>=0 && iter<10)
		{
			return iDeadClip1[client][iter];
		}
	}
	return 0;
}

public Native_War3_CachedDeadAmmo(Handle:plugin,numParams)
{
	if(numParams==2)
	{
		new client=GetNativeCell(1);
		new id=GetNativeCell(2);
		if (id>=0 && id<32)
		{
			return iDeadAmmo[client][id];
		}
	}
	return 0;
}

public Native_War3_CDWN(Handle:plugin,numParams)
{
	if(numParams==4)
	{
		new client=GetNativeCell(1);
		new iter=GetNativeCell(2);
		if (iter>=0 && iter<10)
		{
			SetNativeString(3,sWepName[client][iter],GetNativeCell(4));
		}
	}
}

public Native_War3_TF_PTC(Handle:plugin,numParams)
{
    if(numParams==3)
    {
        new client = GetNativeCell(1);
        new String:str[32];
        GetNativeString(2, str, 31);
        new Float:pos[3];
        GetNativeArray(3,pos,3);
        TE_ParticleToClient(client,str,pos);
    }
}

TE_ParticleToClient(client,
            String:Name[],
            Float:origin[3]=NULL_VECTOR,
            Float:start[3]=NULL_VECTOR,
            Float:angles[3]=NULL_VECTOR,
            entindex=-1,
            attachtype=-1,
            attachpoint=-1,
            bool:resetParticles=true,
            Float:delay=0.0)
{
    // find string table
    new tblidx = FindStringTable("ParticleEffectNames");
    if (tblidx==INVALID_STRING_TABLE) 
    {
        LogError("Could not find string table: ParticleEffectNames");
        return;
    }
    
    // find particle index
    new String:tmp[256];
    new count = GetStringTableNumStrings(tblidx);
    new stridx = INVALID_STRING_INDEX;
    new i;
    for (i=0; i<count; i++)
    {
        ReadStringTable(tblidx, i, tmp, sizeof(tmp));
        if (StrEqual(tmp, Name, false))
        {
            stridx = i;
            break;
        }
    }
    if (stridx==INVALID_STRING_INDEX)
    {
        LogError("Could not find particle: %s", Name);
        return;
    }
    
    TE_Start("TFParticleEffect");
    TE_WriteFloat("m_vecOrigin[0]", origin[0]);
    TE_WriteFloat("m_vecOrigin[1]", origin[1]);
    TE_WriteFloat("m_vecOrigin[2]", origin[2]);
    TE_WriteFloat("m_vecStart[0]", start[0]);
    TE_WriteFloat("m_vecStart[1]", start[1]);
    TE_WriteFloat("m_vecStart[2]", start[2]);
    TE_WriteVector("m_vecAngles", angles);
    TE_WriteNum("m_iParticleSystemIndex", stridx);
    if (entindex!=-1)
    {
        TE_WriteNum("entindex", entindex);
    }
    if (attachtype!=-1)
    {
        TE_WriteNum("m_iAttachType", attachtype);
    }
    if (attachpoint!=-1)
    {
        TE_WriteNum("m_iAttachmentPointIndex", attachpoint);
    }
    TE_WriteNum("m_bResetParticles", resetParticles ? 1 : 0);    
    if(client==0)
    {
        TE_SendToAll(delay);
    }
    else
    {
        TE_SendToClient(client, delay);
    }
}

///should be deprecated
public Native_War3_SetMaxHP(Handle:plugin,numParams)
{
	new client=GetNativeCell(1);
	new hp=GetNativeCell(2);
	if(client>0 && client<=MaxClients)
        g_iMaxXP[client]=hp;
}

public Native_War3_GetMaxHP(Handle:plugin,numParams)
{
    new client=GetNativeCell(1);
    if(client>0 && client<MaxClients)
        return g_iMaxXP[client];
    return 0;
}

public Native_War3_HTMHP(Handle:plugin,numParams)
{
    if(numParams==2)
    {
        new client = GetNativeCell(1);
        new addhp = GetNativeCell(2);
        new maxhp=War3_GetMaxHP(client);
        new currenthp=GetClientHealth(client);
        
        if (addhp<0)
            LogError("Attempted negative Heal %d:%N's curhp=%d, addhp=%d,maxhp=%d", client, client, currenthp, addhp, maxhp);
        else if (currenthp>0&&currenthp<maxhp){ ///do not make hp lower
            new newhp=currenthp+addhp;
            if (newhp>maxhp){
                newhp=maxhp;
            }
            SetEntityHealth(client,newhp);
        }
    }
}

public Native_War3_HTBHP(Handle:plugin,numParams)
{
    if(numParams==2)
    {
        new client = GetNativeCell(1);
        new addhp = GetNativeCell(2);
        new maxhp=(g_Game=Game_TF)?RoundFloat(float(War3_GetMaxHP(client))*1.5):War3_GetMaxHP(client);
        
        new currenthp=GetClientHealth(client);
        if (addhp<0)
            LogError("Attempted negative HealToBuff %d:%N's curhp=%d, addhp=%d,maxhp=%d", client, client, currenthp, addhp, maxhp);
        else if (currenthp>0&&currenthp<maxhp){ ///do not make hp lower
            new newhp=currenthp+addhp;
            if (newhp>maxhp){
                newhp=maxhp;
            }
            SetEntityHealth(client,newhp);
        }
    }
}

public Native_War3_DecreaseHP(Handle:plugin,numParams)
{
	if(numParams==2)
	{
		new client = GetNativeCell(1);
		new dechp = GetNativeCell(2);
		new newhp=GetClientHealth(client)-dechp;
		if(newhp<0){
			newhp=0;
		}
		SetEntityHealth(client,newhp);
	}
}

public NW3GetW3Revision(Handle:plugin,numParams)
{
	return REVISION_NUM;
}
public NW3GetW3Version(Handle:plugin,numParams)
{
	SetNativeString(1,VERSION_NUM,GetNativeCell(2));
}

public NW3CreateEvent(Handle:plugin,numParams)
{

	new event=GetNativeCell(1);
	new client=GetNativeCell(2);
	DoFwd_War3_Event(W3EVENT:event,client);
}

DoFwd_War3_Event(W3EVENT:event,client)
{
    new dummyreturn;
    Call_StartForward(g_War3GlobalEventFH);
    Call_PushCell(event);
    Call_PushCell(client);
    Call_Finish(dummyreturn);
}

public Action:War3Source_AbilityCommand(client,args)
{
    decl String:command[32];
    GetCmdArg(0,command,32);

    new bool:pressed=(command[0]=='+');

    new num = 0;
    if (IsCharNumeric(command[8]))
        num=_:command[8]-48;

    new result;
    Call_StartForward(g_OnAbilityCommandHandle);
    Call_PushCell(client);
    Call_PushCell(num);
    Call_PushCell(pressed);
    Call_Finish(result);
    return Plugin_Handled;
}

public OnUltimateCommand(client,race,bool:pressed,arg)
{
    if (arg <= 1)
    {
		new result;
		Call_StartForward(g_OnWar3UltimateCommandHandle);
		Call_PushCell(client);
		Call_PushCell(race);
		Call_PushCell(pressed);
		Call_Finish(result);
    }
    else
    {
        new result;
        Call_StartForward(g_OnAbilityCommandHandle);
        Call_PushCell(client);
        Call_PushCell(arg-2);
        Call_PushCell(pressed);
        Call_Finish(result);
    }
}

public Action:OnRaceSelected(client,oldrace,newrace)
{
    new result;
    W3VarArr[OldRace]=oldrace;
    Call_StartForward(g_OnWar3RaceSelectedHandle);
    Call_PushCell(client);
    Call_PushCell(newrace);
    Call_Finish(result);
    return Plugin_Continue;
}

public OnPlayerSpawnEvent(Handle:event, client, race)
{
    g_iMaxXP[client]=GetClientHealth(client);

    new result;
    Call_StartForward(g_OnWar3EventSpawnFH);
    Call_PushCell(client);
    Call_Finish(result);
}

public OnPlayerDeathEvent(Handle:event,victim_index,victim_race, attacker_index,
                          attacker_race,assister_index,assister_race,
                          damage,const String:weapon[], bool:is_equipment,
                          customkill,bool:headshot,bool:backstab,bool:melee)
{
    new result;
    W3VarArr[DeathRace]=victim_race;
    Call_StartForward(g_OnWar3EventDeathFH);
    Call_PushCell(victim_index);
    Call_PushCell(attacker_index);
    Call_Finish(result);
    g_ForceDamageIsBullet=false;

    #if defined _TRACE
        if (DamageStackLen != -1)
        {
            TraceInto("War3Helper", "OnPlayerDeathEvent", "DamageStackLen=%d", DamageStackLen);
            TraceReturn();
        }
    #endif

    DamageStackLen=-1;
}

public Action:OnPlayerHurtEvent(Handle:event, victim_index, victim_race, attacker_index,
                                attacker_race, assister_index, assister_race,
                                damage, absorbed, bool:from_sc)
{
    TraceInto("War3Helper", "OnPlayerHurtEvent", "victim=%N, attacker=%N, damage=%d, absorbed=%d, DamageStackLen=%d", \
              ValidClientIndex(victim_index), ValidClientIndex(attacker_index), damage, absorbed, DamageStackLen);

    if (DamageStackLen < 0)
        DamageStackLen = 0;

    damage += absorbed;

    DamageStack[DamageStackLen]=damage;

    new dummyreturn;
    Call_StartForward(g_OnWar3EventPostHurtFH);
    Call_PushCell(victim_index);
    Call_PushCell(attacker_index);
    Call_PushCell(damage);
    Call_Finish(dummyreturn);

    g_ForceDamageIsBullet=false;
    DamageStackLen--;

    TraceReturn();
}

public Action:OnXPGiven(client,&amount,bool:taken)
{
    //set event vars
    W3VarArr[EventArg1]=0; // W3XPAwardedBy:awardedfromevent
    W3VarArr[EventArg2]=amount;
    W3VarArr[EventArg3]=0;
    DoFwd_War3_Event(OnPreGiveXPGold,client); //fire event
    new addxp=W3VarArr[EventArg2]; //retrieve possibly modified vars
    if (addxp != amount)
    {
        amount = addxp;
        return Plugin_Changed;
    }
    else
        return Plugin_Continue;
}

public Action:OnCrystalsGiven(client,&amount,bool:taken)
{
    //set event vars
    W3VarArr[EventArg1]=0; // W3XPAwardedBy:awardedfromevent
    W3VarArr[EventArg2]=0;
    W3VarArr[EventArg3]=amount;
    DoFwd_War3_Event(OnPreGiveXPGold,client); //fire event
    new addgold=W3VarArr[EventArg3]; //retrieve possibly modified vars
    if (addgold != amount)
    {
        amount = addgold;
        return Plugin_Changed;
    }
    else
        return Plugin_Continue;
}

public Native_War3_ChanceModifier(Handle:plugin,numParams)
{
	if(numParams!=3)
		return _:1.0;
	new attacker=GetNativeCell(1);
	new inflictor=GetNativeCell(2);
	new damagetype=GetNativeCell(3);
	if(attacker<=0 || attacker>MaxClients || !IsValidEdict(attacker))
		return _:1.0;
	if(damagetype&DMG_BURN)
	{
		return _:GetConVarFloat(ChanceModifierBurn);
	}
	if(damagetype&DMG_PLASMA)
	{
		return _:GetConVarFloat(ChanceModifierPlasma);
	}
	if(attacker!=inflictor)
	{
		if(inflictor>0 && IsValidEdict(inflictor))
		{
			new String:ent_name[64];
			GetEdictClassname(inflictor,ent_name,64);
			if(StrContains(ent_name,"obj_sentrygun",false)==0)
			{
				return _:GetConVarFloat(ChanceModifierSentry);
			}
			else if(StrContains(ent_name,"tf_projectile_sentryrocket",false)==0)
			{
				return _:GetConVarFloat(ChanceModifierSentryRocket);
			}
		}
	}
	new String:weapon[64];
	GetClientWeapon(attacker,weapon,64);
	if(StrEqual(weapon,"tf_weapon_minigun",false))
	{
		return _:GetConVarFloat(ChanceModifierHeavy);
	}
	if(StrEqual(weapon,"tf_weapon_syringegun_medic",false))
	{
		return _:GetConVarFloat(ChanceModifierMedic);
	}
	if(StrEqual(weapon,"tf_weapon_smg",false))
	{
		return _:GetConVarFloat(ChanceModifierSMGSniper);
	}
	return _:1.0;
}

public Native_W3ChanceModifier(Handle:plugin,numParams)
{
	if(numParams!=1)
		return _:1.0;
	new attacker=GetNativeCell(1);
	new inflictor=W3GetDamageInflictor();
	new damagetype=W3GetDamageType();
	if(attacker<=0 || attacker>MaxClients || !IsValidEdict(attacker))
		return _:1.0;
	if(damagetype&DMG_BURN)
	{
		return _:GetConVarFloat(ChanceModifierBurn);
	}
	if(damagetype&DMG_PLASMA)
	{
		return _:GetConVarFloat(ChanceModifierPlasma);
	}
	if(attacker!=inflictor)
	{
		if(inflictor>0 && IsValidEdict(inflictor))
		{
			new String:ent_name[64];
			GetEdictClassname(inflictor,ent_name,64);
			if(StrContains(ent_name,"obj_sentrygun",false)==0)
			{
				return _:GetConVarFloat(ChanceModifierSentry);
			}
			else if(StrContains(ent_name,"tf_projectile_sentryrocket",false)==0)
			{
				return _:GetConVarFloat(ChanceModifierSentryRocket);
			}
		}
	}
	new String:weapon[64];
	GetClientWeapon(attacker,weapon,64);
	if(StrEqual(weapon,"tf_weapon_minigun",false))
	{
		return _:GetConVarFloat(ChanceModifierHeavy);
	}
	if(StrEqual(weapon,"tf_weapon_syringegun_medic",false))
	{
		return _:GetConVarFloat(ChanceModifierMedic);
	}
	if(StrEqual(weapon,"tf_weapon_smg",false))
	{
		return _:GetConVarFloat(ChanceModifierSMGSniper);
	}
	return _:1.0;
}

public Native_War3_DamageModPercent(Handle:plugin,numParams)
{
    if(numParams==1)
    {
        new Float:num=GetNativeCell(1); 
        //PrintToChatAll("percent change %f",num);
        damageModifierPercent*=num;
        //1.0*num;
        //PrintToChatAll("2percent change %f",1.0+num);
        //PrintToChatAll("3percent change %f",100.0*(1.0+num));
    }
}

public NW3GetDamageType(Handle:plugin,numParams)
{
	return g_Lastdamagetype;
}

public NW3GetDamageInflictor(Handle:plugin,numParams)
{
	return g_Lastinflictor;
}

public NW3GetDamageIsBullet(Handle:plugin,numParams)
{
	return g_ForceDamageIsBullet      || !g_LastDamageIsWarcraft ||
           !GetDamageFromPlayerHurt() || !GetSuppressDamageForward();
}

public NW3ForceDamageIsBullet(Handle:plugin,numParams)
{
	g_LastDamageIsWarcraft=false;
	g_ForceDamageIsBullet=true;
}

public Native_War3_GetWar3DamageDealt(Handle:plugin,numParams)
{
	return actualdamagedealt;
}

Float:PhysicalArmorMulti(client)
{
    new Float:armor=GetPhysicalArmorSum(client);
    return (1.0-(armor*0.06)/(1+armor*0.06));
}

Float:MagicArmorMulti(client)
{
    new Float:armor=GetMagicalArmorSum(client);
    return (1.0-(armor*0.06)/(1+armor*0.06));
}

public NW3GetPhysicalArmorMulti(Handle:plugin,numParams)
{
    return _:PhysicalArmorMulti(GetNativeCell(1));
}

public NW3GetMagicArmorMulti(Handle:plugin,numParams)
{
    return _:MagicArmorMulti(GetNativeCell(1));
}

stock War3_SetCSArmor(client,amount)
{
    if (War3_GetGame()==Game_CS)
    {
        if (amount>125)
            amount=125;

        SetEntProp(client,Prop_Send,"m_ArmorValue",amount);
    }
}

stock War3_GetCSArmor(client)
{
    if (War3_GetGame()==Game_CS)
        return GetEntProp(client,Prop_Send,"m_ArmorValue");
    else        
        return 0;
}

////if dealdamage is called then player died, posthurt will not be called and stack length stays longer
///since this is single threaded, we assume there is no actual damage exchange when a timer hits, we reset the stack length 
public Action:SecondTimer(Handle:t,any:a)
{
    #if defined _TRACE
        if (DamageStackLen != -1)
        {
            TraceInto("War3Helper", "SecondTimer", "DamageStackLen=%d", DamageStackLen);
            TraceReturn();
        }
    #endif

    DamageStackLen=-1;
}

public OnMapEnd()
{
    #if defined _TRACE
        if (DamageStackLen != -1)
        {
            TraceInto("War3Helper", "OnMapEnd", "DamageStackLen=%d", DamageStackLen);
            TraceReturn();
        }
    #endif

    DamageStackLen=-1;
}

public Native_War3_DealDamage(Handle:plugin,numParams)
{
    if (numParams==9)
    {
        new victim=GetNativeCell(1);
        new damage=GetNativeCell(2);

        TraceInto("War3Helper", "DealDamage", "victim=%N, damage=%d, DamageStackLen=%d", \
                  ValidClientIndex(victim), DamageStackLen);

        if (ValidPlayer(victim,true) && damage>0 )
        {
            actualdamagedealt=0;

            new attacker=GetNativeCell(3);
            new dmg_type=GetNativeCell(4);  //original weapon damage type

            decl String:weapon[64];
            GetNativeString(5,weapon,64);

            new War3DamageOrigin:W3DMGORIGIN=GetNativeCell(6);
            new War3DamageType:WAR3_DMGTYPE=GetNativeCell(7);

            new bool:respectVictimImmunity=GetNativeCell(8);

            if (respectVictimImmunity)
            {
                switch(W3DMGORIGIN)
                {
                    case W3DMGORIGIN_SKILL:
                    {
                        if(W3HasImmunity(victim,Immunity_Skills) )
                            return false;
                    }
                    case W3DMGORIGIN_ULTIMATE:
                    {
                        if(W3HasImmunity(victim,Immunity_Ultimates) )
                            return false;
                    }
                    case W3DMGORIGIN_ITEM:
                    {
                        if(W3HasImmunity(victim,Immunity_Items) )
                            return false;
                    }

                }


                switch(WAR3_DMGTYPE)
                {
                    case W3DMGTYPE_PHYSICAL:
                    {
                        if(W3HasImmunity(victim,Immunity_PhysicalDamage) )
                            return false;
                    }
                    case W3DMGTYPE_MAGIC:
                    {
                        if(W3HasImmunity(victim,Immunity_MagicDamage) )
                            return false;
                    }
                }
            }
            new bool:countAsFirstTriggeredDamage=GetNativeCell(9);
            nextDamageIsWarcraftDamage=!countAsFirstTriggeredDamage;

            new bool:settobullet=bool:W3GetDamageIsBullet(); //just in case someone dealt damage inside this forward and made it "not bullet"
            new Float:oldDamageMulti=damageModifierPercent; //nested damage woudl change the first triggering damage multi
            /////TO DO: IMPLEMENT PHYAISCAL AND MAGIC ARMOR

            actualdamagedealt=damage;
            decl oldcsarmor;
            if((WAR3_DMGTYPE==W3DMGTYPE_TRUEDMG||WAR3_DMGTYPE==W3DMGTYPE_MAGIC)&&War3_GetGame()==CS)
            {
                oldcsarmor=War3_GetCSArmor(victim);
                War3_SetCSArmor(victim,0) ;
            }

            nextDamageIsTrueDamage=(WAR3_DMGTYPE==W3DMGTYPE_TRUEDMG);

            if (damage < 1)
                damage = 1;

            new old_health = GetClientHealth(victim);
            Trace("DealDamage: victim=%N, health=%d, damage=%d, DamageStackLen=%d, weapon=%s", \
                  ValidClientIndex(victim), old_health, damage, DamageStackLen, weapon);

            new new_health = HurtPlayer(victim,damage,attacker,weapon, .type=dmg_type,
                                        .ignore_immunity=!respectVictimImmunity,
                                        .category=W3DMGORIGIN|WAR3_DMGTYPE,
                                        .no_translate=true);


            if (DamageStackLen >= -1)
            {
                actualdamagedealt = DamageStack[DamageStackLen+1];

                #if defined _TRACE
                    new calcDamage = old_health - new_health;
                    if (actualdamagedealt != calcDamage)
                    {
                        Trace("DealDamage Mismatch: victim=%N, damage=%d, calcDamagevictim=%d, actualdamagedealt=%d, DamageStackLen=%d, weapon=%s", \
                              ValidClientIndex(victim), damage, calcDamage, actualdamagedealt, DamageStackLen, weapon);
                    }
                    else
                    {
                        Trace("DealDamage Match: victim=%N, damage=%d, calcDamage=%d, actualdamagedealt=%d, DamageStackLen=%d, weapon=%s", \
                              ValidClientIndex(victim), damage, calcDamage, actualdamagedealt, DamageStackLen, weapon);
                    }
                #endif
            }
            else
            {
                actualdamagedealt = old_health - new_health;

                Trace("DealDamage underflow: victim=%N, damage=%d, actualdamagedealt=%d, DamageStackLen=%d, weapon=%s", \
                      ValidClientIndex(victim), damage, actualdamagedealt, DamageStackLen, weapon);
            }

            if((WAR3_DMGTYPE==W3DMGTYPE_TRUEDMG||WAR3_DMGTYPE==W3DMGTYPE_MAGIC)&&War3_GetGame()==CS)
                War3_SetCSArmor(victim,oldcsarmor);

            if(settobullet){
                W3ForceDamageIsBullet(); //just in case someone dealt damage inside this forward and made it "not bullet"
            }
            damageModifierPercent=oldDamageMulti;

            TraceReturn();
        }
        else
        {
            TraceReturn();
            return false;
        }
    }
    else
        ThrowError("Error War3_DealDamage OLD INCOMPATABLE RACE!!: params %d",numParams);

    return true;
}

public Action:OnPlayerTakeDamage(victim,&attacker,&inflictor,&Float:damage,&damagetype)
{
    new Action:result = Plugin_Continue;
    if(IsPlayerAlive(victim))
    {
        DamageStackLen++;

        TraceInto("War3Helper", "OnPlayerTakeDamage", "victim=%N, attacker=%N, inflictor=%d, damage=%f, DamageStackLen=%d", \
                  ValidClientIndex(victim), ValidClientIndex(attacker), inflictor, damage, DamageStackLen);

        if(DamageStackLen>=sizeof(DamageStackVictim))
        {
            LogError("OnPlayerTakeDamage: damage stack exceeded %d!", sizeof(DamageStackVictim));
            return Plugin_Changed;
        }
        else if (DamageStackLen < 0)
        {
            Trace("OnPlayerTakeDamage: damage stack underflow %d!", DamageStackLen);
            DamageStackLen = 0;
        }

        DamageStackVictim[DamageStackLen]=victim;

        damageModifierPercent=1.0;

        //set these first
        g_Lastdamagetype=damagetype;
        g_Lastinflictor=inflictor;

        new isBulletDamage=true;
        if(nextDamageIsWarcraftDamage || GetSuppressDamageForward())
        {
            nextDamageIsWarcraftDamage=false; //reset this and set g_LastDamageIsWarcraft to that value
            g_LastDamageIsWarcraft=true;
            isBulletDamage=false;
            if(!nextDamageIsTrueDamage)
                damage *= MagicArmorMulti(victim);
        }
        else //count as bullet now
        {
            g_LastDamageIsWarcraft=false;
            if(!nextDamageIsTrueDamage)
                damage *= PhysicalArmorMulti(victim);
        }

        new Action:dummyresult;
        Call_StartForward(FHOnW3TakeDmgAll);
        Call_PushCell(victim);
        Call_PushCell(attacker);
        Call_PushCell(damage);
        Call_Finish(dummyresult);

        if (isBulletDamage)
        {
            Call_StartForward(FHOnW3TakeDmgBullet);
            Call_PushCell(victim);
            Call_PushCell(attacker);
            Call_PushCell(damage);
            Call_Finish(dummyresult);
        }

        if (damageModifierPercent != 1.0)
        {
            damage=damage*damageModifierPercent;
            result = Plugin_Changed;
        }
    }

    TraceReturn("result=%d", result);
    return result;
}

public Action:OnPlayerRunCmd(client, &buttons, &impulse, Float:vel[3], Float:angles[3], &weapon)
{
	if(ValidPlayer(client)){
		bDucking[client]=((buttons & IN_DUCK) != 0);
		
		
		/*if(GetBuffHasOneTrue(client,bStunned)||GetBuffHasOneTrue(client,bDisarm)){
			if((buttons & IN_ATTACK) || (buttons & IN_ATTACK2))
			{
				buttons &= ~IN_ATTACK;
				buttons &= ~IN_ATTACK2;
			}
		}*/
	}
	return Plugin_Continue;
}

// Game Frame tracking
public OnGameFrame()
{
    for(new x=1;x<=MaxClients;x++)
    {
        if(IsClientConnected(x)&&IsClientInGame(x)&&IsPlayerAlive(x)&&!bIgnoreTrackGF[x])
        {
            //bDucking[x]=bool:GetEntData(x,DuckedOffset,1);
            GetClientEyeAngles(x,fAngle[x]);
            GetClientAbsOrigin(x,fPos[x]);
            new cur_wep=0;
            for(new s=0;s<10;s++)
            {
                // null values
                iWeapon[x][s][0]=0;
                iWeapon[x][s][1]=0;
            }
            for(new s=0;s<32;s++)
            {
                iAmmo[x][s]=GetEntData(x,AmmoOffset+(s*4),4);
            }
            for(new s=0;s<10;s++)
            {
                new ent=GetEntDataEnt2(x,MyWeaponsOffset+(s*4));
                if(ent>0)
                {
                    iWeapon[x][cur_wep][0]=ent;
                    iWeapon[x][cur_wep][1]=GetEntData(ent,Clip1Offset,4);
                    ++cur_wep;
                }
            }
        }
    }
}

War3Source_InitCVars()
{
    ChanceModifierPlasma=CreateConVar("war3_chancemodifier_directburn","0.0625","From 0.0 to 1.0 chance modifier for direct burns (plasma)");
    ChanceModifierBurn=CreateConVar("war3_chancemodifier_burn","0.125","From 0.0 to 1.0 chance modifier for burns");
    ChanceModifierHeavy=CreateConVar("war3_chancemodifier_heavy","0.125","From 0.0 to 1.0 chance modifier for heavy gun");
    ChanceModifierSentry=CreateConVar("war3_chancemodifier_sentry","0.125","From 0.0 to 1.0 chance modifier for sentry");
    ChanceModifierSentryRocket=CreateConVar("war3_chancemodifier_sentryrocket","0.0625","From 0.0 to 1.0 chance modifier for sentry rockets");
    ChanceModifierMedic=CreateConVar("war3_chancemodifier_medic","0.125","From 0.0 to 1.0 chance modifier for medic needle gun");
    ChanceModifierSMGSniper=CreateConVar("war3_chancemodifier_smgsniper","0.5","From 0.0 to 1.0 chance modifier for sniper SMG");

    hUseMetric=CreateConVar("war3_metric_system","0","Do you want use metric system? 1-Yes, 0-No");
    W3VarArr[hUseMetricCvar]=_:hUseMetric;

    return true;
}

bool:War3Source_InitHooks()
{
	RegConsoleCmd("+ability",War3Source_AbilityCommand);
	RegConsoleCmd("-ability",War3Source_AbilityCommand);
	RegConsoleCmd("+ability1",War3Source_AbilityCommand);
	RegConsoleCmd("-ability1",War3Source_AbilityCommand);
	RegConsoleCmd("+ability2",War3Source_AbilityCommand);
	RegConsoleCmd("-ability2",War3Source_AbilityCommand);
	RegConsoleCmd("+ability3",War3Source_AbilityCommand);
	RegConsoleCmd("-ability3",War3Source_AbilityCommand);
	RegConsoleCmd("+ability4",War3Source_AbilityCommand);
	RegConsoleCmd("-ability4",War3Source_AbilityCommand);
	RegConsoleCmd("+ability5",War3Source_AbilityCommand);
	RegConsoleCmd("-ability5",War3Source_AbilityCommand);
	RegConsoleCmd("+ability6",War3Source_AbilityCommand);
	RegConsoleCmd("-ability6",War3Source_AbilityCommand);
	return true;
}

bool:War3Source_InitNatives()
{
    CreateNative("W3ChanceModifier",Native_W3ChanceModifier);
    CreateNative("War3_ChanceModifier",Native_War3_ChanceModifier);
    CreateNative("W3LOS",NW3LOS);
    CreateNative("War3_GetAimEndPoint",NWar3_GetAimEndPoint);
    CreateNative("War3_GetAimTraceMaxLen",NWar3_GetAimTraceMaxLen);
    CreateNative("War3_GetTargetInViewCone",Native_War3_GetTargetInViewCone);
    CreateNative("War3_CachedAngle",Native_War3_CachedAngle);
    CreateNative("War3_CachedPosition",Native_War3_CachedPosition);
    CreateNative("War3_CachedDucking",Native_War3_CachedDucking);
    CreateNative("War3_CachedWeapon",Native_War3_CachedWeapon);
    CreateNative("War3_CachedClip1",Native_War3_CachedClip1);
    CreateNative("War3_CachedAmmo",Native_War3_CachedAmmo);
    CreateNative("War3_CachedDeadClip1",Native_War3_CachedDeadClip1);
    CreateNative("War3_CachedDeadAmmo",Native_War3_CachedDeadAmmo);
    CreateNative("War3_CachedDeadWeaponName",Native_War3_CDWN);
    CreateNative("War3_TF_ParticleToClient",Native_War3_TF_PTC);
    CreateNative("War3_HealToMaxHP",Native_War3_HTMHP);
    CreateNative("War3_HealToBuffHP",Native_War3_HTBHP);
    CreateNative("War3_DecreaseHP",Native_War3_DecreaseHP);
    CreateNative("War3_GetMaxHP",Native_War3_GetMaxHP);
    CreateNative("War3_SetMaxHP",Native_War3_SetMaxHP);	

    CreateNative("W3CreateEvent",NW3CreateEvent);//foritems

    CreateNative("War3_RegisterDelayTracker",NWar3_RegisterDelayTracker);
    CreateNative("War3_TrackDelay",NWar3_TrackDelay);
    CreateNative("War3_TrackDelayExpired",NWar3_TrackDelayExpired);

    CreateNative("W3GetW3Version",NW3GetW3Version);
    CreateNative("W3GetW3Revision",NW3GetW3Revision);

    CreateNative("W3GetDamageType",NW3GetDamageType);
    CreateNative("W3GetDamageInflictor",NW3GetDamageInflictor);
    CreateNative("W3GetDamageIsBullet",NW3GetDamageIsBullet);
    CreateNative("W3ForceDamageIsBullet",NW3ForceDamageIsBullet);

    CreateNative("War3_DealDamage",Native_War3_DealDamage);
    CreateNative("War3_DamageModPercent",Native_War3_DamageModPercent);
    CreateNative("War3_GetWar3DamageDealt",Native_War3_GetWar3DamageDealt);

    CreateNative("W3GetPhysicalArmorMulti",NW3GetPhysicalArmorMulti);//foritems
    CreateNative("W3GetMagicArmorMulti",NW3GetMagicArmorMulti);//foritems

    CreateNative("W3GetVar",NW3GetVar);
    CreateNative("W3SetVar",NW3SetVar);

    CreateNative("W3GetRaceString",NW3GetRaceString);
    CreateNative("W3GetRaceSkillString",NW3GetRaceSkillString);

    CreateNative("War3_AddRaceSkillT",NWar3_AddRaceSkillT);

    return true;
}

bool:War3Source_InitForwards()
{
    g_OnWar3UltimateCommandHandle=CreateGlobalForward("OnWar3UltimateCommand",ET_Ignore,Param_Cell,Param_Cell,Param_Cell,Param_Cell);
    g_OnAbilityCommandHandle=CreateGlobalForward("OnAbilityCommand",ET_Ignore,Param_Cell,Param_Cell,Param_Cell);
    g_OnWar3RaceSelectedHandle=CreateGlobalForward("OnWar3RaceSelected",ET_Ignore,Param_Cell,Param_Cell);
    g_OnWar3EventSpawnFH=CreateGlobalForward("OnWar3EventSpawn",ET_Ignore,Param_Cell);
    g_OnWar3EventDeathFH=CreateGlobalForward("OnWar3EventDeath",ET_Ignore,Param_Cell,Param_Cell);
    g_OnWar3PluginReadyHandle=CreateGlobalForward("OnWar3LoadRaceOrItemOrdered",ET_Ignore,Param_Cell);//ordered
    g_OnWar3PluginReadyHandle2=CreateGlobalForward("OnWar3LoadRaceOrItemOrdered2",ET_Ignore,Param_Cell);//ordered
    g_OnWar3PluginReadyHandle3=CreateGlobalForward("OnWar3PluginReady",ET_Ignore); //unodered rest of the items or races. backwards compatable..
    g_War3InterfaceExecFH=CreateGlobalForward("War3InterfaceExec",ET_Ignore);

    FHOnW3TakeDmgAll=CreateGlobalForward("OnW3TakeDmgAll",ET_Hook,Param_Cell,Param_Cell,Param_Cell);
    FHOnW3TakeDmgBullet=CreateGlobalForward("OnW3TakeDmgBullet",ET_Hook,Param_Cell,Param_Cell,Param_Cell);
    g_OnWar3EventPostHurtFH=CreateGlobalForward("OnWar3EventPostHurt",ET_Ignore,Param_Cell,Param_Cell,Param_Cell);
    g_War3GlobalEventFH=CreateGlobalForward("OnWar3Event",ET_Ignore,Param_Cell,Param_Cell);

    return true;
}

bool:War3Source_InitOffset()
{
	new bool:ret=true;

	MyWeaponsOffset=FindSendPropOffs("CBaseCombatCharacter","m_hMyWeapons");
	if(MyWeaponsOffset==-1)
	{
		PrintToServer("[War3Source] Error finding weapon list offset.");
		ret=false;
	}

	/*DuckedOffset=FindSendPropOffs("CBasePlayer","m_bDucked");
	if(DuckedOffset==-1)
	{
		PrintToServer("[War3Source] Error finding ducked offset.");
		ret=false;
	}*/

	Clip1Offset=FindSendPropOffs("CBaseCombatWeapon","m_iClip1");
	if(Clip1Offset==-1)
	{
		PrintToServer("[War3Source] Error finding clip1 offset.");
		ret=false;
	}

	AmmoOffset=FindSendPropOffs("CBasePlayer","m_iAmmo");
	if(AmmoOffset==-1)
	{
		PrintToServer("[War3Source] Error finding ammo offset.");
		ret=false;
	}

	return ret;
}

stock bool:ValidPlayer(client,bool:check_alive=false)
{
    if (IsValidClient(client))
        return (!check_alive || IsPlayerAlive(client));
    else
        return false;
}

public NWar3_RegisterDelayTracker(Handle:plugin,numParams)
{
	if(threadsLoaded<MAXTHREADS){
		return threadsLoaded++;
	}
	LogError("[War3Helper] DELAY TRACKER MAXTHREADS LIMIT REACHED! return -1");
	return -1;
}
public NWar3_TrackDelay(Handle:plugin,numParams)
{
	new index=GetNativeCell(1);
	new Float:delay=GetNativeCell(2);
	expireTime[index]=GetGameTime()+delay;
}
public NWar3_TrackDelayExpired(Handle:plugin,numParams)
{
	return GetGameTime()>expireTime[GetNativeCell(1)];
}

public NW3GetVar(Handle:plugin,numParams){
	return _:W3VarArr[War3Var:GetNativeCell(1)];
}
public NW3SetVar(Handle:plugin,numParams){
	W3VarArr[War3Var:GetNativeCell(1)]=GetNativeCell(2);
}

public NW3GetRaceString(Handle:plugin,numParams)
{
    new race=GetNativeCell(1);

    decl String:longbuf[1000];
    switch (RaceString:GetNativeCell(2))
    {
        case RaceName:
        {
            GetRaceName(race, longbuf, sizeof(longbuf));
        }            
        case RaceShortname:
        {
            GetRaceShortName(race, longbuf, sizeof(longbuf));
        }            
        case RaceDescription, RaceStory:
        {
            GetRaceDescription(race, longbuf, sizeof(longbuf));
        }
        default:
        {
            longbuf[0] = '\0'; 
        }
    }
    SetNativeString(3,longbuf,GetNativeCell(4));
}

public NW3GetRaceSkillString(Handle:plugin,numParams)
{
    new race=GetNativeCell(1);
    new skill=GetNativeCell(2);

    decl String:longbuf[1000];
    switch (SkillString:GetNativeCell(3))
    {
        case SkillName:
        {
            GetUpgradeName(race, skill, longbuf, sizeof(longbuf));
        }            
        case SkillDescription, SkillStory:
        {
            GetUpgradeDescription(race, skill, longbuf, sizeof(longbuf));
        }
        default:
        {
            longbuf[0] = '\0'; 
        }
    }
    SetNativeString(4,longbuf,GetNativeCell(5));
}

//translated
//native War3_AddRaceSkillT(raceid,String:SkillNameIdentifier[],bool:isult,maxskilllevel=DEF_MAX_SKILL_LEVEL,any:...);
public NWar3_AddRaceSkillT(Handle:plugin,numParams)
{
    new raceid=GetNativeCell(1);
    new String:skillname[64];
    GetNativeString(2,skillname,sizeof(skillname));
    new bool:isult=GetNativeCell(3);
    new maxskilllevel=GetNativeCell(4);

    new String:parm[8][64];
    if(numParams>4)
    {
        for(new arg=5, i=0; arg<=numParams; arg++, i++)
            GetNativeString(arg,parm[i],sizeof(parm[]));
    }

    new newskillnum = AddUpgrade(raceid,skillname,_:isult,.max_level=maxskilllevel,
                                 .p1=parm[0], .p2=parm[1], .p3=parm[2], .p4=parm[3],
                                 .p5=parm[4], .p6=parm[5], .p7=parm[6], .p8=parm[7]);

    decl String:description[256];
    GetUpgradeDescription(raceid, newskillnum, description, sizeof(description));

    new category = get_category(description, isult);
    if (category != _:isult)
        SetUpgradeCategory(raceid, newskillnum, category);

    return newskillnum;
}

stock get_category(const String:desc[], bool:isult=false)
{
    if (StrContains(desc, "+ability2") >= 0)
        return 4;
    else if (StrContains(desc, "+ability1") >= 0)
        return 3;
    else if (StrContains(desc, "+ability") >= 0)
        return 2;
    else
        return _:isult;
}

