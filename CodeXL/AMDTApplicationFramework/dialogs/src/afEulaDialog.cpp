//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file afEulaDialog.cpp
///
//==================================================================================

// Qt
#include <QtWidgets>

// Infra
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTBaseTools/Include/gtString.h>
#include <AMDTOSWrappers/Include/osApplication.h>
#include <AMDTOSWrappers/Include/osDebugLog.h>
#include <AMDTOSWrappers/Include/osDirectory.h>
#include <AMDTOSWrappers/Include/osFile.h>
#include <AMDTOSWrappers/Include/osFileLauncher.h>
#include <AMDTOSWrappers/Include/osProductVersion.h>
#include <AMDTApplicationComponents/Include/acQHTMLWindow.h>
#include <AMDTApplicationComponents/Include/acFunctions.h>

// Local:
#include <AMDTApplicationFramework/Include/afCommandIds.h>
#include <AMDTApplicationFramework/Include/afAppStringConstants.h>
#include <AMDTApplicationFramework/Include/afGlobalVariablesManager.h>
#include <AMDTApplicationFramework/Include/dialogs/afEulaDialog.h>


// ----------------------------------------------------------------------------------
// Class Name:           afEulaDialog::afEulaDialog(QWidget *parent)
// General Description:  Constructor.
// Arguments:
//          parent - the parent window that generated the EULA dialog.
// Author:               Gilad Yarnitzky
// Creation Date:        8/11/2012
// ----------------------------------------------------------------------------------
afEulaDialog::afEulaDialog(QWidget* parent) : m_dlg(parent, afGlobalVariablesManager::ProductIconID())
{
    // Get the CodeXL EULA directory
    osFilePath EULAFilePath;
    bool rc = EULAFilePath.SetInstallRelatedPath(osFilePath::OS_CODEXL_EULA_PATH);
    GT_IF_WITH_ASSERT(rc)
    {
        // Load the page:
        m_dlg.setHtmlStringIntoDialog(EULAFilePath);
    }
}

// ---------------------------------------------------------------------------
// Name:        ~afEulaDialog
// Description:
// Return Val:
// Author:      Avi Shapira
// Date:        21/5/2006
// ---------------------------------------------------------------------------
afEulaDialog::~afEulaDialog()
{

}

// ---------------------------------------------------------------------------
// Name:        afEulaDialog::execute
// Description: Overrides the QDialog execute
// Return Val:  int
// Author:      Avi Shapira
// Date:        21/5/2006
// ---------------------------------------------------------------------------
int afEulaDialog::execute()
{
    int retVal = m_dlg.exec();

    // If the user pressed ok and accepted the EULA terms
    if (QDialog::Accepted == retVal)
    {
        // Get the current application version:
        osProductVersion currentAppVersion;
        osGetApplicationVersion(currentAppVersion);

        // Get the current application's revision number
        int currentApplicationRevisionNuber = currentAppVersion._patchNumber;

        // Get the afGlobalVariablesManager instance:
        afGlobalVariablesManager& theStateManager = afGlobalVariablesManager::instance();

        // Update the revision number
        theStateManager.setEULRevisionNumber(currentApplicationRevisionNuber);

        // Save the options settings into the XML file:
        theStateManager.saveGlobalSettingsToXMLFile();
    }

    return retVal;
}

