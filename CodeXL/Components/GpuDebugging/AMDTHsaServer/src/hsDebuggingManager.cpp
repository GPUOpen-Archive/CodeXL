//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file hsDebuggingManager.cpp
///
//==================================================================================


// HSA and HSA debugging:
#include <hsa.h>
#include <hsa_api_trace.h>
#include <hsa_ext_amd.h>
#include <hsa_ext_finalize.h>
#include <amd_hsa_kernel_code.h>
#include <amd_hsa_tools_interfaces.h>
#include <AMDGPUDebug.h>
#include <DbgInfoDwarfParser.h>

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTBaseTools/Include/gtSet.h>
#include <AMDTOSWrappers/Include/osCriticalSectionLocker.h>
#include <AMDTOSWrappers/Include/osDebugLog.h>
#include <AMDTOSWrappers/Include/osProcess.h>
#include <AMDTOSWrappers/Include/osSocket.h>
#include <AMDTOSWrappers/Include/osStringConstants.h>
#include <AMDTOSWrappers/Include/osThread.h>
#include <AMDTAPIClasses/Include/Events/apKernelDebuggingFailedEvent.h>
#include <AMDTServerUtilities/Include/suBreakpointsManager.h>
#include <AMDTServerUtilities/Include/suSpyAPIFunctions.h>

// Local:
#include <src/hsDebuggingManager.h>
#include <src/hsDebugInfo.h>
#include <src/hsStringConstants.h>

// Static member initializations:
hsDebuggingManager* hsDebuggingManager::ms_pMySingleInstance = nullptr;

////////////////////////////
// Interception functions //
////////////////////////////
#define HS_AMD_ISA_END_PROGRAM_INSTRUCTION 0xbf810000
#define HS_AMD_ISA_INST_NOP 0xbf800000
#define HS_AMD_ISA_INST_TRAP_BASE 0xbf920000
#define HS_AMD_ISA_INST_TRAP_MASK 0xffff0000
static inline bool hsCompareBinaries(const gtUInt32* pBin1, const gtUInt32* pBin2, size_t binSize)
{
    bool retVal = ((nullptr != pBin1) & (nullptr != pBin2));

    if (retVal)
    {
        for (size_t i = 0; binSize > i; ++i)
        {
            gtUInt32 in1 = pBin1[i];
            gtUInt32 in2 = pBin2[i];

            // Allow:
            // * Same instruction
            // * NOP vs. trap
            if ((in1 != in2) &&
                ((HS_AMD_ISA_INST_NOP != in1) || (HS_AMD_ISA_INST_TRAP_BASE != (HS_AMD_ISA_INST_TRAP_MASK & in2))) &&
                ((HS_AMD_ISA_INST_NOP != in2) || (HS_AMD_ISA_INST_TRAP_BASE != (HS_AMD_ISA_INST_TRAP_MASK & in1))))
            {
                retVal = false;
                break;
            }
            else if (HS_AMD_ISA_END_PROGRAM_INSTRUCTION == in1)
            {
                break;
            }
        }
    }

    return retVal;
}
static void hsKenrelNameFromDebugContext(HwDbgContextHandle hDbgCtx, gtString& kernelName)
{
    if (nullptr != hDbgCtx)
    {
        const char* pKernelName = nullptr;
        HwDbgStatus rcNm = HwDbgGetDispatchedKernelName(hDbgCtx, &pKernelName);

        if ((HWDBG_STATUS_SUCCESS == rcNm) && (nullptr != pKernelName))
        {
            kernelName.fromASCIIString(pKernelName);
        }
    }
}
static void hsKernelNameFromBinaryAndPacket(const void* pBinary, size_t binarySize, hsa_kernel_dispatch_packet_t* pAqlPacket, gtString& kernelName)
{
    // Empty the output parameter:
    kernelName.makeEmpty();

    // Get the ISA from the AQL Packet:
    amd_kernel_code_t* pKernelCode = (amd_kernel_code_t*)(pAqlPacket->kernel_object);
    GT_IF_WITH_ASSERT(nullptr != pKernelCode)
    {
        const gtUByte* pAQLISA = (const gtUByte*)((size_t)pKernelCode + (size_t)pKernelCode->kernel_code_entry_byte_offset);

        // Get ISAs from the ELF binary and compare them against the AQL binary:
        gtString firstKernelName;

        HwDbg::KernelBinary kernelBin(const_cast<void*>(pBinary), binarySize);
        std::vector<std::string> elfSyms;
        kernelBin.listELFSymbolNames(elfSyms);
        HwDbg::KernelBinary currentSymbolBin(nullptr, 0);
        gtString currentKernelName;

        osDebugLogSeverity sev = osDebugLog::instance().loggedSeverity();

        for (const std::string& s : elfSyms)
        {
            if (OS_DEBUG_LOG_EXTENSIVE <= sev)
            {
                gtString extnMsg;
                extnMsg.fromASCIIString(s.c_str()).prepend(HS_STR_debugLogCheckingELFSym);
                OS_OUTPUT_DEBUG_LOG(extnMsg.asCharArray(), OS_DEBUG_LOG_EXTENSIVE);
            }

            if ((0 < s.size()) && ('&' == s[0]))
            {
                // Consider main function names as well as the kernel name:
                int offset = ((4 < s.size()) && ('m' == s[1]) && (':' == s[2]) && (':' == s[3]) && ('&' == s[4])) ? 4 : 0;
                currentKernelName.fromASCIIString(&(s.c_str()[offset]));

                if (OS_DEBUG_LOG_DEBUG <= sev)
                {
                    gtString dbgMsg = HS_STR_debugLogComparingISAAndAQL;
                    dbgMsg.append(currentKernelName);
                    OS_OUTPUT_DEBUG_LOG(dbgMsg.asCharArray(), OS_DEBUG_LOG_DEBUG);
                }

                // Get the symbol data:
                bool rcSym = kernelBin.getElfSymbolAsBinary(s, currentSymbolBin);
                GT_IF_WITH_ASSERT(rcSym)
                {
                    GT_IF_WITH_ASSERT(sizeof(amd_kernel_code_t) < currentSymbolBin.m_binarySize)
                    {
                        // Skip the kernel header:
                        const void* pELFISA = (const void*)((size_t)currentSymbolBin.m_pBinaryData + sizeof(amd_kernel_code_t));
                        size_t elfISASize = currentSymbolBin.m_binarySize - sizeof(amd_kernel_code_t);
                        bool rcCmp = hsCompareBinaries((const gtUInt32*)pAQLISA, (const gtUInt32*)pELFISA, elfISASize);

                        if (rcCmp)
                        {
                            OS_OUTPUT_DEBUG_LOG(HS_STR_debugLogFoundISAMatch, OS_DEBUG_LOG_DEBUG);
                            kernelName = currentKernelName;
                            break;
                        }
                    }
                }

                if (firstKernelName.isEmpty())
                {
                    firstKernelName = currentKernelName;
                }
            }
        }

        if (kernelName.isEmpty())
        {
            if (OS_DEBUG_LOG_DEBUG <= sev)
            {
                gtString dbgMsg = HS_STR_debugLogUsingFirstKernel;
                dbgMsg.append(firstKernelName);
                OS_OUTPUT_DEBUG_LOG(dbgMsg.asCharArray(), OS_DEBUG_LOG_DEBUG);
            }

            kernelName = firstKernelName;
        }
    }
}

static void hsPreDispatchCallback(const hsa_dispatch_callback_t* pRTParam, void* pUserArgs)
{
    OS_OUTPUT_DEBUG_LOG(HS_STR_debugLogEnteredPredispatch, OS_DEBUG_LOG_DEBUG);

    GT_UNREFERENCED_PARAMETER(pUserArgs);
    hsDebuggingManager& theDebuggingMgr = hsDebuggingManager::instance();

    if (!theDebuggingMgr.IsDebuggingInProgress())
    {
        // This is needed if a previous instance / application run ended prematurely:
        HwDbgStatus rcClean = HwDbgEndDebugContext(nullptr);
        GT_ASSERT(HWDBG_STATUS_SUCCESS == rcClean);

        // Initialize the debugger:
        hsa_kernel_dispatch_packet_t* pAqlPacket = pRTParam->aql_packet;
        HwDbgState dbeState = { 0, 0, 0, 0 };
        dbeState.pDevice = (void*)pRTParam->agent.handle;
        dbeState.pPacket = pAqlPacket;
        HwDbgContextHandle hDbgCtx = nullptr;
        HwDbgStatus rcDBE = HwDbgBeginDebugContext(dbeState, &hDbgCtx);
        GT_IF_WITH_ASSERT(HWDBG_STATUS_SUCCESS == rcDBE)
        {
            gtString kernelName;
            hsKenrelNameFromDebugContext(hDbgCtx, kernelName);

            if (kernelName.isEmpty())
            {
                const void* pBinary = nullptr;
                size_t binarySize = 0;
                rcDBE = HwDbgGetKernelBinary(hDbgCtx, &pBinary, &binarySize);
                GT_IF_WITH_ASSERT(HWDBG_STATUS_SUCCESS == rcDBE)
                {
                    hsKernelNameFromBinaryAndPacket(pBinary, binarySize, pAqlPacket, kernelName);
                }
            }

            if (!kernelName.isEmpty())
            {
                // Ask the user whether they want to debug this kernel:
                bool shouldDebug = theDebuggingMgr.ShouldDebugKernel(kernelName);

                if (shouldDebug)
                {
                    gtString logMsg = HS_STR_debugLogStartingDebugging;
                    logMsg.append(kernelName);
                    OS_OUTPUT_DEBUG_LOG(logMsg.asCharArray(), OS_DEBUG_LOG_DEBUG);
                    gtUInt32 gs[3] = { pAqlPacket->grid_size_x, pAqlPacket->grid_size_y, pAqlPacket->grid_size_z };
                    gtUInt32 wgs[3] = { pAqlPacket->workgroup_size_x, pAqlPacket->workgroup_size_y, pAqlPacket->workgroup_size_z };
                    bool rcDebug = theDebuggingMgr.StartDebugging((void*)hDbgCtx, kernelName, pAqlPacket->kernarg_address, gs, wgs);
                    GT_ASSERT(rcDebug);
                }
                else
                {
                    gtString logMsg = HS_STR_debugLogNotStartingDebugging;
                    logMsg.append(kernelName);
                    OS_OUTPUT_DEBUG_LOG(logMsg.asCharArray(), OS_DEBUG_LOG_EXTENSIVE);
                    rcDBE = HwDbgEndDebugContext(hDbgCtx);
                    GT_ASSERT(HWDBG_STATUS_SUCCESS == rcDBE);
                }
            }
        }
    }

    OS_OUTPUT_DEBUG_LOG(HS_STR_debugLogExitedPredispatch, OS_DEBUG_LOG_DEBUG);
}

static void hsPostDispatchCallback(const hsa_dispatch_callback_t* pRTParam, void* pUserArgs)
{
    OS_OUTPUT_DEBUG_LOG(HS_STR_debugLogExitedPostdispatch, OS_DEBUG_LOG_DEBUG);

    GT_UNREFERENCED_PARAMETER(pRTParam);
    GT_UNREFERENCED_PARAMETER(pUserArgs);

    OS_OUTPUT_DEBUG_LOG(HS_STR_debugLogExitedPostdispatch, OS_DEBUG_LOG_DEBUG);
}

static bool hsInitializeQueueInterception(hsa_queue_t* queue)
{
    bool retVal = false;
    hsa_status_t rcHSA = hsa_ext_tools_set_callback_functions(queue, &hsPreDispatchCallback, &hsPostDispatchCallback);
    GT_IF_WITH_ASSERT(HSA_STATUS_SUCCESS == rcHSA)
    {
        rcHSA = hsa_ext_tools_set_callback_arguments(queue, nullptr, nullptr);
        GT_IF_WITH_ASSERT(HSA_STATUS_SUCCESS == rcHSA)
        {
            retVal = true;
        }
    }

    if (OS_DEBUG_LOG_DEBUG <= osDebugLog::instance().loggedSeverity())
    {
        gtString logMsg;
        logMsg.appendFormattedString(HS_STR_debugLogInterceptingQueueDispatch, (void*)queue, retVal ? 'Y' : 'N');
        OS_OUTPUT_DEBUG_LOG(logMsg.asCharArray(), OS_DEBUG_LOG_DEBUG);
    }

    return retVal;
}

///////////////////////////////
// HSA API function wrappers //
///////////////////////////////
static ApiTable gs_HSAAPITable;
static ExtTable gs_HSAEXTTable;

#define HS_MIN_QUEUE_SIZE_FOR_DEBUGGING 128
static hsa_status_t hsWrapper_hsa_queue_create(hsa_agent_t agent, uint32_t size, hsa_queue_type_t type, void (*callback)(hsa_status_t status, hsa_queue_t* source, void* data), void* data, uint32_t private_segment_size, uint32_t group_segment_size, hsa_queue_t** queue)
{
    uint32_t usedSize = size;

    if (usedSize < HS_MIN_QUEUE_SIZE_FOR_DEBUGGING)
    {
        usedSize = HS_MIN_QUEUE_SIZE_FOR_DEBUGGING;
        gtString logMsg;
        logMsg.appendFormattedString(HS_STR_debugLogSpecifiedQueueSizeTooSmall, size, usedSize);
        OS_OUTPUT_DEBUG_LOG(logMsg.asCharArray(), OS_DEBUG_LOG_DEBUG);
    }

    hsa_status_t retVal = gs_HSAAPITable.hsa_queue_create_fn(agent, usedSize, type, callback, data, private_segment_size, group_segment_size, queue);

    if (HSA_STATUS_SUCCESS == retVal)
    {
        GT_IF_WITH_ASSERT(nullptr != queue)
        {
            GT_IF_WITH_ASSERT(nullptr != *queue)
            {
                bool rcIntercept = hsInitializeQueueInterception(*queue);
                GT_ASSERT(rcIntercept);
            }
        }
    }
    else
    {
        OS_OUTPUT_DEBUG_LOG(HS_STR_debugLogFailQueueCreationWithModifiedSize, OS_DEBUG_LOG_DEBUG);
        retVal = gs_HSAAPITable.hsa_queue_create_fn(agent, size, type, callback, data, private_segment_size, group_segment_size, queue);
    }

    return retVal;
}

static hsa_status_t hsWrapper_hsa_ext_program_finalize(hsa_ext_program_t program, hsa_isa_t isa, int32_t call_convention, hsa_ext_control_directives_t control_directives, const char* options, hsa_code_object_type_t code_object_type, hsa_code_object_t* code_object)
{
    gtASCIIString finOptions = "-g -O0 -amd-reserved-num-vgprs=4";

    if (nullptr != options)
    {
        finOptions.prepend(' ').prepend(options);
    }

    if (OS_DEBUG_LOG_DEBUG <= osDebugLog::instance().loggedSeverity())
    {
        gtString origOpts;
        origOpts.fromASCIIString((nullptr != options) ? options : "");
        gtString modOpts;
        modOpts.fromASCIIString(finOptions.asCharArray());

        gtString logMsg;
        logMsg.appendFormattedString(HS_STR_debugLogApplyingBuildFlags, (void*)program.handle, origOpts.asCharArray(), modOpts.asCharArray());
        OS_OUTPUT_DEBUG_LOG(logMsg.asCharArray(), OS_DEBUG_LOG_DEBUG);
    }

    hsa_status_t status = gs_HSAEXTTable.hsa_ext_program_finalize_fn(program, isa, call_convention, control_directives, finOptions.asCharArray(), code_object_type, code_object);

    if (HSA_STATUS_SUCCESS != status)
    {
        GT_ASSERT(HSA_STATUS_SUCCESS == status);
        OS_OUTPUT_DEBUG_LOG(HS_STR_debugLogFailProgramBuildWithFlags, OS_DEBUG_LOG_DEBUG);
        status = gs_HSAEXTTable.hsa_ext_program_finalize_fn(program, isa, call_convention, control_directives, options, code_object_type, code_object);
    }

    return status;
}

static hsa_status_t hsWrapper_hsa_shut_down()
{
    // Save the pointer, in case UninitializeInterception touches gs_HSAAPITable:
    decltype(hsa_shut_down)* pfn_hsa_shut_down_real_pointer = gs_HSAAPITable.hsa_shut_down_fn;

    // Stop debugging:
    hsDebuggingManager::instance().UninitializeInterception();

    // Call the real function:
    hsa_status_t retVal = pfn_hsa_shut_down_real_pointer();

    return retVal;
}

extern "C" bool OnLoad(ApiTable* pApiTable, uint64_t runtimeVersion, uint64_t failedToolCount, const char* const* pFailedToolNames)
{
    OS_OUTPUT_DEBUG_LOG(HS_STR_debugLogHSAOnLoad, OS_DEBUG_LOG_DEBUG);

    GT_UNREFERENCED_PARAMETER(runtimeVersion);
    GT_UNREFERENCED_PARAMETER(failedToolCount);
    GT_UNREFERENCED_PARAMETER(pFailedToolNames);
    GT_IF_WITH_ASSERT(nullptr != pApiTable)
    {
        ::memcpy(&gs_HSAAPITable, pApiTable, sizeof(ApiTable));
        pApiTable->hsa_queue_create_fn = &hsWrapper_hsa_queue_create;
        pApiTable->hsa_shut_down_fn = &hsWrapper_hsa_shut_down;

        ExtTable* pExtTable = pApiTable->std_exts_;
        GT_IF_WITH_ASSERT(nullptr != pExtTable)
        {
            ::memcpy(&gs_HSAEXTTable, pExtTable, sizeof(ExtTable));
            pExtTable->hsa_ext_program_finalize_fn = &hsWrapper_hsa_ext_program_finalize;
        }

        HwDbgStatus rcInitDBE = HwDbgInit(pApiTable);
        GT_ASSERT(HWDBG_STATUS_SUCCESS == rcInitDBE);
    }

    return true;
}

extern "C" void OnUnload(ApiTable* pApiTable)
{
    OS_OUTPUT_DEBUG_LOG(HS_STR_debugLogHSAOnUnload, OS_DEBUG_LOG_DEBUG);

    GT_UNREFERENCED_PARAMETER(pApiTable);
    // ::memcpy(pApiTable, &gs_HSAAPITable, sizeof(ApiTable));
    // ExtTable* pExtTable = pApiTable->std_exts_;
    // ::memcpy(pExtTable, &gs_HSAEXTTable, sizeof(ExtTable));
}

////////////////////////
// hsDebuggingManager //
////////////////////////
hsDebuggingManager& hsDebuggingManager::instance()
{
    if (nullptr == ms_pMySingleInstance)
    {
        ms_pMySingleInstance = new hsDebuggingManager;
    }

    return *ms_pMySingleInstance;
}

bool hsDebuggingManager::InitializeInterception()
{
    bool retVal = true;

    // The HSA_AGENT / HSA_TOOLS_LIB environment variable currently handles this, no need to make additional steps in the module constructor.

    return retVal;
}

bool hsDebuggingManager::UninitializeInterception()
{
    bool retVal = true;

    HwDbgStatus rcUninitDBE = HwDbgShutDown();
    GT_ASSERT(HWDBG_STATUS_SUCCESS == rcUninitDBE);

    return retVal;
}

bool hsDebuggingManager::ShouldDebugKernel(const gtString& kernelName)
{
    bool retVal = false;

    static bool firstTime = true;
    static bool alwaysDebug = false;
    static bool neverDebug = false;

    if (firstTime)
    {
        firstTime = false;
        gtString var;
        unsigned int varUInt = 0;
        osGetCurrentProcessEnvVariableValue(OS_STR_envVar_hsAlwaysDebug, var);
        var.toUnsignedIntNumber(varUInt);
        alwaysDebug = (0 < varUInt);

        var.makeEmpty();
        varUInt = 0;
        osGetCurrentProcessEnvVariableValue(OS_STR_envVar_hsNeverDebug, var);
        var.toUnsignedIntNumber(varUInt);
        neverDebug = (0 < varUInt);
    }

    if (alwaysDebug || neverDebug)
    {
        // If both are specified, choose "never":
        GT_ASSERT(alwaysDebug != neverDebug);
        retVal = !neverDebug;
    }
    else if (!kernelName.isEmpty())
    {
        gtVector<gtUInt64> bps;
        bool rcBP = suBreakpointsManager::instance().getHSABreakpointsForKernel(kernelName, bps);

        if (rcBP && (0 < bps.size()))
        {
            retVal = true;
        }
    }

    return retVal;
}

bool hsDebuggingManager::StartDebugging(void* hDebugContext, const gtString& kernelName, const void* kernelArgs, const gtUInt32 gs[3], const gtUInt32 wgs[3])
{
    OS_OUTPUT_DEBUG_LOG(HS_STR_debugLogStartingDebuggingStart, OS_DEBUG_LOG_EXTENSIVE);

    bool retVal = false;
    bool sendFailMessage = true;

    gtString errMsg = HS_STR_debuggingErrorUnknown;

    // Cannot enter debugging if another session is in flight:
    if (m_debuggingCS.tryEntering())
    {
        OS_OUTPUT_DEBUG_LOG(HS_STR_debugLogStartingDebuggingEntered, OS_DEBUG_LOG_EXTENSIVE);
        HwDbgContextHandle hDbgCtx = (HwDbgContextHandle)hDebugContext;

        const void* pBinary = nullptr;
        size_t binarySize = 0;
        HwDbgStatus rcDBE = HwDbgGetKernelBinary(hDbgCtx, &pBinary, &binarySize);

        GT_IF_WITH_ASSERT((HWDBG_STATUS_SUCCESS == rcDBE) && (nullptr != pBinary) && (0 < binarySize))
        {
            m_pCurrentDebugInfo = new(std::nothrow) hsDebugInfo(pBinary, binarySize, kernelArgs);
            GT_IF_WITH_ASSERT(nullptr != m_pCurrentDebugInfo)
            {
                GT_IF_WITH_ASSERT(m_pCurrentDebugInfo->IsInitialized())
                {
                    m_pCurrentDebuggingThread = new(std::nothrow) hsDebugEventThread(hDebugContext, kernelName, m_debuggingCS, gs, wgs);
                    GT_IF_WITH_ASSERT(nullptr != m_pCurrentDebuggingThread)
                    {
                        // TO_DO: Set temporary breakpoints on all PCs only if coming from a kernel BP:
                        m_pCurrentDebuggingThread->SetupStep(AP_KERNEL_STEP_IN);

                        // Apply any breakpoints and/or our "Step in" operation:
                        m_pCurrentDebuggingThread->UpdateBreakpoints();

                        // Execute the thread:
                        m_pCurrentDebuggingThread->execute();

                        retVal = true;
                    }
                    else
                    {
                        errMsg = HS_STR_debuggingErrorMemory;
                    }
                }
                else
                {
                    errMsg = HS_STR_debuggingErrorDebugInfo;
                }
            }
            else
            {
                errMsg = HS_STR_debuggingErrorMemory;
            }
        }
        else
        {
            errMsg = HS_STR_debuggingErrorBinary;
        }

        if (!retVal)
        {
            Cleanup();
        }

        // Allow the debugging thread to run:
        m_debuggingCS.leave();
    }
    else
    {
        errMsg = HS_STR_debuggingErrorConcurrent;

        // Only send the "Concurrent kernel debugging" error once:
        static bool firstTime = true;
        sendFailMessage = firstTime;
        firstTime = false;
    }

    // Send an error event to the client:
    if (!retVal && sendFailMessage)
    {
        apKernelDebuggingFailedEvent failedEve(apKernelDebuggingFailedEvent::AP_KERNEL_DEBUG_FAILURE, CL_SUCCESS, osGetCurrentThreadId(), errMsg);
        bool rcFailEve = suForwardEventToClient(failedEve);
        GT_ASSERT(rcFailEve);
    }

    OS_OUTPUT_DEBUG_LOG(HS_STR_debugLogStartingDebuggingEnd, OS_DEBUG_LOG_EXTENSIVE);

    return retVal;
}

bool hsDebuggingManager::IsDebuggingInProgress() const
{
    bool retVal = false;

    if (nullptr != m_pCurrentDebuggingThread)
    {
        if (m_pCurrentDebuggingThread->isAlive())
        {
            retVal = true;
        }
        else
        {
            // This means we need to clean up the dead thread:
            const_cast<hsDebuggingManager*>(this)->Cleanup();
        }
    }

    return retVal;
}

bool hsDebuggingManager::IsInHSAKernelBreakpoint() const
{
    bool retVal = false;

    if (IsDebuggingInProgress())
    {
        retVal = m_pCurrentDebuggingThread->IsSuspendedAtBreakpoint();
    }

    return retVal;
}

gtUInt64 hsDebuggingManager::GetCurrentAddress() const
{
    gtUInt64 retVal = 0;

    GT_IF_WITH_ASSERT(IsDebuggingInProgress())
    {
        return m_pCurrentDebuggingThread->GetActiveWavefrontPC();
    }

    return retVal;
}

bool hsDebuggingManager::SetNextDebuggingCommand(apKernelDebuggingCommand cmd)
{
    bool retVal = false;

    GT_IF_WITH_ASSERT(IsDebuggingInProgress())
    {
        retVal = m_pCurrentDebuggingThread->SetupStep(cmd);
    }

    return retVal;
}

bool hsDebuggingManager::SetBreakpoint(const gtString& kernelName, gtUInt64 lineNumber)
{
    // Defer the breakpoint to the next call to UpdateBreakpoints():
    bool retVal = suBreakpointsManager::instance().addHSABreakpointForKernel(kernelName, lineNumber);

    return retVal;
}

void hsDebuggingManager::StopKernelRun()
{
    if (IsDebuggingInProgress())
    {
        m_pCurrentDebuggingThread->StopDebugging();
    }
}

void* hsDebuggingManager::GetDebugContextHandle()
{
    void* retVal = nullptr;

    GT_IF_WITH_ASSERT(IsDebuggingInProgress())
    {
        retVal = m_pCurrentDebuggingThread->GetDebugContextHandle();
    }

    return retVal;
}

bool hsDebuggingManager::SetActiveWavefront(gtUInt32 waveIndex, gtUByte threadIndex)
{
    bool retVal = false;

    GT_IF_WITH_ASSERT(IsDebuggingInProgress())
    {
        retVal = m_pCurrentDebuggingThread->SetActiveWavefront(waveIndex, threadIndex);
    }

    return retVal;
}

bool hsDebuggingManager::GetWorkItemId(gtUInt32 waveIndex, gtUByte threadIndex, gtUInt32 o_gid[3], gtUInt32 o_lid[3], gtUInt32 o_wgid[3], bool& o_active) const
{
    bool retVal = false;

    GT_IF_WITH_ASSERT(IsDebuggingInProgress())
    {
        retVal = m_pCurrentDebuggingThread->GetWorkItemId(waveIndex, threadIndex, o_gid, o_lid, o_wgid, o_active);
    }

    return retVal;
}

bool hsDebuggingManager::GetActiveWorkItem(gtUInt32 o_gid[3], gtUInt32 o_lid[3], gtUInt32 o_wgid[3], bool& o_active) const
{
    bool retVal = false;

    GT_IF_WITH_ASSERT(IsDebuggingInProgress())
    {
        retVal = m_pCurrentDebuggingThread->GetActiveWorkItem(o_gid, o_lid, o_wgid, o_active);
    }

    return retVal;
}

gtUInt32 hsDebuggingManager::GetWavefrontCount() const
{
    gtUInt32 retVal = 0;

    GT_IF_WITH_ASSERT(IsDebuggingInProgress())
    {
        retVal = m_pCurrentDebuggingThread->GetWavefrontCount();
    }

    return retVal;
}

gtUByte hsDebuggingManager::GetWorkDimensions() const
{
    gtUByte retVal = 0;

    GT_IF_WITH_ASSERT(IsDebuggingInProgress())
    {
        retVal = m_pCurrentDebuggingThread->GetWorkDims();
    }

    return retVal;
}

const gtString& hsDebuggingManager::GetKernelName() const
{
    GT_IF_WITH_ASSERT(IsDebuggingInProgress())
    {
        return m_pCurrentDebuggingThread->GetKernelName();
    }

    static const gtString emptyStr;
    return emptyStr;
}

hsDebuggingManager::hsDebuggingManager()
    :  m_pCurrentDebugInfo(nullptr), m_pCurrentDebuggingThread(nullptr), m_isDuringCleanup(false)
{
    OS_OUTPUT_DEBUG_LOG(HS_STR_debugLogInitDebuggingMgr, OS_DEBUG_LOG_DEBUG);
}

hsDebuggingManager::~hsDebuggingManager()
{
}

void hsDebuggingManager::Cleanup()
{
    if (!m_isDuringCleanup)
    {
        m_isDuringCleanup = true;

        delete m_pCurrentDebugInfo;
        m_pCurrentDebugInfo = nullptr;

        if (nullptr != m_pCurrentDebuggingThread)
        {
            // Try ending the thread three times:
            // 1. Just wait it out
            // 2. Tell it to end and wait it out
            // 3. Use osThread::terminate
            // If the thread is still alive after all this, just delete the object.
            for (int i = 0; 3 > i; ++i)
            {
                switch (i)
                {
                    case 0: OS_OUTPUT_DEBUG_LOG(HS_STR_debugLogDebugEventThreadWaitingForTermination0, OS_DEBUG_LOG_DEBUG); break;

                    case 1: OS_OUTPUT_DEBUG_LOG(HS_STR_debugLogDebugEventThreadWaitingForTermination1, OS_DEBUG_LOG_DEBUG); m_pCurrentDebuggingThread->beforeTermination(); break;

                    case 2: OS_OUTPUT_DEBUG_LOG(HS_STR_debugLogDebugEventThreadWaitingForTermination2, OS_DEBUG_LOG_DEBUG); m_pCurrentDebuggingThread->terminate(); break;

                    default: break;
                }

                int waitCount = 0;

                while (m_pCurrentDebuggingThread->isAlive() && (waitCount++ < 20))
                {
                    osSleep(100);
                }

                if (!m_pCurrentDebuggingThread->isAlive())
                {
                    break;
                }
            }
        }

        delete m_pCurrentDebuggingThread;
        m_pCurrentDebuggingThread = nullptr;

        m_isDuringCleanup = false;
    }
}

////////////////////////////////////////////
// hsDebuggingManager::hsDebugEventThread //
////////////////////////////////////////////
hsDebuggingManager::hsDebugEventThread::hsDebugEventThread(void* hDebugContext, const gtString& kernelName, osCriticalSection& debuggingCS, const gtUInt32 gs[3], const gtUInt32 wgs[3])
    : osThread(L"hsDebuggingManager::hsDebugEventThread"), m_hDebugContext(hDebugContext), m_kernelName(kernelName), m_isSuspendedAtBreakpoint(false), m_nextCommand(AP_KERNEL_CONTINUE), m_debuggingCS(debuggingCS), m_goOn(true)
{
    OS_OUTPUT_DEBUG_LOG(HS_STR_debugLogDebugEventThreadCreated, OS_DEBUG_LOG_DEBUG);

    m_workGroupSize[0] = wgs[0];
    m_workGroupSize[1] = wgs[1];
    m_workGroupSize[2] = wgs[2];
    m_gridSize[0] = gs[0];
    m_gridSize[1] = gs[1];
    m_gridSize[2] = gs[2];
}

hsDebuggingManager::hsDebugEventThread::~hsDebugEventThread()
{
    OS_OUTPUT_DEBUG_LOG(HS_STR_debugLogDebugEventThreadDestroyed, OS_DEBUG_LOG_DEBUG);
    m_goOn = false;
}

#define HS_DEBUG_EVENT_LOOP_TIMEOUT 10
int hsDebuggingManager::hsDebugEventThread::entryPoint()
{
    OS_OUTPUT_DEBUG_LOG(HS_STR_debugLogDebugEventThreadStartedRunning, OS_DEBUG_LOG_DEBUG);
    osCriticalSectionLocker csLocker(m_debuggingCS);

    suBreakpointsManager& theBkptsMgr = suBreakpointsManager::instance();
    apContextID nullCtx;
    HwDbgContextHandle hDbgCtx = (HwDbgContextHandle)m_hDebugContext;

    m_goOn = (nullptr != m_hDebugContext);

    while (m_goOn)
    {
        HwDbgEventType eve = HWDBG_EVENT_INVALID;
        OS_OUTPUT_DEBUG_LOG(HS_STR_debugLogDebugEventThreadWaitingForEvent, OS_DEBUG_LOG_DEBUG);
        HwDbgStatus rcDBE = HwDbgWaitForEvent(hDbgCtx, HS_DEBUG_EVENT_LOOP_TIMEOUT, &eve);
        OS_OUTPUT_DEBUG_LOG(HS_STR_debugLogDebugEventThreadGotEvent, OS_DEBUG_LOG_DEBUG);
        GT_ASSERT(HWDBG_STATUS_SUCCESS == rcDBE);

        switch (eve)
        {
            case HWDBG_EVENT_POST_BREAKPOINT:
            {
                apBreakReason reason = AP_KERNEL_SOURCE_CODE_BREAKPOINT_HIT;

                switch (m_nextCommand)
                {
                    case AP_KERNEL_CONTINUE:
                        reason = AP_KERNEL_SOURCE_CODE_BREAKPOINT_HIT;
                        break;

                    case AP_KERNEL_STEP_OVER:
                        reason = AP_STEP_OVER_BREAKPOINT_HIT;
                        break;

                    case AP_KERNEL_STEP_OUT:
                        reason = AP_STEP_OUT_BREAKPOINT_HIT;
                        break;

                    case AP_KERNEL_STEP_IN:
                        reason = AP_STEP_IN_BREAKPOINT_HIT;
                        break;

                    default:
                        GT_ASSERT(false);
                        break;
                }

                // Update the wavefront data:
                UpdateWavefrontData();

                m_isSuspendedAtBreakpoint = true;
                m_nextCommand = AP_KERNEL_CONTINUE;
                theBkptsMgr.setBreakReason(reason);
                OS_OUTPUT_DEBUG_LOG(HS_STR_debugLogDebugEventThreadWaitingForCommand, OS_DEBUG_LOG_DEBUG);
                theBkptsMgr.triggerBreakpointException(nullCtx, apMonitoredFunctionsAmount);
                OS_OUTPUT_DEBUG_LOG(HS_STR_debugLogDebugEventThreadGotCommand, OS_DEBUG_LOG_DEBUG);
                m_isSuspendedAtBreakpoint = false;

                // Clear wavefront data:
                m_wavefrontData.clear();

                // If we were somehow told to stop, don't continue the debug event, as the handle might already be invalid:
                if (!m_goOn)
                {
                    // Update the handle in case it was cleared:
                    hDbgCtx = (HwDbgContextHandle)m_hDebugContext;

                    // Stop the loop:
                    break;
                }

                // Update the breakpoints at this time
                UpdateBreakpoints();

                // Resume debugging:
                OS_OUTPUT_DEBUG_LOG(HS_STR_debugLogDebugEventThreadContinueEvent, OS_DEBUG_LOG_DEBUG);
                rcDBE = HwDbgContinueEvent(hDbgCtx, HWDBG_COMMAND_CONTINUE);
                GT_ASSERT(HWDBG_STATUS_SUCCESS == rcDBE);
            }
            break;

            case HWDBG_EVENT_END_DEBUGGING:
                m_goOn = false;
                break;

            case HWDBG_EVENT_TIMEOUT:
                break;

            case HWDBG_EVENT_INVALID:
            default:
                GT_ASSERT(false);
                m_goOn = false;
                break;
        }
    }

    m_hDebugContext = nullptr;

    HwDbgStatus rcEnd = HwDbgEndDebugContext(hDbgCtx);
    GT_ASSERT(HWDBG_STATUS_SUCCESS == rcEnd);

    OS_OUTPUT_DEBUG_LOG(HS_STR_debugLogDebugEventThreadFinishedRunning, OS_DEBUG_LOG_DEBUG);
    return 0;
}

void hsDebuggingManager::hsDebugEventThread::beforeTermination()
{
    OS_OUTPUT_DEBUG_LOG(HS_STR_debugLogDebugEventThreadBeforeTermination, OS_DEBUG_LOG_DEBUG);
    StopDebugging();
}

#define HS_STOP_DEBUGGING_KILLALL_RETRY_COUNT 20
void hsDebuggingManager::hsDebugEventThread::StopDebugging()
{
    m_goOn = false;

    HwDbgContextHandle hDbgCtx = (HwDbgContextHandle)m_hDebugContext;

    if (nullptr != hDbgCtx)
    {
        HwDbgStatus rcKill = HWDBG_STATUS_NOT_INITIALIZED;

        for (int i = 0; (HS_STOP_DEBUGGING_KILLALL_RETRY_COUNT > i) && (HWDBG_STATUS_SUCCESS != rcKill); ++i)
        {
            // Kill all active wavefronts:
            rcKill = HwDbgKillAll(hDbgCtx);

            if (HWDBG_STATUS_SUCCESS != rcKill)
            {
                // This attempt fail, log the attempt and error code:
                gtString logMsg;
                logMsg.appendFormattedString(HS_STR_debugLogDebugEventThreadStopDebuggingKillAllFailed, i + 1, HS_STOP_DEBUGGING_KILLALL_RETRY_COUNT, rcKill);
                OS_OUTPUT_DEBUG_LOG(logMsg.asCharArray(), OS_DEBUG_LOG_DEBUG);

                // Wait for more waves to reach the scheduler, if possible:
                osSleep(10);
            }
        }

        GT_ASSERT(HWDBG_STATUS_SUCCESS == rcKill);
    }

    HwDbgStatus rcEnd = HwDbgEndDebugContext(hDbgCtx);
    GT_ASSERT(HWDBG_STATUS_SUCCESS == rcEnd);
    m_hDebugContext = nullptr;
}

bool hsDebuggingManager::hsDebugEventThread::SetupStep(apKernelDebuggingCommand cmd)
{
    m_nextCommand = cmd;

    return true;
}

void hsDebuggingManager::hsDebugEventThread::UpdateBreakpoints()
{
    HwDbgContextHandle hDbgCtx = (HwDbgContextHandle)m_hDebugContext;

    HwDbgStatus rcDel = HwDbgDeleteAllCodeBreakpoints(hDbgCtx);
    GT_ASSERT(HWDBG_STATUS_SUCCESS == rcDel);

    gtSet<gtUInt64> bpsToSet;

    hsDebuggingManager& theDebuggingMgr = hsDebuggingManager::instance();
    const hsDebugInfo* pDebugInfo = theDebuggingMgr.GetCurrentDebugInfo();
    GT_IF_WITH_ASSERT(nullptr != pDebugInfo)
    {
        if (AP_KERNEL_STEP_IN == m_nextCommand)
        {
            gtVector<gtUInt64> stepBPs;
            bool rcStp = pDebugInfo->GetStepInBreakpoints(stepBPs);
            GT_IF_WITH_ASSERT(rcStp)
            {
                for (const auto& b : stepBPs)
                {
                    bpsToSet.insert(b);
                }
            }
        }
        else
        {
            if ((AP_KERNEL_STEP_OVER == m_nextCommand) || (AP_KERNEL_STEP_OUT == m_nextCommand))
            {
                bool stepOut = (AP_KERNEL_STEP_OUT == m_nextCommand);
                gtVector<gtUInt64> stepBPs;
                bool rcStp = pDebugInfo->GetStepBreakpoints(stepOut, stepBPs);
                GT_IF_WITH_ASSERT(rcStp)
                {
                    for (const auto& b : stepBPs)
                    {
                        bpsToSet.insert(b);
                    }
                }
            }

            gtVector<gtUInt64> realBPLines;
            bool rcBP = suBreakpointsManager::instance().getHSABreakpointsForKernel(m_kernelName, realBPLines);
            GT_IF_WITH_ASSERT(rcBP)
            {
                for (const auto& l : realBPLines)
                {
                    gtVector<gtUInt64> bpAddrs;
                    bool rcAdd = pDebugInfo->LineToAddrs(l, bpAddrs);

                    if (rcAdd)
                    {
                        for (const auto& b : bpAddrs)
                        {
                            bpsToSet.insert(b);
                        }
                    }
                }
            }
        }
    }

    for (const auto& b : bpsToSet)
    {
        HwDbgCodeBreakpointHandle hBP = nullptr;
        HwDbgStatus rcBP = HwDbgCreateCodeBreakpoint(hDbgCtx, b, &hBP);
        GT_ASSERT((HWDBG_STATUS_SUCCESS == rcBP));
    }
}

void hsDebuggingManager::hsDebugEventThread::UpdateWavefrontData()
{
    // Get the waves:
    HwDbgContextHandle hDbgCtx = (HwDbgContextHandle)m_hDebugContext;
    const HwDbgWavefrontInfo* pWaves = nullptr;
    uint32_t waveCount = 0;
    HwDbgStatus rcWv = HwDbgGetActiveWavefronts(hDbgCtx, &pWaves, &waveCount);
    GT_IF_WITH_ASSERT(HWDBG_STATUS_SUCCESS == rcWv)
    {
        // Store the wave data:
        m_wavefrontData.resize(waveCount);

        for (uint32_t i = 0; waveCount > i; ++i)
        {
            const HwDbgWavefrontInfo& currentWave = pWaves[i];
            hsDebugWavefrontData& currentWaveData = m_wavefrontData[i];

            currentWaveData.m_wgId[0] = currentWave.workGroupId.x;
            currentWaveData.m_wgId[1] = currentWave.workGroupId.y;
            currentWaveData.m_wgId[2] = currentWave.workGroupId.z;

            // Store the thread data:
            const uint64_t& em = currentWave.executionMask;

            for (int j = 0; HS_WAVEFRONT_SIZE > j; ++j)
            {
                hsDebugWavefrontData::hsDebugWorkItemData& currentWorkItem = currentWaveData.m_wis[j];
                const HwDbgDim3 wiId = currentWave.workItemId[j];
                currentWorkItem.m_wiId[0] = wiId.x;
                currentWorkItem.m_wiId[1] = wiId.y;
                currentWorkItem.m_wiId[2] = wiId.z;
                currentWorkItem.m_active = (0 != ((1ULL << j) & em));
            }

            currentWaveData.m_waveAddress = currentWave.wavefrontAddress;
            currentWaveData.m_wavePC = currentWave.codeAddress;
        }
    }

    // Set the active wave and work item to the breakpoint hit location:
    m_activeWaveIndex = 0;
    m_activeWorkItemIndex = 0;
}

bool hsDebuggingManager::hsDebugEventThread::SetActiveWavefront(gtUInt32 waveIndex, gtUByte threadIndex)
{
    bool retVal = false;

    // Sanity check:
    GT_IF_WITH_ASSERT((GetWavefrontCount() > waveIndex) && (HS_WAVEFRONT_SIZE > threadIndex))
    {
        m_activeWaveIndex = waveIndex;
        m_activeWorkItemIndex = threadIndex;

        retVal = true;
    }

    return retVal;
}

bool hsDebuggingManager::hsDebugEventThread::GetWorkItemId(gtUInt32 waveIndex, gtUByte threadIndex, gtUInt32 o_gid[3], gtUInt32 o_lid[3], gtUInt32 o_wgid[3], bool& o_active) const
{
    bool retVal = false;

    // Sanity check:
    GT_IF_WITH_ASSERT((GetWavefrontCount() > waveIndex) && (HS_WAVEFRONT_SIZE > threadIndex))
    {
        // Only copy information for active dimensions:
        const hsDebugWavefrontData& wfData = m_wavefrontData[waveIndex];
        const hsDebugWavefrontData::hsDebugWorkItemData& wiData = wfData.m_wis[threadIndex];
        gtUByte d = GetWorkDims();

        switch (d)
        {
            case 3:
                o_lid[2] = wiData.m_wiId[2];
                o_wgid[2] = wfData.m_wgId[2];
                o_gid[2] = (m_workGroupSize[2] * o_wgid[2]) + o_lid[2];

            // Fall through
            case 2:
                o_lid[1] = wiData.m_wiId[1];
                o_wgid[1] = wfData.m_wgId[1];
                o_gid[1] = (m_workGroupSize[1] * o_wgid[1]) + o_lid[1];

            // Fall through
            case 1:
                o_lid[0] = wiData.m_wiId[0];
                o_wgid[0] = wfData.m_wgId[0];
                o_gid[0] = (m_workGroupSize[0] * o_wgid[0]) + o_lid[0];
                o_active = wiData.m_active;

                retVal = true;
                break;

            default:
                GT_ASSERT(false);
                break;
        }
    }

    return retVal;
}

gtUInt64 hsDebuggingManager::hsDebugEventThread::GetActiveWavefrontPC() const
{
    gtUInt64 retVal = 0;

    // Sanity check
    GT_IF_WITH_ASSERT(m_wavefrontData.size() > m_activeWaveIndex)
    {
        retVal = (gtUInt64)m_wavefrontData[m_activeWaveIndex].m_wavePC;
    }

    return retVal;
}
