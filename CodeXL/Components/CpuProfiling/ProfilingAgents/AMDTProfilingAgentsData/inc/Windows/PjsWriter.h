//==================================================================================
// Copyright (c) 2013-2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file PjsWriter.h
/// \brief This file provides the interface to write a PJS (Pre-JIT Symbol) file.
///
//==================================================================================

#ifndef _PJSWRITER_H_
#define _PJSWRITER_H_

#include "../ProfilingAgentsDataDLLBuild.h"
#include "PjsHeader.h"
#include <ProfilingAgents/Utils/CrtFile.h>

class AGENTDATA_API PjsWriter
{
public:
    PjsWriter(const wchar_t* pPJSFileName);
    ~PjsWriter();

    bool WriteFileHeader(const wchar_t* pPreJitModuleName, gtVAddr loadAddr);

    // Note: the startAddress could be 32bit, but we write 64-bit value instead.
    bool AddRecord(gtVAddr startAddr, gtUInt32 size, const char* pSymbolName);
    void Close();

private:
    gtUInt32 m_recordCount;
    wchar_t  m_fileName[OS_MAX_FNAME];
    CrtFile  m_fileStream;
};

#endif // _PJSWRITER_H_
