//==============================================================================
// Copyright (c) 2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief
//==============================================================================

#ifndef _DC_DETOUR_ID3D11DEVICECONTEXT_H_
#define _DC_DETOUR_ID3D11DEVICECONTEXT_H_

#include "DCID3D11DeviceContextVTableManager.h"

/// \addtogroup DCVirtualTablePatching
// @{

//------------------------------------------------------------------------------------
/// ID3D11DeviceContext VTable manager for Profiler
//------------------------------------------------------------------------------------
class DCProfilerDeviceContext11VTManager : public ID3D11DeviceContextVTableManager
{
public:
    /// Constructor
    /// \param pFnMine Release function pointer
    DCProfilerDeviceContext11VTManager(ptrdiff_t* pFnMine);

private:
    /// Copy constructor - disabled
    /// \param obj source
    DCProfilerDeviceContext11VTManager(const DCProfilerDeviceContext11VTManager& obj);

    /// Assignment operator - disabled
    /// \param obj left
    /// \return right
    DCProfilerDeviceContext11VTManager& operator = (const DCProfilerDeviceContext11VTManager& obj);
};

/// DCProfilerDeviceContext11VTManager Accessor
/// \return pointer to DCProfilerDeviceContext11VTManager
DCProfilerDeviceContext11VTManager* GetID3D11DeviceContextVTableManager();

// @}

#endif // _DC_DETOUR_ID3D11DEVICECONTEXT_H_