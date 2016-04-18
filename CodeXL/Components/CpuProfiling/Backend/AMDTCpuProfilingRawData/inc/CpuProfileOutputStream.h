//==================================================================================
// Copyright (c) 2013-2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file CpuProfileOutputStream.h
///
//==================================================================================

#ifndef _CPUPROFILEOUTPUTSTREAM_H_
#define _CPUPROFILEOUTPUTSTREAM_H_

#include "CpuProfilingRawDataDLLBuild.h"
#include <AMDTBaseTools/Include/gtString.h>
#include <ProfilingAgents/Utils/CrtFile.h>

/****************************
 * class CpuProfileOutputStream
 *
 * Description:
 * This is the base class for all writer.
 */
class CP_RAWDATA_API CpuProfileOutputStream
{
public:
    enum OutputStage
    {
        evOut_OK = 0
    };

    CpuProfileOutputStream();
    virtual ~CpuProfileOutputStream();

    bool open(const gtString& path);
    void close();

protected:
    void WriteFormat(const wchar_t* pFormat, ...);

    gtString m_path;
    CrtFile m_fileStream;
    unsigned int m_stage;
};


#endif // _CPUPROFILEOUTPUTSTREAM_H_
