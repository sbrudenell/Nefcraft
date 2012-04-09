/**
 * vim: set ai et ts=4 sw=4 syntax=sourcepawn :
 * File: tf2_player.inc
 * Description: Stocks to return information about TF2 player condition, etc.
 * Author(s): Naris (Murray Wilson)
 */
 
#if defined _tf2_player_included
 #endinput
#endif
#define _tf2_player_included

#include <tf2_stocks>
#include <weapons>

stock const String:TF2_TeamName[TFTeam][] = { "Unassigned", "Spectator", "Red", "Blue" };
#define TF2_GetTeamName(%1) TF2_TeamName[%1]

stock const String:TF2_ClassName[TFClassType][] = {"", "Scout", "Sniper", "Soldier", "Demoman", "Medic",
                                                   "Heavy", "Pyro", "Spy", "Engineer" };
#define TF2_GetClassName(%1) TF2_ClassName[%1]

stock const TF2_ClassHealth[TFClassType] = { 0, 125, 125, 200, 175, 150, 300, 175, 125, 125 };
#define TF2_GetClassHealth(%1) TF2_ClassHealth[%1]

//stock const Float:TF2_ClassSpeed[TFClassType] = { 0.0, 400.0, 300.0, 240.0, 280.0, 320.0, 230.0, 300.0, 300.0, 300.0 };
stock const Float:TF2_ClassSpeed[10] = { 0.0, 400.0, 300.0, 240.0, 280.0, 320.0, 230.0, 300.0, 300.0, 300.0 };
#define TF2_GetClassSpeed(%1) TF2_ClassSpeed[%1]

#define TF2_IsSlowed(%1)                    (((%1) & TF_CONDFLAG_SLOWED) != TF_CONDFLAG_NONE)
#define TF2_IsZoomed(%1)                    (((%1) & TF_CONDFLAG_ZOOMED) != TF_CONDFLAG_NONE)
#define TF2_IsDisguising(%1)                (((%1) & TF_CONDFLAG_DISGUISING) != TF_CONDFLAG_NONE)
#define TF2_IsDisguised(%1)                 (((%1) & TF_CONDFLAG_DISGUISED) != TF_CONDFLAG_NONE)
#define TF2_IsCloaked(%1)                   (((%1) & TF_CONDFLAG_CLOAKED) != TF_CONDFLAG_NONE)
#define TF2_IsUbercharged(%1)               (((%1) & TF_CONDFLAG_UBERCHARGED) != TF_CONDFLAG_NONE)
#define TF2_IsTeleportedGlow(%1)            (((%1) & TF_CONDFLAG_TELEPORTGLOW) != TF_CONDFLAG_NONE)
#define TF2_IsTaunting(%1)                  (((%1) & TF_CONDFLAG_TAUNTING) != TF_CONDFLAG_NONE)
#define TF2_IsUberchargeFading(%1)          (((%1) & TF_CONDFLAG_UBERCHARGEFADE) != TF_CONDFLAG_NONE)
#define TF2_IsCloakFlicker(%1)              (((%1) & TF_CONDFLAG_CLOAKFLICKER) != TF_CONDFLAG_NONE)
#define TF2_IsTeleporting(%1)               (((%1) & TF_CONDFLAG_TELEPORTING) != TF_CONDFLAG_NONE)
#define TF2_IsKritzkrieged(%1)              (((%1) & TF_CONDFLAG_KRITZKRIEGED) != TF_CONDFLAG_NONE)
#define TF2_IsDeadRingered(%1)              (((%1) & TF_CONDFLAG_DEADRINGERED) != TF_CONDFLAG_NONE)
#define TF2_IsBonked(%1)                    (((%1) & TF_CONDFLAG_BONKED) != TF_CONDFLAG_NONE)
#define TF2_IsDazed(%1)                     (((%1) & TF_CONDFLAG_DAZED) != TF_CONDFLAG_NONE)
#define TF2_IsBuffed(%1)                    (((%1) & TF_CONDFLAG_BUFFED) != TF_CONDFLAG_NONE)
#define TF2_IsCharging(%1)                  (((%1) & TF_CONDFLAG_CHARGING) != TF_CONDFLAG_NONE)
#define TF2_IsDemoBuff(%1)                  (((%1) & TF_CONDFLAG_DEMOBUFF) != TF_CONDFLAG_NONE)
#define TF2_IsCritCola(%1)                  (((%1) & TF_CONDFLAG_CRITCOLA) != TF_CONDFLAG_NONE)
#define TF2_IsHealing(%1)                   (((%1) & TF_CONDFLAG_HEALING) != TF_CONDFLAG_NONE)
#define TF2_IsOnFire(%1)                    (((%1) & TF_CONDFLAG_ONFIRE) != TF_CONDFLAG_NONE)
#define TF2_IsOverhealed(%1)                (((%1) & TF_CONDFLAG_OVERHEALED) != TF_CONDFLAG_NONE)
#define TF2_IsJarated(%1)                   (((%1) & TF_CONDFLAG_JARATED) != TF_CONDFLAG_NONE)
#define TF2_IsBleeding(%1)                  (((%1) & TF_CONDFLAG_BLEEDING) != TF_CONDFLAG_NONE)
#define TF2_IsDefenseBuffed(%1)             (((%1) & TF_CONDFLAG_DEFENSEBUFFED) != TF_CONDFLAG_NONE)
#define TF2_IsMilked(%1)                    (((%1) & TF_CONDFLAG_MILKED) != TF_CONDFLAG_NONE)

#define TF2_IsPlayerSlowed(%1)              TF2_IsSlowed(TF2_GetPlayerConditionFlags(%1))
#define TF2_IsPlayerZoomed(%1)              TF2_IsZoomed(TF2_GetPlayerConditionFlags(%1))
#define TF2_IsPlayerDisguising(%1)          TF2_IsDisguising(TF2_GetPlayerConditionFlags(%1))
#define TF2_IsPlayerDisguised(%1)           TF2_IsDisguised(TF2_GetPlayerConditionFlags(%1))
#define TF2_IsPlayerCloaked(%1)             TF2_IsCloaked(TF2_GetPlayerConditionFlags(%1))
#define TF2_IsPlayerUbercharged(%1)         TF2_IsUbercharged(TF2_GetPlayerConditionFlags(%1))
#define TF2_IsPlayerTeleportedGlow(%1)      TF2_IsTeleportedGlow(TF2_GetPlayerConditionFlags(%1))
#define TF2_IsPlayerTaunting(%1)            TF2_IsTaunting(TF2_GetPlayerConditionFlags(%1))
#define TF2_IsPlayerUberchargeFading(%1)    TF2_IsUberchargeFading(TF2_GetPlayerConditionFlags(%1))
#define TF2_IsPlayerCloakFlicker(%1)        TF2_IsCloakFlicker(TF2_GetPlayerConditionFlags(%1))
#define TF2_IsPlayerTeleporting(%1)         TF2_IsTeleporting(TF2_GetPlayerConditionFlags(%1))
#define TF2_IsPlayerKritzkrieged(%1)        TF2_IsKritzkrieged(TF2_GetPlayerConditionFlags(%1))
#define TF2_IsPlayerDeadRingered(%1)        TF2_IsDeadRingered(TF2_GetPlayerConditionFlags(%1))
#define TF2_IsPlayerBonked(%1)              TF2_IsBonked(TF2_GetPlayerConditionFlags(%1))
#define TF2_IsPlayerDazed(%1)               TF2_IsDazed(TF2_GetPlayerConditionFlags(%1))
#define TF2_IsPlayerBuffed(%1)              TF2_IsBuffed(TF2_GetPlayerConditionFlags(%1))
#define TF2_IsPlayerCharging(%1)            TF2_IsCharging(TF2_GetPlayerConditionFlags(%1))
#define TF2_IsPlayerDemoBuff(%1)            TF2_IsDemoBuff(TF2_GetPlayerConditionFlags(%1))
#define TF2_IsPlayerCritCola(%1)            TF2_IsCritCola(TF2_GetPlayerConditionFlags(%1))
#define TF2_IsPlayerHealing(%1)             TF2_IsHealing(TF2_GetPlayerConditionFlags(%1))
#define TF2_IsPlayerOnFire(%1)              TF2_IsOnFire(TF2_GetPlayerConditionFlags(%1))
#define TF2_IsPlayerOverhealed(%1)          TF2_IsOverhealed(TF2_GetPlayerConditionFlags(%1))
#define TF2_IsPlayerJarated(%1)             TF2_IsJarated(TF2_GetPlayerConditionFlags(%1))
#define TF2_IsPlayerBleeding(%1)            TF2_IsBleeding(TF2_GetPlayerConditionFlags(%1))
#define TF2_IsPlayerDefenseBuffed(%1)       TF2_IsDefenseBuffed(TF2_GetPlayerConditionFlags(%1))
#define TF2_IsPlayerMilked(%1)              TF2_IsMilked(TF2_GetPlayerConditionFlags(%1))

stock Float:TF2_GetPlayerSpeed(client)
{
    new Float:speed;
    new TFClassType:class = TF2_GetPlayerClass(client);
    switch (class)
    {
        case TFClass_Sniper:
        {
            // Is the Snipers's rifle zoomed?
            speed = TF2_IsPlayerSlowed(client) ? 80.0 : 300.0; // TF2_GetClassSpeed(class);
        }
        case TFClass_Soldier:
        {
            new weapon = GetActiveWeapon(client);
            if (weapon > 0 && GetEntProp(weapon,  Prop_Send, "m_iItemDefinitionIndex") == 128)
            {
                // The Equalizer is active
                new health = GetClientHealth(client);
                if (health > 160)
                    speed = 240.0;
                else if (health > 120)
                    speed = 264.0;
                else if (health > 80)
                    speed = 288.0;
                else if (health > 40)
                    speed = 336.0;
                else
                    speed = 384.0;
            }
            else
                speed = 240.0; // TF2_GetClassSpeed(class);
        }
        case TFClass_DemoMan:
        {
            // Is The Scotsman's Skullcutter active?
            new weapon = GetActiveWeapon(client);
            speed = (weapon > 0 && GetEntProp(weapon,  Prop_Send, "m_iItemDefinitionIndex") == 172)
                    ? (TF2_IsPlayerCharging(client) ? 638.0 : 238.0)
                    : (TF2_IsPlayerCharging(client) ? 750.0 : 280.0); // TF2_GetClassSpeed(class));
        }
        case TFClass_Heavy:
        {
            new pcond = TF2_GetPlayerConditionFlags(client);
            if (TF2_IsSlowed(pcond))
                speed = 110.0; // The minigun is spinning
            else if (TF2_IsCritCola(pcond))
                speed = 310.0; // Ate Buffalo Steak Sandvitch
            else
            {
                // Are The Heavy's G.R.U.s active?
                new weapon = GetActiveWeapon(client);
                speed =  (weapon > 0 && GetEntProp(weapon,  Prop_Send, "m_iItemDefinitionIndex") == 239)
                         ? 300.0 : 230.0; // TF2_GetClassSpeed(class);
            }
        }
        case TFClass_Pyro:
        {
            speed = TF2_GetClassSpeed(class);

            // Does the Pyro have the Degreaser?
            new weapon = GetPlayerWeaponSlot(client, 0);
            if (weapon > 0 && GetEntProp(weapon,  Prop_Send, "m_iItemDefinitionIndex") == 215)
            {
                // And the Powerjack
                weapon = GetPlayerWeaponSlot(client, 2);
                if (weapon > 0 && GetEntProp(weapon,  Prop_Send, "m_iItemDefinitionIndex") == 214)
                {
                    // And is wearing the Attendant
                    new item = -1;
                    while ((item = FindEntityByClassname(item, "tf_wearable_item")) != -1)
                    {
                        if (GetEntPropEnt(item, Prop_Data, "m_hOwnerEntity") == client &&
                            GetEntProp(item, Prop_Send, "m_iItemDefinitionIndex") == 213)
                        {
                            speed = 330.0;
                        }
                    }
                }
            }
        }
        case TFClass_Engineer:
        {
            // Is the Engineer toting a building?
            speed = (GetEntPropEnt(client,  Prop_Send, "m_hCarriedObject") > 0)
                    ? 225.0 : 300.0; // TF2_GetClassSpeed(class);
        }
        default:
        {
            speed = TF2_GetClassSpeed(class);
        }
    }
    return TF2_IsPlayerDazed(client) ? speed * 0.5 : speed;
}

stock TF2_GetPlayerMaxHealth(client)
{
    new TFClassType:class = TF2_GetPlayerClass(client);
    switch (class)
    {
        case TFClass_Scout:
        {
            // Is The Sandman equipped?
            new weapon = GetPlayerWeaponSlot(client, 0);
            if (weapon > 0)
            {
                new itemIndex = GetEntProp(weapon,  Prop_Send, "m_iItemDefinitionIndex");
                if (itemIndex == 44)
                    return 110;
                else if (itemIndex == 221)
                {
                    // The Scout has the Holy Mackerel,
                    weapon = GetPlayerWeaponSlot(client, 2);
                    if (weapon > 0 && GetEntProp(weapon,  Prop_Send, "m_iItemDefinitionIndex") == 220)
                    {
                        // and the Shortstop,
                        new item = -1;
                        while ((item = FindEntityByClassname(item, "tf_wearable_item")) != -1)
                        {
                            if (GetEntPropEnt(item, Prop_Data, "m_hOwnerEntity") == client &&
                                GetEntProp(item, Prop_Send, "m_iItemDefinitionIndex") == 222)
                            {
                                // And is wearing Mad Milk!
                                return 150;
                            }
                        }
                    }
                }
            }
        }
        case TFClass_DemoMan:
        {
            new weapon = GetPlayerWeaponSlot(client, 2);
            if (weapon > 0)
            {
                new itemIndex = GetEntProp(weapon,  Prop_Send, "m_iItemDefinitionIndex");
                if (itemIndex == 132 || itemIndex == 266)
                {
                    // Is The Eyelander or Horseless Headless Horsemann's Headtaker equipped?
                    return 150 + (GetEntProp(client,  Prop_Send, "m_iDecapitations") * 15);
                }
                else if (itemIndex == 327)
                {
                    // Is The Claidheamh Mor equipped?
                    return 135 + (GetEntProp(client,  Prop_Send, "m_iDecapitations") * 15);
                }
            }
        }
        case TFClass_Engineer:
        {
            // Is The Gunslinger equipped?
            new weapon = GetPlayerWeaponSlot(client, 2);
            if (weapon > 0 && GetEntProp(weapon,  Prop_Send, "m_iItemDefinitionIndex") == 142)
                return 150;
        }
    }
    return TF2_GetClassHealth(class);
}
