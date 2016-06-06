//==============================================================================
// Copyright (c) 2015-2016 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  Some helper functions for using HSA
//==============================================================================

#include "HSAUtils.h"

struct GetGPUDeviceInfo
{
    HSAModule* m_pHsaModule;
    HSADeviceIdList* m_pHsaDeviceIds;
};

HSAUtils::HSAUtils() : m_pHsaModule(nullptr)
{
}

HSAUtils::~HSAUtils()
{
    if (nullptr != m_pHsaModule)
    {
        delete m_pHsaModule;
        m_pHsaModule = nullptr;
    }
}

hsa_status_t HSAUtils::GetGPUDevicesCallback(hsa_agent_t agent, void* pData)
{
    hsa_status_t status = HSA_STATUS_SUCCESS;

    if (nullptr == pData)
    {
        status = HSA_STATUS_ERROR_INVALID_ARGUMENT;
    }
    else if (nullptr == static_cast<GetGPUDeviceInfo*>(pData)->m_pHsaModule || nullptr == static_cast<GetGPUDeviceInfo*>(pData)->m_pHsaDeviceIds)
    {
        status = HSA_STATUS_ERROR_INVALID_ARGUMENT;
    }
    else
    {
        hsa_device_type_t deviceType;
        status = static_cast<GetGPUDeviceInfo*>(pData)->m_pHsaModule->agent_get_info(agent, HSA_AGENT_INFO_DEVICE, &deviceType);

        if (HSA_STATUS_SUCCESS == status)
        {
            if (HSA_DEVICE_TYPE_GPU == deviceType)
            {
                uint32_t deviceId;
                status = static_cast<GetGPUDeviceInfo*>(pData)->m_pHsaModule->agent_get_info(agent, static_cast<hsa_agent_info_t>(HSA_AMD_AGENT_INFO_CHIP_ID), &deviceId);

                if (HSA_STATUS_SUCCESS == status)
                {
                    static_cast<GetGPUDeviceInfo*>(pData)->m_pHsaDeviceIds->push_back(deviceId);
                }
            }
        }
    }

    return status;
}


bool HSAUtils::GetHSADeviceIds(HSADeviceIdList& hsaDeviceIds)
{
    bool retVal = true;

    if (InitHSAModule() && m_pHsaModule->IsModuleLoaded())
    {
        GetGPUDeviceInfo getGPUDeviceInfo = { m_pHsaModule, &hsaDeviceIds };
        //iterate agents, and store the GPU ids for each one
        hsa_status_t status = m_pHsaModule->iterate_agents(GetGPUDevicesCallback, &getGPUDeviceInfo);

        if (HSA_STATUS_SUCCESS != status && HSA_STATUS_INFO_BREAK != status)
        {
            retVal = false;
        }
    }

    return retVal;
}

bool HSAUtils::InitHSAModule()
{
    bool retVal = true;

    if (nullptr == m_pHsaModule)
    {
        retVal = false;
        m_pHsaModule = new(std::nothrow) HSAModule();

        if (nullptr != m_pHsaModule)
        {
            if (m_pHsaModule->IsModuleLoaded())
            {
                retVal = HSA_STATUS_SUCCESS == m_pHsaModule->init();
            }
            else
            {
                delete m_pHsaModule;
                m_pHsaModule = nullptr;
            }
        }
    }

    return retVal;
}
