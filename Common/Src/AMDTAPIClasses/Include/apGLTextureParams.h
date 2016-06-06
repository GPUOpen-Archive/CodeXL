//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apGLTextureParams.h
///
//==================================================================================

//------------------------------ apGLTextureParams.h ------------------------------

#ifndef __APGLTEXTUREPARAMS
#define __APGLTEXTUREPARAMS

// Forward decelerations:
class apParameter;
class apOpenGLParameter;
class apNotAvailableParameter;

// Infra:
#include <AMDTBaseTools/Include/gtPtrVector.h>
#include <AMDTOSWrappers/Include/osFilePath.h>
#include <AMDTOSWrappers/Include/osOSDefinitions.h>

// Local:
#include <AMDTAPIClasses/Include/apAPIClassesDLLBuild.h>
#include <AMDTAPIClasses/Include/apAllocatedObject.h>
#include <AMDTAPIClasses/Include/apTextureType.h>
#include <AMDTAPIClasses/Include/apFileType.h>
#include <AMDTAPIClasses/Include/apGLTextureParameter.h>


// ----------------------------------------------------------------------------------
// Class Name:           AP_API apGLTextureParams : public osTransferableObject
// General Description:
//   Represents an OpenGL texture.parameters (can be either texture level parameters
//   or texture parameters)
// Author:  AMD Developer Tools Team
// Creation Date:        29/10/2008
// ----------------------------------------------------------------------------------
class AP_API apGLTextureParams : public osTransferableObject
{
public:
    // Self functions:
    apGLTextureParams();
    apGLTextureParams(const apGLTextureParams& other);
    virtual ~apGLTextureParams();
    apGLTextureParams& operator=(const apGLTextureParams& other);

    // Default parameters initialization:
    void addDefaultGLTextureParameters();
    void addDefaultGLTextureLevelParameters(bool isNVIDIAPlatform = false);
    void addDefaultGLESTextureParameters();
    void addDefaultGLESTextureLevelParameters();

    // Clear:
    void clearAllParameters();

    // NV Texture shader params:
    void addNVTextureShaderParameters();

    // Multisample texture extension parameters:
    void addMultisampleTextureShaderParameters();

    // Parameters get functions:
    int amountOfTextureParameters() const { return (int)(_textureParameters.size()); };
    const apParameter* getTextureParameterValue(int parameterIndex) const;
    bool getTextureBoolenParameterValue(GLenum parameterName, GLboolean& valueAsBool) const;
    bool getTextureEnumParameterValue(GLenum parameterName, GLenum& valueAsEnum) const;
    bool getTextureIntParameterValue(GLenum parameterName, GLint& valueAsInt)const;
    bool getTextureFloatParameterValue(GLenum parameterName, GLfloat& valueAsFloat) const;
    GLenum getTextureParameterName(int parameterIndex) const;

    // Parameters set functions:
    void setTextureParameterValueFromFloat(GLenum parameterName, GLfloat* pParameterValue);
    void setTextureParameterIntValue(GLenum parameterName, const GLint* pParameterValue);
    void setTextureParameterUIntValue(GLenum parameterName, const GLuint* pParameterValue);
    void setTextureParameterFloatValue(GLenum parameterName, const GLfloat* pParameterValue);
    void setTextureParameterUpdateStatus(int parameterIndex, bool isUpdated);

    // Overrides osTransferableObject:
    virtual osTransferableObjectType type() const;
    virtual bool writeSelfIntoChannel(osChannel& ipcChannel) const;
    virtual bool readSelfFromChannel(osChannel& ipcChannel);

private:

    // Private functions:
    int getTextureParameterIndex(GLenum parameterName) const;
    void deleteAllParameters();
    apParameter* createParameterObj(GLenum parameterName, osTransferableObjectType parameterValueType, void* pParameterValue);
    apOpenGLParameter* createSingleItemParameterObj(osTransferableObjectType parameterType, osTransferableObjectType parameterValueType, void* pParameterValue);
    void setTextureParameterValueFromFloat(GLenum parameterName, void* pParameterValue, osTransferableObjectType paramType = OS_TOBJ_ID_NOT_AVAILABLE_PARAMETER);
    void getParameterType(GLenum paramName, osTransferableObjectType& parameterType, int& amountOfParameterItems);

private:
    // The texture parameters:
    gtPtrVector<apGLTextureParameter*> _textureParameters;

    // Static instance for unavailable parameters:
    static apNotAvailableParameter _stat_NotAvailableParameter;
};


#endif  // __APGLTEXTUREPARAMS

