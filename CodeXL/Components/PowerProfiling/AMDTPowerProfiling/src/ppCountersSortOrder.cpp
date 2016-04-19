//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file ppCountersSortOrder.cpp
///
//==================================================================================

// Qt:
#include <AMDTApplicationComponents/Include/acIcons.h>

// Infra:
#include <AMDTBaseTools/Include/gtSet.h>
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTBaseTools/Include/gtAlgorithms.h>


// Local:
#include <AMDTPowerProfiling/Include/ppStringConstants.h>
#include <AMDTPowerProfiling/src/ppAppController.h>
#include <AMDTPowerProfiling/src/ppCountersSortOrder.h>

// ---------------------------------------------------------------------------

ppCountersSortOrder::ppCountersSortOrder()
    : m_isInitialised(false)
{
}

//----------------------------------------------------------------------------
ppCountersSortOrder::~ppCountersSortOrder()
{
}

//----------------------------------------------------------------------------
void ppCountersSortOrder::SortCountersInCategory(AMDTPwrCategory counterCategory, gtVector<int>& counterIDs)
{
    if (!m_isInitialised)
    {
        InitSortingData();
    }

    gtVector<int> postCountersByOrder = m_postfixByCategory[counterCategory];

    // going over the counter that must be in the end  and remove them from the main list
    for (int i = postCountersByOrder.size() - 1; i >= 0; i--)
    {
        gtVector<int>::iterator found = std::find(counterIDs.begin(), counterIDs.end(), postCountersByOrder.at(i));

        if (found == counterIDs.end())
        {
            // counter is not in input list - remove from prefix list
            postCountersByOrder.removeItem(i);
        }
        else
        {
            // counter is in input list, remove from original list - will be added in the prefix section
            counterIDs.erase(found);
        }
    }

    gtVector<int> preCountersByOrder = m_prefixByCategory[counterCategory];

    // going over the counter that must be at the beginning  and remove them from the main list
    for (int i = preCountersByOrder.size() - 1; i >= 0; i--)
    {
        gtVector<int>::iterator found = std::find(counterIDs.begin(), counterIDs.end(), preCountersByOrder.at(i));

        if (found == counterIDs.end())
        {
            // counter is not in input list - remove from prefix list
            preCountersByOrder.removeItem(i);
        }
        else
        {
            // counter is in input list, remove from original list - will be added in the prefix section
            counterIDs.erase(found);
        }
    }


    gtSort(counterIDs.begin(), counterIDs.end());
    // add fixed counters to start of list
    counterIDs.insert(counterIDs.begin(), preCountersByOrder.begin(), preCountersByOrder.end());
    // add fixed counters to end of list
    counterIDs.insert(counterIDs.end(), postCountersByOrder.begin(), postCountersByOrder.end());
}

//----------------------------------------------------------------------------
void ppCountersSortOrder::GetCountersByName(AMDTPwrCategory category, gtMap < QString, int >& countersByName)
{
    ppAppController& appController = ppAppController::instance();
    const gtMap <AMDTPwrCategory, gtSet <int>> allCounters = appController.GetAllCountersByCategory();
    gtSet<int> categoryCounters;

    countersByName.clear();

    if (allCounters.count(category) != 0)
    {
        categoryCounters = allCounters.at(category);

        for (int counterId : categoryCounters)
        {
            ppControllerCounterData* counterData = appController.GetCounterInformationById(counterId);

            if (nullptr != counterData)
            {
                countersByName[counterData->m_name] = counterId;
            }
        }
    }
}

// ---------------------------------------------------------------------------
void ppCountersSortOrder::InitSortingData(AMDTPwrCategory category, const std::vector<std::string>& orderedCounterNames)
{
    gtMap < QString, int > countersMap;
    GetCountersByName(category, countersMap);

    // Create a vector that contains Ids only of the counters that are present in the counters map
    for (auto counterName : orderedCounterNames)
    {
        QString qCounterName = QString::fromStdString(counterName);
        // If the counter is in the map
        auto it = countersMap.find(qCounterName);

        if (it != countersMap.end())
        {
            // Push the counter id into the ordered vector of active counters
            int counterId = it->second;
            m_prefixByCategory[category].push_back(counterId);
        }
    }
}

// ---------------------------------------------------------------------------
void ppCountersSortOrder::InitSortingData()
{
    m_isInitialised = true;

    // Set the order of the Power counters
    std::vector<std::string> orderedPowerCounters =
    {
        PP_STR_Counter_Power_TotalAPU,
        PP_STR_Counter_Power_Other,
        PP_STR_Counter_Power_RoC,
        PP_STR_Counter_Power_DisplayCtrl,
        PP_STR_Counter_Power_GFXCtrl,
        PP_STR_Counter_Power_MemCtrl,
        PP_STR_Counter_Power_PCIECtrl,
        PP_STR_Counter_Power_IOCtrl,
        PP_STR_Counter_Power_NB,
        PP_STR_Counter_Power_UVD,
        PP_STR_Counter_Power_VCE,
        PP_STR_Counter_Power_ACP,
        PP_STR_Counter_Power_UNB,
        PP_STR_Counter_Power_SMU,
        PP_STR_Counter_Power_IGPU,
        PP_STR_Counter_Power_GFX,
        PP_STR_Counter_Power_CU1,
        PP_STR_Counter_Power_CU0
    };
    InitSortingData(AMDT_PWR_CATEGORY_POWER, orderedPowerCounters);

    // Set the order of the Temperature counters
    std::vector<std::string> orderedTempCounters =
    {
        PP_STR_Counter_Temp_IGPU,
        PP_STR_Counter_Temp_GFX,
        PP_STR_Counter_Temp_CU1,
        PP_STR_Counter_Temp_CU0
    };
    InitSortingData(AMDT_PWR_CATEGORY_TEMPERATURE, orderedTempCounters);

    // Set the order of the Frequency counters
    std::vector<std::string> orderedFreqCounters =
    {
        PP_STR_Counter_Freq_IGPU,
        PP_STR_Counter_AvgFreq_IGPU,
        PP_STR_Counter_AvgFreq_GFX,
        PP_STR_Counter_Freq_Core3,
        PP_STR_Counter_AvgFreq_Core3,
        PP_STR_Counter_Freq_Core2,
        PP_STR_Counter_AvgFreq_Core2,
        PP_STR_Counter_Freq_Core1,
        PP_STR_Counter_AvgFreq_Core1,
        PP_STR_Counter_Freq_Core0,
        PP_STR_Counter_AvgFreq_Core0
    };
    InitSortingData(AMDT_PWR_CATEGORY_FREQUENCY, orderedFreqCounters);

    // Set the order of the Current counters
    std::vector<std::string> orderedCurrentCounters =
    {
        PP_STR_Counter_Current_NB,
        PP_STR_Counter_Current_CPUCore
    };
    InitSortingData(AMDT_PWR_CATEGORY_CURRENT, orderedCurrentCounters);

    // Set the order of the Voltage counters
    std::vector<std::string> orderedVoltageCounters =
    {
        PP_STR_Counter_Voltage_NB,
        PP_STR_Counter_Voltage_CPUCore
    };
    InitSortingData(AMDT_PWR_CATEGORY_VOLTAGE, orderedVoltageCounters);

    // Set the order of the DVFS counters
    std::vector<std::string> orderedDVFSCounters =
    {
        PP_STR_Counter_PState_Core3,
        PP_STR_Counter_PState_Core2,
        PP_STR_Counter_PState_Core1,
        PP_STR_Counter_PState_Core0
    };
    InitSortingData(AMDT_PWR_CATEGORY_DVFS, orderedDVFSCounters);

}

// ---------------------------------------------------------------------------


