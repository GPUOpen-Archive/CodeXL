//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file acDockWidget.h
///
//==================================================================================

//------------------------------ acDockWindow.h ------------------------------

#ifndef __ACDOCKWIDGET
#define __ACDOCKWIDGET

// Infra:
#include <AMDTBaseTools/Include/AMDTDefinitions.h>
#include <AMDTApplicationComponents/Include/acApplicationComponentsDLLBuild.h>

// Local
#include "QDockWidget.h"

// ----------------------------------------------------------------------------------
// Class Name: acDockWidget : public QDockWidget
// General Description:
// Author:               Navin Patel
// Creation Date:        23/5/2015
// ----------------------------------------------------------------------------------
class AC_API acDockWidget : public QDockWidget
{
    Q_OBJECT

public:
    acDockWidget(const QString title, QWidget* parent);

signals:
    void ViewClose(QString& m_title);

protected:
    QString m_title;

    void closeEvent(QCloseEvent* pEvent)
    {
        GT_UNREFERENCED_PARAMETER(pEvent);
        emit ViewClose(m_title);
    }
};

#endif  // __ACDOCKWIDGET
