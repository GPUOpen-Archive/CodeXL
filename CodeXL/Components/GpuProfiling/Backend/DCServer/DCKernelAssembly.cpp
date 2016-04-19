//==============================================================================
// Copyright (c) 2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief
//==============================================================================

#include <sstream>
#include <iostream>
#include "..\Common\StringUtils.h"
#include "..\Common\FileUtils.h"
#include "..\Common\Logger.h"
#include "DCKernelAssembly.h"
#include "DCGPAProfiler.h"
#include "DCServer.h"

using std::map;
using std::string;
using std::pair;
using namespace GPULogger;

extern DCGPAProfiler g_Profiler;

#define MAX_LEN 100

KernelAssembly::KernelAssembly(void): m_bOutputASM(false)
{
    m_strFilePrefix = KERNEL_ASSEMBLY_FILE_PREFIX;
    m_pmutex = new AMDTMutex("KernelAssembly_mutex ");
    m_pD3DDisassemble = NULL;
}

KernelAssembly::~KernelAssembly(void)
{
    for (KernelMap::iterator it = kernelMap.begin(); it != kernelMap.end(); it++)
    {
        it->second.m_pAssemblyBlob->Release();
    }

    kernelMap.clear();

    if (m_pmutex != NULL)
    {
        delete m_pmutex;
    }
}

void KernelAssembly::AddComputeShader(ID3D11ComputeShader* pCS, const void* pShaderByteCode, SIZE_T BytecodeLength)
{
    AMDTScopeLock lock(m_pmutex);
    KernelMap::iterator kmIt = kernelMap.find(pCS);

    if (kmIt != kernelMap.end())
    {
        // shader already existed in table??
        Log(logWARNING, "KernelAssembly::AddComputeShader() shader already existed in table\n");
        return;
    }

    DCKernel dcKernel;
    // search through kernelNameMap
    KernelNameMap::iterator it = kernelNameMap.find(pShaderByteCode);
    string kernelName;
    kernelName.clear();

    if (it != kernelNameMap.end())
    {
        // found kernel name
        kernelName = it->second;
        // remove from kernel name table
        kernelNameMap.erase(it);
    }
    else
    {
        // name not found, no CompileShader* called before
        char cstr[MAX_LEN];
        sprintf_s(cstr, MAX_LEN, "Kernel%zd", kernelMap.size());
        kernelName = cstr;
    }



    ID3DBlob* pDisassembly;

    /// Get function pointer when needed
    if (m_pD3DDisassemble == NULL)
    {
        HMODULE hMod = LoadLibrary(L"D3DCompiler_43.dll");

        if (hMod != NULL)
        {
            m_pD3DDisassemble = (D3DDisassembleProc)GetProcAddress(hMod, "D3DDisassemble");
        }
    }

    if (m_pD3DDisassemble == NULL)
    {
        // If this failed, we can't get kernel name but it's not a fatal error. User can still get profile result.
        // This only happens when user didn't install DXSDK or DX runtime.
        Log(logERROR, "Failed to get D3DDisassemble function pointer.\n");
        return;
    }

    m_pD3DDisassemble(pShaderByteCode,
                      BytecodeLength,
                      0,
                      NULL,
                      &pDisassembly);

    dcKernel.m_pAssemblyBlob = pDisassembly;


    // write HLSL ASM when create shader

    string strOutputDir;

    if (!FileUtils::GetWorkingDirectory(g_Profiler.GetOutputFile(), strOutputDir))
    {
        strOutputDir.clear();
    }

    char chHandle[MAX_LEN];
    sprintf_s(chHandle, MAX_LEN, "_%8p", pCS);
    string strHandle(chHandle);
    kernelName += strHandle;

    dcKernel.m_strKernelName = kernelName;

    char* tmpBuf = new char[pDisassembly->GetBufferSize()];
    memcpy(tmpBuf, pDisassembly->GetBufferPointer(), pDisassembly->GetBufferSize());

    string strSrc(tmpBuf);
    size_t pos = strSrc.find("dcl_thread_group");
    UINT x, y, z;

    if (pos != string::npos)
    {
        std::stringstream ss(std::stringstream::in | std::stringstream::out);
        ss << strSrc;
        ss.seekg(pos + strlen("dcl_thread_group"));

        char chTmp;
        ss >> x >> chTmp >> y >> chTmp >> z;
        dcKernel.m_uiGroupSizeX = x;
        dcKernel.m_uiGroupSizeY = y;
        dcKernel.m_uiGroupSizeZ = z;
    }
    else
    {
        dcKernel.m_uiGroupSizeX = 0;
        dcKernel.m_uiGroupSizeY = 0;
        dcKernel.m_uiGroupSizeZ = 0;
    }

    delete [] tmpBuf;

    kernelMap.insert(pair<ID3D11ComputeShader*, DCKernel>(pCS, dcKernel));

    if (m_bOutputASM)
    {
        WriteAssembly(strOutputDir + m_strFilePrefix + kernelName + ".asm", pDisassembly);
    }
}

void KernelAssembly::AddKernelName(string kernelName, ID3DBlob* pBlob)
{
    AMDTScopeLock lock(m_pmutex);

    KernelNameMap::iterator it = kernelNameMap.find(pBlob->GetBufferPointer());

    //int checksum = GetChecksum(pBlob->GetBufferPointer(),pBlob->GetBufferSize());

    Log(traceMESSAGE, "Shader %s Added", kernelName.c_str());

    if (it == kernelNameMap.end())
    {
        kernelNameMap.insert(pair<const void*, string>(pBlob->GetBufferPointer(), kernelName));
    }
    else
    {
        // happens when the last Blob from CompileShader* get deleted and new Blob happens to have the same address
        // delete the old one
        kernelNameMap.erase(it);
        // replace it with new one
        kernelNameMap.insert(pair<const void*, string>(pBlob->GetBufferPointer(), kernelName));
    }
}

void KernelAssembly::GetKernelInformation(ID3D11ComputeShader* pCS,
                                          ID3DBlob** pBlob,
                                          string& kernelName,
                                          UINT& groupSizeX,
                                          UINT& groupSizeY,
                                          UINT& groupSizeZ) const
{
    KernelMap::const_iterator it = kernelMap.find(pCS);

    if (it == kernelMap.end())
    {
        // no information found
        // could happen in Heaven benchmark
        //Log(logWARNING,"KernelAssembly::GetKernelInformation() : No compute shader found\n");
        *pBlob = NULL;
        kernelName.clear();
        return;
    }

    *pBlob = it->second.m_pAssemblyBlob;
    kernelName = it->second.m_strKernelName;
    groupSizeX = it->second.m_uiGroupSizeX;
    groupSizeY = it->second.m_uiGroupSizeY;
    groupSizeZ = it->second.m_uiGroupSizeZ;
}

void KernelAssembly::WriteAssembly(string fileName, ID3DBlob* pBlob)
{
    if (pBlob == NULL)
    {
        Log(logERROR, "KernelAssembly::WriteAssembly() : NULL Pointer\n");
        return;
    }

    std::cout << "Writing to file: " << fileName << std::endl;

    FILE* f;
    fopen_s(&f, fileName.c_str(), "w");
    SpAssert(f);
    fwrite(pBlob->GetBufferPointer(), sizeof(char), pBlob->GetBufferSize(), f);
    fclose(f);
}

int KernelAssembly::GetChecksum(void* pBuff, UINT size) const
{
    int ret = 0;
    unsigned char* pByteBuf = (unsigned char*)pBuff;

    for (UINT i = 0; i < size; i++)
    {
        ret += pByteBuf[i];
    }

    return ret;
}