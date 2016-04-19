//=====================================================================
// Copyright (c) 2012 Advanced Micro Devices, Inc. All rights reserved.
//
/// \author GPU Developer Tools
/// \file $File: //devtools/main/CodeXL/Components/GpuProfiling/AMDTGpuProfiling/Project.h $
/// \version $Revision: #5 $
/// \brief :  This file contains Project class
//
//=====================================================================
// $Id: //devtools/main/CodeXL/Components/GpuProfiling/AMDTGpuProfiling/Project.h#5 $
// Last checkin:   $DateTime: 2015/11/18 08:31:35 $
// Last edited by: $Author: gyarnitz $
// Change list:    $Change: 548908 $
//=====================================================================
#ifndef _PROJECT_H_
#define _PROJECT_H_

#include <qtIgnoreCompilerWarnings.h>
#include <AMDTBaseTools/Include/gtIgnoreCompilerWarnings.h>
#include <QtCore/qstring.h>

/// Project wrapper object -- this should probably be removed
class gpProject
{
public:
    /// Default ctor
    gpProject();

    /// Ctor that initializes name and path
    /// \param name Name of the project
    /// \param path Path of the project directory
    gpProject(QString& name, QString& path);

    /// Destructor
    ~gpProject();

    /// Project name
    QString m_name;

    /// Project path
    QString m_path;
};

#endif // _PROJECT_H_

