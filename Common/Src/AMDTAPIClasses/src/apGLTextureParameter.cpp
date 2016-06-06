//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apGLTextureParameter.cpp
///
//==================================================================================

//------------------------------ apGLTextureParameter.cpp ------------------------------

// Local:
#include <AMDTAPIClasses/Include/apGLTextureParameter.h>


// ---------------------------------------------------------------------------
// Name:        apGLTextureParameter::apGLTextureParameter
// Description: Constructor.
// Author:  AMD Developer Tools Team
// Date:        4/11/2008
// ---------------------------------------------------------------------------
apGLTextureParameter::apGLTextureParameter()
    : _parameterName(GL_NONE), _isUpdatedFromHardware(false)
{
}


// ---------------------------------------------------------------------------
// Name:        apGLTextureParameter::apGLTextureParameter
// Description: Constructor
// Arguments: parameterName - The parameters name.
//            aptrParameterValue - The parameters value.
// Author:  AMD Developer Tools Team
// Date:        4/11/2008
// ---------------------------------------------------------------------------
apGLTextureParameter::apGLTextureParameter(GLenum parameterName, gtAutoPtr<apParameter>& aptrParameterValue)
    : _parameterName(parameterName), _aptrParameterValue(aptrParameterValue), _isUpdatedFromHardware(false)
{
}


// ---------------------------------------------------------------------------
// Name:        apGLTextureParameter::apGLTextureParameter
// Description: Copy constructor
// Arguments: other - The other parameters class from which I am copied.
// Author:  AMD Developer Tools Team
// Date:        4/11/2008
// ---------------------------------------------------------------------------
apGLTextureParameter::apGLTextureParameter(const apGLTextureParameter& other)
{
    apGLTextureParameter::operator=(other);
}


// ---------------------------------------------------------------------------
// Name:        operator=
// Description: Assignment operator.
// Arguments: other - The other object from which I am copied.
// Return Val: apGLTextureParameter& - reference to self.
// Author:  AMD Developer Tools Team
// Date:        4/11/2008
// ---------------------------------------------------------------------------
apGLTextureParameter& apGLTextureParameter::operator=(const apGLTextureParameter& other)
{
    // Copy class members from other:
    _parameterName = other._parameterName;
    _aptrParameterValue = (apParameter*)((other._aptrParameterValue)->clone());
    _isUpdatedFromHardware = other._isUpdatedFromHardware;

    // Return a reference to self:
    return *this;
}

