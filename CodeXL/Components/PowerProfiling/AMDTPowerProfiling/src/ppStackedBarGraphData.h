//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file ppStackedBarGraphData.h
///
//==================================================================================


#ifndef PPSTACKEDBARGRAPHHDATA
#define PPSTACKEDBARGRAPHHDATA

// Local.
#include <AMDTPowerProfiling/src/ppSessionController.h>

// Framework:
#include <AMDTApplicationComponents/Include/acStackedBarGraphhData.h>

// Infra.
#include <AMDTBaseTools/Include/gtVector.h>


class ppStackedBarGraphData : public acStackedBarGraphhData
{
public:
    ppStackedBarGraphData(const gtVector<HistogramBucket>& buckets, const QColor& color, const QString& name, double valueDevUnit)
    {
        m_graphName = name;
        m_barsColor = color;

        int barNumber = buckets.size();

        for (int i = 0; i < barNumber; i++)
        {
            m_xLabels << new acBarName(buckets[i].m_lowerBound, buckets[i].m_upperBound);
            m_yData << buckets[i].m_value / valueDevUnit;
        }
    }
};

#endif