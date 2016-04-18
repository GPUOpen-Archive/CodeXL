//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gdStateChangeStatisticsView.cpp
///
//==================================================================================

//------------------------------ gdStateChangeStatisticsView.cpp ------------------------------

// Qt
#include <AMDTApplicationComponents/Include/acQtIncludes.h>

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTBaseTools/Include/gtPtrVector.h>
#include <AMDTAPIClasses/Include/apFunctionCallStatistics.h>
#include <AMDTAPIClasses/Include/apGLenumParameter.h>
#include <AMDTAPIClasses/Include/apMonitoredFunctionBreakPoint.h>
#include <AMDTAPIClasses/Include/apStatistics.h>
#include <AMDTAPIClasses/Include/Events/apEventsHandler.h>
#include <AMDTApiFunctions/Include/gaGRApiFunctions.h>
#include <AMDTApplicationComponents/Include/acColours.h>
#include <AMDTApplicationComponents/Include/acIcons.h>
#include <AMDTApplicationComponents/Include/acMessageBox.h>
#include <AMDTApplicationComponents/Include/acFunctions.h>

#if (AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_GENERIC_LINUX_VARIANT)
    #include <AMDTAPIClasses/Include/apGLXParameters.h>
#endif

// AMDTApplicationFramework:
#include <AMDTApplicationFramework/Include/afAppStringConstants.h>
#include <AMDTApplicationFramework/Include/afHTMLContent.h>
#include <AMDTApplicationFramework/src/afUtils.h>

// Local:
#include <AMDTGpuDebuggingComponents/Include/gdApplicationCommands.h>
#include <AMDTGpuDebuggingComponents/Include/gdCommandIDs.h>
#include <AMDTGpuDebuggingComponents/Include/gdStringConstants.h>
#include <AMDTGpuDebuggingComponents/Include/gdAidFunctions.h>
#include <AMDTApplicationFramework/Include/afGlobalVariableChangedEvent.h>
#include <AMDTGpuDebuggingComponents/Include/gdGDebuggerGlobalVariablesManager.h>
#include <AMDTGpuDebuggingComponents/Include/views/gdStateChangeStatisticsView.h>
#include <AMDTGpuDebuggingComponents/Include/commands/gdSaveStateChangeStatisticsCommand.h>

// Minimal percentages to show each type of warning  (note that the number is floating point
// so this is actually the maximum percentage of the tier below:
#define GD_YELLOW_WARNING_MIN_PCT 0
#define GD_ORANGE_WARNING_MIN_PCT 33.33
#define GD_RED_WARNING_MIN_PCT 66.67

// ---------------------------------------------------------------------------
// Name:        gdStateChangeStatisticsView::gdStateChangeStatisticsView
// Description: Constructor.
// Arguments:   parent - My parent window.
// Author:      Avi Shapira
// Date:        29/1/2006
// ---------------------------------------------------------------------------
gdStateChangeStatisticsView::gdStateChangeStatisticsView(QWidget* pParent)
    : gdStatisticsViewBase(pParent, GD_STATISTICS_VIEW_UNKNOWN/*GD_STATISTICS_VIEW_STATE_CHANGE_INDEX*/, GD_STR_StatisticsViewerStateChangeStatisticsShortName),
      _totalAmountOfStateChangeFunctionCalls(0), _totalAmountOfEffectiveCalls(0), _totalAmountOfRedundantCalls(0)
{
    // Add breakpoint actions to context menu:
    _addBreakpointActions = true;

    // Call the base class init function:
    init();
}

// ---------------------------------------------------------------------------
// Name:        gdStateChangeStatisticsView::~gdStateChangeStatisticsView
// Description: Destructor
// Author:      Avi Shapira
// Date:        29/1/2006
// ---------------------------------------------------------------------------
gdStateChangeStatisticsView::~gdStateChangeStatisticsView()
{
}

// ---------------------------------------------------------------------------
// Name:        gdStateChangeStatisticsView::initListCtrlColumns
// Description: Create the vectors of columns widths and titles that is used
//              by base class
// Author:      Sigal Algranaty
// Date:        21/5/2009
// ---------------------------------------------------------------------------
void gdStateChangeStatisticsView::initListCtrlColumns()
{
    // Initialize the "State Change Statistics View" columns:
    _listControlColumnTitles.push_back(GD_STR_StateChangeStatisticsViewerColumn1Title);
    _listControlColumnTitles.push_back(GD_STR_StateChangeStatisticsViewerColumn2Title);
    _listControlColumnTitles.push_back(GD_STR_StateChangeStatisticsViewerColumn3Title);
    _listControlColumnTitles.push_back(GD_STR_StateChangeStatisticsViewerColumn4Title);
    _listControlColumnTitles.push_back(GD_STR_StateChangeStatisticsViewerColumn5Title);

    // Initialize the "State Change Statistics View" widths:
    _listControlColumnWidths.push_back(0.30f);
    _listControlColumnWidths.push_back(0.10f);
    _listControlColumnWidths.push_back(0.15f);
    _listControlColumnWidths.push_back(0.15f);
    _listControlColumnWidths.push_back(0.15f);

    // Set copy postfixes:
    m_columnsPostfixes.push_back("");
    m_columnsPostfixes.push_back("");
    m_columnsPostfixes.push_back("");
    m_columnsPostfixes.push_back("");
    m_columnsPostfixes.push_back("%");

    m_removeThousandSeparatorOnCopy = true;

    _widestColumnIndex = 4;
    _initialSortColumnIndex = 3;
    _sortInfo._sortOrder = Qt::DescendingOrder;
}

// ---------------------------------------------------------------------------
// Name:        gdStateChangeStatisticsView::updateFunctionCallsStatisticsList
// Description: Update the current statistics into the listCTRL
// Arguments: const apStatistics& currentStatistics
// Return Val: void
// Author:      Sigal Algranaty
// Date:        21/7/2008
// ---------------------------------------------------------------------------
bool gdStateChangeStatisticsView::updateFunctionCallsStatisticsList(const apStatistics& currentStatistics)
{
    bool retVal = true;

    // Delete the items and their user data
    clearAllStatisticsItems();

    // Enable the list
    setEnabled(true);

    // Check the current execution mode:
    gaGetDebuggedProcessExecutionMode(_executionMode);

    // For execution mode - add redundant state change statistics
    if (_executionMode == AP_ANALYZE_MODE)
    {
        // Reset column width - since we change it in non analyze mode:
        resetColumnsWidth();

        // Reset total number of function calls:
        _totalAmountOfStateChangeFunctionCalls = 0;
        _totalAmountOfEffectiveCalls = 0;
        _totalAmountOfRedundantCalls = 0;

        // Get the number of function calls statistics:
        int functionStatisticsAmount = currentStatistics.amountOfFunctionCallsStatistics();

        if (functionStatisticsAmount > 0)
        {
            // Fill the list:
            for (int i = 0; i < functionStatisticsAmount; i++)
            {
                const apFunctionCallStatistics* pFunctionCallStatisticsData = NULL;
                bool rc = currentStatistics.getFunctionCallStatistics(i, pFunctionCallStatisticsData);
                GT_IF_WITH_ASSERT(rc && pFunctionCallStatisticsData != NULL)
                {
                    // Get the function id:
                    apMonitoredFunctionId functionId = pFunctionCallStatisticsData->_functionId;

                    // Get the function type:
                    unsigned int functionType = 0;
                    retVal = retVal && gaGetMonitoredFunctionType(functionId, functionType);

                    // Check if the function should be added to list:
                    if (functionType & AP_STATE_CHANGE_FUNC)
                    {
                        // Get the internal function data:
                        const gtVector<apEnumeratorUsageStatistics>& usedEnumerators = pFunctionCallStatisticsData->_usedEnumerators;
                        int enumeratorsVectorSize = usedEnumerators.size();

                        if (enumeratorsVectorSize > 0)
                        {
                            // We have enumerators details for this function:

                            // Get the enumerators of the function calls:
                            for (int j = 0; j < enumeratorsVectorSize; j++)
                            {
                                // Get the enumerator data:
                                apEnumeratorUsageStatistics enumeratorUsageStatisticsData = usedEnumerators[j];

                                // Add the item into the list ctrl:
                                retVal = retVal && addFunctionToList(*pFunctionCallStatisticsData, enumeratorUsageStatisticsData);
                            }
                        }
                        else
                        {
                            retVal = retVal && addFunctionToList(*pFunctionCallStatisticsData);
                        }
                    }
                }
            }

            // Add the "Total" item to the list:
            addTotalItemToList();

            // Sort the item in the list control:
            sortItems(0, _sortInfo._sortOrder);
        }
        else
        {
            // Add a non available item to the list:
            addRow(AF_STR_NotAvailableA);

        }
    }
    else
    {
        // Add a non available item to the list:
        addRow(AF_STR_NotAvailableA);
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gdStateChangeStatisticsView::addStatisticsItem
// Description: Adds function counter statistics item to the list (for function with enumerations)
// Arguments: const apFunctionCallStatistics& functionStatistics - the function statistics object
//            const apEnumeratorUsageStatistics& functionStatistics - the function enumeration details
// Return Val: bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        21/7/2008
// ---------------------------------------------------------------------------
bool gdStateChangeStatisticsView::addFunctionToList(const apFunctionCallStatistics& functionStatistics, const apEnumeratorUsageStatistics& functionEnumStatistics)
{
    bool retVal = true;

    // Get the function ID:
    apMonitoredFunctionId functionId = functionStatistics._functionId;

    // Get the function name:
    gtString functionName;
    retVal = retVal && gaGetMonitoredFunctionName(functionId, functionName);
    GT_IF_WITH_ASSERT(retVal)
    {

        // Get the enumerator name:
        gtString enumeratorAsString;
        GLenum enumerator = functionEnumStatistics._enum;

        // Treat primitive type parameters as such and not as normal enumerators
        // This will cause GL_POINTS and GL_LINES to be displayed correctly.
        osTransferableObjectType enumType = functionEnumStatistics._enumType;

        if (enumType == OS_TOBJ_ID_GL_ENUM_PARAMETER)
        {
            apGLenumParameter enumParam(enumerator);
            enumParam.valueAsString(enumeratorAsString);
        }
        else if (enumType == OS_TOBJ_ID_GL_PRIMITIVE_TYPE_PARAMETER)
        {
            apGLPrimitiveTypeParameter enumParam(enumerator);
            enumParam.valueAsString(enumeratorAsString);
        }
        else if (enumType == OS_TOBJ_ID_GLX_ENUM_PARAMETER)
        {
#if (AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_GENERIC_LINUX_VARIANT)
            apGLXenumParameter enumParam(enumerator);
            enumParam.valueAsString(enumeratorAsString);
#else
            // glX parameters should not be used in Mac or Windows:
            GT_ASSERT(false);
#endif
        }
        else
        {
            GT_ASSERT_EX(((enumType == OS_TOBJ_ID_GL_ENUM_PARAMETER) || (enumType == OS_TOBJ_ID_GL_PRIMITIVE_TYPE_PARAMETER) || (enumType == OS_TOBJ_ID_GL_ENUM_PARAMETER)), L"Could not determine Enumerator type");
        }

        // Set the function name output string:
        gtString functionNameEnum = functionName;
        functionNameEnum.append(L" - ");
        functionNameEnum.append(enumeratorAsString);

        // Calculate total number of calls and #effective calls:
        gtUInt64 totalNumberOfCalls = functionEnumStatistics._amountOfTimesUsed;
        gtUInt64 amountOfEffectiveCalls = functionEnumStatistics._amountOfTimesUsed - functionEnumStatistics._amountOfRedundantTimesUsed;

        // Add this item number of calls to total calls counter:
        _totalAmountOfStateChangeFunctionCalls += totalNumberOfCalls;
        _totalAmountOfEffectiveCalls += amountOfEffectiveCalls;
        _totalAmountOfRedundantCalls += functionEnumStatistics._amountOfRedundantTimesUsed;

        if (functionEnumStatistics._amountOfTimesUsed < functionEnumStatistics._amountOfRedundantTimesUsed)
        {
            gtString assertStr = L"Negative number of effective function calls:";
            assertStr.append(functionName);
            GT_ASSERT_EX(false, assertStr.asCharArray());
        }

        gtString totalCallsAmountString;
        gtString effectiveCallsAmountString;
        gtString redundantCallsAmountString;
        gtString redundantCallsPercentageString;

        // Build the numbers strings:
        totalCallsAmountString.appendFormattedString(L"%ld", totalNumberOfCalls);
        totalCallsAmountString.addThousandSeperators();
        effectiveCallsAmountString.appendFormattedString(L"%ld", amountOfEffectiveCalls);
        effectiveCallsAmountString.addThousandSeperators();
        redundantCallsAmountString.appendFormattedString(L"%ld", functionEnumStatistics._amountOfRedundantTimesUsed);
        redundantCallsAmountString.addThousandSeperators();

        float percentageRedundantCalls = 0;

        if (totalNumberOfCalls > 0)
        {
            percentageRedundantCalls = functionEnumStatistics._amountOfRedundantTimesUsed;
            percentageRedundantCalls *= 100;
            percentageRedundantCalls /= totalNumberOfCalls;
            redundantCallsPercentageString.appendFormattedString(L"%.2f", percentageRedundantCalls);
        }
        else
        {
            redundantCallsPercentageString.append(L"N/A");
        }

        QStringList list;

        // Insert the item (enumerator) into the list:
        list << acGTStringToQString(functionNameEnum);
        list << acGTStringToQString(totalCallsAmountString);
        list << acGTStringToQString(effectiveCallsAmountString);
        list << acGTStringToQString(redundantCallsAmountString);
        list << acGTStringToQString(redundantCallsPercentageString);

        // Get the function icon index & type:
        int iconIndex = -1;
        afIconType iconType = AF_ICON_NONE;
        getFunctionIcon(percentageRedundantCalls, iconIndex, iconType);

        // Get the icon pixmap:
        QPixmap* pPixmap = icon(iconIndex);

        // Add the item data:
        gdStatisticsViewItemData* pItemData = new gdStatisticsViewItemData;


        pItemData->_functionName = functionName;
        pItemData->_totalAmountOfTimesCalled = functionEnumStatistics._amountOfTimesUsed + functionEnumStatistics._amountOfRedundantTimesUsed;
        pItemData->_amountOfRedundantTimesCalled = functionEnumStatistics._amountOfRedundantTimesUsed;
        pItemData->_amountOfEffectiveTimesCalled = functionEnumStatistics._amountOfTimesUsed;
        pItemData->_percentageOfRedundantTimesCalled = percentageRedundantCalls;
        pItemData->_functionId = functionId;
        pItemData->_iconType = iconType;

        addRow(list, pItemData, false, Qt::Unchecked, pPixmap);
    }
    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gdStateChangeStatisticsView::addStatisticsItem
// Description: Adds function counter statistics item to the list
// Arguments: const apFunctionCallStatistics& functionStatistics - the function statistics object
//            gtUInt64 amountOfCalls - the function type amount of calls
//            gtUInt64 averageAmountOfCallsPerFrame - the function type average amount of calls per frame
// Return Val: bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        21/7/2008
// ---------------------------------------------------------------------------
bool gdStateChangeStatisticsView::addFunctionToList(const apFunctionCallStatistics& functionStatistics)
{
    bool retVal = true;

    // Get the function ID:
    apMonitoredFunctionId functionId = functionStatistics._functionId;

    // Get the function name:
    gtString functionName;
    retVal = retVal && gaGetMonitoredFunctionName(functionId, functionName);
    GT_IF_WITH_ASSERT(retVal)
    {
        gtString totalCallsAmountString;
        gtString effectiveCallsAmountString;
        gtString redundantCallsAmountString;
        gtString redundantCallsPercentageString;

        // Calculate total number of calls and #effective calls:
        gtUInt64 totalNumberOfCalls = functionStatistics._amountOfTimesCalled;
        gtUInt64 amountOfEffectiveCalls = functionStatistics._amountOfTimesCalled - functionStatistics._amountOfRedundantTimesCalled;

        if (functionStatistics._amountOfTimesCalled < functionStatistics._amountOfRedundantTimesCalled)
        {
            gtString assertStr = L"Negative number of effective function calls:";
            assertStr.append(functionName);
            GT_ASSERT_EX(false, assertStr.asCharArray());
        }

        // Add this item number of calls to total calls counter:
        _totalAmountOfStateChangeFunctionCalls += totalNumberOfCalls;
        _totalAmountOfEffectiveCalls += amountOfEffectiveCalls;
        _totalAmountOfRedundantCalls += functionStatistics._amountOfRedundantTimesCalled;

        // Build the numbers strings:
        totalCallsAmountString.appendFormattedString(L"%ld", totalNumberOfCalls);
        totalCallsAmountString.addThousandSeperators();
        effectiveCallsAmountString.appendFormattedString(L"%ld", amountOfEffectiveCalls);
        effectiveCallsAmountString.addThousandSeperators();
        redundantCallsAmountString.appendFormattedString(L"%ld", functionStatistics._amountOfRedundantTimesCalled);
        redundantCallsAmountString.addThousandSeperators();

        float percentageRedundantCalls = 0;

        if (totalNumberOfCalls > 0)
        {
            percentageRedundantCalls = functionStatistics._amountOfRedundantTimesCalled;
            percentageRedundantCalls *= 100;
            percentageRedundantCalls /= totalNumberOfCalls;
            redundantCallsPercentageString.appendFormattedString(L"%.2f", percentageRedundantCalls);
        }
        else
        {
            redundantCallsPercentageString.append(L"N/A");
        }

        QStringList list;

        // Insert the item (enumerator) into the list:
        list << acGTStringToQString(functionName);
        list << acGTStringToQString(totalCallsAmountString);
        list << acGTStringToQString(effectiveCallsAmountString);
        list << acGTStringToQString(redundantCallsAmountString);
        list << acGTStringToQString(redundantCallsPercentageString);

        // determine which icon to add:
        // Get the function icon index & type:
        int iconIndex = -1;
        afIconType iconType = AF_ICON_NONE;
        getFunctionIcon(percentageRedundantCalls, iconIndex, iconType);
        QPixmap* pPixmap = icon(iconIndex);

        // Add the item data:
        gdStatisticsViewItemData* pItemData = new gdStatisticsViewItemData;


        pItemData->_functionName = functionName;
        pItemData->_totalAmountOfTimesCalled = totalNumberOfCalls;
        pItemData->_amountOfRedundantTimesCalled = functionStatistics._amountOfRedundantTimesCalled;
        pItemData->_amountOfEffectiveTimesCalled = amountOfEffectiveCalls;
        pItemData->_percentageOfRedundantTimesCalled = percentageRedundantCalls;
        pItemData->_functionId = functionId;
        pItemData->_iconType = iconType;

        addRow(list, pItemData, false, Qt::Unchecked, pPixmap);

        retVal = true;
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gdStateChangeStatisticsView::onSaveStatisticsData
// Description: Export the state change statistics data to a file
// Author:      Sigal Algranaty
// Date:        4/8/2008
// ---------------------------------------------------------------------------
void gdStateChangeStatisticsView::onSaveStatisticsData()
{
    // Open Save Dialog:
    afApplicationCommands* pApplicationCommands = afApplicationCommands::instance();
    GT_IF_WITH_ASSERT(pApplicationCommands != NULL)
    {
        // Show the save file as dialog:
        QString csvFilePathStr;
        bool rc = pApplicationCommands->ShowQTSaveCSVFileDialog(csvFilePathStr, GD_STR_saveStateChageStatisticsFileName, this);

        if (rc)
        {
            // Run the Save Function Calls Statistics command:
            gdSaveStateChangeStatisticsCommand saveStateChangeStatisticsCommand(acQStringToGTString(csvFilePathStr));
            rc = saveStateChangeStatisticsCommand.execute();

            if (!rc)
            {
                acMessageBox::instance().critical(AF_STR_ErrorA, GD_STR_ErrorMessageStateChangeTotalStatisticsFailed, QMessageBox::Ok);
            }
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        gdStateChangeStatisticsView::addTotalItemToList
// Description: Adds the "Total" item to the list control.
// Author:      Yaki Tebeka
// Date:        16/4/2006
// ---------------------------------------------------------------------------
void gdStateChangeStatisticsView::addTotalItemToList()
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
        // Calculate the % of redundant calls:
        float redundantCallsPercentage = ((float)_totalAmountOfRedundantCalls / (float)_totalAmountOfStateChangeFunctionCalls) * 100.0f;

        // Build displayed strings:
        gtASCIIString totalString;
        gtASCIIString totalAmountOfFunctionCallsString;
        gtASCIIString totalAmountOfEffectiveCallsString;
        gtASCIIString totalAmountOfRedundantCallsString;
        gtASCIIString redundantPrecentageString;

        totalString.appendFormattedString("Total (%d items)", listSize);

        totalAmountOfFunctionCallsString.appendFormattedString("%ld", _totalAmountOfStateChangeFunctionCalls);
        totalAmountOfFunctionCallsString.addThousandSeperators();
        totalAmountOfEffectiveCallsString.appendFormattedString("%ld", _totalAmountOfEffectiveCalls);
        totalAmountOfEffectiveCallsString.addThousandSeperators();
        totalAmountOfRedundantCallsString.appendFormattedString("%ld", _totalAmountOfRedundantCalls);
        totalAmountOfRedundantCallsString.addThousandSeperators();
        redundantPrecentageString.appendFormattedString("%.2f", redundantCallsPercentage);

        QStringList list;
        list << totalString.asCharArray();
        list << totalAmountOfFunctionCallsString.asCharArray();
        list << totalAmountOfEffectiveCallsString.asCharArray();
        list << totalAmountOfRedundantCallsString.asCharArray();
        list << redundantPrecentageString.asCharArray();

        addRow(list, NULL);
        setItemBold(rowCount() - 1, -1);
    }
}

// ---------------------------------------------------------------------------
// Name:        gdStateChangeStatisticsView::initializeImageList
// Description: Create and populate the image list for this item
// Author:      Uri Shomroni
// Date:        6/8/2008
// ---------------------------------------------------------------------------
void gdStateChangeStatisticsView::initializeImageList()
{
    QPixmap* pEmptyIcon = new QPixmap;
    acSetIconInPixmap(*pEmptyIcon, AC_ICON_EMPTY);

    QPixmap* pYellowWarningIcon = new QPixmap;
    acSetIconInPixmap(*pYellowWarningIcon, AC_ICON_WARNING_YELLOW);

    QPixmap* pOrangeWarningIcon = new QPixmap;
    acSetIconInPixmap(*pOrangeWarningIcon, AC_ICON_WARNING_ORANGE);

    QPixmap* pRedWarningIcon = new QPixmap;
    acSetIconInPixmap(*pRedWarningIcon, AC_ICON_WARNING_RED);

    _listIconsVec.push_back(pEmptyIcon);            // 0
    _listIconsVec.push_back(pYellowWarningIcon);    // 1
    _listIconsVec.push_back(pOrangeWarningIcon);    // 2
    _listIconsVec.push_back(pRedWarningIcon);       // 3
}


// ---------------------------------------------------------------------------
// Name:        gdStateChangeStatisticsView::getFunctionIcon
// Description:
// Arguments: float percentageRedundantCalls
//            int& iconIndex
//            afIconType& iconType
// Return Val: void
// Author:      Sigal Algranaty
// Date:        25/5/2009
// ---------------------------------------------------------------------------
void gdStateChangeStatisticsView::getFunctionIcon(float percentageRedundantCalls, int& iconIndex, afIconType& iconType)
{
    // determine which icon to add:
    iconIndex = -1;
    iconType = AF_ICON_NONE;

    if (percentageRedundantCalls <= GD_YELLOW_WARNING_MIN_PCT)
    {
        iconIndex = 0;
        iconType = AF_ICON_INFO;
    }
    else if (percentageRedundantCalls <= GD_ORANGE_WARNING_MIN_PCT)
    {
        iconIndex = 1;
        iconType = AF_ICON_WARNING1;
    }
    else if (percentageRedundantCalls <= GD_RED_WARNING_MIN_PCT)
    {
        iconIndex = 2;
        iconType = AF_ICON_WARNING2;
    }
    else
    {
        iconIndex = 3;
        iconType = AF_ICON_WARNING3;
    }
}

// ---------------------------------------------------------------------------
// Name:        gdStateChangeStatisticsView::getItemProperties
// Description: Return an item properties
// Arguments: int itemIndex
//            gtUInt64& effectiveCallsAmount
//            gtUInt64& redundantCallsAmount
//            unsigned long &effectiveColor
//            unsigned long &redundantColor
// Return Val: bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        26/5/2009
// ---------------------------------------------------------------------------
bool gdStateChangeStatisticsView::getItemProperties(int itemIndex, gtUInt64& effectiveCallsAmount, gtUInt64& redundantCallsAmount, long& effectiveColor, long& redundantColor)
{
    bool retVal = false;
    effectiveColor = -1;
    redundantColor = -1;

    // Sanity check:
    GT_IF_WITH_ASSERT((itemIndex >= 0) && (itemIndex < rowCount() - 1))
    {
        // Get item data:
        gdStatisticsViewItemData* pItemData = gdStatisticsViewBase::getItemData(itemIndex);
        GT_IF_WITH_ASSERT(pItemData != NULL)
        {
            effectiveCallsAmount = pItemData->_amountOfEffectiveTimesCalled;
            redundantCallsAmount = pItemData->_amountOfRedundantTimesCalled;

            if (effectiveCallsAmount > 0)
            {
                effectiveColor = _chartColors[0];
            }

            if (redundantCallsAmount > 0)
            {
                redundantColor = _chartColors[1];
            }

            retVal = true;
        }
    }
    return retVal;
}

