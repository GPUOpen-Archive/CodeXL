//==============================================================================
// Copyright (c) 2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief This class loads user PMC library and manages all the PMC load APIs
//==============================================================================

#include "PMCSamplerManager.h"

bool PMCSamplerManager::LoadPMCSamplers(const char* szPath)
{
    LIB_HANDLE pMod = OSUtils::Instance()->GenericLoadLibrary(std::string(szPath));

    if (pMod == NULL)
    {
        std::cout << "Failed to load user PMC dynamic library " << szPath << std::endl;
        return false;
    }

    InitPMCsProc pInit = (InitPMCsProc)OSUtils::Instance()->GetSymbolAddr(pMod, std::string("InitPMCs"));

    const char** ppNames;
    size_t num;
    pInit(&ppNames, &num);

    for (size_t i = 0; i < num; ++i)
    {
        std::stringstream ss;
        ss << "AP_Get";
        ss << ppNames[i];
        m_names.push_back(ppNames[i]);
        GetPMCProc pGetPMC = (GetPMCProc)OSUtils::Instance()->GetSymbolAddr(pMod, ss.str());
        CHECK_PROC_ADDR(pGetPMC, ppNames[i])
        m_callbacks.push_back(pGetPMC);
        std::cout << "PMC " << ppNames[i] << " initialized." << std::endl;
    }

    m_bInit = true;
    return true;
}

void PMCSamplerManager::Sample(std::vector<ULONGLONG>& res)
{
    if (!m_bInit)
    {
        return;
    }

    res.reserve(m_callbacks.size());

    for (std::vector<GetPMCProc>::iterator it = m_callbacks.begin(); it != m_callbacks.end(); ++it)
    {
        res.push_back((*it)());
    }
}
