//==============================================================================
// Copyright (c) 2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief
//==============================================================================

#include "DCDetourCompileShader.h"
#include "DCFuncDefs.h"
#include "Interceptor.h"
#include "DCGPAProfiler.h"
#include "..\Common\Logger.h"

using std::string;

extern DCGPAProfiler g_Profiler;

HRESULT WINAPI Mine_D3DCompileFromFile(
    LPCWSTR pFileName,
    D3D_SHADER_MACRO* pDefines,
    ID3DInclude* pInclude,
    LPCSTR pEntrypoint,
    LPCSTR pTarget,
    UINT Flags1,
    UINT Flags2,
    ID3DBlob** ppCode,
    ID3DBlob** ppErrorMsgs
    )
{
    HRESULT hr;
    hr = Real_D3DCompileFromFile(pFileName, pDefines, pInclude, pEntrypoint, pTarget, Flags1, Flags2, ppCode, ppErrorMsgs);

    if (SUCCEEDED(hr))
    {
        g_Profiler.GetKernelAssemblyManager().AddKernelName(string(pEntrypoint), *ppCode);
    }

    return hr;
}

HRESULT WINAPI Mine_D3DCompile2(
    LPCVOID pSrcData,
    SIZE_T SrcDataSize,
    LPCSTR pSourceName,
    D3D_SHADER_MACRO* pDefines,
    ID3DInclude* pInclude,
    LPCSTR pEntrypoint,
    LPCSTR pTarget,
    UINT Flags1,
    UINT Flags2,
    UINT SecondaryDataFlags,
    LPCVOID pSecondaryData,
    SIZE_T SecondaryDataSize,
    ID3DBlob** ppCode,
    ID3DBlob** ppErrorMsgs)
{
    HRESULT hr;
    hr = Real_D3DCompile2(  pSrcData, SrcDataSize, pSourceName, pDefines, pInclude, pEntrypoint, pTarget, 
                            Flags1, Flags2, SecondaryDataFlags, pSecondaryData, SecondaryDataSize, ppCode, ppErrorMsgs);
    if (SUCCEEDED(hr))
    {
        g_Profiler.GetKernelAssemblyManager().AddKernelName(string(pEntrypoint), *ppCode);
    }

    return hr;
}

HRESULT WINAPI Mine_D3DCompile(
    LPCVOID pSrcData,
    SIZE_T SrcDataSize,
    LPCSTR pSourceName,
    CONST D3D10_SHADER_MACRO* pDefines,
    LPD3D10INCLUDE pInclude,
    LPCSTR pEntrypoint,
    LPCSTR pTarget,
    UINT Flags1,
    UINT Flags2,
    LPD3D10BLOB* ppCode,
    LPD3D10BLOB* ppErrorMsgs
)
{
    HRESULT hr = Real_D3DCompile(
                     pSrcData,
                     SrcDataSize,
                     pSourceName,
                     pDefines,
                     pInclude,
                     pEntrypoint,
                     pTarget,
                     Flags1,
                     Flags2,
                     ppCode,
                     ppErrorMsgs
                 );

    if (SUCCEEDED(hr))
    {
        if (pEntrypoint != NULL)
        {
            g_Profiler.GetKernelAssemblyManager().AddKernelName(string(pEntrypoint), *ppCode);
        }
    }

    return hr;
}

DCDetourD3DCompile::DCDetourD3DCompile(const gtString& strDll) : DetourBase(strDll)
{

}


bool DCDetourD3DCompile::Detach()
{
    // don't detach detour if not attached - this is a valid opeation
    if (!m_bAttached)
    {
        return true;
    }

    DetourBase::Detach();

    LONG error = AMDT::BeginHook();

    if (NO_ERROR == error)
    {
        error |= AMDT::UnhookAPICall(&(PVOID&)Real_D3DCompile, Mine_D3DCompile);
        error |= AMDT::UnhookAPICall(&(PVOID&)Real_D3DCompile2, Mine_D3DCompile2);
        error |= AMDT::UnhookAPICall(&(PVOID&)Real_D3DCompileFromFile, Mine_D3DCompileFromFile);
        error |= AMDT::EndHook();
    }

    if (NO_ERROR != error)
    {
        Log(logERROR, "DetourDetach - D3DCompiler : Failed\n");
    }
    else
    {
        Log(traceMESSAGE, "DetourDetach - D3DCompiler : Successful");
    }

    Real_D3DCompile = NULL;
    return true;
}

bool DCDetourD3DCompile::OnAttach()
{
    m_bAttached = true;

    bool rc1 = AttachD3DCompile();
    bool rc2 = AttachD3DCompile2();
    bool rc3 = AttachD3DCompileFromFile();

    return rc1 & rc2 & rc3;
}

bool DCDetourD3DCompile::AttachD3DCompile()
{
    // if detoured, check if different version is loaded
    if (Real_D3DCompile != NULL)
    {
        D3DCompile_type tmp = (D3DCompile_type)GetProcAddress(m_hMod, "D3DCompile");

        if (tmp != Real_D3DCompile)
        {
            // detach old one
            Log(traceMESSAGE, "Different D3dCompile version found, detach old version");
            Detach();
        }
        else
        {
            Log(traceMESSAGE, "DetourAttach - D3DCompiler D3dCompile : Already detoured");
            return true;
        }
    }

    Real_D3DCompile = (D3DCompile_type)GetProcAddress(m_hMod, "D3DCompile");
    if (Real_D3DCompile != nullptr)
    {

        LONG error = AMDT::BeginHook();

        if (NO_ERROR == error)
        {
            error |= AMDT::HookAPICall(&(PVOID&)Real_D3DCompile, Mine_D3DCompile);
            error |= AMDT::EndHook();
        }

        if (NO_ERROR != error)
        {
            Log(logERROR, "DetourAttach - D3DCompiler D3dCompile: Failed\n");
        }
        else
        {
            Log(traceMESSAGE, "DetourAttach - D3DCompiler D3dCompile: Successful");
        }
    }

    return true;
}

bool DCDetourD3DCompile::AttachD3DCompile2()
{
    // if detoured, check if different version is loaded
    if (Real_D3DCompile2 != NULL)
    {
        D3DCompile2_type tmp = (D3DCompile2_type)GetProcAddress(m_hMod, "D3DCompile2");

        if (tmp != Real_D3DCompile2)
        {
            // detach old one
            Log(traceMESSAGE, "Different D3dCompile2 version found, detach old version");
            Detach();
        }
        else
        {
            Log(traceMESSAGE, "DetourAttach - D3DCompiler D3dCompile2 : Already detoured");
            return true;
        }
    }

    Real_D3DCompile2 = (D3DCompile2_type)GetProcAddress(m_hMod, "D3DCompile2");
    if (Real_D3DCompile2 != nullptr)
    {

        LONG error = AMDT::BeginHook();

        if (NO_ERROR == error)
        {
            error |= AMDT::HookAPICall(&(PVOID&)Real_D3DCompile2, Mine_D3DCompile2);
            error |= AMDT::EndHook();
        }

        if (NO_ERROR != error)
        {
            Log(logERROR, "DetourAttach - D3DCompiler D3DCompile2 : Failed\n");
        }
        else
        {
            Log(traceMESSAGE, "DetourAttach - D3DCompiler D3DCompile2 : Successful");
        }
    }

    return true;
}

bool DCDetourD3DCompile::AttachD3DCompileFromFile()
{
    // if detoured, check if different version is loaded
    if (Real_D3DCompileFromFile != NULL)
    {
        D3DCompileFromFile_type tmp = (D3DCompileFromFile_type)GetProcAddress(m_hMod, "D3DCompileFromFile");

        if (tmp != Real_D3DCompileFromFile)
        {
            // detach old one
            Log(traceMESSAGE, "Different D3dCompileFromFile version found, detach old version");
            Detach();
        }
        else
        {
            Log(traceMESSAGE, "DetourAttach - D3DCompiler D3dCompileFromFile : Already detoured");
            return true;
        }
    }

    Real_D3DCompileFromFile = (D3DCompileFromFile_type)GetProcAddress(m_hMod, "D3DCompileFromFile");
    if (Real_D3DCompileFromFile != nullptr)
    {

        LONG error = AMDT::BeginHook();

        if (NO_ERROR == error)
        {
            error |= AMDT::HookAPICall(&(PVOID&)Real_D3DCompileFromFile, Mine_D3DCompileFromFile);
            error |= AMDT::EndHook();
        }

        if (NO_ERROR != error)
        {
            Log(logERROR, "DetourAttach - D3DCompiler D3DCompileFromFile : Failed\n");
        }
        else
        {
            Log(traceMESSAGE, "DetourAttach - D3DCompiler D3DCompileFromFile : Successful");
        }
    }
    return true;
}