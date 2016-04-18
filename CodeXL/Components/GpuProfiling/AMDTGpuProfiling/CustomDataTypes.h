//=====================================================================
// Copyright (c) 2012 Advanced Micro Devices, Inc. All rights reserved.
//
/// \author GPU Developer Tools
/// \file $File: //devtools/main/CodeXL/Components/GpuProfiling/AMDTGpuProfiling/CustomDataTypes.h $
/// \version $Revision: #5 $
/// \brief  This file contains custom data type
//
//=====================================================================
// $Id: //devtools/main/CodeXL/Components/GpuProfiling/AMDTGpuProfiling/CustomDataTypes.h#5 $
// Last checkin:   $DateTime: 2015/08/31 01:56:31 $
// Last edited by: $Author: salgrana $
// Change list:    $Change: 538740 $
//=====================================================================
#ifndef _CUSTOMDATATYPES_H_
#define _CUSTOMDATATYPES_H_

#include <AMDTBaseTools/Include/gtIgnoreCompilerWarnings.h>

#include <QtWidgets/qtableview.h>
#include <QtWidgets/QWidget>

// Infra:
#include <AMDTApplicationComponents/Include/acVirtualListCtrl.h>

#include <AMDTGpuProfiling/AMDTGpuProfilerDefs.h>


/// QTableView descendant that provides customized mouse events
class TableView : public acVirtualListCtrl
{
    Q_OBJECT

public:
    /// Initializes a new instance of the TableView class.
    TableView(QWidget* parent);

protected:
    /// Gets called on mouse press.
    /// \param event Mouse press event object
    void mousePressEvent(QMouseEvent* event);

signals:
    /// Called on mouse left click
    void mouseLeftClicked(QModelIndex);

    /// Called on mouse right click
    void mouseRightClicked(QModelIndex);
};


#endif //_CUSTOM_DATA_TYPES_H_
