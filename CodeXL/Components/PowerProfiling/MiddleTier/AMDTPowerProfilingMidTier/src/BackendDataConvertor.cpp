//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file BackendDataConvertor.cpp
///
//==================================================================================

// C++.
#include <fstream>
#include <algorithm>

// Local.
#include <AMDTPowerProfilingMidTier/include/BackendDataConvertor.h>
#include <AMDTPowerProfilingMidTier/include/PowerProfilerDefs.h>

// Infra.
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTBaseTools/Include/gtString.h>

// Backend data types.
#include <AMDTPowerProfileAPI/inc/AMDTPowerProfileApi.h>

// Device Type Strings
static const gtVector<gtString> gPwrDeviceTypeString =
{
    L"System",
    L"Package",
    L"CPU CU",
    L"CPU Core",
    L"iGPU",
    L"dGPU",
    L"SVI2"
};

static const gtVector<gtString> gPwrCounterCategoryString =
{
    L"Power",
    L"Frequency",
    L"Temperature",
    L"Voltage",
    L"Current",
    L"DVFS",
    L"Process",
    L"Time",
    L"Generic Count Value"
};

static const gtVector<gtString> gPwrCounterAggregationString =
{
    L"Single",
    L"Cumulative",
    L"Histogram"
};

static const gtVector<gtString> gPwrCounterUnitString =
{
    L"Count index",
    L"Percentage",
    L"Ratio",
    L"Milliseconds",
    L"Joule",
    L"Watt",
    L"Volt",
    L"Milli Ampere",
    L"MHz",
    L"Centigrade"
};

#define PP_UNKNOWN_STRING       L"Unknown"

bool BackendDataConvertor::IsCounterRequired(const AMDTPwrCounterDesc& pCounter)
{
    return (pCounter.m_aggregation == AMDT_PWR_VALUE_SINGLE  &&
            pCounter.m_category != AMDT_PWR_CATEGORY_PROCESS &&
            pCounter.m_category != AMDT_PWR_CATEGORY_COUNT);
}

PPDevice* BackendDataConvertor::CreatePPDevice(const AMDTPwrDevice& bePwrDevice, IPowerProfilerBackendAdapter* pBeAdapter)
{
    gtString deviceNameStr;
    gtString deviceDescStr;

    // Convert the device name string to gtString.
    if (bePwrDevice.m_pName != nullptr)
    {
        deviceNameStr.fromASCIIString(bePwrDevice.m_pName);
    }

    // Convert the device description string to gtString.
    if (bePwrDevice.m_pDescription != nullptr)
    {
        deviceDescStr.fromASCIIString(bePwrDevice.m_pDescription);
    }

    // Recursively, build the list of sub-devices.
    gtList<PPDevice*> subDevices;
    const AMDTPwrDevice* pChildDevice = bePwrDevice.m_pFirstChild;

    while (pChildDevice != nullptr)
    {
        subDevices.push_back(CreatePPDevice(*pChildDevice, pBeAdapter));
        pChildDevice = pChildDevice->m_pNextDevice;
    }

    // Get the device id.
    int deviceId = static_cast<int>(bePwrDevice.m_deviceID);

    // Filter the list of supported counters.
    gtList<AMDTPwrCounterDesc*> filteredSupportedCounters;

    GT_IF_WITH_ASSERT(pBeAdapter != nullptr)
    {
        // Extract the list of counters which are supported by this device.
        gtList<AMDTPwrCounterDesc*> beSupportedCounters;
        PPResult rc = pBeAdapter->GetDeviceCounters(deviceId, beSupportedCounters);

        GT_IF_WITH_ASSERT(rc < PPR_FIRST_ERROR)
        {
            for (AMDTPwrCounterDesc* pSupportedCounter : beSupportedCounters)
            {
                if (pSupportedCounter != nullptr)
                {
                    if (IsCounterRequired(*pSupportedCounter))
                    {
                        // Create a copy of the BE's counter.
                        AMDTPwrCounterDesc* pCounter =
                            BackendDataConvertor::CopyPwrCounterDesc(*pSupportedCounter);

                        // Add that new counter to our supported counters list.
                        filteredSupportedCounters.push_back(pCounter);
                    }
                }
            }
        }
    }
    return new PPDevice(deviceId, bePwrDevice.m_type, deviceNameStr, deviceDescStr, subDevices, filteredSupportedCounters);
}

PPDevice* BackendDataConvertor::CreatePPDevice(const AMDTPwrDevice& bePwrDevice)
{
    gtString deviceNameStr;
    gtString deviceDescStr;

    // Convert the device name string to gtString.
    if (bePwrDevice.m_pName != nullptr)
    {
        deviceNameStr.fromASCIIString(bePwrDevice.m_pName);
    }

    // Convert the device description string to gtString.
    if (bePwrDevice.m_pDescription != nullptr)
    {
        deviceDescStr.fromASCIIString(bePwrDevice.m_pDescription);
    }

    // Recursively, build the list of sub-devices.
    gtList<PPDevice*> subDevices;
    const AMDTPwrDevice* pChildDevice = bePwrDevice.m_pFirstChild;

    while (pChildDevice != nullptr)
    {
        subDevices.push_back(CreatePPDevice(*pChildDevice));
        pChildDevice = pChildDevice->m_pNextDevice;
    }

    // Get the device id.
    int deviceId = static_cast<int>(bePwrDevice.m_deviceID);

    // Convert the list of supported counters.
    gtList<AMDTPwrCounterDesc*> supportedCounters;
    AMDTUInt32 numOfSupportedCounters = 0;
    AMDTPwrCounterDesc* pBeSupportedCounters;
    AMDTResult rc = AMDTPwrGetDeviceCounters(deviceId, &numOfSupportedCounters, &pBeSupportedCounters);
    AMDTPwrCounterDesc* pCurrentCounter = pBeSupportedCounters;

    if (rc == AMDT_STATUS_OK)
    {
        for (size_t i = 0; (i < numOfSupportedCounters) && (pCurrentCounter != nullptr); ++i)
        {
            if (IsCounterRequired(*pCurrentCounter))
            {
                // Create a copy of the BE's counter.
                AMDTPwrCounterDesc* pCounter =
                    BackendDataConvertor::CopyPwrCounterDesc(*pCurrentCounter);

                if(AMDT_PWR_CATEGORY_CONTROLLER != pCounter->m_category)
                {
                    // Add that new counter to our supported counters list.
                    supportedCounters.push_back(pCounter);
                }
            }

            // Advance the pointer.
            ++pCurrentCounter;
        }
    }

    return new PPDevice(deviceId, bePwrDevice.m_type, deviceNameStr, deviceDescStr, subDevices, supportedCounters);
}

bool BackendDataConvertor::AMDTPwrSampleToPPSample(const AMDTPwrSample* pSample, AMDTProfileTimelineSample& buffer)
{
#ifdef DEBUG_LOG_POWER_COUNTER_VALUES
    static std::ofstream outFile;
    static bool isFileInitialized = false;

    if (!isFileInitialized)
    {
        outFile.open("c:\\power_counter_values.csv", ios::out);
        isFileInitialized = true;
    }

#endif

    bool ret = false;
    GT_IF_WITH_ASSERT(pSample != nullptr)
    {
        AMDTPwrCounterValue* pSampleValues = pSample->m_counterValues;
        unsigned numOfValues = pSample->m_numOfValues;

        GT_IF_WITH_ASSERT(pSampleValues != nullptr)
        {
            // Copy the time stamp.
            buffer.m_sampleSystemTime.m_second      = pSample->m_systemTime.m_second;
            buffer.m_sampleSystemTime.m_microSecond = pSample->m_systemTime.m_microSecond;
            buffer.m_sampleElapsedTimeMs            = pSample->m_elapsedTimeMs;

            // Copy the values from the BE's memory to ours, as that memory will
            // be overridden with the next read operation invocation.
            gtVector<AMDTProfileCounterValue>& bufferSampleVals = buffer.m_sampleValues;
            bufferSampleVals.reserve(numOfValues);

            for (size_t i = 0; i < numOfValues; ++i)
            {
                // Copy the value to our container.
                AMDTProfileCounterValue aSampleValue;

                aSampleValue.m_counterId    = pSampleValues[i].m_counterID;
                aSampleValue.m_counterValue = pSampleValues[i].m_counterValue;

                bufferSampleVals.push_back(aSampleValue);

#ifdef DEBUG_LOG_POWER_COUNTER_VALUES
                // Dump to file for debug
                outFile << pSample->m_elapsedTimeMs << "," << pSampleValues[i].m_counterID << "," << pSampleValues[i].m_counterValue << std::endl;
#endif
            }

            ret = true;
        }
    }
    return ret;
}

AMDTPwrCounterDesc* BackendDataConvertor::CopyPwrCounterDesc(const AMDTPwrCounterDesc& other)
{
    // Allocate the object.
    AMDTPwrCounterDesc* pRet = new AMDTPwrCounterDesc;

    // Assign the primitive types.
    pRet->m_deviceId        = other.m_deviceId;
    pRet->m_aggregation     = other.m_aggregation;
    pRet->m_category        = other.m_category;
    pRet->m_counterID       = other.m_counterID;
    pRet->m_maxValue        = other.m_maxValue;
    pRet->m_minValue        = other.m_minValue;
    pRet->m_units           = other.m_units;

    // Copy the description string.
    if (other.m_description != nullptr)
    {
        size_t sz = strlen(other.m_description);
        pRet->m_description = new char[sz + 1];

        if (sz > 0)
        {
            strncpy(pRet->m_description, other.m_description, sz);
        }

        pRet->m_description[sz] = '\0';
    }

    // Copy the name string.
    if (other.m_name != nullptr)
    {
        size_t sz = strlen(other.m_name);
        pRet->m_name = new char[sz + 1];

        if (sz > 0)
        {
            strncpy(pRet->m_name, other.m_name, sz);
        }

        pRet->m_name[sz] = '\0';
    }

    return pRet;
}

AMDTPwrCounterDesc* BackendDataConvertor::ConvertToPwrCounterDesc(const AMDTProfileCounterDesc& counterDesc)
{
    // !!!  It is callers responsibility to free these objects !!!
    AMDTPwrCounterDesc* pPwrCounterDesc = new AMDTPwrCounterDesc;

    if (nullptr != pPwrCounterDesc)
    {
        pPwrCounterDesc->m_counterID = counterDesc.m_id;
        pPwrCounterDesc->m_deviceId = counterDesc.m_deviceId;

        if (!counterDesc.m_name.isEmpty())
        {
            size_t sz = strlen(counterDesc.m_name.asASCIICharArray());

            pPwrCounterDesc->m_name = new char[sz + 1];

            if (sz > 0)
            {
                strncpy(pPwrCounterDesc->m_name, counterDesc.m_name.asASCIICharArray(), sz);
            }

            pPwrCounterDesc->m_name[sz] = '\0';
        }

        if (! counterDesc.m_description.isEmpty())
        {
            size_t sz = strlen(counterDesc.m_description.asASCIICharArray());

            pPwrCounterDesc->m_description = new char[sz + 1];

            if (sz > 0)
            {
                strncpy(pPwrCounterDesc->m_description, counterDesc.m_description.asASCIICharArray(), sz);
            }

            pPwrCounterDesc->m_description[sz] = '\0';
        }

        //pPwrCounterDesc->m_name = strdup(counterDesc.m_name.asASCIICharArray());
        //pPwrCounterDesc->m_description = strdup(counterDesc.m_description.asASCIICharArray());

        pPwrCounterDesc->m_aggregation  = static_cast<AMDTPwrAggregation>(counterDesc.m_type);
        pPwrCounterDesc->m_category     = static_cast<AMDTPwrCategory>(counterDesc.m_category);
        pPwrCounterDesc->m_units        = static_cast<AMDTPwrUnit>(counterDesc.m_unit);
        pPwrCounterDesc->m_maxValue     = 0.0;
        pPwrCounterDesc->m_minValue     = 0.0;
    }

    return pPwrCounterDesc;
}

AMDTProfileCounterDesc* BackendDataConvertor::ConvertToProfileCounterDesc(const AMDTPwrCounterDesc& counterDesc)
{
    // !!!  It is callers responsibility to free these objects !!!
    AMDTProfileCounterDesc* pCounterDesc = new AMDTProfileCounterDesc;

    if (nullptr != pCounterDesc)
    {
        pCounterDesc->m_id = counterDesc.m_counterID;
        pCounterDesc->m_deviceId = counterDesc.m_deviceId;

        if (counterDesc.m_name != nullptr)
        {
            pCounterDesc->m_name.fromASCIIString(counterDesc.m_name);
        }

        if (counterDesc.m_description != nullptr)
        {
            pCounterDesc->m_description.fromASCIIString(counterDesc.m_description);
        }

        pCounterDesc->m_type = counterDesc.m_aggregation;
        AMDTPwrAggregationToString(counterDesc.m_aggregation, pCounterDesc->m_typeStr);

        pCounterDesc->m_category = counterDesc.m_category;
        AMDTPwrCategoryToString(counterDesc.m_category, pCounterDesc->m_categoryStr);

        pCounterDesc->m_unit = counterDesc.m_units;
        AMDTPwrUnitToString(counterDesc.m_units, pCounterDesc->m_unitStr);
    }

    return pCounterDesc;
}

bool BackendDataConvertor::ConvertToProfileCounterDescVec(gtList<AMDTPwrCounterDesc*>& pwrCounterDesc,
                                                          gtVector<AMDTProfileCounterDesc*>& counterDesc)
{
    bool ret = true;

    for (auto aPwrCounterDesc : pwrCounterDesc)
    {
        AMDTProfileCounterDesc* pCounterDesc = ConvertToProfileCounterDesc(*aPwrCounterDesc);

        if (nullptr != pCounterDesc)
        {
            counterDesc.push_back(pCounterDesc);
        }
    }

    return ret;
}

AMDTProfileDevice* BackendDataConvertor::CreateProfileDevice(const AMDTPwrDevice& bePwrDevice)
{
    AMDTProfileDevice* pDevice = nullptr;

    pDevice = new AMDTProfileDevice;

    if (nullptr != pDevice)
    {
        pDevice->m_deviceId = bePwrDevice.m_deviceID;

        if (nullptr != bePwrDevice.m_pName)
        {
            pDevice->m_name.fromASCIIString(bePwrDevice.m_pName);
        }

        if (nullptr != bePwrDevice.m_pDescription)
        {
            pDevice->m_description.fromASCIIString(bePwrDevice.m_pDescription);
        }

        pDevice->m_deviceType = bePwrDevice.m_type;
        AMDTDeviceTypeToString(bePwrDevice.m_type, pDevice->m_deviceTypeStr);

        const AMDTPwrDevice* pChildDevice = bePwrDevice.m_pFirstChild;

        while (pChildDevice != nullptr)
        {
            pDevice->m_subDeviceIds.push_back(pChildDevice->m_deviceID);
            pChildDevice = pChildDevice->m_pNextDevice;
        }
    }

    return pDevice;
}

bool BackendDataConvertor::ConvertToProfileDevice(const AMDTPwrDevice& bePwrDevice, gtVector<AMDTProfileDevice*>& profileDevice)
{
    bool ret = true;
    gtList<const AMDTPwrDevice*> deviceList;

    deviceList.push_back(&bePwrDevice);

    while (deviceList.size() > 0)
    {
        const AMDTPwrDevice* pPwrDevice = deviceList.front();
        deviceList.pop_front();

        AMDTProfileDevice* pDevice = CreateProfileDevice(*pPwrDevice);

        if (nullptr != pDevice)
        {
            profileDevice.push_back(pDevice);
        }

        const AMDTPwrDevice* pChildDevice = pPwrDevice->m_pFirstChild;

        while (pChildDevice != nullptr)
        {
            deviceList.push_back(pChildDevice);
            pChildDevice = pChildDevice->m_pNextDevice;
        }
    }

    return ret;
}

AMDTProfileDevice* BackendDataConvertor::CreateProfileDevice(const PPDevice& aPPDevice)
{
    AMDTProfileDevice* pProfileDevice = nullptr;

    pProfileDevice = new AMDTProfileDevice;

    if (nullptr != pProfileDevice)
    {
        pProfileDevice->m_deviceId      = aPPDevice.m_deviceId;
        pProfileDevice->m_name          = aPPDevice.m_deviceName;
        pProfileDevice->m_description   = aPPDevice.m_deviceDescription;
        pProfileDevice->m_deviceType    = aPPDevice.m_deviceType;

        AMDTDeviceTypeToString(aPPDevice.m_deviceType, pProfileDevice->m_deviceTypeStr);

        for (auto childDevice : aPPDevice.m_subDevices)
        {
            pProfileDevice->m_subDeviceIds.push_back(childDevice->m_deviceId);
        }
    }

    return pProfileDevice;
}

bool BackendDataConvertor::ConvertToProfileDevice(const gtList<PPDevice*>& ppDevices, gtVector<AMDTProfileDevice*>& profileDeviceVec)
{
    bool ret = true;

    for (auto aPPDevice : ppDevices)
    {
        gtList<const PPDevice*> ppDeviceList;
        ppDeviceList.push_back(aPPDevice);

        while (ppDeviceList.size() > 0)
        {
            const PPDevice* pDevice = ppDeviceList.front();
            ppDeviceList.pop_front();

            AMDTProfileDevice* profileDevice = CreateProfileDevice(*pDevice);

            if (nullptr != profileDevice)
            {
                profileDeviceVec.push_back(profileDevice);
            }

            for (auto childPPDevice : pDevice->m_subDevices)
            {
                ppDeviceList.push_back(childPPDevice);
            }
        }
    }

    return ret;
}

bool BackendDataConvertor::AMDTDeviceTypeToString(AMDTDeviceType deviceType, gtString& deviceTypeStr)
{
    bool ret = true;

    if (deviceType >= 0 && (size_t)deviceType < gPwrDeviceTypeString.size())
    {
        deviceTypeStr = gPwrDeviceTypeString[deviceType];
    }
    else
    {
        deviceTypeStr = PP_UNKNOWN_STRING;
    }

    return ret;
}

bool BackendDataConvertor::AMDTPwrUnitToString(AMDTPwrUnit unit, gtString& unitAsStr)
{
    bool ret = true;

    if (unit >= 0 && (size_t)unit < gPwrCounterUnitString.size())
    {
        unitAsStr = gPwrCounterUnitString[unit];
    }
    else
    {
        unitAsStr = PP_UNKNOWN_STRING;
    }

    return ret;
}

bool BackendDataConvertor::AMDTPwrAggregationToString(AMDTPwrAggregation aggregationType, gtString& aggregationAsStr)
{
    bool ret = true;

    if (aggregationType >= 0 && (size_t)aggregationType < gPwrCounterAggregationString.size())
    {
        aggregationAsStr = gPwrCounterAggregationString[aggregationType];
    }
    else
    {
        aggregationAsStr = PP_UNKNOWN_STRING;
    }

    return ret;
}

bool BackendDataConvertor::AMDTPwrCategoryToString(AMDTPwrCategory category, gtString& categoryAsStr)
{
    bool ret = true;

    if (category >= 0 && (size_t)category < gPwrCounterCategoryString.size())
    {
        categoryAsStr = gPwrCounterCategoryString[category];
    }
    else
    {
        categoryAsStr = PP_UNKNOWN_STRING;
    }

    return ret;
}

bool BackendDataConvertor::GetPwrDeviceIdStringMap(gtMap<gtString, int>& deviceIdStrMap)
{
    bool ret = true;

    int i = 0;

    for (const auto& deviceType : gPwrDeviceTypeString)
    {
        deviceIdStrMap.insert(gtMap<gtString, int>::value_type(deviceType, i));
        i++;
    }

    return ret;
}

bool BackendDataConvertor::GetPwrAggregationIdStringMap(gtMap<gtString, int>& aggregationIdStrMap)
{
    bool ret = true;

    int i = 0;

    for (const auto& aggregation : gPwrCounterAggregationString)
    {
        aggregationIdStrMap.insert(gtMap<gtString, int>::value_type(aggregation, i));
        i++;
    }

    return ret;
}

bool BackendDataConvertor::GetPwrUnitIdStringMap(gtMap<gtString, int>& unitIdStrMap)
{
    bool ret = true;

    int i = 0;

    for (const auto& unit : gPwrCounterUnitString)
    {
        unitIdStrMap.insert(gtMap<gtString, int>::value_type(unit, i));
        i++;
    }

    return ret;
}

bool BackendDataConvertor::GetPwrCategoryIdStringMap(gtMap<gtString, int>& categoryIdStrMap)
{
    bool ret = true;

    int i = 0;

    for (const auto& category : gPwrCounterCategoryString)
    {
        categoryIdStrMap.insert(gtMap<gtString, int>::value_type(category, i));
        i++;
    }

    return ret;
}