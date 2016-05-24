//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file AMDTPowerProfileInternal.h
///
//==================================================================================

#ifndef _AMDTPOWERPROFILEPUBLIC_H_
#define _AMDTPOWERPROFILEPUBLIC_H_
#include <AMDTDefinitions.h>
#include <AMDTRawDataFileHeader.h>
#include <AMDTPowerProfileDataTypes.h>

#define MAX_CORE_CNT (64)
#define MAX_COUNTER_CNT (100)
#define MAX_CU_CNT (2)
#define MAX_PID_CNT (500)
#define MAX_INSTANCE_CNT (32)
#define MAX_BIN_CNT (10)
#define MAX_CONFIG_CNT          (10)
#define PWR_MAX_NAME_LEN        (64)
#define PWR_MAX_DESC_LEN        (512)
#define MAX_PATH_LEN            (260)
#define SVI2_ATTR_VALUE_CNT     (2)
#define BASIC_ATTR_VALUE_CNT    (3)
#define AMD_VENDOR_ID           0x1002
#define AMD_VENDOR_ID1          0x1022

typedef enum CXLContextProfileType
{
    PROCESS_PROFILE,
    MODULE_PROFILE,
    IPSAMPLE_PROFILE,
    MAX_PROFILE
} CXLContextProfileType;

//AMDTPwrProcessedDataType: Processed data type
enum AMDTPwrProcessedDataType
{
    PROFILE_DATA_TYPE_INVALID = 0,
    PROFILE_DATA_TYPE_TIME_LINE,
    PROFILE_DATA_TYPE_MAX_CNT
};

//AMDTPWrProfileConfig: defines profile configuration
//There can be multiple spec but attribute list
//is shared across all the specs
//Attributes are the set of interesting profile
//data to be collected for a sampling event
struct AMDTPwrProfileConfig
{
    AMDTUInt32 m_clientId;
    AMDTUInt32 m_specCnt;
    AMDTUInt32 m_attrCnt;
    SamplingSpec* m_pSpecList;
    AMDTUInt16* m_pAttrList;
};

// HardwareType: hardware family type
enum HardwareType
{
    GDT_HAINAN,             ///< HAINAN GPU
    GDT_DEVASTATOR,         ///< DEVASTATOR GPU
    GDT_DEVASTATOR_LITE,        ///< DEVASTATOR LITE GPU
    GDT_SCRAPPER,           ///< SCRAPPER GPU
    GDT_SCRAPPER_LITE,      ///< SCRAPPER LITE GPU
    GDT_BONAIRE,                ///< BONAIRE GPU (mobile is SATURN)
    GDT_HAWAII,             ///< HAWAII GPU
    GDT_KALINDI,                ///< KB APU
    GDT_SPECTRE,                ///< KV APU SPECTRE
    GDT_SPECTRE_SL,         ///< KV APU SPECTRE SL
    GDT_SPECTRE_LITE,       ///< KV APU SPECTRE LITE
    GDT_SPOOKY,             ///< KV APU SPOOKY
    GDT_ICELAND,                ///< ICELAND GPU
    GDT_TONGA,              ///< TONGA GPU
    GDT_CARRIZO,                ///< CZ APU
    GDT_FIJI,               ///< FIJI GPU
    GDT_OROCHI,               ///< OROCHI
    GDT_STONEY,               ///< STONEY
    GDT_LAST,                   ///< last
    GDT_INVALID = 0xFFFFFFFF
};

// DeviceType: Type of the device connected to the platform
enum DeviceType
{
    DEVICE_TYPE_APU,
    DEVICE_TYPE_DGPU,
    DEVICE_TYPE_CPU_NO_SMU,
    DEVICE_TYPE_NPU_NO_SMU,
    DEVICE_TYPE_DGPU_NO_SMU,
    DEVICE_TYPE_OTHERS
};

// PciDeviceInfo: PCIe information
struct PciDeviceInfo
{
    HardwareType    m_hardwareType;
    AMDTUInt32      m_deviceId;
    DeviceType      m_deviceType;
    char            m_modelName[PWR_MAX_NAME_LEN];
    char            m_name[PWR_MAX_NAME_LEN];
    char            m_shortName[PWR_MAX_NAME_LEN];
    AMDTUInt32      m_smuIpVersion;
};

// PlatformInfo: AMD platform information
struct PlatformInfo
{
    HardwareType    m_hardwareType;
    AMDTUInt32      m_family;
    AMDTUInt32      m_modelLow;
    AMDTUInt32      m_modelHigh;
    DeviceType      m_deviceType;
    char            m_name[PWR_MAX_NAME_LEN];
    char            m_shortName[PWR_MAX_NAME_LEN];
};

//AMDTPwrTargetSystemInfo: defines target system information
struct AMDTPwrTargetSystemInfo
{
    bool           m_isAmd;
    bool           m_isAmdApu;
    bool           m_isPlatformSupported;
    AMDTUInt32     m_family;
    AMDTUInt32     m_model;
    AMDTUInt32     m_platformId;
    AMDTUInt32     m_coreCnt;
    AMDTUInt32     m_computeUnitCnt;
    AMDTUInt32     m_coresPerCu;
    AMDTUInt32     m_svi2Cnt;
    AMDTUInt32     m_igpuCnt;
    PciDeviceInfo* m_pNodeInfo;
    SmuList        m_smuTable;
};

/**PwrCategory: Following is the list of counter
     category supported by power profiler.
*/
enum PwrCategory
{
    CATEGORY_POWER,        // Instantaneous power
    CATEGORY_FREQUENCY,    // Frequency
    CATEGORY_TEMPERATURE,  // Temperature in centegrade
    CATEGORY_VOLTAGE,      // Volatage
    CATEGORY_CURRENT,      // Current
    CATEGORY_DVFS,         // P-State, C-State
    CATEGORY_PROCESS,      // PID, TID
    CATEGORY_TIME,         // Time
    CATEGORY_NUMBER,       // generic count value
};


//AMDTPwrAttributeUnitType: Attribute unit types
enum AMDTPwrAttributeUnitType
{
    PWR_UNIT_TYPE_COUNT,             // Count index
    PWR_UNIT_TYPE_PERCENT,           // Percentage
    PWR_UNIT_TYPE_RATIO,             // Ratio
    PWR_UNIT_TYPE_MILLI_SECOND,      // Time
    PWR_UNIT_TYPE_JOULE,             // Energy consumption
    PWR_UNIT_TYPE_WATT,              // Power consumption
    PWR_UNIT_TYPE_VOLT,              // Voltage
    PWR_UNIT_TYPE_MILLI_AMPERE,      // Current
    PWR_UNIT_TYPE_MEGA_HERTZ,        // Frequency type unit
    PWR_UNIT_TYPE_CENTIGRADE         // Temperature type unit
};

enum AMDTPwrAttributeInstanceType
{
    INSTANCE_TYPE_NONCORE_SINGLE,
    INSTANCE_TYPE_PER_CU,
    INSTANCE_TYPE_PER_CORE,
    INSTANCE_TYPE_PER_CORE_MULTIVALUE,
    INSTANCE_TYPE_NONCORE_MULTIVALUE
};

//Following structure are used in Data access APIs
//AttributeTypeInfo: Attribute data type and length
struct AMDTPwrAttributeTypeInfo
{
    AMDTUInt32 m_attrId;
    AMDTUInt16 m_len;                             // Length in bytes
    char    m_name[PWR_MAX_NAME_LEN];             // Name of the attribute
    char    m_description[PWR_MAX_DESC_LEN];      // Description of the attribute
    AMDTPwrAttributeUnitType m_unitType;          // The attribute's unit type
    PwrCategory              m_category;
    AMDTPwrAttributeInstanceType m_instanceType;  // single/Cu/Core
};

//AMDTPwrProfileAttributeList: Attribute list
struct AMDTPwrProfileAttributeList
{
    AMDTUInt32 attrCnt;
    AMDTPwrAttributeTypeInfo* pAttrList;
};

// PwrInstrumentedPowerData:
typedef struct PwrInstrumentedPowerData
{
    AMDTUInt32       m_markerId;
    AMDTUInt32       m_state;
    AMDTFloat32      m_ipc[MAX_CU_CNT];
    AMDTFloat32      m_totalIpc[MAX_CU_CNT];
    AMDTInt32        m_cuPidInst[MAX_CU_CNT];
    AMDTPwrInstrumentedPowerData m_data;
} PwrInstrumentedPowerData;

//AMDTPwrAttributeInfo: Attribute information
struct AMDTPwrAttributeInfo
{
    AMDTUInt32      m_instanceId;
    union
    {
        AMDTUInt64  m_value64;
        AMDTFloat32 m_float32;
    } u;
    AMDTPwrAttributeTypeInfo*  m_pInfo;         //Attribute type information
};

// PowerData:
typedef struct PowerData
{
    AMDTUInt32       m_numberOfPids;
    AMDTUInt32       m_sampleCnt;
    AMDTFloat32      m_totalIpc;
    AMDTPwrProcessInfo  m_process[MAX_PID_CNT];
} PowerData;

typedef struct
{
    AMDTUInt32   m_binCnt;
    AMDTFloat32  m_pRangeStartIndex[MAX_BIN_CNT + 1];
    AMDTFloat32  m_pRangeValue[MAX_BIN_CNT + 1];
} Histogram;

// DerivedCounter: to hold the accumulated and histogram counter value
typedef struct
{
    union
    {
        Histogram   m_histogram[MAX_INSTANCE_CNT];
        AMDTFloat32 m_value[MAX_INSTANCE_CNT];
    };
} DerivedCounter;

//AMDTPwrProcessedDataRecord: Processed profile data
//A processed record is represent by AMDTPwrProcessedDataRecord.
//There could be multiple (indicated by m_attr_cnt) attributes in a record.
//m_pAttr represents the attribute list. Following is an example to access nth
//attribute in that record
//
//    AMDTPwrProcessedDataRecord rec;
//    AttributeInfo* nth_attr = &(rec.m_pAttr+n);
struct AMDTPwrProcessedDataRecord
{
    AMDTUInt64                m_recId;                   //Record-id
    AMDTUInt64                m_ts;
    AMDTPwrProcessedDataType  m_recordType;
    AMDTUInt32                m_attrCnt;                //Number of attributes in the record
    AMDTPwrAttributeInfo      m_attr[MAX_COUNTER_CNT];
};

// Process name structure
typedef struct ProcessName
{
    AMDTUInt32 m_pid;
    char       m_name[AMDT_PWR_EXE_NAME_LENGTH];
    char       m_path[AMDT_PWR_EXE_PATH_LENGTH];

} ProcessName;

#endif //_AMDTPOWERPROFILEPUBLIC_H_

