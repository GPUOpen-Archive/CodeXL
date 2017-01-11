//==================================================================================
// Copyright (c) 2013-2017, Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file JitTaskInfoMapper.h
///
//==================================================================================

#pragma once

#include <AMDTBaseTools/Include/gtMap.h>
#include <AMDTBaseTools/Include/gtString.h>
#include <AMDTOSWrappers/Include/osOSDefinitions.h>
#include <AMDTCommonHeaders/AMDTCommonProfileDataTypes.h>

#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
    class PeFile;
    typedef DWORD TiTimeType;
#else
    typedef gtUInt64 TiTimeType;
#endif

#define TI_TIMETYPE_MAX ((TiTimeType)(-1))

enum ModTypeEnum
{
    evInvalidType,
    evPEModule,
    evJavaModule,
    evManaged,
    evOCLModule
};

struct TiModuleInfo
{
    /* [in]  */  gtUInt64 processID = 0;
    /* [in]  */  unsigned CSvalue = 0;
    /* [in]  */  gtUInt64 sampleAddr = 0;
    /* [in]  */  unsigned cpuIndex = 0;
    /* [in]  */  gtUInt64 deltaTick = 0;
    /* [out] */  gtUInt64 ModuleStartAddr = 0;
    /* [out] */  gtUInt64 Modulesize = 0;
    /* [out] */  ModTypeEnum moduleType = evInvalidType;
    /* [in]  */  unsigned funNameSize = 0;
    /* [out] */  wchar_t* pFunctionName = nullptr;
    /* [out] */  gtUInt64 FunStartAddr = 0;
    /* [in]  */  unsigned jncNameSize = 0;
    /* [out] */  wchar_t* pJncName = nullptr;
    /* [in]  */  unsigned namesize = 0;
    /* [out] */  wchar_t* pModulename = nullptr;
    /* [in]  */  unsigned srcfilesize = 0;
    /* [out] */  wchar_t* pJavaSrcFileName = nullptr;  // This is for Java JITed block;
    /* [out] */  bool kernel = false;                // Used for thread profiling.
    /* [in]  */  unsigned sesdirsize = 0;
    /* [out] */  wchar_t* pSessionDir = nullptr;
    /* [out] */  gtUInt32 moduleId = 0;
    /* [out] */  gtUInt32 instanceId = 0;
#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
    /* [out] */  PeFile* pPeFile = nullptr;
#endif
};

// key of module map.
// This key could be used for all user mode module (unmanaged or managed),
// Java jitted method and clr jitted function.
// FOr the jitted function, the module load address is the JIT start address,
// module load time is the JIT compilation finish time.
// So, we can use same structure and algorithm to categorize sample in
// GetModuleInfo function.
//
struct ModuleKey
{
    gtUInt64   processId;      // process id.
    gtUInt64   moduleLoadAddr; // module load address
    TiTimeType moduleLoadTime; // module load time (in milliseconds)

    // constructor.
    ModuleKey(gtUInt64 ctor_pid, gtUInt64 ctor_mLoadAddr, TiTimeType ctor_mLoadTime) :
        processId(ctor_pid),
        moduleLoadAddr(ctor_mLoadAddr),
        moduleLoadTime(ctor_mLoadTime)
    {
    }

    // this defines the comparison for ModuleKey
    bool operator< (const ModuleKey& other) const
    {
        return (this->processId < other.processId) ? true
               : (this->processId > other.processId) ? false
               : (this->moduleLoadAddr > other.moduleLoadAddr) ? true
               : (this->moduleLoadAddr < other.moduleLoadAddr) ? false
               : (this->moduleLoadTime > other.moduleLoadTime);
    }
};

// Module value for the module map
// THis structure could be used for binary module (unmanaged or managed).
// It could be used for Java or CLR jitted block also.
//
struct ModuleValue
{
    // module image base address;
    gtUInt64 moduleBaseAddr = 0;

    // module image size, or JIT block size
    gtUInt64 moduleSize = 0;

    // module unload time.
    TiTimeType moduleUnloadTime = TI_TIMETYPE_MAX;

    // module path and name.
    wchar_t moduleName[OS_MAX_PATH];

    // module name is converted
    bool bNameConverted = false;

#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
    bool bLoadedPeFile = false;
    PeFile* pPeFile = nullptr;
#endif

    // JIT block copied to run session dir.
    // manage, unmanaged, Java etc.
    ModTypeEnum moduleType = evInvalidType;
    gtUInt32 instanceId = 0;
    gtInt32 moduleId = 0;

    // default constructor
    ModuleValue()
    {
        moduleName[0] = L'\0';

#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
        moduleType = evPEModule;
#else
        moduleType = evJavaModule;
#endif
    }

    // constructor
    ModuleValue(gtUInt64 ctor_mBaseAddr,
                gtUInt64 ctor_mSize,
                TiTimeType ctor_mUnloadTime,
                const wchar_t* ctor_mName,
                ModTypeEnum modType) :
        moduleBaseAddr(ctor_mBaseAddr),
        moduleSize(ctor_mSize),
        moduleUnloadTime(ctor_mUnloadTime),
        moduleType(modType)
    {
        if (nullptr != ctor_mName)
        {
            wcsncpy(moduleName, ctor_mName, OS_MAX_PATH - 1);
        }
    }
};

// Module value for the module map
struct JitBlockValue
{
    gtString categoryName;                    // this could be Java app name in JAVA profile or managed module name;
    wchar_t srcFileName[OS_MAX_PATH];         // Java source file in Java profile, nothing for CLR profile
    wchar_t jncFileName[OS_MAX_PATH];         // JNC file name
    wchar_t movedJncFileName[OS_MAX_PATH];    // JNC moved to user profile session dir.
    bool    bJncMoved = false;
    int     jncIndex = 0;

    // default constructor
    JitBlockValue()
    {
        jncFileName[0] = L'\0';
        srcFileName[0] = L'\0';
        movedJncFileName[0] = L'\0';
    }

    // constructor
    JitBlockValue(const wchar_t* javaApp, const wchar_t* jncfile, const wchar_t* srcfile) :
        categoryName(javaApp)
    {
        jncFileName[0] = L'\0';
        srcFileName[0] = L'\0';
        movedJncFileName[0] = L'\0';

        if (nullptr != srcfile)
        {
            wcsncpy(srcFileName, srcfile, OS_MAX_PATH - 1);
        }

        if (nullptr != jncfile)
        {
            wcsncpy(jncFileName, jncfile, OS_MAX_PATH - 1);
        }
    }
};

// Module map which is used to keep modules info during the profiling.
typedef gtMap<ModuleKey, ModuleValue> ModuleMap;

typedef gtMap<ModuleKey, JitBlockValue> JitBlockInfoMap;

// JIT bitness map
//      Note: we should keep process bitness in proceessInfo structure.
//          However it requires to change file format of ti. Since it's close
//          to release date, I don't change the file format.
// key is process id;
typedef gtMap<gtUInt64, bool> BitnessMap;
