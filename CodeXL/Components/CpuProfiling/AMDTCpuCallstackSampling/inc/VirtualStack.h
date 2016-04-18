//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file VirtualStack.h
///
//==================================================================================

#ifndef _VIRTUALSTACK_H_
#define _VIRTUALSTACK_H_

#include "CpuCallstackSamplingDLLBuild.h"

struct CP_CSS_API VirtualStack
{
    gtVAddr m_top;
    gtUInt32* m_pValues;
    gtUInt16* m_pOffsets;
    unsigned m_count;
    unsigned m_ptrSize;

    bool IsStackAddress(gtVAddr va) const;
    int GetOffset(gtVAddr va) const;
    gtUInt32 GetValue(int offset) const;
    gtUInt32 RecoverFrame(gtUInt32& framePtr, gtUInt32& stackPtr) const;
    gtUInt32 ReadMemory(gtVAddr va, gtUByte* pBuffer, gtUInt32 size) const;
};

#endif // _VIRTUALSTACK_H_
