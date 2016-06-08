//=====================================================================
// Copyright 2012-2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file osCpuid.h
/// \brief  A brief file description that Doxygen makes note of.
//
//=============================================================
// $Id: $
// Last checkin:   $DateTime: $
// Last edited by: $Author: $
// Change list:    $Change: $
//=============================================================
#ifndef _OSCPUID_H
#define _OSCPUID_H

// Infra:
#include <AMDTBaseTools/Include/AMDTDefinitions.h>

// Local:
#include <AMDTOSWrappers/Include/osOSWrappersDLLBuild.h>

/// These enumerations define the current AMD cpu family types returned by CPUID
enum
{
    /// Cpuid is unavailable or family was not returned
    FAMILY_UNKNOWN = 0,
    /// The extended family information is available (or it's a /ref FAMILY_K8)
    FAMILY_EXTENDED = 0x0f,
    /// A K7 cpu
    FAMILY_K7 = 6,
    /// A K8 cpu
    FAMILY_K8 = 0xf,
    /// A Greyhound cpu
    FAMILY_GH = 0x10,
    /// A Griffin cpu
    FAMILY_GR = 0x11,
    /// A Llano cpu
    FAMILY_LN = 0x12,
    /// Ontario (models 0-f), Krishna (10-1f)
    FAMILY_ON = 0x14,
    /// Orochi (models 0-f), Trinity (10-1f), Kaveri (30-3f), Basilisk (40-4f), Geko (50-5f), Carrizo (60-6f)
    /// Pennar (70-7f), Iguana (80-8f)
    FAMILY_OR = 0x15,
    /// Kabini (models 0-f), Thebe (10-1f), Kryptos (20-2f), Samara (50-5f)
    FAMILY_KB = 0x16
};

/// These enumerations define the cpuid functions used
enum
{
    /// Retrieves basic vendor identification and standard cpuid function limits
    CPUID_FN_VENDOR_ID = 0,
    /// Retrieves information about basic features
    CPUID_FN_BASIC_FEATURES = 1,
    /// Retrieves largest extended functionality
    CPUID_FN_MAX_EXT_FN = 0x80000000,
    /// Retrieves information about extended features
    CPUID_FN_EXT_FEATURES = 0x80000001,
    /// Retrieves information about the number of logical cores supported by the processor
    CPUID_FN_SIZE_ID = 0x80000008,
    /// Retrieves information about IBS features
    CPUID_FN_IBS_FEATURES = 0x8000001B,
    /// Retrieves information about Node identifiers
    CPUID_FN_NODE_ID = 0x8000001E,
    /// Retrieves information about Hypervisor
    CPUID_FN_HYPERVISOR_ID = 0x40000000,
    /// Retrieve information about Hypervisor features
    CPUID_FN_HYPERVISOR_FEATURES = 0x40000003
};

/// These enumerations define the bit masks used to identify specific features from the retrieved
/// cpuid information
enum
{
    /// Whether there is a local APIC, from function /ref CPUID_FN_BASIC_FEATURES, EDX
    CPUID_MASK_APIC = 0x200,
    /// Whether IBS is available, from function /ref CPUID_FN_EXT_FEATURES, ECX
    CPUID_MASK_IBS_AVAIL = 0x400,
    /// Whether IBS op counting mode is supported for function /ref CPUID_FN_IBS_FEATURES, EAX
    CPUID_MASK_IBS_OP_CNT = 0x10,
    /// Whether IBS op branch target address is supported for function /ref CPUID_FN_IBS_FEATURES, EAX
    CPUID_MASK_IBS_BR_TGT = 0x20,
    /// Whether IBS extended count is supported for function /ref CPUID_FN_IBS_FEATURES, EAX
    CPUID_MASK_IBS_EXT_CNT = 0x40,
    /// Whether IBS fetch control extnded available for function /ref CPUID_FN_IBS_FEATURES, EAX
    CPUID_MASK_IBS_FETCH_CTL_EXTD = 0x200,
    /// Whether IBS op data 4 available for function /ref CPUID_FN_IBS_FEATURES, EAX
    CPUID_MASK_IBS_OP_DATA4 = 0x400,
    /// Whether a hypervisor is present, from function /ref CPUID_FN_BASIC_FEATURES, ECX
    CPUID_MASK_HYPERVISOR = 0x80000000
};

/// These enumeration define which offset of the dword array contains the register data from cpuid
enum
{
    /// This offset contains EAX data
    EAX = 0,
    /// This offset contains EBX data
    EBX,
    /// This offset contains ECX data
    ECX,
    /// This offset contains EDX data
    EDX,
    /// The maximum number of registers returned by cpuid
    MAX_CPUID_REGS
};

/// These enumerations define the Hypervisor vendor IDs
enum
{
    /// Unknown hypervisor
    HV_VENDOR_UNKNOWN = 0,
    /// VMware Hypervisor
    HV_VENDOR_VMWARE,
    /// Microsoft Hyper-V
    HV_VENDOR_MICROSOFT,
    /// Xen Hypervisor
    HV_VENDOR_XENVMM,
    /// KVM Hypervisor
    HV_VENDOR_KVM
};

/// \union FamilyModelSteppingUnion A simple way to retrieve the encoded bits for cpuid /ref CPUID_FN_BASIC_FEATURES, EAX
union FamilyModelSteppingUnion
{
    struct _info
    {
        /// Bits[3:0] The Cpu stepping
        gtUInt32 stepping: 4;
        /// Bits[7:4] The Cpu base model (model bits [3:0])
        gtUInt32 model: 4;
        /// Bits[11:8] The Cpu base family
        gtUInt32 family: 4;
        /// Bits[15:12] Reserved
        gtUInt32 unknown1: 4;
        /// Bits[19:16] The Cpu extended model (model bits[7:4])
        gtUInt32 extModel: 4;
        /// Bits[27:20] The Cpu extended family (family = family + extFamily)
        gtUInt32 extFamily: 8;
        /// Bits[31:28] Reserverd
        gtUInt32 unknown2: 4;
    } info;
    /// The value of the EAX register
    gtUInt32 eax;
};

// ----------------------------------------------------------------------------------
// Class Name:           OS_API osCpuid
// General Description:
//   Represents a the Cpuid results of a core
//
// Author:      AMD Developer Tools Team
// Creation Date:        4/23/2012
// ----------------------------------------------------------------------------------
/// Responsible for reporting cpuid features
class OS_API osCpuid
{
public:
    /// Constructor
    osCpuid();
    /// Destructor
    ~osCpuid();

    /// Returns whether the Cpu is AMD brand
    bool isCpuAmd();
    /// Returns whether the Cpu has a local APIC (all modern cpus do)
    bool hasLocalApic();
    /// Returns whether Ibs is available
    bool isIbsAvailable();
    /// Returns whether Ibs Ops dispatch counting is available
    bool isIbsOpsDispatchAvailable();
    /// Returns whether Ibs branch target addresses are available
    bool isIbsOpsBrTgtAddrAvailable();
    /// Returns whether the extended count is available
    bool isIbsExtCountAvailable();
    /// Returns whether Ibs Fetch control extended available
    bool isIbsFetchCtlExtdAvailable();
    /// Returns whether Ibs Op Data 4 available
    bool isIbsOpData4Available();
    /// Returns whether a hypervisor is present
    bool hasHypervisor();
    /// Returns whether current hypervisor is supported for CPU profiling
    bool isSupportedHypervisor();
    /// Returns the hypervisor vendor id
    gtUInt32 getHypervisorVendorId();
    /// Returns the family of the core that the instance was created on
    gtUInt32 getFamily();
    /// Returns the model of the core that the instance was created on
    gtUInt32 getModel();
    /// Returns the stepping of the core that the instance was created on
    gtUInt32 getStepping();
    /// Returns the Apic Id of the core that the instance was created on
    gtUInt32 getApicId();
    /// Returns the core of the core that the instance was created on
    gtUInt32 getcore();
    /// Returns the node Id of the core that the instance was created on
    gtUInt32 getNodeId();
    /// Returns the vendor Id string of current hypervisor detected
    bool getHypervisorVendorString(wchar_t* str, gtUInt32 length);
    /// Returns the OS partition type (parent or child) running on MS Hyper-V
    bool isHypervisorRootPartition();

private:
    /// Whether the Cpu is AMD brand
    bool m_isCpuAmd;
    /// Whether the Cpu has a local APIC (all modern cpus do)
    bool m_hasLocalApic;
    /// Whether Ibs is available
    bool m_isIbsAvailable;
    /// Whether Ibs Ops dispatch counting is available
    bool m_isIbsDispatchAvailable;
    /// Whether Ibs branch target addresses are available
    bool m_isIbsBrTgtAddrAvailable;
    /// Whether the extended count is available
    bool m_isIbsExtCountAvailable;
    /// Whether a hypervisor is present
    bool m_hasHypervisor;
    /// Whether Ibs Fetch control extended available
    bool m_isIbsFetchCtlExtdAvailable;
    /// Whether Ibs Op Data 4 available
    bool m_isIbsOpData4Available;
    /// The family of the core that the instance was created on
    gtUInt32 m_family;
    /// The model of the core that the instance was created on
    gtUInt32 m_model;
    /// The stepping of the core that the instance was created on
    gtUInt32 m_stepping;
    /// The Apic Id of the core that the instance was created on
    gtUInt32 m_apicId;
    /// The core that the instance was created on
    gtUInt32 m_core;
    /// The NUMA node of the core that the instance was created on
    gtUInt32 m_node;
    /// Hypervisor vendor id
    gtUInt32 m_hypervisorVendorId;
    /// In case of Microsoft Hv, if current virtual OS is root or child partition
    bool m_isMshvRootPartition;
};

#endif //_OSCPUID_H
