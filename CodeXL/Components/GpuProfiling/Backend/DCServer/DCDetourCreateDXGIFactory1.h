//==============================================================================
// Copyright (c) 2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief This class simply gets the "real" function pointer for CreateDXGIFactory1
//==============================================================================

#ifndef _DC_DETOUR_CREATE_DXGIFACTORY1_H_
#define _DC_DETOUR_CREATE_DXGIFACTORY1_H_

#include <windows.h>
#include <d3d11.h>
#include "DCFuncDefs.h"
#include "DetourBase.h"

/// \addtogroup DCDetour
// @{

//------------------------------------------------------------------------------------
/// This class simply gets the "real" function pointer for CreateDXGIFactory1
//------------------------------------------------------------------------------------
class DCDetourCreateDXGIFactory1 : public DetourBase
{
public:
    /// Constructor
    DCDetourCreateDXGIFactory1() : DetourBase(L"dxgi.dll") {}
    /// Detach detoured CreateDXGIFactory1
    bool Detach();

protected:
    /// Detour CreateDXGIFactory1
    bool OnAttach();

private:
    /// Disable copy constructor
    /// \param[in] obj  the input object
    DCDetourCreateDXGIFactory1(const DCDetourCreateDXGIFactory1& obj);

    /// Disable assignment operator
    /// \param[in] obj  the input object
    /// \return a reference of the object
    DCDetourCreateDXGIFactory1& operator=(const DCDetourCreateDXGIFactory1& obj);
};

// @}

#endif // _DC_DETOUR_CREATE_DXGIFACTORY1_H_
