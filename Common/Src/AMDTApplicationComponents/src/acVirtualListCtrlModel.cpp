//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file acVirtualListCtrlModel.cpp
///
//==================================================================================

//------------------------------ acVirtualListCtrlModel.cpp ------------------------------

// Qt:
#include <QtWidgets>

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>

// Local:
#include <AMDTApplicationComponents/Include/acVirtualListCtrlModel.h>

// ---------------------------------------------------------------------------
// Name:        acVirtualListCtrlModel::acVirtualListCtrlModel
// Description: Constructor
// Arguments:   QWidget* pParent
// Author:      Sigal Algranaty
// Date:        25/12/2011
// ---------------------------------------------------------------------------
acVirtualListCtrlModel::acVirtualListCtrlModel(QWidget* pParent)
    : QAbstractTableModel(pParent)
{
}

// ---------------------------------------------------------------------------
// Name:        acVirtualListCtrlModel::~acVirtualListCtrlModel
// Description: Destructor
// Author:      Sigal Algranaty
// Date:        25/12/2011
// ---------------------------------------------------------------------------
acVirtualListCtrlModel::~acVirtualListCtrlModel()
{

}

// ---------------------------------------------------------------------------
// Name:        acVirtualListCtrlModel::rowCount
// Description: Return the table column count - must be implementd by child
// Arguments:   const QModelIndex &parent
// Return Val:  int
// Author:      Sigal Algranaty
// Date:        25/12/2011
// ---------------------------------------------------------------------------
int acVirtualListCtrlModel::rowCount(const QModelIndex& parent) const
{
    (void)(parent); // unused
    GT_ASSERT_EX(false, L"Function must be implemented by child class");
    return 0;
}

// ---------------------------------------------------------------------------
// Name:        acVirtualListCtrlModel::columnCount
// Description: Return the table row count
// Arguments:   const QModelIndex &parent
// Return Val:  int
// Author:      Sigal Algranaty
// Date:        25/12/2011
// ---------------------------------------------------------------------------
int acVirtualListCtrlModel::columnCount(const QModelIndex& parent) const
{
    (void)(parent); // unused
    GT_ASSERT_EX(false, L"Function must be implemented by child class");
    return 0;
}

// ---------------------------------------------------------------------------
// Name:        acVirtualListCtrlModel::data
// Description: Return the data for the requested column and row
// Arguments:   const QModelIndex &index
//              int role
// Return Val:  QVariant
// Author:      Sigal Algranaty
// Date:        25/12/2011
// ---------------------------------------------------------------------------
QVariant acVirtualListCtrlModel::data(const QModelIndex& index, int role) const
{
    QVariant retVal;

    if ((index.isValid()) && (role == Qt::DisplayRole))
    {
        GT_ASSERT_EX(false, L"Function must be implemented by child class");
    }

    if ((index.isValid()) && (role == Qt::DecorationRole))
    {
        GT_ASSERT_EX(false, L"Function must be implemented by child class");
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        acVirtualListCtrlModel::headerData
// Description: Return the header data
// Arguments:   int section
//              Qt::Orientation orientation
//              int role
// Return Val:  QVariant
// Author:      Sigal Algranaty
// Date:        25/12/2011
// ---------------------------------------------------------------------------
QVariant acVirtualListCtrlModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    (void)(section); // unused
    (void)(orientation); // unused
    QVariant retVal;

    if (role == Qt::DisplayRole)
    {
        GT_ASSERT_EX(false, L"Function must be implemented by child class");
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        appSingleLogDataModel::flags
// Description: Return the flags for the requested item
// Arguments:   const QModelIndex &index
// Return Val:  Qt::ItemFlags
// Author:      Sigal Algranaty
// Date:        25/12/2011
// ---------------------------------------------------------------------------
Qt::ItemFlags acVirtualListCtrlModel::flags(const QModelIndex& index) const
{
    Qt::ItemFlags retVal = Qt::ItemIsEnabled;

    if (index.isValid())
    {
        retVal = QAbstractTableModel::flags(index) | Qt::ItemIsEditable;
    }

    return retVal;
}
