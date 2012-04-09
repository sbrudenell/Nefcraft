/**
 * vim: set ai et ts=4 sw=4 syntax=sourcepawn :
 * File: dissolve.inc
 * Description: dissolve a dead players ragdoll.
 * Author(s): L. Duke (converted to .inc by -=|JFH|=-Naris)
 */
 
#if defined _dissolve_included
 #endinput
#endif
#define _dissolve_included

#include <sourcemod>
#include <sdktools>
#include <entlimit>

stock DissolveRagdoll(client, Float:delay=0.0)
{
    if (delay > 0.0)
    {
        CreateTimer(delay, Dissolve, GetClientUserId(client)); 
    }
    else
    {
        Dissolve(INVALID_HANDLE, client);
    }
}

public Action:Dissolve(Handle:timer, any:userid)
{
    new client = GetClientOfUserId(userid);
    if (client > 0 && IsValidEntity(client))
    {
        if (IsEntLimitReached(.client=client,
                              .message="[DISSOLVE] Unable to spawn an env_entity_dissolver"))
        {
            return;
        }

        new ragdoll = GetEntPropEnt(client, Prop_Send, "m_hRagdoll");
        if (ragdoll < 0)
        {
            PrintToServer("[DISSOLVE] Could not get ragdoll for player!");  
            return;
        }

        new String:dname[32];
        Format(dname, sizeof(dname), "dis_%d", client);

        new ent = CreateEntityByName("env_entity_dissolver");
        if (ent > 0 && IsValidEdict(ent))
        {
            DispatchKeyValue(ragdoll, "targetname", dname);
            DispatchKeyValue(ent, "dissolvetype", "0");
            DispatchKeyValue(ent, "target", dname);
            AcceptEntityInput(ent, "Dissolve");
            AcceptEntityInput(ent, "kill");
        }
        else
            LogMessage("Unable to create env_entity_dissolver!");
    }
}
