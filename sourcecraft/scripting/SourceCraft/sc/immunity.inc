/*
 * vim: set ai et ts=4 sw=4 syntax=sourcepawn :
 * File: immunity.inc
 * Description: All the immunity and attribute types.
 * Author(s): Naris
 */

#if defined _immunity_included
 #endinput
#endif
#define _immunity_included

// The types of immunities you can get/set.
// (Maximum of 32 immunities)
enum Immunity (<<= 1)
{
    Immunity_None = 0,          // No immunity
    Immunity_Ultimates = 1,  	// Immune from ultimates
    Immunity_Upgrades,          // Immune from all upgrades
    Immunity_ShopItems,  	    // Immune from shop items
    Immunity_HealthTaking, 	    // Immune from health taking/physical damage
    Immunity_Explosion,  	    // Immune from explosions
    Immunity_Magic,             // Immune from magical attacks
    Immunity_Energy,            // Immune from "energy attacks" such as beam weapons
    Immunity_Freezing,   	    // Immune from freezing
    Immunity_Burning,    	    // Immune from burning (does NOT include pyro's flamethrower)
    Immunity_Infection,    	    // Immune from infection
    Immunity_Radiation,    	    // Immune from radiation
    Immunity_Poison,    	    // Immune from poison
    Immunity_RangedAttacks,     // Immune from ranged attacks (mostly)
    Immunity_Restore,	        // Immune from healthtaking,uncloaking,motiontaking,blinding,freezing,burning,etc...
    Immunity_Theft,	     	    // Immune from theft
    Immunity_Blindness,    	    // Immune from blindness
    Immunity_Detection, 	    // Immune from detection
    Immunity_Uncloaking, 	    // Immune from uncloaking
    Immunity_MotionTaking, 	    // Immune from being stopped
    Immunity_Silver,            // Immune from reincarnation (Prevents victims from reincarnating)
    Immunity_Drugs,  	        // Immune from drugs (Converts obnoxious visual effects to Blindness (permanent))
//};

// The types of restrictions you can get/set.
// (Maximum of 32 restrictions)
//enum Restriction (<<= 1)
//{
//  Restriction_None = 0,           // No restrictions
    Restriction_PreventBurrow,      // Prevented from burrowing
    Restriction_PreventRespawn,     // Prevented from reincarnating.
    Restriction_PreventUltimates,   // Prevented from using ultimates
    Restriction_PreventUpgrades,    // Prevented from using using upgrades/skills
    Restriction_PreventShopItems,   // Prevented from using shop items
    Restriction_Disarmed,           // Prevented from attacking
    Restriction_Grounded,           // Prevented from flying
    Restriction_Stunned,            // Prevented from moving, attacking and using shopitems, upgrades or ultimates
    Restriction_Frozen              // Prevented from moving
};

// For now, immunities and restrictions are tracked together
// since combined, there are fewer than 32.

#define Restriction         Immunity
#define Restriction_None    Immunity_None

// The categories of damage used by HurtPlayer()
// These MUST be in the same order as the Immnuities
// (Maximum of 32 damage from categories)
enum DamageFrom (<<= 1)
{
    DamageFrom_None = 0,        // No damage category
	DamageFrom_Ultimates = 1,  	// Damage from ultimates
    DamageFrom_Skills,          // Damage from all skills
	DamageFrom_ShopItems,  	    // Damage from shop items
	DamageFrom_HealthTaking, 	// Damage from health taking/physical damage
	DamageFrom_Explosion,  	    // Damage from explosions
    DamageFrom_Magic,           // Damage from magical attack
    DamageFrom_Energy,          // Damage from "energy attacks" such as beam weapons
	DamageFrom_Freezing,   	    // Damage from freezing
	DamageFrom_Burning,    	    // Damage from burning
	DamageFrom_Infection,    	// Damage from infection
	DamageFrom_Radiation,    	// Damage from radiation
	DamageFrom_Poison,    	    // Damage from poison
    DamageFrom_RangedAttacks    // Damage from ranged attacks
};

// The types of attributes you can get/set.
// (Maximum of 32 attributes)
enum Attribute (<<= 1)
{
    Attribute_None = 0,         // No attributes
    Attribute_IsAMole = 1,      // Is a Mole, prevents several things.
    Attribute_IsBurrowed,       // Burrowed, prevents several things.
    Attribute_IsBiological,     // Vulnerable to Biological ailments.
    Attribute_IsMechanical,     // Vulnerable to Mechanical ailments.
    Attribute_IsRobotic,        // Vulnerable to Robotic ailments.
    Attribute_IsEnergy,         // Vulnerable to Energy ailments.
    Attribute_IsFlyer,          // Vulnerable to Flyer ailments.
    Attribute_IsSmall,          // Vulnerable to ailments of Smaller units.
    Attribute_IsLarge,          // Vulnerable to ailments of Larger units.
    Attribute_IsMassive         // Vulnerable to ailments of Massive units.
};

