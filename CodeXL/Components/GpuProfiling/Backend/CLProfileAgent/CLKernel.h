//==============================================================================
// Copyright (c) 2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief This class manages all the information in a CL kernel.
//==============================================================================

#ifndef _CL_KERNEL_H_
#define _CL_KERNEL_H_

#include <map>
#include <vector>
#include "CLBuffer.h"

typedef std::map< cl_uint, const CLBuffer* > CLKernelArgMap;
typedef std::pair< cl_uint, const CLBuffer* > CLKernelArgMapPair;

typedef std::map< cl_uint, char* > CLKernelArgBufferHostMap;
typedef std::pair< cl_uint, char* > CLKernelArgBufferHostMapPair;

/// \addtogroup CLContextManager
// @{

/// This class manages all the information in a CL kernel
class CLKernel
{
public:
    /// default constructor.
    CLKernel() { m_kernel = NULL; }

    /// constructor.
    CLKernel(const cl_kernel& kernel) { m_kernel = kernel; }

    /// destructor.
    ~CLKernel();

    /// Check the kernel stored.
    /// \param kernel  the input CL kernel handle
    /// \return true if the stored kernel is equal to the param kernel, false otherwise
    bool IsKernel(const cl_kernel& kernel) const;

    /// Store the kernel argument related to buffer that we need for arena support.
    /// \param argIdx the index of the kernel arg
    /// \param the buffer object represented by this kernel arg
    void AddKernelBufferArg(cl_uint argIdx, const CLBuffer* buffer);

    /// Flag the specified kernel arg as an SVM kernel arg
    /// \param argIdx the index of the kernel arg which is an SVM pointer
    void AddKernelArgSVMPointer(cl_uint argIdx);

    /// Un-flag the specified kernel arg as an SVM kernel arg or pipe arg
    /// \param argIdx the index of the kernel arg which is an SVM pointer
    void RemoveKernelArgSVMPointerOrPipe(cl_uint argIdx);

    /// Check whether the kernel uses SVM Pointer args
    /// \return true if the kernel uses SVM Pointer kernel args
    bool HasKernelArgSVMPointer() const;

    /// Flag the specified kernel arg as an pipe kernel arg
    /// \param argIdx the index of the kernel arg which is a pipe
    void AddKernelArgPipe(cl_uint argIdx);

    /// Check whether the kernel uses pipe args
    /// \return true if the kernel uses pipe kernel args
    bool HasKernelArgPipe() const;

    /// Load related memory buffers (buffers with both read and write) of a kernel.
    /// \param commandQueue the CL command queue
    /// \return true if successful, false otherwise
    bool LoadArena(const cl_command_queue& commandQueue);

    /// Save related memory buffers (buffers with both read and write) of a kernel.
    /// \param commandQueue the CL command queue
    /// \return true if successful, false otherwise
    bool SaveArena(const cl_command_queue& commandQueue);

    /// Release host buffers
    /// \return true if successful, false otherwise
    bool ClearArena() { ClearArgBufferHostList(); return true; }

    /// When CLBuffer object is about to be released, remove any reference to it
    /// \param memobj ocl mem object
    /// \return true if a reference is found and successfully deleted.
    bool RemoveKernelArg(const cl_mem memobj);

private:
    /// copy constructor.
    CLKernel(const CLKernel& CLKernel);

    /// assignment operator.
    /// \param CLKernel the rhs object
    /// \return a reference to the object
    CLKernel& operator=(const CLKernel& CLKernel);

    /// a utility function to clear the buffer host list.
    void ClearArgBufferHostList();

    cl_kernel                m_kernel;                 ///< a handle to the CL kernel
    CLKernelArgMap           m_kernelArgBufferMap;     ///< a map of buffer to keep track for arena support
    CLKernelArgBufferHostMap m_kernelArgBufferHostMap; ///< a map of the host buffer pointer
    std::vector<cl_uint>     m_svmPointerArgIndices;   ///< list of kernel arg indices which are SVM pointers
    std::vector<cl_uint>     m_pipeArgIndices;         ///< list of kernel arg indices which are pipes
};

// @}

#endif // _CL_KERNEL_H_
