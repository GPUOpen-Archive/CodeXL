#ifndef beProgramBuilderVulkan_h__
#define beProgramBuilderVulkan_h__

// C++.
#include <string>
#include <set>

// Local.
#include <AMDTBackEnd/Include/beProgramBuilder.h>
#include <AMDTBackEnd/Include/beInclude.h>
#include <AMDTBackEnd/Include/beAMDTBackEndDllBuild.h>
#include <AMDTBackEnd/Include/beDataTypes.h>

struct VulkanOptions : public beKA::CompileOptions
{
    VulkanOptions() : m_isSpirvBinariesRequired(false), m_isAmdPalIlBinariesRequired(false),
        m_isAmdPalIlDisassemblyRequired(false), m_isAmdIsaBinariesRequired(false),
        m_isAmdIsaDisassemblyRequired(false), m_isScStatsRequired(false)
    {
        CompileOptions::m_SourceLanguage = beKA::SourceLanguage_GLSL_Vulkan;
    }

    // The target devices.
    std::string m_targetDeviceName;

    // The input shader file names.
    beProgramPipeline m_pipelineShaders;

    // AMD PAL IL binary output file names.
    beProgramPipeline m_palIlBinaryOutputFiles;

    // AMD PAL IL disassembly output files.
    beProgramPipeline m_pailIlDisassemblyOutputFiles;

    // ISA disassembly output file names.
    beProgramPipeline m_isaDisassemblyOutputFiles;

    // Register liveness analysis output file names.
    beProgramPipeline m_liveRegisterAnalysisOutputFiles;

    // Control flow graph output file names.
    beProgramPipeline m_controlFlowGraphOutputFiles;

    // ISA binary output file name.
    beProgramPipeline m_isaBinaryFiles;

    // SC statistics output file name.
    beProgramPipeline m_scStatisticsOutputFiles;

    // True to generate SPIR-V binaries.
    bool m_isSpirvBinariesRequired;

    // True to generate AMD PAL IL binaries.
    bool m_isAmdPalIlBinariesRequired;

    // True to generate AMD PAL IL disassembly.
    bool m_isAmdPalIlDisassemblyRequired;

    // True to generate AMD ISA binaries.
    bool m_isAmdIsaBinariesRequired;

    // True to generate AMD ISA binaries.
    bool m_isAmdIsaDisassemblyRequired;

    // True to perform live register analysis.
    bool m_isLiveRegisterAnalysisRequired;

    // True to generate control flow graph.
    bool m_isControlFlowGraphRequired;

    // True to generate shader compiler statistics.
    bool m_isScStatsRequired;
};

class KA_BACKEND_DECLDIR beProgramBuilderVulkan : public
    beProgramBuilder
{
public:
    beProgramBuilderVulkan();
    ~beProgramBuilderVulkan();

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

    beKA::beStatus Compile(const VulkanOptions& vulkanOptions, bool& cancelSignal, gtString& buildLog);

    /// Sets the set of public device names.
    void SetPublicDeviceNames(const std::set<std::string>& publicDeviceNames);

    /// Extracts the OpenGL version of the installed runtime.
    bool GetVulkanVersion(gtString& vkVersion);

private:

    std::set<std::string> m_publicDevices;
};

#endif // beProgramBuilderVulkan_h__
