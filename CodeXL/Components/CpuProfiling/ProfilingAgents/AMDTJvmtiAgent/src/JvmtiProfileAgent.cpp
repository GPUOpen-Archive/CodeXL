//==================================================================================
// Copyright (c) 2013-2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file JvmtiProfileAgent.cpp
/// \brief JVMTI Java Profile Agent.
///
//==================================================================================

//=====================================================================
// Refer the link for details on JVMTI interfaces
//
// http://docs.oracle.com/javase/1.5.0/docs/guide/jvmti/jvmti.html
// http://www.oracle.com/technetwork/articles/javase/jvmti-136367.html
//
//=====================================================================

// System Headers
#ifdef _WIN32
    #include <process.h>
    #include <inttypes.h>
#else
    #include <unistd.h>
    #include <sys/time.h>
#endif

#include <sys/types.h>
#include <sys/stat.h>
#include <cstring>
#include <map>
#include <cwchar>

#include <JvmtiProfileAgent.h>
#include <AMDTProfilingAgentsData/inc/JvmsParser.h>

#pragma warning(disable: 4996) // The POSIX name for this item is deprecated.

#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
    #define PATH_SEPARATOR L"\\"
    #define wcsncpy_truncate(dst, size, src) wcsncpy_s(dst, size, src, _TRUNCATE)
    #define wcsncat_truncate(dst, size, src) wcsncat_s(dst, size, src, _TRUNCATE)
    #define mbstowcs_truncate(dst, size, src) { size_t retVal; mbstowcs_s(&retVal, dst, size, src, _TRUNCATE); }
#else
    #define PATH_SEPARATOR L"/"
    #define wcsncpy_truncate(dst, size, src) wcsncpy(dst, src, (size) - 1)
    #define wcsncat_truncate(dst, size, src) wcsncat(dst, src, (size) - 1 - wcslen(dst))
    #define mbstowcs_truncate(dst, size, src) mbstowcs(dst, src, (size) - 1)
#endif

//
//    Globals
//
jvmtiEnv*       gJvmti = nullptr; // UNUSED: why do we need this ?
osProcessId     gJvmPID;
std::wstring    gJvmCmd = L"";   // contains Java App name
std::wstring    gSampleDir;      // this is session dir; /tmp/ses1
std::wstring    gProfileDataDir; // java-profile data dir; ex: /tmp/ses1/pid/
JclWriter*      gJCLWriter = nullptr;

std::map<gtUInt64, unsigned int> gReJitMap;

int gJvmtiVerbose = 0;

const char*  gUnknownSrcFile    = "UnknownJITSource";
const char*  gUnknownClassName  = "UnknownClassName";
const char*  gUnknownMethodName = "UnknownMethodName";

//
//   Callback declarations
//

void JNICALL cbVMInit(jvmtiEnv* pJvmtiEnv, JNIEnv* pJniEnv, jthread thread);

void JNICALL cbCompiledMethodLoad(jvmtiEnv*                    pJvmtiEnv,
                                  jmethodID                    method,
                                  jint                         codeSize,
                                  const void*                  codeAddr,
                                  jint                         mapLength,
                                  const jvmtiAddrLocationMap*  map,
                                  const void*                  compileInfo);

void JNICALL cbCompiledMethodUnload(jvmtiEnv* pJvmtiEnv, jmethodID method, const void* codeAddr);

void JNICALL cbDynamicCodeGenerated(jvmtiEnv* pJvmtiEnv, const char* name, const void* codeAddr, jint codeSize);

void JNICALL cbClassLoad(jvmtiEnv* pJvmtiEnv, JNIEnv* pJniEnv, jthread thread, jclass klass);

//
//    Helper Routines
//

static void handleFatalError(jvmtiEnv* pJvmtiEnv, const char* format, ...)
{
    va_list ap;
    va_start(ap, format);
    (void)vfprintf(stderr, format, ap);
    (void)fflush(stderr);
    va_end(ap);

    // Unregister CBs and Event-Notifications and back out gracefully
    if (pJvmtiEnv)
    {
        // Unregister event callbacks
        jvmtiEventCallbacks callbacks;

        (void)memset(&callbacks, 0, sizeof(callbacks));
        pJvmtiEnv->SetEventCallbacks(&callbacks, (jint)sizeof(callbacks));

        // Unregister events we are interested in
        pJvmtiEnv->SetEventNotificationMode(JVMTI_DISABLE,
                                            JVMTI_EVENT_VM_INIT,
                                            (jthread)nullptr);

        pJvmtiEnv->SetEventNotificationMode(JVMTI_DISABLE,
                                            JVMTI_EVENT_COMPILED_METHOD_LOAD,
                                            (jthread)nullptr);

        pJvmtiEnv->SetEventNotificationMode(JVMTI_DISABLE,
                                            JVMTI_EVENT_COMPILED_METHOD_UNLOAD,
                                            (jthread)nullptr);

        pJvmtiEnv->SetEventNotificationMode(JVMTI_DISABLE,
                                            JVMTI_EVENT_DYNAMIC_CODE_GENERATED,
                                            (jthread)nullptr);

        pJvmtiEnv->SetEventNotificationMode(JVMTI_DISABLE,
                                            JVMTI_EVENT_CLASS_LOAD,
                                            (jthread)nullptr);
    }
} // handleFatalError


// handleJvmtiError
//
// Every JVMTI interface returns an error code, which should be checked
// to avoid any cascading errors down the line.
// The interface GetErrorName() returns the actual enumeration constant
// name, making the error messages much easier to understand.
//
static int handleJvmtiError(jvmtiEnv* pJvmtiEnv, jvmtiError errnum, const char* msg)
{
    if (errnum != JVMTI_ERROR_NONE)
    {
        char* errnumStr = nullptr;

        pJvmtiEnv->GetErrorName(errnum, &errnumStr);

        handleFatalError(pJvmtiEnv,
                         "JVMTIProfileAgent Error: %d(%s): %s\n", errnum,
                         (errnumStr == nullptr ? "Unknown" : errnumStr),
                         (msg == nullptr ? "" : msg));

        if (nullptr != errnumStr)
        {
            pJvmtiEnv->Deallocate((unsigned char*)errnumStr);
        }

        return -1;
    }

    return 0;
} // handleJvmtiError

// getProfileDataDir
//
// CodeXL should set the profile data dir
// OSUtils::Instance()->SetEnvVar(CODEXL_PROFILE_SESSION_PATH, .. c_str();
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
             gJvmPID);

#else
    char tmpPath[100] = "/tmp/.codexl-java";

    struct stat buf;

    if (stat(tmpPath, &buf) != 0)
    {
        mode_t u = umask(0x0);

        if (mkdir(tmpPath, 0777) != 0)
        {
            umask(u);
            return JNI_ERR;
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
             WIDE_STR_FORMAT PATH_SEPARATOR L"%d",
             gSampleDir.c_str(),
             gJvmPID);
#endif

    // set the output
    pathStr = sessionPath;

    return 0;
} // getProfileDataDir


// returns current time in microseconds
static bool QueryCurrentTime(gtUInt64& curTime)
{
#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
    LARGE_INTEGER timeStamp;

    BOOL ret = QueryPerformanceCounter(&timeStamp);

    curTime = (ret != FALSE) ? timeStamp.QuadPart : 0;

    return true;
#else
    //TODO: This is should be in sync with the timestamp used by PERF. Otherwise sample attribute will fail.

    struct timeval cTime;
    bool ret = (0 == gettimeofday(&cTime, nullptr));

    if (ret)
    {
        curTime = cTime.tv_sec * 1000000;
        curTime += cTime.tv_usec;
    }

    return ret;
#endif
}


// Fill out the jcl element
static void writeJCLFile(jvmtiEnv*     pJvmtiEnv,
                         jmethodID     method,
                         jint          codeSize,
                         const void*   codeAddr,
                         std::wstring* jncFileName,
                         methodInfo*   mInfo,
                         gtUInt64      loadTimeStamp)
{
    GT_UNREFERENCED_PARAMETER(pJvmtiEnv);
    GT_UNREFERENCED_PARAMETER(method);

    if ((nullptr == gJCLWriter) || (nullptr == mInfo))
    {
        return;
    }

    JitLoadRecord jclLoadRecord;

    jclLoadRecord.loadTimestamp = loadTimeStamp;

    // TBD: what about 32-bit agent ?
    // CodeXL-Linux runs only 64-bit Linux OS and also
    // Currently We don't have plans to provide 32-bit Java-Profile-Agent
    jclLoadRecord.blockStartAddr = (gtUInt64)codeAddr;
    jclLoadRecord.blockEndAddr   = (gtUInt64)codeAddr + codeSize;
    jclLoadRecord.threadID       = 0;

    // Copy the char Class Signature
    wchar_t tempWStr[OS_MAX_PATH];
    memset(tempWStr, 0, sizeof(tempWStr));
    mbstowcs_truncate(tempWStr, OS_MAX_PATH, mInfo->pClassSig);

    // Copy as Class::Method-name
    wcsncpy(jclLoadRecord.classFunctionName, tempWStr, OS_MAX_PATH - 3);
    wcsncat_truncate(jclLoadRecord.classFunctionName, OS_MAX_PATH, L"::");

    if (nullptr != mInfo->pMethodName)
    {
        memset(tempWStr, 0, sizeof(tempWStr));
        mbstowcs_truncate(tempWStr, OS_MAX_PATH, mInfo->pMethodName);
        wcsncat_truncate(jclLoadRecord.classFunctionName, OS_MAX_PATH, tempWStr);
    }

    if (nullptr != mInfo->pSrcFile)
    {
        memset(tempWStr, 0, sizeof(tempWStr));
        mbstowcs_truncate(tempWStr, OS_MAX_PATH, mInfo->pSrcFile);
        wcsncpy_truncate(jclLoadRecord.srcFileName, OS_MAX_PATH, tempWStr);
    }

    wcsncpy_truncate(jclLoadRecord.jncFileName, OS_MAX_PATH, jncFileName->c_str());

    // write the JITLoadRecord for this module
    gJCLWriter->WriteLoadRecord(&jclLoadRecord);
} // writeJCLFile


// Fill out the jcl element, when there is no method information
static void writeNativeToJCLFile(wchar_t*      name,
                                 int           codeSize,
                                 const void*   codeAddr,
                                 std::wstring* jncFileName)
{
    if (nullptr != gJCLWriter)
    {
        JitLoadRecord jclLoadRec;
        gtUInt64 loadTimeStamp = 0;

        QueryCurrentTime(loadTimeStamp);

        jclLoadRec.loadTimestamp  = loadTimeStamp;
        jclLoadRec.blockStartAddr = (gtUInt64)codeAddr;
        jclLoadRec.blockEndAddr   = (gtUInt64)codeAddr + codeSize;
        jclLoadRec.threadID       = 0;

        wcscpy(jclLoadRec.classFunctionName, L"Native Code::");
        wcsncpy_truncate(&jclLoadRec.classFunctionName[13], OS_MAX_PATH - 13, name);

        wcscpy(jclLoadRec.srcFileName, L"Unknown Source File");

        wcsncpy_truncate(jclLoadRec.jncFileName, OS_MAX_PATH, jncFileName->c_str());

        gJCLWriter->WriteLoadRecord(&jclLoadRec);
    }
} // writeNativeToJCLFile


#if 0
// hasInlinedFunctions
//
// Checks whether a compiled method contains inlined methods
//
static jint hasInlinedFunctions(jvmtiCompiledMethodLoadRecordHeader* record)
{
    jint hasInlineInfo = 0;

    if ((nullptr != record)
        && (record->kind == JVMTI_CMLR_INLINE_INFO))
    {
        jvmtiCompiledMethodLoadInlineRecord* inlineRec =
            (jvmtiCompiledMethodLoadInlineRecord*) record;

        if (inlineRec != nullptr && inlineRec->pcinfo != nullptr)
        {
            for (int i = 0; i < inlineRec->numpcs; i++)
            {
                PCStackInfo pcinfo = inlineRec->pcinfo[i];

                if (pcinfo.numstackframes > 1)
                {
                    hasInlineInfo = 1;
                }
            }
        }
    }

    return hasInlineInfo;
} // hasInlinedFunctions
#endif // 0


//
//    JVM Agent initialize and finalize routines
//

// Agent_OnLoad
//
JNIEXPORT jint JNICALL Agent_OnLoad(JavaVM* vm, char* options, void* reserved)
{
    GT_UNREFERENCED_PARAMETER(options);
    GT_UNREFERENCED_PARAMETER(reserved);
    int                  ret = 0;
    jint                 res;
    jvmtiEnv*            jvmti = nullptr;
    jvmtiEventCallbacks  callbacks;
    jvmtiCapabilities    capabilities;
    jvmtiError           error;
    std::wstring         profileDataDir;

    gJvmPID = getpid();

    // Get the jvmti environment
    res = vm->GetEnv((void**)&jvmti, JVMTI_VERSION_1);

    if (res != JNI_OK)
    {
        // This means that the VM was unable to obtain this version of the
        // JVMTI interface, this is a fatal error.
        handleFatalError(jvmti,
                         "Unable to access JVMTI Version 1 (0x%x)."
                         " Is your J2SE a 1.5 or newer version?"
                         " JNIEnv's GetEnv() returned %d\n",
                         JVMTI_VERSION_1, res);
        return JNI_ERR;
    }

    // Get the dir path in which the java profile data files will be written
    // Note: getProfileDataDir gives gSampleDir and profileDataDir
    ret = getProfileDataDir(profileDataDir);

    if (-1 == ret)
    {
        handleFatalError(jvmti, "Could not get profile data dir for Java");
        return JNI_ERR;
    }

    if (gJvmtiVerbose)
    {
        wprintf(L"The Java Profile data dir is " WIDE_STR_FORMAT L".\n",
                profileDataDir.c_str());
    }

    // Save the profileDataDir for future use..
    gProfileDataDir = profileDataDir;

#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
    // The directory access right is 771
    int j = RecursiveMkdir(profileDataDir.c_str());

    if (0 != j)
    {
        wprintf(L"The sample directory " WIDE_STR_FORMAT L" was not able to be created\n",
                profileDataDir.c_str());
        return JNI_ERR;
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
            handleFatalError(jvmti, "Cannot create directory %s",
                             profileDataDir.c_str());
            return JNI_ERR;
        }
    }

#endif

    // Save the profileDataDir for future use..
    gProfileDataDir = profileDataDir;

    // Get the command-line to obtain the Java Application name
    // gJvmCmd will contain the name of Java Application
    GetCmdline();

    // Construct the JCL filename
    wchar_t jclFile[OS_MAX_PATH] = { L'\0' };

    swprintf(jclFile, OS_MAX_PATH - 1, WIDE_STR_FORMAT PATH_SEPARATOR L"%d.jcl", profileDataDir.c_str(), gJvmPID);

    // Initialize JCLWriter
#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
    gJCLWriter = new JclWriter(jclFile, gJvmCmd.c_str(), gJvmPID, false);
#else
    gJCLWriter = new JclWriter(jclFile, gJvmCmd.c_str(), gJvmPID, false);
#endif

    if (nullptr == gJCLWriter)
    {
        handleFatalError(jvmti, "Error: Out of Memory");
        return JNI_ERR;
    }

    // This just writes the header record
    gJCLWriter->Initialize();

    // Preserve the jvmti environment, in case if it is required
    // during unload
    gJvmti = jvmti;

    // Add the required capabilities; We need the following capabilities
    //     can_generate_compiled_method_load_events
    //     can_get_source_file_name
    //            - required for GetSourceFileName()
    //     can_get_line_numbers
    //            - required for GetLineNumberTable()
    //
    //     Note: enabling "can_generate_method_entry_events" will prevent
    //     jvm from inlining. Hence do not use it
    //
    //     The capabilities that are set to 1 will be added. All previous
    //     capabilities are retained
    //
    (void)memset(&capabilities, 0, sizeof(capabilities));

    capabilities.can_generate_compiled_method_load_events = 1;
    capabilities.can_get_source_file_name                 = 1;
    capabilities.can_get_line_numbers                     = 1;

    error = jvmti->AddCapabilities(&capabilities);

    if (0 != handleJvmtiError(jvmti, error, "Failed to get capabilities"))
    {
        return JVMTI_ERROR_INTERNAL;
    }

    // Next we need to provide the callback function pointers to this jvmtiEnv
    (void)memset(&callbacks, 0, sizeof(callbacks));

    // Event-Callbacks are implemented for the following Events
    //     VMInit
    //     JVMTI_EVENT_COMPILED_METHOD_LOAD
    //     JVMTI_EVENT_COMPILED_METHOD_UNLOAD
    //     JVMTI_EVENT_DYNAMIC_CODE_GENERATED
    //     JVMTI_EVENT_CLASS_LOAD
    //
    callbacks.VMInit               = &cbVMInit;
    callbacks.CompiledMethodLoad   = &cbCompiledMethodLoad;
    callbacks.CompiledMethodUnload = &cbCompiledMethodUnload;
    callbacks.DynamicCodeGenerated = &cbDynamicCodeGenerated;
    callbacks.ClassLoad            = &cbClassLoad;

    error = jvmti->SetEventCallbacks(&callbacks,
                                     (jint)sizeof(callbacks));

    if (0 != handleJvmtiError(jvmti, error, "Cannot set jvmti callbacks"))
    {
        // FIXME: should i return JVMTI_ERROR_INTERNAL ?
        // if JVMTI_ERROR_NONE is not returned, VM will get terminated !!
        return JVMTI_ERROR_NONE;
    }

    // Register the interesting Events
    //     JVMTI_EVENT_VM_INIT
    //     JVMTI_EVENT_COMPILED_METHOD_LOAD
    //     JVMTI_EVENT_COMPILED_METHOD_UNLOAD
    //     JVMTI_EVENT_DYNAMIC_CODE_GENERATED
    //     JVMTI_EVENT_CLASS_LOAD
    //
    error = jvmti->SetEventNotificationMode(JVMTI_ENABLE,
                                            JVMTI_EVENT_VM_INIT,
                                            (jthread)nullptr);

    if (0 != handleJvmtiError(jvmti, error, "Cannot set event notification"))
    {
        return JVMTI_ERROR_NONE;
    }

    error = jvmti->SetEventNotificationMode(JVMTI_ENABLE,
                                            JVMTI_EVENT_COMPILED_METHOD_LOAD,
                                            (jthread)nullptr);

    if (0 != handleJvmtiError(jvmti, error, "Cannot set event notification"))
    {
        return JVMTI_ERROR_NONE;
    }

    error = jvmti->SetEventNotificationMode(JVMTI_ENABLE,
                                            JVMTI_EVENT_COMPILED_METHOD_UNLOAD,
                                            (jthread)nullptr);

    if (0 != handleJvmtiError(jvmti, error, "Cannot set event notification"))
    {
        return JVMTI_ERROR_NONE;
    }

    error = jvmti->SetEventNotificationMode(JVMTI_ENABLE,
                                            JVMTI_EVENT_DYNAMIC_CODE_GENERATED,
                                            (jthread)nullptr);

    if (0 != handleJvmtiError(jvmti, error, "Cannot set event notification"))
    {
        return JVMTI_ERROR_NONE;
    }

    error = jvmti->SetEventNotificationMode(JVMTI_ENABLE,
                                            JVMTI_EVENT_CLASS_LOAD,
                                            (jthread)nullptr);

    if (0 != handleJvmtiError(jvmti, error, "Cannot set event notification"))
    {
        return JVMTI_ERROR_NONE;
    }

    return JVMTI_ERROR_NONE;
} // Agent_OnLoad


//
// This is called immediately before the shared library is unloaded.
// This is the last code executed.
//
JNIEXPORT void JNICALL Agent_OnUnload(JavaVM* vm)
{
    GT_UNREFERENCED_PARAMETER(vm);
    // TODO: Should i reset the Event notification and clear the CBs ?

    // Baskar: Sometimes we get the Agent_OnUnload notification while we are
    // in the process of the writing into the JCL file. During that time deleting
    // gJCLWriter will lead to crash. Ideally, here, we need to wait for the
    // JCLWriter to finish its job before deleting.
#if 0

    if (nullptr != gJCLWriter)
    {
        delete gJCLWriter;
        gJCLWriter = nullptr;
    }

#endif //0

} // Agent_OnUnload


//
//    CALLBACK routines
//

// cbVMInit
//
// Callback to VM Initialization Event - VMInit
//

void JNICALL cbVMInit(jvmtiEnv* pJvmtiEnv, JNIEnv* pJniEnv, jthread thread)
{
    GT_UNREFERENCED_PARAMETER(pJvmtiEnv);
    GT_UNREFERENCED_PARAMETER(pJniEnv);
    GT_UNREFERENCED_PARAMETER(thread);
    // pJvmtiEnv->GenerateEvents (JVMTI_EVENT_COMPILED_METHOD_LOAD);
    // pJvmtiEnv->GenerateEvents (JVMTI_EVENT_DYNAMIC_CODE_GENERATED);
} // cbVMInit


// cbCompiledMethodLoad
//
void JNICALL cbCompiledMethodLoad(jvmtiEnv*                    pJvmtiEnv,
                                  jmethodID                    method,
                                  jint                         codeSize,
                                  const void*                  codeAddr,
                                  jint                         mapLength,
                                  const jvmtiAddrLocationMap*  map,
                                  const void*                  compileInfo)
{
    jclass       declaringClass;
    char*        pClassSig = nullptr;
    char*        pMethodName = nullptr;
    char*        pMethodSig = nullptr;
    char*        pSrcFile = nullptr;
    gtUInt64     loadTimeStamp = 0;
    std::wstring jncFile;
    methodInfo   mInfo{nullptr, nullptr, nullptr};

    // Get the module load timestamp
    QueryCurrentTime(loadTimeStamp);

    if (nullptr == gJCLWriter)
    {
        return;
    }

    // Construct the JNC filename
    if (gReJitMap.end() == gReJitMap.find((gtUInt64)codeAddr))
    {
        gReJitMap[(gtUInt64)codeAddr] = 0;
    }

    wchar_t tmpPath[OS_MAX_PATH] = { L'\0' };
    swprintf(tmpPath, OS_MAX_PATH - 1, WIDE_STR_FORMAT PATH_SEPARATOR L"JITCode-%p-%d.jnc",
             gProfileDataDir.c_str(), codeAddr, gReJitMap[(gtUInt64)codeAddr]);
    jncFile = tmpPath;

    ++gReJitMap[(gtUInt64)codeAddr];

    // Get the class signature and Method name
    pJvmtiEnv->GetMethodDeclaringClass(method, &declaringClass);
    pJvmtiEnv->GetClassSignature(declaringClass, &pClassSig, nullptr);
    char parsedClassName[OS_MAX_PATH] = {0};

    if (parseClassSignature(pClassSig, parsedClassName))
    {
        mInfo.pClassSig = parsedClassName;
    }
    else
    {
        mInfo.pClassSig = (char*)gUnknownClassName;
    }

    // Get the Method name, signature
    pJvmtiEnv->GetMethodName(method, &pMethodName, &pMethodSig, nullptr);
    char parsedMethodName[OS_MAX_PATH] = {0};

    if (parseMethodSignature(pMethodName, pMethodSig, parsedMethodName))
    {
        mInfo.pMethodName = parsedMethodName;
    }
    else
    {
        mInfo.pMethodName = (char*)gUnknownMethodName;
    }

    // Get the Source filename of the class
    pJvmtiEnv->GetSourceFileName(declaringClass, &pSrcFile);
    mInfo.pSrcFile = (nullptr != pSrcFile) ? pSrcFile : (char*)gUnknownSrcFile;

    if (gJvmtiVerbose)
    {
        fwprintf(stderr, L"\ncbCompileMethodLoad : 0x%p: " CSTR_FORMAT L"::" CSTR_FORMAT L"",
                 codeAddr, mInfo.pClassSig, mInfo.pMethodName);

        fwprintf(stderr, L"\njncFile name : " WIDE_STR_FORMAT, jncFile.c_str());
        fwprintf(stderr, L"\nsource file : " CSTR_FORMAT, mInfo.pSrcFile);

        fwprintf(stderr, L"\nmethod id : %p", method);
        fwprintf(stderr, L"\ncode addr : %p", codeAddr);
        fwprintf(stderr, L"\ncode size : %d", codeSize);
        fwprintf(stderr, L"\nload timestamp : %llu", loadTimeStamp);

        fwprintf(stderr, L"\nmap length : %d", mapLength);

        if (nullptr != map)
        {
            fwprintf(stderr, L"\nmap start addr : %p\n", map->start_address);
        }
        else
        {
            fwprintf(stderr, L"\nMap is NULL\n.");
        }
    }

    // Write the LOAD record in JCL file
    writeJCLFile(pJvmtiEnv,
                 method,
                 codeSize,
                 codeAddr,
                 &jncFile,  // JNC file name
                 &mInfo,
                 loadTimeStamp);   // method info

    // Write the JNCFile for this compiled method
    WriteJncFile(pJvmtiEnv,
                 method,
                 codeSize,
                 codeAddr,
                 mapLength,
                 map,
                 &jncFile,      // JNC File name
                 &mInfo,        // method info
                 compileInfo);

    if (nullptr != pClassSig)
    {
        pJvmtiEnv->Deallocate((unsigned char*)pClassSig);
    }

    if (nullptr != pMethodName)
    {
        pJvmtiEnv->Deallocate((unsigned char*)pMethodName);
    }

    if (nullptr != pSrcFile)
    {
        pJvmtiEnv->Deallocate((unsigned char*)pSrcFile);
    }
} // cbCompiledMethodLoad


// cbCompileMethodUnload
//
void JNICALL cbCompiledMethodUnload(jvmtiEnv*    pJvmtiEnv,
                                    jmethodID    method,
                                    const void*  codeAddr)
{
    if (nullptr == gJCLWriter)
    {
        return;
    }

    if (gJvmtiVerbose)
    {
        char* classSig = nullptr;
        jclass declaringClass;
        pJvmtiEnv->GetMethodDeclaringClass(method, &declaringClass);
        pJvmtiEnv->GetClassSignature(declaringClass, &classSig, nullptr);

        char* pMethodName = nullptr;
        pJvmtiEnv->GetMethodName(method, &pMethodName, nullptr, nullptr);

        fprintf(stderr, "DEBUG: UNLOAD: 0x%p: %s:%s\n", codeAddr, classSig, pMethodName);
        pJvmtiEnv->Deallocate((unsigned char*)classSig);
        pJvmtiEnv->Deallocate((unsigned char*)pMethodName);
    }

    JitUnloadRecord jclUnloadRec;
    gtUInt64 unloadTimeStamp = 0;

    QueryCurrentTime(unloadTimeStamp);

    jclUnloadRec.unloadTimestamp = unloadTimeStamp;
    jclUnloadRec.blockStartAddr  = reinterpret_cast<gtUInt64>(codeAddr);

    gJCLWriter->WriteUnloadRecord(&jclUnloadRec);
} // cbCompiledMethodUnload


// cbDynamicCodeGenerated
//
void JNICALL cbDynamicCodeGenerated(jvmtiEnv* pJvmtiEnv, const char* name, const void* codeAddr, jint codeSize)
{
    GT_UNREFERENCED_PARAMETER(pJvmtiEnv);

    if (gJvmtiVerbose)
    {
        fprintf(stderr, "cbDynamicCodeGenerated event\n");
        fprintf(stderr, "cbd DEBUG INFO: %p: %s %d\n", codeAddr, name, codeSize);
        fprintf(stderr, "cbd Method name: %s\n", name);
        fprintf(stderr, "cbd Start address: 0x%p\n", codeAddr);
        fprintf(stderr, "cbd End address: 0x%zd\n\n", ((size_t)codeAddr + codeSize));
    }

    // Construct the JNC filename
    std::wstring jncFile;

    if (gReJitMap.end() == gReJitMap.find((gtUInt64)codeAddr))
    {
        gReJitMap[(gtUInt64)codeAddr] = 0;
    }

    wchar_t tmpPath[OS_MAX_PATH] = { L'\0' };
    swprintf(tmpPath, OS_MAX_PATH - 1, WIDE_STR_FORMAT PATH_SEPARATOR L"JITCode-%p-%d.jnc",
             gProfileDataDir.c_str(), codeAddr, gReJitMap[(gtUInt64)codeAddr]);
    jncFile = tmpPath;

    ++gReJitMap[(gtUInt64)codeAddr];

    wchar_t methodName[OS_MAX_PATH];
    memset(methodName, 0, sizeof(methodName));

    mbstowcs_truncate(methodName, OS_MAX_PATH, name);

    // write JCL Load record
    writeNativeToJCLFile(methodName,
                         codeSize,
                         codeAddr,
                         &jncFile);

    // write the JNC file for dynamic code generated
    WriteNativeToJncFile(name,          // Method name
                         codeAddr,
                         codeSize,
                         &jncFile);    // JNC file name
} //cbDynamicCodeGenerated


void JNICALL cbClassLoad(jvmtiEnv* pJvmtiEnv, JNIEnv* pJniEnv, jthread thread, jclass klass)
{
    GT_UNREFERENCED_PARAMETER(pJvmtiEnv);
    GT_UNREFERENCED_PARAMETER(pJniEnv);
    GT_UNREFERENCED_PARAMETER(thread);
    GT_UNREFERENCED_PARAMETER(klass);
}
