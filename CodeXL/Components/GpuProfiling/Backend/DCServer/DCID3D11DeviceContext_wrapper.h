//=====================================================================
//
// Author: Raul Aguaviva
//         Graphics Products Group
//         Developer Tools
//         AMD, Inc.
//
// ID3D11DeviceContext_wrapper.h
//
//
//=====================================================================
// $Id: //devtools/main/CodeXL/Components/GpuProfiling/Backend/DCServer/DCID3D11DeviceContext_wrapper.h#1 $
//
// Last checkin:  $DateTime: 2013/01/15 15:25:13 $
// Last edited by: $Author: chesik $
//=====================================================================
//   ( C ) AMD, Inc.  All rights reserved.
//=====================================================================
//==============================================================================
// Copyright (c) 2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief We create the wrapper because DX runtime often writes to vtable of immediate context and our patched value gets restored sometime.
//         Therefore, we need a wrapper for ID3D11DeviceContext and patch on it so that DX runtime can't touch it.
//==============================================================================

#ifndef _DC_ID3D11DEVICECONTEXT_WRAPPER_H_
#define _DC_ID3D11DEVICECONTEXT_WRAPPER_H_

/// \addtogroup DCVirtualTablePatching
// @{

/// Wrap DeviceContext
/// \param pReal Real DeviceContext
/// \return Wrapped DeviceContext
ID3D11DeviceContext* WrapDeviceContext(ID3D11DeviceContext* pReal);

/// Look up wrapped DeviceContext by real DeviceContext pointer
/// \param pReal Real DeviceContext
/// \return Wrapped DeviceContext
ID3D11DeviceContext* GetWrappedDevice(ID3D11DeviceContext* pReal);

/// Look up real DeviceContext by wrapped DeviceContext pointer
/// \param pWrapped DeviceContext
/// \return pReal Real DeviceContext
ID3D11DeviceContext* GetRealDeviceContext11(ID3D11DeviceContext* pWrapped);

/// Delete all wrappers when DCServer is deteching and client app is about to close
/// Be careful to use this function
/// If client app is still running, this function will cause client app to crash
void CleanupWrappers();

// @}

#endif // _DC_ID3D11DEVICECONTEXT_WRAPPER_H_
