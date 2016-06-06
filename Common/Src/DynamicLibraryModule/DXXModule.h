//==============================================================================
// Copyright (c) 2011-2016 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools
/// \file
/// \brief This class manages the dynamic loading of atidxx{32,64}.dll
//==============================================================================

#ifndef _DXX_MODULE_H_
#define _DXX_MODULE_H_

#ifndef _WIN32
    #error "This makes no sense if you are not on Windows."
#endif

#include "AmdDxGsaCompile.h"
#include "DynamicLibraryModule.h"

#define AMDDXX_INTERFACE_TABLE \
    X(AmdDxGsaCompileShader) \
    X(AmdDxGsaFreeCompiledShader)

// These are also present, but I don't immediately have their prototypes.
// I also don't need them to get started.
// TODO: complete this later?
//    X(AmdDxExtCreate) \
//    X(AmdDxExtCreate11) \
//    X(OpenAdapter10) \
//    X(OpenAdapter10_2) \
//    X(XdxInitXopAdapterServices) \
//    X(XdxInitXopServices)

/// This class handles the dynamic loading of atidxx{32,64}.dll
/// \note There will typically be one of these objects.
///       That instance will be global.
///       There is a trap for the unwary.
///       The order of global ctors is only defined within a single compilation unit.
///       So, one should not use these interfaces before "main" is reached.
///       This is different than calling these functions when the .dll/.so is linked against.
class AMDDXXModule
{
public:

    /// Default name to use for construction.
    /// This is usually aticaldd.dll or libaticaldd.so.
    static const char* s_DefaultModuleName;

    /// Constructor
    /// \param module to load.
    AMDDXXModule(const std::string& moduleName);

    AMDDXXModule();

    /// destructor
    ~AMDDXXModule();

    /// Load module.
    /// \param[in] name The module name.
    /// \return         true if successful, false otherwise
    bool LoadModule(const std::string& name = s_DefaultModuleName);

    /// Unload the caldd shared image.
    void UnloadModule();

    /// Have we sucessfully loaded the cal module?
    /// \returns enumeration value to answer query.
    bool IsLoaded() { return m_ModuleLoaded; }

#define X(SYM) Pfn##SYM SYM;
    AMDDXX_INTERFACE_TABLE;
#undef X

private:
    /// Initialize the internal data
    void Initialize();

    /// Have we loaded the CAL module?
    bool                 m_ModuleLoaded;

    /// Helper.
    DynamicLibraryModule m_DynamicLibraryHelper;
};


// Interface to D3DCompiler_xx.dll

#include <D3Dcompiler.h>

#define D3DCOMPILE_INTERFACE_TABLE \
    X(D3DCompile) \
    X(D3DCompressShaders) \
    X(D3DCreateBlob) \
    X(D3DDecompressShaders) \
    X(D3DDisassemble) \
    X(D3DDisassemble10Effect) \
    X(D3DGetBlobPart) \
    X(D3DGetDebugInfo) \
    X(D3DGetInputAndOutputSignatureBlob) \
    X(D3DGetInputSignatureBlob) \
    X(D3DGetOutputSignatureBlob) \
    X(D3DPreprocess) \
    X(D3DReflect) \
    X(D3DStripShader)

// Missing from the header file.
//    X(D3DAssemble)
//    X(D3DReturnFailure1)
//    X(DebugSetMute)


// pointer prototypes.
// the header file defines some of these, but most are missing.
typedef HRESULT(WINAPI* pD3DGetDebugInfo)
(LPCVOID pSrcData,
 SIZE_T SrcDataSize,
 ID3DBlob** ppDebugInfo);

typedef HRESULT(WINAPI* pD3DReflect)
(LPCVOID pSrcData,
 SIZE_T SrcDataSize,
 REFIID pInterface,
 void** ppReflector);

typedef HRESULT(WINAPI* pD3DDisassemble10Effect)
(ID3D10Effect* pEffect,
 UINT Flags,
 ID3DBlob** ppDisassembly);

typedef HRESULT(WINAPI* pD3DGetInputSignatureBlob)
(LPCVOID pSrcData,
 SIZE_T SrcDataSize,
 ID3DBlob** ppSignatureBlob);

typedef HRESULT(WINAPI* pD3DGetOutputSignatureBlob)
(LPCVOID pSrcData,
 SIZE_T SrcDataSize,
 ID3DBlob** ppSignatureBlob);

typedef HRESULT(WINAPI* pD3DGetInputAndOutputSignatureBlob)
(LPCVOID pSrcData,
 SIZE_T SrcDataSize,
 ID3DBlob** ppSignatureBlob);

typedef HRESULT(WINAPI* pD3DStripShader)
(LPCVOID pShaderBytecode,
 SIZE_T BytecodeLength,
 UINT uStripFlags,
 ID3DBlob** ppStrippedBlob);

typedef HRESULT(WINAPI* pD3DGetBlobPart)
(LPCVOID pSrcData,
 SIZE_T SrcDataSize,
 D3D_BLOB_PART Part,
 UINT Flags,
 ID3DBlob** ppPart);

typedef HRESULT(WINAPI* pD3DCompressShaders)
(UINT uNumShaders,
 D3D_SHADER_DATA* pShaderData,
 UINT uFlags,
 ID3DBlob** ppCompressedData);

typedef HRESULT(WINAPI* pD3DDecompressShaders)
(__in_bcount(SrcDataSize) LPCVOID pSrcData,
 SIZE_T SrcDataSize,
 UINT uNumShaders,
 UINT uStartIndex,
 UINT* pIndices,
 UINT uFlags,
 ID3DBlob** ppShaders,
 UINT* pTotalShaders);

typedef HRESULT(WINAPI* pD3DCreateBlob)
(SIZE_T Size,
 ID3DBlob** ppBlob);

/// This class handles the dynamic loading of D3DCompiler_xx.dll
/// \note There will typically be one of these objects.
///       That instance will be global.
///       There is a trap for the unwary.
///       The order of global ctors is only defined within a single compilation unit.
///       So, one should not use these interfaces before "main" is reached.
///       This is different than calling these functions when the .dll/.so is linked against.
class D3DCompileModule
{
public:

    /// Default name to use for construction.
    /// This is usually aticaldd.dll or libaticaldd.so.
    static const char* s_DefaultModuleName;

    /// Constructor
    /// \param module to load.
    D3DCompileModule(const std::string& moduleName);

    /// destructor
    ~D3DCompileModule();

    /// Load module.
    /// \param[in] name         The module name.
    /// \param[in] pErrorCode   Error code if error occurred loading the module.
    /// \return         true if successful, false otherwise
    bool LoadModule(const std::string& name = s_DefaultModuleName, int* pErrorCode = NULL);

    /// Unload the caldd shared image.
    void UnloadModule();

    /// Have we sucessfully loaded the cal module?
    /// \returns enumeration value to answer query.
    bool IsLoaded() const { return m_isModuleLoaded; }

#define X(SYM) p##SYM SYM;
    D3DCOMPILE_INTERFACE_TABLE;
#undef X

private:
    /// Initialize the internal data
    void Initialize();

    /// Have we loaded the CAL module?
    bool                 m_isModuleLoaded;

    /// Helper.
    DynamicLibraryModule m_DynamicLibraryHelper;
};


#endif
