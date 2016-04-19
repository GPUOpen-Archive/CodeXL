//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file CpuProfileCss.cpp
///
//==================================================================================

// Infra:
#include <AMDTApplicationComponents/Include/acFunctions.h>

// AMDTApplicationFramework:
#include <AMDTApplicationFramework/src/afUtils.h>

// AMDTSharedProfiling:
#include <AMDTSharedProfiling/inc/SessionTreeNodeData.h>

// Local:
#include <inc/CpuProfileCss.h>
#include <inc/Auxil.h>
#include <inc/SessionWindow.h>

CpuProfileCss::ModuleInfo::ModuleInfo(CpuProfileModule& module, ExecutableFile* pExecutable) : m_module(module),
    m_pExecutable(pExecutable)
{
    // Nothing to do with the ExecutableFile without symbols.
    if (nullptr != m_pExecutable && nullptr == m_pExecutable->GetSymbolEngine())
    {
        delete m_pExecutable;
        m_pExecutable = nullptr;
    }
}

bool CpuProfileCss::ModuleInfo::SyncWithSymbolEngine(CpuProfileReader& profileReader, const QString& sessionDir)
{
    m_pExecutable = nullptr;
    QString exePath;

    if (AuxGetExecutablePath(exePath,
                             profileReader,
                             sessionDir,
                             acGTStringToQString(m_module.getPath()),
                             nullptr,
                             &m_module))
    {
        m_pExecutable = ExecutableFile::Open(exePath.toStdWString().c_str(), m_module.getBaseAddr());

        if (nullptr != m_pExecutable)
        {
            // Initialize executable symbol engine:
            if (!AuxInitializeSymbolEngine(m_pExecutable))
            {
                delete m_pExecutable;
                m_pExecutable = nullptr;
            }
        }
    }

    return nullptr != m_pExecutable;
}

void CpuProfileCss::ModuleInfo::DestroySymbolEngine()
{
    if (nullptr != m_pExecutable)
    {
        delete m_pExecutable;
        m_pExecutable = nullptr;
    }
}


CpuProfileCss::CpuProfileCss(CpuSessionWindow& sessionWindow, SessionTreeNodeData& sessionData) : m_sessionWindow(sessionWindow),
    m_sessionData(sessionData),
    m_eventId(EventMaskType(-2)),
    m_pLastAddedModule(nullptr),
    m_pLastAddedFunction(nullptr)
{
}

CpuProfileCss::~CpuProfileCss()
{
    for (FunctionGraph::node_iterator it = m_funcGraph.GetBeginNode(), itEnd = m_funcGraph.GetEndNode(); it != itEnd; ++it)
    {
        CssFunctionMetadata* pMetadata = static_cast<CssFunctionMetadata*>(it->m_val);

        if (nullptr != pMetadata)
        {
            delete pMetadata;
        }
    }
}

bool CpuProfileCss::Initialize(const QString& filePath, gtUInt32 processId)
{
    // Each css file is <pid>.css
    QString cssFile = filePath + PATH_SLASH + QString::number(processId) + ".css";

    CssReader reader(CP_CSS_MAX_UNWIND_DEPTH);
    bool ret = reader.Open(cssFile.toStdWString().c_str());

    if (ret)
    {
        reader.SetCallback(this);
        ret = reader.Read(m_funcGraph, processId);
    }

    Finalize();
    return ret;
}

const CpuProfileModule* CpuProfileCss::FindModule(gtVAddr va) const
{
    ModulesMap::const_iterator it = m_modulesMap.upper_bound(va);
    return (m_modulesMap.begin() != it) ? &(--it)->second.GetModule() : nullptr;
}

CpuProfileModule* CpuProfileCss::FindModule(gtVAddr va)
{
    ModulesMap::iterator it = m_modulesMap.upper_bound(va);
    return (m_modulesMap.begin() != it) ? &(--it)->second.GetModule() : nullptr;
}

const CpuProfileFunction* CpuProfileCss::FindFunction(gtVAddr va) const
{
    const CpuProfileModule* pModule = FindModule(va);
    return (nullptr != pModule) ? pModule->findFunction(va) : nullptr;
}

CpuProfileFunction* CpuProfileCss::FindFunction(gtVAddr va)
{
    CpuProfileModule* pModule = FindModule(va);
    return (nullptr != pModule) ? pModule->findFunction(va) : nullptr;
}

void CpuProfileCss::Finalize()
{
    for (ModulesMap::iterator it = m_modulesMap.begin(), itEnd = m_modulesMap.end(); it != itEnd; ++it)
    {
        it->second.DestroySymbolEngine();
    }
}

bool CpuProfileCss::AddModule(gtVAddr base, const wchar_t* pPath)
{
    ExecutableFile* pExecutable = nullptr;

    int lenPath = static_cast<int>(wcslen(pPath));
    CpuProfileModule* pModule = m_sessionWindow.getModuleDetail(QString::fromWCharArray(pPath), nullptr, &pExecutable);

    if (nullptr == pModule)
    {
        gtString modulePath(pPath, lenPath);
        NameModuleMap* pNameModMap = m_sessionWindow.profileReader().getModuleMap();

        NameModuleMap::iterator it = pNameModMap->insert(NameModuleMap::value_type(modulePath, CpuProfileModule())).first;
        pModule = &it->second;

        pModule->setIndirect(true);
        pModule->setSystemModule(AuxIsSystemModule(modulePath));
        pModule->m_base = base;

        // Indirect modules do not have IMD files to read, so just treat the modules as already read (to prevent future read).
        pModule->m_isImdRead = true;

        pModule->m_modType = CpuProfileModule::UNMANAGEDPE;
        pModule->setPath(modulePath);

        m_sessionWindow.getModuleDetail(QString::fromWCharArray(pPath), nullptr, &pExecutable);
    }
    else
    {
        GT_ASSERT(pModule->getBaseAddr() == base);
    }

    if (nullptr != pExecutable && pModule->m_size < pExecutable->GetImageSize())
    {
        pModule->m_size = pExecutable->GetImageSize();
    }

    m_modulesMap.insert(ModulesMap::value_type(pModule->getBaseAddr(), ModuleInfo(*pModule, pExecutable)));
    return true;
}

bool CpuProfileCss::AddFunction(gtVAddr va, gtVAddr& startVa, gtVAddr& endVa)
{
    bool ret = false;

    CpuProfileModule* pModule = nullptr;
    CpuProfileFunction* pFunction = nullptr;

    ModulesMap::iterator m = m_modulesMap.upper_bound(va);

    ModuleInfo* pModInfo = nullptr;

    if (m_modulesMap.begin() != m)
    {
        // We have found a module which bounds the virtual address from the bottom.
        pModInfo = &(--m)->second;

        // Verify that the module also bounds the virtual address from the top.
        if (va >= (pModInfo->GetModule().getBaseAddr() + static_cast<gtVAddr>(pModInfo->GetModule().m_size)))
        {
            pModInfo = nullptr;
        }
    }

    // If no module found.
    if (nullptr == pModInfo)
    {
        // It might happen that the modules list in the CSS file was not complete.
        NameModuleMap* pNameModMap = m_sessionWindow.profileReader().getModuleMap();

        // Search for the containing module in the session's modules list.
        for (NameModuleMap::iterator it = pNameModMap->begin(), itEnd = pNameModMap->end(); it != itEnd; ++it)
        {
            CpuProfileModule& curMod = it->second;

            ExecutableFile* pExecutable = nullptr;

            // If the module has not been initialized (read from the corresponding IMD file).
            bool synch = !curMod.m_isImdRead;

            if (synch)
            {
                // Initialize the module information by requesting it from the session.
                m_sessionWindow.getModuleDetail(acGTStringToQString(curMod.getPath()), nullptr, &pExecutable);
            }

            // If the module contains the virtual address.
            if (curMod.getBaseAddr() <= va && va < (curMod.getBaseAddr() + static_cast<gtVAddr>(curMod.m_size)))
            {
                // If we have not requested the ExecutableFile of the module, then request it now.
                // This is done here and not as an 'else' for the "if (synch)" above, to prevent unnecessary openings of the
                // executable file (a heavy operation).
                if (!synch)
                {
                    m_sessionWindow.getModuleDetail(acGTStringToQString(curMod.getPath()), nullptr, &pExecutable);
                }

                // We have found a containing module; insert it to the modules map, for future searches.
                m = m_modulesMap.insert(ModulesMap::value_type(curMod.getBaseAddr(), ModuleInfo(curMod, pExecutable))).first;
                pModInfo = &m->second;
                break;
            }

            if (nullptr != pExecutable)
            {
                delete pExecutable;
            }
        }
    }

    if (nullptr != pModInfo)
    {
        pModule = &pModInfo->GetModule();

        AddrFunctionMultMap& funcMap = pModule->getFunctionMap();
        AddrFunctionMultMap::iterator f = funcMap.upper_bound(va);

        if (f != funcMap.begin())
        {
            pFunction = &(--f)->second;
        }

        if (nullptr == pFunction || pFunction->getFuncName().isEmpty() || !pFunction->contains(va) || pModule->isUnchartedFunction(*pFunction))
        {
            ExecutableFile* pExecutable = pModInfo->GetExecutableFile();

            if (nullptr != pExecutable)
            {
                SymbolEngine* pSymbolEngine = pExecutable->GetSymbolEngine();

                gtRVAddr funcRvaEnd = GT_INVALID_RVADDR;
                const FunctionSymbolInfo* pFuncSymbol = pSymbolEngine->LookupFunction(va - pModule->getBaseAddr(), &funcRvaEnd);

                if (nullptr != pFuncSymbol)
                {
                    gtVAddr funcVa = pModule->getBaseAddr() + static_cast<gtVAddr>(pFuncSymbol->m_rva);

                    gtUInt32 funcSize = pFuncSymbol->m_size;

                    if (0 == funcSize)
                    {
                        if (GT_INVALID_RVADDR != funcRvaEnd)
                        {
                            funcSize = funcRvaEnd - pFuncSymbol->m_rva;
                        }
                        else
                        {
                            funcSize = gtUInt32(-1);
                        }
                    }

                    if (nullptr == pFunction || pFunction->getBaseAddr() != funcVa)
                    {
                        gtString srcFileName;
                        SourceLineInfo sourceLine;

                        if (pSymbolEngine->FindSourceLine(pFuncSymbol->m_rva, sourceLine))
                        {
                            int srcFileNameLen = static_cast<int>(wcslen(sourceLine.m_filePath));

                            if (!afUtils::ConvertCygwinPath(sourceLine.m_filePath, srcFileNameLen, srcFileName))
                            {
                                srcFileName.assign(sourceLine.m_filePath, srcFileNameLen);
                            }
                        }
                        else
                        {
                            sourceLine.m_line = 0U;
                        }

                        CpuProfileFunction funcNew((nullptr != pFuncSymbol->m_pName) ? pFuncSymbol->m_pName : L"",
                                                   funcVa,
                                                   funcSize,
                                                   gtString(),
                                                   srcFileName,
                                                   sourceLine.m_line);
                        pFunction = &funcMap.insert(AddrFunctionMultMap::value_type(funcVa, funcNew))->second;
                    }
                    else
                    {
                        if (nullptr != pFuncSymbol->m_pName && pFunction->getFuncName().isEmpty())
                        {
                            pFunction->setFuncName(pFuncSymbol->m_pName);
                        }

                        pFunction->setSize(funcSize);
                    }
                }
            }

            // If we have found a candidate function but it does not contain the virtual address.
            if (nullptr != pFunction && !pModule->isUnchartedFunction(*pFunction) && (pFunction->getFuncName().isEmpty() || !pFunction->contains(va)))
            {
                pFunction = nullptr;
            }
        }

        if (nullptr == pFunction)
        {
            // If we did not find a matching function (that is, including the "uncharted function"),
            // then add the "uncharted function" to the module.
            //
            CpuProfileFunction funcNew;
            funcNew.setBaseAddr(pModule->getBaseAddr());
            funcMap.insert(AddrFunctionMultMap::value_type(pModule->getBaseAddr(), funcNew));
        }
        else
        {
            if (!pModule->isUnchartedFunction(*pFunction))
            {
                startVa = pFunction->getBaseAddr();
                endVa = pFunction->getTopAddr();

                ret = true;
            }
            else
            {
                // No need to save the uncharted function.
                pFunction = nullptr;
            }
        }
    }

    m_pLastAddedModule = pModule;
    m_pLastAddedFunction = pFunction;

    return ret;
}

bool CpuProfileCss::AddMetadata(gtVAddr va, void** ppMetadata)
{
    if (nullptr != ppMetadata)
    {
        CssFunctionMetadata* pMetadata = new CssFunctionMetadata();
        pMetadata->m_pModule = m_pLastAddedModule;
        pMetadata->m_pFunction = m_pLastAddedFunction;
        pMetadata->m_selfCount = 0ULL;
        pMetadata->m_deepCount = 0ULL;
        pMetadata->m_childPathCount = 0ULL;
        pMetadata->m_parentPathCount = 0ULL;
        AuxGetParentFunctionName(m_pLastAddedModule, m_pLastAddedFunction, va, pMetadata->m_funcName);

        *ppMetadata = pMetadata;
    }

    return true;
}
