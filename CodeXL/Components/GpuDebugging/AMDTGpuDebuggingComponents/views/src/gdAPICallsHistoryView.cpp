//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gdAPICallsHistoryView.cpp
///
//==================================================================================

//------------------------------ gdAPICallsHistoryView.cpp ------------------------------

// Qt
#include <qtIgnoreCompilerWarnings.h>
#include <AMDTApplicationComponents/Include/acQtIncludes.h>

// Infra
#include <AMDTOSWrappers/Include/osDebuggingFunctions.h>
#include <AMDTBaseTools/Include/gtString.h>
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTAPIClasses/Include/apAPIConnectionType.h>
#include <AMDTAPIClasses/Include/apParameters.h>
#include <AMDTAPIClasses/Include/apMonitoredFunctionBreakPoint.h>
#include <AMDTAPIClasses/Include/apGLTexture.h>
#include <AMDTAPIClasses/Include/Events/apBreakpointHitEvent.h>
#include <AMDTAPIClasses/Include/Events/apBreakpointsUpdatedEvent.h>
#include <AMDTAPIClasses/Include/Events/apEventsHandler.h>
#include <AMDTAPIClasses/Include/Events/apExceptionEvent.h>
#include <AMDTAPIClasses/Include/Events/apDebuggedProcessRunSuspendedEvent.h>
#include <AMDTAPIClasses/Include/Events/apMonitoredObjectsTreeEvent.h>
#include <AMDTApiFunctions/Include/gaGRApiFunctions.h>
#include <AMDTApplicationComponents/Include/acColours.h>
#include <AMDTApplicationComponents/Include/acMessageBox.h>
#include <AMDTApplicationComponents/Include/acFunctions.h>

// AMDTApplicationFramework:
#include <AMDTApplicationFramework/Include/afAppStringConstants.h>
#include <AMDTApplicationFramework/Include/afGlobalVariableChangedEvent.h>
#include <AMDTApplicationFramework/Include/afProgressBarWrapper.h>
#include <AMDTApplicationFramework/src/afUtils.h>

// Local:
#include <AMDTGpuDebuggingComponents/Include/gdAidFunctions.h>
#include <AMDTGpuDebuggingComponents/Include/gdApplicationCommands.h>
#include <AMDTGpuDebuggingComponents/Include/gdStatusBar.h>
#include <AMDTGpuDebuggingComponents/Include/gdCommandIDs.h>
#include <AMDTGpuDebuggingComponents/Include/gdStringConstants.h>
#include <AMDTGpuDebuggingComponents/Include/gdPropertiesEventObserver.h>
#include <AMDTGpuDebuggingComponents/Include/gdGDebuggerGlobalVariablesManager.h>
#include <AMDTGpuDebuggingComponents/Include/views/gdAPICallsHistoryView.h>
#include <AMDTGpuDebuggingComponents/Include/dialogs/gdBreakpointsDialog.h>
#include <AMDTGpuDebuggingComponents/Include/gdHTMLProperties.h>

// ---------------------------------------------------------------------------
// Name:        gdAPICallsHistoryView::gdAPICallsHistoryView
// Description: Constructor
// Arguments:   pProgressBar - the application progress bar
//              QWidget* pParent
//              wxWindowID winID
// Author:      Sigal Algranaty
// Date:        14/7/2008
// ---------------------------------------------------------------------------
gdAPICallsHistoryView::gdAPICallsHistoryView(afProgressBarWrapper* pProgressBar, QWidget* pParent, bool isGlobal, bool shouldSetCaption)
    : acVirtualListCtrl(pParent, NULL), afBaseView(pProgressBar), _pBreakOnAction(NULL), _pEnableDisaleAllBreakpointsAction(NULL), _pAddRemoveBreakpointsAction(NULL),
      _pTableModel(NULL), m_isGlobal(isGlobal), _previousRowCount(0),
      _amountOfFunctionCalls(0), m_isDataUpdated(false), _processRunSuspendedInContext(AP_OPENGL_CONTEXT, 0), _isDuringSecondChanceExceptionHandling(false),
      _isDebuggedProcessSuspended(false), _activeContextId(AP_OPENGL_CONTEXT, 0), _executionMode(AP_DEBUGGING_MODE),
      _GLCallIconIndex(-1), _CLCallIconIndex(-1), _GLExtCallIconIndex(-1), _osSpecificAPICallIconIndex(-1), _osSpecificExtensionAPICallIconIndex(-1), _stringMarkerIconIndex(-1),
      _textureIconIndex(-1), _glBufferIconIndex(-1), _clBufferIconIndex(-1), _queueIconIndex(-1), _nextFunctionCallIconIndex(-1),
      _programsAndShadersCallIconIndex(-1), _profileIconIndex(-1), _yellowWarningIconIndex(-1), _orangeWarningIconIndex(-1),
      _redWarningIconIndex(-1), _shouldSetCaption(shouldSetCaption)
{
    // Register myself to listen to debugged process events:
    apEventsHandler::instance().registerEventsObserver(*this, AP_APPLICATION_COMPONENTS_EVENTS_HANDLING_PRIORITY);

    // Initialize relevant members:
    initialize();

    // Initialize the icons:
    initializeListIcons();

    // Create the model for the table view:
    _pTableModel = new gdAPICallsHistoryViewModel(this);

    // Set the model:
    setModel(_pTableModel);

    // Set the default line height:
    unsigned int scaledLineHeight = acScalePixelSizeToDisplayDPI(AC_DEFAULT_LINE_HEIGHT);
    verticalHeader()->setDefaultSectionSize((int)scaledLineHeight);
    verticalHeader()->hide();
    setShowGrid(false);

    // Connect my slots to signals:
    bool rcConnect = connect(this, SIGNAL(clicked(const QModelIndex&)), this, SLOT(onCallsHistoryItemClicked(const QModelIndex&)));
    GT_ASSERT(rcConnect);

    // Allow only single selection:
    setSelectionMode(QAbstractItemView::SingleSelection);
    setSelectionBehavior(QAbstractItemView::SelectRows);
}

// ---------------------------------------------------------------------------
// Name:        gdAPICallsHistoryView::initAfterCreation
// Description: Initializes the view after it is created
// Return Val:  void
// Author:      Sigal Algranaty
// Date:        8/2/2011
// ---------------------------------------------------------------------------
void gdAPICallsHistoryView::initAfterCreation()
{
    // Check if the debugged process is suspended:
    _isDebuggedProcessSuspended = gaIsDebuggedProcessSuspended();

    // Update the according to the current debugged process status:
    if (_isDebuggedProcessSuspended)
    {
        apDebuggedProcessRunSuspendedEvent dummySuspendedEvent;
        onProcessRunSuspended(dummySuspendedEvent);
    }
    else
    {
        // Check if the debugged process had started:
        bool isStarted = gaDebuggedProcessExists();

        if (!isStarted)
        {
            onProcessTerminated();
        }
    }

    // Extend the context menu:
    extendContextMenu();
}

// ---------------------------------------------------------------------------
// Name:        gdAPICallsHistoryView::~gdAPICallsHistoryView
// Description: Destructor
// Author:      Yaki Tebeka
// Date:        1/11/2003
// ---------------------------------------------------------------------------
gdAPICallsHistoryView::~gdAPICallsHistoryView()
{
    // Unregister myself from listening to debugged process events:
    apEventsHandler::instance().unregisterEventsObserver(*this);
}

// ---------------------------------------------------------------------------
// Name:        gdAPICallsHistoryView::onEvent
// Description: Is called when a debugged process event occurs.
//              Send the event to the appropriate event function for displaying
//              it to the user.
// Arguments:   eve - The debugged process event.
// Author:      Yaki Tebeka
// Date:        4/4/2004
// ---------------------------------------------------------------------------
void gdAPICallsHistoryView::onEvent(const apEvent& eve, bool& vetoEvent)
{
    (void)(vetoEvent);  // unused
    // Get the event type:
    apEvent::EventType eventType = eve.eventType();

    // Handle the event according to its type:
    switch (eventType)
    {
        case apEvent::AP_DEBUGGED_PROCESS_CREATED:
        {
            onProcessCreated();
        }
        break;

        case apEvent::AP_DEBUGGED_PROCESS_TERMINATED:
        case apEvent::AP_DEBUGGED_PROCESS_CREATION_FAILURE:
        {
            onProcessTerminated();
        }
        break;

        case apEvent::AP_DEBUGGED_PROCESS_RUN_SUSPENDED:
        {
            onProcessRunSuspended(eve);
        }
        break;

        case apEvent::AP_DEBUGGED_PROCESS_RUN_RESUMED:
        {
            onProcessRunResumed();
        }
        break;

        case apEvent::AP_BREAKPOINT_HIT:
        {
            onBreakpointHit((const apBreakpointHitEvent&)eve);
        }
        break;

        case apEvent::AP_EXCEPTION:
        {
            // If this is a second chance exception:
            apExceptionEvent& exceptionEvent = (apExceptionEvent&)eve;

            if (exceptionEvent.isSecondChance())
            {
                // Mark that we are during second chance exception handling:
                _isDuringSecondChanceExceptionHandling = true;
            }
        }
        break;


        case apEvent::AP_EXECUTION_MODE_CHANGED_EVENT:
        {
            // Enable the view only in debug mode:
            bool isEnabled = true;
            bool modeChanged = gdDoesModeChangeApplyToDebuggerViews(eve, isEnabled);

            if (modeChanged)
            {
                setEnabled(isEnabled);

                if (!isEnabled)
                {
                    clearList();
                }
            }
        }
        break;

        case apEvent::GD_MONITORED_OBJECT_SELECTED_EVENT:
        {
            const apMonitoredObjectsTreeSelectedEvent& selectionEvent = (const apMonitoredObjectsTreeSelectedEvent&)eve;
            onTreeItemSelection(selectionEvent);
        }
        break;


        default:
            // Do nothing...
            break;
    }
}


// ---------------------------------------------------------------------------
// Name:        gdAPICallsHistoryView::onProcessCreated
// Description: Is called when a debugged process is created.
// Author:      Yaki Tebeka
// Date:        13/6/2004
// ---------------------------------------------------------------------------
void gdAPICallsHistoryView::onProcessCreated()
{
    // Initialize relevant members:
    initialize();

    // Delete the items from the list
    clearList();
}


// ---------------------------------------------------------------------------
// Name:        gdAPICallsHistoryView::onProcessTerminated
// Description: Is called when the debugged process is terminated.
// Author:      Yaki Tebeka
// Date:        13/6/2004
// ---------------------------------------------------------------------------
void gdAPICallsHistoryView::onProcessTerminated()
{
    // Remove the items from the list:
    _amountOfFunctionCalls = 0;
    m_isDataUpdated = false;
    clearList();

    // Clear the status bar:
    gdStatusBar* pStatusBar = gdStatusBar::instance();

    if (pStatusBar != NULL)
    {
        gtString emptyText;
        pStatusBar->setText(emptyText, 0);
    }
}


// ---------------------------------------------------------------------------
// Name:        gdAPICallsHistoryView::onProcessRunSuspended
// Description: Is called when the debugged process run is suspended.
// Author:      Yaki Tebeka
// Date:        13/6/2004
// ---------------------------------------------------------------------------
void gdAPICallsHistoryView::onProcessRunSuspended(const apEvent& event)
{
    (void)(event);  // unused

    // If we are during second chance exception handling
    if (_isDuringSecondChanceExceptionHandling)
    {
        // Leave the list as it is, it might help finding the problem reason.
    }
    else
    {
        // Set the debugged process run suspended flag:
        _isDebuggedProcessSuspended = gaIsDebuggedProcessSuspended();

        if (gaIsAPIConnectionActive(AP_SPIES_UTILITIES_API_CONNECTION))
        {
            // Update the active context with the current triggering context id:
            bool rcUpdateContext = gaGetBreakpointTriggeringContextId(_processRunSuspendedInContext);
            GT_ASSERT(rcUpdateContext);

            // Set the active context:
            _activeContextId = _processRunSuspendedInContext;
        }

        // Update the list:
        updateList(_activeContextId);
    }
}


// ---------------------------------------------------------------------------
// Name:        gdAPICallsHistoryView::onProcessRunResumed
// Description: Is called when the debugged process run is resumed.
// Author:      Yaki Tebeka
// Date:        13/6/2004
// ---------------------------------------------------------------------------
void gdAPICallsHistoryView::onProcessRunResumed()
{
    // Delete the items from the list:
    _amountOfFunctionCalls = 0;
    m_isDataUpdated = false;
    clearList();

    // Clear the status bar:
    gdStatusBar* pStatusBar = gdStatusBar::instance();

    if (pStatusBar != NULL)
    {
        gtString emptyText;
        pStatusBar->setText(emptyText, 0);
    }
}


// ---------------------------------------------------------------------------
// Name:        gdAPICallsHistoryView::onBreakpointHit
// Description: Is called when the debugged process hits a breakpoint.
// Arguments:   event - Represents the breakpoint event.
// Author:      Yaki Tebeka
// Date:        23/5/2005
// Implementation Notes:
//   We display the
// ---------------------------------------------------------------------------
void gdAPICallsHistoryView::onBreakpointHit(const apBreakpointHitEvent& event)
{
    // Get the break reason:
    (void) event.breakReason();

    int lastFunctionIndex = _amountOfFunctionCalls - 1;

    if (lastFunctionIndex >= 0)
    {
        selectRow(lastFunctionIndex);

        QAbstractItemModel* pModel = model();
        GT_IF_WITH_ASSERT(pModel != NULL)
        {
            QModelIndex lastFunctionModelIndex = pModel->index(lastFunctionIndex, 0);
            scrollTo(lastFunctionModelIndex, QAbstractItemView::EnsureVisible);
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        gdAPICallsHistoryView::addIcon
// Description: Add an icon to the version of icons
// Arguments:   const char * const xpm[]
//              int& iconIndex
// Return Val:  void
// Author:      Sigal Algranaty
// Date:        12/3/2012
// ---------------------------------------------------------------------------
void gdAPICallsHistoryView::addIcon(acIconId iconId, int& iconIndex)
{
    // Create a Qt icon from the pixmap char pointer:
    QPixmap pixmap;
    acSetIconInPixmap(pixmap, iconId);
    QIcon* pNewIcon = new QIcon(pixmap);

    // Add to icons vector:
    _itemsIconsVector.push_back(pNewIcon);

    // Set the new item index:
    iconIndex = _itemsIconsVector.size() - 1;
}

// ---------------------------------------------------------------------------
// Name:        gdAPICallsHistoryView::initializeListIcons
// Description: Creates this view image list, and loads its images
//              from disk.
// Author:      Yaki Tebeka
// Date:        1/11/2003
// ---------------------------------------------------------------------------
void gdAPICallsHistoryView::initializeListIcons()
{
    // Add the Bitmaps into the icons vector file:
    addIcon(AC_ICON_DEBUG_CALLSHISTORY_GL, _GLCallIconIndex);
    addIcon(AC_ICON_DEBUG_CALLSHISTORY_CL, _CLCallIconIndex);
    addIcon(AC_ICON_DEBUG_CALLSHISTORY_GLEXT, _GLExtCallIconIndex);

#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
    addIcon(AC_ICON_DEBUG_CALLSHISTORY_WGL, _osSpecificAPICallIconIndex);
    addIcon(AC_ICON_DEBUG_CALLSHISTORY_WGL, _osSpecificExtensionAPICallIconIndex);
#elif AMDT_BUILD_TARGET == AMDT_LINUX_OS
#if (AMDT_LINUX_VARIANT == AMDT_GENERIC_LINUX_VARIANT)
    addIcon(AC_ICON_DEBUG_CALLSHISTORY_GLX, _osSpecificAPICallIconIndex);
    addIcon(AC_ICON_DEBUG_CALLSHISTORY_GLX, _osSpecificExtensionAPICallIconIndex);
#elif (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT)
    addIcon(AC_ICON_DEBUG_CALLSHISTORY_CGL, _osSpecificAPICallIconIndex);
    addIcon(AC_ICON_DEBUG_CALLSHISTORY_CGL, _osSpecificExtensionAPICallIconIndex);
#else
#error Unknown Linux variant!
#endif
#else
#error Unsupported platform
#endif

    addIcon(AC_ICON_DEBUG_CALLSHISTORY_NEXTMARKER, _stringMarkerIconIndex);
    addIcon(AC_ICON_DEBUG_APPTREE_GL_TEXGENERIC, _textureIconIndex);
    addIcon(AC_ICON_DEBUG_APPTREE_CL_BUFFER, _clBufferIconIndex);
    addIcon(AC_ICON_DEBUG_APPTREE_GL_BUFFER_GENERIC, _glBufferIconIndex);
    addIcon(AC_ICON_DEBUG_APPTREE_CL_QUEUE, _queueIconIndex);
    addIcon(AC_ICON_SOURCE_TOP_PROGRAM_COUNTER, _nextFunctionCallIconIndex);
    addIcon(AC_ICON_DEBUG_APPTREE_GL_SHADER, _programsAndShadersCallIconIndex);
    addIcon(AC_ICON_PROFILE_MODE, _profileIconIndex);
    addIcon(AC_ICON_WARNING_YELLOW, _yellowWarningIconIndex);
    addIcon(AC_ICON_WARNING_ORANGE, _orangeWarningIconIndex);
    addIcon(AC_ICON_WARNING_RED, _redWarningIconIndex);
}


// ---------------------------------------------------------------------------
// Name:        gdAPICallsHistoryView::OnCallsHistorySelected
// Description: Write the details of the selected function to the properties view
// Author:      Avi Shapira
// Date:        3/5/2004
// ---------------------------------------------------------------------------
void gdAPICallsHistoryView::onRowSelected(const QModelIndex& index)
{
    // Call the base class implementation:
    acVirtualListCtrl::onRowSelected(index);

    bool wereSignalsBlocked = blockSignals(true);

    // Make sure we are in the main frame instance of this view:
    if (m_isGlobal)
    {
        // Set the selected function Id into the a var that will be sent top the dialog as a parameter
        int rowIndex = index.row();

        if (rowIndex >= 0)
        {
            // Update the properties:
            updatePropertiesAndStatusBar(rowIndex);
        }
    }

    blockSignals(wereSignalsBlocked);
}
// ---------------------------------------------------------------------------
// Name:        gdAPICallsHistoryView::updateList
// Description: Update the listCtrl when needed
// Author:      Avi Shapira
// ---------------------------------------------------------------------------
void gdAPICallsHistoryView::updateList(apContextID contextId)
{

    // Do not emit selection signals:
    bool wereSignalsBlocked = blockSignals(true);
    bool wereModelSignalsBlocked = selectionModel()->blockSignals(true);

    // Only call API calls when debugged process is suspended:
    if (_isDebuggedProcessSuspended)
    {

        // Get the "Profiling/Debugging" mode:
        bool rc = gaGetDebuggedProcessExecutionMode(_executionMode);
        GT_ASSERT(rc);

        if (_executionMode != AP_PROFILING_MODE)
        {
            // Debug mode & Analyze:

            // Set the list size to the amount of the OpenGL function calls
            int amountOfFunctionCalls = 0;
            rc = gaGetAmountOfCurrentFrameFunctionCalls(contextId, amountOfFunctionCalls);

            if (rc)
            {
                _amountOfFunctionCalls = amountOfFunctionCalls;
            }
        }
        else
        {
            // Profiling mode
            _amountOfFunctionCalls = 1;
        }

        // Set the context id:
        _activeContextId = contextId;

        // The data is now updated from server
        m_isDataUpdated = true;


        if (_amountOfFunctionCalls > 0)
        {
            gtString statusMessage;

            if (_executionMode != AP_PROFILING_MODE)
            {
                // Debugging mode:
                // Write the next function in the status bar:
                GT_IF_WITH_ASSERT(model() != NULL)
                {
                    // Get the last function text:
                    // Get the appropriate function call:
                    gtAutoPtr<apFunctionCall> aptrFunctionCall;
                    rc = gaGetCurrentFrameFunctionCall(_activeContextId, _amountOfFunctionCalls - 1, aptrFunctionCall);

                    if (rc)
                    {
                        // Get the function call as string:
                        // NOTICE: do not use apFunctionCall::asString, since we want to add
                        // logical translation to the function arguments:
                        rc = gdFunctionCallAsString(aptrFunctionCall.pointedObject(), statusMessage, _activeContextId._contextType);
                    }

                    // Update the status string:
                    statusMessage.prepend(GD_STR_CallsHistoryStatusBarStoppedBeforeExecuting);
                }
            }
            else
            {
                // Profiling mode
                statusMessage = GD_STR_PropertiesFunctionCallsNotLoggedInProfilingMode;
            }

            // Clear the status bar:
            gdStatusBar* pStatusBar = gdStatusBar::instance();

            if (pStatusBar != NULL)
            {
                gtString emptyText;
                pStatusBar->setText(statusMessage, 0);
            }
        }

        // Update the model with the changes:
        GT_IF_WITH_ASSERT(_pTableModel != NULL)
        {
            _pTableModel->updateModel();

            // Update the header data
            _pTableModel->headerDataChanged(Qt::Horizontal, 0, 0);
        }

        int lastFunctionIndex = _amountOfFunctionCalls - 1;

        if (lastFunctionIndex >= 0)
        {
            selectRow(lastFunctionIndex);

            QAbstractItemModel* pModel = model();
            GT_IF_WITH_ASSERT(pModel != NULL)
            {
                QModelIndex lastFunctionModelIndex = pModel->index(lastFunctionIndex, 0);
                scrollTo(lastFunctionModelIndex, QAbstractItemView::EnsureVisible);
            }
        }
    }

    // Set the view caption:
    setCaption();

    // Remove the signal block:
    selectionModel()->blockSignals(wereModelSignalsBlocked);
    blockSignals(wereSignalsBlocked);

}


// ---------------------------------------------------------------------------
// Name:        gdAPICallsHistoryView::clearList
// Description: Delete all the items from the listCtrl
// Author:      Avi Shapira
// Date:        8/4/2004
// ---------------------------------------------------------------------------
void gdAPICallsHistoryView::clearList()
{
    GT_IF_WITH_ASSERT(_pTableModel != NULL)
    {
        _pTableModel->updateModel();
    }
}

// ---------------------------------------------------------------------------
// Name:        gdAPICallsHistoryView::getItemIcon
// Description: Given an item index (function index within the list), return the
//              function matching icon
// Arguments:   long item
// Return Val:  QIcon*
// Author:      Sigal Algranaty
// Date:        27/12/2011
// ---------------------------------------------------------------------------
QIcon* gdAPICallsHistoryView::getItemIcon(long item) const
{
    QIcon* pRetVal = NULL;
    int iconIndex = -1;

    // Only call API calls when debugged process is suspended:
    if (_isDebuggedProcessSuspended)
    {
        // Are we in "Profiling" mode:
        if (_executionMode == AP_PROFILING_MODE)
        {
            // profiling mode
            iconIndex = _profileIconIndex;
        }
        else
        {

            // Get the function call details:
            gtAutoPtr<apFunctionCall> aptrFunctionCall;
            bool gotFunctionDetails = gaGetCurrentFrameFunctionCall(_activeContextId, item, aptrFunctionCall);

            if (gotFunctionDetails)
            {
                // If this is the last function in the calls log and the displayed context is
                // the context that caused the debugged process suspension:
                if (((_amountOfFunctionCalls - 1) == item) && (_processRunSuspendedInContext == _activeContextId))
                {
                    iconIndex = _nextFunctionCallIconIndex;
                }
                else
                {
                    // Get the current function id:
                    apMonitoredFunctionId funcId = aptrFunctionCall->functionId();

                    // Get the function icon by the function type:
                    iconIndex = iconByFunctionType(funcId);

                    apExecutionMode currentExecMode = AP_DEBUGGING_MODE;
                    gaGetDebuggedProcessExecutionMode(currentExecMode);

                    // If we are in analyze mode, set get functions ,redundant state changes,
                    // and deprecated functions to have warning icons:
                    if (currentExecMode == AP_ANALYZE_MODE)
                    {
                        // Get the function type:
                        unsigned int funcType = 0;
                        bool rc = gaGetMonitoredFunctionType(funcId, funcType);

                        if (rc)
                        {
                            // For state change functions, check the redundancy status:
                            if (funcType & AP_STATE_CHANGE_FUNC)
                            {
                                // For redundant function calls, get a different item attributes:
                                apFunctionRedundancyStatus redundancyStatus = aptrFunctionCall->getRedundanctStatus();

                                if (redundancyStatus == AP_REDUNDANCY_REDUNDANT)
                                {
                                    iconIndex = _redWarningIconIndex;
                                }
                            }

                            if ((funcType & AP_GET_FUNC) && (iconIndex < 10))
                            {
                                iconIndex = _orangeWarningIconIndex;
                            }

                            if ((aptrFunctionCall->getDeprecationStatus() != AP_DEPRECATION_NONE) && (iconIndex < 9))
                            {
                                iconIndex = _yellowWarningIconIndex;
                            }
                        }
                    }
                }
            }
        }
    }

    if ((iconIndex >= 0) && (iconIndex < (int)_itemsIconsVector.size()))
    {
        // Return the icon from the vector:
        pRetVal = _itemsIconsVector[iconIndex];
    }

    return pRetVal;
}


// ---------------------------------------------------------------------------
// Name:        gdAPICallsHistoryView::iconByFunctionType
// Description: Inputs a function call and outputs the icon that represents its type.
// Arguments:   functionCall - The input function call.
// Return Val:  int - The index of the icon that represents the additional data
//               parameters, or 0 if there are no additional parameters, or they
//               have no representing icon.
// Author:       Sigal Algranaty
// Date:        2/2/2011
// ---------------------------------------------------------------------------
int gdAPICallsHistoryView::iconByFunctionType(int functionID) const
{
    int retVal = -1;

    // If this is "glStringMarkerGREMEDY":
    if (functionID == ap_glStringMarkerGREMEDY)
    {
        // Return the code for the String Marker icon.
        retVal = _stringMarkerIconIndex;
    }
    else
    {
        // First get the specific function type icon:

        // Get the function type:
        unsigned int functionTypeAsUInt = 0;
        gaGetMonitoredFunctionType((apMonitoredFunctionId)functionID, functionTypeAsUInt);
        apFunctionType functionType = (apFunctionType)functionTypeAsUInt;

        // Compare the function type to find an appropriate icon:
        if ((functionType & AP_PROGRAM_SHADER_FUNC) || (functionType & AP_PROGRAM_KERNEL_FUNC))
        {
            retVal = _programsAndShadersCallIconIndex;
        }

        if (functionType & AP_TEXTURE_FUNC)
        {
            retVal = _textureIconIndex;
        }

        if (functionType & AP_BUFFER_FUNC)
        {
            retVal = _glBufferIconIndex;
        }

        if (functionType & AP_BUFFER_IMAGE_FUNC)
        {
            retVal = _clBufferIconIndex;
        }

        if (functionType & AP_QUEUE_FUNC)
        {
            retVal = _queueIconIndex;
        }

        // If there is no specific function type icon, get a generic one:
        if (retVal < 0)
        {
            // Get the function API type:
            unsigned int functionAPITypeAsUInt = 0;
            gaGetMonitoredFunctionAPIType((apMonitoredFunctionId)functionID, functionAPITypeAsUInt);
            apAPIType functionAPIType = (apAPIType)functionAPITypeAsUInt;

            if ((functionAPIType & AP_OPENGL_GENERIC_FUNC) || (functionAPIType & AP_OPENGL_ES_GENERIC_FUNC))
            {
                retVal = _GLCallIconIndex;
            }

            else if ((functionAPIType & AP_OPENGL_EXTENSION_FUNC) || (functionAPIType & AP_OPENGL_ES_EXTENSION_FUNC))
            {
                retVal = _GLExtCallIconIndex;
            }

            else if ((functionAPIType & AP_WGL_FUNC) || (functionAPIType & AP_EGL_FUNC) || (functionAPIType & AP_EGL_EXTENSION_FUNC)
                     || (functionAPIType & AP_GLX_FUNC) || (functionAPIType & AP_CGL_FUNC))
            {
                retVal = _osSpecificAPICallIconIndex;
            }

            else if ((functionAPIType & AP_WGL_EXTENSION_FUNC) || (functionAPIType & AP_GLX_EXTENSION_FUNC))
            {
                retVal = _osSpecificExtensionAPICallIconIndex;
            }

            else if ((functionAPIType & AP_OPENCL_GENERIC_FUNC) || (functionAPIType & AP_OPENCL_EXTENSION_FUNC) || (functionAPIType & AP_OPENCL_AMD_EXTENSION_FUNC))
            {
                retVal = _CLCallIconIndex;
            }
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gdAPICallsHistoryView::onCallsHistoryItemClicked
// Description: A slot handling an item click - Write the details of the selected
//              function to the properties view
// Arguments:   const QModelIndex & index - the index of the item clicked
// Author:      Sigal Algranaty
// Date:        27/12/2011
// ---------------------------------------------------------------------------
void gdAPICallsHistoryView::onCallsHistoryItemClicked(const QModelIndex& index)
{
    // Update the properties:
    updatePropertiesAndStatusBar(index.row());
}

// ---------------------------------------------------------------------------
// Name:        gdAPICallsHistoryView::onTreeItemSelection
// Description:
// Arguments:   const apMonitoredObjectsTreeSelectedEvent& eve
// Return Val:  void
// Author:      Sigal Algranaty
// Date:        27/12/2011
// ---------------------------------------------------------------------------
void gdAPICallsHistoryView::onTreeItemSelection(const apMonitoredObjectsTreeSelectedEvent& eve)
{
    // Get the item data from the event:
    afApplicationTreeItemData* pTreeItemData = (afApplicationTreeItemData*)(eve.selectedItemData());

    if (pTreeItemData != NULL)
    {
        gdDebugApplicationTreeData* pGDData = qobject_cast<gdDebugApplicationTreeData*>(pTreeItemData->extendedItemData());

        if (pGDData != NULL)
        {
            // If the debugged process is suspended:
            bool isDebuggedProcessSuspended = gaIsDebuggedProcessSuspended();

            if (isDebuggedProcessSuspended)
            {
                if (_activeContextId != pGDData->_contextId)
                {
                    // Set the new context:
                    _activeContextId = pGDData->_contextId;
                    updateList(_activeContextId);
                }
            }
        }
        else if (pTreeItemData->m_itemType == AF_TREE_ITEM_APP_ROOT)
        {
            if (!_activeContextId.isDefault())
            {
                // Set the new context:
                _activeContextId._contextId = 0;
                _activeContextId._contextType = AP_NULL_CONTEXT;
                updateList(_activeContextId);
            }
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        gdAPICallsHistoryView::isCurrentFunctionExistAsBreakpoint
// Description: The function checks if the current selected function is set as breakpoint
// Arguments: gtString& funcName - the current selected function name
//            int& funcId - the current selected function id
//            int& breakpointId - the breakpoint id within the persistent data manager data structure
// Return Val: bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        16/6/2008
// ---------------------------------------------------------------------------
bool gdAPICallsHistoryView::isCurrentFunctionExistAsBreakpoint(QString& funcName, apMonitoredFunctionId& funcId, int& breakpointId)
{

    bool retVal = false;

    breakpointId = -1;

    // Get the current selected item index:
    QModelIndexList selectedList = selectedIndexes();

    // Check if the selected function is a set breakpoint, and set the text accordingly:
    if (selectedList.size() > 0)
    {

        // Get the first selected item:
        int selectedIndex = selectedList.first().row();

        GT_IF_WITH_ASSERT(selectedIndex >= 0)
        {
            // Get the history call item text from OpenGL or OpenCL spy:
            gtAutoPtr<apFunctionCall> aptrFunctionCall;
            bool rc = gaGetCurrentFrameFunctionCall(_activeContextId, selectedIndex, aptrFunctionCall);

            if (rc)
            {
                // Get the current function id:
                funcId = aptrFunctionCall->functionId();

                // Get the current function name:
                gtString gtfuncName;
                rc = gaGetMonitoredFunctionName(funcId, gtfuncName);
                GT_IF_WITH_ASSERT(rc)
                {
                    funcName = acGTStringToQString(gtfuncName);

                    // Get the amount of active breakpoints:
                    int amountOfBreakpoints = 0;
                    bool rc1 = gaGetAmountOfBreakpoints(amountOfBreakpoints);
                    GT_IF_WITH_ASSERT(rc1)
                    {
                        // Iterate on the active breakpoints
                        for (int i = 0; i < amountOfBreakpoints; i++)
                        {
                            // Get the current breakpoint
                            gtAutoPtr<apBreakPoint> aptrBreakpoint;
                            bool rc2 = gaGetBreakpoint(i, aptrBreakpoint);
                            GT_IF_WITH_ASSERT(rc2)
                            {
                                // Get the breakpoint type:
                                osTransferableObjectType curentBreakpointType = aptrBreakpoint->type();

                                if (curentBreakpointType == OS_TOBJ_ID_MONITORED_FUNC_BREAKPOINT)
                                {
                                    // Down cast it to apMonitoredFunctionBreakPoint:
                                    apMonitoredFunctionBreakPoint* pFunctionBreakpoint = (apMonitoredFunctionBreakPoint*)(aptrBreakpoint.pointedObject());
                                    GT_IF_WITH_ASSERT(pFunctionBreakpoint != NULL)
                                    {
                                        // Get the breaked on function id:
                                        if (funcId == pFunctionBreakpoint->monitoredFunctionId())
                                        {
                                            if (pFunctionBreakpoint->isEnabled())
                                            {
                                                retVal = true;
                                            }

                                            breakpointId = i;
                                            break;
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gdAPICallsHistoryView::initialize
// Description: Initialize class members.
// Author:      Yaki Tebeka
// Date:        22/7/2008
// ---------------------------------------------------------------------------
void gdAPICallsHistoryView::initialize()
{
    _activeContextId._contextId = 0;
    _activeContextId._contextType = AP_NULL_CONTEXT;
    _amountOfFunctionCalls = 0;
    _processRunSuspendedInContext._contextId = 0;
    _processRunSuspendedInContext._contextType = AP_NULL_CONTEXT;
    _isDuringSecondChanceExceptionHandling = false;
}

// ---------------------------------------------------------------------------
// Name:        gdAPICallsHistoryView::onUpdateBreakpOnFunction
// Description: Check if there is anything that can be found
// Author:      Avi Shapira
// Date:        23/11/2004
// ---------------------------------------------------------------------------
void gdAPICallsHistoryView::onUpdateBreakpOnFunction(bool& isEnabled, bool& isChecked, QString& itemText)
{
    // Check if the selected function is already a set breakpoint, and set the menu item text accordingly:
    QString funcName;
    apMonitoredFunctionId funcId = apMonitoredFunctionsAmount;
    int breakpointId = -1;
    bool isCurrentFuncSetAsBreakpoint = isCurrentFunctionExistAsBreakpoint(funcName, funcId, breakpointId);
    itemText = QString(GD_STR_BreakpointsBreakOn).arg(funcName);

    // Check/Uncheck the item according to the existence of breakpoint at this function:
    isEnabled = !funcName.isEmpty();
    isChecked = isCurrentFuncSetAsBreakpoint;
}

// ---------------------------------------------------------------------------
// Name:        gdAPICallsHistoryView::onUpdateSelectAll
// Description: enables the select all command in the VS edit menu
// Author:      Gilad Yarnitzky
// Date:        6/2/2011
// ---------------------------------------------------------------------------
void gdAPICallsHistoryView::onUpdateSelectAll(bool& isEnabled)
{
    isEnabled = false;
}

// ---------------------------------------------------------------------------
// Name:        gdAPICallsHistoryView::onCopy
// Description: Copy list item to the clipboard
// Arguments:   wxListEvent &eve
// Author:      Avi Shapira
// Date:        23/11/2004
// ---------------------------------------------------------------------------
void gdAPICallsHistoryView::onCopy()
{
    acVirtualListCtrl::onEditCopy();
}

// ---------------------------------------------------------------------------
// Name:        gdAPICallsHistoryView::onEdit_Find()
// Description: load the find dialog
// Arguments:   wxListEvent &eve
// Author:      Avi Shapira
// Date:        19/11/2004
// ---------------------------------------------------------------------------
void gdAPICallsHistoryView::onEdit_Find()
{
    acVirtualListCtrl::onFindClick();
}

// ---------------------------------------------------------------------------
// Name:        gdAPICallsHistoryView::onEdit_FindNext()
// Description: Find next Item
// Author:      Avi Shapira
// Date:        19/11/2004
// ---------------------------------------------------------------------------
void gdAPICallsHistoryView::onEdit_FindNext()
{
    acVirtualListCtrl::onFindNext();
}


// ---------------------------------------------------------------------------
// Name:        gdAPICallsHistoryView::onSelectAll
// Description: Select all the items from the list
// Return Val: void
// Author:      Sigal Algranaty
// Date:        4/8/2008
// ---------------------------------------------------------------------------
void gdAPICallsHistoryView::onSelectAll()
{
    selectAll();
}

// ---------------------------------------------------------------------------
// Name:        gdAPICallsHistoryView::onBreakOnFunction()
// Description: Add a breakpoint
// Author:      Sigal Algranaty
// Date:        16/6/2008
// ---------------------------------------------------------------------------
void gdAPICallsHistoryView::onBreakOnFunction()
{
    QString funcName;
    gtString itemText;
    apMonitoredFunctionId funcId = apMonitoredFunctionsAmount;
    int breakpointId = -1;

    bool rc = true;

    // Check if the function already exist within the program breakpoints:
    bool isCurrentExistAsBreakpoint = isCurrentFunctionExistAsBreakpoint(funcName, funcId, breakpointId);

    // Create new breakpoint:
    apMonitoredFunctionBreakPoint breakpoint;
    breakpoint.setMonitoredFunctionId(funcId);

    // If the breakpoint doesn't exist, set it, otherwise remove it;
    if (isCurrentExistAsBreakpoint)
    {
        // Disable the breakpoint:
        breakpoint.setEnableStatus(false);
    }
    else
    {
        // Enable the breakpoint:
        breakpoint.setEnableStatus(true);
    }

    // Set the breakpoint:
    rc = gaSetBreakpoint(breakpoint);

    // Trigger breakpoints update event:
    // The -1 states the all the breakpoints are updated, and lists should be updated from scratch:
    apBreakpointsUpdatedEvent eve(-1);
    apEventsHandler::instance().registerPendingDebugEvent(eve);

    GT_ASSERT(rc);
}

// ---------------------------------------------------------------------------
// Name:        gdAPICallsHistoryView::onAddBreakpoints
// Description: Execute the context menu "Add / Remove Breakpoints" command
// Author:      Sigal Algranaty
// Date:        7/2/2011
// ---------------------------------------------------------------------------
void gdAPICallsHistoryView::onAddBreakpoints()
{
    // Verify that we can show the dialog (not in profile mode etc.)
    gdApplicationCommands* pApplicationCommandInstance = gdApplicationCommands::gdInstance();
    GT_IF_WITH_ASSERT(pApplicationCommandInstance != NULL)
    {
        pApplicationCommandInstance->openBreakpointsDialog();
    }
}

// ---------------------------------------------------------------------------
// Name:        gdAPICallsHistoryView::onUpdateAddBreakpoints
// Description: Set the enable status of the Add / Remove breakpoints context menu item
// Author:      Sigal Algranaty
// Date:        7/2/2011
// ---------------------------------------------------------------------------
void gdAPICallsHistoryView::onUpdateAddBreakpoints(bool& isEnabled)
{
    gdApplicationCommands* pApplicationCommandInstance = gdApplicationCommands::gdInstance();
    GT_IF_WITH_ASSERT(pApplicationCommandInstance != NULL)
    {
        isEnabled = pApplicationCommandInstance->isBreakpointsDialogCommandEnabled();
    }

}

// ---------------------------------------------------------------------------
// Name:        gdAPICallsHistoryView::onEnableAllBreakpoints
// Description: Enables or disables all breakpoints (this is a mirror of the
//              parallel check box in the breakpoints dialog).
// Author:      Sigal Algranaty
// Date:        7/2/2011
// ---------------------------------------------------------------------------
void gdAPICallsHistoryView::onEnableAllBreakpoints()
{
    // Get if we should enable of disable:
    bool isEnabled = false;

    // Down cast the sender to action:
    QAction* pAction = qobject_cast<QAction*>(sender());
    GT_IF_WITH_ASSERT(pAction != NULL)
    {
        isEnabled = pAction->isChecked();

        // Get the application command instance:
        gdApplicationCommands* pApplicationCommands = gdApplicationCommands::gdInstance();
        GT_IF_WITH_ASSERT(pApplicationCommands != NULL)
        {
            // Check if the action is enabled:
            bool enableAllBreakpointsChecked = false;
            bool enableAllBreakpointsEnabled = pApplicationCommands->isEnableAllBreakpointsCommandEnabled(enableAllBreakpointsChecked);

            if (enableAllBreakpointsEnabled)
            {
                bool rcEnable = pApplicationCommands->enableAllBreakpoints(isEnabled);
                GT_ASSERT(rcEnable);

                // Trigger breakpoints update event:
                // The -1 states the all the breakpoints are updated, and lists should be updated from scratch:
                apBreakpointsUpdatedEvent eve(-1);
                apEventsHandler::instance().registerPendingDebugEvent(eve);
            }
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        gdAPICallsHistoryView::onUpdateEnableDisableAllBreakpoints
// Description: Set the checked / enabled status of the enable / disable all
//              breakpoints menu item.
// Author:      Sigal Algranaty
// Date:        7/2/2011
// ---------------------------------------------------------------------------
void gdAPICallsHistoryView::onUpdateEnableAllBreakpoints(bool& isEnabled, bool& isChecked)
{
    isChecked = false;
    isEnabled = false;

    // Get the application command instance:
    gdApplicationCommands* pApplicationCommands = gdApplicationCommands::gdInstance();
    GT_IF_WITH_ASSERT(pApplicationCommands != NULL)
    {
        // Check if the action is enabled:
        isEnabled = pApplicationCommands->isEnableAllBreakpointsCommandEnabled(isChecked);
    }
}

// ---------------------------------------------------------------------------
// Name:        gdAPICallsHistoryView::onPreviousMarkerClick
// Description: Find the next marker.
// Author:      Avi Shapira
// Date:        1/2/2005
// ---------------------------------------------------------------------------
bool gdAPICallsHistoryView::findNextMarker(apSearchDirection searchDirection)
{
    bool retVal = false;
    int foundIndex = -1;
    long searchStartIndex = -1;

    GT_IF_WITH_ASSERT(_pTableModel != NULL)
    {
        int callsHistoryListSize = _pTableModel->rowCount(QModelIndex());

        // OpenCL does not currently have a string marker extension:
        if ((callsHistoryListSize > 0) && (_activeContextId._contextType == AP_OPENGL_CONTEXT))
        {
            // Find the first selected item:
            if (!selectedIndexes().empty())
            {
                searchStartIndex = selectedIndexes().first().row();
            }

            // If there is no selection in the list
            if (searchStartIndex == -1)
            {
                if (searchDirection == AP_SEARCH_INDICES_DOWN)
                {
                    searchStartIndex = callsHistoryListSize - 1;
                }
                else if (searchDirection == AP_SEARCH_INDICES_UP)
                {
                    searchStartIndex = 0;
                }
            }
            else
            {
                if ((searchDirection == AP_SEARCH_INDICES_DOWN) && (searchStartIndex != 0))
                {
                    searchStartIndex--;
                }
                else if ((searchDirection == AP_SEARCH_INDICES_UP) && (searchStartIndex != callsHistoryListSize - 1))
                {
                    searchStartIndex++;
                }
            }

            // find the marker:
            retVal = gaFindStringMarker(_activeContextId._contextId, searchDirection, searchStartIndex, foundIndex);
        }

        if (foundIndex != -1)
        {
            // found the marker
            selectRow(foundIndex);
        }
        else
        {
            acMessageBox::instance().warning(AF_STR_WarningA, GD_STR_CallsHistoryCannotFindStringMarker, QMessageBox::Ok);
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gdAPICallsHistoryView::canFindMarker
// Description: Check if we can search for a marker
// Return Val:  bool - Success / failure.
// Author:      Avi Shapira
// Date:        2/2/2005
// ---------------------------------------------------------------------------
bool gdAPICallsHistoryView::canFindMarker()
{
    bool retVal = false;

    GT_IF_WITH_ASSERT(_pTableModel != NULL)
    {
        if (_pTableModel->rowCount(QModelIndex()) > 0)
        {
            retVal = true;
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gdAPICallsHistoryView::updateFunctionCallsStatisticsList
// Description: Update function implementation
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        31/3/2010
// ---------------------------------------------------------------------------
bool gdAPICallsHistoryView::updateFunctionCallsStatisticsList(const apStatistics& currentStatistics)
{
    (void)(currentStatistics);  // unused
    updateList(_activeContextId);
    return true;
}

// ---------------------------------------------------------------------------
// Name:        gdAPICallsHistoryView::selectedFunctionIconType
// Description:
// Return Val:  afIconType
// Author:      Sigal Algranaty
// Date:        31/3/2011
// ---------------------------------------------------------------------------
afIconType gdAPICallsHistoryView::functionIconType(int iconIndex)
{
    afIconType retVal = AF_ICON_INFO;

    // Deduce statistics calls viewer icon type:
    if (iconIndex == _yellowWarningIconIndex)
    {
        // Warning level 1:
        retVal = AF_ICON_WARNING1;
    }

    else if (iconIndex == _orangeWarningIconIndex)
    {
        // Warning level 2:
        retVal = AF_ICON_WARNING2;
    }

    else if (iconIndex == _redWarningIconIndex)
    {
        // Warning level 3:
        retVal = AF_ICON_WARNING3;
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gdAPICallsHistoryView::resizeEvent
// Description: Resize the column width to fill the frame
// Arguments:   QResizeEvent *pResizeEvent
// Author:      Sigal Algranaty
// Date:        27/12/2011
// ---------------------------------------------------------------------------
void gdAPICallsHistoryView::resizeEvent(QResizeEvent* pResizeEvent)
{
    GT_IF_WITH_ASSERT(pResizeEvent != NULL)
    {
        // Resize the single column to fill the frame:
        setColumnWidth(0, pResizeEvent->size().width());
    }

    // Call the base class implementation:
    acVirtualListCtrl::resizeEvent(pResizeEvent);
}


// ---------------------------------------------------------------------------
// Name:        gdAPICallsHistoryView::shouldHighlightItem
// Description: Return true iff the function should be highlight
// Arguments:   int functionIndex
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        28/12/2011
// ---------------------------------------------------------------------------
bool gdAPICallsHistoryView::shouldHighlightItem(int functionIndex)
{
    bool retVal = false;

    // Only call API calls when debugged process is suspended:
    if (_isDebuggedProcessSuspended)
    {
        // If the active context is an OpenGL context:
        if ((_activeContextId.isOpenGLContext()) || (_activeContextId.isDefault()))
        {
            // Get the current function call:
            gtAutoPtr<apFunctionCall> aptrFunctionCall;
            bool rc = gaGetCurrentFrameFunctionCall(_activeContextId, functionIndex, aptrFunctionCall);

            if (rc)
            {
                // Get the current function id:
                int funcId = aptrFunctionCall->functionId();

                // If this is a string marker:
                if (funcId == ap_glStringMarkerGREMEDY)
                {
                    retVal = true;
                }
            }
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gdAPICallsHistoryView::extendContextMenu
// Description: Extend the context menu created by base
// Author:      Sigal Algranaty
// Date:        29/12/2011
// ---------------------------------------------------------------------------
void gdAPICallsHistoryView::extendContextMenu()
{
    // Sanity check:
    GT_IF_WITH_ASSERT(m_pContextMenu != NULL)
    {
        // Add separator:
        m_pContextMenu->addSeparator();

        // Add break on action (the action name is updated in aboutToShow slot:
        _pBreakOnAction = m_pContextMenu->addAction(AF_STR_NotAvailableA);
        GT_IF_WITH_ASSERT(_pBreakOnAction != NULL)
        {
            _pBreakOnAction->setCheckable(true);
            bool rc = connect(_pBreakOnAction, SIGNAL(triggered()), this, SLOT(onBreakOnFunction()));
            GT_ASSERT(rc);

        }

        // Add enable / disable all breakpoints:
        _pEnableDisaleAllBreakpointsAction = m_pContextMenu->addAction(GD_STR_EnableDisableAllBreakpoints);
        GT_IF_WITH_ASSERT(_pEnableDisaleAllBreakpointsAction != NULL)
        {
            _pEnableDisaleAllBreakpointsAction->setCheckable(true);
            bool rc = connect(_pEnableDisaleAllBreakpointsAction, SIGNAL(triggered()), this, SLOT(onEnableAllBreakpoints()));
            GT_ASSERT(rc);

        }

        // Add add / remove breakpoints action:
        _pAddRemoveBreakpointsAction = m_pContextMenu->addAction(GD_STR_breakpintsViewOpenDialog);
        GT_ASSERT(_pAddRemoveBreakpointsAction != NULL);

        bool rc = connect(_pAddRemoveBreakpointsAction, SIGNAL(triggered()), this, SLOT(onAddBreakpoints()));
        GT_ASSERT(rc);
    }
}

// ---------------------------------------------------------------------------
// Name:        gdAPICallsHistoryView::updatePropertiesAndStatusBar
// Description: Updates the properties view and status bar with the given row
// Author:      Uri Shomroni
// Date:        30/7/2015
// ---------------------------------------------------------------------------
void gdAPICallsHistoryView::updatePropertiesAndStatusBar(int rowIndex)
{

    // Build the function call properties string:
    gtString functionHTMLPropertiesString;
    gdHTMLProperties htmlPropertiesCreator;
    htmlPropertiesCreator.buildFunctionCallHTMLPropertiesString(_executionMode, _activeContextId, rowIndex, functionHTMLPropertiesString, _pOwnerProgressBar);

    // Set the properties view page:
    gdPropertiesEventObserver::instance().setPropertiesFromText(acGTStringToQString(functionHTMLPropertiesString));

    if (_executionMode != AP_PROFILING_MODE)
    {
        // Add the function index into the status bar:
        gdStatusBar* pStatusBar = gdStatusBar::instance();

        if (pStatusBar != NULL)
        {
            gtString listIndexString = GD_STR_CallsHistoryStatusBarFunctionIndex;
            listIndexString.appendFormattedString(L"%d", rowIndex + 1);
            pStatusBar->setText(listIndexString, 0);
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        gdAPICallsHistoryView::onAboutToShowContextMenu
// Description: Extending the enabling and disabling of context menu items for base class
// Author:      Sigal Algranaty
// Date:        29/12/2011
// ---------------------------------------------------------------------------
void gdAPICallsHistoryView::onAboutToShowContextMenu()
{
    // Call the base class implementation:
    acVirtualListCtrl::onAboutToShowContextMenu();

    bool isEnabled = false, isChecked = false;

    GT_IF_WITH_ASSERT(_pBreakOnAction != NULL)
    {
        QString itemText;
        onUpdateBreakpOnFunction(isEnabled, isChecked, itemText);
        _pBreakOnAction->setEnabled(isEnabled);
        _pBreakOnAction->setChecked(isChecked);
        _pBreakOnAction->setText(itemText);
    }

    GT_IF_WITH_ASSERT(_pEnableDisaleAllBreakpointsAction != NULL)
    {
        onUpdateEnableAllBreakpoints(isEnabled, isChecked);
        _pEnableDisaleAllBreakpointsAction->setEnabled(isEnabled);
        _pEnableDisaleAllBreakpointsAction->setChecked(isChecked);
    }

    GT_IF_WITH_ASSERT(_pAddRemoveBreakpointsAction != NULL)
    {
        onUpdateAddBreakpoints(isEnabled);
        _pAddRemoveBreakpointsAction->setEnabled(isEnabled);
    }
}


// ---------------------------------------------------------------------------
// Name:        gdAPICallsHistoryView::setCaption
// Description: Set the caption for the view
// Arguments:   const gdDebugApplicationTreeData* pTreeItemData
// Author:      Sigal Algranaty
// Date:        6/3/2012
// ---------------------------------------------------------------------------
void gdAPICallsHistoryView::setCaption()
{
    if (_shouldSetCaption)
    {
        afApplicationCommands* pApplicationCommands = afApplicationCommands::instance();
        GT_IF_WITH_ASSERT(pApplicationCommands != NULL)
        {
            // Build the window caption:
            gtString caption = pApplicationCommands->captionPrefix();

            if (_activeContextId.isDefault())
            {
                caption.append(GD_STR_callsHistoryViewCaptionDefault);
                caption.append(GD_STR_callsHistoryViewCaptionNoContextAddition);
            }
            else
            {
                // Get my display string:
                gtString contextStr;
                _activeContextId.toString(contextStr);
                caption.appendFormattedString(GD_STR_callsHistoryViewCaptionWithContext, contextStr.asCharArray());
            }

            // Set the caption for the statistics view:
            pApplicationCommands->setWindowCaption(this, caption);
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        gdAPICallsHistoryViewModel::gdAPICallsHistoryViewModel
// Description: Constructor
// Arguments:   gdAPICallsHistoryView* pParent
// Author:      Sigal Algranaty
// Date:        27/12/2011
// ---------------------------------------------------------------------------
gdAPICallsHistoryViewModel::gdAPICallsHistoryViewModel(gdAPICallsHistoryView* pParent)
    : QAbstractTableModel(pParent), _pAPICallsHistoryView(pParent)
{

}

// ---------------------------------------------------------------------------
// Name:        gdAPICallsHistoryViewModel::~gdAPICallsHistoryViewModel
// Description: Destructor
// Author:      Sigal Algranaty
// Date:        27/12/2011
// ---------------------------------------------------------------------------
gdAPICallsHistoryViewModel::~gdAPICallsHistoryViewModel()
{

}

// ---------------------------------------------------------------------------
// Name:        gdAPICallsHistoryViewModel::updateModel
// Description: Notify the table that a change was done in the model data
// Return Val:  void
// Author:      Sigal Algranaty
// Date:        27/12/2011
// ---------------------------------------------------------------------------
void gdAPICallsHistoryViewModel::updateModel()
{
    // Sanity check:
    GT_IF_WITH_ASSERT(_pAPICallsHistoryView != NULL)
    {
        // Adapt the model to the new amount of rows in the list -> remove or add according to the change in numbers:
        if (_pAPICallsHistoryView->_previousRowCount > _pAPICallsHistoryView->_amountOfFunctionCalls)
        {
            int firstToRemove = _pAPICallsHistoryView->_amountOfFunctionCalls;
            int lastToRemove = _pAPICallsHistoryView->_previousRowCount - 1;

            beginRemoveRows(QModelIndex(), firstToRemove, lastToRemove);
            endRemoveRows();
        }

        else if (_pAPICallsHistoryView->_amountOfFunctionCalls > _pAPICallsHistoryView->_previousRowCount)
        {
            int firstToAdd = _pAPICallsHistoryView->_previousRowCount;
            int lastToAdd = _pAPICallsHistoryView->_amountOfFunctionCalls - 1;
            beginInsertRows(QModelIndex(), firstToAdd, lastToAdd);
            endInsertRows();
        }

        _pAPICallsHistoryView->_previousRowCount = _pAPICallsHistoryView->_amountOfFunctionCalls;
    }
}

// ---------------------------------------------------------------------------
// Name:        gdAPICallsHistoryViewModel::rowCount
// Description: Return the row count - the current active context functions amount
// Arguments:   const QModelIndex &parent
// Return Val:  int
// Author:      Sigal Algranaty
// Date:        27/12/2011
// ---------------------------------------------------------------------------
int gdAPICallsHistoryViewModel::rowCount(const QModelIndex& parent) const
{
    (void)(parent);  // unused
    int retVal = 0;
    GT_IF_WITH_ASSERT(_pAPICallsHistoryView != NULL)
    {
        retVal = _pAPICallsHistoryView->_amountOfFunctionCalls;
        _pAPICallsHistoryView->_previousRowCount = retVal;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gdAPICallsHistoryViewModel::columnCount
// Description: Return the amount of columns
// Arguments:   const QModelIndex &parent
// Return Val:  int
// Author:      Sigal Algranaty
// Date:        27/12/2011
// ---------------------------------------------------------------------------
int gdAPICallsHistoryViewModel::columnCount(const QModelIndex& parent) const
{
    (void)(parent);  // unused
    return 1;
}

// ---------------------------------------------------------------------------
// Name:        gdAPICallsHistoryViewModel::data
// Description: Return the data for the requested row , column
// Arguments:   const QModelIndex &index
//              int role
// Return Val:  QVariant
// Author:      Sigal Algranaty
// Date:        27/12/2011
// ---------------------------------------------------------------------------
QVariant gdAPICallsHistoryViewModel::data(const QModelIndex& index, int role) const
{
    QVariant retVal;

    // Sanity check
    GT_IF_WITH_ASSERT(_pAPICallsHistoryView != NULL)
    {

        if (index.isValid())
        {
            if ((role == Qt::DisplayRole) || (role == Qt::ToolTipRole))
            {
                bool rc = false;
                gtString outputStr;

                // Only call API calls when debugged process is suspended:
                if (_pAPICallsHistoryView->_isDebuggedProcessSuspended)
                {
                    // Are we in "Profiling/Debugging" mode:
                    if (_pAPICallsHistoryView->_executionMode != AP_PROFILING_MODE)
                    {
                        // Get the appropriate function call:
                        gtAutoPtr<apFunctionCall> aptrFunctionCall;
                        rc = gaGetCurrentFrameFunctionCall(_pAPICallsHistoryView->_activeContextId, index.row(), aptrFunctionCall);

                        if (rc)
                        {
                            // Get the function call as string:
                            // NOTICE: do not use apFunctionCall::asString, since we want to add
                            // logical translation to the function arguments:
                            rc = gdFunctionCallAsString(aptrFunctionCall.pointedObject(), outputStr, _pAPICallsHistoryView->_activeContextId._contextType);
                        }
                    }
                    else
                    {
                        // Profiling mode
                        outputStr = GD_STR_CallsHistoryProfilingMode;

                        rc = true;
                    }

                    // In case of failure - display a failed to update line message:
                    if (!rc)
                    {
                        outputStr = AF_STR_Empty;
                    }
                }

                retVal = acGTStringToQString(outputStr);
            }
            else if (role == Qt::TextAlignmentRole)
            {
                retVal = Qt::AlignLeft + Qt::AlignVCenter;
            }

            else if (role == Qt::DecorationRole)
            {
                // Get the item icon from view:
                QIcon* pIcon = _pAPICallsHistoryView->getItemIcon(index.row());

                if (pIcon != NULL)
                {
                    retVal = *pIcon;
                }
            }

            else if (role == Qt::BackgroundRole)
            {
                // Get the default palette:
                retVal = QBrush(Qt::white);

                if (_pAPICallsHistoryView->shouldHighlightItem(index.row()))
                {
                    // The function should be highlight:
                    retVal = QBrush(acQLIST_HIGHLIGHT_COLOUR);
                }
            }
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gdAPICallsHistoryViewModel::headerData
// Description: Return the header data for the list
// Arguments:   int section
//              Qt::Orientation orientation
//              int role
// Return Val:  QVariant
// Author:      Sigal Algranaty
// Date:        27/12/2011
// ---------------------------------------------------------------------------
QVariant gdAPICallsHistoryViewModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    (void)(section);  // unused
    QVariant retVal;

    if (role == Qt::DisplayRole)
    {
        // Get the column title from the config:
        if (orientation == Qt::Horizontal)
        {
            // Update the Context column with the right context number and number of functions:
            gtString columnTitle;
            GT_IF_WITH_ASSERT(_pAPICallsHistoryView != NULL)
            {
                if (gaIsDebuggedProcessSuspended())
                {
                    columnTitle = GD_STR_CallsHistoryTitleUpdating;

                    if (_pAPICallsHistoryView->m_isDataUpdated)
                    {
                        columnTitle.makeEmpty();
                        gtString amountOfFunctionCallsAsString;
                        amountOfFunctionCallsAsString.appendFormattedString(L"%d", _pAPICallsHistoryView->_amountOfFunctionCalls);
                        amountOfFunctionCallsAsString.addThousandSeperators();

                        // Change the column title according to the context type and number and the amount of calls:
                        if (_pAPICallsHistoryView->_activeContextId.isOpenGLContext())
                        {
                            // OpenGL context:
                            columnTitle.append(amountOfFunctionCallsAsString);
                            columnTitle.append(GD_STR_CallsHistoryOpenGLContextPostfix);
                        }
                        else if (_pAPICallsHistoryView->_activeContextId.isOpenCLContext())
                        {
                            columnTitle.append(amountOfFunctionCallsAsString);
                            columnTitle.append(GD_STR_CallsHistoryOpenCLContextPostfix);
                        }
                        else if (_pAPICallsHistoryView->_activeContextId.isDefault())
                        {
                            columnTitle.append(amountOfFunctionCallsAsString);
                            columnTitle.append(GD_STR_CallsHistoryNULLContextPostfix);
                        }
                        else // contextId
                        {
                            // Should not get here:
                            GT_ASSERT_EX(false, L"Unknown context type");
                        }
                    }
                    else
                    {
                        if (gaDebuggedProcessExists())
                        {
                            columnTitle = GD_STR_CallsHistoryProcessRunning;
                        }
                        else
                        {
                            columnTitle = GD_STR_CallsHistoryTitle;
                        }
                    }
                }

                // Set the return value:
                retVal = acGTStringToQString(columnTitle);
            }
        }
    }
    else if (role == Qt::TextAlignmentRole)
    {
        retVal = Qt::AlignLeft + Qt::AlignVCenter;
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gdAPICallsHistoryViewModel::flags
// Description: Item flags
// Arguments:   const QModelIndex &index
// Return Val:  Qt::ItemFlags
// Author:      Sigal Algranaty
// Date:        27/12/2011
// ---------------------------------------------------------------------------
Qt::ItemFlags gdAPICallsHistoryViewModel::flags(const QModelIndex& index) const
{
    Qt::ItemFlags retVal = Qt::ItemIsEnabled;

    if (index.isValid())
    {
        retVal = QAbstractTableModel::flags(index);
    }

    return retVal;

}

