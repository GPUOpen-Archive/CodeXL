//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gsAPIFunctionsImplementations.cpp
///
//==================================================================================

//------------------------------ gsAPIFunctionsImplementations.cpp ------------------------------

// Infra:
#include <AMDTBaseTools/Include/AMDTDefinitions.h>
#include <AMDTAPIClasses/Include/apDefinitions.h>
#include <AMDTOSWrappers/Include/osOSDefinitions.h>
#include <AMDTOSWrappers/Include/osDebuggingFunctions.h>
#include <AMDTOSWrappers/Include/osDebugLog.h>
#include <AMDTOSWrappers/Include/osDirectory.h>
#include <AMDTOSWrappers/Include/osThread.h>
#include <AMDTAPIClasses/Include/apCounterID.h>
#include <AMDTAPIClasses/Include/apCounterType.h>
#include <AMDTAPIClasses/Include/apGLRenderContextInfo.h>
#include <AMDTAPIClasses/Include/apOpenGLStateVariablesManager.h>
#include <AMDTAPIClasses/Include/Events/apSpyProgressEvent.h>
#include <AMDTServerUtilities/Include/suGlobalVariables.h>
#include <AMDTServerUtilities/Include/suSpyAPIFunctions.h>
#include <AMDTServerUtilities/Include/suBreakpointsManager.h>

// Local:
#include <src/gsAPIFunctionsImplementations.h>
#include <src/gsAPIFunctionsStubs.h>
#include <src/gsDeprecationAnalyzer.h>
#include <src/gsMonitoredFunctionPointers.h>
#include <src/gsOpenGLMonitor.h>
#include <src/gsSpyPerformanceCountersManager.h>
#include <src/gsStringConstants.h>
#include <src/gsWrappersCommon.h>
#include <src/gsGLDebugOutputManager.h>
#include <src/gsPipelineMonitor.h>
#include <src/gsSamplersMonitor.h>

#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
    #include <src/gsATIPerformanceCountersManager.h>
#elif ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT))
    #ifdef _GR_IPHONE_BUILD
        // EAGL-only functions:
        #include <src/gsEAGLWrappers.h>

        #ifdef _GR_IPHONE_DEVICE_BUILD
            // iPhone device-only classes:
            #include <src/gsOSPerformanceCountersManager.h>
            #include <src/gsiPhoneGPUPerformanceCountersReader.h>
        #endif
    #endif
#endif


// ---------------------------------------------------------------------------
// Name:        gsMakeRenderContextCurrent
// Description: Makes a given render context current to the calling thread.
// Arguments: renderContextId - The given render context id.
//            if renderContextId == 0, releases the current context instead.
// Return Val: bool  - Success / failure.
// Author:      Yaki Tebeka
// Date:        30/11/2008
// ---------------------------------------------------------------------------
bool gsMakeRenderContextCurrent(int renderContextId)
{
    bool retVal = false;

    // This function is available on windows only, since glXMakeCurrent requires a drawable, which we
    // don't have.
#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
    {
        // We should not call OpenGL functions while the process is being terminated:
        bool isDuringProcessTermination = suIsDuringDebuggedProcessTermination();

        if (!isDuringProcessTermination)
        {
            oaDeviceContextHandle hDC = NULL;
            oaOpenGLRenderContextHandle hRC = NULL;
            bool validParams = true;

            if (renderContextId != AP_NULL_CONTEXT_ID)
            {
                // Get the input render context monitor:
                gsRenderContextMonitor* pRenderContextMonitor = gsOpenGLMonitor::instance().renderContextMonitor(renderContextId);

                if (pRenderContextMonitor == NULL)
                {
                    // The input context id is unknown - trigger an assertion failure:
                    gtString errorMessage = GS_STR_UnkownContextHandleUsed;
                    errorMessage.appendFormattedString(L" (context %d)", renderContextId);
                    GT_ASSERT_EX(false, errorMessage.asCharArray());
                    validParams = false;
                }
                else
                {
                    // Get the render context hDC and hRC
                    hDC = pRenderContextMonitor->deviceContextOSHandle();
                    hRC = pRenderContextMonitor->renderContextOSHandle();
                }
            }

            // Call wglMakeCurrent with the right parameters:
            GT_IF_WITH_ASSERT(validParams)
            {
                SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_wglMakeCurrent);
                BOOL rc1 = gs_stat_realFunctionPointers.wglMakeCurrent(hDC, hRC);
                SU_AFTER_EXECUTING_REAL_FUNCTION(ap_wglMakeCurrent);
                GT_IF_WITH_ASSERT(rc1 == TRUE)
                {
                    retVal = true;
                }

                if (!retVal)
                {
                    // Get the error:
                    DWORD lastError = ::GetLastError();
                    gtString debugMessage;
                    debugMessage.appendFormattedString(L"wglMakeCurrent call failed. Render context id: %d. wglMakeCurrent error id: %x", renderContextId, lastError);
                    debugMessage.appendFormattedString(L"\nwglMakeCurrent parameters:hDC (%u), hRC (%u) ", hDC, hRC);
                    OS_OUTPUT_DEBUG_LOG(debugMessage.asCharArray(), OS_DEBUG_LOG_ERROR);
                }
            }
        }
    }
#elif ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT))
    {
#ifdef _GR_IPHONE_BUILD
        {
            // We should not call OpenGL functions while the process is being terminated:
            bool isDuringProcessTermination = suIsDuringDebuggedProcessTermination();

            if (!isDuringProcessTermination)
            {
                oaOpenGLRenderContextHandle hRC = NULL;
                bool validParams = true;

                if (renderContextId != AP_NULL_CONTEXT_ID)
                {
                    // Get the input render context monitor:
                    gsRenderContextMonitor* pRenderContextMonitor = gsOpenGLMonitor::instance().renderContextMonitor(renderContextId);

                    if (pRenderContextMonitor == NULL)
                    {
                        // The input context id is unknown - trigger an assertion failure:
                        gtString errorMessage = GS_STR_UnkownContextHandleUsed;
                        errorMessage.appendFormattedString(L" (context %d)", renderContextId);
                        GT_ASSERT_EX(false, errorMessage.asCharArray());
                        validParams = false;
                    }

                    GT_IF_WITH_ASSERT(pRenderContextMonitor != NULL)
                    {
                        // Get the render context CGLContextObj:
                        hRC = pRenderContextMonitor->renderContextOSHandle();
                    }
                }

                // If we got valid Context object
                GT_IF_WITH_ASSERT(validParams)
                {
                    retVal = gsMakeEAGLContextCurrent(hRC);
                }
            }
        }
#else // ndef _GR_IPHONE_BUILD
        {
            // We should not call OpenGL functions while the process is being terminated:
            bool isDuringProcessTermination = suIsDuringDebuggedProcessTermination();

            if (!isDuringProcessTermination)
            {
                oaOpenGLRenderContextHandle hRC = NULL;
                bool validParams = true;

                if (renderContextId != AP_NULL_CONTEXT_ID)
                {
                    // Get the input render context monitor:
                    gsRenderContextMonitor* pRenderContextMonitor = gsOpenGLMonitor::instance().renderContextMonitor(renderContextId);

                    if (pRenderContextMonitor == NULL)
                    {
                        // The input context id is unknown - trigger an assertion failure:
                        gtString errorMessage = GS_STR_UnkownContextHandleUsed;
                        errorMessage.appendFormattedString(L" (context %d)", renderContextId);
                        GT_ASSERT_EX(false, errorMessage.asCharArray());
                        validParams = false;
                    }

                    GT_IF_WITH_ASSERT(pRenderContextMonitor != NULL)
                    {
                        // Get the render context CGLContextObj:
                        hRC = pRenderContextMonitor->renderContextOSHandle();
                    }
                }

                // If we got valid Context object
                GT_IF_WITH_ASSERT(validParams)
                {
                    SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_CGLSetCurrentContext);
                    CGLError errCode = gs_stat_realFunctionPointers.CGLSetCurrentContext(hRC);
                    SU_AFTER_EXECUTING_REAL_FUNCTION(ap_CGLSetCurrentContext);
                    GT_IF_WITH_ASSERT(errCode == kCGLNoError)
                    {
                        retVal = true;
                    }

                    if (!retVal)
                    {
                        // Get the error:
                        gtString debugMessage;
                        debugMessage.appendFormattedString(L"CGLSetCurrentContext(%p) call failed. Render context id: %d. CGLError id: %d", hRC, renderContextId, errCode);
                        OS_OUTPUT_DEBUG_LOG(debugMessage.asCharArray(), OS_DEBUG_LOG_ERROR);
                    }
                }
            }
        }
#endif // _GR_IPHONE_BUILD
    }
#else
    (void)(renderContextId); // Resolve the compiler warning for the Linux variant
#endif // AMDT_BUILD_TARGET

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gaBeforeDirectAPIFunctionExecutionImpl
// Description: Implementation of gaBeforeDirectAPIFunctionExecution
// Author:      Yaki Tebeka
// Date:        20/11/2005
// ---------------------------------------------------------------------------
osProcedureAddress gaBeforeDirectAPIFunctionExecutionImpl(apAPIFunctionId functionToBeCalled)
{
    osProcedureAddress retVal = NULL;

    // If we are in DEBUG debug log level - output the id of the function that is about to be executed:
    if (OS_DEBUG_LOG_DEBUG <= osDebugLog::instance().loggedSeverity())
    {
        gtString dbgStr;
        apAPIFunctionIdToString(functionToBeCalled, dbgStr);
        dbgStr.prepend(L"About to execute API function: ");
        OS_OUTPUT_DEBUG_LOG(dbgStr.asCharArray(), OS_DEBUG_LOG_DEBUG);
    }

    switch (functionToBeCalled)
    {
        case GA_FID_gaUpdateCurrentThreadRenderContextDataSnapshot:
            retVal = (osProcedureAddress)&gaUpdateCurrentThreadRenderContextDataSnapshotStub;
            break;

        case GA_FID_gaUpdateCurrentThreadStaticBufferRawData:
            retVal = (osProcedureAddress)&gaUpdateCurrentThreadStaticBufferRawDataStub;
            break;

        case GA_FID_gaUpdateCurrentThreadStaticBuffersDimensions:
            retVal = (osProcedureAddress)&gaUpdateCurrentThreadStaticBuffersDimensionsStub;
            break;

        case GA_FID_gaUpdateCurrentThreadPBuffersDimensions:
            retVal = (osProcedureAddress)&gaUpdateCurrentThreadPBuffersDimensionsStub;
            break;

        case GA_FID_gaUpdateCurrentThreadPBufferStaticBufferRawData:
            retVal = (osProcedureAddress)&gaUpdateCurrentThreadPBufferStaticBufferRawDataStub;
            break;

        case GA_FID_gaUpdateCurrentThreadTextureRawData:
            retVal = (osProcedureAddress)&gaUpdateCurrentThreadTextureRawDataStub;
            break;

        case GA_FID_gaUpdateCurrentThreadTextureParameters:
            retVal = (osProcedureAddress)&gaUpdateCurrentThreadTextureParametersStub;
            break;

        case GA_FID_gaUpdateCurrentThreadRenderBufferRawData:
            retVal = (osProcedureAddress)&gaUpdateCurrentThreadRenderBufferRawDataStub;
            break;

        case GA_FID_gaUpdateCurrentThreadVBORawData:
            retVal = (osProcedureAddress)&gaUpdateCurrentThreadVBORawDataStub;
            break;

        case GA_FID_gaSetCurrentThreadVBODisplayProperties:
            retVal = (osProcedureAddress)&gaSetCurrentThreadVBODisplayPropertiesStub;
            break;

        case GA_FID_gaSetCurrentThreadShaderObjectSourceCode:
            retVal = (osProcedureAddress)&gaSetCurrentThreadShaderObjectSourceCodeStub;
            break;

        case GA_FID_gaCompileCurrentThreadShaderObject:
            retVal = (osProcedureAddress)&gaCompileCurrentThreadShaderObjectStub;
            break;

        case GA_FID_gaLinkCurrentThreadProgramObject:
            retVal = (osProcedureAddress)&gaLinkCurrentThreadProgramObjectStub;
            break;

        case GA_FID_gaValidateCurrentThreadProgramObject:
            retVal = (osProcedureAddress)&gaValidateCurrentThreadProgramObjectStub;
            break;

        default:
            GT_ASSERT(false);
            break;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaGetAmoutOfRenderContextsImpl
// Description:
//   Implementation of gaGetAmoutOfRenderContexts()
//   See its documentation for more details.
// Author:      Yaki Tebeka
// Date:        12/5/2004
// ---------------------------------------------------------------------------
bool gaGetAmoutOfRenderContextsImpl(int& contextsAmount)
{
    // Get the amount of OpenGL render contexts created by the debugged application:
    contextsAmount = gsOpenGLMonitor::instance().amountOfContexts();

    // Debug printout:
    if (OS_DEBUG_LOG_DEBUG <= osDebugLog::instance().loggedSeverity())
    {
        gtString debugMsg = L"gaGetAmoutOfRenderContextsImpl is called. Contexts amount = ";
        debugMsg.appendFormattedString(L"%d", contextsAmount);
        OS_OUTPUT_DEBUG_LOG(debugMsg.asCharArray(), OS_DEBUG_LOG_DEBUG);
    }

    return true;
}

// ---------------------------------------------------------------------------
// Name:        gaGetRenderContextDetailsImpl
// Description: Implementation of gaRenderContextInfo.
//              See its documentation for more details.
// Return Val: bool  - Success / failure.
// Author:      Uri Shomroni
// Date:        11/6/2008
// ---------------------------------------------------------------------------
bool gaGetRenderContextDetailsImpl(int contextId, apGLRenderContextInfo& renderContextInfo)
{
    bool retVal = false;

    // Get the queried render context monitor:
    gsRenderContextMonitor* pRenderContextMonitor = gsOpenGLMonitor::instance().renderContextMonitor(contextId);

    if (pRenderContextMonitor != NULL)
    {
        // Get the render context's share lists target:
        renderContextInfo.setSpyID(contextId);
        renderContextInfo.setSharingContextID(pRenderContextMonitor->getObjectSharingContextID());
        renderContextInfo.setOpenCLSpyID(pRenderContextMonitor->openCLSharedContextID());
        renderContextInfo.setAllocatedObjectId(pRenderContextMonitor->allocatedObjectId(), true);
        retVal = true;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaGetRenderContextGraphicsDetailsImpl
// Description: Implementation of gaRenderContextGraphicsInfo.
//              See its documentation for more details.
// Return Val: bool  - Success / failure.
// Author:      Uri Shomroni
// Date:        19/3/2009
// ---------------------------------------------------------------------------
bool gaGetRenderContextGraphicsDetailsImpl(int contextId, apGLRenderContextGraphicsInfo& renderContextGraphicsInfo)
{
    bool retVal = false;

    // Get the queried render context monitor:
    const gsRenderContextMonitor* pRenderContextMonitor = gsOpenGLMonitor::instance().renderContextMonitor(contextId);

    if (pRenderContextMonitor != NULL)
    {
        retVal = pRenderContextMonitor->constructGraphicsInfo(renderContextGraphicsInfo);
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaGetThreadCurrentRenderContextImpl
// Description:
//   Implementation of gaGetThreadCurrentRenderContextImpl()
//   See its documentation for more details.
// Author:      Yaki Tebeka
// Date:        12/5/2004
// ---------------------------------------------------------------------------
bool gaGetThreadCurrentRenderContextImpl(const osThreadId& threadId, int& contextId)
{
    // Get the input thread current render context:
    contextId = gsOpenGLMonitor::instance().threadCurrentRenderContext(threadId);

    return true;
}

// ---------------------------------------------------------------------------
// Name:        gaUpdateContextDataSnapshotImpl
// Description: Implementation of gaUpdateContextDataSnapshot.
//              See its documentation for more details
// Author:      Yaki Tebeka
// Date:        17/10/2005
// ---------------------------------------------------------------------------
bool gaUpdateContextDataSnapshotImpl(int contextId)
{
    bool retVal = false;

    // This function is available on Windows and Mac only, since glXMakeCurrent requires a drawable, which we don't have:
#if ((AMDT_BUILD_TARGET == AMDT_WINDOWS_OS) || ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT)))
    {
        // Get the input render context monitor:
        suContextMonitor* pRenderContextMonitor = gsOpenGLMonitor::instance().contextMonitor(contextId);
        GT_IF_WITH_ASSERT(pRenderContextMonitor != NULL)
        {
            // Send a progress event to client:
            suSendSpyProgressEvent(AP_BEFORE_CONTEXT_MAKE_CONTEXT);

            // Make the input render context the API thread's current render context:
            bool rc1 = gsMakeRenderContextCurrent(contextId);
            GT_IF_WITH_ASSERT(rc1)
            {
                // Send a progress event to client:
                suSendSpyProgressEvent(AP_AFTER_CONTEXT_MAKE_CONTEXT);

                // Update the context's data snapshot:
                retVal = pRenderContextMonitor->updateContextDataSnapshot(true);

                // Send a progress event to client:
                suSendSpyProgressEvent(AP_BEFORE_MAKE_NULL_CONTEXT_CURRENT);

                // Release the render context
                // We consider the operation a success even if we couldn't release the Context, which shouldn't happen anyway
                bool rc2 = gsMakeRenderContextCurrent(0);
                GT_ASSERT(rc2);


                // Send a progress event to client:
                suSendSpyProgressEvent(AP_AFTER_MAKE_NULL_CONTEXT_CURRENT);
            }
        }
    }
#else
    (void)(contextId); // Resolve the compiler warning for the Linux variant
#endif // ((AMDT_BUILD_TARGET == AMDT_WINDOWS_OS) || ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT)))

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gaUpdateCurrentThreadRenderContextDataSnapshotImpl
// Description:
//   "Current thread" implementation of gaUpdateContextDataSnapshot()
//   See its documentation for more details.
// Author:      Yaki Tebeka
// Date:        11/5/2005
// ---------------------------------------------------------------------------
bool gaUpdateCurrentThreadRenderContextDataSnapshotImpl()
{
    bool retVal = false;

    // Some of the context data updates use OpenGL functions, which we obviously can't
    // use while the process is being terminated.
    bool isDuringProcessTermination = suIsDuringDebuggedProcessTermination();

    if (!isDuringProcessTermination)
    {
        // Get the current thread render context:
        gsRenderContextMonitor* pRenderContextMon = gsOpenGLMonitor::instance().currentThreadRenderContextMonitor();
        GT_IF_WITH_ASSERT(pRenderContextMon != NULL)
        {
            // If this is a real context:
            GT_IF_WITH_ASSERT(pRenderContextMon->spyId() != 0)
            {
                // Update the context data:
                retVal = pRenderContextMon->updateContextDataSnapshot(true);
            }
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gaGetDefaultOpenGLStateVariableValueImpl
// Description:
//   Implementation of gaGetDefaultOpenGLStateVariableValue()
//   See its documentation for more details.
// Author:      Yaki Tebeka
// Date:        1/5/2007
// ---------------------------------------------------------------------------
bool gaGetDefaultOpenGLStateVariableValueImpl(int contextId, int stateVariableId, const apParameter*& pDefaultStateVariableValue)
{
    bool retVal = false;

    const gsRenderContextMonitor* pRenderContextMon = gsOpenGLMonitor::instance().renderContextMonitor(contextId);

    if (pRenderContextMon)
    {
        const gsStateVariablesSnapshot& statesVariablesDefaultValues = pRenderContextMon->getStateVariablesDefaultValues();
        retVal = statesVariablesDefaultValues.getStateVariableValue(stateVariableId, pDefaultStateVariableValue);
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gaGetOpenGLStateVariableValueImpl
// Description:
//   Implementation of gaGetOpenGLStateVariableValue()
//   See its documentation for more details.
// Author:      Yaki Tebeka
// Date:        12/7/2004
// ---------------------------------------------------------------------------
bool gaGetOpenGLStateVariableValueImpl(int contextId, int stateVariableId, const apParameter*& pStateVariableValue)
{
    bool retVal = false;

    const gsRenderContextMonitor* pRenderContextMon = gsOpenGLMonitor::instance().renderContextMonitor(contextId);

    if (pRenderContextMon)
    {
        // Verify that we can access the input context state variables:
        if (!(pRenderContextMon->isInOpenGLBeginEndBlock()))
        {
            const gsStateVariablesSnapshot& statesSnapshot = pRenderContextMon->getStateVariablesSnapshot();
            retVal = statesSnapshot.getStateVariableValue(stateVariableId, pStateVariableValue);
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaGetAmountOfTextureUnitsImpl
// Description: Implementation of gaGetAmountOfTextureUnits.
//              See its documentation for more details.
// Author:      Yaki Tebeka
// Date:        19/4/2005
// ---------------------------------------------------------------------------
bool gaGetAmountOfTextureUnitsImpl(int contextId, int& amountOfTextureUnits)
{
    bool retVal = false;

    // Get the render context monitor:
    const gsRenderContextMonitor* pRenderContextMonitor = gsOpenGLMonitor::instance().renderContextMonitor(contextId);

    if (pRenderContextMonitor)
    {
        const gsTexturesMonitor* texMonitor = pRenderContextMonitor->texturesMonitor();
        GT_IF_WITH_ASSERT(texMonitor != NULL)
        {
            amountOfTextureUnits = pRenderContextMonitor->amountOfTextureUnits();
            retVal = true;
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gaGetActiveTextureUnitImpl
// Description: Implementation of gaGetActiveTextureUnit.
//              See its documentation for more details.
// Author:      Yaki Tebeka
// Date:        19/4/2005
// ---------------------------------------------------------------------------
bool gaGetActiveTextureUnitImpl(int contextId, int& activeTextureUnitId)
{
    bool retVal = false;

    // Get the render context monitor:
    const gsRenderContextMonitor* pRenderContextMonitor = gsOpenGLMonitor::instance().renderContextMonitor(contextId);

    if (pRenderContextMonitor)
    {
        const gsTexturesMonitor* texMonitor = pRenderContextMonitor->texturesMonitor();
        GT_IF_WITH_ASSERT(texMonitor != NULL)
        {
            activeTextureUnitId = pRenderContextMonitor->activeTextureUnitIndex();
            retVal = true;
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gaGetTextureUnitNameImpl
// Description: Implementation of gaGetTextureUnitName.
//              See its documentation for more details.
// Author:      Yaki Tebeka
// Date:        19/4/2005
// ---------------------------------------------------------------------------
bool gaGetTextureUnitNameImpl(int contextId, int textureUnitId, GLenum& textureUnitName)
{
    bool retVal = false;

    // Get the appropriate render context monitor:
    const gsRenderContextMonitor* pRenderContextMonitor = gsOpenGLMonitor::instance().renderContextMonitor(contextId);

    if (pRenderContextMonitor)
    {
        const gsTexturesMonitor* texMonitor = pRenderContextMonitor->texturesMonitor();
        GT_IF_WITH_ASSERT(texMonitor != NULL)
        {
            textureUnitName = pRenderContextMonitor->textureUnitIndexToName(textureUnitId);
            retVal = true;
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gaGetEnabledTexturingModeImpl
// Description:
//   Implementation of gaGetEnabledTexturingMode()
//   See its documentation for more details.
// Author:      Yaki Tebeka
// Date:        30/12/2004
// ---------------------------------------------------------------------------
bool gaGetEnabledTexturingModeImpl(int contextId, int textureUnitId, bool& isTexturingEnabled, apTextureType& enabledTexturingMode)
{
    bool retVal = false;

    // Get the appropriate render context monitor:
    const gsRenderContextMonitor* pRenderContextMonitor = gsOpenGLMonitor::instance().renderContextMonitor(contextId);

    if (pRenderContextMonitor)
    {
        retVal = pRenderContextMonitor->getEnabledTexturingMode(textureUnitId, isTexturingEnabled, enabledTexturingMode);
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gaGetAmountOfTextureObjectsImpl
// Description:
//   Implementation of gaGetAmountOfTextureObjects()
//   See its documentation for more details.
// Author:      Yaki Tebeka
// Date:        30/12/2004
// ---------------------------------------------------------------------------
bool gaGetAmountOfTextureObjectsImpl(int contextId, int& amountOfTextures)
{
    bool retVal = false;

    // Get the appropriate render context monitor:
    const gsRenderContextMonitor* pRenderContextMonitor = gsOpenGLMonitor::instance().renderContextMonitor(contextId);

    if (pRenderContextMonitor)
    {
        // Get its textures monitor:
        const gsTexturesMonitor* texturesMtr = pRenderContextMonitor->texturesMonitor();
        GT_IF_WITH_ASSERT(texturesMtr != NULL)
        {
            amountOfTextures = texturesMtr->amountOfTextureObjects();
            retVal = true;
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gaGetAmountOfTexBufferObjectsImpl
// Description:
//   Implementation of gaGetAmountOfTexBufferObjects()
//   See its documentation for more details.
// Author:      Sigal Algranaty
// Date:        2/8/2009
// ---------------------------------------------------------------------------
bool gaGetAmountOfTexBufferObjectsImpl(int contextId, int& amountOfTexBuffers)
{
    bool retVal = false;

    // Get the appropriate render context monitor:
    const gsRenderContextMonitor* pRenderContextMonitor = gsOpenGLMonitor::instance().renderContextMonitor(contextId);

    if (pRenderContextMonitor)
    {
        // Get its textures monitor:
        const gsTexturesMonitor* texturesMtr = pRenderContextMonitor->texturesMonitor();
        GT_IF_WITH_ASSERT(texturesMtr != NULL)
        {
            amountOfTexBuffers = texturesMtr->amountOfTexBufferObjects();
            retVal = true;
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaGetAmountOfRenderBufferObjectsImpl
// Description:
//   Implementation of gaGetAmountOfRenderBufferObjects()
//   See its documentation for more details.
// Author:      Sigal Algranaty
// Date:        26/5/2008
// ---------------------------------------------------------------------------
bool gaGetAmountOfRenderBufferObjectsImpl(int contextId, int& amountOfRenderBuffers)
{
    bool retVal = false;

    // Get the appropriate render context monitor:
    const gsRenderContextMonitor* pRenderContextMonitor = gsOpenGLMonitor::instance().renderContextMonitor(contextId);

    if (pRenderContextMonitor)
    {
        // Get its RBO monitor:
        const gsRenderBuffersMonitor* renderBufferMtr = pRenderContextMonitor->renderBuffersMonitor();
        GT_IF_WITH_ASSERT(renderBufferMtr != NULL)
        {
            amountOfRenderBuffers = renderBufferMtr->amountOfRenderBufferObjects();
            retVal = true;
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaGetAmountOfFBOsImpl
// Description: Implementation of gaGetAmountOfFBOs.
// Arguments: int contextId
//            int& amountOfFBOs
// Return Val: bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        24/9/2008
// ---------------------------------------------------------------------------
bool gaGetAmountOfFBOsImpl(int contextId, int& amountOfFBOs)
{
    bool retVal = false;

    // Get the appropriate render context monitor:
    const gsRenderContextMonitor* pRenderContextMonitor = gsOpenGLMonitor::instance().renderContextMonitor(contextId);
    GT_IF_WITH_ASSERT(pRenderContextMonitor != NULL)
    {
        // Get its FBO monitor:
        const gsFBOMonitor* fboMtr = pRenderContextMonitor->fboMonitor();
        GT_IF_WITH_ASSERT(fboMtr != NULL)
        {
            amountOfFBOs = fboMtr->amountOfFBOs();
            retVal = true;
        }
    }

    return retVal;
}
// ---------------------------------------------------------------------------
// Name:        gaGetAmountOfPBuffersObjectsImpl
// Description:
//   Implementation of gaGetAmountOfPBuffersObjects()
//   See its documentation for more details.
// Author:      Eran Zinman
// Date:        26/08/2007
// ---------------------------------------------------------------------------
bool gaGetAmountOfPBuffersObjectsImpl(int& amountOfPBuffers)
{
    bool retVal = true;

    // Get the PBuffer monitor
    gsPBuffersMonitor& pbuffersMonitor = gsOpenGLMonitor::instance().pbuffersMonitor();

    // Get amount of PBuffers
    amountOfPBuffers = pbuffersMonitor.amountOfPBuffersObjects();

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gaGetAmountOfSyncObjectsImpl
// Description:
//   Implementation of gaGetAmountOfSyncObjects()
//   See its documentation for more details.
// Author:      Sigal Algranaty
// Date:        28/10/2009
// ---------------------------------------------------------------------------
bool gaGetAmountOfSyncObjectsImpl(int& amountOfSyncObjects)
{
    bool retVal = true;

    // Get the PBuffer monitor
    gsSyncObjectsMonitor& syncsMonitor = gsOpenGLMonitor::instance().syncObjectsMonitor();

    // Get amount of PBuffers
    amountOfSyncObjects = syncsMonitor.amountOfSyncObjects();

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gaGetAmountOfStaticBuffersObjectsImpl
// Description:
//   Implementation of gaGetAmountOfStaticBuffersObjects()
//   See its documentation for more details.
// Author:      Eran Zinman
// Date:        26/08/2007
// ---------------------------------------------------------------------------
bool gaGetAmountOfStaticBuffersObjectsImpl(int contextId, int& amountOfStaticBuffers)
{
    bool retVal = false;

    // Get the appropriate render context monitor:
    const gsRenderContextMonitor* pRenderContextMonitor = gsOpenGLMonitor::instance().renderContextMonitor(contextId);

    if (pRenderContextMonitor)
    {
        // Get its static buffers monitor:
        const gsStaticBuffersMonitor& buffersMtr = pRenderContextMonitor->buffersMonitor();
        amountOfStaticBuffers = buffersMtr.amountOfStaticBuffers();
        retVal = true;
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gaGetAmountOfVBOsImpl
// Description: Implementation of gaGetAmountOfVBOs.
// Arguments: int contextId
//            int& amountOfVBOs
// Return Val: bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        22/10/2008
// ---------------------------------------------------------------------------
bool gaGetAmountOfVBOsImpl(int contextId, int& amountOfVBOs)
{
    bool retVal = false;

    // Get the appropriate render context monitor:
    const gsRenderContextMonitor* pRenderContextMonitor = gsOpenGLMonitor::instance().renderContextMonitor(contextId);
    GT_IF_WITH_ASSERT(pRenderContextMonitor != NULL)
    {
        // Get its VBO monitor:
        const gsVBOMonitor* vboMtr = pRenderContextMonitor->vboMonitor();
        GT_IF_WITH_ASSERT(vboMtr != NULL)
        {
            amountOfVBOs = vboMtr->amountOfVBOs();
            retVal = true;
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaGetVBONameImpl
// Description:
//   Implementation of gaGetVBOName()
//   See its documentation for more details.
// Author:      Sigal Algranaty
// Date:        22/10/2008
// ---------------------------------------------------------------------------
bool gaGetVBONameImpl(int contextId, int vboId, GLuint& vboName)
{
    bool retVal = false;

    // Get the appropriate render context monitor:
    const gsRenderContextMonitor* pRenderContextMonitor = gsOpenGLMonitor::instance().renderContextMonitor(contextId);
    GT_IF_WITH_ASSERT(pRenderContextMonitor != NULL)
    {
        // Get its FBO monitor:
        const gsVBOMonitor* vboMtr = pRenderContextMonitor->vboMonitor();
        GT_IF_WITH_ASSERT(vboMtr != NULL)
        {
            retVal = vboMtr->getVBOName(vboId, vboName);
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaUpdateCurrentThreadPBuffersDimensionsImpl
// Description: Implementation of gaUpdateCurrentThreadPBuffersDimensions()
//              See its documentation for more details.
// Author:      Sigal Algranaty
// Date:        19/10/2008
// ---------------------------------------------------------------------------
bool gaUpdateCurrentThreadPBuffersDimensionsImpl()
{
    bool retVal = false;

    // Some of the context data updates use OpenGL functions, which we obviously can't
    // use while the process is being terminated.
    bool isDuringProcessTermination = suIsDuringDebuggedProcessTermination();

    if (!isDuringProcessTermination)
    {
        // Get the current thread's related render context monitor:
        // Get its static buffers monitor:
        gsPBuffersMonitor& buffersMtr = gsOpenGLMonitor::instance().pbuffersMonitor();
        retVal = buffersMtr.updatePBuffersDimensions();
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gaUpdateCurrentThreadStaticBuffersDimensionsImpl
// Description: Implementation of gaUpdateCurrentThreadStaticBuffersDimensions()
//              See its documentation for more details.
// Author:      Sigal Algranaty
// Date:        19/10/2008
// ---------------------------------------------------------------------------
bool gaUpdateCurrentThreadStaticBuffersDimensionsImpl()
{
    bool retVal = false;

    // Some of the context data updates use OpenGL functions, which we obviously can't
    // use while the process is being terminated.
    bool isDuringProcessTermination = suIsDuringDebuggedProcessTermination();

    if (!isDuringProcessTermination)
    {

        // Get the current thread's related render context monitor:
        gsRenderContextMonitor* pRenderContextMonitor = gsOpenGLMonitor::instance().currentThreadRenderContextMonitor();

        if (pRenderContextMonitor)
        {
            // Get its static buffers monitor:
            gsStaticBuffersMonitor& buffersMtr = pRenderContextMonitor->buffersMonitor();
            retVal = buffersMtr.updateStaticBuffersDimensions();
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaUpdateCurrentThreadStaticBufferRawDataImpl
// Description: Implementation of gaUpdateCurrentThreadStaticBufferRawData()
//              See its documentation for more details.
// Author:      Yaki Tebeka
// Date:        16/10/2007
// ---------------------------------------------------------------------------
bool gaUpdateCurrentThreadStaticBufferRawDataImpl(apDisplayBuffer bufferType)
{
    bool retVal = false;

    // Some of the context data updates use OpenGL functions, which we obviously can't
    // use while the process is being terminated.
    bool isDuringProcessTermination = suIsDuringDebuggedProcessTermination();

    if (!isDuringProcessTermination)
    {
        // Get the current thread's related render context monitor:
        gsRenderContextMonitor* pRenderContextMonitor = gsOpenGLMonitor::instance().currentThreadRenderContextMonitor();

        if (pRenderContextMonitor)
        {
            // Get its static buffers monitor:
            gsStaticBuffersMonitor& buffersMtr = pRenderContextMonitor->buffersMonitor();
            retVal = buffersMtr.updateBufferRawData(bufferType);
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gaUpdateVBORawDataImpl
// Description: Implementation of gaUpdateVBORawDataImpl
//              See its documentation for more details.
// Author:      Sigal Algranaty
// Date:        60/4/2009
// ---------------------------------------------------------------------------
bool gaUpdateVBORawDataImpl(int contextId, const gtVector<GLuint>& vboNamesVector)
{
    bool retVal = false;

    // This function is available on Windows and Mac only, since glXMakeCurrent requires a drawable, which we don't have:
#if ((AMDT_BUILD_TARGET == AMDT_WINDOWS_OS) || ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT)))
    {
        // Get the input render context monitor:
        gsRenderContextMonitor* pRenderContextMonitor = gsOpenGLMonitor::instance().renderContextMonitor(contextId);
        GT_IF_WITH_ASSERT(pRenderContextMonitor != NULL)
        {
            // Get its VBO monitor:
            gsVBOMonitor* pVBOMonitor = pRenderContextMonitor->vboMonitor();
            GT_IF_WITH_ASSERT(pVBOMonitor != NULL)
            {
                // Make the input render context the API thread's current render context:
                bool rc1 = gsMakeRenderContextCurrent(contextId);
                GT_IF_WITH_ASSERT(rc1)
                {
                    retVal = true;

                    // Iterate through the VBOs and update their raw data:
                    int amountOfIndicies = (int)vboNamesVector.size();

                    for (int i = 0; i < amountOfIndicies; i++)
                    {
                        // Get VBO name and update it's raw data:
                        GLuint vboName = vboNamesVector[i];
                        apGLVBO* pVBO = pVBOMonitor->getVBODetails(vboName);

                        if (pVBO != NULL)
                        {
                            // Update the VBO raw data:
                            bool rc = pVBOMonitor->updateVBORawData(pVBO, OA_TEXEL_FORMAT_UNKNOWN);

                            retVal = retVal && rc;
                        }
                    }

                    // Release the render context
                    // We consider the operation a success even if we couldn't release the Context, which shouldn't happen anyway
                    bool rc2 = gsMakeRenderContextCurrent(0);
                    GT_ASSERT(rc2);
                }
            }
        }
    }
#else
    (void)(contextId); // Resolve the compiler warning for the Linux variant
    (void)(vboNamesVector); // Resolve the compiler warning for the Linux variant
#endif // ((AMDT_BUILD_TARGET == AMDT_WINDOWS_OS) || ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT)))

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gaUpdateCurrentThreadVBORawDataImpl
// Description: Implementation of gaUpdateCurrentThreadVBORawData()
//              See its documentation for more details.
// Author:      Sigal Algranaty
// Date:        6/4/2009
// ---------------------------------------------------------------------------
bool gaUpdateCurrentThreadVBORawDataImpl(const gtVector<GLuint>& vboNamesVector)
{
    bool retVal = false;

    // Some of the context data updates use OpenGL functions, which we obviously can't
    // use while the process is being terminated.
    bool isDuringProcessTermination = suIsDuringDebuggedProcessTermination();

    if (!isDuringProcessTermination)
    {
        // Get the current thread's related render context monitor:
        gsRenderContextMonitor* pRenderContextMonitor = gsOpenGLMonitor::instance().currentThreadRenderContextMonitor();

        if (pRenderContextMonitor)
        {
            // Get its VBO monitor:
            gsVBOMonitor* pVBOMonitor = pRenderContextMonitor->vboMonitor();
            GT_IF_WITH_ASSERT(pVBOMonitor != NULL)
            {
                retVal = true;

                // Iterate through the VBOs and update their raw data:
                int amountOfIndicies = (int)vboNamesVector.size();

                for (int i = 0; i < amountOfIndicies; i++)
                {
                    // Get VBO name and update it's raw data:
                    GLuint vboName = vboNamesVector[i];
                    apGLVBO* pVBO = pVBOMonitor->getVBODetails(vboName);

                    if (pVBO)
                    {
                        // Update the VBO raw data:
                        bool rc = pVBOMonitor->updateVBORawData(pVBO, OA_TEXEL_FORMAT_UNKNOWN);

                        retVal = retVal && rc;
                    }
                }
            }
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaSetCurrentThreadVBODisplayPropertiesImpl
// Description: Implementation of gaSetCurrentThreadVBODisplayProperties
// Arguments: GLuint vboName
//            oaTexelDataFormat bufferDisplayFormat
//            int offset
//            GLsizei stride
// Return Val: bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        26/4/2009
// ---------------------------------------------------------------------------
bool gaSetCurrentThreadVBODisplayPropertiesImpl(GLuint vboName, oaTexelDataFormat bufferDisplayFormat, int offset, GLsizei stride)
{
    bool retVal = false;

    // Some of the context data updates use OpenGL functions, which we obviously can't
    // use while the process is being terminated.
    bool isDuringProcessTermination = suIsDuringDebuggedProcessTermination();

    if (!isDuringProcessTermination)
    {
        // Get the current thread's related render context monitor:
        gsRenderContextMonitor* pRenderContextMonitor = gsOpenGLMonitor::instance().currentThreadRenderContextMonitor();

        if (pRenderContextMonitor)
        {
            // Get its VBO monitor:
            gsVBOMonitor* pVBOMonitor = pRenderContextMonitor->vboMonitor();
            GT_IF_WITH_ASSERT(pVBOMonitor != NULL)
            {
                retVal = pVBOMonitor->setVBODisplayProperties(vboName, bufferDisplayFormat, offset, stride);
            }
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaSetVBODisplayPropertiesImpl
// Description: Implementation of gaSetVBODisplayProperties
// Arguments: int contextId
//            GLuint vboName
//            oaTexelDataFormat bufferDisplayFormat
//            int offset
//            GLsizei stride
// Return Val: bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        26/4/2009
// ---------------------------------------------------------------------------
bool gaSetVBODisplayPropertiesImpl(int contextId, GLuint vboName, oaTexelDataFormat bufferDisplayFormat, int offset, GLsizei stride)
{
    bool retVal = false;

    // This function is available on Windows and Mac only, since glXMakeCurrent requires a drawable, which we don't have:
#if ((AMDT_BUILD_TARGET == AMDT_WINDOWS_OS) || ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT)))
    {
        // Get the input render context monitor:
        gsRenderContextMonitor* pRenderContextMonitor = gsOpenGLMonitor::instance().renderContextMonitor(contextId);
        GT_IF_WITH_ASSERT(pRenderContextMonitor != NULL)
        {
            // Get its VBO monitor:
            gsVBOMonitor* pVBOMonitor = pRenderContextMonitor->vboMonitor();
            GT_IF_WITH_ASSERT(pVBOMonitor != NULL)
            {
                // Make the input render context the API thread's current render context:
                bool rc1 = gsMakeRenderContextCurrent(contextId);
                GT_IF_WITH_ASSERT(rc1)
                {
                    // Set the VBO display properties:
                    retVal = pVBOMonitor->setVBODisplayProperties(vboName, bufferDisplayFormat, offset, stride);

                    // Release the render context
                    // We consider the operation a success even if we couldn't release the Context, which shouldn't happen anyway
                    bool rc2 = gsMakeRenderContextCurrent(0);
                    GT_ASSERT(rc2);
                }
            }
        }
    }
#else
    // Resolve the compiler warning for the Linux variant
    (void)(contextId);
    (void)(vboName);
    (void)(bufferDisplayFormat);
    (void)(offset);
    (void)(stride);
#endif // ((AMDT_BUILD_TARGET == AMDT_WINDOWS_OS) || ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT)))

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaUpdateRenderBufferRawDataImpl
// Description: Implementation of gaUpdateRenderBufferRawData
//              See its documentation for more details.
// Author:      Yaki Tebeka
// Date:        30/11/2008
// ---------------------------------------------------------------------------
bool gaUpdateRenderBufferRawDataImpl(int contextId, const gtVector<GLuint>& renderBuffersVector)
{
    bool retVal = false;

    // This function is available on Windows and Mac only, since glXMakeCurrent requires a drawable, which we don't have:
#if ((AMDT_BUILD_TARGET == AMDT_WINDOWS_OS) || ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT)))
    {
        // Get the input render context monitor:
        gsRenderContextMonitor* pRenderContextMonitor = gsOpenGLMonitor::instance().renderContextMonitor(contextId);
        GT_IF_WITH_ASSERT(pRenderContextMonitor != NULL)
        {
            // Get its RBO monitor:
            gsRenderBuffersMonitor* renderBuffersMtr = pRenderContextMonitor->renderBuffersMonitor();
            GT_IF_WITH_ASSERT(renderBuffersMtr != NULL)
            {
                // Make the input render context the API thread's current render context:
                bool rc1 = gsMakeRenderContextCurrent(contextId);
                GT_IF_WITH_ASSERT(rc1)
                {
                    retVal = true;

                    // Iterate through the textures and update their raw data
                    int amountOfIndicies = (int)renderBuffersVector.size();

                    for (int i = 0; i < amountOfIndicies; i++)
                    {
                        // Get texture id and update it's raw data
                        GLuint renderBufferId = renderBuffersVector[i];
                        apGLRenderBuffer* pRenderBufferObj = renderBuffersMtr->getRenderBufferObjectDetails(renderBufferId);

                        if (pRenderBufferObj)
                        {
                            // Update the render buffer raw data:
                            GLuint activeFBO = pRenderContextMonitor->getActiveReadFboName();
                            bool rc = renderBuffersMtr->updateRenderBufferRawData(pRenderBufferObj, activeFBO);

                            retVal = retVal && rc;
                        }
                    }

                    // Release the render context
                    // We consider the operation a success even if we couldn't release the Context, which shouldn't happen anyway
                    bool rc2 = gsMakeRenderContextCurrent(0);
                    GT_ASSERT(rc2);
                }
            }
        }
    }
#else
    // Resolve the compiler warning for the Linux variant
    (void)(contextId);
    (void)(renderBuffersVector);
#endif // ((AMDT_BUILD_TARGET == AMDT_WINDOWS_OS) || ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT)))

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gaUpdateCurrentThreadRenderBufferRawDataImpl
// Description: Implementation of gaUpdateCurrentThreadRenderBufferRawData()
//              See its documentation for more details.
// Author:      Sigal Algranaty
// Date:        25/6/2008
// ---------------------------------------------------------------------------
bool gaUpdateCurrentThreadRenderBufferRawDataImpl(const gtVector<GLuint>& renderBuffersVector)
{
    bool retVal = false;

    // Some of the context data updates use OpenGL functions, which we obviously can't
    // use while the process is being terminated.
    bool isDuringProcessTermination = suIsDuringDebuggedProcessTermination();

    if (!isDuringProcessTermination)
    {
        // Get the current thread's related render context monitor:
        gsRenderContextMonitor* pRenderContextMonitor = gsOpenGLMonitor::instance().currentThreadRenderContextMonitor();

        if (pRenderContextMonitor)
        {
            // Get its RBO monitor:
            gsRenderBuffersMonitor* renderBuffersMtr = pRenderContextMonitor->renderBuffersMonitor();

            GT_IF_WITH_ASSERT(renderBuffersMtr != NULL)
            {
                retVal = true;

                // Iterate through the textures and update their raw data
                int amountOfIndicies = (int)renderBuffersVector.size();

                for (int i = 0; i < amountOfIndicies; i++)
                {
                    // Get texture id and update it's raw data
                    GLuint renderBufferId = renderBuffersVector[i];
                    apGLRenderBuffer* pRenderBufferObj = renderBuffersMtr->getRenderBufferObjectDetails(renderBufferId);

                    if (pRenderBufferObj)
                    {
                        GLuint activeFBO = pRenderContextMonitor->getActiveReadFboName();

                        // Update the texture raw data:
                        bool rc = renderBuffersMtr->updateRenderBufferRawData(pRenderBufferObj, activeFBO);

                        retVal = retVal && rc;
                    }
                }
            }
        }
    }

    return retVal;
}
// ---------------------------------------------------------------------------
// Name:        gaUpdateCurrentThreadRenderBufferRawDataImpl
// Description: Implementation of gaUpdateCurrentThreadRenderBufferRawData()
//              See its documentation for more details.
// Author:      Sigal Algranaty
// Date:        25/6/2008
// ---------------------------------------------------------------------------
bool gaUpdateCurrentThreadTextureParametersImpl(const gtVector<apGLTextureMipLevelID>& texturesVector, bool shouldUpdateOnlyMemoryParams)
{
    bool retVal = false;

    // Some of the context data updates use OpenGL functions, which we obviously can't
    // use while the process is being terminated.
    bool isDuringProcessTermination = suIsDuringDebuggedProcessTermination();

    if (!isDuringProcessTermination)
    {
        // Get the current thread's related render context monitor:
        gsRenderContextMonitor* pRenderContextMonitor = gsOpenGLMonitor::instance().currentThreadRenderContextMonitor();

        if (pRenderContextMonitor)
        {
            retVal = true;

            // Actually update the parameters:
            retVal = gaUpdateTextureParametersImpl(pRenderContextMonitor, texturesVector, shouldUpdateOnlyMemoryParams);
            GT_ASSERT(retVal);
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        updateSingleTextureObject
// Description: Utility function updates single texture object. The function
//              handles 'normal' texture and texture buffer differently
// Arguments: gsTexturesMonitor* pTexturesMonitor
//            gsRenderContextMonitor* pRenderContextMonitor
//            apGLTextureMipLevelID textureId
// Return Val: bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        5/8/2009
// ---------------------------------------------------------------------------
bool updateSingleTextureObject(gsTexturesMonitor* pTexturesMonitor, gsRenderContextMonitor* pRenderContextMonitor, apGLTextureMipLevelID textureId)
{
    bool retVal = false;

    // Get the texture details:
    apGLTexture* pTextureObj = pTexturesMonitor->getTextureObjectDetails(textureId._textureName);
    GT_IF_WITH_ASSERT(pTextureObj != NULL)
    {
        // Check if the texture is a buffer texture:
        apTextureType textureType = pTextureObj->textureType();

        if (textureType == AP_BUFFER_TEXTURE)
        {
            // Update the texture raw data within the VBO monitor:
            int textureBuffer = pTextureObj->bufferName();

            if (textureBuffer != 0)
            {
                gsVBOMonitor* pVBOMonitor = pRenderContextMonitor->vboMonitor();
                GT_IF_WITH_ASSERT(pVBOMonitor != NULL)
                {
                    apGLVBO* pVBO = pVBOMonitor->getVBODetails(textureBuffer);

                    if (pVBO != NULL)
                    {
                        // Get the texture format:
                        oaTexelDataFormat textureDataFormat;
                        GLenum bufferFormat = pTextureObj->bufferInternalFormat();
                        bool rc = oaGLEnumToTexelDataFormat(bufferFormat, textureDataFormat);
                        GT_ASSERT(rc);

                        // Update the VBO raw data:
                        rc = pVBOMonitor->updateVBORawData(pVBO, textureDataFormat);
                        GT_IF_WITH_ASSERT(rc)
                        {
                            // Get the VBO file path:
                            osFilePath vboFilePath;
                            pVBO->getBufferFilePath(vboFilePath);

                            pTextureObj->updateTextureDataFile(GL_TEXTURE_BUFFER, vboFilePath);
                            retVal = true;
                        }
                    }
                }
            }
        }
        else
        {
            // Update the texture raw data:
            bool rc = pTexturesMonitor->updateTextureRawData(textureId);
            GT_IF_WITH_ASSERT(rc)
            {
                retVal = true;
            }
        }
    }
    return retVal;
}
// ---------------------------------------------------------------------------
// Name:        gaUpdateCurrentThreadTextureRawDataImpl
// Description: Implementation of gaUpdateCurrentThreadTextureRawData()
//              See its documentation for more details.
// Author:      Eran Zinman
// Date:        1/12/2007
// ---------------------------------------------------------------------------
bool gaUpdateCurrentThreadTextureRawDataImpl(const gtVector<apGLTextureMipLevelID>& texturesVector)
{
    bool retVal = false;

    // Some of the context data updates use OpenGL functions, which we obviously can't
    // use while the process is being terminated.
    bool isDuringProcessTermination = suIsDuringDebuggedProcessTermination();

    if (!isDuringProcessTermination)
    {
        // Get the current thread's related render context monitor:
        gsRenderContextMonitor* pRenderContextMonitor = gsOpenGLMonitor::instance().currentThreadRenderContextMonitor();

        if (pRenderContextMonitor)
        {
            // Get its textures monitor:
            gsTexturesMonitor* pTexturesMonitor = pRenderContextMonitor->texturesMonitor();
            GT_IF_WITH_ASSERT(pTexturesMonitor != NULL)
            {
                retVal = true;

                // Iterate through the textures and update their raw data
                int amountOfIndicies = (int)texturesVector.size();

                for (int i = 0; i < amountOfIndicies; i++)
                {
                    // Get the current texture id:
                    apGLTextureMipLevelID textureId = texturesVector[i];
                    bool rc = updateSingleTextureObject(pTexturesMonitor, pRenderContextMonitor, textureId);
                    retVal = retVal && rc;
                }
            }
        }
    }

    return retVal;
}



// ---------------------------------------------------------------------------
// Name:        gaUpdateStaticBuffersDimensionsImpl
// Description: Implementation of gaUpdateStaticBuffersDimensions()
//              See its documentation for more details.
// Author:      Sigal Algranaty
// Date:        19/10/2008
// ---------------------------------------------------------------------------
bool gaUpdateStaticBuffersDimensionsImpl(int contextId)
{
    bool retVal = false;

    // This function is available on Windows and Mac only, since glXMakeCurrent requires a drawable, which we don't have:
#if ((AMDT_BUILD_TARGET == AMDT_WINDOWS_OS) || ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT)))
    {
        // Get the input render context monitor:
        gsRenderContextMonitor* pRenderContextMonitor = gsOpenGLMonitor::instance().renderContextMonitor(contextId);
        GT_IF_WITH_ASSERT(pRenderContextMonitor != NULL)
        {
            // Make the input render context the API thread's current render context:
            bool rc1 = gsMakeRenderContextCurrent(contextId);
            GT_IF_WITH_ASSERT(rc1)
            {
                // Get its static buffers monitor, and update the static buffer:
                gsStaticBuffersMonitor& buffersMtr = pRenderContextMonitor->buffersMonitor();
                retVal = buffersMtr.updateStaticBuffersDimensions();

                // Release the render context
                // We consider the operation a success even if we couldn't release the Context, which shouldn't happen anyway
                bool rc2 = gsMakeRenderContextCurrent(0);
                GT_ASSERT(rc2);
            }
        }
    }
#else
    // Resolve the compiler warning for the Linux variant
    (void)(contextId);
#endif // ((AMDT_BUILD_TARGET == AMDT_WINDOWS_OS) || ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT)))

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gaUpdatePBuffersDimensionsImpl
// Description: Implementation of gaUpdatePBuffersDimensions()
//              See its documentation for more details.
// Author:      Sigal Algranaty
// Date:        19/10/2008
// ---------------------------------------------------------------------------
bool gaUpdatePBuffersDimensionsImpl(int contextId)
{
    bool retVal = false;

    // This function is available on Windows and Mac only, since glXMakeCurrent requires a drawable, which we don't have:
#if ((AMDT_BUILD_TARGET == AMDT_WINDOWS_OS) || ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT)))
    {
        // Get the input render context monitor:
        gsRenderContextMonitor* pRenderContextMonitor = gsOpenGLMonitor::instance().renderContextMonitor(contextId);
        GT_IF_WITH_ASSERT(pRenderContextMonitor != NULL)
        {
            // Make the input render context the API thread's current render context:
            bool rc1 = gsMakeRenderContextCurrent(contextId);
            GT_IF_WITH_ASSERT(rc1)
            {
                // Get its static buffers monitor, and update the static buffer:
                gsPBuffersMonitor& buffersMtr = gsOpenGLMonitor::instance().pbuffersMonitor();
                retVal = buffersMtr.updatePBuffersDimensions();

                // Release the render context
                // We consider the operation a success even if we couldn't release the Context, which shouldn't happen anyway
                bool rc2 = gsMakeRenderContextCurrent(0);
                GT_ASSERT(rc2);
            }
        }
    }
#else
    // Resolve the compiler warning for the Linux variant
    (void)(contextId);
#endif // ((AMDT_BUILD_TARGET == AMDT_WINDOWS_OS) || ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT)))

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gaUpdateStaticBufferRawDataImpl
// Description: Implementation of gaUpdateStaticBufferRawDataImpl()
//              See its documentation for more details.
// Author:      Eran Zinman
// Date:        23/10/2007
// ---------------------------------------------------------------------------
bool gaUpdateStaticBufferRawDataImpl(int contextId, apDisplayBuffer bufferType)
{
    bool retVal = false;

    // This function is available on Windows and Mac only, since glXMakeCurrent requires a drawable, which we don't have:
#if ((AMDT_BUILD_TARGET == AMDT_WINDOWS_OS) || ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT)))
    {
        // Get the input render context monitor:
        gsRenderContextMonitor* pRenderContextMonitor = gsOpenGLMonitor::instance().renderContextMonitor(contextId);
        GT_IF_WITH_ASSERT(pRenderContextMonitor != NULL)
        {
            // Make the input render context the API thread's current render context:
            bool rc1 = gsMakeRenderContextCurrent(contextId);
            GT_IF_WITH_ASSERT(rc1)
            {
                // Get its static buffers monitor, and update the static buffer:
                gsStaticBuffersMonitor& buffersMtr = pRenderContextMonitor->buffersMonitor();
                retVal = buffersMtr.updateBufferRawData(bufferType);

                // Release the render context
                // We consider the operation a success even if we couldn't release the Context, which shouldn't happen anyway
                bool rc2 = gsMakeRenderContextCurrent(0);
                GT_ASSERT(rc2);
            }
        }
    }
#else
    // Resolve the compiler warning for the Linux variant
    (void)(contextId);
    (void)(bufferType);
#endif // ((AMDT_BUILD_TARGET == AMDT_WINDOWS_OS) || ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT)))

    return retVal;
}



// ---------------------------------------------------------------------------
// Name:        gaUpdateCurrentThreadPBufferStaticBufferRawDataImpl
// Description:
//   Implementation of gaUpdateCurrentThreadPBufferRawData()
//   See its documentation for more details.
// Author:      Eran Zinman
// Date:        19/1/2008
// ---------------------------------------------------------------------------
bool gaUpdateCurrentThreadPBufferStaticBufferRawDataImpl(int pbufferId, apDisplayBuffer bufferType)
{
    bool retVal = false;

    // Some of the context data updates use OpenGL functions, which we obviously can't
    // use while the process is being terminated.
    bool isDuringProcessTermination = suIsDuringDebuggedProcessTermination();

    if (!isDuringProcessTermination)
    {
        // Get the PBuffers monitor
        gsPBuffersMonitor& pbuffersMtr = gsOpenGLMonitor::instance().pbuffersMonitor();

        // Get the PBuffer object according to the PBuffer id
        gsPBuffer* pbufferObject = pbuffersMtr.getPBufferObjectDetails(pbufferId);
        GT_IF_WITH_ASSERT(pbufferObject != NULL)
        {
            // Check that PBuffer is not deleted
            bool isDeleted = pbufferObject->isDeleted();
            GT_IF_WITH_ASSERT(!isDeleted)
            {
                // Get current thread render context monitor
                gsRenderContextMonitor* pRenderContextMonitor = gsOpenGLMonitor::instance().currentThreadRenderContextMonitor();
                GT_IF_WITH_ASSERT(pRenderContextMonitor != NULL)
                {
                    // Get the currently active context hDC and hRC
                    oaDeviceContextHandle activeHDC = pRenderContextMonitor->deviceContextOSHandle();
                    oaOpenGLRenderContextHandle activeHRC = pRenderContextMonitor->renderContextOSHandle();

                    // Get the PBuffer Handler and hDC
                    oaPBufferHandle pbufferHandler = pbufferObject->pbufferHandler();
                    oaDeviceContextHandle pbufferHDC = pbufferObject->deviceContextOSHandle();
                    oaOpenGLRenderContextHandle pbufferHRC;

                    // Get the PBuffer hRC
                    bool rc1 = pbufferObject->getRenderContextOSHandle(pbufferHRC);
                    GT_IF_WITH_ASSERT(rc1)
                    {
                        // Implementation notes
                        // ********************
                        //
                        // The following operation logic is:
                        //
                        // 1. Save the currently active context in the debugged application.
                        // 2. Set the PBuffer as the currently active context
                        // 3. Extract PBuffer data and save it to disk.
                        // 4. Set the original context as the active context (return to original state).

#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
                        {
                            GT_UNREFERENCED_PARAMETER(pbufferHandler);

                            // Set PBuffer as active context
                            SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_wglMakeCurrent);
                            BOOL rc2 = gs_stat_realFunctionPointers.wglMakeCurrent(pbufferHDC, pbufferHRC);
                            SU_AFTER_EXECUTING_REAL_FUNCTION(ap_wglMakeCurrent);
                            GT_IF_WITH_ASSERT(rc2 != NULL)
                            {
                                // PBuffer is currently active; update the PBuffer static buffer raw data
                                retVal = pbuffersMtr.updatePBufferStaticBufferRawData(pbufferId, bufferType);

                                // Restore previously active render context and device context
                                SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_wglMakeCurrent);
                                BOOL rc3 = gs_stat_realFunctionPointers.wglMakeCurrent(activeHDC, activeHRC);
                                SU_AFTER_EXECUTING_REAL_FUNCTION(ap_wglMakeCurrent);
                                GT_ASSERT(rc3);
                            }
                        }
#elif ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_GENERIC_LINUX_VARIANT))
                        {
                            // On Linux we have two more parameters we need to save except from current hDC and hRC.
                            // 1. "Draw" - Specifies a GLX drawable to render into. Must be an XID representing a GLXWindow, GLXPixmap, or GLXPbuffer.
                            // 2. "Read" - Specifies a GLX drawable to read from. Must be an XID representing a GLXWindow, GLXPixmap, or GLXPbuffer.

                            // Save "Draw" parameter
                            SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glXGetCurrentDrawable);
                            GLXDrawable drawHandler = gs_stat_realFunctionPointers.glXGetCurrentDrawable();
                            SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glXGetCurrentDrawable);

                            // Save "Read" parameter
                            SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glXGetCurrentReadDrawable);
                            GLXDrawable readHandler = gs_stat_realFunctionPointers.glXGetCurrentReadDrawable();
                            SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glXGetCurrentReadDrawable);

                            // Now, Set PBuffer as active context. We will use the pbufferHandler as
                            // current "read" and "write" parameters (as we need to read from the PBuffer)
                            SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glXMakeContextCurrent);
                            Bool rc2 = gs_stat_realFunctionPointers.glXMakeContextCurrent(pbufferHDC, pbufferHandler, pbufferHandler, pbufferHRC);
                            SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glXMakeContextCurrent);
                            GT_IF_WITH_ASSERT(rc2 == True)
                            {
                                // PBuffer is currently active; update the PBuffer static buffer raw data
                                retVal = pbuffersMtr.updatePBufferStaticBufferRawData(pbufferId, bufferType);

                                // Restore previously active render context and device context
                                SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glXMakeContextCurrent);
                                Bool rc3 = gs_stat_realFunctionPointers.glXMakeContextCurrent(activeHDC, drawHandler, readHandler, activeHRC);
                                SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glXMakeContextCurrent);
                                GT_ASSERT(rc3 == True);
                            }
                        }
#elif ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT))
                        {
                            // If the PBuffer is set as a drawable and the context is current, then it is already the current context, just update its raw data
                            retVal = pbuffersMtr.updatePBufferStaticBufferRawData(pbufferId, bufferType);
                        }
#endif
                    }
                }
            }
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gaUpdatePBufferStaticBufferRawDataImpl
// Description:
//   Implementation of gaUpdatePBufferStaticBufferRawData()
//   See its documentation for more details.
// Author:      Eran Zinman
// Date:        31/1/2008
// ---------------------------------------------------------------------------
bool gaUpdatePBufferStaticBufferRawDataImpl(int pbufferId, apDisplayBuffer bufferType)
{
    bool retVal = false;

    // Some of the context data updates use OpenGL functions, which we obviously can't
    // use while the process is being terminated.
    bool isDuringProcessTermination = suIsDuringDebuggedProcessTermination();

    if (!isDuringProcessTermination)
    {
        // Get the PBuffers monitor
        gsPBuffersMonitor& pbuffersMtr = gsOpenGLMonitor::instance().pbuffersMonitor();

        // Get the PBuffer object details
        gsPBuffer* pbufferObject = pbuffersMtr.getPBufferObjectDetails(pbufferId);
        GT_IF_WITH_ASSERT(pbufferObject != NULL)
        {
            // Get the PBuffer spy render context id
            int pbufferContextId = pbufferObject->pbufferContextId();
            GT_IF_WITH_ASSERT(pbufferContextId > 0)
            {
                // Get the PBuffer render context monitor according to the pbufferContextId
                const gsRenderContextMonitor* pRenderContextMonitor = gsOpenGLMonitor::instance().renderContextMonitor(pbufferContextId);
                GT_IF_WITH_ASSERT(pRenderContextMonitor != NULL)
                {
                    // Get the PBuffer Handler, hDC and hRC
                    oaPBufferHandle pbufferHandler = pbufferObject->pbufferHandler();
                    oaDeviceContextHandle pbufferHDC = pbufferObject->deviceContextOSHandle();
                    oaOpenGLRenderContextHandle pbufferHRC = pRenderContextMonitor->renderContextOSHandle();

                    // Make the PBuffer as the "current render context" and as the "current draw and read surface"
                    bool rc1 = false;

#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
                    {
                        GT_UNREFERENCED_PARAMETER(pbufferHandler);

                        // Make PBuffer hRC and hDC as current
                        SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_wglMakeCurrent);
                        BOOL rc2 = gs_stat_realFunctionPointers.wglMakeCurrent(pbufferHDC, pbufferHRC);
                        SU_AFTER_EXECUTING_REAL_FUNCTION(ap_wglMakeCurrent);

                        if (rc2 == TRUE)
                        {
                            rc1 = true;
                        }
                    }
#elif ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_GENERIC_LINUX_VARIANT))
                    {
                        // Make the PBuffer hRC and hDC as current and make the pbufferHandler as active read and draw surfaces
                        SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glXMakeContextCurrent);
                        Bool rc3 = gs_stat_realFunctionPointers.glXMakeContextCurrent(pbufferHDC, pbufferHandler, pbufferHandler, pbufferHRC);
                        SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glXMakeContextCurrent);

                        if (rc3 == True)
                        {
                            rc1 = true;
                        }
                    }
#elif ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT))
                    {
#ifdef _GR_IPHONE_BUILD
                        // Uri, 11/6/09: EAGL doesn't currently support PBuffers, we shouldn't get here
                        GT_ASSERT(false);
#else
                        // We just need to make the context current:
                        rc1 = gsMakeRenderContextCurrent(pbufferContextId);
#endif
                    }
#endif

                    // If PBuffer was made current successfully
                    GT_IF_WITH_ASSERT(rc1)
                    {
                        // Update the PBuffer static buffer raw data
                        retVal = pbuffersMtr.updatePBufferStaticBufferRawData(pbufferId, bufferType);
                    }

                    // Release the PBuffer from being active
#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
                    {
                        // Release the render context
                        bool rcRelease = gsMakeRenderContextCurrent(0);
                        GT_ASSERT(rcRelease);
                    }
#elif ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_GENERIC_LINUX_VARIANT))
                    {
                        SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glXMakeContextCurrent);
                        gs_stat_realFunctionPointers.glXMakeContextCurrent(pbufferHDC, None, None, NULL);
                        SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glXMakeContextCurrent);
                    }
#elif ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT))
                    {
#ifdef _GR_IPHONE_BUILD
                        // Uri, 11/6/09: EAGL doesn't currently support PBuffers, we shouldn't get here
                        GT_ASSERT(false);
#else
                        // Release the render context
                        bool rcRelease = gsMakeRenderContextCurrent(0);
                        GT_ASSERT(rcRelease);
#endif
                    }
#endif
                }
            }
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gaGetAmountOfPBufferStaticBuffersObjectsImpl
// Description:
//   Implementation of gaGetAmountOfPBuffersContentObjects()
//   See its documentation for more details.
// Author:      Eran Zinman
// Date:        26/08/2007
// ---------------------------------------------------------------------------
bool gaGetAmountOfPBufferStaticBuffersObjectsImpl(int pbufferId, int& amountOfStaticBuffers)
{
    bool retVal = true;

    // Get the PBuffer monitor:
    const gsPBuffersMonitor& pbufferMonitor = gsOpenGLMonitor::instance().pbuffersMonitor();

    // Get amount of static buffers inside the PBuffer object
    amountOfStaticBuffers = pbufferMonitor.amountOfPBufferContentBuffers(pbufferId);

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaGetPBufferStaticBufferTypeImpl
// Description:
//   Implementation of gaGetPBufferStaticBufferType()
//   See its documentation for more details.
// Author:      Eran Zinman
// Date:        2/1/2008
// ---------------------------------------------------------------------------
bool gaGetPBufferStaticBufferTypeImpl(int pbufferId, int staticBufferIter, apDisplayBuffer& bufferType)
{
    bool retVal = false;

    // Get the PBuffer monitor:
    const gsPBuffersMonitor& pbufferMonitor = gsOpenGLMonitor::instance().pbuffersMonitor();

    // Get the static buffer type in the PBuffer
    retVal = pbufferMonitor.getPBufferStaticBufferType(pbufferId, staticBufferIter, bufferType);

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaGetBoundTextureImpl
// Description:
//   Implementation of gaGetBoundTexture()
//   See its documentation for more details.
// Author:      Yaki Tebeka
// Date:        30/12/2004
// ---------------------------------------------------------------------------
bool gaGetBoundTextureImpl(int contextId, int textureUnitId, apTextureType bindTarget, GLuint& textureName)
{
    bool retVal = false;

    // Get the appropriate render context monitor:
    const gsRenderContextMonitor* pRenderContextMonitor = gsOpenGLMonitor::instance().renderContextMonitor(contextId);

    if (pRenderContextMonitor)
    {
        const gsTexturesMonitor* texturesMtr = pRenderContextMonitor->texturesMonitor();
        GT_IF_WITH_ASSERT(texturesMtr != NULL)
        {
            textureName = pRenderContextMonitor->bindTextureName(textureUnitId, bindTarget);
            retVal = true;
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaGetTextureObjectNameImpl
// Description:
//   Implementation of gaGetTextureObjectName()
//   See its documentation for more details.
// Author:      Yaki Tebeka
// Date:        30/12/2004
// ---------------------------------------------------------------------------
bool gaGetTextureObjectNameImpl(int contextId, int textureId, GLuint& textureName)
{
    bool retVal = false;

    // Get the appropriate render context monitor:
    const gsRenderContextMonitor* pRenderContextMonitor = gsOpenGLMonitor::instance().renderContextMonitor(contextId);

    if (pRenderContextMonitor)
    {
        // Get its textures monitor:
        const gsTexturesMonitor* texturesMtr = pRenderContextMonitor->texturesMonitor();
        GT_IF_WITH_ASSERT(texturesMtr != NULL)
        {
            retVal = texturesMtr->getTextureObjectName(textureId, textureName);
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gaGetTextureObjectTypeImpl
// Description:
//   Implementation of gaGetTextureObjectType()
//   See its documentation for more details.
// Author:      Sigal Algranaty
// Date:        2/8/2009
// ---------------------------------------------------------------------------
bool gaGetTextureObjectTypeImpl(int contextId, int textureId, apTextureType& textureType)
{
    bool retVal = false;

    // Get the appropriate render context monitor:
    const gsRenderContextMonitor* pRenderContextMonitor = gsOpenGLMonitor::instance().renderContextMonitor(contextId);

    if (pRenderContextMonitor)
    {
        // Get its textures monitor:
        const gsTexturesMonitor* texturesMtr = pRenderContextMonitor->texturesMonitor();
        GT_IF_WITH_ASSERT(texturesMtr != NULL)
        {
            retVal = texturesMtr->getTextureObjectType(textureId, textureType);
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gaGetRenderBufferObjectNameImpl
// Description:
//   Implementation of gaGetRenderBufferObjectName()
//   See its documentation for more details.
// Author:      Sigal ALgranaty
// Date:        26/5/2008
// ---------------------------------------------------------------------------
bool gaGetRenderBufferObjectNameImpl(int contextId, int renderBufferId, GLuint& renderBufferName)
{
    bool retVal = false;

    // Get the appropriate render context monitor:
    const gsRenderContextMonitor* pRenderContextMonitor = gsOpenGLMonitor::instance().renderContextMonitor(contextId);

    if (pRenderContextMonitor)
    {
        // Get its RBO monitor:
        const gsRenderBuffersMonitor* renderBuffersMtr = pRenderContextMonitor->renderBuffersMonitor();
        GT_IF_WITH_ASSERT(renderBuffersMtr != NULL)
        {
            retVal = renderBuffersMtr->getRenderBufferObjectName(renderBufferId, renderBufferName);
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaGetFBONameImpl
// Description:
//   Implementation of gaGetFBONameStub()
//   See its documentation for more details.
// Author:      Sigal ALgranaty
// Date:        4/6/2008
// ---------------------------------------------------------------------------
bool gaGetFBONameImpl(int contextId, int fboId, GLuint& fboName)
{
    bool retVal = false;

    // Get the appropriate render context monitor:
    const gsRenderContextMonitor* pRenderContextMonitor = gsOpenGLMonitor::instance().renderContextMonitor(contextId);
    GT_IF_WITH_ASSERT(pRenderContextMonitor != NULL)
    {
        // Get its FBO monitor:
        const gsFBOMonitor* fboMtr = pRenderContextMonitor->fboMonitor();
        GT_IF_WITH_ASSERT(fboMtr != NULL)
        {
            retVal = fboMtr->getFBOName(fboId, fboName);
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gaUpdateTextureRawDataImpl
// Description:
//   Implementation of gaUpdateTextureRawDataImpl()
//   See its documentation for more details.
// Author:      Eran Zinman
// Date:        1/12/2007
// ---------------------------------------------------------------------------
bool gaUpdateTextureRawDataImpl(int contextId, const gtVector<apGLTextureMipLevelID>& texturesVector)
{
    bool retVal = false;

    // This function is available on Windows and Mac only, since glXMakeCurrent requires a drawable, which we don't have:
#if ((AMDT_BUILD_TARGET == AMDT_WINDOWS_OS) || ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT)))
    {
        // Get the input render context monitor:
        gsRenderContextMonitor* pRenderContextMonitor = gsOpenGLMonitor::instance().renderContextMonitor(contextId);
        GT_IF_WITH_ASSERT(pRenderContextMonitor != NULL)
        {
            // Get its textures monitor:
            gsTexturesMonitor* pTexturesMonitor = pRenderContextMonitor->texturesMonitor();
            GT_IF_WITH_ASSERT(pTexturesMonitor != NULL)
            {
                // Make the input render context the API thread's current render context:
                bool rc1 = gsMakeRenderContextCurrent(contextId);
                GT_IF_WITH_ASSERT(rc1)
                {
                    retVal = true;

                    // Iterate through the textures and update their raw data
                    int amountOfIndicies = (int)texturesVector.size();

                    for (int i = 0; i < amountOfIndicies; i++)
                    {
                        // Get the current texture id:
                        apGLTextureMipLevelID textureId = texturesVector[i];
                        bool rc = updateSingleTextureObject(pTexturesMonitor, pRenderContextMonitor, textureId);
                        retVal = retVal && rc;
                    }

                    // Release the render context
                    // We consider the operation a success even if we couldn't release the Context, which shouldn't happen anyway
                    bool rc2 = gsMakeRenderContextCurrent(0);
                    GT_ASSERT(rc2);
                }
            }
        }
    }
#else
    // Resolve the compiler warning for the Linux variant
    (void)(contextId);
    (void)(texturesVector);
#endif // ((AMDT_BUILD_TARGET == AMDT_WINDOWS_OS) || ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT)))

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gaUpdateTextureParametersImpl
// Description: Used from gaUpdateTextureParametersImpl and gaUpdateCurrentThreadTextureParametersImpl
//              for shared code
// Arguments: gsTexturesMonitor* pTexturesMonitor - the relevant textures monitor
//            const gtVector<apGLTextureMipLevelID>& texturesVector - the vector of textures to update
// Return Val: bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        5/11/2008
// ---------------------------------------------------------------------------
bool gaUpdateTextureParametersImpl(gsRenderContextMonitor* pRenderContextMonitor, const gtVector<apGLTextureMipLevelID>& texturesVector, bool shouldUpdateOnlyMemoryParams)
{
    bool retVal = false;

    // Get its textures monitor:
    gsTexturesMonitor* texturesMtr = pRenderContextMonitor->texturesMonitor();
    GT_IF_WITH_ASSERT(texturesMtr != NULL)
    {
        retVal = true;

        // Iterate through the textures and update their raw data
        int amountOfIndicies = (int)texturesVector.size();

        for (int i = 0; i < amountOfIndicies; i++)
        {
            // Get texture id and update it's parameters:
            apGLTextureMipLevelID textureId = texturesVector[i];
            gsGLTexture* pTextureObj = texturesMtr->getTextureObjectDetails(textureId._textureName);
            GT_IF_WITH_ASSERT(pTextureObj != NULL)
            {
                // Get texture type:
                apTextureType textureType = pTextureObj->textureType();

                // Get bind target:
                GLenum bindTarget = apTextureTypeToTextureBindTarget(textureType);

                // If the texture is binded:
                if (bindTarget != GL_NONE)
                {
                    // Bind the texture for update:
                    texturesMtr->bindTextureForUpdate(textureId._textureName, bindTarget);

                    // Update the texture parameters:
                    bool rc1 = pTextureObj->updateTextureParameters(shouldUpdateOnlyMemoryParams);

                    if (!rc1)
                    {
                        gtString errorMessage;
                        errorMessage.appendFormattedString(L"Update for texture %d parameter had failed", textureId);
                        GT_ASSERT_EX(rc1, errorMessage.asCharArray());
                    }

                    GT_ASSERT(rc1);

                    // Restore the previously binded texture:
                    texturesMtr->restoreBindedTextureAfterUpdate();

                    retVal = retVal && rc1;
                }
            }
        }
    }
    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gaUpdateTextureParametersImpl
// Description: Implementation of gaUpdateTextureParametersImpl()
//   See its documentation for more details.
// Author:      Sigal Algranaty
// Date:        29/10/2008
// ---------------------------------------------------------------------------
bool gaUpdateTextureParametersImpl(int contextId, const gtVector<apGLTextureMipLevelID>& texturesVector, bool shouldUpdateOnlyMemoryParams)
{
    bool retVal = false;

    // This function is available on Windows and Mac only, since glXMakeCurrent requires a drawable, which we don't have:
#if ((AMDT_BUILD_TARGET == AMDT_WINDOWS_OS) || ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT)))
    {
        // Get the input render context monitor:
        gsRenderContextMonitor* pRenderContextMonitor = gsOpenGLMonitor::instance().renderContextMonitor(contextId);
        GT_IF_WITH_ASSERT(pRenderContextMonitor != NULL)
        {
            // Make the input render context the API thread's current render context:
            bool rc1 = gsMakeRenderContextCurrent(contextId);
            GT_IF_WITH_ASSERT(rc1)
            {
                // Actually update the parameters:
                retVal = gaUpdateTextureParametersImpl(pRenderContextMonitor, texturesVector, shouldUpdateOnlyMemoryParams);

                // Release the render context
                // We consider the operation a success even if we couldn't release the Context, which shouldn't happen anyway
                bool rc2 = gsMakeRenderContextCurrent(0);
                GT_ASSERT(rc2);
            }
        }
    }
#else
    // Resolve the compiler warning for the Linux variant
    (void)(contextId);
    (void)(texturesVector);
    (void)(shouldUpdateOnlyMemoryParams);
#endif // ((AMDT_BUILD_TARGET == AMDT_WINDOWS_OS) || ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT)))

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaGetTextureMiplevelDataFilePathImpl
// Description:
//   Implementation of gaGetTextureMiplevelDataFilePath()
//   See its documentation for more details.
// Author:      Sigal Algranaty
// Date:        13/1/2009
// ---------------------------------------------------------------------------
bool gaGetTextureMiplevelDataFilePathImpl(int contextId, apGLTextureMipLevelID miplevelId, int faceIndex, osFilePath& filePath)
{
    bool retVal = false;

    // Get the appropriate render context monitor:
    const gsRenderContextMonitor* pRenderContextMonitor = gsOpenGLMonitor::instance().renderContextMonitor(contextId);

    if (pRenderContextMonitor)
    {
        // Get its textures monitor:
        const gsTexturesMonitor* texturesMtr = pRenderContextMonitor->texturesMonitor();
        GT_IF_WITH_ASSERT(texturesMtr != NULL)
        {
            const apGLTexture* pTexObjDetails = texturesMtr->getTextureObjectDetails(miplevelId._textureName);

            if (pTexObjDetails)
            {
                retVal = pTexObjDetails->getTextureDataFilePath(filePath, faceIndex, miplevelId._textureMipLevel);
            }
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaGetTextureObjectDetailsImpl
// Description:
//   Implementation of gaGetTextureDetails()
//   See its documentation for more details.
// Author:      Yaki Tebeka
// Date:        30/12/2004
// ---------------------------------------------------------------------------
bool gaGetTextureObjectDetailsImpl(int contextId, GLuint textureName, const apGLTexture*& prTextureDetails)
{
    bool retVal = false;

    // Get the appropriate render context monitor:
    const gsRenderContextMonitor* pRenderContextMonitor = gsOpenGLMonitor::instance().renderContextMonitor(contextId);

    if (pRenderContextMonitor)
    {
        // Get its textures monitor:
        const gsTexturesMonitor* texturesMtr = pRenderContextMonitor->texturesMonitor();
        GT_IF_WITH_ASSERT(texturesMtr != NULL)
        {
            const apGLTexture* pTexObjDetails = texturesMtr->getTextureObjectDetails(textureName);

            if (pTexObjDetails)
            {
                prTextureDetails = pTexObjDetails;
                retVal = true;
            }
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaGetActiveFBOImpl
// Description:
//   Implementation of gaGetActiveFBOStub()
//   See its documentation for more details.
// Author:      Sigal Algranaty
// Date:        11/6/2008
// ---------------------------------------------------------------------------
bool gaGetActiveFBOImpl(int contextId, GLuint& fboName)
{
    bool retVal = true;

    // Get the appropriate render context monitor:
    const gsRenderContextMonitor* pRenderContextMonitor = gsOpenGLMonitor::instance().renderContextMonitor(contextId);

    if (pRenderContextMonitor)
    {
        // TO_DO: Uri, 21/10/2015 - should this be two separate API functions?
        fboName = pRenderContextMonitor->getActiveReadFboName();

        if (0 == fboName)
        {
            fboName = pRenderContextMonitor->getActiveDrawFboName();
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaGetFBODetailsImp
// Description:
//   Implementation of gaGetFBODetailsStub()
//   See its documentation for more details.
// Author:      Sigal Algranaty
// Date:        4/6/2008
// ---------------------------------------------------------------------------
bool gaGetFBODetailsImpl(int contextId, GLuint fboName, const apGLFBO*& prFboDetails)
{
    bool retVal = false;

    // Get the appropriate render context monitor:
    const gsRenderContextMonitor* pRenderContextMonitor = gsOpenGLMonitor::instance().renderContextMonitor(contextId);
    GT_IF_WITH_ASSERT(pRenderContextMonitor != NULL)
    {
        // Get its FBO monitor:
        const gsFBOMonitor* fbosMtr = pRenderContextMonitor->fboMonitor();
        GT_IF_WITH_ASSERT(fbosMtr != NULL)
        {
            const apGLFBO* pFBODetailsObj = fbosMtr->getFBODetails(fboName);

            if (pFBODetailsObj)
            {
                prFboDetails = pFBODetailsObj;
                retVal = true;
            }
        }
    }

    return retVal;
}
// ---------------------------------------------------------------------------
// Name:        gaGetRenderBufferObjectDetails
// Description:
//   Implementation of gaGetRenderBufferObjectDetails()
//   See its documentation for more details.
// Author:      Sigal Algranaty
// Date:        26/5/2008
// ---------------------------------------------------------------------------
bool gaGetRenderBufferObjectDetailsImpl(int contextId, GLuint renderBufferName, const apGLRenderBuffer*& prRenderBufferDetails)
{
    bool retVal = false;

    // Get the appropriate render context monitor:
    const gsRenderContextMonitor* pRenderContextMonitor = gsOpenGLMonitor::instance().renderContextMonitor(contextId);

    if (pRenderContextMonitor)
    {
        // Get its RBO monitor:
        const gsRenderBuffersMonitor* renderBufferMtr = pRenderContextMonitor->renderBuffersMonitor();
        GT_IF_WITH_ASSERT(renderBufferMtr != NULL)
        {
            const apGLRenderBuffer* pRenderBufferObjDetails = renderBufferMtr->getRenderBufferObjectDetails(renderBufferName);

            if (pRenderBufferObjDetails)
            {
                prRenderBufferDetails = pRenderBufferObjDetails;
                retVal = true;
            }
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaGetVBODetailsImp
// Description:
//   Implementation of gaGetVBODetailsStub()
//   See its documentation for more details.
// Author:      Sigal Algranaty
// Date:        22/10/2008
// ---------------------------------------------------------------------------
bool gaGetVBODetailsImpl(int contextId, GLuint vboName, const apGLVBO*& prVboDetails)
{
    bool retVal = false;

    // Get the appropriate render context monitor:
    const gsRenderContextMonitor* pRenderContextMonitor = gsOpenGLMonitor::instance().renderContextMonitor(contextId);
    GT_IF_WITH_ASSERT(pRenderContextMonitor != NULL)
    {
        // Get its FBO monitor:
        const gsVBOMonitor* vbosMtr = pRenderContextMonitor->vboMonitor();
        GT_IF_WITH_ASSERT(vbosMtr != NULL)
        {
            const apGLVBO* pVBODetails = vbosMtr->getVBODetails(vboName);

            if (pVBODetails != NULL)
            {
                prVboDetails = pVBODetails;
                retVal = true;
            }
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaGetVBOAttachmentImpl
// Description:
//   Implementation of gaGetVBOAttachmentStub()
//   See its documentation for more details.
// Author:      Sigal Algranaty
// Date:        22/10/2008
// ---------------------------------------------------------------------------
bool gaGetVBOAttachmentImpl(int contextId, GLuint vboName, GLenum& vboLastTarget, gtVector<GLenum>& vboCurrentTargets)
{
    bool retVal = false;

    // Get the appropriate render context monitor:
    const gsRenderContextMonitor* pRenderContextMonitor = gsOpenGLMonitor::instance().renderContextMonitor(contextId);
    GT_IF_WITH_ASSERT(pRenderContextMonitor != NULL)
    {
        // Get its FBO monitor:
        const gsVBOMonitor* vbosMtr = pRenderContextMonitor->vboMonitor();
        GT_IF_WITH_ASSERT(vbosMtr != NULL)
        {
            vboLastTarget = vbosMtr->getVBOLatestAttachment(vboName);
            vbosMtr->getAllCurrentVBOAttachments(vboName, vboCurrentTargets);
            retVal = true;
        }
    }

    return retVal;
}
// ---------------------------------------------------------------------------
// Name:        gaIsTextureImageDirtyImpl
// Description:
//   Implementation of gaIsTextureImageDirtyImpl()
//   See its documentation for more details.
// Author:      Eran Zinman
// Date:        6/2/2008
// ---------------------------------------------------------------------------
bool gaIsTextureImageDirtyImpl(int contextId, apGLTextureMipLevelID textureMiplevelId, bool& dirtyImageExists, bool& dirtyRawDataExists)
{
    bool retVal = false;

    // Get the appropriate render context monitor:
    const gsRenderContextMonitor* pRenderContextMonitor = gsOpenGLMonitor::instance().renderContextMonitor(contextId);

    if (pRenderContextMonitor)
    {
        // Get its textures monitor:
        const gsTexturesMonitor* texturesMtr = pRenderContextMonitor->texturesMonitor();

        GT_IF_WITH_ASSERT(texturesMtr != NULL)
        {
            // Get texture object
            gsGLTexture* pTexObjDetails = texturesMtr->getTextureObjectDetails(textureMiplevelId._textureName);

            if (pTexObjDetails)
            {
                // Get the mip level dirty status from the texture object:
                dirtyRawDataExists = pTexObjDetails->dirtyTextureRawDataExists(textureMiplevelId._textureMipLevel);
                dirtyImageExists = pTexObjDetails->dirtyTextureImageExists(textureMiplevelId._textureMipLevel);
                retVal = true;
            }
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gaMarkAllTextureImagesAsUpdatedImpl
// Description:
//   Implementation of gaMarkAllTextureImagesAsUpdated()
//   See its documentation for more details.
// Author:      Eran Zinman
// Date:        5/2/2008
// ---------------------------------------------------------------------------
bool gaMarkAllTextureImagesAsUpdatedImpl(int contextId, int textureId)
{
    bool retVal = false;

    // Get the appropriate render context monitor:
    const gsRenderContextMonitor* pRenderContextMonitor = gsOpenGLMonitor::instance().renderContextMonitor(contextId);

    if (pRenderContextMonitor)
    {
        // Get its textures monitor:
        const gsTexturesMonitor* texturesMtr = pRenderContextMonitor->texturesMonitor();
        GT_IF_WITH_ASSERT(texturesMtr != NULL)
        {
            // Get texture object
            apGLTexture* pTexObjDetails = texturesMtr->getTextureObjectDetails(textureId);

            if (pTexObjDetails != NULL)
            {
                // Mark the texture images as updated for level 0:
                pTexObjDetails->markAllTextureImagesAsUpdated(0);
                retVal = true;
            }
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaGetStaticBufferObjectDetailsImpl
// Description:
//   Implementation of gaGetStaticBufferObjectDetails()
//   See its documentation for more details.
// Author:      Eran Zinman
// Date:        27/08/2007
// ---------------------------------------------------------------------------
bool gaGetStaticBufferObjectDetailsImpl(int contextId, apDisplayBuffer bufferType, const apStaticBuffer*& prStaticBuffer)
{
    bool retVal = false;

    // Get the appropriate render context monitor:
    const gsRenderContextMonitor* pRenderContextMonitor = gsOpenGLMonitor::instance().renderContextMonitor(contextId);

    if (pRenderContextMonitor)
    {
        // Get its textures monitor:
        const gsStaticBuffersMonitor& buffersMtr = pRenderContextMonitor->buffersMonitor();
        const apStaticBuffer* pStatBufferObjDetails = buffersMtr.getStaticBufferObjectDetails(bufferType);

        if (pStatBufferObjDetails)
        {
            prStaticBuffer = pStatBufferObjDetails;
            retVal = true;
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaGetStaticBufferObjectDetailsImpl
// Description:
//   Implementation of gaGetStaticBufferType()
//   See its documentation for more details.
// Author:      Eran Zinman
// Date:        25/11/2007
// ---------------------------------------------------------------------------
bool gaGetStaticBufferTypeImpl(int contextId, int bufferId, apDisplayBuffer& bufferType)
{
    bool retVal = false;

    // Get the appropriate render context monitor:
    const gsRenderContextMonitor* pRenderContextMonitor = gsOpenGLMonitor::instance().renderContextMonitor(contextId);

    if (pRenderContextMonitor)
    {
        // Get its textures monitor:
        const gsStaticBuffersMonitor& buffersMtr = pRenderContextMonitor->buffersMonitor();
        const apStaticBuffer* pStatBufferObjDetails = buffersMtr.getStaticBufferObjectDetails(bufferId);

        if (pStatBufferObjDetails)
        {
            bufferType = pStatBufferObjDetails->bufferType();
            retVal = true;
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaGetPBufferStaticBufferObjectDetailsImpl
// Description:
//   Implementation of gaGetPBufferStaticBufferDetails()
//   See its documentation for more details.
// Author:      Eran Zinman
// Date:        27/08/2007
// ---------------------------------------------------------------------------
bool gaGetPBufferStaticBufferObjectDetailsImpl(int pbufferId, apDisplayBuffer bufferType, const apStaticBuffer*& prStaticBuffer)
{
    bool retVal = false;

    // Get the PBuffer monitor:
    const gsPBuffersMonitor& pbufferMtr = gsOpenGLMonitor::instance().pbuffersMonitor();
    const apStaticBuffer* pStatBufferObjDetails = pbufferMtr.getPBufferStaticBufferObjectDetails(pbufferId, bufferType);

    if (pStatBufferObjDetails)
    {
        prStaticBuffer = pStatBufferObjDetails;
        retVal = true;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaGetPBufferObjectDetailsImpl
// Description:
//   Implementation of gaGetPBufferDetails()
//   See its documentation for more details.
// Author:      Eran Zinman
// Date:        28/08/2007
// ---------------------------------------------------------------------------
bool gaGetPBufferObjectDetailsImpl(int pbufferId, const apPBuffer*& prPBufferDetails)
{
    bool retVal = false;

    // Get the PBuffer monitor:
    const gsPBuffersMonitor& pbufferMonitor = gsOpenGLMonitor::instance().pbuffersMonitor();

    // Get the specific PBuffer details:
    const apPBuffer* pbufferObjDetails = pbufferMonitor.getPBufferObjectDetails(pbufferId);

    if (pbufferObjDetails)
    {
        prPBufferDetails = pbufferObjDetails;
        retVal = true;
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gaGetSyncObjectDetailsImpl
// Description:
//   Implementation of gaGetSyncObjectDetails()
//   See its documentation for more details.
// Author:      Sigal Algranaty
// Date:        28/10/2009
// ---------------------------------------------------------------------------
bool gaGetSyncObjectDetailsImpl(int syncObjectIndex, const apGLSync*& pSyncDetails)
{
    bool retVal = false;

    pSyncDetails = NULL;

    // Get the PBuffer monitor:
    const gsSyncObjectsMonitor& syncsMonitor = gsOpenGLMonitor::instance().syncObjectsMonitor();

    // Get the specific sync object details:
    pSyncDetails = syncsMonitor.getSyncObjectDetails(syncObjectIndex);

    if (pSyncDetails)
    {
        retVal = true;
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gaGetAmountOfProgramObjectsImpl
// Description:
//   Implementation of gaGetAmountOfProgramObjects.
//   See its documentation for more details.
// Author:      Yaki Tebeka
// Date:        27/4/2005
// ---------------------------------------------------------------------------
bool gaGetAmountOfProgramObjectsImpl(int contextId, int& amountOfPrograms)
{
    bool retVal = false;

    // Get the queried context monitor:
    const gsRenderContextMonitor* pRenderContextMtr = gsOpenGLMonitor::instance().renderContextMonitor(contextId);

    if (pRenderContextMtr)
    {
        // Get its Programs and Shaders monitor:
        const gsProgramsAndShadersMonitor* programsAndShadersMtr = pRenderContextMtr->programsAndShadersMonitor();

        GT_IF_WITH_ASSERT(programsAndShadersMtr != NULL)
        {
            // Get the amount of program objects:
            amountOfPrograms = programsAndShadersMtr->amountOfProgramObjects();

            retVal = true;
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gaGetActiveProgramObjectNameImpl
// Description:
//  Implementation of gaGetActiveProgramObjectName.
//  See its documentation for more details.
// Author:      Yaki Tebeka
// Date:        7/6/2005
// ---------------------------------------------------------------------------
bool gaGetActiveProgramObjectNameImpl(int contextId, GLuint& activeProgramName)
{
    bool retVal = false;

    // Get the queried context monitor:
    const gsRenderContextMonitor* pRenderContextMtr = gsOpenGLMonitor::instance().renderContextMonitor(contextId);

    if (pRenderContextMtr)
    {
        // Get the active program name:
        activeProgramName = pRenderContextMtr->activeProgramName();
        retVal = true;
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gaGetAmountOfProgramObjectsImpl
// Description:
//   Implementation of gaGetAmountOfProgramObjects.
//   See its documentation for more details.
// Author:      Yaki Tebeka
// Date:        27/4/2005
// ---------------------------------------------------------------------------
bool gaGetProgramObjectNameImpl(int contextId, int programId, GLuint& programName)
{
    bool retVal = false;

    // Get the queried context monitor:
    const gsRenderContextMonitor* pRenderContextMtr = gsOpenGLMonitor::instance().renderContextMonitor(contextId);

    if (pRenderContextMtr)
    {
        // Get its Programs and Shaders monitor:
        const gsProgramsAndShadersMonitor* programsAndShadersMtr = pRenderContextMtr->programsAndShadersMonitor();

        GT_IF_WITH_ASSERT(programsAndShadersMtr != NULL)
        {
            // Get the program name:
            programName = programsAndShadersMtr->programObjectName(programId);

            if (programName != 0)
            {
                retVal = true;
            }
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gaGetProgramObjectDetailsImpl
// Description:
//   Implementation of gaGetProgramObjectDetails.
//   See its documentation for more details.
// Author:      Yaki Tebeka
// Date:        27/4/2005
// ---------------------------------------------------------------------------
bool gaGetProgramObjectDetailsImpl(int contextId, GLuint programName, const apGLProgram*& prProgramDetails)
{
    bool retVal = false;

    // Get the queried context monitor:
    const gsRenderContextMonitor* pRenderContextMtr = gsOpenGLMonitor::instance().renderContextMonitor(contextId);

    if (pRenderContextMtr)
    {
        // Get its Programs and Shaders monitor:
        const gsProgramsAndShadersMonitor* programsAndShadersMtr = pRenderContextMtr->programsAndShadersMonitor();

        GT_IF_WITH_ASSERT(programsAndShadersMtr != NULL)
        {
            // Get the program details:
            const apGLProgram* pProgObjDetails = programsAndShadersMtr->programObjectDetails(programName);

            if (pProgObjDetails)
            {
                // Output the program details:
                prProgramDetails = pProgObjDetails;

                retVal = true;
            }
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gaGetProgramActiveUniformsImpl
// Description:
//   Implementation of gaGetProgramActiveUniforms.
//   See its documentation for more details.
// Author:      Yaki Tebeka
// Date:        30/5/2005
// ---------------------------------------------------------------------------
bool gaGetProgramActiveUniformsImpl(int contextId, GLuint programName, const apGLItemsCollection*& activeUniforms)
{
    bool retVal = false;

    // Get the queried context monitor:
    const gsRenderContextMonitor* pRenderContextMtr = gsOpenGLMonitor::instance().renderContextMonitor(contextId);

    if (pRenderContextMtr)
    {
        // Get the Active uniforms monitor:
        const gsProgramsAndShadersMonitor* programsAndShadersMtr = pRenderContextMtr->programsAndShadersMonitor();
        GT_IF_WITH_ASSERT(programsAndShadersMtr != NULL)
        {
            const gsActiveUniformsMonitor& activeUniformsMtr = programsAndShadersMtr->programsActiveUniformsMgr();

            // Get the program active uniforms:
            retVal = activeUniformsMtr.getProgramActiveUniforms(programName, activeUniforms);
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaLinkProgramObjectImpl
// Description:
//   Implementation of gaLinkProgramObject. See its documentation for more details.
// Author:      Uri Shomroni
// Date:        4/11/2009
// ---------------------------------------------------------------------------
bool gaLinkProgramObjectImpl(int contextId, GLuint programName, bool& wasLinkSuccessful, gtString& linkLog)
{
    bool retVal = false;

    // This function is available on Windows and Mac only, since glXMakeCurrent requires a drawable, which we don't have:
#if ((AMDT_BUILD_TARGET == AMDT_WINDOWS_OS) || ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT)))
    {
        // Get this thread render context monitor:
        gsRenderContextMonitor* pCurrentThreadRenderContextMtr = gsOpenGLMonitor::instance().renderContextMonitor(contextId);
        GT_IF_WITH_ASSERT(pCurrentThreadRenderContextMtr != NULL)
        {
            // Get the context programs and shaders monitor:
            gsProgramsAndShadersMonitor* progsAndShadersMtr = pCurrentThreadRenderContextMtr->programsAndShadersMonitor();

            GT_IF_WITH_ASSERT(progsAndShadersMtr != NULL)
            {
                // Make the input render context the API thread's current render context:
                bool rc1 = gsMakeRenderContextCurrent(contextId);
                GT_IF_WITH_ASSERT(rc1)
                {
                    // Link the program:
                    GLuint activeProgramName = pCurrentThreadRenderContextMtr->activeProgramName();
                    retVal = progsAndShadersMtr->linkProgramObject(programName, wasLinkSuccessful, linkLog, activeProgramName);

                    // Release the render context
                    // We consider the operation a success even if we couldn't release the Context, which shouldn't happen anyway
                    bool rc2 = gsMakeRenderContextCurrent(0);
                    GT_ASSERT(rc2);
                }
            }
        }
    }
#else
    // Resolve the compiler warning for the Linux variant
    (void)(contextId);
    (void)(programName);
    (void)(wasLinkSuccessful);
    (void)(linkLog);
#endif // ((AMDT_BUILD_TARGET == AMDT_WINDOWS_OS) || ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT)))

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaLinkCurrentThreadProgramObjectImpl
// Description:
//   Implementation of gaLinkCurrentThreadProgramObject. See its documentation for more details.
// Author:      Yaki Tebeka
// Date:        15/11/2005
// ---------------------------------------------------------------------------
bool gaLinkCurrentThreadProgramObjectImpl(GLuint programName, bool& wasLinkSuccessful, gtString& linkLog)
{
    bool retVal = false;

    // Get this thread render context monitor:
    gsRenderContextMonitor* pCurrentThreadRenderContextMtr = gsOpenGLMonitor::instance().currentThreadRenderContextMonitor();
    GT_IF_WITH_ASSERT(pCurrentThreadRenderContextMtr != NULL)
    {
        // Get the context programs and shaders monitor:
        gsProgramsAndShadersMonitor* progsAndShadersMtr = pCurrentThreadRenderContextMtr->programsAndShadersMonitor();

        GT_IF_WITH_ASSERT(progsAndShadersMtr != NULL)
        {
            // Link the program:
            GLuint activeProgramName = pCurrentThreadRenderContextMtr->activeProgramName();
            retVal = progsAndShadersMtr->linkProgramObject(programName, wasLinkSuccessful, linkLog, activeProgramName);
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaValidateProgramObjectImpl
// Description:
//   Implementation of gaValidateProgramObject. See its documentation for more details.
// Author:      Uri Shomroni
// Date:        4/11/2009
// ---------------------------------------------------------------------------
bool gaValidateProgramObjectImpl(int contextId, GLuint programName, bool& wasValidationSuccessful, gtString& validationLog)
{
    bool retVal = false;

    // This function is available on Windows and Mac only, since glXMakeCurrent requires a drawable, which we don't have:
#if ((AMDT_BUILD_TARGET == AMDT_WINDOWS_OS) || ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT)))
    {
        // Get this thread render context monitor:
        gsRenderContextMonitor* pCurrentThreadRenderContextMtr = gsOpenGLMonitor::instance().renderContextMonitor(contextId);
        GT_IF_WITH_ASSERT(pCurrentThreadRenderContextMtr != NULL)
        {
            // Get the context programs and shaders monitor:
            gsProgramsAndShadersMonitor* progsAndShadersMtr = pCurrentThreadRenderContextMtr->programsAndShadersMonitor();

            GT_IF_WITH_ASSERT(progsAndShadersMtr != NULL)
            {
                // Make the input render context the API thread's current render context:
                bool rc1 = gsMakeRenderContextCurrent(contextId);
                GT_IF_WITH_ASSERT(rc1)
                {
                    // Set the shader source code:
                    retVal = progsAndShadersMtr->validateProgramObject(programName, wasValidationSuccessful, validationLog);

                    // Release the render context
                    // We consider the operation a success even if we couldn't release the Context, which shouldn't happen anyway
                    bool rc2 = gsMakeRenderContextCurrent(0);
                    GT_ASSERT(rc2);
                }
            }
        }
    }
#else
    // Resolve the compiler warning for the Linux variant
    (void)(contextId);
    (void)(programName);
    (void)(wasValidationSuccessful);
    (void)(validationLog);
#endif // ((AMDT_BUILD_TARGET == AMDT_WINDOWS_OS) || ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT)))

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaValidateCurrentThreadProgramObjectImpl
// Description:
//   Implementation of gaValidateCurrentThreadProgramObject. See its documentation for more details.
// Author:      Yaki Tebeka
// Date:        15/11/2005
// ---------------------------------------------------------------------------
bool gaValidateCurrentThreadProgramObjectImpl(GLuint programName, bool& wasValidationSuccessful, gtString& validationLog)
{
    bool retVal = false;

    // Get this thread render context monitor:
    gsRenderContextMonitor* pCurrentThreadRenderContextMtr = gsOpenGLMonitor::instance().currentThreadRenderContextMonitor();
    GT_IF_WITH_ASSERT(pCurrentThreadRenderContextMtr != NULL)
    {
        // Get the context programs and shaders monitor:
        gsProgramsAndShadersMonitor* progsAndShadersMtr = pCurrentThreadRenderContextMtr->programsAndShadersMonitor();

        GT_IF_WITH_ASSERT(progsAndShadersMtr != NULL)
        {
            // Set the shader source code:
            retVal = progsAndShadersMtr->validateProgramObject(programName, wasValidationSuccessful, validationLog);
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gaGetAmountOfShaderObjectsImpl
// Description:
//   Implementation of gaGetAmountOfShaderObjects.
//   See its documentation for more details.
// Author:      Yaki Tebeka
// Date:        27/4/2005
// ---------------------------------------------------------------------------
bool gaGetAmountOfShaderObjectsImpl(int contextId, int& amountOfShaders)
{
    bool retVal = false;

    // Get the queried context monitor:
    const gsRenderContextMonitor* pRenderContextMtr = gsOpenGLMonitor::instance().renderContextMonitor(contextId);

    if (pRenderContextMtr)
    {
        // Get its Programs and Shaders monitor:
        const gsProgramsAndShadersMonitor* programsAndShadersMtr = pRenderContextMtr->programsAndShadersMonitor();

        GT_IF_WITH_ASSERT(programsAndShadersMtr != NULL)
        {
            // Get the amount of shader objects:
            amountOfShaders = programsAndShadersMtr->amountOfShaderObjects();

            retVal = true;
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gaGetShaderObjectNameImpl
// Description:
//   Implementation of gaGetShaderObjectName.
//   See its documentation for more details.
// Author:      Yaki Tebeka
// Date:        27/4/2005
// ---------------------------------------------------------------------------
bool gaGetShaderObjectNameImpl(int contextId, int shaderId, GLuint& shaderName)
{
    bool retVal = false;

    // Get the queried context monitor:
    const gsRenderContextMonitor* pRenderContextMtr = gsOpenGLMonitor::instance().renderContextMonitor(contextId);

    if (pRenderContextMtr)
    {
        // Get its Programs and Shaders monitor:
        const gsProgramsAndShadersMonitor* programsAndShadersMtr = pRenderContextMtr->programsAndShadersMonitor();

        GT_IF_WITH_ASSERT(programsAndShadersMtr != NULL)
        {
            // Get the shader object name:
            shaderName = programsAndShadersMtr->shaderObjectName(shaderId);

            if (shaderName != 0)
            {
                retVal = true;
            }
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gaGetShaderObjectDetailsImpl
// Description:
//   Implementation of gaGetShaderObjectDetails.
//   See its documentation for more details.
// Author:      Yaki Tebeka
// Date:        27/4/2005
// ---------------------------------------------------------------------------
bool gaGetShaderObjectDetailsImpl(int contextId, GLuint shaderName, gtAutoPtr<apGLShaderObject>& aptrShaderDetails)
{
    bool retVal = false;

    // Get the queried context monitor:
    const gsRenderContextMonitor* pRenderContextMtr = gsOpenGLMonitor::instance().renderContextMonitor(contextId);
    GT_IF_WITH_ASSERT(pRenderContextMtr != NULL)
    {
        // Get its Programs and Shaders monitor:
        const gsProgramsAndShadersMonitor* programsAndShadersMtr = pRenderContextMtr->programsAndShadersMonitor();
        GT_IF_WITH_ASSERT(programsAndShadersMtr != NULL)
        {
            // Get the shader details:
            const apGLShaderObject* pShaderObjDetails = programsAndShadersMtr->shaderObjectDetails(shaderName);
            GT_IF_WITH_ASSERT(pShaderObjDetails != NULL)
            {
                // Clone the shader object details:
                apGLShaderObject* pShaderObjDetailsClone = (apGLShaderObject*)(pShaderObjDetails->clone());
                GT_IF_WITH_ASSERT(pShaderObjDetailsClone != NULL)
                {
                    // Output the shader object details:
                    aptrShaderDetails = pShaderObjDetailsClone;

                    retVal = true;
                }
            }
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gaMarkShaderObjectSourceCodeAsForcedImpl
//   Implementation of gaMarkShaderObjectSourceCodeAsForced.
//   See its documentation for more details.
// Author:      Yaki Tebeka
// Date:        15/11/2005
// ---------------------------------------------------------------------------
bool gaMarkShaderObjectSourceCodeAsForcedImpl(int contextId, GLuint shaderName, bool isSourceCodeForced)
{
    bool retVal = false;

    // Get the queried context monitor:
    gsRenderContextMonitor* pRenderContextMtr = gsOpenGLMonitor::instance().renderContextMonitor(contextId);

    if (pRenderContextMtr)
    {
        // Get its Programs and Shaders monitor:
        gsProgramsAndShadersMonitor* programsAndShadersMtr = pRenderContextMtr->programsAndShadersMonitor();

        GT_IF_WITH_ASSERT(programsAndShadersMtr != NULL)
        {
            // Mark the shader object source code as "forced":
            retVal = programsAndShadersMtr->markShaderObjectSourceCodeAsForced(shaderName, isSourceCodeForced);
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaSetShaderObjectSourceCodeImpl
// Description:
//   Implementation of gaSetShaderObjectSourceCode.
//   See its documentation for more details.
// Author:      Uri Shomroni
// Date:        4/11/2009
// ---------------------------------------------------------------------------
bool gaSetShaderObjectSourceCodeImpl(int contextId, GLuint shaderName, const osFilePath& sourceCodeFile)
{
    bool retVal = false;

    // This function is available on Windows and Mac only, since glXMakeCurrent requires a drawable, which we don't have:
#if ((AMDT_BUILD_TARGET == AMDT_WINDOWS_OS) || ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT)))
    {
        // Get this thread render context monitor:
        gsRenderContextMonitor* pCurrentThreadRenderContextMtr = gsOpenGLMonitor::instance().renderContextMonitor(contextId);
        GT_IF_WITH_ASSERT(pCurrentThreadRenderContextMtr != NULL)
        {
            // Get the context programs and shaders monitor:
            gsProgramsAndShadersMonitor* progsAndShadersMtr = pCurrentThreadRenderContextMtr->programsAndShadersMonitor();

            GT_IF_WITH_ASSERT(progsAndShadersMtr != NULL)
            {
                // Make the input render context the API thread's current render context:
                bool rc1 = gsMakeRenderContextCurrent(contextId);
                GT_IF_WITH_ASSERT(rc1)
                {
                    // Set the shader source code:
                    retVal = progsAndShadersMtr->setShaderObjectSourceCode(shaderName, sourceCodeFile);

                    // Release the render context
                    // We consider the operation a success even if we couldn't release the Context, which shouldn't happen anyway
                    bool rc2 = gsMakeRenderContextCurrent(0);
                    GT_ASSERT(rc2);
                }
            }
        }
    }
#else
    // Resolve the compiler warning for the Linux variant
    (void)(contextId);
    (void)(shaderName);
    (void)(sourceCodeFile);
#endif // ((AMDT_BUILD_TARGET == AMDT_WINDOWS_OS) || ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT)))

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaSetCurrentThreadShaderObjectSourceCodeImpl
// Description:
//   Implementation of gaSetCurrentThreadShaderObjectSourceCode.
//   See its documentation for more details.
// Author:      Yaki Tebeka
// Date:        20/11/2005
// ---------------------------------------------------------------------------
bool gaSetCurrentThreadShaderObjectSourceCodeImpl(GLuint shaderName, const osFilePath& sourceCodeFile)
{
    bool retVal = false;

    // Get this thread render context monitor:
    gsRenderContextMonitor* pCurrentThreadRenderContextMtr = gsOpenGLMonitor::instance().currentThreadRenderContextMonitor();
    GT_IF_WITH_ASSERT(pCurrentThreadRenderContextMtr != NULL)
    {
        // Get the context programs and shaders monitor:
        gsProgramsAndShadersMonitor* progsAndShadersMtr = pCurrentThreadRenderContextMtr->programsAndShadersMonitor();

        GT_IF_WITH_ASSERT(progsAndShadersMtr != NULL)
        {
            // Set the shader source code:
            retVal = progsAndShadersMtr->setShaderObjectSourceCode(shaderName, sourceCodeFile);
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaCompileShaderObjectImpl
//   Implementation of gaCompileShaderObject.
//   See its documentation for more details.
// Author:      Uri Shomroni
// Date:        4/11/2009
// ---------------------------------------------------------------------------
bool gaCompileShaderObjectImpl(int contextId, GLuint shaderName, bool& wasCompilationSuccessful, gtString& compilationLog)
{
    bool retVal = false;

    // This function is available on Windows and Mac only, since glXMakeCurrent requires a drawable, which we don't have:
#if ((AMDT_BUILD_TARGET == AMDT_WINDOWS_OS) || ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT)))
    {
        // Get this thread render context monitor:
        gsRenderContextMonitor* pCurrentThreadRenderContextMtr = gsOpenGLMonitor::instance().renderContextMonitor(contextId);
        GT_IF_WITH_ASSERT(pCurrentThreadRenderContextMtr != NULL)
        {
            // Get the context programs and shaders monitor:
            gsProgramsAndShadersMonitor* progsAndShadersMtr = pCurrentThreadRenderContextMtr->programsAndShadersMonitor();

            GT_IF_WITH_ASSERT(progsAndShadersMtr != NULL)
            {
                // Make the input render context the API thread's current render context:
                bool rc1 = gsMakeRenderContextCurrent(contextId);
                GT_IF_WITH_ASSERT(rc1)
                {
                    // Compile the shader:
                    retVal = progsAndShadersMtr->compileShaderObject(shaderName, wasCompilationSuccessful, compilationLog);

                    // Release the render context
                    // We consider the operation a success even if we couldn't release the Context, which shouldn't happen anyway
                    bool rc2 = gsMakeRenderContextCurrent(0);
                    GT_ASSERT(rc2);
                }
            }
        }
    }
#else
    // Resolve the compiler warning for the Linux variant
    (void)(contextId);
    (void)(shaderName);
    (void)(wasCompilationSuccessful);
    (void)(compilationLog);
#endif // ((AMDT_BUILD_TARGET == AMDT_WINDOWS_OS) || ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT)))

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaCompileCurrentThreadShaderObjectImpl
//   Implementation of gaCompileCurrentThreadShaderObject.
//   See its documentation for more details.
// Author:      Yaki Tebeka
// Date:        15/11/2005
// ---------------------------------------------------------------------------
bool gaCompileCurrentThreadShaderObjectImpl(GLuint shaderName, bool& wasCompilationSuccessful, gtString& compilationLog)
{
    bool retVal = false;

    // Get this thread render context monitor:
    gsRenderContextMonitor* pCurrentThreadRenderContextMtr = gsOpenGLMonitor::instance().currentThreadRenderContextMonitor();
    GT_IF_WITH_ASSERT(pCurrentThreadRenderContextMtr != NULL)
    {
        // Get the context programs and shaders monitor:
        gsProgramsAndShadersMonitor* progsAndShadersMtr = pCurrentThreadRenderContextMtr->programsAndShadersMonitor();

        GT_IF_WITH_ASSERT(progsAndShadersMtr != NULL)
        {
            // Compile the shader:
            retVal = progsAndShadersMtr->compileShaderObject(shaderName, wasCompilationSuccessful, compilationLog);
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gaGetAmountOfDisplayListsImpl
// Description: Implementation of gaGetAmountOfDisplayLists.
// Arguments: int contextId
//            int& amountOfDisplayLists
// Return Val: bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        24/9/2008
// ---------------------------------------------------------------------------
bool gaGetAmountOfDisplayListsImpl(int contextId, int& amountOfDisplayLists)
{
    bool retVal = false;

    // Get the appropriate render context monitor:
    const gsRenderContextMonitor* pRenderContextMonitor = gsOpenGLMonitor::instance().renderContextMonitor(contextId);
    GT_IF_WITH_ASSERT(pRenderContextMonitor != NULL)
    {
        // Get its Display lists monitor:
        const gsDisplayListMonitor* pDisplayListMonitor = pRenderContextMonitor->displayListsMonitor();
        GT_IF_WITH_ASSERT(pDisplayListMonitor != NULL)
        {
            amountOfDisplayLists = pDisplayListMonitor->amountOfDisplayLists();
            retVal = true;
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaGetDisplayListObjectNameImpl
// Description: Implementation of gaGetDisplayListObjectName. See its documentation
//              For more details
// Author:      Uri Shomroni
// Date:        17/11/2008
// ---------------------------------------------------------------------------
bool gaGetDisplayListObjectNameImpl(int contextID, int displayListIndex, GLuint& displayListName)
{
    bool retVal = false;

    // Get the appropriate render context monitor:
    const gsRenderContextMonitor* pRenderContextMonitor = gsOpenGLMonitor::instance().renderContextMonitor(contextID);
    GT_IF_WITH_ASSERT(pRenderContextMonitor != NULL)
    {
        // Get its Display lists monitor:
        const gsDisplayListMonitor* pDisplayListMtr = pRenderContextMonitor->displayListsMonitor();
        GT_IF_WITH_ASSERT(pDisplayListMtr != NULL)
        {
            retVal = pDisplayListMtr->getDisplayListName(displayListIndex, displayListName);
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaGetDisplayListObjectDetailsImpl
// Description: Implementation of gaCompileShaderObject.
// Arguments: int contextId
//            GLuint displayListName
//            apGLDisplayList*& pDisplayListDetails
// Return Val: bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        24/9/2008
// ---------------------------------------------------------------------------
bool gaGetDisplayListObjectDetailsImpl(int contextId, GLuint displayListName, const apGLDisplayList*& prDisplayListDetails)
{
    bool retVal = false;

    // Get the appropriate render context monitor:
    const gsRenderContextMonitor* pRenderContextMonitor = gsOpenGLMonitor::instance().renderContextMonitor(contextId);
    GT_IF_WITH_ASSERT(pRenderContextMonitor != NULL)
    {
        // Get its FBO monitor:
        const gsDisplayListMonitor* displayListMtr = pRenderContextMonitor->displayListsMonitor();
        GT_IF_WITH_ASSERT(displayListMtr != NULL)
        {
            const apGLDisplayList* pDisplayListDetails = displayListMtr->getDisplayListDetails(displayListName);

            if (pDisplayListDetails != NULL)
            {
                prDisplayListDetails = pDisplayListDetails;
                retVal = true;
            }
        }
    }
    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaGetLastFrameFunctionCallsStatisticsImpl
// Description: Implementation of gaGetLastFrameFunctionCallsStatistics.
//              See its documentation for more details.
// Author:      Yaki Tebeka
// Date:        30/1/2006
// ---------------------------------------------------------------------------
bool gaGetCurrentStatisticsImpl(int contextId, apStatistics* pStatistics)
{
    bool retVal = false;

    // Get the appropriate render context monitor:
    const suContextMonitor* pRenderContextMon = gsOpenGLMonitor::instance().contextMonitor(contextId);

    if (pRenderContextMon)
    {
        // Get the calls statistics logger:
        const suCallsStatisticsLogger& callsStatisticsLogger = pRenderContextMon->callsStatisticsLogger();

        // Get the calls statistics:
        retVal = callsStatisticsLogger.getCurrentStatistics(pStatistics);
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gaClearFunctionCallsStatisticsImpl
// Description: Implementation of gaClearFunctionCallsStatistics.
//              See its documentation for more details.
// Return Val: bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        17/7/2008
// ---------------------------------------------------------------------------
bool gaClearFunctionCallsStatisticsImpl()
{
    bool retVal = true;

    // Get the OpenGL monitor:
    gsOpenGLMonitor& theMonitor = gsOpenGLMonitor::instance();

    // Get amount of render contexts:
    int amountOfRenderContexts = theMonitor.amountOfContexts();

    // For each context - clear statistics:
    for (int contextId = 0; contextId < amountOfRenderContexts; contextId++)
    {
        bool rcClearFunctionCalls = false;
        // Get the appropriate render context monitor:
        suContextMonitor* pContextMonitor = theMonitor.contextMonitor(contextId);
        GT_IF_WITH_ASSERT(pContextMonitor != NULL)
        {
            // Get the calls statistics logger:
            suCallsStatisticsLogger& callsStatisticsLogger = pContextMonitor->callsStatisticsLogger();

            rcClearFunctionCalls = callsStatisticsLogger.clearFunctionCallsStatistics();
            GT_ASSERT(rcClearFunctionCalls);
        }

        bool rcClearPrimitives = true;

        // Clear the render primitives statistics:
        if (contextId != AP_NULL_CONTEXT_ID)
        {
            rcClearPrimitives = false;

            // Get the render context monitor:
            gsRenderContextMonitor* pRenderContextMonitor = theMonitor.renderContextMonitor(contextId);
            GT_IF_WITH_ASSERT(pRenderContextMonitor != NULL)
            {
                // Get the calls statistics logger:
                gsRenderPrimitivesStatisticsLogger& rednerPrimitivesStatisticsLogger = pRenderContextMonitor->renderPrimitivesStatisticsLogger();

                // Get the calls statistics:
                rcClearPrimitives = rednerPrimitivesStatisticsLogger.clearStatistics();
                GT_ASSERT(rcClearPrimitives);
            }
        }

        retVal = retVal && rcClearPrimitives && rcClearFunctionCalls;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaIsInOpenGLBeginEndBlockImpl
// Description: Implementation of gaIsInOpenGLBeginEndBlock.
//              See its documentation for more details.
// Author:      Avi Shapira
// Date:        5/7/2006
// ---------------------------------------------------------------------------
bool gaIsInOpenGLBeginEndBlockImpl(int contextId)
{
    bool retVal = false;

    // Get the appropriate render context monitor:
    const gsRenderContextMonitor* pRenderContextMon = gsOpenGLMonitor::instance().renderContextMonitor(contextId);

    if (pRenderContextMon)
    {
        // Are we in OpenGL Begin End Block?
        retVal = pRenderContextMon->isInOpenGLBeginEndBlock();
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaGetRenderPrimitivesStatisticsImpl
// Description: Implementation of gaGetRenderPrimitivesStatistics.
// Arguments: int contextId
//            const apRenderPrimitivesStatistics& renderPrimitivesStatistics
// Return Val: bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        14/5/2009
// ---------------------------------------------------------------------------
bool gaGetRenderPrimitivesStatisticsImpl(int contextId, apRenderPrimitivesStatistics& renderPrimitivesStatistics)
{
    bool retVal = false;

    // Get the appropriate render context monitor:
    gsRenderContextMonitor* pRenderContextMon = gsOpenGLMonitor::instance().renderContextMonitor(contextId);

    if (pRenderContextMon)
    {
        // Get the render primitives statistics logger:
        gsRenderPrimitivesStatisticsLogger& renderPrimitivesStatisticsLogger = pRenderContextMon->renderPrimitivesStatisticsLogger();

        // Get the statistics:
        renderPrimitivesStatistics = renderPrimitivesStatisticsLogger.getCurrentStatistics();

        retVal = true;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaGetAmountOfCurrentFrameFunctionCallsImpl
// Description:
//   Implementation of gaGetAmountOfCurrentFrameFunctionCalls()
//   See its documentation for more details.
// Author:      Yaki Tebeka
// Date:        24/4/2004
// ---------------------------------------------------------------------------
bool gaGetAmountOfCurrentFrameFunctionCallsImpl(int contextId, int& amountOfFunctionCalls)
{
    bool retVal = false;

    // Get the appropriate render context monitor:
    const suContextMonitor* pContextMonitor = gsOpenGLMonitor::instance().contextMonitor(contextId);

    if (pContextMonitor)
    {
        // Get its monitored functions calls logger:
        const suCallsHistoryLogger* pCallsLogger = pContextMonitor->callsHistoryLogger();
        GT_IF_WITH_ASSERT(pCallsLogger != NULL)
        {
            // Get the amount of function calls:
            amountOfFunctionCalls = pCallsLogger->amountOfFunctionCalls();
        }

        retVal = true;
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gaGetCurrentFrameFunctionCallImpl
// Description:
//   Implementation of gaGetCurrentFrameFunctionCall()
//   See its documentation for more details.
// Author:      Yaki Tebeka
// Date:        24/4/2004
// ---------------------------------------------------------------------------
bool gaGetCurrentFrameFunctionCallImpl(int contextId, int callIndex, gtAutoPtr<apFunctionCall>& aptrFunctionCall)
{
    bool retVal = false;

    // Get the appropriate render context monitor:
    const suContextMonitor* pContextMonitor = gsOpenGLMonitor::instance().contextMonitor(contextId);

    if (pContextMonitor)
    {
        // Get its monitored functions calls logger:
        const suCallsHistoryLogger* pCallsLogger = pContextMonitor->callsHistoryLogger();
        GT_IF_WITH_ASSERT(pCallsLogger != NULL)
        {
            // Get the amount of function calls:
            int amountOfFuncCalls = pCallsLogger->amountOfFunctionCalls();

            // Verify that the queried call is in the right range:
            if ((0 <= callIndex) && (callIndex < amountOfFuncCalls))
            {
                // Get the requested function call:
                retVal = pCallsLogger->getFunctionCall(callIndex, aptrFunctionCall);
            }
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gaGetCurrentFrameFunctionCallImpl
// Description:
//   Implementation of gaGetCurrentFrameFunctionCall()
//   See its documentation for more details.
// Author:      Yaki Tebeka
// Date:        24/4/2004
// ---------------------------------------------------------------------------
bool gaGetCurrentFrameFunctionCallDeprecationDetailsImpl(int contextId, int callIndex, apFunctionDeprecation& functionCallDeprecation)
{
    bool retVal = false;

    // Get the function call:
    gtAutoPtr<apFunctionCall> aptrFunctionCall;
    gaGetCurrentFrameFunctionCallImpl(contextId, callIndex, aptrFunctionCall);

    // Get the function deprecation details:
    gsDeprecationAnalyzer& deprecationAnalyzer = gsDeprecationAnalyzer::instance();
    retVal = deprecationAnalyzer.getFunctionCallDeprecationDetails(aptrFunctionCall.pointedObject(), aptrFunctionCall->arguments(), functionCallDeprecation);

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gaGetLastFunctionCallImpl
// Description:
//   Implementation of gaGetLastFunctionCall
//   See its documentation for more details.
// Author:      Yaki Tebeka
// Date:        21/6/2005
// ---------------------------------------------------------------------------
bool gaGetLastFunctionCallImpl(int contextId, gtAutoPtr<apFunctionCall>& aptrFunctionCall)
{
    bool retVal = false;

    // Get the appropriate render context monitor:
    const suContextMonitor* pContextMonitor = gsOpenGLMonitor::instance().contextMonitor(contextId);

    if (pContextMonitor)
    {
        // Get its monitored functions calls logger:
        const suCallsHistoryLogger* pCallsLogger = pContextMonitor->callsHistoryLogger();
        GT_IF_WITH_ASSERT(pCallsLogger != NULL)
        {
            // Get the current execution mode:
            apExecutionMode currentExecMode = suDebuggedProcessExecutionMode();

            // If logging is enabled:
            if (pCallsLogger->isLoggingEnabled() && (currentExecMode != AP_PROFILING_MODE))
            {
                // Get the amount of function calls:
                int amountOfFuncCalls = pCallsLogger->amountOfFunctionCalls();

                // Get the last function call:
                retVal = pCallsLogger->getFunctionCall(amountOfFuncCalls - 1, aptrFunctionCall);
            }
            else
            {
                // Logging is disabled, we can only return the last called function id:
                apMonitoredFunctionId lastCalledFuncId = pCallsLogger->lastCalledFunctionId();
                apFunctionCall* pLastFunctionCall = new apFunctionCall(lastCalledFuncId);


                aptrFunctionCall = pLastFunctionCall;
                retVal = true;
            }
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gaFindCurrentFrameFunctionCallImpl
// Description:
//   Implementation of gaFindCurrentFrameFunctionCall()
//   See its documentation for more details.
// Author:      Yaki Tebeka
// Date:        15/12/2004
// ---------------------------------------------------------------------------
bool gaFindCurrentFrameFunctionCallImpl(int contextId, apSearchDirection searchDirection,
                                        int searchStartIndex, const gtString& searchedString,
                                        bool isCaseSensitiveSearch, int& foundIndex)
{
    bool retVal = false;

    // Get the appropriate render context monitor:
    const suContextMonitor* pContextMonitor = gsOpenGLMonitor::instance().contextMonitor(contextId);

    if (pContextMonitor)
    {
        // Get its monitored functions calls logger:
        const suCallsHistoryLogger* pCallsLogger = pContextMonitor->callsHistoryLogger();
        GT_IF_WITH_ASSERT(pCallsLogger != NULL)
        {
            // Get the amount of function calls:
            int amountOfFuncCalls = pCallsLogger->amountOfFunctionCalls();

            // Verify that the search start index is in the right range:
            if ((0 <= searchStartIndex) && (searchStartIndex < amountOfFuncCalls))
            {
                // Search the relevant context calls log:
                retVal = pCallsLogger->findFunctionCall(searchDirection, searchStartIndex, searchedString, isCaseSensitiveSearch, foundIndex);
            }
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gaFindStringMarkerImpl
// Description:
//   Implementation of gaFindStringMarkerImpl()
//   See its documentation for more details.
// Author:      Yaki Tebeka
// Date:        01/02/2005
// ---------------------------------------------------------------------------
bool gaFindStringMarkerImpl(int contextId, apSearchDirection searchDirection,
                            int searchStartIndex, int& foundIndex)
{
    bool retVal = false;

    // Get the appropriate render context monitor:
    const suContextMonitor* pContextMonitor = gsOpenGLMonitor::instance().contextMonitor(contextId);

    if (pContextMonitor)
    {
        // Get its monitored functions calls logger:
        const suCallsHistoryLogger* pCallsLogger = pContextMonitor->callsHistoryLogger();
        GT_IF_WITH_ASSERT(pCallsLogger != NULL)
        {
            // Get the amount of function calls:
            int amountOfFuncCalls = pCallsLogger->amountOfFunctionCalls();

            // Verify that the search start index is in the right range:
            if ((0 <= searchStartIndex) && (searchStartIndex < amountOfFuncCalls))
            {
                // Search the relevant context calls log:
                retVal = pCallsLogger->findStringMarker(searchDirection, searchStartIndex, foundIndex);
            }
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gaGetCurrentOpenGLErrorImpl
// Description:
//   Implementation of gaGetCurrentOpenGLError().
//   See its documentation for more details.
// Return Val:  bool - Success / failure.
// Author:      Yaki Tebeka
// Date:        24/8/2004
// ---------------------------------------------------------------------------
bool gaGetCurrentOpenGLErrorImpl(GLenum& openGLError)
{
    bool retVal = true;

    gsOpenGLMonitor::instance().getCurrentOpenGLError(openGLError);

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaGetContextLogFilePathImpl
// Description:
//   Implementation of gaGetRenderContextLogFilePath().
//   See its documentation for more details.
// Author:      Yaki Tebeka
// Date:        23/3/2005
// ---------------------------------------------------------------------------
bool gaGetContextLogFilePathImpl(int contextId, bool& logFileExists, osFilePath& filePath)
{
    bool retVal = true;

    // Get the HTML file path from the OpenGL monitor:
    retVal = gsOpenGLMonitor::instance().getHTMLLogFilePath(contextId, logFileExists, filePath);

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaForceOpenGLFlushImpl
// Description: Implementation of gaForceOpenGLFlush
//              See its documentation for more details.
// Author:      Yaki Tebeka
// Date:        11/11/2004
// ---------------------------------------------------------------------------
bool gaForceOpenGLFlushImpl(bool isOpenGLFlushForced)
{
    gsOpenGLMonitor::instance().forceOpenGLFlush(isOpenGLFlushForced);

    return true;
}

// ---------------------------------------------------------------------------
// Name:        gaSetInteractiveBreakModeImpl
// Description: Implementation of gaSetInteractiveBreakMode.
//              See its documentation for more details.
// Author:      Yaki Tebeka
// Date:        23/7/2006
// ---------------------------------------------------------------------------
bool gaSetInteractiveBreakModeImpl(bool isInteractiveBreakOn)
{
    gsOpenGLMonitor::instance().setInteractiveBreakMode(isInteractiveBreakOn);
    return true;
}

// ---------------------------------------------------------------------------
// Name:        gaSetOpenGLForceStubImpl
// Description: Implementation of gaSetOpenGLForceStub.
// Arguments:   apOpenGLForcedModeType stubType
//            bool isStubForced
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        6/5/2010
// ---------------------------------------------------------------------------
bool gaSetOpenGLForceStubImpl(apOpenGLForcedModeType stubType, bool isStubForced)
{
    gsOpenGLMonitor::instance().forceStubMode(stubType, isStubForced);

    return true;
}

// ---------------------------------------------------------------------------
// Name:        gaForceOpenGLPolygonRasterModeImpl
// Description: Implementation of gaForceOpenGLPolygonRasterModeImpl
//              See its documentation for more details.
// Author:      Yaki Tebeka
// Date:        11/11/2004
// ---------------------------------------------------------------------------
bool gaForceOpenGLPolygonRasterModeImpl(apRasterMode rasterMode)
{
    gsOpenGLMonitor::instance().forceOpenGLPolygonRasterMode(rasterMode);

    return true;
}


// ---------------------------------------------------------------------------
// Name:        gaCancelOpenGLPolygonRasterModeForcingImpl
// Description: Implementation of gaCancelOpenGLPolygonRasterModeForcingImpl
//              See its documentation for more details.
// Author:      Yaki Tebeka
// Date:        11/11/2004
// ---------------------------------------------------------------------------
bool gaCancelOpenGLPolygonRasterModeForcingImpl()
{
    gsOpenGLMonitor::instance().cancelOpenGLPolygonRasterModeForcing();

    return true;
}


// ---------------------------------------------------------------------------
// Name:        gaSetOpenGLNullDriverImpl
// Description: Implementation of gaSetOpenGLNullDriverImpl
//              See its documentation for more details.
// Author:      Avi Shapira
// Date:        28/2/2005
// ---------------------------------------------------------------------------
bool gaSetOpenGLNullDriverImpl(bool isNULLOpenGLImplOn)
{
    // Set the "NULL OpenGL Implementation" mode:
    gs_stat_isInNullOpenGLImplementationMode = isNULLOpenGLImplOn;

    return true;
}

// ---------------------------------------------------------------------------
// Name:        gaGetSpyPerformanceCountersValuesImpl
// Description: Implementation of gaGetAmountOfSpyPerformanceCounters.
//              See its documentation for more details.
// Author:      Yaki Tebeka
// Date:        24/7/2005
// ---------------------------------------------------------------------------
bool gaGetSpyPerformanceCountersValuesImpl(const double*& pValuesArray, int& amountOfValues)
{
    bool retVal = false;

    // Output the counters amount:
    amountOfValues = gsOpenGLMonitor::instance().spyPerformanceCountersManager().countersAmount();

    // Update the performance counters values:
    retVal = gsOpenGLMonitor::instance().spyPerformanceCountersManager().updateCounterValues();

    if (retVal)
    {
        // Get the updated values:
        pValuesArray = gsOpenGLMonitor::instance().spyPerformanceCountersManager().getCounterValues();
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaGetRemoteOSPerformanceCountersValuesImpl
// Description: Implementation of gaGetRemoteOSPerformanceCountersValues.
//              See its documentation for more details.
// Author:      Uri Shomroni
// Date:        23/11/2009
// ---------------------------------------------------------------------------
bool gaGetRemoteOSPerformanceCountersValuesImpl(const double*& pValuesArray, int& amountOfValues)
{
    bool retVal = false;

    // gsOSPerformanceCountersManager is currently only implemented on the iPhone device:
#ifdef _GR_IPHONE_DEVICE_BUILD

    // Update the performance counters values:
    retVal = gs_stat_osPerformanceManager.updateCounterValues();

    if (retVal)
    {
        // Output the counters amount:
        amountOfValues = gs_stat_osPerformanceManager.amountOfCounters();

        // Get the updated values:
        pValuesArray = gs_stat_osPerformanceManager.getCounterValues();
    }

#else
    // Resolve the compiler warning for the Linux variant
    (void)(pValuesArray);
    (void)(amountOfValues);
#endif

    return retVal;
}


// The below functions are supported on the iPhone device only:
#ifdef _GR_IPHONE_DEVICE_BUILD

// ---------------------------------------------------------------------------
// Name:        gaAddSupportediPhonePerformanceCounterImpl
// Description: Implementation of gaAddSupportediPhonePerformanceCounter.
// Author:      Yaki Tebeka
// Date:        1/3/2010
// ---------------------------------------------------------------------------
bool gaAddSupportediPhonePerformanceCounterImpl(int counterIndex, const gtString& counterName)
{
    bool retVal = false;

    // Add support for the input performance counter:
    retVal = gs_stat_iPhoneGPUPerformanceCountersReader.addSupportediPhonePerformanceCounter(counterIndex, counterName);

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gaInitializeiPhonePerformanceCountersReaderImpl
// Description: Implementation of gaInitializeiPhonePerformanceCountersReader.
// Author:      Yaki Tebeka
// Date:        1/3/2010
// ---------------------------------------------------------------------------
bool gaInitializeiPhonePerformanceCountersReaderImpl()
{
    bool retVal = false;

    // Add support for the input performance counter:
    retVal = gs_stat_iPhoneGPUPerformanceCountersReader.initialize();

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gaGetiPhonePerformanceCountersValuesImpl
// Description: Implementation of gaGetiPhonePerformanceCountersValuesImpl.
// Author:      Sigal Algranaty
// Date:        13/1/2010
// ---------------------------------------------------------------------------
bool gaGetiPhonePerformanceCountersValuesImpl(const double*& pValuesArray, int& amountOfValues)
{
    bool retVal = false;

    // Update the iPhone performance counters values:
    retVal = gs_stat_iPhoneGPUPerformanceCountersReader.updateCountersValues();

    if (retVal)
    {
        // Get the updated values:
        pValuesArray = gs_stat_iPhoneGPUPerformanceCountersReader.getCounterValues();

        // Get the updated total size
        amountOfValues = gs_stat_iPhoneGPUPerformanceCountersReader.amountOfCounters();
    }

    return retVal;
}

#endif // _GR_IPHONE_DEVICE_BUILD


// ---------------------------------------------------------------------------
// Name:        gaGetATIPerformanceCountersValuesImpl
// Description: Implementation of gaGetATIPerformanceCountersValues.
//              See its documentation for more details. Supported on windows only.
// Author:      Sigal Algranaty
// Date:        23/03/2008
// ---------------------------------------------------------------------------
bool gaGetATIPerformanceCountersValuesImpl(const double*& pValuesArray, int& amountOfValues)
{
    bool retVal = true;

    pValuesArray = NULL;
    amountOfValues = 0;

#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
#ifdef OA_DEBUGGER_USE_AMD_GPA

    {
        // Get the updated values:
        pValuesArray = gsOpenGLMonitor::instance().ATIPerformanceCountersManager().getCounterValues();

        // Get the updated total size:
        amountOfValues = gsOpenGLMonitor::instance().ATIPerformanceCountersManager().getCountersAmount();
    }

#endif
#endif

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaActivateATIPerformanceCountersImpl
// Description: Implementation of gaActivateATIPerformanceCounters.
//              See its documentation for more details. Supported on windows only.
// Author:      Sigal Algranaty
// Date:        23/03/2008
// ---------------------------------------------------------------------------
bool gaActivateATIPerformanceCountersImpl(const gtVector<apCounterActivationInfo>& countersActivationInfosVec)
{
    bool retVal = false;

    // Potentially unused parameters:
    (void)(countersActivationInfosVec);

#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
#ifdef OA_DEBUGGER_USE_AMD_GPA

    {
        // Update the ATI performance counters values:
        retVal = gsOpenGLMonitor::instance().ATIPerformanceCountersManager().activateCounters(countersActivationInfosVec);
    }

#endif
#endif

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaEnableGLDebugOutputLoggingImpl
// Description: Implementation of gaEnableGLDebugOutputIntegration.
//              See its documentation for more details. Supported on windows only.
// Arguments: bool glDebugOutputIntegrationEnabled
// Return Val: bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        9/6/2010
// ---------------------------------------------------------------------------
bool gaEnableGLDebugOutputLoggingImpl(bool glDebugOutputIntegrationEnabled)
{
    bool retVal = true;

#if (AMDT_BUILD_TARGET == AMDT_WINDOWS_OS) || ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_GENERIC_LINUX_VARIANT))

    // Set the break on flag:
    gsOpenGLMonitor::instance().setGLDebugOutputLoggingEnabled(glDebugOutputIntegrationEnabled);

    // Change the current mode:
    retVal = gaSetOpenGLForceStubImpl(AP_OPENGL_DEBUG_OUTPUT_PARAMETER_CHANGED, true);
#endif

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaSetGLDebugOutputSeverityEnabledImpl
// Description: Implementation of gaSetGLDebugOutputSeverityEnabled.
//              See its documentation for more details. Supported on windows only.
// Arguments:   unsigned long debugOutputCategoryMask
// Return Val:  bool  - Success / failure.
// Author:      Yaki Tebeka
// Date:        10/6/2010
// ---------------------------------------------------------------------------
bool gaSetGLDebugOutputSeverityEnabledImpl(apGLDebugOutputSeverity severity, bool enabled)
{
    bool retVal = true;

#if (AMDT_BUILD_TARGET == AMDT_WINDOWS_OS) || ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_GENERIC_LINUX_VARIANT))

    // Set the severity:
    gsOpenGLMonitor::instance().setGLDebugOutputSeverityEnabled(severity, enabled);

    // Change the current mode:
    retVal = gaSetOpenGLForceStubImpl(AP_OPENGL_DEBUG_OUTPUT_PARAMETER_CHANGED, true);
#endif

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaSetGLDebugOutputKindMaskImpl
// Description: Implementation of gaSetGLDebugOutputKindMask.
//              See its documentation for more details. Supported on windows only.
// Arguments:   unsigned long debugOutputCategoryMask
// Return Val:  bool  - Success / failure.
// Author:      Yaki Tebeka
// Date:        10/6/2010
// ---------------------------------------------------------------------------
bool gaSetGLDebugOutputKindMaskImpl(const gtUInt64& debugOutputCategoryMask)
{
    bool retVal = true;

#if (AMDT_BUILD_TARGET == AMDT_WINDOWS_OS) || ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_GENERIC_LINUX_VARIANT))

    // Set the category mask:
    gsOpenGLMonitor::instance().setGLDebugOutputKindMask(debugOutputCategoryMask);

    // Change the current mode:
    retVal = gaSetOpenGLForceStubImpl(AP_OPENGL_DEBUG_OUTPUT_PARAMETER_CHANGED, true);
#endif

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gaDoesDebugForcedContextExistImpl
// Description: Implementation of gaDoesDebugForcedContextExist.
//              See its documentation for more details. Supported on windows only.
// Arguments:   bool isDebugOn
//              bool& isDebugContextExist
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        7/7/2010
// ---------------------------------------------------------------------------
bool gaDoesDebugForcedContextExistImpl(bool isDebugOn, bool& isDebugContextExist)
{
    bool retVal = true;

    // Iterate the render context monitors and search for forced debug flag context:
    isDebugContextExist = gsOpenGLMonitor::instance().doesDebugForcedContextExist(isDebugOn);

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gaGetAmountOfPipelineObjectsImpl
// Description: Extracts the amount of program pipelines.
// Arguments:   int contextId -  the id of the relevant context
//              int& amountOfProgramPipelines - output buffer for the result
// Return Val:  bool - Success / failure.
// Author:      Amit Ben-Moshe
// Date:        22/6/2014
// ---------------------------------------------------------------------------
bool gaGetAmountOfPipelineObjectsImpl(int contextId, int& amountOfProgramPipelines)
{
    bool retVal = false;

    // Get the appropriate render context monitor.
    gsRenderContextMonitor* pRenderContextMonitor = gsOpenGLMonitor::instance().renderContextMonitor(contextId);

    if (pRenderContextMonitor != NULL)
    {
        // Get the pipeline monitor.
        gsPipelineMonitor& pplnMonitor = pRenderContextMonitor->pipelinesMonitor();
        amountOfProgramPipelines = static_cast<int>(pplnMonitor.GetAmountOfPipelineObjects());
        retVal = true;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaGetAmountOfPipelineObjectsImpl
// Description: Extracts the data of the program pipeline named pipelineName.
// Arguments:   int contextId -  the id of the relevant context
//              GLuint pipelineName - the name of the relevant pipeline.
//              apGLPipeline& pipelineDataBuffer - output parameter to hold the
//                                                 relevant pipeline's data.
// Return Val:  bool - Success / failure.
// Author:      Amit Ben-Moshe
// Date:        22/6/2014
// ---------------------------------------------------------------------------
bool gaGetPipelineObjectDetailsImpl(int contextId, GLuint pipelineName, apGLPipeline& pipelineDataBuffer)
{
    bool retVal = false;

    // Get the appropriate render context monitor.
    gsRenderContextMonitor* pRenderContextMonitor = gsOpenGLMonitor::instance().renderContextMonitor(contextId);

    if (pRenderContextMonitor != NULL)
    {
        // Get the pipelines monitor, and extract the relevant pipeline's details.
        gsPipelineMonitor& pplnMonitor = pRenderContextMonitor->pipelinesMonitor();
        retVal = pplnMonitor.GetPipelineData(pipelineName, pipelineDataBuffer);
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gaGetPipelineObjectNameImpl
// Description: Extracts the name of the program pipeline which is stored at
//              index pipelineIndex.
// Arguments:   int contextId -  the id of the relevant context.
//              int pipelineIndex -   the index of the relevant pipeline.
//              GLuint pipelineName - output buffer to hold the pipeline name;
//                                    note that the value is not valid if the
//                                    function returned false.
// Return Val:  bool - Success / failure.
// Author:      Amit Ben-Moshe
// Date:        22/6/2014
// ---------------------------------------------------------------------------
bool gaGetPipelineObjectNameImpl(int contextId, int pipelineIndex, GLuint& pipelineName)
{
    bool retVal = false;
    pipelineName = 0;

    // Get the appropriate render context monitor:
    gsRenderContextMonitor* pRenderContextMonitor = gsOpenGLMonitor::instance().renderContextMonitor(contextId);

    if (pRenderContextMonitor != NULL)
    {
        // Get the pipelines monitor:
        gsPipelineMonitor& pplnMonitor = pRenderContextMonitor->pipelinesMonitor();
        retVal = pplnMonitor.GetPipelineNameByIndex(pipelineIndex, pipelineName);
        GT_ASSERT(retVal);
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaGetAmountOfSamplerObjectsImpl
// Description: Extracts the amount of sampler objects in the given context.
// Arguments:   int contextId -  the id of the relevant context
//              int& amountOfSamplers - output buffer for holding the result
// Return Val:  bool - Success / failure.
// Author:      Amit Ben-Moshe
// Date:        22/6/2014
// ---------------------------------------------------------------------------
bool gaGetAmountOfSamplerObjectsImpl(int contextId, int& amountOfSamplers)
{
    bool retVal = false;

    // Get the appropriate render context monitor.
    gsRenderContextMonitor* pRenderContextMonitor = gsOpenGLMonitor::instance().renderContextMonitor(contextId);

    if (pRenderContextMonitor != NULL)
    {
        // Get the samplers monitor.
        gsSamplersMonitor& samplersMonitor = pRenderContextMonitor->samplersMonitor();
        amountOfSamplers = static_cast<int>(samplersMonitor.getAmountOfSamplerObjects());
        retVal = true;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaGetSamplerObjectDetailsImpl
// Description: Extracts the data of the a sampler object.
// Arguments:   int contextId -  the id of the relevant context
//              GLuint samplerName - the name of the relevant sampler
//              apGLSampler& samplerDataBuffer - output buffer for holding the result
// Return Val:  bool - Success / failure.
// Author:      Amit Ben-Moshe
// Date:        22/6/2014
// ---------------------------------------------------------------------------
bool gaGetSamplerObjectDetailsImpl(int contextId, GLuint samplerName, apGLSampler& samplerDataBuffer)
{
    bool retVal = false;

    // Get the appropriate render context monitor.
    gsRenderContextMonitor* pRenderContextMonitor = gsOpenGLMonitor::instance().renderContextMonitor(contextId);

    if (pRenderContextMonitor != NULL)
    {
        // Get the samplers monitor, and extract the relevant sampler's details.
        gsSamplersMonitor& samplerMonitor = pRenderContextMonitor->samplersMonitor();
        retVal = samplerMonitor.getSamplerData(samplerName, samplerDataBuffer);
        GT_ASSERT(retVal);
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaGetSamplerObjectNameImpl
// Description: Extracts the name of the sampler object which is stored at
//              a given index.
// Arguments:   int contextId -       the id of the relevant context.
//              int samplerIndex -    the index of the relevant sampler object.
//              GLuint samplerName -  output buffer to hold the sampler name;
//                                    note that the value is not valid if the
//                                    function returned false.
// Return Val:  bool - Success / failure.
// Author:      Amit Ben-Moshe
// Date:        22/6/2014
// ---------------------------------------------------------------------------
bool gaGetSamplerObjectNameImpl(int contextId, int samplerIndex, GLuint& samplerName)
{
    bool retVal = false;
    samplerName = 0;

    // Get the appropriate render context monitor:
    gsRenderContextMonitor* pRenderContextMonitor = gsOpenGLMonitor::instance().renderContextMonitor(contextId);

    if (pRenderContextMonitor != NULL)
    {
        // Get the samplers monitor:
        gsSamplersMonitor& samplersMonitor = pRenderContextMonitor->samplersMonitor();
        retVal = samplersMonitor.getSamplerNameByIndex(samplerIndex, samplerName);
        GT_ASSERT(retVal);
    }

    return retVal;
}
