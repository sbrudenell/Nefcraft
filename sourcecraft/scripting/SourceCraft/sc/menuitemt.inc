/*
 * vim: set ai et ts=4 sw=4 syntax=sourcepawn :
 * File: menuitemt.inc
 * Description: Add a Translated menu item to a menu.
 * Author: Naris (Murray Wilson)
 */

#if defined _menuitemt_included
 #endinput
#endif
#define _menuitemt_included

/**
 * Appends a new item to the end of a menu, after translating it.
 *
 * @param menu				Menu Handle.
 * @param info				Item information string.
 * @param display			Default item display string to be translated.
 * @param lang              Language/Client to translate display for.
 * @param style				Drawing style flags.  Anything other than DEFAULT or 
 *							DISABLED will be completely ignored when paginating.
 * @return					True on success, false on failure.
 * @error					Invalid Handle or item limit reached.
 */
stock AddMenuItemT(Handle:menu, 
					const String:info[], 
					const String:display[],
                    lang=LANG_SERVER,
					style=ITEMDRAW_DEFAULT)
{
    decl String:translated[128];
    Format(translated, sizeof(translated), "%T", display, lang);
    return AddMenuItem(menu, info, translated, style);
}
