//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file CpuProfileCss.h
///
//==================================================================================

#ifndef _CPUPROFILECSS_H
#define _CPUPROFILECSS_H

#include "StdAfx.h"
#include <AMDTExecutableFormat/inc/ExecutableFile.h>
#include <AMDTCpuCallstackSampling/inc/CssReader.h>
#include <AMDTCpuPerfEventUtils/inc/EventEncoding.h>

class CpuProfileFunction;
class CpuProfileModule;
class CpuProfileReader;

class CpuSessionWindow;
class SessionTreeNodeData;


struct CssFunctionMetadata
{
    CpuProfileModule* m_pModule;
    CpuProfileFunction* m_pFunction;
    gtUInt64 m_selfCount;
    gtUInt64 m_deepCount;
    gtUInt64 m_childPathCount;
    gtUInt64 m_parentPathCount;
    unsigned m_childPathIndex;
    unsigned m_parentPathIndex;
    QString m_funcName;
};

class CpuProfileCss : public CssCallback
{
private:
    class ModuleInfo
    {
    public:
        ModuleInfo(CpuProfileModule& module, ExecutableFile* pExecutable = nullptr);

        const CpuProfileModule& GetModule() const { return m_module; }
        CpuProfileModule& GetModule()       { return m_module; }

        const ExecutableFile* GetExecutableFile() const { return m_pExecutable; }
        ExecutableFile* GetExecutableFile()       { return m_pExecutable; }

        bool SyncWithSymbolEngine(CpuProfileReader& profileReader, const QString& sessionDir);
        void DestroySymbolEngine();

    private:
        ModuleInfo& operator=(ModuleInfo const&) = delete;

        CpuProfileModule& m_module;
        ExecutableFile* m_pExecutable;
    };

    typedef gtMap<gtVAddr, ModuleInfo> ModulesMap;

    CpuSessionWindow& m_sessionWindow;
    SessionTreeNodeData& m_sessionData;

    ModulesMap m_modulesMap;
    FunctionGraph m_funcGraph;
    EventMaskType m_eventId;

    CpuProfileModule* m_pLastAddedModule;
    CpuProfileFunction* m_pLastAddedFunction;

    void Finalize();

public:
    CpuProfileCss(CpuSessionWindow& sessionWindow, SessionTreeNodeData& sessionData);
    ~CpuProfileCss();

    bool Initialize(const QString& filePath, gtUInt32 processId);

    const FunctionGraph& GetFunctionGraph() const { return m_funcGraph; }
    FunctionGraph& GetFunctionGraph()       { return m_funcGraph; }

    const CpuProfileModule* FindModule(gtVAddr va) const;
    CpuProfileModule* FindModule(gtVAddr va);
    const CpuProfileFunction* FindFunction(gtVAddr va) const;
    CpuProfileFunction* FindFunction(gtVAddr va);

    EventMaskType GetEventId() const { return m_eventId; }
    void SetEventId(EventMaskType eventId) { m_eventId = eventId; }


    //////////////////////////////////////////////////////////////////////////
    // CssCallback interface
    //////////////////////////////////////////////////////////////////////////
    virtual bool AddModule(gtVAddr base, const wchar_t* pPath);
    virtual bool AddFunction(gtVAddr va, gtVAddr& startVa, gtVAddr& endVa);
    virtual bool AddMetadata(gtVAddr va, void** ppMetadata = nullptr); // This function must be called right after calling AddFunction

private:
    CpuProfileCss& operator=(CpuProfileCss const&) = delete;

};


#endif // _CPUPROFILECSS_H
