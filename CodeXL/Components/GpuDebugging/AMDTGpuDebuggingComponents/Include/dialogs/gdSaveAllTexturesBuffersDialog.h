//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gdSaveAllTexturesBuffersDialog.h
///
//==================================================================================

//------------------------------ gdSaveAllTexturesBuffersDialog.h ------------------------------

#ifndef __GDSAVEALLTEXTURESBUFFERSDIALOG
#define __GDSAVEALLTEXTURESBUFFERSDIALOG

#include <QtWidgets>
// Forward decelerations:
class acTextCtrl;
struct FIBITMAP;

// Infra:
#include <AMDTBaseTools/Include/gtVector.h>
#include <AMDTAPIClasses/Include/apFileType.h>

// Local:
#include <AMDTGpuDebuggingComponents/Include/gdGDebuggerAppDLLBuild.h>


// ----------------------------------------------------------------------------------
// Class Name:           gdSaveAllTexturesBuffersDialog : public QDialog
// General Description:  A dialog used by the texture viewer to save all textures
//                       and buffers in one strike.
// Author:               Eran Zinman
// Creation Date:        5/1/2008
// ----------------------------------------------------------------------------------
class GD_API gdSaveAllTexturesBuffersDialog : public QDialog
{
    Q_OBJECT

public:
    // Constructor:
    gdSaveAllTexturesBuffersDialog(QWidget* pParent, const QString& title, const gtVector<apFileType>& fileTypes);

    // Destructor:
    ~gdSaveAllTexturesBuffersDialog();

public:
    // Get the selected file type
    bool getFileType(apFileType& fileType);

    // Get the directory to save the image into
    bool getOutputDirectory(gtString& outputDir);

    // Should we overwrite files?
    bool shouldOverwriteFiles();

protected slots:

    // On path button command:
    void onPathButton();

    // On Ok button command:
    void onOkButton();

private:

    // Set dialog layout
    void setDialogLayout();

    // Set dialog default values
    void setDialogValues();

    // Generate file type description
    bool generateFileTypeDescription(apFileType fileType, QString& description);

private:
    // The directory list text control
    QLineEdit* _pDirectoryTextCtrl;

    // Supported file types array:
    const gtVector<apFileType> _fileTypes;

    // File types radio button array (a pointer to an array of radio buttons pointers)
    QRadioButton** _pRadioButtonArray;

    // File overwrite check box
    QCheckBox* _pFileOverwriteCheckBox;

};


#endif  // __GDSAVEALLTEXTURESBUFFERSDIALOG
