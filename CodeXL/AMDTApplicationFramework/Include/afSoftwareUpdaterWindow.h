//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file afSoftwareUpdaterWindow.h
///
//==================================================================================

#ifndef __AFSOFTWAREUPDATERWINDOW_H
#define __AFSOFTWAREUPDATERWINDOW_H

// Infra:
#include <AMDTBaseTools/Include/gtString.h>

// Local:
#include <AMDTApplicationFramework/Include/afApplicationFrameworkDLLBuild.h>
#include <AMDTApplicationComponents/Include/acSoftwareUpdaterWindow.h>


/// -----------------------------------------------------------------------------------------------
/// \class Name: afSoftwareUpdaterWindow
/// \brief Description:  The GUI for the updater object
/// -----------------------------------------------------------------------------------------------
class AF_API afSoftwareUpdaterWindow
{

public:

    friend class afMainAppWindow;

    /// constructor
    afSoftwareUpdaterWindow();

    /// Destructor
    ~afSoftwareUpdaterWindow();

    // Display the updater dialog:
    void displayDialog(bool forceDialogDisplay = true);
    void performAutoCheckForUpdate();

private:

    acSoftwareUpdaterWindow* m_pSoftWareUpdaterCtrl;

};



#endif //__AFSOFTWAREUPDATERWINDOW_H

