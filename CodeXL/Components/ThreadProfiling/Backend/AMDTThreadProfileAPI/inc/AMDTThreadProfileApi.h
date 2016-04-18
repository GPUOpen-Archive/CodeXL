//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file AMDTThreadProfileApi.h
///
//==================================================================================

#ifndef _AMDTTHREADPROFILEAPI_H_
#define _AMDTTHREADPROFILEAPI_H_

#include <AMDTDefinitions.h>
#include <AMDTThreadProfileDataTypes.h>

#if defined(_WIN32)
    #if defined(AMDTTHREADPROFILEAPI_EXPORTS)
        #define AMDT_THREADPROFILE_API __declspec(dllexport)
    #else
        #define AMDT_THREADPROFILE_API __declspec(dllexport)
    #endif
#else
    #define AMDT_THREADPROFILE_API
#endif

//
//      DATA COLLECTION APIS
//

// AMDTGetThreadProfileState
//
// This API returns the current state of the thread profiler.
//
// Parameters:
//  pState  -   pointer of type AMDTThreadProfileState, in which the current state will be returned
//
// Returns:
//  AMDT_STATUS_OK          -   On Success
//  AMDT_ERROR_INVALIDARG   -   If pState is NULL
//  AMDT_ERROR_INTERNAL     -   On internal failures
//
AMDT_THREADPROFILE_API
AMDTResult AMDTGetThreadProfileState(AMDTThreadProfileState* pState);

// AMDTIsThreadProfileAvailable
//
// This API returns the current state of the thread profiler.
//
// Parameters:
//  pIsProfileAvailable     -   pointer of type bool, in which the availability of thread profiling
//                              functionality will be returned.
//
// Returns:
//  AMDT_STATUS_OK          -   On Success
//  AMDT_ERROR_INVALIDARG   -   If pIsProfileAvailable is NULL
//  AMDT_ERROR_INTERNAL     -   On internal failures
//
AMDT_THREADPROFILE_API
AMDTResult AMDTIsThreadProfileAvailable(bool* pIsProfileAvailable);


// AMDTSetThreadProfileConfiguration
//
// This API intializes the profile configuration details for thread profiling.
//
// Parameters:
//  events      -   Used to specify types of events to be collected;
//                  Supported events are AMDT_TP_EVENT_TRACE_CSWITCH and AMDT_TP_EVENT_TRACE_CALLSTACK;
//                  Multiple events should be OR'd
//
//  pFileName   -   Absolute path of the profile data file in which the collected
//                  profile data will be saved.
//
// Returns:
//  AMDT_STATUS_OK                          -   On Success
//  AMDT_ERROR_INVALIDARG                   -   If invalid events or filename is passed
//  AMDT_ERROR_INTERNAL                     -   On internal failures
//  AMDT_ERROR_PROFILE_ALREADY_CONFIGURED   -   If the thread profile is already configured
//  AMDT_ERROR_PROFILE_ALREADY_STARTED      -   If the profile run is in progress already
//  AMDT_ERROR_OUTOFMEMORY                  -   On memory failures
//
AMDT_THREADPROFILE_API
AMDTResult AMDTSetThreadProfileConfiguration(AMDTUInt32  events,
                                             const char* pFileName);

// AMDTStartThreadProfile
//
// API to the start thread profiler.
// AMDTSetThreadProfileConfiguration() should have been called before calling this API
//
// Returns:
//  AMDT_STATUS_OK                       -   On Success
//  AMDT_ERROR_INTERNAL                  -   On internal failures
//  AMDT_ERROR_PROFILE_NOT_CONFIGURED    -   If the thread profile is not yet configured
//  AMDT_ERROR_PROFILE_ALREADY_STARTED   -   If the profile run is in progress already
//
AMDT_THREADPROFILE_API
AMDTResult AMDTStartThreadProfile();


// AMDTStopThreadProfile
//
// API to the stop thread profiler.
// AMDTSetThreadProfileConfiguration() and AMDTStartThreadProfile() should have been called
// before calling this API
//
// Returns:
//  AMDT_STATUS_OK                      -   On Success
//  AMDT_ERROR_INTERNAL                 -   On internal failures
//  AMDT_ERROR_PROFILE_NOT_CONFIGURED   -   If the thread profile is not yet configured
//  AMDT_ERROR_PROFILE_NOT_STARTED      -   If the profile run is not yet started
//
AMDT_THREADPROFILE_API
AMDTResult AMDTStopThreadProfile();


//
//      DATA ACCESS APIS
//

// AMDTOpenThreadProfile
//
// API to the open thread profile data file
//
// Returns:
//  AMDT_STATUS_OK           -   On Success
//  AMDT_ERROR_INTERNAL      -   On internal failures
//  AMDT_ERROR_INVALIDPATH   -   Invalid file path
//  AMDT_ERROR_INVALIDDATA   -   If the file is not of thread profile data type
//  AMDT_ERROR_INVALIDARG    -   If pPath or ppReaderHandle is NULL
//
AMDT_THREADPROFILE_API
AMDTResult AMDTOpenThreadProfile(const char* pPath,
                                 AMDTThreadProfileDataHandle* pReaderHandle);

// AMDTSetFilterProcesses
//
// API to specify the list of interesting processes for which the thread profile
// data will be consumed by the translate layer. In thread profile, PID filtering is a
// post process task. OS will simply collect data for all the context switches.
//
// If not set all the samples from all the threads will be processed.
//
// Note: There is no parent PID in ETW records; Hence children of the launched process
// cannot be identified While processing ETW records.
//
// Returns:
//  AMDT_STATUS_OK           -   On Success
//  AMDT_ERROR_INTERNAL      -   On internal failures
//  AMDT_ERROR_INVALIDARG    -   If pProcessIds or ppReaderHandle is NULL
//
AMDT_THREADPROFILE_API
AMDTResult AMDTSetFilterProcesses(AMDTThreadProfileDataHandle readerHandle,
                                  AMDTUInt32  count,
                                  AMDTProcessId* pProcessIds);

// AMDTProcessThreadProfileData
//
// API to process the thread profile samples from the specified AMDTThreadProfileDataHandle handler.
// This API will process the process/thread/image records to construct internal data structures.
//
// Has to be called before calling any AMDTGet*() apis.
//
// Parameters
//  readerHandle            - handle returned by AMDTOpenThreadProfile()
//
// Returns:
//  AMDT_STATUS_OK          -   On Success
//  AMDT_ERROR_INTERNAL     -   On internal failures
//  AMDT_ERROR_INVALIDARG   -   If ppReaderHandle is NULL
//  AMDT_ERROR_NODATA       -   If there is no thread profile data
//  AMDT_ERROR_OUTOFMEMORY  -   memory issues
//
AMDT_THREADPROFILE_API
AMDTResult AMDTProcessThreadProfileData(AMDTThreadProfileDataHandle readerHandle);


AMDT_THREADPROFILE_API
AMDTResult AMDTGetNumOfProcessors(AMDTThreadProfileDataHandle readerHandle,
                                  AMDTUInt32* pNbrProcessors);

// AMDTGetProcessIds
//
// API to retrieve the list of processes that has thread profile samples
// in the specified AMDTThreadProfileDataHandle handler.
// Has to be called after AMDTProcessThreadProfileData()
//
// Parameters:
//  readerHandle        - handle returned by AMDTOpenThreadProfile()
//  pNbrProcesses       - pointer to get the number of processes
//  size                - size of the pProcesses
//  pProcesses          - pointer to have the process ids; memory has to be allocated by the caller
//
//  to get number of processes
//      AMDTGetProcessIds(readerHandle, &nbrProcesses, 0, NULL);
//
//  to get the processes list
//      AMDTGetProcessIds(readerHandle, &nbrProcesses, 0, NULL);
//      pProcs = malloc(sizeof(AMDTProcessId) * nbrProcesses);
//      AMDTGetProcessIds(readerHandle, NULL, nbrProcesses, pProcs);
//
// Returns:
//  AMDT_STATUS_OK          -   On Success
//  AMDT_ERROR_INTERNAL     -   On internal failures
//  AMDT_ERROR_INVALIDARG   -   If pNbrProcesses or ppProcesses or ppReaderHandle is NULL
//  AMDT_ERROR_NODATA       -   If there is no thread profile data
//
AMDT_THREADPROFILE_API
AMDTResult AMDTGetProcessIds(AMDTThreadProfileDataHandle readerHandle,
                             AMDTUInt32* pNbrProcesses,
                             AMDTUInt32 size,
                             AMDTProcessId* pProcesses);

// AMDTGetProcessData
//
// API to retrieve process data for the given process Id.
// Has to be called after AMDTProcessThreadProfileData()
//
// Parameters:
//  readerHandle        - handle returned by AMDTOpenThreadProfile()
//  pid                 - PID for which the AMDTProcessData is requested
//  pProcessData        - pointer of type AMDTProcessData; memory has to be allocated by the caller
//
// Returns:
//  AMDT_STATUS_OK          -   On Success
//  AMDT_ERROR_INTERNAL     -   On internal failures
//  AMDT_ERROR_INVALIDARG   -   If pProcessData or ppReaderHandle is NULL
//  AMDT_ERROR_NODATA       -   If there is no thread profile data
//
AMDT_THREADPROFILE_API
AMDTResult AMDTGetProcessData(AMDTThreadProfileDataHandle readerHandle,
                              AMDTProcessId pid,
                              AMDTProcessData* pProcessData);

// AMDTGetThreadIds
//
// API to retrieve the list of threads of the given PID that has thread profile samples
// in the specified AMDTThreadProfileDataHandle handler
//
// Parameters
//  readerHandle        - handle returned by AMDTOpenThreadProfile()
//  pid                 - process id for which the thread details to be retrieved;
//                        if pid == -1, all the processes will be considered
//  pNbrThreads         - pointer to get the number of threads
//  size                - size of the pThreads;
//  pThreads            - pointer to have the thread ids; memory to be allocated/freed by the caller
//
//  to get number of threads in the given pid
//      AMDTGetThreadIds(readerHandle, pid, &nbrThreads, 0, NULL);
//
//  to get the processes list
//      AMDTGetThreadIds(readerHandle, pid, &nbrThreads, 0, NULL);
//      pThreads = malloc(sizeof(AMDTThreadId) * nbrThreads);
//      AMDTGetThreadIds(readerHandle, pid, NULL, nbrThreads, pThreads);
//
// Returns:
//  AMDT_STATUS_OK          -   On Success
//  AMDT_ERROR_INVALIDARG   -   If pNbrThreads or ppThreads or ppReaderHandle is NULL
//  AMDT_ERROR_INTERNAL     -   On internal failures
//  AMDT_ERROR_NODATA       -   If there is no thread profile data
//
AMDT_THREADPROFILE_API
AMDTResult AMDTGetThreadIds(AMDTThreadProfileDataHandle readerHandle,
                            AMDTProcessId pid,
                            AMDTUInt32* pNbrThreads,
                            AMDTUInt32 size,
                            AMDTThreadId* pThreads);

// AMDTGetThreadData
//
// API to retrieve process data for the given threadId.
// Has to be called after AMDTProcessThreadProfileData()
//
// Parameters:
//  readerHandle        - handle returned by AMDTOpenThreadProfile()
//  tid                 - TID for which the AMDTThreadData is requested
//  pThreadData         - pointer of type AMDTThreadData; memory has to be allocated by the caller
//
// Returns:
//  AMDT_STATUS_OK          -   On Success
//  AMDT_ERROR_INVALIDARG   -   If pThreadData or ppReaderHandle is NULL
//  AMDT_ERROR_INTERNAL     -   On internal failures
//  AMDT_ERROR_NODATA       -   If there is no thread profile data
//
AMDT_THREADPROFILE_API
AMDTResult AMDTGetThreadData(AMDTThreadProfileDataHandle readerHandle,
                             AMDTThreadId tid,
                             AMDTThreadData* pThreadData);

// AMDTGetThreadSampleData
//
// API to retrieve thread profile samples for the given threadId.
// Has to be called after AMDTProcessThreadProfileData()
//
// Parameters:
//  readerHandle        - handle returned by AMDTOpenThreadProfile()
//  tid                 - TID for which the AMDTThreadSample is requested
//  pNbrRecords         - pointer of type AMDTUInt32; Number of records is returned
//  ppThreadSampleData  - AMDTThreadSample records will be returned;
//                        memory will be allocated by the API
//
// Returns:
//  AMDT_STATUS_OK          -   On Success
//  AMDT_ERROR_INVALIDARG   -   If pNbrRecords or ppThreadData or ppReaderHandle is NULL
//  AMDT_ERROR_INTERNAL     -   On internal failures
//  AMDT_ERROR_NODATA       -   If there is no thread profile data
//  AMDT_ERROR_OUTOFMEMORY  -   memory issues
//
AMDT_THREADPROFILE_API
AMDTResult AMDTGetThreadSampleData(AMDTThreadProfileDataHandle readerHandle,
                                   AMDTThreadId tid,
                                   AMDTUInt32* pNbrRecords,
                                   AMDTThreadSample** ppThreadSampleData);

// AMDTCloseThreadProfile
// API to close the thread profile data file. This would cleanup all the internal data.
//
// Returns:
//  AMDT_STATUS_OK          -   On Success
//  AMDT_ERROR_INVALIDARG   -   If ppReaderHandle is NULL
//  AMDT_ERROR_INTERNAL     -   On internal failures
//
AMDT_THREADPROFILE_API
AMDTResult AMDTCloseThreadProfile(AMDTThreadProfileDataHandle readerHandle);

AMDT_THREADPROFILE_API
AMDTResult AMDTSetSymbolSearchPath(AMDTThreadProfileDataHandle readerHandle,
                                   const char* pSearchPath,
                                   const char* pServerList,
                                   const char* pCachePath);

AMDT_THREADPROFILE_API
AMDTResult AMDTGetFunctionName(AMDTThreadProfileDataHandle readerHandle,
                               AMDTProcessId pid,
                               AMDTUInt64 pc,
                               char** ppFuncName);

AMDT_THREADPROFILE_API
AMDTResult AMDTGetThreadStateString(AMDTThreadState threadState, char** ppString);

AMDT_THREADPROFILE_API
AMDTResult AMDTGetThreadWaitModeString(AMDTThreadWaitMode waitMode, char** ppString);

AMDT_THREADPROFILE_API
AMDTResult AMDTGetThreadWaitReasonString(AMDTThreadWaitReason waitReason, char** ppString);

#endif //_AMDTTHREADPROFILEAPI_H_