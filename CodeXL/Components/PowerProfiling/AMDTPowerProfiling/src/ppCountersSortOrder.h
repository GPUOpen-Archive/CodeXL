//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file ppCountersSortOrder.h
///
//==================================================================================

//------------------------------ ppCountersSortOrder.h ------------------------------

#ifndef __PPCOUNTERSSORTORDER
#define __PPCOUNTERSSORTORDER

#include <qstring.h>
#include <AMDTPowerProfilingMidTier/include/PowerProfilerDefs.h>


class ppCountersSortOrder
{
public:
    ppCountersSortOrder();
    ~ppCountersSortOrder();

    /// SortCountersInCategory determines the sort order for display per counters in category
    /// @param[in] categoryID  the category counterIDs belong to
    /// @param     counterIDs  vector of counters IDs to sort
    void SortCountersInCategory(AMDTPwrCategory counterCategory, gtVector<int>& counterIDs);

private:
    void InitSortingData();
    void InitSortingData(AMDTPwrCategory category, const std::vector<std::string>& orderedCounterNames);

    void GetCountersByName(AMDTPwrCategory category, gtMap < QString, int >& countersByName);

    bool m_isInitialised;
    gtVector<int> m_prefixByCategory[AMDT_PWR_CATEGORY_CNT];
    gtVector<int> m_postfixByCategory[AMDT_PWR_CATEGORY_CNT];

};

#endif



