//=============================================================
// Copyright (c) 2015 Advanced Micro Devices, Inc.
/// \author GPU Developer Tools
/// \brief Hook support fucntions for ALVR
//=============================================================

#ifndef ALVR_INTERCEPTION_H
#define ALVR_INTERCEPTION_H

#include "../Common/LiquidVRSupport.h"

#ifdef LIQUID_VR_SUPPORT

    #include <Interceptor.h>
    #include "LiquidVR.h"

    extern void HookLiquidVR();
    extern void UnhookLiquidVR();

#endif // LIQUID_VR_SUPPORT

#endif //ALVR_INTERCEPTION_H


