//=============================================================
// Copyright (c) 2015 Advanced Micro Devices, Inc.
/// \author GPU Developer Tools
/// \file
/// \brief A wrapper class used to intercept the ALVRFactory member functions
//=============================================================

#ifndef ALVR_FACTORY_WRAPPER_H
#define ALVR_FACTORY_WRAPPER_H

#include "LiquidVr.h"
#include "../Common/LiquidVRSupport.h"
#include "ALVRGPUAffinityWrapper.h"

#ifdef LIQUID_VR_SUPPORT

typedef void(*WrapALVRDeviceExD3D11_type)(ALVRDeviceExD3D11** ppDevice); ///< Function pointer typedf
typedef void(*WrapALVRComputeContext_type)(ALVRComputeContext** ppComputeContext); ///< Function pointer typedf

/// This class wraps the ALVRFactory and allows GPS to intercept the CreateGPUAffinity() function.
struct  ALVRFactoryWrapper : public ALVRFactory
{
private:

    /// Stores the real affinity object pointer
    ALVRFactory* m_pReal;

public:

    /// Constructor
    ALVRFactoryWrapper(ALVRFactory* pReal)
    {
        m_pReal = pReal;
    }

    /// Wrap the CreateGPUAffinity member function
    ALVR_RESULT ALVR_STD_CALL CreateGpuAffinity(ALVRGpuAffinity** ppAffinity)
    {
        ALVR_RESULT res = ALVR_OK;

        if (m_pReal != NULL)
        {
            res = m_pReal->CreateGpuAffinity(ppAffinity);

            ALVRGpuAffinityWrapper* pWrapper = new ALVRGpuAffinityWrapper(*ppAffinity);
            *ppAffinity = pWrapper;
        }

        return res;
    }

    /// Wrap the create device extension
    /// \param pd3dDevice Input ID3DDevice
    /// \param pConfigDesc Input config
    /// \param ppDevice Output ALVR device extension
    /// \return Result code
    virtual ALVR_RESULT ALVR_STD_CALL CreateALVRDeviceExD3D11(
        ID3D11Device* pd3dDevice,
        void* pConfigDesc, // optional configuration info reserved
        ALVRDeviceExD3D11** ppDevice)
    {
        ALVR_RESULT res =  m_pReal->CreateALVRDeviceExD3D11(pd3dDevice, pConfigDesc, ppDevice);

        // Start talking to the DX11 Server
        HMODULE hDX11Module = NULL;
        // Get the DX11Server dll
        GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_PIN, "DX11Server" GDT_PROJECT_SUFFIX ".dll", &hDX11Module);

        // Wrap the device
        WrapALVRDeviceExD3D11_type pWrapALVRDeviceExD3D1 = (WrapALVRDeviceExD3D11_type)GetProcAddress(hDX11Module, "WrapALVRDeviceExD3D11");

        if (pWrapALVRDeviceExD3D1 != NULL)
        {
            pWrapALVRDeviceExD3D1(ppDevice);
        }

        return res;
    }

    /// Wrap the crate  compute fucntion
    /// \param pDevice Input ALVR device
    /// \param gpuIdx GPU index
    /// \param pDesc Compute context description
    /// \param ppContext Output Compute context
    /// \return Result code
    virtual  ALVR_RESULT ALVR_STD_CALL CreateComputeContext(
        ALVRDeviceEx* pDevice,
        unsigned int gpuIdx,
        ALVRComputeContextDesc* pDesc,
        ALVRComputeContext** ppContext)
    {
        ALVR_RESULT res = m_pReal->CreateComputeContext(pDevice, gpuIdx, pDesc, ppContext);

        // Start talking to the DX11 Server
        HMODULE hDX11Module = NULL;
        // Get the DX11Server dll
        GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_PIN, "DX11Server" GDT_PROJECT_SUFFIX ".dll", &hDX11Module);

        // Wrap the compute context
        WrapALVRComputeContext_type pWrapComputeContext = (WrapALVRComputeContext_type)GetProcAddress(hDX11Module, "WrapALVRComputeContext");

        if (pWrapComputeContext != NULL)
        {
            pWrapComputeContext(ppContext);
        }

        return res;
    }
};

#endif // LIQUID_VR_SUPPORT

#endif // ALVR_FACTORY_WRAPPER_H