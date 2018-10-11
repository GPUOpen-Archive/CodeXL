//=====================================================================
// Copyright (c) 2012 Advanced Micro Devices, Inc. All rights reserved.
//
/// \author GPU Developer Tools
/// \file $File: //devtools/main/CodeXL/Components/GpuProfiling/AMDTGpuProfiling/SummaryView.h $
/// \version $Revision: #15 $
/// \brief  This file contains SummaryView class
//
//=====================================================================
// $Id: //devtools/main/CodeXL/Components/GpuProfiling/AMDTGpuProfiling/SummaryView.h#15 $
// Last checkin:   $DateTime: 2015/11/24 07:55:31 $
// Last edited by: $Author: salgrana $
// Change list:    $Change: 549710 $
//=====================================================================
#ifndef _SUMMARY_VIEW_H_
#define _SUMMARY_VIEW_H_

#include <qtIgnoreCompilerWarnings.h>

#include "ui_SummaryViewBase.h"

// Qt:
#include <QtCore>

#include "CLSummarizer.h"
#include "CXLAnalyzerHTMLUtils.h"
#include "Session.h"

// need to undef Bool after all includes so the moc will compile in Linux
#undef Bool

/// The Summary view class
class SummaryView : public QWidget, private Ui::SummaryViewBase
{
    Q_OBJECT

public:

    /// Initializes a new instance of the SummaryView class
    SummaryView(QWidget* parent);

    /// Destroys an instance of the SummaryView class
    virtual ~SummaryView();

    /// Loads a session into the Summary View
    /// \param pSession the session to load
    /// \param pSummarizer the summarizer instance to use
    /// \return true if the session was loaded, false otherwise
    bool LoadSession(TraceSession* pSession, CLSummarizer* pSummarizer);

    /// Resets the Summary view (clears the  combo box)
    void Reset();

    /// Display the requested summary type
    /// \param summaryType - the requested summary type
    void DisplaySummaryPageType(int selectedIndex);

    // get index in combo by page name
    // \param page name
    // \returns the index. if didnt find returns -1
    int GetComboIndexByPageName(const QString& name);

public slots:
    /// Edit actions:
    void OnEditCopy();
    void OnEditSelectAll();

    // TODO: Temporarily disabled due to deprecation of related APIs
    /// Export to CSV context menu slot:
    // void OnExportToCSV();

private:
    /// OpenCL Trace Summarizer
    CLSummarizer* m_pSummarizer;

    /// ComboBox index to the newest page in history
    int m_newestPageIndex;

    /// ComboBox index to the current page in history
    int m_currentPageIndex;

    /// Initialize the data and state
    /// \param pSession the current session
    /// \return True if success in initialization
    bool Initialize(GPUSessionTreeItemData* pSession);

    /// If there is warning in page
    /// \return True if there is warning in page
    bool GetHasErrorWarningPage();

signals:
    /// Signal emitted when a clicked link should be shown in the timeline or trace view
    /// \param strPagename the title of the summary page containing the link clicked on (the title shown in the combo box)
    /// \param threadID the thread id of the call to show in the timeline or trace view
    /// \param callIndex the index of the call to show in the timeline or trace view
    /// \param viewType flag indicating whether the link should be shown in the timeline or the trace view
    void LinkClicked(const QString& strPageName, unsigned int threadID, unsigned int callIndex, AnalyzerHTMLViewType viewType);

protected slots:
    /// Selected summary page changed
    void SelectedPageChanged();

    /// Handler for when a link is clicked
    /// \param url the URL of the link clicked
    void LinkClickedHandler(const QUrl& url);

    /// Context menu:
    void OnContextMenu(const QPoint&);

private:

    /// The currently displayed session:
    TraceSession* m_pDisplayedTraceSession;

    bool m_loading;

    /// Context menu:
    QMenu* m_pContextMenu;
    QAction* m_pCopyAction;
    QAction* m_pSelectAllAction;
    QAction* m_pExportToCSVAction;
};

#endif // _SUMMARY_VIEW_H_

