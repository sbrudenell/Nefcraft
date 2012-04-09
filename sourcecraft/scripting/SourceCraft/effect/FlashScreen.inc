/**
 * vim: set ai et ts=4 sw=4 syntax=sourcepawn :
 * File: FlashScreen.inc
 * Description: FlashScreen Effect
 * Incorporated by: Naris (Murray Wilson)
 * Thanks to: Anthony Iacono (Pimpinjuice)
 *
 */

#if defined _Flash_included
#endinput
#endif
#define _Flash_included

#include "effect/Fade"

// The following are color presets for the FlashScreen function
#define RGBA_COLOR_RED		{255,0,0,3}
#define RGBA_COLOR_GREEN	{0,255,0,3}
#define RGBA_COLOR_BLUE		{0,0,255,3}
#define RGBA_COLOR_YELLOW	{255,255,0,3}
#define RGBA_COLOR_ORANGE	{255,69,0,3}
#define RGBA_COLOR_PURPLE	{128,0,128,3}
#define RGBA_COLOR_CYAN		{255,0,255,3}
#define RGBA_COLOR_WHITE	{255,255,255,3}
#define RGBA_COLOR_BLACK	{0,0,0,3}
#define RGBA_COLOR_GREY		{128,128,128,3}
#define RGBA_COLOR_PINK		{255,20,147,3}
#define RGBA_COLOR_MAROON	{128,0,0,3}
#define RGBA_COLOR_SKYBLUE	{135,206,25,3}
#define RGBA_COLOR_GOLD		{255,215,0,3}
#define RGBA_COLOR_BROWN	{139,69,19,3}
#define RGBA_COLOR_VIOLET	{238,130,238,3}

/**
 * Flashes a player's screen a certain color for a duration.
 * @param client: Client's index.
 * @param color[4]: RGBA value of the color, see above for presets.
 * @param duration: Length of flash, optional.
 * @param flags: Flags of Flash user message, optional.
 * @noreturn
 */
stock FlashScreen(client,const color[4],Float:fade=0.1,Float:hold=0.2,flags=FFADE_IN)
{
    FadeOne(client, RoundFloat(255.0*fade), RoundFloat(255.0*hold), color, flags);
}
