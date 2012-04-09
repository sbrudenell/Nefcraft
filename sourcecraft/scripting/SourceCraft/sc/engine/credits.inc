/*
 * vim: set ai et ts=4 sw=4 syntax=sourcepawn :
 * File: credits.inc
 * Description: Routines to manipulate crystals and vespene (gas). 
 * Author(s): -=|JFH|=-Naris (Murray Wilson)
 * Credits: Anthony Iacono
 */

#define KILL_CRYSTALS_LONGTERM   2
#define KILL_CRYSTALS_SHORTTERM  3

#define TOUCH_CRYSTALS           1
#define ESCORT_AMOUNT            2
#define FLAG_AMOUNT              5
#define POINT_AMOUNT             5
#define BLOCK_AMOUNT             3
#define CHARGE_CRYSTALS         15

#define PLANT_AMOUNT             5
#define DEFUSE_AMOUNT            3
#define EXPLODE_AMOUNT          20

#define KILL_PLANTER_AMOUNT      2
#define KILL_DEFUSER_AMOUNT      2

#define HOSTAGE_AMOUNT           2

#define ROUND_WIN_CRYSTALS       8
#define ROUND_WIN_VESPENE        2
#define GAME_WIN_CRYSTALS        2 // Multiplied by score
#define MVP_CRYSTALS             1 // Multiplied by place * points
#define MVP_ENERGY               1 // Multiplied by place * points

ShowCrystals(client, bool:forceDisplay)
{
    if (GetShopItemCount() >= 1 && !IsFakeClient(client))
    {
        DisplayMessage(client, forceDisplay ? NotSet : Display_Crystals,
                       "%t", "ShowCrystals", GetCrystals(client));
    }
}

ShowVespene(client, bool:forceDisplay)
{
    if (!IsFakeClient(client))
    {
        DisplayMessage(client, forceDisplay ? NotSet : Display_Vespene,
                       "%t", "ShowVespene", GetVespene(client));
    }
}

ShowEnergy(client, bool:forceDisplay)
{
    if (!IsFakeClient(client))
    {
        new limit = GetEnergyLimit(client);
        if (limit < 0 && !forceDisplay)
            limit = GetRaceEnergyLimit(GetRaceHandle(GetRace(client)));

        if (forceDisplay || limit != 0)
        {
            DisplayMessage(client, forceDisplay ? NotSet : Display_Energy, "%t", "ShowEnergy",
                           GetEnergy(client), GetAccumulatedEnergy(client, GetRace(client)));
        }
    }
}

GiveKillCrystals(client)
{
    new addcrystals= (SAVE_ENABLED) ? KILL_CRYSTALS_LONGTERM : KILL_CRYSTALS_SHORTTERM;
    GiveCrystals(client,addcrystals,"Kill");
}

GivePickedUpFlagCrystalsAndGas(client)
{
    GiveCrystals(client,FLAG_AMOUNT,"PickUpIntel");
    GiveVespene(client,FLAG_AMOUNT,"PickUpIntel");
}

TakeDroppedFlagCrystalsAndGas(client)
{
    GiveCrystals(client,-FLAG_AMOUNT,"DropIntel");
    GiveVespene(client,-FLAG_AMOUNT,"DropIntel");
}

GiveFlagCapturedCrystalsAndGas(client)
{
    GiveCrystals(client,FLAG_AMOUNT*2,"CapIntel");
    GiveVespene(client,FLAG_AMOUNT*2,"CapIntel");
}

GiveFlagDefendedCrystalsAndGas(client)
{
    GiveCrystals(client,FLAG_AMOUNT*2,"DefendIntel");
    GiveVespene(client,FLAG_AMOUNT*2,"DefendIntel");
}

GivePointCapturedCrystalsAndGas(client)
{
    GiveCrystals(client,POINT_AMOUNT,"CapPoint");
    GiveVespene(client,POINT_AMOUNT,"CapPoint");
}

GiveCapBlockedCrystalsAndGas(client)
{
    GiveCrystals(client,BLOCK_AMOUNT,"BlockPoint");
    GiveVespene(client,BLOCK_AMOUNT,"BlockPoint");
}

GivePointTouchedCrystals(client)
{
    GiveCrystals(client,TOUCH_CRYSTALS,"TouchPoint");
}

GiveEscortCrystalsAndGas(client, points)
{
    GiveCrystals(client,ESCORT_AMOUNT*points,"Escort");
    GiveVespene(client,ESCORT_AMOUNT,"Escort");
}

GiveChargeDeployedCrystals(client)
{
    GiveCrystals(client,CHARGE_CRYSTALS,"DeployUber");
}

GivePlantCrystalsAndGas(client)
{
    GiveCrystals(client,PLANT_AMOUNT,"PlantBomb");
    GiveVespene(client,PLANT_AMOUNT,"PlantBomb");
}

GiveDefuseCrystalsAndGas(client)
{
    GiveCrystals(client,DEFUSE_AMOUNT,"DefuseBomb");
    GiveVespene(client,DEFUSE_AMOUNT,"DefuseBomb");
}

GiveExplodeCrystalsAndGas(client)
{
    GiveCrystals(client,DEFUSE_AMOUNT,"ExplodeBomb");
    GiveVespene(client,DEFUSE_AMOUNT,"ExplodeBomb");
}

GiveKillPlanterCrystalsAndGas(client)
{
    GiveCrystals(client,KILL_PLANTER_AMOUNT,"KillBomber");
    GiveVespene(client,KILL_PLANTER_AMOUNT,"KillBomber");
}

GiveKillDefuserCrystalsAndGas(client)
{
    GiveCrystals(client,KILL_DEFUSER_AMOUNT,"KillDefuser");
    GiveVespene(client,KILL_DEFUSER_AMOUNT,"KillDefuser");
}

GiveHostageCrystalsAndGas(client)
{
    GiveCrystals(client,ESCORT_AMOUNT,"RescueHostage");
    GiveVespene(client,ESCORT_AMOUNT,"RescueHostage");
}

GiveRoundWinCrystalsAndGas(client)
{
    GiveCrystals(client,ROUND_WIN_CRYSTALS,"WinRound");
    GiveVespene(client,ROUND_WIN_VESPENE,"WinRound");
}

GiveGameWinCrystals(client,score)
{
    GiveCrystals(client,GAME_WIN_CRYSTALS*score,"WinGame");
}

GiveMvpCrystals(client,place,points)
{
    GiveCrystals(client, points*(7-place)*MVP_CRYSTALS,
                 "BeingMVP", place, points);
}

GiveMvpEnergy(client,place,points)
{
    GiveEnergy(client, points*(7-place)*MVP_ENERGY,
               "BeingMVP", place, points);
}

GiveCrystals(client, amount, const String:phrase[],
             place=-1, points=-1)
{
    if (GetShopItemCount() >= 1 && GetClientCount() >= g_iMinPlayers)
    {
        new Action:result = Plugin_Continue;
        Call_StartForward(g_OnCrystalsGivenHandle);
        Call_PushCell(client);
        Call_PushCellRef(amount);
        Call_PushCell(false);
        Call_Finish(result);

        if (result != Plugin_Stop)
        {
            new String:reason[NAME_STRING_LENGTH];
            if (place >= 0 || points >= 0)
            {
                Format(reason, sizeof(reason), "%T",
                       phrase, client, place, points);
            }
            else
            {
                Format(reason, sizeof(reason), "%T",
                       phrase, client);
            }

            new curcrystals = GetCrystals(client);
            new newcrystals = curcrystals+amount;
            if (newcrystals <= g_iMaxCrystals || g_iMaxCrystals <= 0)
            {
                SetCrystals(client,newcrystals);
                if (amount >= 0)
                {
                    LogToGame("%N gained %d crystals(s) for %s", client, amount, reason);
                    DisplayMessage(client, Display_Crystals, "%t", "GainedCrystals",
                                   amount, reason);
                }
                else
                {
                    LogToGame("%N lost %d crystals(s) for %s", client, -amount, reason);
                    DisplayMessage(client, Display_Crystals, "%t", "LostCrystals",
                                   -amount, reason);
                }
            }
            else
            {
                new amt = g_iMaxCrystals-curcrystals;
                if (amt > 0)
                {
                    SetCrystals(client,g_iMaxCrystals);
                    LogToGame("%N gained %d crystal(s) for %s", client, amt, reason);
                    DisplayMessage(client, Display_Crystals, "%t", "GainedMaxCrystals",
                                   amt, g_iMaxCrystals, reason);
                }
            }

            if (IsFakeClient(client))
            {
                // Should the Bot buy something?
                new crystals = GetCrystals(client);
                if (GetRandomInt(0,100) >= ((crystals >= 100) ? 50 : 75))
                {
                    if (crystals > 10)
                    {
                        new shopItemCount = GetShopItemCount();
                        new item = GetRandomInt(0, shopItemCount-1);
                        if (item >= 0)
                        {
                            new Handle:itemHandle = GetShopItemHandle(item);
                            if (itemHandle && GetCrystals(client) >= GetItemCrystals(itemHandle))
                                PurchaseShopItem(client, item);
                        }
                    }
                }
            }
        }
    }
}

GiveVespene(client, amount, const String:phrase[],
            place=-1, points=-1)
{
    if (GetShopItemCount() >= 1 && GetClientCount() >= g_iMinPlayers)
    {
        new Action:result = Plugin_Continue;
        Call_StartForward(g_OnVespeneGivenHandle);
        Call_PushCell(client);
        Call_PushCellRef(amount);
        Call_PushCell(false);
        Call_Finish(result);

        if (result != Plugin_Stop)
        {
            new String:reason[NAME_STRING_LENGTH];
            if (place >= 0 || points >= 0)
            {
                Format(reason, sizeof(reason), "%T",
                       phrase, client, place, points);
            }
            else
            {
                Format(reason, sizeof(reason), "%T",
                       phrase, client);
            }

            new curvespene = GetVespene(client);
            new newvespene = curvespene+amount;
            if (newvespene <= g_iMaxVespene || g_iMaxVespene <= 0)
            {
                SetVespene(client,newvespene);
                if (amount >= 0)
                {
                    LogToGame("%N gained %d vespene for %s", client, amount, reason);
                    DisplayMessage(client, Display_Vespene, "%t", "GainedVespene",
                                   amount, reason);
                }
                else
                {
                    LogToGame("%N lost %d vespene for %s", client, -amount, reason);
                    DisplayMessage(client, Display_Vespene, "%t", "LostVespene",
                                   -amount, reason);
                }
            }
            else
            {
                new amt = g_iMaxVespene-curvespene;
                if (amt > 0)
                {
                    SetVespene(client,g_iMaxVespene);
                    LogToGame("%N gained %d vespene for %s", client, amt, reason);
                    DisplayMessage(client, Display_Vespene, "%t", "GainedMaxVespene",
                                   amt, g_iMaxVespene, reason);
                }
            }
        }
    }
}

GiveEnergy(client, amount, const String:phrase[],
           place=-1, points=-1)
{
    if (GetShopItemCount() >= 1 && GetClientCount() >= g_iMinPlayers)
    {
        new String:reason[NAME_STRING_LENGTH];
        if (place >= 0 || points >= 0)
        {
            Format(reason, sizeof(reason), "%T",
                   phrase, client, place, points);
        }
        else
        {
            Format(reason, sizeof(reason), "%T",
                   phrase, client);
        }

        IncrementAccumulatedEnergy(client,GetRace(client),amount);
        if (amount >= 0)
        {
            LogToGame("%N gained %d accumulated energy for %s", client, amount, reason);
            DisplayMessage(client, Display_Vespene, "%t.", "GainedAccumulatedEnergy",
                           amount, reason);
        }
        else
        {
            LogToGame("%N lost %d accumulated energy for %s", client, -amount, reason);
            DisplayMessage(client, Display_Vespene, "%t", "LostAccumulatedEnergy",
                           -amount, reason);
        }
    }
}
