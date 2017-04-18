//=====================================================================
// Copyright 2011, 2012 Advanced Micro Devices, Inc. All rights reserved.
//
/// \author GPU Developer Tools
/// \version $Revision: #2 $
/// \brief  The entry point for the KernelAnalyzer backend.
//
//=====================================================================
// $Id: //devtools/main/CodeXL/Components/ShaderAnalyzer/AMDTBackEnd/src/beBackend.cpp#2 $
// Last checkin:   $DateTime: 2016/04/18 06:02:03 $
// Last edited by: $Author: salgrana $
// Change list:    $Change: 569613 $
//=====================================================================

#include <locale>
#ifdef _WIN32
    #include <codecvt>
#endif
#include <string>


//#ifdef _WIN32
//    // DX is Windows only.
//    #include <D3D10ShaderObject.h>
//#endif

#define KA_BACKEND_DLL_EXPORT

#include <ADLUtil.h>
// This is from ADL's include directory.
#include <customer/oem_structures.h>
#include <DeviceInfoUtils.h>
#include <ADLUtil.h>

// Carrizo device header.
#include <AMDTBackend/asic_reg/cz_id.h>

// Local.
#include <AMDTBackend/beBackend.h>
#include <AMDTBackend/beProgramBuilderOpenCL.h>
#include <AMDTBackend/beStringConstants.h>
//#ifdef _WIN32
//    #include <AMDTBackend/beProgramBuilderDX.h>
//#endif

// Infra.
#include <AMDTOSAPIWrappers/Include/oaOpenGLRenderContext.h>
#include <AMDTOSWrappers/Include/osFilePath.h>
#include <AMDTOSWrappers/Include/osApplication.h>
#include <AMDTOSAPIWrappers/Include/oaDriver.h>
#include <AMDTOSWrappers/Include/osDebugLog.h>
#include "kaCliLauncher.h"

#define GL_VERSION                        0x1F02
#define LAST_CATALYST_DRIVER_RELEASED     "14.50"
#define NO_CATALYST_DRIVER_INSTALLED      "0.0"

std::vector<std::string> Backend::m_customDxLoadPaths;

Backend::Backend() : m_supportedPublicDevices()
{
    m_beOpenCL = NULL;
//#ifdef _WIN32
//    m_beDX = NULL;
//#endif
    m_driverVersionInfo.clear();
}

beKA::beStatus Backend::Initialize(BuiltProgramKind ProgramKind, LoggingCallBackFuncP callback, const string& sDllModule)
{
    GT_UNREFERENCED_PARAMETER(sDllModule);
#ifdef _WIN32
    GT_UNREFERENCED_PARAMETER(ProgramKind);
#endif
    beKA::beStatus retVal = beStatus_General_FAILED;
    m_LogCallback = callback;

    // Get the driver version only once, unless failed.
    static std::string s_driverVersion;
    static bool s_isDriverVersionExtracted = false;

    if (s_driverVersion.empty())
    {
        s_isDriverVersionExtracted = GetDriverVersionInfo(s_driverVersion) && !s_driverVersion.empty();
    }


    if (m_beOpenCL == NULL)
    {
        m_beOpenCL = new beProgramBuilderOpenCL();
    }

    if (m_beOpenCL != NULL)
    {
        m_beOpenCL->SetLog(callback);

        // Extract the driver version.
        if (s_isDriverVersionExtracted)
        {
            m_beOpenCL->SetDriverVersion(s_driverVersion);
        }

        retVal = m_beOpenCL->Initialize();

        if (m_supportedPublicDevices.empty() && retVal == beStatus_SUCCESS)
        {
            m_beOpenCL->GetSupportedPublicDevices(m_supportedPublicDevices);
        }
    }

//#ifdef _WIN32
//
//    if (ProgramKind == BuiltProgramKind_DX)
//    {
//        // Initialize the DX backend.
//        // Release the old DX driver since we can initialize each run with a different dx dll.
//        if (m_beDX != NULL)
//        {
//            delete m_beDX;
//            m_beDX = NULL;
//        }
//
//        if (m_beDX == NULL)
//        {
//            m_beDX = new beProgramBuilderDX();
//            m_beDX->SetPublicDeviceNames(m_supportedPublicDevices);
//
//            for (const std::string& dir : m_customDxLoadPaths)
//            {
//                m_beDX->AddDxSearchDir(dir);
//            }
//        }
//
//        if (m_beDX != NULL)
//        {
//            // Set the name of the module to be loaded.
//            std::string moduleToLoad;
//
//            if (sDllModule.empty())
//            {
//                // This solves the VS extension issue where devenv.exe looked for the D3D compiler
//                // at its own directory, instead of looking for it at CodeXL's binaries directory.
//                osFilePath defaultCompilerFilePath;
//
//                // Get CodeXL's binaries directory. Both the 32 and 64-bit versions of d3dcompiler are bundled with CodeXL.
//                // We use the 32-bit version by default
//                bool isOk = osGetCurrentApplicationDllsPath(defaultCompilerFilePath, OS_I386_ARCHITECTURE);
//
//                if (isOk)
//                {
//                    // Create the full path to the default D3D compiler.
//                    defaultCompilerFilePath.setFileName(SA_BE_STR_HLSL_optionsDefaultCompilerFileName);
//                    defaultCompilerFilePath.setFileExtension(SA_BE_STR_HLSL_optionsDefaultCompilerFileExtension);
//                    moduleToLoad = defaultCompilerFilePath.asString().asASCIICharArray();
//                }
//            }
//            else
//            {
//                // Use the given name.
//                moduleToLoad = sDllModule;
//            }
//
//            // Initialize the DX backend.
//            m_beDX->SetLog(callback);
//
//            // Extract the driver version.
//            if (s_isDriverVersionExtracted)
//            {
//                m_beDX->SetDriverVersion(s_driverVersion);
//            }
//
//            retVal = m_beDX->Initialize(moduleToLoad);
//        }
//    }
//
//#endif

    return retVal;
}

//
// Backend member functions.
//

Backend::~Backend()
{
    if (m_beOpenCL != NULL)
    {
        m_beOpenCL->DeinitializeOpenCL();
    }

    AMDTDeviceInfoUtils::DeleteInstance();
}

beKA::beStatus Backend::GetDeviceInfo(const std::string& deviceName, GDT_DeviceInfo& Gdtdi)
{
    bool ret = AMDTDeviceInfoUtils::Instance()->GetDeviceInfo(deviceName.c_str(), Gdtdi);
    return ret ? beStatus_SUCCESS : beStatus_NO_DEVICE_FOUND;
}

beStatus Backend::GetDeviceInfo(const std::string& deviceName, const std::vector<GDT_GfxCardInfo>** info)
{
    std::vector<GDT_GfxCardInfo> deviceInfo;
    bool ret = AMDTDeviceInfoUtils::Instance()->GetDeviceInfo(deviceName.c_str(), deviceInfo);

    if (info != NULL)
    {
        *info = ret ? &deviceInfo : NULL;
    }

    return ret ? beStatus_SUCCESS : beStatus_NO_DEVICE_FOUND;
}


//TODO: Note: the following comment doesn't make sense. For each CAL device name, there can be many different GDT_GfxCardInfo instances

// This is a temporary implementation until the following function:
// beStatus Backend::GetDeviceInfo(const std::string& deviceName, const std::vector<GDT_GfxCardInfo>** info)
// is being fixed to accept GDT_GfxCardInfo& instead of the meaningless std::vector that it currently accepts.
// In the end we should have only a single implementation of that function, accepting a GDT_GfxCardInfo&, but
// without the copy which is being made in this implementation.
beStatus Backend::GetDeviceInfo(const std::string& deviceName, GDT_GfxCardInfo& info)
{
    beStatus ret = beStatus_NO_DEVICE_FOUND;
    std::vector<GDT_GfxCardInfo> tmpInfoVector;
    bool rc = AMDTDeviceInfoUtils::Instance()->GetDeviceInfo(deviceName.c_str(), tmpInfoVector);

    if (rc && (tmpInfoVector.empty() == false))
    {
        info = tmpInfoVector[0];
        ret = beStatus_SUCCESS;
    }

    return ret;
}


beStatus Backend::GetDeviceInfo(size_t deviceID, GDT_GfxCardInfo& info)
{
    // TODO check to see if we need to pass rev id here
    return AMDTDeviceInfoUtils::Instance()->GetDeviceInfo(deviceID, 0, info) ? beStatus_SUCCESS : beStatus_NO_DEVICE_FOUND;
}


beStatus Backend::GetDeviceInfoMarketingName(const std::string& deviceName, std::vector<GDT_GfxCardInfo>& info)
{
    bool ret = AMDTDeviceInfoUtils::Instance()->GetDeviceInfoMarketingName(deviceName.c_str(), info);
    return ret ? beStatus_SUCCESS : beStatus_NO_DEVICE_FOUND;
}


beStatus Backend::GetDeviceChipFamilyRevision(
    const GDT_GfxCardInfo& tableEntry,
    unsigned int&          chipFamily,
    unsigned int&          chipRevision)
{
    beStatus retVal = beStatus_SUCCESS;
    chipFamily = (unsigned int) - 1;
    chipRevision = (unsigned int) - 1;

    switch (tableEntry.m_asicType)
    {
        default:
            // The 600's EG, and NI are no longer supported by OpenCL.
            // For GSA the earliest DX/GL support is SI.
            retVal = beStatus_NO_DEVICE_FOUND;
            break;

        case GDT_TAHITI_PRO:
        case GDT_TAHITI_XT:
            chipFamily = FAMILY_SI;
            chipRevision = SI_TAHITI_P_B1;
            retVal = beStatus_SUCCESS;
            break;

        case GDT_PITCAIRN_PRO:
        case GDT_PITCAIRN_XT:
            chipFamily = FAMILY_SI;
            chipRevision = SI_PITCAIRN_PM_A1;
            retVal = beStatus_SUCCESS;
            break;

        case GDT_CAPEVERDE_PRO:
        case GDT_CAPEVERDE_XT:
            chipFamily = FAMILY_SI;
            chipRevision = SI_CAPEVERDE_M_A1;
            retVal = beStatus_SUCCESS;
            break;

        case GDT_OLAND:
            chipFamily = FAMILY_SI;
            chipRevision = SI_OLAND_M_A0;
            retVal = beStatus_SUCCESS;
            break;

        case GDT_HAINAN:
            chipFamily = FAMILY_SI;
            chipRevision = SI_HAINAN_V_A0;
            retVal = beStatus_SUCCESS;
            break;

        case GDT_BONAIRE:
            chipFamily = FAMILY_CI;
            chipRevision = CI_BONAIRE_M_A0;
            break;

        case GDT_HAWAII:
            chipFamily = FAMILY_CI;
            chipRevision = CI_HAWAII_P_A0;
            break;

        case GDT_KALINDI:
            chipFamily = FAMILY_CI;
            chipRevision = CI_BONAIRE_M_A0;
            break;

        case GDT_SPECTRE:
        case GDT_SPECTRE_SL:
        case GDT_SPECTRE_LITE:
            chipFamily = FAMILY_CI;
            chipRevision = KV_SPECTRE_A0;
            break;

        case GDT_SPOOKY:
            chipFamily = FAMILY_CI;
            chipRevision = KV_SPOOKY_A0;
            break;

        case GDT_ICELAND:
            chipFamily = FAMILY_VI;
            chipRevision = VI_ICELAND_M_A0;
            break;

        case GDT_TONGA:
            chipFamily = FAMILY_VI;
            chipRevision = VI_TONGA_P_A0;
            break;

        case GDT_CARRIZO_EMB:
        case GDT_CARRIZO:
            chipFamily = FAMILY_VI;
            chipRevision = CARRIZO_A0;
            break;

        case GDT_STONEY:
            chipFamily = FAMILY_VI;
            chipRevision = STONEY_A0;
            break;

        case GDT_FIJI:
            chipFamily = FAMILY_VI;
            chipRevision = VI_FIJI_P_A0;
            break;

        case GDT_BAFFIN:
            chipFamily = FAMILY_VI;
            chipRevision = VI_BAFFIN_M_A0;
            break;

        case GDT_ELLESMERE:
            chipFamily = FAMILY_VI;
            chipRevision = VI_ELLESMERE_P_A0;
            break;
    }

    return retVal;
}

bool Backend::GetDriverVersionInfo(std::string& version) const
{
    bool ret = false;
    int errCode = 0;
    gtString tmpVersion = oaGetDriverVersion(errCode);

    if (!tmpVersion.isEmpty())
    {
        version = tmpVersion.asASCIICharArray();
        ret = true;
    }
    else
    {
        gtString logMsg;
        logMsg << L"Failed to extract driver version. Error code: " << errCode;
        OS_OUTPUT_DEBUG_LOG(logMsg.asCharArray(), OS_DEBUG_LOG_ERROR);
    }

    return ret;
}


beProgramBuilderOpenCL* Backend::theOpenCLBuilder()
{
    return m_beOpenCL;
}

beProgramBuilderGL* Backend::theOpenGLBuilder()
{
    return m_beOpenGL;
}

//#ifdef _WIN32
//beProgramBuilderDX* Backend::theOpenDXBuilder()
//{
//    return m_beDX;
//}
//#endif

#ifdef _WIN32

void Backend::AddDxSearchDir(const std::string& dir)
{
    if (std::find(m_customDxLoadPaths.begin(), m_customDxLoadPaths.end(), dir) == m_customDxLoadPaths.end())
    {
        m_customDxLoadPaths.push_back(dir);
    }
}

#endif

void Backend::AddSuccessfulBuildDevice(const std::string& device)
{
    if (std::find(m_successfulBuildDevices.begin(), m_successfulBuildDevices.end(), device) == m_successfulBuildDevices.end())
    {
        m_successfulBuildDevices.push_back(device);
    }
}

bool Backend::IsSuccessfulBuildForDevice(const std::string& device)
{
    bool retVal = false;

    // return true only if the device is in the successful build devices list
    if (std::find(m_successfulBuildDevices.begin(), m_successfulBuildDevices.end(), device) != m_successfulBuildDevices.end())
    {
        retVal = true;
    }

    return retVal;
}

void Backend::ClearSuccessfulBuildDevicesList()
{
    m_successfulBuildDevices.clear();
}

bool Backend::GetSupportedPublicDevices(std::set<std::string>& devices)
{
    bool ret = false;

    if (!m_supportedPublicDevices.empty())
    {
        ret = true;
        devices = m_supportedPublicDevices;
    }
    else
    {
        // Get the supported public devices from the OpenCL backend.
        if (m_beOpenCL == nullptr)
        {
            m_beOpenCL = new beProgramBuilderOpenCL;
        }

        beKA::beStatus rc = m_beOpenCL->Initialize();

        if (rc == beStatus_SUCCESS)
        {
            // Retrieve the supported public devices from the OpenCL runtime.
            //m_beOpenCL->GetSupportedPublicDevices(devices);
            ret = GetOpenCLDevices(devices);
        }
    }

    return ret;
}

