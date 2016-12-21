//==================================================================================
// Copyright (c) 2012-2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file CommandsHandler.cpp
/// \brief Implementation of the CodeXL component profile engine logic
///
//==================================================================================
// $Id: //devtools/main/CodeXL/Components/CpuProfiling/AMDTCpuProfiling/src/CommandsHandler.cpp#208 $
// Last checkin:   $11/03/2014 16:19$
// Last edited by: $Karthik Thatipamula$
// Change list:    $491206$
//=============================================================

// Ignore warnings:
#include <AMDTBaseTools/Include/gtIgnoreCompilerWarnings.h>

//Qt
#include <QtCore>
#include <QtWidgets>
#include <QtCore/QObject>

// Standard Lib
#if AMDT_BUILD_TARGET == AMDT_LINUX_OS
    #include <unistd.h>
#endif

// Infra:
#include <AMDTApplicationComponents/Include/acFunctions.h>

// AMDTApplicationFramework:
#include <AMDTApplicationFramework/Include/afGlobalVariablesManager.h>
#include <AMDTApplicationFramework/Include/afMessageBox.h>

// Shared profiling:
#include <SharedProfileManager.h>

//CpuPerfEvent
#include <AMDTCpuPerfEventUtils/inc/EventEngine.h>

//Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTOSWrappers/Include/osOSDefinitions.h>
#include <AMDTOSWrappers/Include/osDirectory.h>
#include <AMDTOSWrappers/Include/osProcess.h>
#include <AMDTOSWrappers/Include/osDebugLog.h>
#include <AMDTOSWrappers/Include/osCpuid.h>
#include <AMDTOSWrappers/Include/osTime.h>
#include <AMDTOSWrappers/Include/osApplication.h>
#include <AMDTOSWrappers/Include/osGeneralFunctions.h>
#include <AMDTOSWrappers/Include/osUser.h>
#include <AMDTOSWrappers/Include/osProductVersion.h>
#include <AMDTAPIClasses/Include/Events/apExecutionModeChangedEvent.h>

#include <AMDTApplicationComponents/Include/acMessageBox.h>

// AMDTApplicationFramework:
#include <AMDTApplicationFramework/Include/afProjectManager.h>
#include <AMDTApplicationFramework/Include/afProgressBarWrapper.h>
#include <AMDTApplicationFramework/Include/afApplicationCommands.h>

#include <AMDTAPIClasses/Include/Events/apEventsHandler.h>
#include <AMDTAPIClasses/Include/Events/apProfileProgressEvent.h>
#include <AMDTAPIClasses/Include/Events/apProfileProcessTerminatedEvent.h>

// AMDTSharedProfiling:
#include <AMDTSharedProfiling/inc/ProfileApplicationTreeHandler.h>
#include <AMDTSharedProfiling/inc/SharedProfileSettingPage.h>
#include <AMDTSharedProfiling/inc/StringConstants.h>

//Local:
#include <inc/CommandsHandler.h>
#include <inc/AmdtCpuProfiling.h>
#include <inc/SessionViewCreator.h>
#include <inc/CpuProjectHandler.h>
#include <inc/CpuProfileTreeHandler.h>
#include <inc/ProfileConfigs.h>
#include <inc/StringConstants.h>
#include <inc/ProcessMonitor.h>
#include <inc/CpuTranslationMonitor.h>
#include <inc/StdAfx.h>
#include <inc/CpuRetryDialog.h>

#include <AMDTExecutableFormat/inc/ExecutableFile.h>


#ifdef PROTO_ONLY
#include "LinuxHack.h"
typedef void* osProcessHandle;
static bool osLaunchSuspendedProcess(const osFilePath& executablePath, const gtString& arguments,
                                     const osFilePath& workDirectory, osProcessId& processId, osProcessHandle& processHandle,
                                     osThreadHandle& processThreadHandle, bool createWindow = true,
                                     bool redirectFiles = false) { return false; }
static bool osResumeSuspendedProcess(const osProcessId& processId, const osProcessHandle& processHandle,
                                     const osThreadHandle& processThreadHandle, bool closeHandles) { return false; }

#endif

static bool IsUserAcceptingProfiling();
static bool IsUserAcceptingPerfCssWarning();

// JVMTI Agent library name
#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
    #define JVMTIAGENT32_NAME  L"CXLJvmtiAgent"        AMDT_DEBUG_SUFFIX_W  AMDT_BUILD_SUFFIX_W
    #define JVMTIAGENT64_NAME  L"CXLJvmtiAgent-x64"    AMDT_DEBUG_SUFFIX_W  AMDT_BUILD_SUFFIX_W
#else
    #ifdef NDEBUG
        #define JVMTIAGENT32_NAME  L"libCXLJvmtiAgent"
        #define JVMTIAGENT64_NAME  L"libCXLJvmtiAgent-x64"
    #else
        #define JVMTIAGENT32_NAME  L"libCXLJvmtiAgent-d"
        #define JVMTIAGENT64_NAME  L"libCXLJvmtiAgent-x64-d"
    #endif
#endif

#define JAVA_PROFILE_ENABLED    1

#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
    #include <ProfilingAgents/AMDTClrProfAgent/inc/ClrProfAgent.h>
#endif // AMDT_WINDOWS_OS

//static member initialization:
CommandsHandler* CommandsHandler::m_pMySingleInstance = nullptr;


CommandsHandler::CommandsHandler(): m_state(CPUPROF_STATE_INVALID), m_profilingEnabled(false) , m_processMonitor(nullptr)
{
    // Initialize the tree data (we use it to transfer file path)
    m_profileSession.m_pParentData = new afApplicationTreeItemData;
}

CommandsHandler::~CommandsHandler()
{
    // Unregister myself from listening to debugged process events:
    apEventsHandler::instance().unregisterEventsObserver(*this);

    fnReleaseProfiling();

    delete m_profileSession.m_pParentData;
}

bool CommandsHandler::registerInstance(CommandsHandler* pCommandsHandlerInstance)
{
    bool retVal = false;

    // Do not allow multiple registration for my instance:
    GT_IF_WITH_ASSERT(nullptr == m_pMySingleInstance)
    {
        m_pMySingleInstance = pCommandsHandlerInstance;
        retVal = m_pMySingleInstance->initialize();
    }

    return retVal;
}

bool CommandsHandler::initialize()
{
    // Register myself to listen to debugged process events:
    apEventsHandler::instance().registerEventsObserver(*this, AP_APPLICATION_COMPONENTS_EVENTS_HANDLING_PRIORITY);

    //Connect to the shared profile manager
    QObject::connect(&(SharedProfileManager::instance()), SIGNAL(profileStarted(const gtString&, const spISharedProfilerPlugin * const, osProcessId)),
                     this, SLOT(onProfileStarted(const gtString&, const spISharedProfilerPlugin * const, osProcessId)));
    QObject::connect(&(SharedProfileManager::instance()), SIGNAL(profileBreak(const bool&, const spISharedProfilerPlugin * const)),
                     this, SLOT(onProfilePaused(const bool&, const spISharedProfilerPlugin * const)));
    QObject::connect(&(SharedProfileManager::instance()), SIGNAL(profileStopped(const spISharedProfilerPlugin * const, bool)),
                     this, SLOT(onProfileStopped(const spISharedProfilerPlugin * const, bool)));
    QObject::connect(this, SIGNAL(profileEnded()), &(SharedProfileManager::instance()), SLOT(onProfileEnded()));

    //Get the list of profiles available for this system
    gtVector<gtString> proList = ProfileConfigs::instance().getListOfProfiles();
    gtVector<gtString>::const_iterator it = proList.begin();
    gtVector<gtString>::const_iterator itEnd = proList.end();
    QStringList guiList;

    for (; it != itEnd; it++)
    {
#if (AMDT_BUILD_TARGET != AMDT_WINDOWS_OS)

        if (CLU_PROFILE_NAME == *it)
        {
            continue;
        }

#endif //(AMDT_BUILD_TARGET != AMDT_WINDOWS_OS)

        //Register each with the shared profile manager
        gtString profileName = CPU_PREFIX;
        profileName.append(*it);
        SharedProfileManager::instance().registerProfileType(profileName, this, CP_STR_cpuProfileTreePathString, (SPM_ALLOW_PAUSE | SPM_ALLOW_STOP));

        ProfileApplicationTreeHandler::instance()->registerSessionTypeTreeHandler(acGTStringToQString(profileName), &CpuProfileTreeHandler::instance());
    }


    // Make sure that the time based profiling is the current session type in the framework:
    gtString selectedProfile = SharedProfileManager::instance().selectedSessionTypeName();
    apExecutionModeChangedEvent executionModeEvent(PM_STR_PROFILE_MODE, SharedProfileManager::instance().indexForSessionType(selectedProfile), true);
    apEventsHandler::instance().registerPendingDebugEvent(executionModeEvent);


    return true;
}

CommandsHandler* CommandsHandler::instance()
{
    return m_pMySingleInstance;
}


void CommandsHandler::ReportErrorMessage(bool appendDriverError, const gtString& userMessage, const wchar_t* pFormatStringForLog, ...) const
{
    // Get a pointer to the variable list of arguments:
    va_list argptr;
    va_start(argptr, pFormatStringForLog);

    const unsigned int sizeErrBuffer = 512U;
    wchar_t errBuffer[sizeErrBuffer];

    // Write the formatted string into the buffer:
    int size = vswprintf(errBuffer, sizeErrBuffer - 1U, pFormatStringForLog, argptr);

    // The buffer was big enough to contains the formatted string:
    if (0 < size)
    {
        // Terminate the string manually:
        errBuffer[size] = L'\0';

        if (appendDriverError && static_cast<unsigned>(size) < sizeErrBuffer)
        {
            fnGetLastProfileError(sizeErrBuffer - static_cast<unsigned>(size), &errBuffer[size]);
        }

        pFormatStringForLog = errBuffer;
    }

    // Output the message to the log:
    OS_OUTPUT_DEBUG_LOG(pFormatStringForLog, OS_DEBUG_LOG_ERROR);

    // If the user message is not empty, output it:
    if (!userMessage.isEmpty())
    {
        acMessageBox::instance().critical(afGlobalVariablesManager::ProductNameA(), acGTStringToQString(userMessage));
    }

    // Terminate the argptr pointer:
    va_end(argptr);
}

void CommandsHandler::ReportHelpMessage(const gtString& userMessage) const
{
    // If the user message is not empty, output it:
    if (!userMessage.isEmpty())
    {
        acMessageBox::instance().information(AF_STR_InformationA, acGTStringToQString(userMessage));
    }
}

HRESULT CommandsHandler::enableProfiling()
{
    HRESULT hr = S_OK;

    if (CPUPROF_STATE_INVALID == m_state)
    {
        hr = fnEnableProfiling();
        m_profilingEnabled = SUCCEEDED(hr);

        if (!m_profilingEnabled)
        {
            ReportErrorMessage(true, CP_STR_FailedToLoadDriver, CP_STR_FailedToLoadDriver);
            hr = E_HANDLE;
        }
        else
        {
            m_state = CPUPROF_STATE_READY;
        }
    }

    return hr;
}

HRESULT CommandsHandler::initializeSessionInfo()
{
    HRESULT hr = S_OK;

    // Get the next iteration for the profile name:
    m_profileSession.m_name.clear();
    osDirectory projectPath;
    afProjectManager::instance().currentProjectFilePath().getFileDirectory(projectPath);
    osDirectory baseDir;
    gtString projName;
    afProjectManager::instance().currentProjectFilePath().getFileName(projName);
    gtString profileName;

    // Get the name for the next session:
    if (!ProfileApplicationTreeHandler::instance()->GetNextSessionNameAndDir(projName, projectPath, profileName, baseDir))
    {
        // Failed to create the profile output dir
        ReportErrorMessage(false, AF_STR_Empty, CP_STR_FailedToCreateOutputFolder, baseDir.directoryPath().asString().asCharArray());
        hr = E_INVALIDARG;
    }

    m_profileSession.m_displayName = acGTStringToQString(profileName);

    osTime timing;
    timing.setFromCurrentTime();

    gtString startTime;
    timing.dateAsString(startTime, osTime::NAME_SCHEME_FILE, osTime::LOCAL);

    m_profileSession.m_startTime = acGTStringToQString(startTime);

    // Sanity check:
    GT_IF_WITH_ASSERT(m_profileSession.m_pParentData != nullptr)
    {
        // Get the project path:
        m_profileSession.m_pParentData->m_filePath.clear();
        m_sessionPath = baseDir.directoryPath().asString();

        m_profileSession.m_pParentData->m_filePath.setFileDirectory(m_sessionPath);
        m_profileSession.m_pParentData->m_filePath.setFileName(profileName);
        m_profileSession.m_pParentData->m_filePath.setFileExtension(DATA_EXT);
        m_profileSession.m_pParentData->m_filePathLineNumber = AF_TREE_ITEM_PROFILE_CPU_OVERVIEW;

        if (m_sessionPath.isEmpty())
        {
            // If the session path wasn't available, bad profile
            ReportErrorMessage(false, AF_STR_Empty, CP_STR_FailedToCreateOutputFolder, m_sessionPath.asCharArray());
            hr = E_INVALIDARG;
        }

        // Create the session directory, if necessary
        if (S_OK == hr)
        {
            osDirectory dirCheck;
            dirCheck.setDirectoryFullPathFromString(m_sessionPath);

            if (!dirCheck.exists())
            {
                dirCheck.create();
            }
        }

        if (SUCCEEDED(hr))
        {
#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
            // Windows currently requires backslashes
            m_sessionPath.appendFormattedString(L"\\%ls.%ls", profileName.asCharArray(), PRD_EXT);
            m_sessionPath.replace(L"/", L"\\");
#else
            m_sessionPath.appendFormattedString(L"/%ls.%ls", profileName.asCharArray(), CAPERF_EXT);
            m_sessionPath.replace(L"\\", L"/");
#endif
        }

    }

    return hr;
}

HRESULT CommandsHandler::setupTimerConfiguration()
{
    HRESULT hr = S_OK;

    //set up timer options
    if (0.0f < m_profileSession.m_msInterval)
    {
        //Convert the resolution to micro seconds
        unsigned int resolution = static_cast<unsigned int>(m_profileSession.m_msInterval * 1000.0f);

        // Profile only on selected cores specified in the CPU Affinity Mask
        // TODO: supporting affinity mask for more than 64 cores (now profile can happen only on cores 0-64)
        // Mohit: we also use the CPU Affinity Mask to set the affinity mask of the launched
        // process which restrict the launched process to run only on the cores specified in the
        // CPU Affinity Mask. This does not seem correct while doing system-wide profiling because
        // for the launched process, the samples shown, are collected from whole process execution
        // but for all other processes, they are collected only from the process execution which happened
        // on the cores specified in the of the CPU Affinity Mask. It seems that CodeAnalyst also had
        // this flaw. Probably at some point, we may want to think about it.
        hr = fnSetTimerConfiguration(m_profileSession.m_startAffinity, &resolution);
    }

    return hr;
}



HRESULT CommandsHandler::setupEventConfiguration(gtString& errorMessage)
{
    HRESULT hr = S_OK;

    if (0 < m_profileSession.m_eventsVector.size())
    {
        osCpuid cpuInfo;

        if (cpuInfo.hasHypervisor())
        {
            if (cpuInfo.getHypervisorVendorId() == HV_VENDOR_VMWARE)
            {
                // VMware supports EBP, only if vPMC is enabled in the guest VM settings
                // Notify user to enable vPMC to perform EBP
                // VMware driver (guest OS) generates BSOD if events are counted in OS mode
                // Notify user that OS events will not be counted
                gtString helpMessage = L"Make sure \'Virtualize CPU performance Counters\' is enabled in"
                                       L"virtual machine settings before running any event based profiling.\n\n"
                                       L"OS mode events will not be counted within guest OS.";
                ReportHelpMessage(helpMessage);
            }
            else if (cpuInfo.getHypervisorVendorId() == HV_VENDOR_MICROSOFT)
            {
#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS

                // EBP is supported on Hyper-V Windows 10 parent/child partition
                // EBP is *not* supported on Hyper-V Windows 8/8.1 parent partition
                if (cpuInfo.isHypervisorRootPartition())
                {
                    osWindowsVersion windowsVersion;

                    if (osGetWindowsVersion(windowsVersion) && windowsVersion != OS_WIN_10)
                    {
                        errorMessage.makeEmpty();
                        errorMessage.appendFormattedString(L"Event based profiling is not supported on Windows 8, 8.1 or older with Hyper-V enabled.\n");

                        hr = E_NOTIMPL;
                    }
                }

#endif
#if AMDT_BUILD_TARGET == AMDT_LINUX_OS
                // EBP is not supported on Hyper-V Linux child partition
                wchar_t vendorName[32] = { 0 };
                cpuInfo.getHypervisorVendorString(vendorName, 32);

                errorMessage.makeEmpty();
                errorMessage.appendFormattedString(L"Hypervisor Detected: %ls\n", vendorName);
                errorMessage.appendFormattedString(L"Event based profiling is not supported on Guest OS.\n");

                hr = E_NOTIMPL;
#endif
            }
            else
            {
                wchar_t vendorName[32] = { 0 };
                cpuInfo.getHypervisorVendorString(vendorName, 32);

                errorMessage.makeEmpty();
                errorMessage.appendFormattedString(L"Virtual Machine Detected : %ls\n", vendorName);
                errorMessage.appendFormattedString(L"Event based profiling is not supported on Guest OS.\n");

                hr = E_NOTIMPL;
            }
        }

#if AMDT_BUILD_TARGET == AMDT_LINUX_OS
        else
        {
            // Xen Dom0 detection process is little different
            if (access("/proc/xen/capabilities", F_OK) == 0)
            {
                errorMessage.makeEmpty();
                errorMessage.appendFormattedString(L"Virtual Machine Detected : Xen (Dom0)\n");
                errorMessage.appendFormattedString(L"Event based profiling is not supported on Xen Host OS.\n");

                hr = E_NOTIMPL;
            }
        }

#endif

        EventConfiguration* pDriverEvents = nullptr;

        if (SUCCEEDED(hr))
        {
            hr = verifyAndSetEvents(&pDriverEvents);
        }

        if (SUCCEEDED(hr))
        {
            // Profile only on selected cores specified in the CPU Affinity Mask
            // TODO: supporting affinity mask for more than 64 cores (now profile can happen only on cores 0-64)
            hr = fnSetEventConfiguration(pDriverEvents, m_profileSession.m_eventsVector.size(),
                                         reinterpret_cast<gtUInt64*>(&m_profileSession.m_startAffinity), 1U);

        }

        // Don't allow the event array allocated in verifyAndSetEventsto leak
        if (nullptr != pDriverEvents)
        {
            delete [] pDriverEvents;
        }
    }

    return hr;
}

HRESULT CommandsHandler::setupIbsConfiguration(gtString& errorMessage)
{
    HRESULT hr = S_OK;

    if (m_profileSession.m_fetchSample || m_profileSession.m_opSample || m_profileSession.m_cluSample)
    {
        if (!isEventFileAvailable())
        {
            hr = E_FAIL;
        }

#if AMDT_BUILD_TARGET == AMDT_LINUX_OS

        if (SUCCEEDED(hr))
        {
            // Xen Dom0 detection process is little different
            if (access("/proc/xen/capabilities", F_OK) == 0)
            {
                errorMessage.makeEmpty();
                errorMessage.appendFormattedString(L"Virtual Machine Detected : Xen (Dom0)\n");
                errorMessage.appendFormattedString(L"Event based profiling is not supported on Xen Host OS.\n");

                hr = E_NOTIMPL;
            }
        }

#endif

#if AMDT_BUILD_TARGET == AMDT_LINUX_OS
#if 0
        // For Linux kernel less than 3.17, IBS is supported only in System-Wide Mode.
        bool isSystemWide = (m_profileSession.m_profileScope != PM_PROFILE_SCOPE_SINGLE_EXE);

        if (SUCCEEDED(hr) && !isSystemWide)
        {
            int majorVersion, minorVersion, buildNumber;

            if (osGetOperatingSystemVersionNumber(majorVersion, minorVersion, buildNumber) &&
                (3 > majorVersion || (3 == majorVersion && 17 > minorVersion)))
            {
                errorMessage = L"IBS is supported only with System-Wide Profile / System-Wide Profile with focus on application.\n"
                               L"Please enable it in Profile Settings";

                hr = E_NOTIMPL;
            }
        }

#endif // 0
#endif // AMDT_LINUX_OS

        if (SUCCEEDED(hr))
        {
            bool isIbsAvailable = false;
            hr = fnGetIbsAvailable(&isIbsAvailable);

            if (!isIbsAvailable)
            {
                errorMessage = L"IBS is not supported on kernels older than 3.5";
                hr = E_NOTIMPL;
            }
        }

        if (SUCCEEDED(hr))
        {
            bool opSample = false;
            bool opCycleCount = false;
            unsigned long opInterval = 0;

            if (m_profileSession.m_opSample || m_profileSession.m_cluSample)
            {
                opSample = true;

                // Using IBS Op settings when profiling both IBS Op and CLU
                if (m_profileSession.m_opSample)
                {
                    opCycleCount = m_profileSession.m_opCycleCount;
                    opInterval = m_profileSession.m_opInterval;
                }
                else
                {
                    opCycleCount = m_profileSession.m_cluCycleCount;
                    opInterval = m_profileSession.m_cluInterval;
                }
            }

            // Profile only on selected cores specified in the CPU Affinity Mask
            // TODO: supporting affinity mask for more than 64 cores (now profile can happen only on cores 0-64)
            hr = fnSetIbsConfiguration(m_profileSession.m_fetchSample ? m_profileSession.m_fetchInterval : 0,
                                       opSample ? opInterval : 0,
                                       true, (!opCycleCount), (gtUInt64*)&m_profileSession.m_startAffinity, 1);
        }
    }

    return hr;
}

bool CommandsHandler::saveEnvironmentVariables(gtList<osEnvironmentVariable>& envVars)
{
    bool retVal = true;
    m_profileSession.m_envVariables.makeEmpty();

    const gtList<osEnvironmentVariable>& appEnvVars = afProjectManager::instance().currentProjectSettings().environmentVariables();

    for (gtList<osEnvironmentVariable>::const_iterator it = appEnvVars.begin(), itEnd = appEnvVars.end(); it != itEnd; ++it)
    {
        const gtString& evName = (*it)._name;
        const gtString& evValue = (*it)._value;

        // Get the current value the current environment variable:
        osEnvironmentVariable currentVal;
        currentVal._name = evName;
        osGetCurrentProcessEnvVariableValue(currentVal._name, currentVal._value);
        envVars.push_back(currentVal);

        // Set the current environment value in this process environment block:
        if (!osSetCurrentProcessEnvVariable(*it))
        {
            retVal = false;
            break;
        }

        m_profileSession.m_envVariables.appendFormattedString(L"%ls=%ls;", evName.asCharArray(), evValue.asCharArray());
    }

    return retVal;
}

void CommandsHandler::loadEnvironmentVariables(const gtList<osEnvironmentVariable>& envVars)
{
    //Restore the stored environmental variables
    for (gtList<osEnvironmentVariable>::const_iterator it = envVars.begin(), itEnd = envVars.end(); it != itEnd; ++it)
    {
        // Set the current environment value in this process environment block:
        osSetCurrentProcessEnvVariable(*it);
    }
}

bool CommandsHandler::trySetupClrProfilingEnvironment(gtList<osEnvironmentVariable>& envVars) const
{
    bool retVal;
#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
    retVal = true;
    osEnvironmentVariable corEnProf;
    corEnProf._name = gtString(L"Cor_Enable_Profiling");
    osGetCurrentProcessEnvVariableValue(corEnProf._name, corEnProf._value);
    envVars.push_back(corEnProf);
    osEnvironmentVariable setCorEnProf;
    setCorEnProf._name = corEnProf._name;
    setCorEnProf._value = gtString(L"0x01");
    osSetCurrentProcessEnvVariable(setCorEnProf);

    osEnvironmentVariable profCompSet;
    profCompSet._name = gtString(L"COMPLUS_ProfAPI_ProfilerCompatibilitySetting");
    osGetCurrentProcessEnvVariableValue(profCompSet._name, profCompSet._value);
    envVars.push_back(profCompSet);
    osEnvironmentVariable setProfCompSet;
    setProfCompSet._name = profCompSet._name;
    setProfCompSet._value = gtString(L"EnableV2Profiler");
    osSetCurrentProcessEnvVariable(setProfCompSet);

    osEnvironmentVariable corProfiler;
    corProfiler._name = gtString(L"COR_PROFILER");
    osGetCurrentProcessEnvVariableValue(corProfiler._name, corProfiler._value);
    envVars.push_back(corProfiler);
    osEnvironmentVariable setCorProfiler;
    setCorProfiler._name = corProfiler._name;
    setCorProfiler._value = L"{" _T(CLSID_AMDTClrProfAgent) L"}";

    osSetCurrentProcessEnvVariable(setCorProfiler);
#else
    GT_UNREFERENCED_PARAMETER(envVars);
    retVal = false;
#endif // AMDT_WINDOWS_OS
    return retVal;
}

bool CommandsHandler::trySetupJavaProfilingEnvironment(gtString& javaAgentArg, bool is64BitApp) const
{
    bool retVal = false;
#ifdef JAVA_PROFILE_ENABLED

    // Check if the launched application is java
    if (m_profileSession.m_exeFullPath.endsWith("java.exe") || m_profileSession.m_exeFullPath.endsWith("java"))
    {
        // Get the current application path
        osFilePath jvmtiAgentPath;

        if (osGetCurrentApplicationDllsPath(jvmtiAgentPath) || osGetCurrentApplicationPath(jvmtiAgentPath))
        {
            // Set the agentpath stuff
            jvmtiAgentPath.setFileName(is64BitApp ? JVMTIAGENT64_NAME : JVMTIAGENT32_NAME);
            jvmtiAgentPath.setFileExtension(OS_MODULE_EXTENSION);

            if (jvmtiAgentPath.exists())
            {
                javaAgentArg = L" -agentpath:";

#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
                javaAgentArg += L'\"';
#endif
                javaAgentArg += jvmtiAgentPath.asString();
#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
                javaAgentArg += L'\"';
#endif
                javaAgentArg += L' ';

                retVal = true;
            }
        }
    }

#endif // JAVA_PROFILE_ENABLED
    return retVal;
}

HRESULT CommandsHandler::tryStartProfiling(int retries) const
{
    HRESULT hr;
    gtString msg;
    int tryNum = 0;
    bool canceled = false;

    do
    {
        if (tryNum > 0)
        {
            msg.makeEmpty();
            msg.appendFormattedString(L"Retry %d to start the CPU profile", tryNum);
            OS_OUTPUT_DEBUG_LOG(msg.asCharArray(), OS_DEBUG_LOG_INFO);
        }

        hr = fnStartProfiling(
                        (0 < m_profileSession.m_startDelay || m_profileSession.m_isProfilePaused), //startPaused
                        m_profileSession.m_isProfilePaused,                                        //pauseIndefinite
                        AMDT_CPU_PROFILING_PAUSE_KEY,
                        nullptr);

        if (S_OK != hr)
        {
            //Log the error
            wchar_t errStr[256];
            fnGetLastProfileError(256, errStr);
            msg.makeEmpty();
            msg.appendFormattedString(L"The driver failed to start profiling. (error code 0x%lx)\n\n%ls", hr, errStr);
            OS_OUTPUT_DEBUG_LOG(msg.asCharArray(), OS_DEBUG_LOG_ERROR);

            if ((HRESULT)E_LOCKED == hr)
            {
                msg.prepend(L"Attempted to wait and retry to get the hardware counter lock\n");

                //Put up retry modal dialog
                CpuRetryDialog retry;
                canceled = (QDialog::Accepted != acMessageBox::instance().doModal((QDialog*)&retry));
            }
        }
    } // BUG?  retry

    while (hr == (HRESULT)E_LOCKED && ++tryNum <= retries && !canceled);

    if (S_OK != hr)
    {
        acMessageBox::instance().critical(CPU_PROF_MESSAGE, acGTStringToQString(msg));
    }
    else
    {
        wchar_t profileType[128] = { L'\0' };
        m_profileSession.m_profileTypeStr.toWCharArray(profileType);
        //Log the profile start
        msg = L"CPU Profile ";
        msg += profileType;
        msg += L" started";
        OS_OUTPUT_DEBUG_LOG(msg.asCharArray(), OS_DEBUG_LOG_INFO);
    }

    return hr;
}

unsigned int* CommandsHandler::constructAttachPidArray(unsigned int& launchedPid, unsigned int& count) const
{
    // Currently CodeXL does not support to profile an already existing process. Of course the
    // user can profile in System-wide mode to profile the already running processes. But in
    // System-wide mode, CodeXL will collect the CSS samples only for the launched application.
    // Hence the user won't be able to see callgraph for the interesting processes that are
    // not launched through CodeXL.
    //
    // Gabor Sines and Mironov Mikhail of Multimedia driver team wanted to use CodeXL to profile
    // a webcam Metro app on Win8. They need to improve the performance and provide the numbers
    // to Microsoft. They are interested in CSS. Since they cannot launch their metro app and its
    // already running, they cannot collect CSS for their metro app.
    //
    // This hack provides a way by which they can specify the PIDs of the existing processes
    // and the CodeXL will collect CSS for the launched application and for the PIDS specified
    // in an env variable "AMD_CODEXL_ATTACH_PID". This env-var contains comma separated PIDs.
    //
    const gtString evName(L"AMD_CODEXL_ATTACH_PID");
    gtString attachPidStr;
    osGetCurrentProcessEnvVariableValue(evName, attachPidStr);

    count = 0U;
    unsigned int* pPidArray;

    if (!attachPidStr.isEmpty())
    {
        QStringList pidStrList = acGTStringToQString(attachPidStr).split(",");
        pPidArray = new unsigned int[pidStrList.size() + 1];

        // Add the PIDs of already existing processes that are specified in AMD_CODEXL_ATTACH_PID env variable
        for (int i = 0; i < pidStrList.size(); i++)
        {
            pPidArray[count++] = pidStrList.at(i).toInt();
        }

        // Add the PID of the launched application
        pPidArray[count++] = launchedPid;
    }
    else
    {
        pPidArray = &launchedPid;
        count++;
    }

    return pPidArray;
}


HRESULT CommandsHandler::SetupProfileSession(const gtString& profileType, osProcessId processId, ProfileSessionScope scope, bool& isLaunchingJava, gtString& projectCmdlineArguments, gtString& errorMessage)
{
    HRESULT retVal = S_OK;

    m_state = CPUPROF_STATE_PROFILING;

    // Get project settings:
    m_profileSession.CopyFrom(CpuProjectHandler::instance().getProjectSettings(), true);
    m_profileSession.m_profileTypeStr = acGTStringToQString(profileType);

    // If the debug level is appropriate, save the raw cpu profile files
    m_profileSession.m_shouldSaveRawFiles = osDebugLog::instance().loggedSeverity() >= OS_DEBUG_LOG_DEBUG;

    if (m_profileSession.m_isProfilePaused)
    {
        m_state = CPUPROF_STATE_PAUSED;
    }

    bool externalProcess = (0 != processId);

    if (PROCESS_ID_UNBOUND == processId)
    {
        m_profileSession.SetShouldCollectCSS(false);
    }

    m_profileSession.m_profileScope = scope;

    const apProjectSettings& project = afProjectManager::instance().currentProjectSettings();
    const bool isWindowsStoreApp = !project.windowsStoreAppUserModelID().isEmpty();

    if (!externalProcess)
    {
        if (!isWindowsStoreApp)
        {
            //Test if the project target exists
            if (!project.executablePath().exists())
            {
                errorMessage.makeEmpty();
                errorMessage.appendFormattedString(L"The target executable doesn't exist: %ls", project.executablePath().asString().asCharArray());
                retVal = E_INVALIDARG;
            }

            if (SUCCEEDED(retVal))
            {
                const gtString& exeFullPath = project.executablePath().asString();
                m_profileSession.m_exeFullPath = acGTStringToQString(exeFullPath);

                ExecutableFile* pExe = ExecutableFile::Open(exeFullPath.asCharArray());

                if (nullptr != pExe)
                {
                    isLaunchingJava = trySetupJavaProfilingEnvironment(projectCmdlineArguments, pExe->Is64Bit());
                    delete pExe;
                }
            }
        }

        if (SUCCEEDED(retVal))
        {
            projectCmdlineArguments += project.commandLineArguments();
            m_profileSession.m_commandArguments = acGTStringToQString(projectCmdlineArguments);

            // Test if the project working directory exists
            if (!project.workDirectory().exists())
            {
                errorMessage.makeEmpty();
                errorMessage.appendFormattedString(L"The working directory doesn't exist: %ls", project.workDirectory().asString().asCharArray());
                retVal = E_INVALIDARG;
            }

            m_profileSession.m_workingDirectory = acGTStringToQString(project.workDirectory().asString());
        }
    }

    else if (PROCESS_ID_UNBOUND != processId)
    {
        osModuleArchitecture processArch;
        osRuntimePlatform processPlatform;
        gtString executablePath, commandLine, workDirectory;

        if (!osGetProcessLaunchInfo(processId, processArch, processPlatform, executablePath, commandLine, workDirectory))
        {
            errorMessage.makeEmpty();
            errorMessage.appendFormattedString(L"Cannot attach to process ID: %llu", static_cast<gtUInt64>(processId));
            retVal = E_FAIL;
        }

        if (OS_JAVA_PLATFORM == processPlatform ||
            OS_DOT_NET_PLATFORM == processPlatform)
        {
            m_profileSession.SetShouldCollectCSS(false);
        }

        m_profileSession.m_exeFullPath = acGTStringToQString(executablePath);
        m_profileSession.m_commandArguments = acGTStringToQString(commandLine);
        m_profileSession.m_workingDirectory = acGTStringToQString(workDirectory);

        QString postfix;
        postfix.sprintf(" (%s)", PM_STR_AttachedSessionPostfix);
        m_profileSession.m_name.append(postfix);
    }
    else
    {
        m_profileSession.m_exeFullPath.clear();
        m_profileSession.m_commandArguments.clear();
        m_profileSession.m_workingDirectory.clear();

        QString postfix;
        postfix.sprintf(" (%s)", PM_STR_SystemWideSessionPostfix);
        m_profileSession.m_name.append(postfix);
    }

    if (S_OK == retVal && !ProfileConfigs::instance().getProfileConfigByType(profileType, &m_profileSession))
    {
        //If the name wasn't found, bad profile
        errorMessage.makeEmpty();
        errorMessage.appendFormattedString(L"The named profile wasn't found: %ls", profileType.asCharArray());
        retVal = E_INVALIDARG;
    }

    if (SUCCEEDED(retVal))
    {

        // Disable CSS if we are doing CLU profiling only (either by CLU or by Custom)
        if (m_profileSession.IsProfilingCluOnly())
        {
            m_profileSession.SetShouldCollectCSS(false);
        }

        // Clear previous configurations:
        fnClearConfigurations();

        retVal = initializeSessionInfo();

        if (SUCCEEDED(retVal))
        {
            retVal = fnSetProfileOutputFile((wchar_t*)m_sessionPath.asCharArray());

            if (!SUCCEEDED(retVal))
            {
                errorMessage.makeEmpty();
                errorMessage.appendFormattedString(L"There was a problem configuring the profile session. (error code 0x%lx)\n\n", retVal);
            }
        }

        //set up timer options
        if (SUCCEEDED(retVal))
        {
            retVal = setupTimerConfiguration();

            if (!SUCCEEDED(retVal))
            {
                errorMessage.makeEmpty();
                errorMessage.appendFormattedString(L"There was a problem configuring the timer profile. (error code 0x%lx)\n\n", retVal);
            }
        }

        if (SUCCEEDED(retVal))
        {
            errorMessage.makeEmpty();
            retVal = setupEventConfiguration(errorMessage);

            if (!SUCCEEDED(retVal))
            {
                if (errorMessage.isEmpty())
                {
                    errorMessage.appendFormattedString(L"There was a problem configuring the event profile. (error code 0x%lx)\n\n", retVal);
                }
            }
        }

        if (SUCCEEDED(retVal))
        {
            errorMessage.makeEmpty();
            retVal = setupIbsConfiguration(errorMessage);

            if (!SUCCEEDED(retVal))
            {
                if (errorMessage.isEmpty())
                {
                    errorMessage.appendFormattedString(L"There was a problem configuring the IBS profile. (error code 0x%lx)\n\n", retVal);
                }
            }
        }
    }

    return retVal;
}


HRESULT CommandsHandler::onStartProfiling(const gtString& profileType, osProcessId processId)
{
    HRESULT retVal = S_FALSE;

    // profile scope is shared-profile-setting and so we need to grab it from SharedProfileSettingPage
    ProfileSessionScope scope = SharedProfileSettingPage::Instance()->CurrentSharedProfileSettings().m_profileScope;

    //[BUG436008]:Warning message needs to be shown for "System wide profile with focus on app" as well
    bool isSystemWide = (scope == PM_PROFILE_SCOPE_SYS_WIDE) || (scope == PM_PROFILE_SCOPE_SYS_WIDE_FOCUS_ON_EXE);

    if (!isProfiling() && (!isSystemWide || IsUserAcceptingProfiling()))
    {
        retVal = enableProfiling();

        if (S_OK == retVal)
        {
            // Setup the session:
            gtString setupErrorMessage, projectCmdlineArguments;
            bool isLaunchingJava = false;
            retVal = SetupProfileSession(profileType, processId, scope, isLaunchingJava, projectCmdlineArguments, setupErrorMessage);

            if (SUCCEEDED(retVal))
            {
                retVal = LaunchProfileSession(processId, isLaunchingJava, projectCmdlineArguments, setupErrorMessage);

                if (!SUCCEEDED(retVal))
                {
                    // Output an error message to the user and to the log:
                    ReportErrorMessage(true, setupErrorMessage, setupErrorMessage.asCharArray());
                }
            }
            else
            {
                // Output an error message to the user and to the log:
                ReportErrorMessage(true, setupErrorMessage, setupErrorMessage.asCharArray());
            }
        }
    }

    return retVal;
}


HRESULT CommandsHandler::LaunchProfileSession(osProcessId processId, bool isLaunchingJava, const gtString& projectCmdlineArguments, gtString& errorMessage)
{
    GT_UNREFERENCED_PARAMETER(isLaunchingJava);
    HRESULT retVal = S_OK;

    bool externalProcess = (0 != processId);

    //launch suspended profile
    osProcessHandle processHandle = nullptr;
    osThreadHandle processThreadHandle;
    bool bCreateWindow = true;

#if AMDT_BUILD_TARGET == AMDT_LINUX_OS
    // NOTE [Suravee]:
    // On Linux, the osProcess uses "xterm -e" to launch target app
    // by default when setting bCreateWindow to true.
    // This has several undesired side effects for Cpu profiling such as:
    // - xterm showing in osv tab.
    // - xterm kills the target application as soon as it exits, which not
    //   always the preferred behavior.
    //
    // Instead, CXL should properly handle stdin/stdout/stderr in the GUI.
    // For now, we will disable "create windows" in Linux, and the
    // stdin/stdout/stderr of the target application
    // will be showing at the command line window that launched CXL.
    bCreateWindow = false;
#endif

    // Saved values of overwritten environment variables for launched process
    gtList<osEnvironmentVariable> storedEnvVars;

    if (!saveEnvironmentVariables(storedEnvVars))
    {
        errorMessage.makeEmpty();
        errorMessage.append(L"There was a problem setting the environment variables");
        retVal = E_INVALIDARG;
    }

    trySetupClrProfilingEnvironment(storedEnvVars);

    bool wasProcessLaunched = false;
    const apProjectSettings& project = afProjectManager::instance().currentProjectSettings();
    const bool isWindowsStoreApp = !project.windowsStoreAppUserModelID().isEmpty();

    if (SUCCEEDED(retVal))
    {
        if (!externalProcess)
        {
            if (isWindowsStoreApp)
            {
                osFilePath exePath;
                wasProcessLaunched = osLaunchSuspendedWindowsStoreApp(project.windowsStoreAppUserModelID(),
                                                                      projectCmdlineArguments,
                                                                      processId,
                                                                      processHandle,
                                                                      exePath);

                if (wasProcessLaunched)
                {
                    m_profileSession.m_exeFullPath = acGTStringToQString(exePath.asString());
                }
            }
            else
            {
                wasProcessLaunched = osLaunchSuspendedProcess(project.executablePath(),
                                                              projectCmdlineArguments,
                                                              project.workDirectory().asFilePath(),
                                                              processId,
                                                              processHandle,
                                                              processThreadHandle,
                                                              bCreateWindow,
                                                              true);
            }

            if (!wasProcessLaunched)
            {
                gtString exeName = isWindowsStoreApp ? project.windowsStoreAppUserModelID().asCharArray() : project.executablePath().asString().asCharArray();
                errorMessage.makeEmpty();
                errorMessage = CP_STR_FailedToLaunchTarget;
                errorMessage.append(exeName);
                retVal = E_FAIL;
            }
        }
        else if (PROCESS_ID_UNBOUND != processId)
        {
            gtString fileName;
            osFilePath attachedTargetPath(acQStringToGTString(m_profileSession.m_exeFullPath));

            if (!attachedTargetPath.isEmpty())
            {
                attachedTargetPath.getFileName(fileName);
            }

            if (!fileName.isEmpty() && fileName != afProjectManager::instance().currentProjectSettings().projectName())
            {
                QString postfix;
                postfix.sprintf(" (%s", PM_STR_AttachedSessionPostfix);
                m_profileSession.m_displayName.append(postfix);
                m_profileSession.m_displayName.append(" - ");
                m_profileSession.m_displayName.append(acGTStringToQString(fileName));
            }
            else
            {
                QString postfix;
                postfix.sprintf(" (%s)", PM_STR_AttachedSessionPostfix);
                m_profileSession.m_displayName.append(postfix);
            }

            m_profileSession.m_displayName.append(')');
        }
        else
        {
            QString postfix;
            postfix.sprintf(" (%s)", PM_STR_SystemWideSessionPostfix);
            m_profileSession.m_displayName.append(postfix);
        }
    }

    unsigned int pid = static_cast<unsigned int>(processId);

    if (SUCCEEDED(retVal))
    {
        if (!externalProcess)
        {
            osSetProcessAffinityMask(processId, processHandle, m_profileSession.m_startAffinity);

            // Log the app launch
            gtString msg;
            msg.appendFormattedString(L"CPU Profile launched application: (%ld) '%ls %ls' Working directory: '%ls'%ls%ls",
                                      pid,
                                      m_profileSession.m_exeFullPath.toStdWString().c_str(),
                                      m_profileSession.m_commandArguments.toStdWString().c_str(),
                                      m_profileSession.m_workingDirectory.toStdWString().c_str(),
                                      m_profileSession.m_envVariables.isEmpty() ? L"" : L" Environmental variables: ",
                                      m_profileSession.m_envVariables.asCharArray());
            OS_OUTPUT_DEBUG_LOG(msg.asCharArray(), OS_DEBUG_LOG_INFO);
        }
    }

    loadEnvironmentVariables(storedEnvVars);

    unsigned int* pPidArray = nullptr;
    unsigned int numPids = 0U;

    if (SUCCEEDED(retVal))
    {
        if (PROCESS_ID_UNBOUND == processId)
        {
            pid = static_cast<unsigned int>(osGetCurrentProcessId());
        }

        // Construct the array of PIDs for which we want to do Call-stack sampling
        pPidArray = constructAttachPidArray(pid, numPids);

        // Swarup - Argument to ShouldCollectCSS() should be 'true', else CSS collection will be enabled for CLR apps.
        // And eventually the driver will collect some meaningless callstack records which are processed
        // but not reported by GUI - waste of time at collection/translation/reporting.
        // CodeXL does not yet support CSS for managed apps like - CLR & Java.
        if (m_profileSession.ShouldCollectCSS(true))
        {
            if (!IsUserAcceptingPerfCssWarning())
            {
                return E_FAIL;
            }

            retVal = fnSetCallStackSampling(pPidArray, numPids,
                                            m_profileSession.GetCssUnwindLevel(),
                                            m_profileSession.m_cssInterval,
                                            m_profileSession.m_cssScope,
                                            m_profileSession.IsFpoChecked());

            if (!SUCCEEDED(retVal))
            {
                errorMessage.makeEmpty();
                errorMessage.appendFormattedString(L"There was a problem configuring the call stack sampling. (error code 0x%lx)\n\n", retVal);
            }
        }
        else
        {
            m_profileSession.SetShouldCollectCSS(false);
        }
    }

    //By default, limit the profile to launched process and children
    if (SUCCEEDED(retVal))
    {
        bool isSystemWide = (m_profileSession.m_profileScope != PM_PROFILE_SCOPE_SINGLE_EXE);

#if AMDT_BUILD_TARGET == AMDT_LINUX_OS

        // On Linux IBS, SWP should always be true.
        if (m_profileSession.m_fetchSample || m_profileSession.m_opSample || m_profileSession.m_cluSample)
        {
            isSystemWide = true;
        }

#endif

        retVal = fnSetFilterProcesses(pPidArray, numPids, isSystemWide);

        if (!SUCCEEDED(retVal))
        {
            errorMessage.makeEmpty();
            errorMessage.appendFormattedString(L"There was a problem setting the process filter. (error code 0x%lx)\n\n", retVal);
        }
    }

    if (1U < numPids)
    {
        delete[] pPidArray;
    }

    // Take note of the profiling start time
    osTime current;
    current.setFromCurrentTime();

    if (SUCCEEDED(retVal))
    {
        retVal = tryStartProfiling(2);
    }

    if (S_OK != retVal)
    {
        m_state = CPUPROF_STATE_READY;

        if (!externalProcess && 0 != processId)
        {
            // Kill the launched process, because the profiling failed to start
            osTerminateProcess(processId);
        }
    }
    else
    {
        // Profile started successfully
        qApp->setOverrideCursor(QCursor(Qt::BusyCursor));

        afProgressBarWrapper::instance().ShowProgressBar(L"Cpu Profiling", 100);

        // Start thread keeping track of process termination
        m_processMonitor = new ProfileProcessMonitor(processId, &m_profileSession, current, externalProcess);
        m_processMonitor->execute();

        if (!externalProcess)
        {
            // Resume the suspended process, now that the profile will catch the startup
            if (isWindowsStoreApp)
            {
                osResumeSuspendedWindowsStoreApp(processHandle, true);
            }
            else
            {
                osResumeSuspendedProcess(processId, processHandle, processThreadHandle, true);
            }
        }
    }

    return retVal;
}

HRESULT CommandsHandler::onTogglePauseProfiling(bool pause)
{
    HRESULT hr = S_FALSE;

    if (pause)
    {
        if (!isPaused())
        {
            // Invoke Pause API
            hr = fnPauseProfiling(nullptr);
            m_state = CPUPROF_STATE_PAUSED;
        }
    }
    else
    {
        if (isPaused())
        {
            // Invoke Resume API
            hr = fnResumeProfiling(nullptr);
            m_state = CPUPROF_STATE_PROFILING;
        }
    }

    return hr;
}

void CommandsHandler::onEvent(const apEvent& eve, bool& vetoEvent)
{
    apEvent::EventType eveType = eve.eventType();

    if (apEvent::AP_PROFILE_PROCESS_TERMINATED == eveType && isProfiling())
    {

        const apProfileProcessTerminatedEvent& processTermEvent = dynamic_cast<const apProfileProcessTerminatedEvent&>(eve);

        if (processTermEvent.profilerName() == CPU_STR_PROJECT_EXTENSION)
        {
            OS_OUTPUT_DEBUG_LOG(L"Launched application terminated", OS_DEBUG_LOG_INFO);
            onStopProfiling();
            vetoEvent = true;
        }

        osCloseProcessRedirectionFiles();
    }
    else if (apEvent::AP_PROFILE_PROGRESS_EVENT == eveType)
    {
        const apProfileProgressEvent& progressEvent = dynamic_cast<const apProfileProgressEvent&>(eve);

        if (progressEvent.profileName() == CPU_STR_PROJECT_EXTENSION)
        {
            if (!progressEvent.aborted())
            {
                if (progressEvent.increment())
                {
                    afProgressBarWrapper::instance().incrementProgressBar(progressEvent.value());
                }
                else
                {
                    afProgressBarWrapper::instance().updateProgressBar(progressEvent.value());
                }

                if (!progressEvent.progress().isEmpty())
                {
                    gtString update = progressEvent.progress();

                    if (isPaused())
                    {
                        update.append(L" - Profiling Paused ... Waiting for resume");
                    }

                    afProgressBarWrapper::instance().setProgressText(update);
                }
            }
            else
            {
                afProgressBarWrapper::instance().hideProgressBar();

                if (100 == progressEvent.value())
                {
                    afProgressBarWrapper::instance().hideProgressBar();

                    if (S_OK == m_translatedRet)
                    {
                        //If we should remove raw files
                        if (!m_profileSession.m_shouldSaveRawFiles)
                        {
                            //standardize the path for the system
                            if (osFilePath::osPathSeparator != L'\\')
                            {
                                m_sessionPath.replace(gtString(L'\\'), osFilePath::osPathSeparator);
                            }
                            else
                            {
                                m_sessionPath.replace(gtString(L'/'), osFilePath::osPathSeparator);
                            }

                            //Remove the raw data file
                            osFile rawFile(m_sessionPath);
                            rawFile.deleteFile();
#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
                            // On windows, remove the supporting task info file:
                            osFilePath tiFilePath(m_sessionPath);
                            tiFilePath.setFileExtension(TI_EXT);
                            osFile tiFile(tiFilePath);
                            tiFile.deleteFile();
#endif
                            // Remove the supporting run info file
                            osFilePath riFilePath(m_sessionPath);
                            riFilePath.setFileExtension(RI_EXT);
                            osFile riFile(riFilePath);
                            riFile.deleteFile();
                        }

                        if (m_profileSession.m_isImported)
                        {
                            GT_IF_WITH_ASSERT(m_profileSession.m_pParentData != nullptr)
                            {
                                // Get the imported file full path
                                QString importedFileFullPath = acGTStringToQString(m_profileSession.m_pParentData->m_filePath.asString());

                                bool imported;
#if AMDT_BUILD_TARGET != AMDT_WINDOWS_OS
                                gtString strSep(osFilePath::osPathSeparator);
                                QString strSeparator(acGTStringToQString(strSep));
                                int lastIndex = importedFileFullPath.lastIndexOf(strSeparator);
                                QString strTemp = importedFileFullPath.mid(lastIndex + 1);
                                strTemp += '.';
                                strTemp += QString::fromWCharArray(DATA_EXT);
                                importedFileFullPath.append(strSeparator);
                                importedFileFullPath.append(strTemp);
#endif
                                CpuProjectHandler::instance().onImportSession(importedFileFullPath, imported);
                            }
                        }

                        else
                        {
                            // Create a new data for the new profile:
                            CPUSessionTreeItemData* pNewItemData = new CPUSessionTreeItemData(m_profileSession);

                            // Add to project list
                            CpuProjectHandler::instance().addSession(pNewItemData, true);
                        }

                        // Save the session list to the project
                        afApplicationCommands::instance()->OnFileSaveProject();
                    }
                    else if ((E_NODATA == m_translatedRet) || (m_translatedRet == E_INVALIDDATA))
                    {
                        // Output a message box with the translation error
                        QString message = (m_translatedRet == E_NODATA) ? CP_Str_TranslateErrorNoData : CP_Str_TranslateErrorInvalidData;
                        acMessageBox::instance().information(afGlobalVariablesManager::instance().ProductNameA(), message);
                    }

                    qApp->restoreOverrideCursor();

                    // Allow another profile now that the translation has finished
                    emit profileEnded();
                    m_processMonitor = nullptr;

                    // Koushik:BUG379141 :: First import should not change the state to
                    // ready, set it ready only if it was already a valid state like
                    // paused/stopped.
                    if (m_state != CPUPROF_STATE_INVALID)
                    {
                        m_state = CPUPROF_STATE_READY;
                    }

                    m_sessionPath.makeEmpty();
                }
                else
                {
                    // Notify the user about the profile abort
                    ReportErrorMessage(false, AF_STR_Empty, progressEvent.progress().asCharArray());
                }
            }

            vetoEvent = true;
        }
    }
}


HRESULT CommandsHandler::onStopProfiling(bool stopAndExit)
{
    HRESULT hr = S_FALSE;

    if (isProfiling())
    {
        // End the profile session:
        osDebugLog::instance().EndSession();

        //terminate process monitor thread
        if (nullptr != m_processMonitor)
        {
            if (!m_processMonitor->processEnded())
            {
                m_processMonitor->terminate();
            }
        }

        //stop profile
        hr = fnStopProfiling();

        if (!SUCCEEDED(hr))
        {
            ReportErrorMessage(true, AF_STR_Empty, L"The driver failed to stop profiling. (error code 0x%lx)\n\n", hr);
        }
        else
        {
            //Log the profile end
            OS_OUTPUT_DEBUG_LOG(L"CPU Profile ended", OS_DEBUG_LOG_INFO);

            //Double check that there is a session file
            if (m_sessionPath.isEmpty())
            {
                hr = E_INVALIDPATH;
            }
        }

        // BUG380033: CXL crashes with segfault, while closing.
        // Don't translate if the user has request to exit while profiling in progress
        if (!stopAndExit)
        {
            osFilePath rawCheck(m_sessionPath);

            if (SUCCEEDED(hr) && (!rawCheck.exists()))
            {
                ReportErrorMessage(false, AF_STR_Empty, L"No profile file was generated:\n%ls", m_sessionPath.asCharArray());
                hr = E_NOFILE;
            }

            afProgressBarWrapper::instance().ShowProgressDialog(L"Finished profiling");

            // We have finished profiling the application and data translation is the next stage.
            // We cannot get this info directly from the process, since the process handler does not have a way to
            // report this state. Also, the actual process might or might not be launched/exited through CodeXL.
            // The best way for now to update the title bar string is from here, where we have a clear picture of the state.
            //TODO: Figure out a way to propagate this information to the Process Handlers.

            gtString titleBarString;
            int runmodes(0);
            afGetCodeXLTitleBarString(titleBarString, (runmodes | AF_DEBUGGED_PROCESS_DATA_TRANSLATING));
            afApplicationCommands::instance()->setApplicationCaption(titleBarString);

            ReaderHandle* pHandle = nullptr;

            if (SUCCEEDED(hr))
            {
                hr = fnOpenProfile(m_sessionPath.asCharArray(), &pHandle);

                if (!SUCCEEDED(hr))
                {
                    ReportErrorMessage(false, AF_STR_Empty, CP_strFailedToOpenRawProfileFile, m_sessionPath.asCharArray(), gtGetErrorString(hr), hr);
                    CpuProjectHandler::instance().emitFileImportedComplete();
                }
            }

            if (SUCCEEDED(hr))
            {
                GT_IF_WITH_ASSERT(m_profileSession.m_pParentData != nullptr)
                {
                    // construct the RI file path
                    osFilePath riFilePath(m_profileSession.m_pParentData->m_filePath);
                    riFilePath.setFileExtension(RI_EXT);

                    // set the current time as profile end time
                    gtString profEndTime;
                    osTime timing;
                    timing.setFromCurrentTime();
                    timing.dateAsString(profEndTime, osTime::NAME_SCHEME_FILE, osTime::LOCAL);

                    // populate RI data
                    RunInfo rInfo;
                    rInfo.m_targetPath = acQStringToGTString(m_profileSession.m_exeFullPath);
                    rInfo.m_wrkDirectory = acQStringToGTString(m_profileSession.m_workingDirectory);
                    rInfo.m_cmdArguments = acQStringToGTString(m_profileSession.m_commandArguments);
                    rInfo.m_envVariables = m_profileSession.m_envVariables;
                    rInfo.m_profType = acQStringToGTString(m_profileSession.m_profileTypeStr);
                    rInfo.m_profDirectory = riFilePath.fileDirectoryAsString();
                    rInfo.m_profStartTime = acQStringToGTString(m_profileSession.m_startTime);
                    rInfo.m_profEndTime = profEndTime;
                    rInfo.m_isCSSEnabled = m_profileSession.ShouldCollectCSS(false);
                    rInfo.m_cssUnwindDepth = m_profileSession.GetCssUnwindLevel();
                    rInfo.m_cssScope = m_profileSession.m_cssScope;
                    rInfo.m_isCssSupportFpo = m_profileSession.IsFpoChecked();
                    rInfo.m_cssInterval = m_profileSession.m_cssInterval;
                    rInfo.m_isProfilingClu = m_profileSession.m_cluSample;
                    rInfo.m_isProfilingIbsOp = m_profileSession.m_opSample;
                    rInfo.m_cpuAffinity = m_profileSession.m_startAffinity;
                    rInfo.m_cpuCount = m_profileSession.m_cores;

                    m_profileSession.m_endTime = acGTStringToQString(profEndTime);

                    // set profile scope
                    if (PM_PROFILE_SCOPE_SYS_WIDE == m_profileSession.m_profileScope)
                    {
                        rInfo.m_profScope = gtString(L"System-Wide");
                    }
                    else if (PM_PROFILE_SCOPE_SYS_WIDE_FOCUS_ON_EXE == m_profileSession.m_profileScope)
                    {
                        rInfo.m_profScope = gtString(L"System-Wide with focus on application");
                    }
                    else
                    {
                        rInfo.m_profScope = gtString(L"Single Application");
                    }

                    // set OS name
                    osGetOSShortDescriptionString(rInfo.m_osName);

                    // Get CodeXL version
                    osProductVersion cxlVersion;
                    osGetApplicationVersion(cxlVersion);
                    rInfo.m_codexlVersion = cxlVersion.toString();

                    // write the RI file
                    hr = fnWriteRunInfo(riFilePath.asString().asCharArray(), &rInfo);

                    if (!SUCCEEDED(hr))
                    {
                        ReportErrorMessage(false, AF_STR_Empty, L"Failed to write the RI file");
                    }
                }

            }

            if (SUCCEEDED(hr))
            {
#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
                //Windows needs the path including the data file
                startTranslating(pHandle, m_profileSession.m_pParentData->m_filePath.asString(), false);
#else

                // NOTICE: On linux, if the last character is '/', the ebp file path cannot be built.
                // This is a workaround for this issue:
                gtString sessionDir = osFilePath(m_profileSession.m_pParentData->m_filePath.asString()).fileDirectoryAsString();
                OS_OUTPUT_DEBUG_LOG(sessionDir.asCharArray(), OS_DEBUG_LOG_ERROR);

                if (sessionDir[sessionDir.length() - 1] == '/')
                {
                    sessionDir[sessionDir.length() - 1] = 0;
                }

                startTranslating(pHandle, sessionDir, false);
#endif
            }
            else
            {
                //If the profile failed, allow another one
                emit profileEnded();
                m_processMonitor = nullptr;
                m_state = CPUPROF_STATE_READY;
            }
        }
    }

    return hr;
}

void CommandsHandler::startTranslating(ReaderHandle* pHandle, const gtString& translatedPath, bool importing)
{
    if (importing && (m_profileSession.m_pParentData != nullptr))
    {
        osFilePath pathTest(translatedPath);
        gtString profName;
        pathTest.getFileName(profName);

        //Reset the session settings to defaults for the import
        CPUSessionTreeItemData reset;
        m_profileSession.CopyFrom(&reset, true);
        m_profileSession.m_name = acGTStringToQString(profName);
        m_profileSession.m_profileTypeStr.clear();
        m_profileSession.m_pParentData->m_filePath.setFullPathFromString(translatedPath);
        m_profileSession.m_isImported = importing;
    }

    afProgressBarWrapper::instance().setProgressRange(100);

    //Launch another thread to monitor the data translation
    m_translatedRet = S_OK;
    CpuTranslationMonitor* pTranslationMonitor = new CpuTranslationMonitor(pHandle, translatedPath, &m_translatedRet);
    pTranslationMonitor->start();
}

bool CommandsHandler::isProfiling() const
{
    //check launch state and profile API
    ProfileState testState(ProfilingUnavailable);

    if (CPUPROF_STATE_INVALID != m_state)
    {
        HRESULT hr = fnGetProfilerState(&testState);

        if (!SUCCEEDED(hr))
        {
            ReportErrorMessage(true, AF_STR_Empty, L"The driver failed to get the state. (error code 0x%lx)\n\n", hr);
            gtTriggerAssertonFailureHandler(_T(__FUNCTION__), _T(__FILE__), __LINE__, L"The driver failed to get the state");
        }
    }

    return ((m_state >= CPUPROF_STATE_PROFILING) && ((Profiling == testState) || (ProfilingPaused == testState)));
}

bool CommandsHandler::isPaused() const
{
    //check profile API
    ProfileState testState(ProfilingUnavailable);

    if (CPUPROF_STATE_INVALID != m_state)
    {
        HRESULT hr = fnGetProfilerState(&testState);

        if (!SUCCEEDED(hr))
        {
            ReportErrorMessage(true, AF_STR_Empty, L"The driver failed to get the state. (error code 0x%lx)\n\n", hr);
            gtTriggerAssertonFailureHandler(_T(__FUNCTION__), _T(__FILE__), __LINE__, L"The driver failed to get the state");
        }
    }

    return ((m_state == CPUPROF_STATE_PAUSED) && (ProfilingPaused == testState));
}

bool CommandsHandler::isProfilingOkay() const
{
    return m_profilingEnabled;
}

void CommandsHandler::onViewResetGUILayout()
{
    AmdtCpuProfiling::instance().resetGuiLayout();
}

//From shared profiling menu or toolbar
void CommandsHandler::onProfileStarted(const gtString& profileTypeStr, const spISharedProfilerPlugin* const pCallback, osProcessId processId)
{
    if (pCallback == static_cast<spISharedProfilerPlugin*>(this))
    {
        gtString profileTypeTemp = profileTypeStr;

        if (profileTypeTemp.startsWith(CPU_PREFIX))
        {
            // Block CPU profiling if we are in a remote session, as this
            // feature is currently not supported. Otherwise, start profiling.
            if (afProjectManager::instance().currentProjectSettings().isRemoteTarget())
            {
                // Notify the user.
                ReportErrorMessage(false, CP_strRemoteCPUProfileNotSupported, CP_strRemoteCPUProfileNotSupported);

                // Notify the system.
                emit profileEnded();
            }
            else
            {
                // CPU profiling session can be executed.
                profileTypeTemp.replace(CPU_PREFIX, L"", false);

                if (S_OK != onStartProfiling(profileTypeTemp, processId))
                {
                    //If the profile failed to start, stop the shared manager from thinking we're profiling
                    emit profileEnded();
                }
                else
                {
                    SharedProfileManager::instance().setPaused(CpuProjectHandler::instance().getProjectSettings()->m_isProfilePaused);
                }
            }
        }
    }
}

//From shared profiling toolbar
void CommandsHandler::onProfilePaused(const bool& toggled, const spISharedProfilerPlugin* const pCallback)
{
    if (pCallback == static_cast<spISharedProfilerPlugin*>(this))
    {
        onTogglePauseProfiling(toggled);
    }
}

//From shared profiling toolbar
void CommandsHandler::onProfileStopped(const spISharedProfilerPlugin* const pCallback, bool stopAndExit)
{
    if (pCallback == static_cast<spISharedProfilerPlugin*>(this))
    {
        onStopProfiling(stopAndExit);
    }
}

bool CommandsHandler::isEventFileAvailable()
{
    bool retVal = true;
    osCpuid cpuInfo;
    osFilePath eventFilePath(osFilePath::OS_CODEXL_DATA_PATH);
    eventFilePath.appendSubDirectory(L"Events");

    EventEngine eventEngine;
    osDirectory fileDirectory;

    eventFilePath.getFileDirectory(fileDirectory);

    if (!eventEngine.Initialize(fileDirectory))
    {
        ReportErrorMessage(false, AF_STR_Empty, L"Unable to find the event files directory: %ls", eventFilePath.asString().asCharArray());
        retVal = false;
    }
    else
    {
        EventsFile* pEventFile = nullptr;
        pEventFile = eventEngine.GetEventFile(cpuInfo.getFamily(), cpuInfo.getModel());
        retVal = (nullptr != pEventFile);

        if (retVal)
        {
            delete pEventFile;
        }
        else
        {
            ReportErrorMessage(false, AF_STR_Empty, L"Unable to find the event file: %ls",
                               eventEngine.GetEventFilePath(cpuInfo.getFamily(), cpuInfo.getModel()).asString().asCharArray());
        }
    }

    return retVal;
}

HRESULT CommandsHandler::verifyAndSetEvents(EventConfiguration** ppDriverEvents)
{
    HRESULT hr = S_OK;
    osCpuid cpuInfo;
    gtUInt32 model = cpuInfo.getModel();
    osFilePath eventFilePath(osFilePath::OS_CODEXL_DATA_PATH);
    eventFilePath.appendSubDirectory(L"Events");

    EventEngine eventEngine;
    osDirectory fileDirectory;

    eventFilePath.getFileDirectory(fileDirectory);

    if (!eventEngine.Initialize(fileDirectory))
    {
        ReportErrorMessage(false, AF_STR_Empty, L"Unable to find the event files directory: %ls", eventFilePath.asString().asCharArray());
        hr = E_NOFILE;
    }

    EventsFile* pEventFile = nullptr;

    if (S_OK == hr)
    {
        pEventFile = eventEngine.GetEventFile(cpuInfo.getFamily(), model);

        if (nullptr == pEventFile)
        {
            ReportErrorMessage(false, AF_STR_Empty, L"Unable to find the event file: %ls",
                               eventEngine.GetEventFilePath(cpuInfo.getFamily(), model).asString().asCharArray());
            hr = E_NOFILE;
        }
    }

    *ppDriverEvents = nullptr;

    if (S_OK == hr)
    {
        *ppDriverEvents = new EventConfiguration[m_profileSession.m_eventsVector.size()];
    }

    unsigned int evCounter = 0;
    unsigned int maxCounter = 0;
    fnGetEventCounters(&maxCounter);

    //duplicate counter allocation algorithm
    int* aAllocCount = new int[maxCounter];
    memset(aAllocCount, 0, (sizeof(int) * maxCounter));

    for (size_t i = 0; i < m_profileSession.m_eventsVector.size(); i++)
    {
        if (S_OK != hr)
        {
            break;
        }

        //validate the event
        unsigned int evSelect = GetEvent12BitSelect(m_profileSession.m_eventsVector[i].pmc);

        CpuEvent eventData;
        pEventFile->FindEventByValue(evSelect, eventData);

        //Don't allow events which are not valid on this cpu
        if (eventData.m_minValidModel > model)
        {
            gtString errorMessage;
            errorMessage.appendFormattedString(CP_STR_FailedToFindEvent, eventData.m_value);
            ReportErrorMessage(false, errorMessage, errorMessage.asCharArray());
            hr = E_NOTAVAILABLE;
            break;
        }

#if AMDT_BUILD_TARGET == AMDT_LINUX_OS

        if (FAMILY_KB == cpuInfo.getFamily())
        {
            if (eventData.m_source == "L2I")
            {
                ReportErrorMessage(false, L"", L"The configuration contains L2I event.\n"
                                   L"L2I PMC events are not yet supported by Linux PERF subsystem for CPU family 16h.\n"
                                   L"Please choose a different configuration");
                hr = E_NOTAVAILABLE;
                break;
            }
        }

#endif // AMDT_LINUX_OS

        if (FAMILY_OR == cpuInfo.getFamily())
        {
            gtString eventSource;
            eventSource.fromUtf8String(eventData.m_source);

            if (eventSource.startsWith(L"NB"))
            {
                gtString errorMessage;
                errorMessage.appendFormattedString(CP_STR_FailedToFindNBEvent, eventData.m_value, eventData.m_name.data());
                ReportErrorMessage(false, errorMessage, errorMessage.asCharArray());
                hr = E_NOTAVAILABLE;
                break;
            }

            //Family 15h limits the performance counters an event can be counted on, so
            // we have to distribute the requested events over the valid counters as
            // fairly as we can.
            unsigned int minCounter = static_cast<unsigned int>(-1);
            unsigned int curCounter = 0U;
            int minVal = 255;
            unsigned int countMask = eventData.m_counters;

            //check the count of events on each available counter
            while (countMask > 0)
            {
                if ((countMask & 1U) > 0)
                {
                    //available counter
                    if (aAllocCount[curCounter] < minVal)
                    {
                        minVal = aAllocCount[curCounter];
                        minCounter = curCounter;
                    }
                }

                countMask >>= 1;
                curCounter++;
            }

            //The distribution is fair across the available counters
            aAllocCount[minCounter]++;

            if (aAllocCount[minCounter] > 8)
            {
                ReportErrorMessage(false, CP_STR_FailedToSetCounterRange, CP_STR_FailedToSetCounterRange);
                hr = E_NOTAVAILABLE;
                break;
            }
        }

        //Save the event configuration that will be written to the hardware
        (*ppDriverEvents)[i].performanceEvent = m_profileSession.m_eventsVector[i].pmc.perf_ctl;
        (*ppDriverEvents)[i].value = m_profileSession.m_eventsVector[i].eventCount;
        (*ppDriverEvents)[i].eventCounter = evCounter;

        if (++evCounter == maxCounter)
        {
            evCounter = 0;
        }
    }

    if (S_OK != hr && nullptr != *ppDriverEvents)
    {
        delete [] *ppDriverEvents;
        *ppDriverEvents = nullptr;
    }

    if (nullptr != aAllocCount)
    {
        delete [] aAllocCount;
    }

    if (nullptr != pEventFile)
    {
        delete pEventFile;
    }

    return hr;
}


static bool IsUserAcceptingProfiling()
{
    bool ret = true;
#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS

    if (!osIsUserAdministrator())
    {
        QMessageBox::StandardButton btnSelected;
        btnSelected = afMessageBox::instance().information(
                          "CPU Profiling privileges notification",
                          "CodeXL does not have administrator privileges. You can proceed with the profile session,"
                          "however data collected for system modules may be incomplete.\n"
                          "Do you want to start profiling anyway?",
                          QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
        ret = QMessageBox::No != btnSelected;
    }

#endif
    return ret;
}

static bool IsUserAcceptingPerfCssWarning()
{
    bool ret = true;
#if AMDT_BUILD_TARGET == AMDT_LINUX_OS
    int majorVersion;
    int minorVersion;
    int buildNumber;

    if (osGetOperatingSystemVersionNumber(majorVersion, minorVersion, buildNumber) &&
        (majorVersion < 3))

    {
        QMessageBox::StandardButton btnSelected;
        btnSelected = afMessageBox::instance().information(
                          "Call Stack Samples Warning",
                          "There are known issues in older kernel modules (before version 3), "
                          "which can cause problems when collecting Call Stack Samples.\n"
                          "Do you want to start profiling with Call Stack Samples anyway?",
                          QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
        ret = QMessageBox::No != btnSelected;
    }

#endif // AMDT_LINUX_OS

    return ret;
}

bool CommandsHandler::IsSpecialExetableCaseSet()
{
    return !afProjectManager::instance().currentProjectSettings().windowsStoreAppUserModelID().isEmpty();
}

bool CommandsHandler::IsProfileEnabled()
{
    return true;
}

void CommandsHandler::updateUI(afExecutionCommandId commandId, QAction* pAction)
{
    if (AF_EXECUTION_ID_BREAK == commandId)
    {
        pAction->setText(PM_STR_MENU_PAUSE_DATA_TOOLBAR);
    }
}

void CommandsHandler::HandleInvalidProjectSettings(bool& isProfileSettingsOK, osProcessId& processId)
{
    // Check if the project is set, if this is a GPU / CPU profile, if the exe exists:
    bool isProjectSet = !afProjectManager::instance().currentProjectSettings().projectName().isEmpty();
    bool isExeExist = !afProjectManager::instance().currentProjectSettings().executablePath().isEmpty();
    ProfileSessionScope scope = SharedProfileSettingPage::Instance()->CurrentSharedProfileSettings().m_profileScope;

    // Open the project settings in the following cases:
    QString infoMessage = PM_STR_StartProfilingNoProjectIsLoaded;

    if (isProjectSet && (scope != PM_PROFILE_SCOPE_SYS_WIDE) && (processId == 0))
    {
        if (acMessageBox::instance().information(AF_STR_InformationA, PM_STR_StartProfilingNoExeNoSystemWide, QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes)
        {
            SharedProfileSettingPage::Instance()->CurrentSharedProfileSettings().m_profileScope = PM_PROFILE_SCOPE_SYS_WIDE;
            isProfileSettingsOK = true;
            processId = PROCESS_ID_UNBOUND;
        }
    }
    // Attach to process:
    else if (isProjectSet && (scope == PM_PROFILE_SCOPE_SINGLE_EXE) && (processId != 0))
    {
        isProfileSettingsOK = true;
    }
    else if (isProjectSet && !isExeExist && (scope == PM_PROFILE_SCOPE_SYS_WIDE))
    {
        isProfileSettingsOK = true;
        processId = PROCESS_ID_UNBOUND;
    }
    else if (!isProjectSet && (processId != 0) && !isExeExist)
    {
        // Attach to process:
        afApplicationCommands::instance()->CreateDefaultProject(PM_STR_PROFILE_MODE);
        isProfileSettingsOK = true;
    }
}
