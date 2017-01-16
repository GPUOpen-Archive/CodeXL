//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file tpOverview.h
///
//==================================================================================

//------------------------------ tpOverview.h ------------------------------

#ifndef __TPOVERVIEW_H
#define __TPOVERVIEW_H

// Qt:
#include <qtIgnoreCompilerWarnings.h>
#include <QtWidgets>

// Infra:
#include <AMDTApplicationComponents/Include/acSplitter.h>
#include <AMDTApplicationComponents/Include/acListCtrl.h>

// Backend:
#include <AMDTThreadProfileApi/inc/AMDTThreadProfileDataTypes.h>

// Local:
#include <inc/tpSessionData.h>

class acQHTMLWindow;

class tpOverview : public acSplitter
{
    Q_OBJECT

public:
    /// constructor
    tpOverview(QWidget* pParent, tpSessionData* pSessionData, tpSessionTreeNodeData* pSessionTreeData);

    virtual ~tpOverview();

private:

    /// Fill the session HTML properties window with data:
    /// \param pSessionTreeData is a pointer to the session data
    void FillHTMLWindow(tpSessionTreeNodeData* pSessionTreeData);


private:

    /// Table with the 5 biggest thread execution time:
    acListCtrl* m_pThreadsExecutionTimeTable;

    /// Session data:
    tpSessionData* m_pSessionData;

    /// The overview HTML window:
    acQHTMLWindow* m_pHTMLView;

};

#endif // __TPOVERVIEW_H
