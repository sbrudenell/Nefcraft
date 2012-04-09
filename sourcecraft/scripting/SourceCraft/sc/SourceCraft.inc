/**
 * vim: set ai et ts=4 sw=4 syntax=sourcepawn :
 * File: SourceCraft.inc
 * Description: Definitions for SourceCraft Natives.
 * Author(s): Naris (Murray Wilson)
 * Credit(s): Anthony Iacono  
 */

#if defined SOURCECRAFT
 #endinput
#endif
#define SOURCECRAFT

#undef REQUIRE_EXTENSIONS
#include <tf2>
#include <tf2_player>
#include <tf2_objects>
#define REQUIRE_EXTENSIONS

#include <gametype>
#include <dmgbits>

#include "sc/defines"
#include "sc/version"
#include "sc/immunity"
#include "sc/cooldown"
#include "sc/settings"
#include "sc/visibility"
#include "sc/round_state"
#include "sc/faction"
#include "sc/client"
#include "sc/invuln"

/**
 * Retrieves the SourceCraftLoaded flag.
 * @return The return value will be true if SourceCraft has completed loading.
 */
native bool:IsSourceCraftLoaded();

/**
 * Gets the state of the round.
 * @return The return value will be the state of the current round.
 */
native RoundStates:GetRoundState();

/**
 * Registers a race with the SourceCraft plugin
 * @param short:            The short name of the race. (max 16, defaults to truncated & lowercased name) 
 * @param required_level:   The overeall level the player must obtain before changing to this race. (default is 0)
 * @param tech_level:       The "tech" level the player must obtain in the race's faction before changing to this race.
 *                          (default is 0)
 * @param max_level:        The maximum level the race can attain. (default is 16)
 * @param initial_energy:   The amount of energy players of this race are given on spawn. (default is 30)
 * @param energy_limit:     The maximum energy players of this race can attain, <0 is unlimited. (default is 100)
 * @param energy_rate:      The rate at which this race gains energy per second, (default is 1)
 * @param faction:          What faction the race is a member of, found in faction.inc. (default is Generic)
 * @param type:             What unit type the race is, found in faction.inc. (default is GenericUnit)
 * @param parent:           Parent race, for summoned races. (default is "" for none).
 * @param translation:      The format of the translation file (defaults to auto detection)
 * @param translation_file: The name of the translation file (defaults to auto detection)
 * @param name:             The name of the race. (max 64, defaults to translation)
 * @param switch_message:   The message displayed to the player when they are switched to a new race.
 *                          If left empty, a default message will be translated or generated. (max 192)
 * @param pending_message:  The message displayed to the player when they try to switch to the race,
 *                          but they are alive so they need to wait until they die or the new round.
 *                          If left empty, a default message will be translated or generated. (max 192)
 * @param image:            The image of the race, to be placed into the database for the webapp. (defaults to translation)
 * @param desc:             The description of the race. (defaults to translation). If the description starts with a %,
 *                          the portion after the % will be used as a translation phrase.
 * @return The return value will be the race index or -1 if there was a problem.
 */         
native CreateRace(const String:short[], required_level=0, tech_level=0, max_level=16,
                  initial_energy=30, energy_limit=100, energy_rate=1, Faction:faction=Generic,
                  UnitType:type=GenericUnit, const String:parent[]="", Translation:translation=Translation_Auto,
                  const String:translation_file[]="", const String:name[]="", const String:switch_message[]="",
                  const String:pending_message[]="", const String:image[]="", const String:desc[]="");

/**
 * Changes the Attributes for an existing race
 * @param race:             The race ID returned from CreateRace()
 * @param required_level:   The overeall level the player must obtain before changing to this race. (default is 0)
 * @param tech_level:       The "tech" level the player must obtain in the race's faction before changing to this race.
 *                          (default is 0)
 * @param max_level:        The maximum level the race can attain. (default is 16)
 * @param initial_energy:   The amount of energy players of this race are given on spawn. (default is 30)
 * @param energy_limit:     The maximum energy players of this race can attain, <0 is unlimited. (default is 100)
 * @param energy_rate:      The rate at which this race gains energy per second, (default is 1)
 * @param faction:          What faction the race is a member of, found in faction.inc. (default is Generic)
 * @param type:             What unit type the race is, found in faction.inc. (default is GenericUnit)
 * @param parent:           Parent race, for summoned races. (default is "" for none).
 * @param translation:      The format of the translation file (defaults to auto detection)
 * @param translation_file: The name of the translation file (defaults to auto detection)
 * @param name:             The name of the race. (max 64, defaults to translation)
 * @param switch_message:   The message displayed to the player when they are switched to a new race.
 *                          If left empty, a default message will be generated. (max 192)
 * @param pending_message:  The message displayed to the play when they try to switch to the race,
 *                          but they are alive so they need to wait until they die or the new round.
 *                          If left empty, a default message will be generated. (max 192)
 * @param short:            The short name of the race. (max 16, defaults to truncated & lowercased name) 
 * @noreturn
 */
native SetRaceAttributes(race, required_level=-1, tech_level=-1, max_level=-1, initial_energy=-1,
                         energy_limit=-1, energy_rate=-1, Faction:faction=Unassigned,
                         UnitType:type=UnassignedUnit, const String:parent[]="",
                         Translation:translation=Translation_Default,
                         const String:translation_file[]="", const String:name[]="",
                         const String:switch_message[]="", const String:pending_message[]="",
                         const String:short[]="");

/**
 * Can be called after a race has been fully created/defined to save
 * and close the config file. Also saves the attributes to the database
 * (if the create or update flags are set in sourcecraft.cfg)
 * This will be called automatically after OnSourceCraftReady() for any
 * races that haven't specifically called it.
 * @param race:     The race ID returned from CreateRace()
 * @param desc:     The description of the race.
 * @param image:    The image of the race.
 */
native RaceComplete(race, const String:desc[]="", const String:image[]="");

/**
 * Retrieves the name of the race
 * @param race:      The race ID returned from CreateRace()
 * @param name:      String to place the name into.
 * @param maxlength: The size of the name buffer.
 * @param lang:      Client to translate the name for, if any.
 * @noreturn
 */
native GetRaceName(race, String:name[], maxlength, lang=LANG_SERVER);

/**
 * Retrieves the description of the race.
 * @param race:         The race ID returned from CreateRace()
 * @param description:  String to place the description into.
 * @param maxlength:    The size of the description buffer.
 * @param lang:         Client to translate the description for, if any.
 * @noreturn
 */
native GetRaceDescription(race, String:description[], maxlength, lang=LANG_SERVER);

/**
 * Retrieves the short name of the race
 * @param race:      The race ID returned from CreateRace()
 * @param short:     String to place the short name into.
 * @param maxlength: The size of the short buffer.
 * @noreturn
 */
native GetRaceShortName(race, String:short[], maxlength);

/**
 * Retrieves the name of the race's parent
 * @param race:      The race ID returned from CreateRace()
 * @param name:      String to place the name into.
 * @param maxlength: The size of the name buffer.
 * @noreturn
 */
native GetRaceParentName(race, String:name[], maxlength);

/**
 * Retrieves the race ID of the race's parent
 * @param race:     The race ID returned from CreateRace()
 * @return The race ID of the parent race, 0 for no parent or -1 for error.
 */
native GetRaceParentId(race);

/**
 * Retrieves the faction of a race
 * @param race:     The race ID returned from CreateRace()
 * @param validate: Set to true to return only valid faction indices (converts Unassigned to Generic)
 * @return The return value will be the Faction of the race
 */
native Faction:GetRaceFaction(race,bool:validate=true);

/**
 * Retrieves the number of races
 * @return The return value will be the number of races.
 */
native GetRaceCount();

/**
 * Retrieves the name of the unit type
 * @param type:      The UnitType
 * @param name:      String to place the name into.
 * @param maxlength: The size of the name buffer.
 * @param lang:      Client to translate the name for, if any.
 * @noreturn
 */
native GetUnitTypeName(UnitType:type, String:name[], maxlength, lang=LANG_SERVER);

/**
 * Retrieves the name of the faction
 * @param faction:   The Faction
 * @param name:      String to place the name into.
 * @param maxlength: The size of the name buffer.
 * @param lang:      Client to translate the name for, if any.
 * @noreturn
 */
native GetFactionName(Faction:faction, String:name[], maxlength, lang=LANG_SERVER);

/**
 * Retrieves the description of the faction
 * @param faction:   The Faction
 * @param name:      String to place the name into.
 * @param maxlength: The size of the name buffer.
 * @param lang:      Client to translate the name for, if any.
 * @noreturn
 */
native GetFactionDescription(Faction:faction, String:desc[], maxlength, lang=LANG_SERVER);

/**
 * Retrieves the "Not Enough Crystals" sound for the faction
 * @param faction:   The Faction
 * @param sound:     String to place the sound into.
 * @param maxlength: The size of the name buffer.
 * @param lang:      Client to translate the name for, if any.
 * @noreturn
 */
native GetFactionCrystalSound(Faction:faction, String:sound[], maxlength, lang=LANG_SERVER);

/**
 * Emits the "Not Enough Crystals" sound to the client
 * @param client:    The client's index.
 * @param faction:   The Faction (defaults to client's current faction)
 * @noreturn
 */
native EmitCrystalSoundToClient(client, Faction:faction=Unassigned);

/**
 * Retrieves the "Not Enough Vespene" sound for the faction
 * @param faction:   The Faction
 * @param sound:     String to place the sound into.
 * @param maxlength: The size of the name buffer.
 * @param lang:      Client to translate the name for, if any.
 * @noreturn
 */
native GetFactionVespeneSound(Faction:faction, String:sound[], maxlength, lang=LANG_SERVER);

/**
 * Emits the "Not Enough Vespene" sound to the client
 * @param client:    The client's index.
 * @param faction:   The Faction (defaults to client's current faction)
 * @noreturn
 */
native EmitVespeneSoundToClient(client, Faction:faction=Unassigned);

/**
 * Retrieves the "Not Enough Energy" sound for the faction
 * @param faction:   The Faction
 * @param sound:     String to place the sound into.
 * @param maxlength: The size of the name buffer.
 * @param lang:      Client to translate the name for, if any.
 * @noreturn
 */
native GetFactionEnergySound(Faction:faction, String:sound[], maxlength, lang=LANG_SERVER);

/**
 * Emits the "Not Enough Energy" sound to the client
 * @param client:    The client's index.
 * @param faction:   The Faction (defaults to client's current faction)
 * @noreturn
 */
native EmitEnergySoundToClient(client, Faction:faction=Unassigned);

/**
 * Retrieves the "Leveled Up" sound for the faction
 * @param faction:   The Faction
 * @param sound:     String to place the sound into.
 * @param maxlength: The size of the name buffer.
 * @param lang:      Client to translate the name for, if any.
 * @noreturn
 */
native GetFactionLevelSound(Faction:faction, String:sound[], maxlength, lang=LANG_SERVER);

/**
 * Emits the "Leveled Up" sound to the client
 * @param client:    The client's index.
 * @param faction:   The Faction (defaults to client's current faction)
 * @noreturn
 */
native EmitLevelSoundToClient(client, Faction:faction=Unassigned);

/**
 * Find the Race ID of a race
 * @param name:     The short name of the race. (max 64)
 * @return The return value will be the race index or -1 if there was a problem.
 */         
native FindRace(const String:name[]);

/**
 * Retrieves the number of upgrades a race has
 * @param race:     The race ID returned from CreateRace()
 * @return The return value will be the number of upgrades a race has or -1 for error.
 */
native GetUpgradeCount(race);

/**
 * Adds an upgrade to a race
 * @param race:             The race ID returned from CreateRace()
 * @param short:            The short name of the upgrade. (max 16, defaults to truncated & lowercased name) 
 * @param upgrade_category: Set to 0 for a "Normal" Upgrade or a category number for an ultimate. (default is 0)
 * @param required_level:   The race level required before this upgrade becomes available, set to 0 for upgrades
 *                          that are always available. The default value of -1 uses the value of sc_minultimate
 *                          (usually 8) for ultimates and 1 for other upgrades.
 * @param max_level:        The maximum level the upgrade can attain. (default is 4)
 * @param energy:           Amount of energy required to invoke upgrade. (default is 0)
 * @param recurring_energy: Recurring amount of energy required the while upgrade is active,
 *                          such as energy per shot. (default is 0)
 * @param vespene:          Amount of vespene required to purchase or invoke upgrade. (default is 0)
 * @param crystals:         Amount of crystals required to purchase or invoke upgrade. (default is 0)
 * @param cost_flag:        If COST_PER_USE, resources will be used to invoke.  (default is COST_PER_USE)
 *                          If COST_TO_PURCHASE, resources will be used to purchase.
 *                          If COST_IS_REFUNDABLE, resources will be used to purchase and refunded when reset.
 * @param cooldown:         Duration of the cooldown, if any (default is 0.0)
 * @param cooldown_type:    What type of cooldown to use, if any (default is Cooldown_None)
 * @param name:             The name of the upgrade. (max 64, defaults to translation)
 * @param desc:             The description of the upgrade. (max 192, defaults to translation)
 *                          If the description starts with a %, the portion after the % will be used as a translation phrase.
 * @param p1-p8:            Optional parameters to use when translating the description, string parameters {1:s} must be defined
 *                          in the translation file as specified here: http://wiki.alliedmods.net/Translations_(SourceMod_Scripting).
 *                          (unless the race's translation format is Translation_W3S, which uses the W3S format)
 * @return The return value will be the upgrade index or -1 if there was a problem.
 */
native AddUpgrade(race, const String:short[], upgrade_category=0, required_level=-1, max_level=4, energy=0,
                  recurring_energy=0, vespene=0, crystals=0, cost_flag=COST_PER_USE, Float:cooldown=0.0,
                  CooldownType:cooldown_type=Cooldown_None, const String:name[]="", const String:desc[]="",
                  const String:p1[]="", const String:p2[]="", const String:p3[]="", const String:p4[]="",
                  const String:p5[]="", const String:p6[]="", const String:p7[]="", const String:p8[]="");

#define AddUltimate AddUpgrade
#define AddAbility  AddUpgrade
#define AddSkill    AddUpgrade

/**
 * Sets an upgrade's cooldown parameters
 * @param race:             The race ID returned from CreateRace()
 * @param upgrade:          The upgrade ID returned from AddUpgrade()
 * @param cooldown:         Duration of the cooldown, if any (default is 0.0)
 * @param cooldown_type:    What type of cooldown to use, if any (default is Cooldown_None)
 * @return The return value will be the upgrade index or -1 if there was a problem.
 */         
native SetUpgradeCooldown(race, upgrade, Float:cooldown=0.0,
                          CooldownType:cooldown_type=Cooldown_None);

/**
 * Gets an upgrade's cooldown parameters
 * @param race:             The race ID returned from CreateRace()
 * @param upgrade:          The upgrade ID returned from AddUpgrade()
 * @param cooldown_type:    Returns what type of cooldown to use
 * @return The return value will be the cooldown duration
 */         
native Float:GetUpgradeCooldown(race, upgrade, &CooldownType:cooldown_type=Cooldown_None);

/**
 * Retrieves the name of an upgrade
 * @param race:      The race ID returned from CreateRace()
 * @param upgrade:   The upgrade ID returned from AddUpgrade()
 * @param name:      String to place the name into.
 * @param maxlength: The size of the name buffer.
 * @param lang:      Client to translate the description for, if any.
 * @noreturn
 */
native GetUpgradeName(race, upgrade, String:name[], maxlength, lang=LANG_SERVER);

/**
 * Retrieves the description of an upgrade
 * @param race:         The race ID returned from CreateRace()
 * @param upgrade:      The upgrade ID returned from AddUpgrade()
 * @param description:  String to place the description into.
 * @param maxlength:    The size of the description buffer.
 * @param lang:         Client to translate the description for, if any.
 * @noreturn
 */
native GetUpgradeDescription(race, upgrade, String:description[], maxlength, lang=LANG_SERVER);

/**
 * Retrieves the short name of an upgrade
 * @param race:      The race ID returned from CreateRace()
 * @param upgrade:   The upgrade ID returned from AddUpgrade()
 * @param short:     String to place the short name into.
 * @param maxlength: The size of the short name buffer.
 * @param lang:      Client to translate the description for, if any.
 * @noreturn
 */
native GetUpgradeShortName(race, upgrade, String:short[], maxlength);

/**
 * Retrieves the max level of an upgrade
 * @param race:     The race ID returned from CreateRace()
 * @param upgrade:  The upgrade ID returned from AddUpgrade()
 * @return The return value will be the max level of the upgrade or -1 if it failed.
 */
native GetUpgradeMaxLevel(race,upgrade);

/**
 * Retrieves the category of an upgrade
 * @param race:     The race ID returned from CreateRace()
 * @param upgrade:  The upgrade ID returned from AddUpgrade()
 * @return The return value will be the category of the upgrade or -1 if it failed.
 */
native GetUpgradeCategory(race,upgrade);

/**
 * Sets the category of an upgrade
 * @param race:             The race ID returned from CreateRace()
 * @param upgrade:          The upgrade ID returned from AddUpgrade()
 * @param upgrade_category: Set to 0 for a "Normal" Upgrade or a category number for an ultimate.
 * @noreturn
 */
native SetUpgradeCategory(race,upgrade,upgrade_category);

/**
 * Retrieves the amount of energy used by an upgrade
 * @param race:     The race ID returned from CreateRace()
 * @param upgrade:  The upgrade ID returned from AddUpgrade()
 * @return The return value will be the amount of energy or -1 if it failed.
 */
native GetUpgradeEnergy(race,upgrade);

/**
 * Retrieves the amount of recurring energy used by an upgrade
 * @param race:     The race ID returned from CreateRace()
 * @param upgrade:  The upgrade ID returned from AddUpgrade()
 * @return The return value will be the amount of recurring energy or -1 if it failed.
 */
native GetUpgradeRecurringEnergy(race,upgrade);

/**
 * Retrieves the amount of vespene used by an upgrade
 * @param race:     The race ID returned from CreateRace()
 * @param upgrade:  The upgrade ID returned from AddUpgrade()
 * @return The return value will be the amount of vespene or -1 if it failed.
 */         
native GetUpgradeVespene(race,upgrade);

/**
 * Retrieves the amount of crystals used by an upgrade
 * @param race:     The race ID returned from CreateRace()
 * @param upgrade:  The upgrade ID returned from AddUpgrade()
 * @return The return value will be the amount of crystals or -1 if it failed.
 */         
native GetUpgradeCrystals(race,upgrade);

/**
 * Registers a shop item with the SourceCraft plugin
 * @param short:            The short name of the item. (max 16) 
 * @param crystals:         The cost of the item in crystals.
 * @param vespene:          The cost of the item in vespene gas.
 * @param max:              The maximum number of this item that the player can own.
 * @param required_level:   The overall level the player must obtain before buying this item.
 * @param translation:      The format of the translation file (defaults to auto detection)
 * @param translation_file: The name of the translation file (defaults to auto detection)
 * @param category:         The category of the item. (max 64, defaults to translation)
 * @param name:             The name of the item. (max 64, defaults to translation)
 * @param desc:             The description of the item. (max 256, defaults to translation)
 * @return The return value will be the item index or -1 if there was a problem.
 */         
native CreateShopItem(const String:short[],crystals,vespene=0,max=1,required_level=0,
                      Translation:translation=Translation_Auto, const String:translation_file[]="",
                      const String:category[]="", const String:name[]="",const String:desc[]="");

/**
 * Retrieves the short name of the shop item
 * @param item:         The item ID returned from CreateShopItem
 * @param name:         String to place the name into.
 * @param maxlength:    The size of the name buffer.
 * @noreturn
 */
native GetItemShortName(item, String:name[], maxlength);

/**
 * Changes a ShopItem's short name.
 * @param item:         The item ID returned from CreateShopItem
 * @param short:        The new short name of the item. (max 16) 
 * @noreturn
 */
native SetItemShortName(const String:short[]);

/**
 * Retrieves the name of the shop item
 * @param item:         The item ID returned from CreateShopItem
 * @param name:         String to place the name into.
 * @param maxlength:    The size of the name buffer.
 * @param lang:         Client to translate the name for, if any.
 * @noreturn
 */
native GetItemName(item, String:name[], maxlength, lang=LANG_SERVER);

/**
 * Retrieves the category of the shop item
 * @param item:         The item ID returned from CreateShopItem
 * @param category:     String to place the category into.
 * @param maxlength:    The size of the category buffer.
 * @param lang:         Client to translate the category for, if any.
 * @noreturn
 */
native GetItemCategory(item, String:category[], maxlength, lang=LANG_SERVER);

/**
 * Retrieves the description of the shop item
 * @param item:         The item ID returned from CreateShopItem
 * @param description:  String to place the description into.
 * @param maxlength:    The size of the description buffer.
 * @param lang:         Client to translate the description for, if any.
 * @noreturn
 */
native GetItemDescription(item, String:description[], maxlength, lang=LANG_SERVER);

/**
 * Retrieves the number of ShopItems
 * @return The return value will be the number of ShopItems.
 */
native GetShopItemCount();

/**
 * Find the ShopItem ID of a shop item
 * @param name:     The name of the item. (max 64)
 * @return The return value will be the item index or -1 if there was a problem.
 */         
native FindShopItem(const String:name[]);

/**
 * Registers a help command with the SourceCraft plugin for help
 * @param name:     The name of the command. (max 64)
 * @param desc:     The description of the command. (max 256) 
 * @return The return value will be the help index or -1 if there was a problem.
 */
native CreateHelpCommand(const String:name[],const String:desc[]);

/**
 * Give health to (heal) a player
 * @param client:   The client's index to heal.
 * @param amount:   The amount of health to give to the client.
 * @param health:   The health of the client (looks up if not provided).
 * @param max:      The max health of the client (looks up if not provided).
 * @noreturn
 * native HealPlayer(client, amount,health=-1,max=-1);
 */
native HealPlayer(client, amount,health=-1,max=-1);

/**
 * Take health from (damage) a player
 * @param victim:           The victim's index you want.
 * @param damage:           The amount to damage to deal to the victim.
 * @param attacker:         The index of the attacker.
 * @param weapon_name:      The name of the weapon used to deal the damage to the client.
 * @param weapon_desc:      The description of the weapon used to deal the damage to the client.
 * @param xp:               Amount of extra xp for a kill (if any).
 * @param explode:          Set to true to cause the player to explode when they die.
 * @param type:             What damage bit flags to use (See DMG_ definitions in dmgbits.inc)
 * @param category:         What DamageFrom bit flags (damage categories) to use
 * @param limit:            Set to limit the damage to the % of the victim's class health (0 == unlimited, default is 90%)
 * @param ignore_armor:     Set to true to ignore the victim's armor.
 * @param ignore_immunity:  Set to true to ignore the victim's immunities.
 * @param in_hurt_event:    Set to true when called from player_hurt event.
 * @param no_suicide:       Set to true to NOT force suicide when health<=0.
 * @param no_forward:       Set to true to supress damage forwarding (OnPlayerTakeDamage & player_event)
 * @param no_translate:     Set to true to supress translating the weapon_name to get the weapon_desc
 * @param prevent_stack:    Set to true to prevent other sourcecraft damage mods from stacking (defaults to true).
 * @return The return value will be the victims remaining health
 */
native HurtPlayer(victim, damage, attacker=0, const String:weapon_name[]="",
                  const String:weapon_desc[]="", xp=0, bool:explode=false,
                  type=DMG_GENERIC, DamageFrom:category=DamageFrom_None,
                  Float:limit=0.90, bool:ignore_armor=false, bool:ignore_immunity=false,
                  bool:in_hurt_event=false, bool:no_suicide=false, bool:no_forward=false,
                  bool:no_translate=false, bool:prevent_stack=true);

/**
 * Kill a player.
 * @param victim:        The victim's index.
 * @param attacker:      The index of the attacker.
 * @param weapon_name:   The name of the weapon used to deal the damage to the client.
 * @param weapon_desc:   The description of the weapon used to deal the damage to the client.
 * @param xp:            Amount of extra xp for a kill.
 * @param explode:       Set to true to cause the player to explode.
 * @param type:          What damage bit flags to use (See DMG_ definitions in dmgbits.inc)
 * @param silent:        Set to true to NOT display a kill message.
 * @param no_suicide:    Set to true to NOT force suicide and only set health=0.
 * @param no_translate:  Set to true to supress translating the weapon_name to get the weapon_desc
 * @noreturn
 */
native KillPlayer(victim, attacker=0, const String:weapon_name[]="",
                  const String:weapon_desc[]="", xp=0,
                  bool:explode=false, type=DMG_GENERIC,
                  bool:silent=false, bool:no_suicide=false,
                  bool:no_translate=false);

/**
 * Take health from (damage) an entity
 * @param victim:   The entity you want to damage.
 * @param damage:   The amount to damage to deal to the victim.
 * @param attacker: The index of the attacker.
 * @param dmg_type: What damage bit flags to use (See DMG_ definitions in dmgbits.inc)
 * @param weapon:   The name of the weapon used to deal the damage to the entity.
 * @noreturn
 */
native DamageEntity(victim,damage,attacker=0,dmg_type=DMG_GENERIC,const String:weapon[]="");

/**
 * Retrieves the Suppress Damage Forwarding Flag set in HurtPlayer()
 * only valid in OnTakePlayerDamage() and OnPlayerHurt() Handlers.
 *
 * @return The return value will be the Suppress Damage Forwarding flag.
 */
native bool:GetDamageFromPlayerHurt();

/**
 * Retrieves the Suppress Damage Forwarding Flag set in HurtPlayer()
 * only valid in OnTakePlayerDamage() and OnPlayerHurt() Handlers.
 *
 * @return The return value will be the Suppress Damage Forwarding flag.
 */
native bool:GetSuppressDamageForward();

/**
 * Retrieves the damage done by the event, calculates it if required.
 * @param event:    The event to process.
 * @param client:   The index of the player that was damaged (0=get userid from event)
 * @return The return value will be the amount of damage
 */
native GetDamage(Handle:event, client=0);

/**
 * Displays damage inflicted upon a player
 * @param client:       The index of the player that inflicted the damage (attacker)
 * @param victim:       The index of the player that got damaged
 * @param hp:           The amount of damage dealt
 * @param weapon:       The short name of the weapon used to inflice the damage (for the log)
 * @param description:  The description of the weapon (displayed to the players)
 * @param no_translate: Set to true to supress translating the weapon_name to get the weapon_desc
 * @noreturn
 */
native DisplayDamage(client, victim, hp=0, const String:weapon[],
                     const String:description[]="",
                     bool:no_translate=false);

/**
 * Displays when a player is killed
 * @param client:       The index of the attacking player (attacker)
 * @param victim:       The index of the player that got killed
 * @param xp:           The amount of Extra XP gained
 * @param weapon:       The short name of the weapon used to inflice the damage (for the log)
 * @param description:  The description of the weapon (displayed to the players)
 * @param no_translate: Set to true to supress translating the weapon_name to get the weapon_desc
 * @noreturn
 */
native DisplayKill(client, victim, xp=0, const String:weapon[],
                   const String:description[]="",
                   bool:no_translate=false);

/**
 * Displays a message to a player
 * @param client:       The index of the player to display the message to
 * @param flags:        The message flags to categorize the message
 * @param fmt:          The format string for the message
 * @param ...: Format arguments (if any)
 * @noreturn
 */
native DisplayMessage(client, Settings:flags, const String:fmt[], any:...);

/**
 * Displays a Hud Message to a player
 * @param client:       The index of the player to display the message to
 * @param fmt:          The format string for the message
 * @param ...:          Format arguments (if any)
 * @noreturn
 */
native HudMessage(client, const String:fmt[], any:...);

/**
 * Clears a client's Hud.
 * @param client:       The index of the player to clear
 * @param contains:     Only clear the HUD Message if it contains this string,
 *                      which can be a format string. If contains is empty,
*                       the HUD is cleared unconditionally.
 * @noreturn
 */
native ClearHud(client, const String:contains[]="", any:...);

/**
 * Retrieves the players HUD Message
 * @param client:       The index of the player
 * @param message:      String to place the message into.
 * @param maxlength:    The size of the message buffer.
 * @noreturn
 */
native GetHudMessage(client, String:message[], maxlength);

/**
 * Shows the client's XP, level and race in chat
 * @param client:       Client index
 * @param forceDisplay: Forces display regardless of client's display settings
 * @noreturn
 */
native ShowXP(client, forceDisplay=false);

/**
 * Shows the client's crystals in chat
 * @param client:       Client index
 * @param forceDisplay: Forces display regardless of client's display settings
 * @noreturn
 */
native ShowCrystals(client, forceDisplay=false);

/**
 * Shows the client's vespene in chat
 * @param client:       Client index
 * @param forceDisplay: Forces display regardless of client's display settings
 * @noreturn
 */
native ShowVespene(client, forceDisplay=false);

/**
 * Shows the client's energy (including accumulated) in chat
 * @param client:       Client index
 * @param forceDisplay: Forces display regardless of client's display settings
 * @noreturn
 */
native ShowEnergy(client, forceDisplay=false);

/**
 * Returns how much armor the client has
 *
 * @param client 	Client
 * @return			The amount of armor the client has
 */
native GetArmor(client);

/**
 * Returns if the client has shields
 *
 * @param client 	Client
 * @return			true if the client has shields (regenerating armor)
 */
native bool:HasShields(client);

/**
 * Sets how much armor the client has
 *
 * @param client 	    Client
 * @param amount        The amount of armor the client has
 * @param percent       Array that holds a range of percent damage that the armor absorbs.
 * @param name:         What to call the armor in the HUD and messages. (max 64) [defaults to either Armor or Shields]
 * @param hasShields    true if the armor is a shield (regenerates)
 * @return			    none
 */
native SetArmor(client, amount, const Float:percent[2]={1.0,1.0},
                const String:name[]="", bool:hasShields=false);

/**
 * Sets only the amount of armor the client has
 *
 * @param client 	    Client
 * @param amount        The amount of armor the client has
 * @return			    none
 */
native SetArmorAmount(client, amount);

/**
 * Sets what % damage the client's armor absorbs
 *
 * @param client 	    Client
 * @param percent       Array that holds a range of percent damage that the armor absorbs.
 * @return			    none
 */
native SetArmorPercent(client, const Float:percent[2]);

/**
 * Gets what % damage the client's armor absorbs
 *
 * @param client 	    Client
 * @param percent       Array to receive the range of percent damage that the armor absorbs.
 * @return			    none
 */
native GetArmorPercent(client, Float:percent[2]);

/**
 * Sets the name of the given client's armor
 * @param client 	    Client
 * @param name:         What to call the armor in the HUD and messages. (max 64)
 * @noreturn
 */
native SetArmorName(client, const String:name[]);

/**
 * Retrieves the name of the given client's armor
 * @param client 	    Client
 * @param name:         String to place the name into.
 * @param maxlength:    The size of the name buffer.
 * @noreturn
 */
native GetArmorName(client, String:name[], maxlength);

/**
 * Increments how much armor the client has
 *
 * @param client 	Client
 * @param amount    The amount to add to the armor the client has
 * @return			none
 */
native IncrementArmor(client, amount);

/**
 * Decrements how much armor the client has
 *
 * @param client 	Client
 * @param amount    The amount to subtract from the armor the client has
 * @return			The amount that was actually subtracted (can be less than amount)
 */
native DecrementArmor(client, amount);

/**
 * Gets the state of all settings for a client.
 * @param client:   The index of the player to get the settings for
 * @return The return value will be all of the client's settings bits.
 */
native Settings:GetSettingsBits(client);

/**
 * Gets the state of a certain setting.
 * @param client:   The index of the player to get the settings for
 * @param setting:  The setting or settings you want to check. 
 * @return The return value will be true if they are set or false if they aren't.
 */
native bool:GetSetting(client,Settings:setting);

/**
 * Returns if the client is changing class
 *
 * @param client 	Client
 * @return			true if the client is in the process of changing class
 */
native bool:IsChangingClass(client);

/**
 * Gets the current race for the player.
 * @param client:   The index of the player.
 * @return The return value will be the race of the player or -1 if it failed.
 */
native GetRace(client);

/**
 * Changes the current race of the player.
 * @param client:   The index of the player.
 * @param race:     The race ID returned from CreateRace.
 * @param force:    Set true to force the race to change now or false to change on death.
 * @param check:    Set true to check race requirements before changing.
 * @noreturn
 */
native ChangeRace(client,race,bool:force=false,bool:check=true);

/**
 * Gets the current level for the provided race.
 * @param player:   The player handle to use. Can be retrieved with GetPlayerHandle.
 * @param race:     The race ID returned from CreateRace or -1 for current race
 * @return The return value will be the level or -1 if it failed.
 */
native GetLevel(client,race=-1);

/**
 * Sets the current level for the provided race.
 * @param client:   The index of the player.
 * @param race:     The race ID returned from CreateRace or -1 for current race
 * @param newlevel: The level you want to set to 
 * @param update:   If true, updates the overall and tech levels also.
 * @noreturn
 */
native SetLevel(client,race,newlevel,bool:update=true);

/**
 * Resets the current level for the provided race, also adjusts Upgrade, Overall and Tech levels as required.
 * @param client:   The index of the player.
 * @param race:     The race ID returned from CreateRace or -1 for current race
 * @param newlevel: The level you want to set to 
 * @noreturn
 */
native ResetLevel(client,race,newlevel);

/**
 * Gets the XP for the provided race.
 * @param client:   The index of the player.
 * @param race:     The race ID returned from CreateRace or -1 for current race
 * @return The return value will be the xp or -1 if it failed.
 */
native GetXP(client,race=-1);

/**
 * Sets the XP for the provided race.
 * @param client:   The index of the player.
 * @param race:     The race ID returned from CreateRace or -1 for current race
 * @param newxp:    The XP you want to set to 
 * @noreturn
 */
native SetXP(client,race,newxp);

/**
 * Resets the XP for the provided race, also adjusts Levels and Upgrades as required.
 * @param client:   The index of the player.
 * @param race:     The race ID returned from CreateRace or -1 for current race
 * @param newxp:    The XP you want to set to 
 * @noreturn
 */
native ResetXP(client,race,newxp);

/**
 * Gets the kill XP for a player, useful for simulating an explosion or anything else.
 * @param client:   Client index
 * @param race:     The race ID returned from CreateRace or -1 for current race
 * @return Will be kill XP or -1 if it failed
 */
native GetKillXP(client,race);

/**
 * Gets the kill XP for a certain level.
 * @param level:   The level to lookup
 * @return Will be the kill XP for the given level
 */
native GetKillLevelXP(level);

/**
 * Gets the XP required for the player to gain the next level.
 * @param client:   Client index
 * @param race:     The race ID returned from CreateRace or -1 for current race
 * @return Will be required XP or -1 if it failed
 */
native GetLevelXP(client,race);

/**
 * Gets the XP required to gain a level.
 * @param level:   The level to lookup
 * @return Will be required XP or -1 if it failed
 */
native GetReqLevelXP(level);

/**
 * Gets the current level for the upgrade for the provided race.
 * @param client:   The index of the player.
 * @param race:     The race ID returned from CreateRace or -1 for current race
 * @param upgrade:  The upgrade you want to lookup with
 * @return The return value will be the upgrade level or -1 if it failed.
 */
native GetUpgradeLevel(client,race,upgrade);

/**
 * Sets the current level for the upgrade for the provided race.
 * @param client:   The index of the player.
 * @param race:     The race ID returned from CreateRace or -1 for current race
 * @param upgrade:  The upgrade you want to lookup with
 * @param newlevel: The level you want to set to 
 * @noreturn
 */
native SetUpgradeLevel(client,race,upgrade,newlevel);

/**
 * Resets the levels for the upgrade for the curerent race.
 * @param client:   The index of the player.
 * @param race:     The race ID returned from CreateRace or -1 for current race
 * @noreturn
 */
native ResetUpgradeLevels(client,race);

/**
 * Gets the amount of Accumulated Energy a player has.
 * @param client:   The index of the player.
 * @param race:     The race ID returned from CreateRace or -1 for current race
 * @return The return value will be the amount of energy or -1 if it failed.
 */
native GetAccumulatedEnergy(client,race);

/**
 * Sets the amount of Accumulated Energy a player has.
 * @param client:     The index of the player.
 * @param race:       The race ID returned from CreateRace or -1 for current race
 * @param neweneergy: The amount you want to set
 * @noreturn
 */
native SetAccumulatedEnergy(client,race,newenergy);

/**
 * Gets the amount of Energy a player has.
 * @param client:   The index of the player.
 * @return The return value will be the amount of energy or -1 if it failed.
 */
native GetEnergy(client);

/**
 * Sets the amount of Energy a player has.
 * @param client:   The index of the player.
 * @param neweneergy: The amount you want to set
 * @noreturn
 */
native SetEnergy(client,newenergy);

/**
 * Gets the Energy accumulation Rate for a player.
 * @param client:   The index of the player.
 * @return The return value will be the amount of energy or -1 if it failed.
 */
native GetEnergyRate(client);

/**
 * Sets the Energy accumulation Rate for a player.
 * @param client:   The index of the player.
 * @param newrate:  The amount you want to set
 * @noreturn
 */
native SetEnergyRate(client,newrate);

/**
 * Gets the Energy Limit for a player.
 * @param client:   The index of the player.
 * @return The return value will be the amount of energy or -1 if it failed.
 */
native GetEnergyLimit(client);

/**
 * Sets the Energy Limit for a player.
 * @param client:   The index of the player.
 * @param newlimit: The amount you want to set
 * @noreturn
 */
native SetEnergyLimit(client,newlimit);

/**
 * Gets the Initial Energy for a player.
 * @param client:   The index of the player.
 * @return The return value will be the amount of energy or -1 if it failed.
 */
native GetInitialEnergy(client);

/**
 * Sets the Initial Energy for a player.
 * @param client:    The index of the player.
 * @param newamount: The amount you want to set
 * @noreturn
 */
native SetInitialEnergy(client,newamount);

/**
 * Gets the maximum amount of Crystals a player can have.
 * @return Maximum amount of Crystals a player can have.
 */
native GetMaxCrystals();

/**
 * Gets the amount of Crystals a player has.
 * @param client:   The index of the player.
 * @return The return value will be the amount of crystals or -1 if it failed.
 */
native GetCrystals(client);

/**
 * Sets the amount of Crystals a player has.
 * @param client:       The index of the player.
 * @param newcrystals:  The amount you want to set
 * @param check:        Set true to check limit before changing.
 * @return The return value will be the amount of crystals set or -1 if it failed.
 */
native SetCrystals(client,newcrystals,bool:check=true);

/**
 * Gets the maximum amount of Vespene (gas) a player can have.
 * @return Maximum amount of Vespene a player can have.
 */
native GetMaxVespene();

/**
 * Gets the amount of Vespene (gas) a player has.
 * @param client:   The index of the player.
 * @return The return value will be the amount of vespene or -1 if it failed.
 */
native GetVespene(client);

/**
 * Sets the amount of Vespene (gas) a player has.
 * @param client:       The index of the player.
 * @param newvespene:   The amount you want to set
 * @param check:        Set true to check limit before changing.
 * @return The return value will be the amount of vespene set or -1 if it failed.
 */
native SetVespene(client,newvespene,bool:check=true);

/**
 * Gets the player's current overall level. (Total level of all races)
 * @param client:   The index of the player.
 * @return The return value will be the overall level or -1 if it failed.
 */
native GetOverallLevel(client);

/**
 * Sets the player's overall level.
 * @param client:   The index of the player.
 * @param newlevel: The level you want to set to 
 * @noreturn
 */
native SetOverallLevel(client,newlevel);

/**
 * Gets if the player owns a specific item.
 * @param client:   The index of the player.
 * @param item:     The item ID returned from CreateShopItem
 * @return The return value will be amount of the specified item that they own (usually 1),
 *         0 if they don't own any or -1 if it failed.
 */
native GetOwnsItem(client,item);

/**
 * Sets if the player owns a specific item.
 * @param client:   The index of the player.
 * @param item:     The item ID returned from CreateShopItem
 * @param owns: 1 if you want them to own it, 0 if you want them not to own it 
 * @noreturn
 */
native SetOwnsItem(client,item,owns=1);

/**
 * Gets the state of all attributes for a player.
 * @param client:   The index of the player.
 * @return The return value will be all of the player's attribute bits.
 */
native GetAttributeBits(client);

/**
 * Sets the state of all attributes for a player.
 * @param client:   The index of the player.
 * @param immunities: The value to set for the player's attributes.
 * @noreturn
 */
native SetAttributeBits(client, immunities);

/**
 * Gets the state of a certain attribute.
 * @param client:   The index of the player.
 * @param attrib:   The attribute or attributes you want to check. 
 * @return The return value will be true if the attribute is set or false if it isn't.
 */
native bool:GetAttribute(client,Attribute:attrib);

/**
 * Gives the player a specific attribute.
 * @param client:    The index of the player.
 * @param attrib:    The attribute or attributes you want to set.
 * @param imm_state: The state to set the attribute to.
 * @noreturn
 */
native SetAttribute(client,Attribute:attrib,bool:imm_state=true);

/**
 * Gets the state of all immunities for a player.
 * @param client:   The index of the player.
 * @return The return value will be all of the player's immunity bits.
 */
native GetImmunityBits(client);

/**
 * Sets the state of all immunities for a player.
 * @param client:   The index of the player.
 * @param immunities: The value to set for the player's immunities.
 * @noreturn
 */
native SetImmunityBits(client, immunities);

/**
 * Gets the state of a certain immunity.
 * @param client:   The index of the player.
 * @param immunity: The immunity or immunities you want to check. 
 * @return The return value will be true if they are immune or false if they aren't.
 */
native bool:GetImmunity(client,Immunity:imm);

/**
 * Gives the player a specific immunity.
 * @param client:    The index of the player.
 * @param immunity:  The immunity or immunities you want to set.
 * @param imm_state: The state to set the immunity to.
 * @param id:        Identifier to allow more than 1 setting per plugin.
 * @noreturn
 */
native SetImmunity(client,Immunity:immunity,bool:imm_state=true,id=0);

// For now, immunities and restrictions are tracked together
// since combined, there are fewer than 32.

#define GetRestriction      GetImmunity
#define SetRestriction      SetImmunity
#define GetRestrictionBits  GetImmunityBits
#define SetRestrictionBits  SetImmunityBits

/**
 * Sets the speed for the player that is handled within the SourceCraft system,
 * good for upgrades and items that set speed so you don't end up making the player slower.
 * Pass -1.0 as speed when they no longer should use the speed, make sure to do this when
 * they no longer have the upgrade or item, or whatever.
 * @param client:   The index of the player.
 * @param speed:    The speed you want to pass.
 * @param apply:    Set to true to apply the setting immedately.
 * @param id:       Identifier to allow more than 1 setting per plugin.
 * @noreturn
 */
native SetSpeed(client,Float:speed=-1.0,bool:apply=false,id=0);

/**
 * Sets the gravity for the player that is handled within the SourceCraft system,
 * good for upgrades and items that set gravity so you don't end up making the player jump lower.
 * Pass -1.0 as gravity when they no longer should use the gravity, make sure to do this when
 * they no longer have the upgrade or item, or whatever.
 * @param client:   The index of the player.
 * @param gravity:  The gravity you want to pass.
 * @param apply:    Set to true to apply the setting immedately.
 * @param id:       Identifier to allow more than 1 setting per plugin.
 * @noreturn
 */
native SetGravity(client,Float:gravity=-1.0,bool:apply=false,id=0);

/**
 * Sets the visibility for the player that is handled within the SourceCraft system,
 * good for upgrades and items that set (in)visibility so you don't end up making the player
 * more visibile. Pass 255 as visibility when they no longer should use the (in)visibility,
 * make sure to do this when they no longer have the upgrade or item, or whatever.
 * @param client:       The index of the player.
 * @param type:         The type of visibility to use.
 * @param visibility:   The visibility (alpha) to use.
 * @param factor1:      The 1st factor (either melee factor or activation delay)
 * @param factor2:      The 2nd factor (either movement factor or duration)
 * @param mode:         The Render mode
 * @param fx:           The Render Fx
 * @param r:            Red Component.
 * @param g:            Green Component.
 * @param b:            Blue Component.
 * @param colorWeapons: Set to true to apply the setting to weapons and carried items also.
 * @param priority:     Priority of the visibility settings, values with highest are used.
 *                      if left at VisibilityPriorityNotSpecified, then a calculated value is used.
 * @param timestamp:    Timestamp of when the visibility was set, used when priorities are tied.
 * @param apply:        Set to true to apply the setting immedately.
 * @param id:           Identifier to allow more than 1 setting per plugin.
 * @noreturn
 */
native SetVisibility(client, VisibilityType:type=NormalVisibility,
                     visibility=-1, Float:factor1=-1.0, Float:factor2=-1.0,
                     RenderMode:mode=RenderMode:-1, RenderFx:fx=RenderFx:-1,
                     r=-1, g=-1, b=-1, bool:colorWeapons=false,
                     VisibilityPriority:priority=VisibilityPriorityNotSpecified,
                     Float:timestamp=0.0, bool:apply=false, id=0);

/**
 * Overrides the speed for SourceCraft, useful for punishing them or whatever.
 * Pass -1.0 as speed when you want the SourceCraft system to take over again,
 * make sure to do this when they are no longer supposed to follow!
 * @param client:   The index of the player.
 * @param speed:    The speed you want to pass.
 * @param apply:    Set to true to apply the setting immedately.
 * @param id:       Identifier to allow more than 1 setting per plugin.
 * @noreturn
 */
native SetOverrideSpeed(client,Float:speed=-1.0,bool:apply=false,id=0);

/**
 * Overrides the gravity for SourceCraft, useful for punishing them or whatever.
 * Pass -1.0 as gravity when you want the SourceCraft system to take over again,
 * make sure to do this when they are no longer supposed to follow!
 * @param client:   The index of the player.
 * @param gravity:  The gravity you want to pass.
 * @param apply:    Set to true to apply the setting immedately.
 * @param id:       Identifier to allow more than 1 setting per plugin.
 * @noreturn
 */
native SetOverrideGravity(client,Float:gravity=-1.0,bool:apply=false,id=0);

/**
 * Overrides the (in)visibility for SourceCraft , useful for punishing them or whatever.
 * Pass -1 as visibility when you want the SourceCraft system to take over again,
 * make sure to do this when they are no longer supposed to follow!
 * @param client:       The index of the player.
 * @param visibility:   The visibility you want to pass.
 * @param apply:        Set to true to apply the setting immedately.
 * @param id:           Identifier to allow more than 1 setting per plugin.
 * @noreturn
 */
native SetOverrideVisiblity(client,visibility=-1,bool:apply=false,id=0);

/**
 * Sets the MoveType for the player that is handled within the SourceCraft system,
 * Pass MOVETYPE_DEFAULT to not alter the MoveType set by the game or other plugins.
 * @param client:   The index of the player.
 * @param mt:       The MoveType you want to pass.
 * @param apply:    Set to true to apply the setting immedately.
 * @param id:       Identifier to allow more than 1 setting per plugin.
 * @noreturn
 */
native SetMoveType(client,MoveType:mt=MOVETYPE_DEFAULT,bool:apply=false,id=0);

/**
 * Applies the Speed, Gravity and Visibility settings for the specified player
 * right away. This function is not required to change the player's state.
 * If this is not called, the settings will be applied automatically within 2 seconds.
 * @param client:   The client's index to apply the settings for.
 * @noreturn
 */
native ApplyPlayerSettings(client);

/**
 * Sets the physical armor percent used by war3source races
 * @param client:   The index of the player.
 * @param armor:    The armor you want to pass.
 * @param id:       Identifier to allow more than 1 setting per plugin.
 * @noreturn
 */
native SetPhysicalArmorPercent(client,Float:armor=0.0,id=0);

/**
 * Gets the sum of the physical armor percent set by any (war3source) plugins
 * @param client:   The index of the player.
 * @noreturn
 */
native Float:GetPhysicalArmorSum(client);

/**
 * Sets the magical armor percent used by war3source races
 * @param client:   The index of the player.
 * @param armor:    The armor you want to pass.
 * @param id:       Identifier to allow more than 1 setting per plugin.
 * @noreturn
 */
native SetMagicalArmorPercent(client,Float:armor=0.0,id=0);

/**
 * Gets the sum of the magical armor percent set by any (war3source) plugins
 * @param client:   The index of the player.
 * @noreturn
 */
native Float:GetMagicalArmorSum(client);

/**
 * Restores (removes ill effects from) a player (removes all overrides)
 * @param client:   The index of the player.
 * @param apply:    Set to true to apply the setting immedately.
 * @return The return value will be true if the restore succeeded.
 */
native bool:RestorePlayer(client,bool:apply=false);

/**
 * Respawns a player
 * @param client: The index of the player.
 * @noreturn
 * native RespawnPlayer(client);
 */
native RespawnPlayer(client);

/**
 * Creates a Cooldown timer for the given client's given race/upgrade
 * @param client:       The index of the player.
 * @param race:         The race and
 * @param upgrade:      The upgrade for the cooldown
 * @param duration:	    Duration of the cooldown, defaults to value set in AddUpgrade().
 * @param name:         The name of the upgrade to use in the client notification (if any), defaults to upgrade name.
 * @param type:         The type of cooldown (for reset and notification), defaults to value set in AddUpgrade().
 * @param expireSound:  The sound to play when the cooldown expires
 * @return The return value will be true if the restore succeeded.
 */
native CreateCooldown(client, race, upgrade, Float:duration=-1.0, const String:name[]="",
                      CooldownType:type=Cooldown_AutoReset     |
                                        Cooldown_ResetOnChange |
                                        Cooldown_CreateNotify  |
                                        Cooldown_ExpireNotify,
                      const String:expireSound[]="");

/**
 * Returns the time of the cooldown expiration.
 * @param client:   The index of the player.
 * @param race:     The race and
 * @param upgrade:  The upgrade for the cooldown
 * @return The return value will be the time of the cooldown expiration, or 0.0 if there isn't any.
 */
native Float:GetCooldownExpireTime(client, race, upgrade);

#define GetCooldownRemaining(%1) (GetCooldownExpireTime(%1)-GetGameTime())

/**
 * Returns the type of the cooldown.
 * @param client:   The index of the player.
 * @param race:     The race and
 * @param upgrade:  The upgrade for the cooldown
 * @return The return value will be the type of the cooldown, or Cooldown_None if there isn't any.
 */
native CooldownType:GetCooldownType(client, race, upgrade);

/**
 * Check if a cooldown has expired yet.
 * @param client:   The index of the player.
 * @param race:     The race and
 * @param upgrade:  The upgrade for the cooldown
 * @param notify:   Set to true to display a message to the client if the cooldown hasn't expired yet.
 * @return The return value will be true if the cooldown has expired (or there is no cooldown)
 */
native bool:HasCooldownExpired(client, race, upgrade, bool:notify=true);

/**
 * Reset a cooldown
 * @param client:   The index of the player.
 * @param race:     The race and
 * @param upgrade:  The upgrade for the cooldown
 * @param notify:   Set to true to display a message to the client
 * @noreturn
 */
native ResetCooldown(client, race, upgrade, bool:notify=true);

/**
 * Retrieves an integer value from a config file
 * @param key		Name of the key
 * @param defvalue	Optional default value to use if the key is not found.
 * @param fileId:   Optional fileId for the config value (race ID, SHOPITEM or MAIN)
 * @param node:     Optional node for the config value (upgrade ID, item ID or NONE)
 * @param section   Optional section for the config value
 * @noreturn
 */
native GetConfigNum(const String:key[], defvalue=0, fileId=MAIN,
                    node=NONE, const String:section[]="");

/**
 * Retrieves a floating point value from a config file
 * @param key		Name of the key
 * @param defvalue	Optional default value to use if the key is not found.
 * @param fileId:   Optional fileId for the config value (race ID, SHOPITEM or MAIN)
 * @param node:     Optional node for the config value (upgrade ID, item ID or NONE)
 * @param section   Optional section for the config value
 * @noreturn
 */
native Float:GetConfigFloat(const String:key[], Float:defvalue=0.0, fileId=MAIN,
                            node=NONE, const String:section[]="");

/**
 * Retrieves a string value from a config file
 * @param key		Name of the key
 * @param value		Buffer to store key value in.
 * @param maxlength	Maximum length of the value buffer.
 * @param defvalue	Optional default value to use if the key is not found.
 * @param fileId:   Optional fileId for the config value (race ID, SHOPITEM or MAIN)
 * @param node:     Optional node for the config value (upgrade ID, item ID or NONE)
 * @param section   Optional section for the config value
 * @noreturn
 */
native GetConfigString(const String:key[], String:value[], maxlength,
                       const String:defvalue[]="", fileId=MAIN,
                       node=NONE, const String:section[]="");

/**
 * Retrieves an array of integer values from a config file
 * @param key		Name of the key
 * @param value	    Array to store values in.
 * @param size      Size of the value array.
 * @param defvalue	Default values to use if the key is not found.
 * @param fileId:   Optional fileId for the config value (race ID, SHOPITEM or MAIN)
 * @param node:     Optional node for the config value (upgrade ID, item ID or NONE)
 * @param section   Optional section for the config value
 * @return The return value will be the number of elements set.
 */
native GetConfigArray(const String:key[], any:value[], size, const any:defvalue[],
                      fileId=MAIN, node=NONE, const String:section[]="");

/**
 * Retrieves an array of floating point values from a config file
 * @param key		Name of the key
 * @param value	    Array to store values in.
 * @param size      Size of the value array.
 * @param defvalue	Default values to use if the key is not found.
 * @param fileId:   Optional fileId for the config value (race ID, SHOPITEM or MAIN)
 * @param node:     Optional node for the config value (upgrade ID, item ID or NONE)
 * @param section   Optional section for the config value
 * @return The return value will be the number of elements set.
 */
native GetConfigFloatArray(const String:key[], Float:value[], size,
                           const Float:defvalue[], fileId=MAIN,
                           node=NONE, const String:section[]="");

/**
 * Loads a translation file for the Sourcecraft plugin.
 * If no extension is specified, .txt is assumed.
 *
 * @param path		Translation file.
 * @noreturn
 */
native LoadSourceCraftTranslations(const String:file[]);

/**
 * Loads a translation file for the Sourcecraft plugin and the calling plugin.
 * If no extension is specified, .txt is assumed.
 *
 * @param path		Translation file.
 * @noreturn
 */
stock AdditionalTranslations(const String:file[])
{
    LoadTranslations(file);
    LoadSourceCraftTranslations(file);
}

/**
 * Called when SourceCraft is ready.
 * @noreturn
 */
forward OnSourceCraftReady();

/**
 * Gets called when the player has been authed.
 * @param client: The client's index.
 * @noreturn
 */
forward OnPlayerAuthed(client);

/**
 * Gets called when someone changes their race (old race is deselected).
 * @param client:  The client's index.
 * @param oldrace: The player's old race. 
 * @param newrace: The player's new race.
 * @return Plugin_Stop will block race change.
 */
forward Action:OnRaceDeselected(client,oldrace,newrace);

/**
 * Gets called when someone changes their race (new race is selected).
 * @param client:  The client's index.
 * @param oldrace: The player's old race. 
 * @param newrace: The player's new race.
 * @return Plugin_Stop will block race change.
 */
forward Action:OnRaceSelected(client,oldrace,newrace);

/**
 * Gets called when a upgrade level is changed.
 * @param client:   The client's index.
 * @param race:     The race effected.
 * @param upgrade:  The upgrade effected.
 * @param level:    The new upgrade level.
 * @noreturn
 */
forward OnUpgradeLevelChanged(client,race,upgrade,level);

/**
 * Gets called when a player purchases an item.
 * @param client:   The client's index.
 * @param item:     The item that was purchased.
 * @noreturn
 */
forward OnItemPurchase(client,item);

/**
 * Gets called when a player no longer has an item.
 * @param client: The client's index.
 * @param item: The item that was purchased.
 */
forward OnItemLost(client,item);

/**
 * Gets called when the +ultimate or -ultimate command is called,
 * IT ISN'T ALWAYS FOR YOUR RACE, YOU NEED TO CHECK!!!
 * @param client:   The client's index.
 * @param race:     The race for which it was called.
 * @param pressed:  If true, +ultimate, false, -ultimate.
 * @param arg:      Value of the argument (0 for no args)
 * @noreturn
 */
forward OnUltimateCommand(client,race,bool:pressed,arg);

/**
 * Gets called when XP is given to a player
 * @param client: The client's index.
 * @param amount: The amount given (updateable).
 * @param taken:  The amount is to be taken instead of given.
 * @return Plugin_Stop will block XP.
 */
forward Action:OnXPGiven(client,&amount,bool:taken);

/**
 * Gets called when crystals are given to a player
 * @param client: The client's index.
 * @param amount: The amount given (updateable).
 * @param taken:  The amount is to be taken instead of given.
 * @return Plugin_Stop will block Crystals.
 */
forward Action:OnCrystalsGiven(client,&amount,bool:taken);

/**
 * Gets called when vespene is given to a player
 * @param client: The client's index.
 * @param amount: The amount given (updateable).
 * @param taken:  The amount is to be taken instead of given.
 * @return Plugin_Stop will block Vespene.
 */
forward Action:OnVespeneGiven(client,&amount,bool:taken);

/**
 * Gets called when a player has been Restored
 * @param client: The client's index.
 * @return Plugin_Stop will block Restore.
 */
forward Action:OnPlayerRestored(client);

/**
 * Called when a player takes damage, requires SDK Hooks until SM natively supports this.
 * @param victim:     Victim index.
 * @param attacker:   Attacker entity.
 * @param inflictor:  Inflictor entity.
 * @param damage:     How much damage is done.
 * @param damagetype: Type of damage.
 * @return Plugin_Changed will change the damage and Plugin_Handled will block it.
 */      
forward Action:OnPlayerTakeDamage(victim,&attacker,&inflictor,&Float:damage,&damagetype);  

/**
 * Gets called for player_hurt
 * @param event:          The event being processed.
 * @param victim_index:   The victim's index.
 * @param attacker_race:  The race of the victim.
 * @param attacker_index: The attacker's index.
 * @param attacker_race:  The race of the attacker.
 * @param assister_index: The attacker's assister.
 * @param assister_race:  The race of the assister.
 * @param damage:         The amount of damage infliced on the victim.
 * @param absorbed:       The amount of damage that was absorbed by armor.
 * @param from_sc:        Set true if the damage originated from SourceCraft.
 * @noreturn
 */
forward Action:OnPlayerHurtEvent(Handle:event, victim_index, victim_race,
                                 attacker_index, attacker_race, assister_index,
                                 assister_race, damage, absorbed, bool:from_sc);

/**
 * Gets called for player_death
 * @param event:          The event being processed.
 * @param victim_index:   The victim's index.
 * @param attacker_race:  The race of the victim.
 * @param attacker_index: The attacker's index.
 * @param attacker_race:  The race of the attacker.
 * @param assister_index: The attacker's assister.
 * @param assister_race:  The race of the assister.
 * @param damage:         The amount of damage infliced on the victim.
 * @param weapon:         The weapon used by the attacker.
 * @param is_equipment:   Set to true if weapon is client weapon, false if from event.
 * @param customkill:     The custom kill value from TF2.
 * @param headshot:       Set to true for headshots.
 * @param backstab:       Set to true for backstabs.
 * @param melee:          Set to true when a melee weapon (knife, etc) was used.
 * @noreturn
 */
forward OnPlayerDeathEvent(Handle:event, victim_index, victim_race,
                           attacker_index, attacker_race, assister_index,
                           assister_race, damage, const String:weapon[],
                           bool:is_equipment, customkill, bool:headshot,
                           bool:backstab, bool:melee);

/**
 * Gets called for player_spawn
 * @param event:          The event being processed.
 * @param client:         The client's index.
 * @param race:           The race of the client.
 * @noreturn
 */
forward OnPlayerSpawnEvent(Handle:event, client, race);

/**
 * Gets called for player_builtobject, but only when it is 1st built and not when it is moved.
 * @param event:          The event being processed.
 * @param client:         The client's index.
 * @param object:         The object's index.
 * @param type:           The type of the object.
 * @noreturn
 */
forward OnPlayerBuiltObject(Handle:event, client, object, TFObjectType:type);

/**
 * Gets called when a health cabinet is used (opened)
 * @param client:  The index of the client that used the cabinet.
 * @param entity:  The index of the cabinet that was used (opened).
 * @noreturn
 */
forward OnCabinetUsed(client,entity);

/**
 * Gets called when a health kit is used (picked up)
 * @param client:  The index of the client that used the healthkit.
 * @param entity:  The index of the healthkit that was used (picked up).
 * @noreturn
 */
forward OnHealthUsed(client,entity);

/**
 * Called when a Cooldown Expires
 * @param client: The index of the client whose cooldown expired
 * @param race: The race and
 * @param upgrade: The upgrade for which the cooldown has expired.
 * @param expiredByTime: Set true if the full time has expired
 *                       or false if it expired due to death,
 *                       changing races or some other means.
 * @return Plugin_Stop (or Plugin_Handled) will block cooldown when expiredByTime is false
 *                                         or notification when expiredByTime is true
 */
forward OnCooldownExpired(client,race,upgrade,bool:expiredByTime);

/**
 * Do not edit below this line!
 */

public SharedPlugin:__pl_sourcecraft = 
{
	name = "sourcecraft",
	file = "sourcecraft.smx",
#if defined REQUIRE_PLUGIN
	required = 1,
#else
	required = 0,
#endif
};

#if !defined REQUIRE_PLUGIN
public __pl_sourcecraft_SetNTVOptional()
{
	MarkNativeAsOptional("IsSourceCraftLoaded");
	MarkNativeAsOptional("GetRoundState");
	MarkNativeAsOptional("CreateRace");
	MarkNativeAsOptional("SetRaceAttributes");
	MarkNativeAsOptional("RaceComplete");
	MarkNativeAsOptional("AddUpgrade");
	MarkNativeAsOptional("SetUpgradeCooldown");
	MarkNativeAsOptional("GetUpgradeCooldown");
	MarkNativeAsOptional("GetRaceName");
	MarkNativeAsOptional("GetRaceDescription");
	MarkNativeAsOptional("GetRaceShortName");
	MarkNativeAsOptional("GetRaceParentName");
	MarkNativeAsOptional("GetRaceParentId");
	MarkNativeAsOptional("GetRaceFaction");
	MarkNativeAsOptional("GetRaceCount");
	MarkNativeAsOptional("GetUnitTypeName");
	MarkNativeAsOptional("GetFactionName");
	MarkNativeAsOptional("GetFactionDescription");
	MarkNativeAsOptional("GetFactionCrystalSound");
	MarkNativeAsOptional("EmitCrystalSoundToClient");
	MarkNativeAsOptional("GetFactionVespeneSound");
	MarkNativeAsOptional("EmitVespeneSoundToClient");
	MarkNativeAsOptional("GetFactionEnergySound");
	MarkNativeAsOptional("EmitEnergySoundToClient");
	MarkNativeAsOptional("GetFactionLevelSound");
	MarkNativeAsOptional("EmitLevelSoundToClient");
	MarkNativeAsOptional("GetUpgradeCount");
	MarkNativeAsOptional("GetUpgradeName");
	MarkNativeAsOptional("GetUpgradeDescription");
	MarkNativeAsOptional("GetUpgradeShortName");
	MarkNativeAsOptional("GetUpgradeMaxLevel");
	MarkNativeAsOptional("GetUpgradeCategory");
	MarkNativeAsOptional("SetUpgradeCategory");
	MarkNativeAsOptional("GetUpgradeEnergy");
	MarkNativeAsOptional("GetUpgradeRecurringEnergy");
	MarkNativeAsOptional("GetUpgradeVespene");
	MarkNativeAsOptional("GetUpgradeCrystals");
	MarkNativeAsOptional("CreateShopItem");
	MarkNativeAsOptional("GetShopItemCount");
	MarkNativeAsOptional("GetItemShortName");
	MarkNativeAsOptional("SetItemShortName");
	MarkNativeAsOptional("GetItemName");
	MarkNativeAsOptional("GetItemDescription");
	MarkNativeAsOptional("GetItemCategory");
	MarkNativeAsOptional("CreateHelpCommand");
	MarkNativeAsOptional("FindShopItem");
	MarkNativeAsOptional("FindRace");
	MarkNativeAsOptional("HealPlayer");
	MarkNativeAsOptional("HurtPlayer");
	MarkNativeAsOptional("KillPlayer");
	MarkNativeAsOptional("DamageEntity");
	MarkNativeAsOptional("GetDamageFromPlayerHurt");
	MarkNativeAsOptional("GetSupressDamageForward");
	MarkNativeAsOptional("GetDamage");
	MarkNativeAsOptional("DisplayDamage");
	MarkNativeAsOptional("DisplayKill");
	MarkNativeAsOptional("DisplayMessage");
	MarkNativeAsOptional("GetHudMessage");
	MarkNativeAsOptional("HudMessage");
	MarkNativeAsOptional("ClearHud");
	MarkNativeAsOptional("ShowXP");
	MarkNativeAsOptional("ShowCrystals");
	MarkNativeAsOptional("ShowVespene");
	MarkNativeAsOptional("ShowEnergy");
	MarkNativeAsOptional("GetArmor");
	MarkNativeAsOptional("SetArmor");
	MarkNativeAsOptional("SetArmorAmount");
	MarkNativeAsOptional("SetArmorName");
	MarkNativeAsOptional("GetArmorName");
	MarkNativeAsOptional("SetArmorPercent");
	MarkNativeAsOptional("GetArmorPercent");
	MarkNativeAsOptional("IncrementArmor");
	MarkNativeAsOptional("DecrementArmor");
	MarkNativeAsOptional("HasShields");
	MarkNativeAsOptional("GetSettingsBits");
	MarkNativeAsOptional("GetSetting");
	MarkNativeAsOptional("IsChangingClass");
	MarkNativeAsOptional("GetLevel");
	MarkNativeAsOptional("SetLevel");
	MarkNativeAsOptional("ResetLevel");
	MarkNativeAsOptional("GetRace");
	MarkNativeAsOptional("GetXP");
	MarkNativeAsOptional("SetXP");
	MarkNativeAsOptional("ResetXP");
	MarkNativeAsOptional("GetKillXP");
	MarkNativeAsOptional("GetLevelXP");
	MarkNativeAsOptional("GetUpgradeLevel");
	MarkNativeAsOptional("SetUpgradeLevel");
	MarkNativeAsOptional("ResetSetUpgradeLevels");
	MarkNativeAsOptional("GetAccumulatedEnergy");
	MarkNativeAsOptional("SetAccumulatedEnergy");
	MarkNativeAsOptional("GetEnergy");
	MarkNativeAsOptional("SetEnergy");
	MarkNativeAsOptional("GetEnergyRate");
	MarkNativeAsOptional("SetEnergyRate");
	MarkNativeAsOptional("GetEnergyLimit");
	MarkNativeAsOptional("SetEnergyLimit");
	MarkNativeAsOptional("GetInitialEnergy");
	MarkNativeAsOptional("SetInitialEnergy");
	MarkNativeAsOptional("GetMaxCrystals");
	MarkNativeAsOptional("GetCrystals");
	MarkNativeAsOptional("SetCrystals");
	MarkNativeAsOptional("GetVespene");
	MarkNativeAsOptional("SetVespene");
	MarkNativeAsOptional("GetOverallLevel");
	MarkNativeAsOptional("SetOverallLevel");
	MarkNativeAsOptional("GetOwnsItem");
	MarkNativeAsOptional("SetOwnsItem");
	MarkNativeAsOptional("GetAttributeBits");
	MarkNativeAsOptional("SetAttributeBits");
	MarkNativeAsOptional("GetAttribute");
	MarkNativeAsOptional("SetAttribute");
	MarkNativeAsOptional("GetImmunityBits");
	MarkNativeAsOptional("SetImmunityBits");
	MarkNativeAsOptional("GetImmunity");
	MarkNativeAsOptional("SetImmunity");
	MarkNativeAsOptional("SetSpeed");
	MarkNativeAsOptional("SetGravity");
	MarkNativeAsOptional("SetVisibility");
	MarkNativeAsOptional("SetOverrideSpeed");
	MarkNativeAsOptional("SetOverrideGravity");
	MarkNativeAsOptional("SetOverrideVisiblity");
	MarkNativeAsOptional("SetMoveType");
	MarkNativeAsOptional("ApplyPlayerSettings");
	MarkNativeAsOptional("SetPhysicalArmorPercent");
	MarkNativeAsOptional("GetPhysicalArmorSum");
	MarkNativeAsOptional("SetMagicalArmorPercent");
	MarkNativeAsOptional("GetMagicalArmorSum");
	MarkNativeAsOptional("RestorePlayer");
	MarkNativeAsOptional("RespawnPlayer");
	MarkNativeAsOptional("CreateCooldown");
	MarkNativeAsOptional("ResetCooldown");
	MarkNativeAsOptional("GetCooldownExpireTime");
	MarkNativeAsOptional("HasCooldownExpired");
	MarkNativeAsOptional("GetCooldownType");
	MarkNativeAsOptional("GetConfigNum");
	MarkNativeAsOptional("GetConfigFloat");
	MarkNativeAsOptional("GetConfigString");
	MarkNativeAsOptional("GetConfigArray");
	MarkNativeAsOptional("GetConfigFloatArray");
	MarkNativeAsOptional("LoadSourceCraftTranslations");
}
#endif


