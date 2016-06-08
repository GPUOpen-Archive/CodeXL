//------------------------------ osMachine.cpp ------------------------------

// Mac OS X
#include <sys/types.h>
#include <sys/sysctl.h>
#include <mach/mach.h>
#include <mach/machine.h>
#include <mach/mach_host.h>
#ifndef _GR_IPHONE_BUILD
    #include <ApplicationServices/ApplicationServices.h> // For Quartz display services + ColorSync
#endif

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTBaseTools/Include/AMDTDefinitions.h>

// Local:
#include <AMDTOSWrappers/Include/osFilePath.h>
#include <AMDTOSWrappers/Include/osMachine.h>
#include <AMDTOSWrappers/Include/osProcess.h>
#include <AMDTOSWrappers/Include/osStringConstants.h>

// For systems that don't define HOST_NAME_MAX:
#ifndef HOST_NAME_MAX
    #define HOST_NAME_MAX 255
#endif
//////////////////////////////////////////////////////////////////////////
// Most of these functions are based on the sysctl function. See its spec:
// http://developer.apple.com/DOCUMENTATION/Darwin/Reference/ManPages/man3/sysctl.3.html


// ---------------------------------------------------------------------------
// Name:        osGetLocalMachineName
// Description: Retrieves the name of the local machine (computer)
// Arguments:   localMachineName - Will get the local machine name.
// Return Val:  bool - Success / failure.
// Author:      Uri Shomroni
// Date:        2/12/2008
// ---------------------------------------------------------------------------
bool osGetLocalMachineName(gtASCIIString& localMachineName)
{
    bool retVal = false;

    localMachineName.makeEmpty();

    int mib[2] = {CTL_KERN, KERN_HOSTNAME};

    // Get the string length:
    gtSize_t hostnameLength = -1;
    int resultCode = sysctl(mib, 2, NULL, &hostnameLength, NULL, 0);
    GT_IF_WITH_ASSERT((resultCode == 0) && (hostnameLength > 0))
    {
        char* hostnameString = new char[hostnameLength];
        resultCode = sysctl(mib, 2, hostnameString, &hostnameLength, NULL, 0);
        GT_IF_WITH_ASSERT(resultCode == 0)
        {
            localMachineName = hostnameString;
            retVal = true;
        }
        delete[] hostnameString;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        osGetLocalMachineName
// Description: Retrieves the name of the local machine (computer)
// Arguments:   localMachineName - Will get the local machine name.
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        13/9/2010
// ---------------------------------------------------------------------------
bool osGetLocalMachineName(gtString& localMachineName)
{
    // Get the machine name in ASCII string:
    gtASCIIString ansiiMachineName;
    bool retVal = osGetLocalMachineName(ansiiMachineName);
    // Convert to Unicode:
    localMachineName.fromASCIIString(ansiiMachineName.asCharArray());
    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        osGetAmountOfLocalMachineCPUs
// Description: Retrieves the amount of local machine CPUs.
// Arguments:   amounfOfCPUs - Will get the amount of local machine CPUs.
// Return Val:  bool - Success / failure.
// Author:      Uri Shomroni
// Date:        2/12/2008
// ---------------------------------------------------------------------------
bool osGetAmountOfLocalMachineCPUs(int& amountOfCPUs)
{
    bool retVal = false;

    amountOfCPUs = 0;

    int mib[2] = {CTL_HW, HW_NCPU};

    gtSize_t amountofCPUsSize = sizeof(int);
    int interimAmountOfCPUs = 0;
    int resultCode = sysctl(mib, 2, &interimAmountOfCPUs, &amountofCPUsSize, NULL, 0);
    GT_IF_WITH_ASSERT((resultCode == 0) && (interimAmountOfCPUs > 0))
    {
        amountOfCPUs = interimAmountOfCPUs;
        retVal = true;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        osGetLocalMachineMemoryLimits
// Description: Retrieves information about the local machine memory limits.
// Arguments:   physicalMemAmount - The amount of physical memory, measured in bytes.
//              virtualMemAmount - The amount of virtual memory, measured in bytes.
// Return Val:  bool - Success / failure.
// Author:      Uri Shomroni
// Date:        2/12/2008
// ---------------------------------------------------------------------------
bool osGetLocalMachineMemoryLimits(unsigned long& physicalMemAmount, unsigned long& virtualMemAmount)
{
    bool rcPhys = false;
    bool rcVirt = false;

    // Get the Physical Memory:
    int mibPhys[2] = {CTL_HW, HW_PHYSMEM};

    gtSize_t physMemAmountSize = sizeof(unsigned int);
    unsigned int interimPhysMemAmount = 0;
    int resultCode = sysctl(mibPhys, 2, &interimPhysMemAmount, &physMemAmountSize, NULL, 0);
    GT_IF_WITH_ASSERT((resultCode == 0) && (interimPhysMemAmount > 0))
    {
        physicalMemAmount = interimPhysMemAmount;
        rcPhys = true;
    }

    // Get virtual memory size:
    // See comments in osGetLocalMachineVirtualMemPageSize for details
    vm_size_t vmPageSize = 4096; // 4Kb is the normal size, if we fail to get the size, we'll assume that's it
    kern_return_t kernErrCode = host_page_size(mach_host_self(), &vmPageSize);
    GT_ASSERT(kernErrCode == KERN_SUCCESS);

    vm_statistics_data_t vmStats;
    unsigned int vmStatsCount = HOST_VM_INFO_COUNT;
    kernErrCode = host_statistics(mach_host_self(), HOST_VM_INFO, (host_info_t)(&vmStats), &vmStatsCount);
    GT_IF_WITH_ASSERT(kernErrCode == KERN_SUCCESS)
    {
        virtualMemAmount = (vmStats.free_count + vmStats.active_count + vmStats.inactive_count + vmStats.wire_count) * vmPageSize;
        rcVirt = true;
    }

    bool retVal = rcPhys && rcVirt;
    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        osGetLocalMachineVirtualMemPageSize
// Description: Retrieves the local machine virtual memory page size.
// Arguments:   pageSize - Will get the page size, measured in bytes.
// Return Val:  bool - Success / failure.
// Author:      Uri Shomroni
// Date:        2/12/2008
// ---------------------------------------------------------------------------
bool osGetLocalMachineVirtualMemPageSize(unsigned long& pageSize)
{
    bool retVal = false;

    int mib[2] = {CTL_VM, VM_SWAPUSAGE};
    gtSize_t memoryStatsSize = sizeof(xsw_usage);
    xsw_usage memoryStats;
    int resultCode = sysctl(mib, 2, &memoryStats, &memoryStatsSize, NULL, 0);
    GT_IF_WITH_ASSERT(resultCode == 0)
    {
        pageSize = memoryStats.xsu_pagesize;
        retVal = true;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        osGetLocalMachineMemoryInformationStrings
// Description: OGets the system memory information as a string, e.g. "512 MB"
// Arguments: totalRam - will contain total amount of RAM
//            availRam - will contain amount of available RAM
//            totalPage - will contain total amount of Page memory
//            availPage - will contain amount of available Page memory
//            totalVirtual - will contain total amount of Virtual memory
//            availVirtual - will contain amount of available Virtual memory
// Return Val: bool  - Success / failure.
// Author:      Uri Shomroni
// Date:        2/12/2008
// ---------------------------------------------------------------------------
bool osGetLocalMachineMemoryInformationStrings(gtString& totalRam, gtString& availRam, gtString& totalPage, gtString& availPage, gtString& totalVirtual, gtString& availVirtual)
{
    // Get the local machine memory info:
    gtUInt64 totalRAMAmount = 0;
    gtUInt64 availRAMAmount = 0;
    gtUInt64 totalPageAmount = 0;
    gtUInt64 availPageAmount = 0;
    gtUInt64 totalVirtualAmount = 0;
    gtUInt64 availVirtualAmount = 0;

    // Get the host:
    bool rcTotalRam = false;
    host_t myHost = mach_host_self();
    kern_return_t kretVal;
    host_basic_info_data_t hostInfoData;
    mach_msg_type_number_t count;
    count = HOST_BASIC_INFO_COUNT;

    // Get the basic host info:
    kretVal = host_info(myHost, HOST_BASIC_INFO, (host_info_t)&hostInfoData, &count);
    GT_IF_WITH_ASSERT(kretVal == KERN_SUCCESS)
    {
        totalRAMAmount = hostInfoData.memory_size;
        availRAMAmount = hostInfoData.max_mem;
        rcTotalRam = true;
    }

    // Get the virtual memory stats from mach:
    // http://developer.apple.com/DOCUMENTATION/Performance/Conceptual/ManagingMemory/Articles/VMPages.html
    // http://www.opensource.apple.com/darwinsource/projects/other/system_cmds-433/vm_stat.tproj/vm_stat.c

    vm_size_t vmPageSize = 4096; // 4Kb is the normal size, if we fail to get the size, we'll assume that's it
    kern_return_t kernErrCode = host_page_size(mach_host_self(), &vmPageSize);
    GT_ASSERT(kernErrCode == KERN_SUCCESS);

    vm_statistics_data_t vmStats;
    unsigned int vmStatsCount = HOST_VM_INFO_COUNT;
    kernErrCode = host_statistics(mach_host_self(), HOST_VM_INFO, (host_info_t)(&vmStats), &vmStatsCount);
    GT_IF_WITH_ASSERT(kernErrCode == KERN_SUCCESS)
    {
        // The statistics we get are in pages, so we need to multiply by the page size:
        totalVirtualAmount = (vmStats.free_count + vmStats.active_count + vmStats.inactive_count + vmStats.wire_count) * vmPageSize;
        availVirtualAmount = vmStats.free_count * vmPageSize;
    }

    bool rcPageMem = false;
    int mibPage[2] = {CTL_VM, VM_SWAPUSAGE};
    gtSize_t pageMemoryStatsSize = sizeof(xsw_usage);
    xsw_usage pagememoryStats;
    int resultCode = sysctl(mibPage, 2, &pagememoryStats, &pageMemoryStatsSize, NULL, 0);
    GT_IF_WITH_ASSERT(resultCode == 0)
    {
        totalPageAmount = pagememoryStats.xsu_total;
        availPageAmount = pagememoryStats.xsu_avail;
        rcPageMem = true;
    }

    // Get the Total RAM
    if (totalRAMAmount != 0)
    {
        totalRam.makeEmpty();
        totalRam.appendFormattedString(L"%u MB", (totalRAMAmount / 1024 / 1024));
    }
    else
    {
        totalRam = OS_STR_NotAvailable;
    }

    // Get the Available RAM
    if (availRAMAmount != 0)
    {
        availRam.makeEmpty();
        availRam.appendFormattedString(L"%u MB", availRAMAmount / 1024 / 1024);
    }
    else
    {
        availRam = OS_STR_NotAvailable;
    }

    // Get the Total Swap Memory
    if (totalPageAmount != 0)
    {
        totalPage.makeEmpty();
        totalPage.appendFormattedString(L"%u MB", totalPageAmount / 1024 / 1024);
    }
    else
    {
        totalPage = OS_STR_NotAvailable;
    }

    // Get the Available Swap Memory
    if (availPageAmount != 0)
    {
        availPage.makeEmpty();
        availPage.appendFormattedString(L"%u MB", availPageAmount / 1024 / 1024);
    }
    else
    {
        availPage = OS_STR_NotAvailable;
    }

    // Get the Total Virtual Memory
    if (totalVirtualAmount != 0)
    {
        totalVirtual.makeEmpty();
        totalVirtual.appendFormattedString(L"%u MB", totalVirtualAmount / 1024 / 1024);
    }
    else
    {
        totalVirtual = OS_STR_NotAvailable;
    }

    // Get the Available Virtual Memory
    if (availVirtualAmount != 0)
    {
        availVirtual.makeEmpty();
        availVirtual.appendFormattedString(L"%u MB", availVirtualAmount / 1024 / 1024);
    }
    else
    {
        availVirtual = OS_STR_NotAvailable;
    }

    bool retVal = rcPageMem && rcTotalRam;
    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        osCPUTypeToString
// Description: Translate a MAC OS cpu type structure to string
// Arguments: cpu_type_t cpuType
// Return Val: gtString
// Author:      Sigal Algranaty
// Date:        24/12/2008
// ---------------------------------------------------------------------------
gtString osCPUTypeToString(cpu_type_t cpuType)
{
    gtString retVal;

    switch (cpuType)
    {
        case CPU_TYPE_ANY:
        {
            retVal = "ANY";
            break;
        }

        case CPU_TYPE_VAX:
        {
            retVal = "VAX";
            break;
        }

        case CPU_TYPE_MC680x0:
        {
            retVal = "MC680x0";
            break;
        }

        case CPU_TYPE_I386: // (same as CPU_TYPE_X86)
        {
            retVal = "I386";
            break;
        }

        case CPU_TYPE_X86_64:
        {
            retVal = "X86_64";
            break;
        }

        case CPU_TYPE_MC98000:
        {
            retVal = "MC98000";
            break;
        }

        case CPU_TYPE_HPPA:
        {
            retVal = "HPPA";
            break;
        }

        case CPU_TYPE_ARM:
        {
            retVal = "ARM";
            break;
        }

        case CPU_TYPE_MC88000:
        {
            retVal = "MC88000";
            break;
        }

        case CPU_TYPE_SPARC:
        {
            retVal = "SPARC";
            break;
        }

        case CPU_TYPE_I860:
        {
            retVal = "I860";
            break;
        }

        case CPU_TYPE_POWERPC:
        {
            retVal = "POWERPC";
            break;
        }

        case CPU_TYPE_POWERPC64:
        {
            retVal = "POWERPC64";
            break;
        }

        default:
        {
            retVal = OS_STR_NotAvailable;
            break;
        }
    }

    return retVal;

}

// ---------------------------------------------------------------------------
// Name:        osGetLocalMachineCPUInformationStrings
// Description: Gets the number of CPUs and their type (e.g. 586) and returns that
//              Information as Strings.
// Arguments: numberOfProcessors - will contain number of processors
//            processorType - will contain types of processors
// Return Val: bool  - Success / failure.
// Author:      Uri Shomroni
// Date:        2/12/2008
// ---------------------------------------------------------------------------
bool osGetLocalMachineCPUInformationStrings(gtString& numberOfProcessors, gtString& processorType)
{
    int numOfCPUsAsInt = -1;
    bool rcAmount = osGetAmountOfLocalMachineCPUs(numOfCPUsAsInt);

    if (rcAmount && (numOfCPUsAsInt > 0))
    {
        numberOfProcessors.makeEmpty();
        numberOfProcessors.appendFormattedString(L"%d", numOfCPUsAsInt);
    }
    else
    {
        numberOfProcessors = OS_STR_NotAvailable;
    }

    bool rcType = false;
    {
        // Get the host:
        host_t myHost = mach_host_self();

        // Get processor type:
        processor_basic_info* procCPUBasicInfo;
        kern_return_t kRetVal;
        mach_msg_type_number_t amountOfBasicInfo;
        unsigned int amountOfCPUs = 0;
        kRetVal = host_processor_info(myHost, PROCESSOR_BASIC_INFO , &amountOfCPUs, (processor_info_array_t*)&procCPUBasicInfo, &amountOfBasicInfo);
        GT_IF_WITH_ASSERT(kRetVal == KERN_SUCCESS)
        {
            // Get the processor type string:
            gtString cpuTypeStr = osCPUTypeToString(procCPUBasicInfo[0].cpu_type);
            processorType.append(cpuTypeStr);

            vm_deallocate(mach_task_self(), (vm_address_t) procCPUBasicInfo, (vm_size_t)(amountOfBasicInfo * sizeof(*procCPUBasicInfo)));

        }
        rcType = true;
    }

    bool retVal = rcAmount && rcType;

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        osGetLocalMachineUserAndDomain
// Description: Gets the current local machine username and domain
// Arguments: userName - user name goes here
//            userDomain - domain goes here
// Return Val: bool  - Success / failure.
// Author:      Uri Shomroni
// Date:        2/12/2008
// ---------------------------------------------------------------------------
bool osGetLocalMachineUserAndDomain(gtString& userName, gtString& userDomain)
{
    userName.makeEmpty();
    userDomain.makeEmpty();
    bool rcUser = osGetCurrentProcessEnvVariableValue("USER", userName);
    bool rcDomain = osGetCurrentProcessEnvVariableValue("USERDOMAIN", userDomain);

    if (userDomain.isEmpty())
    {
        rcDomain = false;

        // This is equivalent to using the getdomainname() function, but it seems that
        // said function actually performs this operation with sysctl, so we do it ourselves:
        int mib[2] = {CTL_KERN, KERN_NISDOMAINNAME};

        // Get the string length:
        gtSize_t domainNameLength = -1;
        int resultCode = sysctl(mib, 2, NULL, &domainNameLength, NULL, 0);
        GT_IF_WITH_ASSERT((resultCode == 0) && (domainNameLength > 0))
        {
            char* domainNameString = new char[domainNameLength];
            resultCode = sysctl(mib, 2, domainNameString, &domainNameLength, NULL, 0);
            GT_IF_WITH_ASSERT(resultCode == 0)
            {
                userDomain = domainNameString;
                rcDomain = true;
            }
            delete[] domainNameString;
        }

        GT_ASSERT(rcDomain);

        if ((!rcDomain) || (userDomain.isEmpty()))
        {
            userDomain = OS_STR_NotAvailable;
        }
    }

    bool retVal = rcUser && rcDomain;

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        osGetLocalMachineSystemPathAndDelims
// Description:
// Arguments: computerPath - will contain the list of system paths, separated by computerPathDelims
//            computerPathDelims - will contain the path list delimiters, in this case
//              only the semicolon.
// Return Val: bool  - Success / failure.
// Author:      Uri Shomroni
// Date:        2/12/2008
// ---------------------------------------------------------------------------
bool osGetLocalMachineSystemPathAndDelims(gtString& computerPath, gtString& computerPathDelims)
{
    // Uri, 2/12/08: This can also be done with sysctl and mib = {CTL_USER, USER_CS_PATH}
    bool retVal = osGetCurrentProcessEnvVariableValue("PATH", computerPath);
    computerPathDelims = osFilePath::osEnvironmentVariablePathsSeparator;
    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        osGetLocalMachineAmountOfMonitors
// Description: returns the number of monitors on the current machine
// Return Val: int
// Author:      Uri Shomroni
// Date:        2/12/2008
// ---------------------------------------------------------------------------
int osGetLocalMachineAmountOfMonitors()
{
    int retVal = 1;

#ifdef _GR_IPHONE_BUILD
    {
        // On iPhone, we assume only 1 monitor :-)
        retVal = 1;
    }
#else
    {
        // Mac OS X desktop:
        CGDisplayCount numberOfDisplays = 0;
        static const CGDisplayCount maxDisplays = ~((CGDisplayCount)0);
        CGDisplayErr errCode = CGGetOnlineDisplayList(maxDisplays, NULL, &numberOfDisplays);
        GT_IF_WITH_ASSERT((errCode == kCGErrorSuccess) && (numberOfDisplays > 0))
        {
            retVal = (int)numberOfDisplays;
        }
    }
#endif

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        osGetDisplayMonitorInfo
// Description: Get the Primary display device name
// Arguments:   deviceName - the device name goes here
//              monitorName - the monitor name goes here
// Return Val:  bool - Success / failure.
// Author:      Uri Shomroni
// Date:        2/12/2008
// ---------------------------------------------------------------------------
bool osGetDisplayMonitorInfo(gtString& deviceName, gtString& monitorName)
{
    bool retVal = false;
    deviceName.makeEmpty();
    monitorName.makeEmpty();

    // Uri, 9/3/09 - If we want actual monitor names, we need to find a table with information
    // like this http://discussions.apple.com/thread.jspa?threadID=1460397. for now, we just print out
    // the same info as in System Preferences > Display > color > Profile > mmod (item 13).
    // (see http://developer.apple.com/documentation/GraphicsImaging/Reference/ColorSync_Manager/Reference/reference.html)

    // This commented out code is supposed to do the same with Quartz, but it gives out different results:
    // http://developer.apple.com/documentation/GraphicsImaging/Reference/Quartz_Services_Ref/Reference/reference.html
    /*  CGDirectDisplayID hMainDisplay = CGMainDisplayID();
        uint32_t unitNumber = CGDisplayUnitNumber(hMainDisplay);
        uint32_t manufacturerCode = CGDisplayVendorNumber(hMainDisplay);
        uint32_t screenModelCode = CGDisplayModelNumber(hMainDisplay);
        uint32_t serialNumber = CGDisplaySerialNumber(hMainDisplay);
        monitorName.appendFormattedString(L"Unit: %08X Manufacturer: %08X Model: %08X Serial Number: %08X");            */

#ifdef _GR_IPHONE_BUILD
    {
        // Yaki 31/5/2009: Since we don't know how to get this information on the iPhone, we use our own constants:
        deviceName = "iPhone";
        monitorName = "iPhone monitor";
        retVal = true;
    }
#else
    {
        // Mac OS X desktop:

        CMProfileRef sysProfile;
        CMError errCode = CMGetSystemProfile(&sysProfile);
        GT_IF_WITH_ASSERT(errCode == noErr)
        {
            Boolean hasMakeAndModelTag = false;
            errCode = CMProfileElementExists(sysProfile, cmMakeAndModelTag, &hasMakeAndModelTag);
            GT_IF_WITH_ASSERT(errCode == noErr)
            {
                if (hasMakeAndModelTag)
                {
                    CMMakeAndModelType* pMakeAndModelInfo = new CMMakeAndModelType;
                    errCode = CMGetProfileElement(sysProfile, cmMakeAndModelTag, NULL, (void*)pMakeAndModelInfo);
                    GT_IF_WITH_ASSERT((errCode == noErr) && (pMakeAndModelInfo != NULL))
                    {
                        OSType manufacturer = pMakeAndModelInfo->makeAndModel.manufacturer;
                        UInt32 model = pMakeAndModelInfo->makeAndModel.model;
                        UInt32 serialNumber = pMakeAndModelInfo->makeAndModel.serialNumber;
                        UInt32 manufactureDate = pMakeAndModelInfo->makeAndModel.manufactureDate;

                        // Uri, 9/3/09: We need to somehow find the profile's MAGIC number and compare it to cmMagicNumber.
                        // it seems that on Intel machines this is in fact CIGAM, so we do need to swap here
                        bool shouldSwap = true;

                        if (shouldSwap)
                        {
                            manufacturer = OSSwapInt32(manufacturer);
                            model = OSSwapInt32(model);
                            serialNumber = OSSwapInt32(serialNumber);
                            manufactureDate = OSSwapInt32(manufactureDate);
                        }

                        monitorName.appendFormattedString(L"Manufacturer: %08X Model: %08X Serial Number: %08X Manufacture Date: %08X",
                                                          manufacturer, model, serialNumber, manufactureDate);
                        retVal = true;
                    }
                }
            }
        }
    }
#endif

    return retVal;
}

