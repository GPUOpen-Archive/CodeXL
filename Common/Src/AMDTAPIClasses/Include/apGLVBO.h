//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apGLVBO.h
///
//==================================================================================

//------------------------------ apGLVBO.h ------------------------------

#ifndef __APGLVBO
#define __APGLVBO

// OpenGL
#include <AMDTOSAPIWrappers/Include/oaOpenGLIncludes.h>

// Infra:
#include <AMDTBaseTools/Include/gtVector.h>
#include <AMDTBaseTools/Include/gtList.h>
#include <AMDTOSWrappers/Include/osFilePath.h>
#include <AMDTOSWrappers/Include/osTransferableObject.h>
#include <AMDTOSAPIWrappers/Include/oaTexelDataFormat.h>

// Local:
#include <AMDTAPIClasses/Include/apAPIClassesDLLBuild.h>
#include <AMDTAPIClasses/Include/apAllocatedObject.h>

// ----------------------------------------------------------------------------------
// Class Name:           AP_API apGLVBO : public apAllocatedObject
//
// General Description:
//   Represents an OpenGL vertex buffer object.
//   See GL_ARB_vertex_buffer_object extension documentation for more details.
//
// Author:  AMD Developer Tools Team
// Creation Date:        22/10/2008
// ----------------------------------------------------------------------------------
class AP_API apGLVBO : public apAllocatedObject
{
public:
    // Self functions:
    apGLVBO();
    apGLVBO(const apGLVBO& other);
    virtual ~apGLVBO();
    apGLVBO& operator=(const apGLVBO& other);

    // Overrides osTransferableObject:
    virtual osTransferableObjectType type() const;
    virtual bool writeSelfIntoChannel(osChannel& ipcChannel) const;
    virtual bool readSelfFromChannel(osChannel& ipcChannel);

    // Buffer targets:
    GLenum lastBufferTarget() const { return m_lastBufferTarget; };
    const gtVector<GLenum> activeBufferTargets() const { return m_activeBufferTargets; };
    const gtVector<GLenum> bufferTargetHistory() const { return m_bufferTargetHistory; };
    void onBindToTarget(GLenum target, bool* o_isRedundant = nullptr);
    void onUnbindFromTarget(GLenum target);

    // Get & Set the buffer file path that contains the buffer content:
    void setBufferFilePath(const osFilePath& filePath) { _bufferFile = filePath; };
    void getBufferFilePath(osFilePath& filePath) const { filePath = _bufferFile; };

    // Dirty:
    void markAsDirty(bool isDirty) {_isDirty = isDirty;};
    bool isDirty() const {return _isDirty;};

    // Buffer display properties:
    void setBufferDisplayProperties(oaTexelDataFormat displayFormat, int offset, GLsizei stride);
    void getBufferDisplayProperties(oaTexelDataFormat& displayFormat, int& offset, GLsizei& stride) const ;
    void setBufferDisplayFormat(oaTexelDataFormat displayFormat);
    oaTexelDataFormat displayFormat() const {return _displayFormat;};

    void setSize(gtSize_t size);
    void setName(GLuint name);

    gtSize_t size() const;
    GLuint name() const;

    // OpenCL interoperability:
    void shareVBOWithCLBuffer(int openCLBufferIndex, int openCLBufferName, int openCLSpyID) {_openCLBufferIndex = openCLBufferIndex; _openCLBufferName = openCLBufferName; _openCLSpyID = openCLSpyID;} ;
    int openCLBufferIndex() const {return _openCLBufferIndex;};
    int openCLBufferName() const {return _openCLBufferName;};
    int openCLSpyID() const {return _openCLSpyID;};

private:

    // The OpenGL VBO name:
    GLuint _vboName;

    // Buffer targets:
    GLenum m_lastBufferTarget;
    gtVector<GLenum> m_activeBufferTargets;
    gtVector<GLenum> m_bufferTargetHistory;

    // VBO size:
    gtSize_t _size;

    // File path to the buffer data content
    osFilePath _bufferFile;

    // On if the VBO was changed since the last update:
    bool _isDirty;

    // User defined buffer displayed format:
    oaTexelDataFormat _displayFormat;

    // User defined buffer offset:
    int _offset;

    // User defined buffer stride (amount of bytes separating between each buffer chunk):
    GLsizei _stride;

    // OpenCL interoperability:
    int _openCLBufferIndex;
    int _openCLBufferName;
    int _openCLSpyID;

};


#endif  // __APGLVBO
