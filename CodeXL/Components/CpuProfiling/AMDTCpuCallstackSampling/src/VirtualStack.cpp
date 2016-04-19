//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file VirtualStack.cpp
///
//==================================================================================

#include <VirtualStack.h>
#include <AMDTBaseTools/Include/gtAlgorithms.h>

int VirtualStack::GetOffset(gtVAddr va) const
{
    return static_cast<int>(va - m_top);
}

bool VirtualStack::IsStackAddress(gtVAddr va) const
{
    // Check if va is in the range [m_top - 1KB, m_top + 16MB)
    return ((va + 1024ULL - m_top) < gtVAddr((16 * 1024 * 1024) + 1024));
}

gtUInt32 VirtualStack::GetValue(int offset) const
{
    gtUInt32 value = 0U;

    if (0 <= offset)
    {
        gtUInt16 valueOffset = static_cast<gtUInt16>(offset / sizeof(gtUInt32));
        gtUInt16 const* const pOffsetsBegin = m_pOffsets;
        gtUInt16 const* const pOffsetsEnd = pOffsetsBegin + m_count;
        gtUInt16 const* pOffset = gtBinarySearch(pOffsetsBegin, pOffsetsEnd, valueOffset);

        if (pOffsetsEnd != pOffset)
        {
            value = m_pValues[pOffset - pOffsetsBegin];
        }
    }

    return value;
}

gtUInt32 VirtualStack::RecoverFrame(gtUInt32& framePtr, gtUInt32& stackPtr) const
{
    gtUInt32 retAddr = 0U;
    int offset = GetOffset(static_cast<gtVAddr>(framePtr));

    if (0 <= offset)
    {
        gtUInt16 valueOffset = static_cast<gtUInt16>(offset / sizeof(gtUInt32));
        gtUInt16 const* const pOffsetsBegin = m_pOffsets;
        gtUInt16 const* const pOffsetsEnd = pOffsetsBegin + m_count;
        gtUInt16 const* pOffset = gtBinarySearch(pOffsetsBegin, pOffsetsEnd, valueOffset);

        if (pOffsetsEnd > (pOffset + 1) && (pOffset[0] + 1) == pOffset[1])
        {
            stackPtr = framePtr + 2 * sizeof(gtUInt32);

            const gtUInt32* pValue = &m_pValues[pOffset - pOffsetsBegin];
            framePtr = pValue[0];
            retAddr = pValue[1];
        }
    }

    return retAddr;
}

gtUInt32 VirtualStack::ReadMemory(gtVAddr va, gtUByte* pBuffer, gtUInt32 size) const
{
    gtUInt32 bytesRead = size;

    gtUInt16 const* const pOffsetsBegin = m_pOffsets;
    gtUInt16 const* const pOffsetsEnd = pOffsetsBegin + m_count;
    gtUInt16 const* pOffset = pOffsetsBegin;
    gtUInt16 currentOffset;

    int baseOffset = GetOffset(va);

    if (0 < baseOffset)
    {
        int alignOffset = (baseOffset % sizeof(gtUInt32));

        if (0 != alignOffset)
        {
            memset(pBuffer, 0, alignOffset);
            pBuffer += alignOffset;
            baseOffset -= alignOffset;
            size -= static_cast<gtUInt32>(alignOffset);
        }

        currentOffset = static_cast<gtUInt16>(baseOffset / sizeof(gtUInt32));
        pOffset = gtLowerBound(pOffset, pOffsetsEnd, currentOffset);
    }
    else
    {
        if (0 != baseOffset)
        {
            baseOffset = -baseOffset;

            if (size < static_cast<gtUInt32>(baseOffset))
            {
                baseOffset = static_cast<int>(size);
            }

            memset(pBuffer, 0, baseOffset);
            pBuffer += baseOffset;
            size -= static_cast<gtUInt32>(baseOffset);
        }

        currentOffset = 0;
    }

    gtUInt32* pBuf32 = reinterpret_cast<gtUInt32*>(pBuffer);

    if (pOffsetsEnd != pOffset)
    {
        while (sizeof(gtUInt32) <= size)
        {
            size -= sizeof(gtUInt32);
            gtUInt32 value = 0;

            if (*pOffset == currentOffset)
            {
                value = m_pValues[pOffset - pOffsetsBegin];
                ++pOffset;

                if (pOffsetsEnd == pOffset || ((*pOffset - currentOffset) * sizeof(gtUInt32)) > size)
                {
                    *pBuf32++ = value;
                    break;
                }
            }

            *pBuf32++ = value;
            currentOffset++;
        }
    }

    if (0U != size)
    {
        memset(pBuf32, 0, size);
    }

    return bytesRead;
}
