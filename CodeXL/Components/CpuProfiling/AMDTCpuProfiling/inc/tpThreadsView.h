//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file tpThreadsView.h
///
//==================================================================================

//------------------------------ tpThreadsView.h ------------------------------

#ifndef __TPTHREADSVIEW_H
#define __TPTHREADSVIEW_H

// Qt:
#include <qtIgnoreCompilerWarnings.h>
#include <QtWidgets>

// AMDTApplicationFramework:
#include <AMDTApplicationFramework/Include/views/afBaseView.h>

// Backend:
#include <AMDTThreadProfileDataTypes.h>

// Local
#include <inc/tpSessionData.h>


class acNavigationChart;
class tpThreadsViewControlPanel;
class tpThreadsTimeline;
class acQHTMLWindow;
class tpThreadsViewLegend;
class tpSessionTreeNodeData;

class tpThreadsView : public QWidget
{
    Q_OBJECT

public:
    tpThreadsView(QWidget* pParent, tpSessionData* pSessionData, tpSessionTreeNodeData* pSessionTreeData);
    virtual ~tpThreadsView();

protected:

    /// Fill the navigation chart with session data:
    void FillNavigationChart();

    /// Resize event (overrides QWidget):
    /// \param event
    virtual void resizeEvent(QResizeEvent* event);

private:

    acNavigationChart* m_pNavigationChart;
    tpThreadsTimeline* m_pTimeline;
    tpThreadsViewLegend* m_pLegend;
    tpThreadsViewControlPanel* m_pControlPanel;

    /// session data
    tpSessionData* m_pSessionData;

};

#endif // __TPTHREADSVIEW_H
