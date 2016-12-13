//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file cxlApplication.cpp
///
//==================================================================================

// =================================================================
// CodeXL executable
//      Initializes the application
//      Call the initialization of the application in AMDTApplication DLL
// =================================================================

// Qt:
#include <QApplication>
#include <QtWidgets>

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>

// AMDTApplicationFramework:
#include <AMDTApplicationFramework/Include/afAidFunctions.h>
#include <AMDTApplicationFramework/Include/afAppStringConstants.h>


// Local:
#include <AMDTApplication/inc/appApplication.h>
#include <AMDTApplication/src/appQtApplication.h>

void tempMsgHandler(QtMsgType messageType, const QMessageLogContext&, const QString&)
{
    GT_UNREFERENCED_PARAMETER(messageType);
}
int main(int argc, char* argv[])
{
    // First, register the memory allocation failure event handler.
    std::set_new_handler(appQtApplication::AppMemAllocFailureHandler);

    qInstallMessageHandler(&tempMsgHandler);


#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS

    // Initialize Qt resources:
    // Q_INIT_RESOURCE(appApplication);
    // force load libraries to bypass Qt:
    HINSTANCE comLibrary = ::LoadLibrary(L"comctl32.dll");
    GT_UNREFERENCED_PARAMETER(comLibrary);

#endif

    appApplicationStart::ApplicationStartupData codexlStartupData;

    // Initialize the Qt application framework:
    appQtApplication app(argc, argv);

    // Set the name of the modules we need:
#if (AMDT_BUILD_TARGET == AMDT_LINUX_OS)
    codexlStartupData.m_dllNamesForLoad.push_back(L"libCXLGpuDebugging");
    codexlStartupData.m_dllNamesForLoad.push_back(L"libCXLSharedProfiling");
    codexlStartupData.m_dllNamesForLoad.push_back(L"libCXLCpuProfiling");
    codexlStartupData.m_dllNamesForLoad.push_back(L"libCXLGpuProfiling");
    codexlStartupData.m_dllNamesForLoad.push_back(L"libCXLAnalyzer");
    codexlStartupData.m_dllNamesForLoad.push_back(L"libCXLPowerProfiling");
#else
    codexlStartupData.m_dllNamesForLoad.push_back(L"CXLGpuDebugging"     AMDT_PROJECT_SUFFIX_W);
    codexlStartupData.m_dllNamesForLoad.push_back(L"CXLSharedProfiling"  AMDT_PROJECT_SUFFIX_W);
    codexlStartupData.m_dllNamesForLoad.push_back(L"CXLCpuProfiling"     AMDT_PROJECT_SUFFIX_W);
    codexlStartupData.m_dllNamesForLoad.push_back(L"CXLGpuProfiling"     AMDT_PROJECT_SUFFIX_W);
    codexlStartupData.m_dllNamesForLoad.push_back(L"CXLAnalyzer"         AMDT_PROJECT_SUFFIX_W);
    codexlStartupData.m_dllNamesForLoad.push_back(L"CXLPowerProfiling"   AMDT_PROJECT_SUFFIX_W);
#endif


    // Get the the CodeXL splash screen file path:
    gtString codeXLImagesPath;
    osFilePath codeXLSplashScreenPath;
    bool rcImgPth = afGetApplicationImagesPath(codeXLImagesPath);
    GT_IF_WITH_ASSERT(rcImgPth)
    {
        codeXLImagesPath.append(osFilePath::osPathSeparator);
        codeXLSplashScreenPath = codeXLImagesPath;

        // Add the splash screen directory and file name to it
        codeXLSplashScreenPath.setFileName(AF_STR_SplashScreenFileName);
        codeXLSplashScreenPath.setFileExtension(AF_STR_pngFileExt);
    }

    codexlStartupData.m_productName = L"CodeXL";
    codexlStartupData.m_applicationIconId = AC_ICON_CODEXL_LOGO;
    codexlStartupData.m_splashScreenFullPath = codeXLSplashScreenPath;
    codexlStartupData.m_initializeExecutionModesManager = true;
    return appApplicationStart::appMain(argc, argv, codexlStartupData);
}
