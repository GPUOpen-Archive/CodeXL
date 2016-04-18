//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gaPrivateAPIFunctions.h
///
//==================================================================================

//------------------------------ gaPrivateAPIFunctions.h ------------------------------

#ifndef __GAPRIVATEAPIFUNCTIONS
#define __GAPRIVATEAPIFUNCTIONS

// -----------------------------------------------------------------------------------------
//  This file contains proxy functions (functions that have implementations in the spy side)
//  but are private
// -----------------------------------------------------------------------------------------

// Forward declarations:
class osSocket;
class osPortAddress;
class apCounterInfo;
struct apDetectedErrorParameters;

// Infra:
#include <AMDTBaseTools/Include/gtString.h>
#include <AMDTBaseTools/Include/gtVector.h>
#include <AMDTAPIClasses/Include/apContextID.h>
#include <AMDTAPIClasses/Include/apCounterID.h>
#include <AMDTOSWrappers/Include/osOSDefinitions.h>
#include <AMDTOSAPIWrappers/Include/oaOpenGLIncludes.h>
#include <AMDTAPIClasses/Include/apBreakReason.h>

// Local:
#include <AMDTAPIClasses/Include/apAPIFunctionId.h>


// API initialization and termination:
void gaGenerateUniquePipeName(gtString& pipeName);
bool gaInitializeAPIConnection(const osPortAddress& spyAPIPortAddress, const osPortAddress& incomingEventsPortAddress);
bool gaInitializeAPIConnection(const gtString& spyAPIPipeName, const gtString& incomingEventsPipeName);

// Spies API Sockets:
osSocket& gaSpiesAPISocket();

// Direct API calls:
osProcedureAddress64 gaBeforeDirectAPIFunctionExecution(apAPIFunctionId functionToBeCalled);

// Breakpoints:
bool gaGetBreakReason(apBreakReason& breakReason);
bool gaGetCurrentOpenGLError(GLenum& openGLError);

// Detected errors:
bool gaGetDetectedErrorParameters(apDetectedErrorParameters& detectedErrorParameters);

// Context data snapshot:
bool gaUpdateContextDataSnapshot(int contextId);
void gaWaitForUpdateContextDataSnapshot();
bool gaUpdateOpenCLContextDataSnapshot(int contextId);

// Spy performance counters:
bool gaGetSpyPerformanceCountersValues(double*& pValuesArray, int& valuesArraySize);

// Remote OS performance counters:
bool gaGetRemoteOSPerformanceCountersValues(double*& pValuesArray, int& valuesArraySize);

// iPhone performance counters:
bool gaAddSupportediPhonePerformanceCounter(int counterIndex, const gtString& counterName);
bool gaInitializeiPhonePerformanceCountersReader();
bool gaGetiPhonePerformanceCountersValues(double*& pValuesArray, int& valuesArraySize);

// ATI performance counters:
bool gaGetATIPerformanceCountersValues(double*& pValuesArray, int& valuesArraySize);
bool gaActivateATIPerformanceCounters(const gtVector<apCounterActivationInfo>& counterIDsVec);

// AMD OpenCL performance counters:
bool gaGetAMDOpenCLPerformanceCountersValues(double*& pValuesArray, int& valuesArraySize);
bool gaActivateAMDOpenCLPerformanceCounters(const gtVector<apCounterActivationInfo>& counterIDsVec);

// Event forwarding:
bool gaCreateEventForwardingTCPConnection(const osPortAddress& portAddress);
bool gaCreateEventForwardingPipeConnection(const gtString& eventsPipeName);

// OpenCL Queue performance counters:
bool gaGetOpenCLQueuePerformanceCountersValues(double*& pValuesArray, int& valuesArraySize);

#endif  // __GAPRIVATEAPIFUNCTIONS
