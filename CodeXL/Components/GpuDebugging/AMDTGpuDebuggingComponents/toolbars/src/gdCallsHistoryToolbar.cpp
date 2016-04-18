//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gdCallsHistoryToolbar.cpp
///
//==================================================================================

//------------------------------ gdCallsHistoryToolbar.cpp ------------------------------

// Qt
#include <AMDTApplicationComponents/Include/acQtIncludes.h>

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTOSWrappers/Include/osFileLauncher.h>
#include <AMDTAPIClasses/Include/apAPIConnectionType.h>
#include <AMDTAPIClasses/Include/Events/apOutputDebugStringEvent.h>
#include <AMDTAPIClasses/Include/Events/apEventsHandler.h>
#include <AMDTAPIClasses/Include/Events/apMonitoredObjectsTreeEvent.h>
#include <AMDTApiFunctions/Include/gaGRApiFunctions.h>
#include <AMDTServerUtilities/Include/suStringConstants.h>
#include <AMDTOpenGLServer/Include/gsPublicStringConstants.h>
#include <AMDTApplicationComponents/Include/acMessageBox.h>
#include <AMDTApplicationComponents/Include/acFunctions.h>

// AMDTApplicationFramework:
#include <AMDTApplicationFramework/Include/afAppStringConstants.h>
#include <AMDTApplicationFramework/Include/afGlobalVariablesManager.h>
#include <AMDTApplicationFramework/Include/afGlobalVariableChangedEvent.h>

// Local:
#include <AMDTGpuDebuggingComponents/Include/gdAidFunctions.h>
#include <AMDTGpuDebuggingComponents/Include/gdAPICallsHistoryPanel.h>
#include <AMDTGpuDebuggingComponents/Include/gdApplicationCommands.h>
#include <AMDTGpuDebuggingComponents/Include/gdCommandIDs.h>
#include <AMDTGpuDebuggingComponents/Include/gdStringConstants.h>
#include <AMDTGpuDebuggingComponents/Include/gdGDebuggerGlobalVariablesManager.h>
#include <AMDTGpuDebuggingComponents/Include/gdPropertiesEventObserver.h>
#include <AMDTGpuDebuggingComponents/Include/toolbars/gdCallsHistoryToolbar.h>
#include <AMDTGpuDebuggingComponents/Include/views/gdAPICallsHistoryView.h>
#include <AMDTGpuDebuggingComponents/Include/gdHTMLProperties.h>

// ---------------------------------------------------------------------------
// Name:        gdCallsHistoryToolbar::gdCallsHistoryToolbar
// Description: Constructor
// Arguments:   QWidget* pParent
// Author:      Sigal Algranaty
// Date:        26/12/2011
// ---------------------------------------------------------------------------
gdCallsHistoryToolbar::gdCallsHistoryToolbar(QWidget* pParent)
    : acToolBar(pParent), _pApplicationCommands(NULL), _pRecordAction(NULL), _pOpenRecordAction(NULL), _pNextMarkerAction(NULL), _pPrevMarkerAction(NULL),
      _selectedContextId(AP_OPENGL_CONTEXT, 0), _amountOfRenderContexts(0), _isDebuggedProcessSuspended(false),
      _isDebuggedProcessRunning(false), _isRecordFileExist(false)
{
    // Add the tools to the toolbar:
    addToolbarTools();

    // Get the application commands instance:
    _pApplicationCommands = gdApplicationCommands::gdInstance();
    GT_ASSERT(_pApplicationCommands != NULL);

    // Register myself to listen to debugged process events:
    apEventsHandler::instance().registerEventsObserver(*this, AP_APPLICATION_COMPONENTS_EVENTS_HANDLING_PRIORITY);
}


// ---------------------------------------------------------------------------
// Name:        gdCallsHistoryToolbar::~gdCallsHistoryToolbar
// Description: Destructor.
// Author:      Yaki Tebeka
// Date:        12/12/2004
// ---------------------------------------------------------------------------
gdCallsHistoryToolbar::~gdCallsHistoryToolbar()
{
    // Unregister myself from listening to debugged process events:
    apEventsHandler::instance().unregisterEventsObserver(*this);
}

// ---------------------------------------------------------------------------
// Name:        gdCallsHistoryToolbar::onUpdateToolbar
// Description: Function that is called whenever we want to update the toolbar
//              buttons status
// Author:      Sigal Algranaty
// Date:        26/12/2011
// ---------------------------------------------------------------------------
void gdCallsHistoryToolbar::onUpdateToolbar()
{
    bool shouldEnable = false;
    bool shouldCheck = false;
    onUpdateRecord(shouldEnable, shouldCheck);
    GT_IF_WITH_ASSERT(_pRecordAction != NULL)
    {
        _pRecordAction->setEnabled(shouldEnable);
        _pRecordAction->setChecked(shouldCheck);
    }

    onUpdateOpenRecord(shouldEnable);
    GT_IF_WITH_ASSERT(_pOpenRecordAction != NULL)
    {
        _pOpenRecordAction->setEnabled(shouldEnable);
    }

    onUpdateMarker(shouldEnable);
    GT_IF_WITH_ASSERT((_pPrevMarkerAction != NULL) && (_pNextMarkerAction != NULL))
    {
        _pPrevMarkerAction->setEnabled(shouldEnable);
        _pNextMarkerAction->setEnabled(shouldEnable);
    }
}

// ---------------------------------------------------------------------------
// Name:        gdCallsHistoryToolbar::onUpdateMarker
// Description: Is called when wxWindows wants to update the status of the
//              "Previous Marker" command button.
// Author:      Avi Shapira
// Date:        1/2/2005
// ---------------------------------------------------------------------------
void gdCallsHistoryToolbar::onUpdateMarker(bool& shouldEnable)
{
    bool canFindMarker = false;

    // Get the application commands instance:
    GT_IF_WITH_ASSERT(_pApplicationCommands != NULL)
    {
        // Get the Calls History view:
        gdAPICallsHistoryPanel* pCallsHistoryPanel = _pApplicationCommands->callsHistoryPanel();
        GT_IF_WITH_ASSERT(pCallsHistoryPanel != NULL)
        {
            gdAPICallsHistoryView* pCallsHistoryView = pCallsHistoryPanel->historyView();
            GT_IF_WITH_ASSERT(pCallsHistoryView != NULL)
            {
                canFindMarker = pCallsHistoryView->canFindMarker();
            }
        }
    }



    shouldEnable = canFindMarker;
}

// ---------------------------------------------------------------------------
// Name:        gdCallsHistoryToolbar::onNextMarkerClick
// Description: Is calls when the user press the "Next Marker" button.
// Author:      Avi Shapira
// Date:        1/2/2005
// ---------------------------------------------------------------------------
void gdCallsHistoryToolbar::onNextMarkerClick()
{
    apSearchDirection searchDirection = AP_SEARCH_INDICES_UP;

    // Get the application commands instance:
    GT_IF_WITH_ASSERT(_pApplicationCommands != NULL)
    {
        // Get the Calls History view:
        gdAPICallsHistoryPanel* pCallsHistoryPanel = _pApplicationCommands->callsHistoryPanel();
        GT_IF_WITH_ASSERT(pCallsHistoryPanel != NULL)
        {
            gdAPICallsHistoryView* pCallsHistoryView = pCallsHistoryPanel->historyView();
            GT_IF_WITH_ASSERT(pCallsHistoryView != NULL)
            {
                pCallsHistoryView->findNextMarker(searchDirection);
            }
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        gdCallsHistoryToolbar::onPreviousMarkerClick
// Description: Is calles when the user press the "Previous Marker" button.
// Author:      Avi Shapira
// Date:        1/2/2005
// ---------------------------------------------------------------------------
void gdCallsHistoryToolbar::onPreviousMarkerClick()
{
    apSearchDirection searchDirection = AP_SEARCH_INDICES_DOWN;

    // Get the application commands instance:
    GT_IF_WITH_ASSERT(_pApplicationCommands != NULL)
    {
        // Get the Calls History view:
        gdAPICallsHistoryPanel* pCallsHistoryPanel = _pApplicationCommands->callsHistoryPanel();
        GT_IF_WITH_ASSERT(pCallsHistoryPanel != NULL)
        {
            gdAPICallsHistoryView* pCallsHistoryView = pCallsHistoryPanel->historyView();
            GT_IF_WITH_ASSERT(pCallsHistoryView != NULL)
            {
                pCallsHistoryView->findNextMarker(searchDirection);
            }
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        gdCallsHistoryToolbar::onUpdateRecord
// Description: Check if the record action should be enabled / checked
// Arguments:   bool& shouldEnable
//              bool& shouldCheck
// Author:      Sigal Algranaty
// Date:        26/12/2011
// ---------------------------------------------------------------------------
void gdCallsHistoryToolbar::onUpdateRecord(bool& shouldEnable, bool& shouldCheck)
{
    // Get the recorder and profiling status:
    apExecutionMode appExecutionMode = AP_DEBUGGING_MODE;
    gaGetDebuggedProcessExecutionMode(appExecutionMode);

    shouldEnable = true;
    shouldCheck = false;

    if (appExecutionMode == AP_PROFILING_MODE)
    {
        shouldEnable = false;
        shouldCheck = false;
    }
    else
    {
        gdGDebuggerGlobalVariablesManager& globalVarsManager = gdGDebuggerGlobalVariablesManager::instance();
        bool recording = globalVarsManager.recording();

        if (recording)
        {
            shouldEnable = true;
            shouldCheck = true;
        }
        else
        {
            const gtString& logFileDir = afGlobalVariablesManager::instance().logFilesDirectoryPath().fileDirectoryAsString();

            // check if the log directory is defined
            if (!logFileDir.isEmpty())
            {
                shouldEnable = true;
                shouldCheck = false;
            }
            else
            {
                shouldEnable = false;
                shouldCheck = false;
            }
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        gdCallsHistoryToolbar::onRecordingClick
// Description: Is calles when the user press the "Record" button.
// Author:      Yaki Tebeka
// Date:        27/12/2003
// ---------------------------------------------------------------------------
void gdCallsHistoryToolbar::onRecordingClick()
{
    // Get the recorder status:
    gdGDebuggerGlobalVariablesManager& globalVarsManager = gdGDebuggerGlobalVariablesManager::instance();
    bool recording = globalVarsManager.recording();

    if (recording)
    {
        bool rc = gaStopMonitoredFunctionsCallsLogFileRecording();
        GT_ASSERT(rc);
        globalVarsManager.stopRecording();

        // Clear the properties view
        gdPropertiesEventObserver::instance().setPropertiesFromText(acGTStringToQString(gdHTMLProperties::emptyHTML()));
    }
    else
    {
        // Start recording
        bool rc = gaStartMonitoredFunctionsCallsLogFileRecording();
        GT_ASSERT(rc);

        // change the global var to recording mode
        globalVarsManager.startRecording();

        // Do not delete the textures in this session
        gaDeleteLogFilesWhenDebuggedProcessTerminates(false);

        // Get the log file directory

        // Get the current process creation data from the afGlobalVariablesManager:
        afGlobalVariablesManager& theStateManager = afGlobalVariablesManager::instance();

        gdHTMLProperties htmlBuilder;
        gtString propertiesViewMessage;
        afHTMLContent htmlContent;
        htmlBuilder.buildStartRecordingWarningPropertiesString(theStateManager.logFilesDirectoryPath().asString(), htmlContent);
        htmlContent.toString(propertiesViewMessage);
        gdPropertiesEventObserver::instance().setPropertiesFromText(acGTStringToQString(propertiesViewMessage));
    }
}

// ---------------------------------------------------------------------------
// Name:        gdCallsHistoryToolbar::onOpenRecordedFileClick
// Description: Is called when the user press the "Open Recorded File" button.
// Arguments:   wxCommandEvent& event
// Author:      Avi Shapira
// Date:        23/3/2005
// ---------------------------------------------------------------------------
void gdCallsHistoryToolbar::onOpenRecordedFileClick()
{
    bool launchFileOk = false;

    if (_isRecordFileExist)
    {
        // Open The log file:
        gtString recordedFileURL = L"file:///";
        recordedFileURL += _recordFilePath.asString();
        osFileLauncher fileLauncher(recordedFileURL);
        launchFileOk = fileLauncher.launchFile();
        GT_ASSERT(launchFileOk);
    }

    if (launchFileOk)
    {
        // Get the properties view:
        // Remove the recoding warning:

        gdHTMLProperties htmlBuilder;
        gtString propertiesViewMessage;
        afHTMLContent htmlContent;
        htmlBuilder.buildOpenRecordingFilePropertiesString(_recordFilePath.asString(), htmlContent);
        htmlContent.toString(propertiesViewMessage);
        gdPropertiesEventObserver::instance().setPropertiesFromText(acGTStringToQString(propertiesViewMessage));
    }
    else
    {
        // Error message:
        acMessageBox::instance().information(AF_STR_InformationA, GD_STR_CallsHistoryToolbarOpenLogFileInfo, QMessageBox::Ok);
    }
}

// ---------------------------------------------------------------------------
// Name:        gdCallsHistoryToolbar::onUpdateOpenRecord
// Description: Is called when wxWindows wants to update the status of the
//              "Open Recorded File" command button.
// Author:      Avi Shapira
// Date:        23/3/2005
// ---------------------------------------------------------------------------
void gdCallsHistoryToolbar::onUpdateOpenRecord(bool& isEnabled)
{
    isEnabled  = (_isRecordFileExist && (_isDebuggedProcessSuspended || (!_isDebuggedProcessRunning)));
}

// ---------------------------------------------------------------------------
// Name:        gdCallsHistoryToolbar::onGlobalVariableChanged
// Description: Triggered when a global variable value is changed - will update the combo with the context
// Arguments:   const gdCodeXLGlobalVariableChangeEvent& stateChangeEvent
// Author:      Avi Shapira
// Date:        11/5/2004
// ---------------------------------------------------------------------------
void gdCallsHistoryToolbar::onGlobalVariableChanged(const afGlobalVariableChangedEvent& variableChangedEvent)
{
    // Get id of the global variable that was changed:
    afGlobalVariableChangedEvent::GlobalVariableId variableId = variableChangedEvent.changedVariableId();

    // If the chosen context was changed:
    if (variableId == afGlobalVariableChangedEvent::CHOSEN_CONTEXT_ID)
    {
        // Get the new chosen context id:
        gdGDebuggerGlobalVariablesManager& globalVarsManager = gdGDebuggerGlobalVariablesManager::instance();
        apContextID newContextId = globalVarsManager.chosenContext();

        // Set the new context Id into the member
        _selectedContextId = newContextId;

        // If we are suspended, get the right file path:
        if (_isDebuggedProcessSuspended)
        {
            osFilePath recordFilePath;
            bool rc = gaGetContextLogFilePath(_selectedContextId, _isRecordFileExist, recordFilePath);
            GT_ASSERT(rc);
        }
    }
}


// ---------------------------------------------------------------------------
// Name:        gdAPICallsHistoryView::onTreeItemSelection
// Description:
// Arguments:   const apMonitoredObjectsTreeSelectedEvent& eve
// Return Val:  void
// Author:      Sigal Algranaty
// Date:        27/12/2011
// ---------------------------------------------------------------------------
void gdCallsHistoryToolbar::onTreeItemSelection(const apMonitoredObjectsTreeSelectedEvent& eve)
{
    // Get the item data from the event:
    const afApplicationTreeItemData* pTreeItemData = (afApplicationTreeItemData*)eve.selectedItemData();

    if (pTreeItemData != NULL)
    {
        // If the debugged process is suspended:
        if (_isDebuggedProcessSuspended)
        {
            bool newContext = false;

            if (pTreeItemData->m_itemType == AF_TREE_ITEM_APP_ROOT)
            {
                if (!_selectedContextId.isDefault())
                {
                    // Set the new context:
                    _selectedContextId._contextId = 0;
                    _selectedContextId._contextType = AP_NULL_CONTEXT;
                    newContext = true;
                }
            }
            else
            {
                gdDebugApplicationTreeData* pGDData = qobject_cast<gdDebugApplicationTreeData*>(pTreeItemData->extendedItemData());

                if (pGDData != NULL)
                {
                    if (_selectedContextId != pGDData->_contextId)
                    {
                        // Set the new context Id into the member
                        _selectedContextId = pGDData->_contextId;
                        newContext = true;
                    }
                }
            }

            if (newContext)
            {
                bool rc = gaGetContextLogFilePath(_selectedContextId, _isRecordFileExist, _recordFilePath);
                GT_ASSERT(rc);
            }
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        gdCallsHistoryToolbar::onEvent
// Description: Is called when a debugged process event occurs.
// Arguments:   eve - A class representing the event.
// Author:      Yaki Tebeka
// Date:        13/6/2004
// ---------------------------------------------------------------------------
void gdCallsHistoryToolbar::onEvent(const apEvent& eve, bool& vetoEvent)
{
    (void)(vetoEvent);  // unused
    // Get the event type:
    apEvent::EventType eventType = eve.eventType();

    switch (eventType)
    {
        case apEvent::AP_DEBUGGED_PROCESS_CREATED:
        {
            gdGDebuggerGlobalVariablesManager& globalVarsManager = gdGDebuggerGlobalVariablesManager::instance();
            bool recording = globalVarsManager.recording();

            // Dont delete the textures in this session if recording is on
            gaDeleteLogFilesWhenDebuggedProcessTerminates(!recording);

            // Set the recorded status into the file exists member
            //          _isRecordFileExist = globalVarsManager.recording();

            _isDebuggedProcessSuspended = false;
            _isDebuggedProcessRunning = true;
        }
        break;

        case apEvent::AP_DEBUGGED_PROCESS_TERMINATED:
        case apEvent::AP_DEBUGGED_PROCESS_CREATION_FAILURE:
        {
            _isDebuggedProcessRunning = false;
            _isDebuggedProcessSuspended = false;
        }
        break;

        case apEvent::AP_DEBUGGED_PROCESS_RUN_RESUMED:
        {
            _isDebuggedProcessSuspended = false;

            // Check if "record" button was pressed before we resumed the process
            gdGDebuggerGlobalVariablesManager& globalVarsManager = gdGDebuggerGlobalVariablesManager::instance();
            bool recording = globalVarsManager.recording();

            // State that recording was enabled / disabled before launching the debug process
            bool rc1 = gaResetRecordingWasDoneFlag(recording);
            GT_ASSERT(rc1);
        }
        break;

        case apEvent::AP_DEBUGGED_PROCESS_RUN_SUSPENDED:
        {
            _isDebuggedProcessSuspended = true;

            if (gaIsAPIConnectionActive(AP_SPIES_UTILITIES_API_CONNECTION))
            {
                // Get the (possibly new) file path:
                bool rcFilePath = gaGetContextLogFilePath(_selectedContextId, _isRecordFileExist, _recordFilePath);
                GT_ASSERT(rcFilePath);
            }
        }
        break;

        case apEvent::AP_FLUSH_TEXTURE_IMAGES_EVENT:

        {
            if (_isDebuggedProcessSuspended)
            {
                // Get the (possibly new) file path:
                bool rcFilePath = gaGetContextLogFilePath(_selectedContextId, _isRecordFileExist, _recordFilePath);
                GT_ASSERT(rcFilePath);
            }
        }
        break;

        case apEvent::AP_OUTPUT_DEBUG_STRING:
        {
            const apOutputDebugStringEvent& outputDebugStringEvent = (const apOutputDebugStringEvent&)eve;
            gtString debugString = outputDebugStringEvent.debugString();

            gtString logFileCreatedString = SU_STR_logFileCreated;

            // If the debug string event is a log file created:
            if (debugString.startsWith(logFileCreatedString))
            {
                gtString logFileCreatedPath;

                // Get the log file path out of the debug string:
                debugString.getSubString(logFileCreatedString.length(), 0, logFileCreatedPath);

                // Create an osFilePath from the string and mirror the file:
                osFilePath spyRecordedFilePath(logFileCreatedPath);

                _isRecordFileExist = _recordFilePath.exists();
            }
        }
        break;

        case apEvent::APP_GLOBAL_VARIABLE_CHANGED:
        {
            // Down cast the event:
            const afGlobalVariableChangedEvent& variableChangedEvent = (const afGlobalVariableChangedEvent&)eve;

            // Handle it:
            onGlobalVariableChanged(variableChangedEvent);
        }
        break;

        case apEvent::GD_MONITORED_OBJECT_SELECTED_EVENT:
        {
            onTreeItemSelection((const apMonitoredObjectsTreeSelectedEvent&)eve);
            break;
        }

        case apEvent::AP_EXECUTION_MODE_CHANGED_EVENT:
        {
            // Enable the view only in debug mode:
            bool isEnabled = true;
            bool modeChanged = gdDoesModeChangeApplyToDebuggerViews(eve, isEnabled);

            if (modeChanged)
            {
                setEnabled(isEnabled);
            }
        }
        break;

        default:
            // Do nothing...
            break;
    }
}

// ---------------------------------------------------------------------------
// Name:        gdCallsHistoryToolbar::addToolbarTools
// Description: Add the toolbars tools into the toolbar - will manipulate the
//              toggle for the record button
// Author:      Avi Shapira
// Date:        19/8/2004
// ---------------------------------------------------------------------------
void gdCallsHistoryToolbar::addToolbarTools()
{
    // Define icons for the actions:
    // Add the toolbar Bitmaps
    QPixmap* pRecordIcon = new QPixmap;
    acSetIconInPixmap(*pRecordIcon, AC_ICON_DEBUG_CALLSHISTORY_RECORD);

    QPixmap* pOpenLogFileIcon = new QPixmap;
    acSetIconInPixmap(*pOpenLogFileIcon, AC_ICON_DEBUG_CALLSHISTORY_OPENLOG);

    QPixmap* pNextMarkerIcon = new QPixmap;
    acSetIconInPixmap(*pNextMarkerIcon, AC_ICON_DEBUG_CALLSHISTORY_NEXTMARKER);

    QPixmap* pPrevMarkerIcon = new QPixmap;
    acSetIconInPixmap(*pPrevMarkerIcon, AC_ICON_DEBUG_CALLSHISTORY_PREVIOUSMARKER);

    // Add record action:
    _pRecordAction = addAction(QIcon(*pRecordIcon), GD_STR_CallsHistoryToolbarTooltipStartRecording);
    GT_ASSERT(_pRecordAction != NULL);
    _pRecordAction->setCheckable(true);

    // Add open log file action:
    _pOpenRecordAction = addAction(QIcon(*pOpenLogFileIcon), GD_STR_CallsHistoryToolbarTooltipOpenRecordedFile);
    GT_ASSERT(_pOpenRecordAction != NULL);

    // Separator:
    addSeparator();

    // Next + prev marker:
    _pNextMarkerAction = addAction(QIcon(*pNextMarkerIcon), GD_STR_CallsHistoryToolbarTooltipNextMarker);
    GT_ASSERT(_pNextMarkerAction != NULL);

    _pPrevMarkerAction = addAction(QIcon(*pPrevMarkerIcon), GD_STR_CallsHistoryToolbarTooltipPreviousMarker);
    GT_ASSERT(_pPrevMarkerAction != NULL);

    // Connect the actions to handlers:
    bool rcConnect = connect(_pRecordAction, SIGNAL(triggered()), this, SLOT(onRecordingClick()));
    GT_ASSERT(rcConnect);

    rcConnect = connect(_pOpenRecordAction, SIGNAL(triggered()), this, SLOT(onOpenRecordedFileClick()));
    GT_ASSERT(rcConnect);

    rcConnect = connect(_pNextMarkerAction, SIGNAL(triggered()), this, SLOT(onNextMarkerClick()));
    GT_ASSERT(rcConnect);

    rcConnect = connect(_pPrevMarkerAction, SIGNAL(triggered()), this, SLOT(onPreviousMarkerClick()));
    GT_ASSERT(rcConnect);
}
