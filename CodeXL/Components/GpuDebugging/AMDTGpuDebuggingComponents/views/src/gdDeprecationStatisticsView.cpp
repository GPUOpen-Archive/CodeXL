//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gdDeprecationStatisticsView.cpp
///
//==================================================================================

//------------------------------ gdDeprecationStatisticsView.cpp ------------------------------

// Qt
#include <AMDTApplicationComponents/Include/acQtIncludes.h>

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTBaseTools/Include/gtPtrVector.h>
#include <AMDTAPIClasses/Include/apMonitoredFunctionBreakPoint.h>
#include <AMDTAPIClasses/Include/apFunctionCallStatistics.h>
#include <AMDTAPIClasses/Include/apGLShaderObject.h>
#include <AMDTAPIClasses/Include/apStatistics.h>
#include <AMDTAPIClasses/Include/Events/apEventsHandler.h>
#include <AMDTApplicationComponents/Include/acColours.h>
#include <AMDTApplicationComponents/Include/acIcons.h>
#include <AMDTApiFunctions/Include/gaGRApiFunctions.h>
#include <AMDTApplicationComponents/Include/acFunctions.h>

// AMDTApplicationFramework:
#include <AMDTApplicationFramework/Include/afAppStringConstants.h>

// Local:
#include <AMDTGpuDebuggingComponents/Include/gdCommandIDs.h>
#include <AMDTGpuDebuggingComponents/Include/gdStringConstants.h>
#include <AMDTGpuDebuggingComponents/Include/gdAidFunctions.h>
#include <AMDTGpuDebuggingComponents/Include/gdDeprecationStringConstants.h>
#include <AMDTApplicationFramework/Include/afGlobalVariableChangedEvent.h>
#include <AMDTGpuDebuggingComponents/Include/gdGDebuggerGlobalVariablesManager.h>
#include <AMDTGpuDebuggingComponents/Include/views/gdDeprecationStatisticsView.h>

// Minimal percentages to show each type of warning  (note that the number is floating point
// so this is actually the maximum percentage of the tier below:
#define GD_YELLOW_WARNING_MIN_PCT 0
#define GD_ORANGE_WARNING_MIN_PCT 33.33
#define GD_RED_WARNING_MIN_PCT 66.67

// ---------------------------------------------------------------------------
// Name:        gdDeprecationStatisticsView::gdDeprecationStatisticsView
// Description: Constructor
// Arguments: wxWindow* pParent - my parent window
//            wxWindowID id
//            const wxPoint& pos
//            const wxSize& size
//            long style
// Return Val:
// Author:      Sigal Algranaty
// Date:        25/2/2009
// ---------------------------------------------------------------------------
gdDeprecationStatisticsView::gdDeprecationStatisticsView(QWidget* pParent)
    : gdStatisticsViewBase(pParent, GD_STATISTICS_VIEW_DEPRECATION_INDEX, GD_STR_StatisticsViewerDeprectionStatisticsShortName),
      _totalAmountOfDeprecatedFunctionCalls(0), _totalAmountOfFunctionCallsInFrame(0)
{
    // Add breakpoint actions to context menu:
    _addBreakpointActions = true;

    // Call the base class initialization function:
    init();
}

// ---------------------------------------------------------------------------
// Name:        gdDeprecationStatisticsView::~gdDeprecationStatisticsView
// Description: Destructor
// Author:      Sigal Algranaty
// Date:        25/2/2009
// ---------------------------------------------------------------------------
gdDeprecationStatisticsView::~gdDeprecationStatisticsView()
{
}

// ---------------------------------------------------------------------------
// Name:        gdDeprecationStatisticsView::initListCtrlColumns
// Description: Sets the columns widths and titles
// Author:      Sigal Algranaty
// Date:        25/2/2009
// ---------------------------------------------------------------------------
void gdDeprecationStatisticsView::initListCtrlColumns()
{
    // Create the "Deprecation Statistics View" columns:
    _listControlColumnTitles.push_back(GD_STR_DeprecationStatisticsViewerColumn1Title);
    _listControlColumnTitles.push_back(GD_STR_DeprecationStatisticsViewerColumn2Title);
    _listControlColumnTitles.push_back(GD_STR_DeprecationStatisticsViewerColumn3Title);
    _listControlColumnTitles.push_back(GD_STR_DeprecationStatisticsViewerColumn4Title);
    _listControlColumnTitles.push_back(GD_STR_DeprecationStatisticsViewerColumn5Title);
    _listControlColumnTitles.push_back(GD_STR_DeprecationStatisticsViewerColumn6Title);

    // Create the columns titles:
    _listControlColumnWidths.push_back(0.20f);
    _listControlColumnWidths.push_back(0.25f);
    _listControlColumnWidths.push_back(0.10f);
    _listControlColumnWidths.push_back(0.10f);
    _listControlColumnWidths.push_back(0.14f);
    _listControlColumnWidths.push_back(0.14f);

    // Make sure that thousand separators are removed:
    m_removeThousandSeparatorOnCopy = true;

    // Add postfixes to the columns:
    m_columnsPostfixes.push_back("");
    m_columnsPostfixes.push_back("");
    m_columnsPostfixes.push_back("");
    m_columnsPostfixes.push_back("");
    m_columnsPostfixes.push_back("");
    m_columnsPostfixes.push_back("%");

    _widestColumnIndex = 5;
    _initialSortColumnIndex = 2;
    _sortInfo._sortOrder = Qt::DescendingOrder;
}

// ---------------------------------------------------------------------------
// Name:        gdDeprecationStatisticsView::updateFunctionCallsStatisticsList
// Description: Update the current statistics into the listCTRL
// Arguments: const apStatistics& currentStatistics
// Return Val: void
// Author:      Sigal Algranaty
// Date:        25/2/2009
// ---------------------------------------------------------------------------
bool gdDeprecationStatisticsView::updateFunctionCallsStatisticsList(const apStatistics& currentStatistics)
{
    bool retVal = true;

    // Delete the items and their user data:
    clearAllStatisticsItems();

    // Enable the list
    setEnabled(true);

    // Reset total number of function calls:
    _totalAmountOfDeprecatedFunctionCalls = 0;
    _totalAmountOfFunctionCallsInFrame = 0;

    // Get the number of function calls statistics:
    int functionStatisticsAmount = currentStatistics.amountOfFunctionCallsStatistics();

    if (functionStatisticsAmount > 0)
    {
        // Reset column width - since we change it in non analyze mode:
        resetColumnsWidth();

        // Calculate the total amount of function calls:
        for (int i = 0; i < functionStatisticsAmount; i++)
        {
            const apFunctionCallStatistics* pFunctionCallStatisticsData = NULL;
            bool rc = currentStatistics.getFunctionCallStatistics(i, pFunctionCallStatisticsData);
            GT_IF_WITH_ASSERT(rc && pFunctionCallStatisticsData != NULL)
            {
                // Add the function to the total counter:
                gtUInt64 amountOfTimesCalled = pFunctionCallStatisticsData->_amountOfTimesCalled;
                _totalAmountOfFunctionCallsInFrame += amountOfTimesCalled;
            }
        }

        // Get current execution mode:
        bool rc = gaGetDebuggedProcessExecutionMode(_executionMode);
        GT_ASSERT(rc);

        // Fill the list:
        for (int i = 0; i < functionStatisticsAmount; i++)
        {
            const apFunctionCallStatistics* pFunctionCallStatisticsData = NULL;
            bool ret = currentStatistics.getFunctionCallStatistics(i, pFunctionCallStatisticsData);
            GT_IF_WITH_ASSERT(ret && pFunctionCallStatisticsData != NULL)
            {
                // For analyze mode, count all the deprecation status:
                if (_executionMode == AP_ANALYZE_MODE)
                {
                    // Iterate the function deprecation statuses, and add if not 0:
                    for (int j = AP_DEPRECATION_NONE + 1; j < AP_DEPRECATION_STATUS_AMOUNT; j++)
                    {
                        int currentDeprecationStatusNumOfCalls = (*pFunctionCallStatisticsData)._amountOfDeprecatedTimesCalled[j];

                        if (currentDeprecationStatusNumOfCalls > 0)
                        {
                            // Add the function to the list:
                            retVal = retVal && addFunctionToList(*pFunctionCallStatisticsData, (apFunctionDeprecationStatus)j);
                        }
                    }
                }
                else
                {
                    // Get the function id:
                    const apMonitoredFunctionId functionId = pFunctionCallStatisticsData->_functionId;

                    // Get the function type:
                    unsigned int functionType;
                    bool rc2 = gaGetMonitoredFunctionType(functionId, functionType);
                    GT_ASSERT(rc2);

                    // For debug mode, add only the fully deprecated function:
                    if (functionType & AP_DEPRECATED_FUNC)
                    {
                        retVal = retVal && addFunctionToList(*pFunctionCallStatisticsData, AP_DEPRECATION_FULL);
                    }
                }
            }
        }

        if (_activeContextId.isOpenGLContext())
        {
            // Add GLSL version deprecation:
            addGLSLDeprecationItems();
        }

        // Add the "More..." item to the list:
        // NOTICE: Sigal 26/6/2011 When analyze mode is supported, we should remove this comment!
        // addMoreItemToList();

        // Add the "Total" item to the list:
        addTotalItemToList();

        // Sort the item in the list control:
        sortItems(0, _sortInfo._sortOrder);
    }
    else
    {
        addRow(AF_STR_NotAvailableA);
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gdDeprecationStatisticsView::addFunctionToList
// Description: Adds function counter statistics item to the list (for function with enumerations)
// Arguments: const apFunctionCallStatistics& functionStatistics - the function statistics object
//            apFunctionDeprecationStatus deprecationStatus - the function deprecation status
// Return Val: bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        25/2/2009
// ---------------------------------------------------------------------------
bool gdDeprecationStatisticsView::addFunctionToList(const apFunctionCallStatistics& functionStatistics, apFunctionDeprecationStatus deprecationStatus)
{
    bool retVal = true;

    // Get the function ID:
    apMonitoredFunctionId functionId = functionStatistics._functionId;

    // Get the function name:
    gtString functionName;
    retVal = retVal && gaGetMonitoredFunctionName(functionId, functionName);
    GT_IF_WITH_ASSERT(retVal)
    {

        // Get total number of calls:
        gtUInt64 totalNumberOfCalls = 0;

        if (_executionMode == AP_DEBUGGING_MODE)
        {
            totalNumberOfCalls = functionStatistics._amountOfTimesCalled;
        }
        else
        {
            // Check the number of function call (for this deprecation status):
            totalNumberOfCalls = functionStatistics._amountOfDeprecatedTimesCalled[deprecationStatus];
        }

        // Add this item number of calls to total calls counter:
        _totalAmountOfDeprecatedFunctionCalls += totalNumberOfCalls;

        gtString totalCallsAmountString;
        gtString totalCallsPercentageString;

        totalCallsAmountString.appendFormattedString(L"%ld", totalNumberOfCalls);
        totalCallsAmountString.addThousandSeperators();

        float percentageCalled = 0;

        if (_totalAmountOfFunctionCallsInFrame > 0)
        {
            // Calculate the appearers percentage:
            percentageCalled = totalNumberOfCalls;
            percentageCalled *= 100;
            percentageCalled /= _totalAmountOfFunctionCallsInFrame;
            totalCallsPercentageString.appendFormattedString(L"%.2f", percentageCalled);
        }
        else
        {
            totalCallsPercentageString.append(AF_STR_NotAvailable);
        }

        apAPIVersion deprecatedAtVersion = AP_GL_VERSION_NONE;
        apAPIVersion removedAtVersion = AP_GL_VERSION_NONE;

        if ((_executionMode == AP_DEBUGGING_MODE) || (deprecationStatus == AP_DEPRECATION_FULL))
        {
            // Get the deprecated at and removed at versions:
            gaGetMonitoredFunctionDeprecationVersion(functionId, deprecatedAtVersion, removedAtVersion);
        }
        else
        {
            // Get the deprecation version from the deprecation status class:
            bool rc = apFunctionDeprecation::getDeprecationAndRemoveVersionsByStatus(deprecationStatus, deprecatedAtVersion, removedAtVersion);
            GT_ASSERT(rc);
        }

        // Build the deprecation versions strings:
        gtString deprecatedAtStr;
        gtString removedAtStr;
        bool rc1 = apAPIVersionToString(deprecatedAtVersion, deprecatedAtStr);
        GT_ASSERT(rc1);
        bool rc2 = apAPIVersionToString(removedAtVersion, removedAtStr);
        GT_ASSERT(rc2);

        // Add the deprecation status to the function name:
        // (we have items in format of: glTexImage - Deprecated Argument Value
        gtString deprecationStatusStr;
        bool rc = apFunctionDeprecationStatusToString(deprecationStatus, deprecationStatusStr);
        GT_ASSERT(rc);

        // Define the item strings:
        QStringList rowStrings;

        rowStrings << acGTStringToQString(functionName);
        rowStrings << acGTStringToQString(deprecationStatusStr);
        rowStrings << acGTStringToQString(totalCallsAmountString);
        rowStrings << acGTStringToQString(totalCallsPercentageString);
        rowStrings << acGTStringToQString(deprecatedAtStr);
        rowStrings << acGTStringToQString(removedAtStr);

        // Add the item data:
        gdStatisticsViewItemData* pItemData = new gdStatisticsViewItemData;

        // Fill the item data;
        pItemData->_functionName = functionName;
        pItemData->_functionId = functionId;
        pItemData->_totalAmountOfTimesCalled = totalNumberOfCalls;
        pItemData->_percentageOfTimesCalled = percentageCalled;
        pItemData->_deprecatedAtVersion = deprecatedAtVersion;
        pItemData->_removedAtVersion = removedAtVersion;
        pItemData->_deprecationStatus = deprecationStatus;

        // Add row to table:
        addRow(rowStrings, pItemData, false, Qt::Unchecked, icon(1));

    }
    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gdDeprecationStatisticsView::addTotalItemToList
// Description: Adds the "Total" item to the list control.
// Author:      Sigal Algranaty
// Date:        25/2/2009
// ---------------------------------------------------------------------------
void gdDeprecationStatisticsView::addTotalItemToList()
{
    // Get the amount of list items:
    int listSize = rowCount();

    if (listSize == 0)
    {
        // Add an item stating that there are not items:
        addEmptyListItem();
    }
    else
    {

        // Calculate the % of deprecated function calls:
        float deprecatedCallsPercentage = ((float)_totalAmountOfDeprecatedFunctionCalls / (float)_totalAmountOfFunctionCallsInFrame) * 100.0f;

        // Build displayed strings:
        gtASCIIString totalString;
        gtASCIIString totalAmountOfFunctionCallsString;
        gtASCIIString totalPrecentageString;

        totalString.appendFormattedString("Total (%d items)", listSize);

        totalAmountOfFunctionCallsString.appendFormattedString("%ld", _totalAmountOfDeprecatedFunctionCalls);
        totalAmountOfFunctionCallsString.addThousandSeperators();
        totalPrecentageString.appendFormattedString("%.2f", deprecatedCallsPercentage);

        QStringList list;
        list << totalString.asCharArray();
        list << AF_STR_EmptyA;
        list << totalAmountOfFunctionCallsString.asCharArray();
        list << totalPrecentageString.asCharArray();
        list << AF_STR_EmptyA;
        list << AF_STR_EmptyA;

        addRow(list, NULL, false, Qt::Unchecked, icon(0));

        setItemBold(rowCount() - 1);

    }
}

// ---------------------------------------------------------------------------
// Name:        gdDeprecationStatisticsView::addTotalItemToList
// Description: Adds the "More..." item to the list control in debugging mode.
// Author:      Sigal Algranaty
// Date:        17/3/2009
// ---------------------------------------------------------------------------
void gdDeprecationStatisticsView::addMoreItemToList()
{
    if (_executionMode != AP_ANALYZE_MODE)
    {
        // Get the amount of list items:
        QStringList list;

        // Insert the item (enumerator) into the list:
        list << AF_STR_MoreA;
        list << GD_STR_StateChangeStatisticsViewerNonAnalyzeModeA;
        list << AF_STR_NotAvailableA;
        list << AF_STR_NotAvailableA;
        list << AF_STR_NotAvailableA;
        list << AF_STR_NotAvailableA;

        // Get the icon:
        QPixmap* pIcon = icon(2);

        addRow(list, NULL, false, Qt::Unchecked, pIcon);
    }
}

// ---------------------------------------------------------------------------
// Name:        gdDeprecationStatisticsView::initializeImageList
// Description: Create and populate the image list for this item
// Author:      Sigal Algranaty
// Date:        25/2/2009
// ---------------------------------------------------------------------------
void gdDeprecationStatisticsView::initializeImageList()
{
    // Create the icons from the xpm files:
    QPixmap* pEmptyIcon = new QPixmap;
    acSetIconInPixmap(*pEmptyIcon, AC_ICON_EMPTY);

    QPixmap* pYellowWarningIcon = new QPixmap;
    acSetIconInPixmap(*pYellowWarningIcon, AC_ICON_WARNING_YELLOW);

    QPixmap* pInfoIcon = new QPixmap;
    acSetIconInPixmap(*pInfoIcon, AC_ICON_WARNING_INFO);

    // Add the icons to the list
    _listIconsVec.push_back(pEmptyIcon);            // 0
    _listIconsVec.push_back(pYellowWarningIcon);    // 1
    _listIconsVec.push_back(pInfoIcon);             // 2
}


// ---------------------------------------------------------------------------
// Name:        gdDeprecationStatisticsView::getItemDeprecationVersions
// Description:  Given an item index, the function returns the monitored function
//               OpenGL deprecation version, and OpenGL remove version
// Arguments: int itemIndex
//            apAPIVersion& deprecatedAtVersion
//            apAPIVersion& removedAtVersion
// Return Val: bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        2/3/2009
// ---------------------------------------------------------------------------
bool gdDeprecationStatisticsView::getItemDeprecationVersions(int itemIndex, apAPIVersion& deprecatedAtVersion, apAPIVersion& removedAtVersion)
{
    bool retVal = false;

    gdStatisticsViewItemData* pItemData = gdStatisticsViewBase::getItemData(itemIndex);
    GT_IF_WITH_ASSERT(pItemData != NULL)
    {
        deprecatedAtVersion = pItemData->_deprecatedAtVersion;
        removedAtVersion = pItemData->_removedAtVersion;
        retVal = true;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gdDeprecationStatisticsView::getItemDeprecationStatus
// Description: Return a selected item deprecation status
// Arguments: int itemIndex
//            apFunctionDeprecationStatus& functionDeprecationStatus
// Return Val: bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        10/3/2009
// ---------------------------------------------------------------------------
bool gdDeprecationStatisticsView::getItemDeprecationStatus(int itemIndex, apFunctionDeprecationStatus& functionDeprecationStatus)
{
    bool retVal = false;

    gdStatisticsViewItemData* pItemData = gdStatisticsViewBase::getItemData(itemIndex);
    GT_IF_WITH_ASSERT(pItemData != NULL)
    {
        functionDeprecationStatus = pItemData->_deprecationStatus;
        retVal = true;
    }
    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gdDeprecationStatisticsView::getItemFunctionId
// Description: Return a selected item deprecation status
// Arguments: int itemIndex
//            int& functionID
// Return Val: bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        10/3/2009
// ---------------------------------------------------------------------------
bool gdDeprecationStatisticsView::getItemFunctionId(int itemIndex, int& functionID)
{
    bool retVal = false;

    // Get the list size:
    gdStatisticsViewItemData* pItemData = gdStatisticsViewBase::getItemData(itemIndex);
    GT_IF_WITH_ASSERT(pItemData != NULL)
    {
        functionID = pItemData->_functionId;
        retVal = true;
    }
    return retVal;
}



// ---------------------------------------------------------------------------
// Name:        gdDeprecationStatisticsView::addGLSLDeprecationItems
// Description: Go through the shader objects and check their version
//              Add deprecation item for a shader object with a deprecated version
// Author:      Sigal Algranaty
// Date:        24/3/2009
// ---------------------------------------------------------------------------
void gdDeprecationStatisticsView::addGLSLDeprecationItems()
{
    int amountOfShaders = 0;
    bool rc = gaGetAmountOfShaderObjects(_activeContextId._contextId, amountOfShaders);

    if (rc)
    {
        // Iterate the shaders
        // (check if there is a shader which is not belong to any program):
        for (int curShaderId = 0; curShaderId < amountOfShaders; curShaderId++)
        {
            // Get the current shader name:
            GLuint curShaderName = 0;
            rc = gaGetShaderObjectName(_activeContextId._contextId, curShaderId, curShaderName);

            if (rc)
            {
                // Get the current shader details:
                gtAutoPtr<apGLShaderObject> aptrShaderDetails = NULL;
                rc = gaGetShaderObjectDetails(_activeContextId._contextId, curShaderName, aptrShaderDetails);

                // Check the shader version deprecation status:
                apGLShaderObject* pShaderObject = aptrShaderDetails.pointedObject();

                if (rc && (pShaderObject != NULL))
                {
                    // Get the shader version;
                    apGLSLVersion shaderVersion = pShaderObject->shaderVersion();

                    if ((shaderVersion == AP_GLSL_VERSION_1_0) || (shaderVersion == AP_GLSL_VERSION_1_1) || (shaderVersion == AP_GLSL_VERSION_1_2))
                    {
                        // Add deprecation:
                        gtString functionName;
                        functionName.appendFormattedString(GD_STR_DeprecationShaderName, pShaderObject->shaderName());
                        gtString deprecationStatusStr;
                        rc = apFunctionDeprecationStatusToString(AP_DEPRECATION_GLSL_VERSION, deprecationStatusStr);

                        // Get the deprecation version from the deprecation status class:
                        apAPIVersion deprecatedAtVersion, removedAtVersion;
                        rc = apFunctionDeprecation::getDeprecationAndRemoveVersionsByStatus(AP_DEPRECATION_GLSL_VERSION, deprecatedAtVersion, removedAtVersion);
                        GT_ASSERT(rc);

                        // Build the deprecation versions strings:
                        gtString deprecatedAtStr;
                        gtString removedAtStr;
                        bool rc1 = apAPIVersionToString(deprecatedAtVersion, deprecatedAtStr);
                        GT_ASSERT(rc1);
                        bool rc2 = apAPIVersionToString(removedAtVersion, removedAtStr);
                        GT_ASSERT(rc2);

                        // Add the item:
                        QStringList rowStrings;

                        // Insert the item into the list:
                        rowStrings << acGTStringToQString(functionName);
                        rowStrings << acGTStringToQString(deprecationStatusStr);
                        rowStrings << AF_STR_NotAvailableA;
                        rowStrings << AF_STR_NotAvailableA;
                        rowStrings << acGTStringToQString(deprecatedAtStr);
                        rowStrings << acGTStringToQString(removedAtStr);

                        // Add the item data:
                        gdStatisticsViewItemData* pItemData = new gdStatisticsViewItemData;

                        // Fill the item data;
                        pItemData->_functionName = functionName;
                        pItemData->_functionId = ap_glShaderSource;
                        pItemData->_totalAmountOfTimesCalled = 0;
                        pItemData->_percentageOfTimesCalled = 0;
                        pItemData->_deprecatedAtVersion = deprecatedAtVersion;
                        pItemData->_removedAtVersion = removedAtVersion;
                        pItemData->_deprecationStatus = AP_DEPRECATION_GLSL_VERSION;

                        // Add the row:
                        addRow(rowStrings, pItemData, false, Qt::Unchecked, icon(1));
                    }
                }
            }
        }
    }
}

// -------------------------------------------------------------------------- -
// Name:        gdDeprecationStatisticsView::getItemTooltip
// Description: Get an item tooltip
// Arguments: int itemIndex
// Return Val: gtString
// Author:      Yuri Rshtunique
// Date:        November 10, 2014
// ---------------------------------------------------------------------------
gtString gdDeprecationStatisticsView::getItemTooltip(int itemIndex)
{
    gtString retVal;
    // Get the item data:
    gdStatisticsViewItemData* pItemData = (gdStatisticsViewItemData*)getItemData(itemIndex);

    if (pItemData != NULL)
    {
        // Build the tooltip string:
        gtString functionName = pItemData->_functionName;
        gtString deprecationStatusStr;
        bool rc = apFunctionDeprecationStatusToString(pItemData->_deprecationStatus, deprecationStatusStr);
        GT_ASSERT(rc);
        retVal.appendFormattedString(L"%ls: %ls", functionName.asCharArray(), deprecationStatusStr.asCharArray());
    }

    return retVal;
}
