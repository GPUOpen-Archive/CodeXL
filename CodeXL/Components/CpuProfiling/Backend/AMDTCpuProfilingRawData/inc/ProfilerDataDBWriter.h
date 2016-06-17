//=====================================================================
// Copyright (c) 2013-2016 Advanced Micro Devices, Inc. All rights reserved.
//
/// \file $File: $
/// \version $Revision: $
/// \brief Interface for the ProfilerDataDBWriter class.
//
//=====================================================================
// $Id: $
// Last checkin:   $DateTime: $
// Last edited by: $Author: $
// Change list:    $Change: $
//=====================================================================

#ifndef _PROFILEDATADBWRITER_H_
#define _PROFILEDATADBWRITER_H_

#include <AMDTOSWrappers/Include/osSynchronizedQueue.h>
#include <AMDTOSWrappers/Include/osThread.h>
#include <AMDTOSWrappers/Include/osTimeInterval.h>
#include <AMDTDbAdapter/inc/AMDTProfileDbAdapter.h>
#include <AMDTCpuCallstackSampling/inc/CallGraph.h>
#include <AMDTCpuPerfEventUtils/inc/EventsFile.h>
#include "CpuProfilingRawDataDLLBuild.h"
#include "CpuProfileInfo.h"
#include "CpuProfileModule.h"
#include "CpuProfileProcess.h"

enum TranslatedDataType
{
    TRANSLATED_DATA_TYPE_UNKNOWN_INFO = 0,
    TRANSLATED_DATA_TYPE_SESSION_INFO,
    TRANSLATED_DATA_TYPE_TOPOLOGY_INFO,
    TRANSLATED_DATA_TYPE_EVENT_INFO,
    TRANSLATED_DATA_TYPE_SAMPLINGCONFIG_INFO,
    TRANSLATED_DATA_TYPE_CORECONFIG_INFO,
    TRANSLATED_DATA_TYPE_MODULE_INFO,
    TRANSLATED_DATA_TYPE_PROCESS_INFO,
    TRANSLATED_DATA_TYPE_THREAD_INFO,
    TRANSLATED_DATA_TYPE_MODINSTANCE_INFO,
    TRANSLATED_DATA_TYPE_FUNCTION_INFO,
    TRANSLATED_DATA_TYPE_SAMPLE_INFO,
    TRANSLATED_DATA_TYPE_CSS_FRAME_INFO,
    TRANSLATED_DATA_TYPE_CSS_LEAF_INFO,
};

struct TranslatedDataContainer
{
    TranslatedDataType m_type = TRANSLATED_DATA_TYPE_UNKNOWN_INFO;
    // Can't use smart ptr as it is a void ptr
    void *m_data = nullptr;

    // define default ctor
    TranslatedDataContainer() = default;

    // define ctor
    TranslatedDataContainer(TranslatedDataType type, void* data) :
        m_type(type), m_data(data) {}
};

class ProfilerDataDBWriter;

class ProfilerDataWriterThread : public osThread
{
public:
    explicit ProfilerDataWriterThread(ProfilerDataDBWriter &writer) : osThread(L"DB Writer Thread"), m_Writer(writer) {};
    ProfilerDataWriterThread() = delete;
    ProfilerDataWriterThread(const ProfilerDataWriterThread&) = delete;
    ProfilerDataWriterThread& operator=(ProfilerDataWriterThread&) = delete;
    ProfilerDataWriterThread(ProfilerDataWriterThread&&) = default;
    ProfilerDataWriterThread& operator=(ProfilerDataWriterThread&&) = default;
    ~ProfilerDataWriterThread() = default;

    int entryPoint() override;

    void requestExit()
    {
        exitRequested = true;
    }

private:
    ProfilerDataDBWriter &m_Writer;
    bool exitRequested = false;
};

class CP_RAWDATA_API ProfilerDataDBWriter
{
public:
    ProfilerDataDBWriter()
    {
        m_pCpuProfDbAdapter = new amdtProfileDbAdapter;
        m_pWriterThread = new ProfilerDataWriterThread(*this);
    }

    ~ProfilerDataDBWriter()
    {
        if (m_pWriterThread != nullptr)
        {
            osTimeInterval timeout;
            timeout.setAsMilliSeconds(500);
            m_pWriterThread->waitForThreadEnd(timeout);

            m_pWriterThread->terminate();
            delete m_pWriterThread;
        }

        if (m_pCpuProfDbAdapter != nullptr)
        {
            m_pCpuProfDbAdapter->CloseDb();
            delete m_pCpuProfDbAdapter;
        }

        ClearWriterQueue();
    }

    bool Initialize(const gtString& path);

    // Push data to queue by reference.
    void Push(TranslatedDataContainer& dcSrc)
    {
        TranslatedDataContainer dcDest;
        std::swap(dcDest, dcSrc);
        m_translatedDataQueue.push(dcDest);
    }

    // Push data to move queue by rvalue reference.
    void Push(TranslatedDataContainer&& dcSrc)
    {
        TranslatedDataContainer dcDest;
        std::swap(dcDest, dcSrc);
        m_translatedDataQueue.push(dcDest);
    }

    TranslatedDataContainer Pop()
    {
        return m_translatedDataQueue.pop();
    }

    bool isEmpty() const
    {
        return m_translatedDataQueue.isEmpty();
    }

    // This will be called by the single writer thread
    void Write(TranslatedDataType type, void* data);
#if 0
    bool Write(CpuProfileInfo& profileInfo,
        gtUInt64 cpuAffinity,
        const PidProcessMap& procMap,
        gtVector<std::tuple<gtUInt32, gtUInt32>>& processThreadList,
        const NameModuleMap& modMap,
        const gtHashMap<gtUInt32, std::tuple<gtString, gtUInt64, gtUInt64>>& modInstanceInfo,
        const CoreTopologyMap* topMap = nullptr);
    bool Write(const CPACallStackFrameInfoList& csFrameInfoList);
    bool Write(const CPACallStackLeafInfoList& csLeafInfoList);
#endif

private:
    void PackSamplingConfigInfo(gtVector<std::pair<EventMaskType, gtUInt32>>& events, AMDTProfileSamplingConfigVec& samplingConfigs);
    void PackCounterInfo(gtVector<EventMaskType>& events, AMDTProfileCounterDescVec& counterInfo);
#if 0
    void PackSessionInfo(const CpuProfileInfo& profileInfo, gtUInt64 cpuAffinity, AMDTProfileSessionInfo& sessionInfo);
    void PackCoreTopology(const CoreTopologyMap& coreTopology, CPAdapterTopologyMap& cpaTopology);
    void PackSamplingEvents(const CpuProfileInfo& profileInfo,
                            AMDTProfileCounterDescVec& events,
                            AMDTProfileSamplingConfigVec& samplingConfigs);
    void PackProcessInfo(const PidProcessMap& processMap, CPAProcessList& processList);
    void PackModuleInfo(const NameModuleMap& modMap, CPAModuleList& moduleList);
    void PackModuleInstanceInfo(const NameModuleMap& modMap, const gtHashMap<gtUInt32, std::tuple<gtString, gtUInt64, gtUInt64>>& modInstanceInfoMap, CPAModuleInstanceList& moduleInstanceList);
    void PackProcessThreadInfo(const gtVector<std::tuple<gtUInt32, gtUInt32>>& processThreadList, CPAProcessThreadList& procThreadIdList);
    void PackCoreSamplingConfigInfo(const NameModuleMap& modMap, CPACoreSamplingConfigList& coreConfigList);
    void PackSampleInfo(const NameModuleMap& modMap, CPASampeInfoList& sampleList);
    void PackFunctionInfo(const NameModuleMap& modMap, CPAFunctionInfoList& funcInfoList);
#endif
    // TODO: These helper functions should be in utils
    gtString ConvertQtToGTString(const QString& inputStr);
    void DecodeSamplingEvent(EventMaskType encoded, gtUInt16& event, gtUByte& unitMask, bool& bitOs, bool& bitUsr);
    bool InitializeEventsXMLFile(gtUInt32 cpuFamily, gtUInt32 cpuModel, EventsFile& eventsFile);
    void ClearWriterQueue();

    unsigned long m_cpuFamily = 0;
    unsigned long m_cpuModel = 0;

    amdtProfileDbAdapter* m_pCpuProfDbAdapter = nullptr;

    // Queue to be used by multiple-producers (PRD/CAPERF translator) and single-consumer (i.e. DB writer).
    osSynchronizedQueue<TranslatedDataContainer> m_translatedDataQueue;

    // This thread would be responsible for writing the translated data to DB.
    ProfilerDataWriterThread *m_pWriterThread = nullptr;
};

#endif //_CPUPROFILEDATADBWRITER_H_
