//==================================================================================
// Copyright (c) 2013-2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file CpuProfileOutputStream.cpp
///
//==================================================================================

#include <CpuProfileOutputStream.h>
#include <AMDTOSWrappers/Include/osOSDefinitions.h>
#include <cstdarg>

CpuProfileOutputStream::CpuProfileOutputStream()
{
    m_stage = evOut_OK;
}

CpuProfileOutputStream::~CpuProfileOutputStream()
{
    close();
}

bool CpuProfileOutputStream::open(const gtString& path)
{
    bool ret = m_fileStream.open(path.asCharArray(), WINDOWS_SWITCH(FMODE_TEXT("w, ccs=UTF-8"), FMODE_TEXT("wb")));

    if (ret)
    {
        m_path = path;
    }

    return ret;
}

void CpuProfileOutputStream::close()
{
    if (m_fileStream.isOpened())
    {
        m_fileStream.close();
    }
}

void CpuProfileOutputStream::WriteFormat(const wchar_t* pFormat, ...)
{
    va_list args;
    va_start(args, pFormat);
    vfwprintf(m_fileStream.getHandler(), pFormat, args);
    va_end(args);
}
