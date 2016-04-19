//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file TrapFrameTraverser.cpp
///
//==================================================================================

#include "TrapFrameTraverser.h"
#include "StackFrameTraverser.h"
#include <dia2.h>

typedef unsigned char  UCHAR;
typedef unsigned short USHORT;
typedef unsigned long  ULONG;

#define X86_MODE_MASK               1
#define X86_EFLAGS_V86_MASK         0x00020000

typedef struct _X86_KTRAP_FRAME
{


    //
    //  Following 4 values are only used and defined for DBG systems,
    //  but are always allocated to make switching from DBG to non-DBG
    //  and back quicker.  They are not DEVL because they have a non-0
    //  performance impact.
    //

    ULONG   DbgEbp;         // Copy of User EBP set up so KB will work.
    ULONG   DbgEip;         // EIP of caller to system call, again, for KB.
    ULONG   DbgArgMark;     // Marker to show no args here.
    ULONG   DbgArgPointer;  // Pointer to the actual args

    //
    //  Temporary values used when frames are edited.
    //
    //
    //  NOTE:   Any code that want's ESP must materialize it, since it
    //          is not stored in the frame for kernel mode callers.
    //
    //          And code that sets ESP in a KERNEL mode frame, must put
    //          the new value in TempEsp, make sure that TempSegCs holds
    //          the real SegCs value, and put a special marker value into SegCs.
    //

    ULONG   TempSegCs;
    ULONG   TempEsp;

    //
    //  Debug registers.
    //

    ULONG   Dr0;
    ULONG   Dr1;
    ULONG   Dr2;
    ULONG   Dr3;
    ULONG   Dr6;
    ULONG   Dr7;

    //
    //  Segment registers
    //

    ULONG   SegGs;
    ULONG   SegEs;
    ULONG   SegDs;

    //
    //  Volatile registers
    //

    ULONG   Edx;
    ULONG   Ecx;
    ULONG   Eax;

    //
    //  Nesting state, not part of context record
    //

    ULONG   PreviousPreviousMode;

    ULONG   ExceptionList;
    // Trash if caller was user mode.
    // Saved exception list if caller
    // was kernel mode or we're in
    // an interrupt.

    //
    //  FS is TIB/PCR pointer, is here to make save sequence easy
    //

    ULONG   SegFs;

    //
    //  Non-volatile registers
    //

    ULONG   Edi;
    ULONG   Esi;
    ULONG   Ebx;
    ULONG   Ebp;

    //
    //  Control registers
    //

    ULONG   ErrCode;
    ULONG   Eip;
    ULONG   SegCs;
    ULONG   EFlags;

    ULONG   HardwareEsp;    // WARNING - segSS:esp are only here for stacks
    ULONG   HardwareSegSs;  // that involve a ring transition.

    ULONG   V86Es;          // these will be present for all transitions from
    ULONG   V86Ds;          // V86 mode
    ULONG   V86Fs;
    ULONG   V86Gs;
} X86_KTRAP_FRAME, *PX86_KTRAP_FRAME;


TrapFrameTraverser::TrapFrameTraverser(StackWalkContextX86& context) : m_context(context), m_pFrameData(NULL), m_stackPtr(0)
{
}

TrapFrameTraverser::~TrapFrameTraverser()
{
    if (NULL != m_pFrameData)
    {
        m_pFrameData->Release();
    }
}

void TrapFrameTraverser::Reset(struct IDiaFrameData* pFrameData, VAddrX86 stackPtr)
{
    if (NULL != pFrameData)
    {
        pFrameData->AddRef();
    }

    if (NULL != m_pFrameData)
    {
        m_pFrameData->Release();
    }

    m_pFrameData = pFrameData;
    m_stackPtr = stackPtr;
}

HRESULT TrapFrameTraverser::GetFrameData(StackFrameData& frameData) const
{
    return StackFrameTraverser::GetFrameData(m_context, m_stackPtr, m_pFrameData, frameData);
}

HRESULT TrapFrameTraverser::TraverseNext()
{
    HRESULT hr = E_DIA_FRAME_ACCESS;
    VAddrX86 stackPtr;

    if (m_context.GetRegister(REG_INDEX_ESP, stackPtr))
    {
        X86_KTRAP_FRAME trapFrame;
        int trapFrameSize = m_context.ReadMemory(MEM_TYPE_STACK, stackPtr, trapFrame);

        if (0 < trapFrameSize)
        {
            if ((trapFrameSize == sizeof(trapFrame)) || (
                    // Not shorter then the smallest possible frame type.
                    (trapFrameSize >= (sizeof(trapFrame) - 20)) &&
                    // Not too small for inter-ring frame.
                    (0 == (trapFrame.SegCs & X86_MODE_MASK) || trapFrameSize >= (sizeof(trapFrame) - 16)) &&
                    // Not too small for V86 frame.
                    (0 == (trapFrame.EFlags & X86_EFLAGS_V86_MASK))))
            {
                // If this is a user-mode frame, the real value of Esp is in HardwareEsp.
                if (0 != (trapFrame.SegCs & X86_MODE_MASK) || 0 != (trapFrame.EFlags & X86_EFLAGS_V86_MASK))
                {
                    m_context.SetRegister(REG_INDEX_ESP, trapFrame.HardwareEsp);
                }
                else
                {
                    m_context.SetRegister(REG_INDEX_ESP, trapFrame.TempEsp);
                }

                m_context.SetVFrame(trapFrame.Ebp);
                m_context.SetRegister(REG_INDEX_EBP, trapFrame.Ebp);
                m_context.SetRegister(REG_INDEX_EIP, trapFrame.Eip);
                m_context.SetRegister(REG_INDEX_EBX, trapFrame.Ebx);
            }
        }
    }

    return hr;
}
