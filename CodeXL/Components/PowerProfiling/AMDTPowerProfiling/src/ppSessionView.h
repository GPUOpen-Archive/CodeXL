//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file ppSessionView.h
///
//==================================================================================

//------------------------------ ppSessionView.h ------------------------------

#ifndef __PPSESSIONVIEW_H
#define __PPSESSIONVIEW_H

// Qt:
#include <qtIgnoreCompilerWarnings.h>
#include <QtWidgets>

// Infra:
#include <AMDTOSWrappers/Include/osFilePath.h>
#include <AMDTApplicationComponents/Include/acTabWidget.h>

// AMDTApplicationFramework:
#include <AMDTApplicationFramework/Include/views/afBaseView.h>

// AMDTSharedProfiling:
#include <AMDTSharedProfiling/inc/SharedSessionWindow.h>

// Local:
#include <AMDTPowerProfiling/Include/ppAMDTPowerProfilingDLLBuild.h>
#include <AMDTPowerProfiling/src/ppSessionController.h>

class ppSummaryView;
class ppTimeLineView;


enum ppSessionViewsTabIndex
{
    PP_TAB_TIMELINE_INDEX = 0,
    PP_TAB_SUMMARY_INDEX
};


class PP_API ppSessionView : public SharedSessionWindow
{
    Q_OBJECT

public:
    ppSessionView(QWidget* pParent, ppSessionController::SessionState state);
    virtual ~ppSessionView();

    /// Display a specific tab in the view
    void DisplayTab(int tabIndex);

    /// Set the view data that will enable displaying the tab views:
    /// \param sessionPath the session path
    void SetViewData(const osFilePath& sessionPath);

    /// Starts the database connection:
    void StartDBConnection();

    /// Stops the database connection:
    void StopDBConnection();

    /// Set the session path. Is called when a session is renamed:
    /// \param sessionPath the session path (the offpp file)
    void SetSessionFilePath(const osFilePath& sessionPath);

    /// Activate the current session (start listening to profile events):
    void ActivateSession();

    /// Get a reference to the session controller:
    ppSessionController& SessionController() { return m_sessionController; };

    /// handle closing of view from VS
    void OnCloseMdiWidget();

public slots:

    /// Is handling profile stopped signal:
    void OnProfileStopped(const QString& sessionName);

private:
    // Get Session info based on the sessionPath:
    void UpdateSessionInfo(const osFilePath& sessionPath, gtString& sessionName, gtString& dbFullPath);

    /// Main view layout:
    QLayout* m_pMainLayout;

    /// Holds all the internal views:
    acTabWidget* m_pTabWidget;

    /// Holds the timeline view:
    ppTimeLineView* m_pTimelineView;

    /// Holds the summary view:
    ppSummaryView* m_pSummaryView;

    /// The object used for session data control:
    ppSessionController m_sessionController;

};

#endif // __PPSESSIONVIEW_H
