//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gsThreadLocalData.h
///
//==================================================================================

//------------------------------ gsThreadLocalData.h ------------------------------

#ifndef __GSTHREADLOCALDATA_H
#define __GSTHREADLOCALDATA_H

// Infra:
#include <AMDTOSAPIWrappers/Include/oaOSAPIDefinitions.h>


// ----------------------------------------------------------------------------------
// Struct Name:          gsThreadLocalData
// General Description:
//   Hold per thread data, needed by the OpenGL monitor.
//
// Author:               Yaki Tebeka
// Creation Date:        30/1/2008
// ----------------------------------------------------------------------------------
struct gsThreadLocalData
{
    // The device (display) on which the drawables exist:
    oaDeviceContextHandle _hDC;

    // The surface (drawable) on which OpenGL draws:
    oaDrawableHandle _drawSurface;

    // The surface (drawable) from which OpenGL reads:
    oaDrawableHandle _readSurface;

    // The thread's current render context id:
    int _spyContextId;

public:
    // Constructor:
    gsThreadLocalData();
};


#endif //__GSTHREADLOCALDATA_H

