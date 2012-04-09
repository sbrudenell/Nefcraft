/**
 * vim: set ai et ts=4 sw=4 syntax=sourcepawn :
 * File: ResourceManager.inc
 * Description: Plugin to manage precaching resources globally.
 * Author(s): Naris (Murray Wilson)
 */
 
#if defined _ResourceManager_included
 #endinput
#endif
#define _ResourceManager_included

#define DONT_DOWNLOAD    0
#define DOWNLOAD         1
#define ALWAYS_DOWNLOAD  2

/**
 * Sets up a given sound.
 *
 * @param model			Name of the sound to precache.
 * @param download		If download is 2 the file will be added to the downloadables table,
 *                      If download is 1 the file be added if it's within the allotted number of files.
 * @param precache		If precache is true the file will be precached.
 * @param preload		If preload is true the file will be precached before level startup.
 * @return				Returns a model index (if precached).
 */
native SetupSound(const String:sound[], download=DOWNLOAD,
                  bool:precache=false, bool:preload=false);

/**
 * Prepares a given sound for use.
 *
 * @param decal			Name of the sound to prepare.
 * @param preload		If preload is true the file will be precached before level startup (if required).
 * @noreturn
 */
native PrepareSound(const String:sound[], bool:preload=false);

/**
 * Sets up a given model.
 *
 * @param model			Name of the model to precache.
 * @param index			Returns the model index (if precached).
 * @param download		If download is true the file will be added to the downloadables table.
 * @param precache		If precache is true the file will be precached.
 * @param preload		If preload is true the file will be precached before level startup.
 * @return				Returns a model index (if precached).
 */
native SetupModel(const String:model[], &index=0, bool:download=true,
                  bool:precache=false, bool:preload=false);

/**
 * Prepares a given model for use.
 *
 * @param decal			Name of the model to prepare.
 * @param index			Returns the model index.
 * @param preload		If preload is true the file will be precached before level startup (if required).
 * @return				Returns a model index.
 */
native PrepareModel(const String:model[], &index=0, bool:preload=false);

/**
 * Sets up a given decal.
 *
 * @param decal			Name of the decal to precache.
 * @param index			Returns the decal index (if precached).
 * @param download		If download is true the file will be added to the downloadables table.
 * @param precache		If precache is true the file will be precached.
 * @param preload		If preload is true the file will be precached before level startup.
 * @return				Returns a decal index (if precached).
 */
native SetupDecal(const String:decal[], &index=0, bool:download=true,
                  bool:precache=false, bool:preload=false);

/**
 * Prepares a given decal for use.
 *
 * @param decal			Name of the decal to prepare.
 * @param index			Returns the decal index.
 * @param preload		If preload is true the file will be precached before level startup (if required).
 * @return				Returns a decal index.
 */
native PrepareDecal(const String:model[], &index=0, bool:preload=false);

