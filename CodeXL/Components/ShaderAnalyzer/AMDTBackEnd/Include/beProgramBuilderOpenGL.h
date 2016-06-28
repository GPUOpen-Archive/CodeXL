#ifndef __beProgramBuilderOpenGL_h
#define __beProgramBuilderOpenGL_h

// C++.
#include <string>
#include <set>

// Local.
#include <AMDTBackEnd/Include/beProgramBuilder.h>
#include <AMDTBackEnd/Include/beDataTypes.h>
#include <AMDTBackEnd/Include/beInclude.h>

struct OpenGLOptions : public beKA::CompileOptions
{
    OpenGLOptions() : m_chipFamily(0), m_chipRevision(0), m_isAmdIsaBinariesRequired(true),
        m_isAmdIsaDisassemblyRequired(true), m_isScStatsRequired(true)
    {
        CompileOptions::m_SourceLanguage = beKA::SourceLanguage_GLSL;
    }

    // The target device's chip family.
    size_t m_chipFamily;

    // The target device's chip revision.
    size_t m_chipRevision;

    // The input shader file names.
    beProgramPipeline m_pipelineShaders;

    // ISA disassembly output file names.
    beProgramPipeline m_isaDisassemblyOutputFiles;

    // Register liveness analysis output file names.
    beProgramPipeline m_liveRegisterAnalysisOutputFiles;

    // Register control flow graph output file names.
    beProgramPipeline m_controlFlowGraphOutputFiles;

    // SC statistics output file name.
    beProgramPipeline m_scStatisticsOutputFiles;

    // ISA binary output file name.
    gtString m_programBinaryFile;

    // True to generate AMD ISA binaries.
    bool m_isAmdIsaBinariesRequired;

    // True to generate AMD ISA disassembly.
    bool m_isAmdIsaDisassemblyRequired;

    // True to perform live register analysis.
    bool m_isLiveRegisterAnalysisRequired;

    // True to perform control flow graph.
    bool m_isCfgRequired;

    // True to generate shader compiler statistics.
    bool m_isScStatsRequired;
};

class KA_BACKEND_DECLDIR beProgramBuilderOpenGL :
    public beProgramBuilder
{
public:
    beProgramBuilderOpenGL();
    virtual ~beProgramBuilderOpenGL();

    virtual beKA::beStatus GetKernels(const std::string& device, std::vector<std::string>& kernels) override;

    virtual beKA::beStatus GetBinary(const std::string& device, const beKA::BinaryOptions& binopts, std::vector<char>& binary) override;

    virtual beKA::beStatus GetKernelILText(const std::string& device, const std::string& kernel, std::string& il) override;

    virtual beKA::beStatus GetKernelISAText(const std::string& device, const std::string& kernel, std::string& isa) override;

    virtual beKA::beStatus GetStatistics(const std::string& device, const std::string& kernel, beKA::AnalysisData& analysis) override;

    virtual bool IsInitialized() override;

    virtual void ReleaseProgram() override;

    virtual beKA::beStatus GetDeviceTable(std::vector<GDT_GfxCardInfo>& table) override;

    virtual bool CompileOK(std::string& device) override;

    virtual beKA::beStatus Initialize(const std::string& sDllModule = "") override;

    beKA::beStatus Compile(const OpenGLOptions& vulkanOptions, bool& cancelSignal, gtString& compilerOutput);

    /// Sets the set of public device names.
    void SetPublicDeviceNames(const std::set<std::string>& publicDeviceNames);

    /// Extracts the OpenGL version of the installed runtime.
    bool GetOpenGLVersion(gtString& glVersion);

    /// Retrieve the device ID and Revision ID from the OpenGL backend.
    bool GetDeviceGLInfo(const std::string& deviceName, size_t& deviceFamilyId, size_t& deviceRevision) const;

private:

    std::set<std::string> m_publicDevices;
};

#endif // __beProgramBuilderOpenGL_h
