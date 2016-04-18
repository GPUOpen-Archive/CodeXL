//=====================================================================
//
// Author: Lihan Bin
//         GPU Developer Tools
//         AMD, Inc.
//
// DCDetourIDXGIFactory.h
// Currently not used
//
//=====================================================================
// $Id: //devtools/main/CodeXL/Components/GpuProfiling/Backend/DCServer/DCDetourIDXGIFactory.h#1 $
//
// Last checkin:  $DateTime: 2013/01/15 15:25:13 $
// Last edited by: $Author: chesik $
//=====================================================================
//   ( C ) AMD, Inc. 2010 All rights reserved.
//=====================================================================
//==============================================================================
// Copyright (c) 2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief Currently not used
//==============================================================================

#ifndef _DC_DETOUR_IDXGIFACTORY_H_
#define _DC_DETOUR_IDXGIFACTORY_H_

#include <windows.h>
#include <d3d11.h>

/// \addtogroup DCDetour
// @{

/// Detour IDXGIFactory member functions
/// \param pFactory Pointer to IDXGIFactory
/// \return true if detoured
bool DetourAttachIDXGIFactory(IDXGIFactory1* pFactory);

/// Restore detoured member functions
/// \return true if detached
bool DetourDetachIDXGIFactory();

// @}

#endif // _DC_DETOUR_IDXGIFACTORY_H_