
#ifndef __kaCliLauncher_h
#define __kaCliLauncher_h
#include <AMDTBackEnd/Include/beProgramBuilderOpenCL.h>
#if _WIN32
    #include <AMDTBackEnd/Include//beProgramBuilderDX.h>
#endif
//#include <AMDTBackEnd/Include/beProgramBuilderGL.h>
#include <AMDTKernelAnalyzer/src/kaDataTypes.h>

////-----------------------------------------------------------------------------
/// \brief Name: LaunchOpenCLSession
/// \brief Description: Launches CLI for OpenCL Build and Analysis
/// \param[in] bitness   ba32-bit or ba64-bit Build Architecture
/// \param[in] baseISAFilename  Name of ISA output file
/// \param[in] baseILFilename  Name of IL output file
/// \param[in] baseAnalysFilename  Name of analysis output file
/// \param[in] SelectedDevices set of Devices names
/// \param[in] sourceCodeFullPathName - the source code file name
/// \param[in] shouldBeCanceled - a flag that signals that the build should be canceled.
/// \param[out] cliOutput result the CLI's output
//
bool LaunchOpenCLSession(AnalyzerBuildArchitecture bitness,
                         const std::string& baseISAFilename,
                         const std::string& baseILFilename,
                         const std::string& baseAnalysFilename,
                         const std::string& binaryFilename,
                         const std::set<std::string>& SelectedDevices,
                         const std::string& sourceCodeFullPathName,
                         const bool& shouldBeCancelled,
                         std::string& cliOutput);
//-----------------------------------------------------------------------------
////-----------------------------------------------------------------------------
/// \brief Name: LaunchOpenCLSessionForDevice
/// \brief Description: Launches CLI for OpenCL Build and Analysis
/// \param[in] bitness   ba32-bit or ba64-bit Build Architecture
/// \param[in] baseISAFilename  Name of ISA output file
/// \param[in] baseILFilename  Name of IL output file
/// \param[in] baseAnalysFilename  Name of analysis output file
/// \param[in] SelectedDevices set of Devices names
/// \param[in] sourceCodeFullPathName - the source code file name
/// \param[in] buildOptions - compiler build flags
/// \param[in] shouldBeCanceled - a flag that signals that the build should be canceled.
/// \param[out] cliOutput result the CLI's output
//
bool LaunchOpenCLSessionForDevice(AnalyzerBuildArchitecture bitness,
                                  const std::string& baseISAFilename,
                                  const std::string& baseILFilename,
                                  const std::string& baseAnalysFilename,
                                  const std::string& binaryFilename,
                                  const std::string& Device,
                                  const std::string& sourceCodeFullPathName,
                                  const std::string& buildOptions,
                                  const bool& shouldBeCancelled,
                                  std::string& cliOutput);

//-----------------------------------------------------------------------------

///// \brief Name: LaunchOpenGLSession
///// \brief Description: Launches CLI for OpenGL Build and Analysis
/// \param[in] bitness   ba32-bit or ba64-bit Build Architecture
/// \param[in] baseISAFilename  Name of ISA output file
/// \param[in] baseILFilename  Name of IL output file
/// \param[in] baseAnalysFilename  Name of analysis output file
/// \param[in] SelectedDevices set of Devices names
/// \param[in] sourceCodeFullPathName - the source code file name
/// \param[in] shouldBeCanceled - a flag that signals that the build should be canceled.
/// \param[out] cliOutput result the CLI's output
bool LaunchOpenGLSession(AnalyzerBuildArchitecture bitness, const std::string& shaderType,
                         const std::string& baseISAFilename, const std::string& baseILFilename,
                         const std::string& baseAnalysFilename, const std::set<std::string>& SelectedDevices,
                         const std::string& sourceCodeFullPathName, const bool& shouldBeCancelled, std::string& cliOutput);

////-----------------------------------------------------------------------------
///// \brief Name: LaunchOpenGLSessionForDevice
///// \brief Description: Launches CLI for OpenGL Build and Analysis
/// \param[in] bitness   ba32-bit or ba64-bit Build Architecture
/// \param[in] baseISAFilename  Name of ISA output file
/// \param[in] baseILFilename  Name of IL output file
/// \param[in] baseAnalysFilename  Name of analysis output file
/// \param[in] SelectedDevices set of Devices names
/// \param[in] sourceCodeFullPathName - the source code file name
/// \param[in] shouldBeCanceled - a flag that signals that the build should be canceled.
/// \param[out] cliOutput result the CLI's output
//
bool LaunchOpenGLSessionForDevice(AnalyzerBuildArchitecture bitness,
                                  const std::string& shaderType,
                                  const std::string& baseISAFilename,
                                  const std::string& baseILFilename,
                                  const std::string& baseAnalysFilename,
                                  const std::string& SelectedDevice,
                                  const std::string& sourceCodeFullPathName,
                                  const bool& shouldBeCanceled,
                                  std::string& cliOutput);

//-----------------------------------------------------------------------------

////-----------------------------------------------------------------------------
/// \brief Name: LaunchVulkanSession
/// \brief Description: Launches CLI for OpenCL Build and Analysis
/// \param[in] bitness   ba32-bit or ba64-bit Build Architecture
/// \param[in] inputShaders   the shaders to be compiled for each rendering stage
/// \param[in] baseISAFilename  Name of ISA output file
/// \param[in] baseILFilename  Name of IL output file
/// \param[in] baseAnalysFilename  Name of analysis output file
/// \param[in] SelectedDevices set of Devices names
/// \param[in] shouldBeCanceled - a flag that signals that the build should be canceled.
/// \param[out] cliOutput result the CLI's output
//
bool LaunchVulkanSession(AnalyzerBuildArchitecture bitness,
                         const kaPipelineShaders& inputShaders,
                         const std::string& baseISAFilename,
                         const std::string& baseILFilename,
                         const std::string& baseAnalysFilename,
                         const std::string& binaryFilename,
                         const std::set<std::string>& selectedDevices,
                         const bool& shouldBeCancelled,
                         std::string& cliOutput);

//-----------------------------------------------------------------------------
////-----------------------------------------------------------------------------
/// \brief Name: LaunchVulkanSessionForDevice
/// \brief Description: Launches CLI for OpenCL Build and Analysis
/// \param[in] bitness   ba32-bit or ba64-bit Build Architecture
/// \param[in] inputShaders   the shaders to be compiled for each rendering stage
/// \param[in] baseISAFilename  Name of ISA output file
/// \param[in] baseILFilename  Name of IL output file
/// \param[in] baseAnalysFilename  Name of analysis output file
/// \param[in] device the target device
/// \param[in] shouldBeCanceled - a flag that signals that the build should be canceled.
/// \param[out] cliOutput result the CLI's output
//
bool LaunchRenderSessionForDevice(const BuildType buildType,
                                  AnalyzerBuildArchitecture bitness,
                                  const kaPipelineShaders& inputShaders,
                                  const std::string& baseISAFilename,
                                  const std::string& baseILFilename,
                                  const std::string& baseStatisticsFilename,
                                  const std::string& binaryFilename,
                                  const std::string& device,
                                  const bool& shouldCancel,
                                  std::string& cliOutput);

#if _WIN32
struct DXAdditionalBuildOptions;

/// \brief Name: LaunchDXSessionForDevice
/// \brief Description: Launches CLI for DX Build for a single device.
/// \param[in] bitness   ba32-bit or ba64-bit Build Architecture.
/// \param[in] dxTarget  - shader type.
/// \param[in] dxEntryPoint - shader entry point.
/// \param[in] buildOptions - the build options.
/// \param[in] additionalBuildOptions - additional build options.
/// \param[in] baseISAFilename - the basic ISA file name (from which device-specific ISA file names will be derived).
/// \param[in] baseILFilename - the basic IL file name (from which device-specific ISA file names will be derived).
/// \param[in] dxAsmFileName  the basic DX ASM file name (from which device-specific ISA file names will be derived).
/// \param[in] baseAnalysFilename  the basic analysis file name (from which device-specific ISA file names will be derived).
/// \param[in] selectedDevice - the device for which we would like to build.
/// \param[in] sourceCodeFullPathName - the full path to the file which contains the source code.
/// \param[in] shouldBeCanceled - a flag that signals that the build should be canceled.
/// \param[out] cliOutput result the CLI's output

bool LaunchDXSessionForDevice(AnalyzerBuildArchitecture& bitness,
                              const::string& dxTarget,
                              const std::string& dxEntryPoint,
                              const std::string& buildOptions,
                              const DXAdditionalBuildOptions& additionalBuildOptions,
                              const std::string& baseISAFilename,
                              const std::string& baseBinFName,
                              const std::string& baseILFilename,
                              const std::string& baseAnalysFilename,
                              const std::string& dxAsmFileName,
                              const std::string& selectedDevice,
                              const std::string& sourceCodeFullPathName,
                              bool isIntrinsicsEnabled,
                              const bool& shouldBeCanceled,
                              std::string& cliOutput);


#endif

#endif // __kaCliLauncher_h
