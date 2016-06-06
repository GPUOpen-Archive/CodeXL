//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file appApplication.h
///
//==================================================================================

#ifndef __APPAPPLICATION_H
#define __APPAPPLICATION_H

// Infra:
#include <AMDTOSWrappers/Include/osFilePath.h>
#include <AMDTBaseTools/Include/gtString.h>
#include <AMDTBaseTools/Include/gtASCIIString.h>
#include <AMDTBaseTools/Include/gtList.h>
#include <AMDTApplicationComponents/Include/acIcons.h>

// Local:
#include <AMDTApplication/inc/appApplicationDLLBuild.h>

// Manages what features the main framework application will enable during start up.
class AppCreate
{
public:
    enum EnableOption
    {
        APP_ENABLE_SOURCECODE_MANAGER       = 0x01,  // 000001
        APP_ENABLE_WELCOME_PAGE             = 0x04,  // 000100
        APP_ENABLE_MDICENTRAL_VIEW          = 0x08,  // 001000
        APP_ENABLE_NONE                     = 0x00,
        APP_ENABLE_ALL                      = 0xFF,
        // … add more options with power of two values
    };

    Q_DECLARE_FLAGS(EnableOptions, EnableOption)
};

Q_DECLARE_OPERATORS_FOR_FLAGS(AppCreate::EnableOptions)


class APP_API appApplicationStart
{
public:
    class APP_API ApplicationStartupData
    {
    public:
        ApplicationStartupData();

        ApplicationStartupData& operator=(const ApplicationStartupData& otherString);

        gtString m_productName;
        gtList<gtString> m_dllNamesForLoad;
        acIconId m_applicationIconId;
        osFilePath m_splashScreenFullPath;
        bool m_initializeExecutionModesManager;

        /// Enable by default all available options:
        AppCreate::EnableOptions m_flagEnable = AppCreate::APP_ENABLE_ALL;

    };

    static int appMain(int argc, char* argv[], const ApplicationStartupData& applicationStartData);
    static ApplicationStartupData& ApplicationData() { return m_sApplicationStartupData; }

    static void DynamicallyCall(const gtASCIIString& functionName, int functionType, gtString& returnString, const gtList<gtString>& modulesNames);
    static void LoadSplashScreen(const osFilePath& splashScreenFilePath);

private:

    static ApplicationStartupData m_sApplicationStartupData;


};


#endif //__APPAPPLICATION_H

