//==================================================================================
// Copyright (c) 2013-2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file JclReader.h
/// \brief This file contains an interface to read JCL file for Java code profiling.
///
//==================================================================================

#ifndef _JCLREADER_H_
#define _JCLREADER_H_

#include "ProfilingAgentsDataDLLBuild.h"
#include "JclHeader.h"
#include <ProfilingAgents/Utils/CrtFile.h>

class AGENTDATA_API JclReader
{
public:
    JclReader(const wchar_t* jclName);
    ~JclReader();

    void Close();
    bool Open();
    bool ReadNextRecordType(JitRecordType* pRecordType);
    bool ReadLoadRecord(JitLoadRecord* pJitLoadRec);
    bool ReadUnLoadRecord(JitUnloadRecord* pJitUnloadRec);
    bool ReadHeader(std::wstring* pAppName);
    bool Is32Bit() const { return 0 != m_is32Bit; }

protected:
    CrtFile       m_fileStream;
    wchar_t       m_fileName[OS_MAX_PATH];
    unsigned int  m_is32Bit;
    int           m_version;
};

#endif // _JCLREADER_H_
