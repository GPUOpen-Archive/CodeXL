//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gdStatusBar.h
///
//==================================================================================

//------------------------------ gdStatusBar.h ------------------------------

#ifndef __GDSTATUSBAR
#define __GDSTATUSBAR

// Infra:
#include <AMDTBaseTools/Include/gtString.h>

// Local:
#include <AMDTGpuDebuggingComponents/Include/gdGDebuggerAppDLLBuild.h>


// ----------------------------------------------------------------------------------
// Class Name:           gdStatusBar
// General Description:
//  Abstract base class for different status bar types.
//  (Example: wxWidgets based status bar, Visual Studio status bar, etc)
//
// Author:               Yaki Tebeka
// Creation Date:        7/12/2003
// ----------------------------------------------------------------------------------
class GD_API gdStatusBar
{
public:
    static gdStatusBar* instance();

    virtual ~gdStatusBar();
    virtual void setText(const gtString& text, int fieldIndex = 0) = 0;

protected:
    gdStatusBar();
    static void setSingleInstance(gdStatusBar& instance);

    // Holds this class single instance:
    static gdStatusBar* _pMySingleInstance;
};


#endif  // __GDSTATUSBAR
