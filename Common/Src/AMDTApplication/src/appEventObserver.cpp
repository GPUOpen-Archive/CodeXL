//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file appEventObserver.cpp
///
//==================================================================================

// Qt:
#include <QtWidgets>

// qscintilla related must come after qt:
#include <AMDTApplicationFramework/Include/views/afInformationView.h>

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTAPIClasses/Include/Events/apExecutionModeChangedEvent.h>
#include <AMDTAPIClasses/Include/Events/apMemoryAllocationFailureEvent.h>
#include <AMDTAPIClasses/Include/Events/apEventsHandler.h>
#include <AMDTApplicationComponents/Include/acMessageBox.h>
#include <AMDTApplicationComponents/Include/acFunctions.h>
#include <AMDTApplicationComponents/Include/acSendErrorReportDialog.h>

// AMDTApplicationFramework:
#include <AMDTApplicationFramework/Include/afApplicationCommands.h>
#include <AMDTApplicationFramework/Include/afAppStringConstants.h>
#include <AMDTApplicationFramework/Include/afExecutionModeManager.h>
#include <AMDTApplicationFramework/Include/afGlobalVariableChangedEvent.h>
#include <AMDTApplicationFramework/Include/afMainAppWindow.h>
#include <AMDTApplicationFramework/Include/afProjectManager.h>
#include <AMDTApplicationFramework/Include/afViewCreatorAbstract.h>
#include <AMDTApplicationFramework/Include/afQtCreatorsManager.h>
#include <AMDTApplicationFramework/Include/afPluginConnectionManager.h>
#include <AMDTApplicationFramework/Include/afGlobalVariablesManager.h>

// Local:
#include <inc/appEventObserver.h>
#include <src/appQtApplication.h>

void appMsgHandler(QtMsgType messageType, const QMessageLogContext&, const QString&);

// ---------------------------------------------------------------------------
// Name:        appEventObserver::appEventObserver
// Description: Constructor
// Author:      Sigal Algranaty
// Date:        23/8/2011
// ---------------------------------------------------------------------------
appEventObserver::appEventObserver()
{
    // Register as an events observer:
    apEventsHandler::instance().registerEventsObserver(*this, AP_APPLICATION_FRAMEWORK_EVENTS_HANDLING_PRIORITY);

    qInstallMessageHandler(0);
    qInstallMessageHandler(appMsgHandler);
}

// ---------------------------------------------------------------------------
// Name:        appEventObserver::~appEventObserver
// Description: Destructor
// Author:      Sigal Algranaty
// Date:        23/8/2011
// ---------------------------------------------------------------------------
appEventObserver::~appEventObserver()
{
    // Register as an events observer:
    apEventsHandler::instance().unregisterEventsObserver(*this);
}

// ---------------------------------------------------------------------------
// Name:        appEventObserver::onEvent
// Description: Is called when a debugged process event occurs.
// Arguments:   const apEvent& eve
//              bool& vetoEvent
// Author:      Sigal Algranaty
// Date:        23/8/2011
// ---------------------------------------------------------------------------
void appEventObserver::onEvent(const apEvent& eve, bool& vetoEvent)
{
    GT_UNREFERENCED_PARAMETER(vetoEvent);

    // Get the event type:
    apEvent::EventType eventType = eve.eventType();

    // Handle the event according to its type:
    switch (eventType)
    {
        case apEvent::AP_MDI_CREATED_EVENT:
        {
            OnMDIViewEvent((const apMDIViewCreateEvent&)eve);
        }
        break;

        case apEvent::AP_EXECUTION_MODE_CHANGED_EVENT:
        {
            apExecutionModeChangedEvent changedEvent = (apExecutionModeChangedEvent&)eve;
            afExecutionModeManager::instance().onChangedModeEvent(changedEvent);
        }
        break;

        case apEvent::APP_GLOBAL_VARIABLE_CHANGED:
        {
            // If there is a new project, set the root name:
            const afGlobalVariableChangedEvent& variableChangedEvent = (const afGlobalVariableChangedEvent&)eve;
            OnGlobalVariableChanged(variableChangedEvent);
        }
        break;

        case apEvent::AP_MEMORY_ALLOCATION_FAILURE_EVENT:
        {
            // Get the memory allocation failure event
            apMemoryAllocationFailureEvent& memoryAllocationFailedEvent = (apMemoryAllocationFailureEvent&)eve;
            OnMemoryAllocationFailedEvent(memoryAllocationFailedEvent);
        }
        break;

        case apEvent::AP_EXCEPTION:
        {
            apExceptionEvent& exceptionEvent = (apExceptionEvent&)eve;
            OnExceptionEvent(exceptionEvent);
        }
        break;

        default:
            // Do nothing...
            break;
    }
}


// ---------------------------------------------------------------------------
// Name:        appEventObserver::onMDIViewEvent
// Description: Handle MDI view event
// Arguments:   const apEvent& eve
// Author:      Sigal Algranaty
// Date:        23/8/2011
// ---------------------------------------------------------------------------
void appEventObserver::OnMDIViewEvent(const apMDIViewCreateEvent& mdiEvent)
{
    // Get the views creators:
    gtVector<afViewCreatorAbstract*>& viewsCreators = afQtCreatorsManager::instance().viewsCreators();

    // Get number of creators:
    int numberViewsCreators = (int)viewsCreators.size();

    for (int viewCreatorIndex = 0 ; viewCreatorIndex < numberViewsCreators; viewCreatorIndex++)
    {
        afViewCreatorAbstract* pCurrentCreator = viewsCreators[viewCreatorIndex];
        GT_IF_WITH_ASSERT(pCurrentCreator != NULL)
        {
            // If the creator creates dynamic views:
            if (pCurrentCreator->isDynamic())
            {
                // Check if the view is handling the requested event:
                if (pCurrentCreator->CreatedMDIType() == mdiEvent.CreatedMDIType())
                {
                    // Get the main app window:
                    afMainAppWindow* pMainWindow = afMainAppWindow::instance();
                    GT_IF_WITH_ASSERT(pMainWindow != NULL)
                    {
                        gtString cannotOpenFileMessage;

                        if (pCurrentCreator->CanFileBeOpened(mdiEvent.filePath(), cannotOpenFileMessage))
                        {
                            // Check if the creator contain a view that can display the object:
                            bool doesViewExist = pCurrentCreator->displayExistingView(mdiEvent);

                            if (doesViewExist)
                            {
                                // Get the MDI sub window:
                                afQMdiSubWindow* pSubWindow = pMainWindow->findMDISubWindow(mdiEvent.filePath());
                                GT_IF_WITH_ASSERT(pSubWindow != NULL)
                                {
                                    // Activate the window and the sub window
                                    pMainWindow->activateSubWindow(pSubWindow);
                                    pMainWindow->activateWindow();

                                    // Give it the keyboard focus
                                    pSubWindow->setFocus(Qt::NoFocusReason);
                                }
                            }
                            else
                            {
                                // Get a copy of the event:
                                apEvent* pEvent = mdiEvent.clone();

                                // Set the event for the creator:
                                pCurrentCreator->setCreationEvent(pEvent);

                                // Create the view:
                                pMainWindow->createSingleView(pCurrentCreator, mdiEvent.viewIndex());

                                // Set the event for the creator:
                                pCurrentCreator->setCreationEvent(NULL);

                                // Release the event memory:
                                delete pEvent;
                            }

                            // Re paint the main window:
                            pMainWindow->update();

                            // Update the toolbar commands:
                            pMainWindow->updateToolbars();
                        }
                        else
                        {
                            // Output a message that the view cannot be opened
                            acMessageBox::instance().information(afGlobalVariablesManager::instance().ProductNameA(), acGTStringToQString(cannotOpenFileMessage));
                        }
                    }
                }
            }
        }
    }
}

// ---------------------------------------------------------------------------
void appEventObserver::OnGlobalVariableChanged(const afGlobalVariableChangedEvent& variableChangedEvent)
{
    if (variableChangedEvent.changedVariableId() == afGlobalVariableChangedEvent::CURRENT_PROJECT)
    {
        // When a new project is loaded, set the last active session type and active mode:
        gtString lastActiveMode = afProjectManager::instance().currentProjectSettings().lastActiveMode();
        gtString lastActiveSessionType = afProjectManager::instance().currentProjectSettings().lastActiveSessionType();

        // Set the session type and active mode:
        apExecutionModeChangedEvent executionModeEvent(lastActiveMode, lastActiveSessionType);
        apEventsHandler::instance().registerPendingDebugEvent(executionModeEvent);

        // Get the main application instance:
        afMainAppWindow* pMainAppWindow = afMainAppWindow::instance();
        GT_IF_WITH_ASSERT(pMainAppWindow != NULL)
        {
            // Close all opened MDI windows:
            pMainAppWindow->closeAllSubWindows();
        }

        // Check which layout should be loaded:
        afMainAppWindow::LayoutFormats currentLayout = afMainAppWindow::LayoutNoProject;

        // Check if there is a loaded project:
        bool isProjectLoaded = !afProjectManager::instance().currentProjectFilePath().asString().isEmpty();

        if (isProjectLoaded)
        {
            // Get the layout from the current mode:
            afIExecutionMode* pCurrentMode = afExecutionModeManager::instance().activeMode();

            if (NULL != pCurrentMode)
            {
                currentLayout = pCurrentMode->layoutFormat();
            }
        }

        // Set the current layout:
        afMainAppWindow::instance()->updateLayoutMode(currentLayout);

        // Calculate the title bar's string:
        gtString titleBarString;
        afCalculateCodeXLTitleBarString(titleBarString);

        // Set the caption:
        afApplicationCommands::instance()->setApplicationCaption(titleBarString);


        // Update the hosts list button:
        afExecutionModeManager::instance().UpdateHostsList();
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
void appMsgHandler(QtMsgType messageType, const QMessageLogContext& context, const QString& qtMessage)
{
    (void)(context); // not used
    // Build a string with the message formatted:
    gtString message = acQStringToGTString(qtMessage);

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

    GT_ASSERT_EX(false, message.asCharArray());
}

// ---------------------------------------------------------------------------
void appEventObserver::OnMemoryAllocationFailedEvent(apMemoryAllocationFailureEvent& memoryAllocationFailedEvent)
{
    GT_UNREFERENCED_PARAMETER(memoryAllocationFailedEvent);

    appQtApplication::ClientMemAllocFailureHandler();
}

// ---------------------------------------------------------------------------
void appEventObserver::OnExceptionEvent(const apExceptionEvent& exceptionEvent)
{

    // If this is a second chance event, or a fatal Linux signal, the debugged process is going to die:
    if (exceptionEvent.isSecondChance() || exceptionEvent.isFatalLinuxSignal())
    {
        // Get the crash details from the plugin that reported it:
        osCallStack crashStack;
        bool openCLEnglineLoaded = false;
        bool openGLEnglineLoaded = false;
        bool kernelDebuggingEnteredAtLeastOnce = false;
        afPluginConnectionManager::instance().getExceptionEventDetails(exceptionEvent, crashStack, openCLEnglineLoaded, openGLEnglineLoaded, kernelDebuggingEnteredAtLeastOnce);

        // Update the addition information:
        //updateAdditionInformationString(openCLEnglineLoaded, openGLEnglineLoaded, kernelDebuggingEnteredAtLeastOnce);
        // Build the string:
        QString additionalInformation;
        additionalInformation.append(AF_STR_SendErrorReportAdditionalStringHeader);
        additionalInformation.append(AF_STR_NewLineA);

        // Add opencl engine info:
        additionalInformation.append(AF_STR_SendErrorReportAdditionalStringOpenCL);
        additionalInformation.append(openCLEnglineLoaded ? "Yes" : "No");
        additionalInformation.append(AF_STR_NewLineA);

        // Add opengl engine info:
        additionalInformation.append(AF_STR_SendErrorReportAdditionalStringOpenGL);
        additionalInformation.append(openGLEnglineLoaded ? "Yes" : "No");
        additionalInformation.append(AF_STR_NewLineA);

        // Add Kernel debugging info:
        additionalInformation.append(AF_STR_SendErrorReportAdditionalStringKernel);
        additionalInformation.append(kernelDebuggingEnteredAtLeastOnce ? "Yes" : "No");
        additionalInformation.append(AF_STR_NewLineA);

        // Display the error report dialog:
        QPixmap iconPixMap;
        acSetIconInPixmap(iconPixMap, afGlobalVariablesManager::ProductIconID(), AC_64x64_ICON);
        acSendErrorReportDialog* pSendErrorReportDialog = new acSendErrorReportDialog(NULL, afGlobalVariablesManager::ProductNameA(), iconPixMap);
        GT_IF_WITH_ASSERT(pSendErrorReportDialog != NULL)
        {
            // Get the exception reason:
            osExceptionReason exceptionReason = exceptionEvent.exceptionReason();

            pSendErrorReportDialog->displayErrorReportDialog(exceptionReason, crashStack, additionalInformation);
            delete pSendErrorReportDialog;
        }
    }
}