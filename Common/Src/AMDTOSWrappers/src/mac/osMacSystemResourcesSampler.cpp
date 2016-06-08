//------------------------------ osMacSystemResourcesSampler.cpp ------------------------------

// MAC OS:
#include <mach/mach.h>
#include <mach/mach_host.h>

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>

// Local:
#include <AMDTOSWrappers/Include/osCPUSampledData.h>
#include <AMDTOSWrappers/Include/osDebugLog.h>
#include <AMDTOSWrappers/Include/osMacSystemResourcesSampler.h>
#include <AMDTOSWrappers/Include/osPhysicalMemorySampledData.h>


// ---------------------------------------------------------------------------
// Name:        osMacSystemResourcesSampler::osMacSystemResourcesSampler
// Description: Constructor
// Author:      Uri Shomroni
// Date:        22/11/2009
// ---------------------------------------------------------------------------
osMacSystemResourcesSampler::osMacSystemResourcesSampler()
    : osSystemResourcesDataSampler(), _amountOfCPUs(0)
{
    // Get the host:
    host_name_port_t host = mach_host_self();

    kern_return_t kRetVal;
    host_basic_info_data_t hostInfoData;
    mach_msg_type_number_t count = HOST_BASIC_INFO_COUNT;

    // Get the basic host info:
    kRetVal = host_info(host, HOST_BASIC_INFO, (host_info_t)&hostInfoData, &count);

    if (kRetVal != KERN_SUCCESS)
    {
        gtString dbgMsg;
        dbgMsg.appendFormattedString(L"host_info() call failed: %d", kRetVal);
        OS_OUTPUT_DEBUG_LOG(dbgMsg.asCharArray(), OS_DEBUG_LOG_ERROR);
    }
    else
    {
        // Update amount of host CPUs:
        _amountOfCPUs = hostInfoData.avail_cpus;
    }

    // Set zero-valued structs to each slot:
    osCPUSampledData zeroData;

    for (int i = 0; i < _amountOfCPUs; i++)
    {
        _cpusSampledData.push_back(zeroData);
    }
}

// ---------------------------------------------------------------------------
// Name:        osMacSystemResourcesSampler::~osMacSystemResourcesSampler
// Description: Destructor
// Author:      Uri Shomroni
// Date:        22/11/2009
// ---------------------------------------------------------------------------
osMacSystemResourcesSampler::~osMacSystemResourcesSampler()
{

}

// ---------------------------------------------------------------------------
// Name:        osMacSystemResourcesSampler::updateCPUsData
// Description: Updates CPUs related data.
// Return Val: bool  - Success / failure.
// Author:      Uri Shomroni
// Date:        22/11/2009
// ---------------------------------------------------------------------------
bool osMacSystemResourcesSampler::updateCPUsData()
{
    bool retVal = false;

    GT_IF_WITH_ASSERT(_amountOfCPUs > 0)
    {
        // Get the host:
        host_name_port_t host = mach_host_self();

        // Get the processors info (mach function that updates the processor values):
        processor_cpu_load_info_data_t* procCPULoadInfo;
        mach_msg_type_number_t amountOfLoadInfo;
        unsigned int amountOfCPUs = 0;
        kern_return_t kRetVal = host_processor_info(host, PROCESSOR_CPU_LOAD_INFO, &amountOfCPUs, (processor_info_array_t*)&procCPULoadInfo, &amountOfLoadInfo);
        GT_IF_WITH_ASSERT(kRetVal == KERN_SUCCESS)
        {
            retVal = true;

            if (amountOfCPUs != (unsigned int)_amountOfCPUs)
            {
                // We don't want to write outside our vector:
                GT_ASSERT(amountOfCPUs == (unsigned int)_amountOfCPUs);
                amountOfCPUs = min(amountOfCPUs, (unsigned int)_amountOfCPUs);
            }

            // Sample each of the CPUs:
            for (unsigned int currCPU = 0; currCPU < amountOfCPUs; currCPU++)
            {
                // Get the utilization numbers from the MAC OS API and set them into _cpusSampledData:
                _cpusSampledData[currCPU]._userClockTicks = procCPULoadInfo[currCPU].cpu_ticks[CPU_STATE_USER];
                _cpusSampledData[currCPU]._niceClockTicks = procCPULoadInfo[currCPU].cpu_ticks[CPU_STATE_NICE];
                _cpusSampledData[currCPU]._sysClockTicks = procCPULoadInfo[currCPU].cpu_ticks[CPU_STATE_SYSTEM];
                _cpusSampledData[currCPU]._idleClockTicks = procCPULoadInfo[currCPU].cpu_ticks[CPU_STATE_IDLE];
                _cpusSampledData[currCPU]._totalClockTicks = _cpusSampledData[currCPU]._userClockTicks + _cpusSampledData[currCPU]._niceClockTicks +
                                                             _cpusSampledData[currCPU]._sysClockTicks + _cpusSampledData[currCPU]._idleClockTicks;
            }

            // Release memory:
            vm_deallocate(mach_task_self(), (vm_address_t) procCPULoadInfo, (vm_size_t)(amountOfLoadInfo * sizeof(*procCPULoadInfo)));
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        osMacSystemResourcesSampler::cpusAmount
// Description: Retrieves the amount of host CPUs.
// Author:      Uri Shomroni
// Date:        22/11/2009
// ---------------------------------------------------------------------------
int osMacSystemResourcesSampler::cpusAmount() const
{
    int retVal = _amountOfCPUs;

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        osMacSystemResourcesSampler::getGlobalCPUData
// Description:
//   Retrieves the global CPU statistical data. This data reflects the
//   activities of all system's CPUs.
// Return Val: bool  - Success / failure.
// Author:      Uri Shomroni
// Date:        22/11/2009
// ---------------------------------------------------------------------------
bool osMacSystemResourcesSampler::getGlobalCPUData(osCPUSampledData& cpuStatistics) const
{
    // Currently not implemented:
    return false;
}

// ---------------------------------------------------------------------------
// Name:        osMacSystemResourcesSampler::getCPUData
// Description: Returns the statistical data of a single given CPU.
// Return Val: bool  - Success / failure.
// Author:      Uri Shomroni
// Date:        22/11/2009
// ---------------------------------------------------------------------------
bool osMacSystemResourcesSampler::getCPUData(int cpuIndex, osCPUSampledData& cpuStatistics) const
{
    bool retVal = false;

    // Sanity check:
    GT_IF_WITH_ASSERT((cpuIndex < _amountOfCPUs) && (cpuIndex >= 0))
    {
        // Copy the data:
        cpuStatistics = _cpusSampledData[cpuIndex];

        retVal = true;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        osMacSystemResourcesSampler::updatePhysicalMemoryData
// Description: Updates physical memory related data.
// Return Val: bool  - Success / failure.
// Author:      Uri Shomroni
// Date:        22/11/2009
// ---------------------------------------------------------------------------
bool osMacSystemResourcesSampler::updatePhysicalMemoryData()
{
    // All the reading is done at sample time, so just return true:
    return true;
}

// ---------------------------------------------------------------------------
// Name:        osMacSystemResourcesSampler::getPhysicalMemoryData
// Description: Returns the physical memory statistical data.
// Return Val: bool  - Success / failure.
// Author:      Uri Shomroni
// Date:        22/11/2009
// ---------------------------------------------------------------------------
bool osMacSystemResourcesSampler::getPhysicalMemoryData(osPhysicalMemorySampledData& memoryStatistics) const
{
    bool retVal = false;

    // Get the host:
    host_t myHost = mach_host_self();

    kern_return_t kretVal;
    host_basic_info_data_t hostInfoData;
    mach_msg_type_number_t count;
    count = HOST_BASIC_INFO_COUNT;

    // Get the basic host info:
    kretVal = host_info(myHost, HOST_BASIC_INFO, (host_info_t)&hostInfoData, &count);

    if (kretVal != KERN_SUCCESS)
    {
        gtString dbgMsg;
        dbgMsg.appendFormattedString(L"host_info() call failed: %d", kretVal);
        OS_OUTPUT_DEBUG_LOG(dbgMsg.asCharArray(), OS_DEBUG_LOG_ERROR);
        retVal = false;
    }
    else
    {
        memoryStatistics._totalPhysicalMemory = hostInfoData.memory_size;
        retVal = true;
    }

    vm_statistics_data_t vmStatisticsData;
    mach_msg_type_number_t hostCount;
    vm_size_t hostPageSize;

    hostCount = sizeof(vmStatisticsData) / sizeof(integer_t);
    kretVal = host_statistics(myHost, HOST_VM_INFO , (host_info_t)&vmStatisticsData, &hostCount);
    GT_IF_WITH_ASSERT(kretVal == KERN_SUCCESS)
    {
        // Get page size for this machine:
        host_page_size(myHost, &hostPageSize);

        // Update the sampled memory structure with the new memory values:
        memoryStatistics._freeSwapMemory = ((unsigned long long) vmStatisticsData.free_count) * hostPageSize;
        memoryStatistics._wiredPages = ((unsigned long long) vmStatisticsData.wire_count) * hostPageSize;
        memoryStatistics._activePages = ((unsigned long long) vmStatisticsData.active_count) * hostPageSize;
        memoryStatistics._inactivePages = ((unsigned long long) vmStatisticsData.inactive_count) * hostPageSize;
        memoryStatistics._pageIns = vmStatisticsData.pageins * hostPageSize;
        memoryStatistics._pageOuts = vmStatisticsData.pageouts * hostPageSize;
    }

    return retVal;
}

