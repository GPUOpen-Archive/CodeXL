//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gdAPICallsHistoryPanel.h
///
//==================================================================================

//------------------------------ gdAPICallsHistoryPanel.h ------------------------------

#ifndef __GDAPICALLSHISTORYPANEL_H
#define __GDAPICALLSHISTORYPANEL_H

// Infra:
#include <AMDTBaseTools/Include/gtIgnoreCompilerWarnings.h>

// Qt:
#include <QWidget>

// Forward declarations:
class gdAPICallsHistoryView;
class gdCallsHistoryToolbar;
class afProgressBarWrapper;

// AMDTApplicationFramework:
#include <AMDTApplicationFramework/Include/views/afBaseView.h>

// Local:
#include <AMDTGpuDebuggingComponents/Include/gdGDebuggerAppDLLBuild.h>

// ----------------------------------------------------------------------------------
// Class Name:          gdAPICallsHistoryPanel: public wxPanel
// General Description: Used to encapsulate gdAPICallsHistoryView with a toolbar
// Author:              Gilad Yarnitzky
// Creation Date:       27/2/2011
// ----------------------------------------------------------------------------------
class GD_API gdAPICallsHistoryPanel: public QWidget, public afBaseView
{
    Q_OBJECT

public:

    gdAPICallsHistoryPanel(afProgressBarWrapper* pProgressBar, QWidget* pParent, bool isGlobal);
    ~gdAPICallsHistoryPanel();

    gdAPICallsHistoryView* historyView() { return m_pCallsHistoryView; }

    // Update toolbar commands:
    virtual void updateToolbarCommands();

    virtual void onUpdateEdit_SelectAll(bool& isEnabled);
    virtual void onUpdateEdit_Find(bool& isEnabled);
    virtual void onUpdateEdit_FindNext(bool& isEnabled);

    virtual void onEdit_Copy();
    virtual void onEdit_SelectAll();
    virtual void onEdit_Find();
    virtual void onEdit_FindNext();

    // Block select all (virtual list):
    virtual void selectAllItems(bool bSelect) { (void)(bSelect); };

public slots:

    /// Slots implementing the find command. Notice: This slot names cannot be changed, since it is connected in the construction of the main window
    /// Is called when the main window find is clicked:
    void onFindClick();

    /// Is called when the main window find next is clicked:
    void onFindNext();

    /// Is called when the main window find previous is clicked:
    void onFindPrev();

protected:

    void setFrameLayout(afProgressBarWrapper* pProgressBar, bool isGlobal);

    // Calls History view:
    gdAPICallsHistoryView* m_pCallsHistoryView;

    // Toolbar:
    gdCallsHistoryToolbar* m_pToolbar;
};

#endif //__GDAPICALLSHISTORYPANEL_H

