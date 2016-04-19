//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file vsdPackageConnector.h
///
//==================================================================================

//------------------------------ vsdPackageConnector.h ------------------------------

#ifndef __VSDPACKAGECONNECTOR_H
#define __VSDPACKAGECONNECTOR_H

// Forward declarations:
class vsdProcessDebugger;
class vsdCDebugProcess;

// Visual Studio:
/*#include <dte.h> // for extensibility
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
#include <textmgr.h>*/
#include <msdbg.h>
//#include <dbgmetric.h>

// Local:
#include <CodeXLVSPackageDebugger/Include/vsdPackageDLLBuild.h>



// ----------------------------------------------------------------------------------
// Class Name:          VSD_API vsdPackageConnector
// General Description: A class that connects the gDEBuggerVSPackageCode components to
//                      the Visual Studio package containing it.
// Author:               Uri Shomroni
// Creation Date:        22/12/2011
// ----------------------------------------------------------------------------------
class VSD_API vsdPackageConnector
{
public:
    static vsdPackageConnector& instance();
    ~vsdPackageConnector();

public:
    void initializeWithPackage(IDebugEngine2* piNativeDebugEngine, IDebugProgramProvider2* piNativeProgramProvider);
    void terminatePackageConnection();

    void setDebugPort(IDebugPort2* piDebugPort);
    IDebugPort2* getWrappedDebugPort();

    IDebugProcess2* debuggedProcess();
    void setProgramToBeEnumeratedByDebuggedProcess(IDebugProgram2* piDebugProgram);
    void setDebuggedProcess(vsdCDebugProcess* piDebuggedProcess);

private:
    friend class vsdSingletonsDelete;

private:
    // Only to be called by the instance() method:
    vsdPackageConnector();

private:
    static vsdPackageConnector* ms_pMySingleInstance;

    // Was this class successfully initialized?
    bool m_initialized;

    // The native debug engine:
    IDebugEngine2* m_piNativeDebugEngine;

    // The debugged process interface:
    vsdCDebugProcess* m_piDebuggedProcess;

    // The vsdProcessDebugger:
    vsdProcessDebugger* m_pVSProcessDebugger;
};


#endif //__VSDPACKAGECONNECTOR_H

