//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file acBarGraphData.h
///
//==================================================================================

#ifndef ACBARGRAPHDATA_H
#define ACBARGRAPHDATA_H

#include <qvector.h>

#include <AMDTApplicationComponents/Include/acApplicationComponentsDLLBuild.h>

class AC_API acBarGraphData
{
    //abstract class
protected:
    acBarGraphData() {}

public:
    ~acBarGraphData()
    {
        m_yData.clear();
    }

    QVector<double> m_yData;
};

#endif