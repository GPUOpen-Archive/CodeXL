//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file acGoToLineDialog.h
///
//==================================================================================

//------------------------------ acGoToLineDialog.h ------------------------------

#ifndef __ACGOTOLINEDIALOG
#define __ACGOTOLINEDIALOG

// QT:
#include <QtWidgets>

// Infra:
#include <AMDTBaseTools/Include/AMDTDefinitions.h>
#include <AMDTApplicationComponents/Include/acApplicationComponentsDLLBuild.h>
#include <AMDTApplicationComponents/Include/acDialog.h>

// ----------------------------------------------------------------------------------
// Class Name:          AC_API acGoToLineDialog : public acDialog
// General Description: Go to line dialog
// Author:              Naama Zur
// Creation Date:       08/5/2016
// ----------------------------------------------------------------------------------
class AC_API acGoToLineDialog : public acDialog
{
    Q_OBJECT

public:
    acGoToLineDialog(QWidget* pParent, unsigned int numLines);
    ~acGoToLineDialog();

    unsigned int GetLineNumber()const;

protected slots:
    /// OnOk: go to line
    void OnOk();

private:
    QVBoxLayout* m_pMainLayout;
    QHBoxLayout* m_pButtonBox;
    QPushButton* m_pPushButtonOK;
    QPushButton* m_pPushButtonCancel;

    QLabel*     m_pDialogCaption;
    QLabel*     m_pGotoLineInstructionsLabel;
    QLineEdit*  m_pLineEdit;

    unsigned int m_numSelectedLine;
};



#endif  // acGoToLineDialog
