//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file ppSessionNavigationChart.cpp
///
//==================================================================================

//------------------------------ SessionNavigationChart.cpp ------------------------------

// Local:
#include <AMDTPowerProfiling/src/ppSessionNavigationChart.h>
#include <AMDTPowerProfiling/src/ppColors.h>
#include <AMDTPowerProfiling/src/ppGUIDefs.h>

ppSessionNavigationChart::ppSessionNavigationChart(QWidget* pParent,
                                                   ppSessionController* pSessionController,
                                                   const QString& dataLabel) :
    acNavigationChart(pParent, dataLabel)
{
    m_pSessionController = pSessionController;

    // initial sampling interval
    GT_IF_WITH_ASSERT(pSessionController != nullptr)
    {
        SetInterval(pSessionController->GetSamplingTimeInterval());
    }

    // set the y axis format
    SetNavigationUnitsY(acNavigationChart::eNavigationSingleUnits);
}