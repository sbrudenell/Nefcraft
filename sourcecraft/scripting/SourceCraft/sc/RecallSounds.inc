/**
 * vim: set ai et ts=4 sw=4 syntax=sourcepawn :
 * File: RecallSounds.inc
 * Description: Define SourceCRaft Recall Sounds
 * Author(s): -=|JFH|=-Naris
 */
 
#if defined _RecallSounds_included
 #endinput
#endif
#define _RecallSounds_included

#include <ResourceManager>

#if !defined buttonWav
    stock const String:buttonWav[] = "buttons/button14.wav";
#endif

#if !defined deniedWav
    stock const String:deniedWav[] = "sc/buzz.wav";
#endif

stock const String:recallSrcWav[] = "sc/pshbld00.wav";
stock const String:recallQryWav[] = "sc/pshbld01.wav";
stock const String:recallDstWav[] = "sc/pshbld03.wav";
stock const String:recallErrWav[] = "sc/pprerr00.wav";

stock SetupRecallSounds(bool:precache, bool:preload=false)
{
    SetupSound(recallSrcWav, true, precache, preload);
    SetupSound(recallQryWav, true, precache, preload);
    SetupSound(recallDstWav, true, precache, preload);
    SetupSound(recallErrWav, true, precache, preload);
}
