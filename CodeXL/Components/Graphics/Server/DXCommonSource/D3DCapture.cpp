//=============================================================================
// Copyright (c) 2016 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  Base class for derived D3D capture classes
//=============================================================================

#include <d3d11_1.h>
#include "D3DCapture.h"
#include "../Common/Logger.h"

long D3DCapture::m_sCaptureCount = 0;

//-----------------------------------------------------------------------------
/// Constructor.
//-----------------------------------------------------------------------------
D3DCapture::D3DCapture()
{
    m_pObj = nullptr;

    // Debug variable used to give each capture an index value
    //m_sCaptureCount++;

    // Debug code used to locate which capture is a problem
    //if (m_sCaptureCount % 500 == 0)
    //{
    //    Log(logDEBUG, "gCaptureCount: %ld\n", m_sCaptureCount);
    //}
}

//-----------------------------------------------------------------------------
/// Return interface pointer.
//-----------------------------------------------------------------------------
IUnknown* D3DCapture::GetInterface()
{
    return m_pObj;
}

//-----------------------------------------------------------------------------
/// Return class type.
//-----------------------------------------------------------------------------
CaptureClassType D3DCapture::GetClassType()
{
    return CCT_RegularClass;
}