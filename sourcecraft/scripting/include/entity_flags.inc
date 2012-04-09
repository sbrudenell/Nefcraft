/**
 * vim: set ai et ts=4 sw=4 :
 * File: entityflags.inv
 * Description: Definitions of various entity flags
 * Author(s): Naris
 */

#if defined _entity_flags_included
 #endinput
#endif
#define _entity_flags_included

enum Move_Collide_t
{
    MOVECOLLIDE_DEFAULT = 0,
    MOVECOLLIDE_FLY_BOUNCE = 1
};

enum Collision_Group_t
{
    COLLISION_GROUP_NONE  = 0,
    COLLISION_GROUP_DEBRIS,            // Collides with nothing but world and static stuff
    COLLISION_GROUP_DEBRIS_TRIGGER, // Same as debris, but hits triggers
    COLLISION_GROUP_INTERACTIVE_DEB, // RIS, // Collides with everything except other interactive debris or debris
    COLLISION_GROUP_INTERACTIVE,    // Collides with everything except interactive debris or debris
    COLLISION_GROUP_PLAYER,
    COLLISION_GROUP_BREAKABLE_GLASS,
    COLLISION_GROUP_VEHICLE,
    COLLISION_GROUP_PLAYER_MOVEMENT,  // For HL2, same as Collision_Group_Player
                                        
    COLLISION_GROUP_NPC,            // Generic NPC group
    COLLISION_GROUP_IN_VEHICLE,        // for any entity inside a vehicle
    COLLISION_GROUP_WEAPON,            // for any weapons that need collision detection
    COLLISION_GROUP_VEHICLE_CLIP,    // vehicle clip brush to restrict vehicle movement
    COLLISION_GROUP_PROJECTILE,        // Projectiles!
    COLLISION_GROUP_DOOR_BLOCKER,    // Blocks entities not permitted to get near moving doors
    COLLISION_GROUP_PASSABLE_DOOR,    // Doors that the player shouldn't collide with
    COLLISION_GROUP_DISSOLVING,        // Things that are dissolving are in this group
    COLLISION_GROUP_PUSHAWAY,        // Nonsolid on client and server, pushaway in player code

    COLLISION_GROUP_NPC_ACTOR,        // Used so NPCs in scripts ignore the player.

    LAST_SHARED_COLLISION_GROUP
};

enum SolidFlags_t
{
    FSOLID_CUSTOMRAYTEST        = 0x0001,    // Ignore solid type + always call into the entity for ray tests
    FSOLID_CUSTOMBOXTEST        = 0x0002,    // Ignore solid type + always call into the entity for swept box tests
    FSOLID_NOT_SOLID            = 0x0004,    // Are we currently not solid?
    FSOLID_TRIGGER              = 0x0008,    // This is something may be collideable but fires touch functions
                                            // even when it's not collideable (when the FSOLID_NOT_SOLID flag is set)
    FSOLID_NOT_STANDABLE        = 0x0010,    // You can't stand on this
    FSOLID_VOLUME_CONTENTS      = 0x0020,    // Contains volumetric contents (like water)
    FSOLID_FORCE_WORLD_ALIGNED  = 0x0040,    // Forces the collision rep to be world-aligned even if it's SOLID_BSP or SOLID_VPHYSICS
    FSOLID_USE_TRIGGER_BOUNDS   = 0x0080,    // Uses a special trigger bounds separate from the normal OBB
    FSOLID_ROOT_PARENT_ALIGNED  = 0x0100,    // Collisions are defined in root parent's local coordinate space

    FSOLID_MAX_BITS    = 9
};

enum SolidType_t
{
    SOLID_NONE            = 0,    // no solid model
    SOLID_BSP            = 1,    // a BSP tree
    SOLID_BBOX            = 2,    // an AABB
    SOLID_OBB            = 3,    // an OBB (not implemented yet)
    SOLID_OBB_YAW        = 4,    // an OBB, constrained so that it can only yaw
    SOLID_CUSTOM        = 5,    // Always call into the entity for tests
    SOLID_VPHYSICS        = 6,    // solid vphysics object, get vcollide from the model and collide with that
    SOLID_LAST
};
