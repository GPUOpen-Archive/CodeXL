//
// This file is included inline by file \CodeXL\Components\PowerProfiling\Backend\AMDTPowerProfileAPI\src\AMDTPowerProfileControl.cpp
//
// Node specifi counters
//
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
#ifdef FAMILY15H16H_INTERNAL_COUNTERS
{
    /*attr_id*/COUNTERID_CSTATE_RES,
    /*len*/8,
    /*name*/ "C-State",
    /*description*/ "CPU Core C-State Residency.",
    /*unittype*/(AMDTPwrAttributeUnitType)PWR_UNIT_TYPE_COUNT,
    /*category*/(PwrCategory)CATEGORY_DVFS,
    /*instance type*/(AMDTPwrAttributeInstanceType)INSTANCE_TYPE_PER_CORE,
},
#endif
{
    /*attr_id*/COUNTERID_PSTATE,
    /*len*/8,
    /*name*/ "P-State",
    /*description*/ "CPU Core P-State at the time when sampling was performed.",
    /*unittype*/(AMDTPwrAttributeUnitType)PWR_UNIT_TYPE_COUNT,
    /*category*/(PwrCategory)CATEGORY_DVFS,
    /*instance type*/(AMDTPwrAttributeInstanceType)INSTANCE_TYPE_PER_CORE,
    /*device type*/(AMDTDeviceType)AMDT_PWR_DEVICE_CPU_CORE,
    /*Aggregation type*/(1 << AMDT_PWR_VALUE_SINGLE),
},
#ifdef FAMILY15H16H_INTERNAL_COUNTERS
// Removing node temperature (attr_id: COUNTERID_NODE_TCTL_TEPERATURE).
// Reading MSR gives wrong temperature value. SMU provided temperature
// counters are supported for CU's.
{
    /*attr_id*/COUNTERID_SVI2_CORE_TELEMETRY,
    /*len*/8,
    /*name*/ PP_STR_Counter_Voltage_CPUCore,
    /*description*/ "SVI2 telemetry values for Core - Voltage(V) and Current(mA).",
    /*unittype*/(AMDTPwrAttributeUnitType)PWR_UNIT_TYPE_VOLT,
    /*category*/(PwrCategory)CATEGORY_VOLTAGE,
    /*instance type*/(AMDTPwrAttributeInstanceType)INSTANCE_TYPE_NONCORE_MULTIVALUE,
    /*device type*/(AMDTDeviceType)AMDT_PWR_DEVICE_SVI2,
    /*Aggregation type*/(1 << AMDT_PWR_VALUE_SINGLE),
},
{
    /*attr_id*/COUNTERID_SVI2_NB_TELEMETRY,
    /*len*/8,
    /*name*/ PP_STR_Counter_Voltage_NB,
    /*description*/ "SVI2 telemetry values for North-bridge - Voltage(V) and Current(mA).",
    /*unittype*/(AMDTPwrAttributeUnitType)PWR_UNIT_TYPE_VOLT,
    /*category*/(PwrCategory)CATEGORY_VOLTAGE,
    /*instance type*/(AMDTPwrAttributeInstanceType)INSTANCE_TYPE_NONCORE_MULTIVALUE,
    /*device type*/(AMDTDeviceType)AMDT_PWR_DEVICE_SVI2,
    /*Aggregation type*/(1 << AMDT_PWR_VALUE_SINGLE),
},
#endif
