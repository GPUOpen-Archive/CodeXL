//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file appMDIArea.cpp
///
//==================================================================================

#include <QtWidgets>

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTBaseTools/Include/gtIgnoreCompilerWarnings.h>

// AMDTApplicationFramework:
#include <AMDTApplicationFramework/Include/afMainAppWindow.h>

// Local:
#include <inc/appMDIArea.h>

// ---------------------------------------------------------------------------
// Name:        appMDIArea::appMDIArea
// Description: Constructor
// Author:      Sigal Algranaty
// Date:        1/8/2011
// ---------------------------------------------------------------------------
appMDIArea::appMDIArea()
{
    // Set the tab shape:
    setTabShape(QTabWidget::Rounded);

    // Set the view mode:
    setViewMode(QMdiArea::TabbedView);
    setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
}

// ---------------------------------------------------------------------------
// Name:        appMDIArea::~appMDIArea
// Description: Destructor
// Author:      Sigal Algranaty
// Date:        1/8/2011
// ---------------------------------------------------------------------------
appMDIArea::~appMDIArea()
{
}

// ---------------------------------------------------------------------------
// Name:        appMDIArea::closeActiveSubWindow
// Description: Implementing MDI area slot
// Return Val:  void
// Author:      Sigal Algranaty
// Date:        1/8/2011
// ---------------------------------------------------------------------------
void appMDIArea::closeActiveSubWindow()
{
    afMainAppWindow* pAppWindow = afMainAppWindow::instance();
    GT_IF_WITH_ASSERT(pAppWindow != NULL)
    {
        pAppWindow->updateToolbars();
    }
}
