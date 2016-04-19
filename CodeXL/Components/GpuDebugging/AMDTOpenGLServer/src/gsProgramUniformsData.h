//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gsProgramUniformsData.h
///
//==================================================================================

//------------------------------ gsProgramUniformsData.h ------------------------------

#ifndef __GSPROGRAMUNIFORMSDATA
#define __GSPROGRAMUNIFORMSDATA

// OpenGL:
#include <AMDTOSAPIWrappers/Include/oaOpenGLIncludes.h>

// Infra:
#include <AMDTBaseTools/Include/gtMap.h>
#include <AMDTAPIClasses/Include/apGLItemsCollection.h>
#include <AMDTAPIClasses/Include/apGLProgram.h>


// ----------------------------------------------------------------------------------
// Class Name:           gsProgramUniformsData
// General Description:
//   A class holding a program uniforms data.
//
// Author:               Yaki Tebeka
// Creation Date:        14/6/2005
// ----------------------------------------------------------------------------------
class gsProgramUniformsData
{
public:
    gsProgramUniformsData(GLuint programName, apGLShadingObjectType programType);
    ~gsProgramUniformsData();

    // Query data functions:
    GLuint programName() const { return _programName; };
    apGLShadingObjectType programType() const { return _programType; };
    bool isProgramLinkedSuccessfully() const { return _isProgramLinkedSuccessfully; };
    const apGLItemsCollection& programActiveUniforms() const { return _programActiveUniforms; };
    apGLItemsCollection& programActiveUniforms() { return _programActiveUniforms; };
    GLint getUniformLocation(const gtString& uniformName) const;
    int uniformIndexFromLocation(GLint uniformLocation);

    // Set data functions:
    void setProgramLinkStatus(bool isLinkedSuccessfully) { _isProgramLinkedSuccessfully = isLinkedSuccessfully; };
    void clearUniformsData();
    void setUniformLocation(const gtString& uniformName, GLint uniformLocation, int vectorIndex);
    void addUBOAttachmentPoint(GLuint index, GLuint buffer);
    bool getUBOBuffer(GLuint index, GLuint& buffer);

private:
    // Do not allow the use of my default constructor:
    gsProgramUniformsData();

private:
    // The program name and type:
    GLuint _programName;
    apGLShadingObjectType _programType;

    // Contains true iff the program is successfully linked:
    bool _isProgramLinkedSuccessfully;

    // Contains the program active uniforms:
    apGLItemsCollection _programActiveUniforms;

    // Contains the uniforms locations:
    gtMap<gtString, GLint> _programActiveUniformLocations;

    // Contains the index of a uniform from its location:
    gtMap<GLint, int> _programActiveUniformLocationToIndex;

    // Contain uniform buffer object buffer mappings:
    // first - buffer attachment index, second - buffer name:
    gtMap<GLuint, GLuint> _programActiveUniformBufferObjectAttachments;
};

#endif  // __GSPROGRAMUNIFORMSDATA
