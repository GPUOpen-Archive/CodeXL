//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file ppColoredBarGraphData.h
///
//==================================================================================

#ifndef PPCOLOREDBARGRAPHDATA_H
#define PPCOLOREDBARGRAPHDATA_H

// Framework:
#include <AMDTBaseTools/Include/gtMap.h>
#include <AMDTApplicationComponents/Include/acColoredBarGraphData.h>

// Local:
#include <AMDTPowerProfiling/src/ppDataUtils.h>
#include <AMDTPowerProfiling/Include/ppStringConstants.h>

class ppColoredBarGraphData : public acColoredBarGraphData
{
public:
    ppColoredBarGraphData(const gtMap<int, double>& consumptionPerCounter, int apuId, ppSessionController* pSessionController, const gtVector<int>& counterIDs)
        : acColoredBarGraphData(), m_pSessionController(pSessionController)
    {
        if (m_pSessionController != nullptr)
        {
            int counterNum = counterIDs.size();

            // for all counter Ids in list
            for (int i = 0; i < counterNum; i++)
            {
                int id = counterIDs[i];

                // find id in data map
                GT_IF_WITH_ASSERT(consumptionPerCounter.count(id) > 0)
                {
                    // get bar color
                    m_barsColors << m_pSessionController->GetColorForCounter(id);

                    // get label
                    if (id == apuId)
                    {
                        m_xLabels << PP_STR_Counter_Power_Other;
                    }
                    else
                    {
                        // Get the counter name and cut the category off it:
                        QString counterName = m_pSessionController->GetCounterNameById(id);
                        ppDataUtils::CutCategoryFromCounterName(counterName);
                        m_xLabels << counterName;
                    }

                    // get value
                    m_yData << consumptionPerCounter.at(id) ;
                }
            }
        }
    }

private:
    ppSessionController* m_pSessionController;
};

#endif