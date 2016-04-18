//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file CGCallback.h
///
//==================================================================================

#ifndef _CALLGRAPH_H
#define _CALLGRAPH_H

#include <QtCore>

#include <AMDTBaseTools/Include/gtString.h>
#include <AMDTOSWrappers/Include/osFilePath.h>
#include <AMDTOSWrappers/Include/osFile.h>
#include <AMDTExecutableFormat/inc/ExecutableFile.h>
#include <AMDTCpuCallstackSampling/inc/CssReader.h>
#include <AMDTCpuPerfEventUtils/inc/EventEncoding.h>

class CpuProfileFunction;
class CpuProfileModule;
class CpuProfileReader;

struct CGFunctionMetaData
{
    CpuProfileModule*    m_pModule;
    CpuProfileFunction*  m_pFunction;

    gtUInt64             m_selfCount;
    gtUInt64             m_deepCount;
    gtUInt32             m_pathCount;

    gtString             m_funcName;
};

struct CGModuleInfo
{
    CpuProfileModule&  m_module;
    ExecutableFile*    m_pExecutable;

    CGModuleInfo(CpuProfileModule& module, ExecutableFile* pExecutable) : m_module(module), m_pExecutable(pExecutable)
    {
        if (NULL != m_pExecutable && NULL == m_pExecutable->GetSymbolEngine())
        {
            delete m_pExecutable;
            m_pExecutable = NULL;
        }
    }

    void Clear()
    {
        if (NULL != m_pExecutable)
        {
            delete m_pExecutable;
            m_pExecutable = NULL;
        }
    }

    CGModuleInfo& operator=(const CGModuleInfo&) = delete;
};

typedef gtMap<gtVAddr, CGModuleInfo> AddrModuleInfoMap;

class CGCallback : public CssCallback
{
public:
    CGCallback(CpuProfileReader* pProfileReader);
    ~CGCallback();

    bool Initialize(const gtString& cssFilePath, gtUInt32 pid);
    void Finalize();

    const FunctionGraph& GetFunctionGraph() const { return m_funcGraph; }
    FunctionGraph& GetFunctionGraph() { return m_funcGraph; }

    const CpuProfileModule* FindModule(gtVAddr va) const;
    CpuProfileModule* FindModule(gtVAddr va);

    const CpuProfileFunction* FindFunction(gtVAddr va) const;
    CpuProfileFunction* FindFunction(gtVAddr va);

    EventMaskType GetEventId() const { return m_event; }
    void SetEventId(EventMaskType encodedEvent) { m_event = encodedEvent; }

    // Callback to CssCallback class
    virtual bool AddModule(gtVAddr base, const wchar_t* pPath);
    virtual bool AddFunction(gtVAddr va, gtVAddr& startVa, gtVAddr& endVa);
    virtual bool AddMetadata(gtVAddr va, void** ppMetadata = NULL); // This function must be called right after calling AddFunction

private:
    CpuProfileReader*    m_pProfileReader;

    AddrModuleInfoMap    m_addrModuleMap;

    FunctionGraph        m_funcGraph;
    EventMaskType        m_event;       // Encoded event eventSel + unitmask + os + user ?

    CpuProfileModule*    m_pCurModule;
    CpuProfileFunction*  m_pCurFunction;
};

#endif // _CALLGRAPH_H