//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file SharedProfileProcessMonitor.cpp
///
//==================================================================================

//------------------------------ SharedProfileProcessMonitor.cpp ------------------------------

// Qt:
#include <QtCore>
#include <QtWidgets>

// AMDTBaseTools
#include <AMDTBaseTools/Include/gtAssert.h>

//AMDTOSWrapper
#include <AMDTOSWrappers/Include/osProcess.h>

// Infra:
#include <AMDTAPIClasses/Include/Events/apEventsHandler.h>
#include <AMDTAPIClasses/Include/Events/apProfileProcessTerminatedEvent.h>

// AMDTApplicationFramework:
#include <AMDTApplicationFramework/src/afUtils.h>

// AMDTSharedProfiling:
#include <AMDTSharedProfiling/inc/SharedProfileManager.h>
#include <AMDTSharedProfiling/inc/SharedProfileProcessMonitor.h>


SharedProfileProcessMonitor::SharedProfileProcessMonitor(osProcessId launchedProcessId, const gtString& profileFileExtension) :
    osThread(L"SharedProfileProcessMonitor"), m_launcherProcessId(launchedProcessId), m_processEnded(false), m_profileFileExtension(profileFileExtension)
{
}

SharedProfileProcessMonitor::~SharedProfileProcessMonitor()
{
}

//Overrides osThread
int SharedProfileProcessMonitor::entryPoint()
{
    bool bDone(false);

    //check if the process terminated during the start delay
    if (PROCESS_ID_UNBOUND != m_launcherProcessId)
    {
        while (!bDone && PROCESS_ID_UNBOUND != m_launcherProcessId)
        {
            bDone = osWaitForProcessToTerminate(m_launcherProcessId, 500, NULL);
        }
    }

    m_processEnded = true;

    // Notify the profile that the profile should be terminated
    apProfileProcessTerminatedEvent eve(m_profileFileExtension, m_launcherProcessId);
    apEventsHandler::instance().registerPendingDebugEvent(eve);

    return 0;
}

void SharedProfileProcessMonitor::beforeTermination()
{
    osTerminateProcess(m_launcherProcessId);
}

bool SharedProfileProcessMonitor::processEnded()
{
    return m_processEnded;
}
