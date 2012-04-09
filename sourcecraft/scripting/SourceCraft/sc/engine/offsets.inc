/*
 * vim: set ai et ts=4 sw=4 syntax=sourcepawn :
 * File: offsets.inc
 * Description: Offset definitions.
 * Author(s): -=|JFH|=-Naris (Murray Wilson)
 * Credit(s): PimpinJuice (Anthony Iacono)
 */

new m_clrRender = 0;
new m_hMyWeapons = 0;

bool:InitOffsets()
{
    new bool:ret=true;

    m_clrRender = FindSendPropOffs("CBaseAnimating","m_clrRender");
    if (m_clrRender==-1)
    {
        LogError("Error finding m_clrRender offset.");
        ret=false;
    }

    m_hMyWeapons = FindSendPropInfo("CBasePlayer", "m_hMyWeapons");
    if (m_hMyWeapons==-1)
    {
        LogError("Error finding m_hMyWeapons offset.");
        ret=false;
    }

    return ret;
}

//stock GetEntityRenderAlpha(index)
#define GetEntityRenderAlpha(%1) GetEntData(%1,m_clrRender+3,1)

//stock GetEntityRenderRed(index)
#define GetEntityRenderRed(%1) GetEntData(%1,m_clrRender,1)

//stock GetEntityRenderGreen(index)
#define GetEntityRenderGreen(%1) GetEntData(%1,m_clrRender+1,1)

//stock GetEntityRenderBlue(index)
#define GetEntityRenderBlue(%1) GetEntData(%1,m_clrRender+2,1)

//stock GetEntityRenderRGB(index,rgb[3])
#define GetEntityRenderRGB(%1,%2) GetEntDataArray(%1,m_clrRender,%2,3)

//stock GetEntityRenderColor(index,color[4])
#define GetEntityRenderColor(%1,%2) GetEntDataArray(%1,m_clrRender,%2,4)
