//==================================================================================
// Copyright (c) 2014-2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file AMDTJITProfileAgent.h
/// \brief Profile Agent to profile the dynamically generated code like.
///
//==================================================================================

#ifndef _AMDTJITPROFILEAGENT_H_
#define _AMDTJITPROFILEAGENT_H_

#ifdef __cplusplus
extern "C"
{
#endif

//
//    Macros
//

#if defined (_WIN32)
#define AMDT_JITPROFILE_API    __declspec(dllexport)
#else
#define AMDT_JITPROFILE_API
#endif // defined(_WIN32)


// Return Codes
#define AMDT_JITPROFILE_SUCCESS                  (0)
#define AMDT_JITPROFILE_INITIALIZED              (1)
#define AMDT_JITPROFILE_ENABLED                  (2)

#define AMDT_JITPROFILE_FAILED                  (-1)
#define AMDT_JITPROFILE_ERROR_INVALIDARG        (-2)
#define AMDT_JITPROFILE_ERROR_NOT_INITIALIZED   (-3)
#define AMDT_JITPROFILE_ERROR_DISABLED          (-4)


// variable type for the return codes;
typedef int AMDTJITProfileStatus;


typedef struct _AMDTJitProfileEnv
{
    unsigned int  processId;            // Process ID of the JIT'ed process;
    wchar_t*      pApplicationName;     // Application name;
    wchar_t*      pModuleName;          // Module Name for this dynamic code;
    void*         pReserved;            // UNUSED and should be NULL;
} AMDTJitProfileEnv;


// Source Line Table
typedef struct _AMDTJitSrcLineTable
{
    unsigned int  offset;
    unsigned int  lineNumber;
} AMDTJitSrcLineTable;


// AMDTJitMethodLoad - JITed Method Load details
//
// pCodeAddr     :  Virtual Address at which the JITed Method is loaded;
// codeSize      :  Size of the JITed code blob;
// pMethodName   :  JITed Method Name;
// pClassName    :  JITed Class Name; Used to construct; Can be NULL;
// pSrcFileName  :  Source file path of the JITed function; Can be NULL;
// lineTableSize :  Number of entries in the Source Line Table;
// pLineTable    :  Source Line Table;
// pInfo         :  Unused. Will be used for Inlined Functions support in future;
//
typedef struct _AMDTJitMethodLoad
{
    void*                 pCodeAddr;      // Virtual Address at which the code is loaded
    unsigned int          codeSize;       // Size of the code blob
    wchar_t*              pMethodName;    // Method name
    wchar_t*              pClassName;     // Class name; Can be NULL
    wchar_t*              pSrcFileName;   // Src file path; Can be NULL
    unsigned int          lineTableSize;  // Size of the line table
    AMDTJitSrcLineTable*  pLineTable;     // Source line table info; Can be NULL
    void*                 pInfo;          // Future use; inline info?
} AMDTJitMethodLoad;


// AMDTJitMethodUnload - JITed Method Unload details
//
// pCodeAddr     :  Virtual Address at which the JITed Method is loaded;
// pMethodName   :  JITed Method Name;
// pClassName    :  JITed Class Name; Used to construct; Can be NULL;
//
typedef struct _AMDTJitMethodUnload
{
    void*     pCodeAddr;      // Virtual address at which the code is loaded
    wchar_t*  pMethodName;    // Method name
    wchar_t*  pClassName;     // Class signature
} AMDTJitMethodUnload;


//
//    PUBLIC APIs
//


// AMDTJitProfileInitialize()
//
// Initializes the JIT profile data collection.
// Creates the Temporary folder and starts constructing the JCL/JNC file structures for storing
// profile data. These profile-data-files will be consumed by CodeXL while attributing/reporting
// the profile samples.
//
// Should be the first API to be called by the JavaScript runtime.
//
//   On success, returns AMDT_JITPROFILE_SUCCESS
//   if internal errors, returns AMDT_JITPROFILE_FAILED
//
AMDT_JITPROFILE_API
int AMDTJitProfileInitialize(AMDTJitProfileEnv* pEnv);


// AMDTJitProfileFinalize()
//
// Finalizes the JIT profile data collection.
// Writes all the profile data and closes the file handles.
//
// Once the JavaScript runtime decides to stop the profile, this API should be called to stop
// collecting the metadata about the JIted functions.
//
//   On success, returns AMDT_JITPROFILE_SUCCESS
//   if internal errors, returns AMDT_JITPROFILE_FAILED
//

AMDT_JITPROFILE_API
int AMDTJitProfileFinalize(void);


// AMDTJitProfileStatus
//
// Get the status of the JIT profile
//
// if the JIT profile is enabled, returns AMDT_JITPROFILE_ENABLED
// otherwise, returns AMDT_JITPROFILE_ERROR_DISABLED
//
AMDT_JITPROFILE_API
int AMDTJitProfileStatus(void);


// AMDTJitProfileMethodLoad
//
// API to be called whenever a method is loaded
//
// pEnv        :  Currently UNNUSED and can be NULL
// pMethodInfo :  Contains information about the JITed method that is being loaded
//
// On success, returns AMDT_JITPROFILE_SUCCESS
// if pMethodInfo is NULL, Returns AMDT_JITPROFILE_ERROR_INVALIDARG
// if profile is not enabled, returns AMDT_JITPROFILE_ERROR_DISABLED
// if internal errors, returns AMDT_JITPROFILE_FAILED
//
AMDT_JITPROFILE_API
int AMDTJitProfileMethodLoad(AMDTJitMethodLoad* pMethodInfo);


// AMDTJitProfileMethodUnload
//
// API to be called when a method is unloaded
//
// pEnv        :  Currently UNNUSED and can be NULL
// pMethodInfo :  Contains information about the JITed method that is being unloaded
//
//   On success, returns AMDT_JITPROFILE_SUCCESS
//   if pMethodInfo is NULL, Returns AMDT_JITPROFILE_ERROR_INVALIDARG
//   if profile is not enabled, returns AMDT_JITPROFILE_ERROR_DISABLED
//   if internal errors, returns AMDT_JITPROFILE_FAILED
//
AMDT_JITPROFILE_API
int AMDTJitProfileMethodUnload(AMDTJitMethodUnload* pMethodInfo);

#ifdef __cplusplus
}
#endif

#endif // _AMDTJITPROFILEAGENT_H_