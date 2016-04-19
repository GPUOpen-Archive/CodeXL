//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gdDoubleProgressBarDialog.h
///
//==================================================================================

//------------------------------ gdDoubleProgressBarDialog.h ------------------------------

#ifndef __GDDOUBLEPROGRESSBARDIALOG
#define __GDDOUBLEPROGRESSBARDIALOG

// Qt:
#include <QDialog>
#include <QProgressBar>

QT_BEGIN_NAMESPACE
class QTextEdit;
class QLabel;
class QDialogButtonBox;
QT_END_NAMESPACE
// Infra:
#include <AMDTBaseTools/Include/gtString.h>
#include <AMDTApplicationComponents/Include/acDialog.h>

// Local:
#include <AMDTGpuDebuggingComponents/Include/gdGDebuggerAppDLLBuild.h>

// ----------------------------------------------------------------------------------
// Class Name:           gdDoubleProgressBarDialog : public QDialog
// General Description:  A dialog two have two progress bars that allows you to show
//                       general progress and current item progress.
// Author:               Eran Zinman
// Creation Date:        7/1/2008
// ----------------------------------------------------------------------------------
class GD_API gdDoubleProgressBarDialog : public acDialog
{
public:
    // Constructor:
    gdDoubleProgressBarDialog(QWidget* pParent, const gtString& title, const gtString& subTitle);

public:

    // Updates the text of the current item
    bool updateText(const gtString& newText);

    // Two access functions to update current item progress and total progress
    bool updateTotalProgress(int newValue) { return updateGaugeValue(m_pTotalProgress, newValue); };
    bool updateItemProgress(int newValue) { return updateGaugeValue(m_pItemProgress, newValue); };

    // Add an entry to the log text control
    bool addEntryToLog(const gtASCIIString& logEntry);

    // Notifies the double progress bar dialog that we are done
    void finishOperation();

private:
    // Set dialog layout
    void setDialogLayout();

    // Update a progress bar value
    bool updateGaugeValue(QProgressBar* pGauge, int newValue);

    // Enables the "OK" button (allows the user to close the dialog by clicking "ok");
    void enableOkButton();

private:
    // The text control which log's the progress
    QTextEdit* m_pLogTextCtrl;

    // Two progress bars; One for total progress and one for current item progress
    QProgressBar* m_pTotalProgress;
    QProgressBar* m_pItemProgress;

    // The text description of the current item
    QLabel* m_pItemText;

    // The dialog sub title
    gtString m_dialogSubTitle;

    QDialogButtonBox* m_pDialogButtonBox;

};


#endif  // __GDDOUBLEPROGRESSBARDIALOG
