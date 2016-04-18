//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file CssReader.h
///
//==================================================================================

#ifndef _CSSREADER_H_
#define _CSSREADER_H_

#include <AMDTBaseTools/Include/gtVector.h>
#include <ProfilingAgents/Utils/CrtFile.h>
#include "FunctionGraph.h"

class CssCallback
{
public:
    virtual ~CssCallback() {}

    virtual bool AddModule(gtVAddr base, const wchar_t* pPath) = 0;
    virtual bool AddFunction(gtVAddr va, gtVAddr& startVa, gtVAddr& endVa) = 0;
    virtual bool AddMetadata(gtVAddr va, void** ppMetadata = NULL) = 0;
};

class CP_CSS_API CssReader
{
public:
    CssReader(unsigned maxCallStackDepth);
    ~CssReader();
    CssReader& operator=(const CssReader&) = delete;

    bool Open(const wchar_t* pFilePath);

    bool Read(FunctionGraph& funcGraph, gtUInt32 processId = 0);

    void SetCallback(CssCallback* pCallback) { m_pCallback = pCallback; }

private:
    bool ReadCallStackRecord();
    bool ReadLeafNodeRecord();
    bool ReadCallSiteRecord();
    bool ReadModuleRecord();

    FunctionGraph::Node& AcquireFunctionNode(gtVAddr va);

    void Clear();

    typedef std::pair<FunctionPathBuilder*, unsigned> FunctionPathBuildInfo;

    CrtFile m_inputFile;
    FunctionGraph* m_pFuncGraph;
    gtVector<FunctionPathBuildInfo> m_pathBuilders;

    const unsigned m_maxDepth;

#if defined(CSS_FILE_NOT_STRICT)
    unsigned m_sizeofChar;
#endif

    CssCallback* m_pCallback;
};

#endif // _CSSREADER_H_
