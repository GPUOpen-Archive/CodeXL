//=====================================================================
// Copyright (c) 2012 Advanced Micro Devices, Inc. All rights reserved.
//
/// \author GPU Developer Tools
/// \file $File: //devtools/main/CodeXL/Components/GpuProfiling/AMDTGpuProfiling/gpSessionView.h $
/// \version $Revision: #40 $
/// \brief  This file contains gpSessionView class
//
//=====================================================================
// $Id: //devtools/main/CodeXL/Components/GpuProfiling/AMDTGpuProfiling/gpSessionView.h#40 $
// Last checkin:   $DateTime: 2015/07/05 04:24:05 $
// Last edited by: $Author: salgrana $
// Change list:    $Change: 533050 $
//=====================================================================

#ifndef __GPBASESESSIONVIEW_H_
#define __GPBASESESSIONVIEW_H_

#include <qtIgnoreCompilerWarnings.h>
#include <AMDTBaseTools/Include/gtIgnoreCompilerWarnings.h>

// AMDTSharedProfiling:
#include <AMDTSharedProfiling/inc/SharedSessionWindow.h>

class KernelOccupancyWindow;
class SessionViewTabWidget;
class CodeViewerWindow;
class gpTraceDataModel;
class gpObjectDataModel;

/// Base class used for all GPU session windows.
/// This class will implement:
/// 1. Unified UI look for all GPU session views
/// 2. Shared functionality among all views
class gpBaseSessionView : public SharedSessionWindow
{
    Q_OBJECT

public:

    /// Initializes a new instance of the gpSessionView class.
    /// \param parent the parent widget
    gpBaseSessionView(QWidget* parent);

    /// Destructor
    ~gpBaseSessionView();

    /// Set the data model
    virtual void SetProfileDataModel(gpTraceDataModel* pSessionDataModel) { m_pSessionDataModel = pSessionDataModel; };

    /// Set the object data model
    virtual void SetProfileObjectDataModel(gpObjectDataModel* pSessionObjectDataModel) { m_pSessionObjectDataModel = pSessionObjectDataModel; };

protected slots:

    /// Is handling the occupancy file generation completion:
    /// \param did the file generation succeed?
    /// \param strError if the file generation failed, strError will contain the error description
    /// \param strOccupancyHTMLFileName occupancy file path
    void OnOccupancyFileGenerationFinish(bool success, const QString& strError, const QString& strOccupancyHTMLFileName);

    /// handler for when the user tries to close a tab
    /// \param index the index of the tab being closed
    void OnTabClose(int index);

    // Window activation:
    void OnAboutToActivate();


protected:

    /// Session data model
    gpTraceDataModel* m_pSessionDataModel;

    /// Session Object data model
    gpObjectDataModel* m_pSessionObjectDataModel;

    /// Tab widget displaying the session results
    SessionViewTabWidget*  m_pSessionTabWidget;

    /// Instance of kernel occupancy window
    KernelOccupancyWindow* m_pKernelOccupancyWindow;

    ///< index of kernel occupancy tab
    int m_kernelOccupancyTabIndex;

    /// Name of the kernel for which the occupancy is currently showing
    QString m_currentDisplayedOccupancyKernel;

    /// Instance of code viewer control
    CodeViewerWindow* m_pCodeViewerWindow;

    /// Index of code viewer tab
    int m_codeViewerTabIndex;

    /// True iff the trace view is currently being loaded
    bool m_firstActivation;

};


#endif // __GPBASESESSIONVIEW_H_
