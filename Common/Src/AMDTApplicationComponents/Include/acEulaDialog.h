//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file acEulaDialog.h
///
//==================================================================================

//------------------------------ acEulaDialog.h ------------------------------

#ifndef __ACEULADIALOG
#define __ACEULADIALOG

// Qt
#include <QtWidgets>

// Infra:
#include <AMDTBaseTools/Include/AMDTDefinitions.h>
#include <AMDTOSWrappers/Include/osFilePath.h>

// Local:
#include <AMDTApplicationComponents/Include/acApplicationComponentsDLLBuild.h>
#include <AMDTApplicationComponents/Include/acIcons.h>

class acQHTMLWindow;

// ----------------------------------------------------------------------------------
// Class Name:              acEulaDialog : public QDialog
// General Description:     Pops before the first use of the application. User must
//                          accept the terms of agreement.
// Author:               Guy Ilany
// Creation Date:        19/12/2007
// ----------------------------------------------------------------------------------
class AC_API acEulaDialog : public QDialog
{
    Q_OBJECT

public:
    acEulaDialog(QWidget* parent, const acIconId& productIconId);
    ~acEulaDialog();

    void setHtmlStringIntoDialog(const osFilePath& EULAFilePath);
    int execute();

private:
    QVBoxLayout* _pSizer;
    QHBoxLayout* _pButtonSizer;
    QPushButton* _pNextButton;
    QPushButton* _pExitButton;
    QLabel* _pSubTitle;
    acQHTMLWindow* _pHtmlWindow;
    QRadioButton* _pAgreeRadioButton;
    QRadioButton* _pDoNotAgreeRadioButton;
    bool _loadEULA_OK;
    gtByte* _pEulaBuffer;

private:
    void setDialogLayout();
    void setDialogInitialValues();

protected slots:
    void OnUpdateUINextButton();
    void onHTMLWindowLinkClicked(const QUrl& urlClicked);
};


#endif  // __ACEULADIALOG
