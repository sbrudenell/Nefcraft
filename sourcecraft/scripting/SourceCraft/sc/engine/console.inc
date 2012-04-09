/*
 * vim: set ai et ts=4 sw=4 syntax=sourcepawn :
 * File: console.inc
 * Description: This file handles console variables that can be modified from the server console
 *              or RCON, and it also handles the admin commands that can be called from either
 *              server or client admin.  
 * Author(s): Naris (Murray Wilson)
 * Credits: Anthony Iacono
 */

//#define ADMFLAG_SOURCECRAFT ADMFLAG_RCON
#define ADMFLAG_SOURCECRAFT ADMFLAG_CUSTOM1

InitCVars()
{
    CreateConVar("sc_version",SOURCECRAFT_VERSION,"SourceCraft version.",FCVAR_PLUGIN|FCVAR_REPLICATED|FCVAR_SPONLY|FCVAR_NOTIFY|FCVAR_UNLOGGED|FCVAR_DONTRECORD);

    RegAdminCmd("sc_setxp",CMD_SetXP,ADMFLAG_SOURCECRAFT,"Set a player's XP (for Admins)");
    RegAdminCmd("sc_givexp",CMD_GiveXP,ADMFLAG_SOURCECRAFT,"Give a player XP (for Admins)");
    RegAdminCmd("sc_removexp",CMD_RemoveXP,ADMFLAG_SOURCECRAFT,"Remove some XP from a player (for Admins)");
    RegAdminCmd("sc_setlevel",CMD_SetLevel,ADMFLAG_SOURCECRAFT,"Set a player's level (for Admins)");
    RegAdminCmd("sc_givelevel",CMD_GiveLevel,ADMFLAG_SOURCECRAFT,"Give a player a single level (for Admins)");
    RegAdminCmd("sc_removelevel",CMD_RemoveLevel,ADMFLAG_SOURCECRAFT,"Remove a single level from a player (for Admins)");
    RegAdminCmd("sc_setcrystals",CMD_SetCrystals,ADMFLAG_SOURCECRAFT,"Set a player's crystal count (for Admins)");
    RegAdminCmd("sc_givecrystals",CMD_GiveCrystals,ADMFLAG_SOURCECRAFT,"Give a player crystals (for Admins)");
    RegAdminCmd("sc_removecrystals",CMD_RemoveCrystals,ADMFLAG_SOURCECRAFT,"Remove some crystals from a player (for Admins)");

    RegAdminCmd("sc_setvespene",CMD_SetVespene,ADMFLAG_SOURCECRAFT,"Set a player's vespene count (for Admins)");
    RegAdminCmd("sc_givevespene",CMD_GiveVespene,ADMFLAG_SOURCECRAFT,"Give a player vespene (for Admins)");
    RegAdminCmd("sc_removevespene",CMD_RemoveVespene,ADMFLAG_SOURCECRAFT,"Remove some vespene from a player (for Admins)");

    RegAdminCmd("sc_setenergy",CMD_SetEnergy,ADMFLAG_SOURCECRAFT,"Set a player's energy count (for Admins)");
    RegAdminCmd("sc_giveenergy",CMD_GiveEnergy,ADMFLAG_SOURCECRAFT,"Give a player energy (for Admins)");
    RegAdminCmd("sc_removeenergy",CMD_RemoveEnergy,ADMFLAG_SOURCECRAFT,"Remove some energy from a player (for Admins)");

    RegAdminCmd("sc_admin",AdminMenu,ADMFLAG_SOURCECRAFT,"Brings up the SourceCraft admin panel. (for Admins)");
    RegAdminCmd("sc_fix_levels",CMD_FixLevels,ADMFLAG_SOURCECRAFT,"Re-Calculates the levels for all players in the database. (for Admins)");
}

public Action:CMD_SetXP(client,args)
{
    if (args != 2)
        ReplyToCommand(client,"[SC] %t", "sc_setxp");
    else
    {
        TraceInto("Console", "CMD_SetXP");

        decl String:match[MAX_TARGET_LENGTH];
        GetCmdArg(1,match,sizeof(match));

        decl String:buf[32];
        GetCmdArg(2,buf,sizeof(buf));

        new xp=StringToInt(buf);
        if (xp<0)
            xp=0;

        new bool:tn_is_ml;
        new target_list[MAXPLAYERS];
        new String:target_name[MAX_TARGET_LENGTH];
        new count = ProcessTargetString(match, client, target_list, sizeof(target_list),
                                        COMMAND_FILTER_NO_BOTS, target_name,
                                        sizeof(target_name), tn_is_ml);
        if (count <= COMMAND_TARGET_NONE)
            ReplyToTargetError(client, count);
        else
        {
            for(new x=0;x<count;x++)
            {
                new index = target_list[x];
                if (IsValidClient(index))
                {
                    new race=GetRace(index);
                    SetXP(index,race,xp);
                    LogAction(client, index, "[SC] %L set %L's XP to %d", client, index, xp);
                    ReplyToCommand(client,"[SC] %t", "AdminSetXP", index, xp);
                    CPrintToChat(index, "{green}[SC]{default} %t", "PlayersXPWasSet", client, xp);
                    DoLevelCheck(index);
                }
            }
        }

        TraceReturn();
    }
    return Plugin_Handled;
}

public Action:CMD_GiveXP(client,args)
{
    if (args != 2)
        ReplyToCommand(client,"[SC] %t", "sc_givexp");
    else
    {
        TraceInto("Console", "CMD_GiveXP");

        decl String:match[MAX_TARGET_LENGTH];
        GetCmdArg(1,match,sizeof(match));

        decl String:buf[32];
        GetCmdArg(2,buf,sizeof(buf));

        new xp=StringToInt(buf);
        if (xp<0)
            xp=0;

        new bool:tn_is_ml;
        new target_list[MAXPLAYERS];
        new String:target_name[MAX_TARGET_LENGTH];
        new count = ProcessTargetString(match, client, target_list, sizeof(target_list),
                                        COMMAND_FILTER_NO_BOTS, target_name,
                                        sizeof(target_name), tn_is_ml);
        if (count <= COMMAND_TARGET_NONE)
            ReplyToTargetError(client, count);
        else
        {
            for(new x=0;x<count;x++)
            {
                new index = target_list[x];
                if (IsValidClient(index))
                {
                    new race=GetRace(index);
                    SetXP(index,race,GetXP(index,race)+xp);
                    LogAction(client, index, "[SC] %L gave %L %d XP", client, index, xp);
                    ReplyToCommand(client,"[SC] %t", "AdminGaveXP", index, xp);
                    CPrintToChat(index, "{green}[SC]{default} %t.", "PlayerReceivedXP", client, xp);
                    DoLevelCheck(index);
                }
            }
        }

        TraceReturn();
    }
    return Plugin_Handled;
}

public Action:CMD_RemoveXP(client,args)
{
    if(args!=2)
        ReplyToCommand(client,"[SC] %t", "sc_removexp");
    else
    {
        TraceInto("Console", "CMD_RemoveXP");

        decl String:match[MAX_TARGET_LENGTH];
        GetCmdArg(1,match,sizeof(match));

        decl String:buf[32];
        GetCmdArg(2,buf,sizeof(buf));

        new xp=StringToInt(buf);
        if (xp<0)
            xp=0;

        new bool:tn_is_ml;
        new target_list[MAXPLAYERS];
        new String:target_name[MAX_TARGET_LENGTH];
        new count = ProcessTargetString(match, client, target_list, sizeof(target_list),
                                        COMMAND_FILTER_NO_BOTS, target_name,
                                        sizeof(target_name), tn_is_ml);
        if (count <= COMMAND_TARGET_NONE)
            ReplyToTargetError(client, count);
        else
        {
            for(new x=0;x<count;x++)
            {
                new index = target_list[x];
                if (IsValidClient(index))
                {
                    new race=GetRace(index);
                    new newxp=GetXP(index,race)-xp;
                    if(newxp<0)
                    newxp=0;
                    SetXP(index,race,newxp);
                    LogAction(client, index, "[SC] %L removed %d XP from %L", client, xp, index);
                    ReplyToCommand(client,"[SC] %t", "AdminRemovedXP", xp, index);
                    CPrintToChat(index, "{green}[SC]{default} %t", "PlayerForfeitedXP", client, xp);
                    DoLevelCheck(index);
                }
            }
        }

        TraceReturn();
    }
    return Plugin_Handled;
}

public Action:CMD_SetLevel(client,args)
{
    if(args!=2)
        ReplyToCommand(client,"[SC] %t", "sc_setlevel");
    else
    {
        TraceInto("Console", "CMD_SetLevel");

        decl String:match[MAX_TARGET_LENGTH];
        GetCmdArg(1,match,sizeof(match));

        decl String:buf[32];
        GetCmdArg(2,buf,sizeof(buf));

        new level=StringToInt(buf);
        if (level<0)
            level=0;

        new bool:tn_is_ml;
        new target_list[MAXPLAYERS];
        new String:target_name[MAX_TARGET_LENGTH];
        new count = ProcessTargetString(match, client, target_list, sizeof(target_list),
                                        COMMAND_FILTER_NO_BOTS, target_name,
                                        sizeof(target_name), tn_is_ml);
        if (count <= COMMAND_TARGET_NONE)
            ReplyToTargetError(client, count);
        else
        {
            for(new x=0;x<count;x++)
            {
                new index = target_list[x];
                if (IsValidClient(index))
                {
                    new race = GetRace(index);
                    new maxLevel = GetRaceMaxLevel(GetRaceHandle(race));
                    if (level > maxLevel)
                        level = maxLevel;
                    else if (level < 0)
                        level = 0;

                    LogAction(client, index, "[SC] %L set %L's level to %d", client, index, level);
                    ReplyToCommand(client,"[SC] %t", "AdminSetLevel", index, level);
                    CPrintToChat(index, "{green}[SC]{default} %t", "PlayersLevelWasSet", client, level);

                    ResetLevel(index,race,level);
                }
            }
        }

        TraceReturn();
    }
    return Plugin_Handled;
}

public Action:CMD_GiveLevel(client,args)
{
    if(args!=1)
        ReplyToCommand(client,"[SC] %t", "sc_givelevel");
    else
    {
        TraceInto("Console", "CMD_GiveLevel");

        decl String:match[MAX_TARGET_LENGTH];
        GetCmdArg(1,match,sizeof(match));

        new bool:tn_is_ml;
        new target_list[MAXPLAYERS];
        new String:target_name[MAX_TARGET_LENGTH];
        new count = ProcessTargetString(match, client, target_list, sizeof(target_list),
                                        COMMAND_FILTER_NO_BOTS, target_name,
                                        sizeof(target_name), tn_is_ml);
        if (count <= COMMAND_TARGET_NONE)
            ReplyToTargetError(client, count);
        else
        {
            for(new x=0;x<count;x++)
            {
                new index = target_list[x];
                if (IsValidClient(index))
                {
                    new race = GetRace(index);
                    new newlevel = GetLevel(index,race)+1;
                    if (newlevel > GetRaceMaxLevel(GetRaceHandle(race)))
                        ReplyToCommand(client,"[SC] %t", "PlayerAtMaxLevel", index);
                    else
                    {
                        LogAction(client, index, "[SC] %L gave %L a level", client, index);
                        ReplyToCommand(client,"[SC] %t", "AdminGaveLevel", index);
                        CPrintToChat(index, "{green}[SC]{default} %t", "PlayerReceivedLevel", client);

                        ResetLevel(index,race,newlevel);
                    }
                }
            }
        }

        TraceReturn();
    }
    return Plugin_Handled;
}

public Action:CMD_RemoveLevel(client,args)
{
    if(args!=1)
        ReplyToCommand(client,"[SC] %t", "sc_removelevel");
    else
    {
        TraceInto("Console", "CMD_RemoveLevel");

        decl String:match[MAX_TARGET_LENGTH];
        GetCmdArg(1,match,sizeof(match));

        new bool:tn_is_ml;
        new target_list[MAXPLAYERS];
        new String:target_name[MAX_TARGET_LENGTH];
        new count = ProcessTargetString(match, client, target_list, sizeof(target_list),
                                        COMMAND_FILTER_NO_BOTS, target_name,
                                        sizeof(target_name), tn_is_ml);
        if (count <= COMMAND_TARGET_NONE)
            ReplyToTargetError(client, count);
        else
        {
            for(new x=0;x<count;x++)
            {
                new index = target_list[x];
                if (IsValidClient(index))
                {
                    new race = GetRace(index);
                    new newlevel = GetLevel(index,race)-1;
                    if (newlevel < 0)
                        ReplyToCommand(client,"[SC] %t", "PlayerHasNoLevels", index);
                    else
                    {
                        LogAction(client, index, "[SC] %L removed a level from %L", client, index);
                        ReplyToCommand(client,"[SC] %t", "AdminRemovedLevel", index);
                        CPrintToChat(index, "{green}[SC]{default} %t", "PlayerForfeitedLevel", client);

                        ResetLevel(index,race,newlevel);
                    }
                }
            }
        }

        TraceReturn();
    }
    return Plugin_Handled;
}

public Action:CMD_SetCrystals(client,args)
{
    if(args!=2)
        ReplyToCommand(client,"[SC] %t", "sc_setcrystals");
    else
    {
        TraceInto("Console", "CMD_SetCrystals");

        decl String:match[MAX_TARGET_LENGTH];
        GetCmdArg(1,match,sizeof(match));

        decl String:buf[32];
        GetCmdArg(2,buf,sizeof(buf));

        new crystals = StringToInt(buf);
        if (crystals < 0)
            crystals = 0;
        else if (crystals > g_iMaxCrystals && g_iMaxCrystals > 0)
            crystals = g_iMaxCrystals;
            
        new bool:tn_is_ml;
        new target_list[MAXPLAYERS];
        new String:target_name[MAX_TARGET_LENGTH];
        new count = ProcessTargetString(match, client, target_list, sizeof(target_list),
                                        COMMAND_FILTER_NO_BOTS, target_name,
                                        sizeof(target_name), tn_is_ml);
        if (count <= COMMAND_TARGET_NONE)
            ReplyToTargetError(client, count);
        else
        {
            for(new x=0;x<count;x++)
            {
                new index = target_list[x];
                if (IsValidClient(index))
                {
                    SetCrystals(index,crystals);
                    LogAction(client, index, "[SC] %L set %L's crystals to %d", client, index, crystals);
                    ReplyToCommand(client,"[SC] %t", "AdminSetCrystals", index, crystals);
                    CPrintToChat(index, "{green}[SC]{default} %t", "PlayersCrystalsWereSet", client, crystals);
                }
            }
        }

        TraceReturn();
    }
    return Plugin_Handled;
}

public Action:CMD_GiveCrystals(client,args)
{
    if(args!=2)
        ReplyToCommand(client,"[SC] %t", "sc_givecrystals");
    else
    {
        TraceInto("Console", "CMD_GiveCrystals");

        decl String:match[MAX_TARGET_LENGTH];
        GetCmdArg(1,match,sizeof(match));

        decl String:buf[32];
        GetCmdArg(2,buf,sizeof(buf));

        new crystals=StringToInt(buf);
        if (crystals<0)
            crystals=0;

        new bool:tn_is_ml;
        new target_list[MAXPLAYERS];
        new String:target_name[MAX_TARGET_LENGTH];
        new count = ProcessTargetString(match, client, target_list, sizeof(target_list),
                                        COMMAND_FILTER_NO_BOTS, target_name,
                                        sizeof(target_name), tn_is_ml);
        if (count <= COMMAND_TARGET_NONE)
            ReplyToTargetError(client, count);
        else
        {
            for(new x=0;x<count;x++)
            {
                new index = target_list[x];
                if (IsValidClient(index))
                {
                    new newcreds = GetCrystals(index)+crystals;
                    if (newcreds < 0)
                        newcreds = 0;
                    else if(newcreds > g_iMaxCrystals && g_iMaxCrystals > 0)
                        newcreds = g_iMaxCrystals;

                    SetCrystals(index,newcreds);
                    LogAction(client, index, "[SC] %L gave %L %d crystals", client, index, crystals);
                    ReplyToCommand(client,"[SC] %t", "AdminGaveCrystals", index, crystals);
                    CPrintToChat(index, "{green}[SC]{default} %t", "PlayerReceivedCrystals", client, crystals);
                }
            }
        }

        TraceReturn();
    }
    return Plugin_Handled;
}

public Action:CMD_RemoveCrystals(client,args)
{
    if(args!=2)
        ReplyToCommand(client,"[SC] %t", "sc_removecrystals");
    else
    {
        TraceInto("Console", "CMD_RemoveCrystals");

        decl String:match[MAX_TARGET_LENGTH];
        GetCmdArg(1,match,sizeof(match));

        decl String:buf[32];
        GetCmdArg(2,buf,sizeof(buf));

        new crystals=StringToInt(buf);
        if (crystals<0)
            crystals=0;

        new bool:tn_is_ml;
        new target_list[MAXPLAYERS];
        new String:target_name[MAX_TARGET_LENGTH];
        new count = ProcessTargetString(match, client, target_list, sizeof(target_list),
                                        COMMAND_FILTER_NO_BOTS, target_name,
                                        sizeof(target_name), tn_is_ml);
        if (count <= COMMAND_TARGET_NONE)
            ReplyToTargetError(client, count);
        else
        {
            for(new x=0;x<count;x++)
            {
                new index = target_list[x];
                if (IsValidClient(index))
                {
                    new newcreds = GetCrystals(index)-crystals;
                    if (newcreds < 0)
                        newcreds = 0;
                    else if(newcreds > g_iMaxCrystals && g_iMaxCrystals > 0)
                        newcreds = g_iMaxCrystals;

                    SetCrystals(index,newcreds);
                    LogAction(client, index, "[SC] %L removed %d crystals from %L", client, crystals, index);
                    ReplyToCommand(client,"[SC] %t", "AdminRemovedCrystals", crystals, index);
                    CPrintToChat(index, "{green}[SC]{default} %t", "PlayerForfeitedCrystals", client, crystals);
                }
            }
        }

        TraceReturn();
    }
    return Plugin_Handled;
}

public Action:CMD_SetVespene(client,args)
{
    if(args!=2)
        ReplyToCommand(client,"[SC] %t", "sc_setvespene");
    else
    {
        TraceInto("Console", "CMD_SetVespene");

        decl String:match[MAX_TARGET_LENGTH];
        GetCmdArg(1,match,sizeof(match));

        decl String:buf[32];
        GetCmdArg(2,buf,sizeof(buf));

        new vespene = StringToInt(buf);
        if (vespene < 0)
            vespene = 0;
        else if (vespene > g_iMaxVespene && g_iMaxVespene > 0)
            vespene = g_iMaxVespene;

        new bool:tn_is_ml;
        new target_list[MAXPLAYERS];
        new String:target_name[MAX_TARGET_LENGTH];
        new count = ProcessTargetString(match, client, target_list, sizeof(target_list),
                                        COMMAND_FILTER_NO_BOTS, target_name,
                                        sizeof(target_name), tn_is_ml);
        if (count <= COMMAND_TARGET_NONE)
            ReplyToTargetError(client, count);
        else
        {
            for(new x=0;x<count;x++)
            {
                new index = target_list[x];
                if (IsValidClient(index))
                {
                    SetVespene(index,vespene);
                    LogAction(client, index, "[SC] %L set %L's vespene to %d", client, index, vespene);
                    ReplyToCommand(client,"[SC] %t", "AdminSetVespene", index, vespene);
                    CPrintToChat(index, "{green}[SC]{default} %t", "PlayersVespeneWasSet", client, vespene);
                }
            }
        }

        TraceReturn();
    }
    return Plugin_Handled;
}

public Action:CMD_GiveVespene(client,args)
{
    if(args!=2)
        ReplyToCommand(client,"[SC] %t", "sc_givevespene");
    else
    {
        TraceInto("Console", "CMD_GiveVespene");

        decl String:match[MAX_TARGET_LENGTH];
        GetCmdArg(1,match,sizeof(match));

        decl String:buf[32];
        GetCmdArg(2,buf,sizeof(buf));

        new amount=StringToInt(buf);
        if (amount<0)
            amount=0;

        new bool:tn_is_ml;
        new target_list[MAXPLAYERS];
        new String:target_name[MAX_TARGET_LENGTH];
        new count = ProcessTargetString(match, client, target_list, sizeof(target_list),
                                        COMMAND_FILTER_NO_BOTS, target_name,
                                        sizeof(target_name), tn_is_ml);
        if (count <= COMMAND_TARGET_NONE)
            ReplyToTargetError(client, count);
        else
        {
            for(new x=0;x<count;x++)
            {
                new index = target_list[x];
                if (IsValidClient(index))
                {
                    new vespene = GetVespene(index)+amount;
                    if (vespene < 0)
                        vespene = 0;
                    else if (vespene > g_iMaxVespene && g_iMaxVespene > 0)
                        vespene = g_iMaxVespene;

                    SetVespene(index,vespene);
                    LogAction(client, index, "[SC] %L gave %L %d vespene", client, index, amount);
                    ReplyToCommand(client,"[SC] %t", "AdminGaveVespene", index, amount);
                    CPrintToChat(index, "{green}[SC]{default} %t", "PlayerReceivedVespene", client, amount);
                }
            }
        }

        TraceReturn();
    }
    return Plugin_Handled;
}

public Action:CMD_RemoveVespene(client,args)
{
    if(args!=2)
        ReplyToCommand(client,"[SC] %t", "sc_removevespene");
    else
    {
        TraceInto("Console", "CMD_RemoveVespene");

        decl String:match[MAX_TARGET_LENGTH];
        GetCmdArg(1,match,sizeof(match));

        decl String:buf[32];
        GetCmdArg(2,buf,sizeof(buf));

        new amount=StringToInt(buf);
        if (amount<0)
            amount=0;

        new bool:tn_is_ml;
        new target_list[MAXPLAYERS];
        new String:target_name[MAX_TARGET_LENGTH];
        new count = ProcessTargetString(match, client, target_list, sizeof(target_list),
                                        COMMAND_FILTER_NO_BOTS, target_name,
                                        sizeof(target_name), tn_is_ml);
        if (count <= COMMAND_TARGET_NONE)
            ReplyToTargetError(client, count);
        else
        {
            for(new x=0;x<count;x++)
            {
                new index = target_list[x];
                if (IsValidClient(index))
                {
                    new vespene = GetVespene(index)-amount;
                    if (vespene < 0)
                        vespene = 0;
                    else if (vespene > g_iMaxVespene && g_iMaxVespene > 0)
                        vespene = g_iMaxVespene;

                    SetVespene(index,vespene);
                    LogAction(client, index, "[SC] %L removed %d vespene from %L", client, amount, index);
                    ReplyToCommand(client,"[SC] %t", "AdminRemovedVespene", amount, index);
                    CPrintToChat(index, "{green}[SC]{default} %t", "PlayerForfeitedVespene", client, amount);
                }
            }
        }

        TraceReturn();
    }
    return Plugin_Handled;
}

public Action:CMD_SetEnergy(client,args)
{
    if(args!=2)
        ReplyToCommand(client,"[SC] %t", "sc_setenergy");
    else
    {
        TraceInto("Console", "CMD_SetEnergy");

        decl String:match[MAX_TARGET_LENGTH];
        GetCmdArg(1,match,sizeof(match));

        decl String:buf[32];
        GetCmdArg(2,buf,sizeof(buf));

        new energy=StringToInt(buf);
        if (energy<0)
            energy=0;

        new bool:tn_is_ml;
        new target_list[MAXPLAYERS];
        new String:target_name[MAX_TARGET_LENGTH];
        new count = ProcessTargetString(match, client, target_list, sizeof(target_list),
                                        COMMAND_FILTER_NO_BOTS, target_name,
                                        sizeof(target_name), tn_is_ml);
        if (count <= COMMAND_TARGET_NONE)
            ReplyToTargetError(client, count);
        else
        {
            for(new x=0;x<count;x++)
            {
                new index = target_list[x];
                if (IsValidClient(index))
                {
                    SetEnergy(index,energy);
                    LogAction(client, index, "[SC] %L set %L's energy to %d", client, index, energy);
                    ReplyToCommand(client,"[SC] %t", "AdminSetEnergy", index, energy);
                    CPrintToChat(index, "{green}[SC]{default} %t", "PlayersEnergyWasSet", client, energy);
                }
            }
        }

        TraceReturn();
    }
    return Plugin_Handled;
}

public Action:CMD_GiveEnergy(client,args)
{
    if(args!=2)
        ReplyToCommand(client,"[SC] %t", "sc_giveenergy");
    else
    {
        TraceInto("Console", "CMD_GiveEnergy");

        decl String:match[MAX_TARGET_LENGTH];
        GetCmdArg(1,match,sizeof(match));

        decl String:buf[32];
        GetCmdArg(2,buf,sizeof(buf));

        new amount=StringToInt(buf);
        if (amount<0)
            amount=0;

        new bool:tn_is_ml;
        new target_list[MAXPLAYERS];
        new String:target_name[MAX_TARGET_LENGTH];
        new count = ProcessTargetString(match, client, target_list, sizeof(target_list),
                                        COMMAND_FILTER_NO_BOTS, target_name,
                                        sizeof(target_name), tn_is_ml);
        if (count <= COMMAND_TARGET_NONE)
            ReplyToTargetError(client, count);
        else
        {
            for(new x=0;x<count;x++)
            {
                new index = target_list[x];
                if (IsValidClient(index))
                {
                    new energy = GetEnergy(index)+amount;
                    if (energy < 0)
                        energy = 0;

                    SetEnergy(index,energy);
                    LogAction(client, index, "[SC] %L gave %L %d energy", client, index, amount);
                    ReplyToCommand(client,"[SC] %t", "AdminGaveEnergy", index, amount);
                    CPrintToChat(index, "{green}[SC]{default} %t", "PlayerReceivedEnergy", client, amount);
                }
            }
        }

        TraceReturn();
    }
    return Plugin_Handled;
}

public Action:CMD_RemoveEnergy(client,args)
{
    if(args!=2)
        ReplyToCommand(client,"[SC] %t", "sc_removeenergy");
    else
    {
        TraceInto("Console", "CMD_RemoveEnergy");

        decl String:match[MAX_TARGET_LENGTH];
        GetCmdArg(1,match,sizeof(match));

        decl String:buf[32];
        GetCmdArg(2,buf,sizeof(buf));

        new amount=StringToInt(buf);
        if (amount<0)
            amount=0;

        new bool:tn_is_ml;
        new target_list[MAXPLAYERS];
        new String:target_name[MAX_TARGET_LENGTH];
        new count = ProcessTargetString(match, client, target_list, sizeof(target_list),
                                        COMMAND_FILTER_NO_BOTS, target_name,
                                        sizeof(target_name), tn_is_ml);
        if (count <= COMMAND_TARGET_NONE)
            ReplyToTargetError(client, count);
        else
        {
            for(new x=0;x<count;x++)
            {
                new index = target_list[x];
                if (IsValidClient(index))
                {
                    new energy = GetEnergy(index)-energy;
                    if (energy < 0)
                        energy = 0;

                    SetEnergy(index,energy);
                    LogAction(client, index, "[SC] %L removed %d energy from %L", client, amount, index);
                    ReplyToCommand(client,"[SC] %t", "AdminRemovedEnergy", amount, index);
                    CPrintToChat(index, "{green}[SC]{default} %t", "PlayerForfeitedEnergy", client, amount);
                }
            }
        }

        TraceReturn();
    }
    return Plugin_Handled;
}

public Action:CMD_FixLevels(client,args)
{
    FixLevels(client);
    ReplyToCommand(client,"[SC] %t", "FixLevels");
    return Plugin_Handled;
}
