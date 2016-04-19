//==================================================================================
// Copyright (c) 2013-2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file RunInfoReader.h
///
//==================================================================================

#ifndef _RUNINFOREADER_H_
#define _RUNINFOREADER_H_

#include <RunInfo.h>
#include <ProfilingAgents/Utils/CrtFile.h>

// This class implements the RI file reader
class RunInfoReader
{
public:
    RunInfoReader();
    ~RunInfoReader();

    // reads the RI file
    HRESULT Read(/* [in] */ const wchar_t* pfilePath);

    // Get the RI file data
    HRESULT GetRunInfoData(/* [out] */ RunInfo* pInfo);

private:
    // Read the next record in the RI file
    HRESULT ReadNextRiRecord(CrtFile& file, RiRecord* pRIRec, long& fileSize) const;

    RunInfo m_Info;
};

#endif // _RUNINFOREADER_H_
