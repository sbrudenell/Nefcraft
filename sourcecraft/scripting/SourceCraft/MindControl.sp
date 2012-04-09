/**
 * vim: set ai et ts=4 sw=4 :
 * File: MindControl.sp
 * Description: The MindControl upgrade for SourceCraft.
 * Author(s): -=|JFH|=-Naris
 */
 
#pragma semicolon 1

#include <sourcemod>
#include <sdktools>
#include <raytrace>
#include <range>

#include <tf2_stocks>
#include <tf2_player>
#include <tf2_objects>

#include "sc/SourceCraft"

#include "effect/Smoke"
#include "effect/RedGlow"
#include "effect/BlueGlow"
#include "effect/Lightning"
#include "effect/HaloSprite"
#include "effect/SendEffects"

enum command { update, remove, reset, find_controller, find_builder };

new const String:errorWav[] = "sc/perror.mp3";
new const String:deniedWav[] = "sc/buzz.wav";
new const String:controlWav[] = "sc/pteSum00.wav";

new Handle:m_StolenObjectList[MAXPLAYERS+1] = { INVALID_HANDLE, ... };

public Plugin:myinfo = 
{
    name = "SourceCraft Upgrade - MindControl",
    author = "-=|JFH|=-Naris",
    description = "The MindControl upgrade for SourceCraft.",
    version=SOURCECRAFT_VERSION,
    url = "http://jigglysfunhouse.net/"
};

public APLRes:AskPluginLoad2(Handle:myself, bool:late, String:error[], err_max)
{
    // Only load when running TF2
    if (GetGameType() == tf2)
    {
        // Register Natives
        CreateNative("MindControl",Native_MindControl);
        CreateNative("ControlObject",Native_ControlObject);
        CreateNative("ResetMindControlledObjects",Native_ResetMindControlledObjs);
        RegPluginLibrary("MindControl");
        return APLRes_Success;
    }
    else
    {
        strcopy(error, err_max, "Cannot Load MindControl on mods other than TF2");
        return APLRes_SilentFailure;
    }
}

public OnPluginStart()
{
    LoadTranslations("sc.common.phrases.txt");
    LoadTranslations("sc.mind_control.phrases.txt");

    if (GetGameType() == tf2)
    {
        if(!HookEvent("object_removed", ObjectDestroyed))
            SetFailState("Could not hook the object_removed event.");

        if(!HookEvent("object_destroyed", ObjectDestroyed))
            SetFailState("Could not hook the object_destroyed event.");

        if(!HookEventEx("player_death",CorrectDeathEvent, EventHookMode_Pre))
            SetFailState("Could not hook the player_death pre event.");
    }
}

public OnMapStart()
{
    SetupLightning(false, false);
    SetupHaloSprite(false, false);
    SetupSmokeSprite(false, false);
    SetupBlueGlow(false, false);
    SetupRedGlow(false, false);

    SetupSound(errorWav, true, false, false);
    SetupSound(deniedWav, true, false, false);
    SetupSound(controlWav, true, false, false);
}

public OnClientDisconnect(client)
{
    ResetMindControlledObjects(client, false);
}

public ObjectDestroyed(Handle:event,const String:name[],bool:dontBroadcast)
{
    new index = GetClientOfUserId(GetEventInt(event,"userid"));
    new object = GetEventInt(event,"index");
    new TFObjectType:type = TFObjectType:GetEventInt(event,"objecttype");
    ProcessMindControlledObjects(remove, object, index, type, INVALID_HANDLE);
}

public Action:CorrectDeathEvent(Handle:event,const String:name[],bool:dontBroadcast)
{
    new attacker = GetClientOfUserId(GetEventInt(event,"attacker"));
    if (attacker > 0)
    {
        decl String:weapon[64];
        GetEventString(event, "weapon", weapon, sizeof(weapon));

        if (StrEqual(weapon, "obj_sentrygun"))
        {
            new controller = ProcessMindControlledObjects(find_controller, -1, attacker, TFObject_Sentry);
            if (controller != attacker && IsValidClient(controller))
            {
                SetEventInt(event,"attacker",GetClientUserId(controller));
                return Plugin_Changed;
            }
        }
    }
    return Plugin_Continue;
}

bool:MindControl(client, Float:range, percent, &builder, &TFObjectType:type)
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
                    PrepareSound(deniedWav);
                    EmitSoundToClient(client,deniedWav);
                    return false;
                }
                else if (TF2_IsDisguised(pcond))
                    TF2_RemovePlayerDisguise(client);
            }
            case TFClass_Scout:
            {
                if (TF2_IsPlayerBonked(client))
                {
                    PrepareSound(deniedWav);
                    EmitSoundToClient(client,deniedWav);
                    return false;
                }
            }
        }
    }

    new target = TraceAimTarget(client);
    if (target >= 0)
    {
        new Float:clientLoc[3];
        GetClientAbsOrigin(client, clientLoc);

        new Float:targetLoc[3];
        TR_GetEndPosition(targetLoc);

        if (IsPointInRange(clientLoc,targetLoc,range))
        {
            new Float:distance=GetVectorDistance(clientLoc,targetLoc);
            if (GetRandomFloat(1.0,100.0) <= float(percent) * (1.0 - FloatDiv(distance,range)+0.20))
                return ControlObject(client, target, builder, type);
            else
            {
                PrepareSound(errorWav);
                EmitSoundToClient(client,errorWav); // Chance check failed.
            }
        }
        else
        {
            PrepareSound(errorWav);
            EmitSoundToClient(client,errorWav);
            DisplayMessage(client, Display_Ultimate,
                           "%t", "TargetIsTooFar");
        }
    }
    else
    {
        PrepareSound(deniedWav);
        EmitSoundToClient(client,deniedWav);
    }

    return false;
}

bool:ControlObject(client, target, &builder, &TFObjectType:type)
{
    if (IsValidEntity(target) && IsValidEdict(target))
    {
        type = TFObjectType:TF2_GetExtObjectTypeFromClass(target);
        if (type != TFObject_Unknown)
        {
            //Check to see if the object is still being built
            new placing = GetEntProp(target, Prop_Send, "m_bPlacing");
            new building = GetEntProp(target, Prop_Send, "m_bBuilding");
            new Float:complete = GetEntPropFloat(target, Prop_Send, "m_flPercentageConstructed");
            if (placing == 0 && building == 0 && complete >= 1.0)
            {
                //Find the owner of the object m_hBuilder holds the client index 1 to Maxplayers
                builder = GetEntPropEnt(target, Prop_Send, "m_hBuilder");

                if (builder > 0 && !GetImmunity(builder,Immunity_Ultimates))
                {
                    new team = GetClientTeam(client);
                    if (GetEntProp(target, Prop_Send, "m_iTeamNum") != team)
                    {
                        // Check to see if this target has already been controlled.
                        builder = ProcessMindControlledObjects(update, target, builder, type);

                        // Change the builder to client
                        // But, that seems to cause crashes when
                        // the "real" owner pushes the destruct button :(
                        //SetEntPropEnt(target, Prop_Send, "m_hBuilder", client);

                        //paint red or blue
                        SetEntProp(target, Prop_Send, "m_nSkin", (team==3)?1:0);

                        //Change TeamNum
                        SetVariantInt(team);
                        AcceptEntityInput(target, "TeamNum", -1, -1, 0);

                        //Same thing again but we are changing SetTeam
                        SetVariantInt(team);
                        AcceptEntityInput(target, "SetTeam", -1, -1, 0);

                        //If the gun is controlled, disable it.
                        if (type == TFObject_Sentry &&
                            GetEntProp(target, Prop_Send, "m_bPlayerControlled"))
                        {
                            SetEntProp(target, Prop_Send, "m_bDisabled", 1);
                        }

                        PrepareSound(controlWav);
                        EmitSoundToAll(controlWav,target);

                        new color[4] = { 0, 0, 0, 255 };
                        if (team == 3)
                            color[2] = 255; // Blue
                        else
                            color[0] = 255; // Red

                        new Float:clientLoc[3];
                        GetClientAbsOrigin(client, clientLoc);

                        new Float:targetLoc[3];
                        GetEntPropVector(target, Prop_Send, "m_vecOrigin", targetLoc);

                        TE_SetupBeamPoints(clientLoc, targetLoc, Lightning(), HaloSprite(),
                                           0, 1, 2.0, 10.0,10.0,2,50.0,color,255);
                        TE_SendEffectToAll();

                        TE_SetupSmoke(targetLoc,SmokeSprite(),8.0,2);
                        TE_SendEffectToAll();

                        TE_SetupGlowSprite(targetLoc,(team == 3) ? BlueGlow() : RedGlow(),
                                           5.0,5.0,255);
                        TE_SendEffectToAll();

                        new Float:splashDir[3];
                        splashDir[0] = 0.0;
                        splashDir[1] = 0.0;
                        splashDir[2] = 100.0;
                        TE_SetupEnergySplash(targetLoc, splashDir, true);

                        new target_ref = EntIndexToEntRef(target);
                        new Handle:timer = INVALID_HANDLE;
                        if (type == TFObject_Sentry)
                        {
                            timer = CreateTimer(0.1, CheckSentries, target_ref,
                                                TIMER_REPEAT|TIMER_FLAG_NO_MAPCHANGE);
                        }

                        // Create the Tracking Package
                        new Handle:pack = CreateDataPack();
                        WritePackCell(pack, target_ref);
                        WritePackCell(pack, builder);
                        WritePackCell(pack, _:timer);
                        WritePackCell(pack, _:type);

                        // And add it to the list
                        if (m_StolenObjectList[client] == INVALID_HANDLE)
                            m_StolenObjectList[client] = CreateArray();

                        PushArrayCell(m_StolenObjectList[client], pack);

                        return true;
                    }
                    else
                    {
                        PrepareSound(errorWav);
                        EmitSoundToClient(client,errorWav);
                        DisplayMessage(client, Display_Ultimate,
                                       "%t", "TargetBelongsToTeammate");
                    }
                }
                else
                {
                    PrepareSound(errorWav);
                    EmitSoundToClient(client,errorWav);
                    DisplayMessage(client, Display_Ultimate,
                                   "%t", "TargetIsImmune");
                }
            }
            else
            {
                PrepareSound(errorWav);
                EmitSoundToClient(client,errorWav);
                DisplayMessage(client, Display_Ultimate,
                               "%t", "TargetNotComplete");
            }
        }
        else
        {
            PrepareSound(deniedWav);
            EmitSoundToClient(client,deniedWav);
            DisplayMessage(client, Display_Ultimate,
                           "%t", "TargetInvalid");
        }
    }
    else
    {
        PrepareSound(deniedWav);
        EmitSoundToClient(client,deniedWav);
        DisplayMessage(client, Display_Ultimate,
                       "%t", "TargetInvalid");
    }
    return false;
}

public Action:CheckSentries(Handle:timer,any:ref)
{
    new object = EntRefToEntIndex(ref);
    if (object > 0 && IsValidEdict(object) && IsValidEntity(object))
    {
        // disable the sentry if it is controlled.
        if (GetEntProp(object, Prop_Send, "m_bPlayerControlled"))
            SetEntProp(object, Prop_Send, "m_bDisabled", 1);
        else            
            SetEntProp(object, Prop_Send, "m_bDisabled", 0);

        return Plugin_Continue;
    }

    ProcessMindControlledObjects(remove, .timer=timer);
    return Plugin_Stop;
}

ProcessMindControlledObjects(command:cmd, object=-1, builder=-1,
                             TFObjectType:type=TFObject_Unknown,
                             Handle:timer=INVALID_HANDLE)
{
    if (object > 0 || builder > 0)
    {
        for (new client=1;client<=MaxClients;client++)
        {
            if (m_StolenObjectList[client] != INVALID_HANDLE)
            {
                new size = GetArraySize(m_StolenObjectList[client]);
                for (new index = 0; index < size; index++)
                {
                    new Handle:pack = GetArrayCell(m_StolenObjectList[client], index);
                    if (pack != INVALID_HANDLE)
                    {
                        ResetPack(pack);
                        new pack_ref = ReadPackCell(pack);
                        new pack_target = EntRefToEntIndex(pack_ref);
                        new pack_builder = ReadPackCell(pack);
                        new Handle:pack_timer = Handle:ReadPackCell(pack);
                        new TFObjectType:pack_type = TFObjectType:ReadPackCell(pack);

                        new bool:found;
                        if (object > 0)
                            found = (object == pack_target);
                        else if (timer != INVALID_HANDLE)
                            found = (timer == pack_timer);
                        else
                            found = (builder == pack_builder && type == pack_type);

                        if (found)
                        {
                            if (cmd == remove || pack_target < 0)
                            {
                                CloseHandle(pack);
                                RemoveFromArray(m_StolenObjectList[client], index);
                                if (pack_timer != INVALID_HANDLE && pack_target > 0)
                                    KillTimer(pack_timer);
                            }
                            else if (cmd == reset)
                            {
                                CloseHandle(pack);
                                RemoveFromArray(m_StolenObjectList[client], index);
                                ResetObject(-1, pack_target, pack_builder, false);
                                if (pack_timer != INVALID_HANDLE && pack_target > 0)
                                    KillTimer(pack_timer);
                            }
                            else if (cmd == update)
                            {
                                // Update the tracking package
                                CloseHandle(pack);
                                pack = CreateDataPack();
                                WritePackCell(pack, pack_ref);
                                WritePackCell(pack, -1);
                                WritePackCell(pack, _:pack_timer);
                                WritePackCell(pack, _:type);
                                SetArrayCell(m_StolenObjectList[client], index, pack);
                            }
                            return (cmd == find_controller) ? index : pack_builder;
                        }
                    }
                }
            }
        }
    }
    return (cmd == find_controller) ? 0 : builder;
}

ResetMindControlledObjects(client, bool:kill)
{
    if (m_StolenObjectList[client] != INVALID_HANDLE)
    {
        new size = GetArraySize(m_StolenObjectList[client]);
        for (new index = 0; index < size; index++)
        {
            new Handle:pack = GetArrayCell(m_StolenObjectList[client], index);
            if (pack != INVALID_HANDLE)
            {
                ResetPack(pack);
                new target = EntRefToEntIndex(ReadPackCell(pack));
                new builder = ReadPackCell(pack);
                new Handle:timer = Handle:ReadPackCell(pack);
                CloseHandle(pack);

                ResetObject(client, target, builder, kill);
                //SetArrayCell(m_StolenObjectList[client], index, INVALID_HANDLE);
                if (timer != INVALID_HANDLE && target > 0)
                    KillTimer(timer);
            }
        }
        ClearArray(m_StolenObjectList[client]);
        CloseHandle(m_StolenObjectList[client]);
        m_StolenObjectList[client] = INVALID_HANDLE;
    }
}

ResetObject(client, target, builder, bool:kill)
{
    if (target > 0 && IsValidEntity(target) && IsValidEdict(target))
    {
        // Do we still own it?
        if (client <= 0 || GetEntPropEnt(target, Prop_Send, "m_hBuilder") ==  client)
        {
            // Is the round not ending and the builder valid?
            // (still around and still an engie)?
            if (kill || !IsValidClient(builder) ||
                TF2_GetPlayerClass(builder) != TFClass_Engineer)
            {
                AcceptEntityInput(target, "Kill", -1, -1, 0);
                //RemoveEdict(target); // Remove the object.
            }
            else
            {
                // Give it back.
                new team = GetClientTeam(builder);

                // Change the builder back
                SetEntPropEnt(target, Prop_Send, "m_hBuilder", builder);

                //paint red or blue
                SetEntProp(target, Prop_Send, "m_nSkin", (team==3)?1:0);

                //Change TeamNum
                SetVariantInt(team);
                AcceptEntityInput(target, "TeamNum", -1, -1, 0);

                //Same thing again but we are changing SetTeam
                SetVariantInt(team);
                AcceptEntityInput(target, "SetTeam", -1, -1, 0);

                //Make sure the gun is enabled, in case it was controlled.
                SetEntProp(target, Prop_Send, "m_bDisabled", 0);
            }
        }
    }
}

public Native_MindControl(Handle:plugin,numParams)
{
    new client = GetNativeCell(1);
    new Float:range = Float:GetNativeCell(2);
    new percent = GetNativeCell(3);
    new builder = GetNativeCellRef(4);
    new TFObjectType:type = GetNativeCellRef(5);
    new bool:success = MindControl(client,range,percent, builder, type);
    if (success)
    {
        SetNativeCellRef(4, builder);
        SetNativeCellRef(5, type);
    }
    return success;
}

public Native_ControlObject(Handle:plugin,numParams)
{
    new client = GetNativeCell(1);
    new target = GetNativeCell(2);
    new builder = GetNativeCellRef(3);
    new TFObjectType:type = GetNativeCellRef(4);
    new bool:success = ControlObject(client,target, builder, type);
    if (success)
    {
        SetNativeCellRef(3, builder);
        SetNativeCellRef(4, type);
    }
    return success;
}

public Native_ResetMindControlledObjs(Handle:plugin,numParams)
{
    new client = GetNativeCell(1);
    new bool:kill = bool:GetNativeCell(2);
    ResetMindControlledObjects(client,kill);
}

stock EmitSoundFromOrigin(const String:sound[],const Float:orig[3])
{
    EmitSoundToAll(sound,SOUND_FROM_WORLD,SNDCHAN_AUTO,SNDLEVEL_NORMAL,
                   SND_NOFLAGS,SNDVOL_NORMAL,SNDPITCH_NORMAL,-1,orig,
                   NULL_VECTOR,true,0.0);
}
