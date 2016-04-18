//=============================================================================
// Copyright (c) 2016 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  Base class for derived D3D capture classes
//=============================================================================

#ifndef D3DCAPTURE_H
#define D3DCAPTURE_H
#include <string>

#include "../common/Capture.h"
#include <d3d11_1.h>

//-----------------------------------------------------------------------------
/// D3D Capture classes derive from this class rather than from Capture
/// This class takes care of the interface pointer and is also able to return
/// The class type
//-----------------------------------------------------------------------------
class D3DCapture : public Capture
{
public:

    // D3D Interfaces pointer, could be D3DDeviceContext, D3DContext... any COM pointer
    IUnknown* m_pObj;

    static long m_sCaptureCount;

    //-----------------------------------------------------------------------------
    /// Initializes variables to nullptr
    //-----------------------------------------------------------------------------
    D3DCapture();

    //-----------------------------------------------------------------------------
    /// \return the interface
    //-----------------------------------------------------------------------------
    IUnknown* GetInterface();

    //-----------------------------------------------------------------------------
    /// \return the class type
    //-----------------------------------------------------------------------------
    virtual CaptureClassType GetClassType();
};

#endif //D3DCAPTURE_H