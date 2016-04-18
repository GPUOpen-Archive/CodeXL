//==================================================================================
// Copyright (c) 2013-2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file JvmtiProfileAgent.h
/// \brief JVMTI Java Profile Agent.
///
//==================================================================================

#ifndef _JVMTIPROFILEAGENT_H_
#define _JVMTIPROFILEAGENT_H_

#include <AMDTOSWrappers/Include/osOSDefinitions.h>
#include <AMDTProfilingAgentsData/inc/JclWriter.h>
#include <jvmti.h>
#include <jvmticmlr.h>
#include <BytecodeToSource.h>

//
//    Macros
//

#define UNKNOWNSOURCEFILE   "UnknownJITSource"
#define UNKNOWNCLASSNAME    "UnknownClassName"
#define UNKNOWNFUNCTION     "UnknownFuncName"

#define JVMTIOS_MAX_PATH    260
#define TMP_MAX_PATH        1024

#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
    #define WIDE_STR_FORMAT L"%s"
    #define CSTR_FORMAT L"%S"
#else
    #define WIDE_STR_FORMAT L"%S"
    #define CSTR_FORMAT L"%s"
#endif

//
//    Typedefs
//
typedef struct
{
    char*  pClassSig;
    char*  pMethodName;
    char*  pSrcFile;
} methodInfo;

//
//    Globals
//

extern JclWriter*   gJCLWriter;
extern osProcessId  gJvmPID;
extern std::wstring gJvmCmd;
extern std::wstring gSampleDir;
extern std::wstring gProfileDataDir;

//
//    Helper Routines
//

void GetCmdline();

#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
    int RecursiveMkdir(const wchar_t* pDir);
#endif

// Write the JNC file for compiled code
void JNICALL WriteJncFile(jvmtiEnv*                    jvmti_env,
                          jmethodID                    method,
                          jint                         code_size,
                          const void*                  code_addr,
                          jint                         map_length,
                          const jvmtiAddrLocationMap*  map,
                          std::wstring*                jncFileName,
                          methodInfo*                  mInfo,
                          const void*                  compile_info);

// Write the JNC file for dynamically generated code
int WriteNativeToJncFile(const char*    name,
                         const void*    code_addr,
                         jint           code_size,
                         std::wstring*  jncFileName);

#endif // _JVMTIPROFILEAGENT_H_
