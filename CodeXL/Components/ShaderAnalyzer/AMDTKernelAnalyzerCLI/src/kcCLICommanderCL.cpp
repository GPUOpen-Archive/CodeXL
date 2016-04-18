// C++.
#include <vector>
#include <map>
#include <utility>
#include <sstream>
#include <algorithm>

// Boost.
#include <boost/algorithm/string.hpp>

// Infra.
#include <AMDTBaseTools/Include/AMDTDefinitions.h>
#include <AMDTBaseTools/Include/gtString.h>
#include <AMDTOSWrappers/Include/osFilePath.h>
#include <AMDTOSWrappers/Include/osDirectory.h>
#include <AMDTBaseTools/Include/gtAssert.h>

// Local.
#include <AMDTKernelAnalyzerCLI/src/kcCLICommanderCL.h>
#include <AMDTBackEnd//Include/beProgramBuilderOpenCL.h>
#include <AMDTKernelAnalyzerCLI/src/kcCliStringConstants.h>
#include <AMDTKernelAnalyzerCLI/src/kcFiles.h>
#include <AMDTKernelAnalyzerCLI/src/kcUtils.h>

// Analyzer
#include <AMDTKernelAnalyzer/Include/kaStringConstants.h>
#include <VersionInfo/VersionInfo.h>

// Backend.
#include <DeviceInfoUtils.h>

kcCLICommanderCL::kcCLICommanderCL() : m_isAllKernels(false)
{

}

kcCLICommanderCL::~kcCLICommanderCL()
{
    // No need to call DeleteInstance. The base class singleton performs this.
}

// Obsolete: this routine is being used in the generation of the statistics CSV file.
// It should be removed after this mechanism is refactored.
template<class T> std::string doNAFormat(T ui, T sentinal, T err, char listSeparator, bool shouldAddSeparator /*= true*/)
{
    stringstream s;

    if (ui == sentinal)
    {
        s << "n/a";

        if (shouldAddSeparator)
        {
            s << listSeparator;
        }
    }
    else if (ui == err)
    {
        s << "err";

        if (shouldAddSeparator)
        {
            s << listSeparator;
        }
    }
    else
    {
        s << ui;

        if (shouldAddSeparator)
        {
            s << listSeparator;
        }
    }

    return s.str();
}

template<class T>
static string
doNAFormat(T ui, T sentinal, T err, char listSeparator)
{
    stringstream s;

    if (ui == sentinal)
    {
        s << "n/a" << listSeparator;
    }
    else if (ui == err)
    {
        s << "err" << listSeparator;
    }
    else
    {
        s << ui << listSeparator;
    }

    return s.str();
}



bool kcCLICommanderCL::Init(const Config& config, LoggingCallBackFunc_t callback)
{
    GT_UNREFERENCED_PARAMETER(config);
    m_LogCallback = callback;

    // Initialize the backend.
    be = Backend::Instance();
    beKA::beStatus beRet = be->Initialize(BuiltProgramKind_OpenCL, callback);
    bool ret = (beRet == beKA::beStatus_SUCCESS);

    if (ret)
    {
        // Initialize the devices list.
        beRet = be->theOpenCLBuilder()->GetDevices(m_devices);
        ret = (beRet == beKA::beStatus_SUCCESS);

        // Only external (non-placeholder) and based on CXL version devices should be used.
        if (ret)
        {
            beKA::beStatus beRetInner = be->theOpenCLBuilder()->GetDeviceTable(m_table);
            ret = (beRetInner == beKA::beStatus_SUCCESS);

            if (ret)
            {
                for (vector<GDT_GfxCardInfo>::const_iterator it = m_table.begin(); it != m_table.end(); ++it)
                {
                    if ((m_devices.find(it->m_szCALName) != m_devices.end()))
                    {
                        m_externalDevices.insert(it->m_szCALName);
                    }
                }
            }
        }
    }

    return ret;
}

bool kcCLICommanderCL::Compile(const Config& config)
{
    bool bRet = false;

    // Verify that an input file was specified
    if (config.m_InputFile.size() == 0)
    {
        LogCallBack("Error: Input file must be specified.\n");
    }
    else
    {
        string sSource;
        bRet = KAUtils::ReadProgramSource(config.m_InputFile, sSource);

        if (!bRet)
        {
            std::stringstream logStream;
            logStream << "Error: Unable to read: \'" << config.m_InputFile << "\'." << endl;
            LogCallBack(logStream.str());
        }
        else
        {
            beProgramBuilderOpenCL::OpenCLOptions options;
            options.m_SourceLanguage = SourceLanguage_OpenCL;
            options.m_SelectedDevices = m_asics;
            options.m_Defines = config.m_Defines;
            options.m_OpenCLCompileOptions = config.m_OpenCLOptions;

            int numOfSuccessFulBuilds = 0;
            beKA::beStatus beRet;

            if (config.m_IncludePath.size() > 0)
            {
                beRet = be->theOpenCLBuilder()->Compile(sSource, options, config.m_InputFile, &config.m_IncludePath, numOfSuccessFulBuilds);
            }
            else
            {
                beRet = be->theOpenCLBuilder()->Compile(sSource, options, config.m_InputFile, NULL, numOfSuccessFulBuilds);
            }

            if (beRet == beKA::beStatus_SUCCESS)
            {
                bRet = true;
            }
            else
            {
                bRet = false;
            }
        }
    }

    return bRet;
}

void kcCLICommanderCL::Version(Config& config, LoggingCallBackFunc_t callback)
{
    std::stringstream s_Log;

    if (Init(config, callback))
    {
        std::wstring versionString = STRPRODUCTVER;
        std::string cxlVersion(versionString.begin(), versionString.end());

#ifdef AMDTBASETOOLS_STATIC
        s_Log << STR_GPU_PERF_STUDIO_VERSION_PREFIX << cxlVersion << std::endl;
#else
        s_Log << STR_CXL_VERSION_PREFIX << cxlVersion << std::endl;
#endif
        s_Log << STR_DRIVER_VERSION;
        std::string driverVersion;
        bool isOk = be->GetDriverVersionInfo(driverVersion);

        if (isOk)
        {
            s_Log << driverVersion << std::endl;
        }
        else
        {
            s_Log << STR_ERR_DRIVER_VERSION_EXTRACTION_FAILURE << std::endl;
        }
    }
    else
    {
        s_Log << STR_ERR_INITIALIZATION_FAILURE << std::endl;
    }

    LogCallBack(s_Log.str());
}

bool kcCLICommanderCL::InitRequestedAsicList(const Config& config)
{
    bool bRet = true;

    if (!config.m_ASICs.empty())
    {
        // Take the devices which the user selected.
        m_asics = std::set<std::string>(config.m_ASICs.begin(), config.m_ASICs.end());
    }
    else
    {
        // Take all public devices.
        m_asics = m_externalDevices;
    }

    if (!m_asics.empty())
    {
        // Start by mapping DeviceID and Marketing Name to CAL name.
        // Do this in 2 steps: find the changes, make the changes.
        // Do this because STL iterators don't handle changes while iterating.
        set<string> toBeAdded;
        set<string> toBeErased;

        for (set<string>::const_iterator asicIter = m_asics.begin(); asicIter != m_asics.end(); ++asicIter)
        {
            if (be->GetDeviceInfo(*asicIter, NULL) == beStatus_SUCCESS)
            {
                // this is a CAL name.
                continue;
            }

            // Try to find a DeviceID
            // First check for a valid hex number.
            if (asicIter->find_first_not_of("0123456789abcdefABCDEF") == string::npos)
            {
                istringstream idStringStream(*asicIter);
                size_t id;
                idStringStream >> hex >> id;

                GDT_GfxCardInfo info;

                // TODO: should look up by device id/ rev id pair
                if (be->GetDeviceInfo(id, info) == beStatus_SUCCESS)
                {
                    toBeErased.insert(*asicIter);
                    toBeAdded.insert(info.m_szCALName);
                    continue;
                }
            }

            // There are duplicate marketing names with different ASICs.
            // I think the right thing to do is add all of the possible ASICs.
            vector<GDT_GfxCardInfo> cardInfo;

            if (be->GetDeviceInfoMarketingName(*asicIter, cardInfo) == beStatus_SUCCESS)
            {
                toBeErased.insert(*asicIter);

                for (vector<GDT_GfxCardInfo>::const_iterator infoIter = cardInfo.begin(); infoIter != cardInfo.end(); ++infoIter)
                {
                    toBeAdded.insert(infoIter->m_szCALName);
                }

                continue;
            }
        }

        // Now that we have finished walking "asics", we can erase and add things.
        for (set<string>::const_iterator asicIter = toBeErased.begin(); asicIter != toBeErased.end(); ++asicIter)
        {
            m_asics.erase(*asicIter);
        }

        for (set<string>::const_iterator asicIter = toBeAdded.begin(); asicIter != toBeAdded.end(); ++asicIter)
        {
            m_asics.insert(*asicIter);
        }

        for (set<string>::const_iterator asicIter = m_asics.begin(); asicIter != m_asics.end(); ++asicIter)
        {
            if (m_externalDevices.find(*asicIter) == m_externalDevices.end())
            {
                std::stringstream s_Log;
                s_Log << "Error: '" << *asicIter << "' is not a known device ASIC." << endl;
                s_Log << "  Use --list-asics option to find known device ASICs." << endl;
                LogCallBack(s_Log.str());
                bRet = false;
            }
        }
    }

    return bRet;
}

void kcCLICommanderCL::ListAsics(Config& config, LoggingCallBackFunc_t callback)
{
    if (!Init(config, callback))
    {
        return;
    }

    if (!config.m_bVerbose)
    {
        std::stringstream ss;
        ss << "Devices:" << endl;
        LogCallBack(ss.str());

        for (set<string>::const_iterator devIter = m_externalDevices.begin(); devIter != m_externalDevices.end(); ++devIter)
        {
            std::stringstream sss;
            sss << "   " << *devIter << endl;
            LogCallBack(sss.str());
        }
    }
    else
    {
        // Some headings:
        std::stringstream s_Log;
        s_Log << "Devices:" << endl;
        s_Log << "-------------------------------------" << endl;
        s_Log << "Hardware Generation:" << endl;
        s_Log << "    ASIC name" << endl;
        s_Log << "            DeviceID   Marketing Name" << endl;
        s_Log << "-------------------------------------" << endl;

        // emit the table.
        GDT_HW_GENERATION gen = GDT_HW_GENERATION_NONE;
        string calName;

        for (vector<GDT_GfxCardInfo>::const_iterator it = m_table.begin(); it != m_table.end(); ++it)
        {
            if (gen != it->m_generation)
            {
                gen = it->m_generation;
                std::string sHwGenDisplayName;
                AMDTDeviceInfoUtils::Instance()->GetHardwareGenerationDisplayName(gen, sHwGenDisplayName);

                switch (gen)
                {
                    case GDT_HW_GENERATION_SOUTHERNISLAND:
                        s_Log << sHwGenDisplayName << KA_STR_familyNameSICards ":" << endl;
                        break;

                    case GDT_HW_GENERATION_SEAISLAND:
                        s_Log << sHwGenDisplayName << KA_STR_familyNameCICards ":" << endl;
                        break;

                    case GDT_HW_GENERATION_VOLCANICISLAND:
                        s_Log << sHwGenDisplayName << KA_STR_familyNameVICards ":" << endl;
                        break;

                    default:
                        // You must have a new hardware generation.
                        // Add code...
                        GT_ASSERT_EX(false, L"Unknown hardware generation.");
                        break;
                }
            }

            if (calName != string(it->m_szCALName))
            {
                calName = string(it->m_szCALName);
                s_Log << "    " << calName << endl;
            }

            ostringstream ss;
            ss << hex << it->m_deviceID;
            s_Log << "            " << ss.str() << "       " << string(it->m_szMarketingName) << endl;
        }

        bool foundNewAsic = false;

        for (set<string>::const_iterator devIter = m_externalDevices.begin(); devIter != m_externalDevices.end(); ++devIter)
        {
            if (be->GetDeviceInfo(*devIter, NULL) == beStatus_SUCCESS)
            {
                // Already processed this one.
                continue;
            }

            // Skip CPU devices here.
            // We do them below.
            cl_device_type deviceType;

            if (be->theOpenCLBuilder()->GetDeviceType(*devIter, deviceType) == beStatus_SUCCESS &&
                deviceType == CL_DEVICE_TYPE_CPU)
            {
                continue;
            }

            if (!foundNewAsic)
            {
                foundNewAsic = true;
                s_Log << "New ASICs:" << endl
                      << "(This happens when your AMD Radeon Software drivers are newer than this tool." << endl
                      << " Please check for updates to AMD APP KernelAnalyzer2." << endl;
            }

            s_Log << "   " << *devIter << endl;
        }

        // Print out the CPU device
        bool foundCPU = false;

        for (set<string>::const_iterator devIter = m_externalDevices.begin(); devIter != m_externalDevices.end(); ++devIter)
        {
            cl_device_type deviceType;

            if (be->theOpenCLBuilder()->GetDeviceType(*devIter, deviceType) == beStatus_SUCCESS &&
                deviceType != CL_DEVICE_TYPE_CPU)
            {
                continue;
            }

            if (!foundCPU)
            {
                foundCPU = true;
                s_Log << "CPU devices:" << endl;
            }

            s_Log << "   " << *devIter << endl;
        }

        LogCallBack(s_Log.str());
    }

}

void kcCLICommanderCL::RunCompileCommands(const Config& config, LoggingCallBackFunc_t callback)
{
    if (Init(config, callback))
    {
        if (InitRequestedAsicList(config))
        {
            if (Compile(config))
            {
                InitRequiredKernels(config, m_asics, m_requiredKernels);

                ListKernels(config);

                GetBinary(config);

                GetISAText(config);

                GetILText(config);

                Analysis(config);

                GetMetadata(config);

                GetDebugIL(config);
            }
        }
    }
}

void kcCLICommanderCL::Analysis(const Config& config)
{
    if (config.m_AnalysisFile.size() == 0)
    {
        return;
    }

    if (config.m_Function.size() == 0)
    {
        std::stringstream s_Log;
        s_Log << "Error: Must specify kernel name to get Analysis text" << endl;
        LogCallBack(s_Log.str());
    }

    if ((config.m_SuppressSection.size() > 0) && (config.m_BinaryOutputFile.size() == 0))
    {
        LogCallBack("Warning: --suppress option is valid only with output binary\n");
    }

    // Get the separator for CSV list items.
    char csvSeparator = kcUtils::GetCsvSeparator(config);

    // Get analysis for devices.
    for (const std::string& deviceName : m_asics)
    {
        for (const std::string& kernelName : m_requiredKernels)
        {
            // Show the analysis only for external devices.
            if (m_externalDevices.find(deviceName) == m_externalDevices.end())
            {
                continue;
            }

            beStatus status;

            // Only do GPU devices.
            cl_device_type deviceType;
            status = be->theOpenCLBuilder()->GetDeviceType(deviceName, deviceType);

            if (status != beStatus_SUCCESS ||
                deviceType != CL_DEVICE_TYPE_GPU)
            {
                std::stringstream s_Log;
                s_Log << "Info: Skipping analysis of CPU device '" << deviceName << "'." << endl;
                LogCallBack(s_Log.str());
                continue;
            }

            AnalysisData analysis;
            (void)memset(&analysis, 0, sizeof(analysis));
            status = be->theOpenCLBuilder()->GetStatistics(deviceName, kernelName, analysis);

            if (status != beStatus_SUCCESS)
            {
                if (status == beSattus_WrongKernelName)
                {
                    std::stringstream s_Log;
                    s_Log << "Info: Skipping analysis, wrong kernel name provided: '" << config.m_Function << "'." << endl;
                    LogCallBack(s_Log.str());
                }

                continue;
            }

            // Create the output file.
            ofstream output;
            gtString statsOutputFileName;
            kcUtils::ConstructOutputFileName(config.m_AnalysisFile, KC_STR_DEFAULT_STATISTICS_SUFFIX, kernelName, deviceName, statsOutputFileName);
            osFilePath analysisOutputPath;
            analysisOutputPath.setFullPathFromString(statsOutputFileName);
            osDirectory targetDir;
            analysisOutputPath.getFileDirectory(targetDir);
            gtString targetDirAsStr = targetDir.directoryPath().asString(true);
            analysisOutputPath.setFileDirectory(targetDirAsStr);

            // Create the target directory if it does not exist.
            if (!targetDir.exists())
            {
                bool isTargetDirCreated(targetDir.create());

                if (!isTargetDirCreated)
                {
                    std::stringstream errMsg;
                    errMsg << STR_ERR_CANNOT_FIND_OUTPUT_DIR << std::endl;
                    LogCallBack(errMsg.str());
                }
            }

            output.open(analysisOutputPath.asString().asASCIICharArray());

            if (!output.is_open())
            {
                std::stringstream s_Log;
                s_Log << "Error: Unable to open " << config.m_AnalysisFile << " for write.\n";
                LogCallBack(s_Log.str());
            }
            else
            {
                // Write the headers.
                output << kcUtils::GetStatisticsCsvHeaderString(csvSeparator) << std::endl;

                // Write a line of CSV.
                output << deviceName << csvSeparator;
                output << analysis.maxScratchRegsNeeded << csvSeparator;
                output << analysis.numThreadPerGroup << csvSeparator;
                output << doNAFormat(analysis.wavefrontSize, CAL_NA_Value_64, CAL_ERR_Value_64, csvSeparator);
                output << analysis.LDSSizeAvailable << csvSeparator;
                output << analysis.LDSSizeUsed << csvSeparator;
                output << doNAFormat(analysis.numSGPRsAvailable, CAL_NA_Value_64, CAL_ERR_Value_64, csvSeparator);
                output << doNAFormat(analysis.numSGPRsUsed, CAL_NA_Value_64, CAL_ERR_Value_64, csvSeparator);
                output << doNAFormat(analysis.numVGPRsAvailable, CAL_NA_Value_64, CAL_ERR_Value_64, csvSeparator);
                output << doNAFormat(analysis.numVGPRsUsed, CAL_NA_Value_64, CAL_ERR_Value_64, csvSeparator);
                output << doNAFormat(analysis.numThreadPerGroupX, CAL_NA_Value_64, CAL_ERR_Value_64, csvSeparator);
                output << doNAFormat(analysis.numThreadPerGroupY, CAL_NA_Value_64, CAL_ERR_Value_64, csvSeparator);
                output << doNAFormat(analysis.numThreadPerGroupZ, CAL_NA_Value_64, CAL_ERR_Value_64, csvSeparator);
                output << doNAFormat(analysis.ISASize, (CALuint64)0, CAL_ERR_Value_64, csvSeparator, false);
                output << std::endl;

                // Close the output file.
                output.close();
            }
        }
    }
}

void kcCLICommanderCL::ListKernels(const Config& config)
{
    if (config.m_RequestedCommand != Config::ccListKernels)
    {
        return;
    }

    vector<string> kernels;

    for (set<string>::const_iterator devIter = m_asics.begin(); devIter != m_asics.end(); ++devIter)
    {
        be->theOpenCLBuilder()->GetKernels(*devIter, kernels);

        if (kernels.size() == 0)
        {
            continue;
        }

        std::stringstream s_Log;
        s_Log << *devIter << ":" << endl;

        for (vector<string>::const_iterator kernelIter = kernels.begin(); kernelIter != kernels.end(); ++kernelIter)
        {
            s_Log << "   " << *kernelIter << endl;
        }

        LogCallBack(s_Log.str());
        kernels.clear();
    }
}

void kcCLICommanderCL::GetILText(const Config& config)
{
    if (!config.m_ILFile.empty())
    {
        if (!config.m_ILFile.empty() && !config.m_Function.empty() && be != nullptr)
        {
            beProgramBuilderOpenCL* pBuilder = be->theOpenCLBuilder();

            if (pBuilder != nullptr)
            {
                if ((config.m_SuppressSection.size() > 0) && (config.m_BinaryOutputFile.size() == 0))
                {
                    // Print the warning message.
                    std::stringstream msg;
                    msg << STR_WRN_CL_SUPPRESS_WIHTOUT_BINARY << std::endl;
                    LogCallBack(msg.str());;
                }

                // This variable will hold the IL text.
                std::string ilTextBuffer;

                // Get IL text and make output files.
                for (const std::string& deviceName : m_asics)
                {
                    for (const std::string& kernelName : m_requiredKernels)
                    {
                        beKA::beStatus status = pBuilder->GetKernelILText(deviceName, kernelName, ilTextBuffer);

                        if (status == beStatus_SUCCESS)
                        {
                            gtString ilOutputFileName;
                            kcUtils::ConstructOutputFileName(config.m_ILFile,
                                                             KC_STR_DEFAULT_AMD_IL_SUFFIX, kernelName, deviceName, ilOutputFileName);
                            KAUtils::WriteTextFile(ilOutputFileName.asASCIICharArray(), ilTextBuffer, m_LogCallback);
                        }
                        else
                        {
                            // Inform the user.
                            std::stringstream msg;
                            msg << STR_ERR_CANNOT_DISASSEMBLE_AMD_IL << " for " << deviceName << "(kernel: " << kernelName << ")." << std::endl;
                            m_LogCallback(msg.str().c_str());
                        }

                        // Clear the output buffer.
                        ilTextBuffer.clear();
                    }
                }
            }
        }
    }
}

void kcCLICommanderCL::GetISAText(const Config& config)
{
    if (config.m_ISAFile.size() > 0)
    {
        if ((config.m_SuppressSection.size() > 0) && (config.m_BinaryOutputFile.size() == 0))
        {
            // Print the warning message.
            std::stringstream msg;
            msg << STR_WRN_CL_SUPPRESS_WIHTOUT_BINARY << std::endl;
            LogCallBack(msg.str());;
        }

        beProgramBuilderOpenCL* pClBuilder = be->theOpenCLBuilder();

        if (pClBuilder != NULL)
        {
            std::string sISAIL;
            bool shouldCreateSubDirectories = m_isAllKernels && !m_requiredKernels.empty();
            GT_UNREFERENCED_PARAMETER(shouldCreateSubDirectories);

            // Get ISA text and make output files.
            for (const std::string& deviceName : m_asics)
            {
                for (const std::string& kernelName : m_requiredKernels)
                {
                    beKA::beStatus status = pClBuilder->GetKernelISAText(deviceName, kernelName, sISAIL);

                    if (status == beStatus_SUCCESS)
                    {
                        gtString isaOutputFileName;
                        kcUtils::ConstructOutputFileName(config.m_ISAFile, KC_STR_DEFAULT_ISA_SUFFIX, kernelName, deviceName, isaOutputFileName);
                        KAUtils::WriteTextFile(isaOutputFileName.asASCIICharArray(), sISAIL, m_LogCallback);

                        // Perform live register analysis.
                        bool isRegLivenessRequired = !config.m_LiveRegisterAnalysisFile.empty();

                        if (isRegLivenessRequired)
                        {
                            gtString liveRegAnalysisOutputFileName;
                            kcUtils::ConstructOutputFileName(config.m_LiveRegisterAnalysisFile, KC_STR_DEFAULT_LIVE_REG_ANALYSIS_SUFFIX,
                                                             kernelName, deviceName, liveRegAnalysisOutputFileName);

                            // Call the kcUtils routine to analyze <generatedFileName> and write
                            // the analysis file.
                            kcUtils::PerformLiveRegisterAnalysis(isaOutputFileName, liveRegAnalysisOutputFileName,
                                                                 m_LogCallback);
                        }

                        // Generate control flow graph.
                        bool isCfgRequired = !config.m_ControlFlowGraphFile.empty();

                        if (isCfgRequired)
                        {
                            gtString cfgOutputFileName;
                            kcUtils::ConstructOutputFileName(config.m_ControlFlowGraphFile, KC_STR_DEFAULT_CFG_SUFFIX,
                                                             kernelName, deviceName, cfgOutputFileName);

                            // Call the kcUtils routine to analyze <generatedFileName> and write
                            // the analysis file.
                            kcUtils::PerformLiveRegisterAnalysis(isaOutputFileName, cfgOutputFileName,
                                                                 m_LogCallback);
                        }
                    }
                    else
                    {
                        // Inform the user.
                        std::stringstream msg;
                        msg << STR_ERR_CANNOT_DISASSEMBLE_ISA << " for " << deviceName << "(kernel: " << kernelName << ")." << std::endl;
                        m_LogCallback(msg.str().c_str());
                    }

                    // Clear the output buffer.
                    sISAIL.clear();
                }
            }
        }
    }
}

void kcCLICommanderCL::GetBinary(const Config& config)
{
    if (config.m_BinaryOutputFile.size() > 0 && be != nullptr)
    {
        beProgramBuilderOpenCL* pBuilder = be->theOpenCLBuilder();

        if (pBuilder != nullptr)
        {
            // Create binary output files.
            BinaryOptions binopts;
            binopts.m_SuppressSection = config.m_SuppressSection;

            std::vector<char> binary;

            for (const std::string& deviceName : m_asics)
            {
                beStatus status = pBuilder->GetBinary(deviceName, binopts, binary);

                if (status == beStatus_SUCCESS)
                {
                    gtString binOutputFileName;
                    kcUtils::ConstructOutputFileName(config.m_BinaryOutputFile,
                                                     KC_STR_DEFAULT_BIN_SUFFIX, "", deviceName, binOutputFileName);
                    KAUtils::WriteBinaryFile(binOutputFileName.asASCIICharArray(), binary, m_LogCallback);
                }
                else
                {
                    // Inform the user.
                    std::stringstream msg;
                    msg << STR_ERR_CANNOT_EXTRACT_BINARIES << " for " << deviceName << "." << std::endl;
                    m_LogCallback(msg.str().c_str());
                }

                // Clear the output buffer.
                binary.clear();
            }
        }
    }
}

void kcCLICommanderCL::GetMetadata(const Config& config)
{
    if (config.m_MetadataFile.size() > 0 && be != nullptr)
    {
        if ((config.m_SuppressSection.size() > 0) && (config.m_BinaryOutputFile.size() == 0))
        {
            // Print the warning message.
            std::stringstream msg;
            msg << STR_WRN_CL_SUPPRESS_WIHTOUT_BINARY << std::endl;
            LogCallBack(msg.str());;
        }

        beProgramBuilderOpenCL* pBuilder = be->theOpenCLBuilder();

        if (pBuilder != nullptr)
        {
            // Create the meta-data output files.
            std::string metaDataText;

            for (const std::string& deviceName : m_asics)
            {
                for (const std::string& kernelName : m_requiredKernels)
                {
                    beStatus status = pBuilder->GetKernelMetaDataText(deviceName, kernelName, metaDataText);

                    if (status == beStatus_SUCCESS)
                    {
                        gtString metaDataOutputFileName;
                        kcUtils::ConstructOutputFileName(config.m_MetadataFile, KC_STR_DEFAULT_METADATA_SUFFIX,
                                                         kernelName, deviceName, metaDataOutputFileName);
                        KAUtils::WriteTextFile(metaDataOutputFileName.asASCIICharArray(), metaDataText, m_LogCallback);
                    }
                    else
                    {
                        // Inform the user.
                        std::stringstream msg;
                        msg << STR_ERR_CANNOT_EXTRACT_META_DATA << " for " << deviceName << "(kernel: " << kernelName << ")." << std::endl;
                        m_LogCallback(msg.str().c_str());
                    }

                    // Clear the output buffer.
                    metaDataText.clear();
                }
            }
        }
    }
}

void kcCLICommanderCL::GetDebugIL(const Config& config)
{
    if (config.m_DebugILFile.size() > 0 && be != nullptr)
    {
        beProgramBuilderOpenCL* pBuilder = be->theOpenCLBuilder();

        if (pBuilder != nullptr)
        {
            if ((config.m_SuppressSection.size() > 0) && (config.m_BinaryOutputFile.size() == 0))
            {
                // Print the warning message.
                std::stringstream msg;
                msg << STR_WRN_CL_SUPPRESS_WIHTOUT_BINARY << std::endl;
                LogCallBack(msg.str());
            }

            // Create debug IL output files.
            for (const std::string& deviceName : m_asics)
            {
                for (const std::string& kernelName : m_requiredKernels)
                {
                    std::string text;
                    beStatus status = pBuilder->GetKernelDebugILText(deviceName, kernelName, text);

                    if (status == beStatus_SUCCESS)
                    {
                        gtString debugIlOutputFileName;
                        kcUtils::ConstructOutputFileName(config.m_MetadataFile, KC_STR_DEFAULT_DEBUG_IL_SUFFIX,
                                                         kernelName, deviceName, debugIlOutputFileName);
                        KAUtils::WriteTextFile(debugIlOutputFileName.asASCIICharArray(), text, m_LogCallback);
                    }
                    else
                    {
                        // Inform the user.
                        std::stringstream msg;
                        msg << STR_ERR_CANNOT_EXTRACT_DEBUG_IL << " for " << deviceName << "(kernel: " << kernelName << ")." << std::endl;
                        m_LogCallback(msg.str().c_str());
                    }
                }
            }
        }
    }
}

void kcCLICommanderCL::InitRequiredKernels(const Config& config, const std::set<std::string>& requiredDevices, std::vector<std::string>& requiredKernels)
{
    requiredKernels.clear();

    if (!requiredDevices.empty())
    {
        // We only need a single device name.
        std::set<std::string>::const_iterator firstDevice = requiredDevices.begin();
        const std::string& deviceName = *firstDevice;
        std::string requestedKernel = config.m_Function;
        std::transform(requestedKernel.begin(), requestedKernel.end(), requestedKernel.begin(), ::tolower);
        m_isAllKernels = (requestedKernel.compare("all") == 0);
        beProgramBuilderOpenCL* pClBuilder = be->theOpenCLBuilder();

        if (pClBuilder != NULL)
        {
            if (m_isAllKernels)
            {
                pClBuilder->GetKernels(deviceName, requiredKernels);
            }
            else
            {
                requiredKernels.push_back(config.m_Function);
            }
        }
    }
}
