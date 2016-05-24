//==================================================================================
// Copyright (c) 2013-2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file JitTaskInfoMapper.h
///
//==================================================================================

#ifndef _JITTASKINFOMAPPER_H_
#define _JITTASKINFOMAPPER_H_

#include <AMDTBaseTools/Include/gtMap.h>
#include <AMDTBaseTools/Include/gtString.h>
#include <AMDTOSWrappers/Include/osOSDefinitions.h>
#include <AMDTCommonProfileDataTypes.h>

#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
    #include <AMDTExecutableFormat/inc/PeFile.h>
    typedef DWORD TiTimeType;
#else
    typedef gtUInt64 TiTimeType;
#endif

#define TI_TIMETYPE_MAX ((TiTimeType)(-1))

enum ModTypeEnum
{
    evInvalidType,
#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
    evPEModule,
#endif
    evJavaModule,
    evManaged,
    evOCLModule
};

struct TiModuleInfo
{
    /* [in]  */  gtUInt64 processID;
    /* [in]  */  unsigned CSvalue;
    /* [in]  */  gtUInt64 sampleAddr;
    /* [in]  */  unsigned cpuIndex;
    /* [in]  */  gtUInt64 deltaTick;
    /* [out] */  gtUInt64 ModuleStartAddr;
    /* [out] */  gtUInt64 Modulesize;
    /* [out] */  ModTypeEnum moduleType;
    /* [in]  */  unsigned funNameSize;
    /* [out] */  wchar_t* pFunctionName;
    /* [out] */  gtUInt64 FunStartAddr;
    /* [in]  */  unsigned jncNameSize;
    /* [out] */  wchar_t* pJncName;
    /* [in]  */  unsigned namesize;
    /* [out] */  wchar_t* pModulename;
    /* [in]  */  unsigned srcfilesize;
    /* [out] */  wchar_t* pJavaSrcFileName;  // This is for Java JITed block;
    /* [out] */  bool kernel;                // Used for thread profiling.
    /* [in]  */  unsigned sesdirsize;
    /* [out] */  wchar_t* pSessionDir;
    /* [out] */  gtUInt32 moduleId;
    /* [out] */  gtUInt32 instanceId;
#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
    /* [out] */  PeFile* pPeFile;
#endif
};

// key of module map.
// THis key could be used for all user mode module (unmanaged or managed),
// java jitted method and clr jitted function.
// FOr the jitted function, the module load address is the jit start address,
// module load time is the jit compilation finish time.
// So, we can use same structure and algorithm to categorize sample in
// GetModuleInfo function.
//  -- Lei 03/07/05
//
struct ModuleKey
{
    gtUInt64   processId;      // process id.
    gtUInt64   moduleLoadAddr; // module load address
    TiTimeType moduleLoadTime; // module load time (in milliseconds)

    // constructor.
    ModuleKey(gtUInt64 ctor_pid, gtUInt64 ctor_mLoadAddr, TiTimeType ctor_mLoadTime) : processId(ctor_pid),
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
    gtUInt64 moduleBaseAddr;     // module image base address;
    // nothing for JIT block.
    gtUInt64 moduleSize;         // module image size, or JIT block size

    TiTimeType moduleUnloadTime;   // module unload time.
    // JIT block unload time.

    wchar_t moduleName[OS_MAX_PATH];  // module path and name.
    // JIT function name;

    bool bNameConverted;     // module name is converted

#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
    bool bLoadedPeFile;
    PeFile* pPeFile;
#endif

    // JIT block copied to run session dir.
    ModTypeEnum moduleType;     // manage, unmanaged, java etc.
    gtUInt32 instanceId = 0;
    gtInt32 moduleId = 0;

    // default constructor
    ModuleValue() : moduleBaseAddr(0),
        moduleSize(0),
        moduleUnloadTime(TI_TIMETYPE_MAX),
        bNameConverted(false),
#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
        bLoadedPeFile(false),
        pPeFile(NULL),
        moduleType(evPEModule)
#else
        moduleType(evJavaModule)
#endif
    {
        moduleName[0] = L'\0';
    }

    // constructor
    ModuleValue(gtUInt64 ctor_mBaseAddr,
                gtUInt64 ctor_mSize,
                TiTimeType ctor_mUnloadTime,
                const wchar_t* ctor_mName,
                ModTypeEnum modType) : moduleBaseAddr(ctor_mBaseAddr),
        moduleSize(ctor_mSize),
        moduleUnloadTime(ctor_mUnloadTime),
        bNameConverted(false),
#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
        bLoadedPeFile(false),
        pPeFile(NULL),
#endif
        moduleType(modType)
    {
        wcsncpy(moduleName, ctor_mName, OS_MAX_PATH - 1);
    }
};

// Module vaule for the module map
struct JitBlockValue
{
    gtString categoryName;      // this could be Java app name in JAVA profile or managed module name;
    wchar_t srcFileName[OS_MAX_PATH];     // java source file in java profile, nothing for CLR profile
    wchar_t jncFileName[OS_MAX_PATH];     // jnc file name
    wchar_t movedJncFileName[OS_MAX_PATH];    // jnc moved to user profile session dir.
    bool    bJncMoved;

    // default constructor
    JitBlockValue() : bJncMoved(false)
    {
        jncFileName[0] = L'\0';
        srcFileName[0] = L'\0';
        movedJncFileName[0] = L'\0';
    }

    // constructor
    JitBlockValue(const wchar_t* javaApp, const wchar_t* jncfile, const wchar_t* srcfile)
    {
        memset(jncFileName, 0, OS_MAX_PATH * sizeof(wchar_t));
        memset(srcFileName, 0, OS_MAX_PATH * sizeof(wchar_t));
        movedJncFileName[0] = L'\0';
        // Need to check the length
        // Copy javaAppName
        categoryName = javaApp;

        if (NULL != srcfile)
        {
            wcsncpy(srcFileName, srcfile, OS_MAX_PATH - 1);
        }

        if (NULL != jncfile)
        {
            wcsncpy(jncFileName, jncfile, OS_MAX_PATH - 1);
        }

        bJncMoved = false;
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

#endif // _JITTASKINFOMAPPER_H_
