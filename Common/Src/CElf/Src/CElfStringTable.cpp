//=====================================================================
// Copyright (c) 2010 Advanced Micro Devices, Inc. All rights reserved.
//
/// \author GPU Developer Tools
/// \file $File: //devtools/main/Common/Src/CElf/Src/CElfStringTable.cpp $
/// \version $Revision: #3 $
/// \brief  A set of classes to allow easy manipulation of ELF
///         binaries in memory.
//
//=====================================================================
// $Id: //devtools/main/Common/Src/CElf/Src/CElfStringTable.cpp#3 $
// Last checkin:   $DateTime: 2015/11/02 15:29:15 $
// Last edited by: $Author: callan $
// Change list:    $Change: 546423 $
//=====================================================================

#include <cstring>
#include "CElf.h"


// We will rebuild the string tables when we write out data.
// We will do this by resetting the data to be empty and
// adding back all of the still useful names.
// Implementation detail: remove duplicates by hashing along the way?
// Implementation detail: optimize substring case?  A: No. It's not worth it.


CElfStringTable::CElfStringTable()
{
    // Make assertions about SHT_STRTAB.
    // Call base class ctor.
}

CElfStringTable::~CElfStringTable()
{
    // Clean up symbol table data structures.
}

size_t
CElfStringTable::AddString(
    const std::string& str)
{
    size_t location = m_Data.size();
    // Plus 1 for the NUL.
    m_Data.resize(location + str.size() + 1);
    memcpy(&m_Data[location], str.data(), str.size());
    m_Data[location + str.size()] = '\0';

    return location;
}
