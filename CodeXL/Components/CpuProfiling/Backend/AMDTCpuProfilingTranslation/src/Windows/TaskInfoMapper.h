//==================================================================================
// Copyright (c) 2013-2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file TaskInfoMapper.h
/// \brief The file contains the key and value structures used to WinTaskInfo maps.
///
//==================================================================================

#ifndef _TASKINFOMAPPER_H_
#define _TASKINFOMAPPER_H_

#include <AMDTCpuProfilingTranslation/inc/Windows/TaskInfoInterface.h>
#include <AMDTBaseTools/Include/gtString.h>

//  PARANOIA: We assume that PIDs could possibly be recycled by the OS.
//  For each Process ID, we keep name and time interval.
//
struct ProcessValue
{
    wchar_t     processName[OS_MAX_PATH + 1];     // Process path and name
    DWORD       dProcStartTime;     // Note: times are in milliseconds, relative to start of run.
    DWORD       dProcEndTime;       // process end time. (in milliseconds.)
    bool        bNameConverted;
    bool        bLoadedPeFiles;

    // default constructor
    ProcessValue()
    {
        dProcStartTime = 0;
        dProcEndTime = 0;
        bNameConverted = false;
        bLoadedPeFiles = false;
        processName[0] = L'\0';
    }

    // constructor
    ProcessValue(const wchar_t* ctor_strProcess, int ctor_dStartTime, int ctor_dEndTime) : dProcStartTime(ctor_dStartTime),
        dProcEndTime(ctor_dEndTime)
    {
        wcsncpy(processName, ctor_strProcess, OS_MAX_PATH);
    }
};

// Key for the kernel module map
struct KernelModKey
{
    gtUInt64 keModLoadAddr;      // kernel module load address
    TiTimeType keModLoadTime;      // kernel module load time.

    // constructor
    KernelModKey(gtUInt64 ctor_keModLoadAddr, TiTimeType ctor_keModLoadTime) : keModLoadAddr(ctor_keModLoadAddr),
        keModLoadTime(ctor_keModLoadTime)
    {
    }

    // key comparison rule
    // Note: Please pay attention here, kernel module is sorted by the reversed address order.
    //       In other words, instance with greater load address will stay at the bottom of the map.
    //
    bool operator<(const KernelModKey& other) const
    {
        return (keModLoadAddr > other.keModLoadAddr) ||
               ((keModLoadAddr == other.keModLoadAddr) && (keModLoadTime > other.keModLoadTime));
    }
};

// kernel module map value
struct KernelModValue
{
    gtUInt64 keModEndAddr;   // module end address. // actually it's not end address,
    // it's the start address of next kernel module
    gtUInt64 keModBase;      // module image base
    TiTimeType keModUnloadTime;// module unload time.
    // Note: for the kernel module, we actually don't know exact unload time.
    gtUInt64 keModImageSize; // module image size
    wchar_t keModName[OS_MAX_PATH + 1];   // Kernel module path and name
    bool bNameConverted; // module name is converted.

    bool bLoadedPeFile;
    PeFile* pPeFile;

    gtUInt32 instanceId = 0;

    // default constructor
    KernelModValue() : keModEndAddr(0),
        keModBase(0),
        keModUnloadTime(0),
        keModImageSize(0),
        bNameConverted(false),
        bLoadedPeFile(false),
        pPeFile(NULL)
    {
        keModName[0] = L'\0';
    }

    // constructor
    KernelModValue(gtUInt64 ctor_keModEndAddr, gtUInt64 ctor_keModBase, TiTimeType ctor_keModUnloadTime, wchar_t* ctor_keModName) :
        keModEndAddr(ctor_keModEndAddr),
        keModBase(ctor_keModBase),
        keModUnloadTime(ctor_keModUnloadTime),
        keModImageSize(0),
        bNameConverted(false),
        bLoadedPeFile(false),
        pPeFile(NULL)
    {
        wcsncpy(keModName, ctor_keModName, OS_MAX_PATH);
    }
};


// To enable UNICODE support, we cannot use string anymore.
//
struct WideString
{
    wchar_t wString[OS_MAX_PATH + 1];
    WideString()
    {
        wString[0] = L'\0';
    }
    WideString(const wchar_t* pStr)
    {
        wcsncpy(wString, pStr, OS_MAX_PATH);
    }

    bool operator< (const WideString& other) const
    {
        return (wcscmp(wString, other.wString) < 0);
    }

    bool operator== (const WideString& other) const
    {
        return (wcscmp(wString, other.wString) == 0);
    }

    bool operator> (const WideString& other) const
    {
        return (wcscmp(wString, other.wString) > 0);
    }

    // assignment operator overloading
    WideString& operator= (const WideString& other)
    {
        wcscpy_s(wString, OS_MAX_PATH, other.wString);
    }
};


// Key for the Thread Info map
struct ThreadInfoKey
{
    gtUInt64  processID;      // process ID
    DWORD     threadID;       // thread ID

    // constructor
    ThreadInfoKey(gtUInt64 ctor_procID, DWORD ctor_threadID) : processID(ctor_procID), threadID(ctor_threadID)
    {
    }

    // key comparison rule
    bool operator< (const ThreadInfoKey& other) const
    {
        return (processID < other.processID) || ((processID == other.processID) && (threadID < other.threadID));
    }

    // assignment operator overloading
    ThreadInfoKey& operator= (const ThreadInfoKey& other)
    {
        this->processID = other.processID;
        this->threadID = other.threadID;
    }
};

// thread info map value
struct ThreadInfoValue
{
    DWORD   threadID;           // thread ID
    int     cpuNumCreated;      // processor the number the thread was created on
    int     cpuNumDeleted;      // processor the number the thread was deleted on
    DWORD   threadCreationTime; // thread creation time.
    DWORD   threadDeletionTime; // thread deletion time

    // default constructor
    ThreadInfoValue() : threadID(0),
        cpuNumCreated(-1),
        cpuNumDeleted(-1),
        threadCreationTime(0),
        threadDeletionTime(0)
    {
    }

    // constructor
    ThreadInfoValue(DWORD ctor_threadid, int ctor_cpuCreated, int ctor_cpuDeleted,
                    DWORD ctor_threadCreatTime, DWORD ctor_threadDeleteTime) : threadID(ctor_threadid),
        cpuNumCreated(ctor_cpuCreated),
        cpuNumDeleted(ctor_cpuDeleted),
        threadCreationTime(ctor_threadCreatTime),
        threadDeletionTime(ctor_threadDeleteTime)
    {
    }
};

// thread info map value
struct PreJitModSymbolFile
{
    wchar_t symbolFilePath[OS_MAX_PATH + 1];      // symbol file for preJIT managed module
    bool    bHasSample;         // if the module has sample

    // default constructor
    PreJitModSymbolFile()
    {
        symbolFilePath[0] = L'\0';
        bHasSample = false;
    }

    // constructor
    PreJitModSymbolFile(const wchar_t* pPath)
    {
        wcscpy_s(symbolFilePath, OS_MAX_PATH, pPath);
        bHasSample = false;
    }
};

// Process map which is used to keep processes info during the profiling
typedef gtMap<gtUInt64, ProcessValue> ProcessMap;

// Kernel module map which is used to keep kernel module info during the profiling
typedef gtMap<KernelModKey, KernelModValue> KernelModMap;

// Drive map which is used to keep the Drive info
// key of the map is the device name, value of map is the drive label.
typedef gtMap<WideString, WideString> DriveMap;

typedef gtMap<ModuleKey, JitBlockValue> JitBlockInfoMap;

// Thread Info map
typedef gtMap<ThreadInfoKey, ThreadInfoValue> ThreadInfoMap;

// Map for PreJITed module Symbol file

typedef gtMap<WideString, PreJitModSymbolFile> PreJitModSymbolMap;


#endif // _TASKINFOMAPPER_H_
