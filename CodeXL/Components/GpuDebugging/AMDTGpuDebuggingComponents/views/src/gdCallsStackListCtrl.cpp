//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gdCallsStackListCtrl.cpp
///
//==================================================================================

//------------------------------ gdCallsStackListCtrl.cpp ------------------------------

// Qt
#include <AMDTApplicationComponents/Include/acQtIncludes.h>

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTBaseTools/Include/AMDTDefinitions.h>
#include <AMDTOSWrappers/Include/osApplication.h>
#include <AMDTOSWrappers/Include/osDebugLog.h>
#include <AMDTOSWrappers/Include/osDirectory.h>
#include <AMDTAPIClasses/Include/Events/apCallStackFrameSelectedEvent.h>
#include <AMDTAPIClasses/Include/Events/apEventsHandler.h>
#include <AMDTApplicationComponents/Include/acIcons.h>
#include <AMDTApplicationComponents/Include/acFunctions.h>

// AMDTApplicationFramework:
#include <AMDTApplicationFramework/Include/views/afPropertiesView.h>

// Local:
#include <AMDTGpuDebuggingComponents/Include/views/gdCallsStackListCtrl.h>
#include <AMDTGpuDebuggingComponents/Include/gdApplicationCommands.h>
#include <AMDTGpuDebuggingComponents/Include/gdCommandIDs.h>
#include <AMDTGpuDebuggingComponents/Include/gdPropertiesEventObserver.h>
#include <AMDTGpuDebuggingComponents/Include/gdStringConstants.h>
#include <AMDTGpuDebuggingComponents/Include/gdAidFunctions.h>
#include <AMDTGpuDebuggingComponents/Include/gdHTMLProperties.h>


// ---------------------------------------------------------------------------
// Name:        gdCallsStackListCtrl::gdCallsStackListCtrl
// Description: Constructor
// Arguments: wxWindow* pParent - this window's parent
//            wxWindowID id - this window's ID
// Author:      Uri Shomroni
// Date:        19/10/2008
// ---------------------------------------------------------------------------
gdCallsStackListCtrl::gdCallsStackListCtrl(QWidget* pParent, bool isMainCallStack)
    : acListCtrl(pParent), _callStackSize(0), _isMainCallStack(isMainCallStack), m_sendActivationEvents(false)
{
    // Create and load the image list:
    createAndLoadImageList();

    // Connect selection and activation slots:
    bool rcConnect = connect(this, SIGNAL(itemClicked(QTableWidgetItem*)), this, SLOT(onCallStackListSelected(QTableWidgetItem*)));
    GT_ASSERT(rcConnect);

    rcConnect = connect(this, SIGNAL(itemPressed(QTableWidgetItem*)), this, SLOT(onCallStackListSelected(QTableWidgetItem*)));
    GT_ASSERT(rcConnect);

    rcConnect = connect(this, SIGNAL(itemActivated(QTableWidgetItem*)), this, SLOT(onCallStackListActivated(QTableWidgetItem*)));
    GT_ASSERT(rcConnect);

    rcConnect = connect(this, SIGNAL(currentItemChanged(QTableWidgetItem*, QTableWidgetItem*)), this, SLOT(onCallStackListCurrentItemChanged(QTableWidgetItem*, QTableWidgetItem*)));
    GT_ASSERT(rcConnect);


    // Init headers:
    setColumnCount(1);
    verticalHeader()->hide();
    horizontalHeader()->hide();
    m_displayOnlyFirstColumn = true;

    _emptyCallStackString = AF_STR_NotAvailableA;
}

// ---------------------------------------------------------------------------
// Name:        gdCallsStackListCtrl::~gdCallsStackListCtrl
// Description: Destructor
// Author:      Uri Shomroni
// Date:        19/10/2008
// ---------------------------------------------------------------------------
gdCallsStackListCtrl::~gdCallsStackListCtrl()
{
    int listSize = rowCount();

    // delete the item data in the list
    for (int i = 0; i < listSize; i++)
    {
        gdCallsStackListItemData* pCurrentItemData = (gdCallsStackListItemData*)getItemData(i);

        if (pCurrentItemData != NULL)
        {
            delete pCurrentItemData;
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        gdCallsStackListCtrl::updateCallsStack
// Description: Populates the list control with the callsStack's contents
// Arguments: const osCallStack& callsStack
//            const gtString& emptyStackMessage - the message to display for empty stack
// Author:      Uri Shomroni
// Date:        19/10/2008
// ---------------------------------------------------------------------------
void gdCallsStackListCtrl::updateCallsStack(const osCallStack& callsStack)
{
    // Delete the items and their user data
    deleteListItems();

    // Get the amount of the debugged process threads
    _callStackSize = callsStack.amountOfStackFrames();

    for (int frameIndex = 0; frameIndex < _callStackSize; frameIndex++)
    {
        const osCallStackFrame* currentFrame = NULL;
        currentFrame = callsStack.stackFrame(frameIndex);

        const gtString& functionName = currentFrame->functionName();
        osInstructionPointer functionStartAddress = currentFrame->functionStartAddress();
        osFilePath sourceCodeFilePath = currentFrame->sourceCodeFilePath();
        const int& sourceCodeFileLineNumber = currentFrame->sourceCodeFileLineNumber();
        const osFilePath& sourceCodeModulePath = currentFrame->moduleFilePath();
        const osInstructionPointer& instructionCounterAddress = currentFrame->instructionCounterAddress();

        // Set the user data
        gdCallsStackListItemData* pItemData = new gdCallsStackListItemData;
        pItemData->_functionName = functionName;
        pItemData->_functionStartAddress = functionStartAddress;
        pItemData->_sourceCodeFilePath = sourceCodeFilePath;
        pItemData->_sourceCodeFileLineNumber = sourceCodeFileLineNumber;
        pItemData->_sourceCodeModulePath = sourceCodeModulePath;
        pItemData->_instructionCounterAddress = instructionCounterAddress;

        // Divide the file path to name and full name
        gtString sourceCodeFileName;
        gtString fullFileName;
        sourceCodeFilePath.getFileName(sourceCodeFileName);
        sourceCodeFilePath.getFileNameAndExtension(fullFileName);

#if AMDT_BUILD_CONFIGURATION == AMDT_RELEASE_BUILD
        // If the file is the GRTeaPot example
        // change the path to be relative to the installation directory
        static gtString amdTeaPotLibSrcName1 = L"amdtteapotoclsmokesystem";
        static gtString amdTeaPotLibSrcName2 = L"amdtteapotoglcanvas";
        static gtString amdTeaPotLibSrcName3 = L"amdtteapotrenderstate";
        static gtString amdTeaPotLibSrcName4 = L"amdtfluidgrid";
        static gtString amdTeaPotLibSrcName5 = L"amdtimage";
        static gtString amdTeaPotLibSrcName6 = L"amdtopenclhelper";
        static gtString amdTeaPotLibSrcName7 = L"amdtopenglhelper";
        static gtString amdTeaPotLibSrcName8 = L"amdtopenglmath";

        static gtString amdTeaPotSrcName1 = L"amdtgtkmain";
        static gtString amdTeaPotSrcName2 = L"amdtmainwin";
        static gtString amdTeaPotSrcName3 = L"amdtteapot";

        gtString sourceCodeFileNameLower = sourceCodeFileName;
        gtString sourceCodeFilePathLower = sourceCodeFilePath.asString();
        sourceCodeFileNameLower.toLowerCase();
        sourceCodeFilePathLower.toLowerCase();

        if ((sourceCodeFileNameLower == amdTeaPotLibSrcName1) ||
            (sourceCodeFileNameLower == amdTeaPotLibSrcName2) ||
            (sourceCodeFileNameLower == amdTeaPotLibSrcName3) ||
            (sourceCodeFileNameLower == amdTeaPotLibSrcName4) ||
            (sourceCodeFileNameLower == amdTeaPotLibSrcName5) ||
            (sourceCodeFileNameLower == amdTeaPotLibSrcName6) ||
            (sourceCodeFileNameLower == amdTeaPotLibSrcName7) ||
            (sourceCodeFileNameLower == amdTeaPotLibSrcName8))
        {
            // In debug we will take the source file from its original location
            gtString CodeXLTeaPotDirString;
            osFilePath CodeXLTeapotSrcDir;

            bool rc = CodeXLTeapotSrcDir.SetInstallRelatedPath(osFilePath::OS_CODEXL_TEAPOT_SOURCES_LIB_PATH);
            GT_ASSERT(rc);

            CodeXLTeaPotDirString = CodeXLTeapotSrcDir.asString();

            static bool firstTime = true;

            if (firstTime)
            {
                gtString logMessage;
                logMessage.appendFormattedString(L"sourceCodeFilePath *before* change: %ls\n", sourceCodeFilePath.asString().asCharArray());
                logMessage.appendFormattedString(L"New sourceCodeFilePath directory: %ls\n", CodeXLTeaPotDirString.asCharArray());
                OS_OUTPUT_DEBUG_LOG(logMessage.asCharArray(), OS_DEBUG_LOG_EXTENSIVE);
                firstTime = false;
            }

            sourceCodeFilePath.setFileDirectory(CodeXLTeaPotDirString);
            pItemData->_sourceCodeFilePath = sourceCodeFilePath;
        }
        else if ((sourceCodeFileNameLower == amdTeaPotSrcName1) ||
                 (sourceCodeFileNameLower == amdTeaPotSrcName2) ||
                 (sourceCodeFileNameLower == amdTeaPotSrcName3))
        {
            // In debug we will take the source file from its original location
            gtString CodeXLTeaPotDirString;
            osFilePath CodeXLTeapotSrcDir;

            bool rc = CodeXLTeapotSrcDir.SetInstallRelatedPath(osFilePath::OS_CODEXL_TEAPOT_SOURCES_PATH);
            GT_ASSERT(rc);

            CodeXLTeaPotDirString = CodeXLTeapotSrcDir.asString();

            sourceCodeFilePath.setFileDirectory(CodeXLTeaPotDirString);

            pItemData->_sourceCodeFilePath = sourceCodeFilePath;
        }

#endif

        // Set the full module name into a variable
        gtString fullModuleName;
        sourceCodeModulePath.getFileNameAndExtension(fullModuleName);

        // Set the output string
        gtString listOutputString;

        // We have the function name
        if (!functionName.isEmpty())
        {
            listOutputString.append(functionName);
            listOutputString.append(L" - ");
        }
        else
        {
            gtString instructionCounterAddressAsString;
            gdUserApplicationAddressToDisplayString((osProcedureAddress64)instructionCounterAddress, instructionCounterAddressAsString);
            listOutputString.append(instructionCounterAddressAsString);
            listOutputString.append(L" - ");
        }

        // We have the file name
        if (!fullFileName.isEmpty())
        {
            listOutputString.append(fullFileName);
        }
        else
        {
            listOutputString.append(fullModuleName);
        }

        // We have a valid line number
        if (sourceCodeFileLineNumber > 1)
        {
            listOutputString.appendFormattedString(L", line %d", sourceCodeFileLineNumber);
        }

        // Set the icon for the first image in the list, let the default icon be the empty
        // icon because otherwise, the list width won't be calculated correctly.
        int imageIndex = 0;

        if (frameIndex == 0)
        {
            imageIndex = 1;
        }

        QPixmap* pIcon = NULL;
        GT_IF_WITH_ASSERT((imageIndex >= 0) && (imageIndex < (int)_listIconsVec.size()))
        {
            // Get the icon for this item:
            pIcon = _listIconsVec[imageIndex];
        }

        // Insert the item:
        QStringList list;
        list << acGTStringToQString(listOutputString);

        // bug 377109: do not add the row if this is a 0x00000000 address row
        if (instructionCounterAddress != 0 || (instructionCounterAddress == 0 && _callStackSize < 2))
        {
            addRow(list, pItemData, false, Qt::Unchecked, pIcon);
        }
    }

    // If the list is empty add "N/A"
    if (_callStackSize == 0)
    {
        _emptyCallStackString.isEmpty() ? addRow(_emptyCallStackString) : addMultiLineRow(_emptyCallStackString);
    }
    else
    {
        // In the main frame list:
        if (_isMainCallStack)
        {
            // If the source code viewer is visible (shown)
            // Work interactively - show the source code on each step
            if (rowCount() > 0)
            {
                gdCallsStackListItemData* pItemData = (gdCallsStackListItemData*)(getItemData(0));
                GT_IF_WITH_ASSERT(pItemData != NULL)
                {
                    // Get the selected function properties
                    osFilePath sourceCodeFilePath = pItemData->_sourceCodeFilePath;
                    int sourceCodeFileLineNumber = pItemData->_sourceCodeFileLineNumber;
                    gtString moduleName = pItemData->_sourceCodeModulePath.asString();

                    if (!sourceCodeFilePath.isEmpty())
                    {
                        //Work interactively - show the source code on each step:
                        gdApplicationCommands* pGDApplicationCommands = gdApplicationCommands::gdInstance();
                        GT_IF_WITH_ASSERT(pGDApplicationCommands != NULL)
                        {
                            pGDApplicationCommands->openFileAtLineWithAdditionSourceDir(sourceCodeFilePath, moduleName, sourceCodeFileLineNumber, 0);
                        }
                    }
                }
            }
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        gdCallsStackListCtrl::deleteListItems
// Description: delete the items from the list
// Author:      Avi Shapira
// Date:        11/5/2005
// ---------------------------------------------------------------------------
void gdCallsStackListCtrl::deleteListItems()
{
    int listSize = rowCount();

    // delete the item data
    for (int i = 0; i < listSize; i++)
    {
        delete(gdCallsStackListItemData*)(getItemData(i));
    }

    // Delete Items from the list
    clearList();
}

// ---------------------------------------------------------------------------
// Name:        gdCallsStackListCtrl::setCallStackSize
// Description: Set the call stack size
// Author:      Avi Shapira
// Date:        18/2/2005
// ---------------------------------------------------------------------------
void gdCallsStackListCtrl::setCallStackSize(int callStackSize)
{
    _callStackSize = callStackSize;
}

// ---------------------------------------------------------------------------
// Name:        gdCallsStackListCtrl::createAndLoadImageList
// Description: Creates this view image list, and loads its images
// Author:      Avi Shapira
// Date:        19/10/2004
// ---------------------------------------------------------------------------
void gdCallsStackListCtrl::createAndLoadImageList()
{
    // Add the Bitmaps into the recourse file:
    QPixmap* pEmptyIcon = new QPixmap;
    acSetIconInPixmap(*pEmptyIcon, AC_ICON_EMPTY);

    QPixmap* pNextFunctionIcon = new QPixmap;
    acSetIconInPixmap(*pNextFunctionIcon, AC_ICON_SOURCE_TOP_PROGRAM_COUNTER);

    _listIconsVec.push_back(pEmptyIcon);
    _listIconsVec.push_back(pNextFunctionIcon);
}

// ---------------------------------------------------------------------------
// Name:        gdCallsStackListCtrl::onSetFocus
// Description: Is called when the view get focus
// Arguments:   event - get the event that triggered the context menu
// Author:      Avi Shapira
// Date:        29/6/2008
// ---------------------------------------------------------------------------
void gdCallsStackListCtrl::onSetFocus()
{
    // If this isn't the main frame calls stack view, or we haven't just shown the call stack:
    if (!_isMainCallStack)
    {
        // Get the number of items in the list
        long amountOfCallsStackFrames = rowCount();

        if (amountOfCallsStackFrames > 1)
        {
            QTableWidgetItem* pSelectedItem = NULL;

            if (selectedItems().size() > 0)
            {
                // Get the first selected item
                pSelectedItem = selectedItems().first();

                if (pSelectedItem != NULL)
                {
                    // Call and process the event:
                    onCallStackListSelected(pSelectedItem);
                }
            }
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        gdCallsStackListCtrl::onCallStackListSelected
// Description: Write the details of the selected call-stack function
//              to the properties view
// Arguments:   wxListEvent &eve
// Author:      Avi Shapira
// Date:        12/10/2004
// ---------------------------------------------------------------------------
void gdCallsStackListCtrl::onCallStackListSelected(QTableWidgetItem* pSelectedItem)
{
    // Sanity check
    if (pSelectedItem != NULL)
    {
        // Do not assert here, it's okay if we have no view:
        if (_isMainCallStack)
        {
            // If we selected more than one item, multipleItemsSelected = true
            bool multipleItemsSelected = (selectedItems().size() > 1);

            gdHTMLProperties htmlBuilder;
            gtString propertiesViewMessage;

            if (multipleItemsSelected)
            {
                // Multiple items were selected, show the "Multiple items selected" message:
                afHTMLContent htmlContent;
                htmlBuilder.buildMultipleItemPropertiesString(GD_STR_CallsStackPropertiesTitle, GD_STR_CallsStackPropertiesCallStack, htmlContent);
                htmlContent.toString(propertiesViewMessage);

            }
            else
            {
                // Get the item data:
                gdCallsStackListItemData* pItemData = (gdCallsStackListItemData*)getItemData(pSelectedItem->row());
                GT_IF_WITH_ASSERT(pItemData != NULL)
                {

                    // Build the HTML string for this call stack:
                    afHTMLContent htmlContent;
                    htmlBuilder.buildCallStackPropertiesString(pItemData->_functionName, pItemData->_sourceCodeFilePath, pItemData->_sourceCodeModulePath, pItemData->_sourceCodeFileLineNumber, pItemData->_functionStartAddress, pItemData->_instructionCounterAddress, htmlContent);
                    htmlContent.toString(propertiesViewMessage);
                }
            }

            // Set the properties string:
            gdPropertiesEventObserver::instance().setPropertiesFromText(acGTStringToQString(propertiesViewMessage));
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        gdCallsStackListCtrl::onCallStackListCurrentItemChanged
// Description: Is handling the current item changed signal
// Arguments:   QTableWidgetItem* pCurrentItem
//              QTableWidgetItem* pPreviousItem
// Author:      Sigal Algranaty
// Date:        18/3/2012
// ---------------------------------------------------------------------------
void gdCallsStackListCtrl::onCallStackListCurrentItemChanged(QTableWidgetItem* pCurrentItem, QTableWidgetItem* pPreviousItem)
{
    (void)(pPreviousItem);  // unused
    // Call the selected event:
    onCallStackListSelected(pCurrentItem);
}

// ---------------------------------------------------------------------------
// Name:        gdCallsStackListCtrl::onCallStackListActivated
// Description: Open the source code viewer
// Arguments:   wxListEvent& eve
// Author:      Avi Shapira
// Date:        13/10/2004
// ---------------------------------------------------------------------------
void gdCallsStackListCtrl::onCallStackListActivated(QTableWidgetItem* pActivatedItem)
{
    // Sanity check
    GT_IF_WITH_ASSERT(pActivatedItem != NULL)
    {
        // Get the item data:
        gdCallsStackListItemData* pItemData = (gdCallsStackListItemData*)getItemData(pActivatedItem->row());
        GT_IF_WITH_ASSERT(pItemData != NULL)
        {
            // Get the selected function properties
            osFilePath sourceCodeFilePath = pItemData->_sourceCodeFilePath;
            int sourceCodeFileLineNumber = pItemData->_sourceCodeFileLineNumber;
            gtString moduleName = pItemData->_sourceCodeModulePath.asString();

            // Check if the activated item is the top frame item:
            int activateTextFrameIndex = 0;

            int itemIndex = pActivatedItem->row();

            for (int i = 0; i < itemIndex; i++)
            {
                // Get the current item data:
                gdCallsStackListItemData* pCallStackItemData = (gdCallsStackListItemData*)getItemData(i);
                GT_IF_WITH_ASSERT(pCallStackItemData != NULL)
                {
                    // Check if this frame has "real" source code:
                    if (!pCallStackItemData->_sourceCodeFilePath.isEmpty())
                    {
                        activateTextFrameIndex = 1;
                        break;
                    }
                }
            }

            if (m_sendActivationEvents)
            {
                // Send an event the a frame was selected:
                apCallStackFrameSelectedEvent callStackFrameSelectedEvent(itemIndex);
                apEventsHandler::instance().registerPendingDebugEvent(callStackFrameSelectedEvent);
            }

            if (!sourceCodeFilePath.isEmpty())
            {
                // Open the file at the specific line:
                gdApplicationCommands* pGDApplicationCommandInstance = gdApplicationCommands::gdInstance();
                GT_IF_WITH_ASSERT(pGDApplicationCommandInstance != NULL)
                {
                    pGDApplicationCommandInstance->openFileAtLineWithAdditionSourceDir(sourceCodeFilePath, moduleName, sourceCodeFileLineNumber, activateTextFrameIndex);
                }
            }
            else
            {
                // Open the file at the specific line:
                gdApplicationCommands* pGDApplicationCommandInstance = gdApplicationCommands::gdInstance();
                GT_IF_WITH_ASSERT(pGDApplicationCommandInstance != NULL)
                {
                    pGDApplicationCommandInstance->openFileAtLineWithAdditionSourceDir(sourceCodeFilePath, moduleName, -1, activateTextFrameIndex);
                }
            }
        }
    }
}

