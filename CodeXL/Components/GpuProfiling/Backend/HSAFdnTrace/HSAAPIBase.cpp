//==============================================================================
// Copyright (c) 2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  This file declares a base class for tracing HSA API
//==============================================================================

#include "HSAFunctionDefs.h"
#include "HSAAPIBase.h"
#include "HSATraceStringUtils.h"

HSAAPIBase::HSAAPIBase() :
    APIBase(),
    m_type(HSA_API_Type_UNKNOWN)
{
}

HSAAPIBase::~HSAAPIBase()
{
}

void HSAAPIBase::WriteAPIEntry(std::ostream& sout)
{
    m_strName = HSATraceStringUtils::GetHSAAPINameString(m_type);
    std::string ret = GetRetString();

    if (!ret.empty())
    {
        sout << ret << " = ";
    }

    sout << m_strName << " ( ";
    sout << ToString() << " )";
}

void HSAAPIBase::CreateStackEntry()
{
#ifdef WIN32
    SP_TODO("need to implement CreateStackEntry on Windows");
#else

    if (m_pStackEntry != NULL)
    {
        return;
    }

    for (std::vector<StackEntry>::iterator it = m_stack.begin(); it != m_stack.end(); ++it)
    {
        if (it->m_strModName.empty())
        {
            continue;
        }

        // the top few stack frames are from this agent or from the runtime.  Make sure we skip those to get back to the frame in the app
        if (it->m_strModName.find(HSA_TRACE_AGENT_DLL) == std::string::npos && it->m_strModName.find("hsa-runtime") == std::string::npos)
        {
            StackTracer::Instance()->GetSymbolName(it->m_dwAddress, *it);
            m_pStackEntry = new(std::nothrow) StackEntry(*it);

            if (m_pStackEntry != NULL)
            {
                m_pStackEntry->m_strSymName = "NA";
            }

            break;
        }
    }

#endif
}

void HSAAPIBase::WriteStackEntry(std::ostream& sout)
{
    if (m_pStackEntry == NULL)
    {
        CreateStackEntry();
    }

    APIBase::WriteStackEntry(sout);
}

bool HSAAPIBase::WriteTimestampEntry(std::ostream& sout, bool bTimeout)
{
    SP_UNREFERENCED_PARAMETER(bTimeout);

    if (m_strName.empty())
    {
        m_strName = HSATraceStringUtils::GetHSAAPINameString(m_type);
    }

    // API type
    sout << std::left << std::setw(5) << m_type;

    // APIName
    sout << std::left << std::setw(45) << m_strName;

    // start time
    sout << std::left << std::setw(21) << m_ullStart;

    // end time
    sout << std::left << std::setw(21) << m_ullEnd;

    return true;
}
