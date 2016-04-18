//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file afAidFunctions.h
///
//==================================================================================

#ifndef __AFAIDFUNCTIONS_H
#define __AFAIDFUNCTIONS_H

// Forward declaration:
class QDialog;

// Infra:
#include <AMDTOSWrappers/Include/osFilePath.h>
#include <AMDTOSWrappers/Include/osOSDefinitions.h>

// Local:
#include <AMDTApplicationFramework/Include/afApplicationFrameworkDLLBuild.h>
#include <AMDTApplicationFramework/Include/afIRunModeManager.h>

enum AF_API afInstalledAMDComponents
{
    AF_AMD_GPU_COMPONENT = 0x1,
    AF_AMD_CPU_COMPONENT = 0x2,
    AF_AMD_CATALYST_COMPONENT = 0x4,
    AF_AMD_HSA_COMPONENT = 0x8
};

AF_API void afLoadTitleBarIcon(QDialog* pDlg);
AF_API void afCalculateCodeXLTitleBarString(gtString& titleBarString);
AF_API void afGetCodeXLTitleBarString(gtString& titleBarString, afRunModes runModes);
AF_API void afGetUserDataFolderPath(osFilePath& userDataPath);
AF_API bool afGetApplicationImagesPath(gtString& imagesDirPathAsString);
AF_API bool afCanAllowDifferentSystemPath();
AF_API bool afDefaultProjectFilePath(const osFilePath& executableFilePath, osFilePath& projectFilePath);
AF_API void afGetVisualStudioProjectFilePath(const osFilePath& executablePath, const gtString& projectName, osFilePath& vsProjectFilePath);

// Get version info:
AF_API bool afGetVersionDetails(int& buildVersion, int& majorVersion, int& minorVersion, int& year, int& month, int& day);


#endif //__AFAIDFUNCTIONS_H

