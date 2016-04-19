//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gdGDebuggerGlobalVariablesManager.cpp
///
//==================================================================================

//------------------------------ gdGDebuggerGlobalVariablesManager.cpp ------------------------------

// Qt:
#include <QtWidgets>

// TinyXml
#include <tinyxml.h>

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTBaseTools/Include/gtAlgorithms.h>
#include <AMDTBaseTools/Include/AMDTDefinitions.h>
#include <AMDTBaseTools/Include/gtStringTokenizer.h>
#include <AMDTOSWrappers/Include/osApplication.h>
#include <AMDTOSWrappers/Include/osDebugLog.h>
#include <AMDTOSWrappers/Include/osDirectory.h>
#include <AMDTAPIClasses/Include/apBreakPoint.h>
#include <AMDTAPIClasses/Include/apMonitoredFunctionBreakPoint.h>
#include <AMDTAPIClasses/Include/apFrameTerminators.h>
#include <AMDTAPIClasses/Include/apMonitoredFunctionsManager.h>
#include <AMDTOSWrappers/Include/osProductVersion.h>
#include <AMDTAPIClasses/Include/Events/apBreakpointHitEvent.h>
#include <AMDTAPIClasses/Include/Events/apEventsHandler.h>
#include <AMDTAPIClasses/Include/Events/apKernelWorkItemChangedEvent.h>
#include <AMDTAPIClasses/Include/Events/apExceptionEvent.h>
#include <AMDTApiFunctions/Include/gaGRApiFunctions.h>

// AMDTApplicationFramework:
#include <AMDTApplicationFramework/Include/afAppStringConstants.h>

// Local:
#include <AMDTApplicationFramework/Include/afGlobalVariableChangedEvent.h>
#include <AMDTGpuDebuggingComponents/Include/gdGDebuggerGlobalVariablesManager.h>
#include <AMDTGpuDebuggingComponents/Include/gdStringConstants.h>

// Static member initializations:
gdGDebuggerGlobalVariablesManager* gdGDebuggerGlobalVariablesManager::_pMySingleInstance = NULL;

// ---------------------------------------------------------------------------
// Name:        gdGDebuggerGlobalVariablesManager::gdGDebuggerGlobalVariablesManager
// Description: Constructor
// Author:      Yaki Tebeka
// Date:        24/5/2004
// ---------------------------------------------------------------------------
gdGDebuggerGlobalVariablesManager::gdGDebuggerGlobalVariablesManager()
    :   _chosenContextId(), _chosenThreadIndex(-1), m_chosenThreadIsKernelDebugging(false), _triggeredByTheThread(false), _triggeredByTheContext(false),
        m_maxTreeItemsPerType(GD_DEFAULT_DEBUG_OBJECTS_TREE_MAX_ITEMS_PER_TYPE), _recording(false)
{
    osFilePath CodeXLBinariesPath;

    // Get the CodeXL application path:
    bool rc = CodeXLBinariesPath.SetInstallRelatedPath(osFilePath::OS_CODEXL_BINARIES_PATH);
    GT_IF_WITH_ASSERT(rc)
    {
        // Build the path in which OpenGL ES dlls reside:
#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
        // In Windows, the OpenGL ES spies are in the binaries directory:
        _openGLESProjectDLLsDirectory = CodeXLBinariesPath;
#endif
        m_currentProjectSettings.setDebuggerInstallDir(CodeXLBinariesPath);
    }

    m_currentProjectSettings.setExecutablePath(osFilePath(AF_STR_Empty));
    m_currentProjectSettings.setWorkDirectory(osFilePath(AF_STR_Empty));
    m_currentProjectSettings.setCommandLineArguments(AF_STR_Empty);
    bool rc2 = _openGLProjectSpiesDirectory.SetInstallRelatedPath(osFilePath::OS_CODEXL_SERVERS_PATH);
    GT_IF_WITH_ASSERT(rc2)
    {
        m_currentProjectSettings.setSpiesDirectory(_openGLProjectSpiesDirectory);
    }
#if ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT))
    // In Mac, the OpenGL ES and OpenGL spies are in the same directory:
    _openGLESProjectDLLsDirectory = _openGLProjectSpiesDirectory;
#endif

    // Default Frame terminators:
    m_currentProjectSettings.setFrameTerminators(AP_DEFAULT_FRAME_TERMINATORS);

    // Register myself to listen to debugged process events:
    apEventsHandler::instance().registerEventsObserver(*this, AP_GLOBAL_VARIABLES_MANAGER_EVENTS_HANDLING_PRIORITY);
}


// ---------------------------------------------------------------------------
// Name:        gdGDebuggerGlobalVariablesManager::~gdGDebuggerGlobalVariablesManager
// Description: Destructor
// Author:      Yaki Tebeka
// Date:        24/5/2004
// ---------------------------------------------------------------------------
gdGDebuggerGlobalVariablesManager::~gdGDebuggerGlobalVariablesManager()
{
    // Unregister myself from listening to debugged process events:
    apEventsHandler::instance().unregisterEventsObserver(*this);
}


// ---------------------------------------------------------------------------
// Name:        gdGDebuggerGlobalVariablesManager::instance
// Description: Returns the single instance of this class.
//              (If it does not exist - create it)
// Author:      Avi Shapira
// Date:        11/5/2004
// ---------------------------------------------------------------------------
gdGDebuggerGlobalVariablesManager& gdGDebuggerGlobalVariablesManager::instance()
{
    if (_pMySingleInstance == NULL)
    {
        _pMySingleInstance = new gdGDebuggerGlobalVariablesManager;
        GT_ASSERT(_pMySingleInstance);
    }

    return *_pMySingleInstance;
}

// ---------------------------------------------------------------------------
// Name:        gdGDebuggerGlobalVariablesManager::setChosenContext
// Description: Set the Id of the context that the application GUI currently
//              displays.
// Author:      Avi Shapira
// Date:        13/5/2004
// ---------------------------------------------------------------------------
void gdGDebuggerGlobalVariablesManager::setChosenContext(apContextID contextId)
{
    // If the chosen context was changed:
    if (_chosenContextId != contextId)
    {
        // Set the new chosen context id:
        _chosenContextId = contextId;

        // Trigger a global variable changed (chosen context) event:
        afGlobalVariableChangedEvent eve(afGlobalVariableChangedEvent::CHOSEN_CONTEXT_ID);
        apEventsHandler::instance().registerPendingDebugEvent(eve);

        // If this function call was not caused by a thread change:
        if (!_triggeredByTheThread)
        {
            // Will get the chosen context "current thread" index:
            int curThreadIndex = -1;

            // If this is an OpenGL context:
            if (contextId.isOpenGLContext())
            {
                // Get the context "current thread" id:
                osThreadId curThreadId = OS_NO_THREAD_ID;
                bool rc = gaGetRenderContextCurrentThread(contextId._contextId, curThreadId);

                if (rc && (curThreadId != OS_NO_THREAD_ID))
                {
                    // Translate the thread id to thread index:
                    curThreadIndex = getThreadIndex(curThreadId);
                }

                // Set the chosen thread:
                _triggeredByTheContext = true;
                setChosenThread(curThreadIndex, false);
            }
        }
    }

    // Clear the "called by thread change" flag:
    _triggeredByTheThread = false;
    _triggeredByTheContext = false;
}


// ---------------------------------------------------------------------------
// Name:        gdGDebuggerGlobalVariablesManager::setChosenThread
// Description: Set the Index of the thread that the application GUI currently
//              displays.
// Author:      Avi Shapira
// Date:        9/5/2005
// ---------------------------------------------------------------------------
void gdGDebuggerGlobalVariablesManager::setChosenThread(int threadIndex, bool kernelDebuggingThread)
{
    // If the thread index was changed:
    if ((_chosenThreadIndex != threadIndex) || (m_chosenThreadIsKernelDebugging != kernelDebuggingThread))
    {
        // Set the new thread index
        _chosenThreadIndex = threadIndex;
        m_chosenThreadIsKernelDebugging = kernelDebuggingThread;

        // Set the chosen work item accordingly:
        if (gaIsInKernelDebugging())
        {
            if (m_chosenThreadIsKernelDebugging && (0 < _chosenThreadIndex))
            {
                int firstActiveCoord[3] = { -1, -1, -1};
                bool rcFirst = gaGetFirstValidWorkItem(_chosenThreadIndex - 1, firstActiveCoord);
                GT_ASSERT(rcFirst);
                gaSetKernelDebuggingCurrentWorkItemCoordinate(0, firstActiveCoord[0]);
                gaSetKernelDebuggingCurrentWorkItemCoordinate(1, firstActiveCoord[1]);
                gaSetKernelDebuggingCurrentWorkItemCoordinate(2, firstActiveCoord[2]);
            }
            else // !m_chosenThreadIsKernelDebugging || 0 >= _chosenThreadIndex
            {
                gaSetKernelDebuggingCurrentWorkItemCoordinate(0, -1);
                gaSetKernelDebuggingCurrentWorkItemCoordinate(1, -1);
                gaSetKernelDebuggingCurrentWorkItemCoordinate(2, -1);
            }
        }

        // Trigger a global variable (chosen thread) changed event:
        afGlobalVariableChangedEvent eve(afGlobalVariableChangedEvent::CHOSEN_THREAD_INDEX);
        apEventsHandler::instance().registerPendingDebugEvent(eve);

        // If this function call was not caused by a context change:
        if (!_triggeredByTheContext)
        {
            // Will get the thread's current context id. Only OpenGL contexts can be "current" on a thread.
            apContextID contextId(AP_OPENGL_CONTEXT, 0);

            if (m_chosenThreadIsKernelDebugging)
            {
                // Kernel debugging is OpenCL:
                contextId._contextType = AP_OPENCL_CONTEXT;

                /*
                // The first wavefront in every group is the "inactive" wavefront, get the next wavefront's kernel data:
                int getWavefront = (0 < threadIndex) ? threadIndex : 1;

                // TO_DO
                bool rcWF = gaGetWavefrontInformation((getWavefront - 1), wfInfo);
                GT_IF_WITH_ASSERT(rcWF)
                {
                    contextId._contextId = wfInfo.wavefrontContextIndex();
                }
                */
            }
            else // m_chosenThreadIsKernelDebugging
            {
                // Translate the thread index to OS id:
                osThreadId threadId = OS_NO_THREAD_ID;
                bool rc = gaGetThreadId(_chosenThreadIndex, threadId);

                if (rc)
                {
                    rc = gaGetThreadCurrentRenderContext(threadId, contextId._contextId);

                    if (!rc)
                    {
                        contextId._contextId = 0;
                    }
                    else
                    {
                        contextId._contextType = AP_OPENGL_CONTEXT;
                    }
                }
            }

            // Update the chosen context:
            _triggeredByTheThread = true;
            setChosenContext(contextId);
        }
    }

    // Clear the "called by a context change" flag:
    _triggeredByTheContext = false;
    _triggeredByTheThread = false;
}

// ---------------------------------------------------------------------------
// Name:        gdGDebuggerGlobalVariablesManager::onEvent
// Description: Is called when a debugged process event occur.
// Arguments:   event - Represents the debugged process event.
// Author:      Yaki Tebeka
// Date:        25/5/2004
// ---------------------------------------------------------------------------
void gdGDebuggerGlobalVariablesManager::onEvent(const apEvent& eve, bool& vetoEvent)
{
    (void)(vetoEvent);  // unused
    apEvent::EventType eventType = eve.eventType();

    switch (eventType)
    {
        case apEvent::AP_DEBUGGED_PROCESS_CREATED:
        case apEvent::AP_DEBUGGED_PROCESS_RUN_RESUMED:
        case apEvent::AP_DEBUGGED_PROCESS_TERMINATED:
        {
            // Clear the thread selection (don't use setChosenThread to avoid throwing an event):
            onProcessCreatedEvent();
        }
        break;

        case apEvent::AP_BREAKPOINT_HIT:
        {
            onBreakpointHitEvent((apBreakpointHitEvent&)eve);
        }
        break;

        case apEvent::AP_KERNEL_CURRENT_WORK_ITEM_CHANGED_EVENT:
        {
            // If the work item value was changed to -1, it's being cleared, so we do not need to choose a wavefront:
            const apKernelWorkItemChangedEvent& kernelWIEve = (const apKernelWorkItemChangedEvent&)eve;

            if (-1 < kernelWIEve.workItemValue())
            {
                // When the work item changes, adjust the thread (wavefront) selection to match it.
                // Since the persistent data manager has sent this event, it should already have the updated value:
                int wiCoords[3] = { -1, -1, -1};
                bool rcCoord = gaGetKernelDebuggingCurrentWorkItem(wiCoords[0], wiCoords[1], wiCoords[2]);

                if (rcCoord)
                {
                    // Get the wavefront the current work item belongs to:
                    int wfIndex = -1;
                    bool rcWF = gaGetKernelDebuggingWavefrontIndex(wiCoords, wfIndex);

                    if (rcWF && (-1 < wfIndex))
                    {
                        // Select it (without sending an event, to prevent a feedback loop):
                        _chosenThreadIndex = wfIndex + 1;
                        m_chosenThreadIsKernelDebugging = true;
                    }
                }
            }
        }
        break;

        case apEvent::AP_EXCEPTION:
        {
            // Set a new choosen thread ID.
            const apExceptionEvent& exceptionEvent = (const apExceptionEvent&)eve;

            int nThreadIndex = getThreadIndex(exceptionEvent.triggeringThreadId());

            setChosenThread(nThreadIndex, false);
        }
        break;

        default:
        {
            // Events that are currently not handled.
        }
        break;
    }
}

// ---------------------------------------------------------------------------
// Name:        gdGDebuggerGlobalVariablesManager::startRecording
// Description: The start recording button was pressed - hold the recording status
// Author:      Avi Shapira
// Date:        19/8/2004
// ---------------------------------------------------------------------------
void gdGDebuggerGlobalVariablesManager::startRecording()
{
    _recording = true;
}

// ---------------------------------------------------------------------------
// Name:        gdGDebuggerGlobalVariablesManager::stopRecording
// Description: The stop recording button was pressed - hold the recording status
// Author:      Avi Shapira
// Date:        19/8/2004
// ---------------------------------------------------------------------------
void gdGDebuggerGlobalVariablesManager::stopRecording()
{
    _recording = false;
}


// ---------------------------------------------------------------------------
// Name:        gdGDebuggerGlobalVariablesManager::setCurrentDebugProjectSettings
// Description: Sets the debugged process creation data
// Author:      Uri Shomroni
// Date:        10/5/2009
// ---------------------------------------------------------------------------
bool gdGDebuggerGlobalVariablesManager::setCurrentDebugProjectSettings(const apDebugProjectSettings& projectSettings)
{
    bool retVal = false;

    // Set the rest of the parameters:
    m_currentProjectSettings = projectSettings;

    // Always override the spies directory:
    m_currentProjectSettings.setSpiesDirectory(_openGLProjectSpiesDirectory);

    retVal = true;

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gdGDebuggerGlobalVariablesManager::setDefaultsHardCodedOptionsData
// Description: Set the hard coded options data into the framework
// Author:      Avi Shapira
// Date:        19/1/2005
// ---------------------------------------------------------------------------
void gdGDebuggerGlobalVariablesManager::setDefaultsHardCodedOptionsData()
{
    // Set the value of enableTexturesLogging
    gaEnableImagesDataLogging(true);

    // Set the Selected texture file format into the application
    setImagesFileFormat(AP_PNG_FILE);

}

// ---------------------------------------------------------------------------
// Name:        gdGDebuggerGlobalVariablesManager::setImagesFileFormat
// Description: Set the textures file format
// Author:      Avi Shapira
// Date:        19/1/2005
// ---------------------------------------------------------------------------
void gdGDebuggerGlobalVariablesManager::setImagesFileFormat(apFileType imagesFileFormat)
{
    // Set the value of LoggedImagesFileType
    m_currentProjectSettings.setLoggedImagesFileType(imagesFileFormat);
}

// ---------------------------------------------------------------------------
// Name:        gdGDebuggerGlobalVariablesManager::imagesFileFormat
// Description: Get the textures file format
// Author:      Avi Shapira
// Date:        25/1/2005
// ---------------------------------------------------------------------------
apFileType gdGDebuggerGlobalVariablesManager::imagesFileFormat() const
{
    apFileType loggedImagesFileFormat;

    // Get the process creation data:
    const apDebugProjectSettings& processCreationData = currentDebugProjectSettings();

    // Get the value of loggedImagesFileFormat:
    loggedImagesFileFormat = processCreationData.loggedImagesFileType();

    return loggedImagesFileFormat;
}

// ---------------------------------------------------------------------------
// Name:        gdGDebuggerGlobalVariablesManager::setLoggingLimits
// Description: Set the maximal amounts of items logged for each of:
//              * OpenGL calls per context
//              * OpenCL calls
//              * OpenCL queue commands per command queue
// Author:      Uri Shomroni
// Date:        21/2/2010
// ---------------------------------------------------------------------------
void gdGDebuggerGlobalVariablesManager::setLoggingLimits(unsigned int maxOpenGLCallsPerContext, unsigned int maxOpenCLCallsPerContext, unsigned int maxOpenCLCommandPerQueue)
{
    // Set the values:
    m_currentProjectSettings.setMaxLoggedOpenGLCallsPerContext(maxOpenGLCallsPerContext);
    m_currentProjectSettings.setMaxLoggedOpenCLCallsPerContext(maxOpenCLCallsPerContext);
    m_currentProjectSettings.setMaxLoggedOpenCLCommandsPerQueue(maxOpenCLCommandPerQueue);
}

// ---------------------------------------------------------------------------
// Name:        gdGDebuggerGlobalVariablesManager::getLoggingLimits
// Description: Get the maximal amounts of items logged for each of:
//              * OpenGL calls per context
//              * OpenCL calls
//              * OpenCL queue commands per command queue
// Author:      Uri Shomroni
// Date:        21/2/2010
// ---------------------------------------------------------------------------
void gdGDebuggerGlobalVariablesManager::getLoggingLimits(unsigned int& maxOpenGLCallsPerContext, unsigned int& maxOpenCLCallsPerContext, unsigned int& maxOpenCLCommandPerQueue) const
{
    // Get the process creation data:
    const apDebugProjectSettings& processCreationData = currentDebugProjectSettings();

    // Set the values:
    maxOpenGLCallsPerContext = processCreationData.maxLoggedOpenGLCallsPerContext();
    maxOpenCLCallsPerContext = processCreationData.maxLoggedOpenCLCallsPerContext();
    maxOpenCLCommandPerQueue = processCreationData.maxLoggedOpenCLCommandsPerQueue();
}

// ---------------------------------------------------------------------------
// Name:        gdGDebuggerGlobalVariablesManager::getThreadIndex
// Description: Inputs a thread OS id and returns its API index
//              (or OS_NO_THREAD_ID in case of an error)
//
// Author:      Yaki Tebeka
// Date:        16/5/2005
// ---------------------------------------------------------------------------
int gdGDebuggerGlobalVariablesManager::getThreadIndex(const osThreadId& threadId) const
{
    int retVal = OS_NO_THREAD_ID;

    // Get the amount of debugged process threads:
    int threadsAmount = 0;
    bool rc = gaGetAmountOfDebuggedProcessThreads(threadsAmount);

    if (rc)
    {
        // Iterate the debugged process threads:
        for (int threadIndex = 0; threadIndex < threadsAmount; threadIndex++)
        {
            // Get the current thread id:
            osThreadId currentThreadId = OS_NO_THREAD_ID;
            rc = gaGetThreadId(threadIndex, currentThreadId);

            if (rc)
            {
                // If the current thread id is the thread that we are looking for:
                if (threadId == currentThreadId)
                {
                    retVal = threadIndex;
                    break;
                }
            }
        }
    }


    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gdGDebuggerGlobalVariablesManager::onProcessCreatedEvent
// Description:
// Return Val: void
// Author:      Yaki Tebeka
// Date:        10/7/2007
// ---------------------------------------------------------------------------
void gdGDebuggerGlobalVariablesManager::onProcessCreatedEvent()
{
    _chosenContextId._contextId = 0;
    _chosenContextId._contextType = AP_NULL_CONTEXT;
    _chosenThreadIndex = -1;
    m_chosenThreadIsKernelDebugging = false;
    _triggeredByTheThread = false;
    _triggeredByTheContext = false;
}

// ---------------------------------------------------------------------------
// Name:        gdGDebuggerGlobalVariablesManager::onProcessCreatedEvent
// Description:
// Return Val: void
// Author:      Yaki Tebeka
// Date:        10/7/2007
// ---------------------------------------------------------------------------
void gdGDebuggerGlobalVariablesManager::onBreakpointHitEvent(apBreakpointHitEvent& eve)
{
    // Get the break reason:
    apBreakReason brkReason = eve.breakReason();

    switch (brkReason)
    {

        case AP_FOREIGN_BREAK_HIT:
        case AP_MONITORED_FUNCTION_BREAKPOINT_HIT:
        case AP_NEXT_MONITORED_FUNCTION_BREAKPOINT_HIT:
        case AP_DRAW_MONITORED_FUNCTION_BREAKPOINT_HIT:
        case AP_FRAME_BREAKPOINT_HIT:
        case AP_BREAK_COMMAND_HIT:
        case AP_OPENGL_ERROR_BREAKPOINT_HIT:
        case AP_OPENCL_ERROR_BREAKPOINT_HIT:
        case AP_DETECTED_ERROR_BREAKPOINT_HIT:
        case AP_SOFTWARE_FALLBACK_BREAKPOINT_HIT:
        case AP_GLDEBUG_OUTPUT_REPORT_BREAKPOINT_HIT:
        case AP_REDUNDANT_STATE_CHANGE_BREAKPOINT_HIT:
        case AP_DEPRECATED_FUNCTION_BREAKPOINT_HIT:
        case AP_MEMORY_LEAK_BREAKPOINT_HIT:
        case AP_HOST_BREAKPOINT_HIT:
        {
            // Host code breakpoint, select the triggering thread:
            osThreadId threadToSelect = eve.triggeringThreadId();
            int threadIndexToSelect = getThreadIndex(threadToSelect);

            // If that failed, choose the main thread:
            _chosenThreadIndex = threadIndexToSelect > 0 ? threadIndexToSelect : 0;
            m_chosenThreadIsKernelDebugging = false;
        }
        break;

        case AP_KERNEL_SOURCE_CODE_BREAKPOINT_HIT:
        case AP_KERNEL_FUNCTION_NAME_BREAKPOINT_HIT:
        case AP_STEP_IN_BREAKPOINT_HIT:
        case AP_STEP_OVER_BREAKPOINT_HIT:
        case AP_STEP_OUT_BREAKPOINT_HIT:
        {
            if (gaIsInKernelDebugging())
            {
                // Kernel source breakpoint:
                _chosenThreadIndex = 1;

                // Get the current stepping work item:
                bool isSteppingWorkItemActive = false;
                int wiCoords[3] = { -1, -1, -1 };
                bool rcCoord = gaGetKernelDebuggingCurrentWorkItem(wiCoords[0], wiCoords[1], wiCoords[2]);

                if (rcCoord)
                {
                    // Check if the stepping work item is active:
                    bool isWIActive = gaIsWorkItemValid(wiCoords);

                    if (isWIActive)
                    {
                        isSteppingWorkItemActive = true;

                        // Get the wavefront the current work item belongs to:
                        int wfIndex = -1;
                        bool rcWF = gaGetKernelDebuggingWavefrontIndex(wiCoords, wfIndex);

                        if (rcWF && (-1 < wfIndex))
                        {
                            // Select it (without sending an event):
                            _chosenThreadIndex = wfIndex + 1;
                        }
                    }
                }

                // If it isn't,
                if (!isSteppingWorkItemActive)
                {
                    // Select the first active wavefront, as it's bound to have an active work item at the breakpoint address:
                    _chosenThreadIndex = 1;
                }

                // It's a wavefront either way:
                m_chosenThreadIsKernelDebugging = true;
            }
        }
        break;

        case AP_BEFORE_KERNEL_DEBUGGING_HIT:
        case AP_AFTER_KERNEL_DEBUGGING_HIT:
        {
            // Helper breakpoints, should never reach user-priority event handlers:
            GT_ASSERT(false);
        }
        break;

        default:
        {
            // Unexpected value!
            GT_ASSERT(false);
        }
        break;
    }
}
