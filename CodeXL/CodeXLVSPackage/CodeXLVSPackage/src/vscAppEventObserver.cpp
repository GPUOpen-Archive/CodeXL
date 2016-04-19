//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file vscAppEventObserver.cpp
///
//==================================================================================

//------------------------------ vscAppEventObserver.cpp ------------------------------

#include "stdafx.h"

// Qt:
#include <QtGui>

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTAPIClasses/Include/Events/apExecutionModeChangedEvent.h>
#include <AMDTAPIClasses/Include/Events/apMDIViewCreateEvent.h>
#include <AMDTAPIClasses/Include/Events/apEventsHandler.h>
#include <AMDTAPIClasses/Include/Events/apMemoryAllocationFailureEvent.h>
#include <AMDTOSWrappers/Include/osDebugLog.h>

// AMDTApplicationFramework:
#include <AMDTApplicationFramework/Include/afApplicationCommands.h>
#include <AMDTApplicationFramework/Include/afExecutionModeManager.h>
#include <AMDTApplicationFramework/Include/afGlobalVariableChangedEvent.h>
#include <AMDTApplicationFramework/Include/afProjectManager.h>
#include <AMDTApplicationFramework/Include/afQtCreatorsManager.h>
#include <AMDTApplicationFramework/Include/views/afApplicationTree.h>
#include <AMDTApplicationFramework/Include/afAppStringConstants.h>

// Local:
#include <src/vscAppEventObserver.h>
#include <src/vspWindowsManager.h>

void appMsgHandler(QtMsgType messageType, const QMessageLogContext&, const QString& pMessage);

// ---------------------------------------------------------------------------
// Name:        vscAppEventObserver::vscAppEventObserver
// Description: Constructor
// Author:      Sigal Algranaty
// Date:        23/8/2011
// ---------------------------------------------------------------------------
vscAppEventObserver::vscAppEventObserver()
{
    // Register as an events observer:
    apEventsHandler::instance().registerEventsObserver(*this, AP_APPLICATION_FRAMEWORK_EVENTS_HANDLING_PRIORITY);

    // Install the message handler:
    //qInstallMessageHandler(appMsgHandler);
    qInstallMessageHandler(appMsgHandler);
}

// ---------------------------------------------------------------------------
// Name:        vscAppEventObserver::~vscAppEventObserver
// Description: Destructor
// Author:      Sigal Algranaty
// Date:        23/8/2011
// ---------------------------------------------------------------------------
vscAppEventObserver::~vscAppEventObserver()
{
    // Register as an events observer:
    apEventsHandler::instance().unregisterEventsObserver(*this);
}

// ---------------------------------------------------------------------------
// Name:        vscAppEventObserver::onEvent
// Description: Is called when a debugged process event occurs.
// Arguments:   const apEvent& eve
//              bool& vetoEvent
// Author:      Sigal Algranaty
// Date:        23/8/2011
// ---------------------------------------------------------------------------
void vscAppEventObserver::onEvent(const apEvent& eve, bool& vetoEvent)
{
    GT_UNREFERENCED_PARAMETER(vetoEvent);

    // Get the event type:
    apEvent::EventType eventType = eve.eventType();

    // Handle the event according to its type:
    switch (eventType)
    {
        case apEvent::AP_EXECUTION_MODE_CHANGED_EVENT:
        {
            apExecutionModeChangedEvent changedEvent = (apExecutionModeChangedEvent&)eve;
            afExecutionModeManager::instance().onChangedModeEvent(changedEvent);
            afApplicationCommands* pApplicationCommands = afApplicationCommands::instance();
            GT_IF_WITH_ASSERT(pApplicationCommands != NULL)
            {
                afApplicationTree* pApplicationTree = pApplicationCommands->applicationTree();

                if (pApplicationTree != NULL)
                {
                    pApplicationTree->onModeChanged();
                }

                pApplicationCommands->updateToolbarCommands();
            }
        }
        break;

        case apEvent::APP_GLOBAL_VARIABLE_CHANGED:
        {
            // If there is a new project, set the root name:
            const afGlobalVariableChangedEvent& variableChangedEvent = (const afGlobalVariableChangedEvent&)eve;

            if (variableChangedEvent.changedVariableId() == afGlobalVariableChangedEvent::CURRENT_PROJECT)
            {
                // When a new project is loaded, set the last active session type and active mode:
                gtString lastActiveMode = afProjectManager::instance().currentProjectSettings().lastActiveMode();
                gtString lastActiveSessionType = afProjectManager::instance().currentProjectSettings().lastActiveSessionType();

                if ((!lastActiveMode.isEmpty()) || (!lastActiveSessionType.isEmpty()))
                {
                    // Set the session type and active mode:
                    apExecutionModeChangedEvent executionModeEvent(lastActiveMode, lastActiveSessionType);
                    apEventsHandler::instance().registerPendingDebugEvent(executionModeEvent);
                }

                // Clear the information view:
                afApplicationCommands::instance()->ClearInformationView();
            }
        }
        break;

        case apEvent::APP_UPDATE_UI_EVENT:
        {
            // Get the package wrapper:
            IVsUIShell* piUIShell = vspWindowsManager::instance().getUIShell();
            GT_IF_WITH_ASSERT(piUIShell != NULL)
            {
                // Update all the commands after the focused windows are set:
                piUIShell->UpdateCommandUI(TRUE);
            }
        }
        break;

        case apEvent::AP_MEMORY_ALLOCATION_FAILURE_EVENT:
        {
            afApplicationCommands::instance()->ShowMessageBox(QMessageBox::Icon::Critical, AF_STR_memAllocFailureHeadline, AF_STR_memAllocFailureClientMsg);
        }
        break;

        default:
            // Do nothing...
            break;
    }
}

// ---------------------------------------------------------------------------
// Name:        appMsgHandler
// Description: Handle Qt debug messages
// Arguments:   QtMsgType messageType
//              const char* pMessage
// Author:      Sigal Algranaty
// Date:        8/1/2012
// ---------------------------------------------------------------------------
void appMsgHandler(QtMsgType messageType, const QMessageLogContext&, const QString& pMessage)
{
    // Build a string with the message formatted:
    gtString message;
    message.fromASCIIString(pMessage.toStdString().c_str());

    switch (messageType)
    {
        case QtDebugMsg:
        {
            message.prepend(L"Qt Debug Message: ");
            break;
        }

        case QtWarningMsg:
        {
            message.prepend(L"Qt Warning Message: ");
            break;
        }

        case QtCriticalMsg:
        {
            message.prepend(L"Qt Critical Message: ");
            break;
        }

        case QtFatalMsg:
        {
            message.prepend(L"Qt Fatal Message: ");
            break;
        }

        default:
            break;
    }

    // send the error message to the log and not screen as in SA since this can cause
    // crash in VS when the assert is while deleting a qobject
    OS_OUTPUT_DEBUG_LOG(message.asCharArray(), OS_DEBUG_LOG_DEBUG);
}
