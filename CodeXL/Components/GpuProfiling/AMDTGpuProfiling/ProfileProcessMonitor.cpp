//=============================================================
// (c) 2012 Advanced Micro Devices, Inc.
//
/// \author franksw
/// \version $Revision: #21 $
/// \brief  A brief file description that Doxygen makes note of.
//
//=============================================================
// $Id: //devtools/main/CodeXL/Components/GpuProfiling/AMDTGpuProfiling/ProfileProcessMonitor.cpp#21 $
// Last checkin:   $DateTime: 2016/04/18 06:02:03 $
// Last edited by: $Author: salgrana $
// Change list:    $Change: 569613 $
//=============================================================

#ifdef _WIN32
    #pragma warning(push)
    #pragma warning(disable: 4127)
    #pragma warning(disable: 4251)
    #pragma warning(disable: 4512)
    #pragma warning(disable: 4800)
#endif
#include <QtCore>
#include <QtWidgets>

#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTOSWrappers/Include/osProcess.h>
#include <AMDTAPIClasses/Include/Events/apEventsHandler.h>
#include <AMDTAPIClasses/Include/Events/apProfileProcessTerminatedEvent.h>
#include <AMDTAPIClasses/Include/Events/apProfileProgressEvent.h>
#include <AMDTApplicationFramework/Include/afProgressBarWrapper.h>
#include <AMDTApplicationFramework/src/afUtils.h>

#include <AMDTGpuProfiling/AMDTGpuProfilerDefs.h>
#include "ProfileProcessMonitor.h"
#ifdef _WIN32
#pragma warning(pop)
#endif

// Sigal 5/30/2013:
// Currently the duration displayed in GPU profile is not accurate, since it also displays the time
// used to post process the session in CodeXLGpuProfiler.exe. In the future, we will implement a "real" duration
// report, and then this flag can be enabled
// #define SHOW_PROFILE_DURATION

ProfileProcessMonitor::ProfileProcessMonitor(osProcessId launchedProcessId, ProfileServerRunType runType, bool showProgress) :
    osThread(L"ProfileProcessMonitor"), m_launcherProcessId(launchedProcessId), m_exitCode(0), m_runType(runType)
{
    // Initialize the progress message according to the run type:
    gtString strProgressMsg;

    switch (runType)
    {
        case ProfileServerRunType_Profile:
            m_strProgressMessage = GPU_STR_ProcessMonitorRunType_Profile;
            break;

        case ProfileServerRunType_GenSummary:
            m_strProgressMessage = GPU_STR_ProcessMonitorRunType_GenSummary;
            break;

        case ProfileServerRunType_GenOccupancy:
            m_strProgressMessage = GPU_STR_ProcessMonitorRunType_GenOccupancy;
            break;

        case PerfStudioServerRunType_Application:
            m_strProgressMessage = GPU_STR_ProcessMonitorRunType_Geneneric;
            break;

        default:
            GT_ASSERT(false);
            break;
    }

    if (showProgress)
    {
        afProgressBarWrapper::instance().setProgressDetails(m_strProgressMessage, 0);
    }
}

ProfileProcessMonitor::~ProfileProcessMonitor()
{
    afProgressBarWrapper::instance().hideProgressBar();
}

int ProfileProcessMonitor::entryPoint()
{
    unsigned long i = 0;


#ifdef SHOW_PROFILE_DURATION
    m_strProgressMessage = L"Profile duration: ";
#endif

    //check every .5 seconds if the process terminated
    while (!osWaitForProcessToTerminate(m_launcherProcessId, 500, &m_exitCode))
    {
        updateStatus(m_strProgressMessage, i++);
    }

    return 0;
}

void ProfileProcessMonitor::beforeTermination()
{
    // Notify GpuProfiler that the profile should be terminated
    apProfileProcessTerminatedEvent eve(GPU_STR_GENERAL_SETTINGS, m_exitCode, (int)m_runType);
    apEventsHandler::instance().registerPendingDebugEvent(eve);
    osTerminateProcess(m_launcherProcessId);
}

void ProfileProcessMonitor::updateStatus(const gtString& status, unsigned long halfSeconds)
{
    (void)(halfSeconds); // Unused variable
    gtString durationLabel = status;

#ifdef SHOW_PROFILE_DURATION

    gtString durationStr = afUtils::durationAsString(halfSeconds / 2);
    durationLabel.append(durationStr);
#else
    static gtString dots = L".";
    int dotsLength = dots.length();

    if (dotsLength == 1)
    {
        dots = L"..";
    }
    else if (dotsLength == 2)
    {
        dots = L"...";
    }
    else if (dotsLength == 3)
    {
        dots = L".";
    }

    durationLabel.append(dots);
#endif
    static const gtString profileName(GPU_STR_GENERAL_SETTINGS);
    apProfileProgressEvent eve(profileName, durationLabel, 0);
    apEventsHandler::instance().registerPendingDebugEvent(eve);
}
