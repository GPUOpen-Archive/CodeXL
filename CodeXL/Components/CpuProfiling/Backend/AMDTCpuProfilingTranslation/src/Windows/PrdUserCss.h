//==================================================================================
// Copyright (c) 2014-2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file PrdUserCss.h
/// \brief User CSS processing.
///
//==================================================================================

#ifndef _PRDUSERCSS_H_
#define _PRDUSERCSS_H_

#include <AMDTOSWrappers/Include/osThread.h>
#include "PrdTranslator.h"
#include "RcuScheduler.h"

#define USER_CSS_RCU_DATA_SIZE 4096

class PrdUserCssRcuHandlerPool : public RcuHandlerAbstractFactory
{
public:
    PrdUserCssRcuHandlerPool(PrdTranslator& translator,
                             PrdReader& reader,
                             MemoryMap& mapAddress,
                             gtInt64 fileSize,
                             gtUInt64 lastUserCssRecordOffset);

    PrdUserCssRcuHandlerPool& operator=(const PrdUserCssRcuHandlerPool&) = delete;
    virtual RcuHandler* Create();

    int AddRef();
    int Release();

    bool Read(RcuData& data);

    PrdTranslator::ProcessInfo& AcquireProcessInfo(ProcessIdType pid) { return m_translator.AcquireProcessInfo(pid); }

    osSynchronizedQueue<gtString>& GetStatusesQueue() { return m_statusesQueue; }

private:
    bool RemapPrdFile();

    volatile gtInt32 m_refCount;

    unsigned int m_fileAllocGranularity;
    PrdTranslator& m_translator;
    PrdReader& m_reader;
    MemoryMap& m_mapAddress;
    gtInt64 m_fileSize;
    gtUInt64 m_cssRecordOffset;
    gtUInt64 m_firstRecordOffset;
    gtUInt64 m_minRecordOffset;
    osSynchronizedQueue<gtString> m_statusesQueue;
};


class PrdUserCssRcuHandler : public RcuHandler
{
public:
    PrdUserCssRcuHandler(PrdUserCssRcuHandlerPool& pool);
    virtual ~PrdUserCssRcuHandler();
    PrdUserCssRcuHandler& operator=(const PrdUserCssRcuHandler&) = delete;

    virtual bool Read(RcuData& data);
    virtual void Copy(RcuData*& pSrc, RcuData*& pDest);
    virtual void Update(RcuData& data);

private:
    struct UserCssRecord : PRD_USER_CSS_DATA_RECORD, PRD_USER_CSS_DATA_EXT_RECORD
    {
        PRD_VIRTUAL_STACK_RECORD* GetVirtualStackRecord();
    };

    PrdUserCssRcuHandlerPool& m_pool;
    PrdTranslator::ProcessInfo* m_pProcessInfo;
    ProcessIdType m_processInfoId;
    VirtualStackWalker m_stackWalker;

    wchar_t m_moduleName[OS_MAX_PATH + 1];
    wchar_t m_functionName[OS_MAX_PATH + 1];
    wchar_t m_jncName[OS_MAX_PATH + 1];
    wchar_t m_javaSrcFileName[OS_MAX_PATH + 1];
    osSynchronizedQueue<gtString>& m_statusesQueue;
};

#endif // _PRDUSERCSS_H_
