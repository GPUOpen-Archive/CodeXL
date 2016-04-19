//=====================================================================
// Copyright (c) 2012 Advanced Micro Devices, Inc. All rights reserved.
//
/// \author GPU Developer Tools
/// \file $File: //devtools/main/CodeXL/Components/GpuProfiling/AMDTGpuProfiling/FindToolBarView.h $
/// \version $Revision: #3 $
/// \brief :  This file contains KernelOccupancyWindow class
//
//=====================================================================
// $Id: //devtools/main/CodeXL/Components/GpuProfiling/AMDTGpuProfiling/FindToolBarView.h#3 $
// Last checkin:   $DateTime: 2015/08/31 01:56:31 $
// Last edited by: $Author: salgrana $
// Change list:    $Change: 538740 $
//=====================================================================
#ifndef _FIND_TOOL_BAR_VIEW_H_
#define _FIND_TOOL_BAR_VIEW_H_

#include <qtIgnoreCompilerWarnings.h>
#include <AMDTBaseTools/Include/gtIgnoreCompilerWarnings.h>

#include "ui_FindToolBar.h"


/// UI for Find toolbar (currently unused in CodeXL)
class FindToolBarView : public QWidget, private Ui::FindToolBar
{
public:
    /// Initializes a new instance of the FindToolBarView class.
    FindToolBarView(QWidget* parent = 0);

    /// Destructor
    ~FindToolBarView();
};

#endif // _FIND_TOOL_BAR_VIEW_H_

