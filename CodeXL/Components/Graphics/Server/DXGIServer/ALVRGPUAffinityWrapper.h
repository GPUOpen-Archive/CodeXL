//=============================================================
// Copyright (c) 2015 Advanced Micro Devices, Inc.
/// \author GPU Developer Tools
/// \file
/// \brief A wrapper class used to intercept the ALVRGpuAffinity member functions
//=============================================================

#ifndef ALVR_GPU_AFFINITY_WRAPPER_H
#define ALVR_GPU_AFFINITY_WRAPPER_H

#include "LiquidVR.h"
#include "DXGILayerManager.h"
#include "../Common/LiquidVRSupport.h"
#include "../Common/SharedGlobal.h"

#ifdef LIQUID_VR_SUPPORT

typedef void(*WrapDeviceContext_type)(ID3D11DeviceContext** ppDevCon); ///< Function pointer typedf
typedef void(*WrapDevice_type)(ID3D11Device* pReal, ID3D11Device** ppDev); ///< Function pointer typedf
typedef void(*UnWrapDevice_type)(ID3D11Device** ppDev); ///< Function pointer typedf
typedef void(*RemoveWrappedDeviceContext_type)(ID3D11DeviceContext** ppDevCon); ///< Function pointer typedf
typedef void(*WrapALVRMultiGpuDeviceContext_type)(ALVRMultiGpuDeviceContext** ppDevCon); ///< Function pointer typedf

#include "..\..\Common\Src\AmdDxExt\AmdDxExtPerfProfileApi.h"
#include "..\Common\IUnknownWrapperGUID.h"

/// Gets a count of the the number of GPU's present on the system
/// \param pDevice The input device
/// \param nCount The output count
/// \return False if the profiler extension could not be created for any reason.
bool GPUCount(ID3D11Device* pDevice, UINT* nCount)
{
    HMODULE hDll = NULL;

#ifdef X64
    hDll = GetModuleHandle("atidxx64.dll");
#else
    hDll = GetModuleHandle("atidxx32.dll");
#endif

    if (hDll == NULL)
    {
#ifdef X64
        Log(logERROR, "Unable to initialize because 'atidxx64.dll' is not available.");
#else
        Log(logERROR, "Unable to initialize because 'atidxx32.dll' is not available.");
#endif
        return false;
    }

    PFNAmdDxExtCreate11 AmdDxExtCreate11;
    AmdDxExtCreate11 = (PFNAmdDxExtCreate11)GetProcAddress(hDll, "AmdDxExtCreate11");

    if (AmdDxExtCreate11 == NULL)
    {
        Log(logERROR, "Unable to initialize because extension creation is not available.");
        return false;
    }

    IAmdDxExt* m_pExt = NULL;
    HRESULT hr = AmdDxExtCreate11(pDevice, &m_pExt);

    if (FAILED(hr) || m_pExt == NULL)
    {
        Log(logERROR, "Unable to initialize because extension creation is not available.");
        return false;
    }

    // Get the profiler extension
    IAmdDxExtInterface* pIface = m_pExt->GetExtInterface(AmdDxExtPerfProfileID);

    if (pIface != NULL)
    {
        IAmdDxExtPerfProfile* pControl = static_cast<IAmdDxExtPerfProfile*>(pIface);

        UINT count = 0;

        // Loop through 0-7 GPU's (Liquid VR supports 8 GPUs)
        for (UINT i = 0; i < 8; i++)
        {
            BOOL bRes = false;

            // Check to see if the GPU at index is profilable
            PE_RESULT res = pControl->IsGpuProfileable(i, &bRes) ;

            if (res == PE_OK && bRes == TRUE)
            {
                count++;
            }
        }

        *nCount = count;
    }

    return true;
}


/// Wrap the GPU affinity object
struct ALVRGpuAffinityWrapper : public ALVRGpuAffinity
{
    /// Pointer to the real object
    ALVRGpuAffinity* m_pReal;

public:

    /// Constructor
    /// \param pReal Pointer to the real interface
    ALVRGpuAffinityWrapper(ALVRGpuAffinity* pReal)
    {
        m_pReal = pReal;
    }

#include "../DX11Server/IUnknownWrapper.h"
#include "../DX11Server/ALVRPropertyStorageWrapper.h"

    /// Wrap the EnableGPUAffinity function
    /// \param flags Flags
    /// \return Result code
    virtual ALVR_RESULT ALVR_STD_CALL EnableGpuAffinity(unsigned int flags)
    {
        return m_pReal->EnableGpuAffinity(flags);
    }

    /// Wrap the DisableGPUAffinity function
    /// \return Result code
    virtual ALVR_RESULT ALVR_STD_CALL DisableGpuAffinity()
    {
        return m_pReal->DisableGpuAffinity();
    }

    /// Wrap the WrapDeviceD3D11 function
    /// \param pDevice Input device
    /// \param ppWrappedDevice Output Wrapped Device
    /// \param ppWrappedContext Output DeviceContext
    /// \param ppMultiGpuDeviceContext Output Affinity control object
    /// \return Result code
    virtual ALVR_RESULT ALVR_STD_CALL WrapDeviceD3D11(
        ID3D11Device* pDevice,
        ID3D11Device** ppWrappedDevice,
        ID3D11DeviceContext** ppWrappedContext,
        ALVRMultiGpuDeviceContext** ppMultiGpuDeviceContext)
    {
        printf("DXGI Server ALVRGPUAffinityWrapper::D3D11WrapDevice()\n");

        HMODULE hDX11Module = NULL;

        // Get the DX11Server dll
        GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_PIN, "DX11Server" GDT_PROJECT_SUFFIX ".dll", &hDX11Module);

        // If there are more than 1 GPUs we need to go on to unwrap the GPS wrapped device.
        ID3D11DeviceContext* pDevConOrig = NULL;
        pDevice->GetImmediateContext(&pDevConOrig);

        Log(logDEBUG, "1) LVR - Original Device with GPS creation wrapper 0x%p\n", pDevice);

        // New test. Check to see if the device is wrapped by GPS
        void* pRealDevice = NULL;
        HRESULT hres = pDevice->QueryInterface(IID_IWrappedObject, &pRealDevice);

        if (hres == S_OK)
        {
            ALVR_RESULT res = m_pReal->WrapDeviceD3D11((ID3D11Device*)pRealDevice, ppWrappedDevice, ppWrappedContext, ppMultiGpuDeviceContext);

            // Test to see if the LVR wrapping failed
            if (res != ALVR_OK)
            {
                // Check that the LVR Option was turned off
                if (SG_GET_BOOL(OptionLiquidVR) == true)
                {
                    MessageBoxError("LiquidVR is not supported with your current configuration. Restart your GPU PerfStudio debug session without the LiquidVR (-V or -liquidvr) startup option. Note: this option may have been set in the GPUPerfServer.cfg file, make sure that \"liquidvr=true\" is commented out.\n");
                    exit(0);
                }

                // LVR wrapping will fail if there is only 1 GPU present
                // In this case just return. The GPS wrapper will still be intact
                return res;
            }
        }

        // Check that the LVR Option was turned off
        if (SG_GET_BOOL(OptionLiquidVR) == false)
        {
            MessageBoxError("LiquidVR is being used. Restart your GPU PerfStudio debug session with the LiquidVR (-V or --liquidvr) server option. \n\n\
e.g. GPUPerfServer-x64.exe c:\\MyLiquidVRApp.exe -V \n\n\
Alternatively, you can also set this option permanently in the GPUPerfServer.cfg file by specifying \"liquidvr=true\".\n");
            exit(0);
        }

        Log(logDEBUG, "------------------------------------------------------------\n", pDevice);
        Log(logDEBUG, "2) LVR - D3D11WrapDevice() is being called now.\n", pDevice);
        Log(logDEBUG, "------------------------------------------------------------\n", pDevice);

        ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        // Liquid VR wrapping
        ALVR_RESULT res = m_pReal->WrapDeviceD3D11(pDevice, ppWrappedDevice, ppWrappedContext, ppMultiGpuDeviceContext);
        ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

        if (res != ALVR_OK)
        {
            Log(logERROR, "LiquidVR support has been requested but the system is unable to support it ALVR_RESULT %d\n", res);
            return res;
        }

        Log(logDEBUG, "3) LVR - Post-D3D11WrapDevice: LVR Wrapped Device 0x%p\n", *ppWrappedDevice);

        // 2) Wrap the device
        WrapDevice_type pWrapDev = (WrapDevice_type)GetProcAddress(hDX11Module, "WrapDevice");

        if (pWrapDev != NULL)
        {
            pWrapDev(pDevice, ppWrappedDevice);
            Log(logDEBUG, "4) LVR - Post-D3D11WrapDevice: GPS Wrapped Device 0x%p\n", *ppWrappedDevice);
        }

        // 4) Get the process address of the WrapDeviceContext function
        WrapDeviceContext_type pWrapDevCon = (WrapDeviceContext_type)GetProcAddress(hDX11Module, "WrapDeviceContext");

        if (pWrapDevCon != NULL)
        {
            pWrapDevCon(ppWrappedContext);
        }

        // 5) Wrap the MGPU Control object
        WrapALVRMultiGpuDeviceContext_type pWrapGPUControl = (WrapALVRMultiGpuDeviceContext_type)GetProcAddress(hDX11Module, "WrapALVRMultiGpuDeviceContext");

        if (pWrapGPUControl != NULL)
        {
            pWrapGPUControl(ppMultiGpuDeviceContext);
        }

        return res;
    }
};
#endif // LIQUID_VR_SUPPORT

#endif // ALVR_GPU_AFFINITY_WRAPPER_H