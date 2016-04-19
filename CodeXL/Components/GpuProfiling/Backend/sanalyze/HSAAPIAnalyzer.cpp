//==============================================================================
// Copyright (c) 2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  This file defines base HSAAPIAnalyzer class and HSAAPIAnalyzerManager class which is a IParserListener
//==============================================================================

#include "HSAAPIAnalyzer.h"

void HSAAPIAnalyzerManager::OnParse(HSAAPIInfo* pAPIInfo, bool& stopParsing)
{
    stopParsing = false;

    // On-the-fly analysis
    for (AnalyzerList::iterator it = m_analyzers.begin(); it != m_analyzers.end(); it++)
    {
        if ((*it)->IsEnabled())
        {
            (*it)->Analyze(pAPIInfo);
        }
    }

    // Build flattened APIs list for post-parsing analysis
    if (m_apisToFlatten.size() > 0)
    {
        if (m_apisToFlatten.find(pAPIInfo->m_apiID) != m_apisToFlatten.end())
        {
            m_flattenedAPIs.insert(pAPIInfo);
        }
    }
}
