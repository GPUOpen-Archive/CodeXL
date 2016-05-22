//==================================================================================
// Copyright (c) 2013-2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file CpuProfileProcess.h
///
//==================================================================================

#ifndef _PROCESS_H_
#define _PROCESS_H_

#include "CpuProfileSample.h"
#include <AMDTBaseTools/Include/gtString.h>

/***********************************************************
 * class CpuProfileProcess
 *
 * Description:
 * This class represent a process. It represents each
 * line in the [PROCESS] section
 */
class CP_RAWDATA_API CpuProfileProcess : public AggregatedSample
{
public:
    CpuProfileProcess();

    void clear();

    bool compares(const CpuProfileProcess& p, wchar_t* strerr, size_t maxlen) const;

    const gtString& getPath() const { return m_fullPath; }
    void setPath(const gtString& path) { m_fullPath = path; }

public:
    bool m_is32Bit = false;
    bool m_hasCss = false;

private:
    gtString m_fullPath;
};


/***********************************************************
 * Description:
 * This map represent the [PROCESS] section.
 */
typedef gtMap<ProcessIdType, CpuProfileProcess>  PidProcessMap;

#endif // _PROCESS_H_
