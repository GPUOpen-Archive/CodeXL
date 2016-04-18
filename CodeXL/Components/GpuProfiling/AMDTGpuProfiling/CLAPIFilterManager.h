//=====================================================================
// Copyright (c) 2012 Advanced Micro Devices, Inc. All rights reserved.
//
/// \author GPU Developer Tools
/// \file $File: //devtools/main/CodeXL/Components/GpuProfiling/AMDTGpuProfiling/CLAPIFilterManager.h $
/// \version $Revision: #7 $
/// \brief :  This file contains CLAPIFilterManager
//
//=====================================================================
// $Id: //devtools/main/CodeXL/Components/GpuProfiling/AMDTGpuProfiling/CLAPIFilterManager.h#7 $
// Last checkin:   $DateTime: 2015/08/31 01:56:31 $
// Last edited by: $Author: salgrana $
// Change list:    $Change: 538740 $
//=====================================================================
#ifndef _CLAPI_FILTER_MANAGER_H_
#define _CLAPI_FILTER_MANAGER_H_


#include <qtIgnoreCompilerWarnings.h>

#include <QtCore/qstring.h>
#include <QtCore/qmap.h>

#include "CLAPIDefs.h"
#include "ProjectSettings.h"

/// Class to manage API Filter settings
class CLAPIFilterManager
{
public:
    /// Initializes a new instance of the CLAPIFilterManager class.
    /// \param filename API Filter file
    CLAPIFilterManager(const QString& filename);

    /// Save the list into API filter file
    /// \param filterCLAPIslist will contain filtered API list
    void Save(const QStringList& filterAPIslist, APIToTrace apiTrace);

    /// Gets a value indicating whether or not API filter is enabled
    /// \return True id API Filter is enabled
    bool GetEnabled() const;

    /// Gets a value indicating whether or not API filter is enabled
    /// \return True id API Filter is enabled
    bool IsEnabled(const QString& apiName) const { return m_apiFilterList.contains(apiName); };

    /// Gets the filename of the API filter
    /// \return API filter filename
    QString GetAPIFilterFile() const;

    /// collection of API Filters
    const QStringList& APIFilterSet() const { return m_apiFilterList; };
    void SetAPIFilterSet(const QStringList& filterSet) { m_apiFilterList = filterSet; };

private:
    /// collection of API Filters
    QStringList m_apiFilterList;

    /// API filter filename
    QString m_aPIFilterFile;

};

#endif // _CLAPI_FILTER_MANAGER_H_


