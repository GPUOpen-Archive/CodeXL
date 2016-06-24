//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file Smu7DgpuCounterList.h
///
//==================================================================================

//
// This file is included inline by file \CodeXL\Components\PowerProfiling\Backend\AMDTPowerProfileAPI\src\AMDTPowerProfileControl.cpp
//
// Tonga dGPU
//

{
    /*attr_id*/COUNTERID_PKG_PWR_DGPU,
    /*len*/8,
    /*name*/ PP_STR_Counter_Power_DGPU,
    /*description*/"Average Discrete-GPU Power for the sampling period, reported in Watts. This is an estimated consumption value which is calculated based on dGPU activity levels.",
    /*unittype*/(AMDTPwrAttributeUnitType)PWR_UNIT_TYPE_WATT,
    /*category*/(PwrCategory)CATEGORY_POWER,
    /*instance type*/(AMDTPwrAttributeInstanceType)INSTANCE_TYPE_NONCORE_SINGLE,
    /*device type*/(AMDTDeviceType)AMDT_PWR_DEVICE_EXTERNAL_GPU,
    /*Aggregation type*/(1 << AMDT_PWR_VALUE_SINGLE) | (1 << AMDT_PWR_VALUE_CUMULATIVE),
},
{
    /*attr_id*/COUNTERID_TEMP_MEAS_DGPU,
    /*len*/8,
    /*name*/ PP_STR_Counter_MeasuredTemp_DGPU,
    /*description*/"Measured Discrete-GPU Average Temperature, reported in Celsius. The reported value is normalized and scaled, relative to the specific processor's maximum operating temperature. This value can be used to indicate rise and decline of temperature.",
    /*unittype*/(AMDTPwrAttributeUnitType)PWR_UNIT_TYPE_CENTIGRADE,
    /*category*/(PwrCategory)CATEGORY_TEMPERATURE,
    /*instance type*/(AMDTPwrAttributeInstanceType)INSTANCE_TYPE_NONCORE_SINGLE,
    /*device type*/(AMDTDeviceType)AMDT_PWR_DEVICE_EXTERNAL_GPU,
    /*Aggregation type*/(1 << AMDT_PWR_VALUE_SINGLE),
},
{
    /*attr_id*/COUNTERID_FREQ_DGPU,
    /*len*/8,
    /*name*/ PP_STR_Counter_AvgFreq_DGPU,
    /*description*/"Average Discrete-GPU Frequency for the sampling period, reported in MHz.",
    /*unittype*/(AMDTPwrAttributeUnitType)PWR_UNIT_TYPE_MEGA_HERTZ,
    /*category*/(PwrCategory)CATEGORY_FREQUENCY,
    /*instance type*/(AMDTPwrAttributeInstanceType)INSTANCE_TYPE_NONCORE_SINGLE,
    /*device type*/(AMDTDeviceType)AMDT_PWR_DEVICE_EXTERNAL_GPU,
    /*Aggregation type*/(1 << AMDT_PWR_VALUE_SINGLE) | (1 << AMDT_PWR_VALUE_HISTOGRAM),
},
{
    /*attr_id*/COUNTERID_VOLT_VDDC_LOAD_DGPU,
    /*len*/8,
    /*name*/ PP_STR_Counter_MeasuredVoltage_DGPU,
    /*description*/"Average Discrete-GPU Load voltage for the sampling period, reported in Volts.",
    /*unittype*/(AMDTPwrAttributeUnitType)PWR_UNIT_TYPE_VOLT,
    /*category*/(PwrCategory)CATEGORY_VOLTAGE,
    /*instance type*/(AMDTPwrAttributeInstanceType)INSTANCE_TYPE_NONCORE_SINGLE,
    /*device type*/(AMDTDeviceType)AMDT_PWR_DEVICE_EXTERNAL_GPU,
    /*Aggregation type*/(1 << AMDT_PWR_VALUE_SINGLE),
},
{
    /*attr_id*/COUNTERID_CURR_VDDC_DGPU,
    /*len*/8,
    /*name*/ PP_STR_Counter_MeasuredCurrent_DGPU,
    /*description*/"Average Discrete-GPU current for the sampling period, reported in Milli-Amperes.",
    /*unittype*/(AMDTPwrAttributeUnitType)PWR_UNIT_TYPE_MILLI_AMPERE,
    /*category*/(PwrCategory)CATEGORY_CURRENT,
    /*instance type*/(AMDTPwrAttributeInstanceType)INSTANCE_TYPE_NONCORE_SINGLE,
    /*device type*/(AMDTDeviceType)AMDT_PWR_DEVICE_EXTERNAL_GPU,
    /*Aggregation type*/(1 << AMDT_PWR_VALUE_SINGLE),
},

