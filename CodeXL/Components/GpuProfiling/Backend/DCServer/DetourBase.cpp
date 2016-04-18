//=====================================================================
//
// Author: Lihan
//         GPU Developer Tools
//         AMD, Inc.
//
// DetourBase.cpp
//
//=====================================================================
// $Id: //devtools/main/CodeXL/Components/GpuProfiling/Backend/DCServer/DetourBase.cpp#1 $
//
// Last checkin:  $DateTime: 2016/04/17 09:15:18 $
// Last edited by: $Author: chesik $
//=====================================================================
//   ( C ) AMD, Inc. 2009 All rights reserved.
//=====================================================================
#include "DetourBase.h"
#include "Logger.h"

using std::string;
using std::vector;
using namespace GPULogger;

DetourBase::DetourBase()
{
    m_bAttached = false;
    m_hMod = NULL;
}

DetourBase::DetourBase(const gtString& strDll)
{
    m_bAttached = false;
    m_hMod = NULL;
    gtString dllstr = strDll;
    DLLPair pair;
    pair.m_strDll = dllstr;
    pair.m_bAttached = false;
    m_vecDllPair.push_back(pair);
}

DetourBase::~DetourBase()
{
    m_hMod = NULL;
}

bool DetourBase::OnAttach()
{
    return true;
}

void DetourBase::AddDLL(const gtString& strDll)
{
    DLLPair pair;
    gtString dllstr = strDll;
    pair.m_strDll = dllstr;
    pair.m_bAttached = false;
    m_vecDllPair.push_back(pair);
}

void DetourBase::Attach()
{
    for (vector<DLLPair>::iterator it = m_vecDllPair.begin(); it != m_vecDllPair.end(); it++)
    {
        m_hMod = GetModuleHandle((*it).m_strDll.asCharArray());

        if (!((*it).m_bAttached) && m_hMod != NULL)
        {
            Log(traceMESSAGE, "%ls Loaded", (*it).m_strDll.asCharArray());
            bool bAttached = OnAttach();
            (*it).m_bAttached = bAttached;
        }
    }
}

bool DetourBase::Detach()
{
    m_bAttached = false;
    return true;
}