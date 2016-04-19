// -*- C++ -*-
//=====================================================================
// Copyright 2011 Advanced Micro Devices, Inc. All rights reserved.
//
/// \author GPU Developer Tools
/// \version $Revision: #1 $
/// \brief  The entry point for the KernelAnalyzer backend.
//
//=====================================================================
// $Id: //devtools/main/CodeXL/Components/ShaderAnalyzer/AMDTBackEnd/Include/beBackend.h#1 $
// Last checkin:   $DateTime: 2016/02/28 16:32:28 $
// Last edited by: $Author: igal $
// Change list:    $Change: 561710 $
//=====================================================================

#ifndef _BACKEND_H_
#define _BACKEND_H_

#include <map>
#include <ostream>
#include <set>
#include <string>
#include <vector>

// We need the lazy version of TSingleton.
// Global ctor/dtor objects in shared libraries have problems.
#define USE_POINTER_SINGLETON 1
#include <TSingleton.h>

#include "AMDTBackEnd/Include/beProgramBuilder.h"


#ifdef _WIN32
    #pragma warning(push)
    #pragma warning(disable:4251)
#endif

struct GDT_GfxCardInfo;
class beProgramBuilderOpenCL;
class beProgramBuilderGL;

#ifdef _WIN32
    class beProgramBuilderDX;
#endif

using namespace std;
using namespace beKA;

/// Kernel Analyzer Backend: control the flow from UI/CLI to compiler dll's
//       Note: Don't include inline member functions here. It will make the Windows linker complain about duplicate functions.
class KA_BACKEND_DECLDIR Backend : public TSingleton<Backend>
{
    /// TSingleton needs to be able to use our constructor.
    friend class TSingleton<Backend>;

public:

    //
    // Public member functions
    //
    beStatus Initialize(BuiltProgramKind ProgramKind, LoggingCallBackFuncP callback, const string& sDllModule = "");

    /// dtor
    /// If a Log stream is available, unreleased built programs may be diagnosed.
    ~Backend();

    /// Get the graphics card information.
    /// \param[in]  deviceName A CAL device name.
    /// \param[out] info       A vector of info structures.
    ///   The lifetime of this vector ends with the next call to this interface.
    /// \returns          a status.
    static beKA::beStatus GetDeviceInfo(const std::string& deviceName, const std::vector<GDT_GfxCardInfo>** info);

    /// Get the graphics card information.
    /// \param[in]  deviceID   Device ID (these can be queried from ADL).
    /// \param[out] info       Info data structure.
    /// \returns               a status.
    static beKA::beStatus GetDeviceInfo(size_t deviceID, GDT_GfxCardInfo& info);

    /// Get the graphics card information.
    /// \param[in]  deviceName
    /// \param[out] Device info.
    /// \returns               a status.
    static beKA::beStatus GetDeviceInfo(const std::string& deviceName, GDT_DeviceInfo& Gdtdi);


    /// Get the graphics card information.
    /// \param[in]  deviceName
    /// \param[out] Device info.
    /// \returns               a status.
    static beStatus GetDeviceInfo(const std::string& deviceName, GDT_GfxCardInfo& info);

    /// Get the graphics card information.
    /// \param[in]  deviceName A marketing device name.
    /// \param[out] info       A vector of info structures.
    ///   The lifetime of this vector ends with the next call to this interface.
    ///   Note: There are duplicate marketing names.
    ///         These even span multiple hardware generations.  (ugh!)
    /// \returns          a status.
    static beKA::beStatus GetDeviceInfoMarketingName(const std::string& deviceName, std::vector<GDT_GfxCardInfo>& info);

    /// Get the graphic card family & revision info.
    /// \param[in]  tableEntry   an info structure.
    /// \param[out] chipFamily   The chip family as in atiid.h.
    /// \param[out] chipRevision The chip revision as in *_id.h.
    /// \returns                 a status.
    static beKA::beStatus GetDeviceChipFamilyRevision(const GDT_GfxCardInfo& tableEntry, unsigned int& chipFamily, unsigned int& chipRevision);

    /// Get the version of the catalyst in the format of year.month as it is
    // downloaded from the AMD website i.e 13.11
    bool GetDriverVersionInfo(std::string& version) const;

    beProgramBuilderOpenCL* theOpenCLBuilder();
    beProgramBuilderGL* theOpenGLBuilder();

    /// adds device name to list if its build was successful
    /// \param device is the device name
    void AddSuccessfulBuildDevice(const std::string& device);

    /// return true if the  device name is in the list of successful builds
    /// \param device is the device name
    bool IsSuccessfulBuildForDevice(const std::string& device);

    /// clear the successful build devices list
    void ClearSuccessfulBuildDevicesList();

    /// Extract the list of supported, public, devices
    /// \param devices a set to be populated with the supported device names
    /// \returns true for success, false otherwise
    bool GetSupportedPublicDevices(std::set<std::string>& devices);

#ifdef _WIN32
    beProgramBuilderDX* theOpenDXBuilder();

    /// Adds a directory to the list of custom directories where
    /// the backend would search for the DX binaries.
    /// \param[in]  dir   a full path to a directory.
    static void AddDxSearchDir(const std::string& dir);
#endif

private:

    /// private constructor to adhere to singleton pattern
    Backend();

    /// no assignment.
    Backend operator= (const Backend& backend);

    beProgramBuilderOpenCL* m_beOpenCL;

    beProgramBuilderGL* m_beOpenGL;

    // This vector will hold additional directories
    // where DX binaries should be searched (e.g. D3D default compiler).
    static std::vector<std::string> m_customDxLoadPaths;

    std::set<std::string> m_supportedPublicDevices;

#ifdef _WIN32
    beProgramBuilderDX* m_beDX;
#endif

    /// A string that report catalyst version
    string m_driverVersionInfo;

    /// successful build devices names list
    std::vector<std::string> m_successfulBuildDevices;

    LoggingCallBackFuncP m_LogCallback;

    bool LogCallBack(const std::string& theString)
    {
        bool bRet = false;

        if (m_LogCallback)
        {
            m_LogCallback(theString);
            bRet = true;
        }

        return bRet;
    }

};

#ifdef _WIN32
    #pragma warning(pop)
#endif

#endif
