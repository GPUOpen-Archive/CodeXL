//==============================================================================
// Copyright (c) 2016 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  This file provides access to the hsa runtime module
//==============================================================================

#ifndef _HSA_RT_MODULE_LOADER_H_
#define _HSA_RT_MODULE_LOADER_H_

#include <iostream>
#include <sstream>

#include "TSingleton.h"
#include "Logger.h"

using namespace GPULogger;

/// Singleton to hold a global HSA RT Module
template <typename T>
class HSARTModuleLoader : public TSingleton<HSARTModuleLoader<T> >
{
    /// TSingleton needs to be able to use our constructor.
    friend class TSingleton<HSARTModuleLoader>;

public:
    /// Constructor
    HSARTModuleLoader() : m_hsaRTModule(nullptr)
    {
        Log(GPULogger::logMESSAGE, "Creating HSARTModuleLoader\n");
    }

    /// Destructor
    virtual ~HSARTModuleLoader()
    {
        Log(GPULogger::logMESSAGE, "Destroying HSARTModuleLoader\n");
        delete m_hsaRTModule;
        m_hsaRTModule = nullptr;
    }

    /// Gets the HSA Runtime Module
    /// \return the HSA Runtime Module
    T* GetHSARTModule()
    {
        if (nullptr == m_hsaRTModule)
        {
            m_hsaRTModule = new(std::nothrow) T();

            if (nullptr == m_hsaRTModule || !m_hsaRTModule->IsModuleLoaded())
            {
                std::stringstream ss;
                ss << "Unable to load library: " << m_hsaRTModule->s_defaultModuleName << "\n";
                std::cout << ss.str();
                Log(GPULogger::logERROR, ss.str().c_str());
            }

        }

        return m_hsaRTModule;
    };

    /// Check if the module is loaded
    /// \return true if it is loaded
    bool IsLoaded()
    {
        return nullptr != m_hsaRTModule;
    }


private:
    T* m_hsaRTModule;  ///< the HSA RT module
};

#endif  //_HSA_RT_MODULE_LOADER_H_
