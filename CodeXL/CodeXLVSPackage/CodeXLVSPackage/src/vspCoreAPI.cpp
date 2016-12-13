//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file vspCoreAPI.cpp
///
//==================================================================================


// C++:
#include <iostream>
#include <fstream>

// Windows:
#define WIN32_LEAN_AND_MEAN 1
#include <windows.h>

// Local:
#include <src/vspCoreAPI.h>
#include <src/vscVsUtils.h>

// Global variable initialization:
IVscCoreAPI g_coreAPIPointers = { 0 };
const IVscCoreAPI& gr_coreAPIPointers = g_coreAPIPointers;
bool g_coreAPIPointersInitialized = false;
const bool& gr_coreAPIPointersInitialized = g_coreAPIPointersInitialized;

#define VSP_CORE_MODULE_NAME "\\CodeXLVSPackageCore" AMDT_PROJECT_SUFFIX ".dll"
#define VSP_APP_SETTINGS_FILE_NAME "\\CodeXL_App_Settings.txt"
typedef bool (*VSCGETCOREAPI_PFN)(IVscCoreAPI* o_pCoreAPI);

// Settings file format:
#define VSP_CODEXL_SETTINGS_HEADER "[CodeXL]"
#define VSP_CODEXL_PATH_HEADER "CodeXL_Path="

// Helper object, used to initialize the structure at build time:
class vspCoreAPIInitializer
{
public:
    vspCoreAPIInitializer()
        : m_hCoreDll(nullptr)
    {
    }

    void Initialize(HMODULE hCurrentModule)
    {
        // Get the current module path:
        std::string currentModulePath;
        char pathBuffer[MAX_PATH] = { 0 };
        ::GetModuleFileNameA(hCurrentModule, pathBuffer, MAX_PATH);

        currentModulePath = pathBuffer;
        size_t lastSlash = currentModulePath.rfind('\\');

        if (lastSlash != std::string::npos)
        {
            currentModulePath.erase(lastSlash, std::string::npos);
        }

        // Get the core path:
        std::string corePath;

        // Get the settings file:
        std::string settingsFilePath = currentModulePath;
        settingsFilePath.append(VSP_APP_SETTINGS_FILE_NAME);
        std::ifstream ifs;
        ifs.open(settingsFilePath);

        if (ifs.is_open())
        {
            // Read the file line by line:
            std::string currentLine;
            bool foundCXLHeader = false;

            while (!ifs.eof())
            {
                // Get the next line:
                std::getline(ifs, currentLine);

                // If we are in the [CodeXL] section:
                if (foundCXLHeader)
                {
                    // Check for the correct prefix:
                    static const char headerPrefix[] = VSP_CODEXL_PATH_HEADER;
#define VSP_HEADER_PREFIX_LEN (sizeof(headerPrefix) - 1)

                    if (0 == currentLine.compare(0, VSP_HEADER_PREFIX_LEN, VSP_CODEXL_PATH_HEADER))
                    {
                        // Copy the suffix:
                        corePath = &(currentLine[VSP_HEADER_PREFIX_LEN]);
                        break;
                    }
                }
                else
                {
                    // See if we've reached the section:
                    foundCXLHeader = (VSP_CODEXL_SETTINGS_HEADER == currentLine);
                }
            }

            // Close the input file:
            ifs.close();

            // If we have a path:
            if (0 < corePath.length())
            {
                LoadCoreModuleFromPath(corePath);
            }
        }

        // If the setting files failed or somehow something else didn't work:
        if (!g_coreAPIPointersInitialized)
        {
            // Fall back on the current module path:
            corePath = currentModulePath;

            LoadCoreModuleFromPath(corePath);
        }

        if (g_coreAPIPointersInitialized)
        {
            // Validate we got all the pointers:
            void** ppCorePointers = (void**)&g_coreAPIPointers;

            for (int i = 0; (sizeof(g_coreAPIPointers) / sizeof(void*)) > i; ++i)
            {
                // If a function is missing:
                if (nullptr == ppCorePointers[i])
                {
                    // Fail the operation:
                    g_coreAPIPointersInitialized = false;
                    break;
                }
            }
        }

        if (!g_coreAPIPointersInitialized)
        {
            ClearModuleHandle();
        }
    };

    ~vspCoreAPIInitializer()
    {
    };

private:
    void GetCoreAPIFromModuleHandle()
    {
        if (nullptr != m_hCoreDll)
        {
            VSCGETCOREAPI_PFN pGetCoreAPI = (VSCGETCOREAPI_PFN)::GetProcAddress(m_hCoreDll, "vscGetCoreAPI");

            if (nullptr != pGetCoreAPI)
            {
                g_coreAPIPointersInitialized = pGetCoreAPI(&g_coreAPIPointers);
            }

            // If we failed, the module handle is useless to us:
            if (!g_coreAPIPointersInitialized)
            {
                ClearModuleHandle();
            }
        }
    }

    void LoadCoreModuleFromPath(const std::string& basePath)
    {
        // Set the base as the dll path (for dependent libraries):
        char oldDllDir[MAX_PATH] = { 0 };
        DWORD dwGotOld = ::GetDllDirectoryA(MAX_PATH, oldDllDir);
        ::SetDllDirectoryA(basePath.c_str());

        // Append to PATH:
#define VSP_MAX_ENV_VAR_LEN 0x8000
#define VSP_PATH_ENV_VAR "PATH"
        char oldPathValue[VSP_MAX_ENV_VAR_LEN + 1];
        DWORD dwGetPath = ::GetEnvironmentVariableA(VSP_PATH_ENV_VAR, oldPathValue, VSP_MAX_ENV_VAR_LEN);
        std::string newPathValue = basePath;
        bool oldPathSet = (0 < dwGetPath);

        if (oldPathSet)
        {
            oldPathValue[VSP_MAX_ENV_VAR_LEN] = (char)0;
            newPathValue.append(";").append(oldPathValue);
        }
        else
        {
            DWORD dwLastErr = ::GetLastError();

            if (dwLastErr)
            {
                VSP_ASSERT(ERROR_ENVVAR_NOT_FOUND == dwLastErr);
            }
        }

        BOOL rcEnv = ::SetEnvironmentVariableA(VSP_PATH_ENV_VAR, newPathValue.c_str());
        bool wasPathSet = (TRUE == rcEnv);
        VSP_ASSERT(wasPathSet);

        // Attempt to load from it:
        std::string corePath = basePath;
        corePath.append(VSP_CORE_MODULE_NAME);

        m_hCoreDll = ::LoadLibraryA(corePath.c_str());

        // If we failed, restore the path:
        if (wasPathSet && (nullptr == m_hCoreDll))
        {
            rcEnv = ::SetEnvironmentVariableA(VSP_PATH_ENV_VAR, oldPathSet ? oldPathValue : nullptr);
            VSP_ASSERT(TRUE == rcEnv);
        }

        // Restore the DLL dir either way:
        ::SetDllDirectoryA((0 < dwGotOld) ? oldDllDir : nullptr);

        // Try and get the core module:
        GetCoreAPIFromModuleHandle();

        // If we succeeded, pass the binaries folder to the core:
        if (g_coreAPIPointersInitialized)
        {
            VSCORE(vscSetApplicationBinariesFolder)(basePath.c_str());
        }
    }

    void ClearModuleHandle()
    {
        ::FreeLibrary(m_hCoreDll);
        m_hCoreDll = nullptr;
        g_coreAPIPointersInitialized = false;
    }

private:
    HMODULE m_hCoreDll;
};

static vspCoreAPIInitializer gs_coreInitializer;

bool vspInitializeCoreAPI(void* hCurrentModule)
{
    gs_coreInitializer.Initialize((HMODULE)hCurrentModule);

    return g_coreAPIPointersInitialized;
}
