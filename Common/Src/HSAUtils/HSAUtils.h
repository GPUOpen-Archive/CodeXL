//==============================================================================
// Copyright (c) 2015-2016 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  Some helper functions for using HSA
//==============================================================================

#ifndef _HSA_UTILS_H
#define _HSA_UTILS_H

#include <vector>

#include "HSAModule.h"
#include "TSingleton.h"

typedef std::vector<uint32_t> HSADeviceIdList;

class HSAUtils : public TSingleton < HSAUtils >
{
    friend class TSingleton < HSAUtils > ;

public:
    /// constructor
    HSAUtils();

    // destructor
    ~HSAUtils();

    bool GetHSADeviceIds(HSADeviceIdList& hsaDeviceIds);

private:
    bool InitHSAModule();
    static hsa_status_t GetGPUDevicesCallback(hsa_agent_t agent, void* pData);

    HSAModule* m_pHsaModule;
};

#endif // _HSA_UTILS_H
