//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file afSoftwareUpdaterWindow.cpp
///
//==================================================================================

#include <AMDTApplicationFramework/Include/afSoftwareUpdaterWindow.h>

// Qt:

#include <QtNetwork/QNetworkInterface>
#include <QtNetwork/QNetworkConfiguration>

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTOSWrappers/Include/osDebugLog.h>
#include <AMDTOSWrappers/Include/osDirectory.h>
#include <AMDTOSWrappers/Include/osFileLauncher.h>
#include <AMDTOSWrappers/Include/osGeneralFunctions.h>
#include <AMDTOSWrappers/Include/osProcess.h>
#include <AMDTOSWrappers/Include/osApplication.h>
#include <AMDTOSWrappers/Include/osProductVersion.h>
#include <AMDTOSWrappers/Include/osStringConstants.h>
#include <AMDTApplicationComponents/Include/acDisplay.h>
#include <AMDTApplicationComponents/Include/acFunctions.h>
#include <AMDTApplicationComponents/Include/acIcons.h>

/// Local:
#include <AMDTApplicationFramework/Include/afAppStringConstants.h>
#include <AMDTApplicationFramework/Include/afGlobalVariablesManager.h>
#include <AMDTApplicationFramework/Include/afApplicationCommands.h>
#include <AMDTApplicationComponents/Include/acSoftwareUpdaterProxySetting.h>
#include <AMDTApplicationComponents/inc/acStringConstants.h>

// Display constants:
#define AF_SOFTWARE_UPDATER_BASE_WIDTH 600
#define AF_SOFTWARE_UPDATER_BASE_HEIGHT 480

afSoftwareUpdaterWindow::afSoftwareUpdaterWindow() : m_pSoftWareUpdaterCtrl(nullptr)
{
    QString productName = afGlobalVariablesManager::ProductNameA();

    m_pSoftWareUpdaterCtrl = new acSoftwareUpdaterWindow(productName, afGlobalVariablesManager::ProductIconID());
    GT_IF_WITH_ASSERT(m_pSoftWareUpdaterCtrl != NULL)
    {

        // Build the user config file path:
        gtString updateFileName(AC_STR_CheckForUpdatesXMLFileName);
        updateFileName.replace(AC_STR_CheckForUpdatesPRODUCTNAMEConstW, acQStringToGTString(productName));
        osFilePath userConfigFilePath;
        afGetUserDataFolderPath(userConfigFilePath);
        userConfigFilePath.setFileName(updateFileName);


        // Get application images path:
        gtString iconImagePath;
        bool retVal = afGetApplicationImagesPath(iconImagePath);
        GT_ASSERT(retVal);

        m_pSoftWareUpdaterCtrl->initVersionDetails(userConfigFilePath, iconImagePath);
    }
}

afSoftwareUpdaterWindow::~afSoftwareUpdaterWindow()
{
}

void afSoftwareUpdaterWindow::displayDialog(bool forceDialogDisplay)
{
    GT_IF_WITH_ASSERT(m_pSoftWareUpdaterCtrl != NULL)
    {
        m_pSoftWareUpdaterCtrl->displayDialog(forceDialogDisplay);
    }
}

void afSoftwareUpdaterWindow::performAutoCheckForUpdate()
{
    GT_IF_WITH_ASSERT(m_pSoftWareUpdaterCtrl != NULL)
    {
        m_pSoftWareUpdaterCtrl->performAutoCheckForUpdate();
    }
}