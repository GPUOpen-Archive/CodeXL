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
#include <TaskInfoInterface.h>
#include <CpuProfilingTranslationDLLBuild.h>
#include <AMDTOSWrappers/Include/osApplication.h>

using namespace std;
#define MAX_CORE_CNT 32

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

    //IpAddressInfo:
    typedef struct IpAddressInfo
    {
        AMDTUInt32   m_sampleCnt;
        AMDTFloat32  m_power;
    } IpAddressInfo;

    //Ip map: (ip, ip info)
    typedef gtMap<AMDTUInt64, IpAddressInfo> IpAddressMap;

    // ModuleIntanceInfo:
    typedef struct  ModuleIntanceInfo
    {
        AMDTFloat32  m_power;
        AMDTUInt32   m_sampleCnt;
        IpAddressMap m_ipMap;
    } ModuleIntanceInfo;

    // instance map: (module instance id, instance info)
    typedef gtMap<AMDTUInt32, ModuleIntanceInfo> ModuleInstanceMap;

    // ProcessInfo:
    typedef struct  ProcessInfo
    {
        AMDTFloat32       m_power;
        AMDTUInt32        m_sampleCnt;
        ModuleInstanceMap m_modInstMap;
    } ProcessInfo;

    // SampleData:
    typedef struct SampleData
    {
        AMDTUInt64     m_ip;
        AMDTUInt32     m_processId;
        AMDTUInt32     m_threadId;
        AMDTUInt64     m_timeStamp;
        AMDTUInt32     m_modInstance;
        AMDTFloat32    m_ipc;
        AMDTUInt32     m_sampleCnt;
    } SampleData;

    // process map: (process id, process info)
    typedef gtMap<AMDTUInt32, ProcessInfo> ProcessTreeMap;

    // Interesting module instance list(instance id, module info)
    typedef gtMap<AMDTUInt32, LoadModuleInfo> ModuleInfoMap;

    // sample key(ip+instance, sample info)
    typedef gtMap<AMDTUInt64, SampleData> SampleMap;

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

        // PwrGetModuleProfileData: Provide process data based on process/module/ip profile type
        AMDTResult PwrGetProfileData(CXLContextProfileType type, void** pData, AMDTUInt32* pCnt, AMDTFloat32* pPower);

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

        void SetElapsedTime(AMDTUInt64 raw, AMDTUInt64* pResult);

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

        // ExtractNameAndPath: Helper function to extract name and path from full path
        void ExtractNameAndPath(char* pFullPath, char* pName, char* pPath);

        //AttributePowerToSample: Calculate the power for each sample and insert in the system tree
        AMDTResult AttributePowerToSample(AMDTFloat32* cuPower);

        // ProcessSample: Process each data sample for process/module/ip profiling
        AMDTResult ProcessSample(ContextData* pCtx, AMDTFloat32 ipc, AMDTUInt32 coreId);

        // InsertSampleToSystemTree: Insert each sample to system tree with corresponding power
        AMDTResult InsertSampleToSystemTree(SampleData* pCtx, AMDTFloat32 power, AMDTUInt32 sampleCnt);

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
        AMDTPwrProcessedDataRecord m_data;
        std::queue <AMDTPwrProcessedDataRecord> m_processedData;
        mutable std::mutex m_lock;
        std::condition_variable m_condition;
        AMDTUInt64 m_prevTs;
        AMDTUInt64 m_currentTs;
        AMDTPwrAttributeInfo m_prevSmuSampleData[MAX_PREV_COUNTERS];

        vector <AMDTPwrProcessInfo> m_processList;
        vector <ContextPowerData> m_contextList;
        vector <AMDTPwrModuleData> m_moduleList;

        AMDTFloat32 m_sampleIpcLoad[MAX_CORE_CNT];
        ModuleInfoMap m_moduleTable;
        SampleMap m_sampleMap[MAX_CORE_CNT];
        ProcessTreeMap m_systemTreeMap;
        AMDTUInt64 m_prevTs1;
        AMDTUInt64 m_perfCounter;
        AMDTUInt64 m_perfFreq;
    };

#endif //_POWER_PROFILE_TRANSLATE_H_

