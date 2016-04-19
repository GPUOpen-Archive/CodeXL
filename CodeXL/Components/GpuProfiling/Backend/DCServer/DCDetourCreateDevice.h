//==============================================================================
// Copyright (c) 2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief
//==============================================================================

#ifndef _DC_DETOUR_CREATE_DEVICE_H_
#define _DC_DETOUR_CREATE_DEVICE_H_

#include <windows.h>
#include <d3d11.h>
#include "DetourBase.h"

/// \defgroup DCCodeInjection DCCodeInjection
/// This module handles code injection either through Microsoft Detours or virtual
/// table patching.
/// \ingroup DCServer
// @{
// @}

/// \defgroup DCDetour DCDetour
/// This module handles the code injection through the use of Microsoft Detours.
/// It detours DX create device and compile shader calls.
///
/// \ingroup DCCodeInjection
// @{

//------------------------------------------------------------------------------------
/// This class detours CreateDevice
//------------------------------------------------------------------------------------
class DCDetourD3D11CreateDevice : public DetourBase
{
public:
    /// Default Constructor
    DCDetourD3D11CreateDevice();
    ///// Detach detoured CreateDevice
    bool Detach();

protected:
    ///// Detour CreateDevice
    bool OnAttach();

private:
    /// Disable copy constructor
    /// \param[in] obj  the input object
    DCDetourD3D11CreateDevice(const DCDetourD3D11CreateDevice& obj);

    /// Disable assignment operator
    /// \param[in] obj  the input object
    /// \return a reference of the object
    DCDetourD3D11CreateDevice& operator=(const DCDetourD3D11CreateDevice& obj);
};

// @}

#endif // _DC_DETOUR_CREATE_DEVICE_H_
