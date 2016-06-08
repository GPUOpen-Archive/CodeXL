//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apGLTextureParameter.h
///
//==================================================================================

//------------------------------ apGLTextureParametereter.h ------------------------------

#ifndef __apGLTextureParameterETER_H
#define __apGLTextureParameterETER_H

// Infra:
#include <AMDTBaseTools/Include/gtAutoPtr.h>
#include <AMDTBaseTools/Include/AMDTDefinitions.h>
#include <AMDTOSAPIWrappers/Include/oaOpenGLIncludes.h>

// Local:
#include <AMDTAPIClasses/Include/apBasicParameters.h>


// ----------------------------------------------------------------------------------
// Struct Name:          apGLTextureParameter
// General Description:
//   Represents a single Texture parameter.
//
// Author:  AMD Developer Tools Team
// Creation Date:        4/11/2008
// ----------------------------------------------------------------------------------
struct AP_API apGLTextureParameter
{
    // The texture parameter name:
    GLenum _parameterName;

    // The texture parameter value:
    gtAutoPtr<apParameter> _aptrParameterValue;

    // Contains true if the parameter's value was updated from the real hardware value:
    bool _isUpdatedFromHardware;

public:
    apGLTextureParameter();
    apGLTextureParameter(GLenum parameterName, gtAutoPtr<apParameter>& aptrParameterValue);
    apGLTextureParameter(const apGLTextureParameter& other);
    apGLTextureParameter& operator=(const apGLTextureParameter& other);
};


#endif //__apGLTextureParameterETER_H

