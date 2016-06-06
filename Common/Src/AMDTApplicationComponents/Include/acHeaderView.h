//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file acHeaderView.h
///
//==================================================================================

//------------------------------ acHeaderView.h ------------------------------

#ifndef __ACHEADERVIEW
#define __ACHEADERVIEW

// Qt:
#include <QtWidgets>

// Local:
#include <AMDTApplicationComponents/Include/acApplicationComponentsDLLBuild.h>


class acListCtrl;
// ----------------------------------------------------------------------------------
// Class Name:          acHeaderView : public QHeaderView
// General Description: We define our own header item, in order to catch the mouse
//                      events of the table. Signals are not working good enough,
//                      since it filters out the handle click and release events
// Author:              Sigal Algranaty
// Creation Date:       3/1/2012
// ----------------------------------------------------------------------------------
class AC_API acHeaderView : public QHeaderView
{
    Q_OBJECT
public:

    acHeaderView(Qt::Orientation orientation, acListCtrl* pParentList);
    ~acHeaderView();

    void emitSectionCountChanged(int rowAmount, int neededRowAmount);


protected:

    virtual void mousePressEvent(QMouseEvent* pEvent);
    virtual void mouseReleaseEvent(QMouseEvent* pEvent);

protected:

    acListCtrl* _pParentList;

};

#endif  // __ACHEADERVIEW
