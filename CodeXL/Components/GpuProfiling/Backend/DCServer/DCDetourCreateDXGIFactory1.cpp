//==============================================================================
// Copyright (c) 2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief This class simply gets the "real" function pointer for CreateDXGIFactory1
//==============================================================================

#include "DCDetourCreateDXGIFactory1.h"

bool DCDetourCreateDXGIFactory1::OnAttach()
{
    // don't detour twice
    if (Real_CreateDXGIFactory1 == NULL)
    {
        Real_CreateDXGIFactory1 = (CreateDXGIFactory1_type)GetProcAddress(m_hMod, "CreateDXGIFactory1");
    }

    return true;
}

bool DCDetourCreateDXGIFactory1::Detach()
{
    // don't detach detour if not attached - this is a valid opeation
    if (Real_CreateDXGIFactory1 != NULL)
    {
        Real_CreateDXGIFactory1 = NULL;
    }

    return true;
}
