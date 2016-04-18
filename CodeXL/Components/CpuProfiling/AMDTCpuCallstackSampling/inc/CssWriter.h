//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file CssWriter.h
///
//==================================================================================

#ifndef _CSSWRITER_H_
#define _CSSWRITER_H_

#include <AMDTBaseTools/Include/gtMap.h>
#include <ProfilingAgents/Utils/CrtFile.h>
#include "CallGraph.h"

class ExecutableFile;

class CP_CSS_API CssWriter
{
public:
    CssWriter();
    ~CssWriter();

    bool Open(const wchar_t* pFilePath);

    bool Write(CallGraph& callGraph, ProcessWorkingSetQuery& workingSet, gtUInt32 processId);

private:
    typedef gtHashMap<CallSite*, unsigned> CallSiteIndexMap;

    struct LeafNodeInfo
    {
        unsigned m_index;
        const EventSampleInfo* m_pSample;
        CallStack* m_pCallStack;
    };

    typedef std::multimap<CallSite*, LeafNodeInfo> LeafNodesMap;

    bool WriteCallStackRecord(CallStack& callStack, unsigned index);
    bool WriteLeafNodeRecord(LeafNodesMap::iterator itBegin, LeafNodesMap::iterator itEnd, gtUInt64 totalEventCount);
    bool WriteCallSiteRecord(CallSite& callSite, unsigned index);
    bool WriteModuleRecord(ExecutableFile& exe);
    static void WriteModuleRecordCallback(ExecutableFile& exe, void* pContext);

    unsigned AcquireCallSiteIndex(CallSite* pCallSite);
    unsigned AcquireLeafNodeIndex(const EventSampleInfo* pSample, CallStack* pCallStack);

    unsigned FindCallStackIndex(CallStack* pCallStack) const;

    void Clear();

    CrtFile m_outputFile;
    CallGraph* m_pCallGraph;
    ProcessWorkingSetQuery* m_pWorkingSet;
    CallSiteIndexMap m_callSites;
    LeafNodesMap m_leafNodes;
    unsigned m_leafNodesCount;
};

#endif // _CSSWRITER_H_
