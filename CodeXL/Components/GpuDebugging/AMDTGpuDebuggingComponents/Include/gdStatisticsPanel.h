//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gdStatisticsPanel.h
///
//==================================================================================

//------------------------------ gdStatisticsPanel.h ------------------------------

#ifndef __GDSTATISTICSPANEL_H
#define __GDSTATISTICSPANEL_H

#include <AMDTBaseTools/Include/gtIgnoreCompilerWarnings.h>

// Qt:
#include <QtWidgets>
#include <QFrame>
#include <QSplitter>

// Forward declaration:
QT_BEGIN_NAMESPACE
class QHBoxLayout;
class QMainWindow;
class QVBoxLayout;
QT_END_NAMESPACE

// Forward declaration:
class acToolBar;
class gdStatisticsView;
class apDebuggedProcessRunSuspendedEvent;
class afPropertiesView;
class acChartWindow;
class acWxWidgetWrapper;
class afProgressBarWrapper;

// AMDTApplicationFramework:
#include <AMDTApplicationFramework/Include/afAppStringConstants.h>

// Infra:
#include <AMDTAPIClasses/Include/Events/apIEventsObserver.h>

// AMDTApplicationFramework:
#include <AMDTApplicationFramework/Include/views/afBaseView.h>

// Local:
#include <AMDTGpuDebuggingComponents/Include/gdGDebuggerAppDLLBuild.h>
#include <AMDTGpuDebuggingComponents/Include/views/gdStatisticsViewBase.h>

// ----------------------------------------------------------------------------------
// Class Name:          gdStatisticsPanel : public wxPanel, public afBaseView
// General Description: Used to encapsulate gdStatisticsView with a toolbar
// Author:              Sigal Algranaty
// Creation Date:       10/10/2011
// ----------------------------------------------------------------------------------
class GD_API gdStatisticsPanel : public QSplitter, public apIEventsObserver, public afBaseView
{
    Q_OBJECT

public:

    gdStatisticsPanel(afProgressBarWrapper* pProgressBar, QWidget* pParent, QMainWindow* pMainWindow);
    ~gdStatisticsPanel();

    // Overrides apIEventsObserver:
    virtual const wchar_t* eventObserverName() const { return L"StatisticsPanel"; };
    virtual void onEvent(const apEvent& eve, bool& vetoEvent);

    gdStatisticsView* statisticsView() { return m_pStatisticsView; }

    // Update toolbar commands:
    virtual void updateToolbarCommands();

    bool isActionEnabled(int commandId);
    bool triggerCommand(QWidget* pCurrentFocusedWidget, int commandId);

    virtual void onUpdateEdit_Copy(bool& isEnabled);
    virtual void onUpdateEdit_SelectAll(bool& isEnabled);
    virtual void onUpdateEdit_Find(bool& isEnabled);
    virtual void onUpdateEdit_FindNext(bool& isEnabled);

    virtual void onEdit_Copy();
    virtual void onEdit_SelectAll();
    virtual void onEdit_Find();
    virtual void onEdit_FindNext();

    /// \param[out] selectedText returns the text currently selected
    virtual void GetSelectedText(gtString& selectedText);

    void setFrameLayout(afProgressBarWrapper* pProgressBar, QMainWindow* pMainWindow);
    void createToolbar();
    void createBottomLayout(QMainWindow* pMainWindow);
    void displayPropertiesWindowMessage(const gtString& title = AF_STR_Empty, const gtString& information = AF_STR_Empty);
    void updateChartAndPropertiesViews(gdStatisticsViewIndex windowIndex);
    bool updateStatisticsViewers();
    void onViewShown(bool isViewShown);
    void clearAllViewers();

    // Event handling:
    virtual void onProcessRunSuspended(const apDebuggedProcessRunSuspendedEvent& event);
    void onProcessStarted();
    void onProcessTerminated();

    void onUpdateClearButton(bool& isEnabled);

public slots:

    void onClearButton();
    void onDetailedBatchDataButton();

    virtual void onFindClick();
    virtual void onFindNext();

protected:

    // The nested statistics view:
    gdStatisticsView* m_pStatisticsView;

    // Properties window:
    afPropertiesView* m_pPropertiesView;

    // Statistics Chart:
    acChartWindow* m_pChartWindow;

    // Toolbar:
    acToolBar* m_pToolbar;
    QAction* m_pClearAction;
    QAction* m_pBatchAction;

    // Layout for the whole frame (toolbar + content):
    QVBoxLayout* m_pTopWidgetLayout;

    // The layout for the bottom of the view:
    QHBoxLayout* m_pBottomLayout;

    // Widget containing the control in the bottom and top of the view:
    QWidget* m_pBottomWidget;
    QWidget* m_pTopWidget;

    // Is the debugged process suspended?
    bool m_isDebuggedProcessSuspended;

    // Is the information in the viewer up-to-date?
    bool m_isInfoUpdated;

};


#endif //__GDSTATISTICSPANEL_H

