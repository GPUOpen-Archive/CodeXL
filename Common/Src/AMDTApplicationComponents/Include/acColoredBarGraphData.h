//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file acColoredBarGraphData.h
///
//==================================================================================

#ifndef ACCOLOREDBARGRAPHDATA
#define ACCOLOREDBARGRAPHDATA

// Qt
#include <QtGui>

#include <AMDTApplicationComponents/Include/acBarGraphData.h>

#include <AMDTApplicationComponents/Include/acApplicationComponentsDLLBuild.h>

class AC_API acColoredBarGraphData : public acBarGraphData
{
public:
    acColoredBarGraphData() : acBarGraphData()
    {
    }

    ~acColoredBarGraphData()
    {
        m_barsColors.clear();
        m_xLabels.clear();
    }

    QVector<QColor> m_barsColors;
    QVector<QString> m_xLabels;
};

#endif
