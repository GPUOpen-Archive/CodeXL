//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file TssFrameTraverser.cpp
///
//==================================================================================

#include "TssFrameTraverser.h"
#include "StackFrameTraverser.h"
#include <dia2.h>

typedef unsigned char  UCHAR;
typedef unsigned short USHORT;
typedef unsigned long  ULONG;

#define X86_KGDT_TSS  40
#define X86_KGDT_LDT  72

#define X86_PAGE_SHIFT  12L

#define X86_MAX_RING  3

typedef struct _X86_KTSS
{
    // Intel's TSS format
    ULONG   Previous;
    struct
    {
        ULONG   Esp;
        ULONG   Ss;
    } Ring[X86_MAX_RING];
    ULONG   Cr3;
    ULONG   Eip;
    ULONG   EFlags;
    ULONG   Eax;
    ULONG   Ecx;
    ULONG   Edx;
    ULONG   Ebx;
    ULONG   Esp;
    ULONG   Ebp;
    ULONG   Esi;
    ULONG   Edi;
    ULONG   Es;
    ULONG   Cs;
    ULONG   Ss;
    ULONG   Ds;
    ULONG   Fs;
    ULONG   Gs;
    ULONG   Ldt;
    USHORT  T;
    USHORT  IoMapBase;
} X86_KTSS, *PX86_KTSS;

//
//  LDT descriptor entry
//

typedef struct _X86_LDT_ENTRY
{
    USHORT  LimitLow;
    USHORT  BaseLow;
    union
    {
        struct
        {
            UCHAR   BaseMid;
            UCHAR   Flags1;     // Declare as bytes to avoid alignment
            UCHAR   Flags2;     // Problems.
            UCHAR   BaseHi;
        } Bytes;
        struct
        {
            ULONG   BaseMid : 8;
            ULONG   Type : 5;
            ULONG   Dpl : 2;
            ULONG   Pres : 1;
            ULONG   LimitHi : 4;
            ULONG   Sys : 1;
            ULONG   Reserved_0 : 1;
            ULONG   Default_Big : 1;
            ULONG   Granularity : 1;
            ULONG   BaseHi : 8;
        } Bits;
    } HighWord;
} X86_LDT_ENTRY, *PX86_LDT_ENTRY;

typedef struct _X86_DESCRIPTOR_TABLE_ENTRY
{
    ULONG Selector;
    X86_LDT_ENTRY Descriptor;
} X86_DESCRIPTOR_TABLE_ENTRY, *PX86_DESCRIPTOR_TABLE_ENTRY;

//
// Special Registers for i386
//

typedef struct _X86_DESCRIPTOR
{
    USHORT  Pad;
    USHORT  Limit;
    ULONG   Base;
} X86_DESCRIPTOR, *PX86_DESCRIPTOR;

typedef struct _X86_KSPECIAL_REGISTERS
{
    ULONG Cr0;
    ULONG Cr2;
    ULONG Cr3;
    ULONG Cr4;
    ULONG KernelDr0;
    ULONG KernelDr1;
    ULONG KernelDr2;
    ULONG KernelDr3;
    ULONG KernelDr6;
    ULONG KernelDr7;
    X86_DESCRIPTOR Gdtr;
    X86_DESCRIPTOR Idtr;
    USHORT Tr;
    USHORT Ldtr;
    ULONG Reserved[6];
} X86_KSPECIAL_REGISTERS, *PX86_KSPECIAL_REGISTERS;


TssFrameTraverser::TssFrameTraverser(StackWalkContextX86& context) : m_context(context), m_pFrameData(NULL), m_stackPtr(0)
{
}

TssFrameTraverser::~TssFrameTraverser()
{
    if (NULL != m_pFrameData)
    {
        m_pFrameData->Release();
    }
}

void TssFrameTraverser::Reset(struct IDiaFrameData* pFrameData, VAddrX86 stackPtr)
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

HRESULT TssFrameTraverser::GetFrameData(StackFrameData& frameData) const
{
    return StackFrameTraverser::GetFrameData(m_context, m_stackPtr, m_pFrameData, frameData);
}

HRESULT TssFrameTraverser::GetSegmentDescriptor(X86_DESCRIPTOR_TABLE_ENTRY& descTableEntry)
{
    HRESULT hr = E_DIA_FRAME_ACCESS;

    ULONG tableBase;
    USHORT tableLimit;

    //
    // Fetch the address and limit of the GDT
    //

    if (m_context.ReadFullMemory(MEM_TYPE_ANY, offsetof(X86_KSPECIAL_REGISTERS, Gdtr.Base), tableBase) &&
        m_context.ReadFullMemory(MEM_TYPE_ANY, offsetof(X86_KSPECIAL_REGISTERS, Gdtr.Limit), tableLimit))
    {
        // Find out whether this is a GDT or LDT selector.
        if (0 != (descTableEntry.Selector & 0x4))
        {
            X86_LDT_ENTRY Descriptor;

            //
            // This is an LDT selector, so we reload the TableBase and TableLimit
            // with the LDT's Base & Limit by loading the descriptor for the
            // LDT selector.
            //

            if (!m_context.ReadFullMemory(MEM_TYPE_ANY, tableBase + X86_KGDT_LDT, Descriptor))
            {
                return hr;
            }

            tableBase = ((ULONG)Descriptor.BaseLow) +
                        ((ULONG)Descriptor.HighWord.Bits.BaseMid << 16) +
                        ((ULONG)Descriptor.HighWord.Bytes.BaseHi << 24);

            // LDT can't be greater than 64k
            tableLimit = Descriptor.LimitLow;

            if (Descriptor.HighWord.Bits.Granularity)
            {
                // I suppose it's possible, although silly, to have a LDT with page granularity.
                tableLimit <<= X86_PAGE_SHIFT;
            }
        }

        // The 3 lower bits are irrelevant.
        USHORT index = (USHORT)(descTableEntry.Selector) & ~0x7;

        // Check to make sure that the selector is within the table bounds.
        if (index < tableLimit)
        {
            if (m_context.ReadFullMemory(MEM_TYPE_ANY, tableBase + index, descTableEntry.Descriptor))
            {
                hr = S_OK;
            }
        }
    }

    return hr;
}

HRESULT TssFrameTraverser::TraverseNext()
{
    X86_DESCRIPTOR_TABLE_ENTRY desc;
    desc.Selector = X86_KGDT_TSS;
    HRESULT hr = GetSegmentDescriptor(desc);

    if (S_OK == hr)
    {
        // Check if this is a 32bit task descriptor.
        if (desc.Descriptor.HighWord.Bits.Type == 9 ||
            desc.Descriptor.HighWord.Bits.Type == 0xB)
        {
            //
            // Read in Task State Segment.
            //

            struct
            {
                ULONG   r1[8];
                ULONG   Eip;
                ULONG   EFlags;
                ULONG   Eax;
                ULONG   Ecx;
                ULONG   Edx;
                ULONG   Ebx;
                ULONG   Esp;
                ULONG   Ebp;
                ULONG   Esi;
                ULONG   Edi;
                ULONG   Es;
                ULONG   Cs;
                ULONG   Ss;
                ULONG   Ds;
                ULONG   Fs;
                ULONG   Gs;
            } taskState;

            ULONG stackAddr = ((ULONG)desc.Descriptor.BaseLow) +
                              ((ULONG)desc.Descriptor.HighWord.Bytes.BaseMid << 16) +
                              ((ULONG)desc.Descriptor.HighWord.Bytes.BaseHi << 24);

            if (m_context.ReadFullMemory(MEM_TYPE_ANY, stackAddr, taskState))
            {
                m_context.SetVFrame(taskState.Ebp);
                m_context.SetRegister(REG_INDEX_ESP, taskState.Esp);
                m_context.SetRegister(REG_INDEX_EBP, taskState.Ebp);
                m_context.SetRegister(REG_INDEX_EIP, taskState.Eip);
                m_context.SetRegister(REG_INDEX_EBX, taskState.Ebx);
            }
            else
            {
                hr = E_DIA_FRAME_ACCESS;
            }
        }
        else
        {
            hr = E_DIA_FRAME_ACCESS;
        }
    }

    return hr;
}
