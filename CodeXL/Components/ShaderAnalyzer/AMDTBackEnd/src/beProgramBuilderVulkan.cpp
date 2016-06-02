// C++.
#include <sstream>

// Infra.
#include <AMDTOSWrappers/Include/osDirectory.h>
#include <AMDTOSWrappers/Include/osProcess.h>

// Local.
#include <AMDTBackEnd/Include/beProgramBuilderVulkan.h>
#include <AMDTBackEnd/Include/beInclude.h>
#include <AMDTBackEnd/Include/beUtils.h>
#include <AMDTBackEnd/Include/beStringConstants.h>
#include <DeviceInfoUtils.h>

// Internally-linked utilities.
static bool GetAmdspvPath(std::string& amdspvPath)
{
#ifdef __linux
    amdspvPath = "./amdspv";
#elif _WIN64
    amdspvPath = "x64\\amdspv.exe";
#elif _WIN32
    amdspvPath = "x86\\amdspv.exe";
#endif
    return true;
}

beProgramBuilderVulkan::beProgramBuilderVulkan()
{
}

beProgramBuilderVulkan::~beProgramBuilderVulkan()
{
}

beKA::beStatus beProgramBuilderVulkan::GetKernels(const std::string& device, std::vector<std::string>& kernels)
{
    GT_UNREFERENCED_PARAMETER(device);
    GT_UNREFERENCED_PARAMETER(kernels);

    // TODO: remove as part of refactoring.
    // In the executable-oriented architecture, this operation is no longer meaningful.
    return beKA::beStatus_Invalid;
}

beKA::beStatus beProgramBuilderVulkan::GetBinary(const std::string& device, const beKA::BinaryOptions& binopts, std::vector<char>& binary)
{
    GT_UNREFERENCED_PARAMETER(device);
    GT_UNREFERENCED_PARAMETER(binopts);
    GT_UNREFERENCED_PARAMETER(binary);

    // TODO: remove as part of refactoring.
    // In the executable-oriented architecture, this operation is no longer meaningful.
    return beKA::beStatus_Invalid;
}

beKA::beStatus beProgramBuilderVulkan::GetKernelILText(const std::string& device, const std::string& kernel, std::string& il)
{
    GT_UNREFERENCED_PARAMETER(device);
    GT_UNREFERENCED_PARAMETER(kernel);
    GT_UNREFERENCED_PARAMETER(il);

    // TODO: remove as part of refactoring.
    // In the executable-oriented architecture, this operation is no longer meaningful.
    return beKA::beStatus_Invalid;
}

beKA::beStatus beProgramBuilderVulkan::GetKernelISAText(const std::string& device, const std::string& kernel, std::string& isa)
{
    GT_UNREFERENCED_PARAMETER(device);
    GT_UNREFERENCED_PARAMETER(kernel);
    GT_UNREFERENCED_PARAMETER(isa);

    // TODO: remove as part of refactoring.
    // In the executable-oriented architecture, this operation is no longer meaningful.
    return beKA::beStatus_Invalid;
}

beKA::beStatus beProgramBuilderVulkan::GetStatistics(const std::string& device, const std::string& kernel, beKA::AnalysisData& analysis)
{
    GT_UNREFERENCED_PARAMETER(device);
    GT_UNREFERENCED_PARAMETER(kernel);
    GT_UNREFERENCED_PARAMETER(analysis);

    // TODO: remove as part of refactoring.
    // In the executable-oriented architecture, this operation is no longer meaningful.
    return beKA::beStatus_Invalid;
}

bool beProgramBuilderVulkan::IsInitialized()
{
    // TODO: remove as part of refactoring.
    // In the executable-oriented architecture, this operation is no longer meaningful.
    return true;
}

void beProgramBuilderVulkan::ReleaseProgram()
{
    // TODO: remove as part of refactoring.
    // In the executable-oriented architecture, this operation is no longer meaningful.
    return;
}

beKA::beStatus beProgramBuilderVulkan::GetDeviceTable(std::vector<GDT_GfxCardInfo>& table)
{
    beKA::beStatus ret = beKA::beStatus_General_FAILED;
    table.clear();

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

bool beProgramBuilderVulkan::CompileOK(std::string& device)
{
    GT_UNREFERENCED_PARAMETER(device);

    // TODO: remove as part of refactoring.
    // In the executable-oriented architecture, this operation is no longer meaningful.
    return true;
}

beKA::beStatus beProgramBuilderVulkan::Initialize(const std::string& sDllModule /*= ""*/)
{
    GT_UNREFERENCED_PARAMETER(sDllModule);

    // TODO: remove as part of refactoring.
    // In the executable-oriented architecture, this operation is no longer meaningful.
    return beKA::beStatus_SUCCESS;
}

bool GetGfxIpForVulkan(AMDTDeviceInfoUtils* pDeviceInfo, const VulkanOptions& vulkanOptions, std::string& gfxIpStr)
{
    bool ret = false;
    gfxIpStr.clear();

    if (vulkanOptions.m_targetDeviceName.compare(DEVICE_NAME_KALINDI) == 0 ||
        vulkanOptions.m_targetDeviceName.compare(DEVICE_NAME_GODAVARI) == 0)
    {
        // Special case #1: 7.x devices.
        gfxIpStr = "7.x";
        ret = true;
    }
    else if (vulkanOptions.m_targetDeviceName.compare(DEVICE_NAME_STONEY) == 0 ||
             vulkanOptions.m_targetDeviceName.compare(DEVICE_NAME_AMUR) == 0 ||
             vulkanOptions.m_targetDeviceName.compare(DEVICE_NAME_NOLAN) == 0)
    {
        // Special case #2: 8.1 devices.
        gfxIpStr = "8.1";
        ret = true;
    }
    else
    {
        // The standard case.
        size_t deviceGfxIp = 0;
        GDT_HW_GENERATION hwGeneration;
        bool isDeviceHwGenExtracted = pDeviceInfo->GetHardwareGeneration(vulkanOptions.m_targetDeviceName.c_str(), hwGeneration) &&
                                      beUtils::GdtHwGenToNumericValue(hwGeneration, deviceGfxIp);

        if (isDeviceHwGenExtracted && deviceGfxIp > 0)
        {
            gfxIpStr = std::to_string(deviceGfxIp);
            ret = true;
        }
    }

    return ret;
}

beKA::beStatus beProgramBuilderVulkan::Compile(const VulkanOptions& vulkanOptions, bool& cancelSignal, gtString& buildLog)
{
    GT_UNREFERENCED_PARAMETER(cancelSignal);
    beKA::beStatus ret = beKA::beStatus_General_FAILED;
    buildLog.makeEmpty();

    // Get amdspv's path.
    std::string ambdbilPath;
    GetAmdspvPath(ambdbilPath);

    AMDTDeviceInfoUtils* pDeviceInfo = AMDTDeviceInfoUtils::Instance();

    if (pDeviceInfo != nullptr)
    {
        // Numerical representation of the HW generation.
        std::string deviceGfxIp;

        // Convert the HW generation to the amdspv string.
        bool isDeviceHwGenExtracted = GetGfxIpForVulkan(pDeviceInfo, vulkanOptions, deviceGfxIp);

        if (isDeviceHwGenExtracted && !deviceGfxIp.empty())
        {
            // Build the command for invoking amdspv.
            std::stringstream cmd;
            cmd << ambdbilPath << " -Dall -l -gfxip " << deviceGfxIp << " -set ";

            // Flags for each pipeline stage that specify if its shader is present.
            // We will need them throughout this process.
            bool isVertShaderPresent = false;
            bool isTescShaderPresent = false;
            bool isTeseShaderPresent = false;
            bool isGeomShaderPresent = false;
            bool isFragShaderPresent = false;
            bool isCompShaderPresent = false;

            // You cannot mix compute and non-compute shaders in Vulkan,
            // so this has to be mutually exclusive.
            if (vulkanOptions.m_pipelineShaders.m_computeShader.isEmpty())
            {
                // Vertex shader.
                if (!vulkanOptions.m_pipelineShaders.m_vertexShader.isEmpty())
                {
                    isVertShaderPresent = true;
                    cmd << "in.vert.glsl=\"" << vulkanOptions.m_pipelineShaders.m_vertexShader.asASCIICharArray() << "\" ";
                }

                // Tessellation control shader.
                if (!vulkanOptions.m_pipelineShaders.m_tessControlShader.isEmpty())
                {
                    isTescShaderPresent = true;
                    cmd << "in.tesc.glsl=\"" << vulkanOptions.m_pipelineShaders.m_tessControlShader.asASCIICharArray() << "\" ";
                }

                // Tessellation evaluation shader.
                if (!vulkanOptions.m_pipelineShaders.m_tessEvaluationShader.isEmpty())
                {
                    isTeseShaderPresent = true;
                    cmd << "in.tese.glsl=\"" << vulkanOptions.m_pipelineShaders.m_tessEvaluationShader.asASCIICharArray() << "\" ";
                }

                // Geometry shader.
                if (!vulkanOptions.m_pipelineShaders.m_geometryShader.isEmpty())
                {
                    isGeomShaderPresent = true;
                    cmd << "in.geom.glsl=\"" << vulkanOptions.m_pipelineShaders.m_geometryShader.asASCIICharArray() << "\" ";
                }

                // Fragment shader.
                if (!vulkanOptions.m_pipelineShaders.m_fragmentShader.isEmpty())
                {
                    isFragShaderPresent = true;
                    cmd << "in.frag.glsl=\"" << vulkanOptions.m_pipelineShaders.m_fragmentShader.asASCIICharArray() << "\" ";
                }
            }
            else
            {
                // Compute shader.
                isCompShaderPresent = true;
                cmd << "in.comp.glsl=\"" << vulkanOptions.m_pipelineShaders.m_computeShader.asASCIICharArray() << "\" ";
            }

            // SPIR-V binaries generation.
            if (vulkanOptions.m_isSpirvBinariesRequired)
            {
                // Compute.
                if (!isCompShaderPresent)
                {
                    // Vertex.
                    if (isVertShaderPresent)
                    {
                        cmd << "out.vert.spv=vert.spv ";
                    }

                    // Tessellation control.
                    if (isTescShaderPresent)
                    {
                        cmd << "out.tesc.spv=tesc.spv ";
                    }

                    // Tessellation evaluation.
                    if (isTeseShaderPresent)
                    {
                        cmd << "out.tese.spv=tese.spv ";
                    }

                    // Geometry.
                    if (isGeomShaderPresent)
                    {
                        cmd << "out.geom.spv=geom.spv ";
                    }

                    // Fragment.
                    if (isFragShaderPresent)
                    {
                        cmd << "out.frag.spv=frag.spv ";
                    }
                }
                else
                {
                    // Compute.
                    cmd << "out.comp.spv=comp.spv ";
                }
            }

            // AMD IL Binaries generation (for now we only support PAL IL).
            if (vulkanOptions.m_isAmdPalIlBinariesRequired)
            {
                // Compute.
                if (!isCompShaderPresent)
                {
                    // Vertex.
                    if (isVertShaderPresent)
                    {
                        cmd << "out.vert.palIl=vert.palIl ";
                    }

                    // Tessellation control.
                    if (isTescShaderPresent)
                    {
                        cmd << "out.tesc.palIl=tesc.palIl ";
                    }

                    // Tessellation evaluation.
                    if (isTeseShaderPresent)
                    {
                        cmd << "out.tese.palIl=tese.palIl ";
                    }

                    // Geometry.
                    if (isGeomShaderPresent)
                    {
                        cmd << "out.geom.palIl=geom.palIl ";
                    }

                    // Fragment.
                    if (isFragShaderPresent)
                    {
                        cmd << "out.frag.palIl=frag.palIl ";
                    }
                }
                else
                {
                    // Compute.
                    cmd << "out.comp.palIl=comp.palIl ";
                }
            }

            // AMD IL disassembly generation (for now we only support PAL IL).
            if (vulkanOptions.m_isAmdPalIlDisassemblyRequired)
            {
                // Compute.
                if (!isCompShaderPresent)
                {
                    // Vertex.
                    if (isVertShaderPresent)
                    {
                        cmd << "out.vert.palIlText=\"" << vulkanOptions.m_pailIlDisassemblyOutputFiles.m_vertexShader.asASCIICharArray() << "\" ";
                    }

                    // Tessellation control.
                    if (isTescShaderPresent)
                    {
                        cmd << "out.tesc.palIlText=\"" << vulkanOptions.m_pailIlDisassemblyOutputFiles.m_tessControlShader.asASCIICharArray() << "\" ";
                    }

                    // Tessellation evaluation.
                    if (isTeseShaderPresent)
                    {
                        cmd << "out.tese.palIlText=\"" << vulkanOptions.m_pailIlDisassemblyOutputFiles.m_tessEvaluationShader.asASCIICharArray() << "\" ";
                    }

                    // Geometry.
                    if (isGeomShaderPresent)
                    {
                        cmd << "out.geom.palIlText=\"" << vulkanOptions.m_pailIlDisassemblyOutputFiles.m_geometryShader.asASCIICharArray() << "\" ";
                    }

                    // Fragment.
                    if (isFragShaderPresent)
                    {
                        cmd << "out.frag.palIlText=\"" << vulkanOptions.m_pailIlDisassemblyOutputFiles.m_fragmentShader.asASCIICharArray() << "\" ";
                    }
                }
                else
                {
                    // Compute.
                    cmd << "out.comp.palIlText=\"" << vulkanOptions.m_pailIlDisassemblyOutputFiles.m_computeShader.asASCIICharArray() << "\" ";
                }
            }

            // AMD ISA binary generation.
            if (vulkanOptions.m_isAmdIsaBinariesRequired)
            {
                // Compute.
                if (!isCompShaderPresent)
                {
                    // Vertex.
                    if (isVertShaderPresent)
                    {
                        cmd << "out.vert.isa=\"" << vulkanOptions.m_isaBinaryFiles.m_vertexShader.asASCIICharArray() << "\" ";
                    }

                    // Tessellation control.
                    if (isTescShaderPresent)
                    {
                        cmd << "out.tesc.isa=\"" << vulkanOptions.m_isaBinaryFiles.m_tessControlShader.asASCIICharArray() << "\" ";
                    }

                    // Tessellation evaluation.
                    if (isTeseShaderPresent)
                    {
                        cmd << "out.tese.isa=\"" << vulkanOptions.m_isaBinaryFiles.m_tessEvaluationShader.asASCIICharArray() << "\" ";
                    }

                    // Geometry.
                    if (isGeomShaderPresent)
                    {
                        cmd << "out.geom.isa=\"" << vulkanOptions.m_isaBinaryFiles.m_geometryShader.asASCIICharArray() << "\" ";
                    }

                    // Fragment.
                    if (isFragShaderPresent)
                    {
                        cmd << "out.frag.isa=\"" << vulkanOptions.m_isaBinaryFiles.m_fragmentShader.asASCIICharArray() << "\" ";
                    }
                }
                else
                {
                    // Compute.
                    cmd << "out.comp.isa=\"" << vulkanOptions.m_isaBinaryFiles.m_computeShader.asASCIICharArray() << "\" ";
                }
            }

            // AMD ISA disassembly generation.
            if (vulkanOptions.m_isAmdIsaDisassemblyRequired)
            {
                // Compute.
                if (!isCompShaderPresent)
                {
                    // Vertex.
                    if (isVertShaderPresent)
                    {
                        cmd << "out.vert.isaText=\"" << vulkanOptions.m_isaDisassemblyOutputFiles.m_vertexShader.asASCIICharArray() << "\" ";
                    }

                    // Tessellation control.
                    if (isTescShaderPresent)
                    {
                        cmd << "out.tesc.isaText=\"" << vulkanOptions.m_isaDisassemblyOutputFiles.m_tessControlShader.asASCIICharArray() << "\" ";
                    }

                    // Tessellation evaluation.
                    if (isTeseShaderPresent)
                    {
                        cmd << "out.tese.isaText=\"" << vulkanOptions.m_isaDisassemblyOutputFiles.m_tessEvaluationShader.asASCIICharArray() << "\" ";
                    }

                    // Geometry.
                    if (isGeomShaderPresent)
                    {
                        cmd << "out.geom.isaText=\"" << vulkanOptions.m_isaDisassemblyOutputFiles.m_geometryShader.asASCIICharArray() << "\" ";
                    }

                    // Fragment.
                    if (isFragShaderPresent)
                    {
                        cmd << "out.frag.isaText=\"" << vulkanOptions.m_isaDisassemblyOutputFiles.m_fragmentShader.asASCIICharArray() << "\" ";
                    }
                }
                else
                {
                    // Compute.
                    cmd << "out.comp.isaText=\"" << vulkanOptions.m_isaDisassemblyOutputFiles.m_computeShader.asASCIICharArray() << "\" ";
                }
            }

            // Shader compiler statistics disassembly generation.
            if (vulkanOptions.m_isScStatsRequired)
            {
                // Compute.
                if (!isCompShaderPresent)
                {
                    // Vertex.
                    if (isVertShaderPresent)
                    {
                        cmd << "out.vert.isaInfo=\"" << vulkanOptions.m_scStatisticsOutputFiles.m_vertexShader.asASCIICharArray() << "\" ";
                    }

                    // Tessellation control.
                    if (isTescShaderPresent)
                    {
                        cmd << "out.tesc.isaInfo=\"" << vulkanOptions.m_scStatisticsOutputFiles.m_tessControlShader.asASCIICharArray() << "\" ";
                    }

                    // Tessellation evaluation.
                    if (isTeseShaderPresent)
                    {
                        cmd << "out.tese.isaInfo=\"" << vulkanOptions.m_scStatisticsOutputFiles.m_tessEvaluationShader.asASCIICharArray() << "\" ";
                    }

                    // Geometry.
                    if (isGeomShaderPresent)
                    {
                        cmd << "out.geom.isaInfo=\"" << vulkanOptions.m_scStatisticsOutputFiles.m_geometryShader.asASCIICharArray() << "\" ";
                    }

                    // Fragment.
                    if (isFragShaderPresent)
                    {
                        cmd << "out.frag.isaInfo=\"" << vulkanOptions.m_scStatisticsOutputFiles.m_fragmentShader.asASCIICharArray() << "\" ";
                    }
                }
                else
                {
                    // Compute.
                    cmd << "out.comp.isaInfo=\"" << vulkanOptions.m_scStatisticsOutputFiles.m_computeShader.asASCIICharArray() << "\" ";
                }
            }

            // Redirect build log to a temporary file.
            std::string tmpFileAmdspv;
            const gtString AMPSPV_TMP_OUTPUT_FILE = L"amdspvTempFile.txt";
            osFilePath tmpFilePath(osFilePath::OS_TEMP_DIRECTORY);
            tmpFilePath.setFileName(AMPSPV_TMP_OUTPUT_FILE);
            cmd << "out.glslLog=\"" << tmpFilePath.asString().asASCIICharArray() << "\" ";

            // Launch amdspv.
            gtString amdspvOutput;
            bool isLaunchSuccess = osExecAndGrabOutput(cmd.str().c_str(), cancelSignal, amdspvOutput);

            if (isLaunchSuccess)
            {
                // This is how amdspv signals success.
                const gtString AMDSPV_SUCCESS_TOKEN = L"SUCCESS!";

                if (amdspvOutput.find(AMDSPV_SUCCESS_TOKEN) != -1)
                {
                    ret = beKA::beStatus_SUCCESS;

                    // Delete the ISA binaries.
                    beUtils::DeleteOutputFiles(vulkanOptions.m_isaBinaryFiles);
                }
                else
                {
                    ret = beKA::beStatus_VulkanAmdspvCompilationFailure;

                    // Read the build log.
                    if (tmpFilePath.exists())
                    {
                        // Read the build log.
                        gtString compilerOutput;
                        std::ifstream file(tmpFilePath.asString().asASCIICharArray());
                        std::string tmpCmdOutput((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
                        buildLog << tmpCmdOutput.c_str();

                        // Delete the temporary file.
                        osFile fileToDelete(tmpFilePath);
                        fileToDelete.deleteFile();
                    }
                }
            }
            else
            {
                ret = beKA::beStatus_VulkanAmdspvLaunchFailure;
            }
        }
        else
        {
            ret = beKA::beStatus_GLUnknownHardwareFamily;
        }
    }

    return ret;
}

void beProgramBuilderVulkan::SetPublicDeviceNames(const std::set<std::string>& publicDeviceNames)
{
    m_publicDevices = publicDeviceNames;
}

bool beProgramBuilderVulkan::GetVulkanVersion(gtString& vkVersion)
{
    vkVersion = BE_STR_VULKAN_VERSION;
    return true;
}
