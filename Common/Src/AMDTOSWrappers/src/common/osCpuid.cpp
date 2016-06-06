//=====================================================================
// Copyright 2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file osCpuid.cpp
///
//=====================================================================

// Local:
#include <AMDTOSWrappers/Include/osCpuid.h>

#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
//The __cpuid function is defined in intrin.h
#include <intrin.h>

typedef gtInt32 osCpuidParam_t;
#else
#if AMDT_BUILD_TARGET == AMDT_LINUX_OS
/// \def __cpuid Defines the __cpuid function as a macro for Linux
#if AMDT_ADDRESS_SPACE_TYPE == AMDT_64_BIT_ADDRESS_SPACE
#define __cpuid(info, funType)\
    asm("cpuid": "=a" (info[EAX]), "=b"(info[EBX]), "=c" (info[ECX]), "=d" (info[EDX]): "a" (funType));
#else
#include <cpuid.h>
#undef __cpuid
#define __cpuid(info, funType)\
    __get_cpuid(funType, &info[EAX], &info[EBX], &info[ECX], &info[EDX]);
#endif
#else
#error "Unknown AMDT_BUILD_TARGET"
#endif
typedef gtUInt32 osCpuidParam_t;
#endif

// Hypervisor vendor id strings
#define VENDOR_ID_VMWARE     "VMwareVMware"
#define VENDOR_ID_MICROSOFT  "Microsoft Hv"
#define VENDOR_ID_XEN        "XenVMMXenVMM"
#define VENDOR_ID_KVM        "KVMKVMKVM"

// ---------------------------------------------------------------------------
// Name:        osCpuid::osCpuid
// Description: Constructor.  Uses different cpuid functions for the current core
// Author:      AMD Developer Tools Team
// Date:        4/24/2012
// ---------------------------------------------------------------------------
osCpuid::osCpuid() : m_isCpuAmd(false), m_hasLocalApic(false), m_isIbsAvailable(false),
    m_isIbsDispatchAvailable(false), m_isIbsBrTgtAddrAvailable(false), m_isIbsExtCountAvailable(false), m_hasHypervisor(false),
    m_isIbsFetchCtlExtdAvailable(false), m_isIbsOpData4Available(false), m_family(FAMILY_UNKNOWN), m_model(0), m_stepping(0),
    m_apicId(0), m_core(0), m_node(0), m_hypervisorVendorId(HV_VENDOR_UNKNOWN), m_isMshvRootPartition(false)
{
    //cpuid returns 4 register values: eax, ebx, ecx, and edx as dwords
    osCpuidParam_t info[MAX_CPUID_REGS];
    osCpuidParam_t maxExtCpuidFn;
    osCpuidParam_t cpuidFn;
    char vendorId[13];

    //vendor id and highest standard cpuid function available
    cpuidFn = CPUID_FN_VENDOR_ID;
    __cpuid(info, cpuidFn);

    //vendor string is made of three returned dwords as ascii characters

    memcpy(vendorId, &(info[EBX]), 4);
    memcpy((vendorId + 4), &(info[EDX]), 4);
    memcpy((vendorId + 8), &(info[ECX]), 4);
    vendorId[12] = '\0';
    m_isCpuAmd = (0 == strcmp(vendorId, "AuthenticAMD"));

    if (info[EAX] > 0)
    {
        //If there are more cpuid functions (expected for modern cpus)

        cpuidFn = CPUID_FN_BASIC_FEATURES;
        __cpuid(info, cpuidFn);

        m_hasHypervisor = (0 != (info[ECX] & CPUID_MASK_HYPERVISOR));

        //Use a bitfield to easily separate the different values
        FamilyModelSteppingUnion famMod;
        famMod.eax = info[EAX];
        m_family = famMod.info.family;
        m_model = famMod.info.model;
        m_stepping = famMod.info.stepping;

        if (FAMILY_EXTENDED == m_family)
        {
            m_family += famMod.info.extFamily;
        }

        //From the Bkdg: If ExtendedModel[3:0]=Eh and BaseModel[3:0]=8h, then Model[7:0] = E8h
        m_model |= (famMod.info.extModel << 4);

        m_hasLocalApic = (0 != (info[EDX] & CPUID_MASK_APIC));

        if (m_hasLocalApic)
        {
            // Bits 31:24 (LocalApicId) represent the CPU core
            // within a processor and other bits represent the processor ID.
            m_apicId = (info[EBX] >> 24) & 0xff;
        }

        //Check the maximum extended cpuid functionality
        cpuidFn = CPUID_FN_MAX_EXT_FN;
        __cpuid(info, cpuidFn);
        maxExtCpuidFn = info[EAX];

        if ((m_hasLocalApic) && (maxExtCpuidFn >= CPUID_FN_SIZE_ID))
        {
            int apicIdCoreIdSize = 0;
            unsigned int mask = 1;
            unsigned int numbits = 1;

            cpuidFn = CPUID_FN_SIZE_ID;
            __cpuid(info, cpuidFn);

            // Bits 15:12 (ApicIdCoreIdSiz): The number of bits in the initial APIC20[ApicId]
            // value that indicate core ID within a processor
            apicIdCoreIdSize = (info[ECX] >> 12) & 0xF;

            if (apicIdCoreIdSize)
            {
                unsigned int j;
                numbits = apicIdCoreIdSize;

                for (j = apicIdCoreIdSize; j > 1; j--)
                {
                    mask = (mask << 1) + 1;
                }
            }

            m_core = (m_apicId & ~mask) >> numbits;
        }

        if ((m_hasLocalApic) && (maxExtCpuidFn >= CPUID_FN_NODE_ID))
        {
            cpuidFn = CPUID_FN_NODE_ID;
            __cpuid(info, cpuidFn);
            // Bits 7:0 (NodeId)
            m_node = info[ECX] & 0x0F;
        }
        else
        {
            m_node = m_core;
        }

        if (maxExtCpuidFn >= CPUID_FN_EXT_FEATURES)
        {
            //Read the cpuid extended feature bits
            cpuidFn = CPUID_FN_EXT_FEATURES;
            __cpuid(info, cpuidFn);

            //If IBS sampling is available
            m_isIbsAvailable = (0 != (info[ECX] & CPUID_MASK_IBS_AVAIL));

            if ((m_isIbsAvailable) && (maxExtCpuidFn >= CPUID_FN_IBS_FEATURES))
            {
                //Instruction Based Sampling Identifiers
                cpuidFn = CPUID_FN_IBS_FEATURES;
                __cpuid(info, cpuidFn);

                //If dispatch ops are available for IBS
                m_isIbsDispatchAvailable = (0 != (info[EAX] & CPUID_MASK_IBS_OP_CNT));

                // This bit (BrnTrgt: branch target address reporting supported) will be 1 for
                // GH and beyond
                m_isIbsBrTgtAddrAvailable = (0 != (info[EAX] & CPUID_MASK_IBS_BR_TGT));

                // This bit (OpCntExt: branch target address reporting supported) will be 1 for
                // Llano and beyond
                m_isIbsExtCountAvailable = (0 != (info[EAX] & CPUID_MASK_IBS_EXT_CNT));

                //If Fetch Control Extended available for IBS
                m_isIbsFetchCtlExtdAvailable = (0 != (info[EAX] & CPUID_MASK_IBS_FETCH_CTL_EXTD));

                //If Op Data 4 available for IBS
                m_isIbsOpData4Available = (0 != (info[EAX] & CPUID_MASK_IBS_OP_DATA4));
            }
        }

        // save hypervisor vendor id
        if (true == m_hasHypervisor)
        {
            gtByte vendor[12];

            cpuidFn = CPUID_FN_HYPERVISOR_ID;
            __cpuid(info, cpuidFn);
            memcpy(vendor, &(info[EBX]), 4);
            memcpy(vendor + 4, &(info[ECX]), 4);
            memcpy(vendor + 8, &(info[EDX]), 4);

            if (0 == memcmp(vendor, VENDOR_ID_VMWARE, 12))
            {
                m_hypervisorVendorId = HV_VENDOR_VMWARE;
            }
            else if (0 == memcmp(vendor, VENDOR_ID_MICROSOFT, 12))
            {
                m_hypervisorVendorId = HV_VENDOR_MICROSOFT;

                cpuidFn = CPUID_FN_HYPERVISOR_FEATURES;
                __cpuid(info, cpuidFn);
                m_isMshvRootPartition = (info[EBX] & 1) ? true : false;
            }
            else if (0 == memcmp(vendor, VENDOR_ID_XEN, 12))
            {
                m_hypervisorVendorId = HV_VENDOR_XENVMM;
            }
            else if (0 == memcmp(vendor, VENDOR_ID_KVM, 9))
            {
                m_hypervisorVendorId = HV_VENDOR_KVM;
            }
        }
    }
}

osCpuid::~osCpuid()
{
}

bool osCpuid::isCpuAmd()
{
    return m_isCpuAmd;
}
bool osCpuid::hasLocalApic()
{
    return m_hasLocalApic;
}
bool osCpuid::isIbsAvailable()
{
    return m_isIbsAvailable;
}
bool osCpuid::isIbsOpsDispatchAvailable()
{
    return m_isIbsDispatchAvailable;
}
bool osCpuid::isIbsOpsBrTgtAddrAvailable()
{
    return m_isIbsBrTgtAddrAvailable;
}
bool osCpuid::isIbsExtCountAvailable()
{
    return m_isIbsExtCountAvailable;
}
bool osCpuid::isIbsFetchCtlExtdAvailable()
{
    return m_isIbsFetchCtlExtdAvailable;
}
bool osCpuid::isIbsOpData4Available()
{
    return m_isIbsOpData4Available;
}
bool osCpuid::hasHypervisor()
{
    return m_hasHypervisor;
}
gtUInt32 osCpuid::getFamily()
{
    return m_family;
}
gtUInt32 osCpuid::getModel()
{
    return m_model;
}
gtUInt32 osCpuid::getStepping()
{
    return m_stepping;
}
gtUInt32 osCpuid::getApicId()
{
    return m_apicId;
}
gtUInt32 osCpuid::getcore()
{
    return m_core;
}
gtUInt32 osCpuid::getNodeId()
{
    return m_node;
}
bool osCpuid::isSupportedHypervisor()
{
    bool retVal = false;

    // In future, if we plan to support more hypervisors
    // then, we need to update the below if check
    if (HV_VENDOR_VMWARE == m_hypervisorVendorId
        || HV_VENDOR_MICROSOFT == m_hypervisorVendorId
        || HV_VENDOR_KVM == m_hypervisorVendorId
        || HV_VENDOR_XENVMM == m_hypervisorVendorId)
    {
        retVal = true;
    }

    return retVal;
}
gtUInt32 osCpuid::getHypervisorVendorId()
{
    return m_hypervisorVendorId;
}
bool osCpuid::getHypervisorVendorString(wchar_t* str, gtUInt32 length)
{
    bool retVal = true;

    // Expect a buffer of at least 32 bytes
    if (nullptr == str || length < 32)
    {
        retVal = false;
    }
    else
    {
        switch (m_hypervisorVendorId)
        {
            case HV_VENDOR_VMWARE:
                wcscpy(str, L"VMware");
                break;

            case HV_VENDOR_MICROSOFT:
                wcscpy(str, L"Microsoft Hyper-V");
                break;

            case HV_VENDOR_XENVMM:
                wcscpy(str, L"Xen VMM");
                break;

            case HV_VENDOR_KVM:
                wcscpy(str, L"Linux KVM");
                break;

            case HV_VENDOR_UNKNOWN:
                wcscpy(str, L"Unknown");
                break;

            default:
                wcscpy(str, L"???");
                retVal = false;
                break;
        }
    }

    return retVal;
}
bool osCpuid::isHypervisorRootPartition()
{
    return m_isMshvRootPartition;
}