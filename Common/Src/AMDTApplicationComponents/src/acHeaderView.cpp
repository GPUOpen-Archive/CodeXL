//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file acHeaderView.cpp
///
//==================================================================================

//------------------------------ acHeaderView.cpp ------------------------------

// Qt:
#include <QtWidgets>

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>

// Local:
#include <AMDTApplicationComponents/Include/acHeaderView.h>
#include <AMDTApplicationComponents/Include/acListCtrl.h>

// ---------------------------------------------------------------------------
// Name:        acHeaderView::acHeaderView
// Description: Constructor
// Arguments:   Qt::Orientation orientation
//              acListCtrl* pParentList
// Author:      Sigal Algranaty
// Date:        3/1/2012
// ---------------------------------------------------------------------------
acHeaderView::acHeaderView(Qt::Orientation orientation, acListCtrl* pParentList) :
    QHeaderView(orientation, pParentList), _pParentList(pParentList)
{
}

// ---------------------------------------------------------------------------
// Name:        acHeaderView::~acHeaderView
// Description: Destructor
// Author:      Sigal Algranaty
// Date:        3/1/2012
// ---------------------------------------------------------------------------
acHeaderView::~acHeaderView()
{

}

// ---------------------------------------------------------------------------
// Name:        acHeaderView::mousePressEvent
// Description: Overriding the mouse press event - tell the list to stop resizing
//              columns
// Arguments:   QMouseEvent* pEvent
// Author:      Sigal Algranaty
// Date:        3/1/2012
// ---------------------------------------------------------------------------
void acHeaderView::mousePressEvent(QMouseEvent* pEvent)
{
    // Sanity check
    if (_pParentList != NULL)
    {
        _pParentList->setIgnoreResize(true);
    }

    // Call the base class implementation:
    QHeaderView::mousePressEvent(pEvent);
}

// ---------------------------------------------------------------------------
// Name:        acHeaderView::mousePressEvent
// Description: Overriding the mouse release event - tell the list to continue resizing
//              columns
// Arguments:   QMouseEvent* pEvent
// Author:      Sigal Algranaty
// Date:        3/1/2012
// ---------------------------------------------------------------------------
void acHeaderView::mouseReleaseEvent(QMouseEvent* pEvent)
{
    // Sanity check
    if (_pParentList != NULL)
    {
        _pParentList->setIgnoreResize(false);
    }

    // Call the base class implementation:
    QHeaderView::mouseReleaseEvent(pEvent);
}

void acHeaderView::emitSectionCountChanged(int rowAmount, int neededRowAmount)
{
    emit sectionCountChanged(rowAmount, neededRowAmount);
}
