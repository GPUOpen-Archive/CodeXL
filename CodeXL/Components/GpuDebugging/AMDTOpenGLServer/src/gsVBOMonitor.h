//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gsVBOMonitor.h
///
//==================================================================================

//------------------------------ gsVBOMonitor.h ------------------------------

#ifndef __GSVBOMONITOR
#define __GSVBOMONITOR

// OpenGL:
#include <AMDTOSAPIWrappers/Include/oaOpenGLIncludes.h>

// Infra:
#include <AMDTAPIClasses/Include/apGLVBO.h>

// ----------------------------------------------------------------------------------
// Class Name:           gsVBOMonitor
//
// General Description:
//   Monitors VBO Objects allocated in a given context.
// Author:               Sigal Algranaty
// Creation Date:        23/9/2008
// ----------------------------------------------------------------------------------
class gsVBOMonitor
{
public:
    gsVBOMonitor(int spyContextId);
    ~gsVBOMonitor();

public:
    // On Event functions:
    void onFirstTimeContextMadeCurrent();
    void onVertexBufferObjectGeneration(GLsizei amountOfGeneratedVertexBuffers, GLuint* vertexBufferNames);
    void onVertexBufferObjectDeletion(GLsizei amountOfDeletedVertexBuffers, const GLuint* vertexBufferNames);
    void onVertexBufferObjectTargetBind(GLenum target, GLuint vboName);
    void addTargetToBufferObject(GLenum target, GLuint vboName, bool bind = false);
    void onVertexBufferObjectDirectAccessDataSet(GLuint buffer, GLsizeiptr sizeOfData);
    void onVertexBufferObjectDataSet(GLenum target, GLsizeiptr sizeOfData);
    void onVertexBufferObjectDirectAccessSubDataSet(GLuint buffer);
    void onVertexBufferObjectSubDataSet(GLenum target);

    bool getVBOName(int vboIndex, GLuint& vboName) const;
    GLenum getVBOLatestAttachment(GLuint vboName) const;
    void getAllCurrentVBOAttachments(GLuint vboName, gtVector<GLenum>& vboAttachments) const;
    GLuint getAttachedVBOName(GLenum vboAttachment) const;

    bool updateVBORawData(apGLVBO* pVBO, oaTexelDataFormat bufferDataFormat);

    // VBO display properties:
    bool setVBODisplayProperties(GLuint vboName, oaTexelDataFormat displayFormat, int offset, GLsizei stride);

    // VBO data:
    apGLVBO* getVBODetails(GLuint vboName) const ;
    int amountOfVBOs() const {return (int)_vbos.size();};

    // Memory:
    bool calculateBuffersMemorySize(gtUInt64& buffersMemorySize) const ;

private:
    // Do not allow use of the = operator for this class. Use reference or pointer transferral instead
    gsVBOMonitor& operator=(const gsVBOMonitor& otherMonitor);
    gsVBOMonitor(const gsVBOMonitor& otherMonitor);

    // VBO actions:
    bool removeVBO(GLuint vboName);
    apGLVBO* addNewVBO(GLuint vboName);

    size_t getVBOIndex(GLuint vboName) const;
    void generateVBOFilePath(GLuint vboName, osFilePath& bufferFilePath) const;
    apGLVBO* getBoundVBO(GLenum target);

    // Bind VBO for reading:
    bool bindVBO(GLuint vboName, GLenum target);

    int _spyContextId;

    // Hold the VBOs that reside in this render context:
    gtPtrVector<apGLVBO*> _vbos;

    // Maps VBO OpenGL name to the VBO vector index:
    gtMap<GLuint, int> _vboOpenGLNameToIndex;

    // Map VBO names to bound targets:
    gtMap<GLuint, GLenum> _vboNameToTargetMap;

    GLuint m_bindArrayBufferVBOName;            // GL_ARRAY_BUFFER
    GLuint m_bindAtmoicCounterBufferVBOName;    // GL_ATOMIC_COUNTER_BUFFER
    GLuint m_bindCopyReadBufferVBOName;         // GL_COPY_READ_BUFFER
    GLuint m_bindCopyWriteBufferVBOName;        // GL_COPY_WRITE_BUFFER
    GLuint m_bindDispatchIndirectBufferVBOName; // GL_DISPATCH_INDIRECT_BUFFER
    GLuint m_bindDrawIndirectBufferVBOName;     // GL_DRAW_INDIRECT_BUFFER
    GLuint m_bindElementArrayBufferVBOName;     // GL_ELEMENT_ARRAY_BUFFER
    GLuint m_bindPixelPackBuffer;               // GL_PIXEL_PACK_BUFFER
    GLuint m_bindPixelUnPackBuffer;             // GL_PIXEL_UNPACK_BUFFER
    GLuint m_bindQueryBufferVBOName;            // GL_QUERY_BUFFER
    GLuint m_bindShaderStorageBufferVBOName;    // GL_SHADER_STORAGE_BUFFER
    GLuint m_bindTextureBufferName;             // GL_TEXTURE_BUFFER
    GLuint m_bindTransformFeedbackBufferVBOName;// GL_TRANSFORM_FEEDBACK_BUFFER
    GLuint m_bindUniformBufferName;             // GL_UNIFORM_BUFFER
    GLuint m_bindUniformBufferEXTName;          // GL_UNIFORM_BUFFER_EXT

    // Support unexpected targets:
    gtMap<GLenum, GLuint> m_vboTargetToAttachedNameMap;

    // Note that GL_UNIFORM_BUFFER and GL_UNIFORM_BUFFER_EXT are different enum values,
    // so theoretically these could be two different attachment points.

#if !((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT))

    PFNGLBINDBUFFERPROC _glBindBuffer;
    PFNGLGETBUFFERSUBDATAPROC _glGetBufferSubData;

#endif

};


#endif  // __GSDISPLAYLISTMONITOR
