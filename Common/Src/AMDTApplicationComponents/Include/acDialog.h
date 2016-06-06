//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file acDialog.h
///
//==================================================================================

//------------------------------ acDialog.h ------------------------------

#ifndef __ACDIALOG
#define __ACDIALOG

// QT:
#include <QtWidgets>

// Infra:
#include <AMDTBaseTools/Include/AMDTDefinitions.h>
#include <AMDTBaseTools/Include/gtString.h>
#include <AMDTApplicationComponents/Include/acQtIncludes.h>
#include <AMDTApplicationComponents/Include/acApplicationComponentsDLLBuild.h>


// ----------------------------------------------------------------------------------
// Class Name:          AC_API acDialog : public QDialog
// General Description: Base class for QT dialogs
// Author:              Yoni Rabin
// Creation Date:       24/6/2012
// ----------------------------------------------------------------------------------
class AC_API acDialog : public QDialog
{
public:
    acDialog(QWidget* parent = 0, bool hasOkButton = true, bool hasCancelButton = true, QDialogButtonBox::StandardButton defaultButton = QDialogButtonBox::NoButton, QList<QPushButton*>* pCustomButtons = NULL, Qt::WindowFlags f = (Qt::Dialog | Qt::MSWindowsFixedSizeDialogHint) & ~Qt::WindowContextHelpButtonHint);
    virtual ~acDialog();

    QHBoxLayout* getBottomButtonLayout(bool hasLogo = false, const gtString& filePathwithLogo = L"", const gtString& OKButtonNewText = L"");

    // These are public for now so that we can connect to their events:
    QList<QPushButton*>* m_pCustomButtons;

protected:
    QPixmap* setLogoPixmap(const gtString& filePath);

    // the cancel and ok buttons
    QPushButton* m_pOKButton;

private:
    QDialogButtonBox::StandardButton m_defaultButton;
    QPixmap* m_pLogo;
    bool m_hasOkButton;
    bool m_hasCancelButton;
};


#endif  // __acDialog
