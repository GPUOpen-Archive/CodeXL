//==============================================================================
// Copyright (c) 2013-2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  Interface used to track the currently active core objects
//==============================================================================

#ifndef IMONITOR_H
#define IMONITOR_H

#include "CommonTypes.h"

//===============================================================================================
/// Enumerated types of devices that can be monitored
//===============================================================================================
enum CREATION_TYPE
{
    DX9_DEVICE = 0,
    DX10_DEVICE,
    DX11_DEVICE,
    DXGI_FACTORY,
    DX11_SWAPCHAIN,
    D3D11_DEVICE_CONTEXT,
    OGL_CONTEXT,
    MANTLE_DEVICE,
    DX12_DEVICE,
    VULKAN_DEVICE,
    D3D_USERDEFINEDANNOTATION
};

//===============================================================================================
/// Converts a CREATION_TYPE enum element into a string
//===============================================================================================
const char* CreationTypeToString(CREATION_TYPE type);

//===============================================================================================
/// This interface is used as a base class for API specific Monitors. Monitors are responsible
/// for recording the currently monitored devices.
//===============================================================================================
class IMonitor
{

public:

    //===============================================================================================
    /// Register a device for monitoring
    /// \param eType The type of the device to monitor - see CREATION_TYPE.
    /// \param pPtr The pointer to the device object.
    //===============================================================================================
    virtual void StartMonitoring(CREATION_TYPE eType, void* pPtr) = 0;

    //===============================================================================================
    /// Unregister a device from being monitored
    /// \param eType The type of the device to monitor - see CREATION_TYPE.
    /// \param pPtr The pointer to the device object.
    //===============================================================================================
    virtual void StopMonitoring(CREATION_TYPE eType, void* pPtr) = 0;

};

#endif //IMONITOR_H