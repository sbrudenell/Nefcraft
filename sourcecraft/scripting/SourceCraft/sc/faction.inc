/**
 * vim: set ai et ts=4 sw=4 syntax=sourcepawn :
 * File: faction.inc
 * Description: Defines the faction for SourceCraft
 * Author(s): Naris (Murray Wilson)
 */
 
#if defined _faction_included
 #endinput
#endif
#define _faction_included

enum Faction
{
    Unassigned=-1,
    Generic=0,
    Terran,
    Protoss,
    Zerg,
    HumanAlliance,
    OrcishHoard,
    NightElf,
    UndeadScourge,
    BurningLegion,
    Hellbourne,
    TheLegion,
    Sentinel,
    Naga,
    Titan,
    XelNaga
};

enum UnitType
{
    UnassignedUnit=-1,
    GenericUnit=0,
    Biological,
    Mechanical,
    BioMechanical,
    Robotic,
    Energy,
    Magical,
    Mystical,
    Elemental,
    Undead,
    Demonic,
    Cybernetic,
    Flyer
};

