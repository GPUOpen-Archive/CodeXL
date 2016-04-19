//==================================================================================
// Copyright (c) 2013-2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file PerfProfiler.cpp
///
//==================================================================================

// Standard headers
#include <string.h>
#include <stdlib.h>

// Project headers
#include "PerfProfiler.h"

void PerfProfiler::clear()
{
    m_pTarget = NULL;
    m_pProfileConfig = NULL;
}

void PerfProfiler::setOutputFile(const std::string& file, bool overwrite)
{
    m_outputFile = file;

    if (m_pProfileConfig)
    {
        m_pProfileConfig->setOutputFile(m_outputFile, overwrite);
    }
}
