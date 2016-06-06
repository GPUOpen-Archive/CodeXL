//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file acTabWidget.cpp
///
//==================================================================================

//------------------------------ acTabWidget.cpp ------------------------------

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>

// Local:
#include <AMDTApplicationComponents/Include/acTabWidget.h>



// ---------------------------------------------------------------------------
// Name:        acTabWidget::acTabWidget
// Description: Constructor
// Arguments:   QWidget* pParent
// Author:      Sigal Algranaty
// Date:        12/6/2012
// ---------------------------------------------------------------------------
acTabWidget::acTabWidget(QWidget* pParent)
{
    GT_UNREFERENCED_PARAMETER(pParent);
}

// ---------------------------------------------------------------------------
// Name:        acTabWidget::~acTabWidget
// Description: Destructor
// Author:      Sigal Algranaty
// Date:        12/6/2012
// ---------------------------------------------------------------------------
acTabWidget::~acTabWidget()
{

}

// ---------------------------------------------------------------------------
// Name:        acTabWidget::setTabBarVisible
// Description: Set the tab bar visibility
// Arguments:   bool isVisible
// Author:      Sigal Algranaty
// Date:        12/6/2012
// ---------------------------------------------------------------------------
void acTabWidget::setTabBarVisible(bool isVisible)
{
    GT_IF_WITH_ASSERT(tabBar() != NULL)
    {
        tabBar()->setVisible(isVisible);
    }
}
