//
// This file is included inline by file \CodeXL\Components\PowerProfiling\Backend\AMDTPowerProfileAPI\src\AMDTPowerProfileControl.cpp
//
// Node specifi counters
//
#if 0
// Disabled PID & TID
{
    /*attr_id*/COUNTERID_PID,
    /*len*/8,
    /*name*/ PP_STR_Counter_Process_Id_Prefix,
    /*description*/ "Process Id of the running process at the time when sampling was performed.",
    /*unittype*/(AMDTPwrAttributeUnitType)PWR_UNIT_TYPE_COUNT,
    /*category*/(PwrCategory)CATEGORY_NUMBER,
    /*instance type*/(AMDTPwrAttributeInstanceType)INSTANCE_TYPE_PER_CORE,
    /*device type*/(AMDTDeviceType)AMDT_PWR_DEVICE_CPU_CORE,
    /*Aggregation type*/(1 << AMDT_PWR_VALUE_SINGLE),
},
{
    /*attr_id*/COUNTERID_TID,
    /*len*/8,
    /*name*/ "Thread Id",
    /*description*/ "Thread Id of the running process at the time when sampling was performed.",
    /*unittype*/(AMDTPwrAttributeUnitType)PWR_UNIT_TYPE_COUNT,
    /*category*/(PwrCategory)CATEGORY_NUMBER,
    /*instance type*/(AMDTPwrAttributeInstanceType)INSTANCE_TYPE_PER_CORE,
    /*device type*/(AMDTDeviceType)AMDT_PWR_DEVICE_CPU_CORE,
    /*Aggregation type*/(1 << AMDT_PWR_VALUE_SINGLE),
},
#endif
{
    /*attr_id*/COUNTERID_CEF,
    /*len*/8,
    /*name*/ PP_STR_FrequencyCounterPostfix,
    /*description*/ "Average CPU Core Frequency for the sampling period, reported in MHz. This is the Core Effective Frequency (CEF). The core can go into various P-States within the sampling period, each with its own frequency. The CEF is the average of the core frequencies over the sampling period.",
    /*unittype*/(AMDTPwrAttributeUnitType)PWR_UNIT_TYPE_MEGA_HERTZ,
    /*category*/(PwrCategory)CATEGORY_FREQUENCY,
    /*instance type*/(AMDTPwrAttributeInstanceType)INSTANCE_TYPE_PER_CORE,
    /*device type*/(AMDTDeviceType)AMDT_PWR_DEVICE_CPU_CORE,
    /*Aggregation type*/(1 << AMDT_PWR_VALUE_SINGLE) | (1 << AMDT_PWR_VALUE_HISTOGRAM),
},
{
    /*attr_id*/COUNTERID_SOFTWARE_PSTATE,
    /*len*/8,
    /*name*/ "P-State(software)",
    /*description*/ "CPU Core P-State (software) at the time when sampling was performed.",
    /*unittype*/(AMDTPwrAttributeUnitType)PWR_UNIT_TYPE_COUNT,
    /*category*/(PwrCategory)CATEGORY_DVFS,
    /*instance type*/(AMDTPwrAttributeInstanceType)INSTANCE_TYPE_PER_CORE,
    /*device type*/(AMDTDeviceType)AMDT_PWR_DEVICE_CPU_CORE,
    /*Aggregation type*/(1 << AMDT_PWR_VALUE_SINGLE),
},
{
    /*attr_id*/COUNTERID_PKG_ENERGY,
    /*len*/8,
    /*name*/ "RAPL Package energy",
    /*description*/ "Package level Running Average Power Limit energy reporting in milli joules, shared with all the cores in package.",
    /*unittype*/(AMDTPwrAttributeUnitType)PWR_UNIT_TYPE_JOULE,
    /*category*/(PwrCategory)CATEGORY_ENERGY,
    /*instance type*/(AMDTPwrAttributeInstanceType)INSTANCE_TYPE_NONCORE_SINGLE,
    /*device type*/(AMDTDeviceType)AMDT_PWR_DEVICE_PACKAGE,
    /*Aggregation type*/(1 << AMDT_PWR_VALUE_SINGLE) | (1 << AMDT_PWR_VALUE_CUMULATIVE),
},
{
    /*attr_id*/COUNTERID_CORE_ENERGY,
    /*len*/8,
    /*name*/ "RAPL Core Energy",
    /*description*/ "Core-level Running Average Power Limit energy reporting in milli joules.",
    /*unittype*/(AMDTPwrAttributeUnitType)PWR_UNIT_TYPE_JOULE,
    /*category*/(PwrCategory)CATEGORY_ENERGY,
    /*instance type*/(AMDTPwrAttributeInstanceType)INSTANCE_TYPE_PER_PHYSICAL_CORE,
    /*device type*/(AMDTDeviceType)AMDT_PWR_DEVICE_CPU_CORE,
    /*Aggregation type*/(1 << AMDT_PWR_VALUE_SINGLE) | (1 << AMDT_PWR_VALUE_CUMULATIVE),
},

