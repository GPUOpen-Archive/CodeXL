//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file acHelpAboutDialog.cpp
///
//==================================================================================

//------------------------------ acHelpAboutDialog.cpp ------------------------------
#include <AMDTApplicationComponents/Include/acHelpAboutDialog.h>

// Qt
#include <AMDTApplicationComponents/Include/acQtIncludes.h>

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTOSWrappers/Include/osDirectory.h>
#include <AMDTOSWrappers/Include/osApplication.h>
#include <AMDTOSWrappers/Include/osProductVersion.h>
#include <AMDTOSAPIWrappers/Include/oaDriver.h>
#include <VersionInfo/VersionInfo.h>

#include <AMDTApplicationComponents/inc/acStringConstants.h>
#include <AMDTApplicationComponents/Include/acFunctions.h>

// ---------------------------------------------------------------------------
// Name:        acHelpAboutDialog::acHelpAboutDialog
// Description: Definition of the Help About Dialog
// Arguments:   osExecutedApplicationType executionApplicationType
//              QWidget *pParent
// Author:      Yoni Rabin
// Date:        9/4/2012
// ---------------------------------------------------------------------------
acHelpAboutDialog::acHelpAboutDialog(osExecutedApplicationType executionApplicationType, QWidget* pParent)
    : QDialog(pParent), m_osExecutedApplicationType(executionApplicationType)
{
    GT_UNREFERENCED_PARAMETER(pParent);
}

void acHelpAboutDialog::Init(const QString& title, const QString& productName, const osProductVersion& appVersion, const QString& copyRightCaption, const QString& copyRightInformation,
                             const acIconId& productIconId, const gtString& versionCaption, const gtString& companyLogoBitmapString, bool addDescriptionString)
{
    m_productName = productName;
    // Set the dialog title:
    setWindowTitle(title);

    // Set the dialog icon:
    QPixmap iconPixMap;
    acSetIconInPixmap(iconPixMap, productIconId, AC_64x64_ICON);
    setWindowIcon(iconPixMap);

    // Set window flags (minimize / maximize / close buttons):
    Qt::WindowFlags flags = windowFlags();
    flags &= ~Qt::WindowContextHelpButtonHint;
    setWindowFlags(flags);

    // layout components:
    QHBoxLayout* pMainLayout = new QHBoxLayout;
    pMainLayout->setSizeConstraint(QLayout::SetFixedSize);

    QVBoxLayout* pVerticalLayoutLeft = new QVBoxLayout;
    QVBoxLayout* pVerticalLayoutRight = new QVBoxLayout;


    QString productVersion = acGTStringToQString(appVersion.toString());

    // Set the Help about string:
    QString helpAboutString = AC_STR_NewLineA;

    if (addDescriptionString)
    {
        switch (m_osExecutedApplicationType)
        {
            case OS_STANDALONE_APPLICATION_TYPE:
            {
                helpAboutString.append(acGTStringToQString(STRPRODUCTNAME));
            }
            break;

            case OS_VISUAL_STUDIO_PLUGIN_TYPE:
            {
                helpAboutString.append(acGTStringToQString(VS_PACKAGE_STRPRODUCTNAME));
            }
            break;
        }
    }
    else
    {
        helpAboutString.append(productName);
    }

    if (helpAboutString.indexOf("CodeXL") >= 0)
    {
        // Add the version string to the help caption:
        QString versionStr = QString(m_productName + acGTStringToQString(versionCaption));
        helpAboutString.replace("CodeXL", versionStr);
    }

    helpAboutString.append(AC_STR_NewLineA AC_STR_HelpAboutVersion);
    helpAboutString.append(productVersion);

    // Get the driver version
    int driverError = OA_DRIVER_UNKNOWN;
    gtString driverVersion = oaGetDriverVersion(driverError);

    if (driverError == OA_DRIVER_OK)
    {
        helpAboutString.append(AC_STR_NewLineA AC_STR_HelpAboutRadeonSoftwareVersion);
        helpAboutString.append(acGTStringToQString(driverVersion));
    }

    m_pCompanyLogo = new QPixmap(acGTStringToQString(companyLogoBitmapString), nullptr, Qt::AutoColor);


    // Create the QLabels from the strings:
    QLabel* pHelpAboutStringAsStaticText = new QLabel(helpAboutString);

    QLabel* pCopyRightCaptionAsStaticText = new QLabel(copyRightCaption);

    QLabel* pCopyRightInformationAsStaticText = new QLabel(copyRightInformation);


    // Set QLabels Alignments
    pHelpAboutStringAsStaticText->setAlignment(Qt::AlignCenter);
    pCopyRightCaptionAsStaticText->setAlignment(Qt::AlignCenter);
    pCopyRightInformationAsStaticText->setAlignment(Qt::AlignLeft);

    // Add the web page link:
    QString websiteURL = "<a href=\"" AC_STR_HelpAboutWebPage "\">" AC_STR_HelpAboutWebPageText "</a>";
    QLabel* pWebsiteURL = new QLabel(websiteURL);


    pWebsiteURL->setOpenExternalLinks(true);
    pWebsiteURL->setToolTip(AC_STR_HelpAboutWebPageText);

    // Add Horizontal Line:
    QFrame* pLine = new QFrame();


    pLine->setFrameShape(QFrame::HLine);
    pLine->setFrameShadow(QFrame::Sunken);

    // Add the OK button:
    QDialogButtonBox* pOKButtonBox = new QDialogButtonBox(QDialogButtonBox::Ok, Qt::Vertical);

    bool rc = connect(pOKButtonBox, SIGNAL(accepted()), this, SLOT(accept()));
    GT_ASSERT(rc);


    // Create QLabel to hold the image:
    QLabel* pCompanyLogoLbl = new QLabel();


    // Put the bitmap in a label for display:
    pCompanyLogoLbl->setPixmap(*m_pCompanyLogo);

    // Left Vertical Layout:
    pVerticalLayoutLeft->addStretch(1);
    pVerticalLayoutLeft->addWidget(pHelpAboutStringAsStaticText, 0, Qt::AlignCenter | Qt::AlignBottom);
    pVerticalLayoutLeft->addSpacing(5);
    pVerticalLayoutLeft->addWidget(pWebsiteURL, 0, Qt::AlignCenter | Qt::AlignBottom);
    pVerticalLayoutLeft->addStretch(1);
    pVerticalLayoutLeft->addWidget(pLine);
    pVerticalLayoutLeft->addWidget(pCopyRightCaptionAsStaticText, 0, Qt::AlignCenter);
    pVerticalLayoutLeft->addSpacing(5);
    pVerticalLayoutLeft->addWidget(pCopyRightInformationAsStaticText, 0, Qt::AlignLeft);
    pVerticalLayoutLeft->addSpacing(10);
    pVerticalLayoutLeft->addWidget(pOKButtonBox, 0, Qt::AlignCenter | Qt::AlignBottom);
    pVerticalLayoutLeft->setMargin(10);

    // Right Vertical Layout:
    pVerticalLayoutRight->addWidget(pCompanyLogoLbl);

    // Main Horizontal Layout:
    pMainLayout->addLayout(pVerticalLayoutRight);
    pMainLayout->addLayout(pVerticalLayoutLeft);
    pMainLayout->setMargin(0);

    // Activate:
    setLayout(pMainLayout);
}
