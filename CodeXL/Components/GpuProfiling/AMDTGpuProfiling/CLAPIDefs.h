//=====================================================================
// Copyright (c) 2012 Advanced Micro Devices, Inc. All rights reserved.
//
/// \author GPU Developer Tools
/// \file $File: //devtools/main/CodeXL/Components/GpuProfiling/AMDTGpuProfiling/CLAPIDefs.h $
/// \version $Revision: #8 $
/// \brief  This file contains definitions for CL API Functions
//
//=====================================================================
// $Id: //devtools/main/CodeXL/Components/GpuProfiling/AMDTGpuProfiling/CLAPIDefs.h#8 $
// Last checkin:   $DateTime: 2015/08/31 01:56:31 $
// Last edited by: $Author: salgrana $
// Change list:    $Change: 538740 $
//=====================================================================
#ifndef _CLAPI_DEFS_H_
#define _CLAPI_DEFS_H_

#include <qtIgnoreCompilerWarnings.h>
#include <AMDTBaseTools/Include/gtIgnoreCompilerWarnings.h>


#include <QString>
#include <QStringList>
#include <QMap>
#include <QList>

#include <TSingleton.h>

#include <CLFunctionEnumDefs.h>

/// Group IDs for OpenCL APIs
enum CLAPIGroup
{
    CLAPIGroup_Unknown = 0,
    CLAPIGroup_CLObjectCreate = 0x1,
    CLAPIGroup_CLObjectRetain = 0x2,
    CLAPIGroup_CLObjectRelease = 0x4,
    CLAPIGroup_QueryInfo = 0x8,
    CLAPIGroup_EnqueueDataTransfer = 0x10,
    CLAPIGroup_EnqueueKernel = 0x20,
    CLAPIGroup_EnqueueOther = 0x40,
    CLAPIGroup_OpenGLInterOp = 0x80,
    CLAPIGroup_DirectXInterOp = 0x100,
    CLAPIGroup_Synchronization = 0x200,
    CLAPIGroup_SetCallback = 0x400,
    CLAPIGroup_Extensions = 0x800,
    CLAPIGroup_Other = 0x1000,
};

/// a type representing a set of CLAPIGroup values
typedef unsigned int CLAPIGroups;

/// Singleton class with helpers for dealing with CL function types
class CLAPIDefs : public TSingleton<CLAPIDefs>
{
    friend class TSingleton<CLAPIDefs>;
public:
    /// Gets the OpenCL API function name for the specified CL_FUNC_TYPE
    /// \param type the function type whose function name is requested
    /// \return the OpenCL API function name for the specified CL_FUNC_TYPE
    const QString& GetOpenCLAPIString(CL_FUNC_TYPE type);

    /// Gets the groups for the specified function type
    /// \param type the function type whose groups are requested
    /// \return the groups for the specified function type
    CLAPIGroups GetCLAPIGroup(CL_FUNC_TYPE type);

    /// Gets the CL_FUNC_TYPE value for the given function name
    /// \param name the function name whose CL_FUNC_TYPE value is requested
    /// \return the CL_FUNC_TYPE value for the given function name
    CL_FUNC_TYPE ToCLAPIType(QString name);

    /// Gets the group name from the given CLAPIGroup value
    /// \param group the CLAPIGroup whose name is requested
    /// \return the group name from the given CLAPIGroup value
    const QString GroupToString(CLAPIGroup group);

    /// Is this an OpenCL function string:
    /// \param apiFunctionName the name of the function
    /// \return true iff the string represents an OpenCL function
    bool IsCLAPI(const QString& apiFunctionName) const { return m_openCLAPIString.contains(apiFunctionName); };

    /// Append the list of all OpenCL functions to the list:
    /// \param list[out] the list to append the OpenCL functions for
    void AppendAllCLFunctionsToList(QStringList& list) { list << m_openCLAPIString; };


private:
    /// Initializes the static instance of the CLAPIDefs class
    CLAPIDefs();

    QStringList                   m_openCLAPIString;  ///< list of OpenCL function names
    QMap<QString, CL_FUNC_TYPE>      m_openCLAPIMap;     ///< map of function names to CL_FUNC_TYPE value
    QMap<CL_FUNC_TYPE, CLAPIGroups>  m_clAPIGroupMap;    ///< map of CL_FUNC_TYPE to group id
};

#endif // _CLAPI_DEFS_H_

