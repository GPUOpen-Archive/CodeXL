//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gdFrameworkConnectionManager.cpp
///
//==================================================================================

//------------------------------ gdFrameworkConnectionManager.cpp ------------------------------

// Qt:
#include <QtWidgets>

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTBaseTools/Include/gtStringTokenizer.h>
#include <AMDTOSWrappers/Include/osCallStack.h>
#include <AMDTAPIClasses/Include/apBreakPoint.h>
#include <AMDTAPIClasses/Include/apKernelSourceCodeBreakpoint.h>
#include <AMDTAPIClasses/Include/apSourceCodeBreakpoint.h>
#include <AMDTAPIClasses/Include/apCLProgram.h>
#include <AMDTAPIClasses/Include/Events/apBreakpointsUpdatedEvent.h>
#include <AMDTAPIClasses/Include/Events/apEventsHandler.h>
#include <AMDTAPIClasses/Include/Events/apExceptionEvent.h>
#include <AMDTAPIClasses/Include/Events/apKernelSourceBreakpointsUpdatedEvent.h>
#include <AMDTApiFunctions/Include/gaGRApiFunctions.h>
#include <AMDTApplicationFramework/Include/afAppStringConstants.h>
#include <AMDTApplicationFramework/Include/afPluginConnectionManager.h>
#include <AMDTApplicationFramework/Include/afProjectManager.h>
#include <AMDTApiFunctions/Include/gaGRApiFunctions.h>

// Local:
#include <AMDTGpuDebuggingComponents/Include/gdAidFunctions.h>
#include <AMDTGpuDebuggingComponents/Include/gdFrameworkConnectionManager.h>


// ---------------------------------------------------------------------------
// Name:        gdFrameworkConnectionManager::gdFrameworkConnectionManager
// Description: Constructor
// Author:      Uri Shomroni
// Date:        13/5/2012
// ---------------------------------------------------------------------------
gdFrameworkConnectionManager::gdFrameworkConnectionManager()
{
    afPluginConnectionManager& thePluginConnectionManager = afPluginConnectionManager::instance();

    thePluginConnectionManager.registerBreakpointManager(this);
    thePluginConnectionManager.registerRunModeManager(this);
}

// ---------------------------------------------------------------------------
// Name:        gdFrameworkConnectionManager::~gdFrameworkConnectionManager
// Description: Destructor
// Author:      Uri Shomroni
// Date:        13/5/2012
// ---------------------------------------------------------------------------
gdFrameworkConnectionManager::~gdFrameworkConnectionManager()
{
    afPluginConnectionManager& thePluginConnectionManager = afPluginConnectionManager::instance();

    thePluginConnectionManager.unregisterBreakpointManager(this);
    thePluginConnectionManager.unregisterRunModeManager(this);
}

// ---------------------------------------------------------------------------
// Name:        gdFrameworkConnectionManager::getCurrentRunModeMask
// Description: Gets the modes shown by this plugin
// Author:      Uri Shomroni
// Date:        13/5/2012
// ---------------------------------------------------------------------------
afRunModes gdFrameworkConnectionManager::getCurrentRunModeMask()
{
    afRunModes retVal = 0;

    bool processExists = gaDebuggedProcessExists();
    bool processSuspended = processExists && gaIsDebuggedProcessSuspended();

    if (processExists)
    {
        retVal |= AF_DEBUGGED_PROCESS_EXISTS;
    }

    if (processExists && (!processSuspended))
    {
        retVal |= AF_DEBUGGED_PROCESS_RUNNING;
    }

    if (processExists && processSuspended)
    {
        retVal |= AF_DEBUGGED_PROCESS_SUSPENDED;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gdFrameworkConnectionManager::canStopCurrentRun
// Description: Can we stop the process?
// Author:      Uri Shomroni
// Date:        13/5/2012
// ---------------------------------------------------------------------------
bool gdFrameworkConnectionManager::canStopCurrentRun()
{
    bool retVal = gaDebuggedProcessExists();

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gdFrameworkConnectionManager::stopCurrentRun
// Description: Stop the process
// Return Val:  bool - Success / failure.
// Author:      Uri Shomroni
// Date:        13/5/2012
// ---------------------------------------------------------------------------
bool gdFrameworkConnectionManager::stopCurrentRun()
{
    bool retVal = gaTerminateDebuggedProcess();

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gdFrameworkConnectionManager::getExceptionEventDetails
// Description: Gets the details of an exception event:
// Return Val:  bool - Success / failure.
// Author:      Uri Shomroni
// Date:        13/5/2012
// ---------------------------------------------------------------------------
bool gdFrameworkConnectionManager::getExceptionEventDetails(const apExceptionEvent& exceptionEve, osCallStack& exceptionCallStack, bool& openCLEnglineLoaded, bool& openGLEnglineLoaded, bool& kernelDebuggingEnteredAtLeastOnce)
{
    // Get the call stack:
    osThreadId threadId = exceptionEve.triggeringThreadId();

    // Get the thread's call stack:
    bool rcStk = gaGetThreadCallStack(threadId, exceptionCallStack, false);
    GT_ASSERT(rcStk);

    // Get the additional Information:
    bool rcInfo = gaGetCrashReportAdditionalInformation(openCLEnglineLoaded, openGLEnglineLoaded, kernelDebuggingEnteredAtLeastOnce);
    GT_ASSERT(rcStk);

    bool retVal = rcStk && rcInfo;

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gdFrameworkConnectionManager::numberOfBreakpoints
// Description: Gets the number of breakpoints exported by this plugin
// Author:      Uri Shomroni
// Date:        13/5/2012
// ---------------------------------------------------------------------------
int gdFrameworkConnectionManager::numberOfBreakpoints()
{
    int retVal = 0;
    bool rcBP = gaGetAmountOfBreakpoints(retVal);
    GT_ASSERT(rcBP);

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gdFrameworkConnectionManager::getBreakpoint
// Description: Get the breakpoint at the given index
// Author:      Uri Shomroni
// Date:        13/5/2012
// ---------------------------------------------------------------------------
apBreakPoint* gdFrameworkConnectionManager::getBreakpoint(int breakpointIndex)
{
    apBreakPoint* retVal = NULL;

    gtAutoPtr<apBreakPoint> aptrBP;
    bool rcBP = gaGetBreakpoint(breakpointIndex, aptrBP);
    GT_IF_WITH_ASSERT(rcBP)
    {
        retVal = aptrBP.releasePointedObjectOwnership();
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gdFrameworkConnectionManager::isBreakpointSupported
// Description: Is a breakpoint supported by this plugin?
// Return Val:  bool - Success / failure.
// Author:      Uri Shomroni
// Date:        13/5/2012
// ---------------------------------------------------------------------------
bool gdFrameworkConnectionManager::isBreakpointSupported(const apBreakPoint& breakpoint)
{
    bool retVal = false;

    osTransferableObjectType breakpointType = breakpoint.type();

    switch (breakpointType)
    {
        case OS_TOBJ_ID_MONITORED_FUNC_BREAKPOINT:
        case OS_TOBJ_ID_KERNEL_SOURCE_CODE_BREAKPOINT:
        case OS_TOBJ_ID_SOURCE_CODE_BREAKPOINT:
        case OS_TOBJ_ID_HOST_SOURCE_CODE_BREAKPOINT:
        case OS_TOBJ_ID_KERNEL_FUNC_NAME_BREAKPOINT:
        case OS_TOBJ_ID_GENERIC_BREAKPOINT:
            retVal = true;
            break;

        default:
            retVal = false;
            break;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gdFrameworkConnectionManager::setBreakpoint
// Description: Sets a breakpoint and sends an update event
// Return Val:  bool - Success / failure.
// Author:      Uri Shomroni
// Date:        13/5/2012
// ---------------------------------------------------------------------------
bool gdFrameworkConnectionManager::setBreakpoint(const apBreakPoint& breakpoint)
{
    bool retVal = isBreakpointSupported(breakpoint);
    GT_IF_WITH_ASSERT(retVal)
    {
        // If this is a kernel source code breakpoint, try to bind the source to
        // an OpenCL program handle:
        if (breakpoint.type() == OS_TOBJ_ID_KERNEL_SOURCE_CODE_BREAKPOINT)
        {
            // Get the breakpoint as kernel source code breakpoint:
            apKernelSourceCodeBreakpoint& kernelSourceCodeBP = (apKernelSourceCodeBreakpoint&)breakpoint;

            // Find a program handle for this breakpoint:
            oaCLHandle programHandle = gdFindSourceCodeBreakpointMathingProgramHandle(kernelSourceCodeBP);

            if (programHandle != OA_CL_NULL_HANDLE)
            {
                kernelSourceCodeBP.resolveBreakpoint(programHandle);

                // Resolve the line number:
                int resolvedLineNumber = -1;
                bool rc = gaGetKernelSourceCodeBreakpointResolution(programHandle, kernelSourceCodeBP.lineNumber(), resolvedLineNumber);

                // NOTICE: This return value should not be asserted, since when the kernel is not yet debugged, the breakpoints cannot be resolved:
                if (rc)
                {
                    kernelSourceCodeBP.setLineNumber(resolvedLineNumber);
                }
            }
            else if (gaIsInHSAKernelBreakpoint())
            {
                // Get the current kernel path and name:
                osFilePath currentKernelPath;
                gtString currentKernelName;
                bool rcKrn = gaHSAGetSourceFilePath(currentKernelPath, currentKernelName);

                if (rcKrn && (kernelSourceCodeBP.unresolvedPath() == currentKernelPath))
                {
                    kernelSourceCodeBP.resolvedHSABreakpoint(currentKernelName);
                }
            }
        }

        // Is this a new breakpoint?
        int bpID = -1;
        bool rcExisted = gaGetBreakpointIndex(breakpoint, bpID);
        bool didBPExist = rcExisted && (-1 < bpID);

        retVal = gaSetBreakpoint(breakpoint);
        bool rcID = gaGetBreakpointIndex(breakpoint, bpID);

        // If this is a new breakpoint, or something went wrong, we want to update all breakpoints:
        bool shouldUpdateAll = (OS_TOBJ_ID_HOST_SOURCE_CODE_BREAKPOINT == breakpoint.type()) || (!didBPExist) || !rcID;

        apBreakpointsUpdatedEvent bkptUpdateEve(shouldUpdateAll ? -1 : bpID);
        apEventsHandler::instance().registerPendingDebugEvent(bkptUpdateEve);
    }

    GT_RETURN_WITH_ASSERT(retVal);
}

// ---------------------------------------------------------------------------
// Name:        gdFrameworkConnectionManager::removeBreakpoint
// Description: Removes a breakpoint and sends an update event
// Return Val:  bool - Success / failure.
// Author:      Uri Shomroni
// Date:        13/5/2012
// ---------------------------------------------------------------------------
bool gdFrameworkConnectionManager::removeBreakpoint(const apBreakPoint& breakpoint)
{
    bool retVal = isBreakpointSupported(breakpoint);
    GT_IF_WITH_ASSERT(retVal)
    {
        retVal = false;
        int bpID = -1;
        bool rcID = gaGetBreakpointIndex(breakpoint, bpID);
        GT_IF_WITH_ASSERT(rcID && (bpID > -1))
        {
            retVal = gaRemoveBreakpoint(bpID);

            // Removing a breakpoint should always update the entire list, since indices may shift:
            apBreakpointsUpdatedEvent bkptUpdateEve(-1);
            apEventsHandler::instance().registerPendingDebugEvent(bkptUpdateEve);
        }
    }

    GT_RETURN_WITH_ASSERT(retVal);
}

// ---------------------------------------------------------------------------
// Name:        gdFrameworkConnectionManager::breakpointTypeFromSourcePath
// Description: Gets the breakpoint type matching a source file:
// Author:      Uri Shomroni
// Date:        13/5/2012
// ---------------------------------------------------------------------------
osTransferableObjectType gdFrameworkConnectionManager::breakpointTypeFromSourcePath(const osFilePath& sourceFilePath)
{
    osTransferableObjectType retVal = OS_AMOUNT_OF_TRANSFERABLE_OBJECT_TYPES;

    gtString fileExt;
    sourceFilePath.getFileExtension(fileExt);
    static const gtString clFilesExtension = AF_STR_clSourceFileExtension;
    static const gtString hsailFilesExtension = AF_STR_hsailSourceFileExtension;

    if ((clFilesExtension == fileExt) || (hsailFilesExtension == fileExt))
    {
        retVal = OS_TOBJ_ID_KERNEL_SOURCE_CODE_BREAKPOINT;
    }
    else
    {
        if ((fileExt == gtString(L"cpp")) ||
            (fileExt == gtString(L"c")) ||
            (fileExt == gtString(L"h")) ||
            (fileExt == gtString(L"hxx")) ||
            (fileExt == gtString(L"cc")) ||
            (fileExt == gtString(L"cxx")) ||
            (fileExt == gtString(L"hpp")))
        {
            retVal = OS_TOBJ_ID_HOST_SOURCE_CODE_BREAKPOINT;
        }
        else
        {
            retVal = OS_TOBJ_ID_SOURCE_CODE_BREAKPOINT;
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gdFrameworkConnectionManager::doesBreakpointMatchFile
// Description: Does this breakpoint match this file?
// Return Val:  bool - Success / failure.
// Author:      Uri Shomroni
// Date:        13/5/2012
// ---------------------------------------------------------------------------
bool gdFrameworkConnectionManager::doesBreakpointMatchFile(const apBreakPoint& breakpoint, const osFilePath& sourceFilePath)
{
    bool retVal = false;

    osTransferableObjectType bpType = breakpoint.type();

    switch (bpType)
    {
        case OS_TOBJ_ID_SOURCE_CODE_BREAKPOINT:
        {
            apSourceCodeBreakpoint& sourceCodeBP = (apSourceCodeBreakpoint&)breakpoint;
            retVal = (sourceCodeBP.filePath() == sourceFilePath);
        }
        break;

        case OS_TOBJ_ID_KERNEL_SOURCE_CODE_BREAKPOINT:
        {
            // Get the breakpoint as kernel source code breakpoint:
            apKernelSourceCodeBreakpoint& kernelSourceCodeBP = (apKernelSourceCodeBreakpoint&)breakpoint;

            retVal = false;
            oaCLProgramHandle fileProgramHandle = OA_CL_NULL_HANDLE;
            osFilePath ignored;

            // If both file and BPs are bind to a CL program, compare handles:
            bool isSourceBindToProgram = gaGetOpenCLProgramHandleFromSourceFilePath(sourceFilePath, ignored, fileProgramHandle);

            if (isSourceBindToProgram && (OA_CL_NULL_HANDLE != fileProgramHandle) && (kernelSourceCodeBP.programHandle() != OA_CL_NULL_HANDLE))
            {
                // If the kernel handle is initialized, compare it, otherwise compare the file path:
                if (kernelSourceCodeBP.programHandle() != OA_CL_NULL_HANDLE)
                {
                    retVal = (kernelSourceCodeBP.programHandle() == fileProgramHandle);
                }
            }

            // Otherwise, compare file paths:
            else
            {
                // Check if the source code files folder is set:
                bool isSourceFilesFolderExist = false;

                if (!afProjectManager::instance().currentProjectSettings().SourceFilesDirectories().isEmpty())
                {
                    gtStringTokenizer sourceFoldersTokenizer(afProjectManager::instance().currentProjectSettings().SourceFilesDirectories(), L";");
                    gtString currentSourceDir;

                    while (sourceFoldersTokenizer.getNextToken(currentSourceDir) && !isSourceFilesFolderExist)
                    {
                        osDirectory dir(currentSourceDir);

                        if (dir.exists())
                        {
                            isSourceFilesFolderExist = kernelSourceCodeBP.unresolvedPath() == sourceFilePath;
                        }
                    }
                }

                if (isSourceFilesFolderExist)
                {
                    retVal = (kernelSourceCodeBP.unresolvedPath() == sourceFilePath);
                }
                else
                {
                    gtString fileName1, fileName2;
                    kernelSourceCodeBP.unresolvedPath().getFileNameAndExtension(fileName1);
                    sourceFilePath.getFileNameAndExtension(fileName2);
                    retVal = (fileName1 == fileName2);
                }
            }
        }
        break;

        case OS_TOBJ_ID_MONITORED_FUNC_BREAKPOINT:
        case OS_TOBJ_ID_KERNEL_FUNC_NAME_BREAKPOINT:
        case OS_TOBJ_ID_GENERIC_BREAKPOINT:
        {
            // These breakpoints match no specific file:
            retVal = false;
        }
        break;

        default:
        {
            // Unexpected breakpoint type:
            GT_ASSERT(false);
        }
        break;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gdFrameworkConnectionManager::bindProgramToBreakpoints
// Description: Bind / unbind a program or all context programs to existing breakpoints
// Arguments:   int contextId
//              int programIndex
//              bool unbind
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        19/7/2012
// ---------------------------------------------------------------------------
bool gdFrameworkConnectionManager::bindProgramToBreakpoints(int contextId, int programIndex, bool unbind)
{
    bool retVal = false;

    oaCLProgramHandle programHandle = OA_CL_NULL_HANDLE;
    osFilePath programFilePath;

    if ((contextId >= 0) && (programIndex >= 0))
    {
        // Get the associated program handle and file path:
        apCLProgram programDetails(0);
        bool rc = gaGetOpenCLProgramObjectDetails(contextId, programIndex, programDetails);
        GT_IF_WITH_ASSERT(rc)
        {
            programHandle = programDetails.programHandle();
            programFilePath = programDetails.sourceCodeFilePath();
        }
    }

    // Go through the existing kernel source code breakpoints:
    int amountOfBreakpoints = 0;
    bool rc = gaGetAmountOfBreakpoints(amountOfBreakpoints);

    if (rc)
    {
        for (int i = 0 ; i < amountOfBreakpoints ; i++)
        {
            // Update the real breakpoint:
            gtAutoPtr<apBreakPoint> aptrBreakpoint;
            bool rc2 = gaGetBreakpoint(i, aptrBreakpoint);
            GT_IF_WITH_ASSERT(rc2)
            {
                if (aptrBreakpoint->type() == OS_TOBJ_ID_KERNEL_SOURCE_CODE_BREAKPOINT)
                {
                    // Down cast to kernel source code breakpoint:
                    apKernelSourceCodeBreakpoint* pKernelSourceCodeBreakpoint = (apKernelSourceCodeBreakpoint*)aptrBreakpoint.pointedObject();
                    GT_IF_WITH_ASSERT(pKernelSourceCodeBreakpoint != NULL)
                    {
                        // Change the breakpoints details:
                        if (unbind && !pKernelSourceCodeBreakpoint->isUnresolved())
                        {
                            // Unbind specific:
                            bool shouldUnbindThis = ((programIndex >= 0) && (pKernelSourceCodeBreakpoint->programHandle() == programHandle));

                            shouldUnbindThis = shouldUnbindThis || (programIndex < 0);

                            if (shouldUnbindThis)
                            {
                                gaSetKernelBreakpointProgramHandle(i, OA_CL_NULL_HANDLE);
                            }
                        }

                        else if (!unbind)
                        {
                            // Bind the program:
                            if ((pKernelSourceCodeBreakpoint->unresolvedPath() == programFilePath) && pKernelSourceCodeBreakpoint->isUnresolved())
                            {
                                gaSetKernelBreakpointProgramHandle(i, programHandle);
                            }
                        }
                    }
                }
            }
        }
    }

    return retVal;
}

/// -----------------------------------------------------------------------------------------------
/// \brief Name:        onKernelSourceCodeUpdate
/// \brief Description: Update each of the kernel breakpoints with the program new line numbers
/// \param[in]          eve
/// \return True :
/// \return False:
/// -----------------------------------------------------------------------------------------------
bool gdFrameworkConnectionManager::onKernelSourceCodeUpdate(const apKernelSourceBreakpointsUpdatedEvent& eve)
{
    bool retVal = false;

    // Get the program handle:
    oaCLProgramHandle updatedProgramHandle = eve.debuggedProgramHandle();

    if (updatedProgramHandle != OA_CL_NULL_HANDLE)
    {
        retVal = true;

        // Go through all the kernel source code breakpoints for this file:
        int amountOfBreakpoints = -1;
        gtVector<int> breakpointsToRemove;
        bool rc = gaGetAmountOfBreakpoints(amountOfBreakpoints);
        GT_IF_WITH_ASSERT(rc)
        {
            for (int i = 0 ; i < amountOfBreakpoints ; i++)
            {
                // Get the breakpoint in the current index:
                gtAutoPtr<apBreakPoint> aptrBreakpoint;
                bool rc2 = gaGetBreakpoint(i, aptrBreakpoint);
                GT_IF_WITH_ASSERT(rc2)
                {
                    // Check if the breakpoint is a kernel source code breakpoint:
                    if (aptrBreakpoint->type() == OS_TOBJ_ID_KERNEL_SOURCE_CODE_BREAKPOINT)
                    {
                        // Down cast it to apKernelSourceCodeBreakpoint:
                        apKernelSourceCodeBreakpoint* pKernelSourceCodeBreakpoint = (apKernelSourceCodeBreakpoint*)(aptrBreakpoint.pointedObject());
                        GT_IF_WITH_ASSERT(pKernelSourceCodeBreakpoint != NULL)
                        {
                            if (pKernelSourceCodeBreakpoint->programHandle() == updatedProgramHandle)
                            {
                                // Get the line number (zero based):
                                int lineNumber = pKernelSourceCodeBreakpoint->lineNumber();
                                int newLineNumber = eve.getBreakpointBoundLineNumber(lineNumber);

                                // If the breakpoint line number should be updated:
                                if ((newLineNumber != lineNumber) && (newLineNumber >= 0))
                                {
                                    // This breakpoint should be removed:
                                    breakpointsToRemove.push_back(i);

                                    // Set the new breakpoint with the new line number:
                                    pKernelSourceCodeBreakpoint->setLineNumber(newLineNumber);
                                    rc = gaSetBreakpoint(*pKernelSourceCodeBreakpoint);
                                    retVal = retVal && rc;
                                }
                            }
                        }
                    }
                }
            }

            // Remove the old breakpoints:
            for (int i = breakpointsToRemove.size() - 1; i >= 0 ; i--)
            {
                rc = gaRemoveBreakpoint(breakpointsToRemove[i]);
                GT_ASSERT(rc);
            }

            if (breakpointsToRemove.size() > 0)
            {
                apBreakpointsUpdatedEvent eve1(-1);
                apEventsHandler::instance().registerPendingDebugEvent(eve1);
            }
        }
    }

    return retVal;
}


