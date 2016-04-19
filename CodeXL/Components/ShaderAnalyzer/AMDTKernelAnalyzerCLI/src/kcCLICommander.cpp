#include <map>
#include <utility>
#include <sstream>
#include <boost/algorithm/string.hpp>

#include <AMDTKernelAnalyzerCLI/src/kcCLICommander.h>
#include <AMDTBackEnd//Include/beProgramBuilderOpenCL.h>
#include <DeviceInfo.h>
#include <GDT_Assert.h>
#include <Locale/LocaleSetting.h>
#include <VersionInfo/VersionInfo.h>
#include <AMDTKernelAnalyzerCLI/src/kcFiles.h>

#include <vector>

kcCLICommander::kcCLICommander()
{

}

kcCLICommander::~kcCLICommander()
{
    be->DeleteInstance();
}

bool kcCLICommander::Init(Config& config, LoggingCallBackFuncP callback)
{
    bool bCont = true;

    m_LogCallback = callback;

    // analyze what are we working on
    // Danana- todo: need to decide how to know. not all commands requires file (like -l)
    BuiltProgramKind programKind = BuiltProgramKind_OpenCL;

    if (config.m_InputFile.size() > 0)
    {
        if (config.m_InputFile.find(".cl") != std::string::npos)
        {
            programKind = BuiltProgramKind_OpenCL;
        }
        else if (config.m_InputFile.find(".dx") != std::string::npos)
        {
            programKind = BuiltProgramKind_DX;
        }
    }

    // initialize backend
    be = Backend::Instance();

    if (!be->Initialize(programKind, callback))
    {
        bCont = false;
    }

    // initialize devices list
    if (bCont)
    {
        beKA::beStatus beRet = be->theOpenCLBuilder()->GetDevices(&m_devices);

        if (beRet == beKA::beStatus_SUCCESS)
        {
            bCont = true;
        }
        else
        {
            bCont = false;
        }
    }

    // Only external (non-placeholder) and based on CXL version devices should be used.
    if (bCont)
    {
        beKA::beStatus beRet = be->theOpenCLBuilder()->GetDeviceTable(&m_table);

        for (vector<GDT_GfxCardInfo>::const_iterator it = m_table->begin(); it != m_table->end(); ++it)
        {
            if (m_devices->find(it->m_szCALName) != m_devices->end())
            {
                m_externalDevices.insert(it->m_szCALName);
            }
        }

        if (beRet == beKA::beStatus_SUCCESS)
        {
            bCont = true;
        }
        else
        {
            bCont = false;
        }
    }

    return bCont;
}

bool kcCLICommander::Compile(Config& config)
{
    string sSource;
    bool bRet = KAUtils::ReadProgramSource(config.m_InputFile, sSource);

    if (!bRet)
    {
        std::stringstream s_Log;
        s_Log << "Error: Unable to read: \'" << config.m_InputFile << "\'." << endl;
        LogCallBack(s_Log.str());
    }
    else
    {
        beProgramBuilderOpenCL::OpenCLOptions options;
        options.m_SourceLanguage = SourceLanguage_OpenCL;
        options.m_SelectedDevices = m_asics;
        options.m_Defines = config.m_Defines;
        options.m_OpenCLCompileOptions = config.m_OpenCLOptions;
        // danana- what about it
        string sourceCodePath;
        beKA::beStatus beRet = be->theOpenCLBuilder()->Compile(sSource, options, &sourceCodePath);

        if (beRet == beKA::beStatus_SUCCESS)
        {
            bRet = true;
        }
        else
        {
            bRet = false;
        }
    }

    return bRet;
}

void kcCLICommander::ListKernels(Config& config, LoggingCallBackFuncP callback)
{
    if (!Init(config, callback))
    {
        return;
    }

    if (!InitRequestedAsicList(config))
    {
        return;
    }

    if (!Compile(config))
    {
        return;
    }

    vector<string> kernels;

    const set<string>& requestedDevices = m_asics.empty() ? *m_devices : m_asics;

    for (set<string>::const_iterator devIter = requestedDevices.begin(); devIter != requestedDevices.end(); ++devIter)
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

void kcCLICommander::GetBinary(Config& config, LoggingCallBackFuncP callback)
{
    if (!Init(config, callback))
    {
        return;
    }

    if (!InitRequestedAsicList(config))
    {
        return;
    }

    if (!Compile(config))
    {
        return;
    }

    std::vector<char> binary;
    const set<string>& requestedDevices = m_asics.empty() ? *m_devices : m_asics;

    // Make binary output files.
    for (set<string>::const_iterator devIter = requestedDevices.begin(); devIter != requestedDevices.end(); ++devIter)
    {
        BinaryOptions binopts;
        binopts.m_SuppressSection = config.m_SuppressSection;
        beStatus status = be->theOpenCLBuilder()->GetBinary(*devIter, binopts, binary);

        if (status == beStatus_SUCCESS)
        {
            std::stringstream s_Log;
            KAUtils::WriteBinaryFile(s_Log, config.m_BinaryOutputFile, binary, *devIter);
            LogCallBack(s_Log.str());
        }

        binary.clear();
    }
}

void kcCLICommander::GetISAText(Config& config, LoggingCallBackFuncP callback)
{
    if (!Init(config, callback))
    {
        return;
    }

    if (!InitRequestedAsicList(config))
    {
        return;
    }

    if (!Compile(config))
    {
        return;
    }

    string sISAIL;
    const set<string>& requestedDevices = m_asics.empty() ? *m_devices : m_asics;

    // Get ISA text and make output files.
    for (set<string>::const_iterator devIter = requestedDevices.begin(); devIter != requestedDevices.end(); ++devIter)
    {

        beKA::beStatus status = be->theOpenCLBuilder()->GetKernelISAText(*devIter, config.m_Function, sISAIL);

        if (status == beStatus_SUCCESS)
        {
            std::stringstream s_Log;
            KAUtils::WriteTextFile(s_Log, config.m_ISAFile, "amdisa", sISAIL, *devIter);
            LogCallBack(s_Log.str());
        }

        sISAIL.clear();
    }
}


void kcCLICommander::GetILText(Config& config, LoggingCallBackFuncP callback)
{
    if (!Init(config, callback))
    {
        return;
    }

    if (!InitRequestedAsicList(config))
    {
        return;
    }

    if (!Compile(config))
    {
        return;
    }

    string psISAIL;

    // Get IL text and make output files.
    const set<string>& requestedDevices = m_asics.empty() ? *m_devices : m_asics;

    for (set<string>::const_iterator devIter = requestedDevices.begin(); devIter != requestedDevices.end(); ++devIter)
    {
        string IL;
        beKA::beStatus status = be->theOpenCLBuilder()->GetKernelILText(*devIter, config.m_Function, psISAIL);

        if (status == beStatus_SUCCESS)
        {
            std::stringstream s_Log;
            KAUtils::WriteTextFile(s_Log, config.m_ILFile, "amdil", psISAIL, *devIter);
            LogCallBack(s_Log.str());
        }

        psISAIL.clear();
    }
}

void kcCLICommander::Version(Config& config, LoggingCallBackFuncP callback)
{
    if (!Init(config, callback))
    {
        return;
    }

    const std::string& OpenCLVersionStrings = be->theOpenCLBuilder()->GetOpenCLVersionInfo();

    std::stringstream s_Log;
    s_Log << "CodeXL KernelAnalyzer CLI Version " << STRPRODUCTVER << endl;
    s_Log << OpenCLVersionStrings << endl;
    LogCallBack(s_Log.str());
}


bool kcCLICommander::InitRequestedAsicList(Config& config)
{
    bool bRet = true;
    // general validation. need to be done for all commands that requires ASIC param.
    // Validate requested asics against available devices.
    vector<string>::iterator iter = config.m_ASICs.begin();

    for (; iter != config.m_ASICs.end(); ++iter)
    {
        m_asics.insert(*iter);
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

                if (be->GetDeviceInfo(id, info) == beStatus_SUCCESS)
                {
                    toBeErased.insert(*asicIter);
                    toBeAdded.insert(info.m_szCALName);
                    continue;
                }
            }

            // There are duplicate marketing names with different ASICs.
            // I think the right thing to do is add all of the possible ASICs.
            const vector<GDT_GfxCardInfo>* cardInfo;

            if (be->GetDeviceInfoMarketingName(*asicIter, &cardInfo) == beStatus_SUCCESS)
            {
                toBeErased.insert(*asicIter);

                for (vector<GDT_GfxCardInfo>::const_iterator infoIter = cardInfo->begin(); infoIter != cardInfo->end(); ++infoIter)
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

void kcCLICommander::ListAsics(Config& config, LoggingCallBackFuncP callback)
{

    if (! config.m_bVerbose)
    {
        std::stringstream ss;
        ss << "Devices:" << endl;
        LogCallBack(ss.str());

        for (set<string>::const_iterator devIter = m_externalDevices.begin(); devIter != m_externalDevices.end(); ++devIter)
        {
            std::stringstream ss;
            ss << "   " << *devIter << endl;
            LogCallBack(ss.str());
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

        for (vector<GDT_GfxCardInfo>::const_iterator it = m_table->begin(); it != m_table->end(); ++it)
        {
            if (gen != it->m_generation)
            {
                gen = it->m_generation;

                switch (gen)
                {
                    case GDT_HW_GENERATION_R6XX:
                        s_Log << "R600 (HD 2000 / HD 3000 series):" << endl;
                        break;

                    case GDT_HW_GENERATION_R7XX:
                        s_Log << "R700 (HD 4000 series)" << endl;
                        break;

                    case GDT_HW_GENERATION_EVERGREEN:
                        s_Log << "Evergreen (Mostly HD 5000 series):" << endl;
                        break;

                    case GDT_HW_GENERATION_NORTHERNISLAND:
                        s_Log << "Northern Islands (Some HD 6000 series):" << endl;
                        break;

                    case GDT_HW_GENERATION_SOUTHERNISLAND:
                        s_Log << "Southern Islands (HD 7000 series):" << endl;
                        break;

                    case GDT_HW_GENERATION_SEAISLAND:
                        s_Log << "Sea Islands (HD8000 / Rx 200 series):" << endl;
                        break;

                    case GDT_HW_GENERATION_VOLCANICISLAND:
                        s_Log << "Volcanic Islands (Rx 200 series):" << endl;
                        break;

                    default:
                        // You must have a new hardware generation.
                        // Add code...
                        GDT_Assert(false);
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
                      << "(This happens when your Catalyst drivers are newer than this tool." << endl
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