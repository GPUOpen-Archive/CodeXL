//==============================================================================
// Copyright (c) 2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief
//==============================================================================

#ifndef _DC_DETOUR_ID3D11DEVICE_H_
#define _DC_DETOUR_ID3D11DEVICE_H_

#include <d3d11.h>
#include "DCID3D11DeviceVTManager.h"

/// \addtogroup DCVirtualTablePatching
// @{

//------------------------------------------------------------------------------------
/// ID3D11Device VTable manager for Profiler
//------------------------------------------------------------------------------------
class DCProfilerDevice11VTManager : public DCID3D11DeviceVTManager
{
public:
    /// Constructor
    /// \param pFnMine Release function pointer
    DCProfilerDevice11VTManager(ptrdiff_t* pFnMine);

private:
    /// Copy constructor - disabled
    /// \param obj source
    DCProfilerDevice11VTManager(const DCProfilerDevice11VTManager& obj);

    /// Assignment operator - disabled
    /// \param obj left
    /// \return right
    DCProfilerDevice11VTManager& operator = (const DCProfilerDevice11VTManager& obj);
};

DCProfilerDevice11VTManager* GetID3D11DeviceVTableManager();

// @}

#endif // _DC_DETOUR_ID3D11DEVICE_H_