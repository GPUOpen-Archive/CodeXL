//==============================================================================
// Copyright (c) 2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief This class manages the retrieval of CL kernel source, IL,
///        ISA from the CL run-time.
//==============================================================================

#ifndef _CL__KERNEL_ASSEMBLY_H_
#define _CL__KERNEL_ASSEMBLY_H_

#include <string>
#include <map>
#include <vector>
#include <CL/opencl.h>

#include "CLUtils.h"
#include "ACLModule.h"

/// \defgroup CLKernelAssembly CLKernelAssembly
/// This module handles the retrieval of OpenCL kernel source, IL, ISA, and x86
/// from the OpenCL run-time.
///
/// \ingroup CLProfileAgent
// @{

/// This class manages the assembly files (IL/ISA/x86/source) generation and output
class KernelAssembly
{
public:
    /// Constructor.
    KernelAssembly();

    /// Destructor.
    ~KernelAssembly();

    /// Generate the kernel assembly files.
    /// \param commandQueue      the CL command queue
    /// \param kernel            the CL kernel handle
    /// \param strKernelFunction the kernel function name
    /// \param strKernelHandle   the string uniquely identifying the kernel
    /// \param strOutputDir      the directory to store the output kernel assembly files
    /// \return true if successful, false otherwise
    bool Generate(const cl_command_queue& commandQueue,
                  const cl_kernel&        kernel,
                  const std::string&      strKernelFunction,
                  const std::string&      strKernelHandle,
                  const std::string&      strOutputDir);

    /// return the prefix string for the output .cl/.il/.isa files
    /// \return the prefix string
    std::string GetFilePrefix() const { return m_strFilePrefix; }

    /// Get the KernelInfo structure for the kernel.
    /// \return a copy of the structure that hold the kernel information
    const KernelInfo& GetKernelInfo(std::string& strKernelName) const;

    /// Remove all entries in m_assemblyGenerated
    void Reset()
    {
        m_assemblyGenerated.clear();
    }

    /// gets the flag indicating if the IL file should be written out
    /// \return flag indicating if the IL file should be written out
    bool GetOutputIL() const { return m_bOutputIL; }

    /// gets the flag indicating if the HSAIL file should be written out
    /// \return flag indicating if the HSAIL file should be written out
    bool GetOutputHSAIL() const { return m_bOutputHSAIL; }

    /// gets the flag indicating if the ISA file should be written out
    /// \return flag indicating if the ISA file should be written out
    bool GetOutputISA() const { return m_bOutputISA; }

    /// gets the flag indicating if the CL file should be written out
    /// \return flag indicating if the CL file should be written out
    bool GetOutputCL() const { return m_bOutputCL; }

    /// sets the flag indicating if the IL file should be written out
    /// \param bOutputIL flag indicating if the IL file should be written out
    void SetOutputIL(bool bOutputIL) { m_bOutputIL = bOutputIL; }

    /// sets the flag indicating if the HSAIL file should be written out
    /// \param bOutputHSAIL flag indicating if the HSAIL file should be written out
    void SetOutputHSAIL(bool bOutputHSAIL) { m_bOutputHSAIL = bOutputHSAIL; }

    /// sets the flag indicating if the ISA file should be written out
    /// \param bOutputISA flag indicating if the ISA file should be written out
    void SetOutputISA(bool bOutputISA) { m_bOutputISA = bOutputISA; }

    /// sets the flag indicating if the CL file should be written out
    /// \param bOutputCL flag indicating if the CL file should be written out
    void SetOutputCL(bool bOutputCL) { m_bOutputCL = bOutputCL; }

private:
    // Disable copy constructor.
    /// \param cm  the rhs object
    KernelAssembly(const KernelAssembly& cm);

    /// Disable assignment operator.
    /// \param cm  the rhs object
    /// \return a reference to the object
    KernelAssembly& operator=(const KernelAssembly& cm);

    /// Get the CL kernel source and write to strOutputDir\(strPrefix)strKernelHandle.cl.
    /// \param kernel          the CL kernel handle
    /// \param strKernelHandle the string uniquely identifying the kernel
    /// \param strOutputDir    the directory to store the output file
    /// \return true if successful, false otherwise
    bool DumpCLSource(const cl_kernel&   kernel,
                      const std::string& strKernelHandle,
                      const std::string& strOutputDir) const;

    /// Gets the program binary from OpenCL runtime
    /// \param program the OpenCL program
    /// \param device the OpenCL device
    /// \param[out] pBinary the program binary
    /// \return true if successful, false otherwise
    bool GetProgramBinary(cl_program         program,
                          cl_device_id       device,
                          std::vector<char>* pBinary);

    /*
       /// A utility function to parse the ISA string and collect the SC stats.
       /// \param strISA     the string containing the ISA
       /// \param SCStatsOut the output SC stats
       /// \return true if successful, false otherwise
       bool ParseISA(const std::string& strISA,
                     SCStats&           SCStatsOut) const;
    */

    /// A utility function to parse the SI ISA string and collect the kernel info.
    /// \param strISA     the string containing the ISA
    /// \param kiOut      the output Kernel Info
    /// \return true if successful, false otherwise
    bool ParseISASI(const std::string& strISA,
                    KernelInfo& kiOut) const;

    /// Extract kernel files, using the specified compiler libs
    /// \param pAclModule ACLModule to use
    /// \param pAclCompiler aclCompiler to use
    /// \param vBinary Bif binary for ocl runtime
    /// \param strKernelFunction Kernel name
    /// \param strKernelHandle Kernel object handle
    /// \param strOutputDir Output dir
    /// \param isGPU Is kernel compiled for GPU
    /// \param isGPU Are the specified ACLModule and ACLCompiler instances the HSAIL versions (vs. the AMDIL versions)
    /// \return true if no error
    bool GenerateKernelFilesFromACLModule(ACLModule*         pAclModule,
                                          aclCompiler*       pAclCompiler,
                                          std::vector<char>& vBinary,
                                          const std::string& strKernelFunction,
                                          const std::string& strKernelHandle,
                                          const std::string& strOutputDir,
                                          bool               isGPU,
                                          bool               usesHSAILPath);

    /// Extract kernel files, if compiler lib is detected, use compiler lib, otherwise, fallback to CAL
    /// \param vBinary Bif binary for ocl runtime
    /// \param strKernelFunction Kernel name
    /// \param strKernelHandle Kernel object handle
    /// \param strOutputDir Output dir
    /// \param isGPU Is kernel compiled for GPU
    /// \return true if no error
    bool GenerateKernelFiles(std::vector<char>& vBinary,
                             const std::string& strKernelFunction,
                             const std::string& strKernelHandle,
                             const std::string& strOutputDir,
                             bool               isGPU);

    /// Disassembler callback function
    /// \param pMsg disassembly text
    /// \param size size of the message
    static void DisassembleLogFunction(const char* pMsg, size_t size);

    /// Utility function to check whether, with the current driver and hardware combination, the HSAIL compilation path is the default
    /// \param strDeviceName the name of the device to check
    /// \param device the ocl device
    /// \return true if the current compilation path is the HSAIL path
    bool DoesUseHSAILPath(const std::string& strDeviceName, cl_device_id device);

    std::map<std::string, KernelInfo>          m_assemblyGenerated;          ///< to check whether files with the same handle has been generated or not
    std::string                                m_strFilePrefix;              ///< the prefix string for the .cl/.isa/.il output files
    KernelInfo                                 m_kernelInfoDefault;          ///< the shader compiler stats we can retrieve from the isa file
    bool                                       m_bOutputIL;                  ///< flag indicating whether or not to write out the IL file
    bool                                       m_bOutputISA;                 ///< flag indicating whether or not to write out the ISA file
    bool                                       m_bOutputCL;                  ///< flag indicating whether or not to write out the CL file
    bool                                       m_bOutputHSAIL;               ///< flag indicating whether or not to write out the HSAIL file
    bool                                       m_bInitCAL;                   ///< flag indicating whether or not CALRT is initialized
    static std::string                         m_sTmpDisassemblyLoggerISA;   ///< string to hold the ISA text contents from the disassembly logger
    static std::string                         m_sTmpDisassemblyLoggerHSAIL; ///< string to hold the HSAIL text contents from the disassembly logger
    static unsigned int                        m_sDisassembleCount;          ///< count of the number of unique text items from the disassembly logger

};

// @}

#endif // _CL__KERNEL_ASSEMBLY_H_
