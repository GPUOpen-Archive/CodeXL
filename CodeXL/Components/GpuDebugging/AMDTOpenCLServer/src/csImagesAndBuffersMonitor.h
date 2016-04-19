//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file csImagesAndBuffersMonitor.h
///
//==================================================================================

//------------------------------ csImagesAndBuffersMonitor.h ------------------------------

#ifndef __CSIMAGESANDBUFFERSMONITOR_H
#define __CSIMAGESANDBUFFERSMONITOR_H

// Infra:
#include <AMDTBaseTools/Include/gtPtrVector.h>
#include <AMDTOSWrappers/Include/osCriticalSection.h>
#include <AMDTAPIClasses/Include/apCLBuffer.h>
#include <AMDTAPIClasses/Include/apCLSubBuffer.h>
#include <AMDTAPIClasses/Include/apCLImage.h>
#include <AMDTAPIClasses/Include/apCLPipe.h>
#include <AMDTAPIClasses/Include/apTextureType.h>


// ----------------------------------------------------------------------------------
// Class Name:           csImagesAndBuffersMonitor
// General Description:
//   Monitors OpenCL buffers created within an OpenCL context.
//
// Author:               Yaki Tebeka
// Creation Date:        18/11/2009
// ----------------------------------------------------------------------------------
class csImagesAndBuffersMonitor
{
public:
    csImagesAndBuffersMonitor();
    virtual ~csImagesAndBuffersMonitor();

    // Events:
    void onBufferCreation(cl_mem bufferMemoryHandle, cl_mem_flags flags, size_t size);
    void onSubBufferCreation(cl_mem subBufferMemoryHandle, cl_mem bufferMemoryHandle, cl_mem_flags flags, cl_buffer_create_type buffer_create_type, const void* buffer_create_info);
    void onBufferCreationFromGLVBO(cl_mem bufferMemoryHandle, cl_mem_flags flags, GLuint bufobj);
    void onBufferCreationFromDirectX(cl_mem bufferMemoryHandle, cl_mem_flags flags);
    void onPipeCreation(cl_mem pipeMemoryHandle, cl_mem_flags flags, cl_uint packetSize, cl_uint maxPackets);
    void onImageCreation(cl_mem textureHandle, const cl_image_format* pCLImageFormat, cl_mem_flags flags, apTextureType textureType, gtSize_t width, gtSize_t height, gtSize_t depth);
    void onImageCreation(cl_mem textureHandle, const cl_image_format* pCLImageFormat, cl_mem_flags flags, const cl_image_desc* pImageDesc);
    void onImageCreationFromGLTexture(cl_mem textureHandle, cl_mem_flags flags, GLenum target, GLint miplevel, GLuint texture);
    void onImageCreationFromGL2DTexture(cl_mem textureHandle, cl_mem_flags flags, apTextureType textureType, GLenum target, GLint miplevel, GLuint texture);
    void onTextureCreationFromGL3DTexture(cl_mem textureHandle, cl_mem_flags flags, apTextureType textureType, GLenum target, GLint miplevel, GLuint texture);
    void onTextureCreationFromGLRenderbuffer(cl_mem textureHandle, cl_mem_flags flags, apTextureType textureType, GLuint renderbuffer);
    void onImageCreationFromDirectX(cl_mem textureHandle, cl_mem_flags flags, apTextureType textureType);
    void onMemObjectMarkedForDeletion(cl_mem memobj);

    // Reference count checking:
    void checkForReleasedObjects();

    // Update functions:
    bool updateBufferRawData(int bufferId);
    bool updateSubBufferRawData(int subBufferId);
    bool updateTextureRawData(int textureId);
    bool updateContextDataSnapshot();

    // Spy context id:
    void setSpyContextId(int spyContextId) {_spyContextId = spyContextId;};

    // OpenCL Version:
    void setContextOpenCLVersion(int majorVersion, int minorVersion);

    // General mem object details:
    const apCLMemObject* getMemObjectDetails(oaCLMemHandle memObjHandle) const;
    apCLMemObject* getMemObjectDetails(oaCLMemHandle memObjHandle);
    apCLMemObject* getMemObjectDetails(oaCLMemHandle memObjHandle, int& memoryObjectIndex) const ;

    // Buffers details:
    int amountOfBuffers() const;
    const apCLBuffer* bufferDetails(int bufferId) const;
    apCLBuffer* bufferDetails(int bufferId);
    int bufferObjectMonitorIndex(int bufferName) const;

    // Sub Buffers details:
    const apCLSubBuffer* subBufferDetails(int subBufferName) const;
    apCLSubBuffer* subBufferDetails(int subBufferName);

    // Texture details:
    int amountOfImages() const;
    const apCLImage* imageDetails(int textureId) const;
    apCLImage* imageDetails(int textureId);
    int imageObjectMonitorIndex(int textureName) const;

    // Pipes details:
    int amountOfPipes() const;
    const apCLPipe* pipeDetails(int pipeId) const;
    apCLPipe* pipeDetails(int pipeId);
    int pipeObjectMonitorIndex(int pipeName) const;

    // Memory:
    bool calculateBuffersMemorySize(gtUInt64& buffersMemorySize) const;
    bool calculateImagesMemorySize(gtUInt64& texturesMemorySize) const;
    bool calculatePipesMemorySize(gtUInt64& pipesMemorySize) const;

    // OpenGL interoperability:
    bool handleImageShareWithGLTexture(apCLImage* pTextureObject, int clTextureIndex, GLenum target, GLint miplevel, GLuint texture);
    bool handleImageShareWithGLRenderBuffer(apCLImage* pTextureObject, int clTextureIndex, GLuint renderbuffer);
    bool handleBufferShareWithGLVBO(apCLBuffer* pBufferObject, int clBufferIndex, GLuint glVBOName);

    // Memory object deletion callback:
    static void CL_CALLBACK onMemoryObjectDeletion(cl_mem memobj, void* user_data);
    bool onMemoryObjectDeletion(cl_mem memobj);
    void onMemObjectDestructorCallbackSet(oaCLMemHandle hMem, osProcedureAddress64 pfnNotify, osProcedureAddress64 userData);

private:
    // Raw data file path:
    void generateRawDataFilePath(int bufferId, osFilePath& bufferFilePath, bool isBuffer) const;

    // Command Queue:
    bool initializeCommandQueue();
    bool destroyCommandQueue();

private:
    // Spy context id:
    int _spyContextId;

    // Contains true iff our context supports clSetMemObjectDestructorCallback:
    bool _isMemObjectDestructorCallbackSupported;

    // Prevents objects from being accesses while other objects are deleted:
    osCriticalSection _memObjectDeletionCS;

    // A vector containing buffer monitors:
    gtPtrVector<apCLBuffer*> _bufferMonitors;

    // A vector containing buffer monitors:
    gtPtrVector<apCLSubBuffer*> _subBufferMonitors;

    // The first index free for buffer name:
    int _nextFreeBufferName;

    // The first index free for sub buffer name:
    int _nextFreeSubBufferName;

    // Maps buffer name to the buffer vector index:
    gtMap<int, int> _bufferNameToIndexMap;

    // A vector containing texture monitors:
    gtPtrVector<apCLImage*> _imagesMonitors;

    // The first index free for texture name:
    int _nextFreeImageName;

    // Maps texture name to the texture vector index:
    gtMap<int, int> _imageNameToIndexMap;

    // A vector containing pipe monitors:
    gtPtrVector<apCLPipe*> m_pipeMonitors;

    // The first index free for pipe name:
    int m_nextFreePipeName;

    // Maps pipe name to the pipe vector index:
    gtMap<int, int> m_pipeNameToIndexMap;

    // Get a command queue handle that is used for buffer reading:
    oaCLCommandQueueHandle _commandQueue;
};


#endif //__CSIMAGESANDBUFFERSMONITOR_H

