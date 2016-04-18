//==================================================================================
// Copyright (c) 2013-2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file OsvWriter.h
///
//==================================================================================

#ifndef _OSVWRITER_H_
#define _OSVWRITER_H_

#include <string>
#include <stdio.h>

#include "CpuProfilingRawDataDLLBuild.h"
#include "OsvData.h"
#include <AMDTBaseTools/Include/gtString.h>
#include <ProfilingAgents/Utils/CrtFile.h>

class CP_RAWDATA_API OsvWriter
{
public:
    OsvWriter();
    ~OsvWriter();

    void close();
    void clear();
    bool write(const gtString& path);

    void appendTopic(OsvDataItem* pData);
    bool isOpen() const { return m_fileStream.isOpened(); }

protected:
    bool open(const gtString& path);

private:
    void WriteData(OsvDataItem* pData);
    void WriteOneEntry(OsvDataItem* pData);
    void WriteEndEntry();
    void WriteFormat(const wchar_t* pFormat, ...);

    gtString m_path;
    CrtFile m_fileStream;
    OsvDataItem* m_pOsvData;
    OsvDataItem* m_pCur;
    unsigned int m_curLevel;

};

#endif // _OSVWRITER_H_
