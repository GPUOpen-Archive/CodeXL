//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gdEventStringBuilder.h
///
//==================================================================================

//------------------------------ gdEventStringBuilder.h ------------------------------

#ifndef __GDEVENTSTRINGBUILDER_H
#define __GDEVENTSTRINGBUILDER_H

// Infra:
#include <AMDTBaseTools/Include/gtString.h>
#include <AMDTAPIClasses/Include/Events/apApiConnectionEndedEvent.h>
#include <AMDTAPIClasses/Include/Events/apApiConnectionEstablishedEvent.h>
#include <AMDTAPIClasses/Include/Events/apBreakpointHitEvent.h>
#include <AMDTAPIClasses/Include/Events/apComputeContextCreatedEvent.h>
#include <AMDTAPIClasses/Include/Events/apComputeContextDeletedEvent.h>
#include <AMDTAPIClasses/Include/Events/apDebuggedProcessDetectedErrorEvent.h>
#include <AMDTAPIClasses/Include/Events/apDebuggedProcessOutputStringEvent.h>
#include <AMDTAPIClasses/Include/Events/apExceptionEvent.h>
#include <AMDTAPIClasses/Include/Events/apGDBOutputStringEvent.h>
#include <AMDTAPIClasses/Include/Events/apGDBErrorEvent.h>
#include <AMDTAPIClasses/Include/Events/apGLDebugOutputMessageEvent.h>
#include <AMDTAPIClasses/Include/Events/apMemoryLeakEvent.h>
#include <AMDTAPIClasses/Include/Events/apModuleLoadedEvent.h>
#include <AMDTAPIClasses/Include/Events/apModuleUnloadedEvent.h>
#include <AMDTAPIClasses/Include/Events/apOpenCLErrorEvent.h>
#include <AMDTAPIClasses/Include/Events/apOpenCLProgramCreatedEvent.h>
#include <AMDTAPIClasses/Include/Events/apOpenCLProgramDeletedEvent.h>
#include <AMDTAPIClasses/Include/Events/apOpenCLProgramBuildEvent.h>
#include <AMDTAPIClasses/Include/Events/apOpenCLProgramBuildFailedWithDebugFlagsEvent.h>
#include <AMDTAPIClasses/Include/Events/apOpenCLQueueCreatedEvent.h>
#include <AMDTAPIClasses/Include/Events/apOpenCLQueueDeletedEvent.h>
#include <AMDTAPIClasses/Include/Events/apOutputDebugStringEvent.h>
#include <AMDTAPIClasses/Include/Events/apDebuggedProcessCreatedEvent.h>
#include <AMDTAPIClasses/Include/Events/apRenderContextCreatedEvent.h>
#include <AMDTAPIClasses/Include/Events/apRenderContextDeletedEvent.h>
#include <AMDTAPIClasses/Include/Events/apThreadCreatedEvent.h>
#include <AMDTAPIClasses/Include/Events/apThreadTerminatedEvent.h>

// Local:
#include <AMDTGpuDebuggingComponents/Include/gdGDebuggerAppDLLBuild.h>


// ----------------------------------------------------------------------------------
// Class Name:           GD_API gdEventStringBuilder
// General Description:     Build display string for events. Used to create the string
//                          for stand alone and package applications
// Author:               Gilad Yarnitzky
// Creation Date:        20/2/2011
// ----------------------------------------------------------------------------------
class GD_API gdEventStringBuilder
{
public:
    gdEventStringBuilder()  {};
    ~gdEventStringBuilder() {};

    // Build a string from an event:
    void buildEventString(const apEvent& eve, gtString& eventMessage);

protected:

    // Internal functions that build a string from an event:
    void onProcessCreationString(const apDebuggedProcessCreatedEvent& processCreatedEvent, gtString& messageString);
    void onModuleLoadedString(const apModuleLoadedEvent& dllLoadedEvent, gtString& messageString);
    void onModuleUnloadedString(const apModuleUnloadedEvent& dllUnloadedEvent, gtString& messageString);
    void onExceptionString(const apExceptionEvent& exceptionEvent, gtString& messageString);
    void onOutputDebugString(const apOutputDebugStringEvent& outputDebugStringEvent, gtString& messageString);
    void onOutputGDBString(const apGDBOutputStringEvent& outputGDBStringEvent, gtString& messageString);
    void onMemoryLeakEventString(const apMemoryLeakEvent& memoryLeakEvent, gtString& eventString);
    void onGDBErrorString(const apGDBErrorEvent& gdbErrorEvent, gtString& messageString);
    void onDebuggedProceessOutputString(const apDebuggedProcessOutputStringEvent& outputStringEvent, gtString& messageString);
    void onBreakHitString(const apBreakpointHitEvent& breakpointEvent, gtString& messageString);
    void onDebuggedProcessErrorEventString(const apDebuggedProcessDetectedErrorEvent& errorEvent, gtString& messageString);
    void onDebuggedProcessThreadCreatedEventString(const apThreadCreatedEvent& threadCreatedEvent, gtString& messageString);
    void onDebuggedProcessThreadTerminatedEventString(const apThreadTerminatedEvent& threadTerminatedEvent, gtString& messageString);
    void onAPIConnectionEstablishedEventString(const apApiConnectionEstablishedEvent& apiConnectionEstablishedEvent, gtString& messageString);
    void onAPIConnectionEndedEventString(const apApiConnectionEndedEvent& apiConnectionEndedEvent, gtString& messageString);
    void onRenderContextCreatedEventString(const apRenderContextCreatedEvent& renderContextCreatedEvent, gtString& messageString);
    void onRenderContextDeletedEventString(const apRenderContextDeletedEvent& renderContextDeletedEvent, gtString& messageString);
    void onComputeContextCreatedEventString(const apComputeContextCreatedEvent& computeContextCreatedEvent, gtString& messageString);
    void onComputeContextDeletedEventString(const apComputeContextDeletedEvent& computeContextDeletedEvent, gtString& messageString);
    void onDebugOutputMessageEventString(const apGLDebugOutputMessageEvent& debugOutputMessageEvent, gtString& messageString);
    void onOpenCLErrorMessageEventString(const apOpenCLErrorEvent& clErrorEvent, gtString& messageString);
    void onOpenCLQueueCreatedEventString(const apOpenCLQueueCreatedEvent& queueCreatedEvent, gtString& messageString);
    void onOpenCLQueueDeletedEventString(const apOpenCLQueueDeletedEvent& queueDeletedEvent, gtString& messageString);
    void onOpenCLProgramCreatedEventString(const apOpenCLProgramCreatedEvent& programCreatedEvent, gtString& messageString);
    void onOpenCLProgramDeletedEventString(const apOpenCLProgramDeletedEvent& programDeletedEvent, gtString& messageString);
    void onOpenCLProgramBuildEventString(const apOpenCLProgramBuildEvent& programBuildEvent, gtString& messageString);
    void onOpenCLProgramBuildFailedWithDebugFlagsEventString(const apOpenCLProgramBuildFailedWithDebugFlagsEvent& programBuildFailedEvent, gtString& messageString);

};

#endif //__GDEVENTSTRINGBUILDER_H

