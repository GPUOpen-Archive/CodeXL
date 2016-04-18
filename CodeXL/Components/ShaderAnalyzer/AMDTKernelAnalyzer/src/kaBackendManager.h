//=============================================================================
// Copyright © 2012 Advanced Micro Devices, Inc. All rights reserved.
//
/// \author GPU Developer Tools
/// \file $File: //devtools/main/CodeXL/Components/ShaderAnalyzer/AMDTKernelAnalyzer/src/kaBackendManager.h $
/// \version $Revision: #3 $
/// \brief  This file is the header file for kaBackendManager class.
//
//=============================================================================
// $Id: //devtools/main/CodeXL/Components/ShaderAnalyzer/AMDTKernelAnalyzer/src/kaBackendManager.h#3 $
// Last checkin:   $DateTime: 2016/03/27 08:08:35 $
// Last edited by: $Author: rbober $
// Change list:    $Change: 565785 $
//=============================================================================
#ifndef _BACKEND_MANAGER_H_
#define _BACKEND_MANAGER_H_

#include <list>
#include <memory>
#include <set>
#include <vector>
#include <map>
#include <queue>

using std::multimap;

#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
    #pragma warning(push)
    #pragma warning(disable : 4127)
    #pragma warning(disable : 4251)
    #pragma warning(pop)
#endif

// Qt:
#include <QtWidgets>

// Infra:
#include <AMDTBaseTools/Include/gtString.h>
#include <AMDTBaseTools/Include/gtMap.h>
#include <AMDTOSWrappers/Include/osDirectory.h>

// Local:
#include <AMDTKernelAnalyzer/Include/kaAMDTKernelAnalyzerDLLBuild.h>
#include <AMDTKernelAnalyzer/src/kaDataTypes.h>
#include <AMDTKernelAnalyzer/src/kaProgram.h>

// Backend:
#include <AMDTBackEnd/Include/beBackend.h>
#include <AMDTBackEnd/Emulator/Parser/ISAParser.h>

#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
    #include <AMDTBackEnd/Include/beProgramBuilderDX.h>
#endif

// need to undef Bool after all includes so the moc will compile in Linux
#undef Bool

typedef unsigned int UINT;

class kaProjectDataManager;
class kaSourceFile;
class ParserISA;
class UTDPScheduler;
class kaBackEndSmartPointers;

/// DX build options.
struct DXAdditionalBuildOptions
{
    // Build options mask (applicable only for D3D).
    unsigned int m_buildOptionsMask;

    // The type of the build (FXC, D3D, etc.)
    QString      m_builderType;

    // Full path of the building module.
    QString      m_builderPath;

    // Macros as the user defined.
    QString      m_additionalMacros;

    // Include directories as the user defined.
    QString      m_additionalIncludes;

    // DirectX shader build profile (e.g. vs_5_0, ps_5_0, etc.)
    QString      m_buildProfile;

    // The relevant shader entry point.
    QString      m_entryPoint;

    // Additional build options.
    QString     m_buildOptions;
};

/// OpenGL build options.
struct GLAdditionalBuildOptions
{
    /// Shader Type (Vertex, Fragment, etc.)
    gtString m_shaderType;
};

/// Manage Backend
class KA_API kaBackendManager : public QObject
{
    Q_OBJECT

    friend class kaBackEndSmartPointers;
public:
    // singleton
    static kaBackendManager& instance();

    //-----------------------------------------------------------------------------
    /// Destructor
    //-----------------------------------------------------------------------------
    virtual ~kaBackendManager();

    //-----------------------------------------------------------------------------
    /// Reset
    //-----------------------------------------------------------------------------
    void reset();

    //-----------------------------------------------------------------------------
    /// Build an OpenCL program
    /// \param[in] sourceCode                     A source code
    /// \param[in] selectedDeviceNames            A subset of device names supported
    /// \param[in] buildOptions                   A build option command
    /// \param[in] buildSourcePathAndName         A source code path and name
    /// \param[in] analyze                        An indicator if emulation analysis should be performed
    //-----------------------------------------------------------------------------
    void buildCLProgram(const QString& sourceCode,
                        const std::set<std::string>& selectedDeviceNames,
                        const QString& buildOptions,
                        const QString& buildSourcePathAndName,
                        bool analyze = false);


    //-----------------------------------------------------------------------------
    /// Build program
    /// \param[in] pProgram                       The program to build
    /// \param[in] selectedDeviceNames            A subset of device names supported
    /// \param[in] bitness                        An indicator if 64-bit build or 32-bit build should be performed
    //-----------------------------------------------------------------------------
    void PrepareProgramBuild(kaProgram* pProgram,
                             const std::set<std::string>& selectedDeviceNames,
                             AnalyzerBuildArchitecture bitness = kaBuildArch32_bit);

    //-----------------------------------------------------------------------------
    /// Build an GL program
    /// \param[in] sourceCode           A source code
    /// \param[in] target               A shader target, like "Geometry Shader"
    /// \param[in] selectedDeviceNames  A subset of device names supported, like "Cypress", "Tahiti"
    //-----------------------------------------------------------------------------
    void buildGLProgram(const QString& sourceCode,
                        const std::set<std::string>& selectedDeviceNames,
                        const QString& buildSourcePathAndName,
                        const GLAdditionalBuildOptions& additionalOptions);

    //-----------------------------------------------------------------------------
    /// Build an Vulkan program
    /// \param[in] pVulkanProgram - rendering (with staging shaders) or compute(with one compute shader)
    /// \param[in] selectedDeviceNames  A subset of device names supported, like "Cypress", "Tahiti"
    //-----------------------------------------------------------------------------
    void PrepareProgramBuildInner(kaProgram* pProgram, const BuildType buildType);

    // DX is only supported on Windows.
#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS

    //-------------------------------------------------------------------------------
    /// Add a path to be searched when looking for DX binaries.
    /// \param[in] path     The path to the directory to be searched by the DX loader
    //-------------------------------------------------------------------------------
    static void AddDxBinSearchPath(const gtString& path);

    /// Fills passed DXAdditionalBuildOptions structure with necessary build information
    bool GetCurrentFileAdditonalDXBuildOptions(DXAdditionalBuildOptions& currentFileAdditionalBuildOptions) const;
    bool PreparedDxAdditionOptions(DXAdditionalBuildOptions& additionalDxBuildOptions) const;

#endif

    //-----------------------------------------------------------------------------
    /// Get is in build state
    /// \param[in]  Status_OpenCL_MODULE_NOT_LOADED        If OpenCL Module was not loaded properly.
    //-----------------------------------------------------------------------------
    beStatus isBackendInitialized() const;


    //-----------------------------------------------------------------------------
    /// Get binary
    /// \param[in] deviceName    The name of the target device for which the binary file was built.
    /// \param[in] binFilePath   The binary ELF file that was created by the analyzer during build.
    /// \param[in] source        A suppression flag. Value false will add ".source" to options
    /// \param[in] IL            A suppression flag. Value false will add ".amdil" and ".debugil" to options
    /// \param[in] debugInfo     A suppression flag. Value false will add "" to options
    /// \param[in] llvmIR        A suppression flag. Value false will add ".llvmir" to options
    /// \param[in] isa           A suppression flag. Value false will add ".text" to options
    /// \param[out] outputBuffer Buffer that will receive the contentof the ELF file to export
    //-----------------------------------------------------------------------------
    void getBinary(const gtString& deviceName,
                   const gtString& binFilePath,
                   bool source,
                   bool IL,
                   bool debugInfo,
                   bool llvmIR,
                   bool isa,
                   std::vector<char>& outputBuffer);

    //-----------------------------------------------------------------------------
    /// Get devices names
    /// \returns   A list of device names the backend supports
    //-----------------------------------------------------------------------------
    const std::set<std::string>& getDeviceNames() const;

    //-----------------------------------------------------------------------------
    /// Find out if the backend can export anything
    /// \param[in] selectedDeviceNames  A list of selected devices
    /// \returns   true if is exportable, false otherwise
    //-----------------------------------------------------------------------------
    bool isExportable(const QStringList& selectedDeviceNames) const;

    //-----------------------------------------------------------------------------
    /// Get OpenCL version info
    /// \returns   OpenCL Version (as specified in the standard) <newline>
    ///            Driver Release (e.g. "8.97-gobbledy-gook") <newline>
    ///            Catalyst Version (e.g. "12.4")
    //-----------------------------------------------------------------------------
    const std::string& getOpenCLVersionInfo();


    //-----------------------------------------------------------------------------
    /// Callback function for backend sending messages.
    /// \param[in] message  A message.
    static void backendMessageCallback(const std::string& message);

    /// Static variable for storing backend messages
    static std::queue<std::string> g_backendMessages;

    /// Static variable for protecting g_backendMessages
    static QMutex                  g_backendMessageMutex;

    //-----------------------------------------------------------------------------
    /// Get string list that represent an ASICs tree.
    /// \param[in]  kind          One of the DeviceTableKind enum values.
    /// \param[in]  includeCPU    true for including cpu, false otherwise.
    /// \param[out] pStrListOut   A pointer to a string list to be populated.
    /// \returns Backend::Status_SUCCESS if success, error status if an error occured.
    //-----------------------------------------------------------------------------
    beStatus getASICsTreeList(beKA::DeviceTableKind  kind,
                              bool includeCPU,
                              QStringList* pStrListOut);

    //-----------------------------------------------------------------------------
    /// Get is in build state
    /// \returns   true if is in build process, false otherwise
    //-----------------------------------------------------------------------------
    bool isInBuild() const;

    enum kaStatisticsData
    {
        kaStatScratchRegs = 0,
        kaStatThreadsPerWorkGroup,
        kaStatWavefrontSize,
        kaStatMaxLDSSize,
        kaStatLDSSize,
        kaStatMaxSGPRs,
        kaStatSGPRs,
        kaStatMaxVGPRs,
        kaStatVGPRs,
        kaStatReqdWorkGroupX,
        kaStatReqdWorkGroupY,
        kaStatReqdWorkGroupZ,
        kaStatISASize
    };

    /// Get the last build file
    const QString& LastBuildClFile() { return m_lastFileBuilt; };

    /// Cancels the current build.
    void CancelBuild();

    /// Checks if current build has been canceled.
    /// \retval bool true if canceled
    bool IsBuildCancelled() const;
    /// Checks if current build has been succeeded, i.e. zero failed files.
    /// \retval bool true if succeeded
    bool IsBuildSucceded() const;
    gtVector<int> GetLastBuildProgramFileIds() const;

    /// Prints the build prologue.
    void PrintBuildPrologue(gtString& programName, size_t numOfDevicesToBuild, bool shouldPrependCR);

private:

    enum BuildoOptionsErrors
    {
        KA_BUILD_O_PARENTHESIS_ERROR = 1,
        KA_BUILD_O_WRITABLE_DIR_ERROR,
    };

    // Structure that is used to hold the analysis results:
    struct kaAnalysisResults
    {
        kaAnalysisResults() : iNumOfWavefronts(0), iSgprs(0), iVgprs(0), iCodeLen(0) {};
        ISAProgramGraph::NumOfInstructionsInCategory NumOfInstructionsInCategory[3];
        int iNumOfWavefronts;
        unsigned int iSgprs;
        unsigned int iVgprs;
        unsigned int iCodeLen;
    };

    //-----------------------------------------------------------------------------
    /// Constructor
    /// \param[in] pParent   A parent
    /// Only my instance() function can create me:
    //-----------------------------------------------------------------------------
    kaBackendManager();

    // Only kaSingletonsDelete should delete me:
    friend class kaSingletonsDelete;

    // The single instance of this class:
    static kaBackendManager* m_psMySingleInstance;

    //-----------------------------------------------------------------------------
    /// Get GL compile result
    //-----------------------------------------------------------------------------
    void getGLResult();

    //-----------------------------------------------------------------------------
    /// Outputs a string to the information view
    /// \param[in]  msg        The string to output
    void addStringToInformationView(const std::string& msg);

    //-----------------------------------------------------------------------------
    /// Get string list that represent an ASICs tree.
    /// \param[in]  deviceTable        the device table.
    /// \param[in]  includeCPU    true for including cpu, false otherwise.
    /// \param[out] pNameNameMapOut   A pointer to a map to be populated. Can be NULL.
    /// \param[out] pNameIdMapOut     A pointer to a map to be populated. Can be NULL.
    /// \param[out] pStrListOut       A pointer to a string list to be populated.
    /// \returns Backend::Status_SUCCESS if success, error status if an error occured.
    //-----------------------------------------------------------------------------
    beStatus makeASICsStringList(const std::vector<GDT_GfxCardInfo>& deviceTable,
                                 bool includeCPU,
                                 std::multimap<QString, QString>* pNameNameMapOut,
                                 std::multimap<QString, UINT>* pNameIdMapOut,
                                 QStringList* pStrListOut);

    /// Returns true if the current batch of files needs to be analyzed, and false otherwise.
    bool isAnalysisRequired() const;

    //-----------------------------------------------------------------------------
    /// Sync string list that with persistent data
    /// \param[in] source     A list which contains tree info.
    /// \param[in] pStrList   A pointer to a string list to be synchronized.
    //-----------------------------------------------------------------------------
    void syncASICsTreeListWithPersistent(QStringList& source, QStringList* pStrList);

    //-----------------------------------------------------------------------------
    /// Update the -o outout directory
    /// \param[in/out] options     build options.
    //-----------------------------------------------------------------------------
    void UpdateOBuildOption(QString& optionsStr);


    //-----------------------------------------------------------------------------
    // Convert "XXXXX.YYY" to "XXXXX DEVICENAME.YYY"
    // It is important to separate the kernel name from the deice with a space character, because this
    // is the format that the build files name parser expects
    //-----------------------------------------------------------------------------
    static void GeneralFileNameToDeviceSpecificFileName(const gtString& generalFileName,
                                                        const std::string& device,
                                                        gtString& deviceSpecificFile);

    /// A private thread for running Backend::Compile()
    class BuildThread : public QThread
    {
    public:
        //-----------------------------------------------------------------------------
        /// Constructor
        /// \param[in] kaBackendManager& A reference to the kaBackendManager program
        //-----------------------------------------------------------------------------
        BuildThread(kaBackendManager& owner);

        /// Mark the thread for force ending
        void ForceEnd();

        //-----------------------------------------------------------------------------
        /// Implement BuildThread run()
        //-----------------------------------------------------------------------------
        virtual void run();

        /// Launches an OpenCL build.
        bool BuildOpenclSourceFile(kaSourceFile* pCurrentFile,
                                   const gtString& isaFileName, const gtString& binaryFile, const gtString& ilFileName,
                                   const gtString& analysisFileName, const std::string& sourceCodeFullPathName, int& numOfSuccessfulBuilds);

        /// Launches an OpenGL shader build.
        void BuildGlslShader(kaSourceFile* pCurrentFile,
                             const gtString& isaFileName, const gtString& statisticsFileName, const std::string& sourceCodeFullPathName, int& numOfSuccessfulBuilds);

        /// Launches a Vulkan Program build.
        bool BuildRenderProgram(const gtString& buildOutputDir);

#if AMDT_BUILD_TARGET==AMDT_WINDOWS_OS
        /// Launches a DX shader build.
        bool BuildDxShader(kaSourceFile* pCurrentFile,
                           const gtString& isaFileName, const gtString& dxAsmFileName, const gtString& binFileName, const gtString& statisticsFileName,
                           const std::string& sourceCodeFullPathName, int& numOfSuccessfulBuilds, const gtString& entryPoint, const gtString& profile);
        bool LaunchDxBuild(const gtString& buildOutputDir);

#endif
    private:
        /// Prints the build epilogue.
        void PrintBuildEpilogue(int numOfBuildsOverall, int numOfSuccessfulBuilds, const int devicesBuiltCount) const;
        void UpdateStatisticsFileGroups(gtMap<gtString, gtVector<gtString>>& statisticsFilesGroups, const gtString& statisticsFileName, const std::string& device) const;

        void InitKaStageStatisticsGroups(gtMap<gtString, gtVector<gtString>>& statisticsFilesGroups) const;
        gtString GetBuildOutputDir() const;
        bool LaunchOpenCLBuild(const gtString& buildOutputDir);
        void AggregateOpenCLStatistics(kaSourceFile* pCurrentFile, const std::set<std::string>& devices, const gtString& analysisFileName) const;
        osFilePath GenerateDXStatsFName(kaSourceFile* pCurrentFile, const gtString& statisticsFileName, const std::string& device) const;


        //Members
    public:
        /// Pointer to an external Backend object.
        Backend*                m_pExternalBackend;

        /// Pointer to an external string that holds source code.
        std::string            m_pExternalSourceCode;

        /// Pointer to an external string that holds source code path.
        std::vector<std::string>*            m_pExternalSourceCodePath;

        /// Pointer to an external options.
        beKA::CompileOptions* m_pExternalOptions = nullptr;

        /// Collection of compiler options for DX.
        //std::vector<beKA::CompileOptions*> m_externalOptionsDX;

        DXAdditionalBuildOptions m_dxBuildOptions;

        /// External name ID multi map
        std::multimap<QString, UINT>* m_pExternalNameIdMap;

        /// Backend::Compile() returned status.
        beKA::beStatus m_compileStatus;

        /// Build type.
        BuildType m_buildType;

        /// A list of the target device names.
        std::set<std::string>   m_deviceNames;

        /// Build Architecture(32-bit or 64-bit).
        AnalyzerBuildArchitecture m_bitness;

        /// GL shader type.
        std::string m_glShaderType;

        /// CL kernel name.
        std::string m_kernelName;

        /// cancellation flag set from kaBackendManager
        bool m_shouldBeCanceled;
        bool m_buildSucceded = false;

        /// GL/VK shaders structure
        kaPipelineShaders m_shadersPaths;

        //holds pointer to current built programm
        std::unique_ptr<kaProgram> m_pCurrentProgram = nullptr;

        // A reference to the owning object.
        kaBackendManager& m_owner;
    };

    /// Pointer of an Backend object.
    Backend*                                  m_pBackend;

    // pointer to a class that hides boost:
    kaBackEndSmartPointers*                   m_pBoost;

    /// List of device names
    std::set<std::string>                     m_deviceNames;

    /// A string holding source code`s path.
    std::vector<std::string> m_sourceCodePaths;

    /// A string holding source code`s full path name.
    gtString m_sourceCodeFullPathName;

#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
    /// The common compiler options for DX.
    beProgramBuilderDX::DXOptions m_dxBaseOptions;
#endif

    // A map for mapping device name to market names
    std::multimap<QString, QString> m_deviceNameMaketNameMapCL;

    // A map for mapping device name to market names
    std::multimap<QString, QString> m_deviceNameMaketNameMapDX;

    // A map for mapping device name to market names
    std::multimap<QString, QString> m_deviceNameMaketNameMapGL;

    // A map for mapping device name to id
    std::multimap<QString, UINT>  m_deviceNameIDMapDX;

    /// Flag for the first time run
    bool                                      m_firstTimeRun;

    /// Current device name
    QString                                   m_currentDeviceName;

    /// Current kernel name
    QString                                   m_currentKernelName;

    /// Flag for in build state
    bool                                      m_isInBuild;

    /// List of OpenCL source files pending for build:
    QStringList                               m_sourcesPendingForBuild;
    std::set<std::string>                     m_selectedDeviceNamesForPendingBuild;
    QString                                   m_buildOptionsForPendingBuild;
    QStringList                               m_filePathsPendingForBuild;
    bool                                      m_analyzePendingBuild;
    BuiltProgramKind                          m_buildType;
    AnalyzerBuildArchitecture                 m_buildArchitecture;
    /// Last build cl file. The binaries are relevant to this file only
    QString m_lastFileBuilt;

    /// set to true if initialization of the backend manager completed successfully
    bool m_isInitialized;

    //-----------------------------------------------------------------------------
    /// General setup for a build
    /// \param[in] sourceCode   A source code for a build
    /// \param[in] fileName   file name build built
    /// \param[in] numDevices   number devices build built
    void setUpForBuild(const QString& sourceCode, const std::string& fileName, int numDevices);

    //-----------------------------------------------------------------------------
    /// Send Cal data to somewhere, likely a GUI's data table
    /// \param[in] deviceName   A device name
    /// \param[in] nameNameMap  A device name and marketing name map
    /// \returns a list of device marketing names delimited by new line char
    //-----------------------------------------------------------------------------
    QString getDeviceMarketingNames(const QString& deviceName,
                                    const std::multimap<QString, QString>& nameNameMap) const;

    //-----------------------------------------------------------------------------
    /// Print build summary
    /// \param[in] totalGoodBuild   total good build
    /// \param[in] totalBuild       total build
    //-----------------------------------------------------------------------------
    void printBuildSummary(int totalGoodBuild, int totalBuild);

public slots:

    //-----------------------------------------------------------------------------
    /// Slot function for reading log content.
    ///
    /// This function is mostly driven by a timer signal
    /// \returns   true if a message is sent to the front end, false otherwise
    //-----------------------------------------------------------------------------
    bool readLog();

    //-----------------------------------------------------------------------------
    /// Slot function for receiving compiler finished
    //-----------------------------------------------------------------------------
    void compileResultReady();

signals:

    /// Send a device's text output, name and kernel name
    /// \param[in] text           A compile result text
    /// \param[in] name           A device name
    /// \param[in] currentKernel  A name of current kernel
    void sendTextAndName(const QString& text, const QString& name,
                         const std::vector<std::string>* pKernels, const QString& currentKernel);

    /// Send a device's output text and name
    /// \param[in] text           A compile result text
    /// \param[in] name           A device name
    void sendTextAndName(const QString& text, const QString& name);

    /// Send a list decorated names when a compile is done
    /// \param[in] decoratedNames   A list of decorated names like "Cypress IL", "Cypress ISA"
    void compileResultDone(const std::vector<QString>& decoratedNames);

    /// Send a list of selected device name when a compile is done
    /// \param[in] selectedDeviceNames   A list of  names like "Cypress", "Tahiti"
    void compileResultDoneWithName(const std::vector<QString>& selectedDeviceNames);

    /// Send a list of selected device name when an analysis is done
    /// \param[in] selectedDeviceNames   A list of  names like "Cypress", "Tahiti"
    void analysisResultDoneWithName(const std::vector<QString>& selectedDeviceNames);

    /// Send data field names
    /// \param[in] names  A list of names like "WavefrontSize", "MaxLDSSize"
    void sendDataFieldNames(const std::vector<QString>& names) const;

    /// Send data field tooltips
    /// \param[in] data  A list of tooltips for data names sent in sendDataFieldNames()
    void sendDataFieldTooltips(const std::vector<QString>& data) const;

    /// Send emulator`s analysis data field names
    /// \param[in] names  A list of names like "Throughput", "Bottleneck"
    void sendAnalysisDataFieldNames(const std::vector<QString>& names) const;

    /// Send emulator`s analysis data field tooltips
    /// \param[in] data  A list of tooltips for emulator`s analysis data names sent in sendAnalysisDataFieldNames()
    void sendAnalysisDataFieldTooltips(const std::vector<QString>& data) const;

    /// Send analyzer`s input data
    /// \param[in] names  A list of analyzer`s input data components (global workgroup size,local workgroup size,
    /// branch coherence factor.
    void sendAnalyzerInputFieldData(const std::vector<QString>& data) const;

    /// Send analysis data for a table to display
    /// \param[in] data  A list of data like "0", "256"
    void sendEmulatorData(const std::vector<QString>& data)  const;

    /// Send data for a table to display
    /// \param[in] data  A list of data like "0", "256"
    void sendCalData(const std::vector<QString>& data) const;

    /// Send selected device name tooltips
    /// \param[in] tooltips  A list of device name tooltips.  The paired element has first as device name and the second as tooltip.
    void sendSelectedDeviceNameTooltips(const std::vector<std::pair<QString, QString> >& tooltips);

    /// Send selected device name tooltips
    /// \param[in] tooltips  A list of device name tooltips.  The paired element has first as device name and the second as tooltip.
    void sendAnalysisSelectedDeviceNameTooltips(const std::vector<std::pair<QString, QString> >& tooltips);

    /// Send selected IS/ISA device name tooltips
    /// \param[in] tooltips  A list of device name tooltips.  The paired element has first as device IL/ISA name and the second as tooltip.
    void sendSelectedDeviceILISANameTooltips(const std::vector<std::pair<QString, QString> >& tooltips);

    /// Send when a compilation finished
    void buildComplete(const gtString& sourceCodeFullPathName);

    /// Sends when starting compilation
    void buildStart(const gtString& sourceCodeFullPathName);

    /// Sends when a message needs to be printed to the output
    void printMessageForUser(const QString& message);

    //-----------------------------------------------------------------------------
    /// Show emulation analysis data table
    //-----------------------------------------------------------------------------
    void sendShowAnalysisData();

    //-----------------------------------------------------------------------------
    /// Hide emulation analysis data table
    //-----------------------------------------------------------------------------
    void sendHideAnalysisData();

};

#endif // _BACKEND_MANAGER_H_
