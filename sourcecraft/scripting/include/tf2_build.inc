/**
 * vim: set ai et ts=4 sw=4 syntax=sourcepawn :
 * File: build.inc
 * Description: Functions to spawn buildings.
 * Author(s): -=|JFH|=-Naris
 */
 
#if defined _tf2_build_included
 #endinput
#endif
#define _tf2_build_included

#include <tf2_objects>

stock BuildSentry(hBuilder, const Float:fOrigin[3], const Float:fAngle[3], iLevel=1,
                  bool:bDisabled=false, bool:bMini=false, bool:bShielded=false,
                  iHealth=-1, iMaxHealth=-1, iShells=-1, iRockets=-1,
                  Float:flPercentage=1.0)
{
    static const Float:fBuildMaxs[3] = { 24.0, 24.0, 66.0 };
    //static const Float:fMdlWidth[3] = { 1.0, 0.5, 0.0 };

    new iTeam = GetClientTeam(hBuilder);

    new iSentryHealth;
    new iMaxSentryShells;
    new iMaxSentryRockets;
    if (iLevel >= 1 && iLevel <= 3)
    {
        iSentryHealth = TF2_SentryHealth[iLevel];
        iMaxSentryShells = TF2_MaxSentryShells[iLevel];
        iMaxSentryRockets = TF2_MaxSentryRockets[iLevel];
    }
    else if (iLevel == 4)
    {
        iLevel = 3;
        iSentryHealth = (TF2_SentryHealth[3]+TF2_SentryHealth[4])/2;
        iMaxSentryShells = (TF2_MaxSentryShells[3]+TF2_MaxSentryShells[4])/2;
        iMaxSentryRockets = (TF2_MaxSentryRockets[3]+TF2_MaxSentryRockets[4])/2;
    }
    else if (iLevel < 1 || bMini)
    {
        iLevel = 1;
        iSentryHealth = TF2_SentryHealth[0];
        iMaxSentryShells = TF2_MaxSentryShells[0];
        iMaxSentryRockets = TF2_MaxSentryRockets[0];
    }
    else
    {
        iLevel = 3;
        iSentryHealth = TF2_SentryHealth[4];
        iMaxSentryShells = TF2_MaxSentryShells[4];
        iMaxSentryRockets = TF2_MaxSentryRockets[4];
    }

    if (iShells < 0)
        iRockets = iMaxSentryRockets;

    if (iShells < 0)
        iShells = iMaxSentryShells;

    if (iMaxHealth < 0)
        iMaxHealth = iSentryHealth;

    if (iHealth < 0 || iHealth > iMaxHealth)
        iHealth = iMaxHealth;

    new iSentry = CreateEntityByName(TF2_ObjectClassNames[TFObject_Sentry]);
    if (iSentry > 0 && IsValidEdict(iSentry))
    {
        DispatchSpawn(iSentry);

        TeleportEntity(iSentry, fOrigin, fAngle, NULL_VECTOR);

        decl String:sModel[64];
        if (bMini)
            strcopy(sModel, sizeof(sModel),"models/buildables/sentry1.mdl");
        else
            Format(sModel, sizeof(sModel),"models/buildables/sentry%d.mdl", iLevel);

        SetEntityModel(iSentry,sModel);

        // m_bPlayerControlled is set to make m_bShielded work,
        // but it gets reset almost immediately :(

        SetEntProp(iSentry, Prop_Send, "m_iMaxHealth", 				        iMaxHealth, 4);
        SetEntProp(iSentry, Prop_Send, "m_iHealth", 					    iHealth, 4);
        SetEntProp(iSentry, Prop_Send, "m_bDisabled", 				        bDisabled, 2);
        SetEntProp(iSentry, Prop_Send, "m_bMiniBuilding", 				    bMini, 2);
        SetEntProp(iSentry, Prop_Send, "m_bShielded", 				        bShielded, 2);
        SetEntProp(iSentry, Prop_Send, "m_bPlayerControlled", 				bShielded, 2);
        SetEntProp(iSentry, Prop_Send, "m_iObjectType", 				    _:TFObject_Sentry, 1);
        SetEntProp(iSentry, Prop_Send, "m_iUpgradeLevel", 			        iLevel, 4);
        SetEntProp(iSentry, Prop_Send, "m_iAmmoRockets", 				    iRockets, 4);
        SetEntProp(iSentry, Prop_Send, "m_iAmmoShells" , 				    iShells, 4);
        SetEntProp(iSentry, Prop_Send, "m_iObjectMode", 				    0, 2);
        SetEntProp(iSentry, Prop_Send, "m_iUpgradeMetal", 			        0, 2);
        SetEntProp(iSentry, Prop_Send, "m_bBuilding", 				        0, 2);
        SetEntProp(iSentry, Prop_Send, "m_bPlacing", 					    0, 2);
        SetEntProp(iSentry, Prop_Send, "m_iState", 					        1, 1);
        SetEntProp(iSentry, Prop_Send, "m_bHasSapper", 				        0, 2);
        SetEntProp(iSentry, Prop_Send, "m_nNewSequenceParity", 		        4, 4);
        SetEntProp(iSentry, Prop_Send, "m_nResetEventsParity", 		        4, 4);
        SetEntProp(iSentry, Prop_Send, "m_bServerOverridePlacement", 	    1, 1);
        SetEntProp(iSentry, Prop_Send, "m_nSequence",                       0);

        SetEntPropEnt(iSentry, Prop_Send, "m_hBuilder", 	                hBuilder);

        SetEntPropFloat(iSentry, Prop_Send, "m_flPercentageConstructed", 	flPercentage);
        SetEntPropFloat(iSentry, Prop_Send, "m_flModelWidthScale", 	        1.0);
        SetEntPropFloat(iSentry, Prop_Send, "m_flPlaybackRate", 			1.0);
        SetEntPropFloat(iSentry, Prop_Send, "m_flCycle", 					0.0);

        SetEntPropVector(iSentry, Prop_Send, "m_vecOrigin", 			    fOrigin);
        SetEntPropVector(iSentry, Prop_Send, "m_angRotation", 		        fAngle);
        SetEntPropVector(iSentry, Prop_Send, "m_vecBuildMaxs", 		        fBuildMaxs);
        //SetEntDataVector(iSentry, FindSendPropOffs("CObjectSentrygun","m_flModelWidthScale"),	fMdlWidth, true);

        if (bMini)
        {
            SetEntProp(iSentry, Prop_Send, "m_nSkin", 					    iTeam, 1);
            SetEntProp(iSentry, Prop_Send, "m_nBody", 					    5, 1);
        }
        else
        {
            SetEntProp(iSentry, Prop_Send, "m_nSkin", 					    (iTeam-2), 1);
            SetEntProp(iSentry, Prop_Send, "m_nBody", 					    0, 1);
        }

        SetVariantInt(iTeam);
        AcceptEntityInput(iSentry, "TeamNum", -1, -1, 0);

        SetVariantInt(iTeam);
        AcceptEntityInput(iSentry, "SetTeam", -1, -1, 0);

        SetVariantInt(hBuilder);
        AcceptEntityInput(iSentry, "SetBuilder", -1, -1, 0);

        new Handle:event = CreateEvent("player_builtobject");
        if (event != INVALID_HANDLE)
        {
            SetEventInt(event, "userid", GetClientUserId(hBuilder));
            SetEventInt(event, "object", _:TFObject_Sentry);
            SetEventInt(event, "index", iSentry);
            SetEventBool(event, "sourcemod", true);
            FireEvent(event);
        }
    }
    return iSentry;
}

stock BuildDispenser(hBuilder, const Float:fOrigin[3], const Float:fAngle[3], iLevel=1,
                     bool:iDisabled=false, iHealth=-1, iMaxHealth=-1, iMetal=-1,
                     Float:flPercentage=1.0, TFExtObject:type=TFExtObject_Dispenser)
{
    static const Float:fBuildMaxs[3] = { 24.0, 24.0, 66.0 };

    new iTeam = GetClientTeam(hBuilder);

    if (iMaxHealth < 0)
        iMaxHealth = 150;

    if (iHealth < 0 || iHealth > iMaxHealth)
        iHealth = iMaxHealth;

    if (iMetal < 0)
        iMetal = 1000;

    if (iLevel < 1)
        iLevel = 1;
    else if (iLevel > 3)
        iLevel = 3;

    new iDispenser = CreateEntityByName(TF2_ObjectClassNames[TFObject_Dispenser]);
    if (iDispenser > 0 && IsValidEdict(iDispenser))
    {
        DispatchSpawn(iDispenser);

        TeleportEntity(iDispenser, fOrigin, fAngle, NULL_VECTOR);

        decl String:sModel[64];
        switch (type)
        {
            case TFObject_Amplifier:
            {
                strcopy(sModel, sizeof(sModel),"models/buildables/amplifier_test/amplifier.mdl");
            }
            case TFObject_RepairNode:
            {
                if (iLevel > 1)
                    Format(sModel, sizeof(sModel),"models/buildables/repair_level%d.mdl", iLevel);
                else
                    strcopy(sModel, sizeof(sModel),"models/buildables/dispenser_light.mdl");
            }
            default:
            {
                if (iLevel > 1)
                    Format(sModel, sizeof(sModel),"models/buildables/dispenser_lvl%d_light.mdl", iLevel);
                else
                    strcopy(sModel, sizeof(sModel),"models/buildables/dispenser_light.mdl");
            }
        }

        SetEntityModel(iDispenser,sModel);

        SetEntProp(iDispenser, Prop_Send, "m_iMaxHealth", 				        iMaxHealth, 4);
        SetEntProp(iDispenser, Prop_Send, "m_iHealth", 				            iHealth, 4);
        SetEntProp(iDispenser, Prop_Send, "m_iAmmoMetal", 				        iMetal, 4);
        SetEntProp(iDispenser, Prop_Send, "m_bDisabled", 				        iDisabled, 2);
        SetEntProp(iDispenser, Prop_Send, "m_iObjectType", 			            _:TFObject_Dispenser, 1);
        SetEntProp(iDispenser, Prop_Send, "m_nSkin", 					        (iTeam-2), 1);
        SetEntProp(iDispenser, Prop_Send, "m_iUpgradeLevel", 			        iLevel, 4);
        SetEntProp(iDispenser, Prop_Send, "m_iObjectMode", 				        0, 2);
        SetEntProp(iDispenser, Prop_Send, "m_bBuilding", 				        0, 2);
        SetEntProp(iDispenser, Prop_Send, "m_bPlacing", 				        0, 2);
        SetEntProp(iDispenser, Prop_Send, "m_bHasSapper", 				        0, 2);
        SetEntProp(iDispenser, Prop_Send, "m_nNewSequenceParity", 		        4, 4);
        SetEntProp(iDispenser, Prop_Send, "m_nResetEventsParity", 		        4, 4);
        SetEntProp(iDispenser, Prop_Send, "m_bServerOverridePlacement",         1, 1);
        SetEntProp(iDispenser, Prop_Send, "m_nSequence",                        0);

        SetEntPropEnt(iDispenser, Prop_Send, "m_hBuilder",                      hBuilder);

        SetEntPropFloat(iDispenser, Prop_Send, "m_flPercentageConstructed", 	flPercentage);
        SetEntPropFloat(iDispenser, Prop_Send, "m_flModelWidthScale", 	        1.0);
        SetEntPropFloat(iDispenser, Prop_Send, "m_flPlaybackRate", 			    1.0);
        SetEntPropFloat(iDispenser, Prop_Send, "m_flCycle", 					0.0);

        SetEntPropVector(iDispenser, Prop_Send, "m_vecOrigin", 		            fOrigin);
        SetEntPropVector(iDispenser, Prop_Send, "m_angRotation", 		        fAngle);
        SetEntPropVector(iDispenser, Prop_Send, "m_vecBuildMaxs",		        fBuildMaxs);

        SetVariantInt(iTeam);
        AcceptEntityInput(iDispenser, "TeamNum", -1, -1, 0);

        SetVariantInt(iTeam);
        AcceptEntityInput(iDispenser, "SetTeam", -1, -1, 0);

        SetVariantInt(hBuilder);
        AcceptEntityInput(iDispenser, "SetBuilder", -1, -1, 0); 

        if (!iDisabled)
            AcceptEntityInput(iDispenser, "TurnOn");

        new Handle:event = CreateEvent("player_builtobject");
        if (event != INVALID_HANDLE)
        {
            SetEventInt(event, "userid", GetClientUserId(hBuilder));
            SetEventInt(event, "object", _:TFObject_Dispenser);
            SetEventInt(event, "index", iDispenser);
            SetEventBool(event, "sourcemod", true);
            FireEvent(event);
        }
    }
    return iDispenser;
}

stock BuildTeleporterEntry(hBuilder, const Float:fOrigin[3], const Float:fAngle[3],
                           iLevel=1, bool:iDisabled=false, iHealth=-1, iMaxHealth=-1,
                           Float:flPercentage=1.0)
{
    static const Float:fBuildMaxs[3] = { 28.0, 28.0, 66.0 };
    //static const Float:fMdlWidth[3] = { 1.0, 0.5, 0.0 };

    new iTeam = GetClientTeam(hBuilder);

    if (iMaxHealth < 0)
        iMaxHealth = 150;

    if (iHealth < 0 || iHealth > iMaxHealth)
        iHealth = iMaxHealth;

    if (iLevel < 1)
        iLevel = 1;
    else if (iLevel > 3)
        iLevel = 3;

    new iTeleporter = CreateEntityByName(TF2_ObjectClassNames[TFObject_Teleporter]);
    if (iTeleporter > 0 && IsValidEdict(iTeleporter))
    {
        DispatchSpawn(iTeleporter);

        TeleportEntity(iTeleporter, fOrigin, fAngle, NULL_VECTOR);

        SetEntityModel(iTeleporter,"models/buildables/teleporter_light.mdl");

        SetEntProp(iTeleporter, Prop_Send, "m_iMaxHealth", 				        iMaxHealth, 4);
        SetEntProp(iTeleporter, Prop_Send, "m_iHealth", 					    iHealth, 4);
        SetEntProp(iTeleporter, Prop_Send, "m_bDisabled", 				        iDisabled, 2);
        SetEntProp(iTeleporter, Prop_Send, "m_iObjectType", 				    _:TFObject_Teleporter, 1);
        SetEntProp(iTeleporter, Prop_Send, "m_nSkin", 					        (iTeam-2), 1);
        SetEntProp(iTeleporter, Prop_Send, "m_iUpgradeLevel", 			        iLevel, 4);
        SetEntProp(iTeleporter, Prop_Send, "m_iObjectMode", 				    0, TFObjectMode_Entrance);
        SetEntProp(iTeleporter, Prop_Send, "m_bBuilding", 				        0, 2);
        SetEntProp(iTeleporter, Prop_Send, "m_bPlacing", 					    0, 2);
        SetEntProp(iTeleporter, Prop_Send, "m_bHasSapper", 				        0, 2);
        SetEntProp(iTeleporter, Prop_Send, "m_nNewSequenceParity", 		        4, 4);
        SetEntProp(iTeleporter, Prop_Send, "m_nResetEventsParity", 		        4, 4);
        SetEntProp(iTeleporter, Prop_Send, "m_bServerOverridePlacement", 	    1, 1);
        SetEntProp(iTeleporter, Prop_Send, "m_iState", 	                        1, 1);
        SetEntProp(iTeleporter, Prop_Send, "m_nSequence",                       0);

        SetEntPropEnt(iTeleporter, Prop_Send, "m_hBuilder", 	                hBuilder);

        SetEntPropFloat(iTeleporter, Prop_Send, "m_flPercentageConstructed", 	flPercentage);
        SetEntPropFloat(iTeleporter, Prop_Send, "m_flModelWidthScale", 	        1.0);
        SetEntPropFloat(iTeleporter, Prop_Send, "m_flPlaybackRate", 			1.0);
        SetEntPropFloat(iTeleporter, Prop_Send, "m_flCycle", 					0.0);

        SetEntPropVector(iTeleporter, Prop_Send, "m_vecOrigin", 			    fOrigin);
        SetEntPropVector(iTeleporter, Prop_Send, "m_angRotation", 		        fAngle);
        SetEntPropVector(iTeleporter, Prop_Send, "m_vecBuildMaxs", 		        fBuildMaxs);

        SetVariantInt(iTeam);
        AcceptEntityInput(iTeleporter, "TeamNum", -1, -1, 0);

        SetVariantInt(iTeam);
        AcceptEntityInput(iTeleporter, "SetTeam", -1, -1, 0); 

        SetVariantInt(hBuilder);
        AcceptEntityInput(iTeleporter, "SetBuilder", -1, -1, 0); 

        if (!iDisabled)
            AcceptEntityInput(iTeleporter, "TurnOn");

        new Handle:event = CreateEvent("player_builtobject");
        if (event != INVALID_HANDLE)
        {
            SetEventInt(event, "userid", GetClientUserId(hBuilder));
            SetEventInt(event, "object", _:TFObject_TeleporterEntry);
            SetEventInt(event, "index", iTeleporter);
            SetEventBool(event, "sourcemod", true);
            FireEvent(event);
        }
    }
    return iTeleporter;
}

stock BuildTeleporterExit(hBuilder, const Float:fOrigin[3], const Float:fAngle[3],
                          iLevel=1, bool:iDisabled=false, iHealth=-1, iMaxHealth=-1,
                          Float:flPercentage=1.0)
{
    static const Float:fBuildMaxs[3] = { 28.0, 28.0, 66.0 };

    new iTeam = GetClientTeam(hBuilder);

    if (iMaxHealth < 0)
        iMaxHealth = 150;

    if (iHealth < 0 || iHealth > iMaxHealth)
        iHealth = iMaxHealth;

    if (iLevel < 1)
        iLevel = 1;
    else if (iLevel > 3)
        iLevel = 3;

    new iTeleporter = CreateEntityByName(TF2_ObjectClassNames[TFObject_Teleporter]);
    if (iTeleporter > 0 && IsValidEdict(iTeleporter))
    {
        DispatchSpawn(iTeleporter);

        TeleportEntity(iTeleporter, fOrigin, fAngle, NULL_VECTOR);

        SetEntityModel(iTeleporter,"models/buildables/teleporter_light.mdl");

        SetEntProp(iTeleporter, Prop_Send, "m_iMaxHealth", 				        iMaxHealth, 4);
        SetEntProp(iTeleporter, Prop_Send, "m_iHealth", 				        iHealth, 4);
        SetEntProp(iTeleporter, Prop_Send, "m_bDisabled", 				        iDisabled, 2);
        SetEntProp(iTeleporter, Prop_Send, "m_iObjectType", 			        _:TFObject_Teleporter, 1);
        SetEntProp(iTeleporter, Prop_Send, "m_nSkin", 					        (iTeam-2), 1);
        SetEntProp(iTeleporter, Prop_Send, "m_iUpgradeLevel", 			        iLevel, 4);
        SetEntProp(iTeleporter, Prop_Send, "m_iObjectMode", 				    1, TFObjectMode_Exit);
        SetEntProp(iTeleporter, Prop_Send, "m_bBuilding", 				        0, 2);
        SetEntProp(iTeleporter, Prop_Send, "m_bPlacing", 				        0, 2);
        SetEntProp(iTeleporter, Prop_Send, "m_bHasSapper", 				        0, 2);
        SetEntProp(iTeleporter, Prop_Send, "m_nNewSequenceParity", 		        4, 4 );
        SetEntProp(iTeleporter, Prop_Send, "m_nResetEventsParity", 		        4, 4 );
        SetEntProp(iTeleporter, Prop_Send, "m_bServerOverridePlacement", 	    1, 1);
        SetEntProp(iTeleporter, Prop_Send, "m_iState", 	                        1, 1);
        SetEntProp(iTeleporter, Prop_Send, "m_nSequence",                       0);

        SetEntPropEnt(iTeleporter, Prop_Send, "m_hBuilder", 	                hBuilder);

        SetEntPropFloat(iTeleporter, Prop_Send, "m_flPercentageConstructed", 	flPercentage);
        SetEntPropFloat(iTeleporter, Prop_Send, "m_flModelWidthScale", 	        1.0);
        SetEntPropFloat(iTeleporter, Prop_Send, "m_flPlaybackRate", 			1.0);
        SetEntPropFloat(iTeleporter, Prop_Send, "m_flCycle", 					0.0);

        SetEntPropVector(iTeleporter, Prop_Send, "m_vecOrigin", 			    fOrigin);
        SetEntPropVector(iTeleporter, Prop_Send, "m_angRotation", 		        fAngle);
        SetEntPropVector(iTeleporter, Prop_Send, "m_vecBuildMaxs", 		        fBuildMaxs);

        SetVariantInt(iTeam);
        AcceptEntityInput(iTeleporter, "TeamNum", -1, -1, 0);

        SetVariantInt(iTeam);
        AcceptEntityInput(iTeleporter, "SetTeam", -1, -1, 0); 

        SetVariantInt(hBuilder);
        AcceptEntityInput(iTeleporter, "SetBuilder", -1, -1, 0); 

        if (!iDisabled)
            AcceptEntityInput(iTeleporter, "TurnOn");

        new Handle:event = CreateEvent("player_builtobject");
        if (event != INVALID_HANDLE)
        {
            SetEventInt(event, "userid", GetClientUserId(hBuilder));
            SetEventInt(event, "object", _:TFObject_TeleporterExit);
            SetEventInt(event, "index", iTeleporter);
            SetEventBool(event, "sourcemod", true);
            FireEvent(event);
        }
    }
    return iTeleporter;
}

