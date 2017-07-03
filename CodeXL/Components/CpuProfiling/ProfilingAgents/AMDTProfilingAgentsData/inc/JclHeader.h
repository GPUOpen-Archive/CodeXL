//==================================================================================
// Copyright (c) 2013-2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file JclHeader.h
/// \brief This file contains an interface to read JCL file for Java code profiling.
///
//==================================================================================

#ifndef _JCLHEADER_H_
#define _JCLHEADER_H_

#include <AMDTOSWrappers/Include/osOSDefinitions.h>

//
//    Macros
//
#define JCL_HEADER_SIGNATURE    "_AMDCODEXL_JCL_"
#define MAX_APP_NAME_SIZE       4096

//
//   Globals
//
const unsigned int JCL_VERSION_0 = 0x00;
const unsigned int JCL_VERSION = 0x01;
const char ZERO_PAD = 0x00;

// JIT Record types
//
enum JitRecordType
{
    JIT_LOAD   = 0,
    JIT_UNLOAD = 1
};

// JclHeader
// The header of the JCL file:
//

struct JclHeaderInfo
{
    char     signature[16]; // this always be _AMDCODEXL_JCL_
    gtUInt32 version;       // Major[15-0] , Minor[31-16]
    gtInt32  processID;
    gtUInt32 numRecords;
    gtInt32  is32Bit;       // should this is bool ?
};

struct JclHeader
{
    JclHeaderInfo  hdrInfo;
    wchar_t        appName[OS_MAX_PATH];

    JclHeader()
    {
        hdrInfo.version    = JCL_VERSION;
        hdrInfo.numRecords = 0;
        hdrInfo.is32Bit    = 0;

        memset(appName, 0, OS_MAX_PATH * sizeof(wchar_t));
    }
};


// JitLoadRecord
// Structure to hold the JIT_LOAD record
//
struct JitLoadRecord
{
    gtUInt64   loadTimestamp;
    gtUInt64   blockStartAddr;
    gtUInt64   blockEndAddr;
    gtUInt32   threadID;

    wchar_t  classFunctionName[OS_MAX_PATH];
    wchar_t  jncFileName[OS_MAX_PATH];
    wchar_t  srcFileName[OS_MAX_PATH];  // java/clr source file name

    JitLoadRecord()
    {
        memset(classFunctionName, 0, OS_MAX_PATH * sizeof(wchar_t));
        memset(jncFileName, 0, OS_MAX_PATH * sizeof(wchar_t));
        memset(srcFileName, 0, OS_MAX_PATH * sizeof(wchar_t));
    }
};


// JitUnloadRecord
// Structure to hold the JIT_UNLOAD record
//
struct JitUnloadRecord
{
    gtUInt64  unloadTimestamp;
    gtUInt64  blockStartAddr;
};

#endif // _JCLHEADER_H_
