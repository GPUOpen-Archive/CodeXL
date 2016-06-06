//==============================================================================
// Copyright (c) 2014-2015 Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools
/// \file
/// \brief Description: A struct to hold brig section header information
//==============================================================================
#ifndef BRIGSECTIONHEADER_H_
#define BRIGSECTIONHEADER_H_

struct BrigSectionHeader
{
    uint64_t byteCount;
    uint32_t headerByteCount;
    uint32_t nameLength;
    uint8_t name[1];
};

#endif // BRIGSECTIONHEADER_H_
