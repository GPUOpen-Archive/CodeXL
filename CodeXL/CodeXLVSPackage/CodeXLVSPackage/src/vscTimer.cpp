//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file vscTimer.cpp
///
//==================================================================================

#include "stdafx.h"

// Infra:
#include <AMDTOSWrappers/Include/osStopWatch.h>
#include <AMDTOSWrappers/Include/osDebugLog.h>
#include <AMDTApplicationFramework/Include/afApplicationCommands.h>
#include <AMDTKernelAnalyzer/src/kaApplicationCommands.h>

// Local:
#include <Include/Public/vscTimer.h>
#include <Include/vscTimerQt.h>
#include <Include/Public/vscVspDTEInvoker.h>
#include <src/vspQApplicationWrapper.h>

wchar_t* EMPTY_STR_IF_NULL(const wchar_t* pStr) { return ((pStr != NULL) ? pStr : L""); }

vscTimer::vscTimer()
    : m_pOwner(NULL)
    , m_pQtTimer(nullptr)
    , m_isUpdatedMessageBoxHWND(false)
    , m_lastActiveWindowHwnd(nullptr)
{
    bool rc;

    // Initialize a timer that calls the requested slot on its update function:
    m_pQtTimer = new QTimer(this);

    // Connect the timer to the slot:
    rc = connect(m_pQtTimer, SIGNAL(timeout()), this, SLOT(vscOnClockTick()));
    assert(rc);

    m_pQtTimer->start(VSP_TIMER_TICK_LENGTH_MS);
}

vscTimer::~vscTimer()
{
    if (m_pQtTimer)
    {
        delete m_pQtTimer;
        m_pQtTimer = nullptr;
    }
}

void vscTimer::vscOnClockTick()
{
    bool projectTypeSupported = false;
    bool isProjectOpened = false;
    bool isNonNativeProject = false;

    wchar_t* pExecutableFilePathBuffer = NULL;
    wchar_t* pWorkingDirectoryPathBuffer = NULL;
    wchar_t* pCommandLineArgumentsBuffer = NULL;
    wchar_t* pEnvironmentBuffer = NULL;

    // Measure the time it takes to execute this function to verify CodeXL VS Extension does not slow down Visual Studio
    osStopWatch stopWatch;
    stopWatch.start();

    GT_IF_WITH_ASSERT(m_pOwner != NULL)
    {
        bool shouldUpdateUI = false;

        // Get startup info.
        bool projectInfoChanged = m_pOwner->vscTimerOwner_GetStartupProjectDebugInfo(pExecutableFilePathBuffer, pWorkingDirectoryPathBuffer, pCommandLineArgumentsBuffer,
                                  pEnvironmentBuffer, isProjectOpened, projectTypeSupported, isNonNativeProject);

        if (projectInfoChanged)
        {
            // Update the data members.
            m_newExecPath = EMPTY_STR_IF_NULL(pExecutableFilePathBuffer);
            m_newWorkDir = EMPTY_STR_IF_NULL(pWorkingDirectoryPathBuffer);
            m_newCmdArgs = EMPTY_STR_IF_NULL(pCommandLineArgumentsBuffer);
            m_newExecEnv = EMPTY_STR_IF_NULL(pEnvironmentBuffer);

            // Release the allocated strings.
            m_pOwner->vscTimerOwner_DeleteWCharStr(pExecutableFilePathBuffer);
            m_pOwner->vscTimerOwner_DeleteWCharStr(pWorkingDirectoryPathBuffer);
            m_pOwner->vscTimerOwner_DeleteWCharStr(pCommandLineArgumentsBuffer);
            m_pOwner->vscTimerOwner_DeleteWCharStr(pEnvironmentBuffer);

            // If one of the relevant project settings had changed, update our project settings, and set the current project:
            bool shouldUpdateSettings = ((m_newExecPath != m_previousExecPath) || (m_newWorkDir != m_previousWorkDir) || (m_newCmdArgs != m_previousCmdArgs) || (m_newExecEnv != m_previousExecEnv));

            if (shouldUpdateSettings)
            {
                // Save the current active project:
                afApplicationCommands* pApplicationCommands = afApplicationCommands::instance();
                GT_IF_WITH_ASSERT(pApplicationCommands != NULL)
                {
                    pApplicationCommands->OnFileSaveProject();
                }

                // Save project's settings.
                m_pOwner->vscTimerOwner_UpdateProjectSettingsFromStartupInfo();

                GT_IF_WITH_ASSERT(pApplicationCommands != NULL)
                {
                    afApplicationTree* pApplicationTree = pApplicationCommands->applicationTree();
                    GT_IF_WITH_ASSERT(pApplicationTree != NULL)
                    {
                        pApplicationTree->updateTreeRootText();
                    }
                }

                m_previousCmdArgs = m_newCmdArgs;
                m_previousWorkDir = m_newWorkDir;
                m_previousExecPath = m_newExecPath;
                m_previousExecEnv = m_newExecEnv;

                // Get the UI shell:
                // Update the project settings from the vcxproj file:
                shouldUpdateUI = true;
            }
        }

        // Need to update UI if the document was changed:
        wchar_t* filePathAsStr = nullptr;
        bool hasActiveDocument = false;
        m_pOwner->vscTimerOwner_GetActiveDocumentFileFullPath(filePathAsStr, hasActiveDocument);

        if (hasActiveDocument)
        {
            if (filePathAsStr != m_activeDocPath)
            {
                m_activeDocPath = filePathAsStr;

                // Update the tree for the KA toolbar:
                kaApplicationCommands::instance().ActivateMDITreeItem(m_activeDocPath);

                // Update the UI:
                shouldUpdateUI = true;
            }
        }
        else
        {
            m_activeDocPath = L"";
        }

        if (shouldUpdateUI)
        {
            afApplicationCommands::instance()->updateToolbarCommands();
        }

        // Update the document update manager:
        HWND activeWindowHwnd;

        if (m_pOwner->vscTimerOwner_GetActiveWindowHandle(activeWindowHwnd))
        {
            // Check if it is connected to the foreground application (us the use it if not mark it as null so when we become active we will check it again)
            // Use the same child/parent trick of qt:
            HWND foreGroundWindow = GetForegroundWindow();
            // find the highest parent:
            HWND windowParent = foreGroundWindow;

            while (::GetParent(windowParent) != NULL)
            {
                windowParent = ::GetParent(windowParent);
            }

            // find the children of the active window
            HWND childWindow = ::GetWindow(activeWindowHwnd, GW_CHILD);

            while (childWindow != windowParent && NULL != childWindow)
            {
                childWindow = ::GetWindow(childWindow, GW_HWNDNEXT);
            }

            // If the active window is the forground window or it is in the chain of children then consider
            // it as the active window:
            gtString activeFilePath = EMPTY_STR_IF_NULL(filePathAsStr);

            // get the current qt application
            QApplication* pCurrentApp = qApp;
            vspQApplicationWrapper* pWrapperApp = qobject_cast<vspQApplicationWrapper*>(pCurrentApp);

            if (childWindow == windowParent || activeWindowHwnd == foreGroundWindow)
            {
                if (activeWindowHwnd != m_lastActiveWindowHwnd)
                {
                    m_lastActiveWindowHwnd = activeWindowHwnd;

                    if (pWrapperApp != nullptr)
                    {
                        pWrapperApp->SetMDIActiveWindow(activeWindowHwnd);
                    }

                    // Get the active file from the active document from VS
                    afDocUpdateManager::instance().ActivateView(activeWindowHwnd, activeFilePath);
                }
                else
                {
                    // Need to call ActivateView() even though the active window has not changed because we're checking
                    // if an external application has modified the file outside of the VS editor.
                    // Update all viewers to the current doc without user notification:
                    afDocUpdateManager::instance().ActivateView(activeWindowHwnd, activeFilePath, true);
                }
            }
            // If not, it is probably not in our application or not one of our MDI:
            else
            {
                // Update all viewers to the current doc without user notification:
                afDocUpdateManager::instance().ActivateView(NULL, activeFilePath, true);
                m_lastActiveWindowHwnd = NULL;

                if (pWrapperApp != nullptr)
                {
                    pWrapperApp->SetMDIActiveWindow(nullptr);
                }
            }
        }

        // Delete the allocated string
        m_pOwner->vscTimerOwner_DeleteWCharStr(filePathAsStr);

        // update the acMessagebox Parent hwnd if needed
        if (!m_isUpdatedMessageBoxHWND)
        {
            HWND hwnd = NULL;
            IVsUIShell* pUIShell = vscWindowsManager_GetIVsUiShell();
            GT_IF_WITH_ASSERT(NULL != pUIShell)
            {
                pUIShell->GetDialogOwnerHwnd(&hwnd);

                if (NULL != hwnd)
                {
                    m_isUpdatedMessageBoxHWND = acMessageBox::instance().setParentHwnd(hwnd);
                }
            }
        }
    }

    stopWatch.stop();
    double executionTimeInSeconds = 0.0;
    stopWatch.getTimeInterval(executionTimeInSeconds);
    // To refrain from slowing down Visual Studio the time it takes to execute this function should not exceed 100ms.
    const double EXECUTION_TIME_THRESHOLD = 0.050;

    if (executionTimeInSeconds > EXECUTION_TIME_THRESHOLD)
    {
        gtString logMsg;
        logMsg.appendFormattedString(L"VS Extension periodic handler function execution time %g (seconds) exceeded threshold.", executionTimeInSeconds);
        OS_OUTPUT_DEBUG_LOG(logMsg.asCharArray(), OS_DEBUG_LOG_ERROR);
    }
}


void* vscTimer_CreateInstance()
{
    vscTimer* pInstance = new vscTimer();
    return pInstance;
}

void vscTimer_DestroyInstance(void* pVscTimer)
{
    vscTimer* pTimerInstance = reinterpret_cast<vscTimer*>(pVscTimer);
    GT_IF_WITH_ASSERT(pTimerInstance != NULL)
    {
        delete pTimerInstance;
    }
}

void vscTimer_SetOwner(void* pVscTimer, IVscTimerOwner* pOwner)
{
    vscTimer* pTimerInstance = reinterpret_cast<vscTimer*>(pVscTimer);
    GT_IF_WITH_ASSERT(pTimerInstance != NULL)
    {
        pTimerInstance->SetOwner(pOwner);
    }
}

void vscTimer_OnClockTick(void* pVscTimer)
{
    vscTimer* pTimerInstance = reinterpret_cast<vscTimer*>(pVscTimer);
    GT_IF_WITH_ASSERT(pTimerInstance != NULL)
    {
        pTimerInstance->vscOnClockTick();
    }
}


