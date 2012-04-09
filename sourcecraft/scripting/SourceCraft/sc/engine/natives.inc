/*
 * vim: set ai et ts=4 sw=4 syntax=sourcepawn :
 * File: natives.inc
 * Description: Natives and Forwards
 * Author(s):  -=|JFH|=-Naris (Murray Wilson)
 * Credits: Anthony Iacono  
 */

#include <damage>
#include <tolower>

#include "sc/HealthParticle"
#include "sc/maxhealth"

// Handles
new Handle:g_OnSourceCraftReadyHandle = INVALID_HANDLE;
new Handle:g_OnPlayerAuthedHandle = INVALID_HANDLE;
new Handle:g_OnPlayerRestoredHandle = INVALID_HANDLE;
new Handle:g_OnCrystalsGivenHandle = INVALID_HANDLE;
new Handle:g_OnVespeneGivenHandle = INVALID_HANDLE;
new Handle:g_OnXPGivenHandle = INVALID_HANDLE;
new Handle:g_OnPlayerTakeDamageHandle = INVALID_HANDLE;
new Handle:g_OnPlayerHurtEventHandle = INVALID_HANDLE;
new Handle:g_OnPlayerDeathEventHandle = INVALID_HANDLE;
new Handle:g_OnPlayerSpawnEventHandle = INVALID_HANDLE;
new Handle:g_OnPlayerBuiltObject = INVALID_HANDLE;
new Handle:g_OnCabinetUsedHandle = INVALID_HANDLE;
new Handle:g_OnHealthUsedHandle = INVALID_HANDLE;
new Handle:g_OnUltimateCommandHandle = INVALID_HANDLE;

new bool:g_bDamageFromPlayerHurt = false;
new bool:g_bSupressOnPlayerTakeDamage = false;

/**
 * Retrieves the SourceCraftLoaded flag.
 * @return The return value will be true if SourceCraft has completed loading.
 *
 * native bool:IsSourceCraftLoaded();
 */
public Native_IsSourceCraftLoaded(Handle:plugin,numParams)
{
   return g_bSourceCraftLoaded;
}

/**
 * Gets the state of the round.
 * @return The return value will be the state of the current round.
 *
 * native RoundStates:GetRoundState();
 */
public Native_GetRoundState(Handle:plugin,numParams)
{
   return _:g_RoundState;
}

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
 * @param image:            The image of the race. (defaults to config lookup)
 * @param desc:             The description of the race. (defaults to translation)
 * @return The return value will be the race index or -1 if there was a problem.
 *
 * native CreateRace(const String:short[], required_level=0, tech_level=0, max_level=16,
 *                   initial_energy=30, energy_limit=100, energy_rate=1, Faction:faction=Generic,
 *                   UnitType:type=GenericUnit, const String:parent[]="", Translation:translation=Translation_Auto,
 *                   const String:translation_file[]="", const String:name[]="", const String:switch_message[]="",
 *                   const String:pending_message[]="", const String:image[]="", const String:desc[]="");
 */         
public Native_CreateRace(Handle:plugin,numParams)
{
    decl String:short[SHORT_STRING_LENGTH];
    GetNativeString(1,short,sizeof(short));

    new required_level  = GetNativeCell(2);
    new tech_level      = GetNativeCell(3);
    new max_level       = GetNativeCell(4);
    new initial_energy  = GetNativeCell(5);
    new energy_limit    = GetNativeCell(6);
    new energy_rate     = GetNativeCell(7);
    new Faction:faction = Faction:GetNativeCell(8);
    new UnitType:type   = UnitType:GetNativeCell(9);

    decl String:parent[SHORT_STRING_LENGTH];
    GetNativeString(10,parent,sizeof(parent));

    new Translation:translation = Translation:GetNativeCell(11);

    decl String:translation_file[PLATFORM_MAX_PATH];
    GetNativeString(12,translation_file,sizeof(translation_file));

    decl String:name[NAME_STRING_LENGTH];
    GetNativeString(13,name,sizeof(name));

    decl String:switch_message[LONG_STRING_LENGTH];
    GetNativeString(14,switch_message,sizeof(switch_message));

    decl String:pending_message[LONG_STRING_LENGTH];
    GetNativeString(15,pending_message,sizeof(pending_message));

    decl String:image[PLATFORM_MAX_PATH];
    GetNativeString(16,image,sizeof(image));

    decl String:desc[DESCRIPTION_LENGTH];
    GetNativeString(17,desc,sizeof(desc));

    // If the short name wasn't provided
    if (short[0] == '\0' && name[0] != '\0')
    {
        // generate one
        strcopy(short, sizeof(short), name);
        ReplaceString(short, sizeof(short), " ", "_");
        StringToLower(short);
    }

    return CreateRace(short, required_level, tech_level, max_level, initial_energy,
                      energy_limit, energy_rate, faction, type, parent, translation,
                      translation_file, name, switch_message, pending_message, image,
                      desc);
}

/**
 * Changes the Attributes for an existing race
 * (Also saves them to the database if the create or update flags are set in sourcecraft.cfg)
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
 * 
 * native SetRaceAttributes(race, const String:switch_message[]="",const String:pending_message[]="",
 *                                required_level=-1,tech_level=-1,max_level=-1, initial_energy=-1,energy_limit=-1,
 *                                energy_rate=-1,Faction:faction=Generic, UnitType:type=GenericUnit,
 *                                const String:short[]="", const String:parent[]="", translation=-1);
 */
public Native_SetRaceAttributes(Handle:plugin,numParams)
{
    new Handle:raceHandle = GetRaceHandle(GetNativeCell(1));
    if (raceHandle != INVALID_HANDLE)
    {
        new Handle:messagesHandle=GetArrayCell(raceHandle,RACE_MESSAGES);
        new Handle:stringsHandle=GetArrayCell(raceHandle,RACE_STRINGS);

        new required_level = GetNativeCell(2);
        if (required_level >= 0)
            SetRaceRequiredLevel(raceHandle,required_level);

        new tech_level = GetNativeCell(3);
        if (tech_level >= 0)
            SetRaceTechLevel(raceHandle,tech_level);

        new max_level = GetNativeCell(4);
        if (max_level >= 0)
            SetRaceMaxLevel(raceHandle,max_level);

        new initial_energy = GetNativeCell(5);
        if (initial_energy >= 0)
            SetRaceInitialEnergy(raceHandle,initial_energy);

        new energy_limit = GetNativeCell(6);
        if (energy_limit >= 0)
            SetRaceEnergyLimit(raceHandle,energy_limit);

        new energy_rate = GetNativeCell(7);
        if (energy_rate >= 0)
            SetRaceEnergyRate(raceHandle,energy_rate);

        new Faction:faction = Faction:GetNativeCell(8);
        if (faction > Unassigned)
            SetRaceFaction(raceHandle,faction);

        new UnitType:type = UnitType:GetNativeCell(9);
        if (type > UnassignedUnit)
            SetRaceType(raceHandle,type);

        decl String:parent[SHORT_STRING_LENGTH];
        GetNativeString(10,parent,sizeof(parent));
        if (parent[0] != '\0')
            SetArrayString(stringsHandle,RACE_PARENT_NAME,parent);

        new Translation:translation = Translation:GetNativeCell(11);
        if (translation != Translation_Default)
            SetRaceTranslation(raceHandle,translation);

        decl String:translation_file[PLATFORM_MAX_PATH];
        GetNativeString(12,translation_file,sizeof(translation_file));
        if (translation_file[0] != '\0')
            LoadTranslations(translation_file);

        decl String:name[NAME_STRING_LENGTH];
        GetNativeString(13,name,sizeof(name));
        if (name[0] != '\0')
            SetArrayString(stringsHandle,RACE_NAME,name);

        decl String:switch_message[LONG_STRING_LENGTH];
        GetNativeString(14,switch_message,sizeof(switch_message));
        if (switch_message[0] != '\0')
            SetArrayString(messagesHandle,RACE_SWITCHMESSAGE,switch_message);

        decl String:pending_message[LONG_STRING_LENGTH];
        GetNativeString(15,pending_message,sizeof(pending_message));
        if (pending_message[0] != '\0')
            SetArrayString(messagesHandle,RACE_SWITCHMESSAGE_DEAD,pending_message);

        decl String:short[SHORT_STRING_LENGTH];
        GetNativeString(16,short,sizeof(short));
        if (short[0] != '\0')
            SetArrayString(stringsHandle,RACE_SHORT_NAME,short);

    }
}

/**
 * Can be called after a race has been fully created/defined to save
 * and close the config file. Also saves the attributes to the database
 * (if the create or update flags are set in sourcecraft.cfg)
 * This will be called automatically after OnSourceCraftReady() for any
 * races that haven't specifically called it.
 * @param race:     The race ID returned from CreateRace()
 * @param desc:     The description of the race.
 * @param image:    The image of the race.
 *
 * native RaceComplete(race, const String:desc[]="", const String:image[]="");
 */
public Native_RaceComplete(Handle:plugin,numParams)
{
    decl String:desc[DESCRIPTION_LENGTH];
    GetNativeString(2,desc,sizeof(desc));

    decl String:image[NAME_STRING_LENGTH];
    GetNativeString(3,image,sizeof(image));

    RaceComplete(GetNativeCell(1), desc, image);
}

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
 * @param cost_flag:        If 0, resources will be used to invoke.
 *                          If 1, resources will be used to purchase.
 *                          If 2, resources will be used to purchase and refunded when reset. (default is 0)
 * @param cooldown:         Duration of the cooldown, if any (default is 0.0)
 * @param cooldown_type:    What type of cooldown to use, if any (default is Cooldown_None)
 * @param name:             The name of the upgrade. (max 64, defaults to translation)
 * @param desc:             The description of the upgrade. (max 192, defaults to translation)
 * @return The return value will be the upgrade index or -1 if there was a problem.
 *
 * native AddUpgrade(race, const String:short[], upgrade_category=0, required_level=-1, max_level=4, energy=0,
 *                   recurring_energy=0, vespene=0, crystals=0, cost_flag=COST_PER_USE, Float:cooldown=0.0,
 *                   CooldownType:cooldown_type=Cooldown_None, const String:name[]="", const String:desc[]="",
 *                   const String:p1[]="", const String:p2[]="", const String:p3[]="", const String:p4[]="",
 *                   const String:p5[]="", const String:p6[]="", const String:p7[]="", const String:p8[]="");
 */         
public Native_AddUpgrade(Handle:plugin,numParams)
{
    new race = GetNativeCell(1);
    new Handle:raceHandle = GetRaceHandle(race);
    if (raceHandle != INVALID_HANDLE)
    {
        decl String:short[SHORT_STRING_LENGTH];
        GetNativeString(2,short,sizeof(short));

        new upgrade_category           = GetNativeCell(3);
        new required_level             = GetNativeCell(4);
        new max_level                  = GetNativeCell(5);
        new energy                     = GetNativeCell(6);
        new recurring_energy           = GetNativeCell(7);
        new vespene                    = GetNativeCell(8);
        new crystals                   = GetNativeCell(9);
        new cost_flag                  = GetNativeCell(10);
        new Float:cooldown             = Float:GetNativeCell(11);
        new CooldownType:cooldown_type = CooldownType:GetNativeCell(12);

        decl String:name[NAME_STRING_LENGTH];
        GetNativeString(13,name,sizeof(name));

        decl String:desc[LONG_STRING_LENGTH];
        GetNativeString(14,desc,sizeof(desc));

        decl String:p1[NAME_STRING_LENGTH];
        GetNativeString(15,p1,sizeof(p1));

        decl String:p2[NAME_STRING_LENGTH];
        GetNativeString(15,p1,sizeof(p1));

        decl String:p3[NAME_STRING_LENGTH];
        GetNativeString(15,p1,sizeof(p1));

        decl String:p4[NAME_STRING_LENGTH];
        GetNativeString(15,p1,sizeof(p1));

        decl String:p5[NAME_STRING_LENGTH];
        GetNativeString(15,p1,sizeof(p1));

        decl String:p6[NAME_STRING_LENGTH];
        GetNativeString(15,p1,sizeof(p1));

        decl String:p7[NAME_STRING_LENGTH];
        GetNativeString(15,p1,sizeof(p1));

        decl String:p8[NAME_STRING_LENGTH];
        GetNativeString(15,p1,sizeof(p1));

        // If the short name wasn't provided
        if (short[0] == '\0' && name[0] != '\0')
        {
            // generate one
            strcopy(short, sizeof(short), name);
            ReplaceString(short, sizeof(short), " ", "_");
            StringToLower(short);
        }

        return AddUpgrade(race, raceHandle, short, upgrade_category, required_level, max_level,
                          energy, recurring_energy, vespene, crystals, cost_flag, cooldown,
                          cooldown_type, name, desc, p1, p2, p3, p4, p5, p6, p7, p8);
    }
    else
        return -1;
}

/**
 * Sets an upgrade's cooldown parameters
 * @param race:          The race ID returned from CreateRace()
 * @param upgrade:       The upgrade ID returned from AddUpgrade()
 * @param cooldown:      Duration of the cooldown, if any (default is 0.0)
 * @param cooldown_type: What type of cooldown to use, if any (default is none)
 * @return The return value will be the upgrade index or -1 if there was a problem.
 *
 * native SetUpgradeCooldown(race, upgrade, Float:cooldown=0.0,
 *                           CooldownType:cooldown_flags=Cooldown_None);
 */         
public Native_SetUpgradeCooldown(Handle:plugin,numParams)
{
    new race = GetNativeCell(1);
    new Handle:raceHandle = GetRaceHandle(race);
    if (raceHandle != INVALID_HANDLE)
    {
        new upgrade = GetNativeCell(2);
        new Float:cooldown = Float:GetNativeCell(3);
        new CooldownType:cooldown_type = CooldownType:GetNativeCell(4);
        SetUpgradeCooldown(race, raceHandle, upgrade, cooldown, cooldown_type);
    }
}

/**
 * Gets an upgrade's cooldown parameters
 * @param race:             The race ID returned from CreateRace()
 * @param upgrade:          The upgrade ID returned from AddUpgrade()
 * @param cooldown_type:    Returns what type of cooldown to use
 * @return The return value will be the cooldown duration
 *
 * native GetUpgradeCooldown(race, upgrade, &CooldownType:cooldown_type=Cooldown_None);
 */         
public Native_GetUpgradeCooldown(Handle:plugin,numParams)
{
    new race = GetNativeCell(1);
    new Handle:raceHandle = GetRaceHandle(race);
    if (raceHandle != INVALID_HANDLE)
    {
        new CooldownType:cooldown_type;
        new upgrade = GetNativeCell(2);
        new Float:cooldown = GetUpgradeCooldown(race, raceHandle, upgrade, cooldown_type);
        SetNativeCellRef(3, cooldown_type);
        return _:cooldown;
    }
    return _:-1.0;
}

/**
 * Retrieves the name of the race
 * @param race:      The race ID returned from CreateRace()
 * @param name:      String to place the name into.
 * @param maxlength: The size of the name buffer.
 * @param lang:      Client to translate the name for, if any.
 * @noreturn
 *
 * native GetRaceName(race, String:name[], maxlength, lang=LANG_SERVER);
 */         
public Native_GetRaceName(Handle:plugin,numParams)
{
    new Handle:raceHandle = GetRaceHandle(GetNativeCell(1));
    if (raceHandle != INVALID_HANDLE)
    {
        decl String:name[NAME_STRING_LENGTH];
        new maxlength = GetNativeCell(3);
        new lang = GetNativeCell(4);
        GetRaceName(raceHandle, name, sizeof(name), lang);
        SetNativeString(2, name, maxlength);
    }
}

/**
 * Retrieves the description of the race
 * @param race:         The race ID returned from CreateRace()
 * @param description:  String to place the description into.
 * @param maxlength:    The size of the description buffer.
 * @param lang:         Client to translate the name for, if any.
 * @noreturn
 *
 * native GetRaceDescription(race, String:description[], maxlength, lang=LANG_SERVER);
 */ 
public Native_GetRaceDescription(Handle:plugin,numParams)
{
    new Handle:raceHandle = GetRaceHandle(GetNativeCell(1));
    if (raceHandle != INVALID_HANDLE)
    {
        decl String:description[DESCRIPTION_LENGTH];
        new maxlength = GetNativeCell(3);
        new lang = GetNativeCell(4);
        GetRaceDescription(raceHandle, description, sizeof(description), lang);
        SetNativeString(2, description, maxlength);
    }
}

/**
 * Retrieves the short name of the race
 * @param race: The race ID returned from CreateRace()
 * @param short: String to place the name into.
 * @param maxlength: The size of the name buffer.
 * @noreturn
 *
 * native GetRaceShortName(race, String:name[], maxlength);
 */         
public Native_GetRaceShortName(Handle:plugin,numParams)
{
    new Handle:raceHandle = GetRaceHandle(GetNativeCell(1));
    if (raceHandle != INVALID_HANDLE)
    {
        decl String:short[SHORT_STRING_LENGTH];
        new maxlength = GetNativeCell(3);
        GetRaceShortName(raceHandle, short, sizeof(short));
        SetNativeString(2, short, maxlength);
    }
}

/**
 * Retrieves the name of the race's parent
 * @param race: The race ID returned from CreateRace()
 * @param name: String to place the name into.
 * @param maxlength: The size of the name buffer.
 * @noreturn
 *
 * native GetRaceParentName(race, String:name[], maxlength);
 */         
public Native_GetRaceParentName(Handle:plugin,numParams)
{
    new Handle:raceHandle = GetRaceHandle(GetNativeCell(1));
    if (raceHandle != INVALID_HANDLE)
    {
        decl String:name[SHORT_STRING_LENGTH];
        new maxlength = GetNativeCell(3);
        GetRaceParentName(raceHandle, name, sizeof(name));
        SetNativeString(2, name, maxlength);
    }
}

/**
 * Retrieves the race ID of the race's parent
 * @param race: The race ID returned from CreateRace()
 * @return The race ID of the parent race, 0 for no parent or -1 for error.
 *
 * native GetRaceParentId(race);
 */         
public Native_GetRaceParentId(Handle:plugin,numParams)
{
    new Handle:raceHandle = GetRaceHandle(GetNativeCell(1));
    if (raceHandle != INVALID_HANDLE)
        return GetRaceParentId(raceHandle);
    else        
        return -1;
}

/**
 * Retrieves the faction of a race
 * @param race: The race ID returned from CreateRace()
 * @param validate: Set to true to return only valid faction indices (converts Unassigned to Generic)
 * @return The return value will be the Faction of the race
 *
 * native Faction:GetRaceFaction(race,bool:validate=true);
 */
public Native_GetRaceFaction(Handle:plugin,numParams)
{
    new Handle:raceHandle = GetRaceHandle(GetNativeCell(1));
    if (raceHandle != INVALID_HANDLE)
        return (GetNativeCell(2)) ? (_:GetRaceFactionId(raceHandle)) : (_:GetRaceFaction(raceHandle));
    else
        return _:Generic;
}

/**
 * Retrieves the name of the unit type
 * @param type:      The UnitType
 * @param name:      String to place the name into.
 * @param maxlength: The size of the name buffer.
 * @param lang:      Client to translate the name for, if any.
 * @noreturn
 * 
 * native GetUnitTypeName(UnitType:type, String:name[], maxlength, lang=LANG_SERVER);
 */
public Native_GetUnitTypeName(Handle:plugin,numParams)
{
    decl String:name[NAME_STRING_LENGTH];
    GetUnitTypeName(UnitType:GetNativeCell(1), name, sizeof(name), GetNativeCell(4));
    SetNativeString(2, name, GetNativeCell(3));
}

/**
 * Retrieves the name of the faction
 * @param faction:   The Faction
 * @param name:      String to place the name into.
 * @param maxlength: The size of the name buffer.
 * @param lang:      Client to translate the name for, if any.
 * @noreturn
 *
 * native GetFactionName(Faction:faction, String:name[], maxlength, lang=LANG_SERVER);
 */
public Native_GetFactionName(Handle:plugin,numParams)
{
    decl String:name[NAME_STRING_LENGTH];
    GetFactionName(Faction:GetNativeCell(1), name, sizeof(name), GetNativeCell(4));
    SetNativeString(2, name, GetNativeCell(3));
}

/**
 * Retrieves the description of the faction
 * @param faction:   The Faction
 * @param name:      String to place the name into.
 * @param maxlength: The size of the name buffer.
 * @param lang:      Client to translate the name for, if any.
 * @noreturn
 * 
 * native GetFactionDescription(Faction:faction, String:desc[], maxlength, lang=LANG_SERVER);
 */
public Native_GetFactionDescription(Handle:plugin,numParams)
{
    decl String:desc[NAME_STRING_LENGTH];
    GetFactionDescription(Faction:GetNativeCell(1), desc, sizeof(desc), GetNativeCell(4));
    SetNativeString(2, desc, GetNativeCell(3));
}

/**
 * Retrieves the "Not Enough Crystals" sound for the faction
 * @param faction:   The Faction
 * @param sound:     String to place the sound into.
 * @param maxlength: The size of the name buffer.
 * @param lang:      Client to translate the name for, if any.
 * @noreturn
 *
 * native GetFactionCrystalSound(Faction:faction, String:sound[], maxlength, lang=LANG_SERVER);
 */
public Native_GetFactionCrystalSound(Handle:plugin,numParams)
{
    decl String:sound[NAME_STRING_LENGTH];
    GetFactionCrystalSound(Faction:GetNativeCell(1), sound, sizeof(sound), GetNativeCell(4));
    SetNativeString(2, sound, GetNativeCell(3));
}

/**
 * Emits the "Not Enough Crystals" sound to the client
 * @param client:    The client's index.
 * @param faction:   The Faction (defaults to client's current faction)
 * @noreturn
 * 
 * native EmitCrystalSoundToClient(client, Faction:f=Unassigned);
 */
public Native_EmitCrystalSoundToClient(Handle:plugin,numParams)
{
    new client = GetNativeCell(1);
    new Faction:faction = Faction:GetNativeCell(2);
    if (faction == Unassigned)
    {
        new race = GetRace(client);
        if (race >= 0)
            faction = GetRaceFactionId(GetRaceHandle(race));
        else
            faction = Generic;
    }
    EmitCrystalSoundToClient(client, faction);
}

/**
 * Retrieves the "Not Enough Vespene" sound for the faction
 * @param faction:   The Faction
 * @param sound:     String to place the sound into.
 * @param maxlength: The size of the name buffer.
 * @param lang:      Client to translate the name for, if any.
 * @noreturn
 * 
 * native GetFactionVespeneSound(Faction:f, String:sound[], maxlength, lang=LANG_SERVER);
 */
public Native_GetFactionVespeneSound(Handle:plugin,numParams)
{
    decl String:sound[NAME_STRING_LENGTH];
    GetFactionVespeneSound(Faction:GetNativeCell(1), sound, sizeof(sound), GetNativeCell(4));
    SetNativeString(2, sound, GetNativeCell(3));
}

/**
 * Emits the "Not Enough Vespene" sound to the client
 * @param client:    The client's index.
 * @param faction:   The Faction (defaults to client's current faction)
 * @noreturn
 * 
 * native EmitVespeneSoundToClient(client, Faction:faction=Unassigned);
 */
public Native_EmitVespeneSoundToClient(Handle:plugin,numParams)
{
    new client = GetNativeCell(1);
    new Faction:faction = Faction:GetNativeCell(2);
    if (faction == Unassigned)
    {
        new race = GetRace(client);
        if (race >= 0)
            faction = GetRaceFactionId(GetRaceHandle(race));
        else
            faction = Generic;
    }
    EmitVespeneSoundToClient(client, faction);
}

/**
 * Retrieves the "Not Enough Energy" sound for the faction
 * @param faction:   The Faction
 * @param sound:     String to place the sound into.
 * @param maxlength: The size of the name buffer.
 * @param lang:      Client to translate the name for, if any.
 * @noreturn
 * 
 * native GetFactionEnergySound(Faction:f, String:sound[], maxlength, lang=LANG_SERVER);
 */
public Native_GetFactionEnergySound(Handle:plugin,numParams)
{
    decl String:sound[NAME_STRING_LENGTH];
    GetFactionEnergySound(Faction:GetNativeCell(1), sound, sizeof(sound), GetNativeCell(4));
    SetNativeString(2, sound, GetNativeCell(3));
}

/**
 * Emits the "Not Enough Energy" sound to the client
 * @param client:    The client's index.
 * @param faction:   The Faction (defaults to client's current faction)
 * @noreturn
 * 
 * native EmitEnergySoundToClient(client, Faction:faction=Unassigned);
 */
public Native_EmitEnergySoundToClient(Handle:plugin,numParams)
{
    new client = GetNativeCell(1);
    new Faction:faction = Faction:GetNativeCell(2);
    if (faction == Unassigned)
    {
        new race = GetRace(client);
        if (race >= 0)
            faction = GetRaceFactionId(GetRaceHandle(race));
        else
            faction = Generic;
    }
    EmitEnergySoundToClient(client, faction);
}

/**
 * Retrieves the "Leveled Up" sound for the faction
 * @param faction:   The Faction
 * @param sound:     String to place the sound into.
 * @param maxlength: The size of the name buffer.
 * @param lang:      Client to translate the name for, if any.
 * @noreturn
 * 
 * native GetFactionLevelSound(Faction:f, String:sound[], maxlength, lang=LANG_SERVER);
 */
public Native_GetFactionLevelSound(Handle:plugin,numParams)
{
    decl String:sound[NAME_STRING_LENGTH];
    GetFactionLevelSound(Faction:GetNativeCell(1), sound, sizeof(sound), GetNativeCell(4));
    SetNativeString(2, sound, GetNativeCell(3));
}

/**
 * Emits the "Leveled Up" sound to the client
 * @param client:    The client's index.
 * @param faction:   The Faction (defaults to client's current faction)
 * @noreturn
 * 
 * native EmitLevelSoundToClient(client, Faction:faction=Unassigned);
 */
public Native_EmitLevelSoundToClient(Handle:plugin,numParams)
{
    new client = GetNativeCell(1);
    new Faction:faction = Faction:GetNativeCell(2);
    if (faction == Unassigned)
    {
        new race = GetRace(client);
        if (race >= 0)
            faction = GetRaceFactionId(GetRaceHandle(race));
        else
            faction = Generic;
    }
    EmitLevelSoundToClient(client, faction);
}

/**
 * Retrieves the name of an upgrade
 * @param race:      The race ID returned from CreateRace()
 * @param upgrade:   The upgrade ID returned from AddUpgrade()
 * @param name:      String to place the name into.
 * @param maxlength: The size of the name buffer.
 * @param lang:      Client to translate the name for, if any.
 * @noreturn
 *
 * native GetUpgradeName(race, upgrade, String:name[], maxlength, lang=LANG_SERVER);
 */         
public Native_GetUpgradeName(Handle:plugin,numParams)
{
    new Handle:raceHandle = GetRaceHandle(GetNativeCell(1));
    if (raceHandle != INVALID_HANDLE)
    {
        decl String:name[NAME_STRING_LENGTH];
        new upgrade   = GetNativeCell(2);
        new maxlength = GetNativeCell(4);
        new lang = GetNativeCell(5);
        GetUpgradeName(raceHandle, upgrade, name, sizeof(name), lang);
        SetNativeString(3, name, maxlength);
    }
}

/**
 * Retrieves the description of an upgrade
 * @param race:      The race ID returned from CreateRace()
 * @param upgrade:   The upgrade ID returned from AddUpgrade()
 * @param name:      String to place the description into.
 * @param maxlength: The size of the description buffer.
 * @param lang:      Client to translate the description for, if any.
 * @noreturn
 *
 * native GetUpgradeDescription(race, upgrade, String:description[], maxlength, lang=LANG_SERVER);
 */         
public Native_GetUpgradeDescription(Handle:plugin,numParams)
{
    new Handle:raceHandle = GetRaceHandle(GetNativeCell(1));
    if (raceHandle != INVALID_HANDLE)
    {
        decl String:description[NAME_STRING_LENGTH];
        new upgrade   = GetNativeCell(2);
        new maxlength = GetNativeCell(4);
        new lang = GetNativeCell(5);
        GetUpgradeDescription(raceHandle, upgrade, description, sizeof(description), lang);
        SetNativeString(3, description, maxlength);
    }
}

/**
 * Retrieves the short name of an upgrade
 * @param race:      The race ID returned from CreateRace()
 * @param upgrade:   The upgrade ID returned from AddUpgrade()
 * @param short:     String to place the short name into.
 * @param maxlength: The size of the short name buffer.
 * @param lang:      Client to translate the name for, if any.
 * @noreturn
 *
 * native GetUpgradeShortName(race, upgrade, String:short[], maxlength);
 */         
public Native_GetUpgradeShortName(Handle:plugin,numParams)
{
    new Handle:raceHandle = GetRaceHandle(GetNativeCell(1));
    if (raceHandle != INVALID_HANDLE)
    {
        decl String:short[NAME_STRING_LENGTH];
        new upgrade   = GetNativeCell(2);
        new maxlength = GetNativeCell(4);
        GetUpgradeShortName(raceHandle, upgrade, short, sizeof(short));
        SetNativeString(3, short, maxlength);
    }
}

/**
 * Retrieves the max level of an upgrade
 * @param race:     The race ID returned from CreateRace()
 * @param upgrade:  The upgrade ID returned from AddUpgrade()
 * @return The return value will be the max level of the upgrade or -1 if it failed.
 *
 * native GetUpgradeMaxLevel(race,upgrade);
 */         
public Native_GetUpgradeMaxLevel(Handle:plugin,numParams)
{
    new Handle:raceHandle = GetRaceHandle(GetNativeCell(1));
    if (raceHandle != INVALID_HANDLE)
        return GetUpgradeMaxLevel(raceHandle, GetNativeCell(2));
    else
        return -1;
}

/**
 * Retrieves the category of an upgrade
 * @param race:     The race ID returned from CreateRace()
 * @param upgrade:  The upgrade ID returned from AddUpgrade()
 * @return The return value will be the category of the upgrade or -1 if it failed.
 *
 * native GetUpgradeCategory(race,upgrade);
 */         
public Native_GetUpgradeCategory(Handle:plugin,numParams)
{
    new Handle:raceHandle = GetRaceHandle(GetNativeCell(1));
    if (raceHandle != INVALID_HANDLE)
        return GetUpgradeCategory(raceHandle, GetNativeCell(2));
    else
        return -1;
}

/**
 * Sets the category of an upgrade
 * @param race:             The race ID returned from CreateRace()
 * @param upgrade:          The upgrade ID returned from AddUpgrade()
 * @param upgrade_category: Set to 0 for a "Normal" Upgrade or a category number for an ultimate.
 * @noreturn
 *
 * native SetUpgradeCategory(race,upgrade);
 */         
public Native_SetUpgradeCategory(Handle:plugin,numParams)
{
    new Handle:raceHandle = GetRaceHandle(GetNativeCell(1));
    if (raceHandle != INVALID_HANDLE)
        SetUpgradeCategory(raceHandle, GetNativeCell(2), GetNativeCell(3));
}

/**
 * Retrieves the amount of energy used by an upgrade
 * @param race: The race ID returned from CreateRace()
 * @param upgrade: The upgrade ID returned from AddUpgrade()
 * @return The return value will be the amount of energy or -1 if it failed.
 *
 * native GetUpgradeEnergy(race,upgrade);
 */         
public Native_GetUpgradeEnergy(Handle:plugin,numParams)
{
    new Handle:raceHandle = GetRaceHandle(GetNativeCell(1));
    if (raceHandle != INVALID_HANDLE)
        return GetUpgradeEnergy(raceHandle, GetNativeCell(2));
    else
        return -1;
}

/**
 * Retrieves the recurring amount of energy used by an upgrade
 * @param race: The race ID returned from CreateRace()
 * @param upgrade: The upgrade ID returned from AddUpgrade()
 * @return The return value will be the amount of energy or -1 if it failed.
 *
 * native GetUpgradeRecurringEnergy(race,upgrade);
 */         
public N_GetUpgradeRecurringEnergy(Handle:plugin,numParams)
{
    new Handle:raceHandle = GetRaceHandle(GetNativeCell(1));
    if (raceHandle != INVALID_HANDLE)
        return GetUpgradeRecurringEnergy(raceHandle, GetNativeCell(2));
    else
        return -1;
}

/**
 * Retrieves the amount of vespene used by an upgrade
 * @param race: The race ID returned from CreateRace()
 * @param upgrade: The upgrade ID returned from AddUpgrade()
 * @return The return value will be the amount of vespene or -1 if it failed.
 *
 * native GetUpgradeVespene(race,upgrade);
 */         
public Native_GetUpgradeVespene(Handle:plugin,numParams)
{
    new Handle:raceHandle = GetRaceHandle(GetNativeCell(1));
    if (raceHandle != INVALID_HANDLE)
        return GetUpgradeVespene(raceHandle, GetNativeCell(2));
    else
        return -1;
}

/**
 * Retrieves the amount of crystals used by an upgrade
 * @param race: The race ID returned from CreateRace()
 * @param upgrade: The upgrade ID returned from AddUpgrade()
 * @return The return value will be the amount of crystals or -1 if it failed.
 *
 * native GetUpgradeCrystals(race,upgrade);
 */         
public Native_GetUpgradeCrystals(Handle:plugin,numParams)
{
    new Handle:raceHandle = GetRaceHandle(GetNativeCell(1));
    if (raceHandle != INVALID_HANDLE)
        return GetUpgradeCrystals(raceHandle, GetNativeCell(2));
    else
        return -1;
}

/**
 * Retrieves the number of races
 * @return The return value will be the races or -1 for error.
 *
 * native GetRaceCount(race,upgrade);
 */         
public Native_GetRaceCount(Handle:plugin,numParams)
{
    if (raceArray != INVALID_HANDLE)
        return GetRaceCount();
    else
        return -1;
}

/**
 * Retrieves the number of upgrades a race has
 * @param race: The race ID returned from CreateRace()
 * @return The return value will be the number of upgrades a race has or -1 for error.
 *
 * native GetUpgradeCount(race,upgrade);
 */         
public Native_GetUpgradeCount(Handle:plugin,numParams)
{
    new Handle:raceHandle = GetRaceHandle(GetNativeCell(1));
    if (raceHandle != INVALID_HANDLE)
        return GetUpgradeCount(raceHandle);
    else
        return -1;
}

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
 *
 * native CreateShopItem(String:name[],String:short[],String:desc[],cost,vespene=0,max=1,required_level=0);
 */         
public Native_CreateShopItem(Handle:plugin,numParams)
{
    decl String:short[SHORT_STRING_LENGTH];
    GetNativeString(1,short,sizeof(short));

    new crystals = GetNativeCell(2);
    new vespene = GetNativeCell(3);
    new max = GetNativeCell(4);
    new required_level = GetNativeCell(5);

    new Translation:translation = Translation:GetNativeCell(6);

    decl String:translation_file[PLATFORM_MAX_PATH];
    GetNativeString(7,translation_file,sizeof(translation_file));

    decl String:category[NAME_STRING_LENGTH];
    GetNativeString(8,category,sizeof(category));

    decl String:name[NAME_STRING_LENGTH];
    GetNativeString(9,name,sizeof(name));

    decl String:desc[LONG_STRING_LENGTH];
    GetNativeString(10,desc,sizeof(desc));

    return CreateShopItem(short,crystals,vespene,max,required_level,
                          translation,translation_file,category,name,desc);
}

/**
 * Retrieves the number of shop items
 * @return The return value will be the number of shop items or -1 for error.
 *
 * native GetShopItemCount(race,upgrade);
 */         
public Native_GetShopItemCount(Handle:plugin,numParams)
{
    if (shopVector != INVALID_HANDLE)
        return GetShopItemCount();
    else
        return -1;
}

/**
 * Retrieves the name of the shop item
 * @param item:      The item ID returned from CreateShopItem
 * @param name:      String to place the name into.
 * @param maxlength: The size of the name buffer.
 * @param lang:      Client to translate the name for, if any.
 * @noreturn
 *
 * native GetItemName(item, String:name[], maxlength, lang=LANG_SERVER);
 */         
public Native_GetItemName(Handle:plugin,numParams)
{
    new Handle:itemHandle = GetShopItemHandle(GetNativeCell(1));
    if (itemHandle != INVALID_HANDLE)
    {
        decl String:name[NAME_STRING_LENGTH];
        new maxlength = GetNativeCell(3);
        new lang = GetNativeCell(4);
        GetItemName(itemHandle, name, sizeof(name), lang);
        SetNativeString(2, name, maxlength);
    }
}

/**
 * Retrieves the description of the shop item
 * @param item:         The item ID returned from CreateShopItem
 * @param description:  String to place the description into.
 * @param maxlength:    The size of the description buffer.
 * @param lang:         Client to translate the name for, if any.
 * @noreturn
 *
 * native GetItemDescription(item, String:description[], maxlength, lang=LANG_SERVER);
 */ 
public Native_GetItemDescription(Handle:plugin,numParams)
{
    new Handle:itemHandle = GetShopItemHandle(GetNativeCell(1));
    if (itemHandle != INVALID_HANDLE)
    {
        decl String:description[DESCRIPTION_LENGTH];
        new maxlength = GetNativeCell(3);
        new lang = GetNativeCell(4);
        GetItemDescription(itemHandle, description, sizeof(description), lang);
        SetNativeString(2, description, maxlength);
    }
}


/**
 * Retrieves the category of the shop item
 * @param item:         The item ID returned from CreateShopItem
 * @param category:     String to place the category into.
 * @param maxlength:    The size of the category buffer.
 * @param lang:         Client to translate the name for, if any.
 * @noreturn
 *
 * native GetItemCategory(item, String:category[], maxlength, lang=LANG_SERVER);
 */ 
public Native_GetItemCategory(Handle:plugin,numParams)
{
    new Handle:itemHandle = GetShopItemHandle(GetNativeCell(1));
    if (itemHandle != INVALID_HANDLE)
    {
        decl String:category[NAME_STRING_LENGTH];
        new maxlength = GetNativeCell(3);
        new lang = GetNativeCell(4);
        GetItemCategory(itemHandle, category, sizeof(category), lang);
        SetNativeString(2, category, maxlength);
    }
}

/**
 * Retrieves the short name of the shop item
 * @param item:      The item ID returned from CreateShopItem
 * @param name:      String to place the name into.
 * @param maxlength: The size of the name buffer.
 * @noreturn
 *
 * native GetItemShortName(item, String:name[], maxlength);
 */         
public Native_GetItemShortName(Handle:plugin,numParams)
{
    new Handle:itemHandle = GetShopItemHandle(GetNativeCell(1));
    if (itemHandle != INVALID_HANDLE)
    {
        decl String:name[NAME_STRING_LENGTH];
        new maxlength = GetNativeCell(3);
        GetItemShortName(itemHandle, name, sizeof(name));
        SetNativeString(2, name, maxlength);
    }
}

/**
 * Changes a ShopItem's short name.
 * @param item: The item ID returned from CreateShopItem
 * @param short: The new short name of the item. (max 16) 
 * @return The return value will be the item index or -1 if there was a problem.
 *
 * native SetItemShortName(const String:short[]);
 */
public Native_SetItemShortName(Handle:plugin,numParams)
{
    new item=GetNativeCell(1);
    new Handle:itemHandle=GetShopItemHandle(item);
    if (itemHandle != INVALID_HANDLE)
    {
        decl String:short[SHORT_STRING_LENGTH];
        GetNativeString(2,short,sizeof(short));
        SetItemShortName(itemHandle,short);
    }
}

/**
 * Registers a help command with the SourceCraft plugin for help
 * @param name: The name of the command. (max 64)
 * @param desc: The description of the command. (max 256) 
 * @return The return value will be the help index or -1 if there was a problem.
 *
 * native CreateHelpCommand(const String:name[],const String:desc[]);
 */
public Native_CreateHelpCommand(Handle:plugin,numParams)
{
    decl String:name[NAME_STRING_LENGTH];
    decl String:desc[LONG_STRING_LENGTH];
    GetNativeString(1,name,sizeof(name));
    GetNativeString(2,desc,sizeof(desc));
    return NewHelpCommand(name,desc);
}

/**
 * Find the ShopItem ID of a shop item
 * @param name: The name of the item. (max 64)
 * @return The return value will be the item index or -1 if there was a problem.
 *
 * native FindShopItem(const String:name[]);
 */         
public Native_FindShopItem(Handle:plugin,numParams)
{
    decl String:name[SHORT_STRING_LENGTH];
    GetNativeString(1,name,sizeof(name));
    return FindShopItem(name);
}

/**
 * Find the Race ID of a race
 * @param name: The short name of the race. (max 64)
 * @return The return value will be the race index or -1 if there was a problem.
 *
 * native FindRace(const String:name[]);
 */         
public Native_FindRace(Handle:plugin,numParams)
{
    decl String:name[SHORT_STRING_LENGTH];
    GetNativeString(1,name,sizeof(name));
    return FindRace(name);
}

/**
 * Give health to (heal) a player
 * @param client: The client's index you want.
 * @param amount: The amount of health to give to the client.
 * @param health: The health of the client.
 * @param max:    The max health of the client.
 * @return        The return value will be the amount healed
 *
 * native HealPlayer(client, amount,health=0,max=0);
 */
public Native_HealPlayer(Handle:plugin,numParams)
{
    new client = GetNativeCell(1);
    new amount = GetNativeCell(2);
    new health = GetNativeCell(3);
    new maxhp  = GetNativeCell(4);

    if (health < 0)
        health = GetClientHealth(client);

    if (maxhp < 0)
        maxhp = GetPlayerMaxHealth(client);

    new new_health = health + amount;
    if (new_health > maxhp && maxhp > 0)
        new_health = maxhp;

    if (new_health > health)
    {
        SetEntityHealth(client,new_health);
        ShowHealthParticle(client);

        #if defined TRACK_DAMAGE
            SaveHealth(client);
        #endif
    }
    return new_health - health;
}

/**
 * Take health from (damage) a player
 * @param victim:           The victim's index you want.
 * @param damage:           The amount to damage to deal to the client.
 * @param attacker:         The index of the attacker.
 * @param weapon_name:      The name of the weapon used to deal the damage to the client.
 * @param weapon_desc:      The description of the weapon used to deal the damage to the client.
 * @param xp:               Amount of extra xp for a kill (if any).
 * @param explode:          Set to true to cause the player to explode when they die.
 * @param type:             What damage bit flags to use (See DMG_ definitions in dmgbits.inc)
 * @param category:         What DamageFrom bit flags (damage categories) to use
 * @param limit:            Set to limit the damage to the % of the victim's class health (0 == unlimited, default is 90%)
 * @param ignore_armor:     Set to true to NOT use the victim's armor.
 * @param ignore_immunity:  Set to true to ignore the victim's immunities.
 * @param in_hurt_event:    Set to true when called from player_hurt event.
 * @param no_suicide:       Set to true to NOT force suicide when health<=0.
 * @param no_forward:       Set to true to supress damage forwarding (OnPlayerTakeDamage & player_event)
 * @param no_translate:     Set to true to supress translating the weapon_name to get the weapon_desc
 * @param prevent_stack:    Set to true to prevent other sourcecraft damage mods from stacking (defaults to true).
 * @return The return value will be the victims remaining health
 *
 * native HurtPlayer(victim, damage, attacker=0, const String:weapon_name[]="",
 *                   const String:weapon_desc[]="", xp=0, bool:explode=false,
 *                   type=DMG_GENERIC, DamageFrom:category=DamageFrom_None,
 *                   Float:limit=0.90, bool:ignore_armor=false, bool:ignore_immunity=false,
 *                   bool:in_hurt_event=false, bool:no_suicide=false, bool:no_forward=false,
 *                   bool:no_translate=false, bool:prevent_stack=true);
 */
public Native_HurtPlayer(Handle:plugin,numParams)
{
    new victim = GetNativeCell(1);

    SetTraceCategory("Damage,Immunity");
    TraceInto("Native", "Native_HurtPlayer", "victim=%d:%L, killed=%d", \
              victim, ValidClientIndex(victim), g_Killed[victim]);

    if (victim && !g_Killed[victim])
    {
        new health;
        new damage = GetNativeCell(2);
        new attacker = GetNativeCell(3);

        new xp = GetNativeCell(6);
        new bool:explode = bool:GetNativeCell(7);
        new type = GetNativeCell(8);
        new DamageFrom:category = DamageFrom:GetNativeCell(9);
        new Float:limit = Float:GetNativeCell(10);
        new bool:ignore_armor = bool:GetNativeCell(11);
        new bool:ignore_immunity = bool:GetNativeCell(12);
        new bool:in_hurt_event = bool:GetNativeCell(13);
        new bool:no_suicide = bool:GetNativeCell(14);
        new bool:no_forward = bool:GetNativeCell(15);
        new bool:no_translate = bool:GetNativeCell(16);
        new bool:prevent_stack = bool:GetNativeCell(17);

        decl String:weapon_name[NAME_STRING_LENGTH];
        GetNativeString(4,weapon_name,sizeof(weapon_name));

        decl String:weapon_desc[NAME_STRING_LENGTH];
        GetNativeString(5,weapon_desc,sizeof(weapon_desc));

        Trace("HurtPlayer: victim=%d:%N, attacker=%d:%N, damage=%d,limit=%d,ignore_armor=%d,in_hurt_event=%d, no_suicide=%d, no_forward=%d, category=0x%08x, ignore_immunity=%d", \
              victim, ValidClientIndex(victim), attacker, ValidClientIndex(attacker), damage, limit, ignore_armor, in_hurt_event, \
              no_suicide, no_forward, category, ignore_immunity);

        if (!ignore_immunity)
        {
            new immunities = _:GetImmunityBits(victim);
            if (_:category & immunities)
            {
                // If the damage category bits have any of the bits in the immunites

                TraceReturn();
                return 0;
            }
        }

        if (limit > 0.0)
        {
            new max = RoundToFloor(float(GetPlayerMaxHealth(victim)) * limit);
            if (damage > max)
                damage = max;
        }

        if (prevent_stack)
            g_InjuredBy[victim] = attacker;

        if (in_hurt_event || ignore_armor ||
            (victim != attacker) && GetClientTeam(victim) == GetClientTeam(attacker))
        {
            new absorbed;
            if (ignore_armor)
                absorbed = 0;
            else
            {
                new armor = GetArmor(victim);
                absorbed = RoundFloat(float(damage)
                           * GetRandomFloat(m_ArmorPercent[victim][0],
                                            m_ArmorPercent[victim][1]));
                if (absorbed > armor)
                    absorbed = armor;
                if (absorbed > 0)
                {
                    DecrementArmor(victim,absorbed);
                    damage -= absorbed;
                }

                TraceCat("Armor", "%d:%N's armor absorbed %d of %d, burrowed=%d, m_ArmorPercent=%1.2f,%1.2f, m_ArmorName='%s'", \
                         victim,ValidClientIndex(victim), absorbed, damage, \
                         GetAttribute(victim, Attribute_IsBurrowed), \
                         m_ArmorPercent[victim][0], \
                         m_ArmorPercent[victim][1], \
                         m_ArmorName[victim]);
            }

            health = GetClientHealth(victim) - damage;
            if (health <= 0)
            {
                Trace("%d:%N killed %d:%N with damage", \
                      victim, ValidClientIndex(victim), \
                      attacker, ValidClientIndex(attacker));

                KillPlayer(victim, attacker, weapon_name, weapon_desc,
                           xp, explode, type, false, no_suicide,
                           no_translate, health);

                health = 0;
            }
            else
            {
                Trace("%d:%N damaged %d:%N for %d, reducing health", \
                      victim, ValidClientIndex(victim), \
                      attacker, ValidClientIndex(attacker), \
                      damage);

                DisplayDamage(attacker, victim, damage+absorbed, weapon_name,
                              weapon_desc, no_translate);

                SetEntityHealth(victim,health);
                EmitSoundToAll(sndPain[GetRandomInt(0,sizeof(sndPain)-1)], victim);

                if (!in_hurt_event && !no_forward)
                {
                    new Handle:event = CreateEvent("player_hurt");
                    if (event != INVALID_HANDLE)
                    {
                        SetEventInt(event, "userid", GetClientUserId(victim));
                        SetEventInt(event, "attacker", (attacker > 0) ? GetClientUserId(attacker) : 0);
                        SetEventInt(event, "health", health);
                        SetEventInt(event, "absorbed", absorbed);
                        SetEventString(event, "weapon", weapon_name);

                        switch (GameType)
                        {
                            case tf2:       SetEventInt(event, "damageamount", damage);
                            case cstrike:   SetEventInt(event, "dmg_health", damage);
                            case dod:       SetEventInt(event, "damage", damage);
                            default:        SetEventInt(event, "damage", damage);
                        }

                        if (prevent_stack)
                            SetEventBool(event, "sourcecraft", prevent_stack);

                        FireEvent(event);
                    }
                }

                #if defined TRACK_DAMAGE
                    SaveHealth(victim);
                #endif
            }
        }
        else
        {
            Trace("%d:%N was damaged by %d:%N for %d, Dealing Damage, suppress=%d, health=%d", \
                  victim, ValidClientIndex(victim), attacker, ValidClientIndex(attacker), \
                  damage, no_forward, GetClientHealth(victim));

            g_bDamageFromPlayerHurt = true;
            g_bSupressOnPlayerTakeDamage = no_forward;
            DamagePlayer(victim,damage,attacker,type,weapon_name);

            Trace("After Damage, %d:%N's health=%d, alive=%d", \
                  victim, ValidClientIndex(victim), GetClientHealth(victim), \
                  IsPlayerAlive(victim));


            if (IsPlayerAlive(victim) && GetClientHealth(victim) > 0)
            {
                DisplayDamage(attacker, victim, damage, weapon_name,
                              weapon_desc, no_translate);
            }
            else
            {
                g_Killed[victim] = true;
                g_KilledBy[victim] = attacker;

                if (xp != 0 && attacker > 0 && victim != attacker)
                {
                    if (GetClientTeam(attacker) == GetClientTeam(victim))
                        xp = 0; // No points for Team kills!
                    else if (GetRaceCount() > 1 && GetClientCount() >= g_iMinPlayers)
                    {
                        new race=GetRace(attacker);
                        new newxp=GetXP(attacker,race)+xp;
                        SetXP(attacker,race,newxp);
                    }
                }

                DisplayKill(attacker, victim, xp, weapon_name,
                            weapon_desc, no_translate);
            }

            #if defined TRACK_DAMAGE
                health = SaveHealth(victim);
            #else
                health = GetClientHealth(victim);
            #endif
        }

        TraceReturn("health=%d", health);
        return health;
    }

    TraceReturn("health=0, already killed");
    return 0;
}

/**
 * Kill a player.
 * @param victim: The victim's index.
 * @param attacker: The index of the attacker.
 * @param weapon_name:   The name of the weapon used to deal the damage to the client.
 * @param weapon_desc:   The description of the weapon used to deal the damage to the client.
 * @param xp:            Amount of extra xp for a kill.
 * @param explode:       Set to true to cause the player to explode.
 * @param type:          What damage bit flags to use (See DMG_ definitions in dmgbits.inc)
 * @param silent:        Set to true to NOT display a kill message.
 * @param no_suicide:    Set to true to NOT force suicide and only set health=0.
 * @param no_translate:  Set to true to supress translating the weapon_name to get the weapon_desc
 * @noreturn
 *
 * native KillPlayer(victim, attacker=0, const String:weapon_name[]="",
 *                   const String:weapon_desc[]="", xp=0,
 *                   bool:explode=false, type=DMG_GENERIC,
 *                   bool:silent=false, bool:no_suicide=false,
 *                   bool:no_translate=false);
 */
public Native_KillPlayer(Handle:plugin,numParams)
{
    new victim = GetNativeCell(1);
    if (victim)
    {
        new attacker = GetNativeCell(2);

        new xp = GetNativeCell(5);
        new bool:explode = GetNativeCell(6);
        new type = GetNativeCell(7);
        new bool:silent = GetNativeCell(8);
        new bool:no_suicide = GetNativeCell(9);
        new bool:no_translate = bool:GetNativeCell(10);

        decl String:weapon_name[NAME_STRING_LENGTH];
        GetNativeString(3,weapon_name,sizeof(weapon_name));

        decl String:weapon_desc[NAME_STRING_LENGTH];
        GetNativeString(4,weapon_desc,sizeof(weapon_desc));

        KillPlayer(victim, attacker, weapon_name, weapon_desc,
                   xp, explode, type, silent, no_suicide,
                   no_translate, 0);

        #if defined TRACK_DAMAGE
            SaveHealth(victim);
        #endif

        return 0;
    }
    else
        return -1;
} 

KillPlayer(victim, attacker, const String:weapon_name[], const String:weapon_desc[],
           xp=0, bool:explode=false, type=DMG_GENERIC, bool:silent=false,
           bool:no_suicide=false, bool:no_translate=false, health=0)
{
    TraceInto("Native", "KillPlayer", "victim=%d:%L, attacker=%d:%L", \
              victim, ValidClientIndex(victim), attacker, ValidClientIndex(attacker));

    if (victim > 0)
    {
        g_Killed[victim] = true;
        g_KilledBy[victim] = attacker;

        if (explode && GameType == tf2)
            FakeClientCommand(victim, "explode");
        else if (attacker > 0 && attacker != victim)
        {
            if (health <= 0)
                health = GetClientHealth(victim);

            if (health > 0)
            {
                DamagePlayer(victim,health+GetArmor(victim),attacker,type,weapon_name);

                //Ensure the player died.
                CreateTimer(0.1, KillPlayerTimer, GetClientUserId(victim),
                            TIMER_REPEAT|TIMER_FLAG_NO_MAPCHANGE);
            }
            else
            {
                ForcePlayerSuicide(victim);

                #if defined _TRACE
                    CreateTimer(0.1, TrackSuicide, GetClientUserId(victim),
                                TIMER_REPEAT|TIMER_FLAG_NO_MAPCHANGE);
                #endif
            }
        }
        else if (no_suicide)
        {
            SetEntityHealth(victim, 0);

            #if defined _TRACE
                CreateTimer(0.1, TrackKill, GetClientUserId(victim),
                            TIMER_REPEAT|TIMER_FLAG_NO_MAPCHANGE);
            #endif
        }
        else
        {
            ForcePlayerSuicide(victim);

            #if defined _TRACE
                CreateTimer(0.1, TrackSuicide, GetClientUserId(victim),
                            TIMER_REPEAT|TIMER_FLAG_NO_MAPCHANGE);
            #endif
        }

        if (xp != 0 && attacker > 0 && victim != attacker)
        {
            if (GetClientTeam(attacker) == GetClientTeam(victim))
                xp = 0; // No points for Team kills!
            else if (GetRaceCount() > 1 && GetClientCount() >= g_iMinPlayers)
            {
                new race=GetRace(attacker);
                new newxp=GetXP(attacker,race)+xp;
                SetXP(attacker,race,newxp);
            }
        }

        // Don't display message for console kills
        if (attacker > 0 && !silent)
        {
            DisplayKill(attacker, victim, xp, weapon_name,
                        weapon_desc, no_translate);
        }
    }

    TraceReturn();
}

public Action:KillPlayerTimer(Handle:timer,any:userid)
{
    new client = GetClientOfUserId(userid);

    TraceInto("Native", "KillPlayerTimer", "userid=%d, client=%d:%L", \
              userid, client, ValidClientIndex(client));

    if (g_Killed[client] && IsValidClient(client) && IsPlayerAlive(client))
    {
        #if defined _TRACE
            new max = GetMaxHealth(client);
            new health = GetClientHealth(client);
            new killedBy = g_KilledBy[client];
            if (killedBy > 0)
            {
                 Trace("%L was killed by damage (was killed by %L) and is still alive (health=%d/%d), Forcing Suicide: RoundState=%d", \
                       client, killedBy, health, max, g_RoundState);
            }
            else
            {
                Trace("%L was killed by damage and is still alive (health=%d/%d), Forcing Suicide: RoundState=%d", \
                      client, health, max, g_RoundState);
            }

            CreateTimer(0.1, TrackSuicide, GetClientUserId(client),
                        TIMER_REPEAT|TIMER_FLAG_NO_MAPCHANGE);
        #endif

        ForcePlayerSuicide(client);
    }

    TraceReturn();
    return Plugin_Stop;
}

#if defined _TRACE
public Action:TrackSuicide(Handle:timer,any:userid)
{
    new client = GetClientOfUserId(userid);
    TraceInto("Native", "TrackSuicide", "userid=%d, client=%d:%L", \
              userid, client, ValidClientIndex(client));

    if (IsValidClient(client) && IsPlayerAlive(client))
    {
        new killedBy = g_KilledBy[client];
        if (killedBy > 0)
        {
            if (IsClientInGame(killedBy))
            {
                Trace("%L forced suicide (was killed by %L) and is still alive, RoundState=%d", \
                      client, killedBy, g_RoundState);
            }
            else
            {
                Trace("%L forced suicide (was killed by #%d) and is still alive, RoundState=%d", \
                      client, killedBy, g_RoundState);
            }

            TraceReturn();
            return Plugin_Continue;
        }
        else
        {
            Trace("%L forced suicide (was killed) and is still alive, RoundState=%d", \
                  client, g_RoundState);
        }
    }

    TraceReturn();
    return Plugin_Stop;
}

public Action:TrackKill(Handle:timer,any:userid)
{
    new client = GetClientOfUserId(userid);
    TraceInto("Native", "TrackKill", "userid=%d, client=%d:%L", \
              userid, client, ValidClientIndex(client));

    if (IsValidClient(client) && IsPlayerAlive(client))
    {
        new killedBy = g_KilledBy[client];
        if (killedBy > 0)
        {
            if (IsClientInGame(killedBy))
            {
                Trace("%L was killed by %L and is still alive, RoundState=%d", \
                      client, killedBy, g_RoundState);
            }
            else
            {
                Trace("%L was killed by #%d and is still alive, RoundState=%d", \
                      client, killedBy, g_RoundState);
            }

            TraceReturn();
            return Plugin_Continue;
        }
        else
        {
            Trace("%L was killed and is still alive, RoundState=%d", \
                  client, g_RoundState);
        }
    }

    TraceReturn();
    return Plugin_Stop;
}
#endif

/**
 * Take health from (damage) an entity
 * @param victim: The entity you want to damage.
 * @param damage: The amount to damage to deal to the victim.
 * @param attacker: The index of the attacker.
 * @param dmg_type: What damage bit flags to use (See DMG_ definitions in dmgbits.inc)
 * @param weapon: The name of the weapon used to deal the damage to the entity.
 * @noreturn
 *
 * native DamageEntity(victim,damage,attacker=0,dmg_type=DMG_GENERIC,const String:weapon[]="");
 */
public Native_DamageEntity(Handle:plugin,numParams)
{
    new victim = GetNativeCell(1);
    new damage = GetNativeCell(2);
    new attacker = GetNativeCell(3);
    new dmg_type = GetNativeCell(4);

    decl String:weapon[NAME_STRING_LENGTH];
    GetNativeString(5,weapon,sizeof(weapon));

    DamageEntity(victim,damage,attacker,dmg_type,weapon);
}

/**
 * Retrieves the Damage From PlayerHurt Flag set in HurtPlayer()
 * only valid in OnTakePlayerDamage() and OnPlayerHurt() Handlers.
 *
 * @return The return value will be the Suppress Damage Forwarding flag.
 *
 * native bool:GetDamageFromPlayerHurt();
 */
public Native_GetDamageFromPlayerHurt(Handle:plugin,numParams)
{
    return g_bDamageFromPlayerHurt;
}

/**
 * Retrieves the Suppress Damage Forwarding Flag set in HurtPlayer()
 * only valid in OnTakePlayerDamage() and OnPlayerHurt() Handlers.
 *
 * @return The return value will be the Suppress Damage Forwarding flag.
 *
 * native bool:GetSuppressDamageForward();
 */
public Native_GetSuppressDamageForward(Handle:plugin,numParams)
{
    return g_bSupressOnPlayerTakeDamage;
}

/**
 * Get amount of Damage
 * Retrieves the damage done by the event, calculates it if required.
 * @param event: The event to process.
 * @param client: The index of the player that was damaged (0=get userid from event)
 * @return The return value will be the amount of damage
 *
 * native GetDamage(Handle:event, client=0);
 */
public Native_GetDamage(Handle:plugin,numParams)
{
    new Handle:event = GetNativeCell(1);
    new client = (numParams >= 2) ? GetNativeCell(2) : 0;
    return GetDamage(event, client);
}

/**
 * Displays damage inflicted upon a player
 * @param client:       The index of the player that inflicted the damage (attacker)
 * @param victim:       The index of the player that got damaged
 * @param hp:           The amount of damage dealt
 * @param weapon:       The short name of the weapon used to inflice the damage (for the log)
 * @param description:  The description of the weapon (displayed to the players)
 * @param no_translate: Set to true to supress translating the weapon_name to get the weapon_desc
 * @noreturn
 *
 * native DisplayDamage(client, victim, hp=0, const String:weapon[],
 *                      const String:description[],
 *                      bool:no_translate=false);
 */
public Native_DisplayDamage(Handle:plugin,numParams)
{
    decl String:weapon[NAME_STRING_LENGTH],String:desc[LONG_STRING_LENGTH];
    new client = GetNativeCell(1);
    new victim = GetNativeCell(2);
    new hp = GetNativeCell(3);
    GetNativeString(4,weapon,sizeof(weapon));
    GetNativeString(5,desc,sizeof(desc));
    new bool:no_translate = bool:GetNativeCell(6);
    DisplayDamage(client, victim, hp, weapon,
                  desc, no_translate);
}

/**
 * Displays when a player is killed
 * @param client: The index of the attacking player (attacker)
 * @param victim: The index of the player that got killed
 * @param weapon: The short name of the weapon used to inflice the damage (for the log)
 * @param description: The description of the weapon (displayed to the players)
 * @param xp: The amount of XP gained
 * @param no_translate: Set to true to supress translating the weapon_name to get the weapon_desc
 * @noreturn
 *
 * native DisplayKill(client, victim, const String:weapon[], const String:description[],
 *                    xp=0, bool:no_translate=false);
 */
public Native_DisplayKill(Handle:plugin,numParams)
{
    decl String:weapon[NAME_STRING_LENGTH],String:desc[LONG_STRING_LENGTH];
    new client = GetNativeCell(1);
    new victim = GetNativeCell(2);
    new xp = GetNativeCell(3);
    GetNativeString(4,weapon,sizeof(weapon));
    GetNativeString(5,desc,sizeof(desc));
    new bool:no_translate = bool:GetNativeCell(6);

    g_Killed[victim] = true;
    g_KilledBy[victim] = client;

    DisplayKill(client, victim, xp, weapon,
                desc, no_translate);

    #if defined _TRACE
        CreateTimer(0.1, TrackKill, GetClientUserId(victim),
                    TIMER_REPEAT|TIMER_FLAG_NO_MAPCHANGE);
    #endif
}

/**
 * Displays a message to a player
 * @param client: The index of the player to display the message to
 * @param flags: The message flags to categorize the message
 * @param fmt: The format string for the message
 * @param ...: Format arguments (if any)
 * @noreturn
 *
 * native DisplayMessage(client, flags, const String:fmt[], any:...);
 */
public Native_DisplayMessage(Handle:plugin,numParams)
{
    decl String:buffer[1024], written;
    new client = GetNativeCell(1);
    new Settings:flags = Settings:GetNativeCell(2);

    SetGlobalTransTarget(client);
    FormatNativeString(0, /* Use an output buffer */
                       3, /* Format param */
                       4, /* Format argument #1 */
                       sizeof(buffer), /* Size of output buffer */
                       written, /* Store # of written bytes */
                       buffer); /* Use our buffer */

    if (IsValidClient(client) && !IsFakeClient(client))
    {
        PrintToConsole(client, buffer);
        if (_:(g_ClientSettingsBits[client] & flags) == 0)
            CPrintToChat(client, buffer);
    }

    LogToGame(buffer);
    PrintToServer(buffer);
}

/**
 * Displays a Hud Message to a player
 * @param client: The index of the player to display the message to
 * @param fmt: The format string for the message
 * @param ...: Format arguments (if any)
 * @noreturn
 *
 * native HudMessage(client, const String:fmt[], any:...);
 */
public Native_HudMessage(Handle:plugin,numParams)
{
    new client = GetNativeCell(1);
    if (IsValidClient(client))
    {
        new written = 0;
        SetGlobalTransTarget(client);
        FormatNativeString(0, /* Use an output buffer */
                           2, /* Format param */
                           3, /* Format argument #1 */
                           sizeof(m_HudMessage[]), /* Size of output buffer */
                           written, /* Store # of written bytes */
                           m_HudMessage[client]); /* Use our buffer */

        if (written > 0 && m_HudMessage[client][0] != '\0')
        {
            SetHudTextParams(0.68, 0.94, 1.0, 255, 128, 128, 255);
            ShowSyncHudText(client, m_HudSynch[3], m_HudMessage[client]);
        }
        else
            ClearSyncHud(client, m_HudSynch[3]);
    }
}

/**
 * Clears a client's Hud.
 * @param client: The index of the player to clear
 * @param contains: Only clear the HUD Message if it containsStimpackHud this string (if provided).
 * @param caseSensitive	If true (default), search is case sensitive.
 *						If false, search is case insensitive.
 * @noreturn
 *
 * native ClearHud(client, const String:contains[]="", bool:caseSensitive=true);
 */
public Native_ClearHud(Handle:plugin,numParams)
{
    new client = GetNativeCell(1);
    if (IsValidClient(client))
    {
        new written = 0;
        decl String:contains[sizeof(m_HudMessage[])];
        SetGlobalTransTarget(client);
        FormatNativeString(0, /* Use an output buffer */
                           2, /* Format param */
                           3, /* Format argument #1 */
                           sizeof(contains), /* Size of output buffer */
                           written, /* Store # of written bytes */
                           contains); /* Use our buffer */
        GetNativeString(2,contains,sizeof(contains));

        // Strip any spaces or asterisks (*).
        ReplaceString(contains, sizeof(contains), "*", "");
        ReplaceString(contains, sizeof(contains), " ", "");

        if (written == 0 || contains[0] == '\0' ||
            StrContains(m_HudMessage[client], contains))
        {
            m_HudMessage[client][0] = '\0';
            ClearSyncHud(client, m_HudSynch[3]);
        }
    }
}

/**
 * Retrieves the players HUD Message
 * @param client: The index of the player
 * @param name: String to place the message into.
 * @param maxlength: The size of the message buffer.
 * @noreturn
 *
 * native GetHudMessage(client, String:message[], maxlength);
 */
public Native_GetHudMessage(Handle:plugin,numParams)
{
    new client = GetNativeCell(1);
    if (client > 0)
    {
        SetNativeString(2, m_HudMessage[client], GetNativeCell(3));
    }
}

/**
 * Shows the client's XP, level and race in chat
 * @param client: Client index
 * @param forceDisplay: Forces display regardless of client's display settings
 * @noreturn
 *
 * native ShowXP(client, forceDisplay=false);
 */
public Native_ShowXP(Handle:plugin,numParams)
{
    new client = GetNativeCell(1);
    if (client > 0)
    {
		ShowXP(client, bool:GetNativeCell(2));
	}
}

/**
 * Shows the client's crystals in chat
 * @param client: Client index
 * @param forceDisplay: Forces display regardless of client's display settings
 * @noreturn
 *
 * native ShowCrystals(client, forceDisplay=false);
 */
public Native_ShowCrystals(Handle:plugin,numParams)
{
    new client = GetNativeCell(1);
    if (client > 0)
    {
		ShowCrystals(client, bool:GetNativeCell(2));
	}
}

/**
 * Shows the client's vespene in chat
 * @param client: Client index
 * @param forceDisplay: Forces display regardless of client's display settings
 * @noreturn
 *
 * native ShowVespene(client, forceDisplay=false);
 */
public Native_ShowVespene(Handle:plugin,numParams)
{
    new client = GetNativeCell(1);
    if (client > 0)
    {
		ShowVespene(client, bool:GetNativeCell(2));
	}
}

/**
 * Shows the client's energy (including accumulated) in chat
 * @param client: Client index
 * @param forceDisplay: Forces display regardless of client's display settings
 * @noreturn
 *
 * native ShowEnergy(client, forceDisplay=false);
 */
public Native_ShowEnergy(Handle:plugin,numParams)
{
    new client = GetNativeCell(1);
    if (client > 0)
    {
		ShowEnergy(client, bool:GetNativeCell(2));
	}
}

/**
 * Returns how much armor the client has
 *
 * @param client 	Client
 * @return			The amount of armor the client has
 *
 * native GetArmor(client);
 */
public Native_GetArmor(Handle:plugin,numParams)
{
    return GetArmor(GetNativeCell(1));
}

/**
 * Returns if the client has shields
 *
 * @param client 	Client
 * @return			true if the client has shields (regenerating armor)
 *
 * native bool:HasShields(client);
 */
public Native_HasShields(Handle:plugin,numParams)
{
    return HasShields(GetNativeCell(1));
}

/**
 * Sets how much armor the client has
 *
 * @param client 	    Client
 * @param amount        The amount of armor the client has
 * @param percent       Array that holds a range of percent damage that the armor absorbs.
 * @param name:         What to call the armor in messages. (max 64) [defaults to either Armor or Shields]
 * @param hasShields    true if the armor is a shield (regenerates)
 * @return			    none
 *
 * native SetArmor(client, amount, const Float:percent[2],
 *                 const String:name[]="", bool:hasShields=false);
 */
public Native_SetArmor(Handle:plugin,numParams)
{
    new client = GetNativeCell(1);
    new amount = GetNativeCell(2);
    new bool:hasShields = GetNativeCell(5);

    GetNativeArray(3, m_ArmorPercent[client], sizeof(m_ArmorPercent[]));

    new bytes;
    GetNativeString(4, m_ArmorName[client], sizeof(m_ArmorName[]), bytes);

    SetTraceCategory("Armor");
    TraceInto("Native", "Native_SetArmor", "client=%d:%L, amount=%d, percent=%1.2f,%1.2f, name='%s', bytes=%d, sizeof=%d,%d", \
              client, ValidClientIndex(client), amount, m_ArmorPercent[client][0], m_ArmorPercent[client][1], \
              m_ArmorName[client], bytes, sizeof(m_ArmorPercent), sizeof(m_ArmorPercent[]));

    SetArmor(client, amount);
    SetShields(client, hasShields);

    if (amount > 0 && bytes == 0)
    {
        if (hasShields)
            strcopy(m_ArmorName[client], sizeof(m_ArmorName[]), "Shields");
        else
            strcopy(m_ArmorName[client], sizeof(m_ArmorName[]), "Armor");
    }

    TraceReturn("m_ArmorName='%s'", m_ArmorName[client]);
}

/**
 * Sets only the amount of armor the client has
 *
 * @param client 	    Client
 * @param amount        The amount of armor the client has
 * @return			    none
 *
 * native SetArmorAmount(client, amount);
 */
public Native_SetArmorAmount(Handle:plugin,numParams)
{
    new client = GetNativeCell(1);
    new amount = GetNativeCell(2);

    SetTraceCategory("Armor");
    TraceInto("Native", "Native_SetArmorAmount", "client=%d:%L, old amount=%d, old m_ArmorPercent=%1.2f,%1.2f", \
              client, ValidClientIndex(client), GetArmor(client), m_ArmorPercent[client][0], m_ArmorPercent[client][1]);

    SetArmor(client, amount);

    if (amount > 0 &&
        m_ArmorPercent[client][0] <= 0.0 &&
        m_ArmorPercent[client][1] <= 0.0)
    {
        m_ArmorPercent[client][0] = 1.0;
        m_ArmorPercent[client][1] = 1.0;
    }

    TraceReturn("amount=%d, percent=%1.2f,%1.2f ", amount, m_ArmorPercent[client][0], m_ArmorPercent[client][1]);
}

/**
 * Sets what % damage the client's armor absorbs
 *
 * @param client 	    Client
 * @param percent       Array that holds a range of percent damage that the armor absorbs.
 * @return			    none
 *
 * native SetArmorPercent(client, const Float:percent[2]);
 */
public Native_SetArmorPercent(Handle:plugin,numParams)
{
    new client = GetNativeCell(1);

    SetTraceCategory("Armor");
    TraceInto("Native", "Native_SetArmorPercent", "client=%d:%L, old m_ArmorPercent=%1.2f,%1.2f", \
              client, ValidClientIndex(client), m_ArmorPercent[client][0], m_ArmorPercent[client][1]);

    GetNativeArray(2, m_ArmorPercent[client], sizeof(m_ArmorPercent[]));

    TraceReturn("percent=%1.2f,%1.2f, sizeof=%d,%d", m_ArmorPercent[client][0], m_ArmorPercent[client][1], \
                sizeof(m_ArmorPercent), sizeof(m_ArmorPercent[]));
}

/**
 * Gets what % damage the client's armor absorbs
 *
 * @param client 	    Client
 * @param percent       Array to receive the range of percent damage that the armor absorbs.
 * @return			    none
 *
 * native GetArmorPercent(client, Float:percent[2]);
 */
public Native_GetArmorPercent(Handle:plugin,numParams)
{
    new client = GetNativeCell(1);

    SetTraceCategory("Armor");
    TraceInto("Native", "Native_GetArmorPercent", "client=%d:%L", \
              client, ValidClientIndex(client));

    SetNativeArray(2, m_ArmorPercent[client], sizeof(m_ArmorPercent[]));

    TraceReturn("percent=%1.2f,%1.2f", m_ArmorPercent[client][0], m_ArmorPercent[client][1]);
}

/**
 * Sets the name of the given client's armor
 * @param client 	    Client
 * @param name:         String to set the armor name to.
 * @noreturn
 *
 * native SetArmorName(client, const String:name[]);
 */         
public Native_SetArmorName(Handle:plugin,numParams)
{
    new client = GetNativeCell(1);

    SetTraceCategory("Armor");
    TraceInto("Native", "Native_SetArmorName", "client=%d:%L, old ArmorName='%s'", \
              client, ValidClientIndex(client), m_ArmorName[client]);

    GetNativeString(2,m_ArmorName[client],sizeof(m_ArmorName[]));

    TraceReturn("set m_ArmorName='%s'", m_ArmorName[client]);
}

/**
 * Retrieves the name of the given client's armor
 * @param client 	    Client
 * @param name:         String to place the name into.
 * @param maxlength:    The size of the name buffer.
 * @noreturn
 *
 * native GetArmorName(client, String:name[], maxlength);
 */         
public Native_GetArmorName(Handle:plugin,numParams)
{
    new client = GetNativeCell(1);
    new maxlength = GetNativeCell(3);

    SetTraceCategory("Armor");
    TraceInto("Native", "Native_GetArmorName", "client=%d:%L", \
              client, ValidClientIndex(client));

    SetNativeString(2, m_ArmorName[client], maxlength);

    TraceReturn("m_ArmorName='%s'", m_ArmorName[client]);
}

/**
 * Increments how much armor the client has
 *
 * @param client 	Client
 * @param amount    The amount to add to the armor the client has
 * @return			none
 *
 * native IncrementArmor(client, amount);
 */
public Native_IncrementArmor(Handle:plugin,numParams)
{
    m_Armor[GetNativeCell(1)] += GetNativeCell(2);
}

/**
 * Decrements how much armor the client has
 *
 * @param client 	Client
 * @param amount    The amount to subtract from the armor the client has
 * @return			The amount that was actually subtracted (can be less than amount)
 *
 * native DecrementArmor(client, amount);
 */
public Native_DecrementArmor(Handle:plugin,numParams)
{
    new client = GetNativeCell(1);
    new amount = GetNativeCell(2);

    new armor = m_Armor[client];
    if (amount > armor)
        amount = armor;

    if (amount > 0)
        m_Armor[client] -= amount;

    return amount;
}

/**
 * Gets the state of all settings for a client.
 * @param client: The index of the player to get the settings for
 * @return The return value will be all of the client's settings bits.
 *
 * native Settings:GetSettingsBits(client);
 */
public Native_GetSettingsBits(Handle:plugin,numParams)
{
    return _:GetSettingsBits(GetNativeCell(1));
}

/**
 * Gets the state of all settings for a client.
 * @param client: The index of the player to get the settings for
 * @return The return value will be all of the client's settings bits.
 *
 * native bool:GetSetting(client,Settings:setting);
 */
public Native_GetSetting(Handle:plugin,numParams)
{
    return _:GetSetting(GetNativeCell(1), Settings:GetNativeCell(2));
}

/**
 * Returns if the client is changing class
 *
 * @param client 	Client
 * @return			true if the client is in the process of changing class
 * 
 * native bool:IsChangingClass(client);
 */
public Native_IsChangingClass(Handle:plugin,numParams)
{
    return IsChangingClass(GetNativeCell(1));
}

/**
 * Gets the current race for the player.
 * @param client: The index of the player.
 * @return The return value will be the race of the player or -1 if it failed.
 *
 * native GetRace(client);
 */
public Native_GetRace(Handle:plugin,numParams)
{
    new client = GetNativeCell(1);
    if (client > 0)
        return GetRace(client);
    else
        return -1;
}

/**
 * Changes the current race of the player.
 * @param client: The index of the player.
 * @param race: The race ID returned from CreateRace.
 * @param force: Set true to force the race to change now or false to change on death.
 * @param check: Set true to check race requirements before changing.
 * @noreturn
 *
 * native ChangeRace(client,race,bool:force=false,bool:check=true);
 */
public Native_ChangeRace(Handle:plugin,numParams)
{
    new client = GetNativeCell(1);
    if (client > 0)
    {
        new race=GetNativeCell(2);
        if (race > 0)
        {
            new bool:force=bool:GetNativeCell(3);
            new bool:check=bool:GetNativeCell(4);
            ChangeRace(client, race, GetRaceHandle(race), force, check);
        }
    }
}

/**
 * Gets the current level for the provided race.
 * @param client: The index of the player.
 * @param race: The race ID returned from CreateRace or -1 for current race
 * @return The return value will be the level or -1 if it failed.
 *
 * native GetLevel(client,race=-1);
 */
public Native_GetLevel(Handle:plugin,numParams)
{
    new client = GetNativeCell(1);
    if (IsValidClient(client))
    {
        new race = GetNativeCell(2);
        if (race <= 0)
            race = GetRace(client);

        if (race != -1)
            return GetLevel(client,race);
    }
    return -1;
}

/**
 * Sets the current level for the provided race.
 * @param client:   The index of the player.
 * @param race:     The race ID returned from CreateRace or -1 for current race
 * @param newlevel: The level you want to set to 
 * @param update:   If true, updates the overall and tech levels also.
 * @noreturn
 *
 * native SetLevel(client,race,newlevel,bool:update=true);
 */
public Native_SetLevel(Handle:plugin,numParams)
{
    new client = GetNativeCell(1);
    new race = GetNativeCell(2);
    if (race <= 0)
        race = GetRace(client);

    if (client > 0 && race > 0)
        SetLevel(client,race,GetNativeCell(3), bool:GetNativeCell(4));
}

/**
 * Resets the current level for the provided race, also adjusts Upgrade levels as required.
 * @param client: The index of the player.
 * @param race: The race ID returned from CreateRace or -1 for current race
 * @param newlevel: The level you want to set to 
 * @noreturn
 *
 * native ResetLevel(client,race,newlevel);
 */
public Native_ResetLevel(Handle:plugin,numParams)
{
    new client = GetNativeCell(1);
    new race = GetNativeCell(2);
    if (race <= 0)
        race = GetRace(client);

    if (client > 0 && race > 0)
        ResetLevel(client,race,GetNativeCell(3));
}

/**
 * Gets the current level for the upgrade for the provided race.
 * @param client: The index of the player.
 * @param race: The race ID returned from CreateRace or -1 for current race
 * @param upgrade: The upgrade you want to lookup with
 * @return The return value will be the upgrade level or -1 if it failed.
 *
 * native GetUpgradeLevel(client,race,upgrade,const String:from[]);
 */
public Native_GetUpgradeLevel(Handle:plugin,numParams)
{
    new level = -1;
    new client  = GetNativeCell(1);
    new upgrade = GetNativeCell(3);

    new race = GetNativeCell(2);
    if (race <= 0)
        race = GetRace(client);

    TraceInto("Natives", "GetUpgradeLevel", "client=%d, race=%d, upgrade=%d; level=%d", client, race, upgrade, level);

    if (client > 0 && race > 0 && upgrade >= 0)
        level = GetUpgradeLevel(client,race,upgrade);

    TraceReturn("level=%d", level);
    return level;
}

/**
 * Sets the current level for the upgrade for the provided race.
 * @param client: The index of the player.
 * @param race: The race ID returned from CreateRace or -1 for current race
 * @param upgrade: The upgrade you want to lookup with
 * @param newlevel: The level you want to set to 
 * @noreturn
 *
 * native SetUpgradeLevel(client,race,upgrade,newlevel);
 */
public Native_SetUpgradeLevel(Handle:plugin,numParams)
{
    new client = GetNativeCell(1);
    new race=GetNativeCell(2);
    if (race <= 0)
        race = GetRace(client);

    new upgrade=GetNativeCell(3);
    if (client > 0 && race > 0 && upgrade >= 0)
        SetUpgradeLevel(client,race,upgrade,GetNativeCell(4));
}

/**
 * Resets the levels for the upgrade for the curerent race.
 * @param client: The index of the player.
 * @param race:     The race ID returned from CreateRace or -1 for current race
 * @noreturn
 *
 * native ResetUpgradeLevels(client,race);
 */
public Native_ResetUpgradeLevels(Handle:plugin,numParams)
{
    new client = GetNativeCell(1);
    if (client > 0)
        ResetUpgradesNow(client, GetNativeCell(2));
}

/**
 * Gets the XP for the provided race.
 * @param client: The index of the player.
 * @param race: The race ID returned from CreateRace or -1 for current race
 * @return The return value will be the xp or -1 if it failed.
 *
 * native GetXP(client,race=-1);
 */
public Native_GetXP(Handle:plugin,numParams)
{
    new client = GetNativeCell(1);

    new race = (numParams >= 2) ? GetNativeCell(2) : -1;
    if (race > 0)
        race = GetRace(client);

    if (client > 0 && race > 0)
        return GetXP(client,race);
    else
        return -1;
}

/**
 * Sets the XP for the provided race.
 * @param client: The index of the player.
 * @param race: The race ID returned from CreateRace or -1 for current race
 * @param newxp: The XP you want to set to 
 * @noreturn
 *
 * native SetXP(client,race,newxp);
 */
public Native_SetXP(Handle:plugin,numParams)
{
    new client = GetNativeCell(1);
    new newxp  = GetNativeCell(3);

    new race=GetNativeCell(2);
    if (race == -1)
        race = GetRace(client);

    if (client > 0 && race > 0 && newxp >= 0)
    {
        SetXP(client,race,newxp);
        DoLevelCheck(client);
    }
}

/**
 * Resets the XP for the provided race, also adjusts Levels and Upgrades as required.
 * @param client: The index of the player.
 * @param race: The race ID returned from CreateRace or -1 for current race
 * @param newxp: The XP you want to set to 
 * @noreturn
 *
 * native ResetXP(client,race,newxp);
 */
public Native_ResetXP(Handle:plugin,numParams)
{
    new client = GetNativeCell(1);
    new newxp  = GetNativeCell(3);

    new race=GetNativeCell(2);
    if (race == -1)
        race = GetRace(client);

    if (client > 0 && race > 0 && newxp >= 0)
        ResetXP(client,race,newxp);
}

/**
 * Gets the kill XP for a player, useful for simulating an explosion or anything else.
 * @param client: Client index
 * @param race: The race ID returned from CreateRace or -1 for current race
 * @return Will be kill XP or -1 if it failed
 *
 * native GetKillXP(client,race);
 */
public Native_GetKillXP(Handle:plugin,numParams)
{
    new client = GetNativeCell(1);
    new race = GetNativeCell(2);
    if (race == -1)
        race = GetRace(client);

    return KillXP(GetLevel(client,race));
}

/**
 * Gets the kill XP for a player, useful for simulating an explosion or anything else.
 * @param level:   The level to lookup
 * @return Will be kill XP or -1 if it failed
 *
 * native GetKillLevelXP(level);
 */
public Native_GetKillLevelXP(Handle:plugin,numParams)
{
    new level = GetNativeCell(1);
    return KillXP((level > 0) ? level : 0);
}

/**
 * Gets the XP required for the player to gain the next level.
 * @param client: Client index
 * @param race: The race ID returned from CreateRace or -1 for current race
 * @return Will be required XP
 *
 * native GetLevelXP(client,race);
 */
public Native_GetLevelXP(Handle:plugin,numParams)
{
    new client = GetNativeCell(1);
    new race=GetNativeCell(2);
    if (race == -1)
        race = GetRace(client);

    return ReqLevelXP(GetLevel(client,race));
}

/**
 * Gets the XP required to gain a level.
 * @param level:   The level to lookup
 * @return Will be required XP
 *
 * native GetReqLevelXP(client,race);
 */
public Native_GetReqLevelXP(Handle:plugin,numParams)
{
    new level = GetNativeCell(1);
    return ReqLevelXP((level > 0) ? level : 0);
}

/**
 * Gets the amount of Energy a player has.
 * @param client: The index of the player.
 * @return The return value will be the amount of energy or -1 if it failed.
 *
 * native GetEnergy(client);
 */
public Native_GetEnergy(Handle:plugin,numParams)
{
    new client = GetNativeCell(1);
    if (client > 0)
        return GetEnergy(client);
    else
        return -1;
}

/**
 * Sets the amount of Energy a player has.
 * @param client: The index of the player.
 * @param neweneergy: The amount you want to set
 * @noreturn
 *
 * native SetEnergy(client,newenergy);
 */
public Native_SetEnergy(Handle:plugin,numParams)
{
    new client = GetNativeCell(1);
    if (client > 0)
        SetEnergy(client,GetNativeCell(2));
}

/**
 * Gets the Energy accumulation Rate for a player.
 * @param client: The index of the player.
 * @return The return value will be the amount of energy or -1 if it failed.
 *
 * native GetEnergyRate(client);
 */
public Native_GetEnergyRate(Handle:plugin,numParams)
{
    new client = GetNativeCell(1);
    if (client > 0)
        return GetEnergyRate(client);
    else
        return -1;
}

/**
 * Sets the Energy accumulation Rate for a player.
 * @param client: The index of the player.
 * @param newrate: The amount you want to set
 * @noreturn
 *
 * native SetEnergyRate(client,newrate);
 */
public Native_SetEnergyRate(Handle:plugin,numParams)
{
    new client = GetNativeCell(1);
    if (client > 0)
        SetEnergyRate(client,GetNativeCell(2));
}

/**
 * Gets the Energy Limit for a player.
 * @param client: The index of the player.
 * @return The return value will be the amount of energy or -1 if it failed.
 *
 * native GetEnergyLimit(client);
 */
public Native_GetEnergyLimit(Handle:plugin,numParams)
{
    new client = GetNativeCell(1);
    if (client > 0)
        return GetEnergyLimit(client);
    else
        return -1;
}

/**
 * Sets the Energy Limit for a player.
 * @param client: The index of the player.
 * @param newlimit: The amount you want to set
 * @noreturn
 *
 * native SetEnergyLimit(client,newlimit);
 */
public Native_SetEnergyLimit(Handle:plugin,numParams)
{
    new client = GetNativeCell(1);
    if (client > 0)
        SetEnergyLimit(client,GetNativeCell(2));
}

/**
 * Gets the Initial Energy for a player.
 * @param client: The index of the player.
 * @return The return value will be the amount of energy or -1 if it failed.
 *
 * native GetInitialEnergy(client);
 */
public Native_GetInitialEnergy(Handle:plugin,numParams)
{
    new client = GetNativeCell(1);
    if (client > 0)
        return GetInitialEnergy(client);
    else
        return -1;
}

/**
 * Sets the Initial Energy for a player.
 * @param client: The index of the player.
 * @param newamount: The amount you want to set
 * @noreturn
 *
 * native SetInitialEnergy(client,newamount);
 */
public Native_SetInitialEnergy(Handle:plugin,numParams)
{
    new client = GetNativeCell(1);
    if (client > 0)
        SetInitialEnergy(client,GetNativeCell(2));
}

/**
 * Gets the amount of Accumulated Energy a player has.
 * @param client: The index of the player.
 * @param race: The race ID returned from CreateRace or -1 for current race
 * @return The return value will be the amount of energy or -1 if it failed.
 *
 * native GetAccumulatedEnergy(client,race);
 */
public Native_GetAccumulatedEnergy(Handle:plugin,numParams)
{
    new client = GetNativeCell(1);
    new race=GetNativeCell(2);
    if (race <= 0)
        race = GetRace(client);

    if (client > 0 && race > 0)
        return GetAccumulatedEnergy(client,race);
    else
        return -1;
}

/**
 * Sets the amount of Accumulated Energy a player has.
 * @param client: The index of the player.
 * @param race: The race ID returned from CreateRace or -1 for current race
 * @param neweneergy: The amount you want to set
 * @noreturn
 *
 * native SetAccumulatedEnergy(client,race,newenergy);
 */
public Native_SetAccumulatedEnergy(Handle:plugin,numParams)
{
    new client = GetNativeCell(1);
    new race=GetNativeCell(2);
    if (race > 0)
        race = GetRace(client);

    if (client > 0 && race > 0)
        SetAccumulatedEnergy(client,race,GetNativeCell(3));
}

/**
 * Gets the maximum amount of Crystals a player can have.
 * @return The return value will be the maximum amount of Crystals a player can have.
 *
 * native GetMaxCrystals();
 */
public Native_GetMaxCrystals(Handle:plugin,numParams)
{
    return g_iMaxCrystals;
}

/**
 * Gets the amount of Crystals a player has.
 * @param client: The index of the player.
 * @return The return value will be the amount of crystals or -1 if it failed.
 *
 * native GetCrystals(client);
 */
public Native_GetCrystals(Handle:plugin,numParams)
{
    new client = GetNativeCell(1);
    if (client > 0)
        return GetCrystals(client);
    else
        return -1;
}

/**
 * Sets the amount of Crystals a player has.
 * @param client: The index of the player.
 * @param newcrystals: The amount you want to set
 * @param check: Set true to check limit before changing.
 * @return The return value will be the amount of crystals set or -1 if it failed.
 *
 * native SetCrystals(client,newcrystals,bool:check=true);
 */
public Native_SetCrystals(Handle:plugin,numParams)
{
    new client = GetNativeCell(1);
    if (client > 0)
    {
        new amount = GetNativeCell(2);
        if (amount > g_iMaxCrystals &&
            g_iMaxCrystals > 0 &&
            GetNativeCell(3))
        {
            amount = g_iMaxCrystals;
        }

        SetCrystals(client,amount);
        return amount;
    }
    else
        return -1;
}

/**
 * Gets the maximum amount of Vespene (gas) a player can have.
 * @return The return value will be the maximum amount of Vespene a player can have.
 *
 * native GetMaxVespene();
 */
public Native_GetMaxVespene(Handle:plugin,numParams)
{
    return g_iMaxVespene;
}

/**
 * Gets the amount of Vespene (gas) a player has.
 * @param client: The index of the player.
 * @return The return value will be the amount of vespene or -1 if it failed.
 *
 * native GetVespene(client);
 */
public Native_GetVespene(Handle:plugin,numParams)
{
    new client = GetNativeCell(1);
    if (client > 0)
        return GetVespene(client);
    else
        return -1;
}

/**
 * Sets the amount of Vespene (gas) a player has.
 * @param client: The index of the player.
 * @param newvespene: The amount you want to set
 * @noreturn
 *
 * native SetVespene(client,newvespene);
 */
public Native_SetVespene(Handle:plugin,numParams)
{
    new client = GetNativeCell(1);
    if (client > 0)
    {
        new amount = GetNativeCell(2);
        if (amount > g_iMaxVespene &&
            g_iMaxVespene > 0 &&
            GetNativeCell(3))
        {
            amount = g_iMaxVespene;
        }
        SetVespene(client,amount);
        return amount;
    }
    else
        return -1;
}

/**
 * Gets the player's current overall level. (Total level of all races)
 * @param client: The index of the player.
 * @return The return value will be the overall level or -1 if it failed.
 *
 * native GetOverallLevel(client);
 */
public Native_GetOverallLevel(Handle:plugin,numParams)
{
    new client = GetNativeCell(1);
    if (client > 0)
        return GetOverallLevel(client);
    else        
        return -1;
}

/**
 * Sets the player's overall level.
 * @param client: The index of the player.
 * @param newlevel: The level you want to set to 
 * @noreturn
 *
 * native SetOverallLevel(client,newlevel);
 */
public Native_SetOverallLevel(Handle:plugin,numParams)
{
    new client = GetNativeCell(1);
    if (client > 0)
        SetOverallLevel(client,GetNativeCell(2));
}

/**
 * Gets if the player owns a specific item.
 * @param client: The index of the player.
 * @param item: The item ID returned from CreateShopItem
 * @return The return value will be amount of the specified item that they own (usually 1),
 *         0 if they don't own any or -1 if it failed.
 *
 * native GetOwnsItem(client,item);
 */
public Native_GetOwnsItem(Handle:plugin,numParams)
{
    new client = GetNativeCell(1);
    new item=GetNativeCell(2);
    if (client > 0 && item >= 0)
        return GetOwnsItem(client,item);
    else
        return -1;
}

/**
 * Sets if the player owns a specific item.
 * @param client: The index of the player.
 * @param item: The item ID returned from CreateShopItem
 * @param owns: 1 if you want them to own it, 0 if you want them not to own it 
 * @noreturn
 *
 * native SetOwnsItem(client,item,owns=1);
 */
public Native_SetOwnsItem(Handle:plugin,numParams)
{
    new client = GetNativeCell(1);
    new item=GetNativeCell(2);
    new owns=(numParams >= 3) ? GetNativeCell(3) : 1;
    if (client > 0 && item >= 0)
    {
        SetOwnsItem(client,item,owns);
        if (owns == 0)
        {
            new result;
            Call_StartForward(g_OnItemLostHandle);
            Call_PushCell(client);
            Call_PushCell(item);
            Call_Finish(result);
        }
    }
}

/**
 * Gets the state of all attributes for a player.
 * @param client: The index of the player.
 * @return The return value will be all of the player's attribute bits.
 *
 * native GetAttributeBits(client);
 */
public Native_GetAttributeBits(Handle:plugin,numParams)
{
    new client = GetNativeCell(1);
    if (client > 0)
        return _:GetAttributeBits(client);
    else
        return 0;
}

/**
 * Sets the state of all attributes for a player.
 * @param client: The index of the player.
 * @param attributes: The value to set for the player's attributes.
 * @noreturn
 *
 * native SetAttributeBits(client, attributes);
 */
public Native_SetAttributeBits(Handle:plugin,numParams)
{
    new client = GetNativeCell(1);

    SetTraceCategory("Attribute");
    TraceInto("Native", "Native_SetAttributeBits", "Set %d:%N's attribute bits to 0x%08x", \
              client, ValidClientIndex(client), GetNativeCell(2));

    if (client > 0)
        SetAttributeBits(client,GetNativeCell(2));

    TraceReturn();
}

/**
 * Gets the state of a certain attribute.
 * @param client: The index of the player.
 * @param attribute: The attribute or attributes you want to check. 
 * @return The return value will be true if the attribute is set or false if it isn't.
 *
 * native bool:GetAttribute(client,Attribute:attribute);
 */
public Native_GetAttribute(Handle:plugin,numParams)
{
    new client = GetNativeCell(1);
    if (client > 0)
        return GetAttribute(client,Attribute:GetNativeCell(2));
    else        
        return false;
}

/**
 * Gives the player a specific attribute.
 * @param client: The index of the player.
 * @param attribute: The attribute or attributes you want to set.
 * @param imm_state: The state to set the attribute to.
 * @noreturn
 *
 * native SetAttribute(client,Attribute:attribute,bool:attrib_state=true);
 */
public Native_SetAttribute(Handle:plugin,numParams)
{
    new client = GetNativeCell(1);
    if (client > 0)
    {
        new bool:attrib_state = (numParams >= 3) ? (bool:GetNativeCell(3)) : true;
        SetAttribute(client,Attribute:GetNativeCell(2),attrib_state);
    }
}

/**
 * Gets the state of all immunities for a player.
 * @param client: The index of the player.
 * @return The return value will be all of the player's immunity bits.
 *
 * native GetImmunityBits(client);
 */
public Native_GetImmunityBits(Handle:plugin,numParams)
{
    new client = GetNativeCell(1);
    if (client > 0)
        return _:GetImmunityBits(client);
    else
        return 0;
}

/**
 * Sets the state of all immunities for a player.
 * @param client: The index of the player.
 * @param immunities: The value to set for the player's immunities.
 * @noreturn
 *
 * native SetImmunityBits(client, immunities);
 */
public Native_SetImmunityBits(Handle:plugin,numParams)
{
    new client = GetNativeCell(1);

    SetTraceCategory("Immunity");
    TraceInto("Native", "Native_SetImmunityBits", "Set %d:%N's immunity bits to 0x%08x", \
              client, ValidClientIndex(client), GetNativeCell(2));

    if (client > 0)
        SetImmunityBits(client,GetNativeCell(2));

    TraceReturn();
}

/**
 * Gets the state of a certain immunity.
 * @param client: The index of the player.
 * @param immunity: The immunity or immunities you want to check. 
 * @return The return value will be true if they are immune or false if they aren't.
 *
 * native bool:GetImmunity(client,Immunity:immunity);
 */
public Native_GetImmunity(Handle:plugin,numParams)
{
    new client = GetNativeCell(1);
    if (client > 0)
        return GetImmunity(client,Immunity:GetNativeCell(2));
    else        
        return false;
}

/**
 * Gives the player a specific immunity.
 * @param client: The index of the player.
 * @param immunity: The immunity or immunities you want to set.
 * @param imm_state: The state of immunity, true if they are, false if they aren't.
 * @param id: Identifier to allow more than 1 setting per plugin.
 * @noreturn
 *
 * native SetImmunity(client,Immunity:immunity,bool:imm_state=true,id=0);
 */
public Native_SetImmunity(Handle:plugin,numParams)
{
    new client = GetNativeCell(1);
    if (client > 0)
    {
        new Immunity:imm = Immunity:GetNativeCell(2);
        new bool:imm_state = (numParams >= 3) ? (bool:GetNativeCell(3)) : true;

        new Handle:propData=GetPropData(client);
        if (propData != INVALID_HANDLE)
        {
            new id=GetNativeCell(4);
            new Handle:propInfo = GetPropInfo(propData, plugin, id);
            if (propInfo == INVALID_HANDLE)
            {
                // create a new entry
                PushArrayCell(propData, CreateProperties(client, plugin, id,
                                        .immunities=(imm_state) ? imm : Immunity_None));

                SetImmunity(client,imm,imm_state);
            }
            else
            {
                // update existing entry
                new Immunity:immunities = GetPropInfoImmunities(propInfo);

                if (imm_state)
                    immunities |= imm;
                else
                    immunities &= ~imm;

                SetPropInfoImmunities(propInfo,immunities);

                SetImmunity(client,imm,imm_state);

                if (!imm_state)
                {
                    // Get the client's current sum immunities
                    immunities=GetImmunityBits(client);

                    // Go though the property array and or in immunities from all plugins
                    new size=GetArraySize(propData);
                    for (new x=0;x<size;x++)
                    {
                        new Handle:prop=GetArrayCell(propData,x);
                        immunities |= GetPropInfoImmunities(prop);
                    }
                    SetImmunityBits(client, immunities);
                }
            }
        }
    }
}

/**
 * Sets the speed for the player that is handled within the SourceCraft system,
 * good for upgrades and items that set speed so you don't end up making the player slower.
 * Pass -1.0 as speed when they no longer should use the speed, make sure to do this when
 * they no longer have the upgrade or item, or whatever.
 * @param client: The index of the player.
 * @param speed: The speed you want to pass.
 * @param apply: Set to true to apply the setting immedately.
 * @param id: Identifier to allow more than 1 setting per plugin.
 * @noreturn
 *
 * native SetSpeed(client,Float:speed=-1.0,bool:apply=false,id=0);
 */
public Native_SetSpeed(Handle:plugin,numParams)
{
    new client = GetNativeCell(1);
    if (client > 0)
    {
        new Handle:propData=GetPropData(client);
        if (propData != INVALID_HANDLE)
        {
            new Float:speed = GetNativeCell(2);
            new bool:apply  = GetNativeCell(3);
            new id          = GetNativeCell(4);

            new Float:oldSpeed = speed;
            new Handle:propInfo = GetPropInfo(propData, plugin, id);
            if (propInfo == INVALID_HANDLE)
            {
                // create a new entry
                PushArrayCell(propData, CreateProperties(client, plugin, id,
                                        .speed=speed));
            }
            else
            {
                // update existing entry
                oldSpeed = GetPropInfoSpeed(propInfo);
                SetPropInfoSpeed(propInfo,speed);
            }

            if (speed != oldSpeed || (speed >= 0.0 && speed != 1.0))
                m_SpeedAltered[client] = true;

            if (apply)
            {
                UpdatePlayerProperties(client);
            }
        }
    }
}

/**
 * Sets the gravity for the player that is handled within the SourceCraft system,
 * good for upgrades and items that set gravity so you don't end up making the player jump lower.
 * Pass -1.0 as gravity when they no longer should use the gravity, make sure to do this when
 * they no longer have the upgrade or item, or whatever.
 * @param client: The index of the player.
 * @param gravity: The gravity you want to pass.
 * @param apply: Set to true to apply the setting immedately.
 * @param id: Identifier to allow more than 1 setting per plugin.
 * @noreturn
 *
 * native SetGravity(client,Float:gravity=-1.0,bool:apply=false,id=0);
 */
public Native_SetGravity(Handle:plugin,numParams)
{
    new client = GetNativeCell(1);
    if (client > 0)
    {
        new Handle:propData=GetPropData(client);
        if (propData != INVALID_HANDLE)
        {
            new Float:gravity = GetNativeCell(2);
            new bool:apply    = GetNativeCell(3);
            new id            = GetNativeCell(4);

            new Float:oldGravity = gravity;
            new Handle:propInfo = GetPropInfo(propData, plugin, id);
            if (propInfo == INVALID_HANDLE)
            {
                // create a new entry
                PushArrayCell(propData, CreateProperties(client, plugin, id,
                                        .gravity=gravity));
            }
            else
            {
                // update existing entry
                oldGravity = GetPropInfoGravity(propInfo);
                SetPropInfoGravity(propInfo,gravity);
            }

            if (gravity != oldGravity || (gravity >= 0.0 && gravity != 1.0))
                m_GravityAltered[client] = true;

            if (apply)
            {
                UpdatePlayerProperties(client);
            }
        }
    }
}

/**
 * Sets the visibility for the player that is handled within the SourceCraft system,
 * good for upgrades and items that set (in)visibility so you don't end up making the player
 * more visibile. Pass 255 as visibility when they no longer should use the (in)visibility,
 * make sure to do this when they no longer have the upgrade or item, or whatever.
 * @param client:       The index of the player.
 * @param player:       The player ID to use. Can be retrieved with GetPlayer.
 * @param type:         The type of visibility to use.
 * @param visibility:   The visibility you want to pass.
 * @param factor1:      The 1st factor (either melee factor or activation delay)
 * @param factor2:      The 2nd factopr (either movement factor or duration)
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
 *
 * native SetVisibility(client,VisibilityType:type,visibility,Float:factor1,
 *                      Float:factor2,RenderFx:fx,RenderMode:mode,r,g,b,
 *                      bool:colorWeapons=false,bool:apply=false,id=0);
 */
public Native_SetVisibility(Handle:plugin,numParams)
{
    new client = GetNativeCell(1);
    if (client > 0)
    {
        SetTraceCategory("Visibility");
        TraceInto("Native", "Native_SetVisibility", "client=%d:%L", \
                  client, ValidClientIndex(client));

        new Handle:propData=GetPropData(client);
        if (propData != INVALID_HANDLE)
        {
            new VisibilityType:type         = GetNativeCell(2);
            new visibility                  = GetNativeCell(3);
            new Float:factor1               = GetNativeCell(4);
            new Float:factor2               = GetNativeCell(5);
            new RenderMode:mode             = GetNativeCell(6);
            new RenderFx:fx                 = GetNativeCell(7); 
            new r                           = GetNativeCell(8);
            new g                           = GetNativeCell(9);
            new b                           = GetNativeCell(10);
            new bool:colorWeapons           = GetNativeCell(11);
            new VisibilityPriority:priority = GetNativeCell(12);
            new Float:timestamp             = GetNativeCell(13);
            new bool:apply                  = GetNativeCell(14);
            new id                          = GetNativeCell(15);

            new oldVisibility  = visibility;
            new Handle:propInfo = GetPropInfo(propData, plugin, id);
            if (propInfo == INVALID_HANDLE)
            {
                Trace("Create new properties for %d:%L: visibility=%d,type=%d,f1=%f,f2=%f,mode=%d,fx=%d,r=%d,g=%d,b=%d", \
                      client, ValidClientIndex(client), visibility, type, factor1, factor2, mode, fx, r, g, b);

                // create a new entry
                PushArrayCell(propData, CreateProperties(client, plugin, id,
                                        .visibility=visibility, .type=type,
                                        .factor1=factor1, .factor2=factor2, .mode=mode,
                                        .fx=fx, .r=r, .g=g, .b=b, .priority=priority,
                                        .timestamp=timestamp, .colorWeapons=colorWeapons));
            }
            else
            {
                Trace("Update properties for %d:%L: visibility=%d, type=%d,f1=%f,f2=%f,mode=%d,fx=%d,r=%d,g=%d,b=%d", \
                      client, ValidClientIndex(client), visibility, type, factor1, factor2, mode, fx, r, g, b);

                // update existing entry
                oldVisibility = GetPropInfoVisibility(propInfo);
                             
                if (type != AdditiveVisibility || visibility >= 0)
                    SetPropInfoVisibility(propInfo,visibility);

                if (type != AdditiveVisibility)
                    SetPropInfoVisibilityType(propInfo,type);
                if (type != AdditiveVisibility || factor1 >= 0.0)
                    SetPropInfoVisibilityFactor1(propInfo,factor1);
                if (type != AdditiveVisibility || factor2 >= 0.0)
                    SetPropInfoVisibilityFactor2(propInfo,factor2);

                if (type != AdditiveVisibility || mode != RenderMode:-1)
                    SetPropInfoVisibilityMode(propInfo,mode);
                if (type != AdditiveVisibility || fx != RenderFx:-1)
                    SetPropInfoVisibilityFx(propInfo,fx);

                if (type != AdditiveVisibility || r >= 0)
                    SetPropInfoVisibilityR(propInfo,r);
                if (type != AdditiveVisibility || g >= 0)
                    SetPropInfoVisibilityG(propInfo,g);
                if (type != AdditiveVisibility || b >= 0)
                    SetPropInfoVisibilityB(propInfo,b);

                if (type != AdditiveVisibility || colorWeapons || r >= 0 || g >= 0 || b >= 0)
                    SetPropInfoVisibilityWeapons(propInfo,colorWeapons);

                if (type != AdditiveVisibility || priority >= VisibilityPriorityNone)
                    SetPropInfoVisibilityPriority(propInfo,priority);

                if (type != AdditiveVisibility || timestamp != 0.0)
                {
                    SetPropInfoVisibilityTime(propInfo, (timestamp == 0.0) ? GetGameTime() : timestamp);
                }
            }

            if (type > NormalVisibility || visibility != oldVisibility ||
                (visibility >= 0 && visibility < 255) ||
                priority >= VisibilityPriorityNone)
            {
                Trace("Set Visibility altered for %d:%L", \
                      client, ValidClientIndex(client));

                m_VisibilityAltered[client] = true;
            }

            if (apply)
            {
                UpdatePlayerProperties(client);
            }
        }

        TraceReturn();
    }
}

/**
 * Overrides the speed for SourceCraft, useful for punishing them or whatever.
 * Pass -1.0 as speed when you want the SourceCraft system to take over again,
 * make sure to do this when they are no longer supposed to follow!
 * @param client: The index of the player.
 * @param speed: The speed you want to pass.
 * @param apply: Set to true to apply the setting immedately.
 * @param id: Identifier to allow more than 1 setting per plugin.
 * @noreturn
 *
 * native SetOverrideSpeed(client,Float:speed=-1.0,bool:apply=false,id=0);
 */
public Native_SetOverrideSpeed(Handle:plugin,numParams)
{
    new client = GetNativeCell(1);
    if (client > 0)
    {
        new Handle:propData=GetPropData(client);
        if (propData != INVALID_HANDLE)
        {
            new Float:speed = GetNativeCell(2);
            new bool:apply  = GetNativeCell(3);
            new id          = GetNativeCell(4);

            new Float:oldSpeed = speed;
            new Handle:propInfo = GetPropInfo(propData, plugin, id);
            if (propInfo == INVALID_HANDLE)
            {
                // create a new entry
                PushArrayCell(propData, CreateProperties(client, plugin, id,
                                        .forceSpeed=speed));
            }
            else
            {
                // update existing entry
                oldSpeed = GetPropInfoOverrideSpeed(propInfo);
                SetPropInfoOverrideSpeed(propInfo,speed);
            }

            if (speed != oldSpeed || (speed >= 0.0 && speed != 1.0))
                m_SpeedAltered[client] = true;

            if (apply)
            {
                UpdatePlayerProperties(client);
            }
        }
    }
}

/**
 * Overrides the gravity for SourceCraft, useful for punishing them or whatever.
 * Pass -1.0 as gravity when you want the SourceCraft system to take over again,
 * make sure to do this when they are no longer supposed to follow!
 * @param client: The index of the player.
 * @param gravity: The gravity you want to pass.
 * @param apply: Set to true to apply the setting immedately.
 * @param id: Identifier to allow more than 1 setting per plugin.
 * @noreturn
 *
 * native SetOverrideGravity(client,Float:gravity=-1.0,bool:apply=false,id=0);
 */
public Native_SetOverrideGravity(Handle:plugin,numParams)
{
    new client = GetNativeCell(1);
    if (client > 0)
    {
        new Handle:propData=GetPropData(client);
        if (propData != INVALID_HANDLE)
        {
            new Float:gravity = GetNativeCell(2);
            new bool:apply    = GetNativeCell(3);
            new id            = GetNativeCell(4);

            new Float:oldGravity = gravity;
            new Handle:propInfo = GetPropInfo(propData, plugin, id);
            if (propInfo == INVALID_HANDLE)
            {
                // create a new entry
                PushArrayCell(propData, CreateProperties(client, plugin, id,
                                        .forceGravity=gravity));
            }
            else
            {
                // update existing entry
                oldGravity = GetPropInfoOverrideGravity(propInfo);
                SetPropInfoOverrideGravity(propInfo,gravity);
            }

            if (gravity != oldGravity || (gravity >= 0.0 && gravity != 1.0))
                m_GravityAltered[client] = true;

            if (apply)
            {
                UpdatePlayerProperties(client);
            }
        }
    }
}

/**
 * Overrides the (in)visibility for SourceCraft , useful for punishing them or whatever.
 * Pass -1 as visibility when you want the SourceCraft system to take over again,
 * make sure to do this when they are no longer supposed to follow!
 * @param client: The index of the player.
 * @param visibility: The visibility you want to pass.
 * @param apply: Set to true to apply the setting immedately.
 * @param id: Identifier to allow more than 1 setting per plugin.
 * @noreturn
 *
 * native SetOverrideVisiblity(client,visibility=-1,bool:apply=false,id=0);
 */
public Native_SetOverrideVisiblity(Handle:plugin,numParams)
{
    new client = GetNativeCell(1);
    if (client > 0)
    {
        new Handle:propData=GetPropData(client);
        if (propData != INVALID_HANDLE)
        {
            new visibility = GetNativeCell(2);
            new bool:apply = GetNativeCell(3);
            new id         = GetNativeCell(4);

            new oldVisibility=visibility;
            new Handle:propInfo = GetPropInfo(propData, plugin, id);
            if (propInfo == INVALID_HANDLE)
            {
                // create a new entry
                PushArrayCell(propData, CreateProperties(client, plugin, id,
                                        .forceVisibility=visibility));
            }
            else
            {
                // update existing entry
                oldVisibility = GetPropInfoOverrideVisibility(propInfo);
                SetPropInfoOverrideVisibility(propInfo,visibility);
            }

            if (visibility != oldVisibility ||
                (visibility >= 0 && visibility < 255))
            {
                m_VisibilityAltered[client] = true;
            }

            if (apply)
            {
                UpdatePlayerProperties(client);
            }
        }
    }
}

/**
 * Sets the MoveType for the player that is handled within the SourceCraft system,
 * Pass MOVETYPE_DEFAULT to not alter the MoveType set by the game or other plugins.
 * @param client:   The index of the player.
 * @param mt:       The MoveType you want to pass.
 * @param apply:    Set to true to apply the setting immedately.
 * @param id:       Identifier to allow more than 1 setting per plugin.
 * @noreturn
 * 
 * native SetMoveType(client,MoveType:mt=MOVETYPE_DEFAULT,bool:apply=false,id=0);
 */
public Native_SetMoveType(Handle:plugin,numParams)
{
    new client = GetNativeCell(1);
    if (client > 0)
    {
        new Handle:propData=GetPropData(client);
        if (propData != INVALID_HANDLE)
        {
            new MoveType:mt = MoveType:GetNativeCell(2);
            new bool:apply  = GetNativeCell(3);
            new id          = GetNativeCell(4);

            new MoveType:oldMt = mt;
            new Handle:propInfo = GetPropInfo(propData, plugin, id);
            if (propInfo == INVALID_HANDLE)
            {
                // create a new entry
                PushArrayCell(propData, CreateProperties(client, plugin, id,
                                        .mt=mt));
            }
            else
            {
                // update existing entry
                oldMt = GetPropInfoMoveType(propInfo);
                SetPropInfoMoveType(propInfo,mt);
            }

            if (mt != oldMt || mt != MOVETYPE_DEFAULT)
                m_MoveTypeAltered[client] = true;

            if (apply)
            {
                UpdatePlayerProperties(client);
            }
        }
    }
}

/**
 * Applies the Speed, Gravity and Visibility settings for the specified player
 * right away. This function is not required to change the player's state.
 * If this is not called, the settings will be applied automatically within 2 seconds.
 * @param client: The client's index to apply the settings for.
 * @noreturn
 *
 * native ApplyPlayerSettings(client);
 */
public Native_ApplyPlayerSettings(Handle:plugin,numParams)
{
    new client = GetNativeCell(1);
    UpdatePlayerProperties(client);
}

/**
 * Sets the physical armor percent used by war3source races
 * @param client:   The index of the player.
 * @param armor:    The armor you want to pass.
 * @param id:       Identifier to allow more than 1 setting per plugin.
 * @noreturn
 *
 * native SetPhysicalArmorPercent(client,Float:armor=0.0,id=0);
 */
public Native_SetPhysicalArmorPercent(Handle:plugin,numParams)
{
    new client = GetNativeCell(1);
    if (client > 0)
    {
        new Handle:propData=GetPropData(client);
        if (propData != INVALID_HANDLE)
        {
            new Float:armor=GetNativeCell(2);

            new id=GetNativeCell(3);
            new Handle:propInfo = GetPropInfo(propData, plugin, id);
            if (propInfo == INVALID_HANDLE)
            {
                // create a new entry
                PushArrayCell(propData, CreateProperties(client, plugin, id,
                                        .physicalArmorPercent=armor));
            }
            else
            {
                // update existing entry
                SetPropInfoPhysicalArmor(propInfo,armor);
            }
        }
    }
}

/**
 * Gets the sum of the physical armor percent set by any (war3source) plugins
 * @param client:   The index of the player.
 * @noreturn
 *
 * native GetPhysicalArmorSum(client);
 */
public Native_GetPhysicalArmorSum(Handle:plugin,numParams)
{
    new client = GetNativeCell(1);
    if (client > 0)
        return _:SumFloatClientProperty(client, PROPINFO_PHYSICAL_ARMOR);
    else
        return _:-1.0;
}

/**
 * Sets the magical armor percent used by war3source races
 * @param client:   The index of the player.
 * @param armor:    The armor you want to pass.
 * @param id:       Identifier to allow more than 1 setting per plugin.
 * @noreturn
 *
 * native SetMagicalArmorPercent(client,Float:armor=0.0,id=0);
 */
public Native_SetMagicalArmorPercent(Handle:plugin,numParams)
{
    new client = GetNativeCell(1);
    if (client > 0)
    {
        new Handle:propData=GetPropData(client);
        if (propData != INVALID_HANDLE)
        {
            new Float:armor=GetNativeCell(2);

            new id=GetNativeCell(3);
            new Handle:propInfo = GetPropInfo(propData, plugin, id);
            if (propInfo == INVALID_HANDLE)
            {
                // create a new entry
                PushArrayCell(propData, CreateProperties(client, plugin, id,
                                        .magicalArmorPercent=armor));
            }
            else
            {
                // update existing entry
                SetPropInfoMagicalArmor(propInfo,armor);
            }
        }
    }
}

/**
 * Gets the sum of the magical armor percent set by any (war3source) plugins
 * @param client:   The index of the player.
 * @noreturn
 *
 * native GetMagicalArmorSum(client);
 */
public Native_GetMagicalArmorSum(Handle:plugin,numParams)
{
    new client = GetNativeCell(1);
    if (client > 0)
        return _:SumFloatClientProperty(client, PROPINFO_MAGICAL_ARMOR);
    else
        return _:-1.0;
}

/**
 * Restores Settings for a player (removes all overrides)
 * @param client: The index of the player.
 * @param apply: Set to true to apply the setting immedately.
 * @return The return value will be true if the restore succeeded.
 *
 * native bool:RestorePlayer(client,bool:apply=false);
 */
public Native_RestorePlayer(Handle:plugin,numParams)
{
    new client = GetNativeCell(1);
    if (client > 0)
    {
        new Action:result=Plugin_Continue;
        Call_StartForward(g_OnPlayerRestoredHandle);
        Call_PushCell(client);
        Call_Finish(result);

        if (result != Plugin_Stop)
        {
            new Handle:propData=GetPropData(client); // Player's properties
            if (propData != INVALID_HANDLE)
            {
                new size=GetArraySize(propData);
                for (new y=0;y<size;y++)
                {
                    new Handle:propInfo = GetArrayCell(propData,y);
                    SetPropInfoOverrideSpeed(propInfo, -1.0);
                    SetPropInfoOverrideGravity(propInfo, -1.0);
                    SetPropInfoOverrideVisibility(propInfo, -1.0);
                    SetPropInfoVisibilityR(propInfo, -1);
                    SetPropInfoVisibilityG(propInfo, -1);
                    SetPropInfoVisibilityB(propInfo, -1);
                }

                if (GetNativeCell(2))
                {
                    UpdatePlayerProperties(client);
                }
                return true;
            }
        }
    }
    return false;
}

/**
 * Respawns a player
 * @param client: The index of the player.
 * @noreturn
 *
 * native RespawnPlayer(client);
 */
public Native_RespawnPlayer(Handle:plugin,numParams)
{
    new client = GetNativeCell(1);

    TraceInto("Respawn", "RespawnPlayer", "client=%d:%L", \
              client, ValidClientIndex(client));

    KillHUDTimer(client);
    KillPropertyTimer(client);
    SetEntPropFloat(client, Prop_Send, "m_flDeathTime", 0.0);

    if (GameType == tf2)
        TF2_RespawnPlayer(client);
    else if (GameType == cstrike)
        CS_RespawnPlayer(client);
    else if (GameType != dod)
        DispatchSpawn(client);

    TraceReturn("Respawn %d:%L", client, ValidClientIndex(client));
}

/**
 * Creates a Cooldown timer for the given client's given race/upgrade
 * @param client:       The index of the player.
 * @param race:         The race and
 * @param upgrade:      The upgrade for the cooldown
 * @param duration:	    Duration of the cooldown.
 * @param name:         The name of the upgrade to use in the client notification (if any)
 * @param type:         The type of cooldown (for reset and notification)
 * @param expireSound:  The sound to play when the cooldown exipres
 * @return The return value will be true if the restore succeeded.
 * 
 * native CreateCooldown(client, race, upgrade, Float:duration=0.0, const String:name[]="",
 *                       CooldownType:type=Cooldown_ResetOnChange |
 *                                         Cooldown_ResetOnDeath  |
 *                                         Cooldown_CreateNotify  |
 *                                         Cooldown_ExpireNotify,
 *                       const String:expireSound[]="")
 */
public Native_CreateCooldown(Handle:plugin,numParams)
{
    new client = GetNativeCell(1);
    new race = GetNativeCell(2);
    new upgrade = GetNativeCell(3);
    new Float:duration = Float:GetNativeCell(4);
    new CooldownType:type = CooldownType:GetNativeCell(6);

    decl String:name[NAME_STRING_LENGTH];
    GetNativeString(5,name,sizeof(name));

    decl String:expireSound[LONG_STRING_LENGTH];
    GetNativeString(7,expireSound,sizeof(expireSound));

    new Handle:raceHandle = GetRaceHandle(race);

    if (duration <= 0.0)
    {
        duration = GetUpgradeCooldownDuration(raceHandle,upgrade);
        type |= GetUpgradeCooldownType(raceHandle,upgrade);
    }

    if (name[0] == '\0')
        GetUpgradeName(raceHandle, upgrade, name, sizeof(name), client);

    CreateCooldown(client, race, upgrade, duration,
                   name, type, expireSound);
}

/**
 * Returns the time of the cooldown expiration.
 * @param client:   The index of the player.
 * @param race:     The race and
 * @param upgrade:  The upgrade for the cooldown
 * @return The return value will be the time of the cooldown expiration, or 0.0 if there isn't any.
 *
 * native Float:GetCooldownExpireTime(client, race, upgrade);
 */
public Native_GetCooldownExpireTime(Handle:plugin,numParams)
{
    return _:GetCooldownExpireTime(GetNativeCell(1), GetNativeCell(2), GetNativeCell(3));
}

/**
 * Returns the type of the cooldown.
 * @param client:   The index of the player.
 * @param race:     The race and
 * @param upgrade:  The upgrade for the cooldown
 * @return The return value will be the type of the cooldown, or Cooldown_None if there isn't any.
 *
 * native CooldownType:GetCooldownType(client, race, upgrade);
 */
public Native_GetCooldownType(Handle:plugin,numParams)
{
    return _:GetCooldownType(GetNativeCell(1), GetNativeCell(2), GetNativeCell(3));
}

/**
 * Check if a cooldown has expired yet.
 * @param client:   The index of the player.
 * @param race:     The race and
 * @param upgrade:  The upgrade for the cooldown
 * @param notify:   Set to true to display a message to the client if the cooldown hasn't expired yet.
 * @return The return value will be true if the cooldown has expired (or there is no cooldown)
 * 
 * native bool:HasCooldownExpired(client, race, upgrade, bool:notify=true);
 */
public Native_HasCooldownExpired(Handle:plugin,numParams)
{
    return HasCooldownExpired(GetNativeCell(1), GetNativeCell(2),
                              GetNativeCell(3), bool:GetNativeCell(4));
}

/**
 * Reset a cooldown
 * @param client:   The index of the player.
 * @param race:     The race and
 * @param upgrade:  The upgrade for the cooldown
 * @param notify:   Set to true to display a message to the client
 * @noreturn
 */
public Native_ResetCooldown(Handle:plugin,numParams)
{
    ResetCooldown(GetNativeCell(1), GetNativeCell(2),
                  GetNativeCell(3), bool:GetNativeCell(4));
}

/**
 * Retrieves an integer value from a config file
 * @param key		Name of the key
 * @param defvalue	Optional default value to use if the key is not found.
 * @param fileId:   Optional fileId for the config value (race ID, SHOPITEM or MAIN)
 * @param node:     Optional node for the config value (upgrade ID, item ID or NONE)
 * @param section   Optional section for the config value
 * 
 * @noreturn
 * native GetConfigNum(const String:key[], defvalue=0, fileId=MAIN,
 *                     node=NONE, const String:section[]="");
 */
public Native_GetConfigNum(Handle:plugin,numParams)
{
    decl String:key[NAME_STRING_LENGTH];
    GetNativeString(1,key,sizeof(key));

    decl String:section[NAME_STRING_LENGTH];
    GetNativeString(5,section,sizeof(section));

    return GetConfigNum(key, GetNativeCell(2), GetNativeCell(3), GetNativeCell(4), section);
}

/**
 * Retrieves a floating point value from a config file
 * @param key		Name of the key
 * @param defvalue	Optional default value to use if the key is not found.
 * @param fileId:   Optional fileId for the config value (race ID, SHOPITEM or MAIN)
 * @param node:     Optional node for the config value (upgrade ID, item ID or NONE)
 * @param section   Optional section for the config value
 * @noreturn
 * 
 * native Float:GetConfigFloat(const String:key[], Float:defvalue=0.0, fileId=MAIN,
 *                             node=NONE, const String:section[]="");
 */
public Native_GetConfigFloat(Handle:plugin,numParams)
{
    decl String:key[NAME_STRING_LENGTH];
    GetNativeString(1,key,sizeof(key));

    decl String:section[NAME_STRING_LENGTH];
    GetNativeString(5,section,sizeof(section));

    return _:GetConfigFloat(key, Float:GetNativeCell(2), GetNativeCell(3), GetNativeCell(4), section);
}

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
 * 
 * native GetConfigString(const String:key[], String:value[], maxlength,
                          const String:defvalue[]="", fileId=MAIN,
                          node=NONE, const String:section[]="");
 */
public Native_GetConfigString(Handle:plugin,numParams)
{
    decl String:key[NAME_STRING_LENGTH];
    GetNativeString(1,key,sizeof(key));

    decl String:defvalue[LONG_STRING_LENGTH];
    GetNativeString(4,defvalue,sizeof(defvalue));

    decl String:section[NAME_STRING_LENGTH];
    GetNativeString(7,section,sizeof(section));

    decl String:value[LONG_STRING_LENGTH];
    GetConfigString(key, value, sizeof(value), defvalue,
                    GetNativeCell(5), GetNativeCell(6), section);

    SetNativeString(2, value, GetNativeCell(3));
}

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
 * 
 * native GetConfigArray(const String:key[], any:value[], size, const any:defvalue[],
 *                       fileId=MAIN, node=NONE, const String:section[]="");
 */
public Native_GetConfigArray(Handle:plugin,numParams)
{
    decl String:key[NAME_STRING_LENGTH];
    GetNativeString(1,key,sizeof(key));

    decl String:section[NAME_STRING_LENGTH];
    GetNativeString(7,section,sizeof(section));

    decl any:value[256];
    decl any:defvalue[256];
    GetNativeArray(4, defvalue, sizeof(defvalue));

    new size = GetNativeCell(3);
    if (size > sizeof(value))
        size = sizeof(value);

    new count = GetConfigArray(key, value, size, defvalue,
                               GetNativeCell(5), // fileId
                               GetNativeCell(6), // node
                               section);

    SetNativeArray(6, value, size);                               
    return count;                         
}

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
 * 
 * native GetConfigFloatArray(const String:key[], Float:value[], size,
 *                            const Float:defvalue[], fileId=MAIN,
 *                            node=NONE, const String:section[]="");
 */
public Native_GetConfigFloatArray(Handle:plugin,numParams)
{
    decl String:key[NAME_STRING_LENGTH];
    GetNativeString(1,key,sizeof(key));

    decl String:section[NAME_STRING_LENGTH];
    GetNativeString(7,section,sizeof(section));

    decl Float:value[256];
    decl Float:defvalue[256];
    GetNativeArray(4, defvalue, sizeof(defvalue));

    new size = GetNativeCell(3);
    if (size > sizeof(value))
        size = sizeof(value);

    new count = GetConfigFloatArray(key, value, size, defvalue,
                                    GetNativeCell(5), // fileId
                                    GetNativeCell(6), // node
                                    section);

    SetNativeArray(6, value, size);                               
    return count;                         
}

/**
 * Loads a translation file for the Sourcecraft plugin.
 * If no extension is specified, .txt is assumed.
 *
 * @param path		Translation file.
 * @noreturn
 * 
 * native LoadSourceCraftTranslations(const String:file[]);
 */
public Native_LoadSCTranslations(Handle:plugin,numParams)
{
    decl String:file[PLATFORM_MAX_PATH];
    GetNativeString(1,file,sizeof(file));
    LoadTranslations(file);
}

InitNatives()
{
    CreateNative("IsSourceCraftLoaded",Native_IsSourceCraftLoaded);
    CreateNative("GetRoundState",Native_GetRoundState);
    CreateNative("CreateRace",Native_CreateRace);
    CreateNative("SetRaceAttributes",Native_SetRaceAttributes);
    CreateNative("RaceComplete",Native_RaceComplete);
    CreateNative("AddUpgrade",Native_AddUpgrade);
    CreateNative("SetUpgradeCooldown",Native_SetUpgradeCooldown);
    CreateNative("GetUpgradeCooldown",Native_GetUpgradeCooldown);
    CreateNative("GetRaceName",Native_GetRaceName);
    CreateNative("GetRaceDescription",Native_GetRaceDescription);
    CreateNative("GetRaceShortName",Native_GetRaceShortName);
    CreateNative("GetRaceParentName",Native_GetRaceParentName);
    CreateNative("GetRaceParentId",Native_GetRaceParentId);
    CreateNative("GetRaceFaction",Native_GetRaceFaction);
    CreateNative("GetRaceCount",Native_GetRaceCount);
    CreateNative("GetUnitTypeName",Native_GetUnitTypeName);
    CreateNative("GetFactionName",Native_GetFactionName);
    CreateNative("GetFactionDescription",Native_GetFactionDescription);
    CreateNative("GetFactionCrystalSound",Native_GetFactionCrystalSound);
    CreateNative("EmitCrystalSoundToClient",Native_EmitCrystalSoundToClient);
    CreateNative("GetFactionVespeneSound",Native_GetFactionVespeneSound);
    CreateNative("EmitVespeneSoundToClient",Native_EmitVespeneSoundToClient);
    CreateNative("GetFactionEnergySound",Native_GetFactionEnergySound);
    CreateNative("EmitEnergySoundToClient",Native_EmitEnergySoundToClient);
    CreateNative("GetFactionLevelSound",Native_GetFactionLevelSound);
    CreateNative("EmitLevelSoundToClient",Native_EmitLevelSoundToClient);
    CreateNative("GetUpgradeCount",Native_GetUpgradeCount);
    CreateNative("GetUpgradeName",Native_GetUpgradeName);
    CreateNative("GetUpgradeDescription",Native_GetUpgradeDescription);
    CreateNative("GetUpgradeShortName",Native_GetUpgradeShortName);
    CreateNative("GetUpgradeMaxLevel",Native_GetUpgradeMaxLevel);
    CreateNative("GetUpgradeCategory",Native_GetUpgradeCategory);
    CreateNative("SetUpgradeCategory",Native_SetUpgradeCategory);
    CreateNative("GetUpgradeEnergy",Native_GetUpgradeEnergy);
    CreateNative("GetUpgradeRecurringEnergy",N_GetUpgradeRecurringEnergy);
    CreateNative("GetUpgradeVespene",Native_GetUpgradeVespene);
    CreateNative("GetUpgradeCrystals",Native_GetUpgradeCrystals);
    CreateNative("CreateShopItem",Native_CreateShopItem);
    CreateNative("GetShopItemCount",Native_GetShopItemCount);
    CreateNative("GetItemName",Native_GetItemName);
    CreateNative("GetItemDescription",Native_GetItemDescription);
    CreateNative("GetItemCategory",Native_GetItemCategory);
    CreateNative("GetItemShortName",Native_GetItemShortName);
    CreateNative("SetItemShortName",Native_SetItemShortName);
    CreateNative("CreateHelpCommand",Native_CreateHelpCommand);
    CreateNative("FindShopItem",Native_FindShopItem);
    CreateNative("FindRace",Native_FindRace);
    CreateNative("HealPlayer",Native_HealPlayer);
    CreateNative("HurtPlayer",Native_HurtPlayer);
    CreateNative("KillPlayer",Native_KillPlayer);
    CreateNative("DamageEntity",Native_DamageEntity);
    CreateNative("GetDamageFromPlayerHurt",Native_GetDamageFromPlayerHurt);
    CreateNative("GetSuppressDamageForward",Native_GetSuppressDamageForward);
    CreateNative("GetDamage",Native_GetDamage);
    CreateNative("DisplayDamage",Native_DisplayDamage);
    CreateNative("DisplayKill",Native_DisplayKill);
    CreateNative("DisplayMessage",Native_DisplayMessage);
    CreateNative("GetHudMessage",Native_GetHudMessage);
    CreateNative("HudMessage",Native_HudMessage);
    CreateNative("ClearHud",Native_ClearHud);
    CreateNative("ShowXP",Native_ShowXP);
    CreateNative("ShowCrystals",Native_ShowCrystals);
    CreateNative("ShowVespene",Native_ShowVespene);
    CreateNative("ShowEnergy",Native_ShowEnergy);
    CreateNative("GetArmor",Native_GetArmor);
    CreateNative("SetArmor",Native_SetArmor);
    CreateNative("HasShields",Native_HasShields);
    CreateNative("SetArmorName",Native_SetArmorName);
    CreateNative("GetArmorName",Native_GetArmorName);
    CreateNative("SetArmorPercent",Native_SetArmorPercent);
    CreateNative("GetArmorPercent",Native_GetArmorPercent);
    CreateNative("SetArmorAmount",Native_SetArmorAmount);
    CreateNative("IncrementArmor",Native_IncrementArmor);
    CreateNative("DecrementArmor",Native_DecrementArmor);
    CreateNative("GetSettingsBits",Native_GetSettingsBits);
    CreateNative("GetSetting",Native_GetSetting);
    CreateNative("IsChangingClass",Native_IsChangingClass);
    CreateNative("GetRace",Native_GetRace);
    CreateNative("ChangeRace",Native_ChangeRace);
    CreateNative("GetXP",Native_GetXP);
    CreateNative("SetXP",Native_SetXP);
    CreateNative("GetLevel",Native_GetLevel);
    CreateNative("SetLevel",Native_SetLevel);
    CreateNative("ResetLevel",Native_ResetLevel);
    CreateNative("GetUpgradeLevel",Native_GetUpgradeLevel);
    CreateNative("SetUpgradeLevel",Native_SetUpgradeLevel);
    CreateNative("ResetUpgradeLevels",Native_ResetUpgradeLevels);
    CreateNative("ResetXP",Native_ResetXP);
    CreateNative("GetKillXP",Native_GetKillXP);
    CreateNative("GetKillLevelXP",Native_GetKillLevelXP);
    CreateNative("GetLevelXP",Native_GetLevelXP);
    CreateNative("GetReqLevelXP",Native_GetReqLevelXP);
    CreateNative("GetAccumulatedEnergy",Native_GetAccumulatedEnergy);
    CreateNative("SetAccumulatedEnergy",Native_SetAccumulatedEnergy);
    CreateNative("GetEnergy",Native_GetEnergy);
    CreateNative("SetEnergy",Native_SetEnergy);
    CreateNative("GetEnergyRate",Native_GetEnergyRate);
    CreateNative("SetEnergyRate",Native_SetEnergyRate);
    CreateNative("GetEnergyLimit",Native_GetEnergyLimit);
    CreateNative("SetEnergyLimit",Native_SetEnergyLimit);
    CreateNative("GetInitialEnergy",Native_GetInitialEnergy);
    CreateNative("SetInitialEnergy",Native_SetInitialEnergy);
    CreateNative("GetMaxCrystals",Native_GetMaxCrystals);
    CreateNative("GetCrystals",Native_GetCrystals);
    CreateNative("SetCrystals",Native_SetCrystals);
    CreateNative("GetMaxVespene",Native_GetMaxVespene);
    CreateNative("GetVespene",Native_GetVespene);
    CreateNative("SetVespene",Native_SetVespene);
    CreateNative("GetOverallLevel",Native_GetOverallLevel);
    CreateNative("SetOverallLevel",Native_SetOverallLevel);
    CreateNative("GetOwnsItem",Native_GetOwnsItem);
    CreateNative("SetOwnsItem",Native_SetOwnsItem);
    CreateNative("GetAttributeBits",Native_GetAttributeBits);
    CreateNative("SetAttributeBits",Native_SetAttributeBits);
    CreateNative("GetAttribute",Native_GetAttribute);
    CreateNative("SetAttribute",Native_SetAttribute);
    CreateNative("GetImmunityBits",Native_GetImmunityBits);
    CreateNative("SetImmunityBits",Native_SetImmunityBits);
    CreateNative("GetImmunity",Native_GetImmunity);
    CreateNative("SetImmunity",Native_SetImmunity);
    CreateNative("SetSpeed",Native_SetSpeed);
    CreateNative("SetGravity",Native_SetGravity);
    CreateNative("SetVisibility",Native_SetVisibility);
    CreateNative("SetOverrideSpeed",Native_SetOverrideSpeed);
    CreateNative("SetOverrideGravity",Native_SetOverrideGravity);
    CreateNative("SetOverrideVisiblity",Native_SetOverrideVisiblity);
    CreateNative("SetMoveType",Native_SetMoveType);
    CreateNative("ApplyPlayerSettings",Native_ApplyPlayerSettings);
    CreateNative("SetPhysicalArmorPercent",Native_SetPhysicalArmorPercent);
    CreateNative("GetPhysicalArmorSum",Native_GetPhysicalArmorSum);
    CreateNative("SetMagicalArmorPercent",Native_SetMagicalArmorPercent);
    CreateNative("GetMagicalArmorSum",Native_GetMagicalArmorSum);
    CreateNative("RestorePlayer",Native_RestorePlayer);
    CreateNative("RespawnPlayer",Native_RespawnPlayer);
    CreateNative("CreateCooldown",Native_CreateCooldown);
    CreateNative("ResetCooldown",Native_ResetCooldown);
    CreateNative("GetCooldownExpireTime",Native_GetCooldownExpireTime);
    CreateNative("HasCooldownExpired",Native_HasCooldownExpired);
    CreateNative("GetCooldownType",Native_GetCooldownType);
    CreateNative("GetConfigNum",Native_GetConfigNum);
    CreateNative("GetConfigFloat",Native_GetConfigFloat);
    CreateNative("GetConfigString",Native_GetConfigString);
    CreateNative("GetConfigArray",Native_GetConfigArray);
    CreateNative("GetConfigFloatArray",Native_GetConfigFloatArray);
    CreateNative("LoadSourceCraftTranslations",Native_LoadSCTranslations);
    RegPluginLibrary("sourcecraft");
}

InitForwards()
{
    g_OnSourceCraftReadyHandle=CreateGlobalForward("OnSourceCraftReady",ET_Ignore);

    g_OnPlayerAuthedHandle=CreateGlobalForward("OnPlayerAuthed",ET_Ignore,Param_Cell);
    g_OnPlayerRestoredHandle=CreateGlobalForward("OnPlayerRestored",ET_Event,Param_Cell);
    g_OnUltimateCommandHandle=CreateGlobalForward("OnUltimateCommand",ET_Ignore,Param_Cell,Param_Cell,Param_Cell,Param_Cell);
    g_OnRaceSelectedHandle=CreateGlobalForward("OnRaceSelected",ET_Hook,Param_Cell,Param_Cell,Param_Cell);
    g_OnRaceDeselectedHandle=CreateGlobalForward("OnRaceDeselected",ET_Event,Param_Cell,Param_Cell,Param_Cell);
    g_OnUpgradeLevelChangedHandle=CreateGlobalForward("OnUpgradeLevelChanged",ET_Ignore,Param_Cell,Param_Cell,Param_Cell,Param_Cell);
    g_OnItemPurchaseHandle=CreateGlobalForward("OnItemPurchase",ET_Ignore,Param_Cell,Param_Cell);
    g_OnItemLostHandle=CreateGlobalForward("OnItemLost",ET_Ignore,Param_Cell,Param_Cell);

    g_OnXPGivenHandle=CreateGlobalForward("OnXPGiven",ET_Event,Param_Cell,Param_CellByRef,Param_Cell);
    g_OnCrystalsGivenHandle=CreateGlobalForward("OnCrystalsGiven",ET_Event,Param_Cell,Param_CellByRef,Param_Cell);
    g_OnVespeneGivenHandle=CreateGlobalForward("OnVespeneGiven",ET_Event,Param_Cell,Param_CellByRef,Param_Cell);
    g_OnPlayerTakeDamageHandle=CreateGlobalForward("OnPlayerTakeDamage",ET_Event,Param_Cell,Param_CellByRef,Param_CellByRef,Param_FloatByRef,Param_CellByRef);

    g_OnPlayerHurtEventHandle=CreateGlobalForward("OnPlayerHurtEvent", ET_Event,Param_Cell,Param_Cell,Param_Cell,Param_Cell,Param_Cell,Param_Cell,Param_Cell,Param_Cell,Param_Cell,Param_Cell);
    g_OnPlayerDeathEventHandle=CreateGlobalForward("OnPlayerDeathEvent", ET_Ignore,Param_Cell,Param_Cell,Param_Cell,Param_Cell,Param_Cell,Param_Cell,Param_Cell,Param_Cell,Param_String,Param_Cell,Param_Cell,Param_Cell,Param_Cell,Param_Cell);
    g_OnPlayerSpawnEventHandle=CreateGlobalForward("OnPlayerSpawnEvent", ET_Ignore,Param_Cell,Param_Cell,Param_Cell);
    g_OnPlayerBuiltObject=CreateGlobalForward("OnPlayerBuiltObject", ET_Ignore,Param_Cell,Param_Cell,Param_Cell,Param_Cell);

    g_OnCabinetUsedHandle = CreateGlobalForward("OnCabinetUsed", ET_Ignore,Param_Cell,Param_Cell);
    g_OnHealthUsedHandle = CreateGlobalForward("OnHealthUsed", ET_Ignore,Param_Cell,Param_Cell);
    g_OnCooldownExpiredHandle=CreateGlobalForward("OnCooldownExpired",ET_Ignore,Param_Cell,Param_Cell,Param_Cell,Param_Cell);
}

