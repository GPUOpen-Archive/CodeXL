//==================================================================================
// Copyright (c) 2013-2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file EventEncoding.cpp
/// \brief Encoding/decoding functions between PERF_CTL and EventMaskType.
///
//==================================================================================

#include <EventEncoding.h>

EventMaskType EncodeEvent(gtUInt16 event, gtUByte unitMask, bool bitOs, bool bitUsr)
{
    // Since gtUInt16 is 16 bits, this will combine them
    gtUInt32 ret = event;
    ret |= unitMask << 16;
    ret |= static_cast<gtUInt32>(bitOs) << 25;
    ret |= static_cast<gtUInt32>(bitUsr) << 24;
    return static_cast<EventMaskType>(ret);
}

EventMaskType EncodeEvent(PERF_CTL ctl)
{
    EventMaskType ret = GetEvent12BitSelect(ctl) | (ctl.ucUnitMask << 16) | (ctl.bitOsEvents << 25) | (ctl.bitUsrEvents << 24);
    return ret;
}

void DecodeEvent(EventMaskType encoded, gtUInt16* pEvent, gtUByte* pUnitMask, bool* pBitOs, bool* pBitUsr)
{
    if (NULL != pEvent)
    {
        gtUInt16 mask = static_cast<gtUInt16>(-1);
        *pEvent = encoded & mask;
    }

    if (NULL != pUnitMask)
    {
        gtUByte mask = static_cast<gtUByte>(-1);
        *pUnitMask = (encoded >> 16) & mask;
    }

    if (NULL != pBitOs)
    {
        *pBitOs = (encoded >> 25) & 1U;
    }

    if (NULL != pBitUsr)
    {
        *pBitUsr = (encoded >> 24) & 1U;
    }
}

void DecodeEvent(EventMaskType encoded, PERF_CTL* pCtl)
{
    if (NULL != pCtl)
    {
        gtUInt32 sel = encoded & 0xFFFU;
        pCtl->ucEventSelect = sel & 0xFFU;
        pCtl->ucEventSelectHigh = (sel >> 8) & 0xFU;
        pCtl->ucUnitMask = (encoded >> 16) & 0xFFU;
        pCtl->bitOsEvents = (encoded >> 25) & 1U;
        pCtl->bitUsrEvents = (encoded >> 24) & 1U;
    }
}

unsigned int GetEvent12BitSelect(PERF_CTL ctl)
{
    unsigned int sel = ((ctl.ucEventSelectHigh << 8) + ctl.ucEventSelect) & 0xFFF;

    if (FAKE_L2I_MASK_VALUE == ctl.FakeL2IMask)
    {
        // For L2I events - we are actually returning a 16-bit event id after
        // prefixing it by a fake value in bits [15:12]
        sel += (FAKE_L2I_EVENT_ID_PREFIX << 12);
    }

    return sel;
}
