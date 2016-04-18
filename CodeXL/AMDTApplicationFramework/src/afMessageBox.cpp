//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file afMessageBox.cpp
///
//==================================================================================

// Infra:
#include <AMDTApplicationComponents/Include/acMessageBox.h>
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTOSWrappers/Include/osOSDefinitions.h>

// Local:
#include <AMDTApplicationFramework/Include/afMessageBox.h>
#include <AMDTApplicationFramework/Include/afGlobalVariablesManager.h>

// Static members initialization:
afMessageBox* afMessageBox::m_spMessageBoxSingleInstance = nullptr;

// ---------------------------------------------------------------------------
// Name:        afMessageBox::afMessageBox
// Description: Constructor
// Author:      Gilad Yarnitzky
// Date:        31/12/2012
// ---------------------------------------------------------------------------
afMessageBox::afMessageBox()
{
}


// ---------------------------------------------------------------------------
// Name:        afMessageBox::~afMessageBox
// Description: Destructor
// Author:      Gilad Yarnitzky
// Date:        31/12/2012
// ---------------------------------------------------------------------------
afMessageBox::~afMessageBox()
{

}

// ---------------------------------------------------------------------------
// Name:        afMessageBox::instance
// Description: Returns the single instance of this class. Creates it on
//              the first call to this function.
// Return Val:  acMessageBox&
// Author:      Gilad Yarnitzky
// Date:        31/12/2012
// ---------------------------------------------------------------------------
afMessageBox& afMessageBox::instance()
{
    // If my single instance was not created yet - create it:
    if (nullptr == m_spMessageBoxSingleInstance)
    {
        m_spMessageBoxSingleInstance = new afMessageBox;

    }

    return *m_spMessageBoxSingleInstance;
}

// ---------------------------------------------------------------------------
QMessageBox::StandardButton afMessageBox::ShowMessageBox(QMessageBox::Icon type, const QString& title, const QString& text, QMessageBox::StandardButtons buttons, QMessageBox::StandardButton defaultButton)
{
    QMessageBox::StandardButton retVal = QMessageBox::Ok;

    if (initDialog(title))
    {
        switch (type)
        {
            case QMessageBox::Warning:
                retVal = acMessageBox::instance().warning(title, text, buttons, defaultButton);
                break;

            case QMessageBox::Critical:
                retVal = acMessageBox::instance().critical(title, text, buttons, defaultButton);
                break;

            case QMessageBox::Question:
                retVal = acMessageBox::instance().question(title, text, buttons, defaultButton);
                break;

            //case QMessageBox::NoIcon:
            case QMessageBox::Information:
            default:
                retVal = acMessageBox::instance().information(title, text, buttons, defaultButton);
                break;
        }
    }

    terminateDialog(title);
    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        afMessageBox::critical
// Description: open a critical message box
// Return Val:  QMessageBox::StandardButton
// Author:      Gilad Yarnitzky
// Date:        31/12/2012
// ---------------------------------------------------------------------------
QMessageBox::StandardButton afMessageBox::critical(const QString& title, const QString& text, QMessageBox::StandardButtons buttons, QMessageBox::StandardButton defaultButton)
{
    QMessageBox::StandardButton retVal = QMessageBox::Ok;

    if (initDialog(title))
    {
        retVal = acMessageBox::instance().critical(title, text, buttons, defaultButton);
    }

    terminateDialog(title);

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        afMessageBox::critical
// Description: open an information message box
// Return Val:  QMessageBox::StandardButton
// Author:      Gilad Yarnitzky
// Date:        31/12/2012
// ---------------------------------------------------------------------------
QMessageBox::StandardButton afMessageBox::information(const QString& title, const QString& text, QMessageBox::StandardButtons buttons, QMessageBox::StandardButton defaultButton)
{
    QMessageBox::StandardButton retVal = QMessageBox::Ok;

    if (initDialog(title))
    {
        retVal = acMessageBox::instance().information(title, text, buttons, defaultButton);
    }

    terminateDialog(title);

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        afMessageBox::critical
// Description: open a critical question box
// Return Val:  QMessageBox::StandardButton
// Author:      Gilad Yarnitzky
// Date:        31/12/2012
// ---------------------------------------------------------------------------
QMessageBox::StandardButton afMessageBox::question(const QString& title, const QString& text, QMessageBox::StandardButtons buttons, QMessageBox::StandardButton defaultButton)
{
    QMessageBox::StandardButton retVal = QMessageBox::Ok;

    if (initDialog(title))
    {
        retVal = acMessageBox::instance().question(title, text, buttons, defaultButton);
    }

    terminateDialog(title);

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        afMessageBox::critical
// Description: open a warning message box
// Return Val:  QMessageBox::StandardButton
// Author:      Gilad Yarnitzky
// Date:        31/12/2012
// ---------------------------------------------------------------------------
QMessageBox::StandardButton afMessageBox::warning(const QString& title, const QString& text, QMessageBox::StandardButtons buttons, QMessageBox::StandardButton defaultButton)
{
    QMessageBox::StandardButton retVal = QMessageBox::Ok;

    if (initDialog(title))
    {
        retVal = acMessageBox::instance().warning(title, text, buttons, defaultButton);
    }

    terminateDialog(title);

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        afMessageBox::initDialog
// Description: Prepare the dialog and check if it needs opening at all
// Author:      Gilad Yarnitzky
// Date:        31/12/2012
// ---------------------------------------------------------------------------
bool afMessageBox::initDialog(const QString& title)
{
    bool retVal = true;

    // Check if the dialog needs to open at all:
    QString nonConstTitle(title);

    if (!afGlobalVariablesManager::instance().isSetNotToShowAgain(nonConstTitle))
    {
        acMessageBox& messageBox = acMessageBox::instance();
        messageBox.useHideDialogButton();
    }
    else
    {
        retVal = false;
    }

    return retVal;
}

// terminate dialog:

// ---------------------------------------------------------------------------
// Name:        afMessageBox::terminateDialog
// Description: Clear the acDialog we use and store user decision
// Author:      Gilad Yarnitzky
// Date:        31/12/2012
// ---------------------------------------------------------------------------
void afMessageBox::terminateDialog(const QString& title)
{
    QString nonConstTitle(title);
    acMessageBox& messageBox = acMessageBox::instance();
    Qt::CheckState state = messageBox.hideDialogState();

    if (Qt::Checked == state)
    {
        afGlobalVariablesManager::instance().setNotToShowAgain(nonConstTitle);
    }
}
