//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file acDataViewGridTable.h
///
//==================================================================================

//------------------------------ acDataViewGridTable.h ------------------------------

#ifndef __ACDATAVIEWGRIDTABLE
#define __ACDATAVIEWGRIDTABLE

// Qt:
#include <QtWidgets>

// Local:
#include <AMDTApplicationComponents/Include/acDataView.h>
#include <AMDTApplicationComponents/Include/acApplicationComponentsDLLBuild.h>


// ----------------------------------------------------------------------------------
// Class Name:           acDataViewGridTable : public QAbstractTableModel
// General Description:  This class gives access to the "virtual" table which is
//                       actually the raw data.
// Author:               Eran Zinman
// Creation Date:        29/7/2007
// ----------------------------------------------------------------------------------
class AC_API acDataViewGridTable : public QAbstractTableModel
{
    Q_OBJECT

public:

    // Default CTOR (required by Qt 5.2):
    acDataViewGridTable() {}

    // Constructor:
    acDataViewGridTable(acDataViewItem* pDataViewItem);

    // Destructor:
    ~acDataViewGridTable();

public:

    // When inheriting the model, implement these member functions, to provide the model functionality:
    virtual int rowCount(const QModelIndex& parent = QModelIndex()) const;
    virtual int columnCount(const QModelIndex& parent = QModelIndex())const;

    virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole)const;
    virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;

    Qt::ItemFlags flags(const QModelIndex& index) const;

    void setCellBestFitSize(const QSize& size) {m_currentBestFitSize = size;};
protected:

    gtString getColumnText(int col) const;
    gtString getRowText(int row) const;

private:
    // Holds the information about the raw data associated with this table
    acDataViewItem* m_pDataViewItem;

    // Contain the current best fit cell size:
    QSize m_currentBestFitSize;
};



#endif  // __ACDATAVIEWGRIDTABLE
