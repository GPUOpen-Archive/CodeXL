//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file suSpyAPIFunctions.h
///
//==================================================================================

//------------------------------ suSpyAPIFunctions.h ------------------------------

#ifndef __SUSPYAPIFUNCTIONS_H
#define __SUSPYAPIFUNCTIONS_H

// Forward decelerations:
class osSocket;
class apEvent;

// Infra:
#include <AMDTAPIClasses/Include/apAPIConnectionType.h>
#include <AMDTAPIClasses/Include/apAPIFunctionId.h>

// Local:
#include <AMDTServerUtilities/Include/suSpiesUtilitiesDLLBuild.h>


// ----------------- For spies usage -----------------

// The type of an API function stub:
typedef void (*suAPIStubFunction)(osSocket& apiSocket);

SU_API bool suRegisterAPIFunctionStub(apAPIFunctionId functionId, suAPIStubFunction apiStubFunction);
SU_API void suRegisterAPIConnectionAsActive(apAPIConnectionType apiType);
SU_API void suRegisterAPIConnectionAsInactive(apAPIConnectionType apiType);
SU_API bool suMainThreadStarsInitializingAPIConnection(apAPIConnectionType apiType);
SU_API void suMainThreadEndedInitializingAPIConnection(apAPIConnectionType apiType);
SU_API void suMarkAPIConnectionAsInitialized(apAPIConnectionType apiType);
SU_API bool suIsAPIConnectionInitialized(apAPIConnectionType apiType);
SU_API osSocket* suSpiesAPISocket();
SU_API bool suForwardEventToClient(const apEvent& eve);
SU_API bool suIsAPIThreadRunning();
SU_API void suBeforeEnteringTerminationAPILoop();
SU_API void suRunTerminationAPILoop();
SU_API bool suIsTerminationInitiatedByAPI();
SU_API void suBeforeDirectFunctionExecution();
SU_API void suAfterDirectFunctionExecution();
SU_API bool suIsDuringDirectFunctionExecution();
SU_API void suSupressSpyEvents(bool supressSpyEvents);
SU_API bool suAreSpyEventsSuppressed();
SU_API void suSendSpyProgressEvent(int progress = 1);


// ----------------- For internal usage -----------------

void suInitializeSpyAPIFunctionsInfra();
bool suExecuteAPIFunctionStub(apAPIFunctionId functionId, osSocket& apiSocket);
bool suWaitForDirectFunctionExecutionEnd();
bool suIsInProcessTerminationAPILoop();
void suSetTerminationInitiatedByAPI();
void suHandleAPILoopTerminatingCalls(apAPIFunctionId executedFunctionId, bool& shouldAPILoopContinueRunning);
void suHandleAPILoopBehaviourDuringDebuggedProcessTermination(apAPIFunctionId executedFunctionId, bool& shouldAPILoopContinueRunning);

#endif //__SUSPYAPIFUNCTIONS_H

