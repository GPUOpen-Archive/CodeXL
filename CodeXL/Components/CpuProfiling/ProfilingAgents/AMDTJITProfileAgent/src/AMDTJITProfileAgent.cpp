//==================================================================================
// Copyright (c) 2014-2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file AMDTJITProfileAgent.cpp
/// \brief Profile Agent to profile the dynamically generated code like.
///
//==================================================================================

// System Headers
#ifdef _WIN32
    #include <process.h>
#else
    #include <sys/time.h>
#endif

#include <sys/types.h>
#include <sys/stat.h>
#include <map>
#include <wchar.h>

// OS_MAX_PATH is defined here
#include <AMDTBaseTools/Include/AMDTDefinitions.h>
#include <AMDTOSWrappers/Include/osOSDefinitions.h>
#include <AMDTProfilingAgentsData/inc/JncWriter.h>
#include <AMDTProfilingAgentsData/inc/JclWriter.h>
#include <AMDTJITProfileAgent.h>

//
//    Macros
//
#define METHOD_ID_START     0xFFFFF000

#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
    #define STR_FORMAT L"%s"
    #define PATH_SEPARATOR L"\\"
    #define wcsncpy_truncate(dst, size, src) wcsncpy_s(dst, size, src, _TRUNCATE)
    #define wcsncat_truncate(dst, size, src) wcsncat_s(dst, size, src, _TRUNCATE)
    #define mbstowcs_truncate(dst, size, src) { size_t retVal; mbstowcs_s(&retVal, dst, size, src, _TRUNCATE); }
#else
    #define STR_FORMAT L"%S"
    #define PATH_SEPARATOR L"/"
    #define wcsncpy_truncate(dst, size, src) wcsncpy(dst, src, (size) - 1)
    #define wcsncat_truncate(dst, size, src) wcsncat(dst, src, (size) - 1 - wcslen(dst))
    #define mbstowcs_truncate(dst, size, src) mbstowcs(dst, src, (size) - 1)
#endif

//
//    Globals
//

AMDTJitProfileEnv  gJitEnv;            // UNUSED ?
unsigned int       gJitPID;
std::wstring       gJitCmd;            // JIT Application name
std::wstring       gSampleDir;         // Session directory - /tmp/session1/
std::wstring       gProfileDataDir;    // JIT Profile data dir; ex: /tmp/session11/PID/
JclWriter*         gJCLWriter = NULL;
int                gCurMethodId = 0;

int gJitVerbose = 0;

// Is JIT profile enabled
bool gAMDTJitProfileEnabled = false;
static bool gAMDTJitProfileInitialized = false;

// Method name map
std::map<std::wstring, unsigned int> gMethodMap;

// method address map
std::map<gtUInt64, unsigned int> gJitAddrMap;

typedef struct _methodInfo
{
    wchar_t*  pClassName;
    wchar_t*  pMethodName;
    wchar_t*  pSrcFileName;
} methodInfo;

const wchar_t*  gUnknownSrcFile    = L"UnknownJITSource";
const wchar_t*  gUnknownClassName  = L"UnknownClassName";
const wchar_t*  gUnknownMethodName = L"UnknownMethodName";
const wchar_t*  gEnvVar = L"AMDT_JITPROFILE_ENABLE";

//
// Helper functions
//
static void jitError(AMDTJitProfileEnv* pEnv, const char* format, ...);

static unsigned int getUniqueMethodId(const wchar_t* pMethodName,
                                      const wchar_t* pClassName);

static int getCmdline(AMDTJitProfileEnv* pEnv);

static int getProfileDataDir(std::wstring& pathStr);

static bool queryCurrentTime(gtUInt64& curTime);

#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
    static int recursiveMkdir(const wchar_t* pDir);
#endif


static int writeJCLFile(methodInfo*    pMethodInfo,
                        const void*   codeAddr,
                        gtUInt32      codeSize,
                        std::wstring* pJncFileName);

// Write the JNC file for JIT code
static int writeJncFile(methodInfo*           pMethodInfo,
                        const void*           codeAddr,
                        gtUInt32              codeSize,
                        std::wstring*         pJncFileName,
                        gtUInt32              lineTableSize,
                        AMDTJitSrcLineTable*  pLineTable,
                        const void*           pCompileInfo);

static int jitProfileInitialize(AMDTJitProfileEnv* pEnv);

static int jitProfileFinalize(AMDTJitProfileEnv* pEnv);

static int jitProfileEnable(bool enable);


// Undocumented API's
AMDT_JITPROFILE_API int AMDTJitProfileEnable(void);
AMDT_JITPROFILE_API int AMDTJitProfileDisable(void);
AMDT_JITPROFILE_API int AMDTJitProfileEnableDebugLog(int logLevel);

//
// Helper Functions
//

// jitError
//
// Helper function to emit an error message and return an appropriate error code.
//
static void
jitError(AMDTJitProfileEnv* pEnv, const char* format, ...)
{
    GT_UNREFERENCED_PARAMETER(pEnv);

    va_list ap;
    va_start(ap, format);
    (void)vfprintf(stderr, format, ap);
    (void)fflush(stderr);
    va_end(ap);

    // cleanup.. currently nothing to cleanup
    // if (NULL != pEnv) { }

    return;
} // jitError


// getCmdline
//
// Retrieves the command-line of the current process and set gJitCmd. Appends "(Dynamic Code)"
// to the command-line.
//
static int getCmdline(AMDTJitProfileEnv* pEnv)
{
    if (NULL != pEnv && NULL != pEnv->pModuleName)
    {
        gJitCmd = pEnv->pModuleName;
    }
    else
    {
#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
        gJitCmd.clear();
        gJitCmd = GetCommandLine();

        // remove the trailing 2 char \" \ in the commandline returned by GetCommandLineW
        size_t len = gJitCmd.size();
        gJitCmd.erase(len - 2, 2);

        // Add " (Dynamic Code)" prefix to the JIT application name
        gJitCmd = gJitCmd + L" (Dynamic Code)";
#endif // AMDT_WINDOWS_OS
    }

    return AMDT_JITPROFILE_SUCCESS;
} // getCmdline


// getProfileDataDir
//
// Construct the Profile Data Directory in which the JNC/JCL profile-data-files will be written.
//
static int getProfileDataDir(std::wstring& pathStr)
{
    wchar_t  sessionPath[OS_MAX_PATH] = { L'\0' };

#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
    GetTempPathW(OS_MAX_PATH, sessionPath);

    gSampleDir = sessionPath;

    sessionPath[0] = L'\0';
    swprintf(sessionPath,
             OS_MAX_PATH,
             L"%s\\%d",
             gSampleDir.c_str(),
             gJitPID);

#else
    // char tmpPath[OS_MAX_PATH] = "/tmp/.codexl-java";
    char tmpPath[OS_MAX_PATH] = "/tmp/.codexl-jit";

    struct stat buf;

    if (stat(tmpPath, &buf) != 0)
    {
        mode_t u = umask(0x0);

        if (mkdir(tmpPath, 0777) != 0)
        {
            umask(u);
            return AMDT_JITPROFILE_FAILED;
        }

        umask(u);
    }

    // convert char to wchar_t
    memset(sessionPath, 0, (OS_MAX_PATH * sizeof(wchar_t)));
    mbstowcs(sessionPath, tmpPath, OS_MAX_PATH - 1);
    gSampleDir = sessionPath;

    memset(sessionPath, 0, (OS_MAX_PATH * sizeof(wchar_t)));
    swprintf(sessionPath,
             OS_MAX_PATH - 1,
             STR_FORMAT PATH_SEPARATOR L"%d",
             gSampleDir.c_str(),
             gJvmPID);
#endif

    // set the output
    pathStr = sessionPath;

    return AMDT_JITPROFILE_SUCCESS;
} // getProfileDataDir


#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
// recursiveMkdir
//
// This is a Windows specific helper function to create the temporary folder recursively.
//
static int recursiveMkdir(const wchar_t* pDir)
{
    int ret = 0;
    wchar_t tempDir[FILENAME_MAX] = { L'\0' };
    wchar_t* pNextDir = const_cast<wchar_t*>(pDir);

    memset(tempDir, 0, (FILENAME_MAX) * sizeof(wchar_t));

    while ((pNextDir = wcschr(pNextDir, L'\\')) != NULL)
    {
        pNextDir ++;
        wcsncpy_s(tempDir, FILENAME_MAX, pDir, (pNextDir - pDir));
        _wmkdir(tempDir);
    }

    if (0 == ret)
    {
        ret = _wmkdir(pDir);
    }

    return ret;
} // recursiveMkdir
#endif // AMDT_WINDOWS_OS


// queryCurrentTime
//
// returns current time in microseconds
//
static bool queryCurrentTime(gtUInt64& curTime)
{
#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
    return FALSE != QueryPerformanceCounter(reinterpret_cast<LARGE_INTEGER*>(&curTime));
#else
    // Linux:
    // TODO: This is should be in sync with the timestamp used by PERF.
    // Otherwise sample attribute will fail.

    struct timeval cTime;
    bool ret = (0 == gettimeofday(&cTime, NULL));

    if (ret)
    {
        curTime = cTime.tv_sec * 1000000;
        curTime += cTime.tv_usec;
    }

    return ret;
#endif
} // queryCurrentTime


// getUniqueMethodId
//
// Generate an unique method-id for the given ClassName::MethodName signature.
// pMethodName can be NULL.
//
static unsigned int
getUniqueMethodId(const wchar_t* pMethodName,
                  const wchar_t* pClassName)
{
    std::wstring methodSig;

    // pClassName can be NULL;
    if (NULL == pMethodName)
    {
        return (unsigned int)AMDT_JITPROFILE_FAILED;
    }

    if (! gCurMethodId)
    {
        gCurMethodId = METHOD_ID_START;
    }

    if (NULL != pClassName)
    {
        methodSig.append(pClassName);
        methodSig.append(L"::");
    }

    methodSig.append(pMethodName);

    if (gMethodMap.end() == gMethodMap.find(methodSig))
    {
        gMethodMap[methodSig] = ++gCurMethodId;
    }

    return gCurMethodId;
} // getUniqueMethodId


// jitProfileInitialize
//
static int jitProfileInitialize(AMDTJitProfileEnv* pEnv)
{
    int             ret = 0;
    std::wstring    profileDataDir;

    if (true == gAMDTJitProfileInitialized)
    {
        return AMDT_JITPROFILE_SUCCESS;
    }

    gJitPID = (NULL == pEnv) ? _getpid() : pEnv->processId;

    // Get the command-line to obtain the Application name;
    // gJitCmd will contain the name of JIT Application;
    ret = getCmdline(pEnv);

    if (AMDT_JITPROFILE_SUCCESS != ret)
    {
        jitError(pEnv, "Could not get the command line");
        return ret;
    }

    if (gJitVerbose)
    {
        wprintf(L"Command Line: %s\n", gJitCmd.c_str());
    }

    // Get the dir path in which the JIT profile data files will be written
    // Note: getProfileDataDir gives gSampleDir and profileDataDir
    ret = getProfileDataDir(profileDataDir);

    if (AMDT_JITPROFILE_SUCCESS != ret)
    {
        jitError(pEnv, "Could not get profile data dir for JIT application");
        return ret;
    }

    if (gJitVerbose)
    {
        wprintf(L"The JIT Profile data dir is " STR_FORMAT L".\n",
                profileDataDir.c_str());
    }

    // Save the profileDataDir for future use..
    gProfileDataDir = profileDataDir;

#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
    // The directory access right is 771
    int j = recursiveMkdir(profileDataDir.c_str());

    if (0 != j)
    {
        wprintf(L"The sample directory " STR_FORMAT L" was not able to be created\n",
                profileDataDir.c_str());
        return AMDT_JITPROFILE_FAILED;
    }

    profileDataDir = gProfileDataDir;
#else
    // Linux
    struct stat buf;
    string tmpStr(profileDataDir.begin(), profileDataDir.end());

    if (stat(tmpStr.c_str(), &buf) != 0)
    {
        if (mkdir(tmpStr.c_str(), 0x01fd) != 0)
        {
            jitError(pEnv, "Cannot create directory %s",
                     profileDataDir.c_str());
            return AMDT_JITPROFILE_FAILED;
        }
    }

#endif

    // Save the profileDataDir for future use..
    gProfileDataDir = profileDataDir;

    // Construct the JCL filename
    wchar_t jclFile[OS_MAX_PATH] = { L'\0' };

    swprintf(jclFile, OS_MAX_PATH - 1, STR_FORMAT PATH_SEPARATOR L"%d.jcl", profileDataDir.c_str(), gJitPID);

    // Initialize JCLWriter
    gJCLWriter = new JclWriter(jclFile, gJitCmd.c_str(), gJitPID);

    if (NULL == gJCLWriter)
    {
        jitError(pEnv, "Error: Out of Memory");
        return AMDT_JITPROFILE_FAILED;
    }

    // This just writes the header record
    gJCLWriter->Initialize();

    return AMDT_JITPROFILE_SUCCESS;
} // jitProfileInitialize


// jitProfileFinalize
//
static int jitProfileFinalize(AMDTJitProfileEnv* pEnv)
{
    (void)(pEnv); // unused

    gJitAddrMap.clear();
    gMethodMap.clear();

    if (NULL != gJCLWriter)
    {
        delete gJCLWriter;
        gJCLWriter = NULL;
    }

    // TODO: If the profile is not enabled, delete the Temp folder and the JCL file created
    //if (false == gAMDTJitProfileEnabled)
    //{
    //}

    return AMDT_JITPROFILE_SUCCESS;
} // jitProfileFinalize


static int jitProfileEnable(bool enable)
{
    int ret = AMDT_JITPROFILE_ENABLED;

    // JIT Profile has to be initialized
    if (false == gAMDTJitProfileInitialized)
    {
        return AMDT_JITPROFILE_ERROR_NOT_INITIALIZED;
    }

    // Enable JIT profile
    if (true == enable)
    {
        if (false == gAMDTJitProfileEnabled)
        {
            gAMDTJitProfileEnabled = true;
            ret = AMDT_JITPROFILE_SUCCESS;
        }
    }
    else
    {
        // Disable JIT profile
        ret = AMDT_JITPROFILE_ERROR_DISABLED;

        if (true == gAMDTJitProfileEnabled)
        {
            gAMDTJitProfileEnabled = false;
            ret = AMDT_JITPROFILE_SUCCESS;
        }
    }

    // TODO: Do we need to do any check whether Temp folder and JCL file is created properly ?

    return ret;
} // jitProfileEnable


// writeJCLFile
//
// Construct JCL Load records and write into the JCL file
//
static int writeJCLFile(methodInfo*   pMethodInfo,
                        const void*   pCodeAddr,
                        gtUInt32      codeSize,
                        std::wstring* pJncFileName)
{
    gtUInt64  loadTimeStamp = 0;

    if ((NULL == pCodeAddr) || (NULL == pMethodInfo))
    {
        return AMDT_JITPROFILE_ERROR_INVALIDARG;
    }

    if ((NULL == gJCLWriter))
    {
        return AMDT_JITPROFILE_FAILED;
    }

    JitLoadRecord jclLoadRecord;

    // Get the module load timestamp
    queryCurrentTime(loadTimeStamp);

    jclLoadRecord.loadTimestamp  = loadTimeStamp;
    jclLoadRecord.blockStartAddr = (gtUInt64)pCodeAddr;
    jclLoadRecord.blockEndAddr   = (gtUInt64)pCodeAddr + codeSize;
    jclLoadRecord.threadID       = 0;

    // Construct method signature as ClassName::MethodName
    if (NULL != pMethodInfo->pClassName)
    {
        wcsncpy_truncate(jclLoadRecord.classFunctionName, (OS_MAX_PATH - 3), pMethodInfo->pClassName);
        wcsncat_truncate(jclLoadRecord.classFunctionName, OS_MAX_PATH, L"::");
    }

    if (NULL != pMethodInfo->pMethodName)
    {
        wcsncat_truncate(jclLoadRecord.classFunctionName, OS_MAX_PATH, pMethodInfo->pMethodName);
    }

    if (NULL != pMethodInfo->pSrcFileName)
    {
        wcsncpy_truncate(jclLoadRecord.srcFileName, OS_MAX_PATH, pMethodInfo->pSrcFileName);
    }

    wcsncpy_truncate(jclLoadRecord.jncFileName, OS_MAX_PATH, pJncFileName->c_str());

    // write the JITLoadRecord for this module
    gJCLWriter->WriteLoadRecord(&jclLoadRecord);

    return AMDT_JITPROFILE_SUCCESS;
} // writeJCLFile


// writeJncFile
//
// Write the JNC file for the given compiled method.
//
static int writeJncFile(methodInfo*           pMethodInfo,
                        const void*           codeAddr,
                        gtUInt32              codeSize,
                        std::wstring*         pJncFileName,
                        gtUInt32              lineTableSize,
                        AMDTJitSrcLineTable*  pLineTable,
                        const void*           pCompileInfo)
{
    (void)(pCompileInfo); // UNUSED
    JncWriter    jncWriter;
    char*        pCodeAddr = (char*)codeAddr;
    std::wstring className;

    if ((NULL == pMethodInfo) || (NULL == codeAddr))
    {
        return AMDT_JITPROFILE_ERROR_INVALIDARG;
    }

    if (gJitVerbose)
    {
        wprintf(L"In writeJncFile, jncFileName is %s\n", pJncFileName->c_str());
    }

    if (NULL != pMethodInfo->pClassName)
    {
        className = pMethodInfo->pClassName;
    }

#if 0
#if AMDT_ADDRESS_SPACE_TYPE == AMDT_64_BIT_ADDRESS_SPACE
    gtUInt64 jitLoadAddr = (gtUInt64)pCodeAddr;
#else
    // FIXME: why is code_size added to jitLoadAddr
    unsigned int jitLoadAddr = (unsigned int)pCodeAddr + codeSize;
#endif
#else
    gtUInt64 jitLoadAddr = (gtUInt64)pCodeAddr;
#endif

    jncWriter.SetJNCFileName(pJncFileName->c_str());
    jncWriter.SetJITStartAddr(jitLoadAddr);
    jncWriter.SetJITFuncName((!className.empty()) ? className.c_str() : NULL,
                             pMethodInfo->pMethodName,        // Method name
                             pMethodInfo->pSrcFileName);      // JIT source file name

    if ((NULL != pLineTable) && (lineTableSize > 0))
    {
        // TODO: construct JavaSrcLineInfo from line table provided by JIT
        if (gJitVerbose)
        {
            wprintf(L"In writeJncFile, number of source lines is %d\n",
                    lineTableSize);
        }

        jncWriter.WriteJITNativeCode((gtUByte*)pCodeAddr,
                                     codeSize,
                                     (JavaSrcLineInfo*)pLineTable, // FIXME
                                     lineTableSize);
    }
    else
    {
        bool ret = jncWriter.WriteJITNativeCode((gtUByte*)pCodeAddr, codeSize);

        if (!ret && gJitVerbose)
        {
            printf("WriteJITNativeCode Failed\n");
        }
    }

    jncWriter.Close();

    return AMDT_JITPROFILE_SUCCESS;
}


//
//                                  !!! Public APIs !!!
//

// AMDTJitProfileInitialize
//
// This API initializes the AMD JIT Profile environment.
// This is called during the AMDTJitProfileAgent.dll load time, alternatively
// the client can directly call this API to intialize.
//
// Returns:
//    AMDT_JITPROFILE_SUCCESS - on success
//    AMDT_JITPROFILE_FAILED  - if initialize failed, due to internal errors
//
int AMDTJitProfileInitialize(AMDTJitProfileEnv* pEnv)
{
    int ret = AMDT_JITPROFILE_SUCCESS;

    if (false == gAMDTJitProfileInitialized)
    {
        ret = jitProfileInitialize(pEnv);

        gAMDTJitProfileInitialized = (AMDT_JITPROFILE_SUCCESS == ret) ? true : false;
    }

    if ((AMDT_JITPROFILE_SUCCESS == ret)
        && (false == gAMDTJitProfileEnabled))
    {
        ret = jitProfileEnable(true);

        gAMDTJitProfileEnabled = ((AMDT_JITPROFILE_SUCCESS == ret) || (AMDT_JITPROFILE_ENABLED == ret)) ? true : false;
    }

    return ret;
} // AMDTProfileInitialize


// AMDTJitProfileFinialize
//
// This API cleans up the AMD JIT Profile environment once the profile is completed.
// This is called during the AMDTJitProfileAgent.dll unload time, alternatively
// the client can directly call this API after the profile is completed.
//
// Returns:
//    AMDT_JITPROFILE_SUCCESS - on success
//    AMDT_JITPROFILE_FAILED  - if initialize failed, due to internal errors

// This is called during agent unload time
//
int AMDTJitProfileFinalize(void)
{
    return jitProfileFinalize(NULL);
} // AMDTProfileFinalize


// AMDTJitProfileEnable()
//
//
// This enables the JIT profile.
// Before calling this API, AMDTJitProfileInitialzie() should be called.
// By default this will be called by AMDTJitProfileInitialzie().
//
// Returns:
//   AMDT_JITPROFILE_SUCCESS               - On success
//   AMDT_JITPROFILE_ERROR_NOT_INITIALIZED - if not initialized
//   AMDT_JITPROFILE_ENABLED               - if already enabled
//   AMDT_JITPROFILE_FAILED                - if any internal errors
//
int AMDTJitProfileEnable(void)
{
    return jitProfileEnable(true);
} // AMDTJitProfileEnable


// AMDTJitProfileDisable()
//
// This disables the JIT profile. Subsequent AMDTJitProfileMethodLoad()
// and AMDTJitProfileMethodUnload() calls will not create any JNC files.
// Before calling this API, AMDTJitProfileInitialzie() should be called.
//
// Returns:
//   AMDT_JITPROFILE_SUCCESS               - On success
//   AMDT_JITPROFILE_ERROR_NOT_INITIALIZED - if not initialized
//   AMDT_JITPROFILE_ERROR_DISABLED        - if already disabled
//   AMDT_JITPROFILE_FAILED                - if any internal errors
//
//
int AMDTJitProfileDisable(void)
{
    return jitProfileEnable(false);
} // AMDTJitProfileDisable


// AMDTJitProfileStatus
//
// Get the status of the JIT profile
//
//   Returns:
//     AMDT_JITPROFILE_INITIALIZED     - if the JIT profile is initialized but not enabled
//     AMDT_JITPROFILE_ENABLED         - if the JIT profile is initialized and enabled
//     AMDT_JITPROFILE_ERROR_DISABLED  - if the JIT profile is disabled
//
int AMDTJitProfileStatus(void)
{
    int ret = AMDT_JITPROFILE_ERROR_DISABLED;

    if (true == gAMDTJitProfileEnabled)
    {
        ret = AMDT_JITPROFILE_ENABLED;
    }
    else if (true == gAMDTJitProfileInitialized)
    {
        ret = AMDT_JITPROFILE_INITIALIZED;
    }

    return ret;
} // AMDTJitProfileStatus


// AMDTJitProfileMethodLoad
//
// API to be called whenever a method is loaded
//
int AMDTJitProfileMethodLoad(AMDTJitMethodLoad* pMethodInfo)
{
    std::wstring jncFile;
    methodInfo mInfo;

    if (false == gAMDTJitProfileEnabled)
    {
        return AMDT_JITPROFILE_ERROR_DISABLED;
    }

    if (NULL == pMethodInfo)
    {
        return AMDT_JITPROFILE_ERROR_INVALIDARG;
    }

    gtUInt64 codeAddr = (gtUInt64)pMethodInfo->pCodeAddr;
    unsigned int codeSize = pMethodInfo->codeSize;

    // Construct the JNC filename
    if (gJitAddrMap.end() == gJitAddrMap.find(codeAddr))
    {
        gJitAddrMap[codeAddr] = 0;
    }

    wchar_t tmpPath[OS_MAX_PATH] = { L'\0' };
    swprintf(tmpPath, OS_MAX_PATH - 1, STR_FORMAT PATH_SEPARATOR L"JITCode-%llx-%d.jnc",
             gProfileDataDir.c_str(), codeAddr, gJitAddrMap[codeAddr]);
    jncFile = tmpPath;

    ++gJitAddrMap[codeAddr];

    // Get the class signature and Method name
    // TDB: should we use "UnknownClassName" if the AMDTJitMethodInfo::pClassName is NULL
    // mInfo.pClassName = (NULL != pMethodInfo->pClassName)
    //                   ? pMethodInfo->pClassName : (wchar_t*)gUnknownClassName;
    mInfo.pClassName = pMethodInfo->pClassName;

    // Get the Method name
    mInfo.pMethodName = (NULL != pMethodInfo->pMethodName)
                        ? pMethodInfo->pMethodName : (wchar_t*)gUnknownMethodName;

    // Get the Source filename of the class
    mInfo.pSrcFileName = (NULL != pMethodInfo->pSrcFileName)
                         ? pMethodInfo->pSrcFileName : (wchar_t*)gUnknownSrcFile;

    if (gJitVerbose)
    {
        wprintf(L"\nAMDTJitProfileMethodLoad : %s::%s - 0x%lx\n",
                mInfo.pClassName, mInfo.pMethodName, (unsigned long)codeAddr);
        wprintf(L"Class name : %s\n", mInfo.pClassName);
        wprintf(L"Method name : %s\n", mInfo.pMethodName);
        wprintf(L"Source file : %s\n", mInfo.pSrcFileName);
        wprintf(L"Generated Method id : %u\n", getUniqueMethodId(mInfo.pMethodName, mInfo.pClassName));
        wprintf(L"JITed CodeAddr : 0x%llx\n", codeAddr);
        wprintf(L"JITed Code Size : %d\n", codeSize);
        wprintf(L"JncFile name : " STR_FORMAT L"\n", jncFile.c_str());
        // TODO: print line table
    }

    // Write the LOAD record in JCL file
    int ret;
    ret = writeJCLFile(&mInfo,
                       pMethodInfo->pCodeAddr,
                       codeSize,
                       &jncFile);

    if (AMDT_JITPROFILE_SUCCESS != ret)
    {
        return ret;
    }

    // Write the JNCFile for this compiled method
    ret = writeJncFile(&mInfo,
                       pMethodInfo->pCodeAddr,
                       codeSize,
                       &jncFile,
                       pMethodInfo->lineTableSize,
                       pMethodInfo->pLineTable,
                       NULL);

    return ret;
} // AMDTJitProfileMethodLoad


// AMDTJitProfileMethodUnload
//
// API to be called when a method is unloaded
//
int AMDTJitProfileMethodUnload(AMDTJitMethodUnload* pMethodInfo)
{
    if (false == gAMDTJitProfileEnabled)
    {
        return AMDT_JITPROFILE_ERROR_DISABLED;
    }

    if (NULL == pMethodInfo)
    {
        return AMDT_JITPROFILE_ERROR_INVALIDARG;
    }

    if (NULL == gJCLWriter)
    {
        return AMDT_JITPROFILE_FAILED;
    }

    gtUInt64 codeAddr = (gtUInt64)pMethodInfo->pCodeAddr;

    // TDB: should we use "UnknownClassName" if the AMDTJitMethodInfo::pClassName is NULL
    // wchar_t* pClassSig = (NULL != pMethodInfo->pClassName)
    //                   ? pMethodInfo->pClassName : (wchar_t*)gUnknownClassName;
    wchar_t* pClassSig = pMethodInfo->pClassName;

    wchar_t* pMethodName = (NULL != pMethodInfo->pMethodName)
                           ? pMethodInfo->pMethodName : (wchar_t*)gUnknownMethodName;

    if (gJitVerbose)
    {
        wprintf(L"AMDTJitProfileMethodUnload: 0x%lx: %s:%s\n",
                (unsigned long) codeAddr, pClassSig, pMethodName);
    }

    JitUnloadRecord jclUnloadRec;
    gtUInt64 unloadTimeStamp = 0;

    queryCurrentTime(unloadTimeStamp);

    jclUnloadRec.unloadTimestamp = unloadTimeStamp;
    jclUnloadRec.blockStartAddr  = codeAddr;

    gJCLWriter->WriteUnloadRecord(&jclUnloadRec);

    return AMDT_JITPROFILE_SUCCESS;
} // AMDTJitProfileMethodUnload


// AMDTJitProfileEnableDebugLog
//
// API to enable debug logging
//
// logLevel = 0, will disable debug logging
// logLevel = 1, will enable debug logging
//
int AMDTJitProfileEnableDebugLog(int logLevel)
{
    gJitVerbose = logLevel;

    return AMDT_JITPROFILE_SUCCESS;
} // AMDTJitProfileEnableDebugLog