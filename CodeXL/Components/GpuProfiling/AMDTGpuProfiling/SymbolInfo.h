//=====================================================================
// Copyright (c) 2012 Advanced Micro Devices, Inc. All rights reserved.
//
/// \author GPU Developer Tools
/// \file $File: //devtools/main/CodeXL/Components/GpuProfiling/AMDTGpuProfiling/SymbolInfo.h $
/// \version $Revision: #4 $
/// \brief :  This file contains the SymbolInfo class
//
//=====================================================================
// $Id: //devtools/main/CodeXL/Components/GpuProfiling/AMDTGpuProfiling/SymbolInfo.h#4 $
// Last checkin:   $DateTime: 2015/09/03 05:06:23 $
// Last edited by: $Author: salgrana $
// Change list:    $Change: 539203 $
//=====================================================================
#ifndef _SYMBOLINFO_H_
#define _SYMBOLINFO_H_

#include <qtIgnoreCompilerWarnings.h>
#include <QString>



/// Class that holds symbol table information for a given API
class SymbolInfo
{
public:

    /// Initializes a new instance of the SymbolInfo class
    SymbolInfo();

    /// Initializes a new instance of the SymbolInfo class
    /// \param strApiName the OpenCL API Name
    /// \param strSymbolName the symbol Name
    /// \param strFileName the file Name
    /// \param nlineNumber the line number
    SymbolInfo(const QString& strApiName, const QString& strSymbolName, const QString& strFileName, int nlineNumber);

    /// Gets the OpenCL API name
    /// \return the OpenCL API name
    QString ApiName() const;

    /// Gets the symbol name
    /// \return the symbol name
    QString SymbolName() const;

    /// Gets the file name
    /// \return the file name
    QString FileName() const;

    /// Gets the line number
    /// \return the line number
    int LineNumber() const;

private:
    /// Disable default copy constructor
    SymbolInfo(const SymbolInfo&);

    /// Disable default assignment operator
    SymbolInfo& operator=(const SymbolInfo&);

    QString m_strApiName;    ///< the API name
    QString m_strSymbolName; ///< the symbol name
    QString m_strFileName;   ///< the source file name
    int     m_nLineNumber;   ///< the line number
};

#endif // _SYMBOLINFO_H_
