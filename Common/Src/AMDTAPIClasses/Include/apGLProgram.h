//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apGLProgram.h
///
//==================================================================================

//------------------------------ apGLProgram.h ------------------------------

#ifndef __APGLPROGRAM
#define __APGLPROGRAM

// Predeclerations:
class apGLShaderObject;

// OpenGL:
#include <AMDTOSAPIWrappers/Include/oaOpenGLIncludes.h>

// Infra:
#include <AMDTBaseTools/Include/gtList.h>
#include <AMDTBaseTools/Include/gtString.h>

// Local:
#include <AMDTAPIClasses/Include/apAllocatedObject.h>
#include <AMDTAPIClasses/Include/apAPIClassesDLLBuild.h>
#include <AMDTAPIClasses/Include/apGLShadingObjectType.h>


// ----------------------------------------------------------------------------------
// Class Name:           AP_API apGLProgram : public osTransferableObject
// General Description:
//   Represents an OpenGL shading language program.
//
// Author:  AMD Developer Tools Team
// Creation Date:        04/04/2005
// ----------------------------------------------------------------------------------
class AP_API apGLProgram : public apAllocatedObject
{
public:
    // Self functions:
    apGLProgram(GLuint programName = 0, apGLShadingObjectType programType = AP_GL_2_0_SHADING_OBJECT);
    apGLProgram(const apGLProgram& other);
    virtual ~apGLProgram();
    apGLProgram& operator=(const apGLProgram& other);

    apGLShadingObjectType programType() const { return _programType; };
    GLuint programName() const { return _programName; };
    bool isProgramLinkedSuccesfully() const { return _isProgramLinkedSuccessfully; };
    const gtString& programLinkLog() const { return _programLinkLog; };
    bool isMarkedForDeletion() const { return _isMarkedForDeletion; };
    bool wasUsedInLastFrame() const { return _wasUsedInLastFrame; };
    GLint geometryInputType() const { return _geometryInputType; };
    GLint geometryOutputType() const { return _geometryOutputType; };
    GLint maxGeometryOutputVertices() const { return _maxGeometryOutputVertices; };

    bool isShaderObjectAttached(GLuint shaderName) const;
    const gtList<GLuint>& shaderObjects() const { return _shaderObjects; };

    // On event functions:
    virtual bool onShaderObjectAttached(const apGLShaderObject& shaderObj);
    virtual bool onShaderObjectDetached(const apGLShaderObject& shaderObj);
    virtual void onProgramLink(bool wasLinkSuccessful, const gtString& linkLog);
    virtual void onProgramMarkedForDeletion();
    virtual bool onProgramParameteri(const GLenum& pname, const GLint& value);

    void setWasUsedInLastFrame(bool wasProgramUsedInLastFrame) { _wasUsedInLastFrame = wasProgramUsedInLastFrame; };

    // Overrides osTransferableObject:
    virtual osTransferableObjectType type() const;
    virtual bool writeSelfIntoChannel(osChannel& ipcChannel) const;
    virtual bool readSelfFromChannel(osChannel& ipcChannel);

private:
    void clearVectors();
    void copyVectors(const apGLProgram& other);

private:
    // The OpenGL program name:
    GLuint _programName;

    // The program type:
    apGLShadingObjectType _programType;

    // The Program Shaders objects OpenGL names:
    gtList<GLuint> _shaderObjects;

    // Contains true iff the program was linked successfully:
    bool _isProgramLinkedSuccessfully;

    // Contains the program link log:
    gtString _programLinkLog;

    // Contains true iff this program was marked for deletion:
    bool _isMarkedForDeletion;

    // Contains true iff this program was used in the last frame:
    bool _wasUsedInLastFrame;

    // Geometry Parameters - default values are specified in the geometry_shader4 spec in section 2.15.5
    GLint _geometryInputType;           // Default value is GL_TRIANGLES
    GLint _geometryOutputType;          // Default value is GL_TRIANGLE_STRIP
    GLint _maxGeometryOutputVertices;   // Default value is 0
};


#endif  // __APGLPROGRAM
