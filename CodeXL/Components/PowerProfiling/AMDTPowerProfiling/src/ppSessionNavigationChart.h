//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file ppSessionNavigationChart.h
///
//==================================================================================

//------------------------------ kaExportBinariesDialog.h ------------------------------
#ifndef __SESSIONNAVIGATIONCHART_H
#define __SESSIONNAVIGATIONCHART_H

#include <qtIgnoreCompilerWarnings.h>
#include <QtWidgets>
#include <qobject.h>
#include <qcustomplot.h>

// Local:
#include <AMDTPowerProfiling/src/ppSessionView.h>

// AMDTApplicationComponents
#include <AMDTApplicationComponents/Include/acNavigationChart.h>


class ppSessionNavigationChart : public acNavigationChart
{
    Q_OBJECT

public:
    ///constructor
    ppSessionNavigationChart(QWidget* pParent, ppSessionController* pSessionController, const QString& dataLabel);

    virtual ~ppSessionNavigationChart() {};

private:
    /// Pointer to session controller:
    ppSessionController* m_pSessionController;
};



#endif // __SESSIONNAVIGATIONCHART_H


