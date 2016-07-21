//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file csAPIFunctionsImplementations.cpp
///
//==================================================================================

//------------------------------ csAPIFunctionsImplementations.cpp ------------------------------

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTBaseTools/Include/gtQueue.h>
#include <AMDTOSWrappers/Include/osDebuggingFunctions.h>
#include <AMDTOSWrappers/Include/osDebugLog.h>
#include <AMDTOSWrappers/Include/osThread.h>

#include <AMDTAPIClasses/Include/apCLBuffer.h>
#include <AMDTAPIClasses/Include/apCLCommandQueue.h>
#include <AMDTAPIClasses/Include/apCLContext.h>
#include <AMDTAPIClasses/Include/apCounterID.h>
#include <AMDTAPIClasses/Include/apCLDevice.h>
#include <AMDTAPIClasses/Include/apCLEnqueuedCommands.h>
#include <AMDTAPIClasses/Include/apCLKernel.h>
#include <AMDTAPIClasses/Include/apCLKernel.h>
#include <AMDTAPIClasses/Include/apCLObjectID.h>
#include <AMDTAPIClasses/Include/apCLPipe.h>
#include <AMDTAPIClasses/Include/apCLProgram.h>
#include <AMDTAPIClasses/Include/apCLImage.h>
#include <AMDTAPIClasses/Include/apExecutionMode.h>
#include <AMDTAPIClasses/Include/apExpression.h>

// Spies Utilities:
#include <AMDTServerUtilities/Include/suGlobalVariables.h>
#include <AMDTServerUtilities/Include/suBreakpointsManager.h>
#include <AMDTServerUtilities/Include/suIKernelDebuggingManager.h>

// Local:
#include <src/csAPIFunctionsImplementations.h>
#include <src/csCallsHistoryLogger.h>
#include <src/csGlobalVariables.h>
#include <src/csOpenCLMonitor.h>
#include <src/csStringConstants.h>


// ---------------------------------------------------------------------------
// Name:        gaGetAmountOfOpenCLFunctionCallsImpl
// Description: Implementation of gaGetAmountOfOpenCLFunctionCalls()
//   See its documentation for more details.
// Return Val: bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        12/11/2009
// ---------------------------------------------------------------------------
bool gaGetAmountOfOpenCLFunctionCallsImpl(int contextId, int& amountOfFunctionCalls)
{
    bool retVal = false;

    // Get the appropriate context monitor:
    const suContextMonitor* pContextMonitor = csOpenCLMonitor::instance().contextMonitor(contextId);

    if (pContextMonitor != NULL)
    {
        // Get its monitored functions calls logger:
        const suCallsHistoryLogger* pCallsHistoryLogger = pContextMonitor->callsHistoryLogger();
        GT_IF_WITH_ASSERT(pCallsHistoryLogger != NULL)
        {
            // Get the amount of function calls:
            amountOfFunctionCalls = pCallsHistoryLogger->amountOfFunctionCalls();

            retVal = true;
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gaGetCurrentFrameFunctionCallImpl
// Description: Implementation of gaGetOpenCLFunctionCall()
//   See its documentation for more details.
// Author:      Sigal Algranaty
// Date:        12/11/2009
// ---------------------------------------------------------------------------
bool gaGetOpenCLFunctionCallImpl(int contextId, int callIndex, gtAutoPtr<apFunctionCall>& aptrFunctionCall)
{
    bool retVal = false;

    // Get the appropriate context monitor:
    const suContextMonitor* pContextMonitor = csOpenCLMonitor::instance().contextMonitor(contextId);

    if (pContextMonitor != NULL)
    {
        // Get its monitored functions calls logger:
        const suCallsHistoryLogger* pCallsHistoryLogger = pContextMonitor->callsHistoryLogger();
        GT_IF_WITH_ASSERT(pCallsHistoryLogger != NULL)
        {
            // Get the amount of function calls:
            int amountOfFunctionCalls = pCallsHistoryLogger->amountOfFunctionCalls();

            // Verify that the queried call is in the right range:
            if ((0 <= callIndex) && (callIndex < amountOfFunctionCalls))
            {
                // Get the requested function call:
                retVal = pCallsHistoryLogger->getFunctionCall(callIndex, aptrFunctionCall);
            }
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gaGetLastOpenCLFunctionCallImpl
// Description: Implementation of gaGetLastOpenCLFunctionCallImpl()
//   See its documentation for more details.
// Arguments: gtAutoPtr<apFunctionCall>& aptrFunctionCall
// Return Val: void
// Author:      Sigal Algranaty
// Date:        12/11/2009
// ---------------------------------------------------------------------------
bool gaGetLastOpenCLFunctionCallImpl(int contextId, gtAutoPtr<apFunctionCall>& aptrFunctionCall)
{
    bool retVal = false;

    // Get the appropriate context monitor:
    const suContextMonitor* pContextMonitor = csOpenCLMonitor::instance().contextMonitor(contextId);

    if (pContextMonitor != NULL)
    {
        // Get its monitored functions calls logger:
        const suCallsHistoryLogger* pCallsHistoryLogger = pContextMonitor->callsHistoryLogger();
        GT_IF_WITH_ASSERT(pCallsHistoryLogger != NULL)
        {

            apExecutionMode currentExecMode = suDebuggedProcessExecutionMode();

            // If logging is enabled:
            if (pCallsHistoryLogger->isLoggingEnabled() && ((currentExecMode != AP_DEBUGGING_MODE)))
            {
                // Get the amount of function calls:
                int amountOfFuncCalls = pCallsHistoryLogger->amountOfFunctionCalls();

                // Get the last function call:
                retVal = pCallsHistoryLogger->getFunctionCall(amountOfFuncCalls - 1, aptrFunctionCall);
            }
            else
            {
                // Logging is disabled, we can only return the last called function id:
                apMonitoredFunctionId lastCalledFuncId =  pCallsHistoryLogger->lastCalledFunctionId();
                apFunctionCall* pLastFunctionCall = new apFunctionCall(lastCalledFuncId);
                aptrFunctionCall = pLastFunctionCall;
                retVal = true;

            }
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gaFindOpenCLFunctionCallImpl
// Description: Implementation of gaFindOpenCLFunctionCall()
//   See its documentation for more details.
// Arguments: apSearchDirection searchDirection
//            int searchStartIndex
//            const gtString& searchedString
//            bool isCaseSensitiveSearch
//            int& foundIndex
// Return Val: bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        15/11/2009
// ---------------------------------------------------------------------------
bool gaFindOpenCLFunctionCallImpl(int contextId, apSearchDirection searchDirection, int searchStartIndex, const gtString& searchedString, bool isCaseSensitiveSearch, int& foundIndex)
{
    bool retVal = false;

    // Get the appropriate context monitor:
    const suContextMonitor* pContextMonitor = csOpenCLMonitor::instance().contextMonitor(contextId);

    if (pContextMonitor != NULL)
    {
        // Get its monitored functions calls logger:
        const suCallsHistoryLogger* pCallsHistoryLogger = pContextMonitor->callsHistoryLogger();
        GT_IF_WITH_ASSERT(pCallsHistoryLogger != NULL)
        {
            // Get the amount of function calls:
            int amountOfFunctionCalls = pCallsHistoryLogger->amountOfFunctionCalls();

            // Verify that the search start index is in the right range:
            if ((0 <= searchStartIndex) && (searchStartIndex < amountOfFunctionCalls))
            {
                // Search the relevant context calls log:
                retVal = pCallsHistoryLogger->findFunctionCall(searchDirection, searchStartIndex, searchedString, isCaseSensitiveSearch, foundIndex);
            }
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaGetOpenCLHandleObjectDetailsImpl
// Description: Implementation of gaGetOpenCLHandleObjectDetails()
// Arguments: void* handle
//            const apCLObjectID* pCLOjbectIDDetails
// Return Val: bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        8/12/2009
// ---------------------------------------------------------------------------
bool gaGetOpenCLHandleObjectDetailsImpl(oaCLHandle handle, const apCLObjectID*& pCLOjbectIDDetails)
{
    bool retVal = false;

    // Get the OpenCL handle monitor:
    const csOpenCLHandleMonitor& handlesMonitor = csOpenCLMonitor::instance().openCLHandleMonitor();

    // Get the object details:
    pCLOjbectIDDetails = NULL;
    pCLOjbectIDDetails = handlesMonitor.getCLHandleObjectDetails(handle);

    if (pCLOjbectIDDetails != NULL)
    {
        retVal = true;
    }

    return retVal;
}



// ---------------------------------------------------------------------------
// Name:        gaGetAmountOfOpenCLContextsImpl
// Description: Implementation of gaGetAmountOfOpenCLContexts()
//   See its documentation for more details.
// Arguments: apSearchDirection searchDirection
//            int searchStartIndex
//            const gtString& searchedString
//            bool isCaseSensitiveSearch
//            int& foundIndex
// Return Val: bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        15/11/2009
// ---------------------------------------------------------------------------
bool gaGetAmountOfOpenCLContextsImpl(int& amountOfRenderContexts)
{
    // Get the amount of OpenCL contexts:
    csOpenCLMonitor& theOpenCLMonitor = csOpenCLMonitor::instance();
    amountOfRenderContexts = theOpenCLMonitor.amountOfContexts();

    return true;
}



// ---------------------------------------------------------------------------
// Name:        gaUpdateOpenCLContextDataSnapshotImpl
// Description: Implementation of gaUpdateOpenCLContextDataSnapshot()
//   See its documentation for more details.
// Arguments: int contextId
// Return Val: bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        24/11/2009
// ---------------------------------------------------------------------------
bool gaUpdateOpenCLContextDataSnapshotImpl(int contextId)
{
    bool retVal = false;

    // Get the OpenCL context monitor:
    csOpenCLMonitor& theOpenCLMonitor = csOpenCLMonitor::instance();

    suContextMonitor* pContextMonitor = theOpenCLMonitor.contextMonitor(contextId);
    GT_IF_WITH_ASSERT(pContextMonitor != NULL)
    {
        // Update the context data snapshot:
        retVal = pContextMonitor->updateContextDataSnapshot(true);
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaGetAmountOfOpenCLProgramObjectsImpl
// Description: Implementation of gaGetAmountOfOpenCLProgramObjects()
//   See its documentation for more details.
// Arguments: int contextId
//            int& amountOfPrograms
// Return Val: bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        17/11/2009
// ---------------------------------------------------------------------------
bool gaGetAmountOfOpenCLProgramObjectsImpl(int contextId, int& amountOfPrograms)
{
    bool retVal = false;

    // Get the appropriate render context monitor:
    csContextMonitor* pCLContextMonitor = csOpenCLMonitor::instance().clContextMonitor(contextId);
    GT_IF_WITH_ASSERT(pCLContextMonitor != NULL)
    {
        // Get its programs monitor:
        csProgramsAndKernelsMonitor& programsMonitor = pCLContextMonitor->programsAndKernelsMonitor();

        // Update context data snapshot, to see if any programs were deleted:
        bool rc = programsMonitor.updateContextDataSnapshot();
        GT_ASSERT(rc || pCLContextMonitor->contextInformation().wasMarkedForDeletion());

        // Get the program count:
        amountOfPrograms = programsMonitor.amountOfPrograms();
        retVal = true;
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gaGetOpenCLProgramObjectDetailsImpl
// Description: Implementation of gaGetOpenCLProgramObjectDetails()
//   See its documentation for more details.
// Arguments: int contextId
//            int programId
//            const apCLProgram*& pCLProgramDetails
// Return Val: bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        17/11/2009
// ---------------------------------------------------------------------------
bool gaGetOpenCLProgramObjectDetailsImpl(int contextId, int programId, const apCLProgram*& pCLProgramDetails)
{
    bool retVal = false;

    // Get the appropriate render context monitor:
    const csContextMonitor* pCLContextMonitor = csOpenCLMonitor::instance().clContextMonitor(contextId);
    GT_IF_WITH_ASSERT(pCLContextMonitor != NULL)
    {
        // Get its programs monitor:
        const csProgramsAndKernelsMonitor& programsMonitor = pCLContextMonitor->programsAndKernelsMonitor();
        const apCLProgram* pProgObject = programsMonitor.programMonitor(programId);

        if (pProgObject)
        {
            pCLProgramDetails = pProgObject;
            retVal = true;
        }
    }

    return retVal;
}
// ---------------------------------------------------------------------------
// Name:        gaSetOpenCLProgramCodeImpl
// Description: Implementation of gaSetOpenCLProgramCode
//   See its documentation for more details.
// Author:      Uri Shomroni
// Date:        18/1/2010
// ---------------------------------------------------------------------------
bool gaSetOpenCLProgramCodeImpl(oaCLProgramHandle programHandle, const osFilePath& newSourcePath)
{
    bool retVal = false;

    csContextMonitor* pContextMonitor = csOpenCLMonitor::instance().contextContainingProgram(programHandle);
    GT_IF_WITH_ASSERT(pContextMonitor != NULL)
    {
        retVal = pContextMonitor->programsAndKernelsMonitor().setProgramSourceCode(programHandle, newSourcePath);
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaBuildOpenCLProgramImpl
// Description: Implementation of gaBuildOpenCLProgram
//   See its documentation for more details.
// Arguments:   pFailedProgramData - will get failure data if the build fails
//              and we know why. It is the caller's responsibility to release
//              this if allocated.
// Author:      Uri Shomroni
// Date:        18/1/2010
// ---------------------------------------------------------------------------
bool gaBuildOpenCLProgramImpl(oaCLProgramHandle programHandle, apCLProgram*& pFailedProgramData)
{
    bool retVal = false;

    csContextMonitor* pContextMonitor = csOpenCLMonitor::instance().contextContainingProgram(programHandle);
    GT_IF_WITH_ASSERT(pContextMonitor != NULL)
    {
        retVal = pContextMonitor->programsAndKernelsMonitor().buildProgram(programHandle, pFailedProgramData);
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaGetOpenCLProgramHandleFromSourceFilePathImpl
// Description: Implementation of gaGetOpenCLProgramHandleFromSourceFilePath
//   See its documentation for more details.
// Author:      Uri Shomroni
// Date:        9/11/2010
// ---------------------------------------------------------------------------
bool gaGetOpenCLProgramHandleFromSourceFilePathImpl(const osFilePath& sourceFilePath, osFilePath& newTempSourceFilePath, oaCLProgramHandle& programHandle)
{
    bool retVal = false;

    programHandle = cs_stat_openCLMonitorInstance.programHandleFromSourcePath(sourceFilePath, newTempSourceFilePath);
    retVal = (programHandle != OA_CL_NULL_HANDLE);

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaGetOpenCLKernelObjectDetailsImpl
// Description: Implementation of gaGetOpenCLKernelObjectDetails()
//   See its documentation for more details.
// Return Val: bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        22/11/2009
// ---------------------------------------------------------------------------
bool gaGetOpenCLKernelObjectDetailsImpl(oaCLKernelHandle kernelHandle, const apCLKernel*& pCLKernelDetails)
{
    bool retVal = false;

    // Look for the context in which the input program was created:
    csOpenCLMonitor& theOpenCLMonitor = csOpenCLMonitor::instance();
    csContextMonitor* pContextMonitor = theOpenCLMonitor.contextContainingKernel(kernelHandle);
    GT_IF_WITH_ASSERT(pContextMonitor != NULL)
    {
        // Get the kernel monitor:
        csProgramsAndKernelsMonitor& kernelsMonitor = pContextMonitor->programsAndKernelsMonitor();
        const apCLKernel* pKernelMonitor = kernelsMonitor.kernelMonitor(kernelHandle);

        if (pKernelMonitor != NULL)
        {
            pCLKernelDetails = pKernelMonitor;
            retVal = true;
        }
        else // pKernelMonitor == NULL
        {
            // The kernel doesn't exist.
            if (osDebugLog::instance().loggedSeverity() >= OS_DEBUG_LOG_DEBUG)
            {
                // If we are in DEBUG log level, test if this kernel ever existed:
                const apCLObjectID* pKernelAsCLObject = theOpenCLMonitor.openCLHandleMonitor().getCLHandleObjectDetails(kernelHandle);

                if (pKernelAsCLObject == NULL)
                {
                    // The kernel never existed, print a log message:
                    OS_OUTPUT_DEBUG_LOG(L"Attempted to get details of non-existant kernel", OS_DEBUG_LOG_DEBUG)
                }
            }
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaGetKernelDebuggingLocationImpl
// Description: Implementation of gaGetKernelDebuggingLocation()
//   See its documentation for more details.
// Return Val:  bool - Success / failure.
// Author:      Uri Shomroni
// Date:        9/11/2010
// ---------------------------------------------------------------------------
bool gaGetKernelDebuggingLocationImpl(oaCLProgramHandle& debuggedProgramHandle, int& currentLineNumber)
{
    bool retVal = false;

    // Get the program containing the currently debugged kernel:
    debuggedProgramHandle = cs_stat_openCLMonitorInstance.programContainingKernel(cs_stat_pIKernelDebuggingManager->currentlyDebuggedKernel());
    retVal = (debuggedProgramHandle != OA_CL_NULL_HANDLE);

    // Get the line number:
    int coordinate[3] = { -1, -1, -1};
    cs_stat_pIKernelDebuggingManager->getFirstValidWorkItem(-1, coordinate);
    currentLineNumber = cs_stat_pIKernelDebuggingManager->getCurrentKernelDebugLineNumber(coordinate);

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaGetCurrentlyDebuggedKernelDetailsImpl
// Description: Implementation of gaGetCurrentlyDebuggedKernelDetails()
//   See its documentation for more details.
// Return Val:  bool - Success / failure.
// Author:      Uri Shomroni
// Date:        19/12/2010
// ---------------------------------------------------------------------------
bool gaGetCurrentlyDebuggedKernelDetailsImpl(const apCLKernel*& pKernelDetails)
{
    bool retVal = false;

    // If we are currently debugging any kernel:
    oaCLKernelHandle kernelHandle = cs_stat_pIKernelDebuggingManager->currentlyDebuggedKernel();

    if (kernelHandle != OA_CL_NULL_HANDLE)
    {
        // Get its containing context:
        csContextMonitor* pContextMonitor = cs_stat_openCLMonitorInstance.contextContainingKernel(kernelHandle);
        GT_IF_WITH_ASSERT(pContextMonitor != NULL)
        {
            // Get the kernel:
            csProgramsAndKernelsMonitor& programsAndKernelsMonitor = pContextMonitor->programsAndKernelsMonitor();
            programsAndKernelsMonitor.updateContextDataSnapshot();
            pKernelDetails = programsAndKernelsMonitor.kernelMonitor(kernelHandle);
            GT_IF_WITH_ASSERT(pKernelDetails != NULL)
            {
                // Mark success:
                retVal = true;
            }
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaGetCurrentlyDebuggedKernelCallStackImpl
// Description: Implementation of gaGetCurrentlyDebuggedKernelCallStack()
//   See its documentation for more details.
// Return Val:  bool - Success / failure.
// Author:      Uri Shomroni
// Date:        2/11/2010
// ---------------------------------------------------------------------------
bool gaGetCurrentlyDebuggedKernelCallStackImpl(const int coordinate[3], osCallStack& kernelStack)
{
    bool retVal = cs_stat_pIKernelDebuggingManager->getCurrentKernelCallStack(coordinate, kernelStack);

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaSetKernelDebuggingCommandImpl
// Description: Implementation of gaSetKernelDebuggingCommand()
//   See its documentation for more details.
// Return Val:  bool - Success / failure.
// Author:      Uri Shomroni
// Date:        2/11/2010
// ---------------------------------------------------------------------------
bool gaSetKernelDebuggingCommandImpl(apKernelDebuggingCommand command)
{
    bool retVal = true;

    cs_stat_pIKernelDebuggingManager->setNextKernelDebuggingCommand(command);

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaSetKernelSteppingWorkItemImpl
// Description: Implementation of gaSetKernelSteppingWorkItem()
//   See its documentation for more details.
// Return Val:  bool - Success / failure.
// Author:      Uri Shomroni
// Date:        16/2/2011
// ---------------------------------------------------------------------------
bool gaSetKernelSteppingWorkItemImpl(const int coordinate[3])
{
    bool retVal = cs_stat_pIKernelDebuggingManager->setSteppingWorkItem(coordinate);

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaIsWorkItemValidImpl
// Description: Implementation of gaIsWorkItemValid()
//   See its documentation for more details.
// Arguments: const int coordinate[3]
// Return Val:  bool - Success / failure.
// Author:      Uri Shomroni
// Date:        14/11/2011
// ---------------------------------------------------------------------------
bool gaIsWorkItemValidImpl(const int coordinate[3])
{
    bool retVal = cs_stat_pIKernelDebuggingManager->isWorkItemValid(coordinate);

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaGetFirstValidWorkItemImpl
// Description: Implementation of gaGetFirstValidWorkItem()
//   See its documentation for more details.
// Arguments: int coordinate[3]
// Return Val:  bool - Success / failure.
// Author:      Uri Shomroni
// Date:        14/11/2011
// ---------------------------------------------------------------------------
bool gaGetFirstValidWorkItemImpl(int wavefrontIndex, int coordinate[3])
{
    bool retVal = cs_stat_pIKernelDebuggingManager->getFirstValidWorkItem(wavefrontIndex, coordinate);

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaCanGetKernelVariableValueImpl
// Description: Implementation of gaCanGetKernelVariableValue()
//   See its documentation for more details.
// Return Val:  bool - Success / failure.
// Author:      Uri Shomroni
// Date:        25/1/2011
// ---------------------------------------------------------------------------
bool gaCanGetKernelVariableValueImpl(const gtString& variableName, const int coordinate[3])
{
    bool retVal = false;

    // If we are currently debugging any kernel:
    oaCLKernelHandle kernelHandle = cs_stat_pIKernelDebuggingManager->currentlyDebuggedKernel();

    if (kernelHandle != OA_CL_NULL_HANDLE)
    {
        retVal = suIKernelDebuggingManager::isPseudoVariable(variableName);

        if (!retVal)
        {
            retVal = cs_stat_pIKernelDebuggingManager->doesVariableExistInCurrentScope(variableName, coordinate);
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaGetKernelDebuggingExpressionValueImpl
// Description: Implementation of gaGetKernelDebuggingExpressionValue()
//   See its documentation for more details.
// Return Val:  bool - Success / failure.
// Author:      Uri Shomroni
// Date:        23/1/2011
// ---------------------------------------------------------------------------
bool gaGetKernelDebuggingExpressionValueImpl(const gtString& variableName, const int workItem[3], int evalDepth, apExpression& variableValue)
{
    bool retVal = false;

    // If we are currently debugging any kernel:
    oaCLKernelHandle kernelHandle = cs_stat_pIKernelDebuggingManager->currentlyDebuggedKernel();

    if (kernelHandle != OA_CL_NULL_HANDLE)
    {
        // Local struct used to evaluate values:
        struct csMemberEvaluationHelper
        {
        public:
            apExpression* m_pVariable;
            gtString m_qualifiedName;
            int m_depth;
        };

        variableValue.m_name = variableName;

        if (!suIKernelDebuggingManager::isPseudoVariable(variableName))
        {
            retVal = cs_stat_pIKernelDebuggingManager->getVariableValueString(variableName, workItem, variableValue.m_value, variableValue.m_valueHex, variableValue.m_type);

            // If we are getting children:
            if (retVal && (0 < evalDepth))
            {
                gtQueue<csMemberEvaluationHelper> evalQueue;
                csMemberEvaluationHelper rootEvalHelper = { &variableValue, variableName, 0 };
                evalQueue.push(rootEvalHelper);

                while (!evalQueue.empty())
                {
                    // Get the next member:
                    csMemberEvaluationHelper nextEval = evalQueue.front();
                    evalQueue.pop();

                    apExpression* pCurrentVar = nextEval.m_pVariable;
                    GT_IF_WITH_ASSERT(nullptr != pCurrentVar)
                    {
                        bool rcVal = cs_stat_pIKernelDebuggingManager->getVariableValueString(nextEval.m_qualifiedName, workItem, pCurrentVar->m_value, pCurrentVar->m_valueHex, pCurrentVar->m_type);
                        GT_ASSERT(rcVal);

                        // Only go down to the requested depth:
                        if (nextEval.m_depth < evalDepth)
                        {
                            gtVector<gtString> memberNames;
                            bool rcMem = cs_stat_pIKernelDebuggingManager->getVariableMembers(nextEval.m_qualifiedName, workItem, memberNames);
                            GT_ASSERT(rcMem);

                            for (const gtString& memNm : memberNames)
                            {
                                apExpression* pMember = pCurrentVar->addChild();
                                GT_IF_WITH_ASSERT(nullptr != pMember)
                                {
                                    gtString qualifiedName = nextEval.m_qualifiedName;
                                    qualifiedName.append('.').append(memNm);
                                    pMember->m_name = memNm;
                                    csMemberEvaluationHelper memEval = { pMember, qualifiedName, nextEval.m_depth + 1 };
                                    evalQueue.push(memEval);
                                }
                            }
                        }
                    }
                }
            }
        }
        else // suIKernelDebuggingManager::isPseudoVariable(variableName)
        {
            int globalWorkGeometry[10] = {0};
            retVal = cs_stat_pIKernelDebuggingManager->getGlobalWorkGeometry(globalWorkGeometry[0], &globalWorkGeometry[1], &globalWorkGeometry[4], &globalWorkGeometry[7]);

            if (retVal)
            {
                retVal = suIKernelDebuggingManager::getPseudoVariableValueString(variableName, workItem, globalWorkGeometry, variableValue.m_value, variableValue.m_valueHex, variableValue.m_type);

                if (retVal && (0 < evalDepth))
                {
                    gtQueue<csMemberEvaluationHelper> evalQueue;
                    csMemberEvaluationHelper rootEvalHelper = { &variableValue, variableName, 0 };
                    evalQueue.push(rootEvalHelper);

                    while (!evalQueue.empty())
                    {
                        // Get the next member:
                        csMemberEvaluationHelper nextEval = evalQueue.front();
                        evalQueue.pop();

                        apExpression* pCurrentVar = nextEval.m_pVariable;
                        GT_IF_WITH_ASSERT(nullptr != pCurrentVar)
                        {
                            bool rcVal = suIKernelDebuggingManager::getPseudoVariableValueString(nextEval.m_qualifiedName, workItem, globalWorkGeometry ,pCurrentVar->m_value, pCurrentVar->m_valueHex, pCurrentVar->m_type);
                            GT_ASSERT(rcVal);

                            // Only go down to the requested depth:
                            if (nextEval.m_depth < evalDepth)
                            {
                                gtVector<gtString> memberNames;
                                bool rcMem = suIKernelDebuggingManager::getPseudoVariableMembers(nextEval.m_qualifiedName, globalWorkGeometry, memberNames);
                                GT_ASSERT(rcMem);

                                for (const gtString& memNm : memberNames)
                                {
                                    apExpression* pMember = pCurrentVar->addChild();
                                    GT_IF_WITH_ASSERT(nullptr != pMember)
                                    {
                                        gtString qualifiedName = nextEval.m_qualifiedName;
                                        qualifiedName.append('.').append(memNm);
                                        pMember->m_name = memNm;
                                        csMemberEvaluationHelper memEval = { pMember, qualifiedName, nextEval.m_depth + 1 };
                                        evalQueue.push(memEval);
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
// Name:        gaGetKernelDebuggingAvailableVariablesImpl
// Description: Implementation of gaGetKernelDebuggingAvailableVariables
//   See its documentation for more details.
// Return Val:  bool - Success / failure.
// Author:      Uri Shomroni
// Date:        21/2/2011
// ---------------------------------------------------------------------------
bool gaGetKernelDebuggingAvailableVariablesImpl(const int coordinate[3], gtVector<apExpression>& variables, int evalDepth, bool getLeaves, int stackFrameDepth, bool onlyNames)
{
    bool retVal = false;

    // If we are currently debugging any kernel:
    oaCLKernelHandle kernelHandle = cs_stat_pIKernelDebuggingManager->currentlyDebuggedKernel();

    if (kernelHandle != OA_CL_NULL_HANDLE)
    {
        gtVector<gtString> variableNames;

        // Since these are pseudo-variables, they should not be reported in the list of actual variable leaf locations:
        if (!getLeaves)
        {
            // Put in the (always available) pseudo variables:
            variableNames.push_back(SU_PSEUDO_VAR_DISPATCH_DETAILS_NAME);
        }

        // Get the available variables:
        retVal = cs_stat_pIKernelDebuggingManager->getAvailableVariables(coordinate, variableNames, getLeaves, stackFrameDepth);

        // We know the output vector size:
        variables.reserve(variableNames.size());

        for (const gtString& varNm : variableNames)
        {
            apExpression expr;

            if (onlyNames)
            {
                expr.m_name = varNm;
            }
            else
            {
                bool rcVal = gaGetKernelDebuggingExpressionValueImpl(varNm, coordinate, getLeaves ? 0 : evalDepth, expr);
                GT_ASSERT(rcVal);
            }

            variables.push_back(expr);
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaGetKernelDebuggingAmountOfActiveWavefrontsImpl
// Description: Implementation of gaGetKernelDebuggingAmountOfActiveWavefronts
//   See its documentation for more details.
// Return Val:  bool - Success / failure.
// Author:      Uri Shomroni
// Date:        4/4/2013
// ---------------------------------------------------------------------------
bool gaGetKernelDebuggingAmountOfActiveWavefrontsImpl(int& amountOfWavefronts)
{
    bool retVal = false;

    // If we are currently debugging any kernel:
    oaCLKernelHandle kernelHandle = cs_stat_pIKernelDebuggingManager->currentlyDebuggedKernel();

    if (kernelHandle != OA_CL_NULL_HANDLE)
    {
        // Get the available variables:
        retVal = cs_stat_pIKernelDebuggingManager->getAmountOfActiveWavefronts(amountOfWavefronts);
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaGetKernelDebuggingActiveWavefrontIDImpl
// Description: Implementation of gaGetKernelDebuggingActiveWavefrontID
//   See its documentation for more details.
// Return Val:  bool - Success / failure.
// Author:      Uri Shomroni
// Date:        30/10/2013
// ---------------------------------------------------------------------------
bool gaGetKernelDebuggingActiveWavefrontIDImpl(int wavefrontIndex, int& wavefrontId)
{
    bool retVal = false;

    // If we are currently debugging any kernel:
    oaCLKernelHandle kernelHandle = cs_stat_pIKernelDebuggingManager->currentlyDebuggedKernel();

    if (kernelHandle != OA_CL_NULL_HANDLE)
    {
        // Get the available variables:
        retVal = cs_stat_pIKernelDebuggingManager->getActiveWavefrontID(wavefrontIndex, wavefrontId);
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaGetKernelDebuggingWavefrontIndexImpl
// Description: Implementation of gaGetKernelDebuggingWavefrontIndex
//   See its documentation for more details.
// Return Val:  bool - Success / failure.
// Author:      Uri Shomroni
// Date:        24/3/2013
// ---------------------------------------------------------------------------
bool gaGetKernelDebuggingWavefrontIndexImpl(const int coordinate[3], int& wavefrontIndex)
{
    bool retVal = false;

    // If we are currently debugging any kernel:
    oaCLKernelHandle kernelHandle = cs_stat_pIKernelDebuggingManager->currentlyDebuggedKernel();

    if (kernelHandle != OA_CL_NULL_HANDLE)
    {
        // Get the available variables:
        retVal = cs_stat_pIKernelDebuggingManager->getWavefrontIndex(coordinate, wavefrontIndex);
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaUpdateKernelVariableValueRawDataImpl
// Description: Implementation of gaUpdateKernelVariableValueRawData()
//   See its documentation for more details.
// Return Val:  bool - Success / failure.
// Author:      Uri Shomroni
// Date:        3/3/2011
// ---------------------------------------------------------------------------
bool gaUpdateKernelVariableValueRawDataImpl(const gtString& variableName, bool& variableTypeSupported, osFilePath& variableRawDataFilePath)
{
    bool retVal = false;

    // If we are currently debugging any kernel:
    oaCLKernelHandle kernelHandle = cs_stat_pIKernelDebuggingManager->currentlyDebuggedKernel();

    if (kernelHandle != OA_CL_NULL_HANDLE)
    {
        // Get the available variables:
        retVal = cs_stat_pIKernelDebuggingManager->exportVariableValuesToFile(variableName, variableTypeSupported, variableRawDataFilePath);
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaGetKernelSourceCodeBreakpointResolutionImpl
// Description: Implementation of gaGetKernelSourceCodeBreakpointResolution()
//   See its documentation for more details.
// Return Val:  bool - Success / failure.
// Author:      Uri Shomroni
// Date:        5/5/2011
// ---------------------------------------------------------------------------
bool gaGetKernelSourceCodeBreakpointResolutionImpl(oaCLProgramHandle programHandle, int requestedLineNumber, int& resolvedLineNumber)
{
    bool retVal = cs_stat_pIKernelDebuggingManager->getKernelSourceBreakpointResolution(programHandle, requestedLineNumber, resolvedLineNumber);

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gaSetKernelDebuggingEnableImpl
// Description: Implementation of gaSetKernelDebuggingEnable
// Arguments:   bool kernelEnabled
// Return Val:  bool - Success / failure.
// Author:      Gilad Yarnitzky
// Date:        6/11/2011
// ---------------------------------------------------------------------------
bool gaSetKernelDebuggingEnableImpl(bool kernelEnabled)
{
    bool retVal = cs_stat_pIKernelDebuggingManager->setKernelDebuggingEnableState(kernelEnabled);

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaSetMultipleKernelDebugDispatchModeImpl
// Description: Implementation of gaSetMultipleKernelDebugDispatchMode
//   See its documentation for more details.
// Author:      Uri Shomroni
// Date:        5/7/2015
// ---------------------------------------------------------------------------
bool gaSetMultipleKernelDebugDispatchModeImpl(apMultipleKernelDebuggingDispatchMode mode)
{
    bool retVal = cs_stat_pIKernelDebuggingManager->setMultipleKernelDebugDispatchMode(mode);

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaGetOpenCLDeviceObjectDetailsImpl
// Description: Implementation of gaGetOpenCLDeviceObjectDetails()
//   See its documentation for more details.
// Author:      Sigal Algranaty
// Date:        22/11/2009
// ---------------------------------------------------------------------------
bool gaGetOpenCLDeviceObjectDetailsImpl(int deviceId, const apCLDevice*& pCLDeviceDetails)
{
    bool retVal = false;

    // Look for the context in which the input program was created:
    csOpenCLMonitor& theOpenCLMonitor = csOpenCLMonitor::instance();
    csDevicesMonitor& devicesMonitor = theOpenCLMonitor.devicesMonitor();

    // Get the devices details:
    const apCLDevice* pDeviceObj = devicesMonitor.getDeviceObjectDetailsByIndex(deviceId);
    GT_IF_WITH_ASSERT(pDeviceObj != NULL)
    {
        pCLDeviceDetails = pDeviceObj;
        retVal = true;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaGetOpenCLPlatformAPIIDImpl
// Description: Implementation of gaGetOpenCLPlatformName()
//   See its documentation for more details.
// Author:      Sigal Algranaty
// Date:        11/4/2010
// ---------------------------------------------------------------------------
bool gaGetOpenCLPlatformAPIIDImpl(oaCLPlatformID platformId, int& platformName)
{
    bool retVal = false;

    // Get the devices monitor:
    csOpenCLMonitor& theOpenCLMonitor = csOpenCLMonitor::instance();
    csDevicesMonitor& devicesMonitor = theOpenCLMonitor.devicesMonitor();

    // Get the platform name:
    retVal = devicesMonitor.getPlatformAPIID(platformId, platformName);
    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaGetAmountOfOpenCLBufferObjectsImpl
// Description: Implementation of gaGetAmountOfOpenCLBufferObjects()
//   See its documentation for more details.
// Arguments: int contextId
//            int& amountOfBuffers
// Return Val: bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        19/11/2009
// ---------------------------------------------------------------------------
bool gaGetAmountOfOpenCLBufferObjectsImpl(int contextId, int& buffersAmount)
{
    bool retVal = false;

    // Get the appropriate render context monitor:
    const csContextMonitor* pCLContextMonitor = csOpenCLMonitor::instance().clContextMonitor(contextId);

    if (pCLContextMonitor)
    {
        // Get the buffers monitor:
        const csImagesAndBuffersMonitor& buffersMonitor = pCLContextMonitor->imagesAndBuffersMonitor();

        // Get amount of buffers:
        buffersAmount = buffersMonitor.amountOfBuffers();
        retVal = true;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaGetOpenCLBufferObjectDetailsImpl
// Description: Implementation of gaGetOpenCLBufferObjectDetails()
//   See its documentation for more details.
// Arguments: int contextId
//            int bufferId
//            const apCLBuffer*& pCLBufferDetails
// Return Val: bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        19/11/2009
// ---------------------------------------------------------------------------
bool gaGetOpenCLBufferObjectDetailsImpl(int contextId, int bufferId, const apCLBuffer*& pCLBufferDetails)
{
    bool retVal = false;

    // Get the appropriate render context monitor:
    const csContextMonitor* pCLContextMonitor = csOpenCLMonitor::instance().clContextMonitor(contextId);

    if (pCLContextMonitor)
    {
        // Get the buffers monitor:
        const csImagesAndBuffersMonitor& buffersMonitor = pCLContextMonitor->imagesAndBuffersMonitor();

        // Get the buffer:
        const apCLBuffer* pBuffObject = buffersMonitor.bufferDetails(bufferId);

        if (pBuffObject != NULL)
        {
            pCLBufferDetails = pBuffObject;
            retVal = true;
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaGetOpenCLSubBufferObjectDetailsImpl
// Description: Implementation of gaGetOpenCLSubBufferObjectDetails()
//   See its documentation for more details.
// Arguments:   int contextId
//              int bufferId
//              const apCLBuffer*& pCLSubBufferDetails
// Return Val:  bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        26/10/2010
// ---------------------------------------------------------------------------
bool gaGetOpenCLSubBufferObjectDetailsImpl(int contextId, int subBufferName, const apCLSubBuffer*& pCLSubBufferDetails)
{
    bool retVal = false;

    // Get the appropriate render context monitor:
    const csContextMonitor* pCLContextMonitor = csOpenCLMonitor::instance().clContextMonitor(contextId);

    if (pCLContextMonitor)
    {
        // Get the buffers monitor:
        const csImagesAndBuffersMonitor& buffersMonitor = pCLContextMonitor->imagesAndBuffersMonitor();

        // Get the sub buffer:
        const apCLSubBuffer* pSubBuffObject = buffersMonitor.subBufferDetails(subBufferName);

        if (pSubBuffObject != NULL)
        {
            pCLSubBufferDetails = pSubBuffObject;
            retVal = true;
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaUpdateOpenCLBufferRawDataImpl
// Description: Implementation of gaUpdateOpenCLBufferRawData()
//   See its documentation for more details.
// Arguments: gtVector<int> buffers
// Return Val: bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        2/12/2009
// ---------------------------------------------------------------------------
bool gaUpdateOpenCLBufferRawDataImpl(int contextId, const gtVector<int>& buffers)
{
    bool retVal = false;

    // Cannot update buffers during kernel debugging:
    if (OA_CL_NULL_HANDLE == cs_stat_pIKernelDebuggingManager->currentlyDebuggedKernel())
    {
        // Get the appropriate render context monitor:
        csContextMonitor* pCLContextMonitor = csOpenCLMonitor::instance().clContextMonitor(contextId);

        if (pCLContextMonitor)
        {
            // Get the buffers monitor:
            csImagesAndBuffersMonitor& buffersMonitor = pCLContextMonitor->imagesAndBuffersMonitor();

            retVal = true;

            int numberOfBuffers = (int)buffers.size();

            for (int i = 0; i < numberOfBuffers; i++)
            {
                // Get buffer id:
                int bufferId = buffers[i];

                // Update current buffer:
                bool rc = buffersMonitor.updateBufferRawData(bufferId);

                if (!rc)
                {
                    gtString message;
                    message.appendFormattedString(L"Failed to update buffer data. buffer id: %d", bufferId);
                    OS_OUTPUT_DEBUG_LOG(message.asCharArray(), OS_DEBUG_LOG_ERROR);
                }

                retVal = retVal && rc;
            }
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gaUpdateOpenCLSubBufferRawDataImpl
// Description: Implementation of gaUpdateOpenCLSubBufferRawData()
//   See its documentation for more details.
// Arguments:   gtVector<int> subBuffers
// Return Val:  bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        11/1/2011
// ---------------------------------------------------------------------------
bool gaUpdateOpenCLSubBufferRawDataImpl(int contextId, const gtVector<int>& subBuffers)
{
    bool retVal = false;

    // Cannot update buffers during kernel debugging:
    if (OA_CL_NULL_HANDLE == cs_stat_pIKernelDebuggingManager->currentlyDebuggedKernel())
    {
        // Get the appropriate render context monitor:
        csContextMonitor* pCLContextMonitor = csOpenCLMonitor::instance().clContextMonitor(contextId);

        if (pCLContextMonitor)
        {
            // Get the buffers monitor:
            csImagesAndBuffersMonitor& buffersMonitor = pCLContextMonitor->imagesAndBuffersMonitor();

            retVal = true;

            int numberOfSubBuffers = (int)subBuffers.size();

            for (int i = 0; i < numberOfSubBuffers; i++)
            {
                // Get sub-buffer id:
                int subBufferId = subBuffers[i];

                // Update current buffer:
                bool rc = buffersMonitor.updateSubBufferRawData(subBufferId);

                if (!rc)
                {
                    gtString message;
                    message.appendFormattedString(L"Failed to update sub buffer data. buffer id: %d", subBufferId);
                    OS_OUTPUT_DEBUG_LOG(message.asCharArray(), OS_DEBUG_LOG_ERROR);
                }

                retVal = retVal && rc;
            }
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaGetAmountOfOpenCLImageObjectsImpl
// Description: Implementation of gaGetAmountOfOpenCLImageObjects()
//   See its documentation for more details.
// Arguments: int contextId
//            int& amountOfTextures
// Return Val: bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        1/12/2009
// ---------------------------------------------------------------------------
bool gaGetAmountOfOpenCLImageObjectsImpl(int contextId, int& amountOfTextures)
{
    bool retVal = false;

    // Get the appropriate render context monitor:
    const csContextMonitor* pCLContextMonitor = csOpenCLMonitor::instance().clContextMonitor(contextId);

    if (pCLContextMonitor)
    {
        // Get the texture monitor:
        const csImagesAndBuffersMonitor& texturesMonitor = pCLContextMonitor->imagesAndBuffersMonitor();

        // Get amount of texture:
        amountOfTextures = texturesMonitor.amountOfImages();
        retVal = true;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaGetOpenCLImageObjectDetailsImpl
// Description: Implementation of gaGetOpenCLImageObjectDetails()
//   See its documentation for more details.
// Arguments: int contextId
//            int textureId
//            const apCLImage*& pCLTextureDetails
// Return Val: bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        1/12/2009
// ---------------------------------------------------------------------------
bool gaGetOpenCLImageObjectDetailsImpl(int contextId, int imageId, const apCLImage*& pCLImageDetails)
{
    bool retVal = false;

    // Get the appropriate render context monitor:
    const csContextMonitor* pCLContextMonitor = csOpenCLMonitor::instance().clContextMonitor(contextId);

    if (pCLContextMonitor)
    {
        // Get the textures monitor:
        const csImagesAndBuffersMonitor& imagesMonitor = pCLContextMonitor->imagesAndBuffersMonitor();

        // Get the buffer:
        const apCLImage* pImageObject = imagesMonitor.imageDetails(imageId);

        if (pImageObject != NULL)
        {
            pCLImageDetails = pImageObject;
            retVal = true;
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gaUpdateOpenCLImageRawDataImpl
// Description: Implementation of gaUpdateOpenCLImageRawDataImpl()
//   See its documentation for more details.
// Arguments: gtVector<int> images
// Return Val: bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        2/12/2009
// ---------------------------------------------------------------------------
bool gaUpdateOpenCLImageRawDataImpl(int contextId, const gtVector<int>& images)
{
    bool retVal = false;

    // Cannot update buffers during kernel debugging:
    if (OA_CL_NULL_HANDLE == cs_stat_pIKernelDebuggingManager->currentlyDebuggedKernel())
    {
        // Get the appropriate render context monitor:
        csContextMonitor* pCLContextMonitor = csOpenCLMonitor::instance().clContextMonitor(contextId);

        if (pCLContextMonitor)
        {
            // Get the buffers monitor:
            csImagesAndBuffersMonitor& imagesMonitor = pCLContextMonitor->imagesAndBuffersMonitor();

            retVal = true;

            int numberOfImages = (int)images.size();

            for (int i = 0; i < numberOfImages; i++)
            {
                // Get image id:
                int imageId = images[i];

                // Update current texture:
                bool rc = imagesMonitor.updateTextureRawData(imageId);

                if (!rc)
                {
                    gtString message;
                    message.appendFormattedString(L"Failed to update image data. image id: %d", imageId);
                    OS_OUTPUT_DEBUG_LOG(message.asCharArray(), OS_DEBUG_LOG_ERROR);
                }

                retVal = retVal && rc;
            }
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaGetAmountOfOpenCLPipeObjectsImpl
// Description: Implementation of gaGetAmountOfOpenCLPipeObjects()
//   See its documentation for more details.
// Return Val: bool  - Success / failure.
// Author:      Uri Shomroni
// Date:        1/10/2014
// ---------------------------------------------------------------------------
bool gaGetAmountOfOpenCLPipeObjectsImpl(int contextId, int& pipesAmount)
{
    bool retVal = false;

    // Get the appropriate render context monitor:
    const csContextMonitor* pCLContextMonitor = csOpenCLMonitor::instance().clContextMonitor(contextId);

    if (NULL != pCLContextMonitor)
    {
        // Get the buffers monitor:
        const csImagesAndBuffersMonitor& buffersMonitor = pCLContextMonitor->imagesAndBuffersMonitor();

        // Get amount of pipes:
        pipesAmount = buffersMonitor.amountOfPipes();
        retVal = true;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaGetOpenCLPipeObjectDetailsImpl
// Description: Implementation of gaGetOpenCLPipeObjectDetails()
//   See its documentation for more details.
// Return Val: bool  - Success / failure.
// Author:      Uri Shomroni
// Date:        1/10/2014
// ---------------------------------------------------------------------------
bool gaGetOpenCLPipeObjectDetailsImpl(int contextId, int pipeId, const apCLPipe*& pCLPipeDetails)
{
    bool retVal = false;

    // Get the appropriate render context monitor:
    const csContextMonitor* pCLContextMonitor = csOpenCLMonitor::instance().clContextMonitor(contextId);

    if (pCLContextMonitor)
    {
        // Get the buffers monitor:
        const csImagesAndBuffersMonitor& buffersMonitor = pCLContextMonitor->imagesAndBuffersMonitor();

        // Get the pipe:
        const apCLPipe* pPipeObject = buffersMonitor.pipeDetails(pipeId);

        if (pPipeObject != NULL)
        {
            pCLPipeDetails = pPipeObject;
            retVal = true;
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gaGetAmountOfCommandQueuesImpl
// Description: Implementation of gaGetAmountOfCommandQueues
//   See its documentation for more details.
// Return Val: bool  - Success / failure.
// Author:      Uri Shomroni
// Date:        2/12/2009
// ---------------------------------------------------------------------------
bool gaGetAmountOfCommandQueuesImpl(int contextId, int& amountOfQueues)
{
    bool retVal = false;

    const csContextMonitor* pContextMonitor = cs_stat_openCLMonitorInstance.clContextMonitor(contextId);
    GT_IF_WITH_ASSERT(pContextMonitor != NULL)
    {
        amountOfQueues = pContextMonitor->commandQueuesMonitor().amountOfQueues();

        retVal = true;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaGetCommandQueueDetailsImpl
// Description: Implementation of gaGetCommandQueueDetails
//   See its documentation for more details.
// Return Val: bool  - Success / failure.
// Author:      Uri Shomroni
// Date:        2/12/2009
// ---------------------------------------------------------------------------
bool gaGetCommandQueueDetailsImpl(int contextId, int queueIndex, const apCLCommandQueue*& pCLCommandQueue)
{
    bool retVal = false;

    const csContextMonitor* pContextMonitor = cs_stat_openCLMonitorInstance.clContextMonitor(contextId);
    GT_IF_WITH_ASSERT(pContextMonitor != NULL)
    {
        const csCommandQueueMonitor* pCommandQueueMtr = pContextMonitor->commandQueuesMonitor().commandQueueMonitor(queueIndex);
        GT_IF_WITH_ASSERT(pCommandQueueMtr != NULL)
        {
            pCLCommandQueue = &(pCommandQueueMtr->commandQueueInfo());

            retVal = true;
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaGetAmountOfCommandsInQueueImpl
// Description: Implementation of gaGetAmountOfCommandsInQueue
//   See its documentation for more details.
// Return Val: bool  - Success / failure.
// Author:      Uri Shomroni
// Date:        2/12/2009
// ---------------------------------------------------------------------------
bool gaGetAmountOfCommandsInQueueImpl(oaCLCommandQueueHandle queueHandle, int& amountOfCommands)
{
    bool retVal = false;

    const csCommandQueueMonitor* pCommandQueuetMonitor = cs_stat_openCLMonitorInstance.commandQueueMonitor(queueHandle);
    GT_IF_WITH_ASSERT(pCommandQueuetMonitor != NULL)
    {
        amountOfCommands = pCommandQueuetMonitor->amountOfCommandsInQueue();
        retVal = true;
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gaGetAmountOfEventsInQueueImpl
// Description: Implementation of gaGetAmountOfEventsInQueue
//   See its documentation for more details.
// Return Val: bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        17/2/2010
// ---------------------------------------------------------------------------
bool gaGetAmountOfEventsInQueueImpl(oaCLCommandQueueHandle queueHandle, int& amountOfCommands)
{
    bool retVal = false;

    const csCommandQueueMonitor* pCommandQueuetMonitor = cs_stat_openCLMonitorInstance.commandQueueMonitor(queueHandle);
    GT_IF_WITH_ASSERT(pCommandQueuetMonitor != NULL)
    {
        amountOfCommands = pCommandQueuetMonitor->amountOfEventsInQueue();
        retVal = true;
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gaGetEnqueuedCommandDetailsImpl
// Description: Implementation of gaGetEnqueuedCommandDetails
//   See its documentation for more details.
// Return Val: bool  - Success / failure.
// Author:      Uri Shomroni
// Date:        2/12/2009
// ---------------------------------------------------------------------------
bool gaGetEnqueuedCommandDetailsImpl(oaCLCommandQueueHandle queueHandle, int commandIndex, const apCLEnqueuedCommand*& pCommand)
{
    bool retVal = false;

    const csCommandQueueMonitor* pCommandQueuetMonitor = cs_stat_openCLMonitorInstance.commandQueueMonitor(queueHandle);
    GT_IF_WITH_ASSERT(pCommandQueuetMonitor != NULL)
    {
        const apCLEnqueuedCommand* pCommandDetails = pCommandQueuetMonitor->getEnqueuedCommandDetails(commandIndex);
        GT_IF_WITH_ASSERT(pCommandDetails != NULL)
        {
            pCommand = pCommandDetails;

            retVal = true;
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaGetAmountOfOpenCLSamplersImpl
// Description: Implementation of gaGetOpenCLSamplerObjectDetails
//   See its documentation for more details.
// Return Val:  bool - Success / failure.
// Author:      Uri Shomroni
// Date:        29/3/2010
// ---------------------------------------------------------------------------
bool gaGetAmountOfOpenCLSamplersImpl(int contextId, int& amountOfSamplers)
{
    bool retVal = false;

    const csContextMonitor* pContextMonitor = cs_stat_openCLMonitorInstance.clContextMonitor(contextId);
    GT_IF_WITH_ASSERT(pContextMonitor != NULL)
    {
        amountOfSamplers = pContextMonitor->samplersMonitor().amountOfSamplers();

        retVal = true;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaGetOpenCLSamplerObjectDetailsImpl
// Description: Implementation of gaGetOpenCLSamplerObjectDetails
//   See its documentation for more details.
// Return Val:  bool - Success / failure.
// Author:      Uri Shomroni
// Date:        29/3/2010
// ---------------------------------------------------------------------------
bool gaGetOpenCLSamplerObjectDetailsImpl(int contextId, int samplerIndex, const apCLSampler*& pCLSampler)
{
    bool retVal = false;

    const csContextMonitor* pContextMonitor = cs_stat_openCLMonitorInstance.clContextMonitor(contextId);
    GT_IF_WITH_ASSERT(pContextMonitor != NULL)
    {
        // Get the samplers monitor:
        const csSamplersMonitor& samplersMonitor = pContextMonitor->samplersMonitor();
        pCLSampler = samplersMonitor.getSamplerDetails(samplerIndex);
        retVal = true;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaGetAmountOfOpenCLEventsImpl
// Description: Implementation of gaGetOpenCLEventObjectDetails
//   See its documentation for more details.
// Return Val:  bool - Success / failure.
// Author:      Uri Shomroni
// Date:        22/8/2013
// ---------------------------------------------------------------------------
bool gaGetAmountOfOpenCLEventsImpl(int contextId, int& amountOfEvents)
{
    bool retVal = false;

    const csContextMonitor* pContextMonitor = cs_stat_openCLMonitorInstance.clContextMonitor(contextId);
    GT_IF_WITH_ASSERT(pContextMonitor != NULL)
    {
        amountOfEvents = pContextMonitor->eventsMonitor().amountOfEvents();

        retVal = true;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaGetOpenCLEventObjectDetailsImpl
// Description: Implementation of gaGetOpenCLEventObjectDetails
//   See its documentation for more details.
// Return Val:  bool - Success / failure.
// Author:      Uri Shomroni
// Date:        22/8/2013
// ---------------------------------------------------------------------------
bool gaGetOpenCLEventObjectDetailsImpl(int contextId, int eventIndex, const apCLEvent*& pCLEvent)
{
    bool retVal = false;

    const csContextMonitor* pContextMonitor = cs_stat_openCLMonitorInstance.clContextMonitor(contextId);
    GT_IF_WITH_ASSERT(pContextMonitor != NULL)
    {
        // Get the events monitor:
        pCLEvent = pContextMonitor->eventsMonitor().eventDetailsByIndex(eventIndex);
        retVal = (NULL != pCLEvent);
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaGetOpenCLContextDetailsImpl
// Description: Implementation of gaGetOpenCLContextDetails.
//              See its documentation for more details.
// Return Val: bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        7/11/2009
// ---------------------------------------------------------------------------
bool gaGetOpenCLContextDetailsImpl(int contextId, const apCLContext*& pContextInfo)
{
    bool retVal = false;

    // Get the queried context monitor:
    csContextMonitor* pContextMonitor = csOpenCLMonitor::instance().clContextMonitor(contextId);

    if (pContextMonitor != NULL)
    {
        pContextInfo = &pContextMonitor->contextInformation();
        retVal = true;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaSetCLBufferDisplayPropertiesImpl
// Description: Implementation of gaSetCLBufferDisplayProperties
// Arguments: int contextId
//            int bufferId
//            oaTexelDataFormat bufferDisplayFormat
//            int offset
//            gtSize_t stride
// Return Val: bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        8/12/2009
// ---------------------------------------------------------------------------
bool gaSetCLBufferDisplayPropertiesImpl(int contextId, int bufferId, oaTexelDataFormat bufferDisplayFormat, int offset, gtSize_t stride)
{
    bool retVal = false;

    // Get the input render context monitor:
    csContextMonitor* pContextMonitor = cs_stat_openCLMonitorInstance.clContextMonitor(contextId);
    GT_IF_WITH_ASSERT(pContextMonitor != NULL)
    {
        // Get the buffers monitor:
        csImagesAndBuffersMonitor& buffersMonitor = pContextMonitor->imagesAndBuffersMonitor();

        apCLBuffer* pBuffer = buffersMonitor.bufferDetails(bufferId);
        GT_IF_WITH_ASSERT(pBuffer != NULL)
        {
            pBuffer->setBufferDisplayProperties(bufferDisplayFormat, offset, stride);
            retVal = true;
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gaSetCLSubBufferDisplayPropertiesImpl
// Description: Implementation of gaSetCLBufferDisplayProperties
// Arguments:   int contextId
//              int subBufferId
//              oaTexelDataFormat bufferDisplayFormat
//              int offset
//              gtSize_t stride
// Return Val:  bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        11/1/2011
// ---------------------------------------------------------------------------
bool gaSetCLSubBufferDisplayPropertiesImpl(int contextId, int subBufferId, oaTexelDataFormat bufferDisplayFormat, int offset, gtSize_t stride)
{
    bool retVal = false;

    // Get the input render context monitor:
    csContextMonitor* pContextMonitor = cs_stat_openCLMonitorInstance.clContextMonitor(contextId);
    GT_IF_WITH_ASSERT(pContextMonitor != NULL)
    {
        // Get the buffers monitor:
        csImagesAndBuffersMonitor& buffersMonitor = pContextMonitor->imagesAndBuffersMonitor();

        apCLSubBuffer* pSubBuffer = buffersMonitor.subBufferDetails(subBufferId);
        GT_IF_WITH_ASSERT(pSubBuffer != NULL)
        {
            pSubBuffer->setSubBufferDisplayProperties(bufferDisplayFormat, offset, stride);
            retVal = true;
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaGetAMDOpenCLPerformanceCountersValuesImpl
// Description: Implementation of gaGetAMDOpenCLPerformanceCountersValues.
//              See its documentation for more details. Supported on windows only.
// Author:      Sigal Algranaty
// Date:        25/2/2010
// ---------------------------------------------------------------------------
bool gaGetAMDOpenCLPerformanceCountersValuesImpl(const double*& pValuesArray, int& amountOfValues)
{
    bool retVal = true;

    pValuesArray = NULL;
    amountOfValues = 0;

#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
#ifdef OA_DEBUGGER_USE_AMD_GPA
    {
        // Get the updated values:
        pValuesArray = csOpenCLMonitor::instance().AMDPerformanceCountersManager().getCounterValues();

        // Get the updated total size:
        amountOfValues = csOpenCLMonitor::instance().AMDPerformanceCountersManager().getCountersAmount();
    }
#endif
#endif

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gaActivateAMDOpenCLPerformanceCountersImpl
// Description: Implementation of gaActivateAMDOpenCLPerformanceCounters.
//              See its documentation for more details. Supported on windows only.
// Author:      Sigal Algranaty
// Date:        2/03/2010
// ---------------------------------------------------------------------------
bool gaActivateAMDOpenCLPerformanceCountersImpl(const gtVector<apCounterActivationInfo>& countersActivationInfosVec)
{
    bool retVal = false;

    (void)(countersActivationInfosVec); // unused
#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
#ifdef OA_DEBUGGER_USE_AMD_GPA
    {
        // Update the AMD performance counters values:
        retVal = csOpenCLMonitor::instance().AMDPerformanceCountersManager().activateCounters(countersActivationInfosVec);
    }
#endif
#endif

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gaGetOpenCLQueuePerformanceCountersValuesImpl
// Description: Implementation of gaGetOpenCLQueuePerformanceCountersValues.
//              See its documentation for more details. Supported on windows only.
// Author:      Sigal Algranaty
// Date:        8/3/2010
// ---------------------------------------------------------------------------
bool gaGetOpenCLQueuePerformanceCountersValuesImpl(const double*& pValuesArray, int& amountOfValues)
{
    bool retVal = true;

    pValuesArray = NULL;
    amountOfValues = 0;

    // Update the counter values:
    csOpenCLQueuesPerformanceCountersManager& countersMgr = csOpenCLMonitor::instance().openCLQueueCountersManager();
    bool rc = countersMgr.updateCounterValues();
    GT_IF_WITH_ASSERT(rc)
    {
        // Get the updated values:
        pValuesArray = countersMgr.getCounterValues();

        // Get the updated total size:
        amountOfValues = countersMgr.countersAmount();
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gaGetCLContextLogFilePathImpl
// Description:
//   Implementation of gaGetCLContextLogFilePath().
//   See its documentation for more details.
// Author:      Sigal Algranaty
// Date:        23/3/2010
// ---------------------------------------------------------------------------
bool gaGetCLContextLogFilePathImpl(int contextId, bool& logFileExists, osFilePath& filePath)
{
    bool retVal = true;

    // Get the HTML file path from the OpenCL monitor:
    retVal = csOpenCLMonitor::instance().getHTMLLogFilePath(contextId, logFileExists, filePath);

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gaGetCurrentOpenCLStatisticsImpl
// Description: Implementation of gaGetCurrentOpenCLStatistics.
//              See its documentation for more details.
// Author:      Sigal Algranaty
// Date:        25/3/2010
// ---------------------------------------------------------------------------
bool gaGetCurrentOpenCLStatisticsImpl(int contextId, apStatistics* pStatistics)
{
    bool retVal = false;

    // Get the appropriate render context monitor:
    const suContextMonitor* pRenderContextMon = csOpenCLMonitor::instance().contextMonitor(contextId);

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
// Name:        gaClearOpenCLFunctionCallsStatisticsImpl
// Description: Implementation of gaClearOpenCLFunctionCallsStatistics.
//              See its documentation for more details.
// Return Val:  bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        28/3/2010
// ---------------------------------------------------------------------------
bool gaClearOpenCLFunctionCallsStatisticsImpl()
{
    bool retVal = true;

    // Get the OpenCL monitor:
    csOpenCLMonitor& theMonitor = csOpenCLMonitor::instance();

    // Get amount of contexts:
    int amountOfContexts = theMonitor.amountOfContexts();

    // For each context - clear statistics:
    for (int contextId = 0; contextId < amountOfContexts; contextId++)
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

        retVal = retVal && rcClearFunctionCalls;
    }

    return retVal;
}



// ---------------------------------------------------------------------------
// Name:        gaSetOpenCLOperationExecutionImpl
// Description: Implementation of gaSetOpenCLOperationExecution.
//              See its documentation for more details.
// Arguments:   apOpenCLExecutionType executionType
//            bool isExecutionOn
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        9/5/2010
// ---------------------------------------------------------------------------
bool gaSetOpenCLOperationExecutionImpl(apOpenCLExecutionType executionType, bool isExecutionOn)
{
    bool retVal = true;

    csOpenCLMonitor::instance().setOpenCLOperationExecution(executionType, isExecutionOn);

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gaSetKernelSourceFilePathImpl
// Description:
//   Implementation of gaSetKernelSourceFilePath. See its documentation for more details.
// Author:      Gilad Yarnitzky
// Date:        20/4/2011
// ---------------------------------------------------------------------------
bool gaSetKernelSourceFilePathImpl(gtVector<osFilePath>& programsFilePath)
{
    bool retVal = true;

    retVal = csOpenCLMonitor::instance().setKernelSourceFilePath(programsFilePath);

    return retVal;
}

