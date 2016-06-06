//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file acStackedBarGraphhData.h
///
//==================================================================================


#ifndef ACSTACKEDBARGRAPHHDATA
#define ACSTACKEDBARGRAPHHDATA

#include <AMDTApplicationComponents/Include/acBarGraphData.h>

#include <AMDTApplicationComponents/Include/acApplicationComponentsDLLBuild.h>

class AC_API acStackedBarGraphhData : public acBarGraphData
{
public:
    acStackedBarGraphhData() {}

    ~acStackedBarGraphhData()
    {
        int count = m_xLabels.count();

        for (int i = 0; i < count; i++)
        {
            delete m_xLabels[i];
        }

        m_xLabels.clear();
    }

    QVector<acBarName*> m_xLabels;
    QColor m_barsColor;
    QString m_graphName;
};

#endif