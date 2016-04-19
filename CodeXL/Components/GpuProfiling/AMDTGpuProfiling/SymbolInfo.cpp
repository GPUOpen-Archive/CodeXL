//=====================================================================
// Copyright (c) 2012 Advanced Micro Devices, Inc. All rights reserved.
//
/// \author GPU Developer Tools
/// \file $File: //devtools/main/CodeXL/Components/GpuProfiling/AMDTGpuProfiling/SymbolInfo.cpp $
/// \version $Revision: #4 $
/// \brief :  This file contains the SymbolInfo class
//
//=====================================================================
// $Id: //devtools/main/CodeXL/Components/GpuProfiling/AMDTGpuProfiling/SymbolInfo.cpp#4 $
// Last checkin:   $DateTime: 2016/04/18 06:02:03 $
// Last edited by: $Author: salgrana $
// Change list:    $Change: 569613 $
//=====================================================================

// Local:
#include <AMDTGpuProfiling/SymbolInfo.h>


SymbolInfo::SymbolInfo() :
    m_strApiName(""),
    m_strSymbolName(""),
    m_strFileName(""),
    m_nLineNumber(0)
{

}

SymbolInfo::SymbolInfo(const QString& strApiName, const QString& strSymbolName, const QString& strFileName, int nlineNumber) :
    m_strApiName(strApiName),
    m_strSymbolName(strSymbolName),
    m_strFileName(strFileName),
    m_nLineNumber(nlineNumber)
{
    // .st file contains &nbsp; in place of spaces in filenames, replace those with spaces so that the file can be opened/read
    m_strFileName = m_strFileName.replace("&nbsp;", " ");
}

QString SymbolInfo::ApiName() const
{
    return m_strApiName;
}

QString SymbolInfo::SymbolName() const
{
    return m_strSymbolName;
}

QString SymbolInfo::FileName() const
{
    return m_strFileName;
}

int SymbolInfo::LineNumber() const
{
    return m_nLineNumber;
}

