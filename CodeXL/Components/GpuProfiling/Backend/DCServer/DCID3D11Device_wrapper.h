//==============================================================================
// Copyright (c) 2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief
//==============================================================================


#ifndef _DC_ID3D11DEVICE_WRAPPER_H_
#define _DC_ID3D11DEVICE_WRAPPER_H_

/// \addtogroup DCVirtualTablePatching
// @{

/// Wrap Device
/// \param pReal Real Device
/// \return Wrapped Device
ID3D11Device* GetWrappedDevice(ID3D11Device* pReal);

/// Look up wrapped Device by real Device pointer
/// \param pReal Real Device
/// \return Wrapped Device
ID3D11Device* WrapDevice(ID3D11Device* pReal);

/// Look up real Device by wrapped Device pointer
/// \param pWrapped Device
/// \return pReal Real Device
ID3D11Device* GetRealDevice11(ID3D11Device* pWrapped);

// @}


#endif // _DC_ID3D11DEVICE_WRAPPER_H_