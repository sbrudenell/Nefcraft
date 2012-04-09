/**
 * vim: set ai et ts=4 sw=4 syntax=sourcepawn :
 * File: weapons.inc
 * Description: Stocks to return information about weapons.
 * Author(s): Naris (Murray Wilson)
 */
 
#if defined _weapons_included
 #endinput
#endif
#define _weapons_included

#define MAX_SLOTS 48

stock GetActiveWeapon(client)
{
    return GetEntPropEnt(client, Prop_Send, "m_hActiveWeapon");
}

stock GetCurrentWeaponClass(client, String:name[], maxlength)
{
    new index = GetEntPropEnt(client, Prop_Send, "m_hActiveWeapon");
    if (index > 0 && IsValidEntity(index))
        GetEntityNetClass(index, name, maxlength);
    else
        name[0] = '\0';
}

stock GetCurrentWeaponEdictClass(client, String:name[], maxlength)
{
    new index = GetEntPropEnt(client, Prop_Send, "m_hActiveWeapon");
    if (index > 0 && IsValidEntity(index))
        GetEdictClassname(index, name, maxlength);
    else
        name[0] = '\0';
}

stock GetClip(weapon)
{
    return GetEntProp(weapon, Prop_Send, "m_iClip1", 1);
}

stock SetClip(weapon, amount)
{
    SetEntProp(weapon, Prop_Send, "m_iClip1", amount, 1);
}

stock GetAmmoType(weapon)
{
    return GetEntProp(weapon, Prop_Send, "m_iPrimaryAmmoType");
}

/**
 * Prevents a player from attacking for duration
 *
 * @param client 	Client
 * @param duration 	When to allow the next attack
 * @param melee 	Set melee attacks also?
 * @return			none
 */
stock SetNextAttack(client, Float:duration=0.0)
{
    new Float:nextAttack = GetGameTime()+duration;
    new offset=FindSendPropInfo("CBasePlayer", "m_hMyWeapons");
    for(new i=0;i<MAX_SLOTS;i++)
    {
        new weapon=GetEntDataEnt2(client,offset);
        if (weapon > 0 && IsValidEdict(weapon))
        {
            SetEntPropFloat(weapon, Prop_Send, "m_flNextSecondaryAttack", nextAttack);
            SetEntPropFloat(weapon, Prop_Send, "m_flNextPrimaryAttack", nextAttack);
        }
        offset += 4;
    }
}

