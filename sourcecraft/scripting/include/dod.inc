/**
 * vim: set ai et ts=4 sw=4 syntax=sourcepawn :
 * File: dod.inc
 * Description: Stocks for DoD
 * Author(s): -=|JFH|=-Naris (Murray Wilson)
 */
 
#if defined _dod_included
 #endinput
#endif
#define _dod_included

enum DODTeam
{
	DODTeam_Unassigned = 0,
	DODTeam_Spectator = 1,
	DODTeam_Allies = 2,
	DODTeam_Axis = 3	
};

enum DODClassType
{
	DODClass_Unassigned = -1,
	DODClass_Rifleman = 0,
	DODClass_Assault,
	DODClass_Support,
	DODClass_Sniper,
	DODClass_MachineGunner,
	DODClass_Rocketman
};

stock const String:DOD_TeamName[DODTeam][] = { "Unassigned", "Spectator", "Allies", "Axis" };
#define DOD_GetTeamName(%1) DOD_TeamName[%1]

stock const String:DOD_ClassName[][] = { "", "Rifleman", "Assault", "Support", "Sniper", "MachineGunner", "Rocketman" };
#define DOD_GetClassName(%1) DOD_ClassName[%1]

/**
 * Get's a Clients current class.
 *
 * @param client		Player's index.
 * @return				Current DODClassType of player.
 * @error				Invalid client index.
 */
stock DODClassType:DOD_GetPlayerClass(client)
{
	return DODClassType:GetEntProp(client, Prop_Send, "m_iPlayerClass");
}

enum DODWeapon
{
	DODWeapon_None = 0,
	DODWeapon_Knife,
	DODWeapon_Spade,
	DODWeapon_Colt,
	DODWeapon_P38,
	DODWeapon_C96,
	DODWeapon_Garand,
	DODWeapon_K98,
	DODWeapon_ScopedK98,
	DODWeapon_M1Carbine,
	DODWeapon_Springfield,
	DODWeapon_Thompson,
	DODWeapon_MP40,
	DODWeapon_MP44,
	DODWeapon_BAR,
	DODWeapon_30CalMG,
	DODWeapon_MG42,
	DODWeapon_Bazooka,
	DODWeapon_Panzerschrek,
	DODWeapon_FragGrenade,
	DODWeapon_StickGrenade,
	DODWeapon_USSmokeGrenade,
	DODWeapon_GermanSmokeGrenade,
	DODWeapon_USRifleGrenade,
	DODWeapon_GermanRifleGrenade
};

stock const String:DOD_WeaponEntity[DODWeapon][] = {
    "",
	"weapon_amerknife",
	"weapon_spade",
	"weapon_colt",
	"weapon_p38",
	"weapon_c96",
	"weapon_garand",
	"weapon_k98",
	"weapon_k98_scoped",
	"weapon_m1carbine",
	"weapon_spring",
	"weapon_thompson",
	"weapon_mp40",
	"weapon_mp44",
	"weapon_bar",
	"weapon_30cal",
	"weapon_mg42",
	"weapon_bazooka",
	"weapon_pschreck",
	"weapon_frag_us",
	"weapon_frag_ger",
	"weapon_smoke_us",
	"weapon_smoke_ger",
	"weapon_riflegren_us",
	"weapon_riflegren_ger"
};
 
stock const DOD_AmmoOffset[DODWeapon] = {
    0,  // none
	0,	// weapon_amerknife
	0,	// weapon_spade
	4,	// weapon_colt
	8,	// weapon_p38
	12,	// weapon_c96
	16,	// weapon_garand
	20,	// weapon_k98
	20,	// weapon_k98s
	24,	// weapon_m1carb
	28,	// weapon_spring
	32,	// weapon_thompson
	32,	// weapon_mp40
	32,	// weapon_mp44
	36,	// weapon_bar
	40,	// weapon_30cal
	44,	// weapon_mg42
	48,	// weapon_bazooka
	48,	// weapon_pschreck
	52,	// weapon_frag_us
	56,	// weapon_frag_ger
	68,	// weapon_smoke_us
	72,	// weapon_smoke_ger
	84,	// weapon_riflegren_us
	88	// weapon_riflegren_ger
};
