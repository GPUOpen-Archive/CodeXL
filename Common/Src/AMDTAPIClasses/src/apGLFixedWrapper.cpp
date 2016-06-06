//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apGLFixedWrapper.cpp
///
//==================================================================================

//------------------------------ apGLFixedWrapper.cpp ------------------------------

// Local:
#include <AMDTAPIClasses/Include/apGLFixedWrapper.h>

// GLfixed type conversion constants:
float AP_FIXED_TO_FLOAT_FACTOR = 1.0f / 65536.0f;
float AP_FLOAT_TO_FIXED_FACTOR = 65536.0f;
