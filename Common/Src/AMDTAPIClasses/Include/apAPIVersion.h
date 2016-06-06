//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apAPIVersion.h
///
//==================================================================================

//------------------------------ apAPIVersion.h ------------------------------

#ifndef __APOPENGLVERSION_H
#define __APOPENGLVERSION_H

// Infra
#include <AMDTBaseTools/Include/gtString.h>

// Local:
#include <AMDTAPIClasses/Include/apAPIClassesDLLBuild.h>

enum apAPIVersion
{
    AP_GL_VERSION_1_0,
    AP_GL_VERSION_1_1,
    AP_GL_VERSION_1_2,
    AP_GL_VERSION_1_3,
    AP_GL_VERSION_1_4,
    AP_GL_VERSION_1_5,
    AP_GL_VERSION_2_0,
    AP_GL_VERSION_2_1,
    AP_GL_VERSION_3_0,
    AP_GL_VERSION_3_1,
    AP_GL_VERSION_3_2,
    AP_GL_VERSION_3_3,
    AP_GL_VERSION_4_0,
    AP_GL_VERSION_4_1,
    AP_GL_VERSION_4_2,
    AP_GL_VERSION_4_3,
    AP_GL_VERSION_NONE,
    AP_CL_VERSION_1_0,
    AP_CL_VERSION_1_1,
    AP_CL_VERSION_1_2,
    AP_CL_VERSION_2_0,
    AP_CL_VERSION_NONE
};

enum apGLSLVersion
{
    AP_GLSL_VERSION_1_0,
    AP_GLSL_VERSION_1_1,
    AP_GLSL_VERSION_1_2,
    AP_GLSL_VERSION_1_3,
    AP_GLSL_VERSION_1_4,
    AP_GLSL_VERSION_NONE
};


AP_API bool apAPIVersionToString(apAPIVersion version, gtString& versionAsStr);
AP_API bool apGLSLVersionToString(apGLSLVersion version, gtString& versionAsStr);
AP_API bool apStringToGLSLVersion(const gtString& versionAsStr, apGLSLVersion& version);
AP_API apAPIVersion apOpenGLVersionFromInts(int oglMajorVersion, int oglMinorVersion = 0);
AP_API void apOpenGLVersionToInts(apAPIVersion oglVersion, int& oglMajorVersion, int& oglMinorVersion);

#endif //__APOPENGLVERSION_H

