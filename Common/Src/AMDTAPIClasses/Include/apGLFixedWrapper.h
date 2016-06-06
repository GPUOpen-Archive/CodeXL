//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apGLFixedWrapper.h
///
//==================================================================================

//------------------------------ apGLFixedWrapper.h ------------------------------

#ifndef __APGLFIXEDWRAPPER_H
#define __APGLFIXEDWRAPPER_H

// Infra:
#include <AMDTOSAPIWrappers/Include/oaOpenGLIncludes.h>

// Local:
#include <AMDTAPIClasses/Include/apAPIClassesDLLBuild.h>

// GLfixed type conversion constants:
#define AP_FIXED_POINT_SHIFT 16
extern float AP_FIXED_TO_FLOAT_FACTOR;
extern float AP_FLOAT_TO_FIXED_FACTOR;


// ----------------------------------------------------------------------------------
// Class Name:           AP_API apGLFixedWrapper
// General Description:
//   Wrapps GLfixed to allow type conversion operations to / from it.
//
// Author:  AMD Developer Tools Team
// Creation Date:        8/4/2004
//
// Implementation notes:
//   The memory layout of this class must be identical to GLfixed. This allows casting
//   a pointer to GLfixed array an apGLFixedWrapper array pointer.
// ----------------------------------------------------------------------------------
class AP_API apGLFixedWrapper
{
public:
    apGLFixedWrapper(const GLfixed& fixedNum = 0) { _data = fixedNum; };

    void setFromInt(const GLint intNum) { _data = (intNum << AP_FIXED_POINT_SHIFT); };
    void setFromFloat(const GLfloat floatNum) { _data = (GLfixed)(floatNum * AP_FLOAT_TO_FIXED_FACTOR); };

    GLfixed asFixed() const { return _data; };
    GLint asInt() const { return (_data >> AP_FIXED_POINT_SHIFT); };
    GLfloat asFloat() const { return ((float)_data * AP_FIXED_TO_FLOAT_FACTOR); };

    operator float() const { return ((float)_data * AP_FIXED_TO_FLOAT_FACTOR); };

private:
    GLfixed _data;
};


#endif //__APGLFIXEDWRAPPER_H

