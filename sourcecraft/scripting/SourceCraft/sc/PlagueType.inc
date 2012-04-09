/**
 * vim: set ai et ts=4 sw=4 syntax=sourcepawn :
 * File: PlagueType.inc
 * Description: The types of Plague for the Plague/Infection upgrade for SourceCraft.
 * Author(s): -=|JFH|=-Naris
 */
 
#if defined plague_type_included
 #endinput
#endif
#define plague_type_included

enum PlagueType (<<= 1)
{
    NormalPlague = 0,
    UltimatePlague = 1,     // Plague is an Ultimate
	ContagiousPlague,       // Plague can spread
    EnsnaringPlague,        // Victim is ensnared/can't move
    ExplosivePlague,        // Victim explodes upon death
    FatalPlague,            // Plague can kill
    IrradiatePlague,        // Plague is radiation
    PoisonousPlague,        // Plague is a poison
    InfectiousPlague,       // Plague is an infection
    FlamingPlague           // Plague is fire
    //BuildingPlague        // Plague also affects buildings
};

enum ExplosionType (<<= 1)
{
    NormalExplosion = 0,
	OnDeathExplosion = 1,   // Explosion occurred upon death
    FlamingExplosion,       // Explosion also sets victims on fire
    NonFatalExplosion,      // Explosion doesn't kill player
    SkillExplosion,         // Explosion is a Skill/Upgrade, not an Ultimate
    SmallExplosion,         // Use the Explosion Effect instead of the BigExplosion
    RingExplosion           // Also use the BeamRingPoint Effect
};
