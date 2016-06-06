//==============================================================================
// Copyright (c) 2015-2016 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools
/// \file
/// \brief  This class manages the dynamic loading of HSA entry points hsa-runtime{32,64}.dll and libhsa-runtime{32,64}.so
//==============================================================================

#include <string>
#include "HSAModule.h"

#if defined(_WIN64) || defined(__LP64__)
    #if defined(_WIN32) || defined(__CYGWIN__)
        const char* HSAModule::s_defaultModuleName = "hsa-runtime64.dll";
    #else // LINUX
        const char* HSAModule::s_defaultModuleName = "libhsa-runtime64.so.1";
    #endif
#else
    #pragma message("HSA Foundation runtime does not support 32-bit builds")
    #if defined(_WIN32) || defined(__CYGWIN__)
        const char* HSAModule::s_defaultModuleName = "hsa-runtime.dll";
    #else // LINUX
        const char* HSAModule::s_defaultModuleName = "libhsa-runtime.so.1";
    #endif
#endif

HSAModule::HSAModule(void) : m_isModuleLoaded(false)
{
    Initialize();
    LoadModule();
}

HSAModule::~HSAModule(void)
{
    UnloadModule();
}

void HSAModule::Initialize()
{
#define X(SYM) SYM = nullptr;
    HSA_RUNTIME_API_TABLE;
    HSA_EXT_FINALIZE_API_TABLE;
    HSA_EXT_IMAGE_API_TABLE;
    HSA_EXT_AMD_API_TABLE;
#undef X

    m_isModuleLoaded = false;
}

void HSAModule::UnloadModule()
{
    m_dynamicLibraryHelper.UnloadModule();
    Initialize();
}

bool HSAModule::LoadModule(const std::string& moduleName)
{
    // Load from specified module
    bool bLoaded = m_dynamicLibraryHelper.LoadModule(moduleName);

    if (!bLoaded)
    {
        // Load from deafult module
        bLoaded = m_dynamicLibraryHelper.LoadModule(s_defaultModuleName);
    }

    if (bLoaded)
    {

#define MAKE_STRING(s) "hsa_"#s
#define X(SYM) SYM = reinterpret_cast<decltype(::hsa_##SYM)*>(m_dynamicLibraryHelper.GetProcAddress(MAKE_STRING(SYM)));
        HSA_RUNTIME_API_TABLE;
        HSA_EXT_AMD_API_TABLE;
#undef X
#undef MAKE_STRING

        // initialize the extension functions
        if (nullptr != system_extension_supported && nullptr != system_get_extension_table)
        {
            bool extensionSupported = false;
            bool mustCallShutdown = false;
            hsa_status_t status = system_extension_supported(HSA_EXTENSION_FINALIZER, 1, 0, &extensionSupported);

            if (HSA_STATUS_ERROR_NOT_INITIALIZED == status)
            {
                // hsa runtime not initialized yet, initialize it now
                mustCallShutdown = true;
                status = init();

                if (HSA_STATUS_SUCCESS == status)
                {
                    status = system_extension_supported(HSA_EXTENSION_FINALIZER, 1, 0, &extensionSupported);
                }
            }

            if ((HSA_STATUS_SUCCESS == status) && extensionSupported)
            {
                hsa_ext_finalizer_1_00_pfn_t finalizerTable;
                status = system_get_extension_table(HSA_EXTENSION_FINALIZER, 1, 0, &finalizerTable);

                if (HSA_STATUS_SUCCESS == status)
                {

#define X(SYM) SYM = finalizerTable.hsa_##SYM;
                    HSA_EXT_FINALIZE_API_TABLE;
#undef X
                }
            }

            status = system_extension_supported(HSA_EXTENSION_IMAGES, 1, 0, &extensionSupported);

            if ((HSA_STATUS_SUCCESS == status) && extensionSupported)
            {
                hsa_ext_images_1_00_pfn_t imagesTable;
                status = system_get_extension_table(HSA_EXTENSION_IMAGES, 1, 0, &imagesTable);

                if (HSA_STATUS_SUCCESS == status)
                {

#define X(SYM) SYM = imagesTable.hsa_##SYM;
                    HSA_EXT_IMAGE_API_TABLE;
#undef X
                }
            }

            status = system_extension_supported(HSA_EXTENSION_AMD_LOADED_CODE_OBJECT, 1, 0, &extensionSupported);

            if ((HSA_STATUS_SUCCESS == status) && extensionSupported)
            {
                hsa_ven_amd_loaded_code_object_1_00_pfn_t loadedCodeObjectTable;
                status = system_get_extension_table(HSA_EXTENSION_AMD_LOADED_CODE_OBJECT, 1, 0, &loadedCodeObjectTable);

                if (HSA_STATUS_SUCCESS == status)
                {

#define X(SYM) SYM = loadedCodeObjectTable.hsa_##SYM;
                    HSA_VEN_AMD_LOADED_CODE_OBJECT_API_TABLE;
#undef X
                }
            }

            if (mustCallShutdown)
            {
                // if we initialzed the runtime, then shut it down now
                shut_down();
            }
        }

#undef X

        // Check if we initialized all the function pointers
#define X(SYM) && SYM != nullptr
        m_isModuleLoaded = true HSA_RUNTIME_API_TABLE HSA_EXT_FINALIZE_API_TABLE HSA_EXT_IMAGE_API_TABLE HSA_EXT_AMD_API_TABLE;
#undef X

    }

    return m_isModuleLoaded;
}
