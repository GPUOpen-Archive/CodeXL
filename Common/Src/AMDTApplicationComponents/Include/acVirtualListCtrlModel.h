//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file acVirtualListCtrlModel.h
///
//==================================================================================

//------------------------------ acVirtualListCtrlModel.h ------------------------------

#ifndef __ACVIRTUALLISTCTRLMODEL_H
#define __ACVIRTUALLISTCTRLMODEL_H

// Qt:
#include <QtWidgets>

// Local:
#include <AMDTBaseTools/Include/gtString.h>
#include <AMDTApplicationComponents/Include/acApplicationComponentsDLLBuild.h>

Q_DECLARE_METATYPE(void*);

// ----------------------------------------------------------------------------------
// Class Name:          AC_API QAbstractTableModel : public QAbstractTableModel
// General Description: A class used for providing a data model for virtual list
// Author:              Sigal Algranaty
// Creation Date:       25/12/2011
// ----------------------------------------------------------------------------------
class AC_API acVirtualListCtrlModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    // Constructor:
    acVirtualListCtrlModel(QWidget* pParent);

    // Destructor:
    ~acVirtualListCtrlModel();


    // When inheriting the model, implement these member functions, to provide the model functinoality:
    virtual int rowCount(const QModelIndex& parent = QModelIndex()) const;
    virtual int columnCount(const QModelIndex& parent = QModelIndex())const;

    virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole)const;
    virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
    Qt::ItemFlags flags(const QModelIndex& index) const;

};

#endif //__ACVIRTUALLISTCTRLMODEL_H

