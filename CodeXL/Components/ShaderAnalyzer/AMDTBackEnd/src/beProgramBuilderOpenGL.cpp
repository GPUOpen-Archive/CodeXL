// C++.
#include <sstream>

// Infra.
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTOSWrappers/Include/osDirectory.h>
#include <AMDTOSWrappers/Include/osProcess.h>

// Local.
#include <AMDTBackEnd/Include/beProgramBuilderOpenGL.h>
#include <AMDTBackEnd/Include/beUtils.h>
#include <AMDTBackEnd/Include/beBackend.h>

// Device info.
#include <DeviceInfoUtils.h>

// Internally-linked utilities.
static bool GetVirtualContextPath(std::string& virtualContextPath)
{
#ifdef __linux
    virtualContextPath = "./VirtualContext";
#elif _WIN32
    virtualContextPath = "x86\\VirtualContext.exe";
#else
    virtualContextPath = "x64\\VirtualContext.exe";
#endif
    return true;
}

beProgramBuilderOpenGL::beProgramBuilderOpenGL()
{

}

beProgramBuilderOpenGL::~beProgramBuilderOpenGL()
{
}

beKA::beStatus beProgramBuilderOpenGL::GetKernels(const std::string& device, std::vector<std::string>& kernels)
{
    GT_UNREFERENCED_PARAMETER(device);
    GT_UNREFERENCED_PARAMETER(kernels);

    // TODO: remove as part of refactoring.
    // In the executable-oriented architecture, this operation is no longer meaningful.
    return beKA::beStatus_Invalid;
}

beKA::beStatus beProgramBuilderOpenGL::GetBinary(const std::string& device, const beKA::BinaryOptions& binopts, std::vector<char>& binary)
{
    GT_UNREFERENCED_PARAMETER(device);
    GT_UNREFERENCED_PARAMETER(binopts);
    GT_UNREFERENCED_PARAMETER(binary);

    // TODO: remove as part of refactoring.
    // In the executable-oriented architecture, this operation is no longer meaningful.
    return beKA::beStatus_Invalid;
}

beKA::beStatus beProgramBuilderOpenGL::GetKernelILText(const std::string& device, const std::string& kernel, std::string& il)
{
    GT_UNREFERENCED_PARAMETER(device);
    GT_UNREFERENCED_PARAMETER(kernel);
    GT_UNREFERENCED_PARAMETER(il);

    // TODO: remove as part of refactoring.
    // In the executable-oriented architecture, this operation is no longer meaningful.
    return beKA::beStatus_Invalid;
}

beKA::beStatus beProgramBuilderOpenGL::GetKernelISAText(const std::string& device, const std::string& kernel, std::string& isa)
{
    GT_UNREFERENCED_PARAMETER(device);
    GT_UNREFERENCED_PARAMETER(kernel);
    GT_UNREFERENCED_PARAMETER(isa);

    // TODO: remove as part of refactoring.
    // In the executable-oriented architecture, this operation is no longer meaningful.
    return beKA::beStatus_Invalid;
}

beKA::beStatus beProgramBuilderOpenGL::GetStatistics(const std::string& device, const std::string& kernel, beKA::AnalysisData& analysis)
{
    GT_UNREFERENCED_PARAMETER(device);
    GT_UNREFERENCED_PARAMETER(kernel);
    GT_UNREFERENCED_PARAMETER(analysis);

    // TODO: remove as part of refactoring.
    // In the executable-oriented architecture, this operation is no longer meaningful.
    return beKA::beStatus_Invalid;
}

bool beProgramBuilderOpenGL::IsInitialized()
{
    // TODO: remove as part of refactoring.
    // In the executable-oriented architecture, this operation is no longer meaningful.
    return true;
}

void beProgramBuilderOpenGL::ReleaseProgram()
{
    // TODO: remove as part of refactoring.
    // In the executable-oriented architecture, this operation is no longer meaningful.
    return;
}

beKA::beStatus beProgramBuilderOpenGL::GetDeviceTable(std::vector<GDT_GfxCardInfo>& table)
{
    beKA::beStatus ret = beKA::beStatus_General_FAILED;
    table.clear();

    // Get the list of public device names.
    if (m_publicDevices.empty())
    {
        Backend* pBackend = Backend::Instance();

        if (pBackend != nullptr)
        {
            pBackend->GetSupportedPublicDevices(m_publicDevices);
        }
    }

    // Go through the list of public devices, as received from the OpenCL runtime.
    std::vector<GDT_GfxCardInfo> cardList;

    for (const std::string& publicDevice : m_publicDevices)
    {
        if (AMDTDeviceInfoUtils::Instance()->GetDeviceInfo(publicDevice.c_str(), cardList))
        {
            table.insert(table.end(), cardList.begin(), cardList.end());
            cardList.clear();
        }
    }

    std::sort(table.begin(), table.end(), beUtils::GfxCardInfoSortPredicate);

    if (!table.empty())
    {
        ret = beKA::beStatus_SUCCESS;
    }

    return ret;
}

bool beProgramBuilderOpenGL::CompileOK(std::string& device)
{
    GT_UNREFERENCED_PARAMETER(device);

    // TODO: remove as part of refactoring.
    // In the executable-oriented architecture, this operation is no longer meaningful.
    return true;
}

beKA::beStatus beProgramBuilderOpenGL::Initialize(const std::string& sDllModule /*= ""*/)
{
    GT_UNREFERENCED_PARAMETER(sDllModule);

    // TODO: remove as part of refactoring.
    // In the executable-oriented architecture, this operation is no longer meaningful.
    return beKA::beStatus_SUCCESS;
}

beKA::beStatus beProgramBuilderOpenGL::Compile(const OpenGLOptions& glOptions, bool& cancelSignal, gtString& vcOutput)
{
    GT_UNREFERENCED_PARAMETER(cancelSignal);
    beKA::beStatus ret = beKA::beStatus_General_FAILED;

    // Clear the output buffer if needed.
    if (!vcOutput.isEmpty())
    {
        vcOutput.makeEmpty();
    }

    // Get VC's path.
    std::string vcPath;
    GetVirtualContextPath(vcPath);

    AMDTDeviceInfoUtils* pDeviceInfo = AMDTDeviceInfoUtils::Instance();

    if (pDeviceInfo != nullptr)
    {
        const char VC_CMD_DELIMITER = ';';

        // Build the command for invoking Virtual Context.
        std::stringstream cmd;

        // ISA.
        cmd << vcPath << " \"" << glOptions.m_isaDisassemblyOutputFiles.m_vertexShader.asASCIICharArray() << VC_CMD_DELIMITER;
        cmd << glOptions.m_isaDisassemblyOutputFiles.m_tessControlShader.asASCIICharArray() << VC_CMD_DELIMITER;
        cmd << glOptions.m_isaDisassemblyOutputFiles.m_tessEvaluationShader.asASCIICharArray() << VC_CMD_DELIMITER;
        cmd << glOptions.m_isaDisassemblyOutputFiles.m_geometryShader.asASCIICharArray() << VC_CMD_DELIMITER;
        cmd << glOptions.m_isaDisassemblyOutputFiles.m_fragmentShader.asASCIICharArray() << VC_CMD_DELIMITER;
        cmd << glOptions.m_isaDisassemblyOutputFiles.m_computeShader.asASCIICharArray() << VC_CMD_DELIMITER;

        // Program binary.
        cmd << glOptions.m_programBinaryFile.asASCIICharArray() << VC_CMD_DELIMITER;

        // Statistics.
        cmd << glOptions.m_scStatisticsOutputFiles.m_vertexShader.asASCIICharArray() << VC_CMD_DELIMITER;
        cmd << glOptions.m_scStatisticsOutputFiles.m_tessControlShader.asASCIICharArray() << VC_CMD_DELIMITER;
        cmd << glOptions.m_scStatisticsOutputFiles.m_tessEvaluationShader.asASCIICharArray() << VC_CMD_DELIMITER;
        cmd << glOptions.m_scStatisticsOutputFiles.m_geometryShader.asASCIICharArray() << VC_CMD_DELIMITER;
        cmd << glOptions.m_scStatisticsOutputFiles.m_fragmentShader.asASCIICharArray() << VC_CMD_DELIMITER;
        cmd << glOptions.m_scStatisticsOutputFiles.m_computeShader.asASCIICharArray() << VC_CMD_DELIMITER;

        // Target device info.
        cmd << glOptions.m_chipFamily << VC_CMD_DELIMITER << glOptions.m_chipRevision << VC_CMD_DELIMITER;

        // Input shaders.
        cmd << glOptions.m_pipelineShaders.m_vertexShader.asASCIICharArray() << VC_CMD_DELIMITER;
        cmd << glOptions.m_pipelineShaders.m_tessControlShader.asASCIICharArray() << VC_CMD_DELIMITER;
        cmd << glOptions.m_pipelineShaders.m_tessEvaluationShader.asASCIICharArray() << VC_CMD_DELIMITER;
        cmd << glOptions.m_pipelineShaders.m_geometryShader.asASCIICharArray() << VC_CMD_DELIMITER;
        cmd << glOptions.m_pipelineShaders.m_fragmentShader.asASCIICharArray() << VC_CMD_DELIMITER;
        cmd << glOptions.m_pipelineShaders.m_computeShader.asASCIICharArray() << VC_CMD_DELIMITER;

        // An additional delimiter for the version slot.
        cmd << VC_CMD_DELIMITER;
        cmd << "\"";

        // Build the GL program.
        bool isCompilerOutputRelevant = true;
        bool isLaunchSuccess = osExecAndGrabOutput(cmd.str().c_str(), cancelSignal, vcOutput);

        if (isLaunchSuccess)
        {
            const gtString VC_ERROR_TOKEN = L"error:";
            gtString vcOutputInLowerCase = vcOutput;
            vcOutputInLowerCase.toLowerCase();

            if (vcOutputInLowerCase.find(VC_ERROR_TOKEN) == -1)
            {
                ret = beKA::beStatus_SUCCESS;
                isCompilerOutputRelevant = false;
            }
            else
            {
                ret = beKA::beStatus_GLOpenGLBuildError;
            }
        }
        else
        {
            ret = beKA::beStatus_GLOpenGLVirtualContextFailedToLaunch;
        }

        // Clear the output if irrelevant.
        if (!isCompilerOutputRelevant)
        {
            vcOutput.makeEmpty();
        }
    }

    return ret;
}

void beProgramBuilderOpenGL::SetPublicDeviceNames(const std::set<std::string>& publicDeviceNames)
{
    m_publicDevices = publicDeviceNames;
}

bool beProgramBuilderOpenGL::GetOpenGLVersion(gtString& glVersion)
{
    // Get VC's path.
    std::string vcPath;
    GetVirtualContextPath(vcPath);

    // Build the command for invoking Virtual Context.
    std::stringstream cmd;
    cmd << vcPath << " \";;;;;;;;;;;;;;;;;;;;;version;\"";

    // A flag for canceling the operation, we will not use it.
    bool dummyCancelFlag = false;
    bool isLaunchSuccess = osExecAndGrabOutput(cmd.str().c_str(), dummyCancelFlag, glVersion);

    return isLaunchSuccess;
}

bool beProgramBuilderOpenGL::GetDeviceGLInfo(const std::string& deviceName, size_t& deviceFamilyId, size_t& deviceRevision) const
{
    bool ret = false;

    // This map will hold the device values as expected by the OpenGL backend.
    static std::map<std::string, std::pair<size_t, size_t>> glBackendValues;
    if (glBackendValues.empty())
    {
        // Fill in the values if that's the first time.
        glBackendValues["Bonaire"] = std::pair<int, int>(120, 20);
        glBackendValues["Capeverde"] = std::pair<int, int>(110, 40);
        glBackendValues["Carrizo"] = std::pair<int, int>(130, 1);
        glBackendValues["Fiji"] = std::pair<int, int>(130, 60);
        glBackendValues["Hainan"] = std::pair<int, int>(110, 75);
        glBackendValues["Hawaii"] = std::pair<int, int>(120, 40);
        glBackendValues["Iceland"] = std::pair<int, int>(130, 19);
        glBackendValues["Kalindi"] = std::pair<int, int>(120, 129);
        glBackendValues["Mullins"] = std::pair<int, int>(120, 161);
        glBackendValues["Oland"] = std::pair<int, int>(110, 60);
        glBackendValues["Pitcairn"] = std::pair<int, int>(110, 20);
        glBackendValues["Spectre"] = std::pair<int, int>(120, 1);
        glBackendValues["Spooky"] = std::pair<int, int>(120, 65);
        glBackendValues["Tahiti"] = std::pair<int, int>(110, 0);
        glBackendValues["Tonga"] = std::pair<int, int>(130, 20);
    }

    // Fetch the relevant value.
    auto deviceIter = glBackendValues.find(deviceName);
    if (deviceIter != glBackendValues.end())
    {
        deviceFamilyId = deviceIter->second.first;
        deviceRevision = deviceIter->second.second;
        ret = true;
    }

    return ret;
}
