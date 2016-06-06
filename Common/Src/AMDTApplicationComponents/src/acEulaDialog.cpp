//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file acEulaDialog.cpp
///
//==================================================================================

//------------------------------ acEulaDialog.cpp ------------------------------
#include <AMDTApplicationComponents/Include/acEulaDialog.h>

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
#include <AMDTApplicationComponents/inc/acStringConstants.h>

// Local:
//#include <AMDTApplicationFramework/Include/afAidFunctions.h>
//#include <AMDTApplicationFramework/Include/afCommandIds.h>
//#include <AMDTApplicationFramework/Include/afAppStringConstants.h>
//#include <AMDTApplicationFramework/Include/afGlobalVariablesManager.h>


// ----------------------------------------------------------------------------------
// Class Name:           acEulaDialog::acEulaDialog(QWidget *parent)
// General Description:  Constructor.
// Arguments:
//          parent - the parent window that generated the EULA dialog.
// Author:               Gilad Yarnitzky
// Creation Date:        8/11/2012
// ----------------------------------------------------------------------------------
acEulaDialog::acEulaDialog(QWidget* parent, const acIconId& productIconId)
    :  QDialog(nullptr), _loadEULA_OK(false), _pEulaBuffer(nullptr)
{
    GT_UNREFERENCED_PARAMETER(parent);

    // Set the dialog title:
    setWindowTitle(AC_STR_EULALinuxTitle);

    // Set window flags (minimize / maximize / close buttons):
    Qt::WindowFlags flags = windowFlags();
    flags &= ~Qt::WindowContextHelpButtonHint;
    setWindowFlags(flags);

    // Add the Icon to the dialog
    QPixmap iconPixMap;
    acSetIconInPixmap(iconPixMap, productIconId, AC_64x64_ICON);
    setWindowIcon(iconPixMap);

    // Set the dialog layout
    setDialogLayout();

    // Set my size:
    resize(QSize(800, 550));
}

// ---------------------------------------------------------------------------
// Name:        ~acEulaDialog
// Description:
// Return Val:
// Author:      Avi Shapira
// Date:        21/5/2006
// ---------------------------------------------------------------------------
acEulaDialog::~acEulaDialog()
{
    delete _pEulaBuffer;
    _pEulaBuffer = nullptr;
}

// ---------------------------------------------------------------------------
// Name:        acEulaDialog::execute
// Description: Overrides the QDialog execute
// Return Val:  int
// Author:      Avi Shapira
// Date:        21/5/2006
// ---------------------------------------------------------------------------
int acEulaDialog::execute()
{
    int retVal = exec();

    // If the user pressed ok and accepted the EULA terms
    if (QDialog::Accepted == retVal)
    {
        // Get the current application version:
        osProductVersion currentAppVersion;
        osGetApplicationVersion(currentAppVersion);
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        SetDialogLayout
// Description:
// Author:      Avi Shapira
// Date:        21/5/2006
// ---------------------------------------------------------------------------
void acEulaDialog::setDialogLayout()
{
    _pHtmlWindow = new acQHTMLWindow(nullptr);


    // connect the anchor:
    connect(_pHtmlWindow, SIGNAL(anchorClicked(const QUrl&)), this, SLOT(onHTMLWindowLinkClicked(const QUrl&)));

    // Create the "I agree / Do not agree" radio button
    _pAgreeRadioButton = new QRadioButton(AC_STR_EULA_Agree);


    _pDoNotAgreeRadioButton = new QRadioButton(AC_STR_EULA_DoNotAgree);


    // Add the connection
    connect(_pDoNotAgreeRadioButton, SIGNAL(clicked()), this, SLOT(OnUpdateUINextButton()));
    connect(_pAgreeRadioButton, SIGNAL(clicked()), this, SLOT(OnUpdateUINextButton()));

    _pNextButton = new QPushButton(AC_STR_EULA_NextButton);


    _pExitButton = new QPushButton(AC_STR_EULA_ExitButton);


    // add the connection
    bool rcConnect = connect(_pNextButton, SIGNAL(clicked()), this, SLOT(accept()));
    GT_ASSERT(rcConnect);
    rcConnect = connect(_pExitButton, SIGNAL(clicked()), this, SLOT(reject()));
    GT_ASSERT(rcConnect);

    _pSubTitle = new QLabel(AC_STR_EULASubTitle);


    // Create the button layout and and the widget to it:
    _pButtonSizer = new QHBoxLayout();


    _pButtonSizer->addWidget(new QLabel(), 1);
    _pButtonSizer->addWidget(_pNextButton, 0, Qt::AlignRight | Qt::AlignBottom);
    _pButtonSizer->addWidget(_pExitButton, 0, Qt::AlignRight | Qt::AlignBottom);

    // Create the main layout and add all the controls to it:
    _pSizer = new QVBoxLayout;


    _pSizer->addWidget(_pSubTitle, 0, Qt::AlignLeft | Qt::AlignTop);
    _pSizer->addSpacing(10);
    _pSizer->addWidget(_pHtmlWindow, 1, 0);
    _pSizer->addSpacing(10);
    _pSizer->addWidget(_pAgreeRadioButton, 0, Qt::AlignLeft);
    _pSizer->addSpacing(10);
    _pSizer->addWidget(_pDoNotAgreeRadioButton, 0, Qt::AlignLeft | Qt::AlignBottom);
    _pSizer->addSpacing(10);
    _pSizer->addLayout(_pButtonSizer);

    // Set the I do not agree radio button to be the default:
    _pDoNotAgreeRadioButton->setChecked(true);
    _pDoNotAgreeRadioButton->setFocus();

    setLayout(_pSizer);

    OnUpdateUINextButton();
}

// ---------------------------------------------------------------------------
// Name:        acEulaDialog::setHtmlStringIntoDialog
// Description: Set html string into the output dialog.
// Author:      Avi Shapira
// Date:        21/5/2006
// ---------------------------------------------------------------------------
void acEulaDialog::setHtmlStringIntoDialog(const osFilePath& EULAdir)
{
    bool rc = false;

    // Get the CodeXL EULA directory
    osFilePath EULAFilePath = EULAdir;
    EULAFilePath.appendSubDirectory(AC_STR_EULADirectory);

    EULAFilePath.setFileName(AC_STR_EULAFileName);
    EULAFilePath.setFileExtension(AC_STR_EULAFileExtension);

    // If the license agreement file exists:
    if (EULAFilePath.isRegularFile())
    {
        osFile htmlFile(EULAFilePath);
        rc = htmlFile.open(EULAFilePath, osFile::OS_ASCII_TEXT_CHANNEL);

        GT_IF_WITH_ASSERT(rc)
        {
            unsigned long fileSize;
            rc = htmlFile.getSize(fileSize);
            GT_IF_WITH_ASSERT(rc)
            {
                // Load the file into a buffer;
                _pEulaBuffer = new gtByte[fileSize + 1];
                gtSize_t readData;

                // Read data and add end string:
                rc = htmlFile.readAvailableData(_pEulaBuffer, fileSize, readData);
                _pEulaBuffer[fileSize] = 0;
                GT_IF_WITH_ASSERT(rc)
                {
                    // Convert the buffer into html:
                    QString htmlAsString((char*)_pEulaBuffer);
                    _pHtmlWindow->QTextBrowser::setHtml(htmlAsString);
                    rc = true;
                }
            }
        }
        htmlFile.close();
    }


    if (rc)
    {
        _loadEULA_OK = true;
    }
    else
    {
        // We failed to load the html file:
        OS_OUTPUT_DEBUG_LOG(AC_STR_EULALoadHtmlError, OS_DEBUG_LOG_ERROR);

        _pHtmlWindow->setText(acGTStringToQString(AC_STR_EULALoadHtmlError));
    }
}


// ---------------------------------------------------------------------------
// Name:        acEulaDialog::OnUpdateUINextButton
// Description: Update the "Next" Button
// Arguments:   wxUpdateUI &eve
// Author:      Avi Shapira
// Date:        21/5/2006
// ---------------------------------------------------------------------------
void acEulaDialog::OnUpdateUINextButton()
{
    bool isEnable = false;

    if (_loadEULA_OK && _pAgreeRadioButton->isChecked())
    {
        isEnable = true;
    }

    _pNextButton->setEnabled(isEnable);
}

// ---------------------------------------------------------------------------
// Name:        acEulaDialog::onHTMLWindowLinkClicked
// Description: Handles the event of a user clicking on a link in the HTML
//              window by opening the default browser with the link's target
// Arguments: eve - the clicking event
// Author:      Uri Shomroni
// Date:        2/4/2008
// ---------------------------------------------------------------------------
void acEulaDialog::onHTMLWindowLinkClicked(const QUrl& urlClicked)
{
    QString urlAsQString = urlClicked.toString();
    gtString clickedLinkTarget;
    clickedLinkTarget.fromASCIIString(urlAsQString.toLatin1().data());
    osFileLauncher fileLauncher(clickedLinkTarget.asCharArray());
    bool rc = fileLauncher.launchFile();
    GT_ASSERT(rc);
}

