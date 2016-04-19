//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file PowerProfileTranslate.h
///
//==================================================================================

#ifndef _POWER_PROFILE_TRANSLATE_H_
#define _POWER_PROFILE_TRANSLATE_H_
#include <AMDTDefinitions.h>
#include <OsFileWrapper.h>
#include <AMDTRawDataFileHeader.h>
#include <RawDataReader.h>
#include <AMDTPowerProfileInternal.h>
#include <PowerProfileDriverInterface.h>
#include <queue>
#include <mutex>
#include <condition_variable>

using namespace std;

#define TRANSLATION_POOL_SIZE 2*1048576 // 2MB
// Maximum value to check the validity
#define MAX_POWER         256
#define MAX_TEMPERATURE   256
#define MAX_GPU_FREQUENCY 2000
//SMU register encoding divisor factor
#define PWR_DIV_FACTOR 256U
#define TEMP_DIV_FACTOR 256U
#define VOLT_DIV_FACTOR 32768.0
#define CURRENT_DIV_FACTOR 256.0
#define FREQ_DIV_FACTOR 100U
#define RESI_DIV_FACTOR 256U
#define SMU7_PROCESS_TEPERATURE_DATA(x) (x & 0x80000000)? -(((AMDTFloat32)(~x)) / (AMDTFloat32)256.0):((AMDTFloat32)((AMDTFloat32)x / (AMDTFloat32)256.0))
#define SMU7_PROCESS_POWER_DATA(x) AMDTFloat32(x)/ (AMDTFloat32)256.0
#define SMU7_PROCESS_FREQUENCY_DATA(x) AMDTFloat32(x)/ (AMDTFloat32)100.0
#define SMU7_PROCESS_VOLTAGE_DATA(x) AMDTFloat32(x)/ (AMDTFloat32)32768.0
#define SMU7_PROCESS_CURRENT_DATA(x) AMDTFloat32(x)/ (AMDTFloat32)256.0

#define DECODE_TEPERATURE(x) (x & 0x80000000)? -(((AMDTFloat32)(~x)) / (AMDTFloat32)256.0):((AMDTFloat32)((AMDTFloat32)x / (AMDTFloat32)256.0))
#define DECODE_POWER(x) AMDTFloat32(x)/ (AMDTFloat32)256.0
#define DECODE_FREQUENCY(x) AMDTFloat32(x)/ (AMDTFloat32)100.0
#define DECODE_VOLTAGE(x) AMDTFloat32(x)/ (AMDTFloat32)32768.0
#define DECODE_CURRENT(x) AMDTFloat32(x)/ (AMDTFloat32)256.0
#define MAX_PREV_COUNTERS 32

#define DECODE_SMU7_RAW_DATA(in, out, cat) switch(cat)\
    {\
    case CATEGORY_POWER:out = DECODE_POWER(in);break;\
    case CATEGORY_FREQUENCY:out = DECODE_FREQUENCY(in);break;\
    case CATEGORY_TEMPERATURE:out = DECODE_TEPERATURE(in);break;\
    case CATEGORY_VOLTAGE:out = DECODE_VOLTAGE(in);break;\
    case CATEGORY_CURRENT:out = DECODE_CURRENT(in);break;\
    case CATEGORY_DVFS:out = (AMDTFloat32)in;break;\
    case CATEGORY_PROCESS:out = (AMDTFloat32)in;break;\
    case CATEGORY_TIME:out = (AMDTFloat32)in;break;\
    case CATEGORY_NUMBER:out = (AMDTFloat32)in;break;\
    }\

    // IEEE754Decode: IEEE754 float decoding.Please check the IEEE 754 decoding for details
    union IEEE754Decode
    {
        AMDTUInt32  u32;
        AMDTFloat32 f32;
    };
#define SMU8_PROCESS_RAWDATA(out, raw) {union IEEE754Decode pack; pack.u32 = raw; out = pack.f32;}


    /****************************************************************************/

    typedef struct SampleProcessData
    {
        AMDTUInt32  m_pid;
        AMDTUInt32  m_cnt;
        AMDTFloat32 m_power;
        AMDTUInt32  m_cu;
    } SampleProcessData;

    typedef struct ProcessProfileData
    {
        AMDTUInt64  m_pid;
        AMDTUInt32  m_cnt;
        AMDTFloat32 m_power;
    } ProcessProfileData;

    typedef struct SampleConfig
    {
        AMDTUInt16 m_configId;
        AMDTUInt16 m_attrCnt;
        AMDTUInt16 m_sampleId;
        AMDTUInt16 m_counter[MAX_COUNTER_CNT];
    } SampleConfig;

    typedef struct SampleConfigList
    {
        AMDTUInt16 m_cfgCnt;
        AMDTUInt16 fill;
        SampleConfig* m_pCfg;
    } SampleConfigList;

    class PowerProfileTranslate
    {
    public:
        PowerProfileTranslate();
        virtual ~PowerProfileTranslate();

        //InitPowerProfiler - Initialize power profiler
        AMDTResult InitPowerTranslate(const wchar_t* pFileName, AMDTUInt64 flag);

        // Cleanup all memory allocations done in Init function
        void ClosePowerProfileTranslate();


        //GetProfileType - profile type
        AMDTResult GetProfileType();

        //GetConfigCoreMask - prepare the core mask activated for the profiling
        AMDTResult GetConfigCoreMask(AMDTUInt64* pMask);

        AMDTResult TranslateRawData();

        AMDTResult GetProcessedList(AMDTPwrProcessedDataRecord* pData);

        // Get the profile start/end absolute time stamps
        AMDTResult GetSessionTimeStamps(AMDTUInt64& startTs, AMDTUInt64& endTs)
        {
            AMDTResult ret = AMDT_ERROR_FAIL;

            if (NULL != m_rawFileHld)
            {
                ret = m_rawFileHld->GetSessionTimeStamps(&startTs, &endTs);
            }

            return ret;
        }

        RawDataReader* GetRawDataHandle() { return m_rawFileHld;}

        //Prepare the configured attribute list
        AMDTResult PrepareAttrList();

        //GetTargetSystemInfo
        AMDTResult GetTargetSystemInfo(AMDTUInt32* pFamily, AMDTUInt32* pModel, AMDTUInt32* pCoreCnt, AMDTUInt32* pCuCnt)\
        {
            *pFamily = m_rawFileHld->GetTargetFamily();
            * pModel = m_rawFileHld->GetTargetModel();
            * pCoreCnt = m_targetCoreCnt;
            * pCuCnt = m_targetCuCnt;
            return AMDT_STATUS_OK;
        }

        // GetInstrumentedData: Get the intrumented power for the selected marker
        AMDTResult GetInstrumentedData(AMDTUInt32 markerId, PwrInstrumentedPowerData** ppData);

        // Get the sampling period
        AMDTUInt32  GetSamplingPeriod() const { return m_samplingPeriod; }

    protected:
        RawDataReader* m_rawFileHld;
    private:

        // Decode dGPU counter data
        AMDTResult DecodeDgpuData(AMDTUInt8* pData,
                                  AMDTUInt32* pOffset,
                                  AMDTUInt32 counterId,
                                  AMDTPwrAttributeInfo* pInfo,
                                  AMDTUInt32* pLen);

        //DecodeRegisters -decode register values to meaningful data
        AMDTResult DecodeData(AMDTUInt32 coreId,
                              AMDTUInt8* pData,
                              AMDTUInt32* pOffset,
                              AMDTUInt32 counterId,
                              AMDTPwrAttributeInfo* pInfo,
                              AMDTUInt32* pLen);

        //GetAttributeLength
        AMDTResult GetAttributeLength(AMDTUInt32 attrId, AMDTUInt32 coreCnt, AMDTUInt32* pLen);

        void SetElapsedTime(AMDTUInt64 raw, AMDTFloat32* pResult);

        void CalculatePidPower(AMDTFloat32* cuPower, AMDTUInt32 cuCnt);

        void GetProcessName(AMDTPwrProcessInfo* pInfo);

        // ReadSharedBuffer: Populate the shared buffer to the core specific buffers
        AMDTResult ReadSharedBuffer(SharedBuffer* pBuffer, AMDTUInt64 coreMask);

        // UpdateSharedBufferInfo: update the buffer meta data once processing is done
        AMDTResult UpdateSharedBufferInfo(SharedBuffer* pBuffer, AMDTUInt64 coreMask);

        // ProcessMarkerRecords: process marker records
        void ProcessMarkerRecords(AMDTUInt8* pRaw, AMDTUInt32 bufferSize, AMDTUInt32* pOffset);

        // GetPrevSampleData: Read previous valid record values
        AMDTFloat32 GetPrevSampleData(AMDTUInt32 counterId, AMDTUInt32 inst);

        // SaveCurrentSampleData: Insert previous valid data. So that it can be used in case
        // there is junk value in the next sample
        bool SaveCurrentSampleData(AMDTPwrAttributeInfo* pData);

        //Attribute list in the raw record
        SampleConfigList m_configList;

        AMDTUInt32 m_hostCoreCnt;
        AMDTUInt32 m_targetCoreCnt;
        AMDTUInt32 m_targetCuCnt;
        AMDTUInt32 m_coresPerCu;
        AMDTUInt64 m_sampleOffset;
        AMDTUInt32 m_consumedRecId;

        //Online specific parameters
        AMDTUInt32 m_recordIdx;

        // Platform id
        AMDTUInt32 m_platformId;

        SharedBuffer* m_pSahredBuffer;
        AMDTPwrTargetSystemInfo m_sysInfo;
        bool m_isRunning;


        //Ring buffer
        PowerData   m_samplePower[MAX_CU_CNT];
        AMDTUInt32  m_samplingPeriod;
        AMDTUInt32  m_recIdFactor;
        ProfileType m_profileType;
        AMDTUInt32  m_cuCnt;
        AMDTPwrProcessedDataRecord m_data;
        std::queue <AMDTPwrProcessedDataRecord> m_processedData;
        mutable std::mutex m_lock;
        std::condition_variable m_condition;
        AMDTFloat32 m_prevTs;
        AMDTFloat32 m_currentTs;
        AMDTPwrAttributeInfo m_prevSmuSampleData[MAX_PREV_COUNTERS];
    };

#endif //_POWER_PROFILE_TRANSLATE_H_

