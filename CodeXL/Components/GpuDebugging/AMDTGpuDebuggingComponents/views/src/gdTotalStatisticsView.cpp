//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gdTotalStatisticsView.cpp
///
//==================================================================================

//------------------------------ gdTotalStatisticsView.cpp ------------------------------

// Qt
#include <AMDTApplicationComponents/Include/acQtIncludes.h>

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTBaseTools/Include/gtPtrVector.h>
#include <AMDTAPIClasses/Include/apFunctionCallStatistics.h>
#include <AMDTAPIClasses/Include/apFunctionType.h>
#include <AMDTAPIClasses/Include/apStatistics.h>
#include <AMDTAPIClasses/Include/apFrameTerminators.h>
#include <AMDTAPIClasses/Include/Events/apEventsHandler.h>
#include <AMDTApiFunctions/Include/gaGRApiFunctions.h>
#include <AMDTApplicationComponents/Include/acColours.h>
#include <AMDTApplicationComponents/Include/acIcons.h>
#include <AMDTApplicationComponents/Include/acFunctions.h>

// AMDTApplicationFramework:
#include <AMDTApplicationFramework/Include/afAppStringConstants.h>
#include <AMDTApplicationFramework/src/afUtils.h>

// Local:
#include <AMDTGpuDebuggingComponents/Include/gdCommandIDs.h>
#include <AMDTGpuDebuggingComponents/Include/gdStringConstants.h>
#include <AMDTGpuDebuggingComponents/Include/gdGDebuggerGlobalVariablesManager.h>
#include <AMDTApplicationFramework/Include/afGlobalVariableChangedEvent.h>
#include <AMDTGpuDebuggingComponents/Include/gdAidFunctions.h>
#include <AMDTGpuDebuggingComponents/Include/views/gdTotalStatisticsView.h>
#include <AMDTGpuDebuggingComponents/Include/views/gdStatisticsViewBase.h>
#include <AMDTGpuDebuggingComponents/Include/commands/gdSaveTotalStatisticsCommand.h>


// ---------------------------------------------------------------------------
// Name:        gdTotalStatisticsView::gdTotalStatisticsView
// Description: Constructor.
// Arguments:   parent - My parent window.
// Author:      Avi Shapira
// Date:        29/1/2006
// ---------------------------------------------------------------------------
gdTotalStatisticsView::gdTotalStatisticsView(QWidget* pParent)
    : gdStatisticsViewBase(pParent, GD_STATISTICS_VIEW_TOTAL_INDEX, GD_STR_StatisticsViewerTotalStatisticsShortName),
      _pFunctionTypeStrings(NULL), _pFunctionTypeAmountsByType(NULL),
      _pFunctionTypeAverageAmountsByType(NULL), _totalAmountOfFunctionCalls(0), _totalAmountOfFunctionCallsInFullFrames(0),
      _amountOfFullFrames(0)
{
    // Call init function of base class:
    init();

    // Initialize function type to function amount index mapping:
    initFunctionTypeMapping();
}

// ---------------------------------------------------------------------------
// Name:        gdTotalStatisticsView::~gdTotalStatisticsView
// Description: Destructor
// Author:      Avi Shapira
// Date:        29/1/2006
// ---------------------------------------------------------------------------
gdTotalStatisticsView::~gdTotalStatisticsView()
{
    // Delete the amount of calls pointers:
    if (_pFunctionTypeAverageAmountsByType != NULL)
    {
        delete[] _pFunctionTypeAverageAmountsByType;
    }

    if (_pFunctionTypeAmountsByType != NULL)
    {
        delete[] _pFunctionTypeAmountsByType;
    }

    if (_pFunctionTypeStrings != NULL)
    {
        delete[] _pFunctionTypeStrings;
    }
}



// ---------------------------------------------------------------------------
// Name:        gdTotalStatisticsView::initFunctionTypeMapping
// Description:
// Return Val: void
// Author:      Sigal Algranaty
// Date:        27/7/2008
// ---------------------------------------------------------------------------
void gdTotalStatisticsView::initFunctionTypeMapping()
{
    // Allocate an array for the amount of calls:
    _pFunctionTypeAmountsByType = new gtUInt64[GD_NUMBER_OF_FUNCTION_TYPES_INDICES];

    // Allocate an array for the average amount of calls:
    _pFunctionTypeAverageAmountsByType = new gtUInt64[GD_NUMBER_OF_FUNCTION_TYPES_INDICES];

    for (int i = 0; i < GD_NUMBER_OF_FUNCTION_TYPES_INDICES; i++)
    {
        _pFunctionTypeAmountsByType[i] = 0;
        _pFunctionTypeAverageAmountsByType[i] = 0;
    }

    // Initialize the function type strings pointer:
    _pFunctionTypeStrings = new gtString[GD_NUMBER_OF_FUNCTION_TYPES_INDICES];

    // Initialize function type strings:
    _pFunctionTypeStrings[GD_DRAW_FUNCTIONS_INDEX] = GD_STR_TotalStatisticsViewerDrawFunctions;
    _pFunctionTypeStrings[GD_RASTER_FUNCTIONS_INDEX] = GD_STR_TotalStatisticsViewerRasterFunctions;
    _pFunctionTypeStrings[GD_GET_FUNCTIONS_INDEX] = GD_STR_TotalStatisticsViewerGetFunctions;
    _pFunctionTypeStrings[GD_STATE_CHANGE_FUNCTIONS_INDEX] = GD_STR_TotalStatisticsViewerStateChangeFunctions;
    _pFunctionTypeStrings[GD_DEPRECATED_FUNCTIONS_INDEX] = GD_STR_TotalStatisticsViewerDeprecatedFunctions;
    _pFunctionTypeStrings[GD_EFFECTIVE_STATE_CHANGE_FUNCTIONS_INDEX] = GD_STR_TotalStatisticsViewerEffectiveStateChangeFunctions;
    _pFunctionTypeStrings[GD_REDUNDANT_STATE_CHANGE_FUNCTIONS_INDEX] = GD_STR_TotalStatisticsViewerRedundantStateChangeFunctions;
    _pFunctionTypeStrings[GD_PROGRAM_AND_SHADERS_FUNCTIONS_INDEX] = GD_STR_TotalStatisticsViewerProgramsAndShadersFunctions;
    _pFunctionTypeStrings[GD_PROGRAM_AND_KERNELS_FUNCTIONS_INDEX] = GD_STR_TotalStatisticsViewerProgramsAndKernelsFunctions;
    _pFunctionTypeStrings[GD_TEXTURE_FUNCTIONS_INDEX] = GD_STR_TotalStatisticsViewerTextureFunctions;
    _pFunctionTypeStrings[GD_MATRIX_FUNCTIONS_INDEX] = GD_STR_TotalStatisticsViewerMatrixFunctions;
    _pFunctionTypeStrings[GD_NAME_FUNCTIONS_INDEX] = GD_STR_TotalStatisticsViewerNameFunctions;
    _pFunctionTypeStrings[GD_QUERY_FUNCTIONS_INDEX] = GD_STR_TotalStatisticsViewerQueryFunctions;
    _pFunctionTypeStrings[GD_BUFFER_FUNCTIONS_INDEX] = GD_STR_TotalStatisticsViewerBufferFunctions;
    _pFunctionTypeStrings[GD_BUFFERS_AND_IMAGE_FUNCTIONS_INDEX] = GD_STR_TotalStatisticsViewerBufferImageFunctions;
    _pFunctionTypeStrings[GD_QUEUE_FUNCTIONS_INDEX] = GD_STR_TotalStatisticsViewerQueueFunctions;
    _pFunctionTypeStrings[GD_SYNC_FUNCTIONS_INDEX] = GD_STR_TotalStatisticsViewerSyncFunctions;
    _pFunctionTypeStrings[GD_FEEDBACK_FUNCTIONS_INDEX] = GD_STR_TotalStatisticsViewerFeedbackFunctions;
    _pFunctionTypeStrings[GD_VERTEX_ARRAY_FUNCTIONS_INDEX] = GD_STR_TotalStatisticsViewerVertexArrayFunctions;
    _pFunctionTypeStrings[GD_DEBUG_FUNCTIONS_INDEX] = GD_STR_TotalStatisticsViewerDebugFunctions;
    _pFunctionTypeStrings[GD_CL_NULL_CONTEXT_FUNCTIONS_INDEX] = GD_STR_TotalStatisticsViewerCLNullContextFunctions;
    _pFunctionTypeStrings[GD_GL_NULL_CONTEXT_FUNCTIONS_INDEX] = GD_STR_TotalStatisticsViewerGLNullContextFunctions;
    _pFunctionTypeStrings[GD_CL_CONTEXT_BOUND_FUNCTIONS_INDEX] = GD_STR_TotalStatisticsViewerCLContextBoundFunctions;
    _pFunctionTypeStrings[GD_GL_CONTEXT_BOUND_FUNCTIONS_INDEX] = GD_STR_TotalStatisticsViewerGLContextBoundFunctions;
}

// ---------------------------------------------------------------------------
// Name:        gdTotalStatisticsView::initListCtrlColumns
// Description: Add the columns attribute to the base class members
// Author:      Sigal Algranaty
// Date:        21/7/2008
// ---------------------------------------------------------------------------
void gdTotalStatisticsView::initListCtrlColumns()
{
    // Add the "Total Statistics View" columns titles:
    _listControlColumnTitles.push_back(GD_STR_TotalStatisticsViewerColumn1Title);
    _listControlColumnTitles.push_back(GD_STR_TotalStatisticsViewerColumn2Title);
    _listControlColumnTitles.push_back(GD_STR_TotalStatisticsViewerColumn3Title);
    _listControlColumnTitles.push_back(GD_STR_TotalStatisticsViewerColumn4Title);
    _listControlColumnTitles.push_back(GD_STR_TotalStatisticsViewerColumn5Title);

    // Add the "Total Statistics View" columns widths:
    _listControlColumnWidths.push_back(0.25f);
    _listControlColumnWidths.push_back(0.10f);
    _listControlColumnWidths.push_back(0.10f);
    _listControlColumnWidths.push_back(0.10f);
    _listControlColumnWidths.push_back(0.10f);
    _widestColumnIndex = 4;
    _initialSortColumnIndex = 0;
    _sortInfo._sortOrder = Qt::DescendingOrder;
}

// ---------------------------------------------------------------------------
// Name:        gdTotalStatisticsView::increaseCounterByType
// Description: Checks the function type, and increase the counter specific type
// Arguments: const apFunctionCallStatistics& functionCallStatisticsData
// Return Val: bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        21/7/2008
// ---------------------------------------------------------------------------
bool gdTotalStatisticsView::increaseCounterByType(const apFunctionCallStatistics& functionCallStatisticsData)
{
    bool retVal = false;

    // Get the function ID:
    apMonitoredFunctionId functionId = functionCallStatisticsData._functionId;

    // Get the function type:
    unsigned int functionTypeAsUInt = 0;
    bool rc = gaGetMonitoredFunctionType(functionId, functionTypeAsUInt);
    apFunctionType funcType = (apFunctionType)functionTypeAsUInt;

    // Get the function API type:
    unsigned int functionAPITypeAsUInt = 0;
    rc = gaGetMonitoredFunctionAPIType(functionId, functionAPITypeAsUInt);
    apAPIType functionAPIType = (apAPIType)functionAPITypeAsUInt;

    // Increase the relevant counter:
    GT_IF_WITH_ASSERT(rc)
    {
        if (funcType & AP_DRAW_FUNC)
        {
            _pFunctionTypeAmountsByType[GD_DRAW_FUNCTIONS_INDEX] += functionCallStatisticsData._amountOfTimesCalled;
            _pFunctionTypeAverageAmountsByType[GD_DRAW_FUNCTIONS_INDEX] += functionCallStatisticsData._averageAmountPerFrame;
        }

        if (funcType & AP_RASTER_FUNC)
        {
            _pFunctionTypeAmountsByType[GD_RASTER_FUNCTIONS_INDEX] += functionCallStatisticsData._amountOfTimesCalled;
            _pFunctionTypeAverageAmountsByType[GD_RASTER_FUNCTIONS_INDEX] += functionCallStatisticsData._averageAmountPerFrame;
        }

        if (funcType & AP_GET_FUNC)
        {
            _pFunctionTypeAmountsByType[GD_GET_FUNCTIONS_INDEX] += functionCallStatisticsData._amountOfTimesCalled;
            _pFunctionTypeAverageAmountsByType[GD_GET_FUNCTIONS_INDEX] += functionCallStatisticsData._averageAmountPerFrame;
        }

        if (funcType & AP_STATE_CHANGE_FUNC)
        {
            _pFunctionTypeAmountsByType[GD_STATE_CHANGE_FUNCTIONS_INDEX] += functionCallStatisticsData._amountOfTimesCalled;
            _pFunctionTypeAverageAmountsByType[GD_STATE_CHANGE_FUNCTIONS_INDEX] += functionCallStatisticsData._averageAmountPerFrame;

            // For state change functions, count separately the redundant calls:
            if (_executionMode == AP_ANALYZE_MODE)
            {
                _pFunctionTypeAmountsByType[GD_REDUNDANT_STATE_CHANGE_FUNCTIONS_INDEX] += functionCallStatisticsData._amountOfRedundantTimesCalled;
                _pFunctionTypeAverageAmountsByType[GD_REDUNDANT_STATE_CHANGE_FUNCTIONS_INDEX] += functionCallStatisticsData._averageRedundantAmountPerFrame;
                _pFunctionTypeAmountsByType[GD_EFFECTIVE_STATE_CHANGE_FUNCTIONS_INDEX] += functionCallStatisticsData._amountOfTimesCalled - functionCallStatisticsData._amountOfRedundantTimesCalled;
                _pFunctionTypeAverageAmountsByType[GD_EFFECTIVE_STATE_CHANGE_FUNCTIONS_INDEX] += functionCallStatisticsData._averageAmountPerFrame - functionCallStatisticsData._averageRedundantAmountPerFrame;
            }
        }

        if (funcType & AP_DEPRECATED_FUNC)
        {
            // OpenGL ES does not have deprecation:
            _pFunctionTypeAmountsByType[GD_DEPRECATED_FUNCTIONS_INDEX] += functionCallStatisticsData._amountOfTimesCalled;
            _pFunctionTypeAverageAmountsByType[GD_DEPRECATED_FUNCTIONS_INDEX] += functionCallStatisticsData._averageAmountPerFrame;
        }

        if (funcType & AP_PROGRAM_SHADER_FUNC)
        {
            _pFunctionTypeAmountsByType[GD_PROGRAM_AND_SHADERS_FUNCTIONS_INDEX] += functionCallStatisticsData._amountOfTimesCalled;
            _pFunctionTypeAverageAmountsByType[GD_PROGRAM_AND_SHADERS_FUNCTIONS_INDEX] += functionCallStatisticsData._averageAmountPerFrame;
        }

        if (funcType & AP_TEXTURE_FUNC)
        {
            _pFunctionTypeAmountsByType[GD_TEXTURE_FUNCTIONS_INDEX] += functionCallStatisticsData._amountOfTimesCalled;
            _pFunctionTypeAverageAmountsByType[GD_TEXTURE_FUNCTIONS_INDEX] += functionCallStatisticsData._averageAmountPerFrame;
        }

        if (funcType & AP_MATRIX_FUNC)
        {
            _pFunctionTypeAmountsByType[GD_MATRIX_FUNCTIONS_INDEX] += functionCallStatisticsData._amountOfTimesCalled;
            _pFunctionTypeAverageAmountsByType[GD_MATRIX_FUNCTIONS_INDEX] += functionCallStatisticsData._averageAmountPerFrame;
        }

        if (funcType & AP_NAME_FUNC)
        {
            _pFunctionTypeAmountsByType[GD_NAME_FUNCTIONS_INDEX] += functionCallStatisticsData._amountOfTimesCalled;
            _pFunctionTypeAverageAmountsByType[GD_NAME_FUNCTIONS_INDEX] += functionCallStatisticsData._averageAmountPerFrame;

        }

        if (funcType & AP_QUERY_FUNC)
        {
            _pFunctionTypeAmountsByType[GD_QUERY_FUNCTIONS_INDEX] += functionCallStatisticsData._amountOfTimesCalled;
            _pFunctionTypeAverageAmountsByType[GD_QUERY_FUNCTIONS_INDEX] += functionCallStatisticsData._averageAmountPerFrame;

        }

        if (funcType & AP_BUFFER_FUNC)
        {
            _pFunctionTypeAmountsByType[GD_BUFFER_FUNCTIONS_INDEX] += functionCallStatisticsData._amountOfTimesCalled;
            _pFunctionTypeAverageAmountsByType[GD_BUFFER_FUNCTIONS_INDEX] += functionCallStatisticsData._averageAmountPerFrame;
        }

        if (funcType & AP_BUFFER_IMAGE_FUNC)
        {
            _pFunctionTypeAmountsByType[GD_BUFFERS_AND_IMAGE_FUNCTIONS_INDEX] += functionCallStatisticsData._amountOfTimesCalled;
            _pFunctionTypeAverageAmountsByType[GD_BUFFERS_AND_IMAGE_FUNCTIONS_INDEX] += functionCallStatisticsData._averageAmountPerFrame;

        }

        if (funcType & AP_QUEUE_FUNC)
        {
            _pFunctionTypeAmountsByType[GD_QUEUE_FUNCTIONS_INDEX] += functionCallStatisticsData._amountOfTimesCalled;
            _pFunctionTypeAverageAmountsByType[GD_QUEUE_FUNCTIONS_INDEX] += functionCallStatisticsData._averageAmountPerFrame;
        }

        if (funcType & AP_SYNCHRONIZATION_FUNC)
        {
            _pFunctionTypeAmountsByType[GD_SYNC_FUNCTIONS_INDEX] += functionCallStatisticsData._amountOfTimesCalled;
            _pFunctionTypeAverageAmountsByType[GD_SYNC_FUNCTIONS_INDEX] += functionCallStatisticsData._averageAmountPerFrame;
        }

        if (funcType & AP_FEEDBACK_FUNC)
        {
            _pFunctionTypeAmountsByType[GD_FEEDBACK_FUNCTIONS_INDEX] += functionCallStatisticsData._amountOfTimesCalled;
            _pFunctionTypeAverageAmountsByType[GD_FEEDBACK_FUNCTIONS_INDEX] += functionCallStatisticsData._averageAmountPerFrame;
        }

        if (funcType & AP_VERTEX_ARRAY_FUNC)
        {
            _pFunctionTypeAmountsByType[GD_VERTEX_ARRAY_FUNCTIONS_INDEX] += functionCallStatisticsData._amountOfTimesCalled;
            _pFunctionTypeAverageAmountsByType[GD_VERTEX_ARRAY_FUNCTIONS_INDEX] += functionCallStatisticsData._averageAmountPerFrame;
        }

        if (funcType & AP_DEBUG_FUNC)
        {
            _pFunctionTypeAmountsByType[GD_DEBUG_FUNCTIONS_INDEX] += functionCallStatisticsData._amountOfTimesCalled;
            _pFunctionTypeAverageAmountsByType[GD_DEBUG_FUNCTIONS_INDEX] += functionCallStatisticsData._averageAmountPerFrame;
        }

        if (funcType & AP_PROGRAM_KERNEL_FUNC)
        {
            _pFunctionTypeAmountsByType[GD_PROGRAM_AND_KERNELS_FUNCTIONS_INDEX] += functionCallStatisticsData._amountOfTimesCalled;
            _pFunctionTypeAverageAmountsByType[GD_PROGRAM_AND_KERNELS_FUNCTIONS_INDEX] += functionCallStatisticsData._averageAmountPerFrame;
        }

        bool isGLFunc = (0 != (functionAPIType & (AP_OPENGL_API_FUNC_MASK)));
        bool isCLFunc = (0 != (functionAPIType & (AP_OPENCL_API_FUNC_MASK)));

        if (funcType & AP_NULL_CONTEXT_FUNCTION)
        {
            if (isCLFunc)
            {
                _pFunctionTypeAmountsByType[GD_CL_NULL_CONTEXT_FUNCTIONS_INDEX] += functionCallStatisticsData._amountOfTimesCalled;
                _pFunctionTypeAverageAmountsByType[GD_CL_NULL_CONTEXT_FUNCTIONS_INDEX] += functionCallStatisticsData._averageAmountPerFrame;
            }
            else if (isGLFunc)
            {
                _pFunctionTypeAmountsByType[GD_GL_NULL_CONTEXT_FUNCTIONS_INDEX] += functionCallStatisticsData._amountOfTimesCalled;
                _pFunctionTypeAverageAmountsByType[GD_GL_NULL_CONTEXT_FUNCTIONS_INDEX] += functionCallStatisticsData._averageAmountPerFrame;
            }
            else
            {
                // Unknown API:
                GT_ASSERT(false);
            }
        }
        else
        {
            if (isCLFunc)
            {
                // Context bound functions is a function that is not a null context function:
                _pFunctionTypeAmountsByType[GD_CL_CONTEXT_BOUND_FUNCTIONS_INDEX] += functionCallStatisticsData._amountOfTimesCalled;
                _pFunctionTypeAverageAmountsByType[GD_CL_CONTEXT_BOUND_FUNCTIONS_INDEX] += functionCallStatisticsData._averageAmountPerFrame;
            }
            else if (isGLFunc)
            {
                // Context bound functions is a function that is not a null context function:
                _pFunctionTypeAmountsByType[GD_GL_CONTEXT_BOUND_FUNCTIONS_INDEX] += functionCallStatisticsData._amountOfTimesCalled;
                _pFunctionTypeAverageAmountsByType[GD_GL_CONTEXT_BOUND_FUNCTIONS_INDEX] += functionCallStatisticsData._averageAmountPerFrame;
            }
            else
            {
                // Unknown API:
                GT_ASSERT(false);
            }
        }

        retVal = true;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gdTotalStatisticsView::addFunctionTypeCountersToList
// Description: For each function type, add it's counting to the list
// Return Val: bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        21/7/2008
// ---------------------------------------------------------------------------
bool gdTotalStatisticsView::addFunctionTypeCountersToList()
{
    bool retVal = true;

    // For each function type, add the function type to the list:
    for (int i = 0; i < GD_NUMBER_OF_FUNCTION_TYPES_INDICES; i++)
    {
        int iconIndex = 0;

        if (i == GD_GET_FUNCTIONS_INDEX)
        {
            if (_activeContextId.isOpenGLContext())
            {
                iconIndex = 1;
            }
        }

        if (i == GD_REDUNDANT_STATE_CHANGE_FUNCTIONS_INDEX)
        {
            iconIndex = 2;
        }

        if ((i == GD_DEPRECATED_FUNCTIONS_INDEX) ||
            (i == GD_CL_CONTEXT_BOUND_FUNCTIONS_INDEX) ||
            (i == GD_GL_CONTEXT_BOUND_FUNCTIONS_INDEX))
        {
            iconIndex = 3;
        }

        // Convert the index to function type index:
        gdFuncCallsViewTypes functionType = (gdFuncCallsViewTypes)i;

        // Check if this function type should be displayed for this context type:
        bool shouldAddFunctionTypeItem = false;
        bool shouldAddUnavailableItem = false;
        gtString unavailableMessage;
        shouldShowFunctionType(functionType, shouldAddFunctionTypeItem, shouldAddUnavailableItem, unavailableMessage);

        // For non-analyze mode, add the effective and redundant state change category with unavailable message:
        if (shouldAddFunctionTypeItem)
        {
            // Add the function type calls amounts to the list:
            retVal = retVal && addStatisticsItem(_pFunctionTypeStrings[i], _pFunctionTypeAmountsByType[i], _pFunctionTypeAverageAmountsByType[i], iconIndex, functionType);
        }
        else if (shouldAddUnavailableItem)
        {
            // NOTICE: Sigal 26/6/2011 When analyze mode is supported, we should remove this comment!
            // retVal = retVal && addUnavailableItem(unavailableMessage, functionType);
        }
    }


    return retVal;
}
// ---------------------------------------------------------------------------
// Name:        gdTotalStatisticsView::updateFunctionCallsStatisticsList
// Description: Update the current statistics into the listCTRL
// Arguments: const apStatistics& currentStatistics
// Return Val: void
// Author:      Sigal Algranaty
// Date:        21/7/2008
// ---------------------------------------------------------------------------
bool gdTotalStatisticsView::updateFunctionCallsStatisticsList(const apStatistics& currentStatistics)
{
    bool retVal = true;

    // Check the current execution mode:
    gaGetDebuggedProcessExecutionMode(_executionMode);

    // Clear all counters and data:
    clearAllStatisticsItems();

    // Enable the list
    setEnabled(true);

    _totalAmountOfFunctionCalls = 0;

    // Get the number of function calls statistics:
    int functionStatisticsAmount = currentStatistics.amountOfFunctionCallsStatistics();

    if (functionStatisticsAmount > 0)
    {
        // Calculate the total amount of function calls:
        for (int i = 0; i < functionStatisticsAmount; i++)
        {
            const apFunctionCallStatistics* pFunctionCallStatisticsData = NULL;
            bool rc = currentStatistics.getFunctionCallStatistics(i, pFunctionCallStatisticsData);
            GT_IF_WITH_ASSERT(rc && (pFunctionCallStatisticsData != NULL))
            {
                // Add the function to the total counter:
                gtUInt64 amountOfTimesCalled = pFunctionCallStatisticsData->_amountOfTimesCalled;
                _totalAmountOfFunctionCalls += amountOfTimesCalled;

                // Add the function to the specific function type counters:
                retVal = retVal && increaseCounterByType(*pFunctionCallStatisticsData);
            }
        }

        _amountOfFullFrames = currentStatistics.amountOfFullFrames();
        _totalAmountOfFunctionCallsInFullFrames = currentStatistics.amountOfFunctionCallsInFullFrames();

        // Calculate average count per frame for each function type:
        if (_amountOfFullFrames > 0)
        {
            for (int i = 0; i < GD_NUMBER_OF_FUNCTION_TYPES_INDICES; i++)
            {
                if (_amountOfFullFrames <= 0)
                {
                    _pFunctionTypeAverageAmountsByType[i] = 0;
                }
            }
        }

        // Add the function specific type counters to the list:
        bool rc = addFunctionTypeCountersToList();
        retVal = retVal && rc;

        // Add the total items to the list:
        addTotalItemToList();

        // Sort the item in the list control:
        sortItems(0, _sortInfo._sortOrder);

        // Update heading tool tips
        setListHeadingToolTips();
    }
    else
    {
        addRow(AF_STR_NotAvailableA);
    }


    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gdTotalStatisticsView::addStatisticsItem
// Description: Adds function counter statistics item to the list
// Arguments: gtString& functionTypeName
//            gtUInt64 amountOfCalls - the function type amount of calls
//            gtUInt64 averageAmountOfCallsPerFrameForType - the function type average amount of calls per frame
//            wxColour* pBGColor - the background color for the item. If null, do not color
// Return Val: bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        21/7/2008
// ---------------------------------------------------------------------------
bool gdTotalStatisticsView::addStatisticsItem(gtString functionTypeName, gtUInt64 amountOfCalls, gtUInt64 averageAmountOfCallsPerFrameForType, int iconIndex, gdFuncCallsViewTypes functionType)
{
    gtASCIIString amountOfTimesCalledString;
    gtASCIIString percentageCalledString;
    gtASCIIString amountOfTimesCalledPerFrameString;
    gtASCIIString percentagePerFrameString;
    amountOfTimesCalledString.appendFormattedString("%ld", amountOfCalls);
    amountOfTimesCalledString.addThousandSeperators();
    amountOfTimesCalledPerFrameString.appendFormattedString("%ld", averageAmountOfCallsPerFrameForType);
    amountOfTimesCalledPerFrameString.addThousandSeperators();

    float percentageCalled = 0;

    if (_totalAmountOfFunctionCalls > 0)
    {
        // Calculate the appearers percentage:
        percentageCalled = amountOfCalls;
        percentageCalled *= 100;
        percentageCalled /= _totalAmountOfFunctionCalls;
        percentageCalledString.appendFormattedString("%.2f", percentageCalled);
    }
    else
    {
        percentageCalledString.append(AF_STR_NotAvailableA);
    }

    gtUInt64 averageAmountOfTotalCallsPerFrame = 0;

    if (_amountOfFullFrames > 0)
    {
        averageAmountOfTotalCallsPerFrame = (gtUInt64)_totalAmountOfFunctionCallsInFullFrames / _amountOfFullFrames;
    }

    float percentagePerFrameCalled = 0;

    if (averageAmountOfTotalCallsPerFrame > 0)
    {
        percentagePerFrameCalled = averageAmountOfCallsPerFrameForType;
        percentagePerFrameCalled *= 100;
        percentagePerFrameCalled /= averageAmountOfTotalCallsPerFrame;
        percentagePerFrameString.appendFormattedString("%.2f", percentagePerFrameCalled);
    }
    else
    {
        // Check if this is an OpenCL context:
        bool isCLContext = (_activeContextId.isOpenCLContext());
        bool frameTerminatorsExist = true;

        if (isCLContext)
        {
            // Check if the project contain OpenCL frame terminators:
            const apDebugProjectSettings processCreationData = gdGDebuggerGlobalVariablesManager::instance().currentDebugProjectSettings();
            unsigned int frameTerminators = processCreationData.frameTerminatorsMask();
            frameTerminatorsExist = ((frameTerminators & AP_CL_GREMEDY_COMPUTATION_FRAME_TERMINATORS) ||
                                     (frameTerminators & AP_CL_FLUSH_TERMINATOR) ||
                                     (frameTerminators & AP_CL_FINISH_TERMINATOR) ||
                                     (frameTerminators & AP_CL_WAIT_FOR_EVENTS_TERMINATOR) ||
                                     (frameTerminators & AP_ALL_CL_FRAME_TERMINATORS));
        }

        if (frameTerminatorsExist)
        {
            percentagePerFrameString.append(AF_STR_NotAvailableA);
        }
        else
        {
            percentagePerFrameString.append(GD_STR_TotalStatisticsViewerNoFrameTerminators);
        }
    }

    // Insert the item:
    QStringList list;
    list << acGTStringToQString(functionTypeName);
    list << acGTASCIIStringToQString(percentageCalledString);
    list << acGTASCIIStringToQString(amountOfTimesCalledString);
    list << acGTASCIIStringToQString(percentagePerFrameString);
    list << acGTASCIIStringToQString(amountOfTimesCalledPerFrameString);

    // Get the icon:
    QPixmap* pIcon = icon(iconIndex);

    // Add the item data:
    gdStatisticsViewItemData* pItemData = new gdStatisticsViewItemData;

    pItemData->_functionType = functionType;
    pItemData->_totalAmountOfTimesCalled = amountOfCalls;
    pItemData->_averagePercentOfCalls = percentagePerFrameCalled;
    pItemData->_percentageOfTimesCalled = percentageCalled;
    pItemData->_averageAmountOfTimesCalled = averageAmountOfCallsPerFrameForType;

    // Add row to table:
    addRow(list, pItemData, false, Qt::Unchecked, pIcon);

    return true;
}

// ---------------------------------------------------------------------------
// Name:        gdTotalStatisticsView::addUnavailableItem
// Description: Adds an item to the list control with an unavailable string
// Arguments: gtString functionTypeName
// Return Val: bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        4/8/2008
// ---------------------------------------------------------------------------
bool gdTotalStatisticsView::addUnavailableItem(gtString functionTypeName, gdFuncCallsViewTypes functionType)
{
    QStringList unavailableStrList;

    unavailableStrList << AF_STR_NotAvailableA;

    // Insert the item (enumerator) into the list:
    unavailableStrList << acGTStringToQString(functionTypeName);
    unavailableStrList << AF_STR_NotAvailableA;
    unavailableStrList << AF_STR_NotAvailableA;
    unavailableStrList << AF_STR_NotAvailableA;
    unavailableStrList << GD_STR_StateChangeStatisticsViewerNonAnalyzeModeA;

    // Add the item data:
    gdStatisticsViewItemData* pItemData = new gdStatisticsViewItemData;

    pItemData->_functionType = functionType;
    pItemData->_totalAmountOfTimesCalled = -1;
    pItemData->_averagePercentOfCalls = -1;
    pItemData->_percentageOfTimesCalled = -1;
    pItemData->_averageAmountOfTimesCalled = -1;
    pItemData->_isItemAvailable = false;

    // Add the row:
    addRow(unavailableStrList, pItemData, false, Qt::Unchecked, icon(0));

    return true;
}

// ---------------------------------------------------------------------------
// Name:        gdTotalStatisticsView::initializeImageList
// Description: Create and populate the image list for this item
// Author:      Uri Shomroni
// Date:        6/8/2008
// ---------------------------------------------------------------------------
void gdTotalStatisticsView::initializeImageList()
{
    // Create the icons from the xpm files:
    QPixmap* pEmptyIcon = new QPixmap;
    acSetIconInPixmap(*pEmptyIcon, AC_ICON_EMPTY);

    QPixmap* pOrangeWarningIcon = new QPixmap;
    acSetIconInPixmap(*pOrangeWarningIcon, AC_ICON_WARNING_ORANGE);

    QPixmap* pRedWarningIcon = new QPixmap;
    acSetIconInPixmap(*pRedWarningIcon, AC_ICON_WARNING_RED);

    QPixmap* pYellowWarningIcon = new QPixmap;
    acSetIconInPixmap(*pYellowWarningIcon, AC_ICON_WARNING_YELLOW);

    // Add the icons to vector:
    _listIconsVec.push_back(pEmptyIcon);            // 0
    _listIconsVec.push_back(pOrangeWarningIcon);    // 1
    _listIconsVec.push_back(pRedWarningIcon);       // 2
    _listIconsVec.push_back(pYellowWarningIcon);    // 3
}

// ---------------------------------------------------------------------------
// Name:        gdTotalStatisticsView::addTotalItemToList
// Description: Adds the "Total" item to the list control.
// Author:      Yaki Tebeka
// Date:        16/4/2006
// ---------------------------------------------------------------------------
void gdTotalStatisticsView::addTotalItemToList()
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
        // Build displayed strings:
        gtASCIIString totalString = AF_STR_TotalA;

        gtASCIIString totalAmountOfFunctionCallsString;
        totalAmountOfFunctionCallsString.appendFormattedString("%d", _totalAmountOfFunctionCalls);
        totalAmountOfFunctionCallsString.addThousandSeperators();

        gtASCIIString totalAmountOfFullFramesFunctionCallsString;

        if (_amountOfFullFrames > 0)
        {
            gtUInt64 averageAmountOfCallsPerFrame = _totalAmountOfFunctionCallsInFullFrames / _amountOfFullFrames;
            totalAmountOfFullFramesFunctionCallsString.appendFormattedString("%ld", averageAmountOfCallsPerFrame);
            totalAmountOfFullFramesFunctionCallsString.addThousandSeperators();
        }

        QStringList list;
        // Insert the item (enumerator) into the list:
        list << totalString.asCharArray();
        list << "";
        list << totalAmountOfFunctionCallsString.asCharArray();
        list << "";
        list << totalAmountOfFullFramesFunctionCallsString.asCharArray();

        // Add the item:
        addRow(list, NULL, false, Qt::Unchecked, icon(0));

        // Add a number of frames item:
        gtString numOfFramesString;
        gtString tmp;
        tmp.appendFormattedString(L"%ld", _amountOfFullFrames);
        tmp.addThousandSeperators();
        numOfFramesString.appendFormattedString(GD_STR_TotalStatisticsViewerFramesNumber, tmp.asCharArray());

        QStringList list2;

        list2 << "";
        list2 << "";
        list2 << "";
        list2 << acGTStringToQString(numOfFramesString);
        list2 << acGTStringToQString(numOfFramesString);

        setItemBold(rowCount() - 1, -1);
    }
}

// ---------------------------------------------------------------------------
// Name:        gdTotalStatisticsView::clearAllStatisticsItems
// Description: delete the items from the list
// Author:      Avi Shapira
// Date:        29/1/2006
// ---------------------------------------------------------------------------
void gdTotalStatisticsView::clearAllStatisticsItems()
{
    // Call base class:
    gdStatisticsViewBase::clearAllStatisticsItems();

    // Reset all function types amount of calls:
    for (int i = 0; i < GD_NUMBER_OF_FUNCTION_TYPES_INDICES; i++)
    {
        _pFunctionTypeAverageAmountsByType[i] = 0;
        _pFunctionTypeAmountsByType[i] = 0;
    }

    // Clear local counters:
    _totalAmountOfFunctionCalls = 0;
    _totalAmountOfFunctionCallsInFullFrames = 0;
    _amountOfFullFrames = 0;
}


// ---------------------------------------------------------------------------
// Name:        gdTotalStatisticsView::getItemNumOfCalls
// Description: Return an item number of calls
// Arguments: int itemIndex
//            gtUInt64& numberOfCalls
//            bool& isItemAvailable
// Return Val: bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        26/5/2009
// ---------------------------------------------------------------------------
bool gdTotalStatisticsView::getItemNumOfCalls(int itemIndex, gtUInt64& numberOfCalls, bool& isItemAvailable)
{
    bool retVal = false;

    // If it is a 'real' item (not the total item):
    if (itemIndex < rowCount() - 1)
    {
        // Get the item data for this item:
        gdStatisticsViewItemData* pViewItemData = (gdStatisticsViewItemData*)getItemData(itemIndex);
        GT_IF_WITH_ASSERT(pViewItemData != NULL)
        {
            // Get the item amount of times called:
            numberOfCalls = pViewItemData->_totalAmountOfTimesCalled;
            isItemAvailable = pViewItemData->_isItemAvailable;
            retVal = true;
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gdTotalStatisticsView::getItemFunctionType
// Description: Return an item function type
// Arguments: int itemIndex
//            gdFuncCallsViewTypes& functionType
// Return Val: bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        26/5/2009
// ---------------------------------------------------------------------------
bool gdTotalStatisticsView::getItemFunctionType(int itemIndex, gdFuncCallsViewTypes& functionType)
{
    bool retVal = false;

    // If it is a 'real' item (not the total item):
    if (itemIndex < rowCount() - 1)
    {
        // Get the item data for this item:
        gdStatisticsViewItemData* pViewItemData = (gdStatisticsViewItemData*)getItemData(itemIndex);
        GT_IF_WITH_ASSERT(pViewItemData != NULL)
        {
            // Get the item function type:
            functionType = pViewItemData->_functionType;
            retVal = true;
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gdTotalStatisticsView::getItemChartColor
// Description: Overrides base class item color
// Arguments: int itemIndex
//            int amountOfCurrentItemsForColorSelection
//            bool useSavedColors
//            unsigned long& color
// Return Val: bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        26/5/2009
// ---------------------------------------------------------------------------
bool gdTotalStatisticsView::getItemChartColor(int itemIndex, int& amountOfCurrentItemsForColorSelection, bool useSavedColors, unsigned long& color)
{
    (void)(useSavedColors);  // unused
    bool retVal = false;

    // Sanity check:
    GT_IF_WITH_ASSERT((itemIndex < rowCount()) && (itemIndex >= 0))
    {
        // Get item data:
        gdStatisticsViewItemData* pItemData = (gdStatisticsViewItemData*)getItemData(itemIndex);
        GT_IF_WITH_ASSERT(pItemData != NULL)
        {
            retVal = true;

            // Get function type:
            gdFuncCallsViewTypes functionType = pItemData->_functionType;

            // Check if item is available:
            bool isItemAvailable = pItemData->_isItemAvailable;

            // Get amount of colors:
            int colorsAmount = amountOfChartColors() - 1;

            color = 0xFFFFFF;

            if (functionType == GD_GET_FUNCTIONS_INDEX)
            {
                // Translate the "get function" special color to a u long color
                int r = (int)(acQORANGE_WARNING_COLOUR.red());
                int g = (int)(acQORANGE_WARNING_COLOUR.green());
                int b = (int)(acQORANGE_WARNING_COLOUR.blue());
                color = (b << 16) | (g << 8) | r;
            }
            else if (functionType == GD_REDUNDANT_STATE_CHANGE_FUNCTIONS_INDEX)
            {
                // Translate the "redundant state change" special color to a u long color
                int r = (int)(acQRED_WARNING_COLOUR.red());
                int g = (int)(acQRED_WARNING_COLOUR.green());
                int b = (int)(acQRED_WARNING_COLOUR.blue());
                color = (b << 16) | (g << 8) | r;
            }
            else if (functionType == GD_DEPRECATED_FUNCTIONS_INDEX)
            {
                // Translate the "deprecated function" special color to a u long color
                int r = (int)(acQYELLOW_WARNING_COLOUR.red());
                int g = (int)(acQYELLOW_WARNING_COLOUR.green());
                int b = (int)(acQYELLOW_WARNING_COLOUR.blue());
                color = (b << 16) | (g << 8) | r;
            }
            else
            {
                if (isItemAvailable)
                {
                    // Use a normal color (use all the color besides the last one - the highlight color:
                    color = _chartColors[amountOfCurrentItemsForColorSelection % (colorsAmount - 1)];

                    // Increase number of items use normal colors:
                    amountOfCurrentItemsForColorSelection++;
                }
            }
        }
    }
    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gdTotalStatisticsView::getItemTooltip
// Description: Get an item tooltip
// Arguments: int itemIndex
// Return Val: gtString
// Author:      Sigal Algranaty
// Date:        24/6/2009
// ---------------------------------------------------------------------------
gtString gdTotalStatisticsView::getItemTooltip(int itemIndex)
{
    gtString retVal;
    // Get the item data:
    gdStatisticsViewItemData* pItemData = (gdStatisticsViewItemData*)getItemData(itemIndex);

    if (pItemData != NULL)
    {
        // Build the tooltip string:
        gtString functionTypeStr = functionTypeToString(pItemData->_functionType);
        retVal.appendFormattedString(L"%ls: %.2f%%", functionTypeStr.asCharArray(), pItemData->_percentageOfTimesCalled);
    }

    return retVal;

}

const char* gdTotalStatisticsView::saveStatisticsDataFileName()
{
    return GD_STR_saveFunctionTypesStatisticsFileName;
}

// ---------------------------------------------------------------------------
// Name:        gdTotalStatisticsView::setHeaderToolTips
// Description: Set tooltips for columns headings
// Arguments:   void
// Return Val:  void
// Author:      Ofra Moyal Cohen
// Date:        24/11/2014
// ---------------------------------------------------------------------------
void gdTotalStatisticsView::setListHeadingToolTips()
{
    QHeaderView* pHeaderView = horizontalHeader();

    GT_IF_WITH_ASSERT(NULL != pHeaderView)
    {
        gtVector<QString> headings;
        headings.push_back(GD_STR_TotalStatisticsViewerColumn1Tooltip);
        headings.push_back(GD_STR_TotalStatisticsViewerColumn2Tooltip);
        headings.push_back(GD_STR_TotalStatisticsViewerColumn3Tooltip);
        headings.push_back(QString(GD_STR_TotalStatisticsViewerColumn4Tooltip).arg(_amountOfFullFrames));
        headings.push_back(QString(GD_STR_TotalStatisticsViewerColumn5Tooltip).arg(_amountOfFullFrames));

        int numOfCol = std::min(pHeaderView->count(), int(headings.size()));

        for (int i = 0; i < numOfCol; i++)
        {
            horizontalHeaderItem(i)->setToolTip(headings[i]);
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        gdTotalStatisticsView::shouldShowFunctionType
// Description: Checks if the input function type should currently be shown
// Arguments:   bool showOpenGL - should current configuration display OpenGL functions
//              bool showOpenCL - should current configuration display OpenCL functions
//              gdFuncCallsViewTypes functionType - the current function type
//              bool& shouldAddFunctionTypeItem - output - should this function type be added
//              bool& shouldAddUnavailableItem - should this function type be added as unavailable item
//              gtString& unavailableMessage
// Return Val:  void
// Author:      Sigal Algranaty
// Date:        28/3/2010
// ---------------------------------------------------------------------------
void gdTotalStatisticsView::shouldShowFunctionType(gdFuncCallsViewTypes functionType, bool& shouldAddFunctionTypeItem, bool& shouldAddUnavailableItem, gtString& unavailableMessage)
{
    // Check if the function type is available at this mode:
    shouldAddUnavailableItem = (_executionMode != AP_ANALYZE_MODE);

    // Check if the function is of type redundant or effective state change:
    shouldAddUnavailableItem = shouldAddUnavailableItem && ((functionType == GD_EFFECTIVE_STATE_CHANGE_FUNCTIONS_INDEX) || (functionType == GD_REDUNDANT_STATE_CHANGE_FUNCTIONS_INDEX));

    // Display unavailable items only for OpenGL:
    shouldAddUnavailableItem = shouldAddUnavailableItem && (_activeContextId.isOpenGLContext());

    // Add redundant & effective function types only for OpenGL

    // Handle unavailable items:
    if (shouldAddUnavailableItem)
    {
        shouldAddFunctionTypeItem = false;

        if (functionType == GD_EFFECTIVE_STATE_CHANGE_FUNCTIONS_INDEX)
        {
            unavailableMessage = GD_STR_TotalStatisticsViewerEffectiveStateChangeFunctions;
        }
        else if (functionType == GD_REDUNDANT_STATE_CHANGE_FUNCTIONS_INDEX)
        {
            unavailableMessage = GD_STR_TotalStatisticsViewerRedundantStateChangeFunctions;
        }
    }
    else
    {
        if (_activeContextId.isOpenGLContext())
        {
            // Item types for OpenGL:
            // Check if the function type is one of the OpenGL function types:
            shouldAddFunctionTypeItem = ((functionType == GD_GET_FUNCTIONS_INDEX) ||
                                         (functionType == GD_REDUNDANT_STATE_CHANGE_FUNCTIONS_INDEX) ||
                                         (functionType == GD_EFFECTIVE_STATE_CHANGE_FUNCTIONS_INDEX) ||
                                         (functionType == GD_STATE_CHANGE_FUNCTIONS_INDEX) ||
                                         (functionType == GD_DEPRECATED_FUNCTIONS_INDEX) ||
                                         (functionType == GD_DRAW_FUNCTIONS_INDEX) ||
                                         (functionType == GD_RASTER_FUNCTIONS_INDEX) ||
                                         (functionType == GD_PROGRAM_AND_SHADERS_FUNCTIONS_INDEX) ||
                                         (functionType == GD_TEXTURE_FUNCTIONS_INDEX) ||
                                         (functionType == GD_MATRIX_FUNCTIONS_INDEX) ||
                                         (functionType == GD_NAME_FUNCTIONS_INDEX) ||
                                         (functionType == GD_QUERY_FUNCTIONS_INDEX) ||
                                         (functionType == GD_BUFFER_FUNCTIONS_INDEX) ||
                                         (functionType == GD_SYNC_FUNCTIONS_INDEX) ||
                                         (functionType == GD_FEEDBACK_FUNCTIONS_INDEX) ||
                                         (functionType == GD_VERTEX_ARRAY_FUNCTIONS_INDEX) ||
                                         (functionType == GD_DEBUG_FUNCTIONS_INDEX));
        }

        else if (_activeContextId.isOpenCLContext())
        {
            // Check if the function type is one of the OpenCL function types:
            shouldAddFunctionTypeItem = ((functionType == GD_QUEUE_FUNCTIONS_INDEX) ||
                                         (functionType == GD_BUFFERS_AND_IMAGE_FUNCTIONS_INDEX) ||
                                         (functionType == GD_GET_FUNCTIONS_INDEX) ||
                                         (functionType == GD_PROGRAM_AND_KERNELS_FUNCTIONS_INDEX) ||
                                         (functionType == GD_SYNC_FUNCTIONS_INDEX) ||
                                         (functionType == GD_DEBUG_FUNCTIONS_INDEX));
        }

        else if (_activeContextId.isDefault())
        {
            shouldAddFunctionTypeItem = ((functionType == GD_GL_NULL_CONTEXT_FUNCTIONS_INDEX)  ||
                                         (functionType == GD_CL_NULL_CONTEXT_FUNCTIONS_INDEX)  ||
                                         (functionType == GD_GL_CONTEXT_BOUND_FUNCTIONS_INDEX) ||
                                         (functionType == GD_CL_CONTEXT_BOUND_FUNCTIONS_INDEX));
        }
    }
}

