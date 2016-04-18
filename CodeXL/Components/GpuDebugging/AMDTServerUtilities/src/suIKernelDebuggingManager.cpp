//==================================================================================
// Copyright (c) 2012-2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file suIKernelDebuggingManager.cpp
/// \brief Description: Contains common functions and static variable initialization
///
//==================================================================================

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTBaseTools/Include/gtMap.h>
#include <AMDTOSWrappers/Include/osDebugLog.h>
#include <AMDTOSWrappers/Include/osThread.h>
#include <AMDTOSAPIWrappers/Include/oaDriver.h>

/// Local:
#include <AMDTServerUtilities/Include/suIKernelDebuggingManager.h>
#include <AMDTServerUtilities/Include/suGlobalVariables.h>
#include <AMDTServerUtilities/Include/suSpyAPIFunctions.h>
#include <AMDTServerUtilities/Include/suStringConstants.h>

/// Initializing Debugging manager's critical section:
osCriticalSection suIKernelDebuggingManager::m_sCriticalSection;
osCriticalSection suIKernelDebuggingManager::m_sDispatchCriticalSection;
bool suIKernelDebuggingManager::m_sDispatchInFlight = false;
apMultipleKernelDebuggingDispatchMode suIKernelDebuggingManager::m_sMultipleKernelDebugDispatchMode = AP_MULTIPLE_KERNEL_DISPATCH_WAIT;


//////////////////////////////////////////////////////////////////////////
// Pseudo variable utilities:
//////////////////////////////////////////////////////////////////////////

// ----------------------------------------------------------------------------------
// Class Name:          suKernelDebuggingPseudoVariable
// General Description: An enumeration of pseudo-variables supported by our implementation
// Author:             Uri Shomroni
// Creation Date:      28/1/2014
// ----------------------------------------------------------------------------------
enum suKernelDebuggingPseudoVariable
{
    SU_PSEUDO_VAR_DISPATCH_DETAILS,     // +
    SU_PSEUDO_VAR_NDRANGE,              // ++
    SU_PSEUDO_VAR_WORK_DIM,             // |-
    SU_PSEUDO_VAR_GLOBAL_SIZE,          // |++
    SU_PSEUDO_VAR_GLOBAL_SIZE_X,        // ||-
    SU_PSEUDO_VAR_GLOBAL_SIZE_Y,        // ||-
    SU_PSEUDO_VAR_GLOBAL_SIZE_Z,        // ||-
    SU_PSEUDO_VAR_LOCAL_SIZE,           // |++
    SU_PSEUDO_VAR_LOCAL_SIZE_X,         // ||-
    SU_PSEUDO_VAR_LOCAL_SIZE_Y,         // ||-
    SU_PSEUDO_VAR_LOCAL_SIZE_Z,         // ||-
    SU_PSEUDO_VAR_GLOBAL_OFFSET,        // |++
    SU_PSEUDO_VAR_GLOBAL_OFFSET_X,      // ||-
    SU_PSEUDO_VAR_GLOBAL_OFFSET_Y,      // ||-
    SU_PSEUDO_VAR_GLOBAL_OFFSET_Z,      // ||-
    SU_PSEUDO_VAR_GLOBAL_WORKGROUPS,    // |++
    SU_PSEUDO_VAR_GLOBAL_WORKGROUPS_X,  // ||-
    SU_PSEUDO_VAR_GLOBAL_WORKGROUPS_Y,  // ||-
    SU_PSEUDO_VAR_GLOBAL_WORKGROUPS_Z,  // ||-
    SU_PSEUDO_VAR_TOTAL_WORKGROUPS,     // |-
    SU_PSEUDO_VAR_TOTAL_WORKITEMS,      // |-
    SU_PSEUDO_VAR_CURRENT_WORKITEM,     // ++
    SU_PSEUDO_VAR_CURRENT_GLOBALID,     //  ++
    SU_PSEUDO_VAR_CURRENT_GLOBALID_X,   //  |-
    SU_PSEUDO_VAR_CURRENT_GLOBALID_Y,   //  |-
    SU_PSEUDO_VAR_CURRENT_GLOBALID_Z,   //  |-
    SU_PSEUDO_VAR_CURRENT_LOCALID,      //  ++
    SU_PSEUDO_VAR_CURRENT_LOCALID_X,    //  |-
    SU_PSEUDO_VAR_CURRENT_LOCALID_Y,    //  |-
    SU_PSEUDO_VAR_CURRENT_LOCALID_Z,    //  |-
    SU_PSEUDO_VAR_CURRENT_GROUPID,      //  ++
    SU_PSEUDO_VAR_CURRENT_GROUPID_X,    //   -
    SU_PSEUDO_VAR_CURRENT_GROUPID_Y,    //   -
    SU_PSEUDO_VAR_CURRENT_GROUPID_Z,    //   -

    SU_PSEUDO_VAR_NONE
};

// Pseudo variable strings:
#define SU_PSEUDO_VAR_NAME_SEPARATOR L"."
#define SU_PSEUDO_VAR_NAME_X L"x"
#define SU_PSEUDO_VAR_NAME_Y L"y"
#define SU_PSEUDO_VAR_NAME_Z L"z"
#define SU_PSEUDO_VAR_NDRANGE_NAME L"NDRange"
#define SU_PSEUDO_VAR_WORK_DIM_NAME L"Work dimensions"
#define SU_PSEUDO_VAR_GLOBAL_SIZE_NAME L"Global work size"
#define SU_PSEUDO_VAR_LOCAL_SIZE_NAME L"Local work size"
#define SU_PSEUDO_VAR_GLOBAL_OFFSET_NAME L"Global work offset"
#define SU_PSEUDO_VAR_GLOBAL_WORKGROUPS_NAME L"Global work-groups"
#define SU_PSEUDO_VAR_TOTAL_WORKGROUPS_NAME L"Number of work-groups"
#define SU_PSEUDO_VAR_TOTAL_WORKITEMS_NAME L"Number of work-items"
#define SU_PSEUDO_VAR_CURRENT_WORKITEM_NAME L"Current work-item"
#define SU_PSEUDO_VAR_CURRENT_GLOBALID_NAME L"Global ID"
#define SU_PSEUDO_VAR_CURRENT_LOCALID_NAME L"Local ID"
#define SU_PSEUDO_VAR_CURRENT_GROUPID_NAME L"Group ID"


// ---------------------------------------------------------------------------
// Name:        suPseudoVariableIDFromName
// Description: Gets the pseudo-variable identifier from the variable name
// Author:      Uri Shomroni
// Date:        28/1/2014
// ---------------------------------------------------------------------------
static suKernelDebuggingPseudoVariable suPseudoVariableIDFromName(const gtString& pseudoVarName)
{
    suKernelDebuggingPseudoVariable retVal = SU_PSEUDO_VAR_NONE;

    // Only initialize this map on the first call to the function:
    static bool s_initializeMap = true;
    static gtMap<gtString, suKernelDebuggingPseudoVariable> s_pseudoVarNameToId;

    if (s_initializeMap)
    {
        s_initializeMap = false;

        gtString str1 = SU_PSEUDO_VAR_DISPATCH_DETAILS_NAME;
        s_pseudoVarNameToId[str1] = SU_PSEUDO_VAR_DISPATCH_DETAILS;
        str1.append(SU_PSEUDO_VAR_NAME_SEPARATOR);
        {
            gtString str2 = str1;
            str2.append(SU_PSEUDO_VAR_NDRANGE_NAME);
            s_pseudoVarNameToId[str2] = SU_PSEUDO_VAR_NDRANGE;
            str2.append(SU_PSEUDO_VAR_NAME_SEPARATOR);
            {
                gtString str3 = str2;
                str3.append(SU_PSEUDO_VAR_WORK_DIM_NAME);
                s_pseudoVarNameToId[str3] = SU_PSEUDO_VAR_WORK_DIM;
            }
            {
                gtString str3 = str2;
                str3.append(SU_PSEUDO_VAR_GLOBAL_SIZE_NAME);
                s_pseudoVarNameToId[str3] = SU_PSEUDO_VAR_GLOBAL_SIZE;
                str3.append(SU_PSEUDO_VAR_NAME_SEPARATOR);

                {
                    gtString str4 = str3;
                    str4.append(SU_PSEUDO_VAR_NAME_X);
                    s_pseudoVarNameToId[str4] = SU_PSEUDO_VAR_GLOBAL_SIZE_X;
                    str4 = str3;
                    str4.append(SU_PSEUDO_VAR_NAME_Y);
                    s_pseudoVarNameToId[str4] = SU_PSEUDO_VAR_GLOBAL_SIZE_Y;
                    str4 = str3;
                    str4.append(SU_PSEUDO_VAR_NAME_Z);
                    s_pseudoVarNameToId[str4] = SU_PSEUDO_VAR_GLOBAL_SIZE_Z;
                }
            }
            {
                gtString str3 = str2;
                str3.append(SU_PSEUDO_VAR_LOCAL_SIZE_NAME);
                s_pseudoVarNameToId[str3] = SU_PSEUDO_VAR_LOCAL_SIZE;
                str3.append(SU_PSEUDO_VAR_NAME_SEPARATOR);

                {
                    gtString str4 = str3;
                    str4.append(SU_PSEUDO_VAR_NAME_X);
                    s_pseudoVarNameToId[str4] = SU_PSEUDO_VAR_LOCAL_SIZE_X;
                    str4 = str3;
                    str4.append(SU_PSEUDO_VAR_NAME_Y);
                    s_pseudoVarNameToId[str4] = SU_PSEUDO_VAR_LOCAL_SIZE_Y;
                    str4 = str3;
                    str4.append(SU_PSEUDO_VAR_NAME_Z);
                    s_pseudoVarNameToId[str4] = SU_PSEUDO_VAR_LOCAL_SIZE_Z;
                }
            }
            {
                gtString str3 = str2;
                str3.append(SU_PSEUDO_VAR_GLOBAL_OFFSET_NAME);
                s_pseudoVarNameToId[str3] = SU_PSEUDO_VAR_GLOBAL_OFFSET;
                str3.append(SU_PSEUDO_VAR_NAME_SEPARATOR);

                {
                    gtString str4 = str3;
                    str4.append(SU_PSEUDO_VAR_NAME_X);
                    s_pseudoVarNameToId[str4] = SU_PSEUDO_VAR_GLOBAL_OFFSET_X;
                    str4 = str3;
                    str4.append(SU_PSEUDO_VAR_NAME_Y);
                    s_pseudoVarNameToId[str4] = SU_PSEUDO_VAR_GLOBAL_OFFSET_Y;
                    str4 = str3;
                    str4.append(SU_PSEUDO_VAR_NAME_Z);
                    s_pseudoVarNameToId[str4] = SU_PSEUDO_VAR_GLOBAL_OFFSET_Z;
                }
            }
            {
                gtString str3 = str2;
                str3.append(SU_PSEUDO_VAR_GLOBAL_WORKGROUPS_NAME);
                s_pseudoVarNameToId[str3] = SU_PSEUDO_VAR_GLOBAL_WORKGROUPS;
                str3.append(SU_PSEUDO_VAR_NAME_SEPARATOR);

                {
                    gtString str4 = str3;
                    str4.append(SU_PSEUDO_VAR_NAME_X);
                    s_pseudoVarNameToId[str4] = SU_PSEUDO_VAR_GLOBAL_WORKGROUPS_X;
                    str4 = str3;
                    str4.append(SU_PSEUDO_VAR_NAME_Y);
                    s_pseudoVarNameToId[str4] = SU_PSEUDO_VAR_GLOBAL_WORKGROUPS_Y;
                    str4 = str3;
                    str4.append(SU_PSEUDO_VAR_NAME_Z);
                    s_pseudoVarNameToId[str4] = SU_PSEUDO_VAR_GLOBAL_WORKGROUPS_Z;
                }
            }
            {
                gtString str3 = str2;
                str3.append(SU_PSEUDO_VAR_TOTAL_WORKGROUPS_NAME);
                s_pseudoVarNameToId[str3] = SU_PSEUDO_VAR_TOTAL_WORKGROUPS;
            }
            {
                gtString str3 = str2;
                str3.append(SU_PSEUDO_VAR_TOTAL_WORKITEMS_NAME);
                s_pseudoVarNameToId[str3] = SU_PSEUDO_VAR_TOTAL_WORKITEMS;
            }
        }

        {
            gtString str2 = str1;
            str2.append(SU_PSEUDO_VAR_CURRENT_WORKITEM_NAME);
            s_pseudoVarNameToId[str2] = SU_PSEUDO_VAR_CURRENT_WORKITEM;
            str2.append(SU_PSEUDO_VAR_NAME_SEPARATOR);
            {
                gtString str3 = str2;
                str3.append(SU_PSEUDO_VAR_CURRENT_GLOBALID_NAME);
                s_pseudoVarNameToId[str3] = SU_PSEUDO_VAR_CURRENT_GLOBALID;
                str3.append(SU_PSEUDO_VAR_NAME_SEPARATOR);

                {
                    gtString str4 = str3;
                    str4.append(SU_PSEUDO_VAR_NAME_X);
                    s_pseudoVarNameToId[str4] = SU_PSEUDO_VAR_CURRENT_GLOBALID_X;
                    str4 = str3;
                    str4.append(SU_PSEUDO_VAR_NAME_Y);
                    s_pseudoVarNameToId[str4] = SU_PSEUDO_VAR_CURRENT_GLOBALID_Y;
                    str4 = str3;
                    str4.append(SU_PSEUDO_VAR_NAME_Z);
                    s_pseudoVarNameToId[str4] = SU_PSEUDO_VAR_CURRENT_GLOBALID_Z;
                }
            }
            {
                gtString str3 = str2;
                str3.append(SU_PSEUDO_VAR_CURRENT_LOCALID_NAME);
                s_pseudoVarNameToId[str3] = SU_PSEUDO_VAR_CURRENT_LOCALID;
                str3.append(SU_PSEUDO_VAR_NAME_SEPARATOR);

                {
                    gtString str4 = str3;
                    str4.append(SU_PSEUDO_VAR_NAME_X);
                    s_pseudoVarNameToId[str4] = SU_PSEUDO_VAR_CURRENT_LOCALID_X;
                    str4 = str3;
                    str4.append(SU_PSEUDO_VAR_NAME_Y);
                    s_pseudoVarNameToId[str4] = SU_PSEUDO_VAR_CURRENT_LOCALID_Y;
                    str4 = str3;
                    str4.append(SU_PSEUDO_VAR_NAME_Z);
                    s_pseudoVarNameToId[str4] = SU_PSEUDO_VAR_CURRENT_LOCALID_Z;
                }
            }
            {
                gtString str3 = str2;
                str3.append(SU_PSEUDO_VAR_CURRENT_GROUPID_NAME);
                s_pseudoVarNameToId[str3] = SU_PSEUDO_VAR_CURRENT_GROUPID;
                str3.append(SU_PSEUDO_VAR_NAME_SEPARATOR);

                {
                    gtString str4 = str3;
                    str4.append(SU_PSEUDO_VAR_NAME_X);
                    s_pseudoVarNameToId[str4] = SU_PSEUDO_VAR_CURRENT_GROUPID_X;
                    str4 = str3;
                    str4.append(SU_PSEUDO_VAR_NAME_Y);
                    s_pseudoVarNameToId[str4] = SU_PSEUDO_VAR_CURRENT_GROUPID_Y;
                    str4 = str3;
                    str4.append(SU_PSEUDO_VAR_NAME_Z);
                    s_pseudoVarNameToId[str4] = SU_PSEUDO_VAR_CURRENT_GROUPID_Z;
                }
            }
        }

        // Also add specific names for some of the variables:
        s_pseudoVarNameToId[L"get_work_dim()"] = SU_PSEUDO_VAR_WORK_DIM;
        s_pseudoVarNameToId[L"get_global_size(0)"] = SU_PSEUDO_VAR_GLOBAL_SIZE_X;
        s_pseudoVarNameToId[L"get_global_size(1)"] = SU_PSEUDO_VAR_GLOBAL_SIZE_Y;
        s_pseudoVarNameToId[L"get_global_size(2)"] = SU_PSEUDO_VAR_GLOBAL_SIZE_Z;
        s_pseudoVarNameToId[L"get_global_id(0)"] = SU_PSEUDO_VAR_CURRENT_GLOBALID_X;
        s_pseudoVarNameToId[L"get_global_id(1)"] = SU_PSEUDO_VAR_CURRENT_GLOBALID_Y;
        s_pseudoVarNameToId[L"get_global_id(2)"] = SU_PSEUDO_VAR_CURRENT_GLOBALID_Z;
        s_pseudoVarNameToId[L"get_local_size(0)"] = SU_PSEUDO_VAR_LOCAL_SIZE_X;
        s_pseudoVarNameToId[L"get_local_size(1)"] = SU_PSEUDO_VAR_LOCAL_SIZE_Y;
        s_pseudoVarNameToId[L"get_local_size(2)"] = SU_PSEUDO_VAR_LOCAL_SIZE_Z;
        s_pseudoVarNameToId[L"get_local_id(0)"] = SU_PSEUDO_VAR_CURRENT_LOCALID_X;
        s_pseudoVarNameToId[L"get_local_id(1)"] = SU_PSEUDO_VAR_CURRENT_LOCALID_Y;
        s_pseudoVarNameToId[L"get_local_id(2)"] = SU_PSEUDO_VAR_CURRENT_LOCALID_Z;
        s_pseudoVarNameToId[L"get_num_groups(0)"] = SU_PSEUDO_VAR_GLOBAL_WORKGROUPS_X;
        s_pseudoVarNameToId[L"get_num_groups(1)"] = SU_PSEUDO_VAR_GLOBAL_WORKGROUPS_Y;
        s_pseudoVarNameToId[L"get_num_groups(2)"] = SU_PSEUDO_VAR_GLOBAL_WORKGROUPS_Z;
        s_pseudoVarNameToId[L"get_group_id(0)"] = SU_PSEUDO_VAR_CURRENT_GROUPID_X;
        s_pseudoVarNameToId[L"get_group_id(1)"] = SU_PSEUDO_VAR_CURRENT_GROUPID_Y;
        s_pseudoVarNameToId[L"get_group_id(2)"] = SU_PSEUDO_VAR_CURRENT_GROUPID_Z;
        s_pseudoVarNameToId[L"get_global_offset(0)"] = SU_PSEUDO_VAR_GLOBAL_OFFSET_X;
        s_pseudoVarNameToId[L"get_global_offset(1)"] = SU_PSEUDO_VAR_GLOBAL_OFFSET_Y;
        s_pseudoVarNameToId[L"get_global_offset(2)"] = SU_PSEUDO_VAR_GLOBAL_OFFSET_Z;
    }

    // Now that the map is initialized, get the id from the name:
    gtMap<gtString, suKernelDebuggingPseudoVariable>::const_iterator findIter = s_pseudoVarNameToId.find(pseudoVarName);
    gtMap<gtString, suKernelDebuggingPseudoVariable>::const_iterator endIter = s_pseudoVarNameToId.end();

    if (endIter != findIter)
    {
        retVal = findIter->second;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        suPseudoVarAidEvalUInt
// Description: Shorthand function for pseudo-variable evaluation
// Author:      Uri Shomroni
// Date:        28/1/2014
// ---------------------------------------------------------------------------
static inline void suPseudoVarAidEvalUInt(int i_val, gtString& o_str, gtString& o_strHex)
{
    o_str.appendFormattedString(L"%d", i_val);
    o_strHex.appendFormattedString(L"%#10x", i_val);
}

// ---------------------------------------------------------------------------
// Name:        void suPseudoVarAidEvalUIntn
// Description: Shorthand function for pseudo-variable evaluation
// Author:      Uri Shomroni
// Date:        28/1/2014
// ---------------------------------------------------------------------------
static inline void suPseudoVarAidEvalUIntn(int i_n, const int i_vals[3], gtString& o_str, gtString& o_strHex)
{
    if (i_n > 1)
    {
        o_str = L"{";
        o_strHex = L"{";
    }

    suPseudoVarAidEvalUInt(i_vals[0], o_str, o_strHex);

    if (i_n > 1)
    {
        o_str.append(L", ");
        o_strHex.append(L", ");
        suPseudoVarAidEvalUInt(i_vals[1], o_str, o_strHex);

        if (i_n > 2)
        {
            o_str.append(L", ");
            o_strHex.append(L", ");
            suPseudoVarAidEvalUInt(i_vals[2], o_str, o_strHex);
        }

        o_str.append(L"}");
        o_strHex.append(L"}");
    }
}

suIKernelDebuggingManager::suIKernelDebuggingManager() : m_isInitialized(false), m_isKernelDebuggingEnabled(true)
{
}

suIKernelDebuggingManager::~suIKernelDebuggingManager()
{
}

/// -----------------------------------------------------------------------------------------------
/// synchronizeWithKernelDebuggingCallback
/// \brief Description: Static method which causes synchronization by using a static critical section shared by debugging managers.
/// -----------------------------------------------------------------------------------------------
void suIKernelDebuggingManager::synchronizeWithKernelDebuggingCallback()
{
    OS_OUTPUT_DEBUG_LOG(L"Sync with kernel debugging callback started", OS_DEBUG_LOG_EXTENSIVE);
    suIKernelDebuggingManager::m_sCriticalSection.enter();
    suIKernelDebuggingManager::m_sCriticalSection.leave();
    OS_OUTPUT_DEBUG_LOG(L"Sync with kernel debugging callback ended", OS_DEBUG_LOG_EXTENSIVE);
}

// ---------------------------------------------------------------------------
// Name:        suIKernelDebuggingManager::reportKernelDebuggingFailure
// Description: Reports kernel debugging failure to the client app.
// Author:      Uri Shomroni
// Date:        9/3/2011
// ---------------------------------------------------------------------------
void suIKernelDebuggingManager::reportKernelDebuggingFailure(cl_int openCLError, apKernelDebuggingFailedEvent::apKernelDebuggingFailureReason failureReason, gtString& errorString)
{
    // Send an "kernel debugging failed" event to the client:
    osThreadId currentThreadId = osGetCurrentThreadId();
    apKernelDebuggingFailedEvent kernelDebuggingFailedEvent(failureReason, openCLError, currentThreadId, errorString);
    bool rcEve = suForwardEventToClient(kernelDebuggingFailedEvent);
    GT_ASSERT(rcEve);
}

// ---------------------------------------------------------------------------
// Name:        suIKernelDebuggingManager::setKernelDebuggingEnableState
// Description: Set if kernel debugging is enabled at all
// Arguments:   bool iKernelDebuggingState
// Return Val:  void
// Author:      Gilad Yarnitzky
// Date:        6/11/2011
// ---------------------------------------------------------------------------
bool suIKernelDebuggingManager::setKernelDebuggingEnableState(bool kernelDebuggingState)
{
    if (OS_DEBUG_LOG_EXTENSIVE <= osDebugLog::instance().loggedSeverity())
    {
        gtString logMsg;
        logMsg.appendFormattedString(L"Setting kernel debugging enable state. Previous value: %c. New Value: %c", m_isKernelDebuggingEnabled ? 'T' : 'F', kernelDebuggingState ? 'T' : 'F');
        OS_OUTPUT_DEBUG_LOG(logMsg.asCharArray(), OS_DEBUG_LOG_EXTENSIVE);
    }

    m_isKernelDebuggingEnabled = kernelDebuggingState;

    // Return true iff this is applied properly:
    return m_isInitialized;
}
// ---------------------------------------------------------------------------
// Name:        suIKernelDebuggingManager::isAMDKernelDebuggingEnabled
// Description: How should kernel debugging managers handle multiple kernel dispatches
// Author:      Uri Shomroni
// Date:        5/7/2015
// ---------------------------------------------------------------------------
bool suIKernelDebuggingManager::setMultipleKernelDebugDispatchMode(apMultipleKernelDebuggingDispatchMode mode)
{
    m_sMultipleKernelDebugDispatchMode = mode;

    return true;
}

// ---------------------------------------------------------------------------
// Name:        suIKernelDebuggingManager::isAMDKernelDebuggingEnabled
// Description: Handling access to the dispatch mechanism
// Return Val:  false if dispatch should be vetoed, true otherwise.
// Author:      Uri Shomroni
// Date:        5/7/2015
// ---------------------------------------------------------------------------
bool suIKernelDebuggingManager::beforeCheckingKernelDebuggingDisptach(osCriticalSectionDelayedLocker& dispatchLock)
{
    bool retVal = true;

    switch (m_sMultipleKernelDebugDispatchMode)
    {
        case AP_MULTIPLE_KERNEL_DISPATCH_CONCURRENT:
            // Allow concurrent, don't do anything else:
            retVal = true;
            break;

        case AP_MULTIPLE_KERNEL_DISPATCH_WAIT:
            // Allow concurrent, but lock the dispatch critical section:
            osWaitForFlagToTurnOff(m_sDispatchInFlight, ULONG_MAX);
            dispatchLock.attachToCriticalSection(m_sDispatchCriticalSection);
            retVal = true;
            break;

        case AP_MULTIPLE_KERNEL_DISPATCH_NO_DEBUG:
            // Disallow concurrent, don't lock:
            retVal = !m_sDispatchInFlight;
            break;

        default:
            // Unexpected Value!
            GT_ASSERT(false);
            break;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        suIKernelDebuggingManager::setDisptachInFlight
// Description: Set the dispatch in flight flag.
// Author:      Uri Shomroni
// Date:        6/7/2015
// ---------------------------------------------------------------------------
void suIKernelDebuggingManager::setDisptachInFlight(bool inFlight)
{
    osCriticalSectionLocker dispatchCSLocker(m_sDispatchCriticalSection);

    // Make sure that this function is not called needlessly:
    GT_ASSERT(m_sDispatchInFlight != inFlight);

    m_sDispatchInFlight = inFlight;
}

// ---------------------------------------------------------------------------
// Name:        suIKernelDebuggingManager::isAMDKernelDebuggingEnabled
// Description: Returns true iff the AMD kernel debugging API was initialized
//              properly, all functions pointers were acquired, etc.
// Return Val:  bool - Success / failure.
// Author:      Uri Shomroni
// Date:        24/10/2010
// ---------------------------------------------------------------------------
bool suIKernelDebuggingManager::isAMDKernelDebuggingEnabled()
{
    return m_isInitialized && m_isKernelDebuggingEnabled;
}

// ---------------------------------------------------------------------------
// Name:        suIKernelDebuggingManager::programBuildFlagsSupported
// Description: Returns true if the build flags are supported by the current
//              kernel debugger. The default value is "yes" for all flags.
// Author:      Uri Shomroni
// Date:        25/11/2014
// ---------------------------------------------------------------------------
bool suIKernelDebuggingManager::programBuildFlagsSupported(const char* buildFlags, gtString& failureReason)
{
    GT_UNREFERENCED_PARAMETER(buildFlags);
    GT_UNREFERENCED_PARAMETER(failureReason);
    return true;
}
// ---------------------------------------------------------------------------
// Name:        suIKernelDebuggingManager::shouldAppendLegacyBuildFlags
// Description: Returns true if the system configuration requires adding the legacy (1.2) build flags
// Author:      Uri Shomroni
// Date:        25/11/2014
// ---------------------------------------------------------------------------
bool suIKernelDebuggingManager::shouldAppendLegacyBuildFlags(const gtASCIIString& currentFlags, int amdOCLRuntimeVer) const
{
    bool retVal = false;

    // If the flags already contain the legacy flag, or we are building an OpenCL 2.0 kernel,
    // There is no need to add the flag
    if ((-1 == currentFlags.find(SU_STR_kernelDebuggingForcedBuildOptionLegacyASCII)) && (-1 == currentFlags.find("-cl-std=CL2.0")) && (-1 == currentFlags.find("-cl-std=cl2.0")))
    {
        // Check if we are on a non-HSA driver (where AMDIL doesn't exist and kernel debugging is not expected to work with the AMDIL path anyway):
        if (!oaIsHSADriver())
        {
            // If we properly got the AMD OpenCL runtime version:
            if (-1 < amdOCLRuntimeVer)
            {
                // Determine if it's a new enough driver to require the flag:
                retVal = (2004 <= amdOCLRuntimeVer);
            }
            else
            {
                // Get the driver version:
                int err = 0;
                gtString driverVersion = oaGetDriverVersion(err);
                int firstDot = driverVersion.find('.');

                if (0 < firstDot)
                {
                    driverVersion.truncate(0, firstDot - 1);
                }

                int driverVerAsInt = 0;

                // Catalyst 16.3 and up require the legacy flag:
                // Catalyst 16.4 appears as 15.11 and not 16.20:
                bool rcNum = driverVersion.toIntNumber(driverVerAsInt);

                if (rcNum && (15 < driverVerAsInt))
                {
                    retVal = true;
                }
                else if (rcNum && (15 == driverVerAsInt) && (0 < firstDot))
                {
                    driverVersion = oaGetDriverVersion(err);
                    int secondDot = driverVersion.find('.', firstDot + 1);

                    if (0 > secondDot)
                    {
                        secondDot = driverVersion.length();
                    }

                    driverVersion.truncate(firstDot + 1, secondDot - 1);
                    rcNum = driverVersion.toIntNumber(driverVerAsInt);

                    if (11 == driverVerAsInt)
                    {
                        retVal = true;
                    }
                }
            }
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        csAMDKernelDebuggingManager::extractDerefernceDataFromExpresssion
// Description: If variableName is an expression of type "varName[123]" or "*varName",
//              gets the dereference data from it
// Author:      Uri Shomroni
// Date:        5/6/2011
// ---------------------------------------------------------------------------
void suIKernelDebuggingManager::extractDerefernceDataFromExpresssion(const gtString& variableName, gtString& variableBaseName, gtString& dereferencedVariableMember, bool& shouldDereference, int& variableIndex)
{
    // If it is not dereferenced, the base name is the full name:
    variableBaseName = variableName;
    shouldDereference = false;
    variableIndex = 0;

    if (variableName.startsWith('*') && (variableName.find('*', 1) == -1))
    {
        // *p is like p[0]:
        int firstPeriod = variableName.find('.');

        if (firstPeriod < 1)
        {
            variableName.getSubString(1, -1, variableBaseName);
        }
        else // firstPeriod >= 1
        {
            variableName.getSubString(1, firstPeriod - 1, variableBaseName);
            variableName.getSubString(firstPeriod, -1, dereferencedVariableMember);
        }

        variableIndex = 0;
        shouldDereference = true;
    }
    else if ((variableName.count('[') == 1) && (variableName.count(']') == 1))
    {
        // We currently only support dereferencing of the type varName[123] (and parsing of varName[123].memberName):
        int leftBracket = variableName.find('[');
        int rightBracket = variableName.find(']');

        if ((leftBracket > 0) && (rightBracket > leftBracket + 1))
        {
            // Get the substrings:
            gtString baseName;
            variableName.getSubString(0, leftBracket - 1, baseName);
            gtString nameSuffix;

            if (rightBracket < variableName.length() - 1)
            {
                // Get the suffix:
                variableName.getSubString(rightBracket + 1, -1, nameSuffix);
            }

            gtString variableIndexAsString;
            variableName.getSubString(leftBracket + 1, rightBracket - 1, variableIndexAsString);

            if (variableIndexAsString.toIntNumber(variableIndex))
            {
                // If it's a valid number, dereference the variable:
                variableBaseName = baseName;
                dereferencedVariableMember = nameSuffix;
                shouldDereference = true;
            }
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        csAMDKernelDebuggingManager::matchMemberAliases
// Description: In OpenCL C, the floatn, intn, and other vector types have aliases
//              for some of their members:
//              a.x <-> a.s0
//              a.y <-> a.s1
//              a.z <-> a.s2
//              a.w <-> a.s3
//              a.sa <-> a.sA
//              ...
//              a.sf <-> a.sF
//              Try to match these aliases for the supplied variable name
// Return Val:  bool - Success / failure.
// Author:      Uri Shomroni
// Date:        16/5/2011
// ---------------------------------------------------------------------------
bool suIKernelDebuggingManager::matchOpenCLMemberAliases(const gtString& variableName, gtString& variableNameWithAlias)
{
    bool retVal = false;
    variableNameWithAlias.makeEmpty();

    // See if this is a vector member:
    int lastPeriod = variableName.reverseFind('.');

    if ((lastPeriod > -1) && (lastPeriod < variableName.length()))
    {
        // Get the original suffix
        gtString originalSuffix;
        variableName.getSubString(lastPeriod + 1, -1, originalSuffix);

        // Try to match one of the aliases:
        bool aliasFound = false;
        gtString suffixAlias;

        static const wchar_t* matchingAliases[20] =
        {
            L"x", L"s0",    // 2
            L"y", L"s1",    // 4
            L"z", L"s2",    // 6
            L"w", L"s3",    // 8
            L"sa", L"sA",   // 10
            L"sb", L"sB",   // 12
            L"sc", L"sC",   // 14
            L"sd", L"sD",   // 16
            L"se", L"sE",   // 18
            L"sf", L"sF",   // 20
        };

        // Check the aliases in pairs:
        for (int i = 0; i < 20; i += 2)
        {
            if (originalSuffix == matchingAliases[i])
            {
                // First -> second:
                suffixAlias = matchingAliases[i + 1];
                aliasFound = true;
                break;
            }
            else if (originalSuffix == matchingAliases[i + 1])
            {
                // Second -> first:
                suffixAlias = matchingAliases[i];
                aliasFound = true;
                break;
            }
        }

        // If there's an alias, return it:
        if (aliasFound)
        {
            // Get the base name:
            variableNameWithAlias = variableName;
            variableNameWithAlias.truncate(0, lastPeriod);
            variableNameWithAlias.append(suffixAlias);

            // The alias was found:
            retVal = true;
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        csAMDKernelDebuggingManager::filePathFromVariableName
// Description: Creates a raw data file path from a variable name
// Author:      Uri Shomroni
// Date:        3/3/2011
// ---------------------------------------------------------------------------
void suIKernelDebuggingManager::filePathFromVariableName(const gtString& variableName, osFilePath& rawDataFilePath)
{
    // Put the file in the log directory:
    rawDataFilePath = suCurrentSessionLogFilesDirectory();
    gtString fileName = variableName;

    // Replace characters that are legal in variable expressions, but not in file names:
    static const gtString pointerOrigString = L"*";
    static const gtString pointerReplacementString = L"{p}";
    static const gtString refOrigString = L"&";
    static const gtString refReplacementString = L"{r}";
    fileName.replace(pointerOrigString, pointerReplacementString);
    fileName.replace(refOrigString, refReplacementString);

    // Add the prefix:
    fileName.prepend(SU_STR_variableRawFileNamePrefix);

    // Set the file name and extension:
    rawDataFilePath.setFileName(fileName);
    rawDataFilePath.setFileExtension(SU_STR_rawFileExtension);
}

// ---------------------------------------------------------------------------
// Name:        suIKernelDebuggingManager::isPseudoVariable
// Description: Does a string represent a pseudo-variable?
// Author:      Uri Shomroni
// Date:        28/1/2014
// ---------------------------------------------------------------------------
bool suIKernelDebuggingManager::isPseudoVariable(const gtString& variableName)
{
    suKernelDebuggingPseudoVariable varId = suPseudoVariableIDFromName(variableName);

    return (SU_PSEUDO_VAR_NONE != varId);
}

// ---------------------------------------------------------------------------
// Name:        suIKernelDebuggingManager::getPseudoVariableValueString
// Description: Evaluate pseudo-variables
// Arguments: const gtString& variableName - variable name
//            const int coordinate[3] - current work item
//            const int globalWorkGeometry[10] - 0 = work dim; 1-3 = global size; 4-6 = local size; 7-9 = global offset
// Return Val:  bool - Success / failure.
// Author:      Uri Shomroni
// Date:        28/1/2014
// ---------------------------------------------------------------------------
bool suIKernelDebuggingManager::getPseudoVariableValueString(const gtString& variableName, const int coordinate[3], const int globalWorkGeometry[10], gtString& o_valueString, gtString& o_valueStringHex, gtString& o_variableType)
{
    bool retVal = false;

    suKernelDebuggingPseudoVariable varId = suPseudoVariableIDFromName(variableName);

    // Shorthand for input values:
    const int& workDim = globalWorkGeometry[0];
    const int* gSize = &globalWorkGeometry[1];
    const int* lSize = &globalWorkGeometry[4];
    const int* gOff = &globalWorkGeometry[7];

    // Division-by-zero safety. Values that are outside the dimension will not be accessed, so we can set them to 0:
#define SU_SAFEDIV(x, y) ((0 != y) ? (x / y) : 0)
#define SU_SAFEMOD(x, y) ((0 != y) ? (x % y) : 0)

    // Calculate other values:
    int gWGs[3] =       {SU_SAFEDIV(gSize[0], lSize[0]),    SU_SAFEDIV(gSize[1], lSize[1]),     SU_SAFEDIV(gSize[2], lSize[2])};
    int wiNoOff[3] =    {coordinate[0] - gOff[0],           coordinate[1] - gOff[1],            coordinate[2] - gOff[2]};
    int wiLID[3] =      {SU_SAFEMOD(wiNoOff[0], lSize[0]),  SU_SAFEMOD(wiNoOff[1], lSize[1]),   SU_SAFEMOD(wiNoOff[2], lSize[2])};
    int wiGrID[3] =     {SU_SAFEDIV(wiNoOff[0], lSize[0]),  SU_SAFEDIV(wiNoOff[1], lSize[1]),   SU_SAFEDIV(wiNoOff[2], lSize[2])};

#undef SU_SAFEDIV
#undef SU_SAFEMOD

    // Validate variables according to the work geometry:
    switch (varId)
    {
        case SU_PSEUDO_VAR_DISPATCH_DETAILS:
        case SU_PSEUDO_VAR_NDRANGE:
        case SU_PSEUDO_VAR_WORK_DIM:
        case SU_PSEUDO_VAR_GLOBAL_SIZE:
        case SU_PSEUDO_VAR_GLOBAL_SIZE_X:
        case SU_PSEUDO_VAR_LOCAL_SIZE:
        case SU_PSEUDO_VAR_LOCAL_SIZE_X:
        case SU_PSEUDO_VAR_GLOBAL_OFFSET:
        case SU_PSEUDO_VAR_GLOBAL_OFFSET_X:
        case SU_PSEUDO_VAR_GLOBAL_WORKGROUPS:
        case SU_PSEUDO_VAR_GLOBAL_WORKGROUPS_X:
        case SU_PSEUDO_VAR_TOTAL_WORKGROUPS:
        case SU_PSEUDO_VAR_TOTAL_WORKITEMS:
        case SU_PSEUDO_VAR_CURRENT_WORKITEM:
        case SU_PSEUDO_VAR_CURRENT_GLOBALID:
        case SU_PSEUDO_VAR_CURRENT_GLOBALID_X:
        case SU_PSEUDO_VAR_CURRENT_LOCALID:
        case SU_PSEUDO_VAR_CURRENT_LOCALID_X:
        case SU_PSEUDO_VAR_CURRENT_GROUPID:
        case SU_PSEUDO_VAR_CURRENT_GROUPID_X:
            retVal = true;
            break;

        case SU_PSEUDO_VAR_GLOBAL_SIZE_Y:
        case SU_PSEUDO_VAR_LOCAL_SIZE_Y:
        case SU_PSEUDO_VAR_GLOBAL_OFFSET_Y:
        case SU_PSEUDO_VAR_GLOBAL_WORKGROUPS_Y:
        case SU_PSEUDO_VAR_CURRENT_GLOBALID_Y:
        case SU_PSEUDO_VAR_CURRENT_LOCALID_Y:
        case SU_PSEUDO_VAR_CURRENT_GROUPID_Y:
            retVal = 1 < workDim;
            break;

        case SU_PSEUDO_VAR_GLOBAL_SIZE_Z:
        case SU_PSEUDO_VAR_LOCAL_SIZE_Z:
        case SU_PSEUDO_VAR_GLOBAL_OFFSET_Z:
        case SU_PSEUDO_VAR_GLOBAL_WORKGROUPS_Z:
        case SU_PSEUDO_VAR_CURRENT_GLOBALID_Z:
        case SU_PSEUDO_VAR_CURRENT_LOCALID_Z:
        case SU_PSEUDO_VAR_CURRENT_GROUPID_Z:
            retVal = 2 < workDim;
            break;

        case SU_PSEUDO_VAR_NONE:
        default:
            retVal = false;
            break;
    }

    if (retVal)
    {
        // Flatten the vector variables if the work dimension is 1:
        if (1 == workDim)
        {
            switch (varId)
            {
                case SU_PSEUDO_VAR_GLOBAL_SIZE:
                    varId = SU_PSEUDO_VAR_GLOBAL_SIZE_X;
                    break;

                case SU_PSEUDO_VAR_LOCAL_SIZE:
                    varId = SU_PSEUDO_VAR_LOCAL_SIZE_X;
                    break;

                case SU_PSEUDO_VAR_GLOBAL_OFFSET:
                    varId = SU_PSEUDO_VAR_GLOBAL_OFFSET_X;
                    break;

                case SU_PSEUDO_VAR_GLOBAL_WORKGROUPS:
                    varId = SU_PSEUDO_VAR_GLOBAL_WORKGROUPS_X;
                    break;

                case SU_PSEUDO_VAR_CURRENT_GLOBALID:
                    varId = SU_PSEUDO_VAR_CURRENT_GLOBALID_X;
                    break;

                case SU_PSEUDO_VAR_CURRENT_LOCALID:
                    varId = SU_PSEUDO_VAR_CURRENT_LOCALID_X;
                    break;

                case SU_PSEUDO_VAR_CURRENT_GROUPID:
                    varId = SU_PSEUDO_VAR_CURRENT_GROUPID_X;
                    break;

                default:
                    // Leave the id as-is
                    break;
            }
        }

        // Calculate the values:
        switch (varId)
        {
            case SU_PSEUDO_VAR_DISPATCH_DETAILS:
            case SU_PSEUDO_VAR_NDRANGE:
            case SU_PSEUDO_VAR_CURRENT_WORKITEM:
            {
                static const gtString structValString = L"{...}";
                o_valueString = structValString;
                o_valueStringHex = structValString;
            }
            break;

            case SU_PSEUDO_VAR_WORK_DIM:
                suPseudoVarAidEvalUInt(workDim, o_valueString, o_valueStringHex);
                break;

            case SU_PSEUDO_VAR_GLOBAL_SIZE:
                suPseudoVarAidEvalUIntn(workDim, gSize, o_valueString, o_valueStringHex);
                break;

            case SU_PSEUDO_VAR_GLOBAL_SIZE_X:
                suPseudoVarAidEvalUInt(gSize[0], o_valueString, o_valueStringHex);
                break;

            case SU_PSEUDO_VAR_GLOBAL_SIZE_Y:
                suPseudoVarAidEvalUInt(gSize[1], o_valueString, o_valueStringHex);
                break;

            case SU_PSEUDO_VAR_GLOBAL_SIZE_Z:
                suPseudoVarAidEvalUInt(gSize[2], o_valueString, o_valueStringHex);
                break;

            case SU_PSEUDO_VAR_LOCAL_SIZE:
                suPseudoVarAidEvalUIntn(workDim, lSize, o_valueString, o_valueStringHex);
                break;

            case SU_PSEUDO_VAR_LOCAL_SIZE_X:
                suPseudoVarAidEvalUInt(lSize[0], o_valueString, o_valueStringHex);
                break;

            case SU_PSEUDO_VAR_LOCAL_SIZE_Y:
                suPseudoVarAidEvalUInt(lSize[1], o_valueString, o_valueStringHex);
                break;

            case SU_PSEUDO_VAR_LOCAL_SIZE_Z:
                suPseudoVarAidEvalUInt(lSize[2], o_valueString, o_valueStringHex);
                break;

            case SU_PSEUDO_VAR_GLOBAL_OFFSET:
                suPseudoVarAidEvalUIntn(workDim, gOff, o_valueString, o_valueStringHex);
                break;

            case SU_PSEUDO_VAR_GLOBAL_OFFSET_X:
                suPseudoVarAidEvalUInt(gOff[0], o_valueString, o_valueStringHex);
                break;

            case SU_PSEUDO_VAR_GLOBAL_OFFSET_Y:
                suPseudoVarAidEvalUInt(gOff[1], o_valueString, o_valueStringHex);
                break;

            case SU_PSEUDO_VAR_GLOBAL_OFFSET_Z:
                suPseudoVarAidEvalUInt(gOff[2], o_valueString, o_valueStringHex);
                break;

            case SU_PSEUDO_VAR_GLOBAL_WORKGROUPS:
                suPseudoVarAidEvalUIntn(workDim, gWGs, o_valueString, o_valueStringHex);
                break;

            case SU_PSEUDO_VAR_GLOBAL_WORKGROUPS_X:
                suPseudoVarAidEvalUInt(gWGs[0], o_valueString, o_valueStringHex);
                break;

            case SU_PSEUDO_VAR_GLOBAL_WORKGROUPS_Y:
                suPseudoVarAidEvalUInt(gWGs[1], o_valueString, o_valueStringHex);
                break;

            case SU_PSEUDO_VAR_GLOBAL_WORKGROUPS_Z:
                suPseudoVarAidEvalUInt(gWGs[2], o_valueString, o_valueStringHex);
                break;

            case SU_PSEUDO_VAR_TOTAL_WORKGROUPS:
            {
                int totWGs = gWGs[0];

                if (workDim > 1)
                {
                    totWGs *= gWGs[1];

                    if (workDim > 2)
                    {
                        totWGs *= gWGs[2];
                    }
                }

                suPseudoVarAidEvalUInt(totWGs, o_valueString, o_valueStringHex);
            }
            break;

            case SU_PSEUDO_VAR_TOTAL_WORKITEMS:
            {
                int totWIs = gSize[0];

                if (workDim > 1)
                {
                    totWIs *= gSize[1];

                    if (workDim > 2)
                    {
                        totWIs *= gSize[2];
                    }
                }

                suPseudoVarAidEvalUInt(totWIs, o_valueString, o_valueStringHex);
            }
            break;

            case SU_PSEUDO_VAR_CURRENT_GLOBALID:
                suPseudoVarAidEvalUIntn(workDim, coordinate, o_valueString, o_valueStringHex);
                break;

            case SU_PSEUDO_VAR_CURRENT_GLOBALID_X:
                suPseudoVarAidEvalUInt(coordinate[0], o_valueString, o_valueStringHex);
                break;

            case SU_PSEUDO_VAR_CURRENT_GLOBALID_Y:
                suPseudoVarAidEvalUInt(coordinate[1], o_valueString, o_valueStringHex);
                break;

            case SU_PSEUDO_VAR_CURRENT_GLOBALID_Z:
                suPseudoVarAidEvalUInt(coordinate[2], o_valueString, o_valueStringHex);
                break;

            case SU_PSEUDO_VAR_CURRENT_LOCALID:
                suPseudoVarAidEvalUIntn(workDim, wiLID, o_valueString, o_valueStringHex);
                break;

            case SU_PSEUDO_VAR_CURRENT_LOCALID_X:
                suPseudoVarAidEvalUInt(wiLID[0], o_valueString, o_valueStringHex);
                break;

            case SU_PSEUDO_VAR_CURRENT_LOCALID_Y:
                suPseudoVarAidEvalUInt(wiLID[1], o_valueString, o_valueStringHex);
                break;

            case SU_PSEUDO_VAR_CURRENT_LOCALID_Z:
                suPseudoVarAidEvalUInt(wiLID[2], o_valueString, o_valueStringHex);
                break;

            case SU_PSEUDO_VAR_CURRENT_GROUPID:
                suPseudoVarAidEvalUIntn(workDim, wiGrID, o_valueString, o_valueStringHex);
                break;

            case SU_PSEUDO_VAR_CURRENT_GROUPID_X:
                suPseudoVarAidEvalUInt(wiGrID[0], o_valueString, o_valueStringHex);
                break;

            case SU_PSEUDO_VAR_CURRENT_GROUPID_Y:
                suPseudoVarAidEvalUInt(wiGrID[1], o_valueString, o_valueStringHex);
                break;

            case SU_PSEUDO_VAR_CURRENT_GROUPID_Z:
                suPseudoVarAidEvalUInt(wiGrID[2], o_valueString, o_valueStringHex);
                break;

            default:
                // Unexpected value!
                GT_ASSERT(false);
                break;
        }

        // Set the variable type:
        switch (varId)
        {
            case SU_PSEUDO_VAR_DISPATCH_DETAILS:
            case SU_PSEUDO_VAR_NDRANGE:
            case SU_PSEUDO_VAR_CURRENT_WORKITEM:
                o_variableType = L"struct";
                break;

            case SU_PSEUDO_VAR_GLOBAL_SIZE:
            case SU_PSEUDO_VAR_LOCAL_SIZE:
            case SU_PSEUDO_VAR_GLOBAL_OFFSET:
            case SU_PSEUDO_VAR_GLOBAL_WORKGROUPS:
            case SU_PSEUDO_VAR_CURRENT_GLOBALID:
            case SU_PSEUDO_VAR_CURRENT_LOCALID:
            case SU_PSEUDO_VAR_CURRENT_GROUPID:
            {
                switch (workDim)
                {
                    case 1:
                        o_variableType = L"uint";
                        break;

                    case 2:
                        o_variableType = L"uint2";
                        break;

                    case 3:
                        o_variableType = L"uint3";
                        break;

                    default:
                        GT_ASSERT(false);
                        break;
                }
            }
            break;

            case SU_PSEUDO_VAR_WORK_DIM:
            case SU_PSEUDO_VAR_GLOBAL_SIZE_X:
            case SU_PSEUDO_VAR_GLOBAL_SIZE_Y:
            case SU_PSEUDO_VAR_GLOBAL_SIZE_Z:
            case SU_PSEUDO_VAR_LOCAL_SIZE_X:
            case SU_PSEUDO_VAR_LOCAL_SIZE_Y:
            case SU_PSEUDO_VAR_LOCAL_SIZE_Z:
            case SU_PSEUDO_VAR_GLOBAL_OFFSET_X:
            case SU_PSEUDO_VAR_GLOBAL_OFFSET_Y:
            case SU_PSEUDO_VAR_GLOBAL_OFFSET_Z:
            case SU_PSEUDO_VAR_GLOBAL_WORKGROUPS_X:
            case SU_PSEUDO_VAR_GLOBAL_WORKGROUPS_Y:
            case SU_PSEUDO_VAR_GLOBAL_WORKGROUPS_Z:
            case SU_PSEUDO_VAR_TOTAL_WORKGROUPS:
            case SU_PSEUDO_VAR_TOTAL_WORKITEMS:
            case SU_PSEUDO_VAR_CURRENT_GLOBALID_X:
            case SU_PSEUDO_VAR_CURRENT_GLOBALID_Y:
            case SU_PSEUDO_VAR_CURRENT_GLOBALID_Z:
            case SU_PSEUDO_VAR_CURRENT_LOCALID_X:
            case SU_PSEUDO_VAR_CURRENT_LOCALID_Y:
            case SU_PSEUDO_VAR_CURRENT_LOCALID_Z:
            case SU_PSEUDO_VAR_CURRENT_GROUPID_X:
            case SU_PSEUDO_VAR_CURRENT_GROUPID_Y:
            case SU_PSEUDO_VAR_CURRENT_GROUPID_Z:
                o_variableType = L"uint";
                break;

            default:
                // Unexpected value!
                GT_ASSERT(false);
                break;
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        suIKernelDebuggingManager::getPseudoVariableMembers
// Description: Gets the "members" of a pseudo variable
// Author:      Uri Shomroni
// Date:        28/1/2014
// ---------------------------------------------------------------------------
bool suIKernelDebuggingManager::getPseudoVariableMembers(const gtString& variableName, const int globalWorkGeometry[10], gtVector<gtString>& o_memberNames)
{
    bool retVal = false;

    suKernelDebuggingPseudoVariable varId = suPseudoVariableIDFromName(variableName);

    // Shorthand for input values:
    const int& workDim = globalWorkGeometry[0];

    // Validate variables according to the work geometry:
    switch (varId)
    {
        case SU_PSEUDO_VAR_DISPATCH_DETAILS:
        case SU_PSEUDO_VAR_NDRANGE:
        case SU_PSEUDO_VAR_WORK_DIM:
        case SU_PSEUDO_VAR_GLOBAL_SIZE:
        case SU_PSEUDO_VAR_GLOBAL_SIZE_X:
        case SU_PSEUDO_VAR_LOCAL_SIZE:
        case SU_PSEUDO_VAR_LOCAL_SIZE_X:
        case SU_PSEUDO_VAR_GLOBAL_OFFSET:
        case SU_PSEUDO_VAR_GLOBAL_OFFSET_X:
        case SU_PSEUDO_VAR_GLOBAL_WORKGROUPS:
        case SU_PSEUDO_VAR_GLOBAL_WORKGROUPS_X:
        case SU_PSEUDO_VAR_TOTAL_WORKGROUPS:
        case SU_PSEUDO_VAR_TOTAL_WORKITEMS:
        case SU_PSEUDO_VAR_CURRENT_WORKITEM:
        case SU_PSEUDO_VAR_CURRENT_GLOBALID:
        case SU_PSEUDO_VAR_CURRENT_GLOBALID_X:
        case SU_PSEUDO_VAR_CURRENT_LOCALID:
        case SU_PSEUDO_VAR_CURRENT_LOCALID_X:
        case SU_PSEUDO_VAR_CURRENT_GROUPID:
        case SU_PSEUDO_VAR_CURRENT_GROUPID_X:
            retVal = true;
            break;

        case SU_PSEUDO_VAR_GLOBAL_SIZE_Y:
        case SU_PSEUDO_VAR_LOCAL_SIZE_Y:
        case SU_PSEUDO_VAR_GLOBAL_OFFSET_Y:
        case SU_PSEUDO_VAR_GLOBAL_WORKGROUPS_Y:
        case SU_PSEUDO_VAR_CURRENT_GLOBALID_Y:
        case SU_PSEUDO_VAR_CURRENT_LOCALID_Y:
        case SU_PSEUDO_VAR_CURRENT_GROUPID_Y:
            retVal = 1 < workDim;
            break;

        case SU_PSEUDO_VAR_GLOBAL_SIZE_Z:
        case SU_PSEUDO_VAR_LOCAL_SIZE_Z:
        case SU_PSEUDO_VAR_GLOBAL_OFFSET_Z:
        case SU_PSEUDO_VAR_GLOBAL_WORKGROUPS_Z:
        case SU_PSEUDO_VAR_CURRENT_GLOBALID_Z:
        case SU_PSEUDO_VAR_CURRENT_LOCALID_Z:
        case SU_PSEUDO_VAR_CURRENT_GROUPID_Z:
            retVal = 2 < workDim;
            break;

        case SU_PSEUDO_VAR_NONE:
        default:
            retVal = false;
            break;
    }

    if (retVal)
    {
        switch (varId)
        {
            case SU_PSEUDO_VAR_DISPATCH_DETAILS:
                o_memberNames.push_back(SU_PSEUDO_VAR_NDRANGE_NAME);
                o_memberNames.push_back(SU_PSEUDO_VAR_CURRENT_WORKITEM_NAME);
                break;

            case SU_PSEUDO_VAR_NDRANGE:
                o_memberNames.push_back(SU_PSEUDO_VAR_WORK_DIM_NAME);
                o_memberNames.push_back(SU_PSEUDO_VAR_GLOBAL_SIZE_NAME);
                o_memberNames.push_back(SU_PSEUDO_VAR_LOCAL_SIZE_NAME);
                o_memberNames.push_back(SU_PSEUDO_VAR_GLOBAL_OFFSET_NAME);
                o_memberNames.push_back(SU_PSEUDO_VAR_GLOBAL_WORKGROUPS_NAME);
                o_memberNames.push_back(SU_PSEUDO_VAR_TOTAL_WORKGROUPS_NAME);
                o_memberNames.push_back(SU_PSEUDO_VAR_TOTAL_WORKITEMS_NAME);
                break;

            case SU_PSEUDO_VAR_CURRENT_WORKITEM:
                o_memberNames.push_back(SU_PSEUDO_VAR_CURRENT_GLOBALID_NAME);
                o_memberNames.push_back(SU_PSEUDO_VAR_CURRENT_LOCALID_NAME);
                o_memberNames.push_back(SU_PSEUDO_VAR_CURRENT_GROUPID_NAME);
                break;

            case SU_PSEUDO_VAR_GLOBAL_SIZE:
            case SU_PSEUDO_VAR_LOCAL_SIZE:
            case SU_PSEUDO_VAR_GLOBAL_OFFSET:
            case SU_PSEUDO_VAR_GLOBAL_WORKGROUPS:
            case SU_PSEUDO_VAR_CURRENT_GLOBALID:
            case SU_PSEUDO_VAR_CURRENT_LOCALID:
            case SU_PSEUDO_VAR_CURRENT_GROUPID:
            {
                if (1 < workDim)
                {
                    o_memberNames.push_back(SU_PSEUDO_VAR_NAME_X);
                    o_memberNames.push_back(SU_PSEUDO_VAR_NAME_Y);

                    if (2 < workDim)
                    {
                        o_memberNames.push_back(SU_PSEUDO_VAR_NAME_Z);
                    }
                }
            }
            break;

            case SU_PSEUDO_VAR_WORK_DIM:
            case SU_PSEUDO_VAR_GLOBAL_SIZE_X:
            case SU_PSEUDO_VAR_GLOBAL_SIZE_Y:
            case SU_PSEUDO_VAR_GLOBAL_SIZE_Z:
            case SU_PSEUDO_VAR_LOCAL_SIZE_X:
            case SU_PSEUDO_VAR_LOCAL_SIZE_Y:
            case SU_PSEUDO_VAR_LOCAL_SIZE_Z:
            case SU_PSEUDO_VAR_GLOBAL_OFFSET_X:
            case SU_PSEUDO_VAR_GLOBAL_OFFSET_Y:
            case SU_PSEUDO_VAR_GLOBAL_OFFSET_Z:
            case SU_PSEUDO_VAR_GLOBAL_WORKGROUPS_X:
            case SU_PSEUDO_VAR_GLOBAL_WORKGROUPS_Y:
            case SU_PSEUDO_VAR_GLOBAL_WORKGROUPS_Z:
            case SU_PSEUDO_VAR_TOTAL_WORKGROUPS:
            case SU_PSEUDO_VAR_TOTAL_WORKITEMS:
            case SU_PSEUDO_VAR_CURRENT_GLOBALID_X:
            case SU_PSEUDO_VAR_CURRENT_GLOBALID_Y:
            case SU_PSEUDO_VAR_CURRENT_GLOBALID_Z:
            case SU_PSEUDO_VAR_CURRENT_LOCALID_X:
            case SU_PSEUDO_VAR_CURRENT_LOCALID_Y:
            case SU_PSEUDO_VAR_CURRENT_LOCALID_Z:
            case SU_PSEUDO_VAR_CURRENT_GROUPID_X:
            case SU_PSEUDO_VAR_CURRENT_GROUPID_Y:
            case SU_PSEUDO_VAR_CURRENT_GROUPID_Z:
                // No members:
                break;

            default:
                // Unexpected value!
                GT_ASSERT(false);
                break;
        }
    }

    return retVal;
}
