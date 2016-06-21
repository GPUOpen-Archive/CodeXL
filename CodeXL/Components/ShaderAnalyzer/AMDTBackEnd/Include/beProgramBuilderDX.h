#pragma once
#include "beProgramBuilder.h"
#include <DXXModule.h>
#include <vector>
#include <string>
#include <utility>

using namespace beKA;
class CElf;
class CElfSection;

class KA_BACKEND_DECLDIR beProgramBuilderDX : public beProgramBuilder
{
public:

    ~beProgramBuilderDX(void);

    /// Options specific to D3D/DX/DirectCompute.
    struct DXOptions : public CompileOptions
    {
        /// The file name
        std::string m_FileName;
        /// The entry point for the shader.
        std::string m_Entrypoint;

        /// A string that specifies the shader target or set of shader
        /// features to compile against. The shader target can be
        /// shader model 2, shader model 3, shader model 4, or shader
        /// model 5. In DX, the target can also be an effect type (for
        /// example, fx_4_1).  So we expect to see ps_5_0, vs_5_0 &c.
        std::string m_Target;

        /// Device name.
        std::string m_deviceName;

        /// Which chip family?  See ...\drivers\inc\asic_reg\atiid.h for the list.
        /// Currently only SI is supported.
        UINT m_ChipFamily;

        /// Which device?  See ...\drivers\inc\asic_reg\si_id.h &c. for the lists.
        UINT m_ChipRevision;

        /// Specify a register presure vs. schedule pressure option.
        /// User specified SC compilation option.
        enum BiasScheduleToMinimizeRegs
        {
            BiasScheduleToMinimizeRegsUnspecified,
            BiasScheduleToMinimizeRegsTrue,
            BiasScheduleToMinimizeRegsFalse
        } m_BiasScheduleToMinimizeRegs;

        /// Specify an IfConversion.
        /// User specified SC compilation option.
        enum IfConversionKind
        {
            IfConversionUnspecified,
            IfConversionNo,
            IfConversionGuarantee,
            IfConversionHeuristic,
            IfConversionHeuristicOgl,
            IfConversionAlways
        } m_IfConversion;

        /// Compilation flags to pass to D3DCompile.
        /// See D3DCompiler.h in the DX SDK.
        union DXFlags
        {
            DXFlags() : flagsAsInt(0) {}
            DXFlags(unsigned val) : flagsAsInt(val) {}

            unsigned int flagsAsInt;
            struct
            {
                int debug : 1; // 0
                int skip_validation : 1; // 1
                int skip_optimization : 1; // 2
                int pack_matrix_row_major : 1; // 3
                int pack_matrix_column_major : 1; // 4
                int partial_precision : 1; // 5
                int force_vs_software_no_opt : 1; // 6
                int force_ps_software_no_opt : 1; // 7
                int no_preshader : 1; // 8
                int avoid_flow_control : 1; // 9
                int prefer_flow_control : 1; // 10
                int enable_strictness : 1; // 11
                int enable_backwards_compatiblity : 1; // 12
                int ieee_strictness : 1; // 13
                int optimization_level : 2; // 14:15 (level,bits): (0,01)(1,00)(2,11)(3,10)
                int reserved16 : 1; // 16
                int reserved17 : 1; // 17
                int warnings_are_errors : 1; // 18
            } flagsAsBitField;
        } m_DXFlags;

        /// Defines to pass to D3Dompile.
        /// The pairs are (as you might expect): Symbol, Value
        /// So -DDEBUG=1 would be "DEBUG", "1"
        std::vector<std::pair<std::string, std::string> > m_defines;

        std::vector<std::string> m_includeDirectories;

        /// Save the MS Blob as text
        bool m_bDumpMSIntermediate;

    };

public: // inherited functions
    beKA::beStatus GetKernels(const std::string& device, std::vector<std::string>& kernels);
    beKA::beStatus GetBinary(const std::string& device, const beKA::BinaryOptions& binopts, std::vector<char>& binary);
    beKA::beStatus GetISABinary(const std::string& device, std::vector<char>& binary);
    beKA::beStatus GetKernelILText(const std::string& device, const std::string& kernel, std::string& il);
    beKA::beStatus GetKernelISAText(const std::string& device, const std::string& kernel, std::string& isa);
    beKA::beStatus GetStatistics(const std::string& device, const std::string& kernel, beKA::AnalysisData& analysis);
    bool IsInitialized();
    void ReleaseProgram();
    beKA::beStatus GetDeviceTable(std::vector<GDT_GfxCardInfo>& table) override;
    bool CompileOK(std::string& device);
public:
    /// compile the specified source file
    /// \param[in] sourceLanguage   specify the source language- can be HLSL or DXAsm
    /// \param[in] programSource    the string of the source code
    /// \param[in] oclOptions       the compilation options
    /// \param[in] sourcePath       additional source path
    /// \returns               a status.
    /// If a Log stream is available, some failures may be diagnosed.
    virtual beKA::beStatus Compile(beKA::SourceLanguage sourceLanguage, const std::string& programSource, const DXOptions& dxOptions);
    virtual beKA::beStatus GetIntermediateMSBlob(std::string& IntermediateMDBlob);

    /// Used when the DXASM code is generated using the FXC tool, and therefore needs to be injected to the DX builder.
    void SetIntermediateMSBlob(const std::string& intermediateMSCode);

    /// This vector will hold additional directories
    /// where DX binaries should be searched (e.g. D3D default compiler).
    /// \param[in] dir the path where DX binaries should be searched.
    void AddDxSearchDir(const std::string& dir);

    /// Extract the ISA code of a given shader as an ASCII text string.
    /// \param[in]  device   the name of the target device
    /// \param[in]  shader   the name of the shader
    /// \param[in]  shader   the name of the shader's target (PS, CS, VS, etc.)
    /// \param[out] shader   an output parameter to hold the extracted ISA text
    /// \returns               a status.
    beKA::beStatus GetDxShaderISAText(const std::string& device, const std::string& shader,
                                      const std::string& target, std::string& isaBuffer);
    beKA::beStatus GetDxShaderIL(const std::string& device, std::string& isaBuffer);
    /// Extract the size in bytes of ISA code.
    /// \param[in]  isaAsText       the ISA code as text.
    /// \param[out] sizeInBytes    the calculated size in bytes.
    /// \returns    true for success, false otherwise.
    bool GetIsaSize(const std::string& isaAsText, size_t& sizeInBytes) const;

    /// Extract the number of threads per wavefront for a given device.
    /// \param[in]  deviceName     the device in question.
    /// \param[out] wavefrontSize  the number of threads per wavefront for the device.
    /// \returns    true for success, false otherwise.
    bool GetWavefrontSize(const std::string& deviceName, size_t& wavefrontSize) const;

    /// Returns the CELF* for the given device, or nullptr if no such CELF* exists.
    /// \param[in]  deviceName - the name of the device.
    CElf* GetDeviceElf(const std::string& deviceName) const;

    std::vector<char> GetDeviceBinaryElf(const std::string& deviceName) const;

    /// Sets the set of public device names.
    void SetPublicDeviceNames(const std::set<std::string>& publicDeviceNames);

protected:
    /// Ctor
    beProgramBuilderDX();
    beKA::beStatus Initialize(const std::string& sDllModule = "");

private: // members
    /// Interface with atidxx{32,64}.dll
    /// Windows only (since DX is a Windows thing).
    AMDDXXModule                          m_TheAMDDXXModule;

    /// Interface with d3dcompiler_xx.dll
    /// Windows only (since DX is a Windows thing).
    D3DCompileModule                      m_TheD3DCompileModule;

    /// Stream for diagnostic output.
    LoggingCallBackFuncP m_LogCallback;

    bool m_bIsInitialized;

    std::vector<GDT_GfxCardInfo>          m_DXDeviceTable;
    std::set<std::string> m_publicDeviceNames;

    /// Holds additional directories where DX binaries should be searched at.
    std::vector<std::string> m_loaderSearchDirectories;


    ///alias for ELF and its binary representation
    using CelfBinaryPair = std::pair<CElf*, std::vector<char>>;

    /// Maps between a device name and its corresponding ELF pointer and Elf binary.
    std::map<std::string, CelfBinaryPair> m_compiledElf;

    /// Holds the D3D compiler's output.
    std::string m_msIntermediateText;

public:
    static std::string* s_pTranslatedProgram;
    static int* s_pipTranslatedProgramSize;

private: // functions
    /// Wrapper to deal with crashes in the driver.
    /// \param[in]     shaderInput  Input to the private interface.
    /// \param[in,out] shaderOutput Output of the private interface.
    /// \returns       S_OK if all went well.
    HRESULT AmdDxGsaCompileShaderWrapper(const struct _AmdDxGsaCompileShaderInput* shaderInput, struct _AmdDxGsaCompileShaderOutput* shaderOutput);
    beKA::beStatus CompileHLSL(const std::string& programSource, const DXOptions& dxOptions);
    beKA::beStatus CompileDXAsm(const std::string& programSource, const DXOptions& dxOptions);
    beKA::beStatus CompileDXAsmT(const std::string& programSource, const DXOptions& dxOptions);
    const CElfSection* GetISATextSection(const std::string& deviceName) const;
    const CElfSection* GetILDisassemblySection(const std::string& deviceName) const;
    std::string ToLower(const std::string& str) const;
    /// Clears the member variables which hold the build outputs.
    void ClearFormerBuildOutputs();
    void SetDeviceElf(const std::string& deviceName, const AmdDxGsaCompileShaderOutput& shaderOutput);
    bool GetDeviceElfBinPair(const std::string& deviceName, CelfBinaryPair& elfBinPair) const;

    // Friends.
    friend class Backend;
};

