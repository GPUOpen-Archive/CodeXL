
#include <CElf.h>
#include <locale>
// This is from ADL's include directory.
#include <ADLUtil.h>
#include <ACLModuleManager.h>
#include <customer/oem_structures.h>
#include <DeviceInfoUtils.h>
#include <AMDTBaseTools/Include/AMDTDefinitions.h>

#include "Include/beProgramBuilderOpenCL.h"
#include "Emulator/Parser/ISAParser.h"

// Local.
#include <AMDTBackEnd/Include/beUtils.h>

#define CL_STATUS_TABLE \
    X(CL_SUCCESS) \
    X(CL_DEVICE_NOT_FOUND) \
    X(CL_DEVICE_NOT_AVAILABLE) \
    X(CL_COMPILER_NOT_AVAILABLE) \
    X(CL_MEM_OBJECT_ALLOCATION_FAILURE) \
    X(CL_OUT_OF_RESOURCES) \
    X(CL_OUT_OF_HOST_MEMORY) \
    X(CL_PROFILING_INFO_NOT_AVAILABLE) \
    X(CL_MEM_COPY_OVERLAP) \
    X(CL_IMAGE_FORMAT_MISMATCH) \
    X(CL_IMAGE_FORMAT_NOT_SUPPORTED) \
    X(CL_BUILD_PROGRAM_FAILURE) \
    X(CL_MAP_FAILURE) \
    X(CL_MISALIGNED_SUB_BUFFER_OFFSET) \
    X(CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST) \
    X(CL_COMPILE_PROGRAM_FAILURE) \
    X(CL_LINKER_NOT_AVAILABLE) \
    X(CL_LINK_PROGRAM_FAILURE) \
    X(CL_DEVICE_PARTITION_FAILED) \
    X(CL_KERNEL_ARG_INFO_NOT_AVAILABLE) \
    X(CL_INVALID_VALUE) \
    X(CL_INVALID_DEVICE_TYPE) \
    X(CL_INVALID_PLATFORM) \
    X(CL_INVALID_DEVICE) \
    X(CL_INVALID_CONTEXT) \
    X(CL_INVALID_QUEUE_PROPERTIES) \
    X(CL_INVALID_COMMAND_QUEUE) \
    X(CL_INVALID_HOST_PTR) \
    X(CL_INVALID_MEM_OBJECT) \
    X(CL_INVALID_IMAGE_FORMAT_DESCRIPTOR) \
    X(CL_INVALID_IMAGE_SIZE) \
    X(CL_INVALID_SAMPLER) \
    X(CL_INVALID_BINARY) \
    X(CL_INVALID_BUILD_OPTIONS) \
    X(CL_INVALID_PROGRAM) \
    X(CL_INVALID_PROGRAM_EXECUTABLE) \
    X(CL_INVALID_KERNEL_NAME) \
    X(CL_INVALID_KERNEL_DEFINITION) \
    X(CL_INVALID_KERNEL) \
    X(CL_INVALID_ARG_INDEX) \
    X(CL_INVALID_ARG_VALUE) \
    X(CL_INVALID_ARG_SIZE) \
    X(CL_INVALID_KERNEL_ARGS) \
    X(CL_INVALID_WORK_DIMENSION) \
    X(CL_INVALID_WORK_GROUP_SIZE) \
    X(CL_INVALID_WORK_ITEM_SIZE) \
    X(CL_INVALID_GLOBAL_OFFSET) \
    X(CL_INVALID_EVENT_WAIT_LIST) \
    X(CL_INVALID_EVENT) \
    X(CL_INVALID_OPERATION) \
    X(CL_INVALID_GL_OBJECT) \
    X(CL_INVALID_BUFFER_SIZE) \
    X(CL_INVALID_MIP_LEVEL) \
    X(CL_INVALID_GLOBAL_WORK_SIZE) \
    X(CL_INVALID_PROPERTY) \
    X(CL_INVALID_IMAGE_DESCRIPTOR) \
    X(CL_INVALID_COMPILER_OPTIONS) \
    X(CL_INVALID_LINKER_OPTIONS) \
    X(CL_INVALID_DEVICE_PARTITION_COUNT) \
    X(CL_PLATFORM_NOT_FOUND_KHR) \
    X(CL_DEVICE_PARTITION_FAILED_EXT) \
    X(CL_INVALID_PARTITION_COUNT_EXT) \
    X(CL_INVALID_PARTITION_NAME_EXT)

// Retrieves the list of devices according to the given HW generation.
static void AddGenerationDevices(GDT_HW_GENERATION hwGen, std::vector<GDT_GfxCardInfo>& cardList,
    std::set<std::string> &uniqueNamesOfPublishedDevices)
{
    std::vector<GDT_GfxCardInfo> cardListBuffer;
    if (AMDTDeviceInfoUtils::Instance()->GetAllCardsInHardwareGeneration(hwGen, cardListBuffer))
    {
        cardList.insert(cardList.end(), cardListBuffer.begin(), cardListBuffer.end());

        for (const GDT_GfxCardInfo& cardInfo : cardList)
        {
            uniqueNamesOfPublishedDevices.insert(cardInfo.m_szCALName);
        }
    }
}

// statics/utils
/// return a string from the cl error code
// TODO: this should be a Common utility.  It gets used in at least 4 projects.
static std::string GetCLErrorString(cl_int err)
{
    switch (err)
    {
#define X(S) case S: return #S;
            CL_STATUS_TABLE;
#undef X

        default: return ("UKNOWN");
    }
}

/// Predicate for characters we want to allow through.
static bool IsNotNormalChar(char c)
{
    std::locale loc;
    return !std::isprint(c, loc) && !std::isspace(c, loc);
}

std::string* beProgramBuilderOpenCL::s_pISAString = NULL;
std::string beProgramBuilderOpenCL::s_HSAILDisassembly;
size_t beProgramBuilderOpenCL::gs_DisassembleCounter = 0;

/// Filter buildLog of temporary OpenCL file names.
/// \param buildLog The log to be filtered.
/// \returns A new log string with the temporary file names removed.
static std::string PostProcessOpenCLBuildLog(const std::string& buildLog, const std::string& sourceCodeFullPathName)
{
    std::istringstream in(buildLog);
    std::string out;

    // So that we don't go nuts with reallocations...
    out.reserve(buildLog.size());

    do
    {
        std::string line;
        getline(in, line);

        // Adjust lines of the form:
        //   PATH_TO_FILE\OCL123.tmp.cl(123):message.
        // to:
        //   PATH_TO_FILE\file.cl, line 123:message.
        const char suffix[] = ".tmp.cl(";
        std::string::size_type lpos = line.find(suffix);
        std::string::size_type rpos = line.find("):", lpos);

        if (lpos != std::string::npos &&
            rpos != std::string::npos)
        {
            std::string lineNum = line.substr(lpos + sizeof(suffix) - 1, rpos - lpos - sizeof(suffix) + 1);
            line.replace(line.begin(), line.begin() + rpos + 2, ", line " + lineNum + ":");
            line = sourceCodeFullPathName + line;
        }

        // OpenCL started doing this about the time of Catalyst 12.4.
        // Adjust lines of the form:
        //   "X:\Users\name\AppData\Local\Temp\OCL456.tmp.cl", line 123:message
        // Also adjusting lines of the form:
        //   "/tmp/OCL456.cl", line 123:message.
        // to:
        //   PATH_TO_FILE\file.cl, line 123:message

        const char suffix2[] = ", line";
        std::string::size_type pos = line.find(suffix2);

        if (pos != std::string::npos)
        {
            line = line.substr(pos);
            line = sourceCodeFullPathName + line;
        }

        // If for some reason sourceCodeFullPathName is empty, prevent error message from starting with ", line 123:..."
        // change it to "Line 123:.."
        pos = line.find(suffix2);

        if (pos == 0)
        {
            line.replace(pos, sizeof(suffix2) / sizeof(char), "Line");
        }

        // chop off summary message at the end.
        // It also contains the temporary file name.
        pos = line.find(" detected in the compilation of ");

        if (pos != std::string::npos)
        {
            line.replace(pos, std::string::npos, " detected.");
        }

        // Get rid of nasty unprintable characters.
        // Replace them with spaces.
        replace_if(line.begin(), line.end(), IsNotNormalChar, ' ');

        out += line + '\n';
    }
    while (!in.eof());

    // We might have added an extra \n above.
    // This happens when the buildLog already ends in \n.
    // If we find two \n's in a row, assume this happened and remove the extra.
    if (out.size() > 1 &&
        out[out.size() - 1] == '\n' &&
        out[out.size() - 2] == '\n')
    {
        out.resize(out.size() - 1);
    }

    return out;
}

beProgramBuilderOpenCL::beProgramBuilderOpenCL() :
    m_TheOpenCLModule(OpenCLModule::s_DefaultModuleName),
    m_pTheACLModule(nullptr),
    m_pTheACLCompiler(nullptr),
    m_TheCALCLModule(CALCLModule::s_DefaultModuleName),
    m_NumOpenCLDevices(0),
    m_IsIntialized(false),
    m_forceEnding(false),
    m_isLegacyMode(false)
{
}
// interface
bool beProgramBuilderOpenCL::IsInitialized()
{
    return m_IsIntialized;
}

void beProgramBuilderOpenCL::AddValidatedDevices(const std::vector<GDT_GfxCardInfo>& cardList, std::set<string>& uniqueNamesOfPublishedDevices)
{
    for (const GDT_GfxCardInfo& cardInfo : cardList)
    {
        // OpenCL team guarantees that formal driver releases do not expose unpublished devices
        // so we do not double-check them and just push the device name into the devices collection
        if (m_DeviceNames.find(cardInfo.m_szCALName) != m_DeviceNames.end())
        {
            m_OpenCLDeviceTable.push_back(cardInfo); // allow all devices in internal and NDA
            uniqueNamesOfPublishedDevices.insert(cardInfo.m_szCALName);
        }
    }
}

beKA::beStatus beProgramBuilderOpenCL::Initialize(const string& sDllModule/* = ""*/)
{
    (void)(sDllModule); // Unreferenced parameter

    std::set<string> uniqueNamesOfPublishedDevices;

    // Populate the sorted device (card) info table.
    m_OpenCLDeviceTable.clear();

    // DX support now only SI, CI and VI
    if (AMDTDeviceInfoUtils::Instance()->GetAllCardsInHardwareGeneration(GDT_HW_GENERATION_SOUTHERNISLAND, m_OpenCLDeviceTable))
    {
        AddValidatedDevices(m_OpenCLDeviceTable, uniqueNamesOfPublishedDevices);
    }

    if (AMDTDeviceInfoUtils::Instance()->GetAllCardsInHardwareGeneration(GDT_HW_GENERATION_SEAISLAND, m_OpenCLDeviceTable))
    {
        AddValidatedDevices(m_OpenCLDeviceTable, uniqueNamesOfPublishedDevices);
    }

    if (AMDTDeviceInfoUtils::Instance()->GetAllCardsInHardwareGeneration(GDT_HW_GENERATION_VOLCANICISLAND, m_OpenCLDeviceTable))
    {
        AddValidatedDevices(m_OpenCLDeviceTable, uniqueNamesOfPublishedDevices);
    }

    sort(m_OpenCLDeviceTable.begin(), m_OpenCLDeviceTable.end(), beUtils::GfxCardInfoSortPredicate);

    return beKA::beStatus::beStatus_SUCCESS;
}

beKA::beStatus beProgramBuilderOpenCL::InitializeOpenCL()
{
    beKA::beStatus retVal = beKA::beStatus_SUCCESS;

    // Check that the DLLs that we use are present.
    // Also check that the function that we use are there. this check is done here since LoadModule do not verify this.
    if (!isOpenClModuleLoaded())
    {
        return beKA::beStatus_OpenCL_MODULE_NOT_LOADED;
    }


    if (m_TheOpenCLModule.OpenCLLoaded() == OpenCLModule::OpenCL_1_0)
    {
        std::stringstream ss;
        ss << "OpenCL Error: OpenCL V1.1 or later required." << endl;
        LogCallBack(ss.str());

        return beKA::beStatus_OpenCL_MODULE_TOO_OLD;
    }

    // continue with initialization
    retVal = beKA::beStatus_SUCCESS;
    cl_uint numPlatforms = 0;
    cl_int status;

    if (retVal == beKA::beStatus_SUCCESS)
    {
        // Get all the platforms and then pick the AMD one.
        status = m_TheOpenCLModule.GetPlatformIDs(0, NULL, &numPlatforms);

        if (CL_SUCCESS != status)
        {
            std::stringstream ss;
            ss << "OpenCL Error: clGetPlatformIDs failed (" + GetCLErrorString(status) + ")." << endl;
            LogCallBack(ss.str());
            retVal = beKA::beStatus_clGetPlatformIDs_FAILED;
        }
        else
        {
            retVal = beKA::beStatus_SUCCESS;
        }
    }

    if (retVal == beKA::beStatus_SUCCESS)
    {
        // Find the AMD platform.
        cl_platform_id platform = NULL;

        if (numPlatforms != 0)
        {
            std::vector<cl_platform_id>platforms;
            platforms.resize(numPlatforms);

            status = m_TheOpenCLModule.GetPlatformIDs(numPlatforms, &platforms[0], NULL);

            if (CL_SUCCESS != status)
            {
                std::stringstream ss;
                ss << "OpenCL Error: clGetPlatformIDs failed (" + GetCLErrorString(status) + ")." << endl;
                LogCallBack(ss.str());
                retVal = beKA::beStatus_clGetPlatformIDs_FAILED;
            }

            char platformName[256];

            for (unsigned int i = 0; i < numPlatforms; ++i)
            {
                status = m_TheOpenCLModule.GetPlatformInfo(
                             platforms[i],
                             CL_PLATFORM_VENDOR,
                             sizeof(platformName),
                             platformName,
                             NULL);

                if (CL_SUCCESS != status)
                {
                    std::stringstream ss;
                    ss << "OpenCL Error: clGetPlatformIDs failed (" + GetCLErrorString(status) + ")." << endl;
                    LogCallBack(ss.str());

                    retVal = beKA::beStatus_clGetPlatformInfo_FAILED;
                }

                if (0 == strcmp(platformName, "Advanced Micro Devices, Inc."))
                {
                    // found AMD platform
                    platform = platforms[i];
                    break;
                }
            }
        }

        if (NULL == platform)
        {
            std::stringstream ss;
            ss << "OpenCL Error: Can't find AMD platform.\n";
            LogCallBack(ss.str());

            retVal = beKA::beStatus_NO_OPENCL_AMD_PLATFORM;
        }

        if (retVal == beKA::beStatus_SUCCESS)
        {
            // Set up the extension functions now that we have a platform.
            m_TheOpenCLModule.LoadOpenCLExtensions(platform);

            // Create a context including Offline Devices.
            cl_context_properties properties[] =
            {
                CL_CONTEXT_PLATFORM,
                (cl_context_properties)platform,
                CL_CONTEXT_OFFLINE_DEVICES_AMD,
                (cl_context_properties)1,
                (cl_context_properties)NULL,
                (cl_context_properties)NULL
            };

            m_OpenCLContext = m_TheOpenCLModule.CreateContextFromType(properties, CL_DEVICE_TYPE_ALL, NULL, NULL, &status);

            if (CL_SUCCESS != status)
            {
                std::stringstream ss;
                ss << "OpenCL Error: clCreateContextFromType failed (" + GetCLErrorString(status) + ")." << endl;
                LogCallBack(ss.str());

                retVal = beKA::beStatus_clCreateContextFromType_FAILED;
            }
        }

        size_t contextInfoDevicesSize = 0;

        if (retVal == beKA::beStatus_SUCCESS)
        {
            // Compute the number of devices
            status = m_TheOpenCLModule.GetContextInfo(m_OpenCLContext, CL_CONTEXT_DEVICES, 0, NULL, &contextInfoDevicesSize);

            if (CL_SUCCESS != status)
            {
                std::stringstream ss;
                ss << "OpenCL Error: clGetContextInfo CL_CONTEXT_DEVICES failed (" + GetCLErrorString(status) + ")." << endl;
                LogCallBack(ss.str());

                retVal = beKA::beStatus_clGetContextInfo_FAILED;
            }
        }

        if (retVal == beKA::beStatus_SUCCESS)
        {
            m_NumOpenCLDevices = contextInfoDevicesSize / sizeof(cl_device_id);

            // Get the devices
            m_OpenCLDeviceIDs.resize(m_NumOpenCLDevices);
            status = m_TheOpenCLModule.GetContextInfo(
                         m_OpenCLContext,
                         CL_CONTEXT_DEVICES,
                         m_NumOpenCLDevices * sizeof(cl_device_id),
                         &m_OpenCLDeviceIDs[0],
                         NULL);

            if (CL_SUCCESS != status)
            {
                std::stringstream ss;
                ss << "OpenCL Error: clGetContextInfo CL_CONTEXT_DEVICES failed (" + GetCLErrorString(status) + ")." << endl;
                LogCallBack(ss.str());
                retVal = beKA::beStatus_clGetContextInfo_FAILED;
            }
        }

        if (retVal == beKA::beStatus_SUCCESS)
        {
            // Get the list of official OpenCL/CAL device names.
            // Save their names.
            // Save them into name->id and id->name maps.
            for (size_t i = 0; i < m_NumOpenCLDevices; ++i)
            {
                char asicName[256];
                status = m_TheOpenCLModule.GetDeviceInfo(m_OpenCLDeviceIDs[i], CL_DEVICE_NAME, sizeof(asicName), asicName, NULL);

                if (CL_SUCCESS != status)
                {
                    std::stringstream ss;
                    ss << "OpenCL Error: clGetContextInfo CL_DEVICE_NAME failed (" + GetCLErrorString(status) + ")." << endl;
                    LogCallBack(ss.str());

                    retVal = beKA::beStatus_clGetDeviceInfo_FAILED;
                    break;
                }

                cl_device_type deviceType;
                status = m_TheOpenCLModule.GetDeviceInfo(m_OpenCLDeviceIDs[i], CL_DEVICE_TYPE, sizeof(cl_device_type), &deviceType, NULL);

                if (CL_SUCCESS != status)
                {
                    std::stringstream ss;
                    ss << "OpenCL Error: clGetContextInfo CL_DEVICE_TYPE failed (" + GetCLErrorString(status) + ")." << endl;
                    LogCallBack(ss.str());

                    retVal = beKA::beStatus_clGetDeviceInfo_FAILED;
                    break;
                }

                if (deviceType != CL_DEVICE_TYPE_CPU)
                {
                    m_DeviceNames.insert(string(asicName));
                    m_DeviceIdNameMap[m_OpenCLDeviceIDs[i]] = string(asicName);
                    m_NameDeviceIdMap[string(asicName)] = m_OpenCLDeviceIDs[i];
                    m_NameDeviceTypeMap[string(asicName)] = deviceType;
                }
            }
        }

        // Create a string for m_OpenCLVersionInfo.
        if (retVal == beKA::beStatus_SUCCESS)
        {
            size_t versionLength;
            status = m_TheOpenCLModule.GetPlatformInfo(
                         platform,
                         CL_PLATFORM_VERSION,
                         0,
                         NULL,
                         &versionLength);

            if (CL_SUCCESS != status)
            {
                std::stringstream ss;
                ss << "OpenCL Error: clGetPlatformInfo failed (" + GetCLErrorString(status) + ")." << endl;
                LogCallBack(ss.str());
                // Just log the error and move on...
            }
            else
            {
                char* pszVersion = new char[versionLength];
                status = m_TheOpenCLModule.GetPlatformInfo(
                             platform,
                             CL_PLATFORM_VERSION,
                             versionLength,
                             pszVersion,
                             NULL);

                if (CL_SUCCESS != status)
                {
                    std::stringstream ss;
                    ss << "OpenCL Error: clGetPlatformInfo failed (" + GetCLErrorString(status) + ")." << endl;
                    LogCallBack(ss.str());
                    // Just log the error and move on...
                }
                else
                {
                    // OpenCL 1.1 AMD-APP (898.1)
                    m_OpenCLVersionInfo = string(pszVersion);
                }
            }
        }
    }

    if (retVal == beKA::beStatus_SUCCESS) // no point checking the version if nothing is loaded
    {
        m_IsIntialized = true;
        m_OpenCLVersionInfo += string("\nGraphics Driver Version: ") + m_DriverVersion;
    }

    return retVal;
}

bool beProgramBuilderOpenCL::isOpenClModuleLoaded()
{
    bool retVal = true;

    if ((m_TheOpenCLModule.OpenCLLoaded() == OpenCLModule::OpenCL_None) ||
        ((m_TheOpenCLModule.GetPlatformIDs == NULL) || (m_TheOpenCLModule.GetPlatformInfo == NULL) || (m_TheOpenCLModule.CreateContextFromType == NULL) ||
         (m_TheOpenCLModule.GetContextInfo == NULL) || (m_TheOpenCLModule.GetDeviceInfo == NULL) || (m_TheOpenCLModule.ReleaseContext == NULL) || (m_TheOpenCLModule.CreateKernel == NULL) ||
         (m_TheOpenCLModule.GetKernelWorkGroupInfo == NULL) || (m_TheOpenCLModule.ReleaseKernel == NULL) ||/* (m_TheOpenCLModule.GetKernelInfoAMD == NULL) || */ (m_TheOpenCLModule.CreateProgramWithSource == NULL) ||
         (m_TheOpenCLModule.GetProgramBuildInfo == NULL) || (m_TheOpenCLModule.GetProgramInfo == NULL) || (m_TheOpenCLModule.GetProgramInfo == NULL)))
    {
        retVal = false;
    }

    return retVal;
}

beKA::beStatus beProgramBuilderOpenCL::GetKernels(const std::string& device, std::vector<std::string>& kernels)
{
    // interface guard
    if (!m_IsIntialized)
    {
        return beStatus_OpenCL_MODULE_NOT_LOADED;
    }

    //end guard

    beKA::beStatus retVal = beKA::beStatus_SUCCESS;

    if (m_ElvesMap.count(device) == 0)
    {
        std::stringstream ss;
        ss << "Error: No binary for device \'" << device << "\'.\n";
        LogCallBack(ss.str());

        retVal = beKA::beStatus_NO_BINARY_FOR_DEVICE;
    }

    if (retVal == beKA::beStatus_SUCCESS)
    {
        const CElf& elf = *m_ElvesMap[device];

        if (m_ElvesMap[device] == NULL)
        {
            retVal = beKA::beStatus_NO_BINARY_FOR_DEVICE;
        }

        if (NULL != elf.GetSymbolTable())
        {
            const CElfSymbolTable& symtab = *elf.GetSymbolTable();
            kernels.clear();

            // Look for symbols of the form <kernelPrefix><name><kernelSuffix>.
            string kernelSuffix("_kernel");
            const string kernelPrefix("__OpenCL_");
            const string kernelPrefixHsail("__OpenCL_&__OpenCL_");
            const string kernelSuffixHsail("_kernel_metadata");

            // Apparently there is a new variant for the format of the ELF symbol name, 
            // when AMDIL compilation path is used.
            const string kernelSuffixAmdilAlternative("_binary");
            const string kernelPrefixAmdilAlternative("__ISA_");

            if (elf.GetSection(".text") == NULL)
            {
                if (elf.GetSection(".amdil") != NULL)
                {
                    kernelSuffix = "_amdil";
                }
                else if (elf.GetSection(".rodata") != NULL)
                {
                    kernelSuffix = "_metadata";
                }
            }

            for (CElfSymbolTable::const_SymbolIterator sym = symtab.SymbolsBegin(); sym != symtab.SymbolsEnd(); ++sym)
            {
                string name;
                unsigned char bind;
                unsigned char type;
                unsigned char other;
                CElfSection*  section;
                Elf64_Addr    value;
                Elf64_Xword   size;

                symtab.GetInfo(sym, &name, &bind, &type, &other, &section, &value, &size);

                if (name.substr(0, kernelPrefix.size()) == kernelPrefix &&
                    name.substr(name.length() - kernelSuffix.size(), kernelSuffix.size()) == kernelSuffix)
                {
                    // Pick out the meaty goodness that is the kernel name.
                    kernels.push_back(name.substr(kernelPrefix.size(), name.length() - kernelPrefix.size() - kernelSuffix.size()));
                }
                else if (name.substr(0, kernelPrefixHsail.size()) == kernelPrefixHsail &&
                         name.substr(name.length() - kernelSuffixHsail.size(), kernelSuffixHsail.size()) == kernelSuffixHsail)
                {
                    // Fallback to the HSAIL (BIF) scenario.
                    kernels.push_back(name.substr(kernelPrefixHsail.size(), name.length() - kernelPrefixHsail.size() - kernelSuffixHsail.size()));
                }
                else if (name.substr(0, kernelPrefixAmdilAlternative.size()) == kernelPrefixAmdilAlternative &&
                    name.substr(name.length() - kernelSuffixAmdilAlternative.size(), kernelSuffixAmdilAlternative.size()) == kernelSuffixAmdilAlternative)
                {
                    // Fallback to using the alternative symbol names for AMDIL.
                    kernels.push_back(name.substr(kernelPrefixAmdilAlternative.size(), name.length() - kernelPrefixAmdilAlternative.size() - kernelSuffixAmdilAlternative.size()));
                }
            }
        }
        else
        {
            retVal = beKA::beStatus_NO_BINARY_FOR_DEVICE;
        }
    }

    return retVal;
}

beKA::beStatus beProgramBuilderOpenCL::GetBinary(const std::string& device, const beKA::BinaryOptions& binopts, std::vector<char>& binary)
{
    // interface guard
    if (!m_IsIntialized)
    {
        return beStatus_OpenCL_MODULE_NOT_LOADED;
    }

    //end guard

    beKA::beStatus retVal = beKA::beStatus_SUCCESS;

    if (m_ElvesMap.count(device) == 0)
    {
        std::stringstream ss;
        ss << "No binary for device \'" << device << "\'.\n";
        LogCallBack(ss.str());

        retVal = beKA::beStatus_NO_BINARY_FOR_DEVICE;
    }
    else
    {
        CElf& elf = *m_ElvesMap[device];

        if (binopts.m_SuppressSection.size() == 0)
        {
            // TODO: If CElf::Store were a const member, this could be const. Unfortunately, Store updates some CElf internal state. Maybe that can be improved.
            elf.Store(&binary); // not sure why this is here
            return beKA::beStatus_SUCCESS;
        }

        // Make a copy so that we can abuse it.
        if (retVal == beKA::beStatus_SUCCESS)
        {
            CElf elfCopy(elf);

            for (vector<string>::const_iterator it = binopts.m_SuppressSection.begin();
                 it != binopts.m_SuppressSection.end();
                 ++it)
            {
                elfCopy.RemoveSection(*it);
            }

            elfCopy.Store(&binary);
        }
    }

    return retVal;

}

beKA::beStatus beProgramBuilderOpenCL::GetBinaryFromFile(const std::string& pathToBinary, const beKA::BinaryOptions& binopts, std::vector<char>& outputPath)
{
    beKA::beStatus retVal = beKA::beStatus_SUCCESS;

    CElf elf(pathToBinary);

    if (elf.good())
    {
        if (binopts.m_SuppressSection.size() == 0)
        {
            elf.Store(&outputPath);
        }
        else
        {
            for (vector<string>::const_iterator it = binopts.m_SuppressSection.begin();
                 it != binopts.m_SuppressSection.end();
                 ++it)
            {
                elf.RemoveSection(*it);
            }

            elf.Store(&outputPath);
        }
    }
    else
    {
        retVal = beKA::beStatus_NO_BINARY_FOR_DEVICE;
    }

    return retVal;
}


beKA::beStatus beProgramBuilderOpenCL::GetStatistics(const std::string& device, const std::string& kernel, beKA::AnalysisData& analysis)
{
    // interface guard
    if (!m_IsIntialized)
    {
        return beStatus_OpenCL_MODULE_NOT_LOADED;
    }

    //end guard

    beKA::beStatus bRet;

    if (m_KernelAnalysis.find(device) != m_KernelAnalysis.end())
    {
        std::map<std::string, beKA::AnalysisData> kernel_analysis;
        kernel_analysis = m_KernelAnalysis[device];
        std::map<std::string, beKA::AnalysisData>::const_iterator iter = kernel_analysis.find(kernel);

        if (iter != kernel_analysis.end())
        {
            analysis = kernel_analysis[kernel];

            if (analysis.ISASize == 0)
            {
                std::string isaText;
                bRet = GetKernelISAText(device, kernel, isaText);

                if (bRet == beStatus_SUCCESS)
                {
                    ParserISA isaParser;
                    bool isParseSuccess = isaParser.ParseForSize(isaText);

                    if (isParseSuccess)
                    {
                        analysis.ISASize = isaParser.GetCodeLen();
                    }
                }
            }

            bRet = beKA::beStatus_SUCCESS;
        }
        else
        {
            bRet = beKA::beSattus_WrongKernelName;
        }
    }
    else
    {
        bRet = beKA::beStatus_NO_DEVICE_FOUND;
    }

    return bRet;

}

void beProgramBuilderOpenCL::ReleaseProgram()
{
    for (size_t i = 0; i < m_Elves.size(); ++i)
    {
        delete m_Elves[i];
    }

    m_ElvesMap.clear();

    for (size_t i = 0; i < m_BinDeviceMap.size(); ++i)
    {
        m_BinDeviceMap.clear();
    }

    m_BinDeviceMap.clear();

    m_KernelAnalysis.clear();
    m_Elves.clear();
}

const std::string& beProgramBuilderOpenCL::GetOpenCLVersionInfo()
{
    return m_OpenCLVersionInfo;
}

beKA::beStatus beProgramBuilderOpenCL::DeinitializeOpenCL()
{
    // If TSingleton does lazy loading (and DeleteInstance actually calls a dtor),
    // we want to be tidy and release the context.
    // #if USE_POINTER_SINGLETON
    // looks like Linux doesn't like it now that we moved KA as DLL into CodeXL.let's release it on windows only. USE_POINTER_SINGLETON was always defined .
#if defined(_WIN32)
    if (NULL != m_TheOpenCLModule.ReleaseContext)
    {
        m_TheOpenCLModule.ReleaseContext(m_OpenCLContext);
    }

#else
    // If our singleton is a non-lazy global,
    // then the memory for m_OpenCLContext might be deallocated before this dtor is called.
    // In that case we cannot cleanup.
#endif

    m_OpenCLContext = NULL;

    return beKA::beStatus_SUCCESS;
}



beKA::beStatus beProgramBuilderOpenCL::GetAnalysisInternal(cl_program& program, const std::string& device, const std::string& kernel, beKA::AnalysisData* analysis)
{
    beKA::beStatus ret = beKA::beStatus_SUCCESS;
    std::string strKernelName;
    std::string strKernelNameAlt;

    bool doesUseHsail = !m_isLegacyMode && DoesUseHsailPath(device);
    const size_t NUM_OF_ATTEMPTS = doesUseHsail ? 2 : 1;

    for (size_t i = 0; i < NUM_OF_ATTEMPTS; i++)
    {
        if (doesUseHsail && i > 0)
        {
            // If we haven't succeeded with the HSAIL path, fallback to the AMDIL path.
            doesUseHsail = false;
        }

        if (doesUseHsail)
        {
            strKernelName = "&__OpenCL_" + kernel + "_kernel";
            strKernelNameAlt = kernel;
        }
        else
        {
            strKernelName = kernel;
            strKernelNameAlt = "&__OpenCL_" + kernel + "_kernel";
        }

        cl_device_id deviceId = m_NameDeviceIdMap[device];
        cl_int       status;
        cl_kernel    k = m_TheOpenCLModule.CreateKernel(program, strKernelName.c_str(), &status);

        if (status == CL_INVALID_KERNEL_NAME)
        {
            // Retry with the alternative kernel name.
            k = m_TheOpenCLModule.CreateKernel(program, strKernelNameAlt.c_str(), &status);
        }

        if (status == CL_SUCCESS)
        {

#define X(FIELD,PARAM) Inquire(&FIELD,sizeof(FIELD),PARAM,k,deviceId);
            X(analysis->maxScratchRegsNeeded, CL_KERNELINFO_SCRATCH_REGS)
            X(analysis->numWavefrontPerSIMD, CL_KERNELINFO_WAVEFRONT_PER_SIMD)
            X(analysis->wavefrontSize, CL_KERNELINFO_WAVEFRONT_SIZE)
            X(analysis->numGPRsAvailable, CL_KERNELINFO_AVAILABLE_GPRS)
            X(analysis->numGPRsUsed, CL_KERNELINFO_USED_GPRS)
            X(analysis->LDSSizeAvailable, CL_KERNELINFO_AVAILABLE_LDS_SIZE)
            X(analysis->LDSSizeUsed, CL_KERNELINFO_USED_LDS_SIZE)
            X(analysis->stackSizeAvailable, CL_KERNELINFO_AVAILABLE_STACK_SIZE)
            X(analysis->stackSizeUsed, CL_KERNELINFO_USED_STACK_SIZE)
            X(analysis->numSGPRsAvailable, CL_KERNELINFO_AVAILABLE_SGPRS)
            X(analysis->numSGPRsUsed, CL_KERNELINFO_USED_SGPRS)
            X(analysis->numVGPRsAvailable, CL_KERNELINFO_AVAILABLE_VGPRS)
            X(analysis->numVGPRsUsed, CL_KERNELINFO_USED_VGPRS)
#undef X

            // get the clGetKernelWorkGroupInfo
            size_t compileWorkGroupSize[3];     /**< compileWorkGroupSize WorkGroup size as mentioned in kernel source */
            status = m_TheOpenCLModule.GetKernelWorkGroupInfo(k, deviceId, CL_KERNEL_COMPILE_WORK_GROUP_SIZE, sizeof(size_t) * 3, compileWorkGroupSize, NULL);

            if (CL_SUCCESS == status)
            {
                analysis->numThreadPerGroupX = compileWorkGroupSize[0];
                analysis->numThreadPerGroupY = compileWorkGroupSize[1];
                analysis->numThreadPerGroupZ = compileWorkGroupSize[2];
            }

            // TODO: Add code to gather the other info clGetKernelWorkGroupInfo provides.
            m_TheOpenCLModule.ReleaseKernel(k);

            // SI reports no numGPRs.
            // pre-SI reports no numVGPRs nor numSGPRs.
            // To avoid confusing the user, copy the pre-SI GPR numbers to VGPR.
            // They are vector registers ...  the scalars are new with SI.
            if (analysis->numVGPRsAvailable == 0 && analysis->numSGPRsAvailable == 0)
            {
                analysis->numVGPRsAvailable = analysis->numGPRsAvailable;
                analysis->numVGPRsUsed = analysis->numGPRsUsed;

                analysis->numSGPRsAvailable = beKA::CAL_NA_Value_64;
                analysis->numSGPRsUsed = beKA::CAL_NA_Value_64;
            }

            // No further attempts are required.
            break;
        }
        else if (i == (NUM_OF_ATTEMPTS - 1))
        {
            // Try to extract the statistics from the textual ISA.
            // This should at least give us basic info for until the HSAIL-path
            // statistics generation issue is fixed.
            std::string isa;
            beKA::beStatus rc =  GetKernelISAText(device, kernel, isa);
            bool wereStatsExtracted = false;

            if (rc == beStatus_SUCCESS)
            {
                wereStatsExtracted = ParserISA::ParseHsailStatistics(isa, *analysis);
            }

            if (!wereStatsExtracted)
            {
                ret = beKA::beStatus_NO_STATISTICS_FOR_DEVICE;
                stringstream ss;
                ss << "Error: No statistics for \'" << device << "\'.\n";
                LogCallBack(ss.str());
            }
        }
    }

    return ret;
}

beKA::beStatus beProgramBuilderOpenCL::GetKernelDebugILText(const std::string& device, const std::string& kernel, std::string& debugil)
{
    (void)(&kernel); // Unreferenced parameter

    // interface guard
    if (!m_IsIntialized)
    {
        return beStatus_OpenCL_MODULE_NOT_LOADED;
    }

    //end guard

    beKA::beStatus retVal = beKA::beStatus_SUCCESS;

    if (m_ElvesMap.count(device) == 0)
    {
        stringstream ss;
        ss << "Error: No DebugIL for device \'" << device << "\'.\n";
        LogCallBack(ss.str());

        retVal = beKA::beStatus_NO_DEBUGIL_FOR_DEVICE;
    }

    if (m_NameDeviceTypeMap[device] == CL_DEVICE_TYPE_CPU)
    {
        stringstream ss;
        ss << "Warning: No DebugIL for CPU device \'" << device << "\'.\n";
        LogCallBack(ss.str());

        retVal = beKA::beStatus_NO_DEBUGIL_FOR_DEVICE;
    }

    if (retVal == beKA::beStatus_SUCCESS)
    {
        const CElf& elf = *m_ElvesMap[device];
        const CElfSection* debugilSection = elf.GetSection(".debugil");

        if (debugilSection == NULL)
        {
            stringstream ss;
            ss << "Error: no .debugil section found.  Did you compile with '-g'?\n";
            LogCallBack(ss.str());
            retVal = beStatus_NO_DEBUGIL_FOR_DEVICE;
        }
        else
        {
            const vector<char> data(debugilSection->GetData());
            debugil = string(data.begin(), data.end());
        }
    }

    return retVal;
}

beKA::beStatus beProgramBuilderOpenCL::GetKernelMetaDataText(const std::string& device, const std::string& kernel, std::string& metadata)
{
    beKA::beStatus ret = beStatus_NO_METADATA_FOR_DEVICE;
    std::string amdilName;
    amdilName = "__OpenCL_" + kernel + "_metadata";

    // Basically, for HSAIL we should use the following symbol name
    // for extracting the metadata section: "__OpenCL_&__OpenCL_" + kernel + "_kernel_metadata",
    // but it seems like we are unable to parse it.
    bool doesUseHsail = !m_isLegacyMode && DoesUseHsailPath(device);

    if (!doesUseHsail)
    {
        ret = GetKernelSectionText(device, amdilName, metadata);
    }

    return ret;
}


beKA::beStatus beProgramBuilderOpenCL::GetKernelILText(const std::string& device, const std::string& kernel, std::string& il)
{
    beKA::beStatus ret = beStatus_General_FAILED;
    il.clear();

    bool doesUseHsail = !m_isLegacyMode && DoesUseHsailPath(device);
    const size_t NUM_OF_ATTEMPTS = doesUseHsail ? 2 : 1;

    for (size_t i = 0; i < NUM_OF_ATTEMPTS; i++)
    {
        if (doesUseHsail && i > 0)
        {
            // If we haven't succeeded with the HSAIL path, fallback to the AMDIL path.
            doesUseHsail = false;
        }

        if (doesUseHsail && !s_HSAILDisassembly.empty())
        {
            il = s_HSAILDisassembly;
            ret = beKA::beStatus_SUCCESS;

            // No further attempts are required.
            break;
        }
        else
        {
            std::string amdilName;
            amdilName = "__OpenCL_" + kernel + "_amdil";
            ret = GetKernelSectionText(device, amdilName, il);

            if (ret != beKA::beStatus_SUCCESS)
            {
                // Retry with an alternative name.
                amdilName = "__AMDIL_" + kernel + "_text";
                ret = GetKernelSectionText(device, amdilName, il);
                
                if (ret != beKA::beStatus_SUCCESS && m_NameDeviceTypeMap[device] == CL_DEVICE_TYPE_GPU)
                {
                    // Inform the user about the failure. CPUs are ignored.
                    stringstream ss;
                    ss << "Error: No IL for device \'" << device << "\'.\n";
                    LogCallBack(ss.str());
                }
            }
        }
    }

    return ret;
}

beKA::beStatus beProgramBuilderOpenCL::GetKernelSectionText(const std::string& device, const std::string& kernel, std::string& il)
{
    // interface guard
    if (!m_IsIntialized)
    {
        return beStatus_OpenCL_MODULE_NOT_LOADED;
    }

    //end guard

    beKA::beStatus retVal = beKA::beStatus_SUCCESS;

    if (m_ElvesMap.count(device) == 0)
    {
        stringstream ss;
        ss << "Error: No binary for device \'" << device << "\'.\n";
        LogCallBack(ss.str());

        retVal = beKA::beStatus_NO_BINARY_FOR_DEVICE;
    }

    if (retVal == beKA::beStatus_SUCCESS)
    {
        const CElf& elf = *m_ElvesMap[device];

        if (NULL != elf.GetSymbolTable())
        {
            const CElfSymbolTable& symtab = *elf.GetSymbolTable();
            CElfSymbolTable::const_SymbolIterator symIt = symtab.GetSymbol(kernel);

            if (symIt == symtab.SymbolsEnd())
            {
                retVal = beKA::beStatus_NO_IL_FOR_DEVICE;
            }

            if (retVal == beKA::beStatus_SUCCESS)
            {
                string name;
                unsigned char bind;
                unsigned char type;
                unsigned char other;
                CElfSection*  section;
                Elf64_Addr    value;
                Elf64_Xword   size;

                symtab.GetInfo(symIt, &name, &bind, &type, &other, &section, &value, &size);

                // TODO: Do we need more length checks, or can we trust vector to do it for us?
                // TODO: Check that section is .amdil?
                vector<char> sectionData(section->GetData());
                // Get the part that we care about.
                il = string(sectionData.begin() + size_t(value), sectionData.begin() + size_t(value + size));
                UsePlatformNativeLineEndings(il);
            }
        }
        else
        {
            retVal = beKA::beStatus_NO_IL_FOR_DEVICE;
        }
    }

    return retVal;
}

beKA::beStatus beProgramBuilderOpenCL::GetKernelISAText(const std::string& device, const std::string& kernel, std::string& isa)
{
    // interface guard
    if (!m_IsIntialized)
    {
        return beStatus_OpenCL_MODULE_NOT_LOADED;
    }

    //end guard

    beKA::beStatus retVal = beKA::beStatus_SUCCESS;

    // Because of EPR 378198 specifically check for 2 different devices and not get their ISA if we have the ACL module.
    // Getting the ISA will cause a crash. the EPR is fixed in catalyst 13.8.
    if (device.compare("Devastator") == 0 || device.compare("Scrapper") == 0)
    {
        // extract the version by the format, it should be in the (ver) in the end.
        double dOpenCLPlatforVersion = getOpenCLPlatformVersion();

        // 1306 is the version where the fix exist. checking also if 0 to avoid crash if something was wrong with the version
        if ((dOpenCLPlatforVersion < 1306) || (dOpenCLPlatforVersion == 0))
        {
            stringstream ss;
            ss << "Warning: No ISA for " << device << " for OpenCL version prior 1306, Current version is: " << dOpenCLPlatforVersion << "\n";
            LogCallBack(ss.str());
            retVal = beKA::beStatus_NO_ISA_FOR_DEVICE;
        }
    }

    // CPU ISA is arguably x86 assembly.
    // Since the symbols in .text don't have good sizes,
    // and disassembly would be an extra step, use .astext.
    // It has no symbols, so retuen it all.
    if (m_NameDeviceTypeMap[device] == CL_DEVICE_TYPE_CPU)
    {
        const CElf& elf = *m_ElvesMap[device];
        const CElfSection* astextSection = elf.GetSection(".astext");

        if (astextSection == NULL)
        {
            stringstream ss;
            ss << "Error: no .astext section found.\n";
            LogCallBack(ss.str());
            retVal = beKA::beStatus_NO_ISA_FOR_DEVICE;
        }
        else
        {
            const vector<char> data(astextSection->GetData());
            isa = string(data.begin(), data.end());

            retVal = beKA::beStatus_SUCCESS;
        }
    }
    else // handle the GPU
    {
        string name;

        bool doesUseHsail = !m_isLegacyMode && DoesUseHsailPath(device);
        const size_t NUM_OF_ATTEMPTS = doesUseHsail ? 2 : 1;

        for (size_t i = 0; i < NUM_OF_ATTEMPTS; i++)
        {
            gs_DisassembleCounter = 0;
            s_HSAILDisassembly.clear();
            s_pISAString = new std::string;

            if (doesUseHsail && i > 0)
            {
                // If we haven't succeeded with the HSAIL path, fallback to the AMDIL path.
                doesUseHsail = false;
            }

            ACLModuleManager::Instance()->GetACLModule(doesUseHsail, m_pTheACLModule, m_pTheACLCompiler);

            if (m_pTheACLModule != nullptr && m_pTheACLModule->IsLoaded())
            {
                acl_error aclErr;

                // aclCompiler object
                aclCompiler* pCom = m_pTheACLModule->CompilerInit(NULL, &aclErr);

                if (aclErr != ACL_SUCCESS)
                {
                    std::stringstream ss;
                    ss << "Error: Compiler init failed (ACLModule) " << aclErr << ".\n";
                    LogCallBack(ss.str());
                    retVal = beKA::beStatus_ACLCompile_FAILED;
                }

                aclBinary* pAclBin = NULL;
                std::map<std::string, std::vector<char> >::iterator iter = m_BinDeviceMap.find(device);

                if (retVal == beKA::beStatus_SUCCESS)
                {
                    if (iter == m_BinDeviceMap.end())
                    {
                        std::stringstream ss;
                        ss << "Error: No binary for device \'" << device << "\'.\n";
                        LogCallBack(ss.str());

                        retVal = beKA::beStatus_NO_BINARY_FOR_DEVICE;
                    }
                    else
                    {
                        std::vector<char>& vBin = iter->second;

                        if (vBin.size() < 1)
                        {
                            std::stringstream ss;
                            ss << "Error: No binary for device \'" << device << "\'.\n";
                            LogCallBack(ss.str());

                            retVal = beKA::beStatus_NO_BINARY_FOR_DEVICE;
                        }
                    }
                }

                if (retVal == beKA::beStatus_SUCCESS)
                {
                    std::vector<char>& vBin = iter->second;

                    // aclCompiler object
                    char* cBin = reinterpret_cast<char*>(&vBin[0]);
                    aclErr = ACL_SUCCESS;
                    pAclBin = m_pTheACLModule->ReadFromMem(cBin, vBin.size(), &aclErr);

                    if (aclErr != ACL_SUCCESS)
                    {
                        retVal = beKA::beStatus_ACLBinary_FAILED;
                    }
                    else
                    {
                        try
                        {
                            gs_DisassembleCounter = 0;
                            s_pISAString = &isa;
                            s_pISAString->clear();
                            s_HSAILDisassembly.clear();

                            std::string strKernelName;
                            std::string strKernelNameAlt;

                            // For HSAIL kernels, try the "&__OpenCL..." kernel name first, as that is the most-likely kernel symbol name
                            // For non-HSAIL kernels, try the undecorated kernel name first, as that is the most-likely kernel symbol name
                            // In both cases, fall back to the other name if the most-likely name fails
                            if (doesUseHsail)
                            {
                                strKernelName = "&__OpenCL_" + kernel + "_kernel";
                                strKernelNameAlt = kernel;
                            }
                            else
                            {
                                strKernelName = kernel;
                                strKernelNameAlt = "&__OpenCL_" + kernel + "_kernel";
                            }

                            aclErr = m_pTheACLModule->Disassemble(m_pTheACLCompiler, pAclBin, strKernelName.c_str(), disassembleLogFunction);

                            bool isDisassembleFailed = (aclErr != ACL_SUCCESS);

                            if (isDisassembleFailed)
                            {
                                s_pISAString->clear();
                                s_HSAILDisassembly.clear();
                                gs_DisassembleCounter = 0;
                                aclErr = m_pTheACLModule->Disassemble(m_pTheACLCompiler, pAclBin, strKernelNameAlt.c_str(), disassembleLogFunction);
                            }

                            // Cleanup.
                            m_pTheACLModule->BinaryFini(pAclBin);
                            m_pTheACLModule->CompilerFini(pCom);

                            if (!isDisassembleFailed && !s_pISAString->empty())
                            {
                                // No need for further attempts.
                                break;
                            }

                            if (!doesUseHsail)
                            {
                                if (ACL_SUCCESS != aclErr)
                                {
                                    retVal = beStatus_NO_ISA_FOR_DEVICE;
                                }

                                // No need for further attempts.
                                break;
                            }
                        }
                        catch (...)
                        {
                            retVal = beKA::beStatus_NO_ISA_FOR_DEVICE;
                        }
                    }
                }


                if (!doesUseHsail && retVal != beKA::beStatus_SUCCESS)
                {
                    std::stringstream ss;
                    ss << "Error: Failed getting the disassembly for kernel: " << kernel << " on device: " << device << ".\n";
                    LogCallBack(ss.str());

                    retVal = beKA::beStatus_NO_ISA_FOR_DEVICE;
                }
            }
        }
    }

    return retVal;
}

beKA::beStatus  beProgramBuilderOpenCL::Inquire(void* pParamVal, size_t paramValSize, KernelInfoAMD paramName, cl_kernel kernel, cl_device_id deviceId)
{
    beKA::beStatus retVal = beKA::beStatus_SUCCESS;
    cl_int       status;
    size_t       analysisFieldSize;
    const CALuint64 na = beKA::CAL_NA_Value_64;

    status = m_TheOpenCLModule.GetKernelInfoAMD(kernel, deviceId, paramName, 0, NULL, &analysisFieldSize);

    if (status != CL_SUCCESS)
    {
        memcpy(pParamVal, &na, paramValSize);
        retVal = beKA::beStatus_General_FAILED;
    }

    if ((retVal == beKA::beStatus_SUCCESS) && (paramValSize < analysisFieldSize))
    {
        const CALuint64 err = beKA::CAL_ERR_Value_64;
        memcpy(pParamVal, &err, paramValSize);
        std::stringstream ss;
        ss << "Error: AnalysisData field`s size is too small.\n";
        LogCallBack(ss.str());
        retVal = beKA::beStatus_General_FAILED;
    }

    if (retVal == beKA::beStatus_SUCCESS)
    {
        status = m_TheOpenCLModule.GetKernelInfoAMD(kernel, deviceId, paramName, paramValSize, pParamVal, NULL);

        if (status != CL_SUCCESS)
        {
            memcpy(pParamVal, &na, paramValSize);
            std::stringstream ss;
            ss << "Error: Couldn't get ANalysis info.\n";
            LogCallBack(ss.str());
            retVal = beKA::beStatus_General_FAILED;
        }
    }

    return retVal;
}

beKA::beStatus beProgramBuilderOpenCL::Compile(const std::string& programSource, const OpenCLOptions& oclOptions, const std::string& sourceCodeFullPathName, const std::vector<std::string>* pSourcePath, int& numOfSuccessfulBuilds)
{
    // Init the output variables.
    numOfSuccessfulBuilds = 0;

    // interface guard
    if (!m_IsIntialized)
    {
        return beStatus_OpenCL_MODULE_NOT_LOADED;
    }

    const char* STR_LEGACY_FLAG = "-legacy";

    // Check if we are in legacy mode.
    for (const std::string& opt : oclOptions.m_OpenCLCompileOptions)
    {
        if (opt.compare(STR_LEGACY_FLAG) == 0)
        {
            m_isLegacyMode = true;
            break;
        }
    }

    // clear the force ending flag
    m_forceEnding = false;

    //end guard

    // clear and get ready for a new compilation session
    ReleaseProgram();

    // Get set up to build the program.
    const char* pszSource = programSource.c_str();

    size_t sourceSize[] = { programSource.size() };

    // Create CL program from the CL source kernel
    cl_int status;
    beKA::beStatus retStatus = beKA::beStatus_SUCCESS;
    cl_program program = m_TheOpenCLModule.CreateProgramWithSource(m_OpenCLContext, 1, &pszSource, sourceSize, &status);

    if (CL_SUCCESS != status)
    {
        std::stringstream ss;
        ss << "OpenCL Compile Error: clCreateProgramWithSource failed (" + GetCLErrorString(status) + ")." << endl;
        LogCallBack(ss.str());

        retStatus = beKA::beStatus_clCreateProgramWithSource_FAILED;
    }
    else
    {
        // Collect the -D options.
        string definesAndOptions;

        for (vector<string>::const_iterator it = oclOptions.m_Defines.begin();
             it != oclOptions.m_Defines.end();
             ++it)
        {
            definesAndOptions += "-D" + *it + " ";
        }

        // prepare the include path, Source file directory path name is put in quotes in order to use
        // path name with spaces (e.g. C:\Program Files (x86)) and not encounter CL_INVALID_BUILD_OPTIONS error, while clBuildProgram.
        if (pSourcePath != NULL)
        {
            for (vector<string>::const_iterator it = pSourcePath->begin(); it < pSourcePath->end(); ++it)
            {
                definesAndOptions += " -I \"" + *it + "\" ";
            }
        }

        // Collect the other compiler options.
        bool bIsHoptionRequested = false;

        for (vector<string>::const_iterator it = oclOptions.m_OpenCLCompileOptions.begin();
             it != oclOptions.m_OpenCLCompileOptions.end();
             ++it)
        {
            if (((*it).compare("-h")) || ((*it).compare("-H")))
            {
                bIsHoptionRequested = true;
            }

            definesAndOptions += *it + " ";
        }

        // We want these so that we can present them to the user.
        definesAndOptions += "-fbin-as -fbin-amdil -fbin-source";

        // Which devices do we care about?
        vector<cl_device_id> requestedDevices;

        if (oclOptions.m_SelectedDevices.empty())
        {
            // None were specified by the user, so do them all.
            requestedDevices = m_OpenCLDeviceIDs;
        }
        else
        {
            // Make a vector of device IDs from the requested device list.
            for (set<string>::const_iterator it = oclOptions.m_SelectedDevices.begin();
                 it != oclOptions.m_SelectedDevices.end();
                 ++it)
            {
                if (m_NameDeviceIdMap.count(*it) > 0)
                {
                    requestedDevices.push_back(m_NameDeviceIdMap[*it]);
                }
                else
                {
                    std::stringstream ss;
                    ss << "Error: Unknown device: " << *it << endl;
                    LogCallBack(ss.str());
                    //                return Status_CL_DEVICE_NOT_SUPPORTED;
                }
            }
        }

        m_Elves.resize(m_NumOpenCLDevices);

        vector<cl_device_id>::iterator iterDeviceId = requestedDevices.begin();

        for (int iCompilationNo = 0; iterDeviceId < requestedDevices.end(); iterDeviceId++, iCompilationNo++)
        {
            if (m_forceEnding)
            {
                retStatus = beKA::beStatus_Invalid;
                break;
            }

            std::string errString;
            stringstream ss;
            ss << "Building for ";
            ss << m_DeviceIdNameMap[*iterDeviceId];
            ss << "... ";

            LogCallBack(ss.str());


            retStatus = CompileOpenCLInternal(sourceCodeFullPathName, programSource, oclOptions, *iterDeviceId, program, definesAndOptions, iCompilationNo, errString);

            if (retStatus == beKA::beStatus_SUCCESS)
            {
                std::vector<char>   vBin;
                retStatus = GetProgramBinary(program, *iterDeviceId, &m_BinDeviceMap[m_DeviceIdNameMap[*iterDeviceId]]);

                if (retStatus == beKA::beStatus_SUCCESS)
                {
                    m_ElvesMap[m_DeviceIdNameMap[*iterDeviceId]] = m_Elves[iCompilationNo];
                    //m_BinDeviceMap[m_DeviceIdNameMap[*iterDeviceId]] = vBin;
                }

                if (retStatus == beKA::beStatus_SUCCESS)
                {
                    std::stringstream ssInner;
                    ssInner << "succeeded.\n";

                    if (bIsHoptionRequested)
                    {
                        ssInner << errString;
                        bIsHoptionRequested = false; // we want to print it only once
                    }

                    LogCallBack(ssInner.str());

                    // Increment the successful builds counter.
                    ++numOfSuccessfulBuilds;
                }
            }

            // log and try another device
            if (retStatus != beKA::beStatus_SUCCESS)
            {
                std::stringstream sss;
                sss << "failed.\n";
                sss << errString;
                LogCallBack(sss.str());

                continue; //- don't try to get the analysis
            }


            // get the kernel list
            vector<string> pKernels;
            beStatus retStatusAnalysis = beStatus_Invalid; // don't fail because statistics fail
            retStatus = GetKernels(m_DeviceIdNameMap[*iterDeviceId], pKernels);

            // go over the kernels and get the statistics now
            if (m_forceEnding)
            {
                retStatus = beKA::beStatus_Invalid;
            }

            if (retStatus == beKA::beStatus_SUCCESS)
            {
                // Pass through all the kernels:
                size_t numKernels = pKernels.size();
                std::map<std::string, beKA::AnalysisData> kernel_analysys;

                for (size_t nKernel = 0; nKernel < numKernels; nKernel++)
                {
                    string kernel = (pKernels.at(nKernel));
                    beKA::AnalysisData ad;
                    retStatusAnalysis = GetAnalysisInternal(program, m_DeviceIdNameMap[*iterDeviceId], kernel, &ad);

                    if (m_forceEnding)
                    {
                        retStatus = beKA::beStatus_Invalid;
                    }

                    if (retStatusAnalysis != beKA::beStatus_SUCCESS)
                    {
                        break;
                    }

                    kernel_analysys[kernel] = ad;
                }

                if (retStatusAnalysis == beKA::beStatus_SUCCESS)
                {
                    m_KernelAnalysis[m_DeviceIdNameMap[*iterDeviceId]] = kernel_analysys;
                }
            }
        }
    } // end CreateProgramWithSource


    return retStatus;
}


// this is the internal version of the CompileOpenCL- here we actually do the compilation for a specific device
beKA::beStatus beProgramBuilderOpenCL::CompileOpenCLInternal(const std::string& sourceCodeFullPathName, const std::string& programSource, const OpenCLOptions& oclOptions, cl_device_id requestedDeviceId, cl_program& program, std::string& definesAndOptions, int iCompilationNo, std::string& errString)
{
    // Unused parameters.
    (void)(oclOptions);
#ifndef SAVE_ELF_OBJECTS
    (void)(programSource);
#endif

    beKA::beStatus bRet = beKA::beStatus_SUCCESS;
    cl_int status;
    vector<cl_device_id> requestedDevices;
    requestedDevices.push_back(requestedDeviceId);

    bool bIsBuildSucceeded = true;

    if (!BuildOpenCLProgramWrapper(
            status,
            program,
            (cl_uint)1,
            &requestedDevices[0],
            definesAndOptions.c_str(),
            NULL,
            NULL))
    {
        errString = "OpenCL Compile Error: clBuildProgram raised an unhandled exception.\n";

        return beKA::beStatus_CL_BUILD_PROGRAM_ICE;
    }

    if (CL_SUCCESS != status)
    {
        errString = "OpenCL Compile Error: clBuildProgram failed (";
        errString += GetCLErrorString(status);
        errString += ").\n";
        bIsBuildSucceeded = false;
        //return Status_BuildOpenCLProgramWrapper_FAILED;
    }

    for (size_t i = 0; i < requestedDevices.size(); i++)
    {
        // show the build log (error and warnings)
        size_t buildLogSize = 0;
        cl_int logStatus = m_TheOpenCLModule.GetProgramBuildInfo(
                               program,
                               requestedDevices[i],
                               CL_PROGRAM_BUILD_LOG,
                               0,
                               NULL,
                               &buildLogSize);

        if (logStatus == CL_SUCCESS && buildLogSize > 1)
        {
            char* pszBuildLog = new char[buildLogSize];

            logStatus = m_TheOpenCLModule.GetProgramBuildInfo(
                            program,
                            requestedDevices[i],
                            CL_PROGRAM_BUILD_LOG,
                            buildLogSize,
                            pszBuildLog,
                            NULL);

            string buildLog(pszBuildLog);
            delete[] pszBuildLog;

            // Remove references to OpenCL temporary files.
            string filteredBuildLog = PostProcessOpenCLBuildLog(buildLog, sourceCodeFullPathName);

            // errString += "OpenCL Compile Error: ";
            // This string already ends with a newline.
            // Don't add another one after it.
            errString += filteredBuildLog;
        }
    }

    // we do it here since we want to log the compilation error info
    if (bIsBuildSucceeded == false)
    {
        return beKA::beStatus_BuildOpenCLProgramWrapper_FAILED;
    }

    // Don't bail out here.
    // The compilation may have worked for some devices, but not others.
    // I've seen failures happen only with the RV7xx devices.


    // Get the CL_PROGRAM_DEVICES.
    // These may be in a different order than the CL_CONTEXT_DEVICES.
    // This happens when we specify a subset --
    // which is at the least an ugly misfeature and is arguably a bug.
    int iNumOfOpenCLDevices = 1;
    vector<cl_device_id>programDevices;
    programDevices.resize(iNumOfOpenCLDevices);
    status = m_TheOpenCLModule.GetProgramInfo(
                 program,
                 CL_PROGRAM_DEVICES,
                 iNumOfOpenCLDevices * sizeof(cl_device_id),
                 &programDevices[0],
                 NULL);

    if (CL_SUCCESS != status)
    {
        errString = "OpenCL Compile Error: clGetProgramInfo CL_PROGRAM_DEVICES failed (" + GetCLErrorString(status) + ").\n";
        return beKA::beStatus_clGetProgramInfo_FAILED;
    }

    // Get the binaries.
    vector<size_t> binarySizes;
    binarySizes.resize(iNumOfOpenCLDevices);

    // Get the sizes of the CL binaries
    status = m_TheOpenCLModule.GetProgramInfo(
                 program,
                 CL_PROGRAM_BINARY_SIZES,
                 iNumOfOpenCLDevices * sizeof(size_t),
                 &binarySizes[0],
                 NULL);

    if (CL_SUCCESS != status)
    {
        errString = "OpenCL Compile Error: clGetProgramInfo CL_PROGRAM_BINARY_SIZES failed (" + GetCLErrorString(status) + ").\n";
        return beKA::beStatus_clGetProgramInfo_FAILED;
    }

    // Get the CL binaries
    // TODO: if this were vector<vector<char> >,
    // there would be no cleanup code for the allocated storage.
    // But we would still need to create a char** binaries to pass to clGetProgramInfo.
    vector<char*> binaries;
    binaries.resize(iNumOfOpenCLDevices);

    // we have only one
    size_t size = binarySizes[0];
    binaries[0] = size ? new char[binarySizes[0]] : NULL;


    status = m_TheOpenCLModule.GetProgramInfo(
                 program,
                 CL_PROGRAM_BINARIES,
                 iNumOfOpenCLDevices * sizeof(char*),
                 &binaries[0],
                 NULL);

    if (CL_SUCCESS != status)
    {
        errString = "OpenCL Compile Error: clGetProgramInfo CL_PROGRAM_BINARIES failed (" + GetCLErrorString(status) + ").\n";
    }

    if (binaries[0] != NULL)
    {
        vector<char> vBinary(binaries[0], binaries[0] + binarySizes[0]);
#if SAVE_ELF_OBJECTS
        // debugging code.
        ofstream output;
        output.open("debug.elf", ios::binary);
        output.write(binaries[i], binarySizes[i]);
        output.close();

        output.open("debug.source");
        output.write(programSource.c_str(), programSource.size());
        output.close();
#endif
        m_Elves[iCompilationNo] = new CElf(vBinary);

        if (!m_Elves[iCompilationNo]->good())
        {
            bRet = beKA::beStatus_ACLCompile_FAILED;
        }

        // programDevices[i] -> Elf.
        m_ElvesMap[m_DeviceIdNameMap[requestedDeviceId]] = m_Elves[iCompilationNo];
    }

    // Clean up
    delete[] binaries[0];

    return bRet;
}


beKA::beStatus beProgramBuilderOpenCL::GetProgramBinary(cl_program&     program, cl_device_id&   device, std::vector<char>*  vBinary)
{
    cl_int err = CL_SUCCESS;
    beKA::beStatus retVal = beKA::beStatus_SUCCESS;

    // get a device count for this program.
    size_t nDevices = 0;
    err = m_TheOpenCLModule.GetProgramInfo(program,
                                           CL_PROGRAM_NUM_DEVICES,
                                           sizeof(nDevices),
                                           &nDevices,
                                           NULL);


    // grab the handles to all of the devices in the program.
    vector<cl_device_id> vDevices(nDevices);
    err = m_TheOpenCLModule.GetProgramInfo(program,
                                           CL_PROGRAM_DEVICES,
                                           sizeof(cl_device_id) * nDevices,
                                           &vDevices[0],
                                           NULL);

    // set the device index to match the relevant device
    bool   foundDevice = false;
    size_t deviceIndex = 0;

    for (size_t i = 0; i < nDevices && !foundDevice; i++)
    {
        if (vDevices[i] == device)
        {
            deviceIndex = i;
            foundDevice = true;
        }
    }

    // If this fails, we've done something very wrong
    if (!foundDevice)
    {
        retVal = beKA::beStatus_clGetProgramInfo_FAILED;
    }

    if (retVal == beKA::beStatus_SUCCESS)
    {
        // figure out the sizes of each of the binaries.
        size_t* pBinarySizes = new size_t[nDevices];
        err = m_TheOpenCLModule.GetProgramInfo(program,
                                               CL_PROGRAM_BINARY_SIZES,
                                               sizeof(size_t) * nDevices,
                                               pBinarySizes,
                                               NULL);


        // The slower way, until the runtime gets fixed and released
        // retrieve all of the generated binaries
        // char **ppBinaries = new char*[ nDevices ];
        vBinary->resize(pBinarySizes[deviceIndex]);
        char* pTemp = &(*(vBinary->begin()));

        if (retVal == beKA::beStatus_SUCCESS)
        {
            err = m_TheOpenCLModule.GetProgramInfo(program,
                                                   CL_PROGRAM_BINARIES,
                                                   sizeof(char*)*nDevices,
                                                   &pTemp,
                                                   NULL);

        }
    }

    if (err != CL_SUCCESS)
    {
        std::stringstream msg;
        msg << "GetProgramBinary failed with err = " << err;
        LogCallBack(msg.str());
    }

    return retVal;
}

void beProgramBuilderOpenCL::CalLoggerFunc(const CALchar* line)
{
    // For SI, deal with a bug where the ISA instructions all get passed together.
    // This really shouldn't be necessary, but it's easy enough to deal with.
    string lineString(line);

    // Remove all carriage returns.
    lineString.erase(remove(lineString.begin(), lineString.end(), '\r'),
                     lineString.end());

    // Add a linefeed at the end if there's not one there already.
    if (lineString[lineString.length() - 1] != '\n')
    {
        lineString += '\n';
    }

    // And capture the string up to here.
    (*s_pISAString) += lineString;

}

void beProgramBuilderOpenCL::disassembleLogFunction(const char* pMsg, size_t size)
{
    // For SI, deal with a bug where the ISA instructions all get passed together.
    // This really shouldn't be necessary, but it's easy enough to deal with.
    string lineString(pMsg, size);

    // Remove all carriage returns.
    lineString.erase(remove(lineString.begin(), lineString.end(), '\r'), lineString.end());

    // Add a linefeed at the end if there's not one there already.
    if (lineString[lineString.length() - 1] != '\n')
    {
        lineString += '\n';
    }

    // And capture the string up to here.
    if (gs_DisassembleCounter == 0)
    {
        (*s_pISAString) = lineString;
    }
    else if (gs_DisassembleCounter == 1)
    {
        s_HSAILDisassembly = lineString;
    }

    gs_DisassembleCounter++;
}


bool beProgramBuilderOpenCL::BuildOpenCLProgramWrapper(
    cl_int&             status,                 ///< the normal return value
    cl_program          program,
    cl_uint             num_devices,
    const cl_device_id* device_list,
    const char*         options,
    void (CL_CALLBACK* pfn_notify)(cl_program program, void* user_data),
    void*               user_data)
{
#ifdef _WIN32

    __try
    {
#endif
        status = m_TheOpenCLModule.BuildProgram(
                     program,
                     num_devices,
                     device_list,
                     options,
                     pfn_notify,
                     user_data);
        return true;
#ifdef _WIN32
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
        return false;
    }

#endif
}

beKA::beStatus beProgramBuilderOpenCL::GetDevices(std::set<string>& devices)
{
    beKA::beStatus retVal = beStatus_BACKEND_NOT_INITIALIZED;

    if (m_IsIntialized)
    {
        devices = m_DeviceNames;
        retVal = beStatus_SUCCESS;
    }

    return retVal;
}

beStatus beProgramBuilderOpenCL::GetDeviceType(const std::string& deviceName, cl_device_type& deviceType) const
{
    beStatus retVal = beStatus_NO_SUCH_DEVICE;

    // The device names stored in the map are the ones reported by the OpenCL runtime which sometimes have trailing spaces :-(
    // This is true for CPU devices.
    // So we need to iterate through the map elements and in each element look for sub-string inclusion instead of string equality
    for (auto mapItem : m_NameDeviceTypeMap)
    {
        // Check if this device name from the map contains the device name parameter
        if (mapItem.first.find(deviceName) != std::string::npos)
        {
            // Found a matching device name
            deviceType = mapItem.second;
            retVal = beStatus_SUCCESS;
            break;
        }
    }

    return retVal;
}


beStatus beProgramBuilderOpenCL::GetDeviceTable(std::vector<GDT_GfxCardInfo>& table)
{
    table = m_OpenCLDeviceTable;
    return beStatus_SUCCESS;
}

beProgramBuilderOpenCL::~beProgramBuilderOpenCL(void)
{
}

bool beProgramBuilderOpenCL::CompileOK(std::string& device)
{
    bool bRet = false;

    std::vector<char>& vBin = m_BinDeviceMap.at(device);

    if (vBin.empty() == false)
    {
        bRet = true;
    }

    return bRet;
}

void beProgramBuilderOpenCL::ForceEnd()
{
    m_forceEnding = true;
}

void beProgramBuilderOpenCL::GetSupportedPublicDevices(std::set<std::string>& devices) const
{
    devices = m_DeviceNames;
}

bool beProgramBuilderOpenCL::GetAllGraphicsCards(std::vector<GDT_GfxCardInfo>& cardList, std::set<std::string>& uniqueNamesOfPublishedDevices)
{
    bool ret = true;

    // Retrieve the list of devices for every relevant hardware generations.
    AddGenerationDevices(GDT_HW_GENERATION_SOUTHERNISLAND, cardList, uniqueNamesOfPublishedDevices);
    AddGenerationDevices(GDT_HW_GENERATION_SEAISLAND, cardList, uniqueNamesOfPublishedDevices);
    AddGenerationDevices(GDT_HW_GENERATION_VOLCANICISLAND, cardList, uniqueNamesOfPublishedDevices);
    AddGenerationDevices(GDT_HW_GENERATION_GFX9, cardList, uniqueNamesOfPublishedDevices);

    // Sort the data.
    std::sort(cardList.begin(), cardList.end(), beUtils::GfxCardInfoSortPredicate);

    return ret;
}

double beProgramBuilderOpenCL::getOpenCLPlatformVersion()
{
    // extract the version by the format, it should be in the (ver) in the end.
    double dOpenCLPlatforVersion = 0;
    char sOpenCLVer[256];
    memset(&sOpenCLVer[0], 0, sizeof(char) * 256);
    int iStart = (int)m_OpenCLVersionInfo.find("(");
    int iEnd = (int)m_OpenCLVersionInfo.find(")");
    int iLength = iEnd - iStart;

    if (iLength > 0)
    {
        int i = 0;

        for (iStart += 1; iStart < iEnd; iStart++, i++)
        {
            sOpenCLVer[i] = m_OpenCLVersionInfo[iStart];
        }

        dOpenCLPlatforVersion = atof((char*)sOpenCLVer);
    }

    return dOpenCLPlatforVersion;

}

void beProgramBuilderOpenCL::RemoveNamesOfUnpublishedDevices(const set<string>& uniqueNamesOfPublishedDevices)
{
#ifndef AMDT_BUILD_ACCESS
#error AMDT_BUILD_ACCESS not defined!
#endif
#if (AMDT_BUILD_ACCESS == AMDT_NDA_ACCESS) || (AMDT_BUILD_ACCESS == AMDT_INTERNAL_ACCESS)
    // In NDA and INTERNAL versions this function is a no-op
    GT_UNREFERENCED_PARAMETER(uniqueNamesOfPublishedDevices); // unused
    return;
#elif (AMDT_BUILD_ACCESS == AMDT_PUBLIC_ACCESS)
    // Take advantage of the fact that the m_OpenCLDeviceTable collection contains only published devices,
    // so we look for each name that the OpenCL driver provided in the table, and remove it if it is not found

    for (set<string>::iterator iter = m_DeviceNames.begin();
         iter != m_DeviceNames.end(); /* advancing the iterator is done inside the loop*/)
    {
        bool isDevicePublished = false;
        const string& deviceName = *iter;

        if (uniqueNamesOfPublishedDevices.find(deviceName) != uniqueNamesOfPublishedDevices.end())
        {
            // The device name exists in the OpenCL device table, therefore it is a published device.
            // Nothing more to do with this device name.
            isDevicePublished = true;
        }

        if (isDevicePublished)
        {
            // Keep the published device in the names collection - just iterate to the next name
            iter++;
        }
        // CPU devices do not appear in the DeviceInfo table and we don't have their publish date, so we remove only unpublished GPU devices
        else if (m_NameDeviceTypeMap[deviceName] != CL_DEVICE_TYPE_CPU)
        {
            // Remove name of unpublished device from the names collections
            set<string>::iterator iterToRemove = iter;
            iter++;
            m_NameDeviceTypeMap.erase(deviceName);
            cl_device_id deviceId = m_NameDeviceIdMap[deviceName];
            m_NameDeviceIdMap.erase(deviceName);
            m_DeviceIdNameMap.erase(deviceId);
            m_DeviceNames.erase(iterToRemove);

            // Remove the device ID from the vector of OpenCL devices
            size_t numOfCLDevices = m_OpenCLDeviceIDs.size();

            for (size_t i = 0; i < numOfCLDevices; ++i)
            {
                if (m_OpenCLDeviceIDs[i] == deviceId)
                {
                    m_OpenCLDeviceIDs.erase(m_OpenCLDeviceIDs.begin() + i);
                    break;
                }
            }
        }
        else
        {
            iter++;
        }
    }

#else
#error Unknown build access!
#endif
}

bool beProgramBuilderOpenCL::DoesUseHsailPath(const std::string& deviceName) const
{
    bool retVal = false;
    // HSAIL path is not used on Linux 32-bit regardless of driver version, hardware family
#ifdef _LINUX
#ifdef X86
    retVal = false;
    return retVal;
#endif
#endif

    bool isSiFamily = false;

    // HSAIL path is not used on SI hardware.
    AMDTDeviceInfoUtils::Instance()->IsSIFamily(deviceName.c_str(), isSiFamily);

    if (!isSiFamily)
    {
        retVal = true;
    }

    return retVal;
}
