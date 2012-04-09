/**
 * vim: set ai et ts=4 sw=4 syntax=sourcepawn :
 * File: sm_dod_medic_class.inc
 * Description: Native Interface to sm_dod_medic_class
 * Author(s): -=|JFH|=-Naris (Murray Wilson)
 */

#if defined _sm_dod_medic_class_included
 #endinput
#endif
#define _sm_dod_medic_class_included


/**
 * Assume control of the sm_dod_medic_class plugin
 *
 * @param control 	        Set to true to assume control of sm_dod_medic_class.
 * @param max_health 	    Maximum health a player can be healed to (100 = full health, -1 = use convar).
 * @param self_health 	    Minimum hp before a player can self heal (-1 = use convar).
 * @param restrict_class    Class to restrict Medic to (-1 = use convar).
 * @param min_players 	    Minimum number of players before Medic class available (-1 = use convar).
 * @param max_medics 	    Maximum number of Medics per team (0 = unlimited, -1 = use convar).
 * @return		    	    none
 */
native ControlMedic(bool:control=true, max_health=-1, self_health=-1, restrict_class=-1, min_players=-1, max_medics=-1);

/**
 * Assigns player to be a Medic.
 *
 * @param client 	        Client index
 * @param keep_weapon       Allow the Medic to keep the original classes' weapons.
 * @param pickup            Allow the Medic to pickup and use dropped weapons.
 * @param speed 	        Sets the speed of the medic (-1 = use convar).
 * @param weight 	        Sets the weight (gravity) of the medic (-1 = use convar).
 * @param health 	        Sets the HP of the medic (-1 = use convar).
 * @param heal 	            Maximum amount of health to heal (-1 = use convar).
 * @param packs 	        Number of health packs carried by the Medic (-1 = use convar).
 * @param weapon            The secondary weapon to give to the Medic <0=Pistols,1=Carbine/C96> (-1 = use convar).
 * @param ammo              How much ammo to give to the player on each spawn (-1 = use convar).
 * @param nades             Enable nades for the Medic <0=Disable,1=Smoke,2=Nades> (-1 = use convar).
 * @param num_nades         The amount of nades to give to the Medic (-1 = use convar).
 * @return		    	    none
 */
native AssignMedic(client, bool:keep_weapon, bool:pickup, Float:speed=-1.0, Float:weight=-1.0,
                   health=-1, heal=-1, packs=-1, weapon=-1, ammo=-1, nades=-1, num_nades=-1);

/**
 * Changes a player to not be a Medic.
 *
 * @param client 	        Client index
 * @return		    	    none
 */
native UnassignMedic(client);

/**
 * Gets the specified medic's speed.
 *
 * @param client 	        Client index
 * @return		    	    none
 */
native Float:GetMedicSpeed(client);

/**
 * Gets the specified medic's speed.
 *
 * @param client 	        Client index
 * @return		    	    none
 */
native Float:GetMedicWeight(client);

/**
 * Command for Medic to Heal a patient (player Medic is looking at).
 *
 * @param client 	        Client index of medic
 * @return		            none
 */
native MedicHeal(client);

/**
 * Gets called when Medics heal players
 * @param medic 	Client index of the medic that performed healing
 * @param patient 	Client index of the player that was healed
 * @param amount 	Amount that patient was healed
 */
forward Action:OnMedicHealed(client, patient, amount);

/* DO NOT EDIT BELOW THIS LINE */

public SharedPlugin:__pl_medic_class = 
{
	name = "medic_class",
	file = "sm_dod_medic_class.smx",
#if defined REQUIRE_PLUGIN
	required = 1,
#else
	required = 0,
#endif
};

#if !defined REQUIRE_PLUGIN
public __pl_medic_class_SetNTVOptional()
{
	MarkNativeAsOptional("ControlMedic");
	MarkNativeAsOptional("AssignMedic");
	MarkNativeAsOptional("UnassignMedic");
	MarkNativeAsOptional("GetMedicSpeed");
	MarkNativeAsOptional("GetMedicWeight");
	MarkNativeAsOptional("MedicHeal");
}
#endif
