/**
 * vim: set ai et ts=4 sw=4 syntax=sourcepawn :
 * File: shopitem.inc
 * Description: Defines the Shop Item Data.
 * Author(s): -=|JFH|=-Naris
 */

#define ITEM_STRINGS                        0
#define ITEM_CRYSTALS                       1
#define ITEM_VESPENE                        2
#define ITEM_MAX                            3
#define ITEM_REQUIRED_LEVEL                 4
#define ITEM_TRANSLATION                    5
#define ITEM_SHOP_ID                        6
#define ITEM_DATA_COUNT                     7

#define ITEM_SHORT_NAME                     0
#define ITEM_CATEGORY                       1
#define ITEM_NAME                           2
#define ITEM_DESC                           3
#define ITEM_STRING_COUNT                   4

// Accessor Macros for Shop Item & Index Arrays
#define GetShopItemCount()                  GetArraySize(shopVector)
#define GetShopIndexCount()                 GetArraySize(shopIndex)

#define GetShopItemHandle(%1)               (Handle:GetArrayCell(shopVector,%1))
#define GetItemIndexHandle(%1)              (Handle:GetArrayCell(shopIndex,%1))

// Accessor Macros for Shop Item Data
#define GetShopId(%1)                       GetArrayCell(%1,ITEM_SHOP_ID)
#define SetShopId(%1,%2)                    SetArrayCell(%1,ITEM_SHOP_ID,%2)

#define GetItemCrystals(%1)                 GetArrayCell(%1,ITEM_CRYSTALS)
#define SetItemCrystals(%1,%2)              SetArrayCell(%1,ITEM_CRYSTALS,%2)

#define GetItemVespene(%1)                  GetArrayCell(%1,ITEM_VESPENE)
#define SetItemVespene(%1,%2)               SetArrayCell(%1,ITEM_VESPENE,%2)

#define GetItemMax(%1)                      GetArrayCell(%1,ITEM_MAX)
#define SetItemMax(%1,%2)                   SetArrayCell(%1,ITEM_MAX,%2)

#define GetItemRequiredLevel(%1)            GetArrayCell(%1,ITEM_REQUIRED_LEVEL)
#define SetItemRequiredLevel(%1,%2)         SetArrayCell(%1,ITEM_REQUIRED_LEVEL,%2)

#define GetItemTranslation(%1)              (Translation:GetArrayCell(%1,ITEM_TRANSLATION))
#define SetItemTranslation(%1,%2)           SetArrayCell(%1,ITEM_TRANSLATION,_:%2)

// Accessor Macros for Shop Item Strings
#define GetItemStringHandle(%1)             GetArrayCell(%1,ITEM_STRINGS)

#define GetItemShortName(%1,%2,%3)          GetArrayString(GetItemStringHandle(%1),ITEM_SHORT_NAME,%2,%3)
#define SetItemShortName(%1,%2)             SetArrayString(GetItemStringHandle(%1),ITEM_SHORT_NAME,%2)

#define GetItemCategoryString(%1,%2,%3)     GetArrayString(GetItemStringHandle(%1),ITEM_CATEGORY,%2,%3)
#define SetItemCategoryString(%1,%2)        SetArrayString(GetItemStringHandle(%1),ITEM_CATEGORY,%2)

#define GetItemNameString(%1,%2,%3)         GetArrayString(GetItemStringHandle(%1),ITEM_NAME,%2,%3)
#define SetItemName(%1,%2)                  SetArrayString(GetItemStringHandle(%1),ITEM_NAME,%2)

#define GetItemDescriptionString(%1,%2,%3)  GetArrayString(GetItemStringHandle(%1),ITEM_DESC,%2,%3)
#define SetItemDescription(%1,%2)           SetArrayString(GetItemStringHandle(%1),ITEM_DESC,%2)

// A vector of shopmenu infomation
new Handle:shopVector = INVALID_HANDLE;
new Handle:shopIndex = INVALID_HANDLE;

new Handle:g_OnItemPurchaseHandle = INVALID_HANDLE;
new Handle:g_OnItemLostHandle = INVALID_HANDLE;

bool:InitShopVector()
{
    SetTraceCategory("Array");
    TraceInto("ShopItems", "InitShopVector");

    if (shopVector == INVALID_HANDLE)
        shopVector = CreateArray();

    if (shopIndex == INVALID_HANDLE)
        shopIndex = CreateArray();

    TraceReturn("shopVector=0x%08x, shopIndex=0x%08x", shopVector, shopIndex);

    return (shopVector != INVALID_HANDLE &&
            shopIndex  != INVALID_HANDLE);
}

ClearShopVector()
{
    TraceInto("ShopItems", "ClearShopVector");

    if (shopVector == INVALID_HANDLE)
    {
        new itemCount = GetShopItemCount();
        for(new x=0;x<itemCount;x++)
        {
            new Handle:item=GetArrayCell(shopVector,x);
            new Handle:stringsHandle=GetArrayCell(item,ITEM_STRINGS);
            ClearArray(stringsHandle);
            CloseHandle(stringsHandle);

            ClearArray(item);
            CloseHandle(item);

            SetTraceCategory("Array");
            Trace("CloseArray itemStringsHandle[%d]=0x%08x", x, stringsHandle);
            Trace("CloseArray item[%d]=0x%08x", x, item);
        }
    }

    SetTraceCategory("Array");
    Trace("CloseArray shopVector=0x%08x", shopVector);
    Trace("CloseArray shopIndex=0x%08x", shopIndex);

    ClearArray(shopVector);
    CloseHandle(shopVector);
    shopVector = INVALID_HANDLE;

    ClearArray(shopIndex);
    CloseHandle(shopIndex);
    shopIndex = INVALID_HANDLE;

    TraceReturn();
}

CreateShopItem(const String:short[], crystals, vespene, max, required_level,
               Translation:translation=Translation_Auto, const String:translation_file[]="",
               const String:category[]="", const String:name[]="", const String:desc[]="")
{
    new bool:itemFound;
    new Handle:itemConfig;
    new itemLoaded;

    TraceInto("ShopItems", "CreateShopItem", "name=%s", name);

    // Load shop item config, if any
    if (short[0] == '\0')
        itemConfig = INVALID_HANDLE;
    else
    {
        itemConfig = LoadItemConfig(itemLoaded);
        if (itemLoaded == CONFIG_NOT_LOADED)
        {
            KvRewind(itemConfig);
            itemFound = KvJumpToKey(itemConfig, short, false);
            if (itemFound)
            {
                crystals = KvGetNum(itemConfig,"crystals", crystals);
                vespene = KvGetNum(itemConfig,"vespene", vespene);
                max = KvGetNum(itemConfig,"max", max);
                required_level = KvGetNum(itemConfig,"required_level", required_level);
            }
            else
                SetItemConfigLoaded(CONFIG_WAS_UPDATED);
        }
        else
            itemFound = false;

        if (!itemFound)
        {
            KvRewind(itemConfig);
            if (KvJumpToKey(itemConfig, short, true))
            {
                KvSetNum(itemConfig,"crystals", crystals);

                if (vespene > 0)
                    KvSetNum(itemConfig,"vespene", vespene);

                if (max > 1)
                    KvSetNum(itemConfig,"max", max);

                if (required_level > 0)
                    KvSetNum(itemConfig,"required_level", required_level);

                KvGoBack(itemConfig);
            }
            else
                LogError("Unable to create shopitem %s section", short);
        }

        if (translation_file[0] == '\0')
        {
            // Check for and Load Translations, if any
            decl String:path[PLATFORM_MAX_PATH];
            decl String:file[NAME_STRING_LENGTH];

            switch (translation)
            {
                case Translation_SC:
                {
                    // Check for SourceCraft translations
                    Format(file,sizeof(file),"sc.%s.phrases.txt",short);
                    BuildPath(Path_SM, path, sizeof(path), "translations/%s", file);
                    if (FileExists(path))
                        LoadTranslations(file);
                    else
                    {
                        // Check for SourceCraft translations without the prefix
                        Format(file,sizeof(file),"%s.phrases.txt",short);
                        BuildPath(Path_SM, path, sizeof(path), "translations/%s", file);
                        if (FileExists(path))
                            LoadTranslations(file);
                    }
                }
                case Translation_W3S:
                {
                    // Check for War3Source translations
                    Format(file,sizeof(file),"w3s.item.%s.phrases.txt",short);
                    BuildPath(Path_SM, path, sizeof(path), "translations/%s", file);
                    if (FileExists(path))
                        LoadTranslations(file);
                }
                case Translation_Auto, Translation_Default, Translation_Custom:
                {
                    // Check for SourceCraft translations
                    Format(file,sizeof(file),"sc.%s.phrases.txt",short);
                    BuildPath(Path_SM, path, sizeof(path), "translations/%s", file);
                    if (FileExists(path))
                    {
                        LoadTranslations(file);
                        translation = Translation_SC;
                    }
                    else
                    {
                        // Check for SourceCraft translations without the prefix
                        Format(file,sizeof(file),"%s.phrases.txt",short);
                        BuildPath(Path_SM, path, sizeof(path), "translations/%s", file);
                        if (FileExists(path))
                        {
                            LoadTranslations(file);
                            translation = Translation_SC;
                        }
                        else
                        {
                            // Check for War3Source translations
                            Format(file,sizeof(file),"w3s.item.%s.phrases.txt",short);
                            BuildPath(Path_SM, path, sizeof(path), "translations/%s", file);
                            if (FileExists(path))
                            {
                                LoadTranslations(file);
                                translation = Translation_W3S;
                            }
                            else if (category[0] == '\0' && name[0] == '\0' && desc[0] == '\0')
                            {
                                translation = Translation_SC;
                            }
                        }
                    }
                }
            }
        }
    }

    if (translation_file[0] != '\0')
    {
        LoadTranslations(translation_file);
        if (translation < Translation_Custom)
            translation = Translation_Custom;
    }

    new Handle:newItem=CreateArray();

    new Handle:stringsHandle=CreateArray(ByteCountToCells(192));
    PushArrayString(stringsHandle,short);
    PushArrayString(stringsHandle,category);
    PushArrayString(stringsHandle,name);
    PushArrayString(stringsHandle,desc);

    PushArrayCell(newItem,stringsHandle);
    PushArrayCell(newItem,crystals);
    PushArrayCell(newItem,vespene);
    PushArrayCell(newItem,max);
    PushArrayCell(newItem,required_level);
    PushArrayCell(newItem,translation);

    // Add new Shop Item into Shop Vector.
    new shopId = PushArrayCell(shopVector,newItem); // return index.
    PushArrayCell(newItem,shopId);

    SetTraceCategory("Array");
    Trace("CreateArray item[%d]=0x%08x", shopId, newItem);
    Trace("CreateArray itemStringsHandle[%d]=0x%08x", shopId, stringsHandle);

    decl String:newCategory[64];
    GetItemCategory(newItem, newCategory, sizeof(newCategory));

    decl String:newName[64];
    GetItemName(newItem, newName, sizeof(newName));

    new item=0;
    new indexCount=GetArraySize(shopIndex);
    for(;item<indexCount;item++)
    {
        decl String:itemCategory[64];
        new Handle:itemHandle = GetItemIndexHandle(item);
        GetItemCategory(itemHandle, itemCategory, sizeof(itemCategory));

        new cmpCategory = strcmp(newCategory,itemCategory,false);
        if (cmpCategory <= 0)
        {
            new itemLevel = GetItemRequiredLevel(itemHandle);
            if (itemLevel < 0)
                itemLevel = 65535;
            if (cmpCategory <= 0 || required_level <= itemLevel)
            {
                decl String:itemName[64];
                GetItemName(itemHandle, itemName, sizeof(itemName));
                if (cmpCategory < 0 || required_level < itemLevel ||
                    (cmpCategory == 0 && required_level == itemLevel &&
                     strcmp(newName,itemName,false) < 0))
                {
                    ShiftArrayUp(shopIndex, item);
                    SetArrayCell(shopIndex, item, newItem);
                    indexCount++;
                    break;
                }
            }
        }
    }

    if (item >= indexCount)
        PushArrayCell(shopIndex,newItem);

    TraceReturn("shopId=%d", shopId);
    return shopId;
}

FindShopItem(const String:name[])
{
    decl String:curName[64];
    new size = GetShopItemCount();
    for(new x=0;x<size;x++)
    {
        new Handle:itemHandle=GetArrayCell(shopVector,x);
        new Handle:stringsHandle=GetArrayCell(itemHandle,ITEM_STRINGS);
        GetArrayString(stringsHandle,ITEM_SHORT_NAME,curName,sizeof(curName));
        if (StrEqual(name,curName,false))
            return x;
        else
        {
            GetArrayString(stringsHandle,ITEM_NAME,curName,sizeof(curName));
            if (StrEqual(name,curName,false))
                return x;
        }
    }
    return -1;
}

bool:PurchaseShopItem(client,item)
{
    // Purchase
    new crystals=GetCrystals(client);
    new vespene=GetVespene(client);
    new Handle:itemHandle=GetShopItemHandle(item);
    if (itemHandle != INVALID_HANDLE)
    {
        decl String:itemName[64];
        GetItemName(itemHandle, itemName, sizeof(itemName),client);

        new cost = GetItemCrystals(itemHandle);
        if (crystals >= cost)
        {
            new gas = GetItemVespene(itemHandle);
            if (vespene >= gas)
            {
                new num = GetOwnsItem(client,item);
                if (num < GetItemMax(itemHandle))
                {
                    new result;

                    if (cost > 0)
                        SetCrystals(client,crystals-cost);

                    if (gas > 0)
                        SetVespene(client,vespene-gas);

                    SetOwnsItem(client,item,++num);
                    Call_StartForward(g_OnItemPurchaseHandle);
                    Call_PushCell(client);
                    Call_PushCell(item);
                    Call_Finish(result);

                    if (IsClientInGame(client) && !IsFakeClient(client))
                    {
                        if (num > 1)
                        {
                            CPrintToChat(client, "{green}[SC]{default} %t", "AnotherItemPurchased", itemName, num);
                        }
                        else
                        {
                            CPrintToChat(client, "{green}[SC]{default} %t", "ItemPurchased", itemName);
                        }
                    }
                    return true;
                }
                else if (IsClientInGame(client) && !IsFakeClient(client))
                {
                    if (num > 1)
                    {
                        CPrintToChat(client, "{green}[SC]{default} %t", "AlreadyOwnMax", num, itemName);
                    }
                    else
                    {
                        CPrintToChat(client, "{green}[SC]{default} %t", "AlreadyOwn", itemName);
                    }
                }
            }
            else if (IsClientInGame(client) && !IsFakeClient(client))
            {
                new Faction:faction;
                new race = GetRace(client);
                if (race >= 0)
                {
                    new Handle:raceHandle=GetRaceHandle(race);
                    faction=GetRaceFactionId(raceHandle);
                }
                else
                    faction=Generic;

                EmitVespeneSoundToClient(client, faction);
                CPrintToChat(client, "{green}[SC]{default} %t", "InsufficientVespeneFor", itemName, gas);
            }
        }
        else if (IsClientInGame(client) && !IsFakeClient(client))
        {
            new Faction:faction;
            new race = GetRace(client);
            if (race >= 0)
            {
                new Handle:raceHandle=GetRaceHandle(race);
                faction=GetRaceFactionId(raceHandle);
            }
            else
                faction=Generic;

            EmitCrystalSoundToClient(client, faction);
            CPrintToChat(client, "{green}[SC]{default} %t", "InsufficientCrystalsFor", itemName, cost);
        }
        ShowCrystals(client,false);
    }
    return false;
}

GetItemName(Handle:itemHandle, String:name[], maxlength, lang=LANG_SERVER)
{
    GetItemNameString(itemHandle, name, maxlength);
    if (name[0] == '%')
        Format(name,maxlength,"%T",name[1],lang);
    else
    {
        new Translation:translation = GetItemTranslation(itemHandle);
        if (translation > Translation_Auto)
        {
            // Translate the short name to get the name
            decl String:short[NAME_STRING_LENGTH];
            GetItemShortName(itemHandle, short, sizeof(short));

            if (translation == Translation_W3S)
            {
                // Use the War3Source key
                decl String:key[NAME_STRING_LENGTH];
                Format(key,sizeof(key),"%s_ItemName",short);
                Format(name,maxlength,"%T",key,lang);
            }
            else
            {
                // Translate the short name to get the name
                Format(name,maxlength,"%T",short,lang);
            }
        }
    }
}

GetItemCategory(Handle:itemHandle, String:category[], maxlength, lang=LANG_SERVER)
{
    GetItemCategoryString(itemHandle, category, maxlength);
    if (category[0] == '%')
        Format(category,maxlength,"%T",category[1],lang);
    else
    {
        new Translation:translation = GetItemTranslation(itemHandle);
        if (translation > Translation_Auto)
        {
            decl String:short[NAME_STRING_LENGTH];
            GetItemShortName(itemHandle, short, sizeof(short));

            if (translation == Translation_W3S)
            {
                // Use the War3Source key
                decl String:key[NAME_STRING_LENGTH];
                Format(key,sizeof(key),"%s_ItemName",short);
                Format(category,maxlength,"%T",key,lang);
            }
            else
            {
                // Translate the key to get the category
                decl String:key[NAME_STRING_LENGTH];
                Format(key,sizeof(key),"%s_category",short);
                Format(category,maxlength,"%T",key,lang);
            }
        }
    }
}

GetItemDescription(Handle:itemHandle, String:description[], maxlength, lang=LANG_SERVER)
{
    GetItemDescriptionString(itemHandle, description, maxlength);
    if (description[0] == '%')
        Format(description,maxlength,"%T",description[1],lang);
    else
    {
        new Translation:translation = GetItemTranslation(itemHandle);
        if (translation > Translation_Auto)
        {
            // Translate the key to get the description
            decl String:short[DESCRIPTION_LENGTH];
            GetItemShortName(itemHandle, short, sizeof(short));

            decl String:key[NAME_STRING_LENGTH];
            Format(key,sizeof(key),"%s_desc",short);
            Format(description,maxlength,"%T",key,lang);
        }
    }
}

