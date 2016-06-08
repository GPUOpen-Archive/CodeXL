//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file acImageManagerModel.h
///
//==================================================================================

//------------------------------ acImageManagerModel.h ------------------------------

#ifndef __ACIMAGESMODEL_H
#define __ACIMAGESMODEL_H

// Qt:
#include <QAbstractTableModel>

// Infra:
#include <AMDTBaseTools/Include/gtVector.h>
#include <AMDTOSWrappers/Include/osFilePath.h>

// Local:
#include <AMDTApplicationComponents/Include/acApplicationComponentsDLLBuild.h>

class acImageManager;
// ----------------------------------------------------------------------------------
// Class Name:          acImageManagerModel : public QAbstractTableModel
// General Description: This class is used as data model for the image manager
// Author:              Sigal Algranaty
// Creation Date:       10/6/2012
// ----------------------------------------------------------------------------------
class AC_API acImageManagerModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    // Constructor:
    acImageManagerModel(acImageManager* pManager);

    // Destructor:
    ~acImageManagerModel();

    // When inheriting the model, implement these member functions, to provide the model functionality:
    virtual int rowCount(const QModelIndex& parent = QModelIndex()) const;
    virtual int columnCount(const QModelIndex& parent = QModelIndex())const;

    virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole)const;

    virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
    Qt::ItemFlags flags(const QModelIndex& index) const;
    gtVector<osFilePath> _filePaths;
    QPixmap** _pixmaps;

    void updateModel();

protected:

    acImageManager* m_pImageManager;

};
#endif //__ACIMAGESMODEL_H

