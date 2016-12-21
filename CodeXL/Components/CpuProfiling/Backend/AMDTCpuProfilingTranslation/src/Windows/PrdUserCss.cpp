//==================================================================================
// Copyright (c) 2013-2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file PrdUserCss.cpp
/// \brief User CSS processing.
///
//==================================================================================

#include <new>
#include <AMDTOSWrappers/Include/osAtomic.h>
#include <AMDTOSWrappers/Include/osDebugLog.h>
#include "PrdUserCss.h"
#include <AMDTCpuCallstackSampling/inc/CallStackBuilder.h>



// The size of data block that is read from the PRD file each time to be processed by a worker thread
#define CXL_PRD_MAPVIEW_GRANULARITY      131072000       // 125 MB

#define BEGIN_TICK_COUNT()      if (NULL != pStats) startTime = GetTickCount()
#define END_TICK_COUNT(field)   if (NULL != pStats) pStats->m_values[PrdTranslationStats::field].AtomicAdd(GetTickCount() - startTime)

// If va belongs to an inlined function, then the vector will be filled
// with any nested inlined caller function addresses in the calling order
static bool GetInlinedFuncInfoListByVA(gtVAddr va, gtVector<gtVAddr>& funcList, TiProcessWorkingSetQuery workingSet)
{
    bool ret(false);
    ExecutableFile* pExe = workingSet.FindModule(va);

    if (NULL != pExe && pExe->IsProcessInlineInfo())
    {
        SymbolEngine* pSymbolEngine = pExe->GetSymbolEngine();

        if (NULL != pSymbolEngine)
        {
            gtVector<gtRVAddr> funcRvaList = pSymbolEngine->FindNestedInlineFunctions(pExe->VaToRva(va));

            for (auto& it : funcRvaList)
            {
                funcList.push_back(pExe->RvaToVa(it));
            }

            ret = true;
        }
    }

    return ret;
}

PrdUserCssRcuHandlerPool::PrdUserCssRcuHandlerPool(PrdTranslator& translator,
                                                   PrdReader& reader,
                                                   MemoryMap& mapAddress,
                                                   gtInt64 fileSize,
                                                   gtUInt64 lastUserCssRecordOffset) :
    m_refCount(0),
    m_fileAllocGranularity(64U * 1024U),
    m_translator(translator),
    m_reader(reader),
    m_mapAddress(mapAddress),
    m_fileSize(fileSize),
    m_cssRecordOffset(lastUserCssRecordOffset),
    m_firstRecordOffset(0ULL),
    m_minRecordOffset(lastUserCssRecordOffset + 1ULL)
{
    SYSTEM_INFO sysInfo = {0};
    GetNativeSystemInfo(&sysInfo);

    if (0 != sysInfo.dwAllocationGranularity)
    {
        m_fileAllocGranularity = sysInfo.dwAllocationGranularity;
    }
}

int PrdUserCssRcuHandlerPool::AddRef()
{
    return AtomicAdd(m_refCount, +1) + 1;
}

int PrdUserCssRcuHandlerPool::Release()
{
    gtInt32 refCount = AtomicAdd(m_refCount, -1) - 1;

    if (0 == refCount)
    {
        delete this;
    }

    return refCount;
}

RcuHandler* PrdUserCssRcuHandlerPool::Create()
{
    RcuHandler* pHandler = new PrdUserCssRcuHandler(*this);

    if (NULL != pHandler)
    {
        m_refCount++;
    }

    return pHandler;
}

bool PrdUserCssRcuHandlerPool::Read(RcuData& data)
{
    bool ret = false;

    // Check if we have enough data to parse, otherwise RemapPrdFile will load another chunk of data (currently = 125MB)
    if (RemapPrdFile())
    {
        gtByte* pBuffer = static_cast<gtByte*>(m_mapAddress.GetMappedAddress()) - (m_firstRecordOffset * PRD_RECORD_SIZE);
        pBuffer += m_cssRecordOffset * PRD_RECORD_SIZE;

        PRD_USER_CSS_DATA_RECORD* pCssRecord = reinterpret_cast<PRD_USER_CSS_DATA_RECORD*>(pBuffer);
        PRD_USER_CSS_DATA_EXT_RECORD* pCssExtRecord = reinterpret_cast<PRD_USER_CSS_DATA_EXT_RECORD*>(pCssRecord + 1);

        GT_ASSERT(PROF_REC_USER_CSS == pCssRecord->m_RecordType);

        unsigned int cssRecordsSize = (1U + PrdTranslator::GetUserCallStackAdditionalRecordsCount(pCssRecord->m_Depth, FALSE != pCssRecord->m_Is64Bit)) * PRD_RECORD_SIZE;

        PRD_VIRTUAL_STACK_RECORD* pVirtualStackRecord = reinterpret_cast<PRD_VIRTUAL_STACK_RECORD*>(pBuffer + cssRecordsSize);

        // Always add the next record, so we may have a valid check for the next record's type.
        cssRecordsSize += PRD_RECORD_SIZE;

        if (PROF_REC_VIRTUAL_STACK == pVirtualStackRecord->m_RecordType)
        {
            cssRecordsSize += PrdTranslator::GetVirtualStackAdditionalRecordsCount(pVirtualStackRecord->m_ValuesCount) * PRD_RECORD_SIZE;
        }

        if (cssRecordsSize <= RcuScheduler::GetBufferSize(USER_CSS_RCU_DATA_SIZE))
        {
            // Baskar: initializing all the loadmodules and their symbols is redundant.
            // For many modules there won't be any samples. They un-necessarily consume memory.
            // fnGetModuleInfo() is already being called appropriately to load its symbols for
            // a load-module when a sample needs to be attributed to that load-module
#if 0
            // Make sure the process' executables are opened as they will be used in the translation stage.
            fnLoadProcessExecutableFiles(pCssRecord->m_ProcessHandle, m_statusesQueue);
#endif
            memcpy(data.m_buffer, pBuffer, cssRecordsSize);
            ret = true;
        }

        // Update the progress bar in the GUI
        m_translator.AsyncAddBytesToProgressBar(m_cssRecordOffset - pCssExtRecord->m_PrevUserCssRecordOffset);
        m_cssRecordOffset = pCssExtRecord->m_PrevUserCssRecordOffset;
    }

    if (!ret)
    {
        if (m_translator.m_useProgressSyncObject)
        {
            m_translator.m_useProgressSyncObject = false;
            m_translator.m_progressSyncObject.unlock();
        }
    }

    return ret;
}

// Remapping the file view must abide by the following rules:
// 1. Align on 64KB - this is a Windows requirement
// 2. Align on 40bytes - to allow access to PRD records which are 40 bytes each
bool PrdUserCssRcuHandlerPool::RemapPrdFile()
{
    bool ret = (0ULL != m_cssRecordOffset);

    // If there is no more data to read because we have read beyond the next available record
    if (ret && m_minRecordOffset > m_cssRecordOffset)
    {
        HRESULT res;

        m_mapAddress.DestroyView();

        // Calculate the biggest size that the CSS records take up. The CSS may contain either 32-bit data or 64-bit data,
        // we won't know which one until we read it. So assume the worst case and map the bigger size.
        const unsigned int maxCssRecordsSize = ((1U + PrdTranslator::GetUserCallStackAdditionalRecordsCount(MAX_CSS_VALUES, true)) +
                                                (1U + PrdTranslator::GetVirtualStackAdditionalRecordsCount(MAX_CSS_VALUES))) * PRD_RECORD_SIZE;

        // Calculate the offset to the point where we will end reading from the file
        gtUInt64 offset = m_cssRecordOffset * PRD_RECORD_SIZE + maxCssRecordsSize;

        if (static_cast<gtUInt64>(m_fileSize) < offset)
        {
            offset = static_cast<gtUInt64>(m_fileSize);
        }

        gtUInt32 length;
        gtUInt32 firstWeightRecOffset;

        if (offset > CXL_PRD_MAPVIEW_GRANULARITY)
        {
            offset -= CXL_PRD_MAPVIEW_GRANULARITY;
            offset = gtAlignUp(offset, m_fileAllocGranularity);

            while (0 != (offset % PRD_RECORD_SIZE))
            {
                offset += m_fileAllocGranularity;
            }

            gtUInt64 remaining = static_cast<gtUInt64>(m_fileSize) - offset;

            if (remaining < CXL_PRD_MAPVIEW_GRANULARITY)
            {
                length = static_cast<gtUInt32>(remaining);
            }
            else
            {
                length = CXL_PRD_MAPVIEW_GRANULARITY;
            }
        }
        else
        {
            length = (gtUInt32)offset;
            offset = 0ULL;
        }

        res = PrdTranslator::CreatePRDView(m_reader, offset, length, m_mapAddress, &firstWeightRecOffset);

        if (S_OK == res)
        {
            m_firstRecordOffset = offset / PRD_RECORD_SIZE;
            m_minRecordOffset = (offset > PRD_RECORD_SIZE) ? m_firstRecordOffset : 1ULL;
        }
        else
        {
            OS_OUTPUT_DEBUG_LOG(L"Could not create a mapped file view", OS_DEBUG_LOG_ERROR);
            ret = false;
        }
    }

    return ret;
}


PrdUserCssRcuHandler::PrdUserCssRcuHandler(PrdUserCssRcuHandlerPool& pool) :
    m_pool(pool),
    m_pProcessInfo(NULL),
    m_processInfoId(0),
    m_statusesQueue(pool.GetStatusesQueue())
{
    *m_moduleName       = L'\0';
    *m_functionName     = L'\0';
    *m_jncName          = L'\0';
    *m_javaSrcFileName  = L'\0';

    m_pool.AddRef();
}

PrdUserCssRcuHandler::~PrdUserCssRcuHandler()
{
    m_pool.Release();
}

bool PrdUserCssRcuHandler::Read(RcuData& data)
{
    return m_pool.Read(data);
}

void PrdUserCssRcuHandler::Update(RcuData& data)
{
    UserCssRecord* pCssRecord = reinterpret_cast<UserCssRecord*>(data.m_buffer);
    gtUInt32 maxInlinedFuncs = 0;
    gtUInt32 curInlinedFuncs = 0;

    unsigned depth = pCssRecord->m_Depth;

    if (0U != depth)
    {
        ProcessIdType processId = static_cast<ProcessIdType>(pCssRecord->m_ProcessHandle);

        if (processId != m_processInfoId)
        {
            m_processInfoId = processId;
            m_pProcessInfo = &m_pool.AcquireProcessInfo(processId);
        }

        maxInlinedFuncs = (depth > 32) ? (depth >> 2) : (depth >> 1);
        gtVector<gtUInt64> buffer(depth + maxInlinedFuncs + 1);

        CallStackBuilder callStackBuilder(m_pProcessInfo->m_callGraph,
                                          reinterpret_cast<gtUByte*>(buffer.data()),
                                          static_cast<unsigned>(buffer.capacity() * sizeof(gtUInt64)));

        TiProcessWorkingSetQuery workingSet(static_cast<ProcessIdType>(pCssRecord->m_ProcessHandle));
        gtUInt64 sampleAddr = (gtUInt64)(-1);
        bool is64Bit = (FALSE != pCssRecord->m_Is64Bit);

        // if CallStackBuilder already initialized
        bool isCSBInitialized(false);

        if (is64Bit)
        {
            sampleAddr = pCssRecord->m_CallStack64[0U];
        }
        else
        {
            sampleAddr = pCssRecord->m_CallStack32[0U];
        }

        gtVector<gtVAddr> funcList;
        GetInlinedFuncInfoListByVA(sampleAddr, funcList, workingSet);

        if (funcList.size() > 1)
        {
            auto it = funcList.rbegin();
            sampleAddr = *it;
            it++;

            while (curInlinedFuncs < maxInlinedFuncs && funcList.rend() != it)
            {
                gtVAddr callerVa = *it;

                if (!isCSBInitialized)
                {
                    callStackBuilder.Initialize(callerVa, 0ULL, 0ULL);
                    isCSBInitialized = true;
                }
                else
                {
                    callStackBuilder.Push(callerVa);
                }

                it++;
                curInlinedFuncs++;
            }

            if (funcList.rend() != it)
            {
                gtVAddr callerVa = funcList.front();

                if (!isCSBInitialized)
                {
                    callStackBuilder.Initialize(callerVa, 0ULL, 0ULL);
                    isCSBInitialized = true;
                }
                else
                {
                    callStackBuilder.Push(callerVa);
                }
            }
        }

        if (1U < depth)
        {
            for (unsigned i = 1U; i < depth; ++i)
            {
                funcList.clear();
                gtVAddr currAddr;

                if (is64Bit)
                {
                    currAddr = pCssRecord->m_CallStack64[i];
                }
                else
                {
                    currAddr = pCssRecord->m_CallStack32[i];
                }

                GetInlinedFuncInfoListByVA(currAddr, funcList, workingSet);

                if (funcList.size() > 1)
                {
                    auto it = funcList.rbegin();

                    while (curInlinedFuncs <= maxInlinedFuncs && funcList.rend() != it)
                    {
                        gtVAddr callerVa = *it;

                        if (!isCSBInitialized)
                        {
                            callStackBuilder.Initialize(callerVa, 0ULL, 0ULL);
                            isCSBInitialized = true;
                        }
                        else
                        {
                            callStackBuilder.Push(callerVa);
                        }

                        it++;
                        curInlinedFuncs++;
                    }

                    if (funcList.rend() != it)
                    {
                        gtVAddr callerVa = funcList.front();
                        callStackBuilder.Push(callerVa);
                    }
                }
                else
                {
                    if (!isCSBInitialized)
                    {
                        callStackBuilder.Initialize(currAddr, 0ULL, 0ULL);
                        isCSBInitialized = true;
                    }
                    else
                    {
                        callStackBuilder.Push(currAddr);
                    }
                }
            }
        }


        PrdTranslator::FinalizePartialUserCallStack(*m_pProcessInfo,
                                                    static_cast<ThreadIdType>(pCssRecord->m_ThreadHandle),
                                                    reinterpret_cast<PrdTranslator::TimeRange&>(pCssRecord->m_TickStampBegin),
                                                    sampleAddr,
                                                    callStackBuilder,
                                                    NULL);
    }
}

void PrdUserCssRcuHandler::Copy(RcuData*& pSrc, RcuData*& pDest)
{
    unsigned depth = 0U;
    UserCssRecord* pCssRecord = reinterpret_cast<UserCssRecord*>(pSrc->m_buffer);
    UserCssRecord* pDestCssRecord = reinterpret_cast<UserCssRecord*>(pDest->m_buffer);

    bool is64Bit = (FALSE != pCssRecord->m_Is64Bit);

    if (0U != pCssRecord->m_Depth)
    {
        PRD_VIRTUAL_STACK_RECORD* pVirtualStackRecord = pCssRecord->GetVirtualStackRecord();

        if (NULL != pVirtualStackRecord)
        {
            unsigned valuesCount = pVirtualStackRecord->m_ValuesCount;
            gtUInt64 stackPtr = pVirtualStackRecord->m_StackPointer;
            gtUInt64 framePtr = static_cast<gtUInt64>(pVirtualStackRecord->m_FramePointerOffset) + stackPtr;
            gtUInt64 instructionPtr;

            gtUInt32* pFrameWalk;
            unsigned walkLength;
            unsigned bufferSize = RcuScheduler::GetBufferSize(USER_CSS_RCU_DATA_SIZE);
            gtUByte* pBuffer;

            if (is64Bit)
            {
                instructionPtr = static_cast<gtUInt64>(pCssRecord->m_CallStack64[0]);

                pFrameWalk = NULL;
                walkLength = 0U;

                bufferSize -= offsetof(UserCssRecord, m_CallStack64);
                pBuffer = reinterpret_cast<gtUByte*>(pDestCssRecord->m_CallStack64);
            }
            else
            {
                instructionPtr = static_cast<gtUInt64>(pCssRecord->m_CallStack32[0]);

                pFrameWalk = pCssRecord->m_CallStack32;
                walkLength = pCssRecord->m_Depth;

                bufferSize -= offsetof(UserCssRecord, m_CallStack32);
                pBuffer = reinterpret_cast<gtUByte*>(pDestCssRecord->m_CallStack32);
            }

            // Prepare an interface object that the stack walker can use to query info about the process that was profiled
            TiProcessWorkingSetQuery workingSet(static_cast<ProcessIdType>(pCssRecord->m_ProcessHandle));

            m_stackWalker.Reset(instructionPtr, framePtr, stackPtr,
                                pVirtualStackRecord->m_Values,
                                reinterpret_cast<gtUInt16*>(pVirtualStackRecord->m_Values + valuesCount),
                                valuesCount,
                                workingSet,
                                (is64Bit ? sizeof(gtUInt64) : sizeof(gtUInt32)));

            // Perform virtual stack analysis for overcoming usage of FPO in the build
            depth = m_stackWalker.BackTrace(pBuffer, bufferSize, pFrameWalk, walkLength);
        }
    }

    if (1U < depth)
    {
        memcpy(pDestCssRecord, pCssRecord, sizeof(PRD_USER_CSS_DATA_RECORD));
        pDestCssRecord->m_Depth = static_cast<USHORT>(depth);
    }
    else
    {
        // No need to do memcpy because depth is 1 or 0, meaning there is no virtual stack data or the analysis failed.
        // Just swap the buffers
        pDestCssRecord = pCssRecord;
        RcuData* pTempData = pDest;
        pDest = pSrc;
        pSrc = pTempData;
        depth = pDestCssRecord->m_Depth;
    }

#if AMDT_BUILD_CONFIGURATION == AMDT_DEBUG_BUILD
    pDestCssRecord->m_PrevUserCssRecordOffset = 0ULL;
#endif


    TiModuleInfo modInfo;
    modInfo.processID = pDestCssRecord->m_ProcessHandle;
    modInfo.cpuIndex = 0;
    modInfo.deltaTick = pDestCssRecord->m_TickStampBegin;
    modInfo.pModulename = m_moduleName;
    modInfo.pFunctionName = m_functionName;
    modInfo.pJncName = m_jncName;
    modInfo.pJavaSrcFileName = m_javaSrcFileName;
    modInfo.moduleType = evPEModule;

    if (is64Bit)
    {
        for (unsigned i = 0U; i < depth; ++i)
        {
            *m_moduleName      = L'\0';
            *m_functionName    = L'\0';
            *m_jncName         = L'\0';
            *m_javaSrcFileName = L'\0';

            modInfo.funNameSize     = OS_MAX_PATH;
            modInfo.jncNameSize     = OS_MAX_PATH;
            modInfo.namesize        = OS_MAX_PATH;
            modInfo.srcfilesize     = OS_MAX_PATH;
            modInfo.ModuleStartAddr = 0;
            modInfo.sampleAddr = pDestCssRecord->m_CallStack64[i];

            if (S_OK != fnGetModuleInfo(&modInfo) || evPEModule != modInfo.moduleType)
            {
                pDestCssRecord->m_Depth = (USHORT)i;
                break;
            }
        }
    }
    else
    {
        for (unsigned i = 0U; i < depth; ++i)
        {
            *m_moduleName      = L'\0';
            *m_functionName    = L'\0';
            *m_jncName         = L'\0';
            *m_javaSrcFileName = L'\0';

            modInfo.funNameSize     = OS_MAX_PATH;
            modInfo.jncNameSize     = OS_MAX_PATH;
            modInfo.namesize        = OS_MAX_PATH;
            modInfo.srcfilesize     = OS_MAX_PATH;
            modInfo.ModuleStartAddr = 0;
            modInfo.sampleAddr = pDestCssRecord->m_CallStack32[i];

            if (S_OK != fnGetModuleInfo(&modInfo) || evPEModule != modInfo.moduleType)
            {
                pDestCssRecord->m_Depth = (USHORT)i;
                break;
            }
        }
    }
}


PRD_VIRTUAL_STACK_RECORD* PrdUserCssRcuHandler::UserCssRecord::GetVirtualStackRecord()
{
    PRD_VIRTUAL_STACK_RECORD* pVirtualStackRecord = NULL;

    unsigned int recordsSize = PrdTranslator::GetUserCallStackAdditionalRecordsCount(m_Depth, FALSE != m_Is64Bit);
    recordsSize = (recordsSize + 1U) * PRD_RECORD_SIZE;

    if (recordsSize < RcuScheduler::GetBufferSize(USER_CSS_RCU_DATA_SIZE))
    {
        PRD_VIRTUAL_STACK_RECORD* pNextRecord = reinterpret_cast<PRD_VIRTUAL_STACK_RECORD*>(reinterpret_cast<gtByte*>(this) + recordsSize);

        if (PROF_REC_VIRTUAL_STACK == pNextRecord->m_RecordType)
        {
            pVirtualStackRecord = pNextRecord;
        }
    }

    return pVirtualStackRecord;
}
