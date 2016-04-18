//==============================================================================
// Copyright (c) 2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief
//==============================================================================

#ifndef _DC_ID3D11DEVICE_VTABLE_MANAGER_H_
#define _DC_ID3D11DEVICE_VTABLE_MANAGER_H_

#include "dcdetourhelper.h"
#include "DCID3D11Device_typedefs.h"

/// \addtogroup DCVirtualTablePatching
// @{

//------------------------------------------------------------------------------------
/// ID3D11Device VTable manager. It contains mine_* member function pointers that derived class should assign to and encapsulates Patch()
/// We shouldn't create an instance of it but inherit from it.
//------------------------------------------------------------------------------------
class DCID3D11DeviceVTManager :
    public VTableManager
{
public:
    /// Destructor
    virtual ~DCID3D11DeviceVTManager(void);

    /// Patch vtable
    /// \param ppDev pointer to ID3D11Device pointer
    /// \param bAppCreated Is it created by client app?
    void Patch(ID3D11Device** ppDev, bool bAppCreated);

protected:
    /// Constructor - We shouldn't create an instance of it.
    /// \param pFnMine Release Function pointer
    DCID3D11DeviceVTManager(ptrdiff_t* pFnMine);

private:
    /// Copy constructor - disabled
    /// \param obj source
    DCID3D11DeviceVTManager(const DCID3D11DeviceVTManager& obj);

    /// Assignment operator - disabled
    /// \param obj left
    /// \return right
    DCID3D11DeviceVTManager& operator = (const DCID3D11DeviceVTManager& obj);

protected:
    ID3D11Device_CreateBuffer_type m_pMine_ID3D11Device_CreateBufferFn;      ///< Detoured member function pointer for ID3D11Device
    ID3D11Device_CreateTexture1D_type m_pMine_ID3D11Device_CreateTexture1DFn;      ///< Detoured member function pointer for ID3D11Device
    ID3D11Device_CreateTexture2D_type m_pMine_ID3D11Device_CreateTexture2DFn;      ///< Detoured member function pointer for ID3D11Device
    ID3D11Device_CreateTexture3D_type m_pMine_ID3D11Device_CreateTexture3DFn;      ///< Detoured member function pointer for ID3D11Device
    ID3D11Device_CreateUnorderedAccessView_type m_pMine_ID3D11Device_CreateUnorderedAccessViewFn;      ///< Detoured member function pointer for ID3D11Device
    ID3D11Device_CreateDeferredContext_type m_pMine_ID3D11Device_CreateDeferredContextFn;      ///< Detoured member function pointer for ID3D11Device
    ID3D11Device_GetImmediateContext_type m_pMine_ID3D11Device_GetImmediateContextFn;      ///< Detoured member function pointer for ID3D11Device
    ID3D11Device_CreateComputeShader_type m_pMine_ID3D11Device_CreateComputeShaderFn;      ///< Detoured member function pointer for ID3D11Device
};

// @}

#endif // _DC_ID3D11DEVICE_VTABLE_MANAGER_H_