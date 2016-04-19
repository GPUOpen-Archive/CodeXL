//=============================================================================
/// Copyright (c) 2016 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file   Hook_D3D12.cpp
/// \brief  Responsible for hooking D3D12 entrypoint functions.
//=============================================================================

#include "Hook_D3D12.h"
#include "../DX12LayerManager.h"
#include "../Objects/DX12CreateInfoStructs.h"
#include "../../Common/OSWrappers.h"
#include <AMDTOSWrappers/Include/osFilePath.h>
#include <AMDTOSWrappers/Include/osModule.h>
#include "../../Common/SharedGlobal.h"
#include "../Profiling/DX12CmdListProfiler.h"
#include "../Profiling/DX12FrameProfilerLayer.h"
#ifndef DLL_REPLACEMENT
    #include "Interceptor.h"
#endif
#include "DX12Interceptor.h"
#include "DX12Defines.h"

#include "../Objects/BaseWrappers/Wrapped_ID3D12Device.h"
#include "../Objects/BaseWrappers/Wrapped_ID3D12RootSignatureDeserializer.h"

//-----------------------------------------------------------------------------
/// Gets a debug interface.
/// \param riid The globally unique identifier (GUID) for the debug interface.
/// \param ppvDebug The debug interface, as a pointer to pointer to void.
/// \returns This method returns one of the Direct3D 12 Return Codes.
//-----------------------------------------------------------------------------
HRESULT WINAPI Mine_D3D12GetDebugInterface(_In_ REFIID riid, _COM_Outptr_opt_ void** ppvDebug)
{
    Hook_D3D12& d3d12Interceptor = GetDX12LayerManager()->GetModuleHook();
    HRESULT getDebugInterfaceResult = d3d12Interceptor.mHook_GetDebugInterface.mRealHook(riid, ppvDebug);
    return getDebugInterfaceResult;
}

//-----------------------------------------------------------------------------
/// The Mine_ version of the hooked D3D12CreateDevice function. This must
/// be hooked in order to create a device wrapper instance.
/// \param pAdapter A pointer to the video adapter to use when creating a device.
/// \param MinimumFeatureLevel The minimum D3D_FEATURE_LEVEL required for successful device creation.
/// \param riid The globally unique identifier (GUID) for the device interface. See remarks.
/// \param ppDevice A pointer to a memory block that receives a pointer to the device.
/// \returns S_OK if device creation succeeded, and an error code if it somehow failed.
//-----------------------------------------------------------------------------
HRESULT WINAPI Mine_D3D12CreateDevice(
    IUnknown* pAdapter,
    D3D_FEATURE_LEVEL MinimumFeatureLevel,
    REFIID riid, // Expected: ID3D12Device
    void** ppDevice)
{
    Hook_D3D12& d3d12Interceptor = GetDX12LayerManager()->GetModuleHook();

    if (SG_GET_BOOL(OptionDebugRuntime) == true)
    {
        // Enable the debug layer through the debug interface.
        ID3D12Debug* debugInterface = nullptr;
        HRESULT enableDebugResult = d3d12Interceptor.mHook_GetDebugInterface.mRealHook(__uuidof(ID3D12Debug), (void**)&debugInterface);

        if (enableDebugResult == S_OK)
        {
            // Enable the debug layer
            debugInterface->EnableDebugLayer();
        }
    }

    // Invoke the real runtime CreateDevice function.
    HRESULT deviceCreateResult = d3d12Interceptor.mHook_CreateDevice.mRealHook(pAdapter, MinimumFeatureLevel, riid, ppDevice);

    // If we're going to collect framerate statistics, don't wrap the device, or anything else that a wrapped device may create.
    // This removes the overhead of wrapped ID3D12 objects, while still wrapping DXGI Swapchains for Present calls.
    if (SG_GET_BOOL(OptionCollectFrameStats) == false)
    {
        if (deviceCreateResult == S_OK && ppDevice != nullptr && *ppDevice != nullptr)
        {
            Wrapped_ID3D12DeviceCreateInfo* deviceCreateInfo = new Wrapped_ID3D12DeviceCreateInfo(pAdapter, MinimumFeatureLevel);
            WrapD3D12Device((ID3D12Device**)ppDevice, deviceCreateInfo);
        }
    }

    return deviceCreateResult;
}

//-----------------------------------------------------------------------------
/// The Mine_ version of the hooked D3D12SerializeRootSignature function.
/// \param pRootSignature A pointer to a D3D12_ROOT_SIGNATURE structure that describes a root signature.
/// \param Version A D3D_ROOT_SIGNATURE_VERSION-typed value that specifies the version of root signature.
/// \param ppBlob A pointer to a memory block that receives a pointer to the ID3DBlob interface that you can use to access the serialized root signature.
/// \param ppErrorBlob A pointer to a memory block that receives a pointer to the ID3DBlob interface that you can use to access serializer error messages, or nullptr if there are no errors.
/// \returns S_OK if successful; otherwise, returns one of the Direct3D 12 Return Codes.
//-----------------------------------------------------------------------------
HRESULT WINAPI Mine_D3D12SerializeRootSignature(
    const D3D12_ROOT_SIGNATURE_DESC* pRootSignature,
    D3D_ROOT_SIGNATURE_VERSION Version,
    ID3DBlob** ppBlob,
    ID3DBlob** ppErrorBlob)
{
    Hook_D3D12& d3d12Interceptor = GetDX12LayerManager()->GetModuleHook();
    HRESULT serializeResult = d3d12Interceptor.mHook_SerializeRootSignature.mRealHook(pRootSignature, Version, ppBlob, ppErrorBlob);

    return serializeResult;
}

//-----------------------------------------------------------------------------
/// The Mine_ version of the hooked D3D12CreateRootSignatureDeserializer function.
/// \param pSrcData A pointer to the source data for the serialized root signature.
/// \param SrcDataSizeInBytes The size, in bytes, of the block of memory that pSrcData points to.
/// \param pRootSignatureDeserializerInterface The globally unique identifier (GUID) for the root signature deserializer interface.
/// \param ppRootSignatureDeserializer A pointer to a memory block that receives a pointer to the root signature deserializer.
/// \returns S_OK if successful; otherwise, returns one of the Direct3D 12 Return Codes.
//-----------------------------------------------------------------------------
HRESULT WINAPI Mine_D3D12CreateRootSignatureDeserializer(
    LPCVOID pSrcData,
    SIZE_T SrcDataSizeInBytes,
    REFIID pRootSignatureDeserializerInterface,
    void** ppRootSignatureDeserializer)
{
    Hook_D3D12& d3d12Interceptor = GetDX12LayerManager()->GetModuleHook();
    HRESULT createResult = d3d12Interceptor.mHook_CreateRootSignatureDeserializer.mRealHook(pSrcData, SrcDataSizeInBytes, pRootSignatureDeserializerInterface, ppRootSignatureDeserializer);

    if (createResult == S_OK && *ppRootSignatureDeserializer != nullptr)
    {
        Wrapped_ID3D12RootSignatureDeserializerCreateInfo* rootSignatureDeserializerCreateInfo = new Wrapped_ID3D12RootSignatureDeserializerCreateInfo(pSrcData, SrcDataSizeInBytes);
        WrapD3D12RootSignatureDeserializer((ID3D12RootSignatureDeserializer**)ppRootSignatureDeserializer, rootSignatureDeserializerCreateInfo);
    }
    else
    {
        Log(logWARNING, "D3D12CreateRootSignatureDeserializer failed. Result = '%d'\n", createResult);
    }

    return createResult;
}

// When operating in DLL_REPLACEMENT mode, the following exported functions are used as replacement functions.
#ifdef DLL_REPLACEMENT
HRESULT WINAPI D3D12GetDebugInterface(_In_ REFIID riid, _COM_Outptr_opt_ void** ppvDebug)
{
    CheckUpdateHooks();
    return Mine_D3D12GetDebugInterface(riid, ppvDebug);
}

HRESULT WINAPI D3D12CreateDevice(
    _In_opt_ IUnknown* pAdapter,
    D3D_FEATURE_LEVEL MinimumFeatureLevel,
    _In_ REFIID riid, // Expected: ID3D12Device
    _COM_Outptr_opt_ void** ppDevice)
{
    CheckUpdateHooks();
    return Mine_D3D12CreateDevice(pAdapter, MinimumFeatureLevel, riid, ppDevice);
}

HRESULT WINAPI D3D12SerializeRootSignature(
    _In_ const D3D12_ROOT_SIGNATURE_DESC* pRootSignature,
    _In_ D3D_ROOT_SIGNATURE_VERSION Version,
    _Out_ ID3DBlob** ppBlob,
    _Always_(_Outptr_opt_result_maybenull_) ID3DBlob** ppErrorBlob)
{
    CheckUpdateHooks();
    return Mine_D3D12SerializeRootSignature(pRootSignature, Version, ppBlob, ppErrorBlob);
}

HRESULT WINAPI D3D12CreateRootSignatureDeserializer(
    _In_reads_bytes_(SrcDataSizeInBytes) LPCVOID pSrcData,
    _In_ SIZE_T SrcDataSizeInBytes,
    _In_ REFIID pRootSignatureDeserializerInterface,
    _Out_ void** ppRootSignatureDeserializer)
{
    CheckUpdateHooks();
    return Mine_D3D12CreateRootSignatureDeserializer(pSrcData, SrcDataSizeInBytes, pRootSignatureDeserializerInterface, ppRootSignatureDeserializer);
}
#endif // DLL_REPLACEMENT

//-----------------------------------------------------------------------------
/// Default constructor for Hook_D3D12.
//-----------------------------------------------------------------------------
Hook_D3D12::Hook_D3D12()
    : mRealD3D12(nullptr)
{
}

//-----------------------------------------------------------------------------
/// Default destructor for Hook_D3D12.
//-----------------------------------------------------------------------------
Hook_D3D12::~Hook_D3D12()
{
}

//-----------------------------------------------------------------------------
/// Attach API entrypoints for hooking.
/// \returns True if entrypoints were successfully hooked.
//-----------------------------------------------------------------------------
bool Hook_D3D12::HookInterceptor()
{
    bool bThisInitialized = false;
    char* errorString = nullptr;

    gtString moduleFilename;
    moduleFilename.fromASCIIString(D3D12_DLL);
    osFilePath modulePath(moduleFilename);
    osModuleHandle d3dModuleHandle;

#ifdef DLL_REPLACEMENT
    // check to see if DX12 is loaded already. If not, go get it
    bool success = true;

    if (!osGetLoadedModuleHandle(modulePath, d3dModuleHandle))
    {
        if (!osLoadModule(modulePath, d3dModuleHandle))
        {
            success = false;
        }
    }

    if (success)
    {
        PFN_D3D12_GET_DEBUG_INTERFACE D3D12GetDebugInterface_funcPtr;
        PFN_D3D12_CREATE_DEVICE D3D12CreateDevice_funcPtr;
        PFN_D3D12_SERIALIZE_ROOT_SIGNATURE D3D12SerializeRootSignature_funcPtr;
        PFN_D3D12_CREATE_ROOT_SIGNATURE_DESERIALIZER D3D12CreateRootSignatureDeserializer_funcPtr;

        D3D12CreateDevice_funcPtr = (PFN_D3D12_CREATE_DEVICE)GetProcAddress(mRealD3D12, "D3D12CreateDevice");
        D3D12SerializeRootSignature_funcPtr = (PFN_D3D12_SERIALIZE_ROOT_SIGNATURE)GetProcAddress(mRealD3D12, "D3D12SerializeRootSignature");
        D3D12CreateRootSignatureDeserializer_funcPtr = (PFN_D3D12_CREATE_ROOT_SIGNATURE_DESERIALIZER)GetProcAddress(mRealD3D12, "D3D12CreateRootSignatureDeserializer");
        D3D12GetDebugInterface_funcPtr = (PFN_D3D12_GET_DEBUG_INTERFACE)GetProcAddress(mRealD3D12, "D3D12GetDebugInterface");

        mHook_GetDebugInterface.SetHooks(D3D12GetDebugInterface_funcPtr, Mine_D3D12GetDebugInterface);
        mHook_CreateDevice.SetHooks(D3D12CreateDevice_funcPtr, Mine_D3D12CreateDevice);
        mHook_SerializeRootSignature.SetHooks(D3D12SerializeRootSignature_funcPtr, Mine_D3D12SerializeRootSignature);
        mHook_CreateRootSignatureDeserializer.SetHooks(D3D12CreateRootSignatureDeserializer_funcPtr, Mine_D3D12CreateRootSignatureDeserializer);
        bThisInitialized = true;
    }

#else

    // Attempt to load the where the hooked functions reside.
    if (osLoadModule(modulePath, d3dModuleHandle))
    {
        // Initialize hooking, and then hook the list of entrypoints below.
        LONG hookError = AMDT::BeginHook();

        if (hookError == NO_ERROR)
        {
            // D3D12GetDebugInterface
            osProcedureAddress D3D12GetDebugInterface_funcPtr;

            if (osGetProcedureAddress(d3dModuleHandle, "D3D12GetDebugInterface", D3D12GetDebugInterface_funcPtr, true) == true)
            {
                mHook_GetDebugInterface.SetHooks((PFN_D3D12_GET_DEBUG_INTERFACE)D3D12GetDebugInterface_funcPtr, Mine_D3D12GetDebugInterface);
                bThisInitialized = mHook_GetDebugInterface.Attach();
            }
            else
            {
                Log(logERROR, "Failed to initialize hook for export with name '%s'.", "D3D12GetDebugInterface");
            }

            // D3D12CreateDevice
            osProcedureAddress D3D12CreateDevice_funcPtr;

            if (osGetProcedureAddress(d3dModuleHandle, "D3D12CreateDevice", D3D12CreateDevice_funcPtr, true) == true)
            {
                mHook_CreateDevice.SetHooks((PFN_D3D12_CREATE_DEVICE)D3D12CreateDevice_funcPtr, Mine_D3D12CreateDevice);
                bThisInitialized = mHook_CreateDevice.Attach();
            }
            else
            {
                Log(logERROR, "Failed to initialize hook for export with name '%s'.", "D3D12CreateDevice");
            }

            // D3D12SerializeRootSignature
            osProcedureAddress D3D12SerializeRootSignature_funcPtr;

            if (osGetProcedureAddress(d3dModuleHandle, "D3D12SerializeRootSignature", D3D12SerializeRootSignature_funcPtr, true) == true)
            {
                mHook_SerializeRootSignature.SetHooks((PFN_D3D12_SERIALIZE_ROOT_SIGNATURE)D3D12SerializeRootSignature_funcPtr, Mine_D3D12SerializeRootSignature);
                bThisInitialized = mHook_SerializeRootSignature.Attach();
            }
            else
            {
                Log(logERROR, "Failed to initialize hook for export with name '%s'.", "D3D12SerializeRootSignature");
            }

            // D3D12CreateRootSignatureDeserializer
            osProcedureAddress D3D12CreateRootSignatureDeserializer_funcPtr;

            if (osGetProcedureAddress(d3dModuleHandle, "D3D12CreateRootSignatureDeserializer", D3D12CreateRootSignatureDeserializer_funcPtr, true) == true)
            {
                mHook_CreateRootSignatureDeserializer.SetHooks((PFN_D3D12_CREATE_ROOT_SIGNATURE_DESERIALIZER)D3D12CreateRootSignatureDeserializer_funcPtr, Mine_D3D12CreateRootSignatureDeserializer);
                bThisInitialized = mHook_CreateRootSignatureDeserializer.Attach();
            }
            else
            {
                Log(logERROR, "Failed to initialize hook for export with name '%s'.", "D3D12SerializeRootSignature");
            }

            // Commit the hooked calls.
            hookError = AMDT::EndHook();

            if (hookError != NO_ERROR)
            {
                errorString = "EndHook Failed.";
            }
        }
        else
        {
            errorString = "BeginHook Failed.";
        }
    }
    else
    {
        errorString = "Failed to find API module to hook.";
    }

#endif // DLL_REPLACEMENT

    if (!bThisInitialized)
    {
        Log(logERROR, "%s failed to hook: %s\n", __FUNCTION__, errorString);
    }

    return bThisInitialized;
}

//-----------------------------------------------------------------------------
/// Detach all hooked API entrypoints.
/// \returns True if entrypoints were successfully detached.
//-----------------------------------------------------------------------------
bool Hook_D3D12::UnhookInterceptor()
{
    bool bDetachSuccessful = true;

#ifndef DLL_REPLACEMENT
    char* errorMessage = nullptr;

    LONG hookError = NO_ERROR;

    hookError = AMDT::BeginHook();

    if (hookError == NO_ERROR)
    {
        bDetachSuccessful &= mHook_GetDebugInterface.Detach();
        bDetachSuccessful &= mHook_CreateDevice.Detach();
        bDetachSuccessful &= mHook_SerializeRootSignature.Detach();
        bDetachSuccessful &= mHook_SerializeRootSignature.Detach();

        if (bDetachSuccessful)
        {
            hookError = AMDT::EndHook();

            if (hookError == NO_ERROR)
            {
                bDetachSuccessful = true;
            }
            else
            {
                errorMessage = "EndHook failed.";
            }
        }
        else
        {
            errorMessage = "Unhooking failed at detachment.";
        }
    }
    else
    {
        errorMessage = "BeginHook failed.";
    }

    if (!bDetachSuccessful)
    {
        Log(logERROR, "Hook_D3D12 failed to unhook successfully: %s\n", errorMessage);
    }

#endif // DLL_REPLACEMENT

    return bDetachSuccessful;
}