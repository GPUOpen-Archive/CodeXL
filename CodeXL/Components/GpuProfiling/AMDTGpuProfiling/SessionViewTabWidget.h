//=====================================================================
// Copyright (c) 2012 Advanced Micro Devices, Inc. All rights reserved.
//
/// \author GPU Developer Tools
/// \file $File: //devtools/main/CodeXL/Components/GpuProfiling/AMDTGpuProfiling/SessionViewTabWidget.h $
/// \version $Revision: #5 $
/// \brief  This file contains SessionViewTabWidget class, which prevents the first tab from being closed and allows the middle button to close closable tabs
//
//=====================================================================
// $Id: //devtools/main/CodeXL/Components/GpuProfiling/AMDTGpuProfiling/SessionViewTabWidget.h#5 $
// Last checkin:   $DateTime: 2015/08/31 01:56:31 $
// Last edited by: $Author: salgrana $
// Change list:    $Change: 538740 $
//=====================================================================

#ifndef _SESSION_VIEW_TAB_WIDGET_H_
#define _SESSION_VIEW_TAB_WIDGET_H_

#include <qtIgnoreCompilerWarnings.h>
#include <AMDTBaseTools/Include/gtIgnoreCompilerWarnings.h>

#include <QtCore>
#include <QtWidgets>


/// QTabWidget descendant, which prevents the first tab from being closed and allows the middle button to close closable tabs
class SessionViewTabWidget : public QTabWidget
{
public:

    /// Initializes a new instance of the SessionViewTabWidget class
    /// \param parent the parent object
    SessionViewTabWidget(QWidget* parent = 0);

protected:
    /// Overridden method called when a tab in inserted
    /// \param index the index of the tab inserted
    virtual void tabInserted(int index);

    /// Overridden method called when the mouse is pressed
    /// \param mouseEvent the mouse event parameters
    virtual void mousePressEvent(QMouseEvent* mouseEvent);
};


#endif //_SESSION_VIEW_TAB_WIDGET_H;
