//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file tpThreadsViewLegend.h
///
//==================================================================================

//------------------------------ tpThreadsViewLegend.h ------------------------------

#ifndef __TPTHREADSVIEWLEGEND_H
#define __TPTHREADSVIEWLEGEND_H

// Qt:
#include <QtWidgets>

// Infra:
#include <AMDTApplicationComponents/Include/acTreeCtrl.h>

// Backend:
#include <AMDTThreadProfileAPI/inc/AMDTThreadProfileDataTypes.h>

class tpSessionData;

/// This class is used for displaying a legend with the colors of the displayed timeline
class tpThreadsViewLegend : public acTreeCtrl
{
    Q_OBJECT

public:
    tpThreadsViewLegend(QWidget* pParent, tpSessionData* pSessionData);
    virtual ~tpThreadsViewLegend();

protected:

    /// Contain the current session extracted data:
    tpSessionData* m_pSessionData;

};

#endif // __TPTHREADSVIEWLEGEND_H
