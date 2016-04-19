//==============================================================================
// Copyright (c) 2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief The header file for detouring the CL entry functions
///        related to the memory copies operations.
//==============================================================================

#include <iostream>
#include "CLProfilerMineCLMemory.h"
#include "CLGPAProfiler.h"
#include "CLUtils.h"
#include "CLFunctionDefs.h"
#include "../Common/SeqIDGenerator.h"
#include "../Common/Logger.h"

using namespace std;
extern CLGPAProfiler g_Profiler;

void AddRWBuffer(cl_context   context,
                 cl_mem       mem,
                 cl_mem_flags flags,
                 size_t       size,
                 void*        pHost = NULL)
{
    assert(NULL != mem);

    // only trace read/write buffers, as those are the only ones that need to be saved/restrored during replay
    if ((flags & CL_MEM_WRITE_ONLY) == 0)
    {
        if ((flags & CL_MEM_READ_ONLY) == 0)
        {
            // buffer is read and write
            g_Profiler.AddBuffer(context, mem, flags, size, pHost);
        }
    }
}

void AddRWBuffer(cl_context   context,
                 cl_mem       mem,
                 cl_mem_flags flags)
{
    size_t nSize;
    cl_int nRet = g_realDispatchTable.GetMemObjectInfo(mem, CL_MEM_SIZE, sizeof(size_t), &nSize, NULL);
    assert(nRet == CL_SUCCESS);

    if (nRet == CL_SUCCESS)
    {
        return AddRWBuffer(context, mem, flags, nSize);
    }
}

void AddRWSubBuffer(cl_mem       parentBuffer,
                    cl_mem       mem,
                    cl_mem_flags flags,
                    size_t       size)
{
    assert(NULL != mem);

    // only trace read/write buffers, as those are the only ones that need to be saved/restrored during replay
    if ((flags & CL_MEM_WRITE_ONLY) == 0)
    {
        if ((flags & CL_MEM_READ_ONLY) == 0)
        {
            // buffer is read and write
            g_Profiler.AddSubBuffer(parentBuffer, mem, flags, size);
        }
    }
}

cl_mem CL_API_CALL
Mine_clCreateBuffer(cl_context context,
                    cl_mem_flags flags,
                    size_t       bufferSize,
                    void*        pHost,
                    cl_int*      pErrorCode)
{
    SeqIDGenerator::Instance()->GenerateID();
    cl_mem mem = g_nextDispatchTable.CreateBuffer(context, flags, bufferSize, pHost, pErrorCode);

    if (NULL != mem)
    {
        AddRWBuffer(context, mem, flags, bufferSize, pHost);
    }

    return mem;
}

cl_mem CL_API_CALL Mine_clCreateSubBuffer(
    cl_mem                    buffer,
    cl_mem_flags              flags,
    cl_buffer_create_type     buffer_create_type,
    const void*               buffer_create_info,
    cl_int*                   errcode_ret)
{
    SeqIDGenerator::Instance()->GenerateID();

    cl_mem mem = g_nextDispatchTable.CreateSubBuffer(
                     buffer,
                     flags,
                     buffer_create_type,
                     buffer_create_info,
                     errcode_ret);

    if (NULL != mem && NULL != buffer_create_info)
    {
        const cl_buffer_region* pBufferRegion = static_cast<const cl_buffer_region*>(buffer_create_info);
        size_t bufferSize = pBufferRegion->size;
        AddRWSubBuffer(buffer, mem, flags, bufferSize);
    }

    return mem;
}
cl_mem CL_API_CALL
Mine_clCreateImage2D(cl_context             context,
                     cl_mem_flags           flags,
                     const cl_image_format* pFormat,
                     size_t                 width,
                     size_t                 height,
                     size_t                 rowPitch,
                     void*                  pHost,
                     cl_int*                pErrorCode)
{
    SeqIDGenerator::Instance()->GenerateID();
    return g_nextDispatchTable.CreateImage2D(context, flags, pFormat, width, height, rowPitch, pHost, pErrorCode);
}

cl_mem CL_API_CALL
Mine_clCreateImage3D(cl_context             context,
                     cl_mem_flags           flags,
                     const cl_image_format* pFormat,
                     size_t                 width,
                     size_t                 height,
                     size_t                 depth,
                     size_t                 rowPitch,
                     size_t                 slicePitch,
                     void*                  pHost,
                     cl_int*                pErrorCode)
{
    SeqIDGenerator::Instance()->GenerateID();
    return g_nextDispatchTable.CreateImage3D(context, flags, pFormat, width, height, depth,
                                             rowPitch, slicePitch, pHost, pErrorCode);
}

cl_int CL_API_CALL
Mine_clEnqueueReadBuffer(cl_command_queue commandQueue,
                         cl_mem           buffer,
                         cl_bool          bBlock,
                         size_t           offset,
                         size_t           bufferSize,
                         void*            pBuffer,
                         cl_uint          uEventWaitList,
                         const cl_event*  pEventWaitList,
                         cl_event*        pEvent)
{
    SeqIDGenerator::Instance()->GenerateID();

    return g_nextDispatchTable.EnqueueReadBuffer(commandQueue,
                                                 buffer,
                                                 bBlock,
                                                 offset,
                                                 bufferSize,
                                                 pBuffer,
                                                 uEventWaitList,
                                                 pEventWaitList,
                                                 pEvent);
}

cl_int CL_API_CALL
Mine_clEnqueueWriteBuffer(cl_command_queue commandQueue,
                          cl_mem           buffer,
                          cl_bool          bBlock,
                          size_t           offset,
                          size_t           bufferSize,
                          const void*      pBuffer,
                          cl_uint          uEventWaitList,
                          const cl_event*  pEventWaitList,
                          cl_event*        pEvent)
{
    SeqIDGenerator::Instance()->GenerateID();
    return g_nextDispatchTable.EnqueueWriteBuffer(commandQueue,
                                                  buffer,
                                                  bBlock,
                                                  offset,
                                                  bufferSize,
                                                  pBuffer,
                                                  uEventWaitList,
                                                  pEventWaitList,
                                                  pEvent);
}

cl_int CL_API_CALL
Mine_clEnqueueReadImage(cl_command_queue commandQueue,
                        cl_mem           image,
                        cl_bool          bBlock,
                        const size_t*    pOrigin,
                        const size_t*    pSize,
                        size_t           rowPitch,
                        size_t           slicePitch,
                        void*            pHost,
                        cl_uint          uEventWaitList,
                        const cl_event*  pEventWaitList,
                        cl_event*        pEvent)
{
    SeqIDGenerator::Instance()->GenerateID();
    return g_nextDispatchTable.EnqueueReadImage(commandQueue,
                                                image,
                                                bBlock,
                                                pOrigin,
                                                pSize,
                                                rowPitch,
                                                slicePitch,
                                                pHost,
                                                uEventWaitList,
                                                pEventWaitList,
                                                pEvent);
}

cl_int CL_API_CALL
Mine_clEnqueueWriteImage(cl_command_queue commandQueue,
                         cl_mem           image,
                         cl_bool          bBlock,
                         const size_t*    pOrigin,
                         const size_t*    pSize,
                         size_t           rowPitch,
                         size_t           slicePitch,
                         const void*      pHost,
                         cl_uint          uEventWaitList,
                         const cl_event*  pEventWaitList,
                         cl_event*        pEvent)
{
    SeqIDGenerator::Instance()->GenerateID();

    return g_nextDispatchTable.EnqueueWriteImage(commandQueue,
                                                 image,
                                                 bBlock,
                                                 pOrigin,
                                                 pSize,
                                                 rowPitch,
                                                 slicePitch,
                                                 pHost,
                                                 uEventWaitList,
                                                 pEventWaitList,
                                                 pEvent);
}

void* CL_API_CALL
Mine_clEnqueueMapBuffer(cl_command_queue commandQueue,
                        cl_mem           buffer,
                        cl_bool          bBlock,
                        cl_map_flags     flags,
                        size_t           offset,
                        size_t           bufferSize,
                        cl_uint          uEventWaitList,
                        const cl_event*  pEventWaitList,
                        cl_event*        pEvent,
                        cl_int*          pErrorCode)
{
    SeqIDGenerator::Instance()->GenerateID();

    return g_nextDispatchTable.EnqueueMapBuffer(commandQueue,
                                                buffer,
                                                bBlock,
                                                flags,
                                                offset,
                                                bufferSize,
                                                uEventWaitList,
                                                pEventWaitList,
                                                pEvent,
                                                pErrorCode);
}

void* CL_API_CALL
Mine_clEnqueueMapImage(cl_command_queue commandQueue,
                       cl_mem           image,
                       cl_bool          bBlock,
                       cl_map_flags     flags,
                       const size_t*    pOrigin,
                       const size_t*    pSize,
                       size_t*          rowPitch,
                       size_t*          slicePitch,
                       cl_uint          uEventWaitList,
                       const cl_event*  pEventWaitList,
                       cl_event*        pEvent,
                       cl_int*          pErrorCode)
{
    SeqIDGenerator::Instance()->GenerateID();

    return g_nextDispatchTable.EnqueueMapImage(commandQueue,
                                               image,
                                               bBlock,
                                               flags,
                                               pOrigin,
                                               pSize,
                                               rowPitch,
                                               slicePitch,
                                               uEventWaitList,
                                               pEventWaitList,
                                               pEvent,
                                               pErrorCode);
}

cl_int CL_API_CALL
Mine_clEnqueueUnmapMemObject(cl_command_queue commandQueue,
                             cl_mem           memObj,
                             void*            pMapped,
                             cl_uint          uEventWaitList,
                             const cl_event*  pEventWaitList,
                             cl_event*        pEvent)
{
    SeqIDGenerator::Instance()->GenerateID();

    return g_nextDispatchTable.EnqueueUnmapMemObject(commandQueue,
                                                     memObj,
                                                     pMapped,
                                                     uEventWaitList,
                                                     pEventWaitList,
                                                     pEvent);
}

cl_int CL_API_CALL
Mine_clEnqueueCopyBuffer(cl_command_queue commandQueue,
                         cl_mem           sourceBuffer,
                         cl_mem           destBuffer,
                         size_t           sourceOffset,
                         size_t           destOffset,
                         size_t           bufferSize,
                         cl_uint          uEventWaitList,
                         const cl_event*  pEventWaitList,
                         cl_event*        pEvent)
{
    SeqIDGenerator::Instance()->GenerateID();

    return g_nextDispatchTable.EnqueueCopyBuffer(commandQueue,
                                                 sourceBuffer,
                                                 destBuffer,
                                                 sourceOffset,
                                                 destOffset,
                                                 bufferSize,
                                                 uEventWaitList,
                                                 pEventWaitList,
                                                 pEvent);
}

cl_int CL_API_CALL
Mine_clEnqueueCopyImage(cl_command_queue commandQueue,
                        cl_mem           sourceImage,
                        cl_mem           destImage,
                        const size_t*    pSourceOrigin,
                        const size_t*    pDestOrigin,
                        const size_t*    pSize,
                        cl_uint          uEventWaitList,
                        const cl_event*  pEventWaitList,
                        cl_event*        pEvent)
{
    SeqIDGenerator::Instance()->GenerateID();

    return g_nextDispatchTable.EnqueueCopyImage(commandQueue,
                                                sourceImage,
                                                destImage,
                                                pSourceOrigin,
                                                pDestOrigin,
                                                pSize,
                                                uEventWaitList,
                                                pEventWaitList,
                                                pEvent);
}

cl_int CL_API_CALL
Mine_clEnqueueCopyImageToBuffer(cl_command_queue commandQueue,
                                cl_mem           sourceImage,
                                cl_mem           destBuffer,
                                const size_t*    pSourceOrigin,
                                const size_t*    pSize,
                                size_t           destOffset,
                                cl_uint          uEventWaitList,
                                const cl_event*  pEventWaitList,
                                cl_event*        pEvent)
{
    SeqIDGenerator::Instance()->GenerateID();

    return g_nextDispatchTable.EnqueueCopyImageToBuffer(commandQueue,
                                                        sourceImage,
                                                        destBuffer,
                                                        pSourceOrigin,
                                                        pSize,
                                                        destOffset,
                                                        uEventWaitList,
                                                        pEventWaitList,
                                                        pEvent);
}

cl_int CL_API_CALL
Mine_clEnqueueCopyBufferToImage(cl_command_queue commandQueue,
                                cl_mem           sourceBuffer,
                                cl_mem           destImage,
                                size_t           sourceOffset,
                                const size_t*    pDestOrigin,
                                const size_t*    pSize,
                                cl_uint          uEventWaitList,
                                const cl_event*  pEventWaitList,
                                cl_event*        pEvent)
{
    SeqIDGenerator::Instance()->GenerateID();
    return g_nextDispatchTable.EnqueueCopyBufferToImage(commandQueue,
                                                        sourceBuffer,
                                                        destImage,
                                                        sourceOffset,
                                                        pDestOrigin,
                                                        pSize,
                                                        uEventWaitList,
                                                        pEventWaitList,
                                                        pEvent);
}

cl_int CL_API_CALL
Mine_clEnqueueReadBufferRect(
    cl_command_queue commandQueue,
    cl_mem           buffer,
    cl_bool          bBlockingRead,
    const size_t*    pBufferOffset,
    const size_t*    pHostOffset,
    const size_t*    pRegion,
    size_t           bufferRowPitch,
    size_t           bufferSlicePitch,
    size_t           hostRowPitch,
    size_t           hostSlicePitch,
    void*            ptr,
    cl_uint          uEventWaitList,
    const cl_event*  pEventWaitList,
    cl_event*        pEvent)
{
    if (NULL == g_nextDispatchTable.EnqueueReadBufferRect)
    {
        // this function is only supported by OpenCL 1.1 so it shouldn't
        // be called by OpenCL 1.0 app
        SpBreak("Mine_clEnqueueReadBufferRect called by OpenCL 1.0 application");
        return CL_SUCCESS;
    }

    SeqIDGenerator::Instance()->GenerateID();

    return g_nextDispatchTable.EnqueueReadBufferRect(commandQueue,
                                                     buffer,
                                                     bBlockingRead,
                                                     pBufferOffset,
                                                     pHostOffset,
                                                     pRegion,
                                                     bufferRowPitch,
                                                     bufferSlicePitch,
                                                     hostRowPitch,
                                                     hostSlicePitch,
                                                     ptr,
                                                     uEventWaitList,
                                                     pEventWaitList,
                                                     pEvent);
}

cl_int CL_API_CALL
Mine_clEnqueueWriteBufferRect(
    cl_command_queue commandQueue,
    cl_mem           buffer,
    cl_bool          bBlockingWrite,
    const size_t*    pBufferOffset,
    const size_t*    pHostOffset,
    const size_t*    pRegion,
    size_t           bufferRowPitch,
    size_t           bufferSlicePitch,
    size_t           hostRowPitch,
    size_t           hostSlicePitch,
    const void*      ptr,
    cl_uint          uEventWaitList,
    const cl_event*  pEventWaitList,
    cl_event*        pEvent)
{
    if (NULL == g_nextDispatchTable.EnqueueWriteBufferRect)
    {
        // this function is only supported by OpenCL 1.1 so it shouldn't
        // be called by OpenCL 1.0 app
        SpBreak("Mine_clEnqueueWriteBufferRect called by OpenCL 1.0 application");
        return CL_SUCCESS;
    }

    SeqIDGenerator::Instance()->GenerateID();
    return g_nextDispatchTable.EnqueueWriteBufferRect(commandQueue,
                                                      buffer,
                                                      bBlockingWrite,
                                                      pBufferOffset,
                                                      pHostOffset,
                                                      pRegion,
                                                      bufferRowPitch,
                                                      bufferSlicePitch,
                                                      hostRowPitch,
                                                      hostSlicePitch,
                                                      ptr,
                                                      uEventWaitList,
                                                      pEventWaitList,
                                                      pEvent);
}

cl_int CL_API_CALL
Mine_clEnqueueCopyBufferRect(
    cl_command_queue commandQueue,
    cl_mem           srcBuffer,
    cl_mem           dstBuffer,
    const size_t*    pSrcOrigin,
    const size_t*    pDstOrigin,
    const size_t*    pRegion,
    size_t           srcRowPitch,
    size_t           srcSlicePitch,
    size_t           dstRowPitch,
    size_t           dstSlicePitch,
    cl_uint          uEventWaitList,
    const cl_event*  pEventWaitList,
    cl_event*        pEvent)
{
    if (NULL == g_nextDispatchTable.EnqueueCopyBufferRect)
    {
        // this function is only supported by OpenCL 1.1 so it shouldn't
        // be called by OpenCL 1.0 app
        SpBreak("Mine_clEnqueueCopyBufferRect called by OpenCL 1.0 application");
        return CL_SUCCESS;
    }

    SeqIDGenerator::Instance()->GenerateID();

    return g_nextDispatchTable.EnqueueCopyBufferRect(commandQueue,
                                                     srcBuffer,
                                                     dstBuffer,
                                                     pSrcOrigin,
                                                     pDstOrigin,
                                                     pRegion,
                                                     srcRowPitch,
                                                     srcSlicePitch,
                                                     dstRowPitch,
                                                     dstSlicePitch,
                                                     uEventWaitList,
                                                     pEventWaitList,
                                                     pEvent);
}

// OpenCL OpenGL interop
cl_mem CL_API_CALL
Mine_clCreateFromGLBuffer(cl_context     context,
                          cl_mem_flags   flags,
                          cl_GLuint      bufobj,
                          int*           errcode_ret)
{
    SeqIDGenerator::Instance()->GenerateID();
    cl_mem mem = g_nextDispatchTable.CreateFromGLBuffer(context, flags, bufobj, errcode_ret);

    if (mem == NULL)
    {
        // there is an error with the function so we don't need to record the operation
        return mem;
    }

    AddRWBuffer(context, mem, flags);

    return mem;
}

cl_mem CL_API_CALL
Mine_clCreateFromGLRenderbuffer(cl_context   context,
                                cl_mem_flags flags,
                                cl_GLuint    renderbuffer,
                                cl_int*      errcode_ret)
{
    SeqIDGenerator::Instance()->GenerateID();
    cl_mem mem = g_nextDispatchTable.CreateFromGLRenderbuffer(context, flags, renderbuffer, errcode_ret);

    if (mem == NULL)
    {
        // there is an error with the function so we don't need to record the operation
        return mem;
    }

    AddRWBuffer(context, mem, flags);

    return mem;
}

// OpenCL DX10 interop
#ifdef _WIN32
cl_mem CL_API_CALL Mine_clCreateFromD3D10BufferKHR(cl_context     context,
                                                   cl_mem_flags   flags,
                                                   ID3D10Buffer*  resource,
                                                   cl_int*        errcode_ret)
{
    SeqIDGenerator::Instance()->GenerateID();
    cl_mem mem = ((clCreateFromD3D10BufferKHR_fn)(g_nextDispatchTable._reservedForD3D10KHR[1]))(
                     context,
                     flags,
                     resource,
                     errcode_ret);

    if (mem == NULL)
    {
        // there is an error with the function so we don't need to record the operation
        return mem;
    }

    AddRWBuffer(context, mem, flags);

    return mem;
}
#endif

//******OpenCL 1.2************************//
cl_mem CL_API_CALL
Mine_clCreateImage(cl_context             context,
                   cl_mem_flags           flags,
                   const cl_image_format* pImageFormat,
                   const cl_image_desc*   pImageDesc,
                   void*                  pHost,
                   cl_int*                pErrorCode)
{
    if (NULL == g_nextDispatchTable.CreateImage)
    {
        // this function is only supported by OpenCL 1.2 so it shouldn't
        // be called by OpenCL 1.1 app
        SpBreak("Mine_clCreateImage called by OpenCL 1.1 application");
        return CL_SUCCESS;
    }

    SeqIDGenerator::Instance()->GenerateID();
    return g_nextDispatchTable.CreateImage(context, flags, pImageFormat, pImageDesc, pHost, pErrorCode);
}

cl_int CL_API_CALL
Mine_clEnqueueFillBuffer(cl_command_queue command_queue,
                         cl_mem           buffer,
                         const void*      pPattern,
                         size_t           patternSize,
                         size_t           offset,
                         size_t           size,
                         cl_uint          uEventWaitList,
                         const cl_event*  pEventWaitList,
                         cl_event*        pEvent)
{
    if (NULL == g_nextDispatchTable.EnqueueFillBuffer)
    {
        // this function is only supported by OpenCL 1.2 so it shouldn't
        // be called by OpenCL 1.1 app
        SpBreak("Mine_clEnqueueFillBuffer called by OpenCL 1.1 application");
        return CL_SUCCESS;
    }

    SeqIDGenerator::Instance()->GenerateID();
    return g_nextDispatchTable.EnqueueFillBuffer(command_queue, buffer, pPattern, patternSize, offset, size, uEventWaitList, pEventWaitList, pEvent);
}

cl_int CL_API_CALL
Mine_clEnqueueFillImage(cl_command_queue command_queue,
                        cl_mem           image,
                        const void*      pFillColor,
                        const size_t*    pOrigin,
                        const size_t*    pRegion,
                        cl_uint          uEventWaitList,
                        const cl_event*  pEventWaitList,
                        cl_event*        pEvent)
{
    if (NULL == g_nextDispatchTable.EnqueueFillImage)
    {
        // this function is only supported by OpenCL 1.2 so it shouldn't
        // be called by OpenCL 1.1 app
        SpBreak("Mine_clEnqueueFillImage called by OpenCL 1.1 application");
        return CL_SUCCESS;
    }

    SeqIDGenerator::Instance()->GenerateID();
    return g_nextDispatchTable.EnqueueFillImage(command_queue, image, pFillColor, pOrigin, pRegion, uEventWaitList, pEventWaitList, pEvent);
}

cl_int CL_API_CALL
Mine_clEnqueueMigrateMemObjects(cl_command_queue       command_queue,
                                cl_uint                uMemObjects,
                                const cl_mem*          pMemObjects,
                                cl_mem_migration_flags flags,
                                cl_uint                uEventWaitList,
                                const cl_event*        pEventWaitList,
                                cl_event*              pEvent)
{
    if (NULL == g_nextDispatchTable.EnqueueMigrateMemObjects)
    {
        // this function is only supported by OpenCL 1.2 so it shouldn't
        // be called by OpenCL 1.1 app
        SpBreak("Mine_clEnqueueMigrateMemObjects called by OpenCL 1.1 application");
        return CL_SUCCESS;
    }

    SeqIDGenerator::Instance()->GenerateID();
    return g_nextDispatchTable.EnqueueMigrateMemObjects(command_queue, uMemObjects, pMemObjects, flags, uEventWaitList, pEventWaitList, pEvent);
}
