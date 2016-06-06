//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apParameters.h
///
//==================================================================================

//------------------------------ apParameters.h ------------------------------

#ifndef __APPARAMETERS
#define __APPARAMETERS

#include <AMDTAPIClasses/Include/apBasicParameters.h>
#include <AMDTAPIClasses/Include/apOpenGLParameters.h>
#include <AMDTAPIClasses/Include/apOSRelatedParameters.h>

// OpenCL (ES) does not yet exist on the iPhone:
#ifndef _GR_IPHONE_BUILD
    #include <AMDTAPIClasses/Include/apOpenCLParameters.h>
#endif


#endif  // __APPARAMETERS
