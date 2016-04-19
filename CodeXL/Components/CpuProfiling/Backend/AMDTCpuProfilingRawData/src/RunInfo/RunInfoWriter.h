//==================================================================================
// Copyright (c) 2013-2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file RunInfoWriter.h
///
//==================================================================================

#ifndef _RUNINFOWRITER_H_
#define _RUNINFOWRITER_H_

#include <RunInfo.h>
#include <ProfilingAgents/Utils/CrtFile.h>

// This class implements the RI file writer
class RunInfoWriter
{
public:
    RunInfoWriter();
    ~RunInfoWriter();

    // writes the RI file with the provided data
    HRESULT Write(/* [in] */ const wchar_t* pfilePath, /* [in] */ const RunInfo* pRunInfo);

private:
    // Write the next record in the RI file
    bool WriteNextRIRecord(CrtFile& file, RiRecord* pRIRec);
};

#endif // _RUNINFOWRITER_H_
