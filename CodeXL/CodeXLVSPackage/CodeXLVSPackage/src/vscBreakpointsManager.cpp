//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file vscBreakpointsManager.cpp
///
//==================================================================================

//------------------------------ vscBreakpointsManager.cpp ------------------------------

#include "stdafx.h"

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTBaseTools/Include/gtString.h>
#include <AMDTAPIClasses/Include/apBreakPoint.h>
#include <AMDTAPIClasses/Include/apCLObjectID.h>
#include <AMDTAPIClasses/Include/apCLProgram.h>
#include <AMDTAPIClasses/Include/apHostSourceBreakpoint.h>
#include <AMDTAPIClasses/Include/apKernelFunctionNameBreakpoint.h>
#include <AMDTAPIClasses/Include/apKernelSourceCodeBreakpoint.h>
#include <AMDTAPIClasses/Include/apMonitoredFunctionBreakPoint.h>
#include <AMDTAPIClasses/Include/apMonitoredFunctionsManager.h>
#include <AMDTAPIClasses/Include/Events/apKernelSourceBreakpointsUpdatedEvent.h>
#include <AMDTOpenCLServer/Include/csPublicStringConstants.h>
#include <AMDTGpuDebuggingComponents/Include/gdAidFunctions.h>
#include <AMDTGpuDebuggingComponents/Include/gdStringConstants.h>

// Local:
#include <src/vscBreakpointsManager.h>
#include <src/vscDebugEngine.h>
#include <src/vscDebugEvents.h>
#include <src/vspGRApiFunctions.h>
#include <CodeXLVSPackage/Include/vspStringConstants.h>

// Static members initializations:
vscBreakpointsManager* vscBreakpointsManager::_pMySingleInstance = NULL;
IVscBreakpointsManagerOwner* vscBreakpointsManager::m_pOwner = NULL;

// Performance.
#include <algorithm>

// ---------------------------------------------------------------------------
// Name:        vscBreakpointsManager::vscBreakpointsManager
// Description: Constructor
// Author:      Uri Shomroni
// Date:        28/9/2010
// ---------------------------------------------------------------------------
vscBreakpointsManager::vscBreakpointsManager()
    : _piDebugger(NULL), _pDebugEngine(NULL), _piDebugEventCallback(NULL)
{
}

// ---------------------------------------------------------------------------
// Name:        vscBreakpointsManager::~vscBreakpointsManager
// Description: Destructor
// Author:      Uri Shomroni
// Date:        28/9/2010
// ---------------------------------------------------------------------------
vscBreakpointsManager::~vscBreakpointsManager()
{
    releaseInterfaces();
}

// ---------------------------------------------------------------------------
// Name:        vscBreakpointsManager::setDebugEngine
// Description: Sets the debug engine.
// Author:      Uri Shomroni
// Date:        4/10/2010
// ---------------------------------------------------------------------------
void vscBreakpointsManager::setDebugEngine(vspCDebugEngine* pDebugEngine)
{
    // This should not be called more than once:
    GT_ASSERT((_pDebugEngine == NULL) || (pDebugEngine == NULL));
    _pDebugEngine = pDebugEngine;
}

// ---------------------------------------------------------------------------
// Name:        vscBreakpointsManager::setDebugEventCallback
// Description: Sets the debug event callback interface.
// Author:      Uri Shomroni
// Date:        4/10/2010
// ---------------------------------------------------------------------------
void vscBreakpointsManager::setDebugEventCallback(IDebugEventCallback2* piDebugEventCallback)
{
    // Release any previous interface we were holding:
    if (_piDebugEventCallback != NULL)
    {
        _piDebugEventCallback->Release();
        _piDebugEventCallback = NULL;
    }

    // Set the member:
    GT_IF_WITH_ASSERT(piDebugEventCallback != NULL)
    {
        _piDebugEventCallback = piDebugEventCallback;
        _piDebugEventCallback->AddRef();
    }
}

// ---------------------------------------------------------------------------
// Name:        vscBreakpointsManager::setDebuggerInterface
// Description: Sets the debugger interface and retains it
// Author:      Uri Shomroni
// Date:        28/9/2010
// ---------------------------------------------------------------------------
void vscBreakpointsManager::setDebuggerInterface(IVsDebugger* piDebugger)
{
    // Release any previous interface we were holding:
    if (_piDebugger != NULL)
    {
        _piDebugger->Release();
        _piDebugger = NULL;
    }

    // Set the member:
    GT_IF_WITH_ASSERT(piDebugger != NULL)
    {
        _piDebugger = piDebugger;
        _piDebugger->AddRef();
    }
}

// ---------------------------------------------------------------------------
// Name:        ClearBreakpointsMap
// Description: Auxiliary function that clears map elements to avoid dangling
//              pointers.
// Author:      Amit Ben-Moshe
// Date:        15/07/2013
// ---------------------------------------------------------------------------
template<typename T>
void ClearBreakpointsMap(gtMap<T, vspCDebugBreakpoint*> __map)
{
    // Clear map elements to avoid dangling pointers.
    auto iter = __map.begin();

    while (iter != __map.end())
    {
        (iter++)->second = NULL;
    }
}

// ---------------------------------------------------------------------------
// Name:        vscBreakpointsManager::releaseInterfaces
// Description: Releases the interfaces we hold pointers to
// Author:      Uri Shomroni
// Date:        24/10/2011
// ---------------------------------------------------------------------------
void vscBreakpointsManager::releaseInterfaces()
{
    if (_piDebugger != NULL)
    {
        _piDebugger->Release();
        _piDebugger = NULL;
    }

    // Release our breakpoints:
    int numberOfBreakpoints = (int)_breakpoints.size();

    for (int i = 0; i < numberOfBreakpoints; i++)
    {
        vspCDebugBreakpoint* pCurrentBreakpoint = _breakpoints[i];

        if (pCurrentBreakpoint != NULL)
        {
            pCurrentBreakpoint->Release();
            _breakpoints[i] = NULL;
        }
    }

    // Clear map elements to avoid dangling pointers.
    ClearBreakpointsMap(_breakpointByFuncId);
    ClearBreakpointsMap(_breakpointByGenericType);
    ClearBreakpointsMap(_breakpointByKernelFuncName);


    // Release the callback interface:
    if (_piDebugEventCallback != NULL)
    {
        _piDebugEventCallback->Release();
        _piDebugEventCallback = NULL;
    }

    // Note that to avoid circular dependencies, we do not AddRef to the debug
    // engine (see setDebugEngine() ). Instead we just override the pointer:
    _pDebugEngine = NULL;
}

// ---------------------------------------------------------------------------
// Name:        vscBreakpointsManager::instance
// Description: Returns the single instance of this class. Creates it on
//              the first call to this function.
// Author:      Uri Shomroni
// Date:        28/9/2010
// ---------------------------------------------------------------------------
vscBreakpointsManager& vscBreakpointsManager::instance()
{
    if (_pMySingleInstance == NULL)
    {
        _pMySingleInstance = new vscBreakpointsManager;
    }

    return *_pMySingleInstance;
}

// ---------------------------------------------------------------------------
// Name:        vscBreakpointsManager::setBreakpoint
// Description: Interprets breakpoint into a Visual Studio breakpoint request
//              and sends it to the debugged interface.
// Return Val:  bool - Success / failure.
// Author:      Uri Shomroni
// Date:        28/9/2010
// ---------------------------------------------------------------------------
bool vscBreakpointsManager::setBreakpoint(const apBreakPoint& breakpoint)
{
    bool retVal = false;

    // Get the breakpoint type:
    osTransferableObjectType breakpointType = breakpoint.type();

    switch (breakpointType)
    {
        case OS_TOBJ_ID_MONITORED_FUNC_BREAKPOINT:
        {
            // Request the breakpoint from Visual Studio:
            retVal = requestMonitoredFunctionBreakpoint((const apMonitoredFunctionBreakPoint&)breakpoint);
        }
        break;

        case OS_TOBJ_ID_KERNEL_SOURCE_CODE_BREAKPOINT:
        {
            // Request the breakpoint from Visual Studio:
            retVal = requestKernelSourceCodeBreakpoint((const apKernelSourceCodeBreakpoint&)breakpoint);
        }
        break;

        case OS_TOBJ_ID_KERNEL_FUNC_NAME_BREAKPOINT:
        {
            // Request the breakpoint from Visual Studio:
            retVal = requestKernelFunctionNameBreakpoint((const apKernelFunctionNameBreakpoint&)breakpoint);
        }
        break;

        case OS_TOBJ_ID_GENERIC_BREAKPOINT:
        {
            // Request the breakpoint from Visual Studio:
            retVal = requestGenericBreakpoint((const apGenericBreakpoint&)breakpoint);
        }
        break;

        case OS_TOBJ_ID_HOST_SOURCE_CODE_BREAKPOINT:
        {
            // Request the breakpoint from Visual Studio:
            retVal = requestHostSourceCodeBreakpoint((const apHostSourceCodeBreakpoint&)breakpoint);
        }
        break;

        /*  case OS_TOBJ_ID_xxx_BREAKPOINT:
                {
                    // These breakpoint types are not currently handled, ignore them
                }
                break;                                                              */

        default:
        {
            // Unexpected type!
            GT_ASSERT(false);
        }
        break;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        vscBreakpointsManager::removeBreakpoint
// Description: Interprets breakpoint into a Visual Studio breakpoint removal request
//              and sends it to the debugged interface.
// Return Val:  bool - Success / failure.
// Author:      Uri Shomroni
// Date:        28/9/2010
// ---------------------------------------------------------------------------
bool vscBreakpointsManager::removeBreakpoint(const apBreakPoint& breakpoint)
{
    bool retVal = false;

    // Get the breakpoint type:
    osTransferableObjectType breakpointType = breakpoint.type();

    switch (breakpointType)
    {
        case OS_TOBJ_ID_MONITORED_FUNC_BREAKPOINT:
        {
            // Request the breakpoint from Visual Studio:
            retVal = removeMonitoredFunctionBreakpoint((const apMonitoredFunctionBreakPoint&)breakpoint);
        }
        break;

        case OS_TOBJ_ID_KERNEL_SOURCE_CODE_BREAKPOINT:
        {
            // Remove the breakpoint from Visual Studio:
            retVal = removeKernelSourceCodeBreakpoint((const apKernelSourceCodeBreakpoint&)breakpoint);
        }
        break;

        case OS_TOBJ_ID_KERNEL_FUNC_NAME_BREAKPOINT:
        {
            // Remove the breakpoint from Visual Studio:
            retVal = removeKernelFunctionNameBreakpoint((const apKernelFunctionNameBreakpoint&)breakpoint);
        }
        break;

        case OS_TOBJ_ID_GENERIC_BREAKPOINT:
        {
            // Remove the breakpoint from Visual Studio:
            retVal = removeGenericBreakpoint((const apGenericBreakpoint&)breakpoint);
        }
        break;

        case OS_TOBJ_ID_HOST_SOURCE_CODE_BREAKPOINT:
        {
            // Remove the breakpoint from Visual Studio:
            retVal = removeHostSourceCodeBreakpoint((const apHostSourceCodeBreakpoint&)breakpoint);
        }
        break;

        /*  case OS_TOBJ_ID_xxx_BREAKPOINT:
                {
                    // These breakpoint types are not currently handled, ignore them
                }
                break;                                                              */

        default:
        {
            // Unexpected type!
            GT_ASSERT(false);
        }
        break;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        vscBreakpointsManager::respondToBreakpointRequest
// Description: Creates a breakpoint for piRequest, currently in "pending" mode,
//              and returns it to the caller. It's the caller's responsibility to
//              retain the breakpoint if needed.
// Author:      Uri Shomroni
// Date:        29/9/2010
// ---------------------------------------------------------------------------
vspCDebugBreakpoint* vscBreakpointsManager::respondToBreakpointRequest(IDebugBreakpointRequest2* piRequest)
{
    vspCDebugBreakpoint* retVal = NULL;

    GT_IF_WITH_ASSERT(piRequest != NULL)
    {
        // Create the breakpoint:
        retVal = new vspCDebugBreakpoint(_pDebugEngine, piRequest, 0);

        // Add the breakpoint to our vector:
        _breakpoints.push_back(retVal);
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        vscBreakpointsManager::bindBreakpoint
// Description: Performs the actions needed to IDebugPendingBreakpoint2::Bind()
//              and sends any needed events.
// Arguments:   breakpoint - the breakpoint to be bound.
//              performBind - true = bind the breakpoint and create the API breakpoint
//                          - false = only return the retVal, without actually binding.
// Author:      Uri Shomroni
// Date:        29/9/2010
// ---------------------------------------------------------------------------
HRESULT vscBreakpointsManager::bindBreakpoint(vspCDebugBreakpoint& breakpoint, bool performBind)
{
    HRESULT retVal = S_OK;

    GT_IF_WITH_ASSERT(m_pOwner != NULL)
    {
        IDebugBreakpointRequest2* pBreakpointRequest = breakpoint.breakpointRequest();
        GT_IF_WITH_ASSERT(pBreakpointRequest != NULL)
        {
            bool canBind = false;
            BP_ERROR_TYPE errorType = BPET_GENERAL_WARNING;
            gtString errorMessage = VSP_STR_BreakpointErrorCannotBeBound;

            // Get the breakpoint type:
            BP_LOCATION_TYPE breakpointLocationType = BPLT_NONE;
            HRESULT hr = pBreakpointRequest->GetLocationType(&breakpointLocationType);
            GT_IF_WITH_ASSERT(SUCCEEDED(hr))
            {

                // Will get the monitored function details, if any are relevant:
                apMonitoredFunctionId breakpointFuncId = apMonitoredFunctionsAmount;
                bool isCLCodeBreakpoint = false;
                oaCLProgramHandle openCLProgramHandle = OA_CL_NULL_HANDLE;
                osFilePath sourceFilePath;
                int lineNumber = -1;
                gtString breakpointKernelFunctionName;
                apGenericBreakpointType bpGenericType = AP_BREAK_TYPE_UNKNOWN;

                // Use the request type to determine the fields we would need:
                BPREQI_FIELDS requiredFields = BPREQI_CONDITION;
                BP_LOCATION_TYPE breakpointLocationKind = breakpointLocationType & BPLT_LOCATION_TYPE_MASK;
                BP_LOCATION_TYPE breakpointType = breakpointLocationType & BPLT_TYPE_MASK;
                GT_UNREFERENCED_PARAMETER(breakpointType);

                switch (breakpointLocationKind)
                {
                    case BPLT_FILE_LINE:
                    {
                        // This is a source code (file / line) breakpoint, get the location and language:
                        requiredFields |= BPREQI_BPLOCATION | BPREQI_LANGUAGE;
                    }
                    break;

                    case BPLT_FUNC_OFFSET:
                    {
                        // This is a function code (name / offset) breakpoint, get the location and language:
                        requiredFields |= BPREQI_BPLOCATION | BPREQI_LANGUAGE;
                    }
                    break;

                    case BPLT_CONTEXT:
                    case BPLT_STRING:
                    case BPLT_ADDRESS:
                    case BPLT_RESOLUTION:
                    {
                        // These breakpoint types are not currently supported, just let an error be sent.
                    }
                    break;

                    default:
                    {
                        // Unexpected value!
                        GT_ASSERT(false);
                    }
                    break;
                }

                if (requiredFields != 0)
                {
                    BP_REQUEST_INFO breakpointRequestInfo = { 0 };
                    hr = pBreakpointRequest->GetRequestInfo(requiredFields, &breakpointRequestInfo);

                    // If we got everything we needed:
                    if (SUCCEEDED(hr))
                    {
                        // Check if the breakpoint contain condition:
                        bool isBreakpointConditioned = (breakpointRequestInfo.bpCondition.styleCondition != BP_COND_NONE);

                        if (!isBreakpointConditioned)
                        {
                            // Handle the breakpoint, according to its type:
                            switch (breakpointLocationKind)
                            {
                                case BPLT_FILE_LINE:
                                {
                                    // Get the parameters we requested:
                                    BP_LOCATION& breakpointLocation = breakpointRequestInfo.bpLocation;

                                    IDebugDocumentPosition2* pDocumentPosition = breakpointLocation.bpLocation.bplocCodeFileLine.pDocPos;

                                    if (pDocumentPosition != NULL)
                                    {
                                        // Get the file name:
                                        BSTR fileNameAsBSTR = NULL;
                                        hr = pDocumentPosition->GetFileName(&fileNameAsBSTR);

                                        if (SUCCEEDED(hr) && fileNameAsBSTR != NULL)
                                        {
                                            // See if this file name is the source for some program (if not, it's a real source file,
                                            // in which case we cannot bind the breakpoint):
                                            sourceFilePath.setFullPathFromString(fileNameAsBSTR);
                                            gtString sourceFileExtension;
                                            sourceFilePath.getFileExtension(sourceFileExtension);

                                            oaCLProgramHandle progHandle = OA_CL_NULL_HANDLE;
                                            osFilePath temporaryNewSourceFilePath;
                                            bool rcHand = gaGetOpenCLProgramHandleFromSourceFilePath(sourceFilePath, temporaryNewSourceFilePath, progHandle);
                                            osFilePath breakpointSourceCodeFilePath = sourceFilePath;

                                            if (!temporaryNewSourceFilePath.isEmpty())
                                            {
                                                // If the breakpoint's source should be replaced by new cl temporary file:
                                                breakpointSourceCodeFilePath = temporaryNewSourceFilePath;
                                            }

                                            // Get the range in the code where the breakpoint is:
                                            TEXT_POSITION startPos = { 0 };
                                            TEXT_POSITION endPos = { 0 };
                                            hr = pDocumentPosition->GetRange(&startPos, &endPos);

                                            if (SUCCEEDED(hr))
                                            {
                                                // TO_DO: for some reason, breakpoint requests (clicking the margin or pressing F9) give a
                                                // 10-line range which starts at the line before the breakpoint. So get the actual line #:
                                                lineNumber = (int)(startPos.dwLine) + 1;

                                                // Set the breakpoint request details:
                                                breakpoint.setSourceRequest(breakpointSourceCodeFilePath, lineNumber);

                                                // Check if the breakpoint is enabled, adding disabled breakpoints causes the VS breakpoint events not to be accepted.
                                                bool isBreakpointEnabled = true;
                                                m_pOwner->IsSrcLocationBreakpointEnabled(breakpointSourceCodeFilePath.asString().asCharArray(),
                                                                                         lineNumber, isBreakpointEnabled);

                                                if (rcHand && (progHandle != OA_CL_NULL_HANDLE))
                                                {
                                                    // Copy the values:
                                                    openCLProgramHandle = progHandle;

                                                    // Try to bind the breakpoint:
                                                    canBind = addAPIBreakpointInOpenCLProgram(openCLProgramHandle, lineNumber, performBind, errorType, errorMessage, isBreakpointEnabled);
                                                }
                                                else
                                                {
                                                    canBind = addAPIBreakpointInHostSource(sourceFilePath, lineNumber, performBind, errorType, errorMessage, isBreakpointEnabled);
                                                }
                                            }

                                            isCLCodeBreakpoint = ((openCLProgramHandle != OA_CL_NULL_HANDLE) || (sourceFileExtension == CS_STR_kernelSourceFileExtension));

                                            if (!isCLCodeBreakpoint)
                                            {
                                                errorMessage = VSP_STR_BreakpointErrorC_CPPCodeIsNotSupported;
                                            }

                                            // Release the string:
                                            SysFreeString(fileNameAsBSTR);
                                        }

                                        // Release the position interface:
                                        breakpointLocation.bpLocation.bplocCodeFileLine.pDocPos->Release();
                                        breakpointLocation.bpLocation.bplocCodeFileLine.pDocPos = NULL;
                                    }
                                }
                                break;

                                case BPLT_FUNC_OFFSET:
                                {
                                    // Get the parameters we requested:
                                    BP_LOCATION& breakpointLocation = breakpointRequestInfo.bpLocation;
                                    GUID& breakpointLanguage = breakpointRequestInfo.guidLanguage;

                                    // Get the function address:
                                    IDebugFunctionPosition2* pFunctionPosition = breakpointLocation.bpLocation.bplocCodeFuncOffset.pFuncPos;

                                    if (pFunctionPosition != NULL)
                                    {
                                        // We support C and C++ breakpoints:
                                        if ((breakpointLanguage == guidCPPLang) || (breakpointLanguage == guidCLang))
                                        {
                                            // Get the function Name:
                                            BSTR functionNameAsBSTR = NULL;
                                            hr = pFunctionPosition->GetFunctionName(&functionNameAsBSTR);

                                            if (SUCCEEDED(hr) && (functionNameAsBSTR != NULL))
                                            {
                                                gtString functionName = functionNameAsBSTR;

                                                // Check if the breakpoint is enabled, adding disabled breakpoints causes the VS breakpoint events not to be accepted.
                                                bool isBreakpointEnabled = true;
                                                m_pOwner->IsFuncBreakpointEnabled(functionName.asCharArray(), isBreakpointEnabled);

                                                // Add a breakpoint on this function:
                                                canBind = addAPIBreakpointOnNamedFunction(functionName, performBind, errorType, errorMessage, breakpointFuncId, breakpointKernelFunctionName, bpGenericType, isBreakpointEnabled);

                                                // Release the string:
                                                SysFreeString(functionNameAsBSTR);
                                                functionNameAsBSTR = NULL;
                                            }
                                        }

                                        // Release the position interface:
                                        breakpointLocation.bpLocation.bplocCodeFuncOffset.pFuncPos->Release();
                                        breakpointLocation.bpLocation.bplocCodeFuncOffset.pFuncPos = NULL;
                                    }

                                    // Release the context string if it was allocated:
                                    if (breakpointLocation.bpLocation.bplocCodeFuncOffset.bstrContext != NULL)
                                    {
                                        SysFreeString(breakpointLocation.bpLocation.bplocCodeFuncOffset.bstrContext);
                                        breakpointLocation.bpLocation.bplocCodeFuncOffset.bstrContext = NULL;
                                    }
                                }
                                break;

                                case BPLT_CONTEXT:
                                case BPLT_STRING:
                                case BPLT_ADDRESS:
                                case BPLT_RESOLUTION:
                                {
                                    // These breakpoint types are not currently supported, we should not have gotten here!
                                    GT_ASSERT(false);
                                }
                                break;

                                default:
                                {
                                    // Unexpected value!
                                    GT_ASSERT(false);
                                }
                                break;
                            }
                        }

                        // We do not support conditional breakpoints:
                        if (isBreakpointConditioned)
                        {
                            errorType = BPET_GENERAL_WARNING;
                            errorMessage = VSP_STR_BreakpointErrorConditionIsNotSupported;
                            canBind = false;
                        }

                        // Send any events and update the breakpoint object:
                        onBreakpointBindAttempt(breakpoint, canBind, performBind, errorType, errorMessage);
                    }
                }

                // If we were testing for binding ability and failed, the retVal will still be S_OK:
                if ((!canBind) && (!performBind))
                {
                    // We were checking if binding is possible, so we should return false here:
                    retVal = S_FALSE;
                }

                // If we are making this breakpoint a bound / error breakpoint:
                if (performBind)
                {
                    // Let the breakpoint know its type:
                    setBreakpointType(breakpoint, breakpointLocationKind, isCLCodeBreakpoint, openCLProgramHandle, sourceFilePath, lineNumber, breakpointFuncId, breakpointKernelFunctionName, bpGenericType, canBind);
                }
            }
        }
        else // pBreakpointRequest == NULL
        {
            // No request, the breakpoint object shouldn't even exist!
            retVal = E_FAIL;
        }
    }
    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        vscBreakpointsManager::rebindBreakpointsInKernelSourceFile
// Description: When a new kernel source file is added (i.e. a program is created)
//              attempts to rebind all the breakpoints in the file
// Author:      Uri Shomroni
// Date:        1/5/2011
// ---------------------------------------------------------------------------
void vscBreakpointsManager::rebindBreakpointsInKernelSourceFile(const osFilePath& kernelSourceFilePath)
{
    GT_UNREFERENCED_PARAMETER(kernelSourceFilePath);

    int numberOfBreakpoints = (int)_breakpoints.size();

    for (int i = 0; i < numberOfBreakpoints; i++)
    {
        // Get the current breakpoint:
        vspCDebugBreakpoint* pCurrentBreakpoint = _breakpoints[i];

        if (pCurrentBreakpoint != NULL)
        {
            // If it is an OpenCL source breakpoint that is not yet bound:
            if (pCurrentBreakpoint->breakpointType() == vspCDebugBreakpoint::VSP_BREAKPOINT_OPENCL_C_CODE_ERROR)
            {
                // Attempt to bind it after this change:
                bindBreakpoint(*pCurrentBreakpoint, true);
            }
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        vscBreakpointsManager::onKernelSourceCodeBreakpointsUpdated
// Description: Called when kernel debugging breakpoint bindings are updated
//              while kernel debugging, to update the breakpoints.
// Author:      Uri Shomroni
// Date:        8/5/2011
// ---------------------------------------------------------------------------
void vscBreakpointsManager::onKernelSourceCodeBreakpointsUpdated(const apKernelSourceBreakpointsUpdatedEvent& eve)
{
    // Get the program handle:
    oaCLProgramHandle updatedProgramHandle = eve.debuggedProgramHandle();

    // Iterate the breakpoints:
    int numberOfBreakpoints = (int)_breakpoints.size();

    for (int i = 0; i < numberOfBreakpoints; i++)
    {
        // Sanity check:
        vspCDebugBreakpoint* pCurrentBreakpoint = _breakpoints[i];
        GT_IF_WITH_ASSERT(pCurrentBreakpoint != NULL)
        {
            // If the breakpoint is a successfully bound kernel source breakpoint:
            if (pCurrentBreakpoint->breakpointType() == vspCDebugBreakpoint::VSP_BREAKPOINT_OPENCL_C_CODE)
            {
                // If it's in the right program:
                if (pCurrentBreakpoint->openCLProgramHandle() == updatedProgramHandle)
                {
                    // Get its line number:
                    int requestedLineNumber = pCurrentBreakpoint->openCLProgramLineNumber();

                    if (requestedLineNumber > -1)
                    {
                        // If it matches one of the breakpoint line numbers that was updated:
                        int boundLineNumber = eve.getBreakpointBoundLineNumber(requestedLineNumber);

                        if (boundLineNumber > -1)
                        {
                            // Update the breakpoint and send the binding event to Visual Studio:
                            pCurrentBreakpoint->setOpenCLCCodeBreakpointBinding(boundLineNumber);
                            bindBreakpoint(*pCurrentBreakpoint, true);
                        }
                    }
                }
            }
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        vscBreakpointsManager::deleteBreakpoint
// Description: Performs the actions required by IDebugPendingBreakpoint2::Delete
//              i.e. deletes the actual breakpoint.
// Author:      Uri Shomroni
// Date:        4/10/2010
// ---------------------------------------------------------------------------
void vscBreakpointsManager::deleteBreakpoint(vspCDebugBreakpoint& breakpoint)
{
    // Get the breakpoint type:
    vspCDebugBreakpoint::vspBreakpointType breakpointType = breakpoint.breakpointType();

    vspGRApiFunctions& theVSGRApiFunctions = vspGRApiFunctions::vspInstance();

    switch (breakpointType)
    {
        case vspCDebugBreakpoint::VSP_BREAKPOINT_MONITORED_FUNCTION:
        {
            // This is a monitored function breakpoint, we need to update the API of this deletion:
            apMonitoredFunctionId monitoredFunctionId = breakpoint.breakpointFunctionId();
            GT_IF_WITH_ASSERT(monitoredFunctionId < apMonitoredFunctionsAmount)
            {
                // Find the breakpoint:
                int breakpointIndex = -1;
                apMonitoredFunctionBreakPoint dummyBreakpoint(monitoredFunctionId);
                bool rcIdx = gaGetBreakpointIndex(dummyBreakpoint, breakpointIndex);

                if (rcIdx && (breakpointIndex > -1))
                {
                    // Remove it:
                    bool rcRem = theVSGRApiFunctions.removeAPIBreakpoint(breakpointIndex);
                    GT_ASSERT(rcRem);

                    // Remove it from the map too.
                    auto iter = _breakpointByFuncId.find(monitoredFunctionId);

                    if (iter != _breakpointByFuncId.end())
                    {
                        _breakpointByFuncId.erase(iter);
                    }
                }
            }
        }
        break;

        case vspCDebugBreakpoint::VSP_BREAKPOINT_GENERIC:
        {
            // This is a generic breakpoint, we need to update the API of this deletion:
            apGenericBreakpointType breakpointTypeGen = breakpoint.genericBreakpointType();
            GT_IF_WITH_ASSERT((breakpointTypeGen > AP_BREAK_TYPE_UNKNOWN) && (breakpointTypeGen < AP_AMOUNT_OF_GENERIC_BREAKPOINT_TYPES))
            {
                // Remove the breakpoint:
                bool rcRem = theVSGRApiFunctions.gaRemoveGenericBreakpoint(breakpointTypeGen);
                GT_ASSERT(rcRem);

                // Remove it from the map too.
                auto iter = _breakpointByGenericType.find(breakpointTypeGen);

                if (iter != _breakpointByGenericType.end())
                {
                    _breakpointByGenericType.erase(iter);
                }
            }
        }
        break;

        case vspCDebugBreakpoint::VSP_BREAKPOINT_KERNEL_FUNCTION_NAME:
        {
            // This is a kernel function breakpoint, we need to update the API of this deletion:
            gtString kernelFunctionName = breakpoint.breakpointKernelFunctionName();

            // Find the breakpoint:
            int breakpointIndex = -1;
            apKernelFunctionNameBreakpoint dummyBreakpoint(kernelFunctionName);
            bool rcIdx = gaGetBreakpointIndex(dummyBreakpoint, breakpointIndex);

            if (rcIdx && (breakpointIndex > -1))
            {
                // Remove it:
                bool rcRem = theVSGRApiFunctions.removeAPIBreakpoint(breakpointIndex);
                GT_ASSERT(rcRem);

                auto iter = _breakpointByKernelFuncName.find(kernelFunctionName);

                if (iter != _breakpointByKernelFuncName.end())
                {
                    _breakpointByKernelFuncName.erase(iter);
                }
            }
        }
        break;

        case vspCDebugBreakpoint::VSP_BREAKPOINT_OPENCL_C_CODE:
        {
            // This is a kernel source breakpoint, we need to update the API of this deletion:
            oaCLProgramHandle openCLProgramHandle = breakpoint.openCLProgramHandle();
            int openCLProgramLineNumber = breakpoint.openCLProgramLineNumber();

            // Find the breakpoint:
            int breakpointIndex = -1;
            apKernelSourceCodeBreakpoint dummyBreakpoint(openCLProgramHandle, openCLProgramLineNumber);
            bool rcIdx = gaGetBreakpointIndex(dummyBreakpoint, breakpointIndex);

            if (rcIdx && (breakpointIndex > -1))
            {
                // Remove it:
                bool rcRem = theVSGRApiFunctions.removeAPIBreakpoint(breakpointIndex);
                GT_ASSERT(rcRem);
            }
        }
        break;

        case vspCDebugBreakpoint::VSP_BREAKPOINT_C_CPP_CODE:
        {
            // This is a host source breakpoint, update the process debugger of the change:
            const osFilePath& sourcePath = breakpoint.hostSourcePath();
            int lineNum = breakpoint.hostLineNumber();

            // Find the breakpoint:
            int breakpointIndex = -1;
            apHostSourceCodeBreakpoint dummyBreakpoint(sourcePath, lineNum);
            bool rcIdx = gaGetBreakpointIndex(dummyBreakpoint, breakpointIndex);

            if (rcIdx && (breakpointIndex > -1))
            {
                // Remove it:
                bool rcRem = theVSGRApiFunctions.removeAPIBreakpoint(breakpointIndex);
                GT_ASSERT(rcRem);
            }
        }
        break;

        case vspCDebugBreakpoint::VSP_BREAKPOINT_UNBOUND:
        case vspCDebugBreakpoint::VSP_BREAKPOINT_NON_MONITORED_FUNCTION:
        case vspCDebugBreakpoint::VSP_BREAKPOINT_C_CPP_CODE_ERROR:
        case vspCDebugBreakpoint::VSP_BREAKPOINT_OPENCL_C_CODE_ERROR:
        case vspCDebugBreakpoint::VSP_BREAKPOINT_TYPE_UNKNOWN:
        {
            // No actions are required except removing the breakpoint from our vectors.
        }
        break;

        default:
        {
            // Unexpected breakpoint type!
            GT_ASSERT(false);
        }
        break;
    }

    // Remove the breakpoint from our vectors:
    removeBreakpointFromVectors(breakpoint.breakpointRequest());
}

// ---------------------------------------------------------------------------
// Name:        vscBreakpointsManager::setBreakpointEnableStatus
// Description: Enables or disables a breakpoint
// Author:      Uri Shomroni
// Date:        8/2/2011
// ---------------------------------------------------------------------------
void vscBreakpointsManager::setBreakpointEnableStatus(vspCDebugBreakpoint& breakpoint)
{
    // Get the breakpoint type:
    vspCDebugBreakpoint::vspBreakpointType breakpointType = breakpoint.breakpointType();
    bool enableBreakpoint = breakpoint.isEnabled();

    vspGRApiFunctions& theVSGRApiFunctions = vspGRApiFunctions::vspInstance();

    switch (breakpointType)
    {
        case vspCDebugBreakpoint::VSP_BREAKPOINT_MONITORED_FUNCTION:
        {
            // This is a monitored function breakpoint, we need to update the API of this change:
            apMonitoredFunctionId monitoredFunctionId = breakpoint.breakpointFunctionId();
            GT_IF_WITH_ASSERT(monitoredFunctionId < apMonitoredFunctionsAmount)
            {
                // Create a toggled version of this breakpoint:
                apMonitoredFunctionBreakPoint toggledBreakpoint(monitoredFunctionId);
                toggledBreakpoint.setEnableStatus(enableBreakpoint);

                // Set it into the API:
                bool rcTog = theVSGRApiFunctions.setAPIBreakpoint(toggledBreakpoint);
                GT_ASSERT(rcTog);
            }
        }
        break;

        case vspCDebugBreakpoint::VSP_BREAKPOINT_KERNEL_FUNCTION_NAME:
        {
            // This is a kernel function breakpoint, we need to update the API of this change:
            gtString kernelFunctionName = breakpoint.breakpointKernelFunctionName();

            // Create a toggled version of this breakpoint:
            apKernelFunctionNameBreakpoint toggledBreakpoint(kernelFunctionName);
            toggledBreakpoint.setEnableStatus(enableBreakpoint);

            // Set it into the API:
            bool rcTog = theVSGRApiFunctions.setAPIBreakpoint(toggledBreakpoint);
            GT_ASSERT(rcTog);
        }
        break;

        case vspCDebugBreakpoint::VSP_BREAKPOINT_GENERIC:
        {
            // Get the generic breakpoint type:
            apGenericBreakpointType bpType = breakpoint.genericBreakpointType();

            // Create a toggled version of this breakpoint:
            apGenericBreakpoint toggledBreakpoint(bpType);
            toggledBreakpoint.setEnableStatus(enableBreakpoint);

            // Set it into the API:
            bool rcTog = theVSGRApiFunctions.setAPIBreakpoint(toggledBreakpoint);
            GT_ASSERT(rcTog);
        }
        break;

        case vspCDebugBreakpoint::VSP_BREAKPOINT_OPENCL_C_CODE:
        {
            // This is a kernel source breakpoint, we need to update the API of this change:
            oaCLProgramHandle openCLProgramHandle = breakpoint.openCLProgramHandle();
            int openCLProgramLineNumber = breakpoint.openCLProgramLineNumber();

            // Create a toggled version of this breakpoint:
            apKernelSourceCodeBreakpoint toggledBreakpoint(openCLProgramHandle, openCLProgramLineNumber);
            toggledBreakpoint.setEnableStatus(enableBreakpoint);

            // Set it into the API:
            bool rcTog = theVSGRApiFunctions.setAPIBreakpoint(toggledBreakpoint);
            GT_ASSERT(rcTog);
        }
        break;

        case vspCDebugBreakpoint::VSP_BREAKPOINT_C_CPP_CODE:
        {
            // This is a host source breakpoint, update the process debugger of the change:
            const osFilePath& sourcePath = breakpoint.hostSourcePath();
            int lineNum = breakpoint.hostLineNumber();

            // Create a toggled version of this breakpoint:
            apHostSourceCodeBreakpoint toggledBreakpoint(sourcePath, lineNum);
            toggledBreakpoint.setEnableStatus(enableBreakpoint);

            // Set it into the API:
            bool rcTog = theVSGRApiFunctions.setAPIBreakpoint(toggledBreakpoint);
            GT_ASSERT(rcTog);
        }
        break;

        case vspCDebugBreakpoint::VSP_BREAKPOINT_UNBOUND:
        case vspCDebugBreakpoint::VSP_BREAKPOINT_NON_MONITORED_FUNCTION:
        case vspCDebugBreakpoint::VSP_BREAKPOINT_C_CPP_CODE_ERROR:
        case vspCDebugBreakpoint::VSP_BREAKPOINT_OPENCL_C_CODE_ERROR:
        case vspCDebugBreakpoint::VSP_BREAKPOINT_TYPE_UNKNOWN:
        {
            // No actions are required.
        }
        break;

        default:
        {
            // Unexpected breakpoint type!
            GT_ASSERT(false);
        }
        break;
    }
}

// ---------------------------------------------------------------------------
// Name:        vscBreakpointsManager::getMonitoredFunctionBreakpoint
// Description: If we have a bound breakpoint for funcId, return it. Otherwise,
//              return NULL
// Author:      Uri Shomroni
// Date:        5/10/2010
// ---------------------------------------------------------------------------
vspCDebugBreakpoint* vscBreakpointsManager::getMonitoredFunctionBreakpoint(apMonitoredFunctionId funcId)
{
    vspCDebugBreakpoint* retVal = NULL;

    // Sanity check:
    if (funcId < apMonitoredFunctionsAmount && !_breakpointByFuncId.empty())
    {
        auto iter = _breakpointByFuncId.find(funcId);

        if (iter != _breakpointByFuncId.end())
        {
            vspCDebugBreakpoint* pCurrentBreakpoint = iter->second;
            GT_IF_WITH_ASSERT(pCurrentBreakpoint != NULL)
            {
                // Check if it is bound.
                if (pCurrentBreakpoint->breakpointStatus() == vspCDebugBreakpoint::VSP_BREAKPOINT_BOUND)
                {
                    retVal = pCurrentBreakpoint;
                }
            }
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        vscBreakpointsManager::getGenericBreakpoint
// Description: If we have a bound breakpoint for the requested type, return it,
//              otherwise return null.
// Arguments:   apGenericBreakpointType breakpointType
// Return Val:  vspCDebugBreakpoint*
// Author:      Sigal Algranaty
// Date:        10/7/2011
// ---------------------------------------------------------------------------
vspCDebugBreakpoint* vscBreakpointsManager::getGenericBreakpoint(apGenericBreakpointType breakpointType)
{
    vspCDebugBreakpoint* retVal = NULL;
    auto iter = _breakpointByGenericType.find(breakpointType);

    if (iter != _breakpointByGenericType.end())
    {
        vspCDebugBreakpoint* pCurrentBreakpoint = iter->second;
        GT_IF_WITH_ASSERT(pCurrentBreakpoint != NULL)
        {
            if (pCurrentBreakpoint->breakpointStatus() == vspCDebugBreakpoint::VSP_BREAKPOINT_BOUND)
            {
                retVal = pCurrentBreakpoint;
            }
        }
    }


    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        vscBreakpointsManager::getKernelFunctionNameBreakpoint
// Description: If we have a bound breakpoint for the given kernel function name,
//              return it. Otherwise, return NULL
// Author:      Uri Shomroni
// Date:        27/2/2011
// ---------------------------------------------------------------------------
vspCDebugBreakpoint* vscBreakpointsManager::getKernelFunctionNameBreakpoint(const gtString& kernelFuncName)
{
    vspCDebugBreakpoint* retVal = NULL;

    // Sanity check:
    if (!kernelFuncName.isEmpty())
    {
        // Kernel function.
        auto iter = _breakpointByKernelFuncName.find(kernelFuncName);
        GT_IF_WITH_ASSERT(iter != _breakpointByKernelFuncName.end())
        {
            vspCDebugBreakpoint* pCurrentBreakpoint = iter->second;
            GT_IF_WITH_ASSERT(pCurrentBreakpoint != NULL)
            {
                if (pCurrentBreakpoint->breakpointStatus() == vspCDebugBreakpoint::VSP_BREAKPOINT_BOUND)
                {
                    retVal = pCurrentBreakpoint;
                }
            }
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        vscBreakpointsManager::getKernelSourceCodeBreakpoint
// Description: If we have a bound breakpoint for the given location, return it.
//              Otherwise, return NULL
// Author:      Uri Shomroni
// Date:        9/11/2010
// ---------------------------------------------------------------------------
vspCDebugBreakpoint* vscBreakpointsManager::getKernelSourceCodeBreakpoint(oaCLProgramHandle programHandle, int lineNumber)
{
    vspCDebugBreakpoint* retVal = NULL;

    // Iterate the breakpoints:
    int numberOfBreakpoints = (int)_breakpoints.size();

    for (int i = 0; i < numberOfBreakpoints; i++)
    {
        // Get the current breakpoint:
        vspCDebugBreakpoint* pCurrentBreakpoint = _breakpoints[i];
        GT_IF_WITH_ASSERT(pCurrentBreakpoint != NULL)
        {
            // If it is a kernel source breakpoint:
            if (pCurrentBreakpoint->breakpointType() == vspCDebugBreakpoint::VSP_BREAKPOINT_OPENCL_C_CODE)
            {
                // If it is bound:
                if (pCurrentBreakpoint->breakpointStatus() == vspCDebugBreakpoint::VSP_BREAKPOINT_BOUND)
                {
                    // If this is the correct breakpoint (note that for unbound breakpoints and breakpoints
                    // that are not monitored functions, this functions returns apMonitoredFunctionsAmount):
                    if ((pCurrentBreakpoint->openCLProgramHandle() == programHandle) && ((pCurrentBreakpoint->openCLProgramLineNumber() == lineNumber) || (pCurrentBreakpoint->openCLProgramBoundLineNumber() == lineNumber)))
                    {
                        // We found our breakpoint, return it and stop looking:
                        retVal = pCurrentBreakpoint;
                        break;
                    }
                }
            }
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        vscBreakpointsManager::getHostSourceBreakpoint
// Description: If we have a bound breakpoint for the given location, return it.
//              Otherwise, return NULL
// Author:      Uri Shomroni
// Date:        13/1/2016
// ---------------------------------------------------------------------------
vspCDebugBreakpoint* vscBreakpointsManager::getHostSourceBreakpoint(const osFilePath& sourcePath, int lineNumber)
{
    vspCDebugBreakpoint* retVal = NULL;

    // Iterate the breakpoints:
    int numberOfBreakpoints = (int)_breakpoints.size();

    for (int i = 0; i < numberOfBreakpoints; i++)
    {
        // Get the current breakpoint:
        vspCDebugBreakpoint* pCurrentBreakpoint = _breakpoints[i];
        GT_IF_WITH_ASSERT(pCurrentBreakpoint != NULL)
        {
            // If it is a kernel source breakpoint:
            if (pCurrentBreakpoint->breakpointType() == vspCDebugBreakpoint::VSP_BREAKPOINT_C_CPP_CODE)
            {
                // If it is bound:
                if (pCurrentBreakpoint->breakpointStatus() == vspCDebugBreakpoint::VSP_BREAKPOINT_BOUND)
                {
                    // If this is the correct breakpoint:
                    const osFilePath& currentHostPath = pCurrentBreakpoint->hostSourcePath();
                    int currentLineNumber = pCurrentBreakpoint->hostLineNumber();

                    if ((currentHostPath == sourcePath) && (currentLineNumber == lineNumber))
                    {
                        // We found our breakpoint, return it and stop looking:
                        retVal = pCurrentBreakpoint;
                        break;
                    }
                    else if (sourcePath.asString().isEmpty() || (0 > lineNumber))
                    {
                        // Bad request probably comes from process debugger problems:
                        GT_ASSERT(false);
                        retVal = pCurrentBreakpoint;
                        break;
                    }
                }
            }
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        vscBreakpointsManager::requestMonitoredFunctionBreakpoint
// Description: Adds breakpoint to the VS debugger interface as a function breakpoint
// Return Val:  bool - Success / failure.
// Author:      Uri Shomroni
// Date:        28/9/2010
// ---------------------------------------------------------------------------
bool vscBreakpointsManager::requestMonitoredFunctionBreakpoint(const apMonitoredFunctionBreakPoint& breakpoint)
{
    bool retVal = false;

    GT_IF_WITH_ASSERT(_piDebugger != NULL)
    {
        // Get the function's name:
        apMonitoredFunctionId funcId = breakpoint.monitoredFunctionId();
        const wchar_t* funcName = apMonitoredFunctionsManager::instance().monitoredFunctionName(funcId);

        // Add it as a C++ breakpoint, so that our debug engine will recognize it:
        HRESULT hr = _piDebugger->InsertBreakpointByName(guidCPPLang, funcName);

        retVal = SUCCEEDED(hr);

        if (retVal)
        {
            if (!breakpoint.isEnabled())
            {
                GT_IF_WITH_ASSERT(m_pOwner != NULL)
                {
                    // Disable the breakpoint:
                    bool rcDsb = m_pOwner->DisableFuncBreakpoint(funcName);
                    GT_ASSERT(rcDsb);
                }
            }

            // Get the breakpoint handle:
            vspCDebugBreakpoint* pBreakpoint = getMonitoredFunctionBreakpoint(breakpoint.monitoredFunctionId());

            if (nullptr != pBreakpoint)
            {
                // Set the breakpoint hit count:
                pBreakpoint->SetHitCount(breakpoint.hitCount());
            }
            else
            {
                // If a debug engine is active, the request should have created a vspCDebugBreakpoint:
                GT_ASSERT(nullptr == _pDebugEngine);
            }
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        vscBreakpointsManager::removeMonitoredFunctionBreakpoint
// Description: Removes a breakpoint from the VS debugger interface as a function breakpoint
// Return Val:  bool - Success / failure.
// Author:      Uri Shomroni
// Date:        28/9/2010
// ---------------------------------------------------------------------------
bool vscBreakpointsManager::removeMonitoredFunctionBreakpoint(const apMonitoredFunctionBreakPoint& breakpoint)
{
    bool retVal = false;

    GT_IF_WITH_ASSERT(_piDebugger != NULL)
    {
        // Get the function's name:
        apMonitoredFunctionId funcId = breakpoint.monitoredFunctionId();
        const wchar_t* funcName = apMonitoredFunctionsManager::instance().monitoredFunctionName(funcId);
        BSTR funcNameAsBSTR = SysAllocString(funcName);

        // Look for it as a C/C++ breakpoint, so as that's how our debug engine would have recognized it:
        BOOL isCppBreakpoint = FALSE;
        HRESULT hr = _piDebugger->IsBreakpointOnName(guidCPPLang, funcNameAsBSTR, &isCppBreakpoint);
        bool rcCpp = SUCCEEDED(hr);

        if (rcCpp && (isCppBreakpoint == TRUE))
        {
            // If we managed to get the status, and it IS a breakpoint, remove it:
            hr = _piDebugger->RemoveBreakpointsByName(guidCPPLang, funcNameAsBSTR);
            rcCpp = SUCCEEDED(hr);
        }

        BOOL isCBreakpoint = FALSE;
        hr = _piDebugger->IsBreakpointOnName(guidCLang, funcNameAsBSTR, &isCBreakpoint);
        bool rcC = SUCCEEDED(hr);

        if (rcC && (isCBreakpoint == TRUE))
        {
            // If we managed to get the status, and it IS a breakpoint, remove it:
            hr = _piDebugger->RemoveBreakpointsByName(guidCLang, funcNameAsBSTR);
            rcC = SUCCEEDED(hr);
        }

        // Free the string:
        SysFreeString(funcNameAsBSTR);

        retVal = rcCpp && rcC;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        vscBreakpointsManager::requestKernelSourceCodeBreakpoint
// Description: Adds breakpoint to the VS debugger interface as a source breakpoint
// Return Val:  bool - Success / failure.
// Author:      Uri Shomroni
// Date:        8/11/2010
// ---------------------------------------------------------------------------
bool vscBreakpointsManager::requestKernelSourceCodeBreakpoint(const apKernelSourceCodeBreakpoint& breakpoint)
{
    bool retVal = false;
    GT_IF_WITH_ASSERT(m_pOwner != NULL)
    {
        // Get the code location:
        osFilePath sourceFilePath;
        int lineNum = -1;
        gaCodeLocationFromKernelSourceBreakpoint(breakpoint, sourceFilePath, lineNum);

        // Add the breakpoint:
        retVal = m_pOwner->AddBreakpointInSourceLocation(sourceFilePath.asString().asCharArray(), lineNum, breakpoint.isEnabled());
    }
    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        vscBreakpointsManager::removeKernelSourceCodeBreakpoint
// Description: Removes a breakpoint from the VS debugger interface as a code breakpoint
// Return Val:  bool - Success / failure.
// Author:      Uri Shomroni
// Date:        28/9/2010
// ---------------------------------------------------------------------------
bool vscBreakpointsManager::removeKernelSourceCodeBreakpoint(const apKernelSourceCodeBreakpoint& breakpoint)
{
    bool retVal = false;

    GT_IF_WITH_ASSERT(m_pOwner != NULL)
    {
        // Get the code location:
        osFilePath sourceFilePath;
        int lineNum = -1;
        gaCodeLocationFromKernelSourceBreakpoint(breakpoint, sourceFilePath, lineNum);

        // Remove the breakpoint:
        retVal = m_pOwner->RemoveBreakpointsInSourceLocation(sourceFilePath.asString().asCharArray(), lineNum);
    }
    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        vscBreakpointsManager::requestHostSourceCodeBreakpoint
// Description: Adds breakpoint to the VS debugger interface as a source breakpoint
// Return Val:  bool - Success / failure.
// Author:      Uri Shomroni
// Date:        18/2/2016
// ---------------------------------------------------------------------------
bool vscBreakpointsManager::requestHostSourceCodeBreakpoint(const apHostSourceCodeBreakpoint& breakpoint)
{
    bool retVal = false;
    GT_IF_WITH_ASSERT(nullptr != m_pOwner)
    {
        // Get the code location:
        const osFilePath& sourceFilePath = breakpoint.filePath();
        int lineNum = breakpoint.lineNumber();

        // Add the breakpoint:
        retVal = m_pOwner->AddBreakpointInSourceLocation(sourceFilePath.asString().asCharArray(), lineNum, breakpoint.isEnabled());
    }
    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        vscBreakpointsManager::removeHostSourceCodeBreakpoint
// Description: Removes a breakpoint from the VS debugger interface as a code breakpoint
// Return Val:  bool - Success / failure.
// Author:      Uri Shomroni
// Date:        18/2/2016
// ---------------------------------------------------------------------------
bool vscBreakpointsManager::removeHostSourceCodeBreakpoint(const apHostSourceCodeBreakpoint& breakpoint)
{
    bool retVal = false;

    GT_IF_WITH_ASSERT(nullptr != m_pOwner)
    {
        // Get the code location:
        const osFilePath& sourceFilePath = breakpoint.filePath();
        int lineNum = breakpoint.lineNumber();

        // Remove the breakpoint:
        retVal = m_pOwner->RemoveBreakpointsInSourceLocation(sourceFilePath.asString().asCharArray(), lineNum);
    }
    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        vscBreakpointsManager::requestKernelFunctionNameBreakpoint
// Description: Adds breakpoint to the VS debugger interface as a function breakpoint
// Return Val:  bool - Success / failure.
// Author:      Uri Shomroni
// Date:        27/2/2011
// ---------------------------------------------------------------------------
bool vscBreakpointsManager::requestKernelFunctionNameBreakpoint(const apKernelFunctionNameBreakpoint& breakpoint)
{
    bool retVal = false;

    GT_IF_WITH_ASSERT(_piDebugger != NULL)
    {
        // Get the kernel function's name:
        gtString funcName = breakpoint.kernelFunctionName();

        // Add a prefix so we'll know it's a kernel:
        funcName.prepend(GD_STR_KernelFunctionNameBreakpointPrefix);

        // Add it as a C++ breakpoint, so that our debug engine will recognize it:
        HRESULT hr = _piDebugger->InsertBreakpointByName(guidCPPLang, funcName.asCharArray());

        retVal = SUCCEEDED(hr);

        // If the breakpoint is disabled, disable it:
        if (retVal)
        {
            if (!breakpoint.isEnabled())
            {
                GT_IF_WITH_ASSERT(m_pOwner != NULL)
                {
                    // Disable the breakpoint:
                    bool rcDsb = m_pOwner->DisableFuncBreakpoint(funcName.asCharArray());
                    GT_ASSERT(rcDsb);
                }
            }

            // Get the breakpoint handle:
            vspCDebugBreakpoint* pBreakpoint = getKernelFunctionNameBreakpoint(breakpoint.kernelFunctionName());

            if (nullptr != pBreakpoint)
            {
                // Set the breakpoint hit count:
                pBreakpoint->SetHitCount(breakpoint.hitCount());
            }
            else
            {
                // If a debug engine is active, the request should have created a vspCDebugBreakpoint:
                GT_ASSERT(nullptr == _pDebugEngine);
            }
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        vscBreakpointsManager::removeKernelFunctionNameBreakpoint
// Description: Removes a breakpoint from the VS debugger interface as a kernel
//              function name breakpoint
// Return Val:  bool - Success / failure.
// Author:      Uri Shomroni
// Date:        28/9/2010
// ---------------------------------------------------------------------------
bool vscBreakpointsManager::removeKernelFunctionNameBreakpoint(const apKernelFunctionNameBreakpoint& breakpoint)
{
    bool retVal = false;

    GT_IF_WITH_ASSERT(_piDebugger != NULL)
    {
        // Get the kernel function's name:
        gtString funcName = breakpoint.kernelFunctionName();

        // Add a prefix to match the way we added it:
        funcName.prepend(GD_STR_KernelFunctionNameBreakpointPrefix);
        BSTR funcNameAsBSTR = SysAllocString(funcName.asCharArray());

        // Look for it as a C/C++ breakpoint, so as that's how our debug engine would have recognized it:
        BOOL isCppBreakpoint = FALSE;
        HRESULT hr = _piDebugger->IsBreakpointOnName(guidCPPLang, funcNameAsBSTR, &isCppBreakpoint);
        bool rcCpp = SUCCEEDED(hr);

        if (rcCpp && (isCppBreakpoint == TRUE))
        {
            // If we managed to get the status, and it IS a breakpoint, remove it:
            hr = _piDebugger->RemoveBreakpointsByName(guidCPPLang, funcNameAsBSTR);
            rcCpp = SUCCEEDED(hr);
        }

        BOOL isCBreakpoint = FALSE;
        hr = _piDebugger->IsBreakpointOnName(guidCLang, funcNameAsBSTR, &isCBreakpoint);
        bool rcC = SUCCEEDED(hr);

        if (rcC && (isCBreakpoint == TRUE))
        {
            // If we managed to get the status, and it IS a breakpoint, remove it:
            hr = _piDebugger->RemoveBreakpointsByName(guidCLang, funcNameAsBSTR);
            rcC = SUCCEEDED(hr);
        }

        // Free the string:
        SysFreeString(funcNameAsBSTR);

        retVal = rcCpp && rcC;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        vscBreakpointsManager::requestGenericBreakpoint
// Description: Adds breakpoint to the VS debugger interface as a generic breakpoint
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        10/7/2011
// ---------------------------------------------------------------------------
bool vscBreakpointsManager::requestGenericBreakpoint(const apGenericBreakpoint& breakpoint)
{
    bool retVal = false;

    GT_IF_WITH_ASSERT(_piDebugger != NULL)
    {
        // Get the generic breakpoint name:
        gtString breakpointTypeString;
        breakpoint.breakpointTypeToString(breakpoint.breakpointType(), breakpointTypeString);

        // Add it as a C++ breakpoint, so that our debug engine will recognize it:
        HRESULT hr = _piDebugger->InsertBreakpointByName(guidCPPLang, breakpointTypeString.asCharArray());

        retVal = SUCCEEDED(hr);

        // If the breakpoint is disabled, disable it:
        if (retVal)
        {
            if (!breakpoint.isEnabled())
            {
                GT_IF_WITH_ASSERT(m_pOwner != NULL)
                {
                    bool rcDsb = m_pOwner->DisableFuncBreakpoint(breakpointTypeString.asCharArray());
                    GT_ASSERT(rcDsb);
                }
            }

            // Get the breakpoint handle:
            vspCDebugBreakpoint* pBreakpoint = getGenericBreakpoint(breakpoint.breakpointType());

            if (nullptr != pBreakpoint)
            {
                // Set the breakpoint hit count:
                pBreakpoint->SetHitCount(breakpoint.hitCount());
            }
            else
            {
                // If a debug engine is active, the request should have created a vspCDebugBreakpoint:
                GT_ASSERT(nullptr == _pDebugEngine);
            }
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        vscBreakpointsManager::removeGenericBreakpoint
// Description: Removes a breakpoint from the VS debugger interface as a generic breakpoint
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        10/7/2011
// ---------------------------------------------------------------------------
bool vscBreakpointsManager::removeGenericBreakpoint(const apGenericBreakpoint& breakpoint)
{
    bool retVal = false;

    GT_IF_WITH_ASSERT(_piDebugger != NULL)
    {
        // Get the generic breakpoint name:
        gtString breakpointTypeString;
        breakpoint.breakpointTypeToString(breakpoint.breakpointType(), breakpointTypeString);

        BSTR bpNameAsBSTR = SysAllocString(breakpointTypeString.asCharArray());

        // Look for it as a C/C++ breakpoint, so as that's how our debug engine would have recognized it:
        BOOL isCppBreakpoint = FALSE;
        HRESULT hr = _piDebugger->IsBreakpointOnName(guidCPPLang, bpNameAsBSTR, &isCppBreakpoint);
        bool rcCpp = SUCCEEDED(hr);

        if (rcCpp && (isCppBreakpoint == TRUE))
        {
            // If we managed to get the status, and it IS a breakpoint, remove it:
            hr = _piDebugger->RemoveBreakpointsByName(guidCPPLang, bpNameAsBSTR);
            rcCpp = SUCCEEDED(hr);
        }

        BOOL isCBreakpoint = FALSE;
        hr = _piDebugger->IsBreakpointOnName(guidCLang, bpNameAsBSTR, &isCBreakpoint);
        bool rcC = SUCCEEDED(hr);

        if (rcC && (isCBreakpoint == TRUE))
        {
            // If we managed to get the status, and it IS a breakpoint, remove it:
            hr = _piDebugger->RemoveBreakpointsByName(guidCLang, bpNameAsBSTR);
            rcC = SUCCEEDED(hr);
        }

        // Free the string:
        SysFreeString(bpNameAsBSTR);

        retVal = rcCpp && rcC;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        vscBreakpointsManager::addAPIBreakpointOnNamedFunction
// Description: Adds a Spy breakpoint on the function named breakpointString, if it
//              is a monitored function
// Return Val:  bool - Success / failure.
// Author:      Uri Shomroni
// Date:        29/9/2010
// ---------------------------------------------------------------------------
bool vscBreakpointsManager::addAPIBreakpointOnNamedFunction(const gtString& breakpointString, bool performBind, BP_ERROR_TYPE& errorType, gtString& errorMessage,
                                                            apMonitoredFunctionId& funcId, gtString& breakpointKernelFunctionName, apGenericBreakpointType& breakpointType, bool enabled)
{
    bool retVal = false;

    breakpointKernelFunctionName.makeEmpty();

    vspGRApiFunctions& theVSGRApiFunctions = vspGRApiFunctions::vspInstance();

    // Get the function Id:
    funcId = apMonitoredFunctionsManager::instance().monitoredFunctionId(breakpointString.asCharArray());

    // If the value is legal, the function is monitored, so we can add a breakpoint on it:
    if (funcId < apMonitoredFunctionsAmount)
    {
        if (performBind)
        {
            // Create the breakpoint:
            apMonitoredFunctionBreakPoint newBreakpoint(funcId);
            newBreakpoint.setEnableStatus(enabled);
            retVal = theVSGRApiFunctions.setAPIBreakpoint(newBreakpoint);
        }
        else // !performBind
        {
            // We are just checking validity, return true:
            retVal = true;
        }
    }
    else // funcId >= apMonitoredFunctionsAmount
    {
        // Check if this is a generic breakpoint:
        breakpointType = AP_BREAK_TYPE_UNKNOWN;
        bool isGeneric = apGenericBreakpoint::breakpointTypeFromString(breakpointString, breakpointType);

        if (isGeneric)
        {
            if (performBind)
            {
                // Create the breakpoint:
                apGenericBreakpoint newBreakpoint(breakpointType);
                newBreakpoint.setEnableStatus(enabled);
                retVal = theVSGRApiFunctions.setAPIBreakpoint(newBreakpoint);
            }
            else // !performBind
            {
                // We are just checking validity, return true:
                retVal = true;
            }
        }
        else
        {
            // If this is a kernel function name breakpoint:
            static const gtString kernelFunctionNameBreakpointPrefix = GD_STR_KernelFunctionNameBreakpointPrefix;
            static const int kernelFunctionNameBreakpointPrefixLength = kernelFunctionNameBreakpointPrefix.length();

            if (breakpointString.startsWith(kernelFunctionNameBreakpointPrefix))
            {
                // Get the kernel function name:
                breakpointKernelFunctionName = breakpointString;
                breakpointKernelFunctionName.truncate(kernelFunctionNameBreakpointPrefixLength, -1);

                if (performBind)
                {
                    // Create the breakpoint:
                    apKernelFunctionNameBreakpoint newBreakpoint(breakpointKernelFunctionName);
                    newBreakpoint.setEnableStatus(enabled);
                    retVal = theVSGRApiFunctions.setAPIBreakpoint(newBreakpoint);
                }
                else // !performBind
                {
                    // We are just checking validity, return true:
                    retVal = true;
                }
            }
            else
            {
                // This is not a kernel function or a monitored function, we cannot add a breakpoint on it:
                errorMessage = VSP_STR_BreakpointErrorC_CPPFunctionsAreNotSupported;
            }
        }
    }

    if (retVal)
    {
        errorType = BPET_NONE;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        vscBreakpointsManager::addAPIBreakpointInOpenCLProgram
// Description: Tries to add a breakpoint in program source.
// Return Val:  bool - Success / failure.
// Author:      Uri Shomroni
// Date:        8/11/2010
// ---------------------------------------------------------------------------
bool vscBreakpointsManager::addAPIBreakpointInOpenCLProgram(oaCLProgramHandle& programHandle, int lineNumber, bool performBind, BP_ERROR_TYPE& errorType, gtString& errorMessage, bool enabled)
{
    bool retVal = false;

    // TO_DO: check for programs we cannot add breakpoints on?

    bool canSetCLProgramBreakpoints = true;

    if (canSetCLProgramBreakpoints)
    {
        retVal = true;

        if (performBind)
        {
            // Try to add the breakpoint:
            apKernelSourceCodeBreakpoint newBreakpoint(programHandle, lineNumber);
            newBreakpoint.setEnableStatus(enabled);
            retVal = vspGRApiFunctions::vspInstance().setAPIBreakpoint(newBreakpoint);
        }
    }
    else // !(rcEna && canSetCLProgramBreakpoints)
    {
        errorMessage = VSP_STR_BreakpointErrorKernelBreakpointsDisabled;
    }

    if (retVal)
    {
        errorType = BPET_NONE;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        vscBreakpointsManager::addAPIBreakpointInHostSource
// Description: Tries to add a breakpoint in host source.
// Return Val:  bool - Success / failure.
// Author:      Uri Shomroni
// Date:        12/1/2016
// ---------------------------------------------------------------------------
bool vscBreakpointsManager::addAPIBreakpointInHostSource(const osFilePath& sourcePath, int lineNumber, bool performBind, BP_ERROR_TYPE& errorType, gtString& errorMessage, bool enabled)
{
    bool retVal = false;

    bool canSetHostBreakpoints = gaCanGetHostDebugging();

    if (canSetHostBreakpoints)
    {
        retVal = true;

        if (performBind)
        {
            // Try to add the breakpoint:
            apHostSourceCodeBreakpoint newBreakpoint(sourcePath, lineNumber);
            newBreakpoint.setEnableStatus(enabled);
            retVal = vspGRApiFunctions::vspInstance().setAPIBreakpoint(newBreakpoint);
        }
    }
    else // !canSetHostBreakpoints
    {
        errorMessage = VSP_STR_BreakpointErrorC_CPPCodeIsNotSupported;
    }

    if (retVal)
    {
        errorType = BPET_NONE;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        vscBreakpointsManager::onBreakpointBindAttempt
// Description: Called when a breakpoint bind attempt or breakpoint "can bind"
//              query is performed.
//              Updates the breakpoint object and sends events as needed
// Arguments: breakpoint - the breakpoint object
//            bindSuccess - can this breakpoint be bound?
//            bindAttempted - true = a bind was attempted
//                          - false = a bind was not attempted, this is just a query
//            errorType, errorMessage - error parameters, if bindSuccess = false
// Author:      Uri Shomroni
// Date:        5/10/2010
// ---------------------------------------------------------------------------
void vscBreakpointsManager::onBreakpointBindAttempt(vspCDebugBreakpoint& breakpoint, bool bindSuccess, bool bindAttempted, const BP_ERROR_TYPE& errorType, const gtString& errorMessage)
{
    if (bindSuccess)
    {
        // If we're actually performing a binding operation, do it:
        if (bindAttempted)
        {
            bool wasAlreadyBound = (breakpoint.breakpointStatus() == vspCDebugBreakpoint::VSP_BREAKPOINT_BOUND);
            bool shouldRebind = true;

            // Notify the breakpoint it's been bound:
            breakpoint.bind();

            // If the breakpoint was already bound, send an event showing it was re-bound:
            if (wasAlreadyBound && shouldRebind)
            {
                // Send the breakpoint unbound event:
                vspCDebugBreakpointUnboundEvent* pDebugBreakpointUnboundEvent = new vspCDebugBreakpointUnboundEvent(&breakpoint, BPUR_BREAKPOINT_REBIND);

                pDebugBreakpointUnboundEvent->send(_piDebugEventCallback, _pDebugEngine, NULL, NULL);
                pDebugBreakpointUnboundEvent->Release();
            }

            // Only send the bind event if something changed:
            if (!wasAlreadyBound || shouldRebind)
            {
                // Send the breakpoint bound event:
                vspCDebugBreakpointBoundEvent* pDebugBreakpointBoundEvent = new vspCDebugBreakpointBoundEvent(&breakpoint);

                pDebugBreakpointBoundEvent->send(_piDebugEventCallback, _pDebugEngine, NULL, NULL);
                pDebugBreakpointBoundEvent->Release();
            }
        }
    }
    else // !bindSuccess || isBreakpointConditioned
    {
        // Notify the breakpoint it has an error:
        breakpoint.setError(errorType, errorMessage, bindAttempted);

        // Only send the failure event if we are trying to really bind the breakpoint:
        if (bindAttempted)
        {
            // Send the breakpoint bound event:
            vspCDebugBreakpointErrorEvent* pDebugBreakpointErrorEvent = new vspCDebugBreakpointErrorEvent(&breakpoint);
            pDebugBreakpointErrorEvent->send(_piDebugEventCallback, _pDebugEngine, NULL, NULL);
            pDebugBreakpointErrorEvent->Release();
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        vscBreakpointsManager::setBreakpointType
// Description: Called when a breakpoint is bound or an error is set on it.
//              Lets the breakpoint know its type and relevant parameters.
// Author:      Uri Shomroni
// Date:        5/10/2010
// ---------------------------------------------------------------------------
void vscBreakpointsManager::setBreakpointType(vspCDebugBreakpoint& breakpoint, BP_LOCATION_TYPE breakpointLocationKind, bool isCLCodeBreakpoint, oaCLProgramHandle openCLProgramHandle, const osFilePath& sourcePath, int lineNumber,
                                              apMonitoredFunctionId breakpointFuncId, const gtString& breakpointKernelFunctionName, apGenericBreakpointType breakpointType, bool bindSuccess)
{
    switch (breakpointLocationKind)
    {
        case BPLT_FILE_LINE:
        {
            if (isCLCodeBreakpoint)
            {
                if (openCLProgramHandle != OA_CL_NULL_HANDLE)
                {
                    // This is an OpenCL C code breakpoint we can bind now:
                    breakpoint.setOpenCLCCodeBreakpoint(openCLProgramHandle, lineNumber);
                }
                else // openCLProgramHandle == OA_CL_NULL_HANDLE
                {
                    // This is an OpenCL C code breakpoint we can't bind yet:
                    breakpoint.setOpenCLCCodeErrorBreakpoint();
                }
            }
            else // !isCLCodeBreakpoint
            {
                if (gaCanGetHostDebugging())
                {
                    // This is a source code (file / line) breakpoint we can bind now:
                    breakpoint.setCCppCodeBreakpoint(sourcePath, lineNumber);
                }
                else
                {
                    // This is a source code (file / line) breakpoint we can't bind yet:
                    breakpoint.setCCppCodeErrorBreakpoint();
                }
            }
        }
        break;

        case BPLT_FUNC_OFFSET:
        {
            // This is a function code (name / offset) breakpoint:
            if (bindSuccess)
            {
                // We can bind this breakpoint, so it's either a monitored function breakpoint or a kernel function name breakpoint:
                if (breakpointFuncId < apMonitoredFunctionsAmount)
                {
                    breakpoint.setMonitoredFunctionBreakpoint(breakpointFuncId);

                    // Add the function to the relevant map (vspDebugBreakpoints of type
                    // VSP_BREAKPOINT_MONITORED_FUNCTION are uniquely distinguished by their apMonitoredFunctionId).
                    _breakpointByFuncId[breakpointFuncId] = &breakpoint;
                }
                else if (breakpointType != AP_BREAK_TYPE_UNKNOWN)
                {
                    breakpoint.setGenericBreakpointType(breakpointType);

                    // Add the function to the relevant map (vspDebugBreakpoints of type
                    // VSP_BREAKPOINT_GENERIC are uniquely distinguished by their apGenericBreakpointType property).
                    _breakpointByGenericType[breakpoint.genericBreakpointType()] = &breakpoint;

                }
                else if (!breakpointKernelFunctionName.isEmpty())
                {
                    breakpoint.setKernelFunctionNameBreakpoint(breakpointKernelFunctionName);

                    // Add the function to the relevant map (vspDebugBreakpoints of type
                    // VSP_BREAKPOINT_KERNEL_FUNCTION_NAME are uniquely distinguished by their
                    // breakpointKernelFunctionName property).
                    _breakpointByKernelFuncName[breakpoint.breakpointKernelFunctionName()] = &breakpoint;

                }
            }
            else // !bindSuccess
            {
                // We cannot bind this breakpoint, so it's a non-monitored function breakpoint:
                breakpoint.setNonMonitoredFunctionBreakpoint();
            }
        }
        break;

        case BPLT_CONTEXT:
        case BPLT_STRING:
        case BPLT_ADDRESS:
        case BPLT_RESOLUTION:
        {
            // These breakpoint types are not currently supported:
            breakpoint.setUnknownBreakpoint();
        }
        break;

        default:
        {
            // Unexpected value!
            GT_ASSERT(false);
            breakpoint.setUnknownBreakpoint();
        }
        break;
    }
}

// ---------------------------------------------------------------------------
// Name:        vscBreakpointsManager::removeBreakpointFromVectors
// Description: Removes a breakpoint from the vectors, according to its request interface
// Author:      Uri Shomroni
// Date:        20/10/2010
// ---------------------------------------------------------------------------
void vscBreakpointsManager::removeBreakpointFromVectors(IDebugBreakpointRequest2* piBreakpointRequest)
{
    // Iterate the breakpoints, looking for one created from the same request:
    //int numberOfBreakpoints = (int)_breakpoints.size();
    //vspCDebugBreakpoint* pFoundBreakpoint = NULL;

    auto iter = std::find_if(_breakpoints.begin(), _breakpoints.end(), [piBreakpointRequest](vspCDebugBreakpoint * pBp)
    {
        return ((pBp != NULL) && (pBp->breakpointRequest() == piBreakpointRequest));
    });

    if (iter != _breakpoints.end())
    {
        // Now remove the breakpoint from the vector.

        // Release the breakpoint:
        (*iter)->Release();
        _breakpoints.erase(iter);
    }
}

// ---------------------------------------------------------------------------
// Name:        vscBreakpointsManager::onProcessTerminate
// Description: Is called when the process is being terminated
// Author:      Sigal Algranaty
// Date:        6/11/2011
// ---------------------------------------------------------------------------
void vscBreakpointsManager::onProcessTerminate()
{
    // Reset all the breakpoints' hit count:
    for (int i = 0; i < (int)_breakpoints.size() ; i++)
    {
        vspCDebugBreakpoint* pBreakpoint = _breakpoints[i];

        if (pBreakpoint != NULL)
        {
            pBreakpoint->SetHitCount(0);
        }
    }
}

void vscBreakpointsManager::setOwner(IVscBreakpointsManagerOwner* pOwner)
{
    m_pOwner = pOwner;
}

