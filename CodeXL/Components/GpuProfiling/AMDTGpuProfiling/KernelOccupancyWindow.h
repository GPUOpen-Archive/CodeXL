//=====================================================================
// Copyright (c) 2012 Advanced Micro Devices, Inc. All rights reserved.
//
/// \author GPU Developer Tools
/// \file $File: //devtools/main/CodeXL/Components/GpuProfiling/AMDTGpuProfiling/KernelOccupancyWindow.h $
/// \version $Revision: #8 $
/// \brief :  This file contains KernelOccupancyWindow class
//
//=====================================================================
// $Id: //devtools/main/CodeXL/Components/GpuProfiling/AMDTGpuProfiling/KernelOccupancyWindow.h#8 $
// Last checkin:   $DateTime: 2015/08/31 01:56:31 $
// Last edited by: $Author: salgrana $
// Change list:    $Change: 538740 $
//=====================================================================
#ifndef _KERNEL_OCCUPANCY_WINDOW_H_
#define _KERNEL_OCCUPANCY_WINDOW_H_

#include <qtIgnoreCompilerWarnings.h>
#include <AMDTBaseTools/Include/gtIgnoreCompilerWarnings.h>

#include <QtCore>
#include <QtWidgets>
#include <QtWebKitWidgets/QWebView>

/// UI for Kernel Occupancy view
class KernelOccupancyWindow : public QWidget
{
public:
    /// Initializes a new instance of the KernelOccupancyWindow class.
    KernelOccupancyWindow(QWidget* parent = 0);

    /// Clear Window
    void Clear();

    /// Loads the specified HTML file in the Occupancy window
    /// \param strOutputPage the full path to the Occupancy HTML file to load
    bool LoadOccupancyHTMLFile(const QString& strOutputPage);

protected:
    /// Overridden handler to automatically reload the occupancy web page when the view is resized horizontally
    /// \param event the event params
    void resizeEvent(QResizeEvent* event);

private:
    /// Widget for viewing kernel occupancy
    QWebView* m_pWebBrowser;
};


#endif // _KERNEL_OCCUPANCY_WINDOW_H_

