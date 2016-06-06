//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file acQMessageDialog.h
///
//==================================================================================

//------------------------------ acQMessageDialog.h ------------------------------

#ifndef __ACQMESSAGEDIALOG_H
#define __ACQMESSAGEDIALOG_H

// Warnings:
#include <AMDTBaseTools/Include/gtIgnoreCompilerWarnings.h>

// Qt:
#include <QtWidgets>

// Local:
//#include <AMDTApplicationComponents/Include/acDialog.h>
#include <AMDTApplicationComponents/Include/acApplicationComponentsDLLBuild.h>

// ----------------------------------------------------------------------------------
// Class Name:          acQMessageDialog : public QDialog
// General Description: Implement a message dialog in Qt
// Author:              Sigal Algranaty
// Creation Date:       13/5/2012
// ----------------------------------------------------------------------------------
class AC_API acQMessageDialog : public QDialog
{
public:
    acQMessageDialog(const QString& title, const QString& header, const QString& message, QWidget* pParent, QSize size = QSize(600, 350));
private:
    QTextEdit* m_pTextBox;
    QPalette m_palette;
};

#endif //__ACQMESSAGEDIALOG_H

