/**
 * vim: set ai et ts=4 sw=4 syntax=sourcepawn :
 * File: giveammo.inc
 * Description: GiveAmmo function
 * Author(s): Naris (Murray Wilson)
 */
 
#if defined _giveammo_included
 #endinput
#endif
#define _giveammo_included

#include <gametype>

stock Handle:hGiveAmmo = INVALID_HANDLE;

stock GiveAmmo(client,ammotype,amount,bool:suppress)
{
    if (GetGameType() == tf2)
    {
        if (hGiveAmmo == INVALID_HANDLE)
        {
            StartPrepSDKCall(SDKCall_Entity);
            PrepSDKCall_SetFromConf(hGameConf,SDKConf_Signature,"GiveAmmo");
            PrepSDKCall_AddParameter(SDKType_PlainOldData,SDKPass_Plain);
            PrepSDKCall_AddParameter(SDKType_PlainOldData,SDKPass_Plain);
            PrepSDKCall_AddParameter(SDKType_PlainOldData,SDKPass_Plain);
            hGiveAmmo=EndPrepSDKCall();
        }

        if (hGiveAmmo != INVALID_HANDLE)
            SDKCall(hGiveAmmo,client,amount,ammotype,suppress);
        else
            LogError("Unable to GiveAmmo!");
    }
}
