/**
 * vim: set ai et ts=4 sw=4 syntax=sourcepawn :
 * File: string_to.inc
 * Description: Functions to convert Strings To integers, floats, arrays, etc...
 * Author: Naris (Murray Wilson)
 */

#if defined _string_to_included
 #endinput
#endif
#define _string_to_included

stock StringToIntegerArray(const String:buffer[], Handle:array)
{
    new count=0;
    decl String:num[16];
    for (new index=0, pos=0; index >= 0; pos += index)
    {
        index = BreakString(buffer[pos], num, sizeof(num));
        PushArrayCell(array,StringToInt(num));
        count++;
    }
    return count;
}

stock StringToFloatArray(const String:buffer[], Handle:array)
{
    new count=0;
    decl String:num[16];
    for (new index=0, pos=0; index >= 0; pos += index)
    {
        index = BreakString(buffer[pos], num, sizeof(num));
        PushArrayCell(array,StringToFloat(num));
        count++;
    }
    return count;
}

stock StringToIntegers(const String:buffer[], array[], size)
{
    new count=0;
    decl String:num[16];
    for (new index=0, pos=0; index >= 0; pos += index)
    {
        index = BreakString(buffer[pos], num, sizeof(num));
        array[count] = StringToInt(num);
        if (++count >= size)
            break;
    }
    return count;
}

stock StringToFloats(const String:buffer[], Float:array[], size)
{
    new count=0;
    decl String:num[16];
    for (new index=0, pos=0; index >= 0; pos += index)
    {
        index = BreakString(buffer[pos], num, sizeof(num));
        array[count] = StringToFloat(num);
        if (++count >= size)
            break;
    }
    return count;
}
