/**
 * vim: set ai et ts=4 sw=4 syntax=sourcepawn :
 * File: db.inc
 * Description: Loads and Saves Player data to the DataBase.
 * Author(s): -=|JFH|=-Naris
 */

#if defined _db_included
 #endinput
#endif
#define _db_included

#define ID_STRING_LENGTH                64
#define ERROR_STRING_LENGTH             512

#define QUERY_STRING_LENGTH             512
#define SHORT_QUERY_STRING_LENGTH       128
#define LONG_QUERY_STRING_LENGTH        1024
#define REALLY_LONG_QUERY_STRING_LENGTH 2048

#define DB_MAIN                         0
#define DB_THREAD                       1
#define DB_MAX                          2

#define CONNECTING                      (Handle:-1)

#define CloseDbHandle(%1)       CloseHandle(g_DbHandle[%1]); g_DbHandle[%1] = INVALID_HANDLE
#define DatabaseHandle(%1)      ((g_DbHandle[%1] == INVALID_HANDLE) ? ConnectToDatabase(%1, false) : g_DbHandle[%1])
#define DatabaseConnected(%1)   (g_DbHandle[%1] != CONNECTING && g_DbHandle[%1] != INVALID_HANDLE)
#define DatabaseAvailable(%1)   (g_DbHandle[%1] != CONNECTING && DatabaseHandle(%1) != INVALID_HANDLE && g_DbHandle[%1] != CONNECTING)

new bool:g_MapChanging = false;
#pragma unused g_MapChanging // Prevent unused warnings about g_MapChanging

new Handle:g_DbHandle[DB_MAX]  = { INVALID_HANDLE, ...};
new Handle:g_SaveThreadsInFlight = INVALID_HANDLE;
new Handle:g_PlayersToReload = INVALID_HANDLE;
new String:g_DBConf[64];

bool:InitDatabase()
{
    if (g_SaveThreadsInFlight == INVALID_HANDLE)
        g_SaveThreadsInFlight = CreateTrie();

    if (g_PlayersToReload == INVALID_HANDLE)
        g_PlayersToReload = CreateTrie();

    if (SQL_CheckConfig("sourcecraft"))
        strcopy(g_DBConf, sizeof(g_DBConf), "sourcecraft");
    else
        strcopy(g_DBConf, sizeof(g_DBConf), "default");

    new bool:status = true;
    for (new i=0; i < sizeof(g_DbHandle); i++)
    {
        if (ConnectToDatabase(i, (i != DB_MAIN)) == INVALID_HANDLE)
            status = false;
    }
    return status;
}

ClearDatabase()
{
    CloseDatabase();

    if (g_SaveThreadsInFlight != INVALID_HANDLE)
    {
        CloseHandle(g_SaveThreadsInFlight);
        g_SaveThreadsInFlight = INVALID_HANDLE;
    }

    if (g_PlayersToReload != INVALID_HANDLE)
    {
        CloseHandle(g_PlayersToReload);
        g_PlayersToReload = INVALID_HANDLE;
    }
}

UpdateThreadsInFlight(const String:key[], value)
{
    new threads;
    if (GetTrieValue(g_SaveThreadsInFlight, key, threads))
    {
        threads += value;
        if (threads > 0 || value > 0)
            SetTrieValue(g_SaveThreadsInFlight, key, threads, true);
        else                
            RemoveFromTrie(g_SaveThreadsInFlight, key);
    }

    // If all the save threads have completed
    // and the player needs to be reloaded
    new userid;
    if (threads <= 0 && value < 0 &&
        GetTrieValue(g_PlayersToReload, key, userid))
    {
        RemoveFromTrie(g_PlayersToReload, key);
        if (GetClientOfUserId(userid) > 0)
        {
            CreateTimer(0.1, Retry_LoadPlayerData,
                        userid,TIMER_FLAG_NO_MAPCHANGE);
        }
    }
}

Handle:ConnectToDatabase(connection, bool:threaded)
{
    if (threaded)
    {
        SQL_TConnect(SQL_Connected, g_DBConf,connection);
        return CONNECTING;
    }
    else
    {
        decl String:error[ERROR_STRING_LENGTH]; error[0] = '\0';
        g_DbHandle[connection] = SQL_Connect(g_DBConf,true,error,sizeof(error));
        if (g_DbHandle[connection] == INVALID_HANDLE || error[0] != '\0')
        {
            LogError("Error connecting to the database:%s", error);
            if (g_DbHandle[connection] != INVALID_HANDLE)
            {
                CloseDbHandle(connection);
            }
        }
        else
        {
            decl String:dbident[ID_STRING_LENGTH];
            SQL_ReadDriver(g_DbHandle[connection], dbident, sizeof(dbident));
            LogToGame("Connection %d connected to the %s database",
                      connection, dbident);

            if (SQL_CheckRaceTable(g_DbHandle[connection]))
                SQL_CreateRaceTables(g_DbHandle[connection]);
        }

        return g_DbHandle[connection];
    }
}

public SQL_Connected(Handle:owner, Handle:hndl, const String:error[], any:connection)
{
    g_DbHandle[connection] = hndl;
    if (hndl == INVALID_HANDLE || error[0] != '\0')
	{
        LogError("Error connecting to the database:%s",
                 error);
	}
	else
	{
        LogToGame("Connection %d connected to the database",
                  connection);

        if (g_bCreate)
        {
            CreateFactions(hndl);
            CreatePlayerTables(hndl);
        }
        else
            CheckPlayerTable(hndl);
    }
}

CloseDatabase()
{
    for (new i=0; i < sizeof(g_DbHandle); i++)
    {
        if (g_DbHandle[i] != INVALID_HANDLE)
        {
            if (i > 0)
            {
                // (ensure there are no pending transactions)
                SQL_LockDatabase(g_DbHandle[i]);
                SQL_UnlockDatabase(g_DbHandle[i]);
            }

            LogToGame("Closing Database connection %d", i);
            CloseDbHandle(i);
        }
    }
    g_bCreate = false;
    g_bUpdate = false;
}

public Action:CloseDatabaseTimer(Handle:timer,Handle:dbHandle)
{
    if (dbHandle != INVALID_HANDLE)
    {
        // Close the old database handle
        // (ensure there are no pending transactions)
        SQL_LockDatabase(dbHandle);
        SQL_UnlockDatabase(dbHandle);
        CloseHandle(dbHandle);
    }
}

bool:SQL_CheckRaceTable(Handle:dbHandle)
{
    new Handle:query=SQL_Query(dbHandle,"show tables like 'sc_races'");
    if (query)
    {
        if (!SQL_HasResultSet(query) ||
            !SQL_FetchRow(query))
        {
            LogError("sc_races table not found in Database!");
            g_bCreate = true;
        }
        CloseHandle(query);
    }
    return g_bCreate;
}

SQL_CreateRaceTables(Handle:dbHandle)
{
    // Create the sc_races table, if it doesn't exist
    new String:create_sc_factions[][] = {
        "CREATE TABLE IF NOT EXISTS sc_factions",
        "(faction ENUM ('Terran', 'Protoss', 'Zerg', 'HumanAlliance',",
        "               'OrcishHoard', 'NightElf', 'UndeadScourge', 'BurningLegion',",
        "               'Hellbourne', 'TheLegion', 'Sentinel', 'Naga', 'Titan', 'XelNaga'),",
        " long_name varchar(64),",
        " description text,",
        " image varchar(45),",
        " add_date timestamp NOT NULL default current_timestamp,",
        "PRIMARY KEY(faction))",
        "ENGINE=INNODB" };

    LogMessage("Creating Race Tables");

    decl String:SQLString[LONG_QUERY_STRING_LENGTH];
    ImplodeStrings(create_sc_factions, sizeof(create_sc_factions), " ", SQLString, sizeof(SQLString));
    if (!SQL_FastQuery(dbHandle,SQLString))
    {
        decl String:error[ERROR_STRING_LENGTH]; error[0] = '\0';
        SQL_GetError(dbHandle, error, sizeof(error));
        SetFailState("Error in the creation of the sc_factions SQL table:%s", error);
    }
    else
    {
        // Create the sc_races table, if it doesn't exist
        new String:create_sc_races[][] = {
            "CREATE TABLE IF NOT EXISTS sc_races ",
            "(race_ident int not null auto_increment,",
            " race_name varchar(16) not null,",
            " long_name varchar(64),",
            " required_level int,",
            " tech_level int,",
            " parent_name varchar(16),",
            " description text,",
            " image varchar(45),",
            " add_date timestamp default current_timestamp,",
            " type enum ('Biological', 'Mechanical', 'BioMechanical', 'Robotic',",
            "            'Energy', 'Magical', 'Mystical', 'Elemental', 'Undead',",
            "            'Demonic', 'Cybernetic', 'Flyer'),",
            " faction ENUM ('Terran', 'Protoss', 'Zerg', 'HumanAlliance',",
            "               'OrcishHoard', 'NightElf', 'UndeadScourge', 'BurningLegion',",
            "               'Hellbourne', 'TheLegion', 'Sentinel', 'Naga', 'Titan', 'XelNaga'),",
            "PRIMARY KEY(race_ident),",
            "INDEX(type,race_name),",
            "INDEX(race_name))",
            "ENGINE=INNODB" };

        ImplodeStrings(create_sc_races, sizeof(create_sc_races),
                       " ", SQLString, sizeof(SQLString));

        if (!SQL_FastQuery(dbHandle,SQLString))
        {
            decl String:error[ERROR_STRING_LENGTH]; error[0] = '\0';
            SQL_GetError(dbHandle, error, sizeof(error));
            SetFailState("Error in the creation of the sc_races SQL table:%s", error);
        }
        else
        {
            // Create the sc_upgrades table, if it doesn't exist
            new String:create_sc_upgrades[][] = {
                "CREATE TABLE IF NOT EXISTS sc_upgrades",
                "(race_ident int NOT NULL,",
                " upgrade int NOT NULL,",
                " category int,",
                " upgrade_name VARCHAR(16),",
                " long_name VARCHAR(64),",
                " description TEXT,",
                " add_date timestamp NOT NULL default current_timestamp,",
                "PRIMARY KEY (race_ident, upgrade),",
                "FOREIGN KEY (race_ident)",
                "        REFERENCES sc_races(race_ident)",
                "        ON DELETE CASCADE ON UPDATE CASCADE)",
                "ENGINE=INNODB" };

            ImplodeStrings(create_sc_upgrades, sizeof(create_sc_upgrades),
                           " ", SQLString, sizeof(SQLString));

            if (!SQL_FastQuery(dbHandle,SQLString))
            {
                decl String:error[ERROR_STRING_LENGTH]; error[0] = '\0';
                SQL_GetError(dbHandle, error, sizeof(error));
                SetFailState("Error in the creation of the sc_upgrades SQL table:%s", error);
            }
        }
    }
}

CheckPlayerTable(Handle:dbHandle)
{
    SQL_TQuery(dbHandle,SQL_PlayersChecked,"show tables like 'sc_players'", dbHandle);
}

public SQL_PlayersChecked(Handle:owner, Handle:hndl, const String:error[], any:dbHandle)
{
    if (hndl == INVALID_HANDLE || error[0] != '\0')
        LogError("Error checking the sc_players SQL table:%s", error);
    else
    {
        if (!SQL_HasResultSet(hndl) || !SQL_FetchRow(hndl))
        {
            g_bCreate = true;
            LogError("sc_players table not found in Database!");
            CreatePlayerTables(dbHandle);
        }
        CloseHandle(hndl);
    }
}

CreatePlayerTables(Handle:dbHandle)
{
    // Create the sc_players table, if it doesn't exist
    new String:create_sc_players[][] = {
        "CREATE TABLE IF NOT EXISTS sc_players ",
        "(player_ident int not null auto_increment,",
        " steamid varchar(64) not null,",
        " name varchar(64),",
        " race_ident int default 0,",
        " crystals int default 0,",
        " vespene int default 0,",
        " overall_level int default 0,",
        " settings int default 0,",
        " username varchar(25),",
        " last_update timestamp default current_timestamp,",
        " status ENUM('Disabled','Active') NOT NULL DEFAULT 'Active',",
        " del_rec_date DATETIME,",
        "PRIMARY KEY(player_ident),",
        "UNIQUE INDEX (steamid, del_rec_date),",
        "UNIQUE INDEX (username, del_rec_date))",
        "ENGINE=INNODB" };

    LogMessage("Creating Player Tables");

    decl String:SQLString[LONG_QUERY_STRING_LENGTH];
    ImplodeStrings(create_sc_players, sizeof(create_sc_players),
                   " ", SQLString, sizeof(SQLString));

    SQL_TQuery(dbHandle, SQL_PlayersCreated, SQLString, dbHandle);
}

public SQL_PlayersCreated(Handle:owner, Handle:hndl, const String:error[], any:dbHandle)
{
    if (hndl == INVALID_HANDLE || error[0] != '\0')
        LogError("Error in the creation of the sc_players SQL table:%s", error);
    else
    {
        // Create the sc_player_upgrades table, if it doesn't exist
        new String:create_sc_players_alias[][] = {
            "CREATE TABLE IF NOT EXISTS sc_player_alias",
            "(player_ident int not null,",
            " steamid varchar(64),",
            " name varchar(64),",
            " last_used timestamp default current_timestamp,",
            "PRIMARY KEY(player_ident,steamid,name),",
            "FOREIGN KEY (player_ident)",
            "        REFERENCES sc_players(player_ident)",
            "        ON DELETE CASCADE ON UPDATE CASCADE)",
            "ENGINE=INNODB" };

        decl String:SQLString[QUERY_STRING_LENGTH];
        ImplodeStrings(create_sc_players_alias, sizeof(create_sc_players_alias),
                       " ", SQLString, sizeof(SQLString));

        SQL_TQuery(dbHandle, SQL_PlayerAliasCreated, SQLString, dbHandle);
    }
}

public SQL_PlayerAliasCreated(Handle:owner, Handle:hndl, const String:error[], any:dbHandle)
{
    if (hndl == INVALID_HANDLE || error[0] != '\0')
        LogError("Error in the creation of the sc_player_alias SQL table:%s", error);
    else
    {
        // Create the sc_player_races table, if it doesn't exist
        new String:create_sc_players_races[][] = {
            "CREATE TABLE IF NOT EXISTS sc_player_races ",
            "(player_ident int not null,",
            " race_ident int not null,",
            " xp int default 0,",
            " level int default 0,",
            "PRIMARY KEY(player_ident,race_ident),",
            "FOREIGN KEY (player_ident)",
            "        REFERENCES sc_players(player_ident)",
            "        ON DELETE CASCADE ON UPDATE CASCADE,",
            "FOREIGN KEY (race_ident)",
            "        REFERENCES sc_races(race_ident)",
            "        ON DELETE CASCADE ON UPDATE CASCADE) ",
            "ENGINE=INNODB" };

        decl String:SQLString[QUERY_STRING_LENGTH];
        ImplodeStrings(create_sc_players_races, sizeof(create_sc_players_races),
                       " ", SQLString, sizeof(SQLString));

        SQL_TQuery(dbHandle, SQL_PlayerRacesCreated, SQLString, dbHandle);
    }
}

public SQL_PlayerRacesCreated(Handle:owner, Handle:hndl, const String:error[], any:dbHandle)
{
    if (hndl == INVALID_HANDLE || error[0] != '\0')
        LogError("Error in the creation of the sc_player_races SQL table:%s", error);
    else
    {
        // Create the sc_player_upgrades table, if it doesn't exist
        new String:create_sc_players_upgrades[][] = {
            "CREATE TABLE IF NOT EXISTS sc_player_upgrades ",
            "(player_ident int not null,",
            " race_ident int not null,",
            " upgrade int not null,",
            " upgrade_level int default 0,",
            "PRIMARY KEY(player_ident,race_ident,upgrade),",
            "FOREIGN KEY (player_ident)",
            "        REFERENCES sc_players(player_ident)",
            "        ON DELETE CASCADE ON UPDATE CASCADE,",
            "FOREIGN KEY (race_ident)",
            "        REFERENCES sc_races(race_ident)",
            "        ON DELETE CASCADE ON UPDATE CASCADE,",
            "FOREIGN KEY (player_ident,race_ident)",
            "        REFERENCES sc_player_races(player_ident,race_ident)",
            "        ON DELETE CASCADE ON UPDATE CASCADE,",
            "FOREIGN KEY (race_ident,upgrade)",
            "        REFERENCES sc_upgrades(race_ident,upgrade)",
            "        ON DELETE CASCADE ON UPDATE CASCADE)",
            "ENGINE=INNODB" };

        decl String:SQLString[LONG_QUERY_STRING_LENGTH];
        ImplodeStrings(create_sc_players_upgrades, sizeof(create_sc_players_upgrades),
                       " ", SQLString, sizeof(SQLString));

        SQL_TQuery(dbHandle, SQL_PlayerUpgradesCreated, SQLString, dbHandle);
    }
}

public SQL_PlayerUpgradesCreated(Handle:owner, Handle:hndl, const String:error[], any:dbHandle)
{
    if (hndl == INVALID_HANDLE || error[0] != '\0')
        LogError("Error in the creation of the sc_player_upgrades SQL table:%s", error);
    else
    {
        // Create the sc_player_upgrades table, if it doesn't exist
        new String:create_sc_players_tech[][] = {
            "CREATE TABLE IF NOT EXISTS sc_player_tech",
            "(player_ident int not null,",
            " faction ENUM ('Terran', 'Protoss', 'Zerg', 'HumanAlliance',",
            "               'OrcishHoard', 'NightElf', 'UndeadScourge', 'BurningLegion',",
            "               'Hellbourne', 'TheLegion', 'Sentinel', 'Naga', 'Titan', 'XelNaga'),",
            " tech_count int default 0,",
            " tech_level int default 0,",
            "PRIMARY KEY(player_ident,faction),",
            "FOREIGN KEY (player_ident)",
            "        REFERENCES sc_players(player_ident)",
            "        ON DELETE CASCADE ON UPDATE CASCADE,",
            "FOREIGN KEY (faction)",
            "        REFERENCES sc_factions(faction)",
            "        ON DELETE CASCADE ON UPDATE CASCADE)",
            "ENGINE=INNODB" };

        decl String:SQLString[LONG_QUERY_STRING_LENGTH];
        ImplodeStrings(create_sc_players_tech, sizeof(create_sc_players_tech),
                       " ", SQLString, sizeof(SQLString));

        SQL_TQuery(dbHandle, SQL_PlayerTechCreated, SQLString, dbHandle);
    }
}

public SQL_PlayerTechCreated(Handle:owner, Handle:hndl, const String:error[], any:dbHandle)
{
    if (hndl == INVALID_HANDLE || error[0] != '\0')
        LogError("Error in the creation of the sc_player_tech SQL table:%s", error);
    else
    {
        LogMessage("All SourceCraft tables have been verified/created.");
    }
}

CreateFactions(Handle:dbHandle)
{
    for (new Faction:faction = Terran; faction < Faction:sizeof(g_FactionShortNames); faction++)
    {
        decl String:factionName[NAME_STRING_LENGTH];
        GetFactionName(faction, factionName, sizeof(factionName));

        decl String:description[DESCRIPTION_LENGTH];
        GetFactionDescription(faction, description,sizeof(description));

        decl String:image[PLATFORM_MAX_PATH];
        GetFactionImage(faction, image,sizeof(image));

        InsertFaction(dbHandle, faction, factionName, description, image);
    }
}

InsertFaction(Handle:dbHandle, Faction:faction, const String:longName[],
              const String:description[], const String:image[])
{
    TraceInto("DB", "UpdateFaction", "faction=%d, longName=%s, image=%s, description=%s", \
              faction, longName, image, description);

    decl String:escName[NAME_STRING_LENGTH*4+1];
    if (!SQL_EscapeString(dbHandle, longName, escName, sizeof(escName)))
    {
        LogError("Unable to escape %s!", longName);
        strcopy(escName, sizeof(escName), longName);
    }

    decl String:escDesc[DESCRIPTION_LENGTH*4+1];
    if (!SQL_EscapeString(dbHandle, description, escDesc, sizeof(escDesc)))
    {
        LogError("Unable to escape %s!", description);
        strcopy(escDesc, sizeof(escDesc), description);
    }

    decl String:SQLString[QUERY_STRING_LENGTH+(sizeof(escDesc)*2)];
    Format(SQLString,sizeof(SQLString),
            "INSERT INTO sc_factions (faction,long_name,description,image) VALUES (%d,'%s','%s','%s') ON DUPLICATE KEY UPDATE long_name='%s',description='%s',image='%s'", faction, escName, escDesc, image, escName, escDesc, image);

    Trace("query=%s", SQLString);

    SQL_TQuery(dbHandle, SQL_FactionInserted, SQLString, dbHandle);

    TraceReturn();
}

public SQL_FactionInserted(Handle:owner, Handle:hndl, const String:error[], any:dbHandle)
{
    if (hndl == INVALID_HANDLE || error[0] != '\0')
        LogError("Error inserting/updating faction:%s", error);
}

LoadRaceIdent(const String:shortName[])
{
    TraceInto("DB", "LoadRaceIdent", "shortName=%s", shortName);

    new race_ident = -1;
    if (shortName[0])
    {
        if (DatabaseAvailable(DB_MAIN))
        {
            decl String:SQLString[SHORT_QUERY_STRING_LENGTH];
            Format(SQLString,sizeof(SQLString), "SELECT race_ident FROM sc_races WHERE race_name='%s'", shortName);

            new Handle:query=SQL_Query(g_DbHandle[DB_MAIN],SQLString);
            if (query)
            {
                if (SQL_HasResultSet(query) &&
                    SQL_FetchRow(query))
                {
                    race_ident = SQL_FetchInt(query,0);
                }
                CloseHandle(query);
            }
        }
        else
        {
            LogError("Unable to connect to DB to query %s's race ident.", shortName);
        }
    }

    TraceReturn("race_ident=%d", race_ident);
    return race_ident;
}

LoadRaceData(const String:shortName[], &Faction:faction, &UnitType:type)
{
    TraceInto("DB", "LoadRaceData", "shortName=%s", shortName);

    new race_ident = -1;
    if (shortName[0])
    {
        if (DatabaseAvailable(DB_MAIN))
        {
            decl String:SQLString[SHORT_QUERY_STRING_LENGTH];
            Format(SQLString,sizeof(SQLString), "SELECT race_ident,faction,type FROM sc_races WHERE race_name='%s'", shortName);

            new Handle:query=SQL_Query(g_DbHandle[DB_MAIN],SQLString);
            if (query)
            {
                if (SQL_HasResultSet(query) &&
                    SQL_FetchRow(query))
                {
                    race_ident = SQL_FetchInt(query,0);
                    faction = Faction:SQL_FetchInt(query,1);
                    type = UnitType:SQL_FetchInt(query,2);
                }
                CloseHandle(query);
            }
        }
        else
        {
            LogError("Unable to connect to DB to load %s's race data.", shortName);
        }
    }

    TraceReturn("race_ident=%d", race_ident);
    return race_ident;
}

InsertRaceIdent(const String:shortName[], const String:raceName[], required_level,
                tech_level, const String:parentName[], Faction:faction, UnitType:type,
                const String:description[], const String:image[])
{
    TraceInto("DB", "InsertRaceIdent", "shortName=%s, raceName=%s, required_level=%d, tech_level=%d, parentName=%s", \
              shortName, raceName, required_level, tech_level, parentName);

    new race_ident = -1;
    if (DatabaseAvailable(DB_MAIN))
    {
        decl String:escDesc[DESCRIPTION_LENGTH*2+1];
        if (!SQL_EscapeString(g_DbHandle[DB_MAIN], description, escDesc, sizeof(escDesc)))
        {
            LogError("Unable to escape %s!", description);
            strcopy(escDesc, sizeof(escDesc), description);
        }

        decl String:unitType[NAME_STRING_LENGTH];
        GetUnitTypeName(type, unitType, sizeof(unitType));

        decl String:SQLString[sizeof(escDesc)+SHORT_QUERY_STRING_LENGTH];
        Format(SQLString,sizeof(SQLString),
               "INSERT INTO sc_races (race_name,long_name,required_level,tech_level,parent_name,faction,type,description,image,add_date) VALUES ('%s','%s',%d,%d,'%s',%d,'%s','%s','%s',current_timestamp)",
               shortName, raceName, required_level, tech_level, parentName,
               faction, unitType, escDesc, image);

        Trace("query=%d", SQLString);

        if (SQL_FastQuery(g_DbHandle[DB_MAIN],SQLString))
            race_ident = SQL_GetInsertId(g_DbHandle[DB_MAIN]);
        else
        {
            decl String:error[ERROR_STRING_LENGTH]; error[0] = '\0';
            SQL_GetError(g_DbHandle[DB_MAIN], error, sizeof(error));
            LogError("Unable to insert race %s: %s\nSQL:%s",
                     shortName, error, SQLString);
        }
    }
    else
    {
        LogError("Unable to connect to DB to insert %s's race data.", shortName);
    }

    TraceReturn("race_ident=%d", race_ident);
    return race_ident;
}

UpdateRace(race_ident, const String:shortName[], const String:raceName[], required_level,
           tech_level, const String:parentName[], Faction:faction, UnitType:type,
           const String:description[], const String:image[])
{
    TraceInto("DB", "UpdateRace", "race_ident=%d, shortName=%s, raceName=%s, required_level=%d, tech_level=%d, parentName=%s, faction=%d, type=%d", \
              race_ident, shortName, raceName, required_level, tech_level, parentName, faction, type);

    if (DatabaseAvailable(DB_MAIN))
    {
        decl String:unitType[NAME_STRING_LENGTH];
        GetUnitTypeName(type, unitType, sizeof(unitType));

        decl String:escDesc[DESCRIPTION_LENGTH*2+1];
        decl String:SQLString[sizeof(escDesc)+SHORT_QUERY_STRING_LENGTH];
        Format(SQLString,sizeof(SQLString),
               "UPDATE sc_races SET race_name='%s', long_name='%s', required_level=%d, tech_level=%d, parent_name='%s', faction=%d, type='%s'",
               shortName, raceName, required_level, tech_level, parentName, faction, unitType);

        if (description[0] != '\0')
        {
            if (!SQL_EscapeString(g_DbHandle[DB_MAIN], description, escDesc, sizeof(escDesc)))
            {
                LogError("Unable to escape %s!", description);
                strcopy(escDesc, sizeof(escDesc), description);
            }

            Format(SQLString,sizeof(SQLString), "%s, description='%s'", SQLString, escDesc);
        }

        if (image[0] != '\0')
        {
            Format(SQLString,sizeof(SQLString), "%s, image='%s'", SQLString, image);
        }

        Format(SQLString,sizeof(SQLString), "%s WHERE race_ident=%d", SQLString, race_ident);

        Trace("query=%d", SQLString);

        if (!SQL_FastQuery(g_DbHandle[DB_MAIN],SQLString))
        {
            decl String:error[ERROR_STRING_LENGTH]; error[0] = '\0';
            SQL_GetError(g_DbHandle[DB_MAIN], error, sizeof(error));
            LogError("Unable to update race %s: %s\nSQL:%s",
                     shortName, error, SQLString);
        }
    }
    else
    {
        LogError("Unable to connect to DB to update %s's race data.", shortName);
    }

    TraceReturn();
}

bool:InsertUpgrade(race_ident, upgrade, const String:name[], const String:short[], const String:desc[], upgrade_category)
{
    TraceInto("DB", "InsertUpgrade", "race_ident=%d, upgrade=%d, name=%s, short=%s, desc=%s, category=%d", \
              race_ident, upgrade, name, short, desc, upgrade_category);

    TraceDebug("desc=(length=%d,string=%s)", strlen(desc), desc);

    new bool:status = false;

    if (race_ident > 0)
    {
        if (DatabaseAvailable(DB_MAIN))
        {
            decl String:escDesc[DESCRIPTION_LENGTH*2+1];
            if (!SQL_EscapeString(g_DbHandle[DB_MAIN], desc, escDesc, sizeof(escDesc)))
            {
                LogError("Unable to escape %s!", desc);
                strcopy(escDesc, sizeof(escDesc), desc);
            }

            decl String:SQLString[sizeof(escDesc)+SHORT_QUERY_STRING_LENGTH];
            Format(SQLString,sizeof(SQLString),
                   "INSERT INTO sc_upgrades (race_ident,upgrade,category,upgrade_name,long_name,description) VALUES (%d,%d,%d,'%s','%s','%s') ON DUPLICATE KEY UPDATE category=%d,upgrade_name='%s',long_name='%s',description='%s'",
                   race_ident, upgrade, upgrade_category, short, name, escDesc,
                   upgrade_category, short, name, escDesc);

            TraceDebug("query=%s", SQLString);

            status = SQL_FastQuery(g_DbHandle[DB_MAIN],SQLString);
            if (!status)
            {
                decl String:error[ERROR_STRING_LENGTH]; error[0] = '\0';
                SQL_GetError(g_DbHandle[DB_MAIN], error, sizeof(error));
                LogError("Unable to insert/update upgrade %s for race %d: %s\nSQL:%s",
                         short, race_ident, error, SQLString);
            }
        }
        else
        {
            LogError("Unable to connect to DB to insert %d:%s's upgrade data.", race_ident, short);
        }
    }

    TraceReturn("status=%d", status);
    return status;
}

QueryRaceDescription(race_ident, String:description[], maxlength)
{
    TraceInto("DB", "QueryRaceDescription", "race_ident=%s", race_ident);

    if (DatabaseAvailable(DB_MAIN))
    {
        decl String:SQLString[SHORT_QUERY_STRING_LENGTH];
        Format(SQLString,sizeof(SQLString), "SELECT description FROM sc_races WHERE race_ident=%d", race_ident);

        new Handle:query=SQL_Query(g_DbHandle[DB_MAIN],SQLString);
        if (query)
        {
            if (SQL_HasResultSet(query) && SQL_FetchRow(query))
            {
                SQL_FetchString(query, 0, description, maxlength);
            }
            CloseHandle(query);
        }
    }
    else
    {
        LogError("Unable to connect to DB to load race %d's description.", race_ident);
    }

    TraceReturn("description=%s", description);
}

LoadPlayerData(client)
{
    TraceInto("DB", "LoadPlayerData", "client=%d:%L", \
              client, ValidClientIndex(client));

    decl String:steamid[CLASS_STRING_LENGTH];
    if (GetClientAuthString(client,steamid,sizeof(steamid)))
    {
        // Generate the 'other' steamid and check that one also.
        decl String:steamidalt[CLASS_STRING_LENGTH];
        strcopy(steamidalt, sizeof(steamidalt), steamid);
        steamidalt[6] = (steamid[6] == '0') ? '1' : '0';

        // Check if there are any in flight save threads

        new threads;
        if (GetTrieValue(g_SaveThreadsInFlight, steamid, threads) && threads > 0)
            SetTrieValue(g_PlayersToReload, steamid, GetClientUserId(client), true);
        else if (GetTrieValue(g_SaveThreadsInFlight, steamidalt, threads) && threads > 0)
            SetTrieValue(g_PlayersToReload, steamidalt, GetClientUserId(client), true);

        decl String:SQLString[QUERY_STRING_LENGTH];
        Format(SQLString,sizeof(SQLString),
               "SELECT player_ident, status+0, crystals, vespene, overall_level, settings, name, race_ident FROM sc_players WHERE (steamid = '%s' OR steamid = '%s') AND del_rec_date IS NULL",
               steamid, steamidalt);

        if (DatabaseAvailable(DB_THREAD))
        {
            SetDatabaseLoaded(client,DataLoading);
            SQL_TQuery(g_DbHandle[DB_THREAD], SQL_LoadPlayerData,
                       SQLString, GetClientUserId(client));
        }
        else
        {
            SetDatabaseLoaded(client,DataNotLoaded);
            LogError("Unable to connect to DB to load %d:%L's player data\n",
                     client, ValidClientIndex(client));
        }
    }
    else
    {
        SetDatabaseLoaded(client,DataNotLoaded);
        LogError("Unable to obtain steamid to load %d:%L!",
                 client, ValidClientIndex(client));
    }

    TraceReturn();
}

public SQL_LoadPlayerData(Handle:owner, Handle:query, const String:error[], any:userid)
{
    TraceInto("DB", "SQL_LoadPlayerData");

    new client = GetClientOfUserId(userid);

    if (query == INVALID_HANDLE || error[0] != '\0')
    {
        // Retry Access denied or Lost connection errors.
        if (StrContains(error, "denied") >= 0 || StrContains(error, "connection") >= 0)
        {
            LogError("Querying %d:%L's player data failed: %s",
                     client, ValidClientIndex(client), error);

            // Invalidate and close the DB_THREAD handle (in the main thread)
            CreateTimer(0.1, CloseDatabaseTimer, g_DbHandle[DB_THREAD],TIMER_FLAG_NO_MAPCHANGE);
            g_DbHandle[DB_THREAD] = INVALID_HANDLE;

            if (client > 0)
            {
                // Retry loading the client's data
                CreateTimer(0.1, Retry_LoadPlayerData, userid,TIMER_FLAG_NO_MAPCHANGE);
            }
        }
        else
        {
            LogError("Unable to query %d:%L's player data: %s",
                     client, ValidClientIndex(client), error);

            if (client > 0)
            {
                SetDatabaseLoaded(client,DataNotLoaded);

                if (IsClientInGame(client))
                {
                    PrintToConsole(client, "%t", "UnableToLoad", error);
                    CPrintToChat(client, "%t", "UnableToLoad", error);

                    SetHudTextParams(-1.0, 0.8, 5.0, 255, 0, 0, 255, 2, 0.5, 0.0, 0.5);
                    ShowHudText(client, 1, "%t", "UnableToLoad", error);
                }
            }
        }
    }
    else 
    {
        if (client > 0)
        {
            if (SQL_HasResultSet(query) &&
                SQL_FetchRow(query))
            {
                new player_ident = SQL_FetchInt(query,0);
                SetDatabaseIdent(client,player_ident);

                // Check if there are any in flight save threads

                new threads;
                decl String:key[CLASS_STRING_LENGTH];
                Format(key, sizeof(key), "%d", player_ident);
                if (GetTrieValue(g_SaveThreadsInFlight, key, threads) && threads > 0)
                    SetTrieValue(g_PlayersToReload, key, GetClientUserId(client), true);

                new PlayerStatus:player_status = PlayerStatus:SQL_FetchInt(query,1);
                SetPlayerStatus(client,player_status);

                if (player_status != PlayerDisabled)
                {
                    SetCrystals(client,SQL_FetchInt(query,2));
                    SetVespene(client,SQL_FetchInt(query,3));
                    SetOverallLevel(client,SQL_FetchInt(query,4));
                    SetSettingsBits(client,Settings:SQL_FetchInt(query,5));

                    SQL_FetchString(query, 6, playerPrevName[client], sizeof(playerPrevName[]));

                    #if defined _TRACE
                        decl String:name[MAX_NAME_LENGTH];
                        name[0] = '\0';
                        if (GetClientName(client,name,sizeof(name)) &&
                            name[0] == '\0')
                        {
                            Trace("Loading: Unable to retrieve %d:%L's name (DB has %s)", \
                                   client, ValidClientIndex(client), playerPrevName[client]);
                        }
                        else if (!StrEqual(playerPrevName[client], name))
                        {
                            Trace("Loading: %d:%L changed name from what's in the DB(%s) to:%s", \
                                  client, ValidClientIndex(client), playerPrevName[client], name);
                        }
                    #endif

                    Trace("Loaded %d:%L's Player Data; player_ident=%d, name='%s', race=%d, race_ident=%d, overall_level=%d, crystals=%d, vespene=%d", \
                          client, ValidClientIndex(client), player_ident, playerPrevName[client], GetRace(client), GetDatabaseIdent(client), \
                          GetOverallLevel(client), GetCrystals(client), GetVespene(client));

                    // Load the current Race.
                    new race_ident = SQL_FetchInt(query,7);
                    new raceId = FindRaceForIdent(race_ident);
                    if (raceId >= 0)
                    {
                        new Handle:raceHandle = GetRaceHandle(raceId);
                        if (GetRaceRequiredLevel(raceHandle) < 0)
                        {
                            #if defined _TRACE
                                new child = raceId;
                            #endif

                            // If the race is restricted,
                            // Change to the Parent Race instead (if any)
                            decl String:parentName[NAME_STRING_LENGTH];
                            GetRaceParentName(raceHandle, parentName, sizeof(parentName));
                            raceId = FindRace(parentName);

                            #if defined _TRACE
                                Trace("Parent of %d:%L's locked race=%d is %d:%s", \
                                      client, ValidClientIndex(client), \
                                      child, raceId, parentName);
                            #endif
                        }

                        if (raceId >= 0)
                        {
                            if (GetRaceLoaded(client,raceId) < DataOK)
                            {
                                Trace("Load %d:%L's race=%d Data", \
                                      client, ValidClientIndex(client), \
                                      raceId);

                                LoadPlayerRaceData(client, raceId);
                            }

                            Trace("Set %d:%L's race=%d", \
                                  client, ValidClientIndex(client), \
                                  raceId);

                            SetRace(client,raceId);
                        }
                    }
                }

                SetDatabaseLoaded(client,DataLoaded);
                SetDatabaseSaveTime(client, GetGameTime());
                SetDatabaseSaved(client, true);
            }
            else
            {
                SetDatabaseLoaded(client,DataNotFound);

                new raceId = GetRace(client);
                if (raceId >= 0)
                {
                    new Handle:raceInfo = GetRaceInfo(client, raceId);
                    if (raceInfo != INVALID_HANDLE)
                        SetRaceInfoLoaded(raceInfo,DataNotFound);
                }
            }
        }
        CloseHandle(query);
    }

    TraceReturn();
}

public Action:Retry_LoadPlayerData(Handle:timer,any:userid)
{
    // Retry to load the player's data
    new client = GetClientOfUserId(userid);
    if (IsValidClient(client))
        LoadPlayerData(client);
}

LoadPlayerRaceData(client, raceId)
{
    TraceInto("DB", "LoadPlayerRaceData", "client=%d:%L, raceId=%d", \
              client, ValidClientIndex(client), raceId);

    new player_ident = GetDatabaseIdent(client);
    if (player_ident > 0)
    {
        new Handle:raceHandle = GetRaceHandle(raceId);
        new Handle:raceInfo = GetRaceInfo(client, raceId);
        if (raceHandle != INVALID_HANDLE && raceInfo != INVALID_HANDLE)
        {
            new race_ident = GetRaceIdent(raceHandle);
            decl String:SQLString[SHORT_QUERY_STRING_LENGTH];
            Format(SQLString,sizeof(SQLString),
                   "SELECT xp, level FROM sc_player_races WHERE player_ident = %d AND race_ident = %d",
                   player_ident, race_ident);

            if (DatabaseAvailable(DB_THREAD))
            {
                new Handle:dataPack = CreateDataPack();
                WritePackCell(dataPack, GetClientUserId(client));
                WritePackCell(dataPack, raceId);

                SetRaceInfoLoaded(raceInfo,DataLoading);
                SQL_TQuery(g_DbHandle[DB_THREAD], SQL_LoadPlayerRaceData,
                           SQLString, dataPack);
            }
            else
            {
                decl String:raceName[SHORT_STRING_LENGTH];
                GetRaceShortName(raceHandle, raceName, sizeof(raceName));
                SetRaceInfoLoaded(raceInfo,DataNotLoaded);
                LogError("***ERROR*** Database not available, unable to load %d:%L's %d:%s race data, ident=%d\n",
                         client, ValidClientIndex(client), raceId, raceName, player_ident);
            }

            new Faction:techId = GetRaceFaction(raceHandle);
            if (techId > Generic)
            {
                if (GetTechLoaded(client,techId) < DataOK)
                {
                    Trace("Load %d:%L's techId=%d Data", \
                          client, ValidClientIndex(client), \
                          techId);

                    LoadPlayerTechData(client, techId);
                }
            }
        }
    }
    else if (player_ident >= 0)
    {
        decl String:raceName[SHORT_STRING_LENGTH];
        new Handle:raceHandle = GetRaceHandle(raceId);
        GetRaceShortName(raceHandle, raceName, sizeof(raceName));
        LogError("***ERROR*** Unable to obtain ident to load %d:%L's %d:%s race data, ident=%d\n",
                 client, ValidClientIndex(client), raceId, raceName, player_ident);
    }
    else
    {
        #if defined _TRACE
            decl String:raceName[SHORT_STRING_LENGTH];
            new Handle:raceHandle = GetRaceHandle(raceId);
            GetRaceShortName(raceHandle, raceName, sizeof(raceName));
            Trace("Unable to load %d:%L's %d:%s race data!", \
                  client, ValidClientIndex(client), \
                  raceId, raceName);
        #endif
    }

    TraceReturn();
}

public SQL_LoadPlayerRaceData(Handle:owner, Handle:query, const String:error[], any:data)
{
    ResetPack(data);
    new userid = ReadPackCell(data); 
    new client = GetClientOfUserId(userid); 
    new raceId = ReadPackCell(data);
    CloseHandle(data);

    TraceInto("DB", "SQL_LoadPlayerRaceData", "client=%d:%L, raceId=%d", \
              client, ValidClientIndex(client), raceId);

    if (query == INVALID_HANDLE || error[0] != '\0')
    {
        if (client > 0)
        {
            new Handle:raceInfo = GetRaceInfo(client, raceId);
            if (raceInfo != INVALID_HANDLE)
                SetRaceInfoLoaded(raceInfo,DataNotLoaded);
        }

        decl String:raceName[SHORT_STRING_LENGTH];
        new Handle:raceHandle = GetRaceHandle(raceId);
        GetRaceShortName(raceHandle, raceName, sizeof(raceName));

        // Retry Access denied or Lost connection errors.
        if (StrContains(error, "denied") >= 0 || StrContains(error, "connection") >= 0)
        {
            LogError("Querying %d:%L's %d:%s race data failed: %s",
                     client, ValidClientIndex(client),
                     raceId, raceName, error);

            // Invalidate and close the DB_THREAD handle (in the main thread)
            CreateTimer(0.1, CloseDatabaseTimer, g_DbHandle[DB_THREAD],TIMER_FLAG_NO_MAPCHANGE);
            g_DbHandle[DB_THREAD] = INVALID_HANDLE;

            if (client > 0)
            {
                // Retry loading the client's data
                new Handle:dataPack = CreateDataPack();
                WritePackCell(dataPack, userid);
                WritePackCell(dataPack, raceId);
                CreateTimer(0.1, Retry_LoadPlayerRaceData, dataPack,TIMER_FLAG_NO_MAPCHANGE);
            }
        }
        else
        {
            LogError("Unable to query %d:%L's %d:%s race data: %s",
                     client, ValidClientIndex(client),
                     raceId, raceName, error);

            if (IsValidClient(client))
            {
                PrintToConsole(client, "%t", "UnableToLoadRace", raceName, error);
                CPrintToChat(client, "%t", "UnableToLoadRace", raceName, error);

                SetHudTextParams(-1.0, 0.8, 5.0, 255, 0, 0, 255, 2, 0.5, 0.0, 0.5);
                ShowHudText(client, 1, "%t", "UnableToLoadRace", raceName, error);
            }
        }
    }
    else 
    {
        new player_ident = GetDatabaseIdent(client);
        if (client > 0 && player_ident > 0)
        {
            new Handle:raceHandle = GetRaceHandle(raceId);
            new Handle:raceInfo = GetRaceInfo(client, raceId);
            if (raceHandle != INVALID_HANDLE && raceInfo != INVALID_HANDLE)
            {
                if (SQL_HasResultSet(query) &&
                    SQL_FetchRow(query))
                {
                    new xp = SQL_FetchInt(query,0);
                    new level = SQL_FetchInt(query,1);

                    SetRaceInfoXP(raceInfo,xp);
                    SetRaceInfoLevel(raceInfo,level);
                    SetRaceInfoLoaded(raceInfo,DataOK);
                    SetRaceInfoSavedXP(raceInfo,xp);

                    Trace("Load %d:%L's data, race=%d, xp=%d, level=%d", \
                          client, ValidClientIndex(client), \
                          raceId, xp, level);

                    new race_ident = GetRaceIdent(raceHandle);
                    decl String:SQLString[SHORT_QUERY_STRING_LENGTH];
                    Format(SQLString,sizeof(SQLString),
                           "SELECT upgrade, upgrade_level FROM sc_player_upgrades WHERE player_ident = %d AND race_ident = %d",
                           player_ident,race_ident);

                    if (DatabaseConnected(DB_THREAD))
                    {
                        new Handle:dataPack = CreateDataPack();
                        WritePackCell(dataPack, userid);
                        WritePackCell(dataPack, raceId);

                        SQL_TQuery(g_DbHandle[DB_THREAD], SQL_LoadPlayerUpgradeData,
                                   SQLString, dataPack);
                    }
                    else
                    {
                        decl String:raceName[SHORT_STRING_LENGTH];
                        GetRaceShortName(raceHandle, raceName, sizeof(raceName));
                        LogError("***ERROR*** Database not available, unable to load %d:%L's %d:%s upgrade data, ident=%d\n",
                                 client, ValidClientIndex(client), raceId, raceName, player_ident);
                    }
                }
                else
                    SetRaceInfoLoaded(raceInfo,DataNotFound);

                ShowXP(client,false);
                ShowCrystals(client,false);
                ShowVespene(client,false);
                ShowEnergy(client,false);
            }
        }
        CloseHandle(query);
    }

    TraceReturn();
}

public Action:Retry_LoadPlayerRaceData(Handle:timer,Handle:data)
{
    ResetPack(data);
    new client = GetClientOfUserId(ReadPackCell(data)); 
    new raceId = ReadPackCell(data);
    CloseHandle(data);

    // Retry to load the player's race data
    if (IsValidClient(client))
        LoadPlayerRaceData(client, raceId);
}

public SQL_LoadPlayerUpgradeData(Handle:owner, Handle:query, const String:error[], any:data)
{
    ResetPack(data);
    new userid = ReadPackCell(data); 
    new client = GetClientOfUserId(userid); 
    new raceId = ReadPackCell(data);
    CloseHandle(data);

    TraceInto("DB", "SQL_LoadPlayerUpgradeData", "client=%d:%L, raceId=%d", \
              client, ValidClientIndex(client), raceId);

    if (query == INVALID_HANDLE || error[0] != '\0')
    {
        // Invalidate DB_THREAD handle for denied or Lost connection errors.
        if (StrContains(error, "denied") >= 0 || StrContains(error, "connection") >= 0)
        {
            // Invalidate and close the DB_THREAD handle (in the main thread)
            CreateTimer(0.1, CloseDatabaseTimer, g_DbHandle[DB_THREAD],TIMER_FLAG_NO_MAPCHANGE);
            g_DbHandle[DB_THREAD] = INVALID_HANDLE;
        }

        decl String:raceName[SHORT_STRING_LENGTH];
        new Handle:raceHandle = GetRaceHandle(raceId);
        GetRaceShortName(raceHandle, raceName, sizeof(raceName));

        LogError("Unable to query %d:%L's %d:%s upgrade data: %s",
                 client, ValidClientIndex(client),
                 raceId, raceName, error);
    }
    else 
    {
        if (client > 0)
        {
            new Handle:raceHandle = GetRaceHandle(raceId);
            new Handle:raceInfo = GetRaceInfo(client, raceId);
            if (raceHandle != INVALID_HANDLE && raceInfo != INVALID_HANDLE)
            {
                if (SQL_HasResultSet(query))
                {
                    new levelTotal = 0;
                    new upgrade, ulevel;
                    while (SQL_FetchRow(query))
                    {
                        upgrade = SQL_FetchInt(query,0);
                        ulevel  = SQL_FetchInt(query,1);
                        LoadUpgradeLevel(client, raceId, upgrade, ulevel);
                        levelTotal += ulevel;

                        Trace("Load %d:%L's data, race=%d, upgrade=%d, level=%d, total=%d", \
                              client, ValidClientIndex(client), raceId, upgrade, ulevel, levelTotal);
                    }
                }
                SetRaceInfoLoaded(raceInfo,DataLoaded);
            }
        }
        CloseHandle(query);
    }

    TraceReturn();
}

LoadPlayerTechData(client, Faction:techId)
{
    TraceInto("DB", "LoadPlayerTechData", "client=%d:%L, techId=%d", \
              client, ValidClientIndex(client), techId);

    new player_ident = GetDatabaseIdent(client);
    if (player_ident > 0)
    {
        new Handle:techData = GetTechData(client);
        if (techData != INVALID_HANDLE)
        {
            new Handle:techInfo = GetArrayCell(techData,_:techId);
            if (techInfo != INVALID_HANDLE)
            {
                decl String:SQLString[SHORT_QUERY_STRING_LENGTH];
                Format(SQLString,sizeof(SQLString),
                       "SELECT tech_count, tech_level FROM sc_player_tech WHERE player_ident = %d AND faction = %d",
                       player_ident, techId);

                if (DatabaseAvailable(DB_THREAD))
                {
                    new Handle:dataPack = CreateDataPack();
                    WritePackCell(dataPack, GetClientUserId(client));
                    WritePackCell(dataPack, _:techId);

                    SetTechInfoLoaded(techInfo,DataLoading);
                    SQL_TQuery(g_DbHandle[DB_THREAD], SQL_LoadPlayerTechData,
                               SQLString, dataPack);
                }
                else
                {
                    SetTechInfoLoaded(techInfo,DataNotLoaded);

                    decl String:techName[NAME_STRING_LENGTH];
                    GetFactionName(techId, techName, sizeof(techName));
                    LogError("***ERROR*** Database not available, unable to load %d:%L's %d:%s tech data, ident=%d\n",
                             client, ValidClientIndex(client), techId, techName, player_ident);
                }

            }
        }
    }
    else if (player_ident >= 0)
    {
        decl String:techName[NAME_STRING_LENGTH];
        GetFactionName(techId, techName, sizeof(techName));
        LogError("***ERROR*** Unable to obtain ident to load %d:%L's %d:%s tech data, ident=%d\n",
                 client, ValidClientIndex(client), techId, techName, player_ident);
    }
    else
    {
        decl String:techName[NAME_STRING_LENGTH];
        GetFactionName(techId, techName, sizeof(techName));
        Trace("Unable to load %d:%L's %d:%s tech data!", \
              client, ValidClientIndex(client), techId, techName);
    }

    TraceReturn();
}

public SQL_LoadPlayerTechData(Handle:owner, Handle:query, const String:error[], any:data)
{
    TraceInto("DB", "SQL_LoadPlayerTechData");

    ResetPack(data);
    new userid = ReadPackCell(data); 
    new client = GetClientOfUserId(userid); 
    new Faction:techId = Faction:ReadPackCell(data);
    CloseHandle(data);

    if (query == INVALID_HANDLE || error[0] != '\0')
    {
        // Invalidate DB_THREAD handle for denied or Lost connection errors.
        if (StrContains(error, "denied") >= 0 || StrContains(error, "connection") >= 0)
        {
            // Invalidate and close the DB_THREAD handle (in the main thread)
            CreateTimer(0.1, CloseDatabaseTimer, g_DbHandle[DB_THREAD],TIMER_FLAG_NO_MAPCHANGE);
            g_DbHandle[DB_THREAD] = INVALID_HANDLE;
        }

        decl String:techName[NAME_STRING_LENGTH];
        GetFactionName(techId, techName, sizeof(techName));
        LogError("Unable to query %d:%L's %d:%s tech data: %s",
                 client, ValidClientIndex(client), techId, techName, error);

        if (client > 0)
        {
            new Handle:techData = GetTechData(client);
            new Handle:techInfo = GetArrayCell(techData,_:techId);
            SetTechInfoLoaded(techInfo,DataNotLoaded);
        }
    }
    else 
    {
        if (client > 0)
        {
            new Handle:techData = GetTechData(client);
            if (techData != INVALID_HANDLE)
            {
                new Handle:techInfo = GetArrayCell(techData,_:techId);
                if (techInfo != INVALID_HANDLE)
                {
                    if (SQL_HasResultSet(query) &&
                        SQL_FetchRow(query))
                    {
                        new count = SQL_FetchInt(query,0);
                        new level = SQL_FetchInt(query,1);

                        SetTechInfoCount(techInfo,count);
                        SetTechInfoLevel(techInfo,level);
                        SetTechInfoLoaded(techInfo,DataLoaded);

                        decl String:techName[NAME_STRING_LENGTH];
                        GetFactionName(techId, techName, sizeof(techName));
                        Trace("Loaded %d:%L's %d:%s tech data, count=%d, level=%d", \
                              client,ValidClientIndex(client), techId, techName, count, level);
                    }
                    else
                        SetTechInfoLoaded(techInfo,DataOK);
                }
            }
        }
        CloseHandle(query);
    }

    TraceReturn();
}

SavePlayerData(client)
{
    TraceInto("DB", "SavePlayerData", "client=%d:%L", \
              client, ValidClientIndex(client));

    new bool:threaded = true;
    /*
    new timeleft;
    if (g_MapChanging)
    {
        // Don't use threaded saves when the map is changing!
        threaded = false;
    }
    else if (GetMapTimeLeft(timeleft))
    {
        // or the map is about to end
        threaded = (timeleft > 10);
    }
    else
        threaded = true;
    */

    new player_ident = GetDatabaseIdent(client);
    if (player_ident > 0)
    {
        if (DatabaseAvailable(DB_MAIN))
        {
            new race              = GetRace(client);
            new Handle:raceHandle = GetRaceHandle(race);
            new race_ident        = GetRaceIdent(raceHandle);
            new Settings:bits     = GetSettingsBits(client);
            new overall_level     = GetOverallLevel(client);
            new crystals          = GetCrystals(client);
            new vespene           = GetVespene(client);

            Trace("Save %d:%L's data, race=%d, race_ident=%d, overall_level=%d", \
                  client, ValidClientIndex(client), race, race_ident, overall_level);

            decl String:key[CLASS_STRING_LENGTH];
            decl String:steamid[CLASS_STRING_LENGTH];
            if (IsClientConnected(client) &&
                GetClientAuthString(client,steamid,sizeof(steamid)))
            {
                strcopy(key, sizeof(key), steamid);
            }
            else                
            {
                Format(key, sizeof(key), "%d", player_ident);
                steamid[0] = '\0';
            }

            // Start the thread count at 0
            SetTrieValue(g_SaveThreadsInFlight, key, 0, true);

            decl String:name[MAX_NAME_LENGTH];
            decl String:escName[sizeof(name)*2+1];

            name[0] = escName[0] = '\0';
            if (IsClientInGame(client))
                GetClientName(client,name,sizeof(name));
            else
                strcopy(name, sizeof(name), playerName[client]);

            if (name[0] == '\0')
            {
                strcopy(name, sizeof(name), playerName[client]);
                if (name[0] == '\0')
                    strcopy(name, sizeof(name), playerPrevName[client]);
            }

            if (!SQL_EscapeString(g_DbHandle[DB_MAIN], name, escName, sizeof(escName)))
            {
                LogError("Unable to escape %s!", name);
                strcopy(escName, sizeof(escName), name);
            }

            decl String:SQLString[QUERY_STRING_LENGTH];
            Format(SQLString,sizeof(SQLString), "UPDATE sc_players SET race_ident=%d, crystals=%d, vespene=%d, overall_level=%d, settings=%d, name='%s', last_update=current_timestamp WHERE player_ident = %d", race_ident, crystals, vespene, overall_level, bits, escName, player_ident);

            Trace("Saving %d:%L's Player Data; player_ident=%d, name='%s', race=%d, race_ident=%d, overall_level=%d, crystals=%d, vespene=%d", client, ValidClientIndex(client), player_ident, name, race, race_ident, overall_level, crystals, vespene);

            if (threaded && DatabaseAvailable(DB_THREAD))
            {
                // Start threaded SQL update to update player table
                new Handle:dataPack = CreateDataPack();
                WritePackCell(dataPack, GetClientUserId(client));
                WritePackCell(dataPack, player_ident);
                WritePackCell(dataPack, crystals);
                WritePackCell(dataPack, vespene);
                WritePackString(dataPack, key);

                UpdateThreadsInFlight(key, 1);
                SQL_TQuery(g_DbHandle[DB_THREAD], SQL_UpdatePlayer, SQLString, dataPack);
            }
            else if (DatabaseAvailable(DB_MAIN))
            {
                if (!SQL_FastQuery(g_DbHandle[DB_MAIN],SQLString))
                {
                    new bool:success = false;
                    decl String:error[ERROR_STRING_LENGTH]; error[0] = '\0';
                    SQL_GetError(g_DbHandle[DB_MAIN], error, sizeof(error));

                    // Retry Access denied or Lost connection errors.
                    if (StrContains(error, "denied") >= 0 || StrContains(error, "connection") >= 0)
                    {
                        LogError("Initial attempt to update %d:%L's (ident=%d) player data failed: %s\nSQL:%s",
                                 client, ValidClientIndex(client), player_ident, error, SQLString);

                        CloseDbHandle(DB_MAIN);
                        if (DatabaseAvailable(DB_MAIN))
                        {
                            success = SQL_FastQuery(g_DbHandle[DB_MAIN],SQLString);
                            if (success)
                            {
                                error[0] = '\0';
                                LogError("Second attempt to update %d:%L's (ident=%d) player data succeeded!",
                                         client, ValidClientIndex(client), player_ident);
                            }
                            else
                                SQL_GetError(g_DbHandle[DB_MAIN], error, sizeof(error));
                        }
                    }

                    if (!success)
                    {
                        SetDatabaseSaved(client, false);
                        LogError("Unable to update %d:%L's (ident=%d) player data (crystals=%d,vespene=%d): %s\nSQL:%s",
                                 client, ValidClientIndex(client), player_ident, crystals, vespene, error, SQLString);

                        TraceReturn();
                        return;
                    }
                }
            }
            else
            {
                LogError("Unable to connect to DB to update %d:%L's (ident=%d) player data",
                        client, ValidClientIndex(client), player_ident);

                TraceReturn();
                return;
            }

            if (race > 0)
            {
                Trace("Save %d:%L's race data, race=%d Data", \
                      client, ValidClientIndex(client), race);

                if (SavePlayerRaceData(client, player_ident, race, threaded, key))
                {
                    SetDatabaseSaveTime(client, GetGameTime());
                    SetDatabaseSaved(client, true);
                }
            }
            else
            {
                SetDatabaseSaveTime(client, GetGameTime());
                SetDatabaseSaved(client, true);
            }

            // Check the thread count and cleanup
            // the entry if there aren't any
            UpdateThreadsInFlight(key, 0);

            if (!StrEqual(playerPrevName[client], name))
            {
                Format(SQLString,sizeof(SQLString), "INSERT INTO sc_player_alias (player_ident,steamid,name,last_used) VALUES (%d,'%s','%s',current_timestamp) ON DUPLICATE KEY UPDATE last_used=current_timestamp", player_ident, steamid, escName);
                if (threaded && DatabaseAvailable(DB_THREAD))
                {
                    new Handle:dataPack = CreateDataPack();
                    WritePackCell(dataPack, GetClientUserId(client));
                    WritePackCell(dataPack, player_ident);

                    // Start threaded SQL update to update alias table
                    SQL_TQuery(g_DbHandle[DB_THREAD], SQL_UpdateAlias, SQLString, dataPack);

                    // Assume it worked.
                    strcopy(playerPrevName[client], sizeof(playerPrevName[]), name);
                }
                else if (DatabaseAvailable(DB_MAIN))
                {
                    if (!SQL_FastQuery(g_DbHandle[DB_MAIN],SQLString))
                    {
                        new bool:success = false;
                        decl String:error[ERROR_STRING_LENGTH]; error[0] = '\0';
                        SQL_GetError(g_DbHandle[DB_MAIN], error, sizeof(error));

                        // Retry Access denied or Lost connection errors.
                        if (StrContains(error, "denied") >= 0 || StrContains(error, "connection") >= 0)
                        {
                            LogError("Initial attempt to update %d:%L's (ident=%d) alias data failed: %s\nSQL:%s",
                                     client, ValidClientIndex(client), player_ident, error, SQLString);

                            CloseDbHandle(DB_MAIN);
                            if (DatabaseAvailable(DB_MAIN))
                            {
                                success = SQL_FastQuery(g_DbHandle[DB_MAIN],SQLString);
                                if (success)
                                {
                                    error[0] = '\0';
                                    LogError("Second attempt to update %d:%L's (ident=%d) alias data succeeded!",
                                             client, ValidClientIndex(client), player_ident);
                                }
                                else
                                    SQL_GetError(g_DbHandle[DB_MAIN], error, sizeof(error));
                            }
                        }
                        else if (StrContains(error, "constraint") >= 0)
                        {
                            if (StrContains(error, "player_ident") >= 0)
                            {
                                // Invalidate player_ident for constraint failed errors.
                                SetDatabaseIdent(client,0);
                            }
                        }

                        if (!success)
                        {
                            LogError("Unable to update %d:%L's (ident=%d) alias data: %s\nSQL:%s",
                                     client, ValidClientIndex(client), player_ident, error, SQLString);
                        }
                    }
                    else
                    {
                        strcopy(playerPrevName[client], sizeof(playerPrevName[]), name);
                    }
                }
                else
                {
                    LogError("Unable to connect to DB to update %d:%L's (ident=%d) alias data.",
                             client, ValidClientIndex(client), player_ident);
                }
            }
        }
        else
        {
            LogError("Unable to connect to DB to update %d:%L's (ident=%d) player data.",
                     client, ValidClientIndex(client), player_ident);
        }
    }
    else
        InsertPlayerData(client, threaded);

    TraceReturn();
}

public SQL_UpdatePlayer(Handle:owner, Handle:hndl, const String:error[], any:data)
{
    TraceInto("DB", "SQL_UpdatePlayer");

    if (data == INVALID_HANDLE)
        LogError("Invalid Datapack Passed to SQL_UpdatePlayer!");
    else 
    {
        ResetPack(data);
        new userid = ReadPackCell(data); 
        new client = GetClientOfUserId(userid);
        new player_ident = ReadPackCell(data);
        new crystals = ReadPackCell(data);
        new vespene = ReadPackCell(data);

        decl String:key[CLASS_STRING_LENGTH];
        ReadPackString(data, key, sizeof(key));

        if (hndl != INVALID_HANDLE && error[0] == '\0')
            UpdateThreadsInFlight(key, -1);
        else
        {
            if (client > 0)
                SetDatabaseSaved(client, false);

            // Retry Access denied or Lost connection errors.
            if (StrContains(error, "denied") >= 0 || StrContains(error, "connection") >= 0)
            {
                // Invalidate and close the DB_THREAD handle (in the main thread)
                CreateTimer(0.1, CloseDatabaseTimer, g_DbHandle[DB_THREAD],TIMER_FLAG_NO_MAPCHANGE);
                g_DbHandle[DB_THREAD] = INVALID_HANDLE;

                if (client > 0)
                {
                    // Retry saving the client's data
                    CreateTimer(0.1, Retry_SavePlayerData, userid,TIMER_FLAG_NO_MAPCHANGE);
                }
            }
            else
            {
                RemoveFromTrie(g_SaveThreadsInFlight, key);
                LogError("Unable to update %d:%L's (ident=%d) player data (crystals=%d,vespene=%d,key=%s): %s",
                         client, ValidClientIndex(client), player_ident, crystals, vespene, key, error);

                if (IsValidClient(client))
                {
                    PrintToConsole(client, "%t", "UnableToSave", error);
                    CPrintToChat(client, "%t", "UnableToSave", error);

                    SetHudTextParams(-1.0, 0.8, 5.0, 255, 0, 0, 255, 2, 0.5, 0.0, 0.5);
                    ShowHudText(client, 1, "%t", "UnableToSave", error);
                }
            }
        }
        CloseHandle(data);
    }

    TraceReturn();
}

public Action:Retry_SavePlayerData(Handle:timer,any:userid)
{
    // Retry to save the player's data
    new client = GetClientOfUserId(userid);
    if (IsValidClient(client))
        SavePlayerData(client);
}

public SQL_UpdateAlias(Handle:owner, Handle:hndl, const String:error[], any:data)
{
    TraceInto("DB", "SQL_UpdateAlias");

    if (data == INVALID_HANDLE)
        LogError("Invalid Datapack Passed to SQL_UpdateAlias!");
    else 
    {
        if (hndl == INVALID_HANDLE || error[0] != '\0')
        {
            // Invalidate DB_THREAD handle for denied or Lost connection errors.
            if (StrContains(error, "denied") >= 0 || StrContains(error, "connection") >= 0)
            {
                // Invalidate and close the DB_THREAD handle (in the main thread)
                CreateTimer(0.1, CloseDatabaseTimer, g_DbHandle[DB_THREAD],TIMER_FLAG_NO_MAPCHANGE);
                g_DbHandle[DB_THREAD] = INVALID_HANDLE;
            }

            ResetPack(data);
            new userid = ReadPackCell(data); 
            new client = GetClientOfUserId(userid);
            new player_ident = ReadPackCell(data);

            LogError("Unable to update %d:%L's (ident=%d) alias data: %s",
                     client, ValidClientIndex(client), player_ident, error);
        }
        CloseHandle(data);
    }

    TraceReturn();
}

InsertPlayerData(client, bool:threaded)
{
    TraceInto("DB", "InsertPlayerData", "client=%d:%L, threaded=%d", \
              client, ValidClientIndex(client), threaded);

    decl String:steamid[ID_STRING_LENGTH];
    if (GetClientAuthString(client,steamid,sizeof(steamid)))
    {
        if (DatabaseAvailable(DB_MAIN))
        {
            new race              = GetRace(client);
            new Handle:raceHandle = GetRaceHandle(race);
            new race_ident        = GetRaceIdent(raceHandle);
            new Settings:bits     = GetSettingsBits(client);
            new overall_level     = GetOverallLevel(client);
            new crystals          = GetCrystals(client);
            new vespene           = GetVespene(client);

            decl String:name[MAX_NAME_LENGTH];
            decl String:escName[sizeof(name)*2+1];

            name[0] = escName[0] = '\0';
            if (IsClientInGame(client))
            {
                GetClientName(client,name,sizeof(name));
                if (!SQL_EscapeString(g_DbHandle[DB_MAIN], name, escName, sizeof(escName)))
                {
                    LogError("Unable to escape %s!", name);
                    strcopy(escName, sizeof(escName), name);
                }
            }

            Trace("Insert %d:%L's data, race=%d, race_ident=%d, overall_level=%d", \
                  client, ValidClientIndex(client), race, race_ident, overall_level);

            decl String:SQLString[QUERY_STRING_LENGTH];
            Format(SQLString,sizeof(SQLString), "INSERT INTO sc_players (steamid,name,race_ident,crystals,vespene,overall_level,settings,last_update) VALUES ('%s','%s',%d,%d,%d,%d,%d,current_timestamp) ON DUPLICATE KEY UPDATE race_ident=%d, crystals=%d, vespene=%d, overall_level=%d, settings=%d, last_update=current_timestamp", steamid, escName, race_ident, crystals, vespene, overall_level, bits, race_ident, crystals, vespene, overall_level, bits);

            Trace("Inserting %d:%L's Player Data; steamid=%s, name='%s', race=%d, race_ident=%d, overall_level=%d, crystals=%d, vespene=%d", client, ValidClientIndex(client), steamid, name, race, race_ident, overall_level, crystals, vespene);

            new bool:success = SQL_FastQuery(g_DbHandle[DB_MAIN],SQLString);
            if (!success)
            {
                decl String:error[ERROR_STRING_LENGTH]; error[0] = '\0';
                SQL_GetError(g_DbHandle[DB_MAIN], error, sizeof(error));

                // Retry Access denied or Lost connection errors.
                if (StrContains(error, "denied") >= 0 || StrContains(error, "connection") >= 0)
                {
                    LogError("Initial attempt to insert/update %d:%L's player data failed: %s\nSQL:%s",
                             client, ValidClientIndex(client), error, SQLString);

                    CloseDbHandle(DB_MAIN);
                    if (DatabaseAvailable(DB_MAIN))
                    {
                        success = SQL_FastQuery(g_DbHandle[DB_MAIN],SQLString);
                        if (success)
                        {
                            error[0] = '\0';
                            LogError("Second attempt to insert/update %d:%L's player data succeeded!",
                                     client, ValidClientIndex(client));
                        }
                        else
                            SQL_GetError(g_DbHandle[DB_MAIN], error, sizeof(error));
                    }
                }
            }

            if (success)
            {
                new player_ident = SQL_GetInsertId(g_DbHandle[DB_MAIN]);
                SetDatabaseIdent(client,player_ident);

                if (player_ident > 0 && race > 0)
                {
                    Trace("Save %d:%L's race=%d data", \
                          client, ValidClientIndex(client), \
                          race);

                    if (SavePlayerRaceData(client, player_ident, race, threaded, ""))
                    {
                        SetDatabaseSaveTime(client, GetGameTime());
                        SetDatabaseSaved(client, true);
                    }
                }
            }
            else
            {
                decl String:error[ERROR_STRING_LENGTH]; error[0] = '\0';
                SQL_GetError(g_DbHandle[DB_MAIN], error, sizeof(error));
                LogError("Unable to insert/update %d:%L's player data (crystals=%d,vespene=%d): %s\nSQL:%s",
                         client, ValidClientIndex(client), crystals, vespene, error, SQLString);

                Format(SQLString,sizeof(SQLString),
                       "SELECT player_ident FROM sc_players WHERE steamid = '%s'",
                       steamid);

                new Handle:ident_query=SQL_Query(g_DbHandle[DB_MAIN],SQLString);
                if (ident_query)
                {
                    if (SQL_HasResultSet(ident_query) &&
                        SQL_FetchRow(ident_query))
                    {
                        new player_ident = SQL_FetchInt(ident_query,0);
                        if (player_ident > 0)
                        {
                            Trace("%d:%L's ident=%d", \
                                  client, ValidClientIndex(client), \
                                  player_ident);

                            SetDatabaseIdent(client, player_ident);
                            SavePlayerData(client);
                        }
                        else
                        {
                            SetDatabaseSaved(client, false);
                            LogError("Unable to save %d:%L's data, ident is invalid (%d)!",
                                     client,ValidClientIndex(client), player_ident);
                        }
                    }
                    else
                    {
                        SetDatabaseSaved(client, false);
                        SQL_GetError(g_DbHandle[DB_MAIN], error, sizeof(error));
                        LogError("Unable to query %d:%L's ident: %s\nSQL:%s",
                                  client, ValidClientIndex(client), error, SQLString);
                    }
                    CloseHandle(ident_query);
                }
                else
                {
                    SetDatabaseSaved(client, false);
                    SQL_GetError(g_DbHandle[DB_MAIN], error, sizeof(error));
                    LogError("Unable to query %d:%L's ident: %s\nSQL:%s",
                            client, ValidClientIndex(client), error, SQLString);
                }
            }
        }
        else
        {
            LogError("Unable to connect to DB to insert %d:%L's player data\n",
                     client, ValidClientIndex(client));
        }
    }
    else
    {
        LogError("Unable to obtain steamid when saving %d:%L",
                 client, ValidClientIndex(client));
    }

    TraceReturn();
}

bool:SavePlayerRaceData(client, player_ident, race, bool:threaded, const String:key[])
{
    TraceInto("DB", "SavePlayerRaceData", \
              "client=%d:%L, player_ident=%d, race=%d, threaded=%d, key=%s", \
              client, ValidClientIndex(client), player_ident, race, threaded, key);

    new bool:success    = false;           
    new Handle:raceInfo = GetRaceInfo(client, race);
    if (raceInfo != INVALID_HANDLE)
    {
        new Handle:raceHandle = GetRaceHandle(race);
        new race_ident        = GetRaceIdent(raceHandle);

        new level = GetRaceInfoLevel(raceInfo);
        new xp    = GetRaceInfoXP(raceInfo);

        Trace("%d:%L Save Race Data, race=%d, raceInfo=0x%08x, race_ident=%d, level=%d, xp=%d", \
              client, ValidClientIndex(client), race, raceInfo, race_ident, level, xp);

        new String:SQLString[QUERY_STRING_LENGTH];
        Format(SQLString,sizeof(SQLString), "INSERT INTO sc_player_races (player_ident,race_ident,xp,level) VALUES (%d,%d,%d,%d) ON DUPLICATE KEY UPDATE xp=%d,level=%d", player_ident, race_ident, xp, level, xp, level);

        if (threaded && DatabaseAvailable(DB_THREAD))
        //  && GetRaceInfoLoaded(raceInfo) >= DataLoaded)
        {
            // Only save upgrades by calling ThreadedSavePlayerUpgrades()
            // if the player had levels in this race before
            // (and the sc_player_races entry already exists)
            // Otherwise, attempt to save them AFTER saving
            // the player's race's data has completed!
            new bool:saveUpgrades = (GetRaceInfoLoaded(raceInfo) >= DataLoaded);

            new Handle:dataPack = CreateDataPack();
            WritePackCell(dataPack, GetClientUserId(client));
            WritePackCell(dataPack, player_ident);
            WritePackCell(dataPack, race);
            WritePackCell(dataPack, race_ident);
            WritePackCell(dataPack, xp);
            WritePackCell(dataPack, level);
            WritePackCell(dataPack, !saveUpgrades);
            WritePackString(dataPack, key);

            UpdateThreadsInFlight(key, 1);
            SQL_TQuery(g_DbHandle[DB_THREAD], SQL_SavePlayerRaceData, SQLString, dataPack);

            if (saveUpgrades)
            {
                ThreadedSavePlayerUpgrades(client, player_ident, race_ident,
                                           race, raceInfo, raceHandle, key);
            }

            SetRaceInfoSavedXP(raceInfo,xp);
            success = true; // Assume success when threaded.
        }
        else if (DatabaseAvailable(DB_MAIN))
        {
            decl String:error[ERROR_STRING_LENGTH]; error[0] = '\0';
            decl String:raceName[SHORT_STRING_LENGTH]; raceName[0] = '\0';
            success = SQL_FastQuery(g_DbHandle[DB_MAIN],SQLString);
            if (!success)
            {
                GetRaceShortName(raceHandle, raceName, sizeof(raceName));
                SQL_GetError(g_DbHandle[DB_MAIN], error, sizeof(error));

                // Retry Access denied or Lost connection errors.
                if (StrContains(error, "denied") >= 0 || StrContains(error, "connection") >= 0)
                {
                    LogError("Initial attempt to insert/update %d:%L's (ident=%d) %d:%s race data failed: %s\nSQL:%s",
                             client, ValidClientIndex(client), player_ident, race_ident, raceName, error, SQLString);

                    CloseDbHandle(DB_MAIN);
                    if (DatabaseAvailable(DB_MAIN))
                    {
                        success = SQL_FastQuery(g_DbHandle[DB_MAIN],SQLString);
                        if (success)
                        {
                            error[0] = '\0';
                            LogError("Second attempt to insert/update %d:%L's (ident=%d) %d:%s race data succeeded!",
                                     client, ValidClientIndex(client), player_ident, race_ident, raceName);
                        }
                        else
                            SQL_GetError(g_DbHandle[DB_MAIN], error, sizeof(error));
                    }
                }
                else if (StrContains(error, "constraint") >= 0)
                {
                    if (StrContains(error, "player_ident") >= 0)
                    {
                        // Invalidate player_ident for constraint failed errors.
                        SetDatabaseIdent(client,0);
                    }
                }
            }

            if (success)
            {
                SetRaceInfoSavedXP(raceInfo,xp);
                if (threaded && DatabaseAvailable(DB_THREAD))
                {
                    ThreadedSavePlayerUpgrades(client, player_ident, race_ident,
                                               race, raceInfo, raceHandle, key);

                    success = true; // Assume success when threaded.
                }
                else
                {
                    success = SavePlayerUpgrades(client, player_ident, race_ident,
                                                 race, raceInfo, raceHandle, key);
                }
            }
            else
            {
                SetDatabaseSaved(client, false);
                LogError("Unable to insert/update %d:%L's (ident=%d) %d:%s race data (xp=%d,level=%d): %s\nSQL:%s",
                         client, ValidClientIndex(client), player_ident, race_ident,
                         raceName, xp, level, error, SQLString);
            }
        }
        else
        {
            decl String:raceName[SHORT_STRING_LENGTH];
            GetRaceShortName(raceHandle, raceName, sizeof(raceName));
            LogError("Unable to connect to DB to insert/update %d:%L's (ident=%d) %d:%s race data\n",
                     client, ValidClientIndex(client), player_ident, race_ident, raceName);

            success = false;
        }

        new Faction:techId = GetRaceFaction(raceHandle);
        if (techId > Generic)
        {
            success &= SavePlayerTechData(client, player_ident,
                                          techId, threaded, key);
        }
    }
    else
    {
        LogError("Unable to get %d:%L's %d race data",
                 client, ValidClientIndex(client), race);
    }

    TraceReturn("success=%d", success);
    return success;
}

public SQL_SavePlayerRaceData(Handle:owner, Handle:hndl, const String:error[], any:data)
{
    TraceInto("DB", "SQL_SavePlayerRaceData");

    if (data == INVALID_HANDLE)
        LogError("Invalid Datapack Passed to SQL_SavePlayerRaceData!");
    else 
    {
        ResetPack(data);
        new userid            = ReadPackCell(data); 
        new client            = GetClientOfUserId(userid);
        new player_ident      = ReadPackCell(data); 
        new race              = ReadPackCell(data); 
        new race_ident        = ReadPackCell(data);
        new xp                = ReadPackCell(data);
        new level             = ReadPackCell(data);
        new bool:saveUpgrades = bool:ReadPackCell(data);

        decl String:key[CLASS_STRING_LENGTH];
        ReadPackString(data, key, sizeof(key));

        if (hndl != INVALID_HANDLE && error[0] == '\0')
        {
            UpdateThreadsInFlight(key, -1);

            if (saveUpgrades && IsValidClient(client))
                CreateTimer(0.1, Retry_SavePlayerData, userid,TIMER_FLAG_NO_MAPCHANGE);
        }
        else
        {
            new bool:clientIsValid = IsValidClient(client);
            if (clientIsValid)
            {
                SetSavedXP(client,race,0);
                SetDatabaseSaved(client, false);
            }

            // Invalidate DB_THREAD handle for denied or Lost connection errors.
            if (StrContains(error, "denied") >= 0 || StrContains(error, "connection") >= 0)
            {
                // Invalidate and close the DB_THREAD handle (in the main thread)
                CreateTimer(0.1, CloseDatabaseTimer, g_DbHandle[DB_THREAD],TIMER_FLAG_NO_MAPCHANGE);
                g_DbHandle[DB_THREAD] = INVALID_HANDLE;

                if (clientIsValid)
                {
                    // Retry saving the client's race data
                    new Handle:dataPack = CreateDataPack();
                    WritePackCell(dataPack, userid);
                    WritePackCell(dataPack, player_ident);
                    WritePackCell(dataPack, race);
                    WritePackString(dataPack, key);
                    CreateTimer(0.1, Retry_SavePlayerRaceData, dataPack,TIMER_FLAG_NO_MAPCHANGE);
                }
            }
            else
            {
                RemoveFromTrie(g_SaveThreadsInFlight, key);

                decl String:raceName[SHORT_STRING_LENGTH];
                GetRaceShortName(GetRaceHandle(race), raceName, sizeof(raceName));
                LogError("Unable to insert/update %d:%L's (ident=%d) %d:%s race data (xp=%d,level=%d,key=%s): %s",
                         client, ValidClientIndex(client), player_ident, race_ident, raceName, xp, level, key, error);

                if (clientIsValid)
                {
                    if (StrContains(error, "constraint") >= 0)
                    {
                        if (StrContains(error, "player_ident") >= 0)
                        {
                            // Invalidate player_ident for constraint failed errors.
                            SetDatabaseIdent(client,0);
                        }
                    }

                    PrintToConsole(client, "%t", "UnableToSaveRace", raceName, error);
                    CPrintToChat(client, "%t", "UnableToSaveRace", raceName, error);

                    SetHudTextParams(-1.0, 0.8, 5.0, 255, 0, 0, 255, 2, 0.5, 0.0, 0.5);
                    ShowHudText(client, 1, "%t", "UnableToSaveRace", raceName, error);
                }
            }
        }
        CloseHandle(data);
    }

    TraceReturn();
}

public Action:Retry_SavePlayerRaceData(Handle:timer,Handle:data)
{
    ResetPack(data);
    new client = GetClientOfUserId(ReadPackCell(data)); 
    new player_ident = ReadPackCell(data);
    new raceId = ReadPackCell(data);

    decl String:key[CLASS_STRING_LENGTH];
    ReadPackString(data, key, sizeof(key));

    CloseHandle(data);

    // Retry to save the player's race data
    if (IsValidClient(client))
        SavePlayerRaceData(client, player_ident, raceId, true, key);
}

bool:SavePlayerTechData(client, player_ident, Faction:techId, bool:threaded, const String:key[])
{
    TraceInto("DB", "SavePlayerTechData", \
              "client=%d:%L, player_ident=%d, tech=%d, threaded=%d", \
              client, ValidClientIndex(client), player_ident, techId, threaded);

    new bool:success    = false;           
    new Handle:techInfo = GetTechInfo(client, techId);
    if (techInfo != INVALID_HANDLE)
    {
        new count = GetTechInfoCount(techInfo);
        new level = GetTechInfoLevel(techInfo);

        Trace("%d:%L Save Tech Data, tech=%d, techInfo=0x%08x, count=%d, level=%d", \
              client, ValidClientIndex(client), techId, techInfo, count, level);

        decl String:SQLString[QUERY_STRING_LENGTH];
        Format(SQLString,sizeof(SQLString), "INSERT INTO sc_player_tech (player_ident,faction,tech_count,tech_level) VALUES (%d,%d,%d,%d) ON DUPLICATE KEY UPDATE tech_count=%d,tech_level=%d", player_ident, techId, count, level, count, level);

        if (threaded && DatabaseAvailable(DB_THREAD))
        {
            new Handle:dataPack = CreateDataPack();
            WritePackCell(dataPack, GetClientUserId(client));
            WritePackCell(dataPack, player_ident);
            WritePackCell(dataPack, _:techId);
            WritePackString(dataPack, key);

            UpdateThreadsInFlight(key, 1);
            SQL_TQuery(g_DbHandle[DB_THREAD], SQL_SavePlayerTechData, SQLString, dataPack);
            success = true; // Assume success when threaded.
        }
        else if (DatabaseAvailable(DB_MAIN))
        {
            decl String:error[ERROR_STRING_LENGTH]; error[0] = '\0';
            success = SQL_FastQuery(g_DbHandle[DB_MAIN],SQLString);
            if (!success)
            {
                SQL_GetError(g_DbHandle[DB_MAIN], error, sizeof(error));

                // Retry Access denied or Lost connection errors.
                if (StrContains(error, "denied") >= 0 || StrContains(error, "connection") >= 0)
                {
                    decl String:techName[NAME_STRING_LENGTH];
                    GetFactionName(techId, techName, sizeof(techName));
                    LogError("Initial attempt to insert/update %d:%L's (ident=%d) %d:%s tech data failed: %s\nSQL:%s",
                             client, ValidClientIndex(client), player_ident, techId, techName, error, SQLString);

                    CloseDbHandle(DB_MAIN);
                    if (DatabaseAvailable(DB_MAIN))
                    {
                        success = SQL_FastQuery(g_DbHandle[DB_MAIN],SQLString);
                        if (success)
                        {
                            error[0] = '\0';
                            LogError("Second attempt to insert/update %d:%L's (ident=%d) %d:%s tech data succeeded!",
                                     client, ValidClientIndex(client), player_ident, techId, techName);
                        }
                        else
                            SQL_GetError(g_DbHandle[DB_MAIN], error, sizeof(error));
                    }
                }
                else if (StrContains(error, "constraint") >= 0)
                {
                    if (StrContains(error, "player_ident") >= 0)
                    {
                        // Invalidate player_ident for constraint failed errors.
                        SetDatabaseIdent(client,0);
                    }
                }
            }

            if (!success)
            {
                SetDatabaseSaved(client, false);

                decl String:techName[NAME_STRING_LENGTH];
                GetFactionName(techId, techName, sizeof(techName));
                LogError("Unable to insert/update %d:%L's (ident=%d) %d:%s tech data: %s\nSQL:%s",
                         client, ValidClientIndex(client), player_ident, techId,
                         techName, error, SQLString);
            }
        }
        else
        {
            decl String:techName[NAME_STRING_LENGTH];
            GetFactionName(techId, techName, sizeof(techName));
            LogError("Unable to connect to DB to insert/update %d:%L's (ident=%d) %d:%s tech data",
                     client, ValidClientIndex(client), player_ident, techId, techName);

            success = false;
        }
    }
    else
    {
        LogError("Unable to get %d:%L's %d tech data",
                 client, ValidClientIndex(client), techId);
    }

    TraceReturn("success=%d", success);
    return success;
}

public SQL_SavePlayerTechData(Handle:owner, Handle:hndl, const String:error[], any:data)
{
    TraceInto("DB", "SQL_SavePlayerTechData");

    if (data == INVALID_HANDLE)
        LogError("Invalid Datapack Passed to SQL_SavePlayerTechData!");
    else 
    {
        ResetPack(data);
        new userid            = ReadPackCell(data); 
        new client            = GetClientOfUserId(userid);
        new player_ident      = ReadPackCell(data); 
        new Faction:techId    = Faction:ReadPackCell(data); 

        decl String:key[CLASS_STRING_LENGTH];
        ReadPackString(data, key, sizeof(key));

        if (hndl != INVALID_HANDLE && error[0] == '\0')
            UpdateThreadsInFlight(key, -1);
        else
        {
            if (client > 0)
            {
                SetDatabaseSaved(client, false);
                if (StrContains(error, "constraint") >= 0)
                {
                    if (StrContains(error, "player_ident") >= 0)
                    {
                        // Invalidate player_ident for constraint failed errors.
                        SetDatabaseIdent(client,0);
                    }
                }
            }

            // Invalidate DB_THREAD handle for denied or Lost connection errors.
            if (StrContains(error, "denied") >= 0 || StrContains(error, "connection") >= 0)
            {
                // Invalidate and close the DB_THREAD handle (in the main thread)
                CreateTimer(0.1, CloseDatabaseTimer, g_DbHandle[DB_THREAD],TIMER_FLAG_NO_MAPCHANGE);
                g_DbHandle[DB_THREAD] = INVALID_HANDLE;
            }

            RemoveFromTrie(g_SaveThreadsInFlight, key);

            decl String:techName[NAME_STRING_LENGTH];
            GetFactionName(techId, techName, sizeof(techName));
            LogError("Unable to insert/update %d:%L's (ident=%d) %d:%s tech data: %s",
                     client, ValidClientIndex(client), player_ident, techId, techName, error);
        }
        CloseHandle(data);
    }

    TraceReturn();
}

bool:SavePlayerUpgrades(client, player_ident, race_ident, raceId,
                        Handle:raceInfo, Handle:raceHandle,
                        const String:key[])
{
    TraceInto("DB", "SavePlayerUpgrades", \
              "client=%d:%L, player_ident=%d, race_ident=%d, raceId=%d, raceInfo=0x%08x, raceHandle=0x%08x", \
              client, ValidClientIndex(client), player_ident, race_ident, raceId, raceInfo, raceHandle);

    new bool:success = true;
    if (DatabaseAvailable(DB_MAIN))
    {
        static const String:prepareStmt[] = "INSERT INTO sc_player_upgrades (player_ident,race_ident,upgrade,upgrade_level) VALUES (?,?,?,?) ON DUPLICATE KEY UPDATE upgrade_level=?";

        decl String:error[ERROR_STRING_LENGTH]; error[0] = '\0';
        new Handle:dbReplaceUpgradeStmtHandle = SQL_PrepareQuery(g_DbHandle[DB_MAIN], prepareStmt,
                                                                 error, sizeof(error));

        if (dbReplaceUpgradeStmtHandle == INVALID_HANDLE)
        {
            // Retry Access denied or Lost connection errors.
            if (StrContains(error, "denied") >= 0 || StrContains(error, "connection") >= 0)
            {
                LogError("Initial attempt to prepare %d:%L's Player Upgrade Replace failed: %s",
                         client, ValidClientIndex(client), player_ident, error);

                CloseDbHandle(DB_MAIN);
                if (DatabaseAvailable(DB_MAIN))
                {
                    dbReplaceUpgradeStmtHandle = SQL_PrepareQuery(g_DbHandle[DB_MAIN], prepareStmt,
                                                                  error, sizeof(error));
                    if (dbReplaceUpgradeStmtHandle != INVALID_HANDLE)
                    {
                        LogError("Second attempt to prepare %d:%L's (ident=%d) Player Upgrade Replace succeeded!",
                                 client, ValidClientIndex(client), player_ident);
                    }
                }
            }
        }

        if (dbReplaceUpgradeStmtHandle == INVALID_HANDLE)
        {
            success = false;
            SetDatabaseSaved(client, false);
            LogError("Unable to prepare %d:%L's (ident=%d) Player Upgrade Replace: %s",
                     client, ValidClientIndex(client), player_ident, error);
        }
        else
        {
            new upgradeCount = GetUpgradeCount(raceHandle);
            for (new upgrade = 0; upgrade < upgradeCount; upgrade++)
            {
                new upgradeLevel = GetRaceInfoUpgradeLevel(raceInfo,upgrade);
                if (upgradeLevel != GetRaceInfoSavedUpgradeLevel(raceInfo,upgrade))
                {
                    SQL_BindParamInt(dbReplaceUpgradeStmtHandle, 0, player_ident);
                    SQL_BindParamInt(dbReplaceUpgradeStmtHandle, 1, race_ident);
                    SQL_BindParamInt(dbReplaceUpgradeStmtHandle, 2, upgrade);
                    SQL_BindParamInt(dbReplaceUpgradeStmtHandle, 3, upgradeLevel);
                    SQL_BindParamInt(dbReplaceUpgradeStmtHandle, 4, upgradeLevel);

                    if (SQL_Execute(dbReplaceUpgradeStmtHandle))
                        SetRaceInfoSavedUpgradeLevel(raceInfo,upgrade,upgradeLevel);
                    else
                    {
                        SetDatabaseSaved(client, false);
                        success = false;

                        decl String:raceName[SHORT_STRING_LENGTH];
                        GetRaceShortName(raceHandle, raceName, sizeof(raceName));

                        SQL_GetError(dbReplaceUpgradeStmtHandle, error, sizeof(error));
                        LogError("Unable to insert/update %d:%L's (ident=%d) %d:%s upgrade %d data: %s",
                                 client, ValidClientIndex(client), player_ident, race_ident, raceName,
                                 upgrade, error);

                        if (StrContains(error, "denied") >= 0 || StrContains(error, "connection") >= 0)
                        {
                            // Abort saving upgrade data when connection lost or access denied
                            break;
                        }
                        else if (StrContains(error, "constraint") >= 0)
                        {
                            if (StrContains(error, "player_ident") >= 0)
                            {
                                // Invalidate player_ident for constraint failed errors and then abort.
                                SetDatabaseIdent(client,0);
                                break;
                            }
                            else if (StrContains(error, "sc_upgrades") >= 0)
                            {
                                // Upgrade doesn't exist in DB for the race so add it here
                                decl String:short[SHORT_STRING_LENGTH];
                                decl String:name[NAME_STRING_LENGTH];
                                decl String:desc[LONG_STRING_LENGTH];
                                GetUpgradeShortName(raceHandle,upgrade,short,sizeof(short));
                                GetUpgradeNameAndDesc(raceHandle,upgrade,name,sizeof(name),desc,sizeof(desc));
                                InsertUpgrade(race_ident, upgrade, name, short, desc, GetUpgradeCategory(raceHandle,upgrade));

                                // Then retry saving the client's race data, use a
                                // timer in case more than 1 upgrade needs to be added
                                new Handle:dataPack = CreateDataPack();
                                WritePackCell(dataPack, GetClientUserId(client));
                                WritePackCell(dataPack, player_ident);
                                WritePackCell(dataPack, raceId);
                                WritePackString(dataPack, key);
                                CreateTimer(0.1, Retry_SavePlayerRaceData, dataPack,TIMER_FLAG_NO_MAPCHANGE);
                            }
                        }
                    }
                }
            }

            CloseHandle(dbReplaceUpgradeStmtHandle);
        }
    }
    else
    {
        LogError("Unable to connect to DB to insert/update %d:%L's (ident=%d) upgrade data\n",
                 client, ValidClientIndex(client), player_ident);
        success = false;
    }

    TraceReturn("success=%d", success);
    return success;
}

ThreadedSavePlayerUpgrades(client, player_ident, race_ident, raceId,
                           Handle:raceInfo, Handle:raceHandle,
                           const String:key[])
{
    TraceInto("DB", "ThreadedSavePlayerUpgrades", \
              "client=%d:%L, player_ident=%d, race_ident=%d, raceId=%d, raceInfo=0x%08x, raceHandle=0x%08x, key=%s", \
              client, ValidClientIndex(client), player_ident, race_ident, raceId, raceInfo, raceHandle, key);

    new upgradeCount = GetUpgradeCount(raceHandle);
    for (new upgrade = 0; upgrade < upgradeCount; upgrade++)
    {
        new upgradeLevel = GetRaceInfoUpgradeLevel(raceInfo,upgrade);
        if (upgradeLevel != GetRaceInfoSavedUpgradeLevel(raceInfo,upgrade))
        {
            decl String:SQLString[QUERY_STRING_LENGTH];
            Format(SQLString,sizeof(SQLString),
                   "INSERT INTO sc_player_upgrades (player_ident,race_ident,upgrade,upgrade_level) VALUES (%d,%d,%d,%d) ON DUPLICATE KEY UPDATE upgrade_level=%d",
                   player_ident, race_ident, upgrade, upgradeLevel, upgradeLevel);

            new Handle:dataPack = CreateDataPack();
            WritePackCell(dataPack, GetClientUserId(client));
            WritePackCell(dataPack, player_ident);
            WritePackCell(dataPack, race_ident);
            WritePackCell(dataPack, raceId);
            WritePackCell(dataPack, upgrade);
            WritePackCell(dataPack, upgradeLevel);
            WritePackString(dataPack, key);

            UpdateThreadsInFlight(key, 1);
            SQL_TQuery(g_DbHandle[DB_THREAD], SQL_SavePlayerUpgrades, SQLString, dataPack);
        }
    }

    TraceReturn("cleared 0x%08x", raceInfo);
}

public SQL_SavePlayerUpgrades(Handle:owner, Handle:hndl, const String:error[], any:data)
{
    TraceInto("DB", "SQL_SavePlayerUpgrades");

    if (data == INVALID_HANDLE)
        LogError("Invalid Datapack Passed to SQL_SavePlayerUpgrades!");
    else 
    {
        ResetPack(data);
        new userid = ReadPackCell(data); 
        new client = GetClientOfUserId(userid); 
        new player_ident  = ReadPackCell(data); 
        new race_ident = ReadPackCell(data);
        new raceId = ReadPackCell(data);
        new upgrade = ReadPackCell(data);
        new level = ReadPackCell(data);

        decl String:key[CLASS_STRING_LENGTH];
        ReadPackString(data, key, sizeof(key));

        if (hndl != INVALID_HANDLE && error[0] == '\0')
        {
            UpdateThreadsInFlight(key, -1);
            if (client > 0)
            {
                new Handle:raceInfo = GetRaceInfo(client, raceId);
                if (raceInfo != INVALID_HANDLE)
                    SetRaceInfoSavedUpgradeLevel(raceInfo, upgrade, level);
            }
        }
        else
        {
            if (client > 0)
            {
                new Handle:raceInfo = GetRaceInfo(client, raceId);
                if (raceInfo != INVALID_HANDLE)
                    SetRaceInfoSavedUpgradeLevel(raceInfo, upgrade, -1);
            }

            decl String:raceName[SHORT_STRING_LENGTH];
            new Handle:raceHandle = GetRaceHandle(raceId);
            GetRaceShortName(raceHandle, raceName, sizeof(raceName));
            LogError("Unable to insert/update %d:%L's (ident=%d) %d:%s upgrade %d data: %s",
                     client, ValidClientIndex(client), player_ident, race_ident, raceName,
                     upgrade, error);

            // Invalidate DB_THREAD handle for denied or Lost connection errors.
            if (StrContains(error, "denied") >= 0 || StrContains(error, "connection") >= 0)
            {
                // Invalidate and close the DB_THREAD handle (in the main thread)
                CreateTimer(0.1, CloseDatabaseTimer, g_DbHandle[DB_THREAD],TIMER_FLAG_NO_MAPCHANGE);
                g_DbHandle[DB_THREAD] = INVALID_HANDLE;
                RemoveFromTrie(g_SaveThreadsInFlight, key);
            }
            else if (StrContains(error, "constraint") >= 0)
            {
                if (StrContains(error, "player_ident") >= 0)
                {
                    // Invalidate player_ident for constraint failed errors.
                    SetDatabaseIdent(client,0);
                    RemoveFromTrie(g_SaveThreadsInFlight, key);
                }
                else if (StrContains(error, "sc_upgrades") >= 0)
                {
                    // Retry saving the client's upgrade data
                    // (after adding the upgrade to the sc_upgrade table)
                    new Handle:dataPack = CreateDataPack();
                    WritePackCell(dataPack, userid);
                    WritePackCell(dataPack, player_ident);
                    WritePackCell(dataPack, race_ident);
                    WritePackCell(dataPack, raceId);
                    WritePackCell(dataPack, upgrade);
                    WritePackString(dataPack, key);
                    CreateTimer(0.1, Retry_SavePlayerUpgradeData, dataPack,TIMER_FLAG_NO_MAPCHANGE);
                }
            }
            else
                RemoveFromTrie(g_SaveThreadsInFlight, key);
        }
        CloseHandle(data);
    }

    TraceReturn();
}

public Action:Retry_SavePlayerUpgradeData(Handle:timer,Handle:data)
{
    ResetPack(data);
    new client = GetClientOfUserId(ReadPackCell(data)); 
    new player_ident = ReadPackCell(data);
    new race_ident = ReadPackCell(data);
    new raceId = ReadPackCell(data);
    new upgrade = ReadPackCell(data);

    decl String:key[CLASS_STRING_LENGTH];
    ReadPackString(data, key, sizeof(key));

    CloseHandle(data);

    if (raceId > 0)
    {
        // Upgrade doesn't exist in DB for the race so add it here
        decl String:short[SHORT_STRING_LENGTH];
        decl String:name[NAME_STRING_LENGTH];
        decl String:desc[LONG_STRING_LENGTH];
        new Handle:raceHandle = GetRaceHandle(raceId);
        GetUpgradeShortName(raceHandle,upgrade,short,sizeof(short));
        GetUpgradeNameAndDesc(raceHandle,upgrade,name,sizeof(name),desc,sizeof(desc));
        InsertUpgrade(race_ident, upgrade, name, short, desc, GetUpgradeCategory(raceHandle,upgrade));

        // Retry to save the player's race (including upgrade) data
        if (client > 0)
            SavePlayerRaceData(client, player_ident, raceId, true, key);
    }
}

DisablePlayer(admin, target)
{
    TraceInto("DB", "DisablePlayer", "admin=%d:%L, target=%d:%L's", \
              admin, ValidClientIndex(admin), \
              target, ValidClientIndex(target));

    SetPlayerStatus(target,PlayerDisabled);

    if (DatabaseAvailable(DB_THREAD))
    {
        new player_ident = GetDatabaseIdent(target);
        if (player_ident > 0)
        {
            decl String:SQLString[SHORT_QUERY_STRING_LENGTH];
            Format(SQLString,sizeof(SQLString),
                   "UPDATE LOW_PRIORITY sc_players SET status = 'Disabled' WHERE player_ident = %d",
                   player_ident);

            new Handle:dataPack = CreateDataPack();
            WritePackCell(dataPack, GetClientUserId(admin));
            WritePackCell(dataPack, GetClientUserId(target));
            WritePackCell(dataPack, player_ident);
            WritePackString(dataPack, "");

            SQL_TQuery(g_DbHandle[DB_THREAD], SQL_DisablePlayer, SQLString, dataPack);
        }
        else
        {
            decl String:steamid[ID_STRING_LENGTH];
            if (IsClientConnected(target) &&
                GetClientAuthString(target,steamid,sizeof(steamid)))
            {
                decl String:SQLString[SHORT_QUERY_STRING_LENGTH];
                Format(SQLString,sizeof(SQLString),
                       "UPDATE LOW_PRIORITY sc_players SET status = 'Disabled' WHERE steamid = %d AND del_rec_date IS NULL",
                       steamid);

                new Handle:dataPack = CreateDataPack();
                WritePackCell(dataPack, GetClientUserId(admin));
                WritePackCell(dataPack, GetClientUserId(target));
                WritePackCell(dataPack, player_ident);
                WritePackString(dataPack, steamid);

                SQL_TQuery(g_DbHandle[DB_THREAD], SQL_DisablePlayer, SQLString, dataPack);
            }
            else
                LogError("Unable to obtain steamid when disabling %d:%L",
                         target, ValidClientIndex(target));
        }
    }

    TraceReturn();
}

public SQL_DisablePlayer(Handle:owner, Handle:hndl, const String:error[], any:data)
{
    TraceInto("DB", "SQL_DisablePlayer");

    ResetPack(data);
    new adminid = ReadPackCell(data); 
    new admin = GetClientOfUserId(adminid); 
    new userid = ReadPackCell(data); 
    new target = GetClientOfUserId(userid); 
    new player_ident = ReadPackCell(data);

    decl String:steamid[ID_STRING_LENGTH];
    ReadPackString(data, steamid, sizeof(steamid));

    CloseHandle(data);

    if (hndl == INVALID_HANDLE || error[0] != '\0')
    {
        // Invalidate DB_THREAD handle for denied or Lost connection errors.
        if (StrContains(error, "denied") >= 0 || StrContains(error, "connection") >= 0)
        {
            // Invalidate and close the DB_THREAD handle (in the main thread)
            CreateTimer(0.1, CloseDatabaseTimer, g_DbHandle[DB_THREAD],TIMER_FLAG_NO_MAPCHANGE);
            g_DbHandle[DB_THREAD] = INVALID_HANDLE;
        }
        else
        {
            LogError("Unable to disable player %d:%L's (ident=%d, steamid=%s) data: %s",
                     target,ValidClientIndex(target), player_ident, steamid, error);
        }
    }
    else
    {
        LogAction(admin, target, "%d:%L disabled player %d:%L's (ident=%d, steamid=%s)",
                  admin, ValidClientIndex(admin), target, ValidClientIndex(target),
                  player_ident, steamid);

        LogMessage("%d:%L disabled player %d:%L's (ident=%d, steamid=%s)",
                   admin, ValidClientIndex(admin), target, ValidClientIndex(target),
                   player_ident, steamid);
    }

    TraceReturn();
}


EnablePlayer(admin, target)
{
    TraceInto("DB", "EnablePlayer", "admin=%d:%L, target=%d:%L's", \
              admin, ValidClientIndex(admin), \
              target, ValidClientIndex(target));

    SetPlayerStatus(target,PlayerActive);

    if (DatabaseAvailable(DB_THREAD))
    {
        new player_ident = GetDatabaseIdent(target);
        if (player_ident > 0)
        {
            decl String:SQLString[SHORT_QUERY_STRING_LENGTH];
            Format(SQLString,sizeof(SQLString),
                   "UPDATE LOW_PRIORITY sc_players SET status = 'Active' WHERE player_ident = %d",
                   player_ident);

            new Handle:dataPack = CreateDataPack();
            WritePackCell(dataPack, GetClientUserId(admin));
            WritePackCell(dataPack, GetClientUserId(target));
            WritePackCell(dataPack, player_ident);
            WritePackString(dataPack, "");

            SQL_TQuery(g_DbHandle[DB_THREAD], SQL_EnablePlayer, SQLString, dataPack);
        }
        else
        {
            decl String:steamid[ID_STRING_LENGTH];
            if (IsClientConnected(target) &&
                GetClientAuthString(target,steamid,sizeof(steamid)))
            {
                decl String:SQLString[SHORT_QUERY_STRING_LENGTH];
                Format(SQLString,sizeof(SQLString),
                       "UPDATE LOW_PRIORITY sc_players SET status = 'Active' WHERE steamid = %d AND del_rec_date IS NULL",
                       steamid);

                new Handle:dataPack = CreateDataPack();
                WritePackCell(dataPack, GetClientUserId(admin));
                WritePackCell(dataPack, GetClientUserId(target));
                WritePackCell(dataPack, player_ident);
                WritePackString(dataPack, steamid);

                SQL_TQuery(g_DbHandle[DB_THREAD], SQL_EnablePlayer, SQLString, dataPack);
            }
            else
                LogError("Unable to obtain steamid when disabling %d:%L",
                         target, ValidClientIndex(target));
        }
    }

    TraceReturn();
}

public SQL_EnablePlayer(Handle:owner, Handle:hndl, const String:error[], any:data)
{
    TraceInto("DB", "SQL_EnablePlayer");

    ResetPack(data);
    new adminid = ReadPackCell(data); 
    new admin = GetClientOfUserId(adminid); 
    new userid = ReadPackCell(data); 
    new target = GetClientOfUserId(userid); 
    new player_ident = ReadPackCell(data);

    decl String:steamid[ID_STRING_LENGTH];
    ReadPackString(data, steamid, sizeof(steamid));

    CloseHandle(data);

    if (hndl == INVALID_HANDLE || error[0] != '\0')
    {
        // Invalidate DB_THREAD handle for denied or Lost connection errors.
        if (StrContains(error, "denied") >= 0 || StrContains(error, "connection") >= 0)
        {
            // Invalidate and close the DB_THREAD handle (in the main thread)
            CreateTimer(0.1, CloseDatabaseTimer, g_DbHandle[DB_THREAD],TIMER_FLAG_NO_MAPCHANGE);
            g_DbHandle[DB_THREAD] = INVALID_HANDLE;
        }
        else
        {
            LogError("Unable to enable player %d:%L's (ident=%d, steamid=%s) data: %s",
                     target,ValidClientIndex(target), player_ident, steamid, error);
        }
    }
    else
    {
        LogAction(admin, target, "%d:%L enabled player %d:%L's (ident=%d, steamid=%s)",
                  admin, ValidClientIndex(admin), target, ValidClientIndex(target),
                  player_ident, steamid);

        LogMessage("%d:%L enabled player %d:%L's (ident=%d, steamid=%s)",
                   admin, ValidClientIndex(admin), target, ValidClientIndex(target),
                   player_ident, steamid);
    }

    TraceReturn();
}

DeletePlayer(admin, target)
{
    TraceInto("DB", "DeletePlayer", "admin=%d:%L, target=%d:%L's", \
              admin, ValidClientIndex(admin), \
              target, ValidClientIndex(target));

    // Start out by disabling the player              
    SetPlayerStatus(target,PlayerDisabled);

    if (DatabaseAvailable(DB_THREAD))
    {
        new player_ident = GetDatabaseIdent(target);
        if (player_ident > 0)
        {
            decl String:SQLString[SHORT_QUERY_STRING_LENGTH];
            Format(SQLString,sizeof(SQLString),
            //     "DELETE LOW_PRIORITY FROM sc_players WHERE player_ident = %d",
                   "UPDATE LOW_PRIORITY sc_players SET del_rec_date = NOW() WHERE player_ident = %d",
                   player_ident);

            new Handle:dataPack = CreateDataPack();
            WritePackCell(dataPack, GetClientUserId(admin));
            WritePackCell(dataPack, GetClientUserId(target));
            WritePackCell(dataPack, player_ident);
            WritePackString(dataPack, "");

            SQL_TQuery(g_DbHandle[DB_THREAD], SQL_DeletePlayer, SQLString, dataPack);
        }
        else
        {
            decl String:steamid[ID_STRING_LENGTH];
            if (IsClientConnected(target) &&
                GetClientAuthString(target,steamid,sizeof(steamid)))
            {
                decl String:SQLString[SHORT_QUERY_STRING_LENGTH];
                Format(SQLString,sizeof(SQLString),
                //     "DELETE LOW_PRIORITY FROM sc_players WHERE steamid = %d",
                       "UPDATE LOW_PRIORITY sc_players SET del_rec_date = NOW() WHERE steamid = %d AND del_rec_date IS NULL",
                       steamid);

                new Handle:dataPack = CreateDataPack();
                WritePackCell(dataPack, GetClientUserId(admin));
                WritePackCell(dataPack, GetClientUserId(target));
                WritePackCell(dataPack, player_ident);
                WritePackString(dataPack, steamid);

                SQL_TQuery(g_DbHandle[DB_THREAD], SQL_DeletePlayer, SQLString, dataPack);
            }
            else
                LogError("Unable to obtain steamid when deleting %d:%L",
                         target, ValidClientIndex(target));
        }
    }

    TraceReturn();
}

public SQL_DeletePlayer(Handle:owner, Handle:hndl, const String:error[], any:data)
{
    TraceInto("DB", "SQL_DeletePlayer");

    ResetPack(data);
    new adminid = ReadPackCell(data); 
    new admin = GetClientOfUserId(adminid); 
    new userid = ReadPackCell(data); 
    new target = GetClientOfUserId(userid); 
    new player_ident = ReadPackCell(data);

    decl String:steamid[ID_STRING_LENGTH];
    ReadPackString(data, steamid, sizeof(steamid));

    CloseHandle(data);

    if (hndl == INVALID_HANDLE || error[0] != '\0')
    {
        // Invalidate DB_THREAD handle for denied or Lost connection errors.
        if (StrContains(error, "denied") >= 0 || StrContains(error, "connection") >= 0)
        {
            // Invalidate and close the DB_THREAD handle (in the main thread)
            CreateTimer(0.1, CloseDatabaseTimer, g_DbHandle[DB_THREAD],TIMER_FLAG_NO_MAPCHANGE);
            g_DbHandle[DB_THREAD] = INVALID_HANDLE;
        }
        else
        {
            LogError("Unable to delete player %d:%L's (ident=%d, steamid=%s) data: %s",
                     target,ValidClientIndex(target), player_ident, steamid, error);
        }
    }
    else
    {
        if (target > 0)
            SetDatabaseIdent(target,0);

        LogAction(admin, target, "%d:%L deleted player %d:%L's (ident=%d, steamid=%s)",
                  admin, ValidClientIndex(admin), target, ValidClientIndex(target),
                  player_ident, steamid);

        LogMessage("%d:%L deleted player %d:%L's (ident=%d, steamid=%s)",
                   admin, ValidClientIndex(admin), target, ValidClientIndex(target),
                   player_ident, steamid);
    }

    TraceReturn();
}

FixLevels(client)
{
    if (DatabaseAvailable(DB_THREAD))
    {
        SQL_TQuery(g_DbHandle[DB_THREAD], SQL_FixLevels,
                   "CALL fix_all_levels()",
                   GetClientUserId(client));
    }
    else
    {
        LogError("Unable to connect to DB to run fix_levels script");
    }
}

public SQL_FixLevels(Handle:owner, Handle:hndl, const String:error[], any:userid)
{
    if (hndl == INVALID_HANDLE || error[0] != '\0')
    {
        // Invalidate DB_THREAD handle for denied or Lost connection errors.
        if (StrContains(error, "denied") >= 0 || StrContains(error, "connection") >= 0)
        {
            // Invalidate and close the DB_THREAD handle (in the main thread)
            CreateTimer(0.1, CloseDatabaseTimer, g_DbHandle[DB_THREAD],TIMER_FLAG_NO_MAPCHANGE);
            g_DbHandle[DB_THREAD] = INVALID_HANDLE;
        }

        LogError("Error in executing the fix_all_levels() procedure:%s", error);
    }
    else
    {
        new admin = GetClientOfUserId(userid); 
        LogAction(admin, -1, "%d:%L executed the fix_all_levels() procedure",
                  admin, ValidClientIndex(admin));
    }
}

