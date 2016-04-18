//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gdMultiWatchView.cpp
///
//==================================================================================

//------------------------------ gdMultiWatchView.cpp ------------------------------

// Qt:
#include <QtWidgets>

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTBaseTools/Include/gtIgnoreCompilerWarnings.h>
#include <AMDTApiFunctions/Include/gaGRApiFunctions.h>
#include <AMDTAPIClasses/Include/Events/apEventsHandler.h>
#include <AMDTOpenCLServer/Include/csPublicStringConstants.h>
#include <AMDTApplicationComponents/Include/acFunctions.h>

// Local:
#include <AMDTGpuDebuggingComponents/Include/gdAidFunctions.h>
#include <AMDTGpuDebuggingComponents/Include/gdCommandIDs.h>
#include <AMDTGpuDebuggingComponents/Include/views/gdMultiWatchView.h>

// ---------------------------------------------------------------------------
// Name:        gdMultiWatchView::gdMultiWatchView
// Description: Constructor
// Arguments:   pParent - The view's parent
// Author:      Sigal Algranaty
// Date:        24/2/2011
// ---------------------------------------------------------------------------
gdMultiWatchView::gdMultiWatchView(QWidget* pParent, afProgressBarWrapper* pProgressBar)
    : gdImageDataView(pParent, pProgressBar), m_isShown(false), m_isDebuggedProcessSuspended(true),
      m_ignoreWorkItemChangedEvent(false), m_firstTimeImageIsShown(true), m_pExecutionMaskRawFileHandler(NULL),
      m_pCurrentVariableAvailabilityMaskRawFileHandler(NULL)
{
    // Initialize the debugged process flag:
    m_isDebuggedProcessSuspended = gaIsDebuggedProcessSuspended();

    // Register myself to listen to debugged process events:
    apEventsHandler::instance().registerEventsObserver(*this, AP_APPLICATION_COMPONENTS_EVENTS_HANDLING_PRIORITY);

    // Initialize the current work item:
    m_currentWorkItem[0] = 0;
    m_currentWorkItem[1] = 0;
    m_currentWorkItem[2] = 0;
}

// ---------------------------------------------------------------------------
// Name:        gdMultiWatchView::~gdMultiWatchView
// Description: Destructor
// Author:      Sigal Algranaty
// Date:        8/11/2010
// ---------------------------------------------------------------------------
gdMultiWatchView::~gdMultiWatchView()
{
    // Delete my displayed item data if it was allocated:
    if (_pDisplayedItemTreeData != NULL)
    {
        delete _pDisplayedItemTreeData;
    }

    // Unregister myself from listening to debugged process events:
    apEventsHandler::instance().unregisterEventsObserver(*this);
}


// ---------------------------------------------------------------------------
// Name:        gdMultiWatchView::initialize
// Description:
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        2/3/2011
// ---------------------------------------------------------------------------
void gdMultiWatchView::initialize(QSize& viewSize)
{
    // Set the view size:
    resize(viewSize);

    if (_pDisplayedItemTreeData == NULL)
    {
        // Create a displayed item data:
        _pDisplayedItemTreeData = new afApplicationTreeItemData;


        _pDisplayedItemTreeData->setExtendedData(new gdDebugApplicationTreeData);


        _pDisplayedItemTreeData->m_itemType = AF_TREE_ITEM_CL_KERNEL_VARIABLE;

        _pDisplayedItemTreeData->_itemLoadStatus._loadStatusDescription = AF_ITEM_LOAD_EMPTY_VARIABLE_NAME;
        _pDisplayedItemTreeData->_itemLoadStatus._itemLoadStatusType = AF_ITEM_NOT_LOADED;
    }

    // Set the view frame layout:
    setFrameLayout(viewSize);

    // Sanity check:
    GT_IF_WITH_ASSERT(_pImageControlPanel != NULL)
    {
        // Check if we're in kernel debugging mode:
        bool isInKernelDebugging = gaIsInKernelDebugging();

        // Initialize the variable names combo box:
        _pImageControlPanel->initializeVariableNamesCombo(isInKernelDebugging);
        _pDisplayedItemTreeData->_itemLoadStatus._loadStatusDescription = isInKernelDebugging ? AF_ITEM_LOAD_EMPTY_VARIABLE_NAME : AF_ITEM_LOAD_NOT_IN_KERNEL_DEBUGGING;
    }

    GT_IF_WITH_ASSERT(_pImageViewManager != NULL)
    {
        // First display only text:
        _pImageViewManager->setManagerMode(AC_MANAGER_MODE_TEXT_ITEM);
    }

    // Display an empty variable:
    displaySelectedVariable();

    // Clear the view:
    clearView();

    // Connect the control panel multi watch variable combo to the slot:
    bool rc = connect(_pImageControlPanel->m_pMultiWatchVariableNameCombo, SIGNAL(currentIndexChanged(const QString&)), this, SLOT(onVariableNameComboSelected(const QString&)));
    GT_ASSERT(rc);

}
// ---------------------------------------------------------------------------
// Name:        gdMultiWatchView::onEvent
// Description: Is called when a debugged process event occurs.
//              We use the events for updating the variables iff we're in debugged process
//              suspension & within kernel debugging session.
// Arguments:   eve - The debugged process event.
// Author:      Sigal Algranaty
// Date:        27/2/2011
// ---------------------------------------------------------------------------
void gdMultiWatchView::onEvent(const apEvent& eve, bool& vetoEvent)
{
    (void)(vetoEvent);  // unused

    // Get the event type:
    apEvent::EventType eventType = eve.eventType();

    // Handle the event according to its type:
    switch (eventType)
    {

        case apEvent::AP_DEBUGGED_PROCESS_RUN_STARTED:
        case apEvent::AP_DEBUGGED_PROCESS_RUN_RESUMED:
        case apEvent::AP_DEBUGGED_PROCESS_TERMINATED:
        {
            // Go out from kernel debugging mode:
            m_isDebuggedProcessSuspended = false;

            // Clear the view:
            clearView();
        }
        break;

        case apEvent::AP_DEBUGGED_PROCESS_RUN_SUSPENDED:
        {
            // Set the debugged process flag:
            m_isDebuggedProcessSuspended = true;

            // Reset the flag that handles the bset fit:
            m_firstTimeImageIsShown = true;

            // Check if we're in kernel debugging mode:
            bool isInKernelDebugging = gaIsInKernelDebugging();

            // Sanity check:
            GT_IF_WITH_ASSERT(_pImageControlPanel != NULL)
            {
                // Initialize the variable names combo box:
                _pImageControlPanel->initializeVariableNamesCombo(isInKernelDebugging);
                _pDisplayedItemTreeData->_itemLoadStatus._loadStatusDescription = isInKernelDebugging ? AF_ITEM_LOAD_EMPTY_VARIABLE_NAME : AF_ITEM_LOAD_NOT_IN_KERNEL_DEBUGGING;
            }

            if (isInKernelDebugging)
            {
                // Update the variable display on each suspension in kernel debugging:
                updateVariableDisplay();
            }
        }
        break;

        case apEvent::AP_KERNEL_CURRENT_WORK_ITEM_CHANGED_EVENT:
        {
            if (!m_ignoreWorkItemChangedEvent)
            {
                // Update the current work item:
                updateCurrentWorkItem();
            }
        }
        break;

        default:
            // Do nothing...
            break;
    }
}


// ---------------------------------------------------------------------------
// Name:        gdMultiWatchView::clearView
// Description: Clear my components according to my current debugged execution
//              state
// Author:      Sigal Algranaty
// Date:        13/3/2011
// ---------------------------------------------------------------------------
void gdMultiWatchView::clearView()
{
    // Call my base class implementation:
    gdImageDataView::clearView();

    // Check if we're in kernel debugging mode:
    bool isInKernelDebugging = gaIsInKernelDebugging();

    GT_IF_WITH_ASSERT(_pImageControlPanel != NULL)
    {
        if (_pDisplayedItemTreeData != NULL)
        {
            // Check the current debug situation, and do not load the variable if it is not possible:
            if (!isInKernelDebugging)
            {
                _pDisplayedItemTreeData->setItemLoadStatus(AF_ITEM_LOAD_ERROR, AF_ITEM_LOAD_NOT_IN_KERNEL_DEBUGGING);
            }
            else if (!m_isDebuggedProcessSuspended)
            {
                _pDisplayedItemTreeData->setItemLoadStatus(AF_ITEM_LOAD_ERROR, AF_ITEM_LOAD_PROCESS_IS_RUNNING);
            }
        }

        if (!isInKernelDebugging || !m_isDebuggedProcessSuspended)
        {
            // Disable the variables combo box:
            _pImageControlPanel->enableMultiwatchControls(false);
        }

        // Setup the control panel with the loaded item attributes:
        setupControlPanel();

        // Adjust the item after load:
        adjustViewerAfterItemLoading();

        // Adjust the manager layout after adding the texture
        forceImageManagerRepaint();

        // Apply the actions for the displayed item:
        applyLastViewedItemProperties(_lastViewedItemProperties, false);

        // Write the failure message:
        displayItemMessageIfNeeded();

        // Clear the pixel information:
        _pImageControlPanel->clearPixelInformation(true);

        layout()->activate();
    }
}

// ---------------------------------------------------------------------------
// Name:        gdMultiWatchView::updateVariableDisplay
// Description: Updates the currently selected variable display
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        1/3/2011
// ---------------------------------------------------------------------------
bool gdMultiWatchView::updateVariableDisplay()
{
    bool retVal = false;

    // Sanity check:
    GT_IF_WITH_ASSERT((_pImageControlPanel != NULL) && (_pImageViewManager != NULL))
    {
        // Clear objects:
        _pImageViewManager->setManagerMode(AC_MANAGER_MODE_STANDARD_ITEM);

        // Clear the view:
        clearView();

        // Display the variable name:
        bool rcDisplaySelected = displaySelectedVariable();
        GT_ASSERT(rcDisplaySelected);

        // Select and focus the current work item:
        updateCurrentWorkItem();

        if (_currentViewsDisplayProperties._lastSelectedPageIndex == 0)
        {
            // Get the page and repaint it:
            GT_IF_WITH_ASSERT(_pImageViewManager != NULL)
            {
                // Repaint the
                applyBestFit(_currentZoomLevel);
                _pImageViewManager->forceImagesRepaint();
            }
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gdMultiWatchView::displayVariable
// Description: Display the requested variable
// Arguments:   const gtString& variableName
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        3/3/2011
// ---------------------------------------------------------------------------
bool gdMultiWatchView::displayVariable(const gtString& variableName)
{
    bool retVal = false;
    // Sanity check:
    GT_IF_WITH_ASSERT((_pImageControlPanel != NULL) && (_pDisplayedItemTreeData != NULL))
    {
        // Check if we're in kernel debugging mode:
        bool isInKernelDebugging = gaIsInKernelDebugging();

        // Set the variable name:
        _pImageControlPanel->initializeVariableNamesCombo(isInKernelDebugging);
        _pDisplayedItemTreeData->_itemLoadStatus._loadStatusDescription = isInKernelDebugging ? AF_ITEM_LOAD_EMPTY_VARIABLE_NAME : AF_ITEM_LOAD_NOT_IN_KERNEL_DEBUGGING;

        // Select the requested variable:
        bool isVariableExist = false;
        bool rcSelect = _pImageControlPanel->selectMultiWatchVariableName(variableName, isVariableExist);

        if (rcSelect && isVariableExist)
        {
            // Display the variable:
            displaySelectedVariable();
        }
        else if (!isVariableExist && (!variableName.isEmpty()))
        {
            _pDisplayedItemTreeData->setItemLoadStatus(AF_ITEM_LOAD_ERROR, AF_ITEM_LOAD_VARIABLE_NOT_EXIST);

            displayItemMessageIfNeeded();
        }
    }
    return retVal;
}
// ---------------------------------------------------------------------------
// Name:        gdMultiWatchView::displaySelectedVariable
// Description:
// Arguments:   const gtString& variableName
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        28/2/2011
// ---------------------------------------------------------------------------
bool gdMultiWatchView::displaySelectedVariable()
{
    bool retVal = true;

    // Sanity check:
    GT_IF_WITH_ASSERT((_pImageControlPanel != NULL) && (_pDisplayedItemTreeData != NULL))
    {
        // Check if we're in kernel debugging mode:
        bool isInKernelDebugging = gaIsInKernelDebugging();

        // Check the current debug situation, and do not load the variable if it is not possible:
        if (!isInKernelDebugging)
        {
            _pDisplayedItemTreeData->setItemLoadStatus(AF_ITEM_NOT_LOADED, AF_ITEM_LOAD_NOT_IN_KERNEL_DEBUGGING);
        }
        else if (!m_isDebuggedProcessSuspended)
        {
            _pDisplayedItemTreeData->setItemLoadStatus(AF_ITEM_NOT_LOADED, AF_ITEM_LOAD_PROCESS_IS_RUNNING);
        }
        else
        {
            gdDebugApplicationTreeData* pGDData = qobject_cast<gdDebugApplicationTreeData*>(_pDisplayedItemTreeData->extendedItemData());
            GT_IF_WITH_ASSERT(pGDData != NULL)
            {
                // Get the currently selected variable name:
                QString selectedVariableName = _pImageControlPanel->selectedMultiWatchVariableName();

                if (!selectedVariableName.isEmpty())
                {
                    // Read the execution mask:
                    updateCurrentExecutionMask();
                    // Do not perform assertion here, since we get errors while in kernel initialization:

                    // Load the availability mask for this variable:
                    updateVariableAvailabilityMask(selectedVariableName);
                    // Do not assert the return value, since we do not yet know if the variable is supported anywhere

                    // Setup the item type:
                    _pDisplayedItemTreeData->m_itemType = AF_TREE_ITEM_CL_KERNEL_VARIABLE;
                    pGDData->_multiVariableName = acQStringToGTString(selectedVariableName);

                    // Clear the view:
                    clearView();

                    // Load the variable into the data and image views (for the Z coordinate):
                    bool rcLoad = loadVariable(pGDData->_multiVariableName);

                    if (!rcLoad)
                    {
                        // Display the item as text message if needed:
                        _pDisplayedItemTreeData->setItemLoadStatus(AF_ITEM_LOAD_ERROR, AF_ITEM_LOAD_VARIABLE_LOAD_FAILURE);
                    }

                    // Setup the control panel with the loaded item attributes:
                    setupControlPanel();

                    // Apply best fit for the current mip level:
                    applyBestFit(_currentZoomLevel);

                    // Set the combo zoom level:
                    setTextureManagerZoomLevel(_currentZoomLevel);

                    // Save the last viewed item properties:
                    saveLastViewedItemProperties(_currentZoomLevel);

                    // Adjust the item after load:
                    adjustViewerAfterItemLoading();

                    // Adjust the manager layout after adding the texture
                    forceImageManagerRepaint();

                    // Apply the actions for the displayed item:
                    applyLastViewedItemProperties(_lastViewedItemProperties, false);

                    // Update work item selection:
                    updateCurrentWorkItem();

                    // Save the last viewed item properties:
                    saveLastViewedItemProperties(_currentZoomLevel);
                }

                // Write the failure message:
                displayItemMessageIfNeeded();
            }
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gdMultiWatchView::onVariableNameComboSelected
// Description: Handle the selection change of the variable name combo box
// Author:      Sigal Algranaty
// Date:        28/2/2011
// ---------------------------------------------------------------------------
void gdMultiWatchView::onVariableNameComboSelected(const QString& selectedText)
{
    (void)(selectedText);  // unused
    // Sanity check:
    GT_IF_WITH_ASSERT(_pImageControlPanel != NULL)
    {
        // Check if we're in kernel debugging mode:
        bool isInKernelDebugging = gaIsInKernelDebugging();

        // We should not get here not in kernel debugging mode:
        if (isInKernelDebugging && m_isDebuggedProcessSuspended)
        {
            // Get the currently selected variable index:
            int currentSelectedItem = _pImageControlPanel->selectedMultiWatchVariableIndex();

            // Display the variable name:
            updateVariableDisplay();

            // Set the focus for the combo box:
            _pImageControlPanel->setComboBoxFocus(currentSelectedItem);

            // Select and focus the current work item:
            updateCurrentWorkItem();
        }
    }
}


// ---------------------------------------------------------------------------
// Name:        gdMultiWatchView::loadVariable
// Description: Loads the variable with the requested name to the view
// Arguments:   const gtString& variableName
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        28/2/2011
// ---------------------------------------------------------------------------
bool gdMultiWatchView::loadVariable(const gtString& variableName)
{
    bool retVal = false;

    // Update the variable name:
    osFilePath variableFile;
    bool variableTypeSupported = true;
    bool rcUpdateVar = gaUpdateKernelVariableValueRawData(variableName, variableTypeSupported, variableFile);

    if (rcUpdateVar)
    {
        if (variableTypeSupported)
        {
            // Load the variable file:
            retVal = loadVariableFile(variableName, variableFile);

            if (retVal)
            {
                _pDisplayedItemTreeData->setItemLoadStatus(AF_ITEM_LOAD_SUCCESS);
            }
        }
        else
        {
            // Make sure that the appropriate message is displayed:
            retVal = true;
            _pDisplayedItemTreeData->setItemLoadStatus(AF_ITEM_LOAD_ERROR, AF_ITEM_LOAD_VARIABLE_UNSUPPORTED_TYPE);
        }
    }

    if (!retVal)
    {
        _pDisplayedItemTreeData->setItemLoadStatus(AF_ITEM_LOAD_ERROR, AF_ITEM_LOAD_VARIABLE_LOAD_FAILURE);
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gdMultiWatchView::loadVariableFile
// Description: Load the variable file into the views
// Arguments:   const osFilePath& variableFile
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        28/2/2011
// ---------------------------------------------------------------------------
bool gdMultiWatchView::loadVariableFile(const gtString& variableName, const osFilePath& variableFile)
{
    bool retVal = true;

    // Sanity check:
    GT_IF_WITH_ASSERT(_pDisplayedItemTreeData != NULL)
    {
        // Load the raw data into the images and buffer viewer.
        acRawFileHandler* pRawFileHandler = new acRawFileHandler(false);


        // Load raw data from file
        bool rc1 = pRawFileHandler->loadFromFile(variableFile);
        GT_IF_WITH_ASSERT(rc1)
        {
            // If raw data was loaded successfully
            GT_IF_WITH_ASSERT(pRawFileHandler->isOk())
            {
                // Clear the images manager:
                GT_IF_WITH_ASSERT(_pImageViewManager != NULL)
                {
                    _pImageViewManager->setManagerMode(AC_MANAGER_MODE_STANDARD_ITEM);
                    _pImageViewManager->clearAllObjects();
                }

                // Load the texture to image data views
                retVal = loadItemToImageAndDataViews(pRawFileHandler, variableName);

                // Update the current work item:
                bool rcWorkItem = updateCurrentWorkItem();
                GT_ASSERT(rcWorkItem);
            }
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gdMultiWatchView::updateCurrentWorkItem
// Description: Update the view with the current work item
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        13/3/2011
// ---------------------------------------------------------------------------
bool gdMultiWatchView::updateCurrentWorkItem()
{
    bool retVal = false;

    // Sanity check:
    GT_IF_WITH_ASSERT(_pImageControlPanel != NULL)
    {
        // Update the current work item in the control panel:
        retVal = _pImageControlPanel->updateCurrentWorkItem();
    }
    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gdMultiWatchView::updateCurrentExecutionMask
// Description:
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        9/3/2011
// ---------------------------------------------------------------------------
bool gdMultiWatchView::updateCurrentExecutionMask()
{
    bool retVal = false;

    // Update the execution mask variable data:
    osFilePath execMaskFilePath;
    bool variableTypeSupported = true;
    bool rcUpdate = gaUpdateKernelVariableValueRawData(CS_STR_KernelDebuggingExecutionMaskPseudoVariableName, variableTypeSupported, execMaskFilePath);

    if (rcUpdate)
    {
        if (m_pExecutionMaskRawFileHandler != NULL)
        {
            delete m_pExecutionMaskRawFileHandler;
        }

        // Allocate a new execution mask raw file handler:
        m_pExecutionMaskRawFileHandler = new acRawFileHandler;


        // Load raw data from file:
        bool rc1 = m_pExecutionMaskRawFileHandler->loadFromFile(execMaskFilePath);
        GT_IF_WITH_ASSERT(rc1)
        {
            retVal = true;
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gdMultiWatchView::updateVariableAvailabilityMask
// Description: Update the availability mask for the selected variable
// Return Val:  bool - Success / failure.
// Author:      Uri Shomroni
// Date:        2/5/2013
// ---------------------------------------------------------------------------
bool gdMultiWatchView::updateVariableAvailabilityMask(const QString& varName)
{
    bool retVal = false;

    // Update the execution mask variable data:
    osFilePath availMaskFilePath;
    bool variableTypeSupported = true;
    gtString pseudoVarName = CS_STR_KernelDebuggingAvailabilityMaskPseudoVariableNamePrefix;
    pseudoVarName.append(acQStringToGTString(varName));
    bool rcUpdate = gaUpdateKernelVariableValueRawData(pseudoVarName, variableTypeSupported, availMaskFilePath);

    if (rcUpdate)
    {
        if (m_pCurrentVariableAvailabilityMaskRawFileHandler != NULL)
        {
            delete m_pCurrentVariableAvailabilityMaskRawFileHandler;
        }

        // Allocate a new execution mask raw file handler:
        m_pCurrentVariableAvailabilityMaskRawFileHandler = new acRawFileHandler;


        // Load raw data from file:
        bool rc1 = m_pCurrentVariableAvailabilityMaskRawFileHandler->loadFromFile(availMaskFilePath);
        GT_IF_WITH_ASSERT(rc1)
        {
            retVal = true;
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gdMultiWatchView::getFilterRawDataHandler
// Description: Return the raw file handler filtering the current displayed variable
// Return Val:  acRawFileHandler*
// Author:      Sigal Algranaty
// Date:        9/3/2011
// ---------------------------------------------------------------------------
acRawFileHandler* gdMultiWatchView::getCurrentFilterRawDataHandler(unsigned int index)
{
    acRawFileHandler* pRetVal = NULL;

    // Sanity check:
    GT_IF_WITH_ASSERT(_pImageControlPanel != NULL)
    {
        // Get the current displayed variable name:
        gtString variableName = acQStringToGTString(_pImageControlPanel->selectedMultiWatchVariableName());
        static const gtString availabilityMaskPrefix = CS_STR_KernelDebuggingAvailabilityMaskPseudoVariableNamePrefix;

        if ((variableName != CS_STR_KernelDebuggingExecutionMaskPseudoVariableName) && (variableName != CS_STR_KernelDebuggingWavefrontMaskPseudoVariableName) && (!variableName.startsWith(availabilityMaskPrefix)))
        {
            switch (index)
            {
                case 0:
                    pRetVal = m_pExecutionMaskRawFileHandler;
                    break;

                case 1:
                    pRetVal = m_pCurrentVariableAvailabilityMaskRawFileHandler;
                    break;

                default:
                    // return NULL
                    break;
            }
        }
    }

    return pRetVal;
}


// ---------------------------------------------------------------------------
// Name:        gdImageDataView::handleDataCellDoubleClick
// Description: Nothing to do for base class
// Arguments:    int canvasId
//              acImageItem* pImageItem
//              QPoint posOnImage
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        15/3/2011
// ---------------------------------------------------------------------------
bool gdMultiWatchView::handleDataCellDoubleClick(int canvasId, acImageItem* pImageItem, QPoint posOnImage)
{
    (void)(canvasId);  // unused
    (void)(pImageItem);  // unused
    bool retVal = false;

    // Check if we're in kernel debugging mode:
    bool isInKernelDebugging = gaIsInKernelDebugging();

    // Sanity check:
    GT_IF_WITH_ASSERT(isInKernelDebugging && m_isDebuggedProcessSuspended)
    {
        // Ignore the work item changed event:
        m_ignoreWorkItemChangedEvent = true;

        // Get the kernel global work offset:
        int xOffset = 0, yOffset = 0, zOffset = 0;
        gaGetKernelDebuggingGlobalWorkOffset(xOffset, yOffset, zOffset);

        // Get the kernel debugging amount of dimensions:
        int x = 0, y = 0, z = 0, amountOfDimensions = 0;
        gaGetKernelDebuggingLocalWorkSize(x, y, z, amountOfDimensions);

        // Add the X offset to the selected cell x coordinate:
        int xNewCoord = posOnImage.x() + xOffset;

        // Try to set the new coordinate value:
        retVal = gaSetKernelDebuggingCurrentWorkItemCoordinate(0, xNewCoord);
        GT_ASSERT(retVal);

        // If the Y coordinate is relevant:
        if (amountOfDimensions > 1)
        {
            // Add the X offset to the selected cell x coordinate:
            int yNewCoord = posOnImage.y() + yOffset;

            // Set the Y dimension:
            retVal = retVal && gaSetKernelDebuggingCurrentWorkItemCoordinate(1, yNewCoord);
            GT_ASSERT(retVal);
        }

        // Cancel the ignore of the work item changed event:
        m_ignoreWorkItemChangedEvent = false;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gdMultiWatchView::onNoteBookPageChange
// Description: Is handling notebook page change
// Arguments:   int currentPage
// Author:      Sigal Algranaty
// Date:        2/8/2012
// ---------------------------------------------------------------------------
void gdMultiWatchView::onNoteBookPageChange(int currentPage)
{
    // Call the base class implementation:
    gdImageDataView::onNoteBookPageChange(currentPage);

    if ((currentPage == 1) && m_firstTimeImageIsShown)
    {
        m_firstTimeImageIsShown = false;

        GT_IF_WITH_ASSERT(_pImageViewManager != NULL)
        {
            applyBestFit(_currentZoomLevel);
            _pImageViewManager->forceImagesRepaint();
        }
    }
}

