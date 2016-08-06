//==================================================================================
// Copyright (c) 2014-2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file Collect.cpp
///
//==================================================================================

// Backend:
#include <AMDTCpuPerfEventUtils/inc/EventEngine.h>
#include <AMDTCpuProfilingControl/inc/CpuProfileControl.h>
#include <AMDTCpuPerfEventUtils/inc/DcConfig.h>
#include <AMDTCpuPerfEventUtils/inc/EventsFile.h>
#include <AMDTCpuProfilingTranslation/inc/CpuProfileDataTranslation.h>

// Infra:
#include <AMDTBaseTools/Include/gtMap.h>
#include <AMDTBaseTools/Include/gtString.h>
#include <AMDTOSWrappers/Include/osTime.h>
#include <AMDTOSWrappers/Include/osDirectory.h>
#include <AMDTOSWrappers/Include/osApplication.h>
#include <AMDTOSWrappers/Include/osMachine.h>
#include <AMDTOSWrappers/Include/osGeneralFunctions.h>
#include <AMDTOSWrappers/Include/osFilePath.h>
#include <AMDTOSWrappers/Include/osProductVersion.h>

// Project:
#include <StringConstants.h>
#include <Collect.h>
#include <CommonUtils.h>

#include "AMDTThreadProfileApi.h"
#include <AMDTThreadProfileDataTypes.h>

// Standard Lib
#if AMDT_BUILD_TARGET == AMDT_LINUX_OS
    #include <unistd.h>
#endif

// External function.
extern bool reportError(bool appendDriverError, const wchar_t* pFormatString, ...);
bool g_isFakeSWP = false;


HRESULT CpuProfileCollect::Initialize()
{
    // Setup the Environment
    SetupEnvironment();

    // Validate Profile Type and other options
    ValidateProfile();

    if (IsTP())
    {
        // Set Thread Profiler configuration
        SetTPConfig();
    }
    else
    {
        // Enable Profiling
        EnableProfiling();

        // Configure Profile Types;
        ConfigureProfile();

        SetOutputFilePath();
    }

    return m_error;
}

HRESULT CpuProfileCollect::StartProfiling()
{
    int nbrPids = 0;
    unsigned int* pPidArray = nullptr;
    HRESULT retVal = E_FAIL;

    if (IsTP())
    {
        retVal = StartTPProfiling();
    }
    else
    {
        pPidArray = new unsigned int[m_args.GetPidsList().size() + 1];

        // Add the PID of the launched application
        if (GetLaunchedPid())
        {
            pPidArray[nbrPids++] = GetLaunchedPid();
        }

        // Add the PIDs specified with Attach case "-p" option
        gtVector<int> pidsList = m_args.GetPidsList();

        // Add the PIDs of already existing processes that are specified in AMD_CODEXL_ATTACH_PID env variable
        for (gtVector<int>::const_iterator it = pidsList.begin(), itEnd = pidsList.end(); it != itEnd; ++it)
        {
            pPidArray[nbrPids++] = (*it);
        }

        retVal = StartProfiling(pPidArray, nbrPids);

        if (nullptr != pPidArray)
        {
            delete[] pPidArray;
        }
    }

    return retVal;
}

HRESULT CpuProfileCollect::StartProfiling(unsigned int* pPidArray, int numPids)
{
    if (isStateReady())
    {
        if (isOK() && numPids && IsCSSEnabled())
        {
            // TODO: on Linux kernel version < 3, emit an warning message for CSS
            m_error = fnSetCallStackSampling(pPidArray,
                                             numPids,
                                             m_args.GetUnwindDepth(),
                                             m_args.GetUnwindInterval(),
                                             m_args.GetCSSScope(),
                                             m_args.IsCSSWithFpoSupport());

            if (!isOK())
            {
                reportError(true, L"Failed to enable callstack profiling. (error code 0x%lx)\n", m_error);
            }
        }

        if (isOK() && numPids)
        {
            m_error = fnSetFilterProcesses(pPidArray,
                                           numPids,
                                           g_isFakeSWP,
                                           ! m_args.IsProfileChildren());

            if (!isOK())
            {
                reportError(true, L"Failed to set process filtering. (error code 0x%lx)\n", m_error);
            }
        }

        if (isOK())
        {
            m_profStartTime = GetTimeStr();

            m_error = fnStartProfiling(
                0 <= m_args.GetStartDelay(), //startPaused
                0 == m_args.GetStartDelay(), //pauseIndefinite
                AMDT_CPU_PROFILING_PAUSE_KEY,
                nullptr);

            if (S_OK == m_error)
            {
                m_profileState = CPUPROF_STATE_PROFILING;
            }
            else
            {
                reportError(true, L"The driver failed to start profiling. (error code 0x%lx)\n", m_error);
            }
        }
    }

    return m_error;
}


HRESULT CpuProfileCollect::StopProfiling()
{
    if (IsTP())
    {
        m_error = StopTPProfiling();
    }
    else
    {
        if (isStateProfiling() && isOK())
        {
            m_profEndTime = GetTimeStr();

            //stop profile
            m_error = fnStopProfiling();

            if (SUCCEEDED(m_error))
            {
                // Write TI file
                WriteRunInfo();

                m_profileState = CPUPROF_STATE_READY;
            }
            else
            {
                // If the driver could not write profile data file, due to lack of space(or some other reasons)
                // it throws an abort message to api and the api would return E_ABORT.
                // Just to satisfy QA, if error code is E_ABORT, emit a different error message...
                gtString errMsg(L"The driver failed to stop profiling. (error code 0x%lx)\n");
                bool reportDriverMsg = true;

                if (E_ABORT == m_error)
                {
                    errMsg = L"Unable to write profile data files. (error code 0x%lx)\n";
                    reportDriverMsg = false;
                }

                reportError(reportDriverMsg, errMsg.asCharArray(), m_error);
            }
        }
    }

    return m_error;
}

HRESULT CpuProfileCollect::StartTPProfiling()
{

    m_error = AMDTStartThreadProfile();

    return m_error;
}

HRESULT CpuProfileCollect::StopTPProfiling()
{

    m_error = AMDTStopThreadProfile();

    return m_error;
}

gtString CpuProfileCollect::GetProfileTypeStr()
{
    QString profileName;
    m_profileDcConfig.GetConfigName(profileName);

    gtString str;
    str.resize(profileName.length());

    if (profileName.length())
    {
        str.resize(profileName.toWCharArray(&str[0]));
    }

    return str;
}


//      Private Memeber Functions

void CpuProfileCollect::SetupEnvironment()
{
    // Set up the environmental variable so the event files can be found.
    osEnvironmentVariable eventDataPath;
    eventDataPath._name = L"CPUPerfAPIDataPath";
    osFilePath eventFilePath;

    if (osGetCurrentApplicationDllsPath(eventFilePath) || osGetCurrentApplicationPath(eventFilePath))
    {
        eventFilePath.clearFileName();
        eventFilePath.clearFileExtension();

        eventFilePath.appendSubDirectory(L"Data");
        eventFilePath.appendSubDirectory(L"Events");

        gtString eventFilePathStr = eventFilePath.fileDirectoryAsString();

        eventDataPath._value.appendFormattedString(L"%ls/", eventFilePathStr.asCharArray());
        osSetCurrentProcessEnvVariable(eventDataPath);

        m_error = S_OK;
    }

    return;
}

void CpuProfileCollect::EnableProfiling()
{
    if (isStateInvalid() && isOK())
    {
        // Enable Profiling
        m_error = fnEnableProfiling();

        if ((m_isProfilingEnabled = SUCCEEDED(m_error)))
        {
            m_profileState = CPUPROF_STATE_READY;

            // Clear the configurations
            fnClearConfigurations();
        }
        else
        {
            reportError(true, CP_STR_FailedToLoadDriver);
            m_error = E_HANDLE;
        }
    }
}

void CpuProfileCollect::ValidateProfile()
{
    m_error = E_FAIL;
    osCpuid cpuInfo;

    // Only TBP is supported on non AMD platforms
    if ((!cpuInfo.isCpuAmd()) && (! IsTBP()) && (0 == m_args.GetTbpSamplingInterval()))
    {
        reportError(false, L"Only Time-based profiling (TBP) is supported on non AMD platforms\n");
        return;
    }

    // If APIC is not available, sampling based profiling cannot be supported
    if (! cpuInfo.hasLocalApic())
    {
        reportError(false, L"Local APIC support is not available, hence profiling cannot be done!\n");
        return;
    }

    // Check for valid working dir
    if (! m_args.GetWorkingDir().isEmpty())
    {
        osFilePath workDir(m_args.GetWorkingDir());

        if (! workDir.exists())
        {
            reportError(false, L"Working directory (" STR_FORMAT L") does not exist.\n", m_args.GetWorkingDir().asCharArray());
            return;
        }
    }

    // Validate the core affinity mask
    // TODO: Currently this supports only upto 64 cores
    if (static_cast<gtUInt64>(-1) != m_args.GetCoreAffinityMask())
    {
        int nbrCores = 0;
        gtUInt64 maxAffinity = 0;

        osGetAmountOfLocalMachineCPUs(nbrCores);

        if (nbrCores < 64)
        {
            maxAffinity = (1ULL << nbrCores) - 1;
        }
        else
        {
            maxAffinity = GT_UINT64_MAX;
        }

        if (maxAffinity < m_args.GetCoreAffinityMask())
        {
            reportError(false, L"Invalid core affinity mask (0x%lx) specified with option(-c).\n", m_args.GetCoreAffinityMask());
            return;
        }
    }

    // In SWP with no launch application specified, profile duration is a must
    if (m_args.IsSystemWide()
        && m_args.GetLaunchApp().isEmpty()
        && (0 == m_args.GetProfileDuration()))
    {
        reportError(false, L"Specifiy either Profile Duration(-d) or Launch Application in System-Wide Profile mode\n");
        return;
    }

    // No profile config is specified - "-m" , "-C" , "-T"
    if (m_args.GetProfileConfig().isEmpty()
        && (0 == m_args.GetTbpSamplingInterval())
        && m_args.GetCustomFile().isEmpty())
    {
        reportError(false, L"No Profile Config is specified. Use any of the following options -m or -C or -T !\n");
        return;
    }

    // -T and -m together is invalid
    if ((m_args.GetTbpSamplingInterval() > 0)
        && (! m_args.GetProfileConfig().isEmpty()))
    {
        reportError(false, L"Options -T and -m cannot be used together. Either use -T or -m to specify profile configuration.\n");
        return;
    }

    // -T and -C together is invalid
    if ((m_args.GetTbpSamplingInterval() > 0)
        && (! m_args.GetCustomFile().isEmpty()))
    {
        reportError(false, L"Options -T and -m cannot be used together. Either use -T or -C to specify profile configuration.\n");
        return;
    }

    // -m and -C together is invalid
    if ((! m_args.GetProfileConfig().isEmpty())
        && (! m_args.GetCustomFile().isEmpty()))
    {
        reportError(false, L"Options -m and -C cannot be used together. Either use -m or -C to specify profile configuration.\n");
        return;
    }

    // For the given pids list, check whether we have access to attach..
    // if the process is launched by other users, don't allow the profiling
    gtVector<int> pidsList = m_args.GetPidsList();

    for (gtVector<int>::const_iterator it = pidsList.begin(), itEnd = pidsList.end(); it != itEnd; ++it)
    {
        int tgtPid = (*it);

        if (! osIsProcessAttachable(tgtPid))
        {
            reportError(false, L"Could not profile the given process(PID: %d).\nEither the process is not alive or you do not have permission to attach to this process.\n", tgtPid);
            return;
        }
    }

    // with multiple PIDs attach case, profile duration is must
    if ((m_args.GetPidsList().size() > 0) && (m_args.GetProfileDuration() <= 0))
    {
        reportError(false, L"While attaching multiple process ids, profile duration(-d) needs to be specified.\n");
        return;
    }

    // Terminate app -b ?
    if (m_args.IsTerminateApp())
    {
        // terminate (-b option) app only for launched app case
        if (m_args.GetLaunchApp().isEmpty())
        {
            reportError(false, L"Option -b can be used only if Launch application is specified.\n");
            return;
        }

        // -b without -d is invalid
        if (0 == m_args.GetProfileDuration())
        {
            reportError(false, L"Option to terminate launch application (-b) is invalid without Profile duration option (-d).\n");
            return;
        }
    }

    // validate the output file path
    if (! m_args.GetOutputFile().isEmpty())
    {
        osFilePath outputFile(m_args.GetOutputFile());
        osDirectory osDir;
        outputFile.getFileDirectory(osDir);

        if (! osDir.exists())
        {
            reportError(false, L"Output Directory (" STR_FORMAT L") does not exist.\n",  outputFile.asString(true).asCharArray());
            return;
        }

        if (! osDir.isWriteAccessible())
        {
            reportError(false, L"Output Directory (" STR_FORMAT L") does not have write permission.\n", outputFile.asString(true).asCharArray());
            return;
        }

        gtString fileName;
        outputFile.getFileName(fileName);

        if (fileName.isEmpty())
        {
            reportError(false, L"Output file name (" STR_FORMAT L") is missing with option (-o).\n",
                        outputFile.asString(true).asCharArray());
            return;
        }

        // If special characters like \ / : * ? " < > | are mentioned, report an error
        if ((fileName.find(L"\\") != -1) || (fileName.find(L"/") != -1) || (fileName.find(L":") != -1)
            || (fileName.find(L"*") != -1) || (fileName.find(L"?") != -1) || (fileName.find(L"\"") != -1)
            || (fileName.find(L"<") != -1) || (fileName.find(L">") != -1) || (fileName.find(L"|") != -1))
        {
            reportError(false, L"Output file name (" STR_FORMAT L") contains invalid characters.\n", fileName.asCharArray());
            return;
        }
    }

    // start-delay duration cannot be less than profile duration
    if ((0 != m_args.GetProfileDuration())
        && (m_args.GetProfileDuration() < m_args.GetStartDelay()))
    {
        reportError(false, L"Profile Duration(%d) cannot be less than Start Delay period(%d).\n",
                    m_args.GetProfileDuration(), m_args.GetStartDelay());
        return;
    }

    // If -T is specified, treat this as profile config "tbp"
    if (m_args.GetTbpSamplingInterval() > 0)
    {
        m_args.SetProfileConfig(L"tbp");
    }

    // Check If -g and -G are used, emit an error
    if (m_args.IsCSSWithDefaultValues() && m_args.IsCSSEnabled())
    {
        reportError(false, L"Either use option (-G) or (-g) to enable Callstack sampling.\n");
        return;
    }

#if AMDT_BUILD_TARGET == AMDT_LINUX_OS

    // On Linux, CLU is not supported
    if (IsCLU())
    {
        reportError(false, L"Profile type " STR_FORMAT L" is not supported on Linux.\n",
                    m_args.GetProfileConfig().toUpperCase().asCharArray());
        return;
    }

#endif // Linux

    if (IsCSSEnabled())
    {
        // if CLU and CSS, return error
        if (IsCLU())
        {
            reportError(false, L"Callstack sampling cannot be used with profile type " STR_FORMAT L".\n",
                        m_args.GetProfileConfig().toUpperCase().asCharArray());
            return;
        }

        // If java and CSS, return errror
        if (m_args.GetLaunchApp().endsWith(L"java.exe") || m_args.GetLaunchApp().endsWith(L"java"))
        {
            reportError(false, L"Callstack sampling cannot be used with Java applications.\n");
            return;
        }

        // If CLU and CSS, return error
        if (IsClrApp())
        {
            reportError(false, L"Callstack sampling cannot be used with CLR applications.\n");
            return;
        }

        // If SWP and no launch app and no attach, disable CSS and emit an warning message.
        if (m_args.IsSystemWide()
            && m_args.GetLaunchApp().isEmpty()
            && (! m_args.IsAttach()))
        {
            reportError(false, L"Callstack sampling is disabled in System Wide Profile without launch application.\n");
            m_args.DisableCSS();
        }
    }

    // For predefined profile configs, construct the profile XML file path
    osFilePath profileFilePath;

    if (! m_args.GetProfileConfig().isEmpty())
    {
        // Construct the profile path
        if (osGetCurrentApplicationDllsPath(profileFilePath) || osGetCurrentApplicationPath(profileFilePath))
        {
            profileFilePath.clearFileName();
            profileFilePath.clearFileExtension();

            profileFilePath.appendSubDirectory(L"Data");
            profileFilePath.appendSubDirectory(L"Profiles");

            // For TBP, no need to add family sub directory
            if (!IsTBP())
            {
                gtString familySubDir;
                cpuInfo.getFamily() >= FAMILY_OR
                ? familySubDir.appendFormattedString(L"0x%x_0x%x", cpuInfo.getFamily(), (cpuInfo.getModel() >> 4))
                : familySubDir.appendFormattedString(L"0x%x", cpuInfo.getFamily());

                profileFilePath.appendSubDirectory(familySubDir);
            }

            profileFilePath.setFileName(m_args.GetProfileConfig().toLowerCase());
            profileFilePath.setFileExtension(L"xml");
        }
    }
    else
    {
        // Custom Profile XML file specified by the customer
        profileFilePath = m_args.GetCustomFile();

        if (! profileFilePath.exists())
        {
            reportError(false, L"Given Custom profile XML file does not exist !\n");
            return;
        }
    }

    if (profileFilePath.exists())
    {
        m_profileDcConfig.ReadConfigFile(profileFilePath.asString().asCharArray());

        // if it is special TBP, set the sampling interval given by the user
        if (m_args.GetTbpSamplingInterval())
        {
            m_profileDcConfig.SetTimerInterval(static_cast<float>(m_args.GetTbpSamplingInterval()));
        }
    }
    else
    {
        if (!IsTP())
        {
            reportError(false, L"Invalid Profile Config specified!\n");
            return;
        }
    }

    // Check for valid launch app
    if (!m_args.GetLaunchApp().isEmpty())
    {
        osFilePath exePath(m_args.GetLaunchApp());

        if (!exePath.exists())
        {
            reportError(false, L"Launch application (" STR_FORMAT L") does not exist.\n", m_args.GetLaunchApp().asCharArray());
            return;
        }

        if (!exePath.isExecutable())
        {
            reportError(false, L"Launch application (" STR_FORMAT L") is not an executable binary.\n", m_args.GetLaunchApp().asCharArray());
            return;
        }
    }

    // If there is no launch App, either -a or -p should be specified
    if (m_args.GetLaunchApp().isEmpty()
        && (!m_args.IsSystemWide())
        && (0 == m_args.GetPidsList().size()))
    {
        reportError(false, L"Specify either Launch application or System Wide Profile(-a) or attach processes(-p).\n");
        return;
    }

    m_error = S_OK;
    return;
}

void CpuProfileCollect::VerifyAndSetEvents(EventConfiguration** ppDriverEvents)
{
    int nbrOfEvents = m_profileDcConfig.GetNumberOfEvents();

    if (! nbrOfEvents)
    {
        return;
    }

    osCpuid cpuInfo;
    int model = cpuInfo.getModel();
    osFilePath eventFilePath;

    EventEngine eventEngine;

    // Construct the path for family specific Events XML files
    if (osGetCurrentApplicationDllsPath(eventFilePath) || osGetCurrentApplicationPath(eventFilePath))
    {
        eventFilePath.clearFileName();
        eventFilePath.clearFileExtension();

        eventFilePath.appendSubDirectory(L"Data");
        eventFilePath.appendSubDirectory(L"Events");

        gtString eventFilePathStr = eventFilePath.fileDirectoryAsString();

        if (!eventEngine.Initialize(QString::fromWCharArray(eventFilePathStr.asCharArray(), eventFilePathStr.length())))
        {
            reportError(false, L"Unable to find the event files directory: %ls", eventFilePathStr.asCharArray());
            m_error = E_NOFILE;
        }
    }

    EventsFile* pEventFile = nullptr;

    if (isOK())
    {
        pEventFile = eventEngine.GetEventFile(cpuInfo.getFamily(), model);

        if (nullptr == pEventFile)
        {
            reportError(false, L"Unable to find the event file: %ls",
                        eventEngine.GetEventFilePath(cpuInfo.getFamily(), model).toStdWString().c_str());
            m_error = E_NOFILE;
        }
    }

    // Get the Events info from the DcConfig
    DcEventConfig* pEvents = new DcEventConfig[nbrOfEvents];
    m_profileDcConfig.GetEventInfo(pEvents, nbrOfEvents);

    *ppDriverEvents = nullptr;

    if (isOK())
    {
        *ppDriverEvents = new EventConfiguration[nbrOfEvents];
    }

    unsigned int evCounter = 0;
    unsigned int maxCounter = 0;
    fnGetEventCounters(&maxCounter);

    DcEventConfig* pCurrentConfig = pEvents;
    gtMap<gtUInt64, gtUInt64> eventsMap;

    for (int i = 0; (i < nbrOfEvents) && (S_OK == m_error); i++)
    {
        //validate the event
        unsigned int evSelect = GetEvent12BitSelect(pCurrentConfig->pmc);

        // Duplicate events in custom xml file leads to issues during collection/translation.
        // If there are any duplicate events, report an error
        if (eventsMap.find(pCurrentConfig->pmc.perf_ctl) == eventsMap.end())
        {
            eventsMap.insert(gtMap<gtUInt64, gtUInt64>::value_type(pCurrentConfig->pmc.perf_ctl, pCurrentConfig->eventCount));
        }
        else
        {
            reportError(false, L"Duplicate event (%#x) in the profile configuration.\n", evSelect);
            m_error = E_FAIL;
            break;
        }

        bool retVal = pEventFile->ValidateEvent(evSelect, pCurrentConfig->pmc.ucUnitMask);

        // Don't allow events which are not valid on this cpu
        // TBD: should we check for - (eventData.m_minValidModel > model)
        if (!retVal)
        {
            reportError(false, L"The configuration erronously contains an event (%#x) which is not\n"
                        L"available on this system.  Please choose a different\n"
                        L"configuration.", evSelect);
            m_error = E_NOTAVAILABLE;
            break;
        }

#if AMDT_BUILD_TARGET == AMDT_LINUX_OS
        // On Linux, PERF does not support L2I events based sampling
        CpuEvent eventData;
        pEventFile->FindEventByValue(evSelect, eventData);

        if (FAMILY_KB == cpuInfo.getFamily())
        {
            if (eventData.source.contains("L2I", Qt::CaseInsensitive))
            {
                reportError(false, L"The Profile configuration contains L2I event.\n"
                            L"L2I PMC events based sampling is not supported.\n"
                            L"Please choose a different configuration");
                m_error = E_NOTAVAILABLE;
                break;
            }
        }

        // TBD: what do we do for Windows ?
        // On Linux, PERF does not support NB events based sampling.
        // if (FAMILY_OR == cpuInfo.getFamily())
        {
            if (eventData.source.startsWith("NB", Qt::CaseInsensitive))
            {
                reportError(false, L"The Profile configuration contains a northbridge(NB) event:\n"
                            L"[%#x] %ls\n which is not a valid sampling event. "
                            L"Please fix the configuration",
                            eventData.value, eventData.name.data());
                m_error = E_NOTAVAILABLE;
                break;
            }
        }
#endif // AMDT_LINUX_OS

        // Save the event configuration that will be written to the hardware
        (*ppDriverEvents)[i].performanceEvent = pCurrentConfig->pmc.perf_ctl;
        (*ppDriverEvents)[i].value = pCurrentConfig->eventCount;
        (*ppDriverEvents)[i].eventCounter = evCounter;

        if (++evCounter == maxCounter)
        {
            evCounter = 0;
        }

        pCurrentConfig++;
    }

    eventsMap.clear();

    if (S_OK != m_error && nullptr != *ppDriverEvents)
    {
        delete [] *ppDriverEvents;
        *ppDriverEvents = nullptr;
    }

    if (nullptr != pEventFile)
    {
        delete pEventFile;
    }

    if (nullptr != pEvents)
    {
        delete [] pEvents;
    }
}

void CpuProfileCollect::ConfigureProfile()
{
    if (isStateReady() && isOK())
    {
        g_isFakeSWP = m_args.IsSystemWide();

        // Set TBP Configuration
        SetTbpConfig();

        // Set EBP Configuration
        SetEbpConfig();

        // Set IBS Configuration
        SetIbsConfig();
    }
}

void CpuProfileCollect::SetTbpConfig()
{
    if (isStateReady() && isOK())
    {
        float msInterval = GetTbpSamplingInterval();

        if (msInterval >= 1.0)
        {
            gtUInt64 cpuCoreMask = m_args.GetCoreAffinityMask();

            //set up timer options
            if (0.0f < msInterval)
            {
                // Convert the resolution to micro seconds
                unsigned int resolution = static_cast<unsigned int>(msInterval * 1000.0f);

                m_error = fnSetTimerConfiguration(cpuCoreMask, &resolution);
            }

            if (!SUCCEEDED(m_error))
            {
                reportError(true, L"There was a problem configuring the timer profile. (error code 0x%lx)\n\n", m_error);
            }
        }
    }
}


void CpuProfileCollect::SetEbpConfig()
{
    if (isStateReady() && isOK() && (m_profileDcConfig.GetNumberOfEvents() > 0))
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
                reportError(false, L"Make sure \'Virtualize CPU performance Counters\' is enabled in "
                            L"virtual machine settings before running any event based profiling.\n"
                            L"OS mode events will not be counted within guest OS.");
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
                        gtString errMsg;
                        errMsg.appendFormattedString(L"%ls profiling is not supported on Windows 8, 8.1 or older with Hyper-V enabled.\n",
                                                     m_args.GetProfileConfig().toUpperCase().asCharArray());

                        reportError(false, errMsg.asCharArray());
                        m_error = E_NOTIMPL;
                    }
                }

#endif
#if AMDT_BUILD_TARGET == AMDT_LINUX_OS
                // EBP is not supported on Hyper-V Linux child partition
                wchar_t vendorName[32] = { 0 };
                cpuInfo.getHypervisorVendorString(vendorName, 32);

                gtString errMsg;
                errMsg.appendFormattedString(L"Hypervisor Detected: %ls\n", vendorName);
                errMsg.appendFormattedString(L"%ls profiling is not supported on Guest OS.\n",
                                             m_args.GetProfileConfig().toUpperCase().asCharArray());

                reportError(false, errMsg.asCharArray());
                m_error = E_NOTIMPL;
#endif
            }
            else
            {
                wchar_t vendorName[32] = { 0 };
                cpuInfo.getHypervisorVendorString(vendorName, 32);

                gtString errMsg;
                errMsg.appendFormattedString(L"Hypervisor Detected: %ls\n", vendorName);
                errMsg.appendFormattedString(L"%ls profiling is not supported on Guest OS.\n",
                                             m_args.GetProfileConfig().toUpperCase().asCharArray());

                reportError(false, errMsg.asCharArray());
                m_error = E_NOTIMPL;
            }
        }

#if AMDT_BUILD_TARGET == AMDT_LINUX_OS
        else
        {
            // Xen Dom0 detection process is little different
            if (access("/proc/xen/capabilities", F_OK) == 0)
            {
                gtString errMsg;
                errMsg.appendFormattedString(L"Hypervisor Detected : Xen (Dom0)\n");
                errMsg.appendFormattedString(L"%ls profiling is not supported on Xen Host OS.\n",
                                             m_args.GetProfileConfig().toUpperCase().asCharArray());

                reportError(false, errMsg.asCharArray());
                m_error = E_NOTIMPL;
            }
        }

#endif

        EventConfiguration* pDriverEvents = nullptr;

        if (isOK())
        {
            VerifyAndSetEvents(&pDriverEvents);
        }

        if (isOK())
        {
            // Profile only on selected cores specified in the CPU Affinity Mask
            // TODO: support for more than 64 cores ?
            gtUInt64 cpuCoreMask = m_args.GetCoreAffinityMask();
            m_error = fnSetEventConfiguration(pDriverEvents,
                                              m_profileDcConfig.GetNumberOfEvents(),
                                              cpuCoreMask);

            if (!SUCCEEDED(m_error))
            {
                reportError(true, L"There was a problem configuring the event profile. (error code 0x%lx)\n\n", m_error);
            }
        }

        // Free the event array allocated in VerifyAndSetEvents()
        if (nullptr != pDriverEvents)
        {
            delete [] pDriverEvents;
        }
    }
}

void CpuProfileCollect::SetIbsConfig()
{
    if (isStateReady() && isOK())
    {
        IbsConfig ibsConfig;
        m_profileDcConfig.GetIBSInfo(&ibsConfig);

        CluConfig cluConfig;
        m_profileDcConfig.GetCLUInfo(&cluConfig);

        if (ibsConfig.fetchSampling || ibsConfig.opSampling || cluConfig.cluSampling)
        {
            osCpuid cpuInfo;

            // IBS profiling is not supported on hypervisors
            if (cpuInfo.hasHypervisor())
            {
                gtString errMsg;
                wchar_t vendorName[32] = { 0 };
                cpuInfo.getHypervisorVendorString(vendorName, 32);

                errMsg.appendFormattedString(L"Hypervisor Detected: %ls\n", vendorName);
                errMsg.appendFormattedString(L"%ls profiling is not supported on Guest OS.\n",
                                             m_args.GetProfileConfig().toUpperCase().asCharArray());

                reportError(false, errMsg.asCharArray());
                m_error = E_NOTIMPL;
            }

#if AMDT_BUILD_TARGET == AMDT_LINUX_OS
            else
            {
                // Xen Dom0 detection process is little different
                if (access("/proc/xen/capabilities", F_OK) == 0)
                {
                    gtString errMsg;
                    errMsg.appendFormattedString(L"Hypervisor Detected : Xen (Dom0)\n");
                    errMsg.appendFormattedString(L"%ls profiling is not supported on Xen Host OS.\n",
                                                 m_args.GetProfileConfig().toUpperCase().asCharArray());

                    reportError(false, errMsg.asCharArray());
                    m_error = E_NOTIMPL;
                }
            }

#endif

            if (isOK())
            {
                bool isIbsAvailable = false;
                m_error = fnGetIbsAvailable(&isIbsAvailable);

                if (!isIbsAvailable)
                {
                    gtString errMsg;

#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
                    errMsg = L"IBS profiling is not supported.\n";
#elif AMDT_BUILD_TARGET == AMDT_LINUX_OS
                    errMsg = L"IBS profiling is supported only on Linux Kernel 3.5 and above with System-Wide Profile mode.\n";
#endif
                    reportError(false, errMsg.asCharArray());
                    m_error = E_NOTIMPL;
                }
            }

#if AMDT_BUILD_TARGET == AMDT_LINUX_OS
            g_isFakeSWP = true;
#if 0

            // For linux kernel less than 3.12, IBS is supported only in System-Wide Mode.
            if (isOK() && (! m_args.IsSystemWide()))
            {
                int majorVersion, minorVersion, buildNumber;

                if (osGetOperatingSystemVersionNumber(majorVersion, minorVersion, buildNumber)
                    && (3 > majorVersion || (3 == majorVersion && 17 > minorVersion)))
                {
                    reportError(false, L"IBS is supported only with System-Wide Profile. Please use option (-a) to enable System-Wide Profile.\n");

                    m_error = E_NOTIMPL;
                }
            }

#endif //0
#endif // AMDT_LINUX_OS
        }

        if (isOK())
        {
            bool opSample = false;
            bool opCycleCount = false;
            unsigned long opInterval = 0;

            if (ibsConfig.opSampling || cluConfig.cluSampling)
            {
                opSample = true;

                // Using IBS Op settings when profiling both IBS Op and CLU
                if (ibsConfig.opSampling)
                {
                    opCycleCount = ibsConfig.opCycleCount;
                    opInterval = ibsConfig.opMaxCount;
                }
                else
                {
                    opCycleCount = cluConfig.cluCycleCount;
                    opInterval = cluConfig.cluMaxCount;
                }

                // Profile only on selected cores specified in the CPU Affinity Mask
                // TODO: support for more than 64 cores ?
                gtUInt64 cpuCoreMask = m_args.GetCoreAffinityMask();
                m_error = fnSetIbsConfiguration(cpuCoreMask,
                                                ibsConfig.fetchSampling ? ibsConfig.fetchMaxCount : 0,
                                                opSample ? opInterval : 0,
                                                true,
                                                (!opCycleCount));

                if (!isOK())
                {
                    reportError(true, L"There was a problem configuring the IBS profile. (error code 0x%lx)\n\n", m_error);
                }
            }
        }
    }
}

void CpuProfileCollect::SetTPConfig()
{

    if (isOK())
    {
        bool isTPAvbl = false;

        AMDTResult ret = AMDTIsThreadProfileAvailable(&isTPAvbl);

        if (AMDT_STATUS_OK == ret && isTPAvbl)
        {
            AMDTUInt32 events = AMDT_TP_EVENT_TRACE_CSWITCH;

            events |= IsCSSEnabled() ? AMDT_TP_EVENT_TRACE_CALLSTACK : 0;

            const char* pLogFilePath;
            osFilePath datafilePath;
            GetOutputFilePath(datafilePath);

            if (isOK())
            {
                datafilePath.setFileExtension(THREAD_PROFILE_RAWFILE_EXTENSION);
                pLogFilePath = datafilePath.asString().asASCIICharArray();
            }
            else
            {
                pLogFilePath = strdup("c:\\temp\\test-tp1.etl");
            }

            ret = AMDTSetThreadProfileConfiguration(events, pLogFilePath);

            m_error = ret;
        }

        if (!SUCCEEDED(m_error))
        {
            reportError(true, L"There was a problem configuring the thread profile. (error code 0x%lx)\n\n", m_error);
        }
    }

    return;
}

// Actually this one returns the absolute path with only the base file name.
// with this base name
//  .PRD/.caperf will be added for raw data file
//  .TI will be appended for taskinfo file
//  .RI will be appended for runinfo file
//
void CpuProfileCollect::GetOutputFilePath(osFilePath& outfilePath)
{
    m_error = S_OK;

    if (m_outputFilePath.isEmpty())
    {
        // The default base name is "codexl_cpuprofile"
        gtString outputFile = m_args.GetOutputFile();

        if (outputFile.isEmpty())
        {

#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
            wchar_t  tmpPath[OS_MAX_PATH] = { L'\0' };
            GetTempPathW(OS_MAX_PATH, tmpPath);
            outputFile = gtString(tmpPath);
#else
            // TODO: Use P_tmpdir on Linux
            wchar_t  tmpPath[OS_MAX_PATH] = L"/tmp/";
            outputFile = gtString(tmpPath);
#endif // AMDT_BUILD_TARGET

            // Add the timestamp to the default-output-file
            outputFile.appendFormattedString(STR_FORMAT L"-" STR_FORMAT,
                                             CODEXL_DEFAULT_OUTPUTFILE_NAME,
                                             GetTimeStr().asCharArray());
        }

        m_outputFilePath = osFilePath(outputFile);

        // check if the base dir exists
        osDirectory osDir;
        m_outputFilePath.getFileDirectory(osDir);

        if (! osDir.exists())
        {
            reportError(false, L"Output Directory (" STR_FORMAT L") does not exist\n", outfilePath.fileDirectoryAsString().asCharArray());
            m_error = E_INVALIDPATH;
        }
    }

    if (isOK())
    {
        outfilePath = m_outputFilePath;
    }

    return;
}

void CpuProfileCollect::SetOutputFilePath()
{
    if (isStateReady() && isOK())
    {
        osFilePath datafilePath;
        GetOutputFilePath(datafilePath);

        if (isOK())
        {
            datafilePath.setFileExtension(CPUPROFILE_RAWFILE_EXTENSION);

            m_error = fnSetProfileOutputFile(datafilePath.asString().asCharArray());

            if (! SUCCEEDED(m_error))
            {
                reportError(true, L"There was a problem configuring the profile output file. (error code 0x%lx)\n\n", m_error);
            }
        }
    }
}


void CpuProfileCollect::WriteRunInfo()
{
    if (isStateProfiling() && isOK())
    {
        // construct the RI file path
        osFilePath rifilePath;
        GetOutputFilePath(rifilePath);

        if (isOK())
        {
            rifilePath.setFileExtension(L"ri");

            // populate RI data
            RunInfo rInfo;

            osFilePath wrkDir(m_args.GetLaunchApp());

            rInfo.m_targetPath       = m_args.GetLaunchApp(); // This will not be set for SWP and Attach PIDs
            rInfo.m_wrkDirectory     = m_args.GetWorkingDir().isEmpty() ? wrkDir.fileDirectoryAsString() : m_args.GetWorkingDir();
            rInfo.m_cmdArguments     = m_args.GetLaunchAppArgs();
            // rInfo.m_envVariables;
            rInfo.m_profType         = GetProfileTypeStr();
            rInfo.m_profDirectory    = rifilePath.fileDirectoryAsString();
            rInfo.m_profStartTime    = GetProfileStartTime();
            rInfo.m_profEndTime      = GetProfileEndTime();
            rInfo.m_isCSSEnabled     = IsCSSEnabled();
            rInfo.m_cssUnwindDepth   = m_args.GetUnwindDepth();
            rInfo.m_cssScope         = m_args.GetCSSScope();
            rInfo.m_isCssSupportFpo  = m_args.IsCSSWithFpoSupport();
            rInfo.m_cssInterval      = m_args.GetUnwindInterval();
            rInfo.m_isProfilingClu   = (DCConfigCLU == m_profileDcConfig.GetConfigType()) ? true : false;

            // detect and set profile scope
            if (m_args.IsSystemWide())
            {
                if (m_args.GetLaunchApp().isEmpty())
                {
                    rInfo.m_profScope = gtString(L"System-Wide");
                }
                else
                {
                    rInfo.m_profScope = gtString(L"System-Wide with focus on application");
                }
            }
            else
            {
                rInfo.m_profScope = gtString(L"Single Application");
            }

            // Is IBS OP sampling..
            rInfo.m_isProfilingIbsOp = IsIbsOPSampling();

            // Get number of cores
            int nbrCores = 0;
            if (osGetAmountOfLocalMachineCPUs(nbrCores))
            {
                rInfo.m_cpuCount = static_cast<unsigned int>(nbrCores);
            }

            // If affinity is not passed as argument, then the default value is 0xFF...FF
            gtUInt64 coreAffinityMask = m_args.GetCoreAffinityMask();

            // If default value is 0xFF..FF, then compute the affinity using number of cores
            if (coreAffinityMask + 1 == 0)
            {
                if (nbrCores < 64)
                {
                    coreAffinityMask = (1ULL << nbrCores) - 1;
                }
            }

            rInfo.m_cpuAffinity = coreAffinityMask;

            // set OS name
            osGetOSShortDescriptionString(rInfo.m_osName);

            // Get CodeXL version
            osProductVersion cxlVersion;
            osGetApplicationVersion(cxlVersion);
            rInfo.m_codexlVersion = cxlVersion.toString();

            // write the RI file
            m_error = fnWriteRunInfo(rifilePath.asString().asCharArray(), &rInfo);

            if (!SUCCEEDED(m_error))
            {
                reportError(false, L"Failed to write the Runinfo(RI) file");
            }
        }
    }
}
