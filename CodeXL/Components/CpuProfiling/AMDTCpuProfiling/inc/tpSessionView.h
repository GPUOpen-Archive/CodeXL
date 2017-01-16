//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file tpSessionView.h
///
//==================================================================================

//------------------------------ tpSessionView.h ------------------------------

#ifndef __TPSESSIONVIEW_H
#define __TPSESSIONVIEW_H

// Qt:
#include <qtIgnoreCompilerWarnings.h>
#include <QtWidgets>

// Infra:
#include <AMDTApplicationComponents/Include/acTabWidget.h>
#include <AMDTOSWrappers/Include/osFilePath.h>

// AMDTApplicationFramework:
#include <AMDTApplicationFramework/Include/views/afBaseView.h>

// Backend:
#include <AMDTThreadProfileApi/inc/AMDTThreadProfileDataTypes.h>

// Local
#include <inc/tpSessionData.h>

class tpThreadsView;
class tpOverview;

class tpSessionView : public QWidget, public afBaseView
{
    Q_OBJECT

public:
    tpSessionView(QWidget* pParent, const osFilePath& filePath);
    virtual ~tpSessionView();

    /// Display one of the tabs:
    /// \param tabIndex the tab index
    void DisplayTab(int tabIndex);

protected slots:

    /// Is handling the current changed signal of the tab widget:
    void OnCurrentChanged(int index);

private:

    /// Main view layout:
    QLayout* m_pMainLayout;

    /// Holds all the internal views:
    acTabWidget* m_pTabWidget;

    /// Holds the overview:
    tpOverview* m_pOverview;

    /// Holds the thread view:
    tpThreadsView* m_pThreadsTimelineWidget;

    /// session data
    tpSessionData* m_pSessionData;
};

#endif // __TPSESSIONVIEW_H
