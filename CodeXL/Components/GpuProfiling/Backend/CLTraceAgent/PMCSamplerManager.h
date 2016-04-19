//==============================================================================
// Copyright (c) 2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief This class loads user PMC library and manages all the PMC load APIs
//==============================================================================


#ifndef _PMC_SAMPLER_MANAGER_H_
#define _PMC_SAMPLER_MANAGER_H_

#include <string>
#include <vector>
#include <iostream>
#include <sstream>
#include "APTraceUserPMCExt.h"
#include "../Common/OSUtils.h"

#define CHECK_PROC_ADDR(p, name) if (p == NULL) { std::cout << "Failed to load user PMC " << name << std::endl; continue; }

//------------------------------------------------------------------------------------
/// User PMC sampler manager class
//------------------------------------------------------------------------------------
class PMCSamplerManager : public TSingleton<PMCSamplerManager>
{
    friend class TSingleton<PMCSamplerManager>;
public:
    /// Sample PMC from all callback functions
    /// \param[out] res Array that holds the results
    void Sample(std::vector<ULONGLONG>& res);

    /// Add PMC sampler
    /// \param strName Name of the PMC
    /// \param pGetPMCProc Callback function that is used to query the PMC
    void LoadPMCSampler(const std::string& strName, GetPMCProc pGetPMCProc)
    {
        m_names.push_back(strName);
        m_callbacks.push_back(pGetPMCProc);
    }

    /// Load PMC samplers from a shared lib
    /// \param szPath Lib path
    bool LoadPMCSamplers(const char* szPath);
protected:
    /// Constructor
    PMCSamplerManager()
    {
        m_bInit = false;
    }

private:
    std::vector<GetPMCProc> m_callbacks;   ///< A list of PMC callbacks
    std::vector<std::string> m_names;      ///< A list of PMC names
    bool m_bInit;                          ///< A flag indicating whether or not PMC lib is initialized.
};

#endif //_PMC_SAMPLER_MANAGER_H_
