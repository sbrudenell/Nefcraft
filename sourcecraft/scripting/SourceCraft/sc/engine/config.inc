/*
 * vim: set ai et ts=4 sw=4 syntax=sourcepawn :
 * File: config.inc
 * Description: This file handles configuration files
 * Author(s): Naris (Murray Wilson)
 */

#include <string_to>

new Handle:vecLevelConfiguration     = INVALID_HANDLE;
new Handle:g_SourceCraftConfigHandle = INVALID_HANDLE;
new Handle:g_ItemConfigHandle        = INVALID_HANDLE;
new g_SourceCraftConfigLoaded        = CONFIG_NOT_LOADED;
new g_ItemConfigLoaded               = CONFIG_NOT_LOADED;

GetConfigNum(const String:key[], defvalue=0, fileId=MAIN,
             node=NONE, const String:section[]="")
{
    new value;
    if (fileId >= SHOPITEM)
    {
        new configLoaded;
        new Handle:raceHandle;
        new Handle:configHandle;
        decl String:nodeName[SHORT_STRING_LENGTH];

        if (fileId == SHOPITEM)
        {
            configHandle = LoadItemConfig(configLoaded);

            if (node >= 0)
            {
                new Handle:itemHandle = GetShopItemHandle(node);
                GetItemShortName(itemHandle,nodeName,sizeof(nodeName));
            }
        }
        else
        {
            raceHandle = GetRaceHandle(fileId);
            configHandle = GetRaceConfig(raceHandle, configLoaded);

            if (node >= 0)
                GetUpgradeShortName(raceHandle,node,nodeName,sizeof(nodeName));
        }

        if (configHandle == INVALID_HANDLE)
            ThrowError("Unable obtain config handle for fileId %d", fileId);
        else
        {
            if (node >= 0)
            {
                new bool:nodeFound;
                if (configLoaded == CONFIG_NOT_LOADED)
                    nodeFound = false;
                else
                {
                    KvRewind(configHandle);
                    nodeFound = KvJumpToKey(configHandle, nodeName, false);
                    if (nodeFound)
                    {
                        new bool:sectionFound;
                        if (section[0] != '\0')
                        {
                            sectionFound = KvJumpToKey(configHandle, section, false);
                            if (!sectionFound)
                                KvJumpToKey(configHandle, section, true);
                        }
                        else
                            sectionFound = true;

                        if (sectionFound)
                            value = KvGetNum(configHandle, key, defvalue);
                        else
                        {
                            KvSetNum(configHandle, key, value=defvalue);

                            if (fileId == SHOPITEM)
                                g_ItemConfigLoaded = CONFIG_WAS_UPDATED;
                            else
                                SetRaceConfigIsLoaded(raceHandle,CONFIG_WAS_UPDATED);
                        }
                    }
                    else
                    {
                        if (fileId == SHOPITEM)
                            g_ItemConfigLoaded = CONFIG_WAS_UPDATED;
                        else
                            SetRaceConfigIsLoaded(raceHandle,CONFIG_WAS_UPDATED);
                    }
                }

                if (!nodeFound)
                {
                    KvRewind(configHandle);
                    if (KvJumpToKey(configHandle, nodeName, true))
                    {
                        if (section[0] != '\0')
                        {
                            if (!KvJumpToKey(configHandle, section, true))
                            {
                                LogError("Unable to create section %s in node %s for fileId %d",
                                         section, nodeName, fileId);
                            }
                        }
                        KvSetNum(configHandle, key, value=defvalue);
                    }
                    else
                    {
                        LogError("Unable to create node %s for fileId %d",
                                 nodeName, fileId);
                    }
                }
            }
            else
            {
                if (((fileId == SHOPITEM) ? g_ItemConfigLoaded : GetRaceConfigIsLoaded(raceHandle)) == CONFIG_NOT_LOADED)
                {
                    if (section[0] != '\0')
                    {
                        if (!KvJumpToKey(configHandle, section, true))
                        {
                            LogError("Unable to create section %s in fileId %d",
                                     section, fileId);
                        }
                    }

                    KvSetNum(configHandle, key, value=defvalue);

                    if (fileId == SHOPITEM)
                        g_ItemConfigLoaded = CONFIG_WAS_UPDATED;
                    else
                        SetRaceConfigIsLoaded(raceHandle,CONFIG_WAS_UPDATED);
                }
                else
                {
                    new bool:sectionFound;
                    if (section[0] != '\0')
                    {
                        sectionFound = KvJumpToKey(configHandle, section, false);
                        if (!sectionFound)
                            KvJumpToKey(configHandle, section, true);
                    }
                    else
                        sectionFound = true;

                    if (sectionFound)
                        value = KvGetNum(configHandle, key, defvalue);
                    else
                    {
                        KvSetNum(configHandle, key, value=defvalue);

                        if (fileId == SHOPITEM)
                            g_ItemConfigLoaded = CONFIG_WAS_UPDATED;
                        else
                            SetRaceConfigIsLoaded(raceHandle,CONFIG_WAS_UPDATED);
                    }
                }
            }
        }
    }
    else
    {
        LoadSettings();
        KvRewind(g_SourceCraftConfigHandle);

        new bool:sectionFound;
        if (section[0] != '\0')
        {
            if (g_SourceCraftConfigLoaded == CONFIG_WAS_LOADED)
            {
                sectionFound = KvJumpToKey(g_SourceCraftConfigHandle, section, false);
                if (!sectionFound)
                    g_SourceCraftConfigLoaded = CONFIG_WAS_UPDATED;
            }
            else
                sectionFound = false;

            if (!sectionFound)
            {
                KvRewind(g_SourceCraftConfigHandle);
                if (!KvJumpToKey(g_SourceCraftConfigHandle, section, true))
                {
                    LogError("Unable to create %s section in configs/sourcecraft.cfg",
                             section);
                }
            }
        }
        else
            sectionFound = true;

        if (g_SourceCraftConfigLoaded == CONFIG_NOT_LOADED || !sectionFound)
            KvSetNum(g_SourceCraftConfigHandle, key, value=defvalue);
        else
            value = KvGetNum(g_SourceCraftConfigHandle, key, defvalue);
    }
    return value;
}

Float:GetConfigFloat(const String:key[], Float:defvalue=0.0, fileId=MAIN,
                     node=NONE, const String:section[]="")
{
    new Float:value;
    if (fileId >= SHOPITEM)
    {
        new configLoaded;
        new Handle:raceHandle;
        new Handle:configHandle;
        decl String:nodeName[SHORT_STRING_LENGTH];

        if (fileId == SHOPITEM)
        {
            configHandle = LoadItemConfig(configLoaded);

            if (node >= 0)
            {
                new Handle:itemHandle = GetShopItemHandle(node);
                GetItemShortName(itemHandle,nodeName,sizeof(nodeName));
            }
        }
        else
        {
            raceHandle = GetRaceHandle(fileId);
            configHandle = GetRaceConfig(raceHandle, configLoaded);

            if (node >= 0)
                GetUpgradeShortName(raceHandle,node,nodeName,sizeof(nodeName));
        }

        if (configHandle == INVALID_HANDLE)
            ThrowError("Unable obtain config handle for fileId %d", fileId);
        else
        {
            if (node >= 0)
            {
                new bool:nodeFound;
                if (configLoaded == CONFIG_NOT_LOADED)
                    nodeFound = false;
                else
                {
                    KvRewind(configHandle);
                    nodeFound = KvJumpToKey(configHandle, nodeName, false);
                    if (nodeFound)
                    {
                        new bool:sectionFound;
                        if (section[0] != '\0')
                        {
                            sectionFound = KvJumpToKey(configHandle, section, false);
                            if (!sectionFound)
                                KvJumpToKey(configHandle, section, true);
                        }
                        else
                            sectionFound = true;

                        if (sectionFound)
                            value = KvGetFloat(configHandle, key, defvalue);
                        else
                        {
                            KvSetFloat(configHandle, key, value=defvalue);

                            if (fileId == SHOPITEM)
                                g_ItemConfigLoaded = CONFIG_WAS_UPDATED;
                            else
                                SetRaceConfigIsLoaded(raceHandle,CONFIG_WAS_UPDATED);
                        }
                    }
                    else
                    {
                        if (fileId == SHOPITEM)
                            g_ItemConfigLoaded = CONFIG_WAS_UPDATED;
                        else
                            SetRaceConfigIsLoaded(raceHandle,CONFIG_WAS_UPDATED);
                    }
                }

                if (!nodeFound)
                {
                    KvRewind(configHandle);
                    if (KvJumpToKey(configHandle, nodeName, true))
                    {
                        if (section[0] != '\0')
                        {
                            if (!KvJumpToKey(configHandle, section, true))
                            {
                                LogError("Unable to create section %s in node %s for %d",
                                         section, nodeName, fileId);
                            }
                        }
                        KvSetFloat(configHandle, key, value=defvalue);
                    }
                    else
                    {
                        LogError("Unable to create node %s for %d",
                                 nodeName, fileId);
                    }
                }
            }
            else
            {
                if (((fileId == SHOPITEM) ? g_ItemConfigLoaded : GetRaceConfigIsLoaded(raceHandle)) == CONFIG_NOT_LOADED)
                {
                    if (section[0] != '\0')
                    {
                        if (!KvJumpToKey(configHandle, section, true))
                            LogError("Unable to create %s section", section);
                    }

                    KvSetFloat(configHandle, key, value=defvalue);

                    if (fileId == SHOPITEM)
                        g_ItemConfigLoaded = CONFIG_WAS_UPDATED;
                    else
                        SetRaceConfigIsLoaded(raceHandle,CONFIG_WAS_UPDATED);
                }
                else
                {
                    new bool:sectionFound;
                    if (section[0] != '\0')
                    {
                        sectionFound = KvJumpToKey(configHandle, section, false);
                        if (!sectionFound)
                            KvJumpToKey(configHandle, section, true);
                    }
                    else
                        sectionFound = true;

                    if (sectionFound)
                        value = KvGetFloat(configHandle, key, defvalue);
                    else
                    {
                        KvSetFloat(configHandle, key, value=defvalue);

                        if (fileId == SHOPITEM)
                            g_ItemConfigLoaded = CONFIG_WAS_UPDATED;
                        else
                            SetRaceConfigIsLoaded(raceHandle,CONFIG_WAS_UPDATED);
                    }
                }
            }
        }
    }
    else
    {
        LoadSettings();
        KvRewind(g_SourceCraftConfigHandle);

        new bool:sectionFound;
        if (section[0] != '\0')
        {
            if (g_SourceCraftConfigLoaded == CONFIG_WAS_LOADED)
            {
                sectionFound = KvJumpToKey(g_SourceCraftConfigHandle, section, false);
                if (!sectionFound)
                    g_SourceCraftConfigLoaded = CONFIG_WAS_UPDATED;
            }
            else
                sectionFound = false;

            if (!sectionFound)
            {
                KvRewind(g_SourceCraftConfigHandle);
                if (!KvJumpToKey(g_SourceCraftConfigHandle, section, true))
                {
                    LogError("Unable to create %s section in configs/sourcecraft.cfg",
                             section);
                }
            }
        }
        else
            sectionFound = true;

        if (g_SourceCraftConfigLoaded == CONFIG_NOT_LOADED || !sectionFound)
            KvSetFloat(g_SourceCraftConfigHandle, key, value=defvalue);
        else
            value = KvGetFloat(g_SourceCraftConfigHandle, key, defvalue);
    }
    return value;
}

GetConfigString(const String:key[], String:value[], maxlength,
                const String:defvalue[]="", fileId=MAIN,
                node=NONE, const String:section[]="")
{
    if (fileId >= SHOPITEM)
    {
        new configLoaded;
        new Handle:raceHandle;
        new Handle:configHandle;
        decl String:nodeName[SHORT_STRING_LENGTH];

        if (fileId == SHOPITEM)
        {
            configHandle = LoadItemConfig(configLoaded);

            if (node >= 0)
            {
                new Handle:itemHandle = GetShopItemHandle(node);
                GetItemShortName(itemHandle,nodeName,sizeof(nodeName));
            }
        }
        else
        {
            raceHandle = GetRaceHandle(fileId);
            configHandle = GetRaceConfig(raceHandle, configLoaded);

            if (node >= 0)
                GetUpgradeShortName(raceHandle,node,nodeName,sizeof(nodeName));
        }

        if (configHandle == INVALID_HANDLE)
            ThrowError("Unable obtain config handle for fileId %d", fileId);
        else
        {
            if (node >= 0)
            {
                new bool:nodeFound;
                if (configLoaded == CONFIG_NOT_LOADED)
                    nodeFound = false;
                else
                {
                    KvRewind(configHandle);
                    nodeFound = KvJumpToKey(configHandle, nodeName, false);
                    if (nodeFound)
                    {
                        new bool:sectionFound;
                        if (section[0] != '\0')
                        {
                            sectionFound = KvJumpToKey(configHandle, section, false);
                            if (!sectionFound)
                                KvJumpToKey(configHandle, section, true);
                        }
                        else
                            sectionFound = true;

                        if (sectionFound)
                            KvGetString(configHandle, key, value, maxlength, defvalue);
                        else
                        {
                            strcopy(value, maxlength, defvalue);
                            KvSetString(configHandle, key, value);

                            if (fileId == SHOPITEM)
                                g_ItemConfigLoaded = CONFIG_WAS_UPDATED;
                            else
                                SetRaceConfigIsLoaded(raceHandle,CONFIG_WAS_UPDATED);
                        }
                    }
                    else
                    {
                        if (fileId == SHOPITEM)
                            g_ItemConfigLoaded = CONFIG_WAS_UPDATED;
                        else
                            SetRaceConfigIsLoaded(raceHandle,CONFIG_WAS_UPDATED);
                    }
                }

                if (!nodeFound)
                {
                    KvRewind(configHandle);
                    if (KvJumpToKey(configHandle, nodeName, true))
                    {
                        if (section[0] != '\0')
                        {
                            if (!KvJumpToKey(configHandle, section, true))
                            {
                                LogError("Unable to create section %s in node %s for fileId %d",
                                         section, nodeName, fileId);
                            }
                        }

                        strcopy(value, maxlength, defvalue);
                        KvSetString(configHandle, key, value);
                    }
                    else
                    {
                        LogError("Unable to create node %s for fileId %d",
                                 nodeName, fileId);
                    }
                }
            }
            else
            {
                if (((fileId == SHOPITEM) ? g_ItemConfigLoaded : GetRaceConfigIsLoaded(raceHandle)) == CONFIG_NOT_LOADED)
                {
                    if (section[0] != '\0')
                    {
                        if (!KvJumpToKey(configHandle, section, true))
                        {
                            LogError("Unable to create section %s in fileId %d",
                                     section, fileId);
                        }
                    }

                    strcopy(value, maxlength, defvalue);
                    KvSetString(configHandle, key, value);

                    if (fileId == SHOPITEM)
                        g_ItemConfigLoaded = CONFIG_WAS_UPDATED;
                    else
                        SetRaceConfigIsLoaded(raceHandle,CONFIG_WAS_UPDATED);
                }
                else
                {
                    new bool:sectionFound;
                    if (section[0] != '\0')
                    {
                        sectionFound = KvJumpToKey(configHandle, section, false);
                        if (!sectionFound)
                            KvJumpToKey(configHandle, section, true);
                    }
                    else
                        sectionFound = true;

                    if (sectionFound)
                        KvGetString(configHandle, key, value, maxlength, defvalue);
                    else
                    {
                        strcopy(value, maxlength, defvalue);
                        KvSetString(configHandle, key, value);

                        if (fileId == SHOPITEM)
                            g_ItemConfigLoaded = CONFIG_WAS_UPDATED;
                        else
                            SetRaceConfigIsLoaded(raceHandle,CONFIG_WAS_UPDATED);
                    }
                }
            }
        }
    }
    else
    {
        LoadSettings();
        KvRewind(g_SourceCraftConfigHandle);

        new bool:sectionFound;
        if (section[0] != '\0')
        {
            if (g_SourceCraftConfigLoaded == CONFIG_WAS_LOADED)
            {
                sectionFound = KvJumpToKey(g_SourceCraftConfigHandle, section, false);
                if (!sectionFound)
                    g_SourceCraftConfigLoaded = CONFIG_WAS_UPDATED;
            }
            else
                sectionFound = false;

            if (!sectionFound)
            {
                KvRewind(g_SourceCraftConfigHandle);
                if (!KvJumpToKey(g_SourceCraftConfigHandle, section, true))
                {
                    LogError("Unable to create %s section in configs/sourcecraft.cfg",
                             section);
                }
            }
        }
        else
            sectionFound = true;

        if (g_SourceCraftConfigLoaded == CONFIG_NOT_LOADED || !sectionFound)
        {
            strcopy(value, maxlength, defvalue);
            KvSetString(g_SourceCraftConfigHandle, key, value);
        }
        else
            KvGetString(g_SourceCraftConfigHandle, key, value, maxlength, defvalue);
    }
}

GetConfigArray(const String:key[], any:value[], size, const any:defvalue[],
               fileId=MAIN, node=NONE, const String:section[]="")
{
    new count = size;
    decl String:buffer[LONG_STRING_LENGTH];
    decl String:defstring[LONG_STRING_LENGTH];

    // Build the default string
    for (new i = 0; i < size; i++)
    {
        if (i == 0)
            Format(defstring, sizeof(defstring), "%d", defvalue[i]);
        else
            Format(defstring, sizeof(defstring), "%s %d", defstring, defvalue[i]);
    }

    if (fileId >= SHOPITEM)
    {
        new configLoaded;
        new Handle:raceHandle;
        new Handle:configHandle;
        decl String:nodeName[SHORT_STRING_LENGTH];

        if (fileId == SHOPITEM)
        {
            configHandle = LoadItemConfig(configLoaded);

            if (node >= 0)
            {
                new Handle:itemHandle = GetShopItemHandle(node);
                GetItemShortName(itemHandle,nodeName,sizeof(nodeName));
            }
        }
        else
        {
            raceHandle = GetRaceHandle(fileId);
            configHandle = GetRaceConfig(raceHandle, configLoaded);

            if (node >= 0)
                GetUpgradeShortName(raceHandle,node,nodeName,sizeof(nodeName));
        }

        if (configHandle == INVALID_HANDLE)
            ThrowError("Unable obtain config handle for fileId %d", fileId);
        else
        {
            if (node >= 0)
            {
                new bool:nodeFound;
                if (configLoaded == CONFIG_NOT_LOADED)
                    nodeFound = false;
                else
                {
                    KvRewind(configHandle);
                    nodeFound = KvJumpToKey(configHandle, nodeName, false);
                    if (nodeFound)
                    {
                        new bool:sectionFound;
                        if (section[0] != '\0')
                        {
                            sectionFound = KvJumpToKey(configHandle, section, false);
                            if (!sectionFound)
                                KvJumpToKey(configHandle, section, true);
                        }
                        else
                            sectionFound = true;

                        if (sectionFound)
                        {
                            KvGetString(configHandle, key, buffer, sizeof(buffer), defstring);
                            count = StringToIntegers(buffer, value, size);
                        }
                        else
                        {
                            KvSetString(configHandle, key, defstring);
                            for (new i=0; i < size; i++)
                                value[i] = defvalue[i];

                            if (fileId == SHOPITEM)
                                g_ItemConfigLoaded = CONFIG_WAS_UPDATED;
                            else
                                SetRaceConfigIsLoaded(raceHandle,CONFIG_WAS_UPDATED);
                        }
                    }
                    else
                    {
                        if (fileId == SHOPITEM)
                            g_ItemConfigLoaded = CONFIG_WAS_UPDATED;
                        else
                            SetRaceConfigIsLoaded(raceHandle,CONFIG_WAS_UPDATED);
                    }
                }

                if (!nodeFound)
                {
                    KvRewind(configHandle);
                    if (KvJumpToKey(configHandle, nodeName, true))
                    {
                        if (section[0] != '\0')
                        {
                            if (!KvJumpToKey(configHandle, section, true))
                            {
                                LogError("Unable to create section %s in node %s for fileId %d",
                                         section, nodeName, fileId);
                            }
                        }
                        KvSetString(configHandle, key, defstring);
                        for (new i=0; i < size; i++)
                            value[i] = defvalue[i];
                    }
                    else
                    {
                        LogError("Unable to create node %s for fileId %d",
                                 nodeName, fileId);
                    }
                }
            }
            else
            {
                if (((fileId == SHOPITEM) ? g_ItemConfigLoaded : GetRaceConfigIsLoaded(raceHandle)) == CONFIG_NOT_LOADED)
                {
                    if (section[0] != '\0')
                    {
                        if (!KvJumpToKey(configHandle, section, true))
                        {
                            LogError("Unable to create section %s in fileId %d",
                                     section, fileId);
                        }
                    }

                    KvSetString(configHandle, key, defstring);
                    for (new i=0; i < size; i++)
                        value[i] = defvalue[i];

                    if (fileId == SHOPITEM)
                        g_ItemConfigLoaded = CONFIG_WAS_UPDATED;
                    else
                        SetRaceConfigIsLoaded(raceHandle,CONFIG_WAS_UPDATED);
                }
                else
                {
                    new bool:sectionFound;
                    if (section[0] != '\0')
                    {
                        sectionFound = KvJumpToKey(configHandle, section, false);
                        if (!sectionFound)
                            KvJumpToKey(configHandle, section, true);
                    }
                    else
                        sectionFound = true;

                    if (sectionFound)
                    {
                        KvGetString(configHandle, key, buffer, sizeof(buffer), defstring);
                        count = StringToIntegers(buffer, value, size);
                    }
                    else
                    {
                        KvSetString(configHandle, key, defstring);
                        for (new i=0; i < size; i++)
                            value[i] = defvalue[i];

                        if (fileId == SHOPITEM)
                            g_ItemConfigLoaded = CONFIG_WAS_UPDATED;
                        else
                            SetRaceConfigIsLoaded(raceHandle,CONFIG_WAS_UPDATED);
                    }
                }
            }
        }
    }
    else
    {
        LoadSettings();
        KvRewind(g_SourceCraftConfigHandle);

        new bool:sectionFound;
        if (section[0] != '\0')
        {
            if (g_SourceCraftConfigLoaded == CONFIG_WAS_LOADED)
            {
                sectionFound = KvJumpToKey(g_SourceCraftConfigHandle, section, false);
                if (!sectionFound)
                    g_SourceCraftConfigLoaded = CONFIG_WAS_UPDATED;
            }
            else
                sectionFound = false;

            if (!sectionFound)
            {
                KvRewind(g_SourceCraftConfigHandle);
                if (!KvJumpToKey(g_SourceCraftConfigHandle, section, true))
                {
                    LogError("Unable to create %s section in configs/sourcecraft.cfg",
                             section);
                }
            }
        }
        else
            sectionFound = true;

        if (g_SourceCraftConfigLoaded == CONFIG_NOT_LOADED || !sectionFound)
        {
            KvSetString(g_SourceCraftConfigHandle, key, defstring);
            for (new i=0; i < size; i++)
                value[i] = defvalue[i];
        }
        else
        {
            KvGetString(g_SourceCraftConfigHandle, key, buffer, sizeof(buffer), defstring);
            count = StringToIntegers(buffer, value, size);
        }
    }
    return count;
}

GetConfigFloatArray(const String:key[], Float:value[], size,
                    const Float:defvalue[], fileId=MAIN,
                    node=NONE, const String:section[]="")
{
    new count = size;
    decl String:buffer[LONG_STRING_LENGTH];
    decl String:defstring[LONG_STRING_LENGTH];

    // Build the default string
    for (new i = 0; i < size; i++)
    {
        if (i == 0)
            Format(defstring, sizeof(defstring), "%f", defvalue[i]);
        else
            Format(defstring, sizeof(defstring), "%s %f", defstring, defvalue[i]);
    }

    if (fileId >= SHOPITEM)
    {
        new configLoaded;
        new Handle:raceHandle;
        new Handle:configHandle;
        decl String:nodeName[SHORT_STRING_LENGTH];

        if (fileId == SHOPITEM)
        {
            configHandle = LoadItemConfig(configLoaded);

            if (node >= 0)
            {
                new Handle:itemHandle = GetShopItemHandle(node);
                GetItemShortName(itemHandle,nodeName,sizeof(nodeName));
            }
        }
        else
        {
            raceHandle = GetRaceHandle(fileId);
            configHandle = GetRaceConfig(raceHandle, configLoaded);

            if (node >= 0)
                GetUpgradeShortName(raceHandle,node,nodeName,sizeof(nodeName));
        }

        if (configHandle == INVALID_HANDLE)
            ThrowError("Unable obtain config handle for fileId %d", fileId);
        else
        {
            if (node >= 0)
            {
                new bool:nodeFound;
                if (configLoaded == CONFIG_NOT_LOADED)
                    nodeFound = false;
                else
                {
                    KvRewind(configHandle);
                    nodeFound = KvJumpToKey(configHandle, nodeName, false);
                    if (nodeFound)
                    {
                        new bool:sectionFound;
                        if (section[0] != '\0')
                        {
                            sectionFound = KvJumpToKey(configHandle, section, false);
                            if (!sectionFound)
                                KvJumpToKey(configHandle, section, true);
                        }
                        else
                            sectionFound = true;

                        if (sectionFound)
                        {
                            KvGetString(configHandle, key, buffer, sizeof(buffer), defstring);
                            count = StringToFloats(buffer, value, size);
                        }
                        else
                        {
                            KvSetString(configHandle, key, defstring);
                            for (new i=0; i < size; i++)
                                value[i] = defvalue[i];

                            if (fileId == SHOPITEM)
                                g_ItemConfigLoaded = CONFIG_WAS_UPDATED;
                            else
                                SetRaceConfigIsLoaded(raceHandle,CONFIG_WAS_UPDATED);
                        }
                    }
                    else
                    {
                        if (fileId == SHOPITEM)
                            g_ItemConfigLoaded = CONFIG_WAS_UPDATED;
                        else
                            SetRaceConfigIsLoaded(raceHandle,CONFIG_WAS_UPDATED);
                    }
                }

                if (!nodeFound)
                {
                    KvRewind(configHandle);
                    if (KvJumpToKey(configHandle, nodeName, true))
                    {
                        if (section[0] != '\0')
                        {
                            if (!KvJumpToKey(configHandle, section, true))
                            {
                                LogError("Unable to create section %s in node %s for %d",
                                         section, nodeName, fileId);
                            }
                        }
                        KvSetString(configHandle, key, defstring);
                        for (new i=0; i < size; i++)
                            value[i] = defvalue[i];
                    }
                    else
                    {
                        LogError("Unable to create node %s for %d",
                                 nodeName, fileId);
                    }
                }
            }
            else
            {
                if (((fileId == SHOPITEM) ? g_ItemConfigLoaded : GetRaceConfigIsLoaded(raceHandle)) == CONFIG_NOT_LOADED)
                {
                    if (section[0] != '\0')
                    {
                        if (!KvJumpToKey(configHandle, section, true))
                            LogError("Unable to create %s section", section);
                    }

                    KvSetString(configHandle, key, defstring);
                    for (new i=0; i < size; i++)
                        value[i] = defvalue[i];

                    if (fileId == SHOPITEM)
                        g_ItemConfigLoaded = CONFIG_WAS_UPDATED;
                    else
                        SetRaceConfigIsLoaded(raceHandle,CONFIG_WAS_UPDATED);
                }
                else
                {
                    new bool:sectionFound;
                    if (section[0] != '\0')
                    {
                        sectionFound = KvJumpToKey(configHandle, section, false);
                        if (!sectionFound)
                            KvJumpToKey(configHandle, section, true);
                    }
                    else
                        sectionFound = true;

                    if (sectionFound)
                    {
                        KvGetString(configHandle, key, buffer, sizeof(buffer), defstring);
                        count = StringToFloats(buffer, value, size);
                    }
                    else
                    {
                        KvSetString(configHandle, key, defstring);
                        for (new i=0; i < size; i++)
                            value[i] = defvalue[i];

                        if (fileId == SHOPITEM)
                            g_ItemConfigLoaded = CONFIG_WAS_UPDATED;
                        else
                            SetRaceConfigIsLoaded(raceHandle,CONFIG_WAS_UPDATED);
                    }
                }
            }
        }
    }
    else
    {
        LoadSettings();
        KvRewind(g_SourceCraftConfigHandle);

        new bool:sectionFound;
        if (section[0] != '\0')
        {
            if (g_SourceCraftConfigLoaded == CONFIG_WAS_LOADED)
            {
                sectionFound = KvJumpToKey(g_SourceCraftConfigHandle, section, false);
                if (!sectionFound)
                    g_SourceCraftConfigLoaded = CONFIG_WAS_UPDATED;
            }
            else
                sectionFound = false;

            if (!sectionFound)
            {
                KvRewind(g_SourceCraftConfigHandle);
                if (!KvJumpToKey(g_SourceCraftConfigHandle, section, true))
                {
                    LogError("Unable to create %s section in configs/sourcecraft.cfg",
                             section);
                }
            }
        }
        else
            sectionFound = true;

        if (g_SourceCraftConfigLoaded == CONFIG_NOT_LOADED || !sectionFound)
        {
            KvSetString(g_SourceCraftConfigHandle, key, defstring);
            for (new i=0; i < size; i++)
                value[i] = defvalue[i];
        }
        else
        {
            KvGetString(g_SourceCraftConfigHandle, key, buffer, sizeof(buffer), defstring);
            count = StringToFloats(buffer, value, size);
        }
    }
    return count;
}

bool:ParseSettings()
{
    TraceInto("SourceCraft", "ParseSettings");

    LoadSettings();
    if (g_SourceCraftConfigLoaded == CONFIG_NOT_LOADED)
    {
        // Set options
        KvSetNum(g_SourceCraftConfigHandle, "update", g_bUpdate);
        KvSetNum(g_SourceCraftConfigHandle, "savexp", g_bSaveXP);

        KvSetNum(g_SourceCraftConfigHandle, "min_players", g_iMinPlayers);
        KvSetNum(g_SourceCraftConfigHandle, "min_ultimate", g_iMinUltimate);
        KvSetNum(g_SourceCraftConfigHandle, "max_crystals", g_iMaxCrystals);
        KvSetNum(g_SourceCraftConfigHandle, "max_vespene", g_iMaxVespene);
    }
    else
    {
        // Load options
        g_bUpdate=bool:KvGetNum(g_SourceCraftConfigHandle, "update", g_bUpdate);
        g_bSaveXP=bool:KvGetNum(g_SourceCraftConfigHandle, "savexp", g_bSaveXP);

        g_iMinPlayers=KvGetNum(g_SourceCraftConfigHandle, "min_players", g_iMinPlayers);
        g_iMinUltimate=KvGetNum(g_SourceCraftConfigHandle, "min_ultimate", g_iMinUltimate);
        g_iMaxCrystals=KvGetNum(g_SourceCraftConfigHandle, "max_crystals", g_iMaxCrystals);
        g_iMaxVespene=KvGetNum(g_SourceCraftConfigHandle, "max_vespene", g_iMaxVespene);
    }

    // Load level configuration
    static const String:defaultLevels[][][] =
    {
        // long_term {"required_xp" , "kill_xp" }
        {
            "0 100 200 400 800 1600 3200 6400 12800 25600 51200 61200 71200 81200 91200 101200 111200",
            "6 8 10 12 14 16 20 24 28 32 40 50 60 70 80 90 100"
        },
        // short_term {"required_xp" , "kill_xp" }
        {
            "0 100 300 600 1000 2500 4000 6000 8000 10000 12000 14000 16000 18000 20000 22000 24000",
            "60 80 100 120 140 160 180 200 220 240 260 280 300 320 340 360 380"
        }
    };

    decl String:temp[512];
    new Handle:longterm_required=CreateArray();
    new Handle:longterm_killxp=CreateArray();
    new Handle:shortterm_required=CreateArray();
    new Handle:shortterm_killxp=CreateArray();

    SetTraceCategory("Array");
    Trace("CreateArray longterm_required=%x",longterm_required);
    Trace("CreateArray longterm_killxp=%x",longterm_killxp);
    Trace("CreateArray shortterm_required=%x",shortterm_required);
    Trace("CreateArray shortterm_killxp=%x",shortterm_killxp);

    KvRewind(g_SourceCraftConfigHandle);

    new bool:termFound;
    new bool:levelsFound = KvJumpToKey(g_SourceCraftConfigHandle,"levels");
    if (levelsFound)
        termFound = KvJumpToKey(g_SourceCraftConfigHandle, "long_term");
    else
        termFound = false;

    if (!termFound)
    {
        KvRewind(g_SourceCraftConfigHandle);
        if (!KvJumpToKey(g_SourceCraftConfigHandle, "long_term", true))
        {
            LogError("Unable to create long_term section in configs/sourcecraft.cfg!");
            CloseHandle(g_SourceCraftConfigHandle);
            g_SourceCraftConfigHandle=INVALID_HANDLE;
            g_SourceCraftConfigLoaded=CONFIG_NOT_LOADED;
            TraceReturn("return false");
            return false;
        }
    }

    // required xp, long term
    if (termFound)
        KvGetString(g_SourceCraftConfigHandle,"required_xp",temp,sizeof(temp), defaultLevels[0][0]);
    else
    {
        strcopy(temp, sizeof(temp), defaultLevels[0][0]);
        KvSetString(g_SourceCraftConfigHandle,"required_xp", defaultLevels[0][0]);
    }

    new count = StringToIntegerArray(temp, longterm_required);
    if (count < DEFAULT_MAX_LEVELS+1)
    {
        LogError("Insufficient levels defined for long_term required_xp in configs/sourcecraft.cfg, %d of 16!",
                 count);

        CloseHandle(g_SourceCraftConfigHandle);
        g_SourceCraftConfigHandle=INVALID_HANDLE;
        g_SourceCraftConfigLoaded=CONFIG_NOT_LOADED;
        TraceReturn("return false");
        return false;
    }

    // kill xp, long term
    if (termFound)
        KvGetString(g_SourceCraftConfigHandle,"kill_xp",temp,sizeof(temp), defaultLevels[0][1]);
    else
    {
        strcopy(temp, sizeof(temp), defaultLevels[0][1]);
        KvSetString(g_SourceCraftConfigHandle,"kill_xp", defaultLevels[0][1]);
    }

    count = StringToIntegerArray(temp, longterm_killxp);
    if (count < DEFAULT_MAX_LEVELS+1)
    {
        LogError("Insufficient levels defined for long_term kill_xp in configs/sourcecraft.cfg, %d of 16!",
                 count);

        CloseHandle(g_SourceCraftConfigHandle);
        g_SourceCraftConfigHandle=INVALID_HANDLE;
        g_SourceCraftConfigLoaded=CONFIG_NOT_LOADED;
        TraceReturn("return false");
        return false;
    }

    if (levelsFound)
    {
        KvRewind(g_SourceCraftConfigHandle);
        termFound = KvJumpToKey(g_SourceCraftConfigHandle, "short_term");
    }
    else
        termFound = false;

    if (!termFound)
    {
        KvRewind(g_SourceCraftConfigHandle);
        if (!KvJumpToKey(g_SourceCraftConfigHandle, "short_term", true))
        {
            LogError("Unable to create short_term section in configs/sourcecraft.cfg!");
            CloseHandle(g_SourceCraftConfigHandle);
            g_SourceCraftConfigHandle=INVALID_HANDLE;
            g_SourceCraftConfigLoaded=CONFIG_NOT_LOADED;
            TraceReturn("return false");
            return false;
        }
    }

    // required xp, short term
    if (termFound)
    {
        KvGetString(g_SourceCraftConfigHandle,"required_xp",temp,sizeof(temp), defaultLevels[1][0]);
    }
    else
    {
        strcopy(temp, sizeof(temp), defaultLevels[1][0]);
        KvSetString(g_SourceCraftConfigHandle,"required_xp", defaultLevels[1][0]);
    }

    count = StringToIntegerArray(temp, shortterm_required);
    if (count < DEFAULT_MAX_LEVELS+1)
    {
        LogError("Insufficient levels defined for short_term required_xp in configs/sourcecraft.cfg, %d of 16!",
                 count);

        CloseHandle(g_SourceCraftConfigHandle);
        g_SourceCraftConfigHandle=INVALID_HANDLE;
        g_SourceCraftConfigLoaded=CONFIG_NOT_LOADED;
        TraceReturn("return false");
        return false;
    }

    // kill xp, short term
    if (termFound)
        KvGetString(g_SourceCraftConfigHandle,"kill_xp",temp,sizeof(temp), defaultLevels[1][1]);
    else
    {
        strcopy(temp, sizeof(temp), defaultLevels[1][1]);
        KvSetString(g_SourceCraftConfigHandle,"kill_xp", defaultLevels[1][1]);
    }

    count = StringToIntegerArray(temp, shortterm_killxp);
    if (count < DEFAULT_MAX_LEVELS+1)
    {
        LogError("Insufficient levels defined for short_term kill_xp in configs/sourcecraft.cfg, %d of 16!",
                 count);

        CloseHandle(g_SourceCraftConfigHandle);
        g_SourceCraftConfigHandle=INVALID_HANDLE;
        g_SourceCraftConfigLoaded=CONFIG_NOT_LOADED;
        TraceReturn("return false");
        return false;
    }

    vecLevelConfiguration=CreateArray();
    PushArrayCell(vecLevelConfiguration,longterm_required);
    PushArrayCell(vecLevelConfiguration,longterm_killxp);
    PushArrayCell(vecLevelConfiguration,shortterm_required);
    PushArrayCell(vecLevelConfiguration,shortterm_killxp);

    TraceCat("Array", "vecLevelConfiguration=%x",vecLevelConfiguration);

    // Load URLs
    new bool:urlFound;
    if (g_SourceCraftConfigLoaded)
    {
        KvRewind(g_SourceCraftConfigHandle);
        urlFound = KvJumpToKey(g_SourceCraftConfigHandle, "urls", false);
        if (urlFound)
        {
            KvGetString(g_SourceCraftConfigHandle, "info", g_InfoURL, sizeof(g_InfoURL), g_InfoURL);
            KvGetString(g_SourceCraftConfigHandle, "webapp", g_InfoBaseURL, sizeof(g_InfoBaseURL), g_InfoBaseURL);
            KvGetString(g_SourceCraftConfigHandle, "update", g_UpdateURL, sizeof(g_UpdateURL), g_UpdateURL);
            KvGetString(g_SourceCraftConfigHandle, "wiki", g_WikiURL, sizeof(g_WikiURL), g_WikiURL);
            KvGetString(g_SourceCraftConfigHandle, "bug", g_BugURL, sizeof(g_BugURL), g_BugURL);
        }
    }
    else
        urlFound = false;

    if (!urlFound)
    {
        KvRewind(g_SourceCraftConfigHandle);
        if (KvJumpToKey(g_SourceCraftConfigHandle, "urls", true))
        {
            KvSetString(g_SourceCraftConfigHandle, "info", g_InfoURL);
            KvSetString(g_SourceCraftConfigHandle, "webapp", g_InfoBaseURL);
            KvSetString(g_SourceCraftConfigHandle, "update", g_UpdateURL);
            KvSetString(g_SourceCraftConfigHandle, "wiki", g_WikiURL);
            KvSetString(g_SourceCraftConfigHandle, "bug", g_BugURL);
        }
    }

    //Ensure the config/sc directory exists
    decl String:dir[PLATFORM_MAX_PATH];
    BuildPath(Path_SM,dir,sizeof(dir),"configs/sc");
    if (!DirExists(dir))
    {
        if (!CreateDirectory(dir, 493)) // 0755 == "u+rwx,go+rx"
            LogError("Unable to create %s directory!", dir);
    }

    TraceReturn("return true");
    return true;
}

Handle:GetRaceConfig(Handle:raceHandle, &loaded)
{
    new Handle:configHandle = GetRaceConfigHandle(raceHandle);
    if (configHandle == INVALID_HANDLE)
    {
        decl String:short[SHORT_STRING_LENGTH];
        GetRaceShortName(raceHandle,short,sizeof(short));

        configHandle = LoadRaceConfig(short, loaded);
        SetRaceConfigIsLoaded(raceHandle,loaded);
        SetRaceHasBeenCompleted(raceHandle,false);
    }
    else
        loaded = GetRaceConfigIsLoaded(raceHandle);

    return configHandle;
}

Handle:LoadRaceConfig(const String:short[], &loaded)
{
    new Handle:config = CreateKeyValues(short);

    decl String:path[PLATFORM_MAX_PATH];
    BuildPath(Path_SM,path,sizeof(path),"configs/sc/%s.cfg", short);

    loaded = FileToKeyValues(config,path);
    return config;
}

Handle:LoadItemConfig(&loaded)
{
    if (g_ItemConfigHandle == INVALID_HANDLE)
    {
        g_ItemConfigHandle = CreateKeyValues("shopitems");

        decl String:path[PLATFORM_MAX_PATH];
        BuildPath(Path_SM,path,sizeof(path),"configs/sc/shopitems.cfg");

        g_ItemConfigLoaded = FileToKeyValues(g_ItemConfigHandle,path);
    }

    loaded = g_ItemConfigLoaded;
    return g_ItemConfigHandle;
}

SetItemConfigLoaded(loaded)
{
    g_ItemConfigLoaded = loaded;
}

SaveConfig(Handle:configHandle, const String:name[])
{
    decl String:path[PLATFORM_MAX_PATH];
    BuildPath(Path_SM,path,sizeof(path),"configs/sc/%s.cfg", name);

    KvRewind(configHandle);
    if (!KeyValuesToFile(configHandle, path))
        LogError("Unable to create/update config file %s", path);
}

LoadSettings()
{
    if (g_SourceCraftConfigHandle == INVALID_HANDLE)
    {
        decl String:path[PLATFORM_MAX_PATH];
        BuildPath(Path_SM,path,sizeof(path),"configs/sourcecraft.cfg");

        g_SourceCraftConfigHandle=CreateKeyValues("SourceCraft");
        g_SourceCraftConfigLoaded=FileToKeyValues(g_SourceCraftConfigHandle,path);
    }
}

CompleteConfigs()
{
    TraceInto("Config", "CompleteConfigs");

    // Ensure all races are marked complete and saved.
    new size = GetRaceCount();
    for(new race=1;race<size;race++)
    {
        new Handle:raceHandle=GetRaceHandle(race);
        if (!GetRaceHasBeenCompleted(raceHandle))
            RaceComplete(race);
    }

    // Save and Close the shop item config file
    if (g_ItemConfigHandle != INVALID_HANDLE)
    {
        if (g_ItemConfigLoaded != CONFIG_WAS_LOADED)
            SaveConfig(g_ItemConfigHandle, "shopitems");

        CloseHandle(g_ItemConfigHandle);
        g_ItemConfigHandle = INVALID_HANDLE;
        g_ItemConfigLoaded = CONFIG_NOT_LOADED;
    }

    // Save and Close the main sourcraft config file
    if (g_SourceCraftConfigHandle != INVALID_HANDLE)
    {
        if (g_SourceCraftConfigLoaded != CONFIG_WAS_LOADED)
        {
            decl String:path[PLATFORM_MAX_PATH];
            BuildPath(Path_SM,path,sizeof(path),"configs/sourcecraft.cfg");

            KvRewind(g_SourceCraftConfigHandle);
            if (KeyValuesToFile(g_SourceCraftConfigHandle, path))
                LogMessage("Settings updated in %s", path);
            else                
                LogError("Unable to save settings in %s", path);
        }

        CloseHandle(g_SourceCraftConfigHandle);
        g_SourceCraftConfigHandle=INVALID_HANDLE;
        g_SourceCraftConfigLoaded=CONFIG_NOT_LOADED;
    }

    TraceReturn();
}

