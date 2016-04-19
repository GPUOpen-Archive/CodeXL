//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gdSaveAllTexturesBuffersDialog.cpp
///
//==================================================================================

//------------------------------ gdSaveAllTexturesBuffersDialog.cpp ------------------------------

#include <AMDTBaseTools/Include/gtIgnoreCompilerWarnings.h>

#include <QtWidgets>

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTOSWrappers/Include/osDirectory.h>
#include <AMDTOSWrappers/Include/osApplication.h>
#include <AMDTApplicationComponents/Include/acMessageBox.h>
#include <AMDTApplicationComponents/Include/acFunctions.h>

// Local:
#include <AMDTGpuDebuggingComponents/Include/gdApplicationCommands.h>
#include <AMDTGpuDebuggingComponents/Include/gdStringConstants.h>
#include <AMDTGpuDebuggingComponents/Include/gdCommandIDs.h>
#include <AMDTGpuDebuggingComponents/Include/gdAidFunctions.h>
#include <AMDTGpuDebuggingComponents/Include/gdGDebuggerGlobalVariablesManager.h>
#include <AMDTGpuDebuggingComponents/Include/dialogs/gdSaveAllTexturesBuffersDialog.h>


// ---------------------------------------------------------------------------
// Name:        gdSaveAllTexturesBuffersDialog::gdSaveAllTexturesBuffersDialog
// Description: Constructor
// Arguments:   pParent - Dialog parent
//              fileTypes - A vector containing all supported file types
// Author:      Eran Zinman
// Date:        5/1/2008
// ---------------------------------------------------------------------------
gdSaveAllTexturesBuffersDialog::gdSaveAllTexturesBuffersDialog(QWidget* pParent, const QString& title, const gtVector<apFileType>& fileTypes)
    : QDialog(pParent),
      _pDirectoryTextCtrl(NULL), _fileTypes(fileTypes), _pRadioButtonArray(NULL), _pFileOverwriteCheckBox(NULL)
{
    (void)(title); // unused
    // Set dialog layout
    setDialogLayout();

    // Set dialog default values
    setDialogValues();
}

// ---------------------------------------------------------------------------
// Name:        gdSaveAllTexturesBuffersDialog::~gdSaveAllTexturesBuffersDialog
// Description: Destructor
// Author:      Eran Zinman
// Date:        13/1/2008
// ---------------------------------------------------------------------------
gdSaveAllTexturesBuffersDialog::~gdSaveAllTexturesBuffersDialog()
{
    // Delete export file types array (actual radio buttons will
    // be deleted by WX)
    if (_pRadioButtonArray)
    {
        delete _pRadioButtonArray;
    }
}

// ---------------------------------------------------------------------------
// Name:        gdSaveAllTexturesBuffersDialog::shouldOverwriteFiles
// Description: Checks if we should overwrite files?
// Return Val:  True - overwrite files, false - otherwise
// Author:      Eran Zinman
// Date:        6/1/2008
// ---------------------------------------------------------------------------
bool gdSaveAllTexturesBuffersDialog::shouldOverwriteFiles()
{
    bool retVal = false;

    // Sanity check:
    GT_IF_WITH_ASSERT(_pFileOverwriteCheckBox != NULL)
    {
        retVal = (_pFileOverwriteCheckBox->checkState() == Qt::Checked);
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gdSaveAllTexturesBuffersDialog::onPathButton
// Description: Occurs when "path" button is being clicked
// Arguments:   event - Event details
// Author:      Eran Zinman
// Date:        6/1/2008
// ---------------------------------------------------------------------------
void gdSaveAllTexturesBuffersDialog::onPathButton()
{
    // Sanity check:
    // Get the application commands instance:
    afApplicationCommands* pApplicationCommands = afApplicationCommands::instance();
    GT_IF_WITH_ASSERT((_pDirectoryTextCtrl != NULL) && (pApplicationCommands != NULL))
    {
        // Get current selected path
        gtString currentDir;
        bool rc = getOutputDirectory(currentDir);
        GT_IF_WITH_ASSERT(rc)
        {
            GT_IF_WITH_ASSERT(pApplicationCommands != NULL)
            {
                QString defaultFolder = acGTStringToQString(currentDir);
                QString selectedFolder = pApplicationCommands->ShowFolderSelectionDialog(GD_STR_SaveAllTexturesBufferDialogChooseOutputDir, defaultFolder);

                if (!selectedFolder.isEmpty())
                {
                    // Use the path chosen in the directory dialog
                    _pDirectoryTextCtrl->setText(selectedFolder);
                }

            }
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        gdSaveAllTexturesBuffersDialog::onOkButton
// Description: Upon pressing the Ok Button, the program checks to see if the
//              directory in the textctrl exists. If it doesn't offers to create
//              it. if user chooses not to, stops the closing of the window.
// Arguments: event - Event details
// Author:      Uri Shomroni
// Date:        3/4/2008
// ---------------------------------------------------------------------------
void gdSaveAllTexturesBuffersDialog::onOkButton()
{
    gtString pathAsString;
    bool success = false;
    bool rc1 = getOutputDirectory(pathAsString);
    GT_IF_WITH_ASSERT(rc1)
    {
        /*  if(pathAsString.reverseFind(osFilePath::osPathSeparator) != (pathAsString.length() - 1))
            {
            pathAsString.append(osFilePath::osPathSeparator);
            }*/
        osFilePath directoryPath(pathAsString);
        bool doesDirectoryExist = directoryPath.isDirectory();

        if (doesDirectoryExist)
        {
            success = true;
        }
        else
        {
            gtString createNewDirectoryPrompt;
            createNewDirectoryPrompt.appendFormattedString(GD_STR_SaveAllTexturesAndBuffersDialogCreateNewDirPrompt, pathAsString.asCharArray());
            QMessageBox::StandardButton userSelection = acMessageBox::instance().question(GD_STR_SaveAllTexturesAndBuffersDialogCreateNewDirTitle, acGTStringToQString(createNewDirectoryPrompt), QMessageBox::Yes | QMessageBox::No);

            if (userSelection == QMessageBox::Yes)
            {
                osDirectory directory;
                directory.setDirectoryFullPathFromString(directoryPath.asString());
                //bool rc3 = directoryPath.getFileDirectory(directory);

                bool rc4 = directory.create();
                GT_IF_WITH_ASSERT(rc4)
                {
                    success = true;
                }
                else
                {
                    QString directoryCreationFailed = QString(GD_STR_SaveAllTexturesAndBuffersDialogCreateNewDirFailed).arg(acGTStringToQString(pathAsString));
                    acMessageBox::instance().critical(GD_STR_SaveAllTexturesAndBuffersDialogCreateNewDirFailedTitle, directoryCreationFailed);
                }
            }
        }
    }

    if (success)
    {
        accept();
    }
}

// ---------------------------------------------------------------------------
// Name:        gdSaveAllTexturesBuffersDialog::setDialogLayout
// Description: Sets dialog layout
// Author:      Eran Zinman
// Date:        5/1/2008
// ---------------------------------------------------------------------------
void gdSaveAllTexturesBuffersDialog::setDialogLayout()
{
    // Create main sizer (vertical sizer)
    QVBoxLayout* pMainLayout = new QVBoxLayout;


    QLabel* pLabel = new QLabel(GD_STR_SaveAllTexturesBufferDialogExportSettingsTitle);


    pMainLayout->addWidget(pLabel);

    QGroupBox* pGroupBox = new QGroupBox(GD_STR_SaveAllTexturesBufferDialogExportSettingsOutputFormat);


    pMainLayout->addWidget(pGroupBox);

    QVBoxLayout* pRadioButtonsLayout = new QVBoxLayout(pGroupBox);

    // Get amount of file types to display
    int amountOfFileTypes = _fileTypes.size();

    // Create the file types radio buttons array
    _pRadioButtonArray = new QRadioButton*[amountOfFileTypes];


    // Get image type format
    gdGDebuggerGlobalVariablesManager& theGDGlobalVariablesManager = gdGDebuggerGlobalVariablesManager::instance();
    apFileType imageType = theGDGlobalVariablesManager.imagesFileFormat();

    // Create all radio buttons
    for (int i = 0; i < amountOfFileTypes; i++)
    {
        // Generate a file type description
        QString fileTypeDescription;
        bool rc1 = generateFileTypeDescription(_fileTypes[i], fileTypeDescription);
        GT_IF_WITH_ASSERT(rc1)
        {
            // Create a radio button with the given description
            _pRadioButtonArray[i] = new QRadioButton(fileTypeDescription);
            pRadioButtonsLayout->insertWidget(i, _pRadioButtonArray[i]);
        }

        if (static_cast<apFileType>(i) == imageType)
        {
            _pRadioButtonArray[i]->setChecked(true);
        }
    }

    pRadioButtonsLayout->addStretch(1);


    QVBoxLayout* pGroupBoxLayout = new QVBoxLayout;

    // Create a horizontal sizer that will hold the directory browser
    QHBoxLayout* pDirectoryBrowserLayout = new QHBoxLayout;


    QToolButton* pFolderPathButton = new QToolButton;

    pFolderPathButton->setIcon(style()->standardIcon(QStyle::SP_DirIcon));
    pFolderPathButton->setContentsMargins(0, 0, 0, 0);

    bool rc = connect(pFolderPathButton, SIGNAL(clicked()), this, SLOT(onPathButton()));
    GT_ASSERT(rc);

    // Create a text control for the directory browser:
    _pDirectoryTextCtrl = new QLineEdit;


    // Add both widgets to the horizontal layout:
    pDirectoryBrowserLayout->addWidget(_pDirectoryTextCtrl);
    pDirectoryBrowserLayout->addWidget(pFolderPathButton);

    // Create a overwrite check box
    _pFileOverwriteCheckBox = new QCheckBox(GD_STR_SaveAllTexturesBufferDialogExportSettingsOverwrite);


    // Check the check box
    _pFileOverwriteCheckBox->setChecked(false);

    pGroupBoxLayout->addLayout(pDirectoryBrowserLayout);
    pGroupBoxLayout->addWidget(_pFileOverwriteCheckBox);

    pMainLayout->addLayout(pGroupBoxLayout);

    // Create the dialog buttons:
    QDialogButtonBox* pBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);


    // Connect the Ok button:
    rc = connect(pBox, SIGNAL(accepted()), this, SLOT(onOkButton()));
    GT_ASSERT(rc);

    // Connect Cancel button
    rc = connect(pBox, SIGNAL(rejected()), this, SLOT(reject()));
    GT_ASSERT(rc);

    pMainLayout->addWidget(pBox);

    // Set my layout:
    setLayout(pMainLayout);

    layout();


    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

}

// ---------------------------------------------------------------------------
// Name:        gdSaveAllTexturesBuffersDialog::generateFileTypeDescription
// Description: Generate file type description
// Arguments:   fileType - File type to generate a description for
//              description - Output file type description
// Return Val:  bool - Success / Failure
// Author:      Eran Zinman
// Date:        5/1/2008
// ---------------------------------------------------------------------------
bool gdSaveAllTexturesBuffersDialog::generateFileTypeDescription(apFileType fileType, QString& description)
{
    bool retVal = true;

    switch (fileType)
    {
        case AP_PNG_FILE:
            description = GD_STR_TexturesAndBuffersLoggingFormatPng;
            break;

        case AP_JPEG_FILE:
            description = GD_STR_TexturesAndBuffersLoggingFormatJpeg;
            break;

        case AP_TIFF_FILE:
            description = GD_STR_TexturesAndBuffersLoggingFormatTiff;
            break;

        case AP_BMP_FILE:
            description = GD_STR_TexturesAndBuffersLoggingFormatBmp;
            break;

        case AP_CSV_FILE:
            description = GD_STR_TexturesAndBuffersLoggingFormatCSV;
            break;

        default:
        {
            GT_ASSERT_EX(false, L"Unsupported file types!");
            retVal = false;
        }
        break;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gdSaveAllTexturesBuffersDialog::setDialogValues
// Description: Sets the dialog default values
// Author:      Eran Zinman
// Date:        5/1/2008
// ---------------------------------------------------------------------------
void gdSaveAllTexturesBuffersDialog::setDialogValues()
{
    // Set default directory for output directory
    GT_IF_WITH_ASSERT(_pDirectoryTextCtrl != NULL)
    {
        // Set the default directory to be OS_USER_DOCUMENTS
        osFilePath myDocumentsDir(osFilePath::OS_USER_DOCUMENTS);
        gtString defaultDirectory = myDocumentsDir.asString();

        // Set the directory
        _pDirectoryTextCtrl->setText(acGTStringToQString(defaultDirectory));
    }
}

// ---------------------------------------------------------------------------
// Name:        gdSaveAllTexturesBuffersDialog::getFileType
// Description: Get the selected file type format
// Arguments:   fileType - Output file format
// Return Val:  bool - Success / Failure
// Author:      Eran Zinman
// Date:        5/1/2008
// ---------------------------------------------------------------------------
bool gdSaveAllTexturesBuffersDialog::getFileType(apFileType& fileType)
{
    bool retVal = false;

    // Sanity check:
    GT_IF_WITH_ASSERT(_pRadioButtonArray != NULL)
    {
        // Get amount of file types to display
        int amountOfFileTypes = _fileTypes.size();

        // Loop through all the radio buttons and find the selected radio button
        for (int i = 0; i < amountOfFileTypes; i++)
        {
            GT_IF_WITH_ASSERT(_pRadioButtonArray[i] != NULL)
            {
                // Is radio button checked?
                bool isChecked = _pRadioButtonArray[i]->isChecked();

                if (isChecked)
                {
                    // Return the selected file type
                    fileType = _fileTypes[i];

                    // Break the loop
                    retVal = true;
                    break;
                }
            }
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gdSaveAllTexturesBuffersDialog::getOutputDirectory
// Description: Get the directory to save the files into
// Arguments:   outputDir - Output directory to save the files into.
// Return Val:  bool - Success / Failure
// Author:      Eran Zinman
// Date:        5/1/2008
// ---------------------------------------------------------------------------
bool gdSaveAllTexturesBuffersDialog::getOutputDirectory(gtString& outputDir)
{
    bool retVal = false;

    // Empty output directory
    outputDir.makeEmpty();

    // Sanity check:
    GT_IF_WITH_ASSERT(_pDirectoryTextCtrl != NULL)
    {
        // Get output directory
        outputDir.fromASCIIString(_pDirectoryTextCtrl->text().toLatin1().data());

        retVal = true;
    }

    return retVal;
}

