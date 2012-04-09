/**
 * vim: set ai et ts=4 sw=4 syntax=sourcepawn :
 * File: damage.inc
 * Description: Stocks to damage a player or an entity using a point_hurt entity.
 * Author(s): Naris with credits to PimpinJuice
 */
 
#if defined _damage_included
 #endinput
#endif
#define _damage_included

#include <entlimit>
#include <dmgbits>

stock g_damagePointRef = -1;

stock DamagePlayer(victim,damage,attacker=0,dmg_type=DMG_GENERIC,const String:weapon[]="")
{
    if (damage>0 && victim>0 && IsClientInGame(victim) && IsPlayerAlive(victim))
    {
        decl String:dmg_str[16];
        IntToString(damage,dmg_str,sizeof(dmg_str));

        decl String:dmg_type_str[32];
        IntToString(dmg_type,dmg_type_str,sizeof(dmg_type_str));

        new pointHurt = EntRefToEntIndex(g_damagePointRef);
        if (pointHurt < 1)
        {
            if (!IsEntLimitReached(.message="Unable to create point_hurt in DamagePlayer()"))
            {
                pointHurt=CreateEntityByName("point_hurt");
                if (pointHurt > 0 && IsValidEdict(pointHurt))
                {
                    //DispatchSpawn(pointHurt);
                    g_damagePointRef = EntIndexToEntRef(pointHurt);
                }
                else
                {
                    LogError("Unable to create point_hurt in DamagePlayer()");
                    return;
                }
            }
            else
            {
                LogError("EntLimit reached in DamagePlayer()");
                return;
            }
        }

        if (pointHurt > 0 && IsValidEdict(pointHurt))
        {
            decl String:targetname[16];
            Format(targetname,sizeof(targetname), "target%d", victim);

            DispatchKeyValue(victim,"targetname",targetname);
            DispatchKeyValue(pointHurt,"DamageTarget",targetname);
            DispatchKeyValue(pointHurt,"Damage",dmg_str);
            DispatchKeyValue(pointHurt,"DamageType",dmg_type_str);

            if (weapon[0] != '\0')
                DispatchKeyValue(pointHurt,"classname",weapon);

            DispatchSpawn(pointHurt);

            AcceptEntityInput(pointHurt,"Hurt",(attacker>0)?attacker:-1);
            DispatchKeyValue(pointHurt,"classname","point_hurt");

            IntToString(victim,targetname,sizeof(targetname));
            DispatchKeyValue(victim,"targetname",targetname);
        }
        else
            LogError("Unable to spawn point_hurt in DamagePlayer()");
    }
}

stock DamageEntity(victim,damage,attacker=0,dmg_type=DMG_GENERIC,const String:weapon[]="")
{
    if (damage>0 && victim>0 && IsValidEdict(victim))
    {
        decl String:dmg_str[16];
        IntToString(damage,dmg_str,sizeof(dmg_str));

        decl String:dmg_type_str[32];
        IntToString(dmg_type,dmg_type_str,sizeof(dmg_type_str));

        new pointHurt = EntRefToEntIndex(g_damagePointRef);
        if (pointHurt < 1)
        {
            if (!IsEntLimitReached(.message="Unable to create point_hurt in DamageEntity()"))
            {
                pointHurt=CreateEntityByName("point_hurt");
                if (pointHurt > 0 && IsValidEdict(pointHurt))
                {
                    DispatchSpawn(pointHurt);
                    g_damagePointRef = EntIndexToEntRef(pointHurt);
                }
                else
                {
                    LogError("Unable to create point_hurt in DamageEntity()");
                    return;
                }
            }
            else
            {
                LogError("EntLimit reached in DamagePlayer()");
                return;
            }
        }

        if (pointHurt > 0 && IsValidEdict(pointHurt))
        {
            decl String:targetname[16];
            Format(targetname, sizeof(targetname), "target%d", victim);

            DispatchKeyValue(victim,"targetname",targetname);
            DispatchKeyValue(pointHurt,"DamageTarget",targetname);
            DispatchKeyValue(pointHurt,"Damage",dmg_str);
            DispatchKeyValue(pointHurt,"DamageType",dmg_type_str);

            if (weapon[0] != '\0')
                DispatchKeyValue(pointHurt,"classname",weapon);

            //DispatchSpawn(pointHurt);

            AcceptEntityInput(pointHurt,"Hurt",(attacker>0)?attacker:-1);
            DispatchKeyValue(pointHurt,"classname","point_hurt");

            IntToString(victim,targetname,sizeof(targetname));
            DispatchKeyValue(victim,"targetname",targetname);
        }
        else
            LogError("Unable to spawn point_hurt in DamageEntity()");
    }
}

stock CleanupDamageEntity()
{
    new pointHurt = EntRefToEntIndex(g_damagePointRef);
    if (pointHurt > 0)
        RemoveEdict(pointHurt);

    g_damagePointRef = -1;
}
