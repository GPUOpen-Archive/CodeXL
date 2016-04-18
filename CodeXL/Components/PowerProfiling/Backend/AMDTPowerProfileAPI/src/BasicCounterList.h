//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file BasicCounterList.h
///
//==================================================================================

//Basic Counters
{
    /*attr_id*/COUNTERID_SAMPLE_ID,
    /*len*/8,
    /*name*/ "Sampling spec id",
    /*description*/ "Sample-id",
    /*unittype*/(AMDTPwrAttributeUnitType)PWR_UNIT_TYPE_COUNT,
    /*category*/(PwrCategory)CATEGORY_NUMBER,
    /*instance type*/(AMDTPwrAttributeInstanceType)INSTANCE_TYPE_NONCORE_SINGLE,
},
{
    /*attr_id*/COUNTERID_RECORD_ID,
    /*len*/8,
    /*name*/ "Record id",
    /*description*/ "Record-id",
    /*unittype*/(AMDTPwrAttributeUnitType)PWR_UNIT_TYPE_COUNT,
    /*category*/(PwrCategory)CATEGORY_NUMBER,
    /*instance type*/(AMDTPwrAttributeInstanceType)INSTANCE_TYPE_NONCORE_SINGLE,
},
{
    /*attr_id*/COUNTERID_SAMPLE_TIME,
    /*len*/8,
    /*name*/ "Timestamp",
    /*description*/ "Sample collection timestamp",
    /*unittype*/(AMDTPwrAttributeUnitType)PWR_UNIT_TYPE_MILLI_SECOND,
    /*category*/(PwrCategory)CATEGORY_TIME,
    /*instance type*/(AMDTPwrAttributeInstanceType)INSTANCE_TYPE_NONCORE_SINGLE,
},