//=====================================================================
// Copyright (c) 2012 Advanced Micro Devices, Inc. All rights reserved.
//
/// \author GPU Developer Tools
/// \file $File: //devtools/main/CodeXL/Components/GpuProfiling/AMDTGpuProfiling/CustomDataTypes.cpp $
/// \version $Revision: #4 $
/// \brief  This file contains custom data type
//
//=====================================================================
// $Id: //devtools/main/CodeXL/Components/GpuProfiling/AMDTGpuProfiling/CustomDataTypes.cpp#4 $
// Last checkin:   $DateTime: 2015/08/31 01:56:31 $
// Last edited by: $Author: salgrana $
// Change list:    $Change: 538740 $
//=====================================================================

#include "CustomDataTypes.h"

TableView::TableView(QWidget* parent) : acVirtualListCtrl(parent, NULL, false)
{
}

void TableView::mousePressEvent(QMouseEvent* event)
{
    QModelIndex modelIndex = indexAt(event->pos());

    if (event->button() == Qt::LeftButton)
    {
        mouseLeftClicked(modelIndex);
    }
    else if (event->button() == Qt::RightButton)
    {
        mouseRightClicked(modelIndex);
    }

    acVirtualListCtrl::mousePressEvent(event);
}

