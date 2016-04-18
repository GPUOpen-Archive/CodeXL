//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file PPDevice.cpp
///
//==================================================================================

// Local.
#include <AMDTPowerProfilingMidTier/include/PowerProfilerDefs.h>
#include <AMDTPowerProfilingMidTier/include/PPDevice.h>

// Infra.
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTBaseTools/Include/gtString.h>

// Backend data types.
#include <AMDTPowerProfileAPI/inc/AMDTPowerProfileApi.h>

PPDevice::PPDevice(const PPDevice& other)
{
    m_deviceId = other.m_deviceId;
    m_deviceType = other.m_deviceType;
    m_deviceName = other.m_deviceName;
    m_deviceDescription = other.m_deviceDescription;

    // Copy the list of supported counters.
    for (const auto pCounterDesc : other.m_supportedCounters)
    {
        if (pCounterDesc != NULL)
        {
            AMDTPwrCounterDesc* pSupportedCounter = new AMDTPwrCounterDesc;
            pSupportedCounter->m_aggregation = pCounterDesc->m_aggregation;
            pSupportedCounter->m_category = pCounterDesc->m_category;
            pSupportedCounter->m_counterID = pCounterDesc->m_counterID;
            pSupportedCounter->m_deviceId = pCounterDesc->m_deviceId;
            pSupportedCounter->m_maxValue = pCounterDesc->m_maxValue;
            pSupportedCounter->m_minValue = pCounterDesc->m_minValue;
            pSupportedCounter->m_units = pCounterDesc->m_units;

            // Copy the description string.
            if (pCounterDesc->m_description != NULL)
            {
                size_t sz = strlen(pCounterDesc->m_description);
                pSupportedCounter->m_description = new char[sz + 1];

                if (sz > 0)
                {
                    strncpy(pSupportedCounter->m_description, pCounterDesc->m_description, sz);
                    //std::copy(pCounterDesc->m_description, pCounterDesc->m_description + sz, pSupportedCounter->m_description);
                }

                pSupportedCounter->m_description[sz] = '\0';
            }

            // Copy the name string.
            if (pCounterDesc->m_name != NULL)
            {
                size_t sz = strlen(pCounterDesc->m_name);
                pSupportedCounter->m_name = new char[sz + 1];

                if (sz > 0)
                {
                    strncpy(pSupportedCounter->m_name, pCounterDesc->m_name, sz);
                    //std::copy(pCounterDesc->m_name, pCounterDesc->m_name + sz, pSupportedCounter->m_name);
                }

                pSupportedCounter->m_name[sz] = '\0';
            }

            // Add the counter to our supported counters list.
            m_supportedCounters.push_back(pSupportedCounter);
        }
    }

    // Recursively copy the sub devices.
    for (const auto pOtherSubDevice : other.m_subDevices)
    {
        if (pOtherSubDevice != NULL)
        {
            PPDevice* pSubDevice = new PPDevice(*pOtherSubDevice);
            m_subDevices.push_back(pSubDevice);
        }
    }
}

PPDevice::PPDevice(int deviceId, AMDTDeviceType deviceType, const gtString& deviceName, const gtString& deviceDescription, const gtList<PPDevice*>& subDevices, const gtList<AMDTPwrCounterDesc*>& supportedCounters) :
    m_deviceId(deviceId), m_deviceType(deviceType), m_deviceName(deviceName), m_deviceDescription(deviceDescription),
    m_subDevices(subDevices), m_supportedCounters(supportedCounters)
{

}

PPDevice::~PPDevice()
{
    for (AMDTPwrCounterDesc*& pCounterDesc : m_supportedCounters)
    {
        delete pCounterDesc;
        pCounterDesc = NULL;
    }

    for (PPDevice*& pSubDevice : m_subDevices)
    {
        delete pSubDevice;
        pSubDevice = NULL;
    }
}