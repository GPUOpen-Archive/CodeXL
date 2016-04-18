//=====================================================================
// Copyright (c) 2012 Advanced Micro Devices, Inc. All rights reserved.
//
/// \author GPU Developer Tools
/// \file $File: //devtools/main/CodeXL/Components/GpuProfiling/AMDTGpuProfiling/FindToolBarView.cpp $
/// \version $Revision: #2 $
/// \brief :  This file contains FindToolBarView
//
//=====================================================================
// $Id: //devtools/main/CodeXL/Components/GpuProfiling/AMDTGpuProfiling/FindToolBarView.cpp#2 $
// Last checkin:   $DateTime: 2015/08/31 01:56:31 $
// Last edited by: $Author: salgrana $
// Change list:    $Change: 538740 $
//=====================================================================

#include "FindToolBarView.h"


FindToolBarView::FindToolBarView(QWidget* parent)
{
    setupUi(this);
    setParent(parent);

    setFixedHeight(46);
}


FindToolBarView::~FindToolBarView()
{
}




