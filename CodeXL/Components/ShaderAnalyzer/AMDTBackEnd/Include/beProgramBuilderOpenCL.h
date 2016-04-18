#ifndef _BEPROGRAMBUILDEROPENCL_H_
#define _BEPROGRAMBUILDEROPENCL_H_

// C++.
#include <string>
#include <sstream>

#include "beProgramBuilder.h"

#ifdef _WIN32
    #include <DXXModule.h>
#endif

#include <CALModule.h>
#include <CL/cl.h>
#include <OpenCLModule.h>
#include <ACLModule.h>

#ifdef _WIN32
    #pragma warning(push)
    #pragma warning(disable : 4127)
    #pragma warning(disable : 4251)
#endif

using namespace std;
using namespace beKA;

class CElf;

class KA_BACKEND_DECLDIR beProgramBuilderOpenCL : public beProgramBuilder
{
public:
    virtual ~beProgramBuilderOpenCL(void);

public:

    /// Options specific to OpenCL
    struct OpenCLOptions : public beKA::CompileOptions
    {
        /// OpenCL compilation options passed into clBuildProgram.
        /// A vector of strings, since this is convenient for Boost command line processing.
        std::vector<std::string> m_OpenCLCompileOptions;
        /// OpenCL compilation options passed into clBuildProgram as -Ditems.
        /// A vector of strings, since this is convenient for Boost command line processing.
        std::vector<std::string> m_Defines;
        /// Choose a subset of devices for compilation.
        /// If a subset is not selected,
        /// the backend will compile for all available devices.
        std::set<std::string> m_SelectedDevices;

        /// Indicator if Emulation Analysis should be performed
        bool m_Analyze;
    };

    /// Get list of Kernels/Shaders.
    /// Must be called after Compile is successfully called.
    /// \param[in]  device  The name of the device to choose.
    /// \param[out] kernels Vector of names of Kernels/Shaders compiled.
    /// \returns            a status.
    /// If a Log stream is available, some failures may be diagnosed.
    virtual beKA::beStatus GetKernels(const std::string& device, std::vector<std::string>& kernels);

    /// Get a binary version of the program.
    /// \param[in]  program Handle to the built program.
    /// \param[in]  device  The name of the device to choose.
    /// \param[in]  binopts Options to customize the output object.
    ///                     If NULL, a complete object is returned.
    /// \param[out] binary  A place to return a reference to the binary.
    /// \returns            a status.
    /// If a Log stream is available, some failures may be diagnosed.
    virtual beKA::beStatus GetBinary(const std::string& device, const beKA::BinaryOptions& binopts, std::vector<char>& binary);

    /// Get a binary version of the program.
    /// \param[in]  pathToBinary   path to the binary file to load from disk.
    /// \param[in]  binopts        Options to customize the output object.
    ///                     If NULL, a complete object is returned.
    /// \param[out] outputPath     A place to return a reference to the binary.
    /// \returns            a status.
    /// If a Log stream is available, some failures may be diagnosed.
    virtual beKA::beStatus GetBinaryFromFile(const std::string& pathToBinary, const beKA::BinaryOptions& binopts, std::vector<char>& outputPath);

    /// Analyze a kernel/function.
    /// \param[in]  device   The name of the device.
    /// \param[in]  kernel   The name of the kernel/function to analyze.
    /// \param[out] analysis Data gathered from analysis.
    /// If a Log stream is available, some failures may be diagnosed.
    virtual beKA::beStatus GetStatistics(const std::string& device, const std::string& kernel, beKA::AnalysisData& analysis);

    /// Get version information about the OpenCL runtime.
    /// \returns a string of version information.
    /// The format when successful is:
    ///   OpenCL Version (as specified in the standard)\n
    ///   Driver Release (e.g. "Driver Build Number 8.97-gobbledy-gook")\n
    ///   Catalyst Version (e.g. "Catalyst Version 12.4")
    /// Upon failure, some or all of the parts may be missing.
    /// If a Log stream is available, some failures may be diagnosed.
    const std::string& GetOpenCLVersionInfo();

    /// Checks if the OpenCL part was set up properly
    /// \return true if yes and false otherwise
    virtual bool IsInitialized();

    /// Get a string for a kernel IL.
    /// \param[in]  device     The name of the device.
    /// \param[in]  kernel     The name of the kernel.
    /// \param[out] s          The output as a string.
    /// \returns               a status.
    /// If a Log stream is available, some failures may be diagnosed.
    virtual beKA::beStatus GetKernelILText(const std::string& device, const std::string& kernel, std::string& il);

    /// Get a string for a kernel ISA.
    /// \param[in]  device     The name of the device.
    /// \param[in]  kernel     The name of the kernel.
    /// \param[out] s          The output as a string.
    /// \returns               a status.
    /// If a Log stream is available, some failures may be diagnosed.
    virtual beKA::beStatus GetKernelISAText(const std::string& device, const std::string& kernel, std::string& isa);

    /// Get a string for a DebugIL .
    /// \param[in]  device     The name of the device.
    /// \param[in]  kernel     The name of the kernel.
    /// \param[out] s          The output as a string.
    /// \returns               a status.
    /// If a Log stream is available, some failures may be diagnosed.Compilation needs to be with -g option
    virtual beKA::beStatus GetKernelDebugILText(const std::string& device, const std::string& kernel, std::string& debugil);

    /// Get a string for a kernel Metadata.
    /// \param[in]  device     The name of the device.
    /// \param[in]  kernel     The name of the kernel.
    /// \param[out] s          The output as a string.
    /// \returns               a status.
    /// If a Log stream is available, some failures may be diagnosed.
    virtual beKA::beStatus GetKernelMetaDataText(const std::string& device, const std::string& kernel, std::string& metadata);

    /// compile the specified source file
    /// \param[in] programSource    the string of the source code
    /// \param[in] oclOptions       the compilation options
    /// \param[in] sourceCodeFullPathName    file being compiles full path and name.
    /// \param[in] sourcePath       additional source path. this is optional since the -I can be part of the option string.
    /// \param[out] numOfSuccessfulBuilds       output parameter to hold the number of successful builds.
    /// \returns               a status.
    /// If a Log stream is available, some failures may be diagnosed.
    virtual beKA::beStatus Compile(const std::string& programSource, const OpenCLOptions& oclOptions, const std::string& sourceCodeFullPathName,
                                   const std::vector<std::string>* pSourcePath, int& numOfSuccessfulBuilds);

    /// Get a set of available devices.
    /// \param[out] a container to hold the set of device names.
    /// \returns            a status.
    /// If a Log stream is available, failures to initialize OpenCL and CAL may be diagnosed.
    virtual beKA::beStatus GetDevices(std::set<string>& devices);

    /// Get the type (CPU, GPU) of a device.
    /// \param[in]  deviceName The name of the device.
    /// \param[out] deviceType The kind of device.
    /// \returns          a status.
    virtual beStatus GetDeviceType(const std::string& deviceName, cl_device_type& deviceType) const;

    /// Get a sorted table of devices.
    /// The entries are arranged by Hardware Generation, CAL Name, Marketing Name and Device ID.
    /// Because the table is compiled into the tool,
    /// old versions of the tool may not have have an incomplete table
    /// with respect to the list of CAL names generated by GetDevices.
    /// Users of this table will want to be careful
    /// to make any additional, new, devices available to the user.
    /// \param[out] table A place to leave a reference to the sorted table.
    /// \param[in]  kind  Which kind of table do we want?
    /// \returns          a status.
    beKA::beStatus GetDeviceTable(std::vector<GDT_GfxCardInfo>& table) override;

    /// Free the resources associated with a previous Compile.
    /// \param[in] program
    /// If a Log stream is available, some failures may be diagnosed.
    void ReleaseProgram();

    virtual bool CompileOK(std::string& device);

    /// force ending of the thread in a safe way:
    void ForceEnd();

    /// Retrieves the names of the supported public devices, as exposed
    /// by the OpenCL runtime.
    void GetSupportedPublicDevices(std::set<std::string>& devices) const;

protected:
    /// ctor.
    beProgramBuilderOpenCL();

    beKA::beStatus Initialize(const string& sDllModule = "");

private:
    /// Set up the OpenCL part.
    /// \returns status
    beKA::beStatus InitializeOpenCL();

    /// Free resources used to set up OpenCL.
    /// \returns status
    beKA::beStatus DeinitializeOpenCL();

    /// returns if the opencl Module was loaded
    /// \returns true/false if module loaded or not
    bool isOpenClModuleLoaded();

    /// Add devices that have been validated against the driver-supplied list
    /// \param[in] cardList the list of devices to validate
    /// \param[out] uniqueNamesOfPublishedDevices the validated lists of devices -- only devices reported by the driver will appear here
    void AddValidatedDevices(const std::vector<GDT_GfxCardInfo>& cardList, std::set<string>& uniqueNamesOfPublishedDevices);

    beKA::beStatus GetAnalysisInternal(cl_program& program, const std::string& device, const std::string& kernel, beKA::AnalysisData* analysis);

    /// Get statistics parameter value using.
    /// \param[in] pParamVal    The pointer to memory where the appropriate result being queried is returned.
    /// \param[in] paramValSize The size in bytes of memory pointed to by pParamVal.
    /// \param[in] paramName    The information to query.
    /// \param[in]  kernel      The kernel.
    /// \param[in]  deviceId    The id of the device.
    beKA::beStatus  Inquire(void* pParamVal, size_t paramValSize, KernelInfoAMD paramName, cl_kernel kernel, cl_device_id deviceId);

    beKA::beStatus GetKernelSectionText(const std::string& device, const std::string& kernel, std::string& il);


    // this is the internal version of the CompileOpenCL- here we actually do the compilation for a specific device
    beKA::beStatus CompileOpenCLInternal(const std::string& sourceCodeFullPathName, const std::string& programSource, const OpenCLOptions& oclOptions,
                                         cl_device_id requestedDeviceId, cl_program& program, std::string& definesAndOptions, int iCompilationNo, std::string& errString);

    /// Get a binary for an OpenCL Kernel using the opencl program
    /// \param[in]  program    The built program
    /// \param[in]  device     The name of the device.
    /// \param[out] bin        The device binary.
    beKA::beStatus GetProgramBinary(cl_program&     program, cl_device_id&   device, std::vector<char>*  vBinary);

    /// Callback to get the ISA text line by line from CAL image binary object.
    ///
    /// implicit param[out] s_ISAString Where the ISA is assembled.
    /// \param   line                   The ISA text created by the runtime.
    static void CalLoggerFunc(const CALchar* line);

    /// Disassembler callback function for ACLModule
    /// \param msg disassembly text
    /// \param size size of the message
    static void disassembleLogFunction(const char* msg, size_t size);

    bool BuildOpenCLProgramWrapper(
        cl_int&             status,                 ///< the normal return value
        cl_program          program,
        cl_uint             num_devices,
        const cl_device_id* device_list,
        const char*         options,
        void (CL_CALLBACK* pfn_notify)(cl_program program, void* user_data),
        void*               user_data);

    /// Utility  functions to extract the OpenCL driver version out of the big string
    double getOpenCLPlatformVersion();

    /// Iterate through the device names that the OpenCL driver reported, and remove the names of devices that have not been published yet.
    /// This is done only in the CodeXL public version. In CodeXL NDA and INTERNAL versions this function is no-op.
    void RemoveNamesOfUnpublishedDevices(const set<string>& uniqueNamesOfPublishedDevices);

    // Internal auxiliary function that determines if for a given device the HSAIL
    // path would be used.
    bool DoesUseHsailPath(const std::string& deviceName) const;

    /// The cracked binaries.
    std::vector<CElf*>           m_Elves;

    /// Map from device name to cracked binary.
    std::map<std::string, CElf*> m_ElvesMap;

    /// Map from device to it's compiled binary.
    std::map<std::string, std::vector<char> > m_BinDeviceMap;

    /// Map Device and kernel and it's statistics data
    std::map<std::string, std::map<std::string, beKA::AnalysisData> > m_KernelAnalysis;

    friend class Backend;

    /// Interface with OpenCL.dll/libOpenCL.so
    OpenCLModule                          m_TheOpenCLModule;

    /// Handle to the ACL module.
    ACLModule*                             m_pTheACLModule;

    /// Handle to the ACL compiler.
    aclCompiler*                           m_pTheACLCompiler;

    /// Interface with aticalcl.dll/libaticalcl.so
    CALCLModule                           m_TheCALCLModule;

    /// The OpenCL context used by this Backend.
    /// Set up in Initialize.
    cl_context                            m_OpenCLContext;

    /// The number of OpenCL devices.
    /// Set up in Initialize.
    size_t                                m_NumOpenCLDevices;

    /// The OpenCL device IDs.
    std::vector<cl_device_id>             m_OpenCLDeviceIDs;

    /// The sorted device table for OpenCL.
    std::vector<GDT_GfxCardInfo>          m_OpenCLDeviceTable;

    /// A string to be used to report OpenCL version information.
    std::string                           m_OpenCLVersionInfo;

    /// Temporary used to construct ISA string from OpenCL/CAL callback.
    /// This member shouldn't be static. To be handled in a future cleanup.
    static std::string*                   s_pISAString;

    /// Temporary used to construct IL string from OpenCL/CAL callback.
    /// This member shouldn't be static. To be handled in a future cleanup.
    static std::string                   s_HSAILDisassembly;

    /// Counter used for the disassemble callback. It is being used by the callback
    /// to differentiate between ISA and HSAIL disassembly.
    static size_t gs_DisassembleCounter;

    /// Stream for diagnostic output.
    LoggingCallBackFuncP m_LogCallback;

    /// The device names.
    set<string>                 m_DeviceNames;

    /// Map from OpenCL device ID to OpenCL device name.
    map<cl_device_id, string>   m_DeviceIdNameMap;

    /// Map from OpenCL device name to OpenCL device ID.
    map<string, cl_device_id>   m_NameDeviceIdMap;

    /// Map from OpenCL device name to device type.
    map<string, cl_device_type> m_NameDeviceTypeMap;

    bool m_IsIntialized;

    bool m_forceEnding;

    bool m_isLegacyMode;
};

#endif // _BEPROGRAMBUILDEROPENCL_H_
