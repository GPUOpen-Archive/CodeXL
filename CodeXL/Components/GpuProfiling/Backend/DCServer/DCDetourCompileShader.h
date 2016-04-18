//==============================================================================
// Copyright (c) 2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief
//==============================================================================

#ifndef _DC_DETOUR_COMPILE_SHADER_H_
#define _DC_DETOUR_COMPILE_SHADER_H_

#include <windows.h>
#include <d3d11.h>
#include "DetourBase.h"

#include <AMDTBaseTools/Include/gtString.h>

/// \addtogroup DCDetour
// @{

//------------------------------------------------------------------------------------
/// This class detours D3DCompile
//------------------------------------------------------------------------------------
class DCDetourD3DCompile : public DetourBase
{
public:
    /// Default Constructor
    DCDetourD3DCompile() : DetourBase()
    {
        //m_strCurrentDetachedOne.clear();
    }

    /// Constructor
    /// \param[in] strDll  the filepath to the dll
    DCDetourD3DCompile(const gtString& strDll);

    ///// Detach detoured D3DCompile
    bool Detach();

protected:
    ///// Detour D3DCompile
    bool OnAttach();
    bool AttachD3DCompile();
    bool AttachD3DCompile2();
    bool AttachD3DCompileFromFile();

private:
    /// Disable copy constructor
    /// \param[in] obj  the input object
    DCDetourD3DCompile(const DCDetourD3DCompile& obj);

    /// Disable assignment operator
    /// \param[in] obj  the input object
    /// \return a reference of the object
    DCDetourD3DCompile& operator=(const DCDetourD3DCompile& obj);
};

// @}

#endif // _DC_DETOUR_COMPILE_SHADER_H_
