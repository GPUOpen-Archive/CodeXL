//==================================================================================
// Copyright (c) 2013-2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file CelHeader.h
///
//==================================================================================

#ifndef _CELHEADER_H_
#define _CELHEADER_H_

#include <AMDTOSWrappers/Include/osOSDefinitions.h>
#pragma warning( push )
#pragma warning( disable : 4091)
#pragma warning( disable : 4458)
#pragma warning( disable : 4996)
#include <cor.h>
#include <corprof.h>
#pragma warning( pop )

// CEL_VERSION == 0x04; modified WriteModuleAttachedToAssembly format.
const unsigned int CEL_VERSION = 0x04;
// old format;
const unsigned int CEL_VERSION_03 = 0x03;

#define CEL_HEADER_SIGNATURE "AMDCACEL"

#define NUM_BLOCK_OFFSET    12

enum CLREventType
{
    evInvalidCLREvent = 0,
    evInitialize,
    evShutdown,
    evAppDomainCreationStarted,
    evAppDomainCreationFinished,
    evAppDomainShutdownStarted,
    evAppDomainShutdownFinished,
    evAssemblyLoadStarted,
    evAssemblyLoadFinished,
    evAssemblyUnloadStarted,
    evAssemblyUnloadFinished,
    evModuleLoadStarted,
    evModuleLoadFinished,
    evModuleUnloadStarted,
    evModuleUnloadFinished,
    evModuleAttachedToAssembly,
    evClassLoadStarted,
    evClassLoadFinished,
    evClassUnloadStarted,
    evClassUnloadFinished,
    evFunctionUnloadStarted,
    evJITCompilationStarted,
    evJITCompilationFinished,
    evJITCachedFunctionSearchStarted,
    evJITCachedFunctionSearchFinished,
    evJITFunctionPitched,
    evJITInlining,
    evThreadCreated,
    evThreadDestroyed,
    evThreadAssignedToOSThread,
    evRemotingClientInvocationStarted,
    evRemotingClientSendingMessage,
    evRemotingClientReceivingReply,
    evRemotingClientInvocationFinished,
    evRemotingServerReceivingMessage,
    evRemotingServerInvocationStarted,
    evRemotingServerInvocationReturned,
    evRemotingServerSendingReply,
    evUnmanagedToManagedTransition,
    evManagedToUnmanagedTransition,
    evRuntimeSuspendStarted,
    evRuntimeSuspendFinished,
    evRuntimeSuspendAborted,
    evRuntimeResumeStarted,
    evRuntimeResumeFinished,
    evRuntimeThreadSuspended,
    evRuntimeThreadResumed,
    evMovedReferences,
    evObjectAllocated,
    evObjectsAllocatedByClass,
    evObjectReferences,
    evRootReferences,
    evExceptionThrown,
    evExceptionSearchFunctionEnter,
    evExceptionSearchFunctionLeave,
    evExceptionSearchFilterEnter,
    evExceptionSearchFilterLeave,
    evExceptionSearchCatcherFound,
    evExceptionOSHandlerEnter,
    evExceptionOSHandlerLeave,
    evExceptionUnwindFunctionEnter,
    evExceptionUnwindFunctionLeave,
    evExceptionUnwindFinallyEnter,
    evExceptionUnwindFinallyLeave,
    evExceptionCatcherEnter,
    evExceptionCatcherLeave,
    evCOMClassicVTableCreated,
    evCOMClassicVTableDestroyed,
    evExceptionCLRCatcherFound,
    evExceptionCLRCatcherExecute,
};

struct CelHeader
{
    char        signature[8];  // this always be AMDCACEL
    gtUInt32    version;       // version:[15-0] Major, version:[31-16] minor
    gtUInt32    num_Blocks;
    gtUInt64    processId;
    gtUInt32    b32_bit;       // if its 32-bit
};


struct AppDomainRecord
{
    AppDomainID domainId;
    gtUInt64    loadTime;
    gtUInt64    unloadTime;
    wchar_t     domainName[OS_MAX_FNAME];
};

struct AssemblyRecord
{
    AssemblyID  asmId;
    AppDomainID appId;
    gtUInt64    loadTime;
    gtUInt64    unloadTime;
    wchar_t     asmName[OS_MAX_FNAME];
};

struct ModuleRecord
{
    ModuleID    modId;
    AssemblyID  asmId;
    gtUInt64    loadTime;
    gtUInt64    unloadTime;
    wchar_t     modName[OS_MAX_FNAME];
};

struct ClassRecord
{
    ClassID     classId;
    ModuleID    modId;
    gtUInt64    loadTime;
    gtUInt64    unloadTime;
    wchar_t     className[OS_MAX_FNAME];
};

struct FunctionRecord
{
    ModuleID    modId;
    FunctionID  funcId;
    wchar_t     className[OS_MAX_FNAME];
    wchar_t     funcName[OS_MAX_FNAME];
    wchar_t     jncFileName[OS_MAX_FNAME];
    gtUInt64    loadTime;
    gtUInt64    unloadTime;
    gtUInt64    jitLoadAddr;
    unsigned int codeSize;
};

#endif // _CELHEADER_H_
