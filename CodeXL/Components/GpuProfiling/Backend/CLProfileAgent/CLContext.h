//==============================================================================
// Copyright (c) 2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief This class manages all the information in a CL context.
//==============================================================================

#ifndef _CL_CONTEXT_H_
#define _CL_CONTEXT_H_

#include <vector>
#include "CLKernel.h"
#include "CLBuffer.h"

/// \addtogroup CLContextManager
// @{

/// This class manages all the information in a CL context
class CLContext
{
public:
    /// default constructor.
    CLContext() { m_context = NULL; }

    /// constructor.
    /// \param context the CL context
    CLContext(const cl_context context) { m_context = context; }

    /// destructor.
    ~CLContext();

    /// check the context stored.
    /// \param context the CL context
    /// \return true if the stored context is the same as the param, false otherwise
    bool IsEqual(const cl_context& context) const;

    /// add a kernel to the context.
    /// \param kernel the CL kernel
    void AddKernel(const cl_kernel& kernel);

    /// remove a kernel from the context.
    /// \param kernel the CL kernel
    void RemoveKernel(const cl_kernel& kernel);

    /// Save a kernel arg to the contextManager if the arg value is of type cl_buffer created with CL_MEM_READ_WRITE flag.
    /// \param kernel    the CL kernel
    /// \param argIdx argument binding index
    /// \param pArgValue the argument to the kernel
    /// \return true if the kernel arg is saved successfully, false otherwise
    bool AddKernelArg(const cl_kernel& kernel, cl_uint argIdx, const void* pArgValue);

    /// Flag the specified kernel arg as an SVM kernel arg
    /// \param kernel    the CL kernel
    /// \param argIdx the index of the kernel arg which is an SVM pointer
    /// \return true if the kernel arg is saved successfully, false otherwise
    bool AddKernelArgSVMPointer(const cl_kernel& kernel, cl_uint argIdx);

    /// Check whether the kernel uses SVM Pointer args
    /// \param kernel    the CL kernel
    /// \return true if the kernel uses SVM Pointer kernel args
    bool HasKernelArgSVMPointer(const cl_kernel kernel) const;

    /// Flag the specified kernel arg as an pipe kernel arg
    /// \param kernel    the CL kernel
    /// \param argIdx the index of the kernel arg which is a pipe
    /// \return true if the kernel arg is saved successfully, false otherwise
    bool AddKernelArgPipe(const cl_kernel& kernel, cl_uint argIdx);

    /// Check whether the kernel uses pipe args
    /// \param kernel    the CL kernel
    /// \return true if the kernel uses pipe kernel args
    bool HasKernelArgPipe(const cl_kernel kernel) const;

    /// Add a buffer to the context (clCreateBuffer with CL_MEM_READ_WRITE)
    /// \param buffer     the CL buffer
    /// \param bufferSize the CL buffer size
    /// \param flags      flags used in clCreateBuffer
    /// \param pHost      the host pointer used in clCreateBuffer
    void AddBuffer(const cl_mem& buffer,
                   cl_mem_flags  flags,
                   size_t        bufferSize,
                   void*         pHost);

    /// Add a pipe to the context (clCreatePipe)
    /// \param pipe the CL pipe
    void AddPipe(const cl_mem& pipe);

    /// Remove a buffer object
    /// \param buffer    the CL buffer
    /// \return true if successful, false otherwise
    bool RemoveBuffer(const cl_mem& buffer);

    /// Checks to see if specified buffer is part of this context
    /// \param buffer the buffer to check
    /// \return true if the specified buffer is part of this context, false otherwise
    bool HasBuffer(const cl_mem& buffer);

    /// Load related memory buffers (buffers with both read and write) of a kernel.
    /// \param commandQueue the CL command queue
    /// \param kernel       the CL kernel
    /// \return true if successful, false otherwise
    bool LoadArena(const cl_command_queue& commandQueue, const cl_kernel& kernel);

    /// Save related memory buffers (buffers with both read and write) of a kernel.
    /// \param commandQueue the CL command queue
    /// \param kernel       the CL kernel
    /// \return true if successful, false otherwise
    bool SaveArena(const cl_command_queue& commandQueue, const cl_kernel& kernel);

    /// Release host buffers
    /// \param kernel       the CL kernel
    /// \return true if successful, false otherwise
    bool ClearArena(const cl_kernel& kernel);

private:

    /// copy constructor.
    /// \param clContext  the rhs object
    CLContext(const CLContext& clContext);

    /// assignment operator.
    /// \param clContext  the rhs object
    /// \return the reference to the object
    CLContext& operator=(const CLContext& clContext);

    /// Get the index of a kernel in the kernelList
    /// \param kernel the CL kernel to search for
    /// \return greater than equal to zero if successful, -1 otherwise
    int FindKernelIndex(const cl_kernel& kernel) const;

    /// Get the index of a buffer in the bufferList
    /// \param buffer the CL buffer to search for
    /// \return the index of the specified buffer: greater than or equal to zero if successful, -1 otherwise
    int FindBufferIndex(const cl_mem& buffer) const;

    /// Get the index of a pipe in the pipe list
    /// \param pipe the CL pipe to search for
    /// \return the index of the specified pipe: greater than or equal to zero if successful, - otherwise
    int FindPipeIndex(const cl_mem& pipe) const;


    cl_context m_context;                   ///< cl_context is a pointer
    std::vector< CLKernel* > m_kernelList;  ///< stores a list of kernel used by the context
    std::vector< CLBuffer* > m_bufferList;  ///< store a list of buffer used by the context
    std::vector< cl_mem >    m_pipeList;    ///< store a list of pipes used by the context
};

// @}

#endif // _CL_CONTEXT_H_
