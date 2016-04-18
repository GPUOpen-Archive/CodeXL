//=====================================================================
// Copyright (c) 2012 Advanced Micro Devices, Inc. All rights reserved.
//
/// \author GPU Developer Tools
/// \file $File: //devtools/main/CodeXL/Components/GpuProfiling/AMDTGpuProfiling/SessionViewTabWidget.cpp $
/// \version $Revision: #6 $
/// \brief  This file contains SessionViewTabWidget class, which prevents the first tab from being closed and allows the middle button to close closable tabs
//
//=====================================================================
// $Id: //devtools/main/CodeXL/Components/GpuProfiling/AMDTGpuProfiling/SessionViewTabWidget.cpp#6 $
// Last checkin:   $DateTime: 2015/08/31 01:56:31 $
// Last edited by: $Author: salgrana $
// Change list:    $Change: 538740 $
//=====================================================================

#include "SessionViewTabWidget.h"


SessionViewTabWidget::SessionViewTabWidget(QWidget* parent) : QTabWidget(parent)
{
}

void SessionViewTabWidget::tabInserted(int index)
{
    // hide the close button on the first tab
    if (index == 0)
    {
        tabBar()->tabButton(index, QTabBar::RightSide)->hide();
    }
}

void SessionViewTabWidget::mousePressEvent(QMouseEvent* mouseEvent)
{
    // close the tab in response to the middle mouse button click
    int tabIndex = tabBar()->tabAt(mouseEvent->pos());

    if (mouseEvent->button() == Qt::MiddleButton && tabIndex != 0)
    {
        emit tabCloseRequested(tabIndex);
    }
}
