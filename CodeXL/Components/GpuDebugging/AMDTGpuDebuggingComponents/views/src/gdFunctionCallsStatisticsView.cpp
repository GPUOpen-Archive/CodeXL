//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gdFunctionCallsStatisticsView.cpp
///
//==================================================================================

//------------------------------ gdFunctionCallsStatisticsView.cpp ------------------------------

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
#include <AMDTApplicationComponents/Include/acColours.h>
#include <AMDTApplicationComponents/Include/acIcons.h>
#include <AMDTApiFunctions/Include/gaGRApiFunctions.h>
#include <AMDTApplicationComponents/Include/acFunctions.h>

#if (AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_GENERIC_LINUX_VARIANT)
    #include <AMDTAPIClasses/Include/apGLXParameters.h>
#endif

// AMDTApplicationFramework:
#include <AMDTApplicationFramework/Include/afAppStringConstants.h>
#include <AMDTApplicationFramework/Include/afHTMLContent.h>

// Local:
#include <AMDTGpuDebuggingComponents/Include/gdCommandIDs.h>
#include <AMDTGpuDebuggingComponents/Include/gdStringConstants.h>
#include <AMDTGpuDebuggingComponents/Include/gdAidFunctions.h>
#include <AMDTApplicationFramework/Include/afGlobalVariableChangedEvent.h>
#include <AMDTGpuDebuggingComponents/Include/gdGDebuggerGlobalVariablesManager.h>
#include <AMDTGpuDebuggingComponents/Include/views/gdFunctionCallsStatisticsView.h>


// ---------------------------------------------------------------------------
// Name:        gdFunctionCallsStatisticsView::gdFunctionCallsStatisticsView
// Description: Constructor.
// Arguments:   parent - My parent window.
// Author:      Avi Shapira
// Date:        29/1/2006
// ---------------------------------------------------------------------------
gdFunctionCallsStatisticsView::gdFunctionCallsStatisticsView(QWidget* pParent)
    : gdStatisticsViewBase(pParent, GD_STATISTICS_VIEW_FUNCTION_CALLS_INDEX, GD_STR_StatisticsViewerFunctionCallsStatisticsShortName),
      _totalAmountOfFunctionCallsInFrame(0)
{
    // Add breakpoint actions to context menu:
    _addBreakpointActions = true;

    // Call base class initialization:
    init();
}

// ---------------------------------------------------------------------------
// Name:        gdFunctionCallsStatisticsView::~gdFunctionCallsStatisticsView
// Description: Destructor
// Author:      Avi Shapira
// Date:        29/1/2006
// ---------------------------------------------------------------------------
gdFunctionCallsStatisticsView::~gdFunctionCallsStatisticsView()
{
}

// ---------------------------------------------------------------------------
// Name:        gdFunctionCallsStatisticsView::onEvent
// Description: Overrides apIEventsObserver
// Arguments:   const apEvent& eve
//              bool& vetoEvent
// Author:      Sigal Algranaty
// Date:        28/12/2011
// ---------------------------------------------------------------------------
void gdFunctionCallsStatisticsView::onEvent(const apEvent& eve, bool& vetoEvent)
{
    // Call the base class event handler:
    gdStatisticsViewBase::onEvent(eve, vetoEvent);

    if (eve.eventType() == apEvent::APP_GLOBAL_VARIABLE_CHANGED)
    {
        // Down cast the event:
        const afGlobalVariableChangedEvent& variableChangedEvent = (const afGlobalVariableChangedEvent&)eve;

        // Handle it:
        onGlobalVariableChanged(variableChangedEvent);
    }
}
// ---------------------------------------------------------------------------
// Name:        gdFunctionCallsStatisticsView::initListCtrlColumns
// Description: Initializes the columns titles and widths
// Author:      Avi Shapira
// Date:        29/1/2006
// ---------------------------------------------------------------------------
void gdFunctionCallsStatisticsView::initListCtrlColumns()
{
    // Create the "Function Calls Statistics View" columns:
    _listControlColumnTitles.push_back(GD_STR_FunctionCallsStatisticsViewColumn1Title);
    _listControlColumnTitles.push_back(GD_STR_FunctionCallsStatisticsViewColumn2Title);
    _listControlColumnTitles.push_back(GD_STR_FunctionCallsStatisticsViewColumn3Title);
    _listControlColumnTitles.push_back(GD_STR_FunctionCallsStatisticsViewColumn4Title);

    _listControlColumnWidths.push_back(0.20f);
    _listControlColumnWidths.push_back(0.08f);
    _listControlColumnWidths.push_back(0.08f);
    _listControlColumnWidths.push_back(0.24f);

    // Set copy postfixes:
    m_columnsPostfixes.push_back("");
    m_columnsPostfixes.push_back("%");
    m_columnsPostfixes.push_back("");
    m_columnsPostfixes.push_back("");

    m_removeThousandSeparatorOnCopy = true;

    _widestColumnIndex = 0;
    _initialSortColumnIndex = 2;
    _sortInfo._sortOrder = Qt::DescendingOrder;
}

// ---------------------------------------------------------------------------
// Name:        gdFunctionCallsStatisticsView::getFunctionTypeString
// Description: Returns a string that describes the function type
// Arguments: int functionId
//            gtString& functionType
// Return Val: bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        4/8/2008
// ---------------------------------------------------------------------------
bool gdFunctionCallsStatisticsView::getFunctionTypeString(apMonitoredFunctionId functionId, gtString& functionTypeString)
{
    bool retVal = false;

    // Get the function type:
    unsigned int functionTypeAsUInt;
    retVal = gaGetMonitoredFunctionType(functionId, functionTypeAsUInt);
    apFunctionType funcType = (apFunctionType)functionTypeAsUInt;

    // Increase the relevant counter:
    GT_IF_WITH_ASSERT(retVal)
    {
        if (funcType & AP_GET_FUNC)
        {
            functionTypeString = GD_STR_FunctionCallsStatisticsViewGetFunction;
        }

        if (funcType & AP_DRAW_FUNC)
        {
            if (!functionTypeString.isEmpty())
            {
                functionTypeString.append(L" | ");
            }

            functionTypeString += GD_STR_FunctionCallsStatisticsViewDrawFunction;
        }

        if (funcType & AP_RASTER_FUNC)
        {
            if (!functionTypeString.isEmpty())
            {
                functionTypeString.append(L" | ");
            }

            functionTypeString += GD_STR_FunctionCallsStatisticsViewRasterFunction;
        }

        if (funcType & AP_STATE_CHANGE_FUNC)
        {
            if (!functionTypeString.isEmpty())
            {
                functionTypeString.append(L" | ");
            }

            functionTypeString += GD_STR_FunctionCallsStatisticsViewStateChangeFunction;
        }

        if (funcType & AP_PROGRAM_SHADER_FUNC)
        {
            if (!functionTypeString.isEmpty())
            {
                functionTypeString.append(L" | ");
            }

            functionTypeString += GD_STR_FunctionCallsStatisticsViewProgramsShadersFunction;
        }

        if (funcType & AP_TEXTURE_FUNC)
        {
            if (!functionTypeString.isEmpty())
            {
                functionTypeString.append(L" | ");
            }

            functionTypeString += GD_STR_FunctionCallsStatisticsViewTextureFunction;
        }

        if (funcType & AP_MATRIX_FUNC)
        {
            if (!functionTypeString.isEmpty())
            {
                functionTypeString.append(L" | ");
            }

            functionTypeString += GD_STR_FunctionCallsStatisticsViewMatrixFunction;
        }

        if (funcType & AP_NAME_FUNC)
        {
            if (!functionTypeString.isEmpty())
            {
                functionTypeString.append(L" | ");
            }

            functionTypeString += AF_STR_Name;
        }

        if (funcType & AP_QUERY_FUNC)
        {
            if (!functionTypeString.isEmpty())
            {
                functionTypeString.append(L" | ");
            }

            functionTypeString += GD_STR_FunctionCallsStatisticsViewQueryFunction;
        }

        if (funcType & AP_BUFFER_FUNC)
        {
            if (!functionTypeString.isEmpty())
            {
                functionTypeString.append(L" | ");
            }

            functionTypeString += GD_STR_FunctionCallsStatisticsViewBufferFunction;
        }

        if (funcType & AP_DEPRECATED_FUNC)
        {
            if (!functionTypeString.isEmpty())
            {
                functionTypeString.append(L" | ");
            }

            functionTypeString += GD_STR_FunctionCallsStatisticsViewDeprecatedFunction;
        }

        if (funcType & AP_BUFFER_IMAGE_FUNC)
        {
            if (!functionTypeString.isEmpty())
            {
                functionTypeString.append(L" | ");
            }

            functionTypeString += GD_STR_FunctionCallsStatisticsViewBuffersImagesFunction;
        }

        if (funcType & AP_QUEUE_FUNC)
        {
            if (!functionTypeString.isEmpty())
            {
                functionTypeString.append(L" | ");
            }

            functionTypeString += GD_STR_FunctionCallsStatisticsViewQueueFunction;
        }

        if (funcType & AP_SYNCHRONIZATION_FUNC)
        {
            if (!functionTypeString.isEmpty())
            {
                functionTypeString.append(L" | ");
            }

            functionTypeString += GD_STR_FunctionCallsStatisticsViewSyncFunction;
        }

        if (funcType & AP_FEEDBACK_FUNC)
        {
            if (!functionTypeString.isEmpty())
            {
                functionTypeString.append(L" | ");
            }

            functionTypeString += GD_STR_FunctionCallsStatisticsViewFeedbackFunction;
        }

        if (funcType & AP_VERTEX_ARRAY_FUNC)
        {
            if (!functionTypeString.isEmpty())
            {
                functionTypeString.append(L" | ");
            }

            functionTypeString += GD_STR_FunctionCallsStatisticsViewVertexArrayFunction;
        }

        if (funcType & AP_DEBUG_FUNC)
        {
            if (!functionTypeString.isEmpty())
            {
                functionTypeString.append(L" | ");
            }

            functionTypeString += GD_STR_FunctionCallsStatisticsViewDebugFunction;
        }

        if (funcType & AP_PROGRAM_KERNEL_FUNC)
        {
            if (!functionTypeString.isEmpty())
            {
                functionTypeString.append(L" | ");
            }

            functionTypeString += GD_STR_FunctionCallsStatisticsViewProgramsAndKernelsFunction;
        }

        retVal = true;
    }
    return retVal;
}
// ---------------------------------------------------------------------------
// Name:        gdFunctionCallsStatisticsView::updateFunctionCallsStatisticsList
// Description: Update the Calls Stack into the listCTRL
// Arguments: const apStatistics& currentStatistics
// Return Val: bool  - Success / failure.
// Author:      Avi Shapira
// Date:        29/1/2006
// ---------------------------------------------------------------------------
bool gdFunctionCallsStatisticsView::updateFunctionCallsStatisticsList(const apStatistics& currentStatistics)
{
    bool retVal = true;
    // Delete the items and their user data
    clearAllStatisticsItems();

    // Enable the list
    setEnabled(true);

    // Reset total amount of function calls:
    _totalAmountOfFunctionCallsInFrame = 0;

    // Get the number of function calls statistics:
    int functionStatisticsAmount = currentStatistics.amountOfFunctionCallsStatistics();

    if (functionStatisticsAmount > 0)
    {
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

        // Fill the list:
        for (int i = 0; i < functionStatisticsAmount; i++)
        {
            const apFunctionCallStatistics* pFuncCallStatData = NULL;
            bool rc = currentStatistics.getFunctionCallStatistics(i, pFuncCallStatData);
            GT_IF_WITH_ASSERT(rc && pFuncCallStatData != NULL)
            {
                gtString functionName;
                gtString functionType;

                // Get the function name:
                apMonitoredFunctionId functionId = pFuncCallStatData->_functionId;
                bool rc1 = gaGetMonitoredFunctionName(functionId, functionName);

                if (!rc1)
                {

                    functionName = AF_STR_NotAvailable;
                    const apFunctionCallStatistics* pFuncCallStatsData;
                    bool validStat = currentStatistics.getFunctionCallStatistics(i, pFuncCallStatsData);
                    GT_IF_WITH_ASSERT(validStat && pFuncCallStatsData != NULL)
                    {
                        // Add the function to the total counter:
                        gtUInt64 amountOfTimesCalled = pFuncCallStatsData->_amountOfTimesCalled;
                        _totalAmountOfFunctionCallsInFrame += amountOfTimesCalled;
                    }
                }

                bool rc2 = getFunctionTypeString(functionId, functionType);
                GT_ASSERT(rc2);

                // Get the internal function data:
                const gtVector<apEnumeratorUsageStatistics>& usedEnumerators = pFuncCallStatData->_usedEnumerators;
                int enumeratorsVectorSize = usedEnumerators.size();

                if (enumeratorsVectorSize > 0)
                {
                    // We have enumerators details for this function:

                    // Get the enumerators of the function calls:
                    for (int j = 0; j < enumeratorsVectorSize; j++)
                    {
                        // Get the enumerator data:
                        apEnumeratorUsageStatistics enumeratorUsageStatisticsData = usedEnumerators[j];

                        // Get the enumerator name:
                        gtString enumeratorAsString;
                        GLenum enumerator = enumeratorUsageStatisticsData._enum;

                        // Treat primitive type parameters as such and not as normal enumerators
                        // This will cause GL_POINTS and GL_LINES to be displayed correctly.
                        osTransferableObjectType enumType = usedEnumerators[j]._enumType;

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

                        // Get the amount of times the enumerator called:
                        gtUInt64 amountOfTimesEnumCalled = enumeratorUsageStatisticsData._amountOfTimesUsed;

                        // Add the item into the list ctrl:
                        retVal = retVal && addStatisticsItem(functionId, functionNameEnum, functionType, amountOfTimesEnumCalled);
                    }
                }
                else
                {
                    // We do not have enumerators details for this function:
                    gtUInt64 amountOfTimesCalled = pFuncCallStatData->_amountOfTimesCalled;

                    // Add the item into the list ctrl:
                    retVal = retVal && addStatisticsItem(functionId, functionName, functionType, amountOfTimesCalled);
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
        addRow(AF_STR_NotAvailableA);
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gdFunctionCallsStatisticsView::addStatisticsItem
// Description: Add Statistic item into the list ctrl
// Arguments:   wxListEvent &eve
// Author:      Avi Shapira
// Date:        29/1/2006
// ---------------------------------------------------------------------------
bool gdFunctionCallsStatisticsView::addStatisticsItem(apMonitoredFunctionId functionId, const gtString& functionName, gtString& functionType, gtUInt64 amountOfCalls)
{
    gtString amountOfTimesCalledString;
    gtString percentageCalledString;
    float percentageCalled = 0;

    amountOfTimesCalledString.appendFormattedString(L"%ld", amountOfCalls);
    amountOfTimesCalledString.addThousandSeperators();

    if (_totalAmountOfFunctionCallsInFrame > 0)
    {
        // Calculate the appearers percentage:
        percentageCalled = amountOfCalls;
        percentageCalled *= 100;
        percentageCalled /= _totalAmountOfFunctionCallsInFrame;
        percentageCalledString.appendFormattedString(L"%.2f", percentageCalled);
    }
    else
    {
        percentageCalledString.append(L"N/A");
    }

    // Get the items associated icon:
    afIconType iconType = AF_ICON_NONE;
    int iconIndex = 0;
    getStatisticsItemIcon(functionName, iconType, iconIndex);

    // Get the icon:
    QPixmap* pIcon = icon(iconIndex);

    QStringList list;
    list << acGTStringToQString(functionName);
    list << acGTStringToQString(percentageCalledString);
    list << acGTStringToQString(amountOfTimesCalledString);
    list << acGTStringToQString(functionType);

    // Add the item data:
    gdStatisticsViewItemData* pItemData = new gdStatisticsViewItemData;

    pItemData->_functionName = functionName;
    pItemData->_functionTypeStr = functionType;
    pItemData->_percentageOfTimesCalled = percentageCalled;
    pItemData->_totalAmountOfTimesCalled = amountOfCalls;
    pItemData->_functionId = functionId;
    pItemData->_iconType = iconType;
    addRow(list, pItemData, false, Qt::Unchecked, pIcon);

    return true;
}


// ---------------------------------------------------------------------------
// Name:        gdFunctionCallsStatisticsView::getStatisticsItemIcon
// Description: Inputs a statistics item and returns its associated icon.
// Arguments: functionName - The input item (function name).
// Return Val: int - Will get the items' associated icon.
// Author:      Yaki Tebeka
// Date:        1/9/2008
// ---------------------------------------------------------------------------
void gdFunctionCallsStatisticsView::getStatisticsItemIcon(const gtString& functionName, afIconType& iconType, int& iconIndex) const
{
    iconIndex = 0;
    iconType = AF_ICON_INFO;


    if ((functionName.find(L"glBegin ") > -1) ||
        (functionName == L"glEnd"))
    {
        // Red warning icon:
        // (For deprecated functions)
        iconIndex = 3;
        iconType = AF_ICON_WARNING3;
    }
    else if ((functionName == L"eglGetError") || (functionName == L"glGetError") ||
             (functionName == L"glGetInfoLogARB") || (functionName == L"glGetProgramInfoLog") ||
             (functionName == L"glGetShaderInfoLog") ||

             (functionName == L"glAreTexturesResident") || (functionName == L"glGetAttachedObjectsARB") || (functionName == L"glGetAttachedShaders") ||
             (functionName == L"glGetBooleanv") || (functionName == L"glGetBufferParameteriv") || (functionName == L"glGetBufferParameterivARB") ||
             (functionName == L"glGetBufferSubData") || (functionName == L"glGetBufferSubDataARB") || (functionName == L"glGetClipPlane") ||
             (functionName == L"glGetClipPlanef") || (functionName == L"glGetClipPlanex") || (functionName == L"glGetColorTable") ||
             (functionName == L"glGetColorTableParameterfv") || (functionName == L"glGetColorTableParameteriv") || (functionName == L"glGetCompressedTexImage") ||
             (functionName == L"glGetCompressedTexImageARB") || (functionName == L"glGetConvolutionFilter") || (functionName == L"glGetConvolutionParameterfv") ||
             (functionName == L"glGetConvolutionParameteriv") || (functionName == L"glGetDoublev") || (functionName == L"glGetFixedv") ||
             (functionName == L"glGetFloatv") || (functionName == L"glGetFramebufferAttachmentParameterivEXT") || (functionName == L"glGetHistogram") ||
             (functionName == L"glGetHistogramParameterfv") || (functionName == L"glGetHistogramParameteriv") || (functionName == L"glGetIntegerv") ||
             (functionName == L"glGetLightfv") || (functionName == L"glGetLightiv") || (functionName == L"glGetLightxv") ||
             (functionName == L"glGetLocalConstantBooleanvEXT") || (functionName == L"glGetLocalConstantFloatvEXT") || (functionName == L"glGetLocalConstantIntegervEXT") ||
             (functionName == L"glGetMapdv") || (functionName == L"glGetMapfv") || (functionName == L"glGetMapiv") ||
             (functionName == L"glGetMaterialfv") || (functionName == L"glGetMaterialiv") || (functionName == L"glGetMaterialxv") ||
             (functionName == L"glGetMinmax") || (functionName == L"glGetMinmaxParameterfv") || (functionName == L"glGetMinmaxParameteriv") ||
             (functionName == L"glGetObjectParameterfvARB") || (functionName == L"glGetObjectParameterivARB") || (functionName == L"glGetOcclusionQueryivNV") ||
             (functionName == L"glGetOcclusionQueryuivNV") || (functionName == L"glGetPixelMapfv") || (functionName == L"glGetPixelMapuiv") ||
             (functionName == L"glGetPixelMapusv") || (functionName == L"glGetPointerv") || (functionName == L"glGetPolygonStipple") ||
             (functionName == L"glGetProgramEnvParameterdvARB") || (functionName == L"glGetProgramEnvParameterfvARB") || (functionName == L"glGetProgramiv") ||
             (functionName == L"glGetProgramivARB") || (functionName == L"glGetProgramivNV") || (functionName == L"glGetProgramLocalParameterdvARB") ||
             (functionName == L"glGetProgramLocalParameterfvARB") || (functionName == L"glGetProgramNamedParameterdvNV") || (functionName == L"glGetProgramNamedParameterfvNV") ||
             (functionName == L"glGetProgramParameterdvNV") || (functionName == L"glGetProgramParameterfvNV") || (functionName == L"glGetQueryObjectivARB") ||
             (functionName == L"glGetQueryObjectuivARB") || (functionName == L"glGetRenderbufferParameterivEXT") || (functionName == L"glGetShaderiv") ||
             (functionName == L"glGetShaderSource") || (functionName == L"glGetShaderSourceARB") || (functionName == L"glGetTexEnvfv") ||
             (functionName == L"glGetTexEnviv") || (functionName == L"glGetTexEnvxv") || (functionName == L"glGetTexGendv") ||
             (functionName == L"glGetTexGenfv") || (functionName == L"glGetTexGeniv") || (functionName == L"glGetTexImage") ||
             (functionName == L"glGetTexLevelParameterfv") || (functionName == L"glGetTexLevelParameteriv") || (functionName == L"glGetTexParameterfv") ||
             (functionName == L"glGetTexParameteriv") || (functionName == L"glGetTexParameterxv") || (functionName == L"glGetTrackMatrixivNV") ||
             (functionName == L"glGetUniformfv") || (functionName == L"glGetUniformfvARB") || (functionName == L"glGetUniformiv") ||
             (functionName == L"glGetUniformivARB") || (functionName == L"glGetVariantBooleanvEXT") || (functionName == L"glGetVariantFloatvEXT") ||
             (functionName == L"glGetVariantIntegervEXT") || (functionName == L"glGetVertexAttribdv") || (functionName == L"glGetVertexAttribdvNV") ||
             (functionName == L"glGetVertexAttribfv") || (functionName == L"glGetVertexAttribfvNV") || (functionName == L"glGetVertexAttribiv") ||
             (functionName == L"glGetVertexAttribivNV") || (functionName == L"glGetVertexAttribPointerv") || (functionName == L"glGetVertexAttribPointervNV") ||
             (functionName == L"glIsBuffer") || (functionName == L"glIsBufferARB") || (functionName == L"glIsEnabled") ||
             (functionName == L"glIsFramebufferEXT") || (functionName == L"glIsList") || (functionName == L"glIsOcclusionQueryNV") ||
             (functionName == L"glIsProgram") || (functionName == L"glIsProgramARB") || (functionName == L"glIsProgramNV") ||
             (functionName == L"glIsQuery") || (functionName == L"glIsQueryARB") || (functionName == L"glIsRenderbufferEXT") ||
             (functionName == L"glIsShader") || (functionName == L"glIsTexture") || (functionName == L"glIsVariantEnabledEXT") ||
             (functionName == L"glIsVertexArray") || (functionName == L"glXGetCurrentContext") || (functionName == L"wglGenlockSourceEdgeI3D") ||
             (functionName == L"wglGetCurrentContext") || (functionName == L"wglGetGenlockSampleRateI3D") || (functionName == L"wglGetGenlockSourceDelayI3D") ||
             (functionName == L"wglGetGenlockSourceEdgeI3D") || (functionName == L"wglGetGenlockSourceI3D") || (functionName == L"wglIsEnabledGenlockI3D"))
    {
        // Orange warning icon:
        // (For query functions that slow down performance and their value can be cached by the application)
        iconIndex = 2;
        iconType = AF_ICON_WARNING2;
    }
    else if ((functionName == L"glFinish") || (functionName == L"glFlush") || (functionName == L"glGetTexImage") ||
             (functionName == L"glReadPixels") ||
             (functionName == L"eglGetConfigAttrib") || (functionName == L"eglGetConfigs") || (functionName == L"eglGetDisplay") ||
             (functionName == L"eglGetProcAddress") || (functionName == L"eglQueryString") || (functionName == L"glGetActiveAttrib") ||
             (functionName == L"glGetActiveAttribARB") || (functionName == L"glGetActiveUniform") || (functionName == L"glGetActiveUniformARB") ||
             (functionName == L"glGetAttribLocation") || (functionName == L"glGetAttribLocationARB") || (functionName == L"glGetBufferPointerv") ||
             (functionName == L"glGetBufferPointervARB") || (functionName == L"glGetHandleARB") || (functionName == L"glGetProgramStringARB") ||
             (functionName == L"glGetProgramStringNV") || (functionName == L"glGetString") || (functionName == L"glGetUniformLocation") ||
             (functionName == L"glGetUniformLocationARB") || (functionName == L"glXGetClientString") || (functionName == L"glXGetConfig") ||
             (functionName == L"glXGetCurrentDisplay") || (functionName == L"glXGetCurrentDrawable") || (functionName == L"glXGetCurrentReadDrawable") ||
             (functionName == L"glXGetFBConfigAttrib") || (functionName == L"glXGetFBConfigAttribSGIX") || (functionName == L"glXGetFBConfigFromVisualSGIX") ||
             (functionName == L"glXGetFBConfigs") || (functionName == L"glXGetProcAddress") || (functionName == L"glXGetProcAddressARB") ||
             (functionName == L"glXGetVideoSyncSGI") || (functionName == L"glXGetVisualFromFBConfig") || (functionName == L"glXGetVisualFromFBConfigSGIX") ||
             (functionName == L"glXIsDirect") || (functionName == L"glXQueryContext") || (functionName == L"glXQueryExtension") ||
             (functionName == L"glXQueryExtensionsString") || (functionName == L"glXQueryServerString") || (functionName == L"glXQueryVersion") ||
             (functionName == L"wglDescribeLayerPlane") || (functionName == L"wglDescribePixelFormat") || (functionName == L"wglGetCurrentDC") ||
             (functionName == L"wglGetCurrentReadDCARB") || (functionName == L"wglGetDefaultProcAddress") || (functionName == L"wglGetExtensionsStringARB") ||
             (functionName == L"wglGetLayerPaletteEntries") || (functionName == L"wglGetPixelFormat") || (functionName == L"wglGetPixelFormatAttribfvARB") ||
             (functionName == L"wglGetPixelFormatAttribivARB") || (functionName == L"wglGetProcAddress") || (functionName == L"wglGetSwapIntervalEXT")
            )
    {
        // Yellow warning icon:
        // (For functions that are usually not recommended, and get functions which are needed in OGL initialization but not recommended otherwise)
        iconIndex = 1;
        iconType = AF_ICON_WARNING1;
    }
}

// ---------------------------------------------------------------------------
// Name:        gdFunctionCallsStatisticsView::addTotalItemToList
// Description: Adds the "Total" item to the list control.
// Author:      Yaki Tebeka
// Date:        16/4/2006
// ---------------------------------------------------------------------------
void gdFunctionCallsStatisticsView::addTotalItemToList()
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
        gtASCIIString totalString;
        gtASCIIString numberOfItemsString;
        totalString.appendFormattedString("Total (");
        numberOfItemsString.appendFormattedString("%d", listSize);
        numberOfItemsString.addThousandSeperators();
        totalString.append(numberOfItemsString);
        totalString.append(" items)");

        gtASCIIString totalAmountOfFunctionCallsInFrameString;
        totalAmountOfFunctionCallsInFrameString.appendFormattedString("%d", _totalAmountOfFunctionCallsInFrame);
        totalAmountOfFunctionCallsInFrameString.addThousandSeperators();

        // Insert the item (enumerator) into the list:
        QStringList list;
        list << totalString.asCharArray();
        list << "100";
        list << totalAmountOfFunctionCallsInFrameString.asCharArray();
        list << "";

        addRow(list, NULL, false, Qt::Unchecked, icon(0));
        setItemBold(rowCount() - 1, -1);
    }
}

// ---------------------------------------------------------------------------
// Name:        gdFunctionCallsStatisticsView::onGlobalVariableChanged
// Description: Triggered when a global variable value is changed -
//              will update the list according to the thread id
// Arguments:   const gdCodeXLGlobalVariableChangeEvent& stateChangeEvent
// Author:      Avi Shapira
// Date:        29/1/2006
// ---------------------------------------------------------------------------
void gdFunctionCallsStatisticsView::onGlobalVariableChanged(const afGlobalVariableChangedEvent& variableChangedEvent)
{
    // Get id of the global variable that was changed:
    afGlobalVariableChangedEvent::GlobalVariableId variableId = variableChangedEvent.changedVariableId();

    // If the chosen context was changed:
    if (variableId == afGlobalVariableChangedEvent::CHOSEN_CONTEXT_ID)
    {
        // Get the new chosen context id:
        gdGDebuggerGlobalVariablesManager& globalVarsManager = gdGDebuggerGlobalVariablesManager::instance();
        _activeContextId = globalVarsManager.chosenContext();
    }
}

// ---------------------------------------------------------------------------
// Name:        gdFunctionCallsStatisticsView::initializeImageList
// Description: Create and populate the image list for this item
// Author:      Uri Shomroni
// Date:        7/8/2008
// ---------------------------------------------------------------------------
void gdFunctionCallsStatisticsView::initializeImageList()
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

// -------------------------------------------------------------------------- -
// Name:        gdFunctionCallsStatisticsView::getItemTooltip
// Description: Get an item tooltip
// Arguments:   int itemIndex
// Return Val:  gtString
// Author:      Yuri Rshtunique
// Date:        November 9, 2014
// ---------------------------------------------------------------------------
gtString gdFunctionCallsStatisticsView::getItemTooltip(int itemIndex)
{
    gtString retVal;
    // Get the item data:
    gdStatisticsViewItemData* pItemData = (gdStatisticsViewItemData*)getItemData(itemIndex);

    if (pItemData != NULL)
    {
        // Build the tooltip string:
        gtString functionName = pItemData->_functionName;
        retVal.appendFormattedString(L"%ls: %.2f%%", functionName.asCharArray(), pItemData->_percentageOfTimesCalled);
    }

    return retVal;

}

const char* gdFunctionCallsStatisticsView::saveStatisticsDataFileName()
{
    return GD_STR_saveFunctionCallsStatisticsFileName;
}
