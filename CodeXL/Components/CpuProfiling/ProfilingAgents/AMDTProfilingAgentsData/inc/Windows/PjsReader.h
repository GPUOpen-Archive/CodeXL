//==================================================================================
// Copyright (c) 2013-2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file PjsReader.h
/// \brief This file provides the interface to read a PJS (Pre-JIT Symbol) file.
///
//==================================================================================

#ifndef _PJSREADER_H_
#define _PJSREADER_H_

#include "../ProfilingAgentsDataDLLBuild.h"
#include "PjsHeader.h"
#include <ProfilingAgents/Utils/CrtFile.h>

class AGENTDATA_API PjsReader
{
public:
    PjsReader();
    ~PjsReader();

    bool Open(const wchar_t* pFileName);
    void Close();
    unsigned int GetNumberOfRecords();
    bool GetPreJITModuleName(wchar_t* pModuleName, unsigned int size);
    gtUInt64 GetLoadAddress();

    // always return 64-bit value, caller will decide if truncation
    // is need based on Pre-JITed module bitness.
    bool GetNextRecord(gtUInt64* pStartAddr, gtUInt32* pSize, char* pSymbol, gtUInt32 symbolLength);

private:
    bool ReadHeader();

private:
    wchar_t* m_pModuleName;
    gtUInt64 m_LoadAddr;
    DWORD    m_RecordCount;
    DWORD    m_RecordRead;
    CrtFile  m_fileStream;
};

#endif // _PJSREADER_H_
