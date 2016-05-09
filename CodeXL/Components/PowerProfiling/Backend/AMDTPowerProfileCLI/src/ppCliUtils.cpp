//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file ppCliUtils.cpp
///
//==================================================================================

// Project:
#include <ppCliUtils.h>
#include <string>

// Device Type Strings
const char* STR_AMDT_PWR_DEVICE_SYSTEM = "System";
const char* STR_AMDT_PWR_DEVICE_PACKAGE = "Package";
const char* STR_AMDT_PWR_DEVICE_CPU_COMPUTE_UNIT = "CPU CU";
const char* STR_AMDT_PWR_DEVICE_CPU_CORE = "CPU Core";
const char* STR_AMDT_PWR_DEVICE_INTERNAL_GPU = "iGPU";
const char* STR_AMDT_PWR_DEVICE_EXTERNAL_GPU = "dGPU";
const char* STR_AMDT_PWR_DEVICE_SVI2 = "SVI2";
const char* STR_UNKNOWN_DEVICE_UNKNOWN = "Unknown";

// Category String
const char* STR_AMDT_PWR_CATEGORY_POWER = "Power";
const char* STR_AMDT_PWR_CATEGORY_TEMPERATURE = "Temperature";
const char* STR_AMDT_PWR_CATEGORY_FREQUENCY = "Frequency";
const char* STR_AMDT_PWR_CATEGORY_CURRENT = "Current";
const char* STR_AMDT_PWR_CATEGORY_VOLTAGE = "Voltage";
const char* STR_AMDT_PWR_CATEGORY_DVFS = "DVFS";
const char* STR_AMDT_PWR_CATEGORY_PROCESS = "Process";
const char* STR_AMDT_PWR_CATEGORY_TIME = "Time";
const char* STR_AMDT_PWR_CATEGORY_COUNT = "Generic Count Value";
const char* STR_AMDT_PWR_CATEGORY_UNKNOWN = "Unknown";

// Aggregation Type String
const char* STR_AMDT_PWR_AGG_SINGLE = "Single";
const char* STR_AMDT_PWR_AGG_CUMULATIVE = "Cumulative";
const char* STR_AMDT_PWR_AGG_HISTOGRAM = "Histogram";
const char* STR_AMDT_PWR_AGG_UNKNOWN = "Unknown";

// Unit String
const char* STR_AMDT_PWR_UNIT_COUNT = "Count index";
const char* STR_AMDT_PWR_UNIT_PERCENTAGE = "Percentage";
const char* STR_AMDT_PWR_UNIT_RATIO = "Ratio";
const char* STR_AMDT_PWR_UNIT_MILLISEC = "Milliseconds";
const char* STR_AMDT_PWR_UNIT_JOULE = "Joule";
const char* STR_AMDT_PWR_UNIT_WATT = "Watt";
const char* STR_AMDT_PWR_UNIT_VOLT = "Volt";
const char* STR_AMDT_PWR_UNIT_MILLI_AMPERE = "Milli Ampere";
const char* STR_AMDT_PWR_UNIT_MEGAHERTZ = "MHz";
const char* STR_AMDT_PWR_UNIT_CENTIGRADE = "Centigrade";
const char* STR_AMDT_PWR_UNIT_UNKNOWN = "Unknown";

bool ppCliUtils::GetCategoryString(AMDTPwrCategory category, gtString& categoryStr)
{
    bool ret = true;
    const char* pStr = nullptr;

    switch (category)
    {
        case AMDT_PWR_CATEGORY_POWER:
            pStr = STR_AMDT_PWR_CATEGORY_POWER;
            break;

        case AMDT_PWR_CATEGORY_TEMPERATURE:
            pStr = STR_AMDT_PWR_CATEGORY_TEMPERATURE;
            break;

        case AMDT_PWR_CATEGORY_FREQUENCY:
            pStr = STR_AMDT_PWR_CATEGORY_FREQUENCY;
            break;

        case AMDT_PWR_CATEGORY_CURRENT:
            pStr = STR_AMDT_PWR_CATEGORY_CURRENT;
            break;

        case AMDT_PWR_CATEGORY_VOLTAGE:
            pStr = STR_AMDT_PWR_CATEGORY_VOLTAGE;
            break;

        case AMDT_PWR_CATEGORY_DVFS:
            pStr = STR_AMDT_PWR_CATEGORY_DVFS;
            break;

        case AMDT_PWR_CATEGORY_PROCESS:
            pStr = STR_AMDT_PWR_CATEGORY_PROCESS;
            break;

        case AMDT_PWR_CATEGORY_TIME:
            pStr = STR_AMDT_PWR_CATEGORY_TIME;
            break;

        case AMDT_PWR_CATEGORY_COUNT:
            pStr = STR_AMDT_PWR_CATEGORY_COUNT;
            break;

        default:
            // This category is unknown and should be added.
            pStr = STR_AMDT_PWR_CATEGORY_UNKNOWN;
            ret = false;
            break;
    }

    categoryStr.fromASCIIString(pStr);

    return ret;
} // GetCategoryString

bool ppCliUtils::GetUnitString(AMDTPwrUnit unitType, gtString& unitStr)
{
    bool ret = true;
    const char* pStr = nullptr;

    switch (unitType)
    {
        case AMDT_PWR_UNIT_TYPE_COUNT:
            pStr = STR_AMDT_PWR_UNIT_COUNT;
            break;

        case AMDT_PWR_UNIT_TYPE_PERCENT:
            pStr = STR_AMDT_PWR_UNIT_PERCENTAGE;
            break;

        case AMDT_PWR_UNIT_TYPE_RATIO:
            pStr = STR_AMDT_PWR_UNIT_RATIO;
            break;

        case AMDT_PWR_UNIT_TYPE_MILLI_SECOND:
            pStr = STR_AMDT_PWR_UNIT_MILLISEC;
            break;

        case AMDT_PWR_UNIT_TYPE_JOULE:
            pStr = STR_AMDT_PWR_UNIT_JOULE;
            break;

        case AMDT_PWR_UNIT_TYPE_WATT:
            pStr = STR_AMDT_PWR_UNIT_WATT;
            break;

        case AMDT_PWR_UNIT_TYPE_VOLT:
            pStr = STR_AMDT_PWR_UNIT_VOLT;
            break;

        case AMDT_PWR_UNIT_TYPE_MILLI_AMPERE:
            pStr = STR_AMDT_PWR_UNIT_MILLI_AMPERE;
            break;

        case AMDT_PWR_UNIT_TYPE_MEGA_HERTZ:
            pStr = STR_AMDT_PWR_UNIT_MEGAHERTZ;
            break;

        case AMDT_PWR_UNIT_TYPE_CENTIGRADE:
            pStr = STR_AMDT_PWR_UNIT_CENTIGRADE;
            break;

        default:
            // This unit is unknown and should be added.
            pStr = STR_AMDT_PWR_UNIT_UNKNOWN;
            ret = false;
            break;
    }

    unitStr.fromASCIIString(pStr);

    return ret;
} // GetUnitString

bool ppCliUtils::GetAggregationString(AMDTPwrAggregation aggregationType, gtString& aggregationStr)
{
    bool ret = true;
    const char* pStr = nullptr;

    switch (aggregationType)
    {
        case AMDT_PWR_VALUE_SINGLE:
            pStr = STR_AMDT_PWR_AGG_SINGLE;
            break;

        case AMDT_PWR_VALUE_CUMULATIVE:
            pStr = STR_AMDT_PWR_AGG_CUMULATIVE;
            break;

        case AMDT_PWR_VALUE_HISTOGRAM:
            pStr = STR_AMDT_PWR_AGG_HISTOGRAM;
            break;

        default:
            // This aggregation type is unknown and should be added.
            pStr = STR_AMDT_PWR_AGG_UNKNOWN;
            ret = false;
            break;
    }

    aggregationStr.fromASCIIString(pStr);

    return ret;
}

bool ppCliUtils::GetDeviceTypeString(AMDTDeviceType deviceType, gtString& deviceTypeStr)
{
    bool ret = true;
    const char* pStr = nullptr;

    switch (deviceType)
    {
        case AMDT_PWR_DEVICE_SYSTEM:
            pStr = STR_AMDT_PWR_DEVICE_SYSTEM;
            break;

        case AMDT_PWR_DEVICE_PACKAGE:
            pStr = STR_AMDT_PWR_DEVICE_PACKAGE;
            break;

        case AMDT_PWR_DEVICE_CPU_COMPUTE_UNIT:
            pStr = STR_AMDT_PWR_DEVICE_CPU_COMPUTE_UNIT;
            break;

        case AMDT_PWR_DEVICE_CPU_CORE:
            pStr = STR_AMDT_PWR_DEVICE_CPU_CORE;
            break;

        case AMDT_PWR_DEVICE_INTERNAL_GPU:
            pStr = STR_AMDT_PWR_DEVICE_INTERNAL_GPU;
            break;

        case AMDT_PWR_DEVICE_EXTERNAL_GPU:
            pStr = STR_AMDT_PWR_DEVICE_EXTERNAL_GPU;
            break;

        case AMDT_PWR_DEVICE_SVI2:
            pStr = STR_AMDT_PWR_DEVICE_SVI2;
            break;

        default:
            // This device type is unknown and should be added.
            pStr = STR_UNKNOWN_DEVICE_UNKNOWN;
            ret = false;
            break;
    }

    deviceTypeStr.fromASCIIString(pStr);

    return ret;
}

// !!!  It is callers responsibility to free these objects !!!
AMDTProfileDevice* ppCliUtils::ConvertToProfileDevice(AMDTPwrDevice& beDevice)
{
    AMDTProfileDevice* pDevice = nullptr;

    pDevice = new AMDTProfileDevice;

    if (nullptr != pDevice)
    {
        pDevice->m_deviceId = beDevice.m_deviceID;

        if (nullptr != beDevice.m_pName)
        {
            pDevice->m_name.fromASCIIString(beDevice.m_pName);
        }

        if (nullptr != beDevice.m_pDescription)
        {
            pDevice->m_description.fromASCIIString(beDevice.m_pDescription);
        }

        pDevice->m_deviceType = beDevice.m_type;
        ppCliUtils::GetDeviceTypeString(beDevice.m_type, pDevice->m_deviceTypeStr);

        const AMDTPwrDevice* pChildDevice = beDevice.m_pFirstChild;

        while (pChildDevice != nullptr)
        {
            pDevice->m_subDeviceIds.push_back(pChildDevice->m_deviceID);
            pChildDevice = pChildDevice->m_pNextDevice;
        }
    }

    return pDevice;
}

// !!!  It is callers responsibility to free these objects !!!
AMDTProfileCounterDesc* ppCliUtils::ConvertToProfileCounterDesc(AMDTPwrCounterDesc& counterDesc)
{
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
        ppCliUtils::GetAggregationString(counterDesc.m_aggregation, pCounterDesc->m_typeStr);

        pCounterDesc->m_category = counterDesc.m_category;
        ppCliUtils::GetCategoryString(counterDesc.m_category, pCounterDesc->m_categoryStr);

        pCounterDesc->m_unit = counterDesc.m_units;
        ppCliUtils::GetUnitString(counterDesc.m_units, pCounterDesc->m_unitStr);
    }

    return pCounterDesc;
}

AMDTProfileTimelineSample* ppCliUtils::ConvertPwrSampleToTimelineSample(AMDTPwrSample& aSample)
{
    AMDTProfileTimelineSample* pCurrSample = new AMDTProfileTimelineSample();

    if (nullptr != pCurrSample)
    {
        AMDTPwrCounterValue* pSampleValues = aSample.m_counterValues;
        unsigned numOfValues = aSample.m_numOfValues;

        if (nullptr != pSampleValues)
        {
            // Copy the time stamp.
            pCurrSample->m_sampleSystemTime.m_second        = aSample.m_systemTime.m_second;
            pCurrSample->m_sampleSystemTime.m_microSecond   = aSample.m_systemTime.m_microSecond;
            pCurrSample->m_sampleElapsedTimeMs              = aSample.m_elapsedTimeMs;

            gtVector<AMDTProfileCounterValue>& bufferSampleVals = pCurrSample->m_sampleValues;
            bufferSampleVals.reserve(numOfValues);

            for (size_t i = 0; i < numOfValues; ++i)
            {
                AMDTProfileCounterValue aSampleValue;

                aSampleValue.m_counterId    = pSampleValues[i].m_counterID;
                aSampleValue.m_counterValue = pSampleValues[i].m_counterValue;

                bufferSampleVals.push_back(aSampleValue);
            }
        }
    }

    return pCurrSample;
}