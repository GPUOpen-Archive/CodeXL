//==============================================================================
// Copyright (c) 2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief The header file for detouring the CL entry functions
///        related to the memory copies operations.
//==============================================================================

#ifndef _CL_PROFILER_MINE_CL_MEMORY_H_
#define _CL_PROFILER_MINE_CL_MEMORY_H_

#include <CL/opencl.h>
#include "CLFunctionDefs.h"

/// \addtogroup CLProfileAgent
// @{

cl_mem CL_API_CALL
Mine_clCreateBuffer(cl_context context,
                    cl_mem_flags flags,
                    size_t       bufferSize,
                    void*        pHost,
                    cl_int*      pErrorCode);
cl_mem CL_API_CALL
Mine_clCreateSubBuffer(
    cl_mem                    buffer,
    cl_mem_flags              flags,
    cl_buffer_create_type     buffer_create_type,
    const void*               buffer_create_info,
    cl_int*                   errcode_ret);
cl_mem CL_API_CALL
Mine_clCreateImage2D(cl_context             context,
                     cl_mem_flags           flags,
                     const cl_image_format* pFormat,
                     size_t                 width,
                     size_t                 height,
                     size_t                 rowPitch,
                     void*                  pHost,
                     cl_int*                pErrorCode);
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
                     cl_int*                pErrorCode);
cl_int CL_API_CALL
Mine_clEnqueueReadBuffer(cl_command_queue commandQueue,
                         cl_mem           buffer,
                         cl_bool          bBlock,
                         size_t           offset,
                         size_t           bufferSize,
                         void*            pBuffer,
                         cl_uint          uEventWaitList,
                         const cl_event*  pEventWaitList,
                         cl_event*        pEvent);
cl_int CL_API_CALL
Mine_clEnqueueWriteBuffer(cl_command_queue commandQueue,
                          cl_mem           buffer,
                          cl_bool          bBlock,
                          size_t           offset,
                          size_t           bufferSize,
                          const void*      pBuffer,
                          cl_uint          uEventWaitList,
                          const cl_event*  pEventWaitList,
                          cl_event*        pEvent);
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
                        cl_event*        pEvent);
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
                         cl_event*        pEvent);
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
                        cl_int*          pErrorCode);
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
                       cl_int*          pErrorCode);
cl_int CL_API_CALL
Mine_clEnqueueUnmapMemObject(cl_command_queue commandQueue,
                             cl_mem           memObj,
                             void*            pMapped,
                             cl_uint          uEventWaitList,
                             const cl_event*  pEventWaitList,
                             cl_event*        pEvent);
cl_int CL_API_CALL
Mine_clEnqueueCopyBuffer(cl_command_queue commandQueue,
                         cl_mem           sourceBuffer,
                         cl_mem           destBuffer,
                         size_t           sourceOffset,
                         size_t           destOffset,
                         size_t           bufferSize,
                         cl_uint          uEventWaitList,
                         const cl_event*  pEventWaitList,
                         cl_event*        pEvent);
cl_int CL_API_CALL
Mine_clEnqueueCopyImage(cl_command_queue commandQueue,
                        cl_mem           sourceImage,
                        cl_mem           destImage,
                        const size_t*    pSourceOrigin,
                        const size_t*    pDestOrigin,
                        const size_t*    pSize,
                        cl_uint          uEventWaitList,
                        const cl_event*  pEventWaitList,
                        cl_event*        pEvent);
cl_int CL_API_CALL
Mine_clEnqueueCopyImageToBuffer(cl_command_queue commandQueue,
                                cl_mem           sourceImage,
                                cl_mem           destBuffer,
                                const size_t*    pSourceOrigin,
                                const size_t*    pSize,
                                size_t           destOffset,
                                cl_uint          uEventWaitList,
                                const cl_event*  pEventWaitList,
                                cl_event*        pEvent);
cl_int CL_API_CALL
Mine_clEnqueueCopyBufferToImage(cl_command_queue commandQueue,
                                cl_mem           sourceBuffer,
                                cl_mem           destImage,
                                size_t           sourceOffset,
                                const size_t*    pDestOrigin,
                                const size_t*    pSize,
                                cl_uint          uEventWaitList,
                                const cl_event*  pEventWaitList,
                                cl_event*        pEvent);

cl_mem CL_API_CALL
Mine_clCreateFromGLBuffer(cl_context     context,
                          cl_mem_flags   flags,
                          cl_GLuint      bufobj,
                          int*           errcode_ret);

cl_mem CL_API_CALL
Mine_clCreateFromGLRenderbuffer(cl_context   context,
                                cl_mem_flags flags,
                                cl_GLuint    renderbuffer,
                                cl_int*      errcode_ret);

#ifdef _WIN32
cl_mem CL_API_CALL
Mine_clCreateFromD3D10BufferKHR(cl_context     context,
                                cl_mem_flags   flags,
                                ID3D10Buffer*  resource,
                                cl_int*        errcode_ret);
#endif

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
    cl_event*        pEvent);

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
    const void*            ptr,
    cl_uint          uEventWaitList,
    const cl_event*  pEventWaitList,
    cl_event*        pEvent);

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
    cl_event*        pEvent);

//******OpenCL 1.2************************//
cl_mem CL_API_CALL
Mine_clCreateImage(cl_context             context,
                   cl_mem_flags           flags,
                   const cl_image_format* pImageFormat,
                   const cl_image_desc*   pImageDesc,
                   void*                  pHost,
                   cl_int*                pErrorCode);

cl_int CL_API_CALL
Mine_clEnqueueFillBuffer(cl_command_queue command_queue,
                         cl_mem           buffer,
                         const void*      pPattern,
                         size_t           patternSize,
                         size_t           offset,
                         size_t           size,
                         cl_uint          uEventWaitList,
                         const cl_event*  pEventWaitList,
                         cl_event*        pEvent);

cl_int CL_API_CALL
Mine_clEnqueueFillImage(cl_command_queue command_queue,
                        cl_mem           image,
                        const void*      pFillColor,
                        const size_t*    pOrigin,
                        const size_t*    pRegion,
                        cl_uint          uEventWaitList,
                        const cl_event*  pEventWaitList,
                        cl_event*        pEvent);


cl_int CL_API_CALL
Mine_clEnqueueMigrateMemObjects(cl_command_queue       command_queue,
                                cl_uint                uMemObjects,
                                const cl_mem*          pMemObjects,
                                cl_mem_migration_flags flags,
                                cl_uint                uEventWaitList,
                                const cl_event*        pEventWaitList,
                                cl_event*              pEvent);

// @}

#endif // _CL_PROFILER_MINE_CL_MEMORY_H_
