//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file CssReader.cpp
///
//==================================================================================

#include <CssReader.h>
#include <CssFile.h>
#include <AMDTOSWrappers/Include/osOSDefinitions.h>

CssReader::CssReader(unsigned maxCallStackDepth) : m_pFuncGraph(NULL), m_maxDepth(maxCallStackDepth)
{
}

CssReader::~CssReader()
{
    Clear();
}

void CssReader::Clear()
{
    for (gtVector<FunctionPathBuildInfo>::iterator it = m_pathBuilders.begin(), itEnd = m_pathBuilders.end(); it != itEnd; ++it)
    {
        FunctionPathBuilder* pPathBuilder = it->first;

        if (NULL != pPathBuilder)
        {
            gtUByte* pBuffer = reinterpret_cast<gtUByte*>(pPathBuilder);
            delete [] pBuffer;
        }
    }

    m_pFuncGraph = NULL;
}

bool CssReader::Open(const wchar_t* pFilePath)
{
    return m_inputFile.open(pFilePath, FMODE_TEXT("rb"));
}

bool CssReader::Read(FunctionGraph& funcGraph, gtUInt32 processId)
{
    Clear();
    m_pFuncGraph = &funcGraph;

    bool ret = false;

    if (m_inputFile.isOpened())
    {
        CssFileHeader header;
        m_inputFile.read(header);

        if (CSS_FILE_SIGNATURE == header.m_signature &&
            CALL_PATH_FILE_VERSION <= header.m_versionValue &&
            (0 == processId || header.m_processId == processId))
        {
            gtUInt64 data64;

            m_inputFile.read(data64);
#if defined(CSS_FILE_NOT_STRICT)
            m_sizeofChar = static_cast<unsigned>(data64);

            if (1U == m_sizeofChar || 2U == m_sizeofChar || 4U == m_sizeofChar)
#else
            if (static_cast<gtUInt64>(sizeof(wchar_t)) == data64)
#endif
            {
                ret = true;

                if (0U != header.m_modItemCount)
                {
                    m_inputFile.seekCurrentPosition(CrtFile::ORIGIN_BEGIN, static_cast<long>(header.m_modInfoOffset));

                    for (unsigned i = 0U, count = header.m_modItemCount; i < count; ++i)
                    {
                        ReadModuleRecord();
                    }

                    m_inputFile.seekCurrentPosition(CrtFile::ORIGIN_BEGIN, static_cast<long>(sizeof(header) + sizeof(gtUInt64)));
                }

                if (0U != header.m_callEdgeCount)
                {
                    m_pathBuilders.reserve(header.m_callEdgeCount + (header.m_callEdgeCount / 2));
                    m_pathBuilders.resize(header.m_callEdgeCount);

                    for (unsigned i = 0U, count = header.m_callEdgeCount; i < count; ++i)
                    {
                        if (!ReadCallStackRecord())
                        {
                            ret = false;
                            break;
                        }
                    }
                }

                if (ret)
                {
                    m_inputFile.read(data64);

                    for (unsigned i = 0U, count = static_cast<unsigned>(data64); i < count; ++i)
                    {
                        if (!ReadLeafNodeRecord())
                        {
                            ret = false;
                            break;
                        }
                    }

                    if (ret && NULL != m_pCallback)
                    {
                        m_inputFile.read(data64);

                        for (unsigned i = 0U, count = static_cast<unsigned>(data64); i < count; ++i)
                        {
                            ReadCallSiteRecord();
                        }
                    }
                }
            }
        }
    }

    return ret;
}

bool CssReader::ReadCallStackRecord()
{
    gtUInt64 data64;

    m_inputFile.read(data64);
    unsigned stackIndex = static_cast<unsigned>(data64);

    m_inputFile.read(data64);
    unsigned depth = static_cast<unsigned>(data64);

    bool ret = (m_maxDepth >= depth);

    if (ret)
    {
        unsigned bufferSize = FunctionPathBuilder::CalcRequiredBufferSize(depth);
        gtUByte* pBuffer = new gtUByte[sizeof(FunctionPathBuilder) + bufferSize];
        FunctionPathBuilder* pPathBuilder = new(pBuffer) FunctionPathBuilder(pBuffer + sizeof(FunctionPathBuilder), bufferSize);

        if (static_cast<unsigned>(m_pathBuilders.size()) < stackIndex)
        {
            m_pathBuilders.resize(stackIndex);
        }

        stackIndex--;
        m_pathBuilders[stackIndex].first = pPathBuilder;
        m_pathBuilders[stackIndex].second = unsigned(-1);

        if (0U < depth)
        {
            m_inputFile.read(data64);

            m_inputFile.read(data64);
            gtVAddr siteAddr = data64;

            m_inputFile.read(data64); // Call site index

            pPathBuilder->Initialize(AcquireFunctionNode(siteAddr));

            for (unsigned i = 1U; i < depth; ++i)
            {
                m_inputFile.read(data64);

                m_inputFile.read(data64);
                siteAddr = data64;

                m_inputFile.read(data64); // Call site index

                pPathBuilder->Push(AcquireFunctionNode(siteAddr));
            }
        }

        m_inputFile.read(data64);
        m_inputFile.seekCurrentPosition(CrtFile::ORIGIN_CURRENT,
                                        (static_cast<long>(data64) * (2 * sizeof(gtUInt64))) + // Skip events array
                                        sizeof(gtUInt64) + // Skip self ticks
                                        sizeof(gtUInt64)); // Skip times observed

        m_inputFile.read(data64);
        m_inputFile.seekCurrentPosition(CrtFile::ORIGIN_CURRENT,
                                        static_cast<long>(data64) * sizeof(gtUInt64)); // Skip leaf nodes array
    }

    return ret;
}

bool CssReader::ReadLeafNodeRecord()
{
    bool ret = true;
    gtUInt64 data64;
    LeafFunction leafNode;

    m_inputFile.read(data64);

    m_inputFile.read(data64);
    leafNode.m_pNode = &AcquireFunctionNode(data64);

    m_inputFile.seekCurrentPosition(CrtFile::ORIGIN_CURRENT,
                                    sizeof(gtUInt64) + // Skip times observed
                                    sizeof(gtUInt64) + // Skip leaf index number
                                    sizeof(gtUInt64) + // Skip function index number
                                    sizeof(gtUInt64) + // Skip source line number
                                    sizeof(gtUInt64)); // Skip source file index number

    m_inputFile.read(data64);
    m_inputFile.seekCurrentPosition(CrtFile::ORIGIN_CURRENT,
                                    static_cast<long>(data64) * sizeof(gtUInt64)); // Skip call-stacks array

    m_inputFile.read(data64);
    m_inputFile.seekCurrentPosition(CrtFile::ORIGIN_CURRENT,
                                    static_cast<long>(data64) * (2 * sizeof(gtUInt64))); // Skip events array

    m_inputFile.read(data64);
    m_inputFile.seekCurrentPosition(CrtFile::ORIGIN_CURRENT,
                                    static_cast<long>(data64) * (3 * sizeof(gtUInt64))); // Skip thread events array

    m_inputFile.read(data64);

    for (unsigned i = 0U, count = static_cast<unsigned>(data64); i < count; ++i)
    {
        m_inputFile.read(data64);
        unsigned stackIndex = static_cast<unsigned>(data64) - 1U;

        m_inputFile.read(data64);
        leafNode.m_eventId = static_cast<gtUInt32>(data64);

        m_inputFile.read(data64);
        leafNode.m_count = static_cast<gtUInt32>(data64);

        if (static_cast<unsigned>(m_pathBuilders.size()) > stackIndex)
        {
            FunctionPathBuildInfo& pathInfo = m_pathBuilders[stackIndex];

            if (unsigned(-1) == pathInfo.second)
            {
                FunctionPathBuilder* pPathBuilder = m_pathBuilders[stackIndex].first;
                pathInfo.second = pPathBuilder->Finalize(*m_pFuncGraph, leafNode);
            }
            else
            {
                FunctionGraph::Path& path = *m_pFuncGraph->GetPath(pathInfo.second);

                if (0U != path.GetLength())
                {
                    path.begin()->AddWalkNext(leafNode.m_pNode);
                }

                FunctionGraph::AddLeafNode(path, leafNode);
                leafNode.m_pNode->m_pathIndices.Add(pathInfo.second);
            }
        }
        else
        {
            ret = false;
            break;
        }
    }

    return ret;
}

bool CssReader::ReadCallSiteRecord()
{
    m_inputFile.seekCurrentPosition(CrtFile::ORIGIN_CURRENT,
                                    sizeof(gtUInt64) + // Skip parent module base address
                                    sizeof(gtUInt64) + // Skip call-site address
                                    sizeof(gtUInt64) + // Skip call-site index number
                                    sizeof(gtUInt64) + // Skip times observed
                                    sizeof(gtUInt64) + // Skip function index number
                                    sizeof(gtUInt64) + // Skip source line number
                                    sizeof(gtUInt64)); // Skip source file index number

    gtUInt64 countStacks;
    m_inputFile.read(countStacks);

    if (0ULL != countStacks)
    {
        m_inputFile.seekCurrentPosition(CrtFile::ORIGIN_CURRENT,
                                        static_cast<long>(countStacks) * sizeof(gtUInt64)); // Skip call-stack indices list
    }

    return true;
}

bool CssReader::ReadModuleRecord()
{
    gtUInt64 imageBase;
    m_inputFile.read(imageBase);

    gtUInt32 imageNameSize;
    m_inputFile.read(imageNameSize);

    bool ret = (0ULL != imageNameSize);

    if (ret)
    {
#if defined(CSS_FILE_NOT_STRICT)
        gtUInt32 imageNameBuf[OS_MAX_PATH];

        gtUInt32 imageNameLen = imageNameSize / m_sizeofChar;

        if (OS_MAX_PATH > (imageNameLen - 1))
        {
            m_inputFile.read(imageNameBuf, imageNameSize);
#else
        wchar_t imageName[OS_MAX_PATH];

        if (sizeof(imageName) > (imageNameSize - 1))
        {
            m_inputFile.read(imageName, imageNameSize);
#endif

#if defined(CSS_FILE_NOT_STRICT)
            wchar_t imageNameConv[OS_MAX_PATH];
            wchar_t* imageName;

            if (sizeof(wchar_t) == m_sizeofChar)
            {
                imageName = reinterpret_cast<wchar_t*>(imageNameBuf);
            }
            else
            {
                imageNameConv[0] = L'\0';
                imageNameBuf[OS_MAX_PATH - 1] = 0;

                if (1U == m_sizeofChar)
                {
                    mbstowcs(imageNameConv, reinterpret_cast<char*>(imageNameBuf), OS_MAX_PATH);
                }
                else
                {
                    gtUByte* pBuf = reinterpret_cast<gtUByte*>(imageNameBuf);

                    for (gtUInt32 i = 0; i < imageNameLen; ++i)
                    {
                        //NOTE: This is working only on Little Endian architecture!
                        memcpy(&imageNameConv[i], pBuf, m_sizeofChar);
                        pBuf += m_sizeofChar;
                    }
                }

                imageName = imageNameConv;
            }

#endif

            imageName[OS_MAX_PATH - 1] = L'\0';

            m_pCallback->AddModule(imageBase, imageName);
        }
        else
        {
            m_inputFile.seekCurrentPosition(CrtFile::ORIGIN_CURRENT, static_cast<long>(imageNameSize));
            ret = false;
        }
    }

    return ret;
}

FunctionGraph::Node& CssReader::AcquireFunctionNode(gtVAddr va)
{
    gtVAddr startVa, endVa;

    if (NULL != m_pCallback && m_pCallback->AddFunction(va, startVa, endVa) && va < endVa)
    {
        va = startVa;
    }

    FunctionGraph::node_iterator it = m_pFuncGraph->FindNode(va);

    if (m_pFuncGraph->GetEndNode() == it)
    {
        it = m_pFuncGraph->InsertNode(va);

        if (NULL != m_pCallback)
        {
            m_pCallback->AddMetadata(va, &it->m_val);
        }
    }

    return *it;
}
