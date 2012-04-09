/**
 * vim: set ai et ts=4 sw=4 syntax=sourcepawn :
 * File: topmessage.inc
 * Description: stock for SendTopMessage
 * Author(s): -=|JFH|=-Naris
 */

#if defined _topmessage_included
 #endinput
#endif
#define _topmessage_included

stock SendTopMessage(client, level, time, r, g, b, a, String:text[], any:...)
{
	new String:message[100];
	VFormat(message,sizeof(message),text, 9);
	
	new Handle:kv = CreateKeyValues("message", "title", message);
	KvSetColor(kv, "color", r, g, b, a);
	KvSetNum(kv, "level", level);
	KvSetNum(kv, "time", time);

	CreateDialog(client, kv, DialogType_Msg);

	CloseHandle(kv);
}
