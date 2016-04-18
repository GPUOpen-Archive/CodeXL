//==================================================================================
// Copyright (c) 2012-2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file ProcessMonitor.cpp
/// \brief  The monitoring thread of the launched process during a profile
///
//==================================================================================
// $Id: //devtools/main/CodeXL/Components/CpuProfiling/AMDTCpuProfiling/src/ProcessMonitor.cpp#35 $
// Last checkin:   $DateTime: 2016/04/18 06:02:03 $
// Last edited by: $Author:  AMD Developer Tools Team
// Change list:    $Change: 569613 $
//=============================================================

// Qt:
#include <QtCore>
#include <QtWidgets>

// AMDTBaseTools
#include <AMDTBaseTools/Include/gtAssert.h>

// Backend
#include <AMDTCpuProfilingControl/inc/CpuProfileControl.h>

//AMDTOSWrapper
#include <AMDTOSWrappers/Include/osProcess.h>

// Infra:
#include <AMDTAPIClasses/Include/Events/apEventsHandler.h>
#include <AMDTAPIClasses/Include/Events/apProfileProcessTerminatedEvent.h>
#include <AMDTAPIClasses/Include/Events/apProfileProgressEvent.h>
#include <AMDTApplicationComponents/Include/acFunctions.h>

// AMDTSharedProfiling:
#include <AMDTSharedProfiling/inc/SharedProfileManager.h>
#include <AMDTSharedProfiling/inc/SharedProfileSettingPage.h>

// Local:
#include <inc/ProcessMonitor.h>
#include <inc/CpuProjectHandler.h>
#include <inc/StringConstants.h>
#include <inc/CommandsHandler.h>


ProfileProcessMonitor::ProfileProcessMonitor(osProcessId launchedProcessId, CPUSessionTreeItemData* pSession,
                                             osTime overheadStamp, bool attached):
    osThread(L"ProfileProcessMonitor"), m_launcherProcessId(launchedProcessId), m_pSession(pSession),
    m_processEnded(false), m_attachedProcess(attached), m_actualDuration(0)
{
    osTime current;
    current.setFromCurrentTime();
    m_overheadSec = current.secondsFrom1970() - overheadStamp.secondsFrom1970();
}

ProfileProcessMonitor::~ProfileProcessMonitor()
{
}

void ProfileProcessMonitor::fillDuration(const wchar_t* label, gtUInt32 halfSeconds)
{
    gtString durationStr = acDurationAsString(halfSeconds / 2);
    static const gtString profileName(CPU_STR_PROJECT_EXTENSION);
    durationStr.prepend(label);
    apProfileProgressEvent eve(profileName, durationStr, 0);
    eve.setIncrement(true);
    apEventsHandler::instance().registerPendingDebugEvent(eve);
}

//Overrides osThread
int ProfileProcessMonitor::entryPoint()
{
    bool bDone(false);
    GT_IF_WITH_ASSERT(m_pSession != nullptr)
    {
        bool childProcess = !m_attachedProcess;

        if (m_pSession->m_startDelay > 0)
        {
            //wait for start delay seconds
            for (int i = 1; i <= (m_pSession->m_startDelay * 2); ++i, ++m_actualDuration)
            {
                if (PROCESS_ID_UNBOUND != m_launcherProcessId)
                {
                    //check if the process terminated during the start delay
                    bDone = osWaitForProcessToTerminate(m_launcherProcessId, 500, nullptr, childProcess);

                    if (bDone)
                    {
                        break;
                    }
                }
                else
                {
                    osSleep(500);
                }

                fillDuration(L"Profile delay: ", i);
            }

            //If need to resume profile control
            if ((!bDone) && (!m_pSession->m_isProfilePaused) && (!CommandsHandler::instance()->isPaused()))
            {
                fnResumeProfiling(nullptr);
            }
        }

        bool isProfileScheduled = (m_pSession->m_profileDuration > 0);

        if (!isProfileScheduled)
        {
            gtUInt32 i = 0;

            if (PROCESS_ID_UNBOUND != m_launcherProcessId)
            {
                //check every .5 seconds if the process terminated
                while (!osWaitForProcessToTerminate(m_launcherProcessId, 500, nullptr, childProcess))
                {
                    ++m_actualDuration;
                    fillDuration(L"Profile duration: ", ((m_overheadSec * 2) + i++));
                }
            }
            else
            {
                for (;;)
                {
                    osSleep(500);

                    ++m_actualDuration;
                    fillDuration(L"Profile duration: ", ((m_overheadSec * 2) + i++));
                }
            }
        }
        else
        {
            bool isDone(false);

            //wait for m_duration seconds, plus 0.5 for 0 based
            for (int i = 1; i <= (m_pSession->m_profileDuration * 2) + 1; ++i, ++m_actualDuration)
            {
                //check if the process terminated during the start delay
                if (!isDone && PROCESS_ID_UNBOUND != m_launcherProcessId)
                {
                    isDone = osWaitForProcessToTerminate(m_launcherProcessId, 500, nullptr, childProcess);

                    if (isDone)
                    {
                        break;
                    }
                }
                else
                {
                    //Wait for the m_duration to expire or the user to click stop
                    osSleep(500);
                }

                fillDuration(L"Profile duration: ", ((m_overheadSec * 2) + i));
            }

            if (!m_attachedProcess && m_pSession->m_terminateAfterDataCollectionIsDone && !isDone)
            {
                osTerminateProcess(m_launcherProcessId);
            }
        }

        m_processEnded = true;
        // Notify CpuProfiler that the profile should be terminated
        apProfileProcessTerminatedEvent eve(CPU_STR_PROJECT_EXTENSION, m_launcherProcessId);
        apEventsHandler::instance().registerPendingDebugEvent(eve);
    }
    return 0;
}

void ProfileProcessMonitor::beforeTermination()
{
    bool shouldTerminate = true;
    // Sanity check:
    GT_IF_WITH_ASSERT(m_pSession != nullptr)
    {
        shouldTerminate = (m_pSession->m_terminateAfterDataCollectionIsDone || (m_pSession->m_profileDuration <= 0));

        // Update the profile m_duration
        m_pSession->m_profileDuration = m_actualDuration / 2 + m_overheadSec;

        if (!m_attachedProcess && shouldTerminate)
        {
            //Kill the launched process
            osTerminateProcess(m_launcherProcessId);
        }
    }
}

bool ProfileProcessMonitor::processEnded()
{
    return m_processEnded;
}
