/**
 * vim: set ai et ts=4 sw=4 syntax=sourcepawn :
 * File: faction.inc
 * Description: Defines the faction for SourceCraft
 * Author(s): Naris (Murray Wilson)
 */
 
#if defined _factions_included
 #endinput
#endif
#define _factions_included

stock const String:g_UnitTypeShortNames[UnitType][]= { "", "biological", "mechanical", "bio-mechanical", "robotic",
                                                       "energy", "magical", "mystical", "elemental", "undead",
                                                       "demonic", "cybernetic", "flyer" };

stock const String:g_FactionShortNames[Faction][]  = { "", "the_terrans", "the_protoss", "the_zerg_swarm", "the_human_alliance",
                                                       "the_orcish_hoard", "the_night_elf", "the_undead_scourge",
                                                       "the_burning_legion", "the_hellbourne", "the_legion",
                                                       "the_sentinel", "the_naga", "the_titans", "the_xel_naga" };

GetUnitTypeName(UnitType:type, String:name[], maxlength, lang=LANG_SERVER)
{
    if (type > GenericUnit)
    {
        // Translate the short name to get the name
        Format(name,maxlength,"%T",g_UnitTypeShortNames[type],lang);
    }
    else
        name[0] = '\0';
}

GetFactionName(Faction:faction, String:name[], maxlength, lang=LANG_SERVER)
{
    if (faction > Generic)
    {
        // Translate the short name to get the name
        Format(name,maxlength,"%T",g_FactionShortNames[faction],lang);
    }
    else
        name[0] = '\0';
}

GetFactionDescription(Faction:faction, String:desc[], maxlength, lang=LANG_SERVER)
{
    if (faction > Generic)
    {
        // Translate the short name to get the description
        decl String:key[NAME_STRING_LENGTH];
        Format(key,sizeof(key),"%s_desc",g_FactionShortNames[faction]);
        Format(desc,maxlength,"%T",key,lang);
    }
    else
        desc[0] = '\0';
}

GetFactionImage(Faction:faction, String:image[], maxlength, lang=LANG_SERVER)
{
    if (faction > Generic)
    {
        // Translate the short name + _image to get the image 
        decl String:key[NAME_STRING_LENGTH];
        Format(key,sizeof(key),"%s_image",g_FactionShortNames[faction]);
        Format(image,maxlength,"%T",key,lang);
    }
    else
        image[0] = '\0';
}

GetFactionCrystalSound(Faction:faction, String:sound[], maxlength, lang=LANG_SERVER)
{
    if (faction > Generic)
    {
        // Translate the short name + _image to get the image 
        decl String:key[NAME_STRING_LENGTH];
        Format(key,sizeof(key),"%s_crystal_sound",g_FactionShortNames[faction]);
        Format(sound,maxlength,"%T",key,lang);
    }
    else
        strcopy(sound,maxlength,"sc/taderr00.wav");
}

EmitCrystalSoundToClient(client, Faction:faction)
{
    decl String:sound[PLATFORM_MAX_PATH];
    GetFactionCrystalSound(faction, sound, sizeof(sound), client);
    PrepareSound(sound);
    EmitSoundToClient(client,sound);
}

GetFactionVespeneSound(Faction:faction, String:sound[], maxlength, lang=LANG_SERVER)
{
    if (faction > Generic)
    {
        // Translate the short name + _image to get the image 
        decl String:key[NAME_STRING_LENGTH];
        Format(key,sizeof(key),"%s_vespene_sound",g_FactionShortNames[faction]);
        Format(sound,maxlength,"%T",key,lang);
    }
    else
        strcopy(sound,maxlength,"sc/taderr01.wav");
}

EmitVespeneSoundToClient(client, Faction:faction)
{
    decl String:sound[PLATFORM_MAX_PATH];
    GetFactionVespeneSound(faction, sound, sizeof(sound), client);
    PrepareSound(sound);
    EmitSoundToClient(client,sound);
}

GetFactionEnergySound(Faction:faction, String:sound[], maxlength, lang=LANG_SERVER)
{
    if (faction > Generic)
    {
        // Translate the short name + _image to get the image 
        decl String:key[NAME_STRING_LENGTH];
        Format(key,sizeof(key),"%s_energy_sound",g_FactionShortNames[faction]);
        Format(sound,maxlength,"%T",key,lang);
    }
    else
        strcopy(sound,maxlength,"sc/taderr06.wav");
}

EmitEnergySoundToClient(client, Faction:faction)
{
    decl String:sound[PLATFORM_MAX_PATH];
    GetFactionEnergySound(faction, sound, sizeof(sound), client);
    PrepareSound(sound);
    EmitSoundToClient(client,sound);
}

GetFactionLevelSound(Faction:faction, String:sound[], maxlength, lang=LANG_SERVER)
{
    if (faction > Generic)
    {
        // Translate the short name + _image to get the image 
        decl String:key[NAME_STRING_LENGTH];
        Format(key,sizeof(key),"%s_level_sound",g_FactionShortNames[faction]);
        Format(sound,maxlength,"%T",key,lang);
    }
    else
        strcopy(sound,maxlength,"sc/GenericWarningUpgradeComplete1.mp3");
}

EmitLevelSoundToClient(client, Faction:faction)
{
    decl String:sound[PLATFORM_MAX_PATH];
    GetFactionLevelSound(faction, sound, sizeof(sound), client);
    PrepareSound(sound);
    EmitSoundToClient(client,sound);
}

LoadFactionTranslations()
{
    for (new Faction:faction = Terran; faction < Faction:sizeof(g_FactionShortNames); faction++)
    {
        decl String:file[PLATFORM_MAX_PATH];
        Format(file,sizeof(file),"sc.%s.phrases.txt",g_FactionShortNames[faction]);
        LoadTranslations(file);
    }
}

