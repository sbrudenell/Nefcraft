/**
 * vim: set ai et ts=4 sw=4 syntax=sourcepawn :
 * File: MedicInfect.inc
 * Description: Native Interface to MedicInfect
 * Author(s): -=|JFH|=-Naris (Murray Wilson)
 */

#if defined _medicinfect_included
 #endinput
#endif
#define _medicinfect_included

/**
 * Assume control of the MedicInfect plugin
 *
 * @param plugin_only 	Set to 1 to assume complete control of the MedicInfect plugin.
 * @return		    	none
 */
native ControlMedicInfect(bool:plugin_only=true);

/**
 * Enable a player to utilize medic infections.
 *
 * @param index 	Client index
 * @param arm       Set to true to enable Medic Infections for Client when a Medic.
 * @param amount    Set to amount of damage per heartbeat, 0 = default amount from convar.
 * @param chance    Set to % chance of infection per hit, 100 = default to always infect.
 * @return			none
 */
native SetMedicInfect(client,bool:arm=true,amount=0,chance=100);

/**
 * Infect a player within the prescribed distance and parameters for Infection.
 *
 * @param target    Client index of target
 * @param client 	Client index of infector
 * @param allow     Set to true to enable Medic Infections for targets on the same team.
 * @param irradiate Set to true for Irradiatation instead of Infection.
 * @return			none
 */
native MedicInfect(target, client=0, bool:allow=false, bool:irradiate=false);

/**
 * Infect a player unconditionally, without checking any parameters
 * (other than allow friendly as specified below) 
 *
 * @param target    Client index of target
 * @param client 	Client index of infector
 * @param allow     Set to true to enable Medic Infections for targets on the same team.
 * @param irradiate Set to true for Irradiatation instead of Infection.
 * @return			none
 */
native Infect(target, client=0, bool:allow=false, bool:irradiate=false);

/**
 * Heal a player.
 *
 * @param target    Client index of target
 * @param index 	Client index of healer
 * @return			none
 */
native HealInfect(target,client=0);


/**
 * IsInfected
 *
 * @param target    Client index of target
 * @return			Returns true if the target is infected.
 */
native bool:IsInfected(target);

/**
 * Gets called when a player is infected or healed
 * @param victim: The victim's index.
 * @param infector: The infector's index.
 * @param source: The source's (original infecting medic's) index. 
 * @param player: The player handle of the client.
 * @param infected: Set true when infected and false when healed.
 * @param color: Array for color player was turned (r,b,g,a).
 * @return		false to prevent infection, true to allow infection.
 */
functag OnInfectedFunc Action:public(victim,infector,source,bool:infected,const color[4]);

/**
 * Gets called when a player takes damage from infection
 * @param victim: The victim's index.
 * @param infector: The infector's index.
 * @param player: The player handle of the client.
 * @param amount: Reference for the amount of damage to do, can be altered.
 * @return		false to prevent infection, true to allow infection.
 */
functag OnInfectionHurtFunc Action:public(victim,infector,&amount);

/**
 * Hook Infections and Heals
 *
 * @param func 	Function to call when infections and heals happen.
 * @return		none
 */
native HookInfection(OnInfectedFunc:func);

/**
 * Hook Damage from Infection
 *
 * @param func 	Function to call when damaging a player due to infection
 * @return		none
 */
native HookInfectionHurt(OnInfectionHurtFunc:func);

/* DO NOT EDIT BELOW THIS LINE */

public SharedPlugin:__pl_MedicInfect = 
{
	name = "MedicInfect",
	file = "MedicInfect.smx",
#if defined REQUIRE_PLUGIN
	required = 1,
#else
	required = 0,
#endif
};

#if !defined REQUIRE_PLUGIN
public __pl_MedicInfect_SetNTVOptional()
{
	MarkNativeAsOptional("ControlMedicInfect");
	MarkNativeAsOptional("HookInfection");
	MarkNativeAsOptional("HookInfectionHurt");
	MarkNativeAsOptional("SetMedicInfect");
	MarkNativeAsOptional("MedicInfect");
	MarkNativeAsOptional("HealInfect");
	MarkNativeAsOptional("IsInfected");
}
#endif
