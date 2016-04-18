//=====================================================================
// Copyright (c) 2012 Advanced Micro Devices, Inc. All rights reserved.
//
/// \author GPU Developer Tools
/// \file $File: //devtools/main/CodeXL/Components/GpuProfiling/AMDTGpuProfiling/CounterManager.cpp $
/// \version $Revision: #61 $
/// \brief  This file contains CounterManager class
//
//=====================================================================
// $Id: //devtools/main/CodeXL/Components/GpuProfiling/AMDTGpuProfiling/CounterManager.cpp#61 $
// Last checkin:   $DateTime: 2016/04/18 06:02:03 $
// Last edited by: $Author: salgrana $
// Change list:    $Change: 569613 $
//=====================================================================

#include <qtIgnoreCompilerWarnings.h>
#include <AMDTBaseTools/Include/gtIgnoreCompilerWarnings.h>
#include <QtCore>
#include <QtWidgets>


#if defined(signals)
    #pragma push_macro("signals")
    #undef signals
    #define NEED_TO_POP_SIGNALS_MACRO
#endif
#include "../HSAFdnCommon/HSAFunctionDefs.h"
#if defined (NEED_TO_POP_SIGNALS_MACRO)
    #pragma pop_macro("signals")
#endif

#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTOSWrappers/Include/osApplication.h>
#include <AMDTOSWrappers/Include/osProcess.h>
#include <AMDTOSWrappers/Include/osStringConstants.h>
#include <AMDTOSAPIWrappers/Include/oaDriver.h>
#include <ADLUtil.h>
#include <DeviceInfoUtils.h>

#include <AMDTGpuProfiling/AMDTGpuProfilerDefs.h>
#include "CounterManager.h"

// AMDTApplicationFramework.
#include <AMDTApplicationFramework/Include/afGlobalVariablesManager.h>
#include <AMDTApplicationFramework/Include/afProjectManager.h>


static const gtString s_DEVICE_ENV_VAR = L"CodeXLGPUProfilerDevice";
static const gtString s_HW_FAMILY_ENV_VAR = L"CodeXLGPUProfilerHardwareFamily";

const QString CounterManager::ms_SI_FAMILY_ENV_VAR_VALUE = "SouthernIslands";
const QString CounterManager::ms_CI_FAMILY_ENV_VAR_VALUE = "SeaIslands";
const QString CounterManager::ms_VI_FAMILY_ENV_VAR_VALUE = "VolcanicIslands";

const int CounterManager::ms_SI_PLACEHOLDER_DEVICE_ID = 0x6798; // a Tahiti device
const int CounterManager::ms_CI_PLACEHOLDER_DEVICE_ID = 0x6650; // a Bonaire device
const int CounterManager::ms_VI_PLACEHOLDER_DEVICE_ID = 0x6900; // an Iceland device

const int CounterManager::ms_UNSPECIFIED_REV_ID = 0;

const int CounterManager::ms_AMD_VENDOR_ID = 0x1002;

void CounterManager::Reset()
{
    // Release all resources, do not release modules.
    FreeResources(false);

    // Reset all members.
    m_counterData.clear();
    m_counterNamesHW.clear();
    m_counterIndices.clear();
    m_counterCaptureOptionHW.clear();
    m_publicCounterCount.clear();
    m_availableDevices.clear();
    m_dummyDeviceAdded.clear();

}

// Adjusts the internal state of the object to the context (remote or local session).
void CounterManager::AdjustCounterManagerState(bool isRemoteSession)
{
    // First reset the current state.
    Reset();

    // Now configure the updated state.
    Init(isRemoteSession);
}

// Releases acquired resources, isReleaseModulesRequired determines whether modules will be released.
void CounterManager::FreeResources(bool isReleaseModulesRequired)
{
    // iterate over m_counterGroupHW and free each CounterGroup* contained in the values, then clear everything
    for (QMap<HardwareFamily, QList<CounterGroup*> >::iterator mapIt = m_counterGroupHW.begin(); mapIt != m_counterGroupHW.end(); ++mapIt)
    {
        for (QList<CounterGroup*>::iterator listIt = mapIt.value().begin(); listIt != mapIt.value().end(); ++listIt)
        {
            SAFE_DELETE(*listIt);
        }

        mapIt.value().clear();
    }

    // iterate over m_counterIndices and free each CounterIndices* contained in the values, then clear everything
    for (QMap<HardwareFamily, CounterNameToIndex>::iterator mapIt = m_counterIndices.begin(); mapIt != m_counterIndices.end(); ++mapIt)
    {
        for (CounterNameToIndex::iterator listIt = mapIt.value().begin(); listIt != mapIt.value().end(); ++listIt)
        {
            SAFE_DELETE(*listIt);
        }

        mapIt.value().clear();
    }

    m_counterGroupHW.clear();

    if (isReleaseModulesRequired && m_gpaCountersModuleHandle != NULL)
    {
        osReleaseModule(m_gpaCountersModuleHandle);
    }
}

// Configure the object's state to the current context (remote or local session).
void CounterManager::Init(bool isRemoteSession)
{
    const char* ALL_AVAILABLE_DEVICES_ENV_VAR_VAL = "SouthernIslands;SeaIslands;VolcanicIslands";
    gtString envVarValue;
    QString counterEnvVar = "";

    m_isDummyDevicesAdded = false;

    // available counters are set per the installed hardware, but can be overridden by
    // setting the CodeXLGPUProfilerDevice environment variable to a semi-colon-
    // separated list of hex device ids
    // See Common/Src/DeviceInfo/DeviceInfo.cpp for recognized deviceIds
    if (isRemoteSession || osGetCurrentProcessEnvVariableValue(s_DEVICE_ENV_VAR, envVarValue))
    {
        counterEnvVar = isRemoteSession ? ALL_AVAILABLE_DEVICES_ENV_VAR_VAL :
                        QString(envVarValue.asASCIICharArray());
        QStringList supportedDevices = counterEnvVar.split(";");

        foreach (QString supportedDevice, supportedDevices)
        {
            bool ok = false;
            int deviceId = supportedDevice.toInt(&ok, 16);

            if (ok)
            {
                AddDeviceId(deviceId, ms_UNSPECIFIED_REV_ID);
            }
        }
    }

    // available counters are set per the installed hardware, but can be overridden by
    // setting the CodeXLGPUProfilerHardwareFamily environment variable to (multiple
    // values can be specified using a semi-colon separated list):
    // "SouthernIslands" --> southernIslandsHardwareFamily
    // "SeaIslands"      --> seaIslandsHardwareFamily
    // "VolcanicIslands" --> volcanicIslandsHardwareFamily
    if (counterEnvVar.isEmpty())
    {
        if (osGetCurrentProcessEnvVariableValue(s_HW_FAMILY_ENV_VAR, envVarValue))
        {
            counterEnvVar = QString(envVarValue.asASCIICharArray());
            QStringList supportedFamilies = counterEnvVar.split(";");

            foreach (QString supportedFamily, supportedFamilies)
            {
                if (supportedFamily == ms_VI_FAMILY_ENV_VAR_VALUE)
                {
                    AddDeviceIdToFamily(VOLCANIC_ISLANDS_FAMILY, ms_VI_PLACEHOLDER_DEVICE_ID, ms_UNSPECIFIED_REV_ID);
                }
                else if (supportedFamily == ms_SI_FAMILY_ENV_VAR_VALUE)
                {
                    AddDeviceIdToFamily(SOUTHERN_ISLANDS_FAMILY, ms_SI_PLACEHOLDER_DEVICE_ID, ms_UNSPECIFIED_REV_ID);
                }
                else if (supportedFamily == ms_CI_FAMILY_ENV_VAR_VALUE)
                {
                    AddDeviceIdToFamily(SEA_ISLANDS_FAMILY, ms_CI_PLACEHOLDER_DEVICE_ID, ms_UNSPECIFIED_REV_ID);
                }
            }
        }
    }

    if (counterEnvVar.isEmpty())
    {
        // get the hardware families for all installed adapters
        AsicInfoList asicInfoList;

        if (AMDTADLUtils::Instance()->GetAsicInfoList(asicInfoList) == ADL_SUCCESS)
        {
            for (AsicInfoList::const_iterator it = asicInfoList.begin(); it != asicInfoList.end(); ++it)
            {
                AddDeviceId(it->deviceID, it->revID);
            }
        }
    }

    gtVector<gtUInt32> hsadevices;
    bool rcHSA = oaGetHSADeviceIds(hsadevices);

    if (rcHSA)
    {
        for (const auto& d : hsadevices)
        {
            AddDeviceId(d, ms_UNSPECIFIED_REV_ID);
        }
    }

    int numHardwareFamilies = m_availableDevices.count();

    if (numHardwareFamilies == 0 || (numHardwareFamilies == 1 && m_availableDevices.contains(NA_HARDWARE_FAMILY)))
    {
        m_isDummyDevicesAdded = true;

        // if hardware device is not recognized, then enable all hardware families, and add one device from each family
        if (!m_availableDevices.contains(SOUTHERN_ISLANDS_FAMILY))
        {
            AddDeviceIdToFamily(SOUTHERN_ISLANDS_FAMILY, ms_SI_PLACEHOLDER_DEVICE_ID, ms_UNSPECIFIED_REV_ID);
            m_dummyDeviceAdded.append(SOUTHERN_ISLANDS_FAMILY);
        }

        if (!m_availableDevices.contains(SEA_ISLANDS_FAMILY))
        {
            AddDeviceIdToFamily(SEA_ISLANDS_FAMILY, ms_CI_PLACEHOLDER_DEVICE_ID, ms_UNSPECIFIED_REV_ID);
            m_dummyDeviceAdded.append(SEA_ISLANDS_FAMILY);
        }

        if (!m_availableDevices.contains(VOLCANIC_ISLANDS_FAMILY))
        {
            AddDeviceIdToFamily(VOLCANIC_ISLANDS_FAMILY, ms_VI_PLACEHOLDER_DEVICE_ID, ms_UNSPECIFIED_REV_ID);
            m_dummyDeviceAdded.append(VOLCANIC_ISLANDS_FAMILY);
        }
    }

    SetupCounterData();

}

/// Prevents a default instance of the CounterManager class from being created outside this class.
CounterManager::CounterManager(): m_gpaCountersModuleHandle(NULL), m_gpa_GetAvailableCountersFuncPtr(NULL)
{
    Init(false);
}

CounterManager::~CounterManager()
{
    FreeResources(true);
}

void CounterManager::LoadCountersModule()
{
    if (m_gpaCountersModuleHandle == NULL)
    {
        gtString dllName = L"GPUPerfAPICounters";

#if (AMDT_BUILD_TARGET == AMDT_LINUX_OS)
        dllName.prepend(L"lib");
#endif
        // Find if we're running inside a 64-bit application
        osFilePath appPath;
        bool is64bitApp = false;

        if (osGetCurrentApplicationPath(appPath))
        {
            osIs64BitModule(appPath, is64bitApp);
        }

        // If we're running inside a 64-bit application, then load the 64-bit version of the DLL
        if (is64bitApp)
        {
#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
            dllName.append(L"-x64");
#endif
        }
        else
        {
#if AMDT_BUILD_TARGET == AMDT_LINUX_OS
            dllName.append(L"32");
#endif
        }

        if (Util::IsInternalBuild())
        {
            dllName.append(L"-Internal");
        }

        // enable the following line to use debug version of GPA
        // dllName.append(L"-d");

        osFilePath modulePath;
        GT_ASSERT(Util::GetInstallDirectory(modulePath));

        modulePath.setFileName(dllName);
        modulePath.setFileExtension(OS_MODULE_EXTENSION);

        bool successfulLoad = osLoadModule(modulePath, m_gpaCountersModuleHandle);

        if (successfulLoad)
        {
            osProcedureAddress procedureAddress;

            if (osGetProcedureAddress(m_gpaCountersModuleHandle, "GPA_GetAvailableCounters", procedureAddress, false))
            {
                // Check to make sure the function is found
                GT_IF_WITH_ASSERT(procedureAddress != NULL)
                {
                    m_gpa_GetAvailableCountersFuncPtr = (GPA_GetAvailableCountersProc)procedureAddress;
                }
            }
        }
    }
}

void CounterManager::AddDeviceId(int deviceId, int revId)
{
    GDT_GfxCardInfo gfxCardInfo;

    if (AMDTDeviceInfoUtils::Instance()->GetDeviceInfo(deviceId, revId, gfxCardInfo))
    {
        switch (gfxCardInfo.m_generation)
        {
            case GDT_HW_GENERATION_SOUTHERNISLAND:
                AddDeviceIdToFamily(SOUTHERN_ISLANDS_FAMILY, deviceId, revId);
                break;

            case GDT_HW_GENERATION_SEAISLAND:
                AddDeviceIdToFamily(SEA_ISLANDS_FAMILY, deviceId, revId);
                break;

            case GDT_HW_GENERATION_VOLCANICISLAND:
                AddDeviceIdToFamily(VOLCANIC_ISLANDS_FAMILY, deviceId, revId);
                break;

            default:
                AddDeviceIdToFamily(NA_HARDWARE_FAMILY, deviceId, revId);
                break;
        }
    }
    else
    {
        AddDeviceIdToFamily(NA_HARDWARE_FAMILY, deviceId, revId);
    }
}

void CounterManager::AddDeviceIdToFamily(HardwareFamily hardwareFamily, int deviceId, int revId)
{
    QList<DeviceAndRevInfo> deviceInfoList;

    if (hardwareFamily == NA_HARDWARE_FAMILY)
    {
        Util::LogWarning(QString("Unrecognized device: id %1, rev id: %2").arg(deviceId).arg(revId));
    }

    if (m_availableDevices.contains(hardwareFamily))
    {
        deviceInfoList = m_availableDevices[hardwareFamily];
    }

    bool found = false;

    for (auto deviceInfoIter = deviceInfoList.begin(); deviceInfoIter != deviceInfoList.end() && !found; ++deviceInfoIter)
    {
        if ((*deviceInfoIter).m_deviceID == deviceId && (*deviceInfoIter).m_revID == revId)
        {
            found = true;
        }
    }

    if (!found)
    {
        DeviceAndRevInfo deviceAndRevInfo = { deviceId, revId };
        deviceInfoList.append(deviceAndRevInfo);
        m_availableDevices.insert(hardwareFamily, deviceInfoList);
    }
}

void CounterManager::GetCounterInfoFromAccessor(HardwareFamily hardwareFamily, GPA_ICounterAccessor* pCounterAccessor)
{
    if (pCounterAccessor != NULL)
    {
        QHash<QString, QString> counterData;
        QStringList counterNames;
        QList<CounterGroup*> counterGroups;
        CounterGroup* previousGroup = NULL;
        gpa_uint32 publicCounterCount = pCounterAccessor->GetNumPublicCounters();
        gpa_uint32 counterCount = pCounterAccessor->GetNumCounters();
        int actualCounterCount = 0;
        CounterNameToIndex counterIndexMap;
        CounterIndices* counterIndices;
        QList<bool> captureOption;
        bool defaultCapture = true;

        for (gpa_uint32 i = 0; i < counterCount; i++)
        {
            QString counterName = pCounterAccessor->GetCounterName(i);

            if (!IsIgnoredCounter(counterName))
            {
                counterIndices = new CounterIndices(actualCounterCount, i);
                counterIndexMap.insert(counterName, counterIndices);
                QString counterDescription = pCounterAccessor->GetCounterDescription(i);
                QStringList counterInfo = counterDescription.split('#');
                GT_ASSERT(counterInfo.count() == 3 && counterInfo[0] == "");
                QString groupName = counterInfo[1].trimmed();
                counterDescription = counterInfo[2].trimmed();
                counterData.insert(counterName, counterDescription);
                counterNames.append(counterName);
                actualCounterCount++;
                captureOption.push_back(defaultCapture);

                if (previousGroup != NULL && previousGroup->GetCounterGroupName() == groupName)
                {
                    previousGroup->AddCounterName(counterName);
                }
                else
                {
                    previousGroup = NULL;

                    for (int j = 0; j < counterGroups.count(); j++)
                    {
                        if (counterGroups[j]->GetCounterGroupName() == groupName)
                        {
                            previousGroup = counterGroups[j];
                            break;
                        }
                    }

                    if (previousGroup == NULL)
                    {
                        previousGroup = new CounterGroup(groupName);
                        counterGroups.append(previousGroup);
                    }

                    previousGroup->AddCounterName(counterName);
                }
            }

            if (i == publicCounterCount - 1)
            {
                m_publicCounterCount.insert(hardwareFamily, actualCounterCount);
            }
        }

        m_counterData.insert(hardwareFamily, counterData);
        m_counterNamesHW.insert(hardwareFamily, counterNames);
        m_counterGroupHW.insert(hardwareFamily, counterGroups);
        m_counterIndices.insert(hardwareFamily, counterIndexMap);
        m_counterCaptureOptionHW.insert(hardwareFamily, captureOption);
    }
}

void CounterManager::UpdateCounterIndexMap(const QStringList& counterNames, const CounterNameToIndex& counterIndexMap)
{
    for (int i = 0; i < counterNames.count(); i++)
    {
        QString counterName = counterNames[i];

        if (counterIndexMap.contains(counterName))
        {
            CounterIndices* indices = counterIndexMap[counterName];
            indices->m_actualIndex = i;
        }
    }
}

int CounterManager::GetHWCounterCount(HardwareFamily hardwareFamily)
{
    int retVal = 0;

    if (m_counterNamesHW.contains(hardwareFamily))
    {
        retVal = m_counterNamesHW[hardwareFamily].count();
    }

    return retVal;
}

bool CounterManager::IsHWCounterName(const QString& strCounterName)
{
    bool retVal = true;

    if ((strCounterName == "ThreadGroup") ||
        (strCounterName == "ExecutionOrder") ||
        (strCounterName == "GlobalWorkSize") ||
        (strCounterName == "WorkGroupSize") ||
        (strCounterName == "Time") ||
        (strCounterName == "LocalMemSize") ||
        (strCounterName == "VGPRs") ||
        (strCounterName == "SGPRs") ||
        (strCounterName == "ScratchRegs") ||
        (strCounterName == "FCStacks") ||
        (strCounterName == "KernelOccupancy") ||
        (strCounterName == "ThreadID") ||
        (strCounterName == "CallIndex")
       )
    {
        retVal = false;
    }

    return retVal;
}

bool CounterManager::IsIgnoredCounter(const QString& strCounterName)
{
    bool retVal = false;

    if ((strCounterName == "GPUBusy") || (strCounterName == "ALUStalledByLDS"))
    {
        retVal = true;
    }

    return retVal;
}

bool CounterManager::GetCounterDesc(HardwareFamily hardwareFamily, const QString& strCounterName, QString& strCounterDesc)
{
    bool retVal = false;
    strCounterDesc.clear();

    if (m_counterData.contains(hardwareFamily))
    {
        CounterData& counterData = m_counterData[hardwareFamily];

        if (counterData.contains(strCounterName))
        {
            strCounterDesc = counterData[strCounterName];
        }
    }

    if (!strCounterDesc.isEmpty())
    {
        // break up into multiple lines at the sentence delimiter.  This is so the popup hints
        // with the counter descriptions will be broken up into multiple lines.
        QString sentenceDelimiter = ". ";

        if (strCounterDesc.contains(sentenceDelimiter))
        {
            strCounterDesc = strCounterDesc.replace(sentenceDelimiter, ".\n");
        }

        retVal = true;;
    }

    return retVal;
}

bool CounterManager::IsCounterTypePercentage(HardwareFamily hardwareFamily, const QString& strCounterName, bool& isPercentage)
{
    bool retVal = false;
    isPercentage = false;

    QString counterStr = strCounterName.trimmed();

    // only check public counters, don't bother checking internal counters (as none currently have "percentage" in the description)
    if ((hardwareFamily == NA_HARDWARE_FAMILY) || IsPublicCounter(hardwareFamily, counterStr))
    {
        QString strDescription;

        if (GetCounterDesc(hardwareFamily, counterStr, strDescription))
        {
            retVal = true;
            isPercentage = strDescription.contains("percentage") == true;
        }
    }

    return retVal;
}

bool CounterManager::GetHWCounterName(HardwareFamily hardwareFamily, int index, QString& strHWCounterName)
{
    bool retVal = false;

    if (m_counterNamesHW.contains(hardwareFamily) && index >= 0 && index < m_counterNamesHW[hardwareFamily].count())
    {
        strHWCounterName = m_counterNamesHW[hardwareFamily][index];
        retVal = true;
    }
    else
    {
        strHWCounterName.clear();
    }

    return retVal;
}

bool CounterManager::GetHWCounterIndexFromName(HardwareFamily hardwareFamily, bool internalIndex, const QString& strCounterName, int& index)
{
    bool retVal = false;
    index = -1;

    if (m_counterIndices.contains(hardwareFamily) && m_counterIndices[hardwareFamily].contains(strCounterName))
    {
        if (internalIndex)
        {
            index = m_counterIndices[hardwareFamily][strCounterName]->m_internalIndex;
        }
        else
        {
            index = m_counterIndices[hardwareFamily][strCounterName]->m_actualIndex;
        }

        retVal = true;
    }

    return retVal;
}

bool CounterManager::SetHWCounterCapture(HardwareFamily hardwareFamily, const QString& strCounterName, bool capture)
{
    bool retVal = false;
    int index = -1;

    if (GetHWCounterIndexFromName(hardwareFamily, false, strCounterName, index) &&
        (index >= 0) && (index < m_counterCaptureOptionHW[hardwareFamily].count()))
    {
        m_counterCaptureOptionHW[hardwareFamily][index] = capture;

        retVal = true;
    }

    return retVal;
}

bool CounterManager::GetHWCounterCapture(HardwareFamily hardwareFamily, int index, bool& captureOut)
{
    bool retVal = false;

    if (m_counterCaptureOptionHW.contains(hardwareFamily) && index >= 0 && index < m_counterCaptureOptionHW[hardwareFamily].count())
    {
        captureOut = m_counterCaptureOptionHW[hardwareFamily][index];
        retVal = true;
    }
    else
    {
        captureOut = false;
    }

    return retVal;
}

int CounterManager::GetHWCounterGroupCount(HardwareFamily hardwareFamily)
{
    int retVal = 0;

    if (m_counterGroupHW.contains(hardwareFamily))
    {
        retVal = m_counterGroupHW[hardwareFamily].count();
    }

    return retVal;
}

void CounterManager::SetupCounterData()
{
    LoadCountersModule();

    if (m_gpa_GetAvailableCountersFuncPtr != NULL)
    {
        GPA_ICounterAccessor* pCounterAccessor;
        GPA_ICounterScheduler* pCounterScheduler;

        if (m_availableDevices.contains(VOLCANIC_ISLANDS_FAMILY) && m_availableDevices[VOLCANIC_ISLANDS_FAMILY].count() > 0)
        {
            if (m_gpa_GetAvailableCountersFuncPtr(GPA_API_OPENCL, ms_AMD_VENDOR_ID, m_availableDevices[VOLCANIC_ISLANDS_FAMILY][0].m_deviceID, m_availableDevices[VOLCANIC_ISLANDS_FAMILY][0].m_revID, &pCounterAccessor, &pCounterScheduler) == GPA_STATUS_OK)
            {
                GetCounterInfoFromAccessor(VOLCANIC_ISLANDS_FAMILY, pCounterAccessor);
            }
            else
            {
                Util::LogError("Unable to get counter data for VOLCANIC_ISLANDS_FAMILY");
            }
        }

        if (m_availableDevices.contains(SEA_ISLANDS_FAMILY) && m_availableDevices[SEA_ISLANDS_FAMILY].count() > 0)
        {
            if (m_gpa_GetAvailableCountersFuncPtr(GPA_API_OPENCL, ms_AMD_VENDOR_ID, m_availableDevices[SEA_ISLANDS_FAMILY][0].m_deviceID, m_availableDevices[SEA_ISLANDS_FAMILY][0].m_revID, &pCounterAccessor, &pCounterScheduler) == GPA_STATUS_OK)
            {
                GetCounterInfoFromAccessor(SEA_ISLANDS_FAMILY, pCounterAccessor);
            }
            else
            {
                Util::LogError("Unable to get counter data for SEA_ISLANDS_FAMILY");
            }
        }

        if (m_availableDevices.contains(SOUTHERN_ISLANDS_FAMILY) && m_availableDevices[SOUTHERN_ISLANDS_FAMILY].count() > 0)
        {
            if (m_gpa_GetAvailableCountersFuncPtr(GPA_API_OPENCL, ms_AMD_VENDOR_ID, m_availableDevices[SOUTHERN_ISLANDS_FAMILY][0].m_deviceID, m_availableDevices[SOUTHERN_ISLANDS_FAMILY][0].m_revID, &pCounterAccessor, &pCounterScheduler) == GPA_STATUS_OK)
            {
                GetCounterInfoFromAccessor(SOUTHERN_ISLANDS_FAMILY, pCounterAccessor);
            }
            else
            {
                Util::LogError("Unable to get counter data for SOUTHERN_ISLANDS_FAMILY");
            }
        }
    }

    CounterData localCounterData;
    localCounterData.insert("ExecutionOrder",
                            "The order of execution for the kernel dispatch operations in the program.");
    localCounterData.insert("ThreadID",
                            "The thread ID of the host thread that made the OpenCL API call that initiated the kernel dispatch operation.");
    localCounterData.insert("CallIndex",
                            "The call index of the OpenCL API call that initiated the kernel dispatch operation.");
    localCounterData.insert("ThreadGroup",
                            "The size of Thread Group.");
    localCounterData.insert("GlobalWorkSize",
                            "The global work-item size of the kernel.");
    localCounterData.insert("WorkGroupSize",
                            "The work-group size of the kernel.");
    localCounterData.insert("Time",
                            "The time spent executing the kernel in milliseconds (does not include the kernel setup time).");

    localCounterData.insert("LocalMemSize",
                            "The amount of local memory (LDS for GPU) in bytes being used by the kernel.");
    localCounterData.insert("VGPRs",
                            "The number of general purpose vector registers used by the kernel (valid only for GPU devices).");

    localCounterData.insert("SGPRs",
                            "The number of general purpose scalar registers used by the kernel (valid only for GCN-based GPU devices).");

    QString temp = "The number of scratch registers used by the kernel (valid only for GPU devices).\n";
    temp += "If non zero, this is typically the main bottleneck.\n";
    temp += "To reduce this number, reduce the number of GPR used by the kernel.";
    localCounterData.insert("ScratchRegs", temp);

    temp = "The size of the flow control stack used by the kernel (valid only for GPU devices).\n";
    temp += "This number may affect the number of wavefronts in-flight.  To reduce the stack size,\n";
    temp += "reduce the amount of flow control nesting in the kernel.";
    localCounterData.insert("FCStacks", temp);

    temp = "The kernel occupancy (only meaningful for GPU devices).\n";
    temp += "This is an estimate of the number of in-flight wavefronts on a compute unit as a percentage\n";
    temp += "of the theoretical maximum number of wavefronts that the compute unit can support.";
    localCounterData.insert("KernelOccupancy", temp);

    m_counterData.insert(NA_HARDWARE_FAMILY, localCounterData);
}

bool CounterManager::IsPublicCounter(HardwareFamily hardwareFamily, const QString& strCounterName)
{
    int counterIndex;
    return GetHWCounterIndexFromName(hardwareFamily, false, strCounterName, counterIndex) &&
           m_publicCounterCount.contains(hardwareFamily) &&
           counterIndex < m_publicCounterCount[hardwareFamily];
}

bool CounterManager::GetHWCounterGroupName(HardwareFamily hardwareFamily, int index, QString& strCounterGroupName)
{
    bool retVal = false;

    if (m_counterGroupHW.contains(hardwareFamily) && index >= 0 && index < m_counterGroupHW[hardwareFamily].count())
    {
        strCounterGroupName = m_counterGroupHW[hardwareFamily][index]->GetCounterGroupName();
        retVal = true;
    }
    else
    {
        strCounterGroupName.clear();
    }

    return retVal;
}

int CounterManager::GetHWCounterCountInGroup(HardwareFamily hardwareFamily, int index)
{
    int retVal = 0;

    if (m_counterGroupHW.contains(hardwareFamily) && index >= 0 && index < m_counterGroupHW[hardwareFamily].count())
    {
        retVal = m_counterGroupHW[hardwareFamily][index]->GetCount();
    }

    return retVal;
}

bool CounterManager::GetHWCounterNameInGroup(HardwareFamily hardwareFamily, int groupIndex, int counterIndex, QString& strCounterName)
{
    bool retVal = false;

    if (m_counterGroupHW.contains(hardwareFamily) && groupIndex >= 0 && groupIndex < m_counterGroupHW[hardwareFamily].count())
    {
        retVal = m_counterGroupHW[hardwareFamily][groupIndex]->GetCounterName(counterIndex, strCounterName);
    }
    else
    {
        strCounterName.clear();
    }

    return retVal;
}

bool CounterManager::GenerateCounterFile(QString& strCounterFile)
{
    bool retVal = false;
    // create a writer and open the file
    QFile file(strCounterFile);

    if (file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        QTextStream out(&file);
        int selectedCount = 0;

        QList<QString> countersWritten;

        QList<HardwareFamily> keyList;
        keyList = m_counterNamesHW.keys();

        for (QList<HardwareFamily>::const_iterator it = keyList.begin(); it != keyList.end(); ++it)
        {
            HardwareFamily hwFamily = *it;

            for (int i = 0; i < m_counterNamesHW[hwFamily].count(); ++i)
            {
                if (m_counterCaptureOptionHW[hwFamily][i])
                {
                    QString strCounterName = m_counterNamesHW[hwFamily][i];

                    if (countersWritten.indexOf(strCounterName) == -1)
                    {
                        countersWritten.append(strCounterName);
                        out << strCounterName << QString("\n");
                        ++selectedCount;
                    }
                }
            }
        }

        // close the stream
        file.close();
        retVal = true;
    }

    return retVal;
}

bool CounterManager::IsHardwareFamilySupported(HardwareFamily hardwareFamily)
{
    return m_availableDevices.contains(hardwareFamily);
}

bool CounterManager::GetHardwareFamilyDisplayName(HardwareFamily hardwareFamily, QString& strDisplayName)
{
    GDT_HW_GENERATION generation = GDT_HW_GENERATION_NONE;
    strDisplayName.clear();

    switch (hardwareFamily)
    {
        case SOUTHERN_ISLANDS_FAMILY:
            generation = GDT_HW_GENERATION_SOUTHERNISLAND;
            break;

        case SEA_ISLANDS_FAMILY:
            generation = GDT_HW_GENERATION_SEAISLAND;
            break;

        case VOLCANIC_ISLANDS_FAMILY:
            generation = GDT_HW_GENERATION_VOLCANICISLAND;
            break;

        default:
            generation = GDT_HW_GENERATION_NONE;
            break;
    }

    std::string strGenerationName;
    bool retVal = AMDTDeviceInfoUtils::Instance()->GetHardwareGenerationDisplayName(generation, strGenerationName);

    if (retVal)
    {
        strDisplayName.append(strGenerationName.c_str());
    }


    return retVal;
}

void CounterManager::GetHardwareFamilyDevicesDisplayList(HardwareFamily hardwareFamily, QString& strDevicesDisplayList)
{
    strDevicesDisplayList.clear();

    if (!m_dummyDeviceAdded.contains(hardwareFamily) && m_availableDevices.contains(hardwareFamily))
    {
        foreach (DeviceAndRevInfo deviceInfo, m_availableDevices[hardwareFamily])
        {
            GDT_GfxCardInfo gfxCardInfo;

            if (AMDTDeviceInfoUtils::Instance()->GetDeviceInfo(deviceInfo.m_deviceID, deviceInfo.m_revID, gfxCardInfo))
            {
                if (!strDevicesDisplayList.isEmpty())
                {
                    strDevicesDisplayList.append(" ;");
                }

                strDevicesDisplayList.append(gfxCardInfo.m_szMarketingName);
            }
        }
    }
}

bool CounterManager::GetHardwareFamilyPassCountDisplayList(HardwareFamily hardwareFamily,
                                                           const QStringList& enabledCounters,
                                                           const bool isGpuTimeCollected,
                                                           QList<int>& numReqPassesForDevs)
{
    bool retVal = false;

    if (!m_dummyDeviceAdded.contains(hardwareFamily) &&
        m_availableDevices.contains(hardwareFamily))
    {
        LoadCountersModule();

        if (m_gpa_GetAvailableCountersFuncPtr != NULL)
        {
            GPA_ICounterAccessor* pCounterAccessor;
            GPA_ICounterScheduler* pCounterScheduler;

            foreach (DeviceAndRevInfo deviceInfo, m_availableDevices[hardwareFamily])
            {
                if (m_gpa_GetAvailableCountersFuncPtr(GPA_API_OPENCL, ms_AMD_VENDOR_ID, deviceInfo.m_deviceID, deviceInfo.m_revID, &pCounterAccessor, &pCounterScheduler) == GPA_STATUS_OK)
                {
                    pCounterScheduler->SetCounterAccessor(pCounterAccessor, ms_AMD_VENDOR_ID, deviceInfo.m_deviceID, deviceInfo.m_revID);

                    pCounterScheduler->DisableAllCounters();
                    int counterIndex;

                    for (const QString& enabledCounter : enabledCounters)
                    {
                        if (GetHWCounterIndexFromName(hardwareFamily, true, enabledCounter, counterIndex))
                        {
                            pCounterScheduler->EnableCounter(counterIndex);
                        }
                    }

                    gpa_uint32 numRequiredPasses;
                    pCounterScheduler->GetNumRequiredPasses(&numRequiredPasses);

                    if (isGpuTimeCollected)
                    {
                        ++numRequiredPasses;
                    }

                    numReqPassesForDevs << numRequiredPasses;
                    retVal = true;
                }
                else
                {
                    Util::LogError(QString("Unable to get number of passes for hardware family %1").arg(hardwareFamily));
                }
            }
        }
    }

    return retVal;
}

bool CounterManager::GetHardwareFamilyPassCountLimitedCountersList(HardwareFamily hardwareFamily, const QStringList& countersList,
                                                                   unsigned int numRequiredPassesForDevs, QStringList& enabledCounters)
{
    bool retVal = false;

    if (!m_dummyDeviceAdded.contains(hardwareFamily) &&
        m_availableDevices.contains(hardwareFamily))
    {
        LoadCountersModule();

        if (m_gpa_GetAvailableCountersFuncPtr != NULL)
        {
            GPA_ICounterAccessor* pCounterAccessor;
            GPA_ICounterScheduler* pCounterScheduler;

            foreach (DeviceAndRevInfo deviceInfo, m_availableDevices[hardwareFamily])
            {
                if (m_gpa_GetAvailableCountersFuncPtr(GPA_API_OPENCL, ms_AMD_VENDOR_ID, deviceInfo.m_deviceID, deviceInfo.m_revID, &pCounterAccessor, &pCounterScheduler) == GPA_STATUS_OK)
                {
                    pCounterScheduler->SetCounterAccessor(pCounterAccessor, ms_AMD_VENDOR_ID, deviceInfo.m_deviceID, deviceInfo.m_revID);

                    pCounterScheduler->DisableAllCounters();
                    int counterIndex;

                    for (const QString& counterName : countersList)
                    {
                        if (GetHWCounterIndexFromName(hardwareFamily, true, counterName, counterIndex))
                        {
                            pCounterScheduler->EnableCounter(counterIndex);

                            gpa_uint32 numRequiredPasses = 0;
                            pCounterScheduler->GetNumRequiredPasses(&numRequiredPasses);

                            if (numRequiredPasses <= numRequiredPassesForDevs)
                            {
                                // Append the counter to the enabled counters list:
                                enabledCounters << counterName;
                            }
                            else
                            {
                                // Disable the counter:
                                pCounterScheduler->DisableCounter(counterIndex);
                            }
                        }
                    }

                    retVal = true;
                }
                else
                {
                    Util::LogError(QString("Unable to get number of passes for hardware family %1").arg(hardwareFamily));
                }
            }
        }
    }

    return retVal;
}

