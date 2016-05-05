//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file StdAfx.h
///
//==================================================================================

// stdafx.h : include file for standard system include files,
//      or project specific include files that are used frequently,
//      but are changed infrequently


#pragma once

// Infra:
//#include <AMDTBaseTools/Include/gtIgnoreCompilerWarnings.h>
// Windows Platform headers and control defines
#define STRICT
#define _WIN32_WINNT 0x0601 // Visual Studio requires Windows 2000 or better

#ifndef NOMINMAX
    #define NOMINMAX // Windows Platform min and max macros cause problems for the Standard C++ Library
#endif
#ifndef WIN32_LEAN_AND_MEAN
    #define WIN32_LEAN_AND_MEAN // Exclude rarely-used stuff from the Windows Platform headers
#endif

// ATL headers and control defines
#define _ATL_APARTMENT_THREADED
#define _ATL_REGISTER_PER_USER

#include <atlbase.h>
#include <atlcom.h>
#include <atlwin.h>
#include <atlstr.h>
#include <atlfile.h>
#include <atlsafe.h>

// Windows Platform headers
#include <windowsx.h> // REVIEW - what is this for?
#include <richedit.h>
#include <TOM.h>


// Visual Studio Platform headers
#include <dte.h> // for extensibility
#include <objext.h> // for ILocalRegistry
#include <vshelp.h> // for Help
#include <uilocale.h> // for IUIHostLocale2
#include <IVsQueryEditQuerySave2.h> // for IVsQueryEditQuerySave2
#include <vbapkg.h> // for IVsMacroRecorder
#include <fpstfmt.h> // for IPersistFileFormat
#include <VSRegKeyNames.h>
#include <stdidcmd.h>
#include <stdiduie.h> // For status bar consts.
#include <textfind.h>
#include <textmgr.h>

// Debugging
#include <msdbg.h>
#include "dbgmetric_short.h"

// D3D11 has to be included before VSL, since they both define an operator== for const RECT&:
#include <d3d11.h>

// VSL headers
#define VSLASSERT _ASSERTE
#define VSLASSERTEX(exp, szMsg) _ASSERT_BASE(exp, szMsg)
#define VSLTRACE ATLTRACE

#include <VSLPackage.h>
#include <VSLCommandTarget.h>
#include <VSLWindows.h>
#include <VSLControls.h>
#include <VSLFile.h>
#include <VSLContainers.h>
#include <VSLComparison.h>
#include <VSLAutomation.h>
#include <VSLFindAndReplace.h>
#include <VSLShortNameDefines.h>

using namespace VSL;

