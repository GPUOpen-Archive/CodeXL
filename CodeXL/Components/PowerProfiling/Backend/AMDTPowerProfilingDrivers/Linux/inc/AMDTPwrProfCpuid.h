//===============================================================================
//
// Copyright(c) 2015 Advanced Micro Devices, Inc.All Rights Reserved
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files(the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and / or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions :
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//
//=================================================================================

#ifndef _PWR_PROF_DRV_CPUID_H_
#define _PWR_PROF_DRV_CPUID_H_

/// The enumeration used to retrieve data from the __cpuid intrinsic
enum
{
    /// The offset of EAX data
    EAX_OFFSET,
    /// The offset of EBX data
    EBX_OFFSET,
    /// The offset of ECX data
    ECX_OFFSET,
    /// The offset of EDX data
    EDX_OFFSET,
    /// The number of values in the CPUID array
    NUM_CPUID_OFFSETS
};

#define CPUID_FnVendorIdentification 0
#define CPUID_FnBasicFeatures 1
// LogicalProcessorCount: logical processor count
#define CPUID_FnBasicFeatures_EBX_LogicalProcessorCount_OFFSET 16
#define CPUID_FnBasicFeatures_EBX_LogicalProcessorCount (0xFF << CPUID_FnBasicFeatures_EBX_LogicalProcessorCount_OFFSET)
// LocalApicId: initial local APIC physical ID
#define CPUID_FnBasicFeatures_EBX_LocalApicId_OFFSET 24
#define CPUID_FnBasicFeatures_EBX_LocalApicId (0xFF << CPUID_FnBasicFeatures_EBX_LocalApicId_OFFSET)
// Hypervisor: used by hypervisor to indicate guest status
#define CPUID_FnBasicFeatures_ECX_Hypervisor (1 << 31)
// APIC: advanced programmable interrupt controller (APIC) exists and is enabled
#define CPUID_FnBasicFeatures_EDX_APIC (1 << 9)
#define CPUID_FnThermalAndPowerManagement 6
// ARAT: always running APIC timer
#define CPUID_FnThermalAndPowerManagement_EAX_ARAT (1 << 2)
// EffFreq: effective frequency interface
#define CPUID_FnThermalAndPowerManagement_ECX_EffFreq (1 << 0)
#define CPUID_FnExtendedVendorIdentification 0x80000000
#define CPUID_FnAmdExtendedFeatures 0x80000001
// IBS: Instruction Based Sampling
#define CPUID_FnAmdExtendedFeatures_ECX_IBS (1 << 10)
// PerfCtrExtCore: core performance counter extensions support
#define CPUID_FnAmdExtendedFeatures_ECX_PerfCtrExtCore (1 << 23)
// PerfCtrExtNB: NB performance counter extensions support
#define CPUID_FnAmdExtendedFeatures_ECX_PerfCtrExtNB (1 << 24)
// PerfCtrExtL2I: L2I performance counter extensions support
#define CPUID_FnAmdExtendedFeatures_ECX_PerfCtrExtL2I (1 << 28)
#define CPUID_FnSizeIdentifiers 0x80000008
// ApicIdCoreIdSize: APIC ID size
#define CPUID_FnSizeIdentifiers_ECX_ApicIdCoreIdSize_OFFSET 12
#define CPUID_FnSizeIdentifiers_ECX_ApicIdCoreIdSize (0xF << CPUID_FnSizeIdentifiers_ECX_ApicIdCoreIdSize_OFFSET)
#define CPUID_FnL1CacheIdentifiers 0x80000005
#define CPUID_FnL2L3CacheIdentifiers 0x80000006
#define CPUID_FnProcessorCapabilities 0x80000007
#define CPUID_FnTLB1GIdentifiers 0x80000019
#define CPUID_FnIbsIdentifiers 0x8000001B
// RdWrOpCnt: read write of op counter supported
#define CPUID_FnIbsIdentifiers_EAX_RdWrOpCnt (1 << 3)
// BrnTrgt: branch target address reporting supported
#define CPUID_FnIbsIdentifiers_EAX_BrnTrgt (1 << 5)
// OpCntExt: IbsOpCurCnt and IbsOpMaxCnt extend by 7 bits
#define CPUID_FnIbsIdentifiers_EAX_OpCntExt (1 << 6)
// RipInvalidChk: invalid RIP indication supported
#define CPUID_FnIbsIdentifiers_EAX_RipInvalidChk (1 << 7)
#define CPUID_FnIdentifiers 0x8000001E
#define CPUID_FnIdentifiers_ECX_NodeId 0xF

#endif // _PWR_PROF_DRV_CPUID_H_

