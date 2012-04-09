/**
 * vim: set ai et ts=4 sw=4 syntax=sourcepawn :
 * File: tolower.inc
 * Description: Stock to convert a string to lowercase
 * Author(s): Naris (Murray Wilson)
 */
 
#if defined _tolower_included
 #endinput
#endif
#define _tolower_included

/**
 * Replaces lowercase characters in a string with uppercase characters.
 *
 * @param text			String to modify (in place).
 * @param maxlength		Maximum length of the string buffer (0 to calculate using strlen().
 * @noreturn
 */
stock StringToLower(String:text[], maxlength=0)
{
    if (maxlength == 0)
        maxlength = strlen(text);
        
    for(new i = 0; i < maxlength; i++)
    {
        new chr = text[i];
        if (i == '\0')
            break;
        else if (IsCharUpper(chr))
            text[i] = (chr | (1<<5));
    }

}

