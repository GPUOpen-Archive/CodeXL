#ifndef _BEPROGRAMBUILDER_H_
#define _BEPROGRAMBUILDER_H_


// Disable warning:
#pragma warning( disable : 4996 )
#pragma warning( disable : 4251 )

//#include "Include/beBackend.h"
#include "AMDTBackend/beInclude.h"
#include <DeviceInfo.h>
class backend;


class beProgramBuilder
{
public:

    virtual ~beProgramBuilder() {};

    /// Get list of Kernels/Shaders.
    /// Must be called after Compile is successfully called.
    /// \param[in]  device  The name of the device to choose.
    /// \param[out] kernels Vector of names of Kernels/Shaders compiled.
    /// \returns            a status.
    /// If a Log stream is available, some failures may be diagnosed.
    virtual beKA::beStatus GetKernels(const std::string& device, std::vector<std::string>& kernels) = 0;

    /// Get a binary version of the program.
    /// \param[in]  program Handle to the built program.
    /// \param[in]  device  The name of the device to choose.
    /// \param[in]  binopts Options to customize the output object.
    ///                     If NULL, a complete object is returned.
    /// \param[out] binary  A place to return a reference to the binary.
    /// \returns            a status.
    /// If a Log stream is available, some failures may be diagnosed.
    virtual beKA::beStatus GetBinary(const std::string& device, const beKA::BinaryOptions& binopts, std::vector<char>& binary) = 0;

    /// Get a string for a kernel IL.
    /// \param[in]  device     The name of the device.
    /// \param[in]  kernel     The name of the kernel.
    /// \param[out] s          The output as a string.
    /// \returns               a status.
    /// If a Log stream is available, some failures may be diagnosed.
    virtual beKA::beStatus GetKernelILText(const std::string& device, const std::string& kernel, std::string& il) = 0;

    /// Get a string for a kernel ISA.
    /// \param[in]  device     The name of the device.
    /// \param[in]  kernel     The name of the kernel.
    /// \param[out] s          The output as a string.
    /// \returns               a status.
    /// If a Log stream is available, some failures may be diagnosed.
    virtual beKA::beStatus GetKernelISAText(const std::string& device, const std::string& kernel, std::string& isa) = 0;

    /// Return the statistics for a specific kernel on a certain device.
    /// \param[in]  device     The name of the device.
    /// \param[in]  kernel     The name of the kernel.
    /// \param[out] analysis   The output as a analysis.
    /// \returns               a status.
    /// If a Log stream is available, some failures may be diagnosed.
    virtual beKA::beStatus GetStatistics(const std::string& device, const std::string& kernel, beKA::AnalysisData& analysis) = 0;

    /// return true if the module was loaded and initialized properly
    virtual bool IsInitialized() = 0;

    /// release all data relevant to the previous compilations
    virtual void ReleaseProgram() = 0;

    /// retrieve all devices as got from the loaded module
    virtual beKA::beStatus GetDeviceTable(std::vector<GDT_GfxCardInfo>& table) = 0;

    /// returns true if previous compilation succeeded for the certain device. false otherwise
    virtual bool CompileOK(std::string& device) = 0;

    /// Set callback function for diagnostic output.
    /// \param[in] callback A pointer to callback function. Use nullptr to avoid output generation.
    void SetLog(LoggingCallBackFuncP callback)
    {
        m_LogCallback = callback;
    }

protected:
    /// Ctor
    //virtual beProgramBuilder() = 0;

    virtual beKA::beStatus Initialize(const std::string& sDllModule = "") = 0;

    /// Stream for diagnostic output. set externally.
    LoggingCallBackFuncP m_LogCallback;

    /// Logging callback type.
    bool LogCallBack(const std::string& theString)
    {
        bool bRet = false;

        if (m_LogCallback)
        {
            m_LogCallback(theString);
            bRet = true;
        }

        return bRet;
    }

    ///// Force the string to use native line endings.
    ///// Linux will get LF.  Windows gets CR LF.
    ///// This makes notepad and other Windows tools happier.
    //virtual void UsePlatformNativeLineEndings(std::string& text);

    void SetDriverVersion(std::string sDriverVersion) { m_DriverVersion = sDriverVersion; };
    std::string GetDriverVersion() const { return m_DriverVersion; };
    /// check if the device should be in public verion or not
    bool IsPublishedDevice(const std::string& sDevice);
    /// just make number 4 digit long by multiply with 0
    void MakeNumber4Digit(int& iTheNumber);

    friend class Backend;

    std::string m_DriverVersion;

};

#endif
