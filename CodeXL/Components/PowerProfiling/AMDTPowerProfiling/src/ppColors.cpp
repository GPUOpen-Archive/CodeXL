//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file ppColors.cpp
///
//==================================================================================


#include <AMDTPowerProfiling/src/ppColors.h>
#include <AMDTPowerProfileApi.h>

ppColorsMap* ppColorsMap::m_colorsSingleInstance = nullptr;
ppHierarchyMap* ppHierarchyMap::m_hierarchyMapSingleInstance = nullptr;

#define AC_AMD_HUE_CNT 6
acAMDColorHue g_colourTable[]=
{
    AC_AMD_RED,
    AC_AMD_PURPLE,
    AC_AMD_CYAN,
    AC_AMD_GREEN,
    AC_AMD_ORANGE,
    AC_AMD_GRAY,
};

ppColorsMap::ppColorsMap()
{
    AMDTResult ret = AMDT_ERROR_FAIL;
    AMDTUInt32 nbrCounters = 0;
    AMDTPwrCounterDesc* pCounters = nullptr;
    ret = AMDTPwrGetDeviceCounters(AMDT_PWR_ALL_DEVICES, &nbrCounters, &pCounters);

    if(AMDT_STATUS_OK == ret)
    {
        AMDTUInt32 cnt = 0;
        AMDTUInt32 counterIdx[AMDT_PWR_CATEGORY_CNT];

        memset(counterIdx, 0, AMDT_PWR_CATEGORY_CNT * sizeof(AMDTUInt32));

        for (cnt = 0; cnt < nbrCounters; cnt++)
        {
            AMDTPwrCounterDesc* pInfo = pCounters + cnt;

            if(nullptr != pInfo)
            {
                AMDTUInt32 shed = counterIdx[pInfo->m_category] > AC_AMD_HUE_CNT ? (counterIdx[pInfo->m_category] - AC_AMD_HUE_CNT): 0;
                m_colorsMap[pInfo->m_counterID] = acGetAMDColorScaleColor(g_colourTable[counterIdx[pInfo->m_category] % AC_AMD_HUE_CNT], shed);
                counterIdx[pInfo->m_category]++;
            }
        }
    }
}

ppColorsMap& ppColorsMap::Instance()
{
    // If this class single instance was not already created
    if (m_colorsSingleInstance == nullptr)
    {
        // Create it
        m_colorsSingleInstance = new ppColorsMap;
        GT_ASSERT(m_colorsSingleInstance);
    }

    return *m_colorsSingleInstance;
}

QColor ppColorsMap::GetColorForCounterName(const unsigned int& counterIdx)
{
    QColor retColor = Qt::red;
    retColor = m_colorsMap[counterIdx];
    return retColor;
}

//-----------------------------------------------------------------------------

ppHierarchyMap::ppHierarchyMap()
{
    m_hierarchyMap[PP_STR_Counter_Power_UVD] = PP_STR_Counter_Power_NB;
    m_hierarchyMap[PP_STR_Counter_Power_VCE] = PP_STR_Counter_Power_NB;
    m_hierarchyMap[PP_STR_Counter_Power_ACP] = PP_STR_Counter_Power_NB;
    m_hierarchyMap[PP_STR_Counter_Power_UNB] = PP_STR_Counter_Power_NB;
    m_hierarchyMap[PP_STR_Counter_Power_SMU] = PP_STR_Counter_Power_NB;
}

ppHierarchyMap& ppHierarchyMap::Instance()
{
    // If this class single instance was not already created
    if (m_hierarchyMapSingleInstance == nullptr)
    {
        // Create it
        m_hierarchyMapSingleInstance = new ppHierarchyMap;
        GT_ASSERT(m_hierarchyMapSingleInstance);
    }

    return *m_hierarchyMapSingleInstance;
}

bool ppHierarchyMap::IsChildCounter(const QString& counterName) const
{
    bool ret = false;

    if (m_hierarchyMap.contains(counterName))
    {
        ret = true;
    }

    return ret;
}

QString ppHierarchyMap::GetCounterParent(const QString& counterName) const
{
    QString ret = "";

    if (m_hierarchyMap.contains(counterName))
    {
        ret = m_hierarchyMap[counterName];
    }

    return ret;
}

QList<QString> ppHierarchyMap::GetChildrenListForCounter(const QString& counterName)
{
    QList<QString> childrenList;

    QMap<QString, QString>::Iterator it = m_hierarchyMap.begin();
    QMap<QString, QString>::Iterator itEnd = m_hierarchyMap.end();

    for (; it != itEnd; it++)
    {
        QString tmp = it.value();

        if (counterName == tmp)
        {
            childrenList.append(it.key());
        }
    }

    return childrenList;
}