/**
 * vim: set ai et ts=4 sw=4 syntax=sourcepawn :
 * File: display.inc
 * Description: Display information on Damage and Kills
 * Author(s): Naris (Murray Wilson)
 */

#if defined _display_included
 #endinput
#endif
#define _display_included

DisplayDamage(client, victim, hp, const String:weapon[],
              const String:desc[], bool:no_translate=false)
{
    if (victim > 0)
    {
        new bool:clientInGame;
        new bool:clientIsFake;
        decl String:steamID[64];
        decl String:team[64];

        if (IsValidClient(client))
        {
            GetClientAuthString(client,steamID,sizeof(steamID));
            GetTeamName(GetClientTeam(client), team, sizeof(team));
            clientIsFake = IsFakeClient(client);
            clientInGame = true;
        }
        else
        {
            clientIsFake = false;
            clientInGame = false;
            steamID[0] = '\0';
            team[0] = '\0';
        }

        new bool:victimInGame;
        new bool:victimIsFake;
        decl String:victimID[64];
        decl String:victimTeam[64];

        if (victim == client)
        {
            strcopy(victimID, sizeof(victimID), steamID);
            strcopy(victimTeam, sizeof(victimTeam), team);
            victimIsFake = clientIsFake;
            victimInGame = clientInGame;
        }
        else if (IsValidClient(victim))
        {
            GetClientAuthString(victim,victimID,sizeof(victimID));
            GetTeamName(GetClientTeam(victim), victimTeam, sizeof(victimTeam));
            victimIsFake = IsFakeClient(victim);
            victimInGame = true;
        }
        else
        {
            victimIsFake = false;
            victimInGame = false;
            victimTeam[0] = '\0';
            victimID[0] = '\0';
        }

        if (clientInGame)
        {
            if (weapon[0])
            {
               LogToGame("\"%N<%d><%s><%s>\" damaged \"%N<%d><%s><%s>\" for %d with %s",
                          client, client, steamID, team, victim, victim, victimID, victimTeam, hp, weapon);
            }
            else
            {
               LogToGame("\"%N<%d><%s><%s>\" damaged \"%N<%d><%s><%s>\" for %d",
                          client, client, steamID, team, victim, victim, victimID, victimTeam, hp);
            }

            if (!clientIsFake)
            {
                if (weapon[0])
                {
                    PrintToConsole(client, "[SC] \"%N<%d><%s><%s>\" damaged \"%N<%d><%s><%s>\" for %d with %s",
                                   client, client, steamID, team, victim, victim, victimID, victimTeam, hp, weapon);
                }
                else
                {
                    PrintToConsole(client, "[SC] \"%N<%d><%s><%s>\" damaged \"%N<%d><%s><%s>\" for %d",
                                   client, client, steamID, team, victim, victim, victimID, victimTeam, hp);
                }
            }

            if (victim != client && victimInGame && !victimIsFake)
            {
                if (weapon[0])
                {
                    PrintToConsole(victim, "[SC] \"%N<%d><%s><%s>\" damaged \"%N<%d><%s><%s>\" for %d with %s",
                                   client, client, steamID, team, victim, victim, victimID, victimTeam, hp, weapon);
                }
                else
                {
                    PrintToConsole(victim, "[SC] \"%N<%d><%s><%s>\" damaged \"%N<%d><%s><%s>\" for %d",
                                   client, client, steamID, team, victim, victim, victimID, victimTeam, hp);
                }
            }
        }
        else if (victimInGame)
        {
            if (weapon[0])
            {
                LogToGame("\"<%d><%s><%s>\" damaged \"%N<%d><%s><%s>\" for %d with %s",
                          client, steamID, team, victim, victim, victimID, victimTeam, hp, weapon);
            }
            else
            {
                LogToGame("\"<%d><%s><%s>\" damaged \"%N<%d><%s><%s>\" for %d",
                          client, steamID, team, victim, victim, victimID, victimTeam, hp);
            }

            if (!victimIsFake)
            {
                if (weapon[0])
                {
                    PrintToConsole(victim, "[SC] \"<%d><%s><%s>\" damaged \"%N<%d><%s><%s>\" for %d with %s",
                                   client, steamID, team, victim, victim, victimID, victimTeam, hp, weapon);
                }
                else
                {
                    PrintToConsole(victim, "[SC] \"<%d><%s><%s>\" damaged \"%N<%d><%s><%s>\" for %d",
                                   client, steamID, team, victim, victim, victimID, victimTeam, hp);
                }
            }
        }

        if (victim == client)
        {
            if (victimInGame && !victimIsFake &&
                (g_ClientSettingsBits[victim] & Display_Damage_Taken) == NotSet)
            {
                new String:description[NAME_STRING_LENGTH];
                if (desc[0] == '%')
                    Format(description, sizeof(description), "%T", desc[1], victim);
                else if (desc[0] == '\0' && weapon[0] != '\0' && !no_translate)
                    Format(description, sizeof(description), "%T", weapon, victim);
                else
                    strcopy(description, sizeof(description), desc);

                if (description[0])
                {
                    CPrintToChat(victim, "{green}[SC]{default} %t", "DamagedSelfWith",
                                         hp, description);
                }
                else
                {
                    CPrintToChat(victim, "{green}[SC]{default} %t", "DamagedSelf", hp);
                }
            }
        }
        else
        {
            if (victimInGame && !victimIsFake &&
                (g_ClientSettingsBits[victim] & Display_Damage_Taken) == NotSet)
            {
                new String:description[NAME_STRING_LENGTH];
                if (desc[0] == '%')
                    Format(description, sizeof(description), "%T", desc[1], victim);
                else if (desc[0] == '\0' && weapon[0] != '\0' && !no_translate)
                    Format(description, sizeof(description), "%T", weapon, victim);
                else
                    strcopy(description, sizeof(description), desc);

                if (clientInGame)
                {
                    if (description[0])
                    {
                        CPrintToChat(victim, "{green}[SC]{default} %t", "DamagedByWith",
                                             client, hp, description);
                    }
                    else
                    {
                        CPrintToChat(victim, "{green}[SC]{default} %t", "DamagedBy",
                                             client, hp);
                    }
                }
                else
                {
                    if (desc[0])
                    {
                        CPrintToChat(victim, "{green}[SC]{default} %t", "DamagedWith",
                                             hp, desc);
                    }
                    else
                    {
                        CPrintToChat(victim, "{green}[SC]{default} %t", "Damaged", hp);
                    }
                }
            }

            if (clientInGame && !clientIsFake &&
                (g_ClientSettingsBits[victim] & Display_Damage_Done) == NotSet)
            {
                new String:description[NAME_STRING_LENGTH];
                if (desc[0] == '%')
                    Format(description, sizeof(description), "%T", desc[1], client);
                else if (desc[0] == '\0' && weapon[0] != '\0' && !no_translate)
                    Format(description, sizeof(description), "%T", weapon, client);
                else
                    strcopy(description, sizeof(description), desc);

                if (description[0])
                {
                    CPrintToChat(client, "{green}[SC]{default} %t", "YouDamagedWith",
                                         victim, hp, description);
                }
                else
                {
                    CPrintToChat(client, "{green}[SC]{default} %t", "YouDamaged",
                                         victim, hp);
                }
            }
        }
    }
}

DisplayKill(client, victim, xp=0, const String:weapon[]="",
            const String:desc[]="", bool:no_translate=false)
{
    if (victim > 0)
    {
        new bool:clientInGame;
        new bool:clientIsFake;
        decl String:steamID[64];
        decl String:team[64];

        if (IsValidClient(client))
        {
            GetClientAuthString(client,steamID,sizeof(steamID));
            GetTeamName(GetClientTeam(client), team, sizeof(team));
            clientIsFake = IsFakeClient(client);
            clientInGame = true;
        }
        else
        {
            clientIsFake = false;
            clientInGame = false;
            steamID[0] = 0;
            team[0] = 0;
        }

        new bool:victimInGame;
        new bool:victimIsFake;
        decl String:victimID[64];
        decl String:victimTeam[64];

        if (victim == client)
        {
            strcopy(victimID, sizeof(victimID), steamID);
            strcopy(victimTeam, sizeof(victimTeam), team);
            victimIsFake = clientIsFake;
            victimInGame = clientInGame;
        }
        else if (IsValidClient(victim))
        {
            GetClientAuthString(victim,victimID,sizeof(victimID));
            GetTeamName(GetClientTeam(victim), victimTeam, sizeof(victimTeam));
            victimIsFake = IsFakeClient(victim);
            victimInGame = true;
        }
        else
        {
            victimIsFake = false;
            victimInGame = false;
            victimTeam[0] = 0;
            victimID[0] = 0;
        }

        if (clientInGame)
        {
            if (weapon[0])
            {
                if (xp != 0)
                {
                    LogToGame("\"%N<%d><%s><%s>\" killed \"%N<%d><%s><%s>\" with %s (%d XP)",
                              client, client, steamID, team, victim, victim, victimID, victimTeam, weapon,  xp);
                }
                else
                {
                    LogToGame("\"%N<%d><%s><%s>\" killed \"%N<%d><%s><%s>\" with %s",
                              client, client, steamID, team, victim, victim, victimID, victimTeam, weapon);
                }
            }
            else
            {
                if (xp != 0)
                {
                    LogToGame("\"%N<%d><%s><%s>\" killed \"%N<%d><%s><%s>\" (%d XP)",
                              client, client, steamID, team, victim, victim, victimID, victimTeam,  xp);
                }
                else
                {
                    LogToGame("\"%N<%d><%s><%s>\" killed \"%N<%d><%s><%s>\"",
                              client, client, steamID, team, victim, victim, victimID, victimTeam);
                }
            }

            if (!clientIsFake)
            {
                if (weapon[0])
                {
                    if (xp != 0)
                    {
                        PrintToConsole(client, "[SC] \"%N<%d><%s><%s>\" killed \"%N<%d><%s><%s>\" with %s (%d XP)",
                                       client, client, steamID, team, victim, victim, victimID, victimTeam, weapon,  xp);
                    }
                    else
                    {
                        PrintToConsole(client, "[SC] \"%N<%d><%s><%s>\" killed \"%N<%d><%s><%s>\" with %s",
                                       client, client, steamID, team, victim, victim, victimID, victimTeam, weapon);
                    }
                }
                else
                {
                    if (xp != 0)
                    {
                        PrintToConsole(client, "[SC] \"%N<%d><%s><%s>\" killed \"%N<%d><%s><%s>\" (%d XP)",
                                       client, client, steamID, team, victim, victim, victimID, victimTeam,  xp);
                    }
                    else
                    {
                        PrintToConsole(client, "[SC] \"%N<%d><%s><%s>\" killed \"%N<%d><%s><%s>\"",
                                       client, client, steamID, team, victim, victim, victimID, victimTeam);
                    }
                }
            }

            if (victim != client && victimInGame && !victimIsFake)
            {
                if (weapon[0])
                {
                    if (xp != 0)
                    {
                        PrintToConsole(victim, "[SC] \"%N<%d><%s><%s>\" killed \"%N<%d><%s><%s>\" with %s (%d XP)",
                                       client, client, steamID, team, victim, victim, victimID, victimTeam, weapon,  xp);
                    }
                    else
                    {
                        PrintToConsole(victim, "[SC] \"%N<%d><%s><%s>\" killed \"%N<%d><%s><%s>\" with %s",
                                       client, client, steamID, team, victim, victim, victimID, victimTeam, weapon);
                    }
                }
                else
                {
                    if (xp != 0)
                    {
                        PrintToConsole(victim, "[SC] \"%N<%d><%s><%s>\" killed \"%N<%d><%s><%s>\" (%d XP)",
                                       client, client, steamID, team, victim, victim, victimID, victimTeam,  xp);
                    }
                    else
                    {
                        PrintToConsole(victim, "[SC] \"%N<%d><%s><%s>\" killed \"%N<%d><%s><%s>\"",
                                       client, client, steamID, team, victim, victim, victimID, victimTeam);
                    }
                }
            }
        }
        else if (victimInGame)
        {
            if (weapon[0])
            {
                if (xp != 0)
                {
                    LogToGame("\"<%d><%s><%s>\" killed \"%N<%d><%s><%s>\" with %s (%d XP)",
                              client, steamID, team, victim, victim, victimID, victimTeam, weapon,  xp);
                }
                else
                {
                    LogToGame("\"<%d><%s><%s>\" killed \"%N<%d><%s><%s>\" with %s",
                              client, steamID, team, victim, victim, victimID, victimTeam, weapon);
                }
            }
            else
            {
                if (xp != 0)
                {
                    LogToGame("\"<%d><%s><%s>\" killed \"%N<%d><%s><%s>\" (%d XP)",
                              client, steamID, team, victim, victim, victimID, victimTeam,  xp);
                }
                else
                {
                    LogToGame("\"<%d><%s><%s>\" killed \"%N<%d><%s><%s>\"",
                              client, steamID, team, victim, victim, victimID, victimTeam);
                }
            }

            if (victimInGame && !victimIsFake)
            {
                if (weapon[0])
                {
                    if (xp != 0)
                    {
                        PrintToConsole(victim, "[SC] \"<%d><%s><%s>\" killed \"%N<%d><%s><%s>\" with %s (%d XP)",
                                       client, steamID, team, victim, victim, victimID, victimTeam, weapon,  xp);
                    }
                    else
                    {
                        PrintToConsole(victim, "[SC] \"<%d><%s><%s>\" killed \"%N<%d><%s><%s>\" with %s",
                                       client, steamID, team, victim, victim, victimID, victimTeam, weapon);
                    }
                }
                else
                {
                    if (xp != 0)
                    {
                        PrintToConsole(victim, "[SC] \"<%d><%s><%s>\" killed \"%N<%d><%s><%s>\" (%d XP)",
                                       client, steamID, team, victim, victim, victimID, victimTeam,  xp);
                    }
                    else
                    {
                        PrintToConsole(victim, "[SC] \"<%d><%s><%s>\" killed \"%N<%d><%s><%s>\"",
                                       client, steamID, team, victim, victim, victimID, victimTeam);
                    }
                }
            }
        }

        if (victim == client)
        {
            if (victimInGame && !victimIsFake &&
                (g_ClientSettingsBits[victim] & Display_Deaths) == NotSet)
            {
                new String:description[NAME_STRING_LENGTH];
                if (desc[0] == '%')
                    Format(description, sizeof(description), "%T", desc[1], victim);
                else if (desc[0] == '\0' && weapon[0] != '\0' && !no_translate)
                    Format(description, sizeof(description), "%T", weapon, victim);
                else
                    strcopy(description, sizeof(description), desc);

                if (description[0])
                {
                    CPrintToChat(victim, "{green}[SC]{default} %t", "KilledSelfWith",
                                         description);
                }
                else
                {
                    CPrintToChat(victim, "{green}[SC]{default} %t", "KilledSelf");
                }
            }
        }
        else
        {
            if (victimInGame && !victimIsFake &&
                (g_ClientSettingsBits[victim] & Display_Deaths) == NotSet)
            {
                new String:description[NAME_STRING_LENGTH];
                if (desc[0] == '%')
                    Format(description, sizeof(description), "%T", desc[1], victim);
                else if (desc[0] == '\0' && weapon[0] != '\0' && !no_translate)
                    Format(description, sizeof(description), "%T", weapon, victim);
                else
                    strcopy(description, sizeof(description), desc);

                if (description[0])
                {
                    CPrintToChat(victim, "{green}[SC]{default} %t", "KilledByWith",
                                         client, description);
                }
                else
                {
                    CPrintToChat(victim, "{green}[SC]{default} %t", "KilledBy", client);
                }
            }

            if (clientInGame && !clientIsFake &&
                (g_ClientSettingsBits[client] & Display_Kills) == NotSet)
            {
                new String:description[NAME_STRING_LENGTH];
                if (desc[0] == '%')
                    Format(description, sizeof(description), "%T", desc[1], client);
                else if (desc[0] == '\0' && weapon[0] != '\0' && !no_translate)
                    Format(description, sizeof(description), "%T", weapon, client);
                else
                    strcopy(description, sizeof(description), desc);

                if (description[0])
                {
                    if (xp)
                    {
                        CPrintToChat(client, "{green}[SC]{default} %t", "GainedXPForKillWith",
                                             xp, victim, description);
                    }
                    else
                    {
                        CPrintToChat(client, "{green}[SC]{default} %t", "YouKilledWith",
                                             victim, description);
                    }
                }
                else
                {
                    if (xp)
                    {
                        CPrintToChat(client, "{green}[SC]{default} %t", "GainedXPForKill",
                                             xp, victim);
                    }
                    else
                    {
                        CPrintToChat(client, "{green}[SC]{default} %t", "YouKilled",
                                             victim);
                    }
                }
            }
        }
    }
}

DisplayMessage(client, Settings:flags, const String:fmt[], any:...)
{
    decl String:buffer[1024];

    SetGlobalTransTarget(client);
    VFormat(buffer, sizeof(buffer), fmt, 4);

    // Debug
    if (StrContains(buffer, "%") >= 0)
    {
        LogError("Attempt to Display bad Message to %N %s",
                 ValidClientIndex(client), buffer);
        return;
    }

    new bool:displayToClient = (IsValidClient(client) && !IsFakeClient(client));
    if (displayToClient)
    {
        if ((g_ClientSettingsBits[client] & flags) == NotSet)
            CPrintToChat(client, "{green}[SC]{default} %s", buffer);
    }

    // Convert control characters (color codes) to spaces
    for (new i = strlen(buffer)-1; i >= 0; i--)
    {
        if (buffer{i} < ' ')
            buffer{i} = ' ';
    }

    // Remove color codes
    ReplaceString(buffer, sizeof(buffer), "{default}", "", false);
    ReplaceString(buffer, sizeof(buffer), "{green}", "", false);
    ReplaceString(buffer, sizeof(buffer), "{lightgreen}", "", false);
    ReplaceString(buffer, sizeof(buffer), "{red}", "", false);
    ReplaceString(buffer, sizeof(buffer), "{blue}", "", false);
    ReplaceString(buffer, sizeof(buffer), "{olive}", "", false);
    ReplaceString(buffer, sizeof(buffer), "{custom}", "", false);

    if (displayToClient)
    {
        PrintToConsole(client, buffer);

        if (StrContains(buffer, "Your", false) != -1)
        {
            decl String:owner[MAX_NAME_LENGTH];
            GetClientName(client,owner,sizeof(owner));
            StrCat(owner, sizeof(owner), "'s");
            ReplaceString(buffer, sizeof(buffer), "Your", owner, false);
        }

        if (StrContains(buffer, "You have", false) != -1)
        {
            decl String:name[MAX_NAME_LENGTH];
            GetClientName(client,name,sizeof(name));
            StrCat(name, sizeof(name), " has");
            ReplaceString(buffer, sizeof(buffer), "You have", name, false);
        }

        if (StrContains(buffer, "You", false) != -1)
        {
            decl String:name[MAX_NAME_LENGTH];
            GetClientName(client,name,sizeof(name));
            ReplaceString(buffer, sizeof(buffer), "You", name, false);
        }
    }

    LogToGame(buffer);
    PrintToServer(buffer);
}
