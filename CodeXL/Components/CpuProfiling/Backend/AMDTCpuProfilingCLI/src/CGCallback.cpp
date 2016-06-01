//==================================================================================
// Copyright (c) 2014-2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file CGCallback.cpp
///
//==================================================================================

// Qt:
#include <QtGui>

// Backend:
#include <AMDTCpuPerfEventUtils/inc/EventEngine.h>

// Project:
#include <CGCallback.h>
#include <Utils.h>

bool InitializeSymbolEngine(ExecutableFile* pExecutable,
                            gtString& searchPath,
                            gtString& serverList,
                            gtString& cachePath);
bool SyncWithSymbolEngine(CpuProfileReader& profileReader, CpuProfileModule& module, ExecutableFile** ppEexecutable);
CpuProfileModule* getModuleDetail2(CpuProfileReader& profileReader, const gtString& modulePath, ExecutableFile** ppExe);
bool GetParentFunctionName(const CpuProfileModule* pModule, const CpuProfileFunction* pFunction, gtVAddr va, gtString& funcName);

bool ConvertCygwinPath(const wchar_t* pPath, int len, gtString& convertedPath);

bool InitializeSymbolEngine(ExecutableFile* pExecutable,
                            gtString& searchPath,
                            gtString& serverList,
                            gtString& cachePath)
{
    bool retVal = false;

    if (NULL != pExecutable)
    {
        const wchar_t* pSearchPath = (!searchPath.isEmpty()) ? searchPath.asCharArray() : NULL;
        const wchar_t* pServerList = (!serverList.isEmpty()) ? serverList.asCharArray() : NULL;
        const wchar_t* pCachePath = (NULL != pServerList && !cachePath.isEmpty()) ? cachePath.asCharArray() : NULL;

        retVal = pExecutable->InitializeSymbolEngine(pSearchPath, pServerList, pCachePath);
    }

    return retVal;
}

bool SyncWithSymbolEngine(CpuProfileReader& profileReader, CpuProfileModule& module, ExecutableFile** ppEexecutable)
{
    (void) profileReader; // unused
    bool retVal = true;
    ExecutableFile* pEexecutable = nullptr;

    // TODO.. how to get the module path - use module path as exepath
    // we need to use the path provided by the user
    gtString exePath = module.getPath();

    pEexecutable = ExecutableFile::Open(exePath.asCharArray(), module.getBaseAddr());

    if (ppEexecutable != nullptr && nullptr != *ppEexecutable)
    {
        // Initialize executable symbol engine:
        // TODO - how to fill these stuff from args
        gtString searchPath;
        gtString serverList;
        gtString cachePath;

        if (InitializeSymbolEngine(pEexecutable, searchPath, serverList, cachePath))
        {
            delete pEexecutable;
            pEexecutable = nullptr;
        }

    }

    if (nullptr != pEexecutable)
    {
        *ppEexecutable = pEexecutable;
        retVal = true;
    }

    return retVal;
}


CpuProfileModule* getModuleDetail2(CpuProfileReader& profileReader, const gtString& modulePath, ExecutableFile** ppExe)
{
    CpuProfileModule* pModule = NULL;

    if (NULL != ppExe)
    {
        *ppExe = NULL;
    }

    NameModuleMap* pModuleMap = profileReader.getModuleMap();

    gtString searchPath;
    gtString serverList;
    gtString cachePath;

    if (NULL != pModuleMap && !pModuleMap->empty())
    {
        NameModuleMap::iterator mit = pModuleMap->find(modulePath);

        if (pModuleMap->end() != mit)
        {
            if (mit->second.m_isImdRead)
            {
                pModule = &mit->second;

                if (NULL != ppExe)
                {
                    // Get an executable handler for this process:
                    *ppExe = ExecutableFile::Open(modulePath.asCharArray(), pModule->getBaseAddr());

                    if (NULL != *ppExe)
                    {
                        // Initialize executable symbol engine:
                        InitializeSymbolEngine(*ppExe, searchPath, serverList, cachePath);
                    }
                }
            }
            else
            {
                pModule = profileReader.getModuleDetail(modulePath);

                if (NULL != pModule)
                {
                    pModule->setSystemModule(IsSystemModule(pModule->getPath()));

                    // Get an executable handler for this process:
                    gtString exePath = modulePath; // FIXME no user input and no cache

                    pModule->m_symbolsLoaded = SyncWithSymbolEngine(profileReader, *pModule, ppExe);

                    if (NULL != ppExe && NULL == *ppExe)
                    {
                        // Get an executable handler for this process:
                        *ppExe = ExecutableFile::Open(exePath.asCharArray(), pModule->getBaseAddr());

                        if (NULL != *ppExe)
                        {
                            // Initialize executable symbol engine:
                            InitializeSymbolEngine(*ppExe, searchPath, serverList, cachePath);
                        }
                    }
                }
            }
        }
    }

    return pModule;
}

bool GetParentFunctionName(const CpuProfileModule* pModule, const CpuProfileFunction* pFunction, gtVAddr va, gtString& funcName)
{
    if (NULL != pModule && NULL == pFunction)
    {
        pFunction = pModule->findFunction(va);
    }

    bool ret = !(NULL == pFunction ||
                 pFunction->getFuncName().isEmpty() ||
                 (NULL != pModule && pModule->isUnchartedFunction(*pFunction)));

    if (ret)
    {
        funcName = pFunction->getFuncName();
    }
    else
    {
        if (NULL != pFunction && !pFunction->getFuncName().isEmpty())
        {
            funcName = pFunction->getFuncName();
        }
        else if (NULL != pModule)
        {
            gtString modFile;

            if (pModule->extractFileName(modFile))
            {
                funcName = modFile;
                funcName += '!';
            }
        }

        funcName.appendFormattedString(L"0x%08llx", static_cast<unsigned long long>(va));
    }

    return ret;
}


bool ConvertCygwinPath(const wchar_t* pPath, int len, gtString& convertedPath)
{
#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS

    bool ret = (10 < len && 0 == memcmp(pPath, L"/cygdrive/", 10 * sizeof(wchar_t)));

    if (ret)
    {
        pPath += 10;
        len -= 9;
        convertedPath.resize(static_cast<size_t>(len));

        int i = 0;

        while (L'/' != *pPath)
        {
            convertedPath[i++] = *pPath++;
        }

        convertedPath[i++] = L':';
        convertedPath[i++] = L'\\';

        ++pPath;

        for (; i < len; ++i, ++pPath)
        {
            convertedPath[i] = (L'/' == *pPath) ? L'\\' : *pPath;
        }
    }

    return ret;

#else

    (void)pPath;         // Unused
    (void)len;           // Unused
    (void)convertedPath; // Unused
    return false;

#endif
}



CGCallback::CGCallback(CpuProfileReader* pProfileReader)
    : m_pProfileReader(pProfileReader),
      m_event(EventMaskType(-2)),
      m_pCurModule(NULL),
      m_pCurFunction(NULL)
{
}

CGCallback::~CGCallback()
{
    for (FunctionGraph::node_iterator it = m_funcGraph.GetBeginNode(), itEnd = m_funcGraph.GetEndNode(); it != itEnd; ++it)
    {
        CGFunctionMetaData* pMetadata = static_cast<CGFunctionMetaData*>(it->m_val);

        if (NULL != pMetadata)
        {
            delete pMetadata;
        }
    }
}

bool CGCallback::Initialize(const gtString& path, gtUInt32 pid)
{
    if (path.isEmpty())
    {
        return false;
    }

    // Each css file is <pid>.css
    gtString fileName;
    fileName.appendFormattedString(L"%d", pid);

    osFilePath cssFile(path);
    cssFile.setFileName(fileName);
    cssFile.setFileExtension(L"css");

    if (! cssFile.exists())
    {
        return false;
    }

    CssReader reader(CP_CSS_MAX_UNWIND_DEPTH);
    bool ret = reader.Open(cssFile.asString().asCharArray()); // (cssFile.toStdWString().c_str());

    if (ret)
    {
        reader.SetCallback(this);
        ret = reader.Read(m_funcGraph, pid);
    }

    Finalize();

    return ret;
}

const CpuProfileModule* CGCallback::FindModule(gtVAddr va) const
{
    AddrModuleInfoMap::const_iterator it = m_addrModuleMap.upper_bound(va);
    return (m_addrModuleMap.begin() != it) ? &(--it)->second.m_module : NULL;
}

CpuProfileModule* CGCallback::FindModule(gtVAddr va)
{
    AddrModuleInfoMap::iterator it = m_addrModuleMap.upper_bound(va);
    return (m_addrModuleMap.begin() != it) ? &(--it)->second.m_module : NULL;
}

const CpuProfileFunction* CGCallback::FindFunction(gtVAddr va) const
{
    const CpuProfileModule* pModule = FindModule(va);
    return (NULL != pModule) ? pModule->findFunction(va) : NULL;
}

CpuProfileFunction* CGCallback::FindFunction(gtVAddr va)
{
    CpuProfileModule* pModule = FindModule(va);
    return (NULL != pModule) ? pModule->findFunction(va) : NULL;
}

void CGCallback::Finalize()
{
    for (AddrModuleInfoMap::iterator it = m_addrModuleMap.begin(), itEnd = m_addrModuleMap.end(); it != itEnd; ++it)
    {
        (*it).second.Clear();
    }
}

bool CGCallback::AddModule(gtVAddr base, const wchar_t* pPath)
{
    ExecutableFile* pExecutable = NULL;

    int lenPath = static_cast<int>(wcslen(pPath));

    CpuProfileModule* pModule = getModuleDetail2(*m_pProfileReader, pPath, &pExecutable);

    if (NULL == pModule)
    {
        gtString modulePath(pPath, lenPath);
        NameModuleMap* pNameModMap = m_pProfileReader->getModuleMap();

        NameModuleMap::iterator it = pNameModMap->insert(NameModuleMap::value_type(modulePath, CpuProfileModule())).first;
        pModule = &it->second;

        pModule->setIndirect(true);
        pModule->setSystemModule(IsSystemModule(modulePath));
        pModule->m_base = base;

        // Indirect modules do not have IMD files to read, so just treat the modules as already read (to prevent future read).
        pModule->m_isImdRead = true;

        pModule->m_modType = CpuProfileModule::UNMANAGEDPE;
        pModule->setPath(modulePath);

        getModuleDetail2(*m_pProfileReader, pPath, &pExecutable);
    }
    else
    {
        GT_ASSERT(pModule->getBaseAddr() == base);
    }

    if (NULL != pExecutable && pModule->m_size < pExecutable->GetImageSize())
    {
        pModule->m_size = pExecutable->GetImageSize();
    }

    // TODO: remove the if condn later
    if (NULL != pModule && NULL != pExecutable)
    {
        m_addrModuleMap.insert(AddrModuleInfoMap::value_type(pModule->getBaseAddr(), CGModuleInfo(*pModule, pExecutable)));
    }

    return true;
}


bool CGCallback::AddFunction(gtVAddr va, gtVAddr& startVa, gtVAddr& endVa)
{
    bool ret = false;

    CpuProfileModule* pModule = NULL;
    CpuProfileFunction* pFunction = NULL;

    AddrModuleInfoMap::iterator m = m_addrModuleMap.upper_bound(va);

    CGModuleInfo* pModInfo = NULL;

    if (m_addrModuleMap.begin() != m)
    {
        // We have found a module which bounds the virtual address from the bottom.
        pModInfo = &(--m)->second;

        // Verify that the module also bounds the virtual address from the top.
        if (va >= (pModInfo->m_module.getBaseAddr() + static_cast<gtVAddr>(pModInfo->m_module.m_size)))
        {
            pModInfo = NULL;
        }
    }

    // If no module found.
    if (NULL == pModInfo)
    {
        // It might happen that the modules list in the CSS file was not complete.
        NameModuleMap* pNameModMap = m_pProfileReader->getModuleMap();

        // Search for the containing module in the session's modules list.
        for (NameModuleMap::iterator it = pNameModMap->begin(), itEnd = pNameModMap->end(); it != itEnd; ++it)
        {
            CpuProfileModule& curMod = it->second;

            ExecutableFile* pExecutable = NULL;

            // If the module has not been initialized (read from the corresponding IMD file).
            bool synch = !curMod.m_isImdRead;

            if (synch)
            {
                // Initialize the module information by requesting it from the session.
                getModuleDetail2(*m_pProfileReader, curMod.getPath(), &pExecutable);
            }

            // If the module contains the virtual address.
            if (curMod.getBaseAddr() <= va && va < (curMod.getBaseAddr() + static_cast<gtVAddr>(curMod.m_size)))
            {
                // If we have not requested the ExecutableFile of the module, then request it now.
                // This is done here and not as an 'else' for the "if (synch)" above, to prevent unnecessary openings of the
                // executable file (a heavy operation).
                if (!synch)
                {
                    getModuleDetail2(*m_pProfileReader, curMod.getPath(), &pExecutable);

                }

                // We have found a containing module; insert it to the modules map, for future searches.
                m = m_addrModuleMap.insert(AddrModuleInfoMap::value_type(curMod.getBaseAddr(), CGModuleInfo(curMod, pExecutable))).first;
                pModInfo = &m->second;
                break;
            }

            if (NULL != pExecutable)
            {
                delete pExecutable;
            }
        }
    }

    if (NULL != pModInfo)
    {
        pModule = &pModInfo->m_module;

        AddrFunctionMultMap& funcMap = pModule->getFunctionMap();
        AddrFunctionMultMap::iterator f = funcMap.upper_bound(va);

        if (f != funcMap.begin())
        {
            pFunction = &(--f)->second;
        }

        if (NULL == pFunction || pFunction->getFuncName().isEmpty() || !pFunction->contains(va) || pModule->isUnchartedFunction(*pFunction))
        {
            ExecutableFile* pExecutable = pModInfo->m_pExecutable;

            if (NULL != pExecutable)
            {
                SymbolEngine* pSymbolEngine = pExecutable->GetSymbolEngine();

                gtRVAddr funcRvaEnd = GT_INVALID_RVADDR;
                const FunctionSymbolInfo* pFuncSymbol = pSymbolEngine->LookupFunction(va - pModule->getBaseAddr(), &funcRvaEnd);

                if (NULL != pFuncSymbol)
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

                    if (NULL == pFunction || pFunction->getBaseAddr() != funcVa)
                    {
                        gtString srcFileName;
                        SourceLineInfo sourceLine;

                        if (pSymbolEngine->FindSourceLine(pFuncSymbol->m_rva, sourceLine))
                        {
                            int srcFileNameLen = static_cast<int>(wcslen(sourceLine.m_filePath));

                            // FIXME
                            if (!ConvertCygwinPath(sourceLine.m_filePath, srcFileNameLen, srcFileName))
                            {
                                srcFileName.assign(sourceLine.m_filePath, srcFileNameLen);
                            }
                        }
                        else
                        {
                            sourceLine.m_line = 0U;
                        }

                        CpuProfileFunction funcNew((NULL != pFuncSymbol->m_pName) ? pFuncSymbol->m_pName : L"",
                                                   funcVa,
                                                   funcSize,
                                                   gtString(),
                                                   srcFileName,
                                                   sourceLine.m_line);
                        pFunction = &funcMap.insert(AddrFunctionMultMap::value_type(funcVa, funcNew))->second;
                    }
                    else
                    {
                        if (NULL != pFuncSymbol->m_pName && pFunction->getFuncName().isEmpty())
                        {
                            pFunction->setFuncName(pFuncSymbol->m_pName);
                        }

                        pFunction->setSize(funcSize);
                    }
                }
            }

            // If we have found a candidate function but it does not contain the virtual address.
            if (NULL != pFunction && !pModule->isUnchartedFunction(*pFunction) && (pFunction->getFuncName().isEmpty() || !pFunction->contains(va)))
            {
                pFunction = NULL;
            }
        }

        if (NULL == pFunction)
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
                pFunction = NULL;
            }
        }
    }

    m_pCurModule = pModule;
    m_pCurFunction = pFunction;

    return ret;
}

bool CGCallback::AddMetadata(gtVAddr va, void** ppMetadata)
{
    if (NULL != ppMetadata)
    {
        CGFunctionMetaData* pMetadata = new CGFunctionMetaData();

        pMetadata->m_pModule = m_pCurModule;
        pMetadata->m_pFunction = m_pCurFunction;

        pMetadata->m_selfCount = 0ULL;
        pMetadata->m_deepCount = 0ULL;

        gtString funcName;
        GetParentFunctionName(m_pCurModule, m_pCurFunction, va, funcName);
        pMetadata->m_funcName = funcName;

        *ppMetadata = pMetadata;
    }

    return true;
}
