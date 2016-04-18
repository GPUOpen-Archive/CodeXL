//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file vspGRApiFunctions.h
///
//==================================================================================

//------------------------------ vspGRApiFunctions.h ------------------------------

#ifndef __VSPGRAPIFUNCTIONS_H
#define __VSPGRAPIFUNCTIONS_H

// Core interfaces:
#include <Include\Public\CoreInterfaces\IVscGRApiFunctionsOwner.h>

// Infra:
#include <AMDTApiFunctions/Include/gaGRApiFunctions.h>


// ----------------------------------------------------------------------------------
// Class Name:           vspGRApiFunctions : public gaGRApiFunctions
// General Description: Overrides several API functions with Visual Studio-specific
//                      implementations.
// Author:               Uri Shomroni
// Creation Date:        28/9/2010
// ----------------------------------------------------------------------------------
class vspGRApiFunctions : public gaGRApiFunctions
{
public:
    vspGRApiFunctions();
    virtual ~vspGRApiFunctions();
    static vspGRApiFunctions& vspInstance();

    static void setOwner(IVscGRApiFunctionsOwner* pOwner);

    // Overrides gaGRApiFunctions:
    virtual bool gaSetBreakpoint(const apBreakPoint& breakpoint);
    virtual bool gaRemoveBreakpoint(int breakpointId);
    virtual bool gaRemoveAllBreakpoints();

    virtual bool gaSetHexDisplayMode(bool hexMode);

    // Allow calling the real functions:
    bool setAPIBreakpoint(const apBreakPoint& breakpoint);
    bool removeAPIBreakpoint(int breakpointIndex);
    bool removeAllAPIBreakpoints();

private:
    static IVscGRApiFunctionsOwner* m_pOwner;
};

#endif //__VSPGRAPIFUNCTIONS_H

