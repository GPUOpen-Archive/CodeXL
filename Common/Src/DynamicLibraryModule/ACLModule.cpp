//==============================================================================
// Copyright (c) 2011-2016 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools
/// \file
/// \brief  This class manages the dynamic loading of amdoclcl{32,64}.dll
//==============================================================================

#include "ACLModule.h"

#if defined (_WIN64) || defined(__LP64__)
    #if defined _WIN32 || defined __CYGWIN__
        const char* ACLModule::s_DefaultModuleName = "amdocl12cl64.dll";
        const char* ACLModule::s_OldDefaultModuleName = "amdoclcl64.dll";
        const char* ACLModule::s_TMP_MODULE_NAME = "amdocl64.dll";
        const char* ACLModule::s_HSA_COMPILER_LIB_NAME = "amdhsacl64.dll";
    #else // LINUX
        const char* ACLModule::s_DefaultModuleName = "libamdocl12cl64.so"; // NOT SURE ABOUT IT
        const char* ACLModule::s_OldDefaultModuleName = "libamdoclcl64.so"; // NOT SURE ABOUT IT
        const char* ACLModule::s_TMP_MODULE_NAME = "libamdocl64.so";
        const char* ACLModule::s_HSA_COMPILER_LIB_NAME = "libamdhsacl64.so";
    #endif
#else
    #if defined _WIN32 || defined __CYGWIN__
        const char* ACLModule::s_DefaultModuleName = "amdocl12cl.dll";
        const char* ACLModule::s_OldDefaultModuleName = "amdoclcl.dll";
        const char* ACLModule::s_TMP_MODULE_NAME = "amdocl.dll";
        const char* ACLModule::s_HSA_COMPILER_LIB_NAME = "amdhsacl.dll";
    #else // LINUX
        const char* ACLModule::s_DefaultModuleName = "libamdocl12cl.so"; // NOT SURE ABOUT IT
        const char* ACLModule::s_OldDefaultModuleName = "libamdoclcl.so"; // NOT SURE ABOUT IT
        const char* ACLModule::s_TMP_MODULE_NAME = "libamdocl.so";
        const char* ACLModule::s_HSA_COMPILER_LIB_NAME = "libamdhsacl.so";
    #endif
#endif

ACLModule::ACLModule(void) : m_bModuleLoaded(false)
{
    Initialize();
    LoadModule();
}

ACLModule::~ACLModule(void)
{
    UnloadModule();
}

void ACLModule::Initialize()
{
#define X(SYM) SYM = NULL;
    ACL_API_TABLE;
#undef X

    m_bModuleLoaded = false;
}

void ACLModule::UnloadModule()
{
    m_DynamicLibraryHelper.UnloadModule();
    Initialize();
}

bool ACLModule::LoadModule(const std::string& moduleName)
{
    // Load from specified module
    bool bLoaded = m_DynamicLibraryHelper.LoadModule(moduleName);

    if (!bLoaded)
    {
        // Load from amdocl12cl.dll
        bLoaded = m_DynamicLibraryHelper.LoadModule(s_DefaultModuleName);

        if (!bLoaded)
        {
            // Load from amdoclcl.dll
            bLoaded = m_DynamicLibraryHelper.LoadModule(s_OldDefaultModuleName);

            if (!bLoaded)
            {
                // Load from amdocl.dll
                bLoaded = m_DynamicLibraryHelper.LoadModule(s_TMP_MODULE_NAME);

                if (!bLoaded)
                {
                    // Load from amdhsacl.dll
                    bLoaded = m_DynamicLibraryHelper.LoadModule(s_HSA_COMPILER_LIB_NAME);
                }
            }
        }
    }

    if (bLoaded)
    {

#define MAKE_STRING(s) "acl"#s
#define X(SYM) SYM = reinterpret_cast<acl##SYM##Proc>(m_DynamicLibraryHelper.GetProcAddress(MAKE_STRING(SYM)));
        ACL_API_TABLE;
#undef X
#undef MAKE_STRING

        // Decide if we got everything
        // Temporarily remove the check here as there are a couple of APIs that are not exposed yet.
        //#define X(SYM) && SYM != NULL
        //        m_bModuleLoaded = true ACL_API_TABLE;
        //#undef X

        m_bModuleLoaded = (CompilerInit != NULL);
    }

    return m_bModuleLoaded;
}
