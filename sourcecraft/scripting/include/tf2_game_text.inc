/**
 * vim: set ai et ts=4 sw=4 syntax=sourcepawn :
 * File: tf2_game_text.inc
 * Description: Display a game_text_tf message
 * Author(s): Naris (Murray Wilson)
 */
 
#if defined _tf2_game_text_included
 #endinput
#endif
#define _tf2_game_text_included

stock tf2_game_text(const String:message[], const String:icon[]="")
{
	new Text_Ent = CreateEntityByName("game_text_tf");
    if (Text_Ent > 0 && IsValidEdict(Text_Ent))
    {
        DispatchKeyValue(Text_Ent,"message",message);
        DispatchKeyValue(Text_Ent,"display_to_team","0");
        DispatchKeyValue(Text_Ent,"icon", icon[0] ? icon : "leaderboard_dominated");
        DispatchKeyValue(Text_Ent,"targetname","game_text1");
        DispatchKeyValue(Text_Ent,"background","0");
        DispatchSpawn(Text_Ent);

        AcceptEntityInput(Text_Ent, "Display", Text_Ent, Text_Ent);

        CreateTimer(10.0, KillGameText, Text_Ent);
    }

	return Plugin_Handled;
}

public Action:KillGameText(Handle:timer, any:ent)
{
    if (IsValidEntity(ent))
    {
        decl String:classname[50];
        if (GetEdictClassname(ent, classname, sizeof(classname)) &&
            StrEqual(classname, "game_text_tf", false))
        {
            AcceptEntityInput(ent, "kill");
        }
    }
	return Plugin_Stop;
}
