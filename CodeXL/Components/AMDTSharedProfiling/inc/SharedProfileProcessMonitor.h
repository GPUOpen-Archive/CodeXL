//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file SharedProfileProcessMonitor.h
///
//==================================================================================

//------------------------------ SharedProfileProcessMonitor.h ------------------------------

#ifndef _SHAREDPROFILEPROCESSMONITOR_H
#define _SHAREDPROFILEPROCESSMONITOR_H

// Qt:
#include <QtCore>
#include <QtWidgets>

// Infra:
#include <AMDTOSWrappers/Include/osThread.h>

// Local:
#include "LibExport.h"

class AMDTSHAREDPROFILING_API SharedProfileProcessMonitor : public osThread
{
public:
    SharedProfileProcessMonitor(osProcessId launchedProcessId, const gtString& profileFileExtension);
    ~SharedProfileProcessMonitor();

    /// Overrides osThread:
    virtual int entryPoint();
    virtual void beforeTermination();

    bool processEnded();

private:

    /// Disallow use of my default constructor:
    SharedProfileProcessMonitor();

    /// The process Id of the launcher process:
    osProcessId m_launcherProcessId;

    /// stores if the process was ended so the thread can be killed if needed:
    bool m_processEnded;


    /// Contain the extension for the executed profile session:
    gtString m_profileFileExtension;

};

#endif //_SHAREDPROFILEPROCESSMONITOR_H
