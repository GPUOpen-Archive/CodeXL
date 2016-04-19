//==============================================================================
// Copyright (c) 2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief This class manages the CL contexts stored.
//==============================================================================

#ifndef _CL_CONTEXT_MANAGER_H_
#define _CL_CONTEXT_MANAGER_H_

#include "CLContext.h"

/// \defgroup CLContextManager CLContextManager
/// This module manages the OpenCL context to be used for loading and saving the memory
/// state to dispatch kernel multiple times with a consistent result.
/// We need to track all the arguments to the kernel because at the point when we
/// dispatch a kernel, we can't query the arguments through the OpenCL run-time (no such query).
///
/// \ingroup CLProfileAgent
// @{

/// This class manages the CL contexts stored
class CLContextManager
{
public:
    /// default constructor
    CLContextManager() { }

    /// destructor.
    ~CLContextManager();

    /// Load related memory buffers (buffers with both read and write) of a kernel for arena support.
    /// \param context      the CL context
    /// \param commandQueue the CL command queue
    /// \param kernel       the CL kernel
    /// \return true if successful, false otherwise
    bool LoadArena(const cl_context& context, const cl_command_queue& commandQueue, const cl_kernel& kernel);

    /// Save related memory buffers (buffers with both read and write) of a kernel for arena support.
    /// \param context      the CL context
    /// \param commandQueue the CL command queue
    /// \param kernel       the CL kernel
    /// \return true if successful, false otherwise
    bool SaveArena(const cl_context& context, const cl_command_queue& commandQueue, const cl_kernel& kernel);

    /// Clear host memories
    /// \param context      the CL context
    /// \param kernel       the CL kernel
    /// \return true if successful, false otherwise
    bool ClearArena(const cl_context& context, const cl_kernel& kernel);

    /// Save context to the list.
    /// \param context the CL context to be saved
    /// \return true if successful, false otherwise (context had been added)
    bool AddContext(const cl_context& context);

    /// Remove context from the list.
    /// \param context the CL context to be saved
    /// \return true if successful, false otherwise
    bool RemoveContext(const cl_context& context);

    /// Save kernel to the context list.
    /// \param context the CL context
    /// \param kernel  the CL kernel to be saved
    /// \return true if successful, false if we can't find the context in the contextList
    bool AddKernelToContext(const cl_context& context, const cl_kernel& kernel);

    /// Remove kernel from the context list.
    /// \param context the CL context
    /// \param kernel  the CL kernel to be removed
    /// \return true if successful, false if we can't find the context in the contextList
    bool RemoveKernelFromContext(const cl_context& context, const cl_kernel& kernel);

    /// Save a kernel arg to the contextManager if the arg value is of type cl_buffer created for read and write.
    /// \param context   the CL context
    /// \param kernel    the CL kernel
    /// \param argIdx argument binding index
    /// \param pArgValue the argument to the kernel
    /// \return true if the kernel arg is saved successfully, false otherwise
    bool AddKernelArg(const cl_context& context, const cl_kernel& kernel, cl_uint argIdx, const void* pArgValue);

    /// Flag the specified kernel arg as an SVM kernel arg
    /// \param context   the CL context
    /// \param kernel    the CL kernel
    /// \param argIdx the index of the kernel arg which is an SVM pointer
    /// \return true if the kernel arg is saved successfully, false otherwise
    bool AddKernelArgSVMPointer(const cl_context& context, const cl_kernel kernel, cl_uint argIdx);

    /// Check whether the kernel uses SVM Pointer args
    /// \param context   the CL context
    /// \param kernel    the CL kernel
    /// \return true if the kernel uses SVM Pointer kernel args
    bool HasKernelArgSVMPointer(const cl_context& context, const cl_kernel kernel);

    /// Flag the specified kernel arg as a pipe kernel arg
    /// \param context   the CL context
    /// \param kernel    the CL kernel
    /// \param argIdx the index of the kernel arg which is a pipe
    /// \return true if the kernel arg is saved successfully, false otherwise
    bool AddKernelArgPipe(const cl_context& context, const cl_kernel kernel, cl_uint argIdx);

    /// Check whether the kernel uses pipe args
    /// \param context   the CL context
    /// \param kernel    the CL kernel
    /// \return true if the kernel uses pipe kernel args
    bool HasKernelArgPipe(const cl_context& context, const cl_kernel kernel);

    /// Save buffer to the context list.
    /// \param context    the CL context
    /// \param buffer     the CL buffer to be saved
    /// \param flags      the flags parameter when the buffer was created
    /// \param bufferSize the size of the CL buffer
    /// \param pHost      the host pointer (may be NULL) for the buffer when it was created
    /// \return true if successful, false if we can't find the context in the contextList
    bool AddBufferToContext(const cl_context& context,
                            const cl_mem&     buffer,
                            cl_mem_flags      flags,
                            size_t            bufferSize,
                            void*             pHost);

    /// Save sub buffer to the parent's context list.
    /// \param parentBuffer the parent buffer from which the sub buffer was created
    /// \param subBuffer    the CL buffer to be saved
    /// \param flags        the flags parameter when the buffer was created
    /// \param bufferSize   the size of the CL buffer
    /// \return true if successful, false if we can't find the parent's context in the contextList
    bool AddSubBuffer(const cl_mem& parentBuffer,
                      const cl_mem& subBuffer,
                      cl_mem_flags  flags,
                      size_t        bufferSize);

    /// Save pipe to the context list
    /// \param context    the CL context
    /// \param pipe       the CL pipe to be saved
    /// \return true if successful, false if we can't find the context in the contextList
    bool AddPipeToContext(const cl_context context, const cl_mem pipe);

    /// Search for CLContext object, delete buffer from the CLContext object
    /// \param context   the CL context
    /// \param buffer    the CL buffer
    /// \return true if successful.
    bool RemoveBuffer(const cl_context& context,
                      const cl_mem&     buffer);

private:
    // disable copy constructor and assignment
    CLContextManager(const CLContextManager& cm);
    CLContextManager& operator=(const CLContextManager& cm);

    /// Get the index of a context in the contextList.
    int FindContextIndex(const cl_context& context);

    /// Get the iterator of a context in the contextList.
    std::vector< CLContext* >::iterator FindContext(const cl_context& context);

    std::vector< CLContext* > m_contextList;    ///< the context list stored
};

// @}

#endif // _CL_CONTEXT_MANAGER_H_
