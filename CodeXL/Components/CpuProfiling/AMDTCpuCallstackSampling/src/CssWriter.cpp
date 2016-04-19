//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file CssWriter.cpp
///
//==================================================================================

#include <CssWriter.h>
#include <CssFile.h>
#include <AMDTBaseTools/Include/gtSet.h>
#include <AMDTBaseTools/Include/gtVector.h>
#include <AMDTExecutableFormat/inc/ExecutableFile.h>

template <typename Ty>
bool RewriteDataValue(CrtFile& outputFile, const Ty& data, unsigned offset)
{
    // Rewind to the value's position
    outputFile.seekCurrentPosition(CrtFile::ORIGIN_CURRENT, 0L - static_cast<long>(offset + sizeof(data)));
    // Write the value
    outputFile.write(data);
    // Get back to the right position
    outputFile.seekCurrentPosition(CrtFile::ORIGIN_CURRENT, static_cast<long>(offset));

    return true;
}


CssWriter::CssWriter() : m_pCallGraph(NULL), m_pWorkingSet(NULL), m_leafNodesCount(0U)
{
}

CssWriter::~CssWriter()
{
    Clear();
}

void CssWriter::Clear()
{
    m_callSites.clear();
    m_leafNodes.clear();
    m_leafNodesCount = 0U;
    m_pCallGraph = NULL;
    m_pWorkingSet = NULL;
}

bool CssWriter::Open(const wchar_t* pFilePath)
{
    return m_outputFile.open(pFilePath, FMODE_TEXT("wb"));
}

bool CssWriter::Write(CallGraph& callGraph, ProcessWorkingSetQuery& workingSet, gtUInt32 processId)
{
    Clear();
    m_pCallGraph = &callGraph;
    m_pWorkingSet = &workingSet;

    bool ret = m_outputFile.isOpened();

    if (ret)
    {
        CssFileHeader header;

        header.m_signature = CSS_FILE_SIGNATURE;
        header.m_versionValue = CALL_GRAPH_FILE_VERSION;
        header.m_processId = processId;

        header.m_callEdgeCount = callGraph.GetCallStacksCount();

        // We will rewrite this value of the header later (when it will be available)
        header.m_modInfoOffset = 0;
        header.m_modItemCount = 0;

        m_outputFile.write(header);

        gtUInt64 data64;

        // Write the size (in bytes) of the Character Set used
        data64 = sizeof(wchar_t);
        m_outputFile.write(data64);

        // Write Call-Stacks Array
        unsigned index = 0U;

        for (CallGraph::stack_iterator it = callGraph.GetBeginCallStack(), itEnd = callGraph.GetEndCallStack(); it != itEnd; ++it)
        {
            CallStack& callStack = **it;
            WriteCallStackRecord(callStack, ++index);
        }

        // Write the number of leaf nodes
        data64 = m_leafNodesCount;
        m_outputFile.write(data64);

        // Write Leaf-Nodes Array
        for (LeafNodesMap::iterator it = m_leafNodes.begin(), itEnd = m_leafNodes.end(); it != itEnd;)
        {
            gtUInt64 totalEventCount = it->second.m_pSample->m_count;
            LeafNodesMap::iterator itBegin = it;
            CallSite* pCallSite = itBegin->first;

            while (++it != itEnd)
            {
                if (pCallSite != it->first)
                {
                    break;
                }

                totalEventCount += it->second.m_pSample->m_count;
            }

            WriteLeafNodeRecord(itBegin, it, totalEventCount);
        }


        data64 = m_callSites.size();
        m_outputFile.write(data64);

        // Write Call-Sites Array
        for (CallSiteIndexMap::iterator it = m_callSites.begin(), itEnd = m_callSites.end(); it != itEnd; ++it)
        {
            WriteCallSiteRecord(*it->first, it->second);
        }

        data64 = 0ULL;
        // Write Functions Array
        m_outputFile.write(data64);
        // Write Symbols Array
        m_outputFile.write(data64);
        // Write Source-Files Array
        m_outputFile.write(data64);

        long modInfoOffset;
        m_outputFile.currentPosition(modInfoOffset);
        header.m_modInfoOffset = modInfoOffset;
        header.m_modItemCount = workingSet.ForeachModule(WriteModuleRecordCallback, this);

        m_outputFile.seekCurrentPosition(CrtFile::ORIGIN_BEGIN, offsetof(CssFileHeader, m_modInfoOffset));
        m_outputFile.write(header.m_modInfoOffset);
        m_outputFile.seekCurrentPosition(CrtFile::ORIGIN_BEGIN, offsetof(CssFileHeader, m_modItemCount));
        m_outputFile.write(header.m_modItemCount);
    }

    return ret;
}

unsigned CssWriter::AcquireCallSiteIndex(CallSite* pCallSite)
{
    unsigned index;

    CallSiteIndexMap::iterator it = m_callSites.find(pCallSite);

    if (m_callSites.end() == it)
    {
        index = m_callSites.size() + 1;
        m_callSites.insert(CallSiteIndexMap::value_type(pCallSite, index));
    }
    else
    {
        index = it->second;
    }

    return index;
}

unsigned CssWriter::AcquireLeafNodeIndex(const EventSampleInfo* pSample, CallStack* pCallStack)
{
    LeafNodeInfo info;
    info.m_pSample = pSample;
    info.m_pCallStack = pCallStack;

    LeafNodesMap::iterator it = m_leafNodes.find(pSample->m_pSite);

    if (m_leafNodes.end() == it)
    {
        info.m_index = ++m_leafNodesCount;
    }
    else
    {
        info.m_index = it->second.m_index;
    }

    m_leafNodes.insert(LeafNodesMap::value_type(pSample->m_pSite, info));

    return info.m_index;
}

unsigned CssWriter::FindCallStackIndex(CallStack* pCallStack) const
{
    unsigned index = 0U;

    if (0U != pCallStack->GetDepth())
    {
        PathIndexSet& callStackIndices = pCallStack->begin()->m_callStackIndices;

        for (PathIndexSet::const_iterator it = callStackIndices.begin(), itEnd = callStackIndices.end(); it != itEnd; ++it)
        {
            if (m_pCallGraph->GetCallStack(*it) == pCallStack)
            {
                index = *it + 1U;
                break;
            }
        }
    }
    else
    {
        if (NULL != m_pCallGraph->GetEmptyCallStack(index))
        {
            index++;
        }
    }

    return index;
}

bool CssWriter::WriteCallStackRecord(CallStack& callStack, unsigned index)
{
    gtUInt64 data64;

    // Write the call-stack's index number
    data64 = index;
    m_outputFile.write(data64);

    // Write the number of call-sites
    data64 = callStack.GetDepth();
    m_outputFile.write(data64);

    // Write the call-sites
    for (CallStack::iterator it = callStack.begin(), itEnd = callStack.end(); it != itEnd; ++it)
    {
        CallSite& callSite = *it;

        // Write the parent module's base address
        ExecutableFile* pExe = m_pWorkingSet->FindModule(callSite.m_traverseAddr);
        data64 = (NULL != pExe) ? pExe->GetLoadAddress() : 0ULL;
        m_outputFile.write(data64);

        // Write the call-site's address
        data64 = callSite.m_traverseAddr;
        m_outputFile.write(data64);

        // Write the call-site's index number
        data64 = AcquireCallSiteIndex(&callSite);
        m_outputFile.write(data64);
    }


    // Reserve space for the events' number value
    m_outputFile.write(data64);

    gtUInt64 selfTicks = 0;
    unsigned countEvents = 0U;
    gtSet<unsigned> leafNodeIndices;
    const EventSampleList& samples = callStack.GetEventSampleList();

    if (!samples.IsEmpty())
    {
        EventSampleList::const_iterator it = samples.begin(), itEnd = samples.end();

        leafNodeIndices.insert(AcquireLeafNodeIndex(&*it, &callStack));

        // The list is guaranteed to be sorted by Event ID
        gtUInt32 eventId = it->m_eventId;
        gtUInt64 aggCount = it->m_count;

        while (++it != itEnd)
        {
            const EventSampleInfo& sampleInfo = *it;

            leafNodeIndices.insert(AcquireLeafNodeIndex(&sampleInfo, &callStack));

            if (sampleInfo.m_eventId != eventId)
            {
                data64 = eventId;
                m_outputFile.write(data64);
                m_outputFile.write(aggCount);

                selfTicks += aggCount;
                eventId = sampleInfo.m_eventId;
                aggCount = sampleInfo.m_count;

                countEvents++;
            }
            else
            {
                aggCount += sampleInfo.m_count;
            }
        }

        data64 = eventId;
        m_outputFile.write(data64);
        m_outputFile.write(aggCount);

        selfTicks += aggCount;

        countEvents++;
    }

    // Write the number of events
    data64 = countEvents;
    RewriteDataValue(m_outputFile, data64, countEvents * (2 * sizeof(gtUInt64)));

    // Write self ticks count
    m_outputFile.write(selfTicks);
    // Write the number of times this object was observed
    m_outputFile.write(selfTicks);

    // Write the number of leaf nodes
    data64 = leafNodeIndices.size();
    m_outputFile.write(data64);

    // Write the leaf nodes
    for (gtSet<unsigned>::const_iterator it = leafNodeIndices.begin(), itEnd = leafNodeIndices.end(); it != itEnd; ++it)
    {
        data64 = *it;
        m_outputFile.write(data64);
    }

    return true;
}

bool CssWriter::WriteLeafNodeRecord(LeafNodesMap::iterator itBegin, LeafNodesMap::iterator itEnd, gtUInt64 totalEventCount)
{
    gtUInt64 data64;
    CallSite* pCallSite = itBegin->first;

    // Write the parent module's base address
    ExecutableFile* pExe = m_pWorkingSet->FindModule(pCallSite->m_traverseAddr);
    data64 = (NULL != pExe) ? pExe->GetLoadAddress() : 0ULL;
    m_outputFile.write(data64);

    // Write the leaf-node's address
    data64 = pCallSite->m_traverseAddr;
    m_outputFile.write(data64);

    // Write the number of times this object was observed
    m_outputFile.write(totalEventCount);

    // Write the leaf-node's index
    data64 = itBegin->second.m_index;
    m_outputFile.write(data64);

    data64 = 0ULL;
    // Write the parent function's index
    m_outputFile.write(data64);
    // Write the associated source file's line number
    m_outputFile.write(data64);
    // Write the owning source file index
    m_outputFile.write(data64);

    unsigned count;
    typedef gtVector<std::pair<unsigned, LeafNodesMap::iterator> > CallStacksVec;
    CallStacksVec callStacksVec;
    callStacksVec.reserve(256);

    // Reserve space for the call-stacks' number value
    m_outputFile.write(data64);

    CallStack* pPrevCallStack = NULL;

    for (LeafNodesMap::iterator it = itBegin; it != itEnd; ++it)
    {
        if (pPrevCallStack != it->second.m_pCallStack)
        {
            pPrevCallStack = it->second.m_pCallStack;
            unsigned index = FindCallStackIndex(pPrevCallStack);
            callStacksVec.push_back(std::make_pair(index, it));
            data64 = index;
            m_outputFile.write(data64);
        }
    }

    // Write the number of call-stacks
    count = callStacksVec.size();
    data64 = count;
    RewriteDataValue(m_outputFile, data64, count * sizeof(gtUInt64));

    // Add anchor
    callStacksVec.push_back(std::make_pair(0U, itEnd));


    // Reserve space for the events' number value
    m_outputFile.write(data64);

    // Write the aggregated events info
    count = 0U;

    if (callStacksVec.size() > 1)
    {
        CallStacksVec callStacksIts = callStacksVec;
        gtUInt32 eventId = 0;

        do
        {
            gtUInt32 eventIdNext = gtUInt32(-1);
            gtUInt64 aggCount = 0ULL;

            for (CallStacksVec::iterator itCs = callStacksIts.begin(), itCsEnd = callStacksIts.end() - 1,
                 itCsRangeEnd = callStacksVec.begin() + 1; itCs != itCsEnd; ++itCs, ++itCsRangeEnd)
            {
                for (LeafNodesMap::iterator& it = itCs->second, itrEnd = itCsRangeEnd->second; it != itrEnd; ++it)
                {
                    const EventSampleInfo* pSample = it->second.m_pSample;

                    if (pSample->m_eventId == eventId)
                    {
                        aggCount += pSample->m_count;
                    }
                    else
                    {
                        if (pSample->m_eventId < eventIdNext)
                        {
                            eventIdNext = pSample->m_eventId;
                        }

                        break;
                    }
                }
            }

            if (0ULL != aggCount)
            {
                data64 = eventId;
                m_outputFile.write(data64);
                m_outputFile.write(aggCount);

                count++;
            }

            eventId = eventIdNext;
        }
        while (gtUInt32(-1) != eventId);
    }

    // Write the number of events
    data64 = count;
    RewriteDataValue(m_outputFile, data64, count * (2 * sizeof(gtUInt64)));


    // Reserve space for the thread filtered events' number value
    m_outputFile.write(data64);

    // Write the thread filtered events info
    count = 0U;

    if (callStacksVec.size() > 1)
    {
        CallStacksVec callStacksIts = callStacksVec;
        gtUInt32 eventId = 0;
        gtUInt32 threadId = 0;

        do
        {
            gtUInt32 eventIdNext = gtUInt32(-1);
            gtUInt32 threadIdNext = gtUInt32(-1);
            gtUInt64 aggCount = 0ULL;

            for (CallStacksVec::iterator itCs = callStacksIts.begin(), itCsEnd = callStacksIts.end() - 1,
                 itCsRangeEnd = callStacksVec.begin() + 1; itCs != itCsEnd; ++itCs, ++itCsRangeEnd)
            {
                for (LeafNodesMap::iterator& it = itCs->second, itrEnd = itCsRangeEnd->second; it != itrEnd; ++it)
                {
                    const EventSampleInfo* pSample = it->second.m_pSample;

                    if (pSample->m_eventId == eventId)
                    {
                        if (pSample->m_threadId == threadId)
                        {
                            aggCount += pSample->m_count;
                        }
                        else
                        {
                            if (pSample->m_threadId < threadIdNext)
                            {
                                eventIdNext = eventId;
                                threadIdNext = pSample->m_threadId;
                            }

                            break;
                        }
                    }
                    else
                    {
                        if (pSample->m_eventId < eventIdNext)
                        {
                            eventIdNext = pSample->m_eventId;
                            threadIdNext = gtUInt32(-1);
                        }

                        break;
                    }
                }
            }

            if (0ULL != aggCount)
            {
                data64 = eventId;
                m_outputFile.write(data64);
                m_outputFile.write(aggCount);
                data64 = threadId;
                m_outputFile.write(data64);

                count++;
            }

            eventId = eventIdNext;
            threadId = threadIdNext;
        }
        while (gtUInt32(-1) != eventId);
    }

    // Write the number of thread filtered events
    data64 = count;
    RewriteDataValue(m_outputFile, data64, count * (3 * sizeof(gtUInt64)));


    // Reserve space for the call-stack filtered events' number value
    m_outputFile.write(data64);

    // Write the call-stack filtered events info
    count = 0U;

    if (callStacksVec.size() > 1)
    {
        for (CallStacksVec::iterator itCs = callStacksVec.begin(), itCsEnd = callStacksVec.end() - 1; itCs != itCsEnd; ++itCs)
        {
            gtUInt32 eventId = 0;
            gtUInt64 aggCount = 0ULL;
            LeafNodesMap::iterator it = itCs->second, itrEnd = (itCs + 1)->second;

            while (it != itrEnd)
            {
                const EventSampleInfo* pSample = it->second.m_pSample;

                if (pSample->m_eventId == eventId)
                {
                    aggCount += pSample->m_count;
                    ++it;
                }
                else
                {
                    if (0ULL != aggCount)
                    {
                        data64 = itCs->first;
                        m_outputFile.write(data64);
                        data64 = eventId;
                        m_outputFile.write(data64);
                        m_outputFile.write(aggCount);

                        count++;
                        aggCount = 0ULL;
                    }

                    eventId = pSample->m_eventId;
                }
            }

            if (0ULL != aggCount)
            {
                data64 = itCs->first;
                m_outputFile.write(data64);
                data64 = eventId;
                m_outputFile.write(data64);
                m_outputFile.write(aggCount);

                count++;
            }
        }
    }

    // Write the number of call-stack filtered events
    data64 = count;
    RewriteDataValue(m_outputFile, data64, count * (3 * sizeof(gtUInt64)));

    return true;
}

bool CssWriter::WriteCallSiteRecord(CallSite& callSite, unsigned index)
{
    gtUInt64 data64;

    // Write the parent module's base address
    ExecutableFile* pExe = m_pWorkingSet->FindModule(callSite.m_traverseAddr);
    data64 = (NULL != pExe) ? pExe->GetLoadAddress() : 0ULL;
    m_outputFile.write(data64);

    // Write the call-site's address
    data64 = callSite.m_traverseAddr;
    m_outputFile.write(data64);

    // Write the call-site's index number
    data64 = index;
    m_outputFile.write(data64);

    gtUInt64 totalEventCount = 0ULL;
    // Reserve space for the number of times this object was observed value
    m_outputFile.write(totalEventCount);

    data64 = 0ULL;
    // Write the parent function's index
    m_outputFile.write(data64);
    // Write the associated source file's line number
    m_outputFile.write(data64);
    // Write the owning source file index
    m_outputFile.write(data64);


    // Reserve space for the call-stacks' number value
    m_outputFile.write(data64);

    // Write the call-stacks info
    unsigned count = 0U;
    std::pair<LeafNodesMap::iterator, LeafNodesMap::iterator> range = m_leafNodes.equal_range(&callSite);

    for (PathIndexSet::const_iterator it = callSite.m_callStackIndices.begin(), itEnd = callSite.m_callStackIndices.end(); it != itEnd; ++it)
    {
        index = *it;
        CallStack* pCallStack = m_pCallGraph->GetCallStack(index);

        if (NULL != pCallStack)
        {
            LeafNodesMap::iterator itr = range.first;

            while (itr != range.second)
            {
                if (itr->second.m_pCallStack == pCallStack)
                {
                    break;
                }

                ++itr;
            }

            if (itr == range.second)
            {
                data64 = index + 1;
                m_outputFile.write(data64);

                count++;

                const EventSampleList& samples = pCallStack->GetEventSampleList();

                for (EventSampleList::const_iterator itSample = samples.begin(), itSampleEnd = samples.end();
                     itSample != itSampleEnd; ++itSample)
                {
                    totalEventCount += itSample->m_count;
                }
            }
        }
    }

    // Write the number of call-stack filtered events
    data64 = count;
    count *= sizeof(gtUInt64);
    RewriteDataValue(m_outputFile, data64, count);

    // Write the number of times this object was observed
    RewriteDataValue(m_outputFile, totalEventCount, count + (4 * sizeof(gtUInt64)));

    return true;
}

bool CssWriter::WriteModuleRecord(ExecutableFile& exe)
{
    gtUInt64 imageBase = exe.GetLoadAddress();
    m_outputFile.write(imageBase);

    const wchar_t* pImageName = exe.GetFilePath();
    gtUInt32 imageNameSize = (wcslen(pImageName) + 1) * sizeof(wchar_t);

    m_outputFile.write(imageNameSize);
    m_outputFile.write(pImageName, imageNameSize);

    return true;
}

void CssWriter::WriteModuleRecordCallback(ExecutableFile& exe, void* pContext)
{
    static_cast<CssWriter*>(pContext)->WriteModuleRecord(exe);
}
