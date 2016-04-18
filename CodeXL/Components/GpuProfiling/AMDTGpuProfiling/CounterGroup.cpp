//=====================================================================
// Copyright (c) 2012 Advanced Micro Devices, Inc. All rights reserved.
//
/// \author GPU Developer Tools
/// \file $File: //devtools/main/CodeXL/Components/GpuProfiling/AMDTGpuProfiling/CounterGroup.cpp $
/// \version $Revision: #5 $
/// \brief  This file contains CounterGroup class
//
//=====================================================================
// $Id: //devtools/main/CodeXL/Components/GpuProfiling/AMDTGpuProfiling/CounterGroup.cpp#5 $
// Last checkin:   $DateTime: 2015/08/31 01:56:31 $
// Last edited by: $Author: salgrana $
// Change list:    $Change: 538740 $
//=====================================================================
#include <AMDTGpuProfiling/CounterGroup.h>


CounterGroup::CounterGroup(const QString& strName)
{
    m_groupName = strName;
}

void CounterGroup::AddCounterName(const QString& strCounterName)
{
    m_counterNames.push_back(strCounterName);
}

int CounterGroup::GetCount() const
{
    return m_counterNames.count();
}

bool CounterGroup::GetCounterName(int index, QString& strCounterName)
{
    bool retVal = false;

    if (index >= 0 && index < m_counterNames.count())
    {
        strCounterName = m_counterNames[index];
        retVal = true;
    }
    else
    {
        strCounterName.clear();
    }

    return retVal;
}

void CounterGroup::SortCounters(SortFunc sortFunc)
{
    sortFunc(m_counterNames);
}

