/**
 * vim: set ai et ts=4 sw=4 syntax=sourcepawn :
 * File: range.inc
 * Description: Range and Distance functions and variables
 * Author(s): Naris (Murray Wilson)
 */
 
#if defined _range_included
 #endinput
#endif
#define _range_included

stock Float:TargetRange(client,index)
{
    new Float:start[3];
    new Float:end[3];
    GetClientAbsOrigin(client,start);
    GetClientAbsOrigin(index,end);
    return GetVectorDistance(start,end);
}

stock bool:IsInRange(client,index,Float:maxdistance)
{
    return (TargetRange(client,index)<maxdistance);
}

stock bool:IsPointInRange(const Float:start[3], const Float:end[3],Float:maxdistance)
{
    return (GetVectorDistance(start,end)<maxdistance);
}

stock PowerOfRange(Float:location[3],Float:radius,Float:check_location[3],maxhp,
                   Float:factor=0.20,bool:limit=true,&Float:distance=0.0)
{
    if (radius <= 0.0)
        return maxhp;
    else
    {
        distance=GetVectorDistance(location,check_location);
        if (limit && distance > radius)
            return 0;
        else
        {
            new Float:percent=1-FloatDiv(distance,radius)+factor;
            return (percent > 0.0) ? RoundFloat(float(maxhp)*percent) : 0;
        }
    }
}

