//==================================================================================
// Copyright (c) 2013-2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file InlineInstance.h
///
//==================================================================================

#ifndef _INLINEINSTANCE_H_
#define _INLINEINSTANCE_H_

#include "CpuProfilingRawDataDLLBuild.h"
#include <AMDTBaseTools/Include/gtList.h>
#include <AMDTBaseTools/Include/gtString.h>
#include <AMDTOSWrappers/Include/osOSDefinitions.h>

/*******************************************
 * class InlineInstance
 *
 * Description:
 * This class represent each Java inline instance
 */
class CP_RAWDATA_API InlineInstance
{
public:
    InlineInstance() : m_addr(0), m_size(0) {}

    bool operator<(const InlineInstance& other) const
    {
        return m_addr < other.m_addr || (m_addr == other.m_addr && m_symbol < other.m_symbol);
    }

public:
    gtVAddr  m_addr;
    gtString m_symbol;
    gtUInt32 m_size;
};

/*******************************************
 * Description:
 * This map represents the Java inline instance
 * map listed in the prolog of IMD file
 */
typedef gtList<InlineInstance> InlineInstanceList;

#endif // _INLINEINSTANCE_H_