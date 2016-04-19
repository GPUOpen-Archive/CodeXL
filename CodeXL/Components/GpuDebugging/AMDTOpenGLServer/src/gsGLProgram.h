//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gsGLProgram.h
///
//==================================================================================

//------------------------------ gsGLProgram.h ------------------------------

#ifndef __GSGLPROGRAM
#define __GSGLPROGRAM

// Infra:
#include <AMDTAPIClasses/Include/apGLProgram.h>


// ----------------------------------------------------------------------------------
// Class Name:           gsGLProgram : public apGLProgram
// General Description:
//   Represents a shading languge program.
//   Inherits apGLProgram and data to id data needed by the spy.
//
// Author:               Yaki Tebeka
// Creation Date:        12/4/2005
// ----------------------------------------------------------------------------------
class gsGLProgram : public apGLProgram
{
public:
    gsGLProgram(GLuint programName = 0, apGLShadingObjectType programType = AP_GL_2_0_SHADING_OBJECT);

    // Overrides apGLProgram:
    virtual bool onShaderObjectAttached(const apGLShaderObject& shaderObj);
    virtual bool onShaderObjectDetached(const apGLShaderObject& shaderObj);

    // Self functions:
    void onStubGeometryShaderAttached() { _isStubGeometryShaderAttached = true; };
    void onStubGeometryShaderDetached() { _isStubGeometryShaderAttached = false; };

    void onStubFragmentShaderAttached() { _isStubFragmentShaderAttached = true; };
    void onStubFragmentShaderDetached() { _isStubFragmentShaderAttached = false; };

    int amountOfAttachedGeometryShaders() const { return _amountOfAttachedGeometryShaders; };
    bool isStubGeometryShaderAttached() const { return _isStubGeometryShaderAttached; };

    int amountOfAttachedFragmentShaders() const { return _amountOfAttachedFragmentShaders; };
    bool isStubFragmentShaderAttached() const { return _isStubFragmentShaderAttached; };

    void setProgramRestoredFromStubFS(bool wasProgramRestored) { _wasProgramRestoredFromStubFS = wasProgramRestored; };
    bool wasProgramRestoredFromStubFS() const { return _wasProgramRestoredFromStubFS; };

    bool wasUsedInCurrentFrame() const { return _wasUsedInCurrentFrame; };
    void setWasUsedInCurrentFrame(bool wasProgramUsedInCurrentFrame) { _wasUsedInCurrentFrame = wasProgramUsedInCurrentFrame; };

private:
    // The amount of geometry shaders;
    int _amountOfAttachedGeometryShaders;

    // Contains true iff the stub geometry shader is attached to this program:
    bool _isStubGeometryShaderAttached;

    // The amount of fragment shaders;
    int _amountOfAttachedFragmentShaders;

    // Contains true iff the stub fragment shader is attached to this program:
    bool _isStubFragmentShaderAttached;

    // Contains true iff the program was restored from forced stub fragment shader mode:
    bool _wasProgramRestoredFromStubFS;

    // Contains true iff this program was used in the current frame:
    bool _wasUsedInCurrentFrame;
};


#endif  // __GSGLPROGRAM
