//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gwKernelWorkItemToolbar.cpp
///
//==================================================================================

//------------------------------ gwKernelWorkItemToolbar.cpp ------------------------------

// Qt:
#include <QtWidgets>
#include <QComboBox>
#include <QLabel>
#include <QKeyEvent>

// Infra:
#include <AMDTBaseTools/Include/gtASCIIString.h>
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTAPIClasses/Include/Events/apAfterKernelDebuggingEvent.h>
#include <AMDTAPIClasses/Include/Events/apBeforeKernelDebuggingEvent.h>
#include <AMDTAPIClasses/Include/Events/apEventsHandler.h>
#include <AMDTAPIClasses/Include/Events/apKernelWorkItemChangedEvent.h>
#include <AMDTApplicationComponents/Include/acDefinitions.h>
#include <AMDTApplicationComponents/Include/acDisplay.h>
#include <AMDTApplicationFramework/Include/afAppStringConstants.h>
#include <AMDTApiFunctions/Include/gaGRApiFunctions.h>
#include <AMDTGpuDebuggingComponents/Include/gdGDebuggerGlobalVariablesManager.h>

// Local:
#include <AMDTGpuDebugging/Include/gwKernelWorkItemToolbar.h>
#include <AMDTGpuDebugging/Include/gwStringConstants.h>

#define GW_WORK_ITEM_COMBO_MAX_ITEMS 8
#define GW_THREADS_COMBO_BOX_WIDTH_CHARS 30
#define GW_WORK_ITEM_COMBO_BOX_WIDTH_CHARS 8


// ---------------------------------------------------------------------------
// Name:        gwKernelWorkItemToolbar::gwKernelWorkItemToolbar
// Description: Constructor
// Arguments:   QWidget* pParent
// Author:      Sigal Algranaty
// Date:        15/9/2011
// ---------------------------------------------------------------------------
gwKernelWorkItemToolbar::gwKernelWorkItemToolbar(QWidget* pParent) :
    acToolBar(pParent, GW_STR_kernelWorkItemsToolbarName), m_shouldRebuildWICombos(false), m_shouldRebuildThreadsCombo(false), m_pThreadsCombobox(NULL), m_threadsComboSeparatorIndex(-1),
    m_selectedIndexComboA(0), m_selectedIndexComboB(0), m_selectedIndexComboC(0)
{
    // Create the toolbar object name:
    QString toolBarQtName(GW_STR_kernelWorkItemsToolbarName);
    setObjectName(toolBarQtName);

    // Set the labels values:
    _wiLabelStrings[0] = GW_STR_kernelWorkItemsXCaption;
    _wiLabelStrings[1] = GW_STR_kernelWorkItemsYCaption;
    _wiLabelStrings[2] = GW_STR_kernelWorkItemsZCaption;

    // Create the widgets:
    m_pThreadsCombobox = new QComboBox(this);
    unsigned int minWidth = acScalePixelSizeToDisplayDPI(AC_DEFAULT_TEXT_AVERAGE_CHAR_WIDTH * GW_THREADS_COMBO_BOX_WIDTH_CHARS);
    m_pThreadsCombobox->setMinimumWidth((int)minWidth);
    addWidget(m_pThreadsCombobox);
    bool rcThdCnc = connect(m_pThreadsCombobox, SIGNAL(currentIndexChanged(int)), this, SLOT(comboSelectionChange(int)));
    GT_ASSERT(rcThdCnc);

    unsigned int wiComboWidth = acScalePixelSizeToDisplayDPI(AC_DEFAULT_TEXT_AVERAGE_CHAR_WIDTH * GW_WORK_ITEM_COMBO_BOX_WIDTH_CHARS);

    for (int i = 0 ; i < 3 ; i++)
    {
        // Initialize a label:
        _pWILabels[i] = new QLabel(this);
        _pWILabels[i]->setMargin(5);

        // Set the text:
        _pWILabels[i]->setText(_wiLabelStrings[i]);

        // Add the static text:
        addWidget(_pWILabels[i]);

        // Create the current combo box:
        _pCoordComboBoxes[i] = new gwWorkItemCombo(this);

        // Make the combo box editable:
        _pCoordComboBoxes[i]->setEditable(true);

        // Initialize the validator for the combo box:
        _pWIComboValidators[i] = new QIntValidator(_pCoordComboBoxes[i]);

        // Set the combo box validator:
        _pCoordComboBoxes[i]->setValidator(_pWIComboValidators[i]);

        // Set the minimum width:
        _pCoordComboBoxes[i]->setMinimumWidth((int)wiComboWidth);

        // Add the combo box:
        addWidget(_pCoordComboBoxes[i]);

        // Connect the combo boxes to a callback:
        bool rcConnect = connect(_pCoordComboBoxes[i], SIGNAL(currentIndexChanged(int)), this, SLOT(comboSelectionChange(int)));
        GT_ASSERT(rcConnect);

        // Register to the combo box's line edit's return press event.
        rcConnect = connect(_pCoordComboBoxes[i]->lineEdit(), SIGNAL(editingFinished()), SLOT(comboLineEditEditingFinishedHandler()));
        GT_ASSERT(rcConnect);

        // Do not allow the user to insert new items.
        _pCoordComboBoxes[i]->setInsertPolicy(QComboBox::NoInsert);
    }

    // Initialize combo box enable with false:
    m_areWIComboBoxesEnabled[0] = false;
    m_areWIComboBoxesEnabled[1] = false;
    m_areWIComboBoxesEnabled[2] = false;

    updateToolbarValues();
}

// ---------------------------------------------------------------------------
// Name:        gwKernelWorkItemToolbar::updateToolbarThreadValues
// Description: Updates the threads combo box
// Author:      Uri Shomroni
// Date:        2/4/2013
// ---------------------------------------------------------------------------
void gwKernelWorkItemToolbar::updateToolbarThreadValues()
{
    GT_IF_WITH_ASSERT(NULL != m_pThreadsCombobox)
    {
        bool isProcessSuspended = gaIsDebuggedProcessSuspended();

        if (isProcessSuspended)
        {
            if (m_shouldRebuildThreadsCombo)
            {
                // Create the thread ids list:
                QStringList threadItems;
                int numberOfActiveThreads = -1;
                bool rcAmt = gaGetAmountOfDebuggedProcessThreads(numberOfActiveThreads);
                GT_IF_WITH_ASSERT(rcAmt)
                {
                    // For each thread:
                    for (int i = 0; i < numberOfActiveThreads; i++)
                    {
                        // Get the thread ID:
                        osThreadId tid = OS_NO_THREAD_ID;
                        bool rcTID = gaGetThreadId(i, tid);
                        GT_ASSERT(rcTID);

                        // Create the thread's combobox string:
                        QString currString;

                        if (0 == i)
                        {
                            currString.sprintf("Main thread (%#06x)", (unsigned int)tid);
                        }
                        else
                        {
                            currString.sprintf("Thread %d (%#06x)", i, (unsigned int)tid);
                        }

                        // Add it to the list:
                        threadItems << currString;
                    }
                }

                // Create the wavefronts list:
                bool isInKernelDebugging = gaIsInKernelDebugging();
                QStringList wavefrontItems;

                if (isInKernelDebugging)
                {
                    int numberOfActiveWavefronts = -1;
                    bool rcWF = gaGetKernelDebuggingAmountOfActiveWavefronts(numberOfActiveWavefronts);
                    GT_IF_WITH_ASSERT((rcWF) && (0 < numberOfActiveWavefronts))
                    {
                        for (int i = -1; i < numberOfActiveWavefronts; i++)
                        {
                            QString wfString;

                            if (-1 < i)
                            {
                                int wavefrontId = -1;
                                bool rcWfID = gaGetKernelDebuggingActiveWavefrontID(i, wavefrontId);

                                if (rcWfID && (-1 < wavefrontId))
                                {
                                    wfString.sprintf("Wavefront %d", wavefrontId);
                                }
                                else
                                {
                                    // Unexpected error:
                                    wfString.sprintf("Unknown wavefront #%d", i);
                                }
                            }
                            else // -1 >= i
                            {
                                wfString = "Inactive work items";
                            }

                            wavefrontItems << wfString;
                        }
                    }
                }

                // Set the combo strings:
                m_pThreadsCombobox->blockSignals(true);

                // If we disabled an item, re-enable it:
                if (-1 < m_threadsComboSeparatorIndex)
                {
                    /*
                    // See http://stackoverflow.com/questions/11439773/disable-item-in-qt-combobox :
                    // Get the index of the value to disable
                    QModelIndex modelIndex = m_pThreadsCombobox->model()->index(m_threadsComboSeparatorIndex, 0);

                    // This is the effective 'enable' flag
                    QVariant v(1 | 32);

                    // Enable the item:
                    m_pThreadsCombobox->model()->setData(modelIndex, v, Qt::UserRole - 1);
                    */

                    // Clear the index:
                    m_threadsComboSeparatorIndex = -1;
                }

                m_pThreadsCombobox->clear();
                m_pThreadsCombobox->addItems(threadItems);

                if (!wavefrontItems.empty())
                {
                    // Add a separator item:
                    m_threadsComboSeparatorIndex = m_pThreadsCombobox->count();
                    QString sep("--------");
                    m_pThreadsCombobox->addItem(sep);

                    /*
                    // Disable the separator:
                    // See http://stackoverflow.com/questions/11439773/disable-item-in-qt-combobox :
                    // Get the index of the value to disable
                    QModelIndex modelIndex = m_pThreadsCombobox->model()->index(m_threadsComboSeparatorIndex, 0);

                    // This is the effective 'enable' flag
                    QVariant v(1 | 32);

                    // Disable the item:
                    m_pThreadsCombobox->model()->setData(modelIndex, v, Qt::UserRole - 1);
                    */

                    // Add the wavefronts:
                    m_pThreadsCombobox->addItems(wavefrontItems);
                }

                m_pThreadsCombobox->blockSignals(false);
                /*/
                if (gaIsHostBreakPoint())
                {
                    int index = -1;
                    GT_IF_WITH_ASSERT(gaGetBreakpointTriggeringThreadIndex(index))
                    {
                        gdGDebuggerGlobalVariablesManager& theGlobalVarsMgr = gdGDebuggerGlobalVariablesManager::instance();
                        theGlobalVarsMgr.setChosenThread(index, false);
                    }
                }
                */

                // Enable the combo:
                m_pThreadsCombobox->setEnabled(true);

                // Clear the update flag only if kernel debugging was handled otherwise the m_threadsComboSeparatorIndex is not handled correctly and another updated is needed:
                if (isInKernelDebugging && -1 != m_threadsComboSeparatorIndex)
                {
                    m_shouldRebuildThreadsCombo = false;
                }
            }

            // Select the active thread or wavefront:
            gdGDebuggerGlobalVariablesManager& theGlobalVarsMgr = gdGDebuggerGlobalVariablesManager::instance();
            bool isWavefrontSelected = theGlobalVarsMgr.isKernelDebuggingThreadChosen();
            int selectedThreadIndex = theGlobalVarsMgr.chosenThread();

            if (!isWavefrontSelected)
            {
                // A thread is selected:
                GT_IF_WITH_ASSERT((0 > m_threadsComboSeparatorIndex) || (selectedThreadIndex < m_threadsComboSeparatorIndex))
                {
                    // Ignore when the selection changes to thread #-1 (such as when an OpenCL context is selected in the tree):
                    if (0 <= selectedThreadIndex)
                    {
                        // Set the selection:
                        m_pThreadsCombobox->blockSignals(true);
                        m_pThreadsCombobox->setCurrentIndex(selectedThreadIndex);
                        m_pThreadsCombobox->blockSignals(false);
                    }
                }
            }
            else // isWavefrontSelected
            {
                GT_IF_WITH_ASSERT(0 < m_threadsComboSeparatorIndex)
                {
                    selectedThreadIndex += (m_threadsComboSeparatorIndex + 1);
                    GT_IF_WITH_ASSERT((m_pThreadsCombobox->count() > selectedThreadIndex) && (selectedThreadIndex > m_threadsComboSeparatorIndex))
                    {
                        // Set the selection:
                        m_pThreadsCombobox->blockSignals(true);
                        m_pThreadsCombobox->setCurrentIndex(selectedThreadIndex);
                        m_pThreadsCombobox->blockSignals(false);
                    }
                }
            }
        }
        else // !isProcessSuspended
        {
            // When we are running or when no process exists, disable the combo and set its contents to "N/A":
            QString naString = AF_STR_NotAvailableA;

            // Set the value:
            m_pThreadsCombobox->blockSignals(true);
            m_pThreadsCombobox->clear();
            m_pThreadsCombobox->addItem(naString);
            m_pThreadsCombobox->blockSignals(false);

            m_pThreadsCombobox->setEnabled(false);
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        gwKernelWorkItemToolbar::updateToolbarWorkItemValues
// Description: Is handling before kernel debugging event
// Arguments:   const apBeforeKernelDebuggingEvent& event
// Author:      Sigal Algranaty
// Date:        15/9/2011
// ---------------------------------------------------------------------------
void gwKernelWorkItemToolbar::updateToolbarWorkItemValues()
{
    // Sanity check:
    GT_IF_WITH_ASSERT((_pCoordComboBoxes[0] != NULL) && (_pCoordComboBoxes[1] != NULL) && (_pCoordComboBoxes[2] != NULL))
    {
        // Only update the toolbar values in kernel debugging:
        bool isInKernelDebugging = gaIsInKernelDebugging();

        if (isInKernelDebugging)
        {
            // Rebuild the kernel WI toolbar:
            if (m_shouldRebuildWICombos)
            {
                rebuildWITooblarValues();
            }

            // Get the indices from the persistent data manager:
            int currentValues[3] = { -1, -1, -1};
            bool isCurrentWorkItemValid = gaGetKernelDebuggingCurrentWorkItem(currentValues[0], currentValues[1], currentValues[2]);

            if (isCurrentWorkItemValid)
            {
                isCurrentWorkItemValid = gaIsWorkItemValid(currentValues);
            }

            // If the current work items are not valid, get the current combo values:
            if (!isCurrentWorkItemValid)
            {
                currentValues[0] = _pCoordComboBoxes[0]->currentText().toInt();
                currentValues[1] = _pCoordComboBoxes[1]->currentText().toInt();
                currentValues[2] = _pCoordComboBoxes[2]->currentText().toInt();
                isCurrentWorkItemValid = gaIsWorkItemValid(currentValues);
            }

            // If even that is bad, try the first valid work item:
            if (!isCurrentWorkItemValid)
            {
                isCurrentWorkItemValid = gaGetFirstValidWorkItem(-1, currentValues);

                if (isCurrentWorkItemValid)
                {
                    isCurrentWorkItemValid = gaIsWorkItemValid(currentValues);
                }
            }

            if (isCurrentWorkItemValid)
            {
                // Get the offset for the work size:
                int globalWorkOffset[3] = {0, 0, 0};
                gaGetKernelDebuggingGlobalWorkOffset(0, globalWorkOffset[0]);
                gaGetKernelDebuggingGlobalWorkOffset(1, globalWorkOffset[1]);
                gaGetKernelDebuggingGlobalWorkOffset(2, globalWorkOffset[2]);

                for (int i = 0; i < 3; i++)
                {
                    // If the user deleted the value within the combo box, refresh it to the selected one:
                    int currentCoordIndex = currentValues[i] - globalWorkOffset[i];
                    int currentComboItemCount = _pCoordComboBoxes[i]->count();

                    if (0 < currentCoordIndex)
                    {
                        _pCoordComboBoxes[i]->blockSignals(true);

                        if (currentComboItemCount > currentCoordIndex)
                        {
                            _pCoordComboBoxes[i]->setCurrentIndex(currentCoordIndex);
                        }
                        else
                        {
                            QString txt;
                            txt.sprintf("%d", currentValues[i]);
                            _pCoordComboBoxes[i]->setEditText(txt);
                        }

                        _pCoordComboBoxes[i]->blockSignals(false);
                        _pCoordComboBoxes[i]->update();
                    }
                }
            }
        }
        else if (gaIsInHSAKernelBreakpoint())
        {
            // In HSA kernel debugging, we only call this function when a breakpoint is hit, so we always rebuild the combo:
            rebuildWITooblarValues();

            // We also always select the first thread / breakpoint location - so we do not need to change the active work item in this case.
        }

        // Set the combo boxes enable / disable state:
        for (int i = 0 ; i < 3; i++)
        {
            _pCoordComboBoxes[i]->setEnabled(m_areWIComboBoxesEnabled[i]);
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        gwKernelWorkItemToolbar::onAfterKernelDebuggingEvent
// Description: Is handling after kernel debugging event
// Arguments:   const apAfterKernelDebuggingEvent& event
// Author:      Sigal Algranaty
// Date:        15/9/2011
// ---------------------------------------------------------------------------
void gwKernelWorkItemToolbar::onAfterKernelDebuggingEvent(const apAfterKernelDebuggingEvent& event)
{
    (void)(event); // unused
    // Disable the WI combos:
    m_areWIComboBoxesEnabled[0] = false;
    m_areWIComboBoxesEnabled[1] = false;
    m_areWIComboBoxesEnabled[2] = false;
    m_selectedIndexComboA = m_selectedIndexComboB = m_selectedIndexComboC = 0;
    m_shouldRebuildWICombos = true;
    updateToolbarWorkItemValues();
}

// ---------------------------------------------------------------------------
// Name:        gwKernelWorkItemToolbar::comboSelectionChange
// Description: Handles the combo boxes selection change
// Arguments:   int selectedItemIndex
// Author:      Sigal Algranaty
// Date:        15/9/2011
// ---------------------------------------------------------------------------
void gwKernelWorkItemToolbar::comboSelectionChange(int selectedItemIndex)
{
    // Down cast the sender to a combo box:
    QComboBox* pComboBox = qobject_cast<QComboBox*>(sender());
    GT_IF_WITH_ASSERT(pComboBox != NULL)
    {
        // If this is the threads combo:
        if (m_pThreadsCombobox == pComboBox)
        {
            // What item was selected?
            gdGDebuggerGlobalVariablesManager& theGlobalVarsMgr = gdGDebuggerGlobalVariablesManager::instance();

            if ((0 >= m_threadsComboSeparatorIndex) || (m_threadsComboSeparatorIndex > selectedItemIndex))
            {
                // This is a thread, select it:
                theGlobalVarsMgr.setChosenThread(selectedItemIndex, false);
            }
            else if (m_threadsComboSeparatorIndex < selectedItemIndex) // && 0 < m_threadsComboSeparatorIndex
            {
                // This is a wavefront, select it:
                theGlobalVarsMgr.setChosenThread(selectedItemIndex - (m_threadsComboSeparatorIndex + 1), true);
            }
            else // 0 < m_threadsComboSeparatorIndex == selectedItemIndex
            {
                // This should not happen!
                GT_ASSERT(false);
            }
        }
        else
        {
            // This is one of the WI combos, find the combo box coordinate:
            int coordinate = -1;

            for (int i = 0; i < 3; i++)
            {
                if (pComboBox == _pCoordComboBoxes[i])
                {
                    coordinate = i;
                    break;
                }
            }

            GT_IF_WITH_ASSERT((coordinate >= 0) && (coordinate < 3))
            {
                if (gaIsInKernelDebugging())
                {
                    // Get the string of the selected item:
                    gtASCIIString selectedItemText(pComboBox->itemText(selectedItemIndex).toLatin1().data());

                    if (!selectedItemText.isEmpty())
                    {
                        // Get the string as int:
                        int selectedValue = -1;
                        bool rc = selectedItemText.toIntNumber(selectedValue);
                        GT_IF_WITH_ASSERT(rc)
                        {
                            // Set the kernel work item:
                            rc = gaSetKernelDebuggingCurrentWorkItemCoordinate(coordinate, selectedValue);
                            GT_ASSERT(rc);

                            // Update the cache with the successful selection.
                            updateComboSelectionCache(coordinate, selectedValue);
                        }
                        else
                        {
                            // Fetch the former (current) index.
                            int currentIndex = fetchComboSelectionFromCache(coordinate);

                            // Updating the selection is only relevant if the item is available
                            // (that is, the item was not optimized away from the combo's item list).
                            pComboBox->blockSignals(true);

                            if (currentIndex < GW_WORK_ITEM_COMBO_MAX_ITEMS)
                            {
                                // Update the selected index.
                                pComboBox->setCurrentIndex(currentIndex);
                            }
                            else
                            {
                                // Update the text.
                                QString txt;
                                txt.sprintf("%d", currentIndex);
                                pComboBox->setEditText(txt);
                            }

                            pComboBox->blockSignals(false);
                            pComboBox->update();
                        }
                    }
                }
                else if (gaIsInHSAKernelBreakpoint())
                {
                    GT_IF_WITH_ASSERT(0 <= selectedItemIndex)
                    {
                        // Simply set the thread Index:
                        bool rcHSAWI = gaHSASetActiveWorkItemIndex(selectedItemIndex);
                        GT_ASSERT(rcHSAWI);
                    }
                }
            }
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        gwKernelWorkItemToolbar::populateComboBox
// Description: Helper routine that inserts items into a Work Items combo box.
// Arguments:   QComboBox* pComboBox - the combo box to be populated
//              QIntValidator* pValidator - the validator corresponding to pComboBox
//              int comboIndex - the index (0, 1 or 2) of the combo box, identifies the combo box
//              bool& isEnabledBuffer -this flag will be set with a value indicating whether the combo box should be enabled or not
//              int currBoxCount - the value of the item with the currently largest value in the combo box
//              int currBoxOffset- the value of the item with the currently smallest value in the combo box
// Author:      Amit Ben-Moshe
// Date:        28/10/2013
// ---------------------------------------------------------------------------
void gwKernelWorkItemToolbar::populateComboBox(QComboBox* pComboBox, QIntValidator* pValidator,
                                               bool& isEnabledBuffer, int currBoxCount, int currBoxOffset)
{
    // First, clear the items.
    pComboBox->clear();

    // Define the upper-lower bounds of the combo work items.
    const int coordHighBound = currBoxOffset + currBoxCount;

    // Calculate the number of items that should be inside the combo box.
    const int numOfItems = (currBoxCount - currBoxOffset + 1);

    // Set the combo box enabled/disabled flag.
    isEnabledBuffer = (numOfItems > 1);

    // If the item count is larger than POPULATION_THRESHOLD, we do not populate the combo box.
    if (isEnabledBuffer)
    {
        bool isInThreshold = (numOfItems <= GW_WORK_ITEM_COMBO_MAX_ITEMS);
        const int uppermostItem = isInThreshold ? coordHighBound : GW_WORK_ITEM_COMBO_MAX_ITEMS;

        // Where our items will be held.
        QStringList itemsStrings;

        if ((uppermostItem - currBoxOffset) > 0)
        {
            // Even if the box is disabled, show the "0" value.
            for (int i = currBoxOffset; (i < uppermostItem) || (i == currBoxOffset); i++)
            {
                // Create the current zoom coordinate string.
                QString currentCoord;
                currentCoord.sprintf("%d", i);
                itemsStrings << currentCoord;
            }

            if (!isInThreshold)
            {
                // Insert the last elements.
                itemsStrings << "...";
                QString lastItem;
                lastItem.sprintf("Type a number (%d - %d)", currBoxOffset, coordHighBound - 1);
                itemsStrings << lastItem;
            }
        }

        // Check if this coordinate should be visible.
        const bool isCoordVisible = !itemsStrings.isEmpty();

        if (isCoordVisible)
        {
            // Add the items.
            pComboBox->blockSignals(true);
            pComboBox->addItems(itemsStrings);
            const int longestItemSize = itemsStrings.last().size();
            pComboBox->view()->setMinimumWidth(longestItemSize * 8);
            pComboBox->blockSignals(false);
        }

        // Update the validator.
        GT_IF_WITH_ASSERT(pValidator != NULL)
        {
            pValidator->setRange(currBoxOffset, coordHighBound - 1);
        }
    }

    // Build a tooltip for the current coordinate range:
    QString range;

    if (coordHighBound >= 2)
    {
        range.sprintf("Enter a valid Work Item index between %d and %d", currBoxOffset, coordHighBound - 1);
    }

    pComboBox->setToolTip(range);
}

/// -----------------------------------------------------------------------------------------------
/// \brief Name:        rebuildWITooblarValues
/// \brief Description:
/// \return void
/// -----------------------------------------------------------------------------------------------
void gwKernelWorkItemToolbar::rebuildWITooblarValues()
{
    for (int i = 0; i < 3; i++)
    {
        GT_IF_WITH_ASSERT(NULL != _pCoordComboBoxes[i])
        {
            // OpenCL values:
            if (gaIsInKernelDebugging())
            {
                // Get the updated count values.
                int currBoxCount = 0;
                gaGetKernelDebuggingGlobalWorkSize(i, currBoxCount);

                // Get the updated offset values.
                int currBoxOffset = 0;
                gaGetKernelDebuggingGlobalWorkOffset(i, currBoxOffset);

                // Fill the items if needed.
                populateComboBox(_pCoordComboBoxes[i], _pWIComboValidators[i], m_areWIComboBoxesEnabled[i], currBoxCount, currBoxOffset);

                // Select the current item:
                int currIndex = -1;
                bool rcCurr = gaGetKernelDebuggingCurrentWorkItemCoordinate(i, currIndex);

                if (rcCurr && (-1 < currIndex) && (currBoxOffset <= currIndex) && (currBoxOffset + currBoxCount > currIndex))
                {
                    int boxIndex = currIndex - currBoxOffset;
                    _pCoordComboBoxes[i]->blockSignals(true);

                    if (boxIndex < GW_WORK_ITEM_COMBO_MAX_ITEMS)
                    {
                        _pCoordComboBoxes[i]->setCurrentIndex(boxIndex);
                    }
                    else
                    {
                        // Update the text.
                        QString txt;
                        txt.sprintf("%d", currIndex);
                        _pCoordComboBoxes[i]->setEditText(txt);
                    }

                    _pCoordComboBoxes[i]->blockSignals(false);
                    _pCoordComboBoxes[i]->update();
                }
            }
            else if (gaIsInHSAKernelBreakpoint())
            {
                // HSA values:
                bool updatedCombo = false;

                // Only show one combo box:
                if (0 == i)
                {
                    gtUByte dims = 0;
                    bool rcDims = gaHSAGetWorkDims(dims);

                    GT_IF_WITH_ASSERT(rcDims && (0 < dims) && (4 > dims))
                    {
                        gtVector<gtUInt32> gidLidWgid;
                        bool rcWI = gaHSAListWorkItems(gidLidWgid);
                        const unsigned int coordCount = (unsigned int)gidLidWgid.size();
                        GT_IF_WITH_ASSERT(rcWI && (0 < coordCount) && (0 == (coordCount % 9)))
                        {
                            _pCoordComboBoxes[i]->clear();

                            // Where our items will be held.
                            QStringList itemsStrings;

                            const unsigned int wiCount = coordCount / 9;
                            unsigned int k = 0;

                            // This construct is needed in order to skip values.
                            // When printf is used once, it changes %%u to %u and %%%%u to %%u.
                            // This means: %u = a number in the first printf
                            //             %%u = a number in the second printf
                            //             %%%%u = a number in the third printf
                            const char* paramFormat = ((1 == dims) ? "%u [%%u, %%%%u]" :
                                                       ((2 == dims) ? "%u, %u [(lid: %%u, %%u) (wg: %%%%u, %%%%u)]" :
                                                        /*3 == dims ?*/"%u, %u, %u [(lid: %%u, %%u, %%u) (wg: %%%%u, %%%%u, %%%%u)]"));

                            for (unsigned int j = 0; wiCount > j; ++j)
                            {
                                // Create the current coordinate string.
                                // Note we want to skip indices we don't print:
                                QString s1;
                                gtUInt32 c1 = gidLidWgid[k++];
                                gtUInt32 c2 = gidLidWgid[k++];
                                gtUInt32 c3 = gidLidWgid[k++];
                                s1.sprintf(paramFormat, c1, c2, c3);
                                QString s2;
                                c1 = gidLidWgid[k++];
                                c2 = gidLidWgid[k++];
                                c3 = gidLidWgid[k++];
                                s2.sprintf(s1.toLatin1().constData(), c1, c2, c3);
                                s1.clear();
                                c1 = gidLidWgid[k++];
                                c2 = gidLidWgid[k++];
                                c3 = gidLidWgid[k++];
                                s1.sprintf(s2.toLatin1().constData(), c1, c2, c3);
                                itemsStrings << s1;
                            }

                            GT_ASSERT(coordCount == k);

                            // Add the items.
                            _pCoordComboBoxes[i]->blockSignals(true);
                            _pCoordComboBoxes[i]->addItems(itemsStrings);
                            const int longestItemSize = itemsStrings.last().size();
                            _pCoordComboBoxes[i]->view()->setMinimumWidth(longestItemSize * 12);
                            _pCoordComboBoxes[i]->blockSignals(false);

                            // Update the validator.
                            GT_IF_WITH_ASSERT(_pWIComboValidators[i] != NULL)
                            {
                                _pWIComboValidators[i]->setRange(0, k - 1);
                            }

                            updatedCombo = true;
                        }
                    }
                }

                m_areWIComboBoxesEnabled[i] = updatedCombo;
            }
        }
    }
}

/// -----------------------------------------------------------------------------------------------
/// \brief Name:        updateToolbarValues
/// \brief Description: Updates the toolbar
/// \return void
/// -----------------------------------------------------------------------------------------------
void gwKernelWorkItemToolbar::updateToolbarValues()
{
    // Update the threads combo:
    updateToolbarThreadValues();

    // If we're in kernel debugging, update the work item combos. Otherwise, disable them:
    updateToolbarWorkItemValues();
}

/// -----------------------------------------------------------------------------------------------
/// \brief Name:        resetTooblarValues
/// \brief Description: Enable the rebuild of the WI toolbar
/// \return void
/// -----------------------------------------------------------------------------------------------
void gwKernelWorkItemToolbar::resetTooblarValues(bool rebuildThreadValues, bool rebuildWIValues)
{
    if (rebuildWIValues)
    {
        m_shouldRebuildWICombos = true;
    }

    if (rebuildThreadValues)
    {
        m_shouldRebuildThreadsCombo = true;
    }
}

// ---------------------------------------------------------------------------
// Name:        gwWorkItemCombo::comboLineEditReturnPressed
// Description: Handles an event where the user hits Enter in a QComboBox's LineEdit's
// Author:      Amit Ben-Moshe
// Date:        28/10/2013
// ---------------------------------------------------------------------------
void gwKernelWorkItemToolbar::comboLineEditEditingFinishedHandler()
{
    QLineEdit* pLineEdit = qobject_cast<QLineEdit*>(sender());
    GT_IF_WITH_ASSERT(pLineEdit != NULL)
    {
        // Identify the coordinate.
        int coordinate = -1;

        if (_pCoordComboBoxes[0] != NULL && pLineEdit == _pCoordComboBoxes[0]->lineEdit())
        {
            coordinate = 0;
        }
        else if (_pCoordComboBoxes[1] != NULL && pLineEdit == _pCoordComboBoxes[1]->lineEdit())
        {
            coordinate = 1;
        }
        else if (_pCoordComboBoxes[2] != NULL && pLineEdit == _pCoordComboBoxes[2]->lineEdit())
        {
            coordinate = 2;
        }

        GT_IF_WITH_ASSERT(coordinate > -1)
        {
            // Get the string of the selected item.
            gtASCIIString selectedItemText(pLineEdit->text().toLatin1().data());

            if (!selectedItemText.isEmpty())
            {
                // Get the string as int.
                int selectedValue = -1;
                bool rc = selectedItemText.toIntNumber(selectedValue);

                if (gaIsInKernelDebugging())
                {
                    GT_IF_WITH_ASSERT(rc)
                    {
                        // Update the cache with the successful selection.
                        updateComboSelectionCache(coordinate, selectedValue);

                        // Set the kernel work item.
                        rc = gaSetKernelDebuggingCurrentWorkItemCoordinate(coordinate, selectedValue);
                        GT_ASSERT(rc);
                    }
                }
                else if (gaIsInHSAKernelBreakpoint())
                {
                    QComboBox* pComboBox = _pCoordComboBoxes[coordinate];

                    if (rc)
                    {
                        // Update the server:
                        bool rcIdx = gaHSASetActiveWorkItemIndex(selectedValue);

                        GT_IF_WITH_ASSERT(rcIdx)
                        {
                            // Update the combo:
                            pComboBox->blockSignals(true);
                            pComboBox->setCurrentIndex(selectedValue);
                            pComboBox->blockSignals(false);
                        }
                        else
                        {
                            selectedValue = pComboBox->currentIndex();
                        }
                    }
                    else // !rc
                    {
                        selectedValue = pComboBox->currentIndex();
                    }

                    // Update the text:
                    pLineEdit->blockSignals(true);
                    pLineEdit->setText(pComboBox->itemText(selectedValue));
                    pLineEdit->blockSignals(false);
                }
            }
        }

    }
}

void gwKernelWorkItemToolbar::updateComboSelectionCache(int comboIndex, int selectedItem)
{
    // Update the cache.
    if (comboIndex == 0)
    {
        m_selectedIndexComboA = selectedItem;
    }
    else if (comboIndex == 1)
    {
        m_selectedIndexComboB = selectedItem;
    }
    else if (comboIndex == 2)
    {
        m_selectedIndexComboC = selectedItem;
    }
}

int gwKernelWorkItemToolbar::fetchComboSelectionFromCache(int comboIndex)
{
    int ret = -1;

    if (comboIndex == 0)
    {
        ret = m_selectedIndexComboA;
    }
    else if (comboIndex == 1)
    {
        ret = m_selectedIndexComboB;
    }
    else if (comboIndex == 2)
    {
        ret = m_selectedIndexComboC;
    }

    return ret;
}


// ---------------------------------------------------------------------------
// Name:        gwWorkItemCombo::gwWorkItemCombo
// Description: Constructor
// Arguments:   QWidget* pParent
// Author:      Sigal Algranaty
// Date:        8/2/2012
// ---------------------------------------------------------------------------
gwWorkItemCombo::gwWorkItemCombo(QWidget* pParent) : QComboBox(pParent)
{

}

// ---------------------------------------------------------------------------
// Name:        gwWorkItemCombo::keyPressEvent
// Description: Overrides QWidget key press event (avoid from leaving an empty
//              combo box
// Arguments:   QKeyEvent *pEvent
// Author:      Sigal Algranaty
// Date:        8/2/2012
// ---------------------------------------------------------------------------
void gwWorkItemCombo::keyPressEvent(QKeyEvent* pEvent)
{
    // Sanity check
    GT_IF_WITH_ASSERT(pEvent != NULL)
    {
        // Get the pressed key:
        int currentKey = pEvent->key();

        if (currentKey == Qt::Key_Return)
        {
            // Check if the combo is empty:
            if (currentText().isEmpty())
            {
                // Set the current index to 0:
                setCurrentIndex(0);
            }
        }
    }

    // Call the base class implementation:
    QComboBox::keyPressEvent(pEvent);
}
