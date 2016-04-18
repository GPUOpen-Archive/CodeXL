//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file afHelpAboutDialog.cpp
///
//==================================================================================

// Qt
#include <AMDTApplicationComponents/Include/acQtIncludes.h>

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTApplicationComponents/Include/acFunctions.h>
#include <AMDTOSWrappers/Include/osDirectory.h>
#include <AMDTOSAPIWrappers/Include/oaDriver.h>
#include <AMDTOSWrappers/Include/osApplication.h>
#include <AMDTOSWrappers/Include/osProductVersion.h>
#include <VersionInfo/VersionInfo.h>

// AMDTApplicationFramework:
#include <AMDTApplicationFramework/Include/dialogs/afHelpAboutDialog.h>
#include <AMDTApplicationFramework/Include/afMainAppWindow.h>
#include <AMDTApplicationFramework/Include/afAidFunctions.h>
#include <AMDTApplicationComponents/inc/acStringConstants.h>
#include <AMDTApplicationFramework/Include/afGlobalVariablesManager.h>


#include <AMDTApplicationComponents/Include/acHelpAboutDialog.h>


// ---------------------------------------------------------------------------
// Name:        afHelpAboutDialog::afHelpAboutDialog
// Description: Definition of the Help About Dialog
// Arguments:   osExecutedApplicationType executionApplicationType
//              QWidget *pParent
// Author:      Yoni Rabin
// Date:        9/4/2012
// ---------------------------------------------------------------------------
afHelpAboutDialog::afHelpAboutDialog(osExecutedApplicationType executionApplicationType, QWidget* pParent)
    : acHelpAboutDialog(executionApplicationType, pParent)
{
    // The bitmap of the Company logo:
    gtString companyLogoBitmapString;

    // Get the CodeXL images path:
    bool rc = afGetApplicationImagesPath(companyLogoBitmapString);
    GT_IF_WITH_ASSERT(rc)
    {
        companyLogoBitmapString.append(osFilePath::osPathSeparator);
        companyLogoBitmapString.append(AF_STR_CodeXLAboutLogoFileName);
    }

    QString name = afGlobalVariablesManager::ProductNameA();
    QString title = QString(AC_STR_HelpAboutGLCLTitle + name);

    // Get the product version:
    osProductVersion appVersion;
    osGetApplicationVersion(appVersion);

    // Set the copyright information and caption strings to empty text
    QString copyRightInformation;
    QString copyRightCaption;

    Init(title, name, appVersion, copyRightCaption, copyRightInformation, afGlobalVariablesManager::ProductIconID(), afGlobalVariablesManager::instance().versionCaption(), companyLogoBitmapString, true);
}
