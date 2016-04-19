//=====================================================================
// Copyright (c) 2012 Advanced Micro Devices, Inc. All rights reserved.
//
/// \author GPU Developer Tools
/// \file $File: //devtools/main/CodeXL/Components/GpuProfiling/AMDTGpuProfiling/Project.cpp $
/// \version $Revision: #3 $
/// \brief :  This file contains Project
//
//=====================================================================
// $Id: //devtools/main/CodeXL/Components/GpuProfiling/AMDTGpuProfiling/Project.cpp#3 $
// Last checkin:   $DateTime: 2015/11/18 08:31:35 $
// Last edited by: $Author: gyarnitz $
// Change list:    $Change: 548908 $
//=====================================================================

#include "Project.h"


gpProject::gpProject()
{
}

gpProject::gpProject(QString& name, QString& path)
{
    m_name = name;
    m_path = path;
}

gpProject::~gpProject()
{

}


