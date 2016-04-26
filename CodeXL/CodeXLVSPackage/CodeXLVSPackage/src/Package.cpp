//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file Package.cpp
///
//==================================================================================
#include "stdafx.h"

#include <CodeXLVSPackage.h>

// C++:
#include <sstream>
#include <iostream>
#include <vector>
#include <algorithm>

// VS:
#include <VSLErrorHandlers.h>
#include <VSLShortNameDefines.h>
#include <Include/vspStringConstants.h>
#include <dbgmetric.h>

// Local:
#include <src/Package.h>
#include <CodeXLVSPackage/Include/vspCommandIDs.h>
#include <src/vspCoreAPI.h>
#include <CodeXLVSPackage/Include/vspStringConstants.h>
#include <src/vspTimer.h>
#include <src/vspUtils.h>
#include <src/vscVsUtils.h>


// Defines the maximal size of work items, where the combo box is detailed:
#define VSP_MAX_WORKITEM_DETAILED 64
#define VSP_MAX_WORKITEM_IN_NON_DETAILED_COMBO 8
// ---------------------------------------------------------------------------
// Name:        CCodeXLVSPackagePackage::CCodeXLVSPackagePackage
// Description: Constructor
// Author:      Sigal Algranaty
// Date:        25/1/2011
// ---------------------------------------------------------------------------
CodeXLVSPackage::CodeXLVSPackage() :
    m_dwEditorCookie(0),
    m_dwProfileSessionEditorCookie(0),
    m_dwKernelAnalyzerEditorCookie(0),
    _oleCommandTargetCookie1(0),
    _oleCommandTargetCookie2(0),
    _pPackageCommandHandler(nullptr),
    _pCallsHistoryToolWindow(nullptr),
    _pPropertiesToolWindow(nullptr),
    _pObjectsExplorerToolWindow(nullptr),
    _pStatisticsToolWindow(nullptr),
    _pMemoryToolWindow(nullptr),
    _pStateVariablesToolWindow(nullptr),
    m_pCoreImpl(VSCORE(vsc_CreateInstance())),
    m_isSited(false),
    // This is the ex-vspProgressBarWrapper object's member
    _userCookie(0),
    m_pTimer(nullptr)
{
    VSP_ASSERT(m_pCoreImpl != nullptr);

    // Initialize a Qt application instance:
    VSCORE(vsc_InitQtApp)();

    // Commented out until assertions amount is decreased:
    createDialogBasedAssertionFailureHandler();

    // Create the command handler for package commands:
    _pPackageCommandHandler = new vspPackageCommandHandler;

    // Create the tool windows:
    _pCallsHistoryToolWindow = new vspToolWindow(GetVsSiteCache());
    _pPropertiesToolWindow = new vspToolWindow(GetVsSiteCache());
    _pObjectsExplorerToolWindow = new vspToolWindow(GetVsSiteCache());
    _pStatisticsToolWindow = new vspToolWindow(GetVsSiteCache());
    _pMemoryToolWindow = new vspToolWindow(GetVsSiteCache());
    _pStateVariablesToolWindow = new vspToolWindow(GetVsSiteCache());

    // Initialize the multi watch tool windows:
    for (int i = 0 ; i < VSP_AMOUNT_OF_MULTIWATCH_VIEWS; i++)
    {
        _pMultiWatchToolWindows[i] = new vspToolWindow(GetVsSiteCache());
    }
}


// ---------------------------------------------------------------------------
// Name:        CCodeXLVSPackagePackage::~CCodeXLVSPackagePackage
// Description: Destructor
// Author:      Sigal Algranaty
// Date:        29/5/2011
// ---------------------------------------------------------------------------
CodeXLVSPackage::~CodeXLVSPackage()
{
    if (m_pTimer != nullptr)
    {
        delete m_pTimer;
        m_pTimer = nullptr;
    }

    // Destroy the core implementation object.
    VSCORE(vsc_DestroyInstance)(m_pCoreImpl);
    m_pCoreImpl = nullptr;
}

// ---------------------------------------------------------------------------
// Name:        CCodeXLVSPackagePackage::createDialogBasedAssertionFailureHandler
// Description: Creates the dialog based asserting failure handler.
// Author:      Sigal Algranaty
// Date:        25/1/2011
// ---------------------------------------------------------------------------
bool CodeXLVSPackage::createDialogBasedAssertionFailureHandler()
{
    VSCORE(vsc_CreateDialogBasedAssertionFailureHandler)(m_pCoreImpl);
    return true;
}

// ---------------------------------------------------------------------------
// Name:        CCodeXLVSPackagePackage::CCodeXLVSPackagePackage::UpdateRegistry
// Description: DLL is registered with VS via a pkgdef file. Don't do anything if asked to
//              self-register.
// Author:      Sigal Algranaty
// Date:        25/1/2011
// ---------------------------------------------------------------------------
HRESULT WINAPI CodeXLVSPackage::UpdateRegistry(BOOL bRegister)
{
    GT_UNREFERENCED_PARAMETER(bRegister);
    return S_OK;
}

// ---------------------------------------------------------------------------
// Name:        CCodeXLVSPackagePackage::PostSited
// Description: This method will be called after IVsPackage::SetSite is called with a valid site
// Arguments:   IVsPackageEnums::SetSiteResult /*result*/
// Author:      Sigal Algranaty
// Date:        25/1/2011
// ---------------------------------------------------------------------------
void CodeXLVSPackage::PostSited(IVsPackageEnums::SetSiteResult /*result*/)
{
    // Register the DTE connector.
    VSCORE(vscVspDTEInvoker_SetIVspDTEConnector)(&(vspDTEConnector::instance()));

    // Register as the owner of all core objects except for vspWindowsManager,
    // which will be owned near the end of this function (PostSited) to ensure that
    // we are not trying to open any windows before the VS GUI is initialized.
    VSCORE(vscApplicationCommands_SetOwner)(this);
    VSCORE(vscSetVscProgressBarWrapperOwner)(this);
    VSCORE(vscSourceCodeViewerOwner_SetOwner)(this);
    VSCORE(vscSetVscDebugEngineOwner)(this);
    VSCORE(vscSetVscBreakpoinstManagerOwner)(this);
    VSCORE(vscSetVscEventsObserverOwner)(this);
    VSCORE(vscGRApiFunctions_SetOwner)(this);
    VSCORE(vscSetVscWindowsManagerOwner)(this);

    // Set Qt message box icon:
    VSCORE(vsc_SetAppMessageBoxIcon)();

    // Register the editor factory
    CComPtr<IVsRegisterEditors> spIVsRegisterEditors;
    CHKHR(GetVsSiteCache().QueryService(SID_SVsRegisterEditors, &spIVsRegisterEditors));

    if (m_dwEditorCookie == 0)
    {
        // Create the editor factory
        CComObject<EditorFactory>* pFactory = new CComObject<EditorFactory>;

        if (nullptr == pFactory)
        {
            ERRHR(E_OUTOFMEMORY);
        }

        CComPtr<IVsEditorFactory> spIVsEditorFactory = static_cast<IVsEditorFactory*>(pFactory);
        CHKHR(spIVsRegisterEditors->RegisterEditor(CLSID_CodeXLVSPackageEditorFactory, spIVsEditorFactory, &m_dwEditorCookie));
    }

    if (m_dwProfileSessionEditorCookie == 0)
    {
        // Create the editor factory
        CComObject<ProfileSessionEditorFactory>* pFactory = new CComObject<ProfileSessionEditorFactory>;

        if (nullptr == pFactory)
        {
            ERRHR(E_OUTOFMEMORY);
        }

        CComPtr<IVsEditorFactory> spIVsEditorFactory = static_cast<IVsEditorFactory*>(pFactory);
        CHKHR(spIVsRegisterEditors->RegisterEditor(CLSID_CodeXLVSPackageProfileSessionEditorFactory, spIVsEditorFactory, &m_dwProfileSessionEditorCookie));
    }

    if (m_dwKernelAnalyzerEditorCookie == 0)
    {
        // Create the editor factory
        CComObject<KernelAnalyzerEditorFactory>* pFactory = new CComObject<KernelAnalyzerEditorFactory>;

        if (nullptr == pFactory)
        {
            ERRHR(E_OUTOFMEMORY);
        }

        CComPtr<IVsEditorFactory> spIVsEditorFactory = static_cast<IVsEditorFactory*>(pFactory);
        CHKHR(spIVsRegisterEditors->RegisterEditor(CLSID_CodeXLVSPackageKernelAnalyzerEditorFactory, spIVsEditorFactory, &m_dwKernelAnalyzerEditorCookie));
    }

    // Initialize the package:
    VSCORE(vsc_PostSited_InitPackage)(m_pCoreImpl);

    // Initialize the package timer:
    VSP_ASSERT(nullptr == m_pTimer);
    m_pTimer = new vspTimer;
    VSP_ASSERT(m_pTimer != nullptr);

    // Register our custom debug engine using the appropriate registry entries:
    registerDebugEngineInTemporaryHive();

    // Register our language services:
    std::wstring tempRegistryRootString = getVSRegistryRootPath(true);
    registerLanguageServices(tempRegistryRootString);

    // Create the send error report dialog:
    createSendErrorReportDialog();

    // Get the visual studio debugger:
    IVsDebugger* piDebugger = nullptr;
    HRESULT hr = GetVsSiteCache().QueryService(SID_SVsShellDebugger, &piDebugger);
    VSP_ASSERT(SUCCEEDED(hr));

    // Pass it to relevant classes:
    VSCORE(vsc_PostSited_InitBreakpointsManager)(piDebugger);

    // Release the debugger interface:
    piDebugger->Release();

    // Get the Design Time Extensions (DTE) interface:
    VxDTE::_DTE* piDTE = nullptr;
    hr = GetVsSiteCache().QueryService(SID_SDTE, &piDTE);
    VSP_ASSERT(SUCCEEDED(hr));

    // Pass it to the DTE connector:
    vspDTEConnector::instance().setDTEInterface(piDTE);

    // Release the DTE interface:
    piDTE->Release();

    // Get the UI Shell interface:
    IVsUIShell* piUIShell = nullptr;
    hr = GetVsSiteCache().QueryService(SID_SVsUIShell, &piUIShell);
    VSP_ASSERT(SUCCEEDED(hr));

    // Pass it to the windows manager:
    VSCORE(vscWindowsManager_SetIVsUiShell)(piUIShell);

    // Release the UI Shell interface:
    piUIShell->Release();

    // Set me in my wrapper:
    vspPackageWrapper::instance().setPackage(this);

    // Initialize package debugger:
    VSCORE(vsc_InitPackageDebugger)(getVSRegistryRootPath(false), tempRegistryRootString.c_str());

    // Initialize event observer:
    VSCORE(vsc_InitAppEventObserver)(m_pCoreImpl);

    // Register as ole command target:
    IVsRegisterPriorityCommandTarget* piOleTarget = nullptr;
    hr = GetVsSiteCache().QueryService(SID_SVsRegisterPriorityCommandTarget, &piOleTarget);
    VSP_ASSERT(SUCCEEDED(hr));

    hr = piOleTarget->RegisterPriorityCommandTarget(0, this, &_oleCommandTargetCookie1);
    VSP_ASSERT(SUCCEEDED(hr));

    VSP_ASSERT(_pPackageCommandHandler != nullptr);

    if (_pPackageCommandHandler != nullptr)
    {
        piOleTarget->RegisterPriorityCommandTarget(0, _pPackageCommandHandler, &_oleCommandTargetCookie2);
    }

    // Initialize hex mode:
    vspDTEConnector::instance().syncHexDisplayModeWithVS();

    // Check if gDEBugger is installed:
    checkgDEBBugerInstallation();

    // Initialize first mode as debugger by default:
    VSCORE(vsc_PostSited_InitAsDebugger)();

    // Initialize the CodeXL extension GUI components.
    VSCORE(vsc_InitGuiComponents)();

    // Raise the "sited" flag:
    m_isSited = true;

    m_installedComponentsBitmask = VSCORE(vsc_GetInstalledComponentsBitmask)();
}


// ---------------------------------------------------------------------------
// Name:        CCodeXLVSPackagePackage::PreClosing
// Description: Is called before the package is closed
// Author:      Sigal Algranaty
// Date:        25/1/2011
// ---------------------------------------------------------------------------
void CodeXLVSPackage::PreClosing()
{
    // Lower the "sited" flag:
    m_isSited = false;

    // Stop the periodic timer that looks for project changes
    if (m_pTimer != nullptr)
    {
        delete m_pTimer;
        m_pTimer = nullptr;
    }

    if (_pCallsHistoryToolWindow != nullptr)
    {
        delete _pCallsHistoryToolWindow;
        _pCallsHistoryToolWindow = nullptr;
    }

    if (_pPropertiesToolWindow != nullptr)
    {
        delete _pPropertiesToolWindow;
        _pPropertiesToolWindow = nullptr;
    }

    if (_pObjectsExplorerToolWindow != nullptr)
    {
        delete _pObjectsExplorerToolWindow;
        _pObjectsExplorerToolWindow = nullptr;
    }

    if (_pStatisticsToolWindow != nullptr)
    {
        delete _pStatisticsToolWindow;
        _pStatisticsToolWindow = nullptr;
    }

    if (_pMemoryToolWindow != nullptr)
    {
        delete _pMemoryToolWindow;
        _pMemoryToolWindow = nullptr;
    }

    if (_pStateVariablesToolWindow != nullptr)
    {
        delete _pStateVariablesToolWindow;
        _pStateVariablesToolWindow = nullptr;
    }

    // Clear the pointer to the UI shell:
    VSCORE(vscWindowsManager_SetIVsUiShell)(nullptr);

    // Clear the package pointer from the package wrapper:
    vspPackageWrapper::instance().clearPackage();

    // Un-register our custom debug engine by removing the registry entries related to it:
    unregisterDebugEngineFromTemporaryHive();

    // Unregister the editor factory
    CComPtr<IVsRegisterEditors> spIVsRegisterEditors;
    CHKHR(GetVsSiteCache().QueryService(SID_SVsRegisterEditors, &spIVsRegisterEditors));

    if (m_dwEditorCookie != 0)
    {
        CHKHR(spIVsRegisterEditors->UnregisterEditor(m_dwEditorCookie));
    }

    if (m_dwProfileSessionEditorCookie != 0)
    {
        CHKHR(spIVsRegisterEditors->UnregisterEditor(m_dwProfileSessionEditorCookie));
    }

    if (m_dwKernelAnalyzerEditorCookie != 0)
    {
        CHKHR(spIVsRegisterEditors->UnregisterEditor(m_dwKernelAnalyzerEditorCookie));
    }


    // Unregister from ole command target:
    IVsRegisterPriorityCommandTarget* piOleTarget = nullptr;
    HRESULT hr = GetVsSiteCache().QueryService(SID_SVsRegisterPriorityCommandTarget, &piOleTarget);
    VSP_ASSERT(SUCCEEDED(hr));

    hr = piOleTarget->UnregisterPriorityCommandTarget(_oleCommandTargetCookie1);
    VSP_ASSERT(SUCCEEDED(hr));

    hr = piOleTarget->UnregisterPriorityCommandTarget(_oleCommandTargetCookie2);
    VSP_ASSERT(SUCCEEDED(hr));

    // Release the DTE from the DTE connector:
    vspDTEConnector::instance().releaseDTEInterface();

    VSCORE(vsc_PreClosing)(m_pCoreImpl);
}

// ---------------------------------------------------------------------------
// Name:        CCodeXLVSPackagePackage::TriggerClockTick
// Description: Trigger an execution of the OnClockTick() handler
// Author:      Doron Ofek
// Date:        Oct-6, 2015
// ---------------------------------------------------------------------------
void CodeXLVSPackage::TriggerClockTick()
{
    if (m_pTimer != nullptr)
    {
        m_pTimer->onClockTick();
    }
}

// ---------------------------------------------------------------------------
// Name:        CCodeXLVSPackagePackage::onCallsHistoryView
// Description: Command handler for CodeXL->Calls History command
// Author:      Sigal Algranaty
// Date:        25/1/2011
// ---------------------------------------------------------------------------
void CodeXLVSPackage::onCallsHistoryView(CommandHandler* pSender, DWORD flags, VARIANT* pIn, VARIANT* pOut)
{
    GT_UNREFERENCED_PARAMETER(pSender);
    GT_UNREFERENCED_PARAMETER(flags);
    GT_UNREFERENCED_PARAMETER(pIn);
    GT_UNREFERENCED_PARAMETER(pOut);

    verifyBaseViewsCreated();
    VSP_ASSERT(_pCallsHistoryToolWindow != nullptr);

    if (_pCallsHistoryToolWindow != nullptr)
    {
        int viewId = VSCORE(vsc_GetCallsHistoryListId());
        _pCallsHistoryToolWindow->setMyWindowCommandID(viewId);
        _pCallsHistoryToolWindow->CreateAndShow();
    }
}


// ---------------------------------------------------------------------------
// Name:        CCodeXLVSPackagePackage::onCodeXLExplorerView
// Description: Command handler for CodeXL->Explorer command

// Author:      Sigal Algranaty
// Date:        25/1/2011
// ---------------------------------------------------------------------------
void CodeXLVSPackage::onCodeXLExplorerView(CommandHandler* pSender, DWORD flags, VARIANT* pIn, VARIANT* pOut)
{
    GT_UNREFERENCED_PARAMETER(pSender);
    GT_UNREFERENCED_PARAMETER(flags);
    GT_UNREFERENCED_PARAMETER(pIn);
    GT_UNREFERENCED_PARAMETER(pOut);

    verifyBaseViewsCreated();
    VSP_ASSERT(_pObjectsExplorerToolWindow != nullptr);

    if (_pObjectsExplorerToolWindow != nullptr)
    {
        int viewId = VSCORE(vsc_GetObjectNavigationTreeId());
        _pObjectsExplorerToolWindow->setMyWindowCommandID(viewId);

        // Invoke the core logic.
        VSCORE(vsc_OnCodeExplorerView)();
    }
}


// ---------------------------------------------------------------------------
// Name:        CCodeXLVSPackagePackage::onPropertiesView
// Description: Command handler for CodeXL->Properties command
// Author:      Sigal Algranaty
// Date:        25/1/2011
// ---------------------------------------------------------------------------
void CodeXLVSPackage::onPropertiesView(CommandHandler* pSender, DWORD flags, VARIANT* pIn, VARIANT* pOut)
{
    GT_UNREFERENCED_PARAMETER(pSender);
    GT_UNREFERENCED_PARAMETER(flags);
    GT_UNREFERENCED_PARAMETER(pIn);
    GT_UNREFERENCED_PARAMETER(pOut);

    verifyBaseViewsCreated();

    VSP_ASSERT(_pPropertiesToolWindow != nullptr);

    if (_pPropertiesToolWindow != nullptr)
    {
        int viewId = VSCORE(vsc_GetPropertiesViewId());
        _pPropertiesToolWindow->setMyWindowCommandID(viewId);
        _pPropertiesToolWindow->CreateAndShow();
    }

}

// ---------------------------------------------------------------------------
// Name:        CCodeXLVSPackagePackage::onMemoryView
// Description: Command handler for CodeXL->Memory command
// Author:      Sigal Algranaty
// Date:        25/1/2011
// ---------------------------------------------------------------------------
void CodeXLVSPackage::onMemoryView(CommandHandler* pSender, DWORD flags, VARIANT* pIn, VARIANT* pOut)
{
    GT_UNREFERENCED_PARAMETER(pSender);
    GT_UNREFERENCED_PARAMETER(flags);
    GT_UNREFERENCED_PARAMETER(pIn);
    GT_UNREFERENCED_PARAMETER(pOut);

    verifyBaseViewsCreated();

    VSP_ASSERT(_pMemoryToolWindow != nullptr);

    if (_pMemoryToolWindow != nullptr)
    {
        int viewId = VSCORE(vsc_GetMemoryAnalysisViewerId());
        _pMemoryToolWindow->setMyWindowCommandID(viewId);
        _pMemoryToolWindow->CreateAndShow();
    }
}

// ---------------------------------------------------------------------------
// Name:        CCodeXLVSPackagePackage::onStatisticsView
// Description: Command handler for CodeXL->Statistics command
// Author:       Sigal Algranaty
// Date:         27/1/2011
// ---------------------------------------------------------------------------
void CodeXLVSPackage::onStatisticsView(CommandHandler* pSender, DWORD flags, VARIANT* pIn, VARIANT* pOut)
{
    GT_UNREFERENCED_PARAMETER(pSender);
    GT_UNREFERENCED_PARAMETER(flags);
    GT_UNREFERENCED_PARAMETER(pIn);
    GT_UNREFERENCED_PARAMETER(pOut);

    verifyBaseViewsCreated();
    VSP_ASSERT(_pStatisticsToolWindow != nullptr);

    if (_pStatisticsToolWindow != nullptr)
    {
        int viewId = VSCORE(vsc_GetStatisticsViewId());
        _pStatisticsToolWindow->setMyWindowCommandID(viewId);
        _pStatisticsToolWindow->CreateAndShow();
    }
}


// ---------------------------------------------------------------------------
// Name:        CCodeXLVSPackagePackage::onStateVariablesView
// Description: Command handler for CodeXL->StateVariables command
// Author:       Yuri Rshtunique
// Date:         28/8/2014
// ---------------------------------------------------------------------------
void CodeXLVSPackage::onStateVariablesView(CommandHandler* pSender, DWORD flags, VARIANT* pIn, VARIANT* pOut)
{
    GT_UNREFERENCED_PARAMETER(pSender);
    GT_UNREFERENCED_PARAMETER(flags);
    GT_UNREFERENCED_PARAMETER(pIn);
    GT_UNREFERENCED_PARAMETER(pOut);

    verifyBaseViewsCreated();
    VSP_ASSERT(_pStateVariablesToolWindow != nullptr);

    if (_pStateVariablesToolWindow != nullptr)
    {
        int viewId = VSCORE(vsc_GetStateVariablesViewId());
        _pStateVariablesToolWindow->setMyWindowCommandID(viewId);
        _pStateVariablesToolWindow->CreateAndShow();
    }
}

// ---------------------------------------------------------------------------
// Name:        CCodeXLVSPackagePackage::onAddKernelMultiWatchFromSourceCode
// Description: Command handler for CodeXL->Add MultiWatch - for the command
//              instance within the source code context menu
// Author:      Sigal Algranaty
// Date:        27/2/2011
// ---------------------------------------------------------------------------
void CodeXLVSPackage::onAddKernelMultiWatchFromSourceCode(CommandHandler* pSender, DWORD flags, VARIANT* pIn, VARIANT* pOut)
{
    GT_UNREFERENCED_PARAMETER(pSender);
    GT_UNREFERENCED_PARAMETER(flags);
    GT_UNREFERENCED_PARAMETER(pIn);
    GT_UNREFERENCED_PARAMETER(pOut);

    // Get the current text editor selected text:
    std::wstring selectedText;
    bool rcGetText = vspDTEConnector::instance().getSelectedEditorText(selectedText);
    VSP_ASSERT(rcGetText);

    if (rcGetText)
    {
        int freeWindowIndex = VSCORE(vsc_OnAddKernelMultiWatchFromSourceCode());

        // Remove all the spaces from the beginning and end of the selected text:
        vspVsUtilsRemoveAllSpaces(selectedText);

        // Open the first available MultiWatch view:
        openMultiWatchView(freeWindowIndex, selectedText);
    }
}


// ---------------------------------------------------------------------------
// Name:        CCodeXLVSPackagePackage::onAddKernelMultiWatchFromLocalsView
// Description: Command handler for CodeXL->Add MultiWatch - for the command
//              instance within the local view context menu
// Author:      Sigal Algranaty
// Date:        12/4/2011
// ---------------------------------------------------------------------------
void CodeXLVSPackage::onAddKernelMultiWatchFromLocalsView(CommandHandler* pSender, DWORD flags, VARIANT* pIn, VARIANT* pOut)
{
    GT_UNREFERENCED_PARAMETER(pSender);
    GT_UNREFERENCED_PARAMETER(flags);
    GT_UNREFERENCED_PARAMETER(pIn);
    GT_UNREFERENCED_PARAMETER(pOut);

    // Get the current text editor selected text:
    std::wstring selectedText;
    bool rcGetText = vspDTEConnector::instance().getSelectedWindowText(selectedText);
    VSP_ASSERT(rcGetText);

    if (rcGetText)
    {
        // Get the index for the first hidden multi watch view:
        int freeWindowIndex = VSCORE(vsc_OnAddKernelMultiWatchFromLocalsView());

        // Remove all the spaces from the beginning and end of the selected text:
        vspVsUtilsRemoveAllSpaces(selectedText);

        // Open the first available MultiWatch view:
        openMultiWatchView(freeWindowIndex, selectedText);
    }
}

// ---------------------------------------------------------------------------
// Name:        CCodeXLVSPackagePackage::onAddKernelMultiWatchFromLocalsView
// Description: Command handler for CodeXL->Add MultiWatch - for the command
//              instance within the local view context menu
// Author:      Sigal Algranaty
// Date:        12/4/2011
// ---------------------------------------------------------------------------
void CodeXLVSPackage::onAddKernelMultiWatchFromWatchView(CommandHandler* pSender, DWORD flags, VARIANT* pIn, VARIANT* pOut)
{
    GT_UNREFERENCED_PARAMETER(pSender);
    GT_UNREFERENCED_PARAMETER(flags);
    GT_UNREFERENCED_PARAMETER(pIn);
    GT_UNREFERENCED_PARAMETER(pOut);

    // Get the current text editor selected text:
    std::wstring selectedText;
    bool rcGetText = vspDTEConnector::instance().getSelectedWindowText(selectedText);
    VSP_ASSERT(rcGetText);

    if (rcGetText)
    {
        // Get the index for the first hidden multi watch view:
        int freeWindowIndex = VSCORE(vsc_OnAddKernelMultiWatchFromWatchView());

        // Remove all the spaces from the beginning and end of the selected text:
        vspVsUtilsRemoveAllSpaces(selectedText);

        // Open the first available MultiWatch view:
        openMultiWatchView(freeWindowIndex, selectedText);
    }
}

// ---------------------------------------------------------------------------
// Name:        CCodeXLVSPackagePackage::onAddKernelMultiWatch1
// Description: Command handler for CodeXL->Add MultiWatch
// Author:      Sigal Algranaty
// Date:        27/2/2011
// ---------------------------------------------------------------------------
void CodeXLVSPackage::onAddKernelMultiWatch1(CommandHandler* pSender, DWORD flags, VARIANT* pIn, VARIANT* pOut)
{
    GT_UNREFERENCED_PARAMETER(pSender);
    GT_UNREFERENCED_PARAMETER(flags);
    GT_UNREFERENCED_PARAMETER(pIn);
    GT_UNREFERENCED_PARAMETER(pOut);

    openMultiWatchView(0);
}

// ---------------------------------------------------------------------------
// Name:        CCodeXLVSPackagePackage::onAddKernelMultiWatch2
// Description: Command handler for CodeXL->Add MultiWatch
// Author:      Sigal Algranaty
// Date:        27/2/2011
// ---------------------------------------------------------------------------
void CodeXLVSPackage::onAddKernelMultiWatch2(CommandHandler* pSender, DWORD flags, VARIANT* pIn, VARIANT* pOut)
{
    GT_UNREFERENCED_PARAMETER(pSender);
    GT_UNREFERENCED_PARAMETER(flags);
    GT_UNREFERENCED_PARAMETER(pIn);
    GT_UNREFERENCED_PARAMETER(pOut);

    openMultiWatchView(1);
}

// ---------------------------------------------------------------------------
// Name:        CCodeXLVSPackagePackage::onAddKernelMultiWatch3
// Description: Command handler for CodeXL->Add MultiWatch
// Author:      Sigal Algranaty
// Date:        27/2/2011
// ---------------------------------------------------------------------------
void CodeXLVSPackage::onAddKernelMultiWatch3(CommandHandler* pSender, DWORD flags, VARIANT* pIn, VARIANT* pOut)
{
    GT_UNREFERENCED_PARAMETER(pSender);
    GT_UNREFERENCED_PARAMETER(flags);
    GT_UNREFERENCED_PARAMETER(pIn);
    GT_UNREFERENCED_PARAMETER(pOut);

    openMultiWatchView(2);
}

// ---------------------------------------------------------------------------
// Name:        CCodeXLVSPackagePackage::onUpdateMultiWatchViews
// Description: Called to update the multiwatch command's visibility
// Author:      Uri Shomroni
// Date:        9/3/2011
// ---------------------------------------------------------------------------
void CodeXLVSPackage::onUpdateMultiWatchViews(CommandHandler& handler, OLECMD* pOleCmd, OLECMDTEXT* pOleText)
{
    GT_UNREFERENCED_PARAMETER(&handler);
    GT_UNREFERENCED_PARAMETER(pOleText);

    if (pOleCmd != nullptr)
    {
        // Show the multi-watch view only when the debug UI context is active in VS:
        bool shouldShow = vspPackageWrapper::instance().isVSUIContextActive(UICONTEXT_Debugging);

        // Initialize the command flags:
        bool shouldEnable = true;
        pOleCmd->cmdf = OLECMDF_SUPPORTED;

        // Add invisible flag is the command should not be shown:
        if (!shouldShow)
        {
            pOleCmd->cmdf |= OLECMDF_INVISIBLE;
        }
        else
        {
            // If the command is visible, make it enabled:
            if (shouldEnable)
            {
                pOleCmd->cmdf |= OLECMDF_ENABLED;
            }
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        CCodeXLVSPackagePackage::onUpdateMode
// Description: Called to update a mode's menu item status
// Author:      Uri Shomroni
// Date:        14/5/2012
// ---------------------------------------------------------------------------
void CodeXLVSPackage::onUpdateMode(CommandHandler& handler, OLECMD* pOleCmd, OLECMDTEXT* pOleText)
{
    DWORD commandId = handler.GetId().GetId();
    bool isChecked = false;

    bool isEnabled = VSCORE(vsc_OnUpdateMode_IsEnabled());

    switch (commandId)
    {
        case cmdidDebugMode:
        {
            isChecked = VSCORE(vsc_OnUpdateMode_DebugMode_IsChecked());

            // Update the command text:
            if (pOleText != nullptr)
            {
                int bufferSize = (int)pOleText->cwBuf;
                int commandStrLength = 0;
                wchar_t* pCmdNameBuf = nullptr;
                VSCORE(vsc_OnUpdateMode_DebugMode_Text(bufferSize, pCmdNameBuf, commandStrLength));
                VSP_ASSERT(pCmdNameBuf != nullptr);

                if (pCmdNameBuf != nullptr)
                {
                    // Copy the characters to the buffer:
                    lstrcpy(pOleText->rgwz, pCmdNameBuf);
                    pOleText->cwActual = (ULONG)(commandStrLength + 1);
                    VSCORE(vscDeleteWcharString(pCmdNameBuf));
                }
            }

            break;
        }

        case cmdidCodeXLProfileDropdownMenu:
        {
            isChecked = VSCORE(vsc_OnUpdateMode_CXLProfileDropDownMenu_IsChecked());
            break;
        }

        case cmdidProfileMode:
        {
            isChecked = VSCORE(vsc_OnUpdateMode_CXLProfileDropDownMenu_IsChecked());

            // Set the command text:
            if (pOleText != nullptr)
            {
                int bufferSize = (int)pOleText->cwBuf;
                int commandStrLength = 0;
                wchar_t* pCmdNameBuf = nullptr;
                VSCORE(vsc_OnUpdateMode_CXLProfileDropDownMenu_CoreLogic(bufferSize, pCmdNameBuf, commandStrLength));
                VSP_ASSERT(pCmdNameBuf != nullptr);

                if (pCmdNameBuf != nullptr)
                {
                    // Copy the characters to the buffer:
                    lstrcpy(pOleText->rgwz, pCmdNameBuf);
                    pOleText->cwActual = (ULONG)(commandStrLength + 1);
                    VSCORE(vscDeleteWcharString(pCmdNameBuf));
                }
            }

            break;
        }

        case cmdidFrameAnalysis:
        {
            isChecked = VSCORE(vsc_OnUpdateMode_FrameAnalysisMode_IsChecked());

            // Update the command text:
            if (pOleText != nullptr)
            {
                int bufferSize = (int)pOleText->cwBuf;
                int commandStrLength = 0;
                wchar_t* pCmdNameBuf = nullptr;
                VSCORE(vsc_OnUpdateMode_FrameAnalysis_Text(bufferSize, pCmdNameBuf, commandStrLength));
                VSP_ASSERT(pCmdNameBuf != nullptr);

                if (pCmdNameBuf != nullptr)
                {
                    // Copy the characters to the buffer:
                    lstrcpy(pOleText->rgwz, pCmdNameBuf);
                    pOleText->cwActual = (ULONG)(commandStrLength + 1);
                    VSCORE(vscDeleteWcharString(pCmdNameBuf));
                }
            }

            break;
        }
        break;

        case cmdidAnalyzeMode:
        {
            // Check if analyze mode should be checked:
            isChecked = VSCORE(vsc_OnUpdateMode_AnalyzeMode_IsChecked());

            // AF_AMD_GPU_COMPONENT & AF_AMD_CATALYST_COMPONENT from afAidFunctions:
            isEnabled = isEnabled && (m_installedComponentsBitmask & 0x4) && (m_installedComponentsBitmask & 0x1);

            // Set the command text:
            if (pOleText != nullptr)
            {
                int bufferSize = (int)pOleText->cwBuf;
                int commandStrLength = 0;
                wchar_t* pCmdNameBuf = nullptr;
                VSCORE(vsc_OnUpdateMode_AnalyzeMode_Text(bufferSize, pCmdNameBuf, commandStrLength));
                VSP_ASSERT(pCmdNameBuf != nullptr);

                if (pCmdNameBuf != nullptr)
                {
                    // Copy the characters to the buffer:
                    lstrcpy(pOleText->rgwz, pCmdNameBuf);
                    pOleText->cwActual = (ULONG)(commandStrLength + 1);
                    VSCORE(vscDeleteWcharString(pCmdNameBuf));
                }
            }

            break;
        }

        default:
        {
            // Unexpected value!
            VSP_ASSERT(false);
        }
        break;
    }

    DWORD commandFlags = OLECMDF_SUPPORTED;

    if (isChecked)
    {
        commandFlags |= OLECMDF_LATCHED;
    }

    if (isEnabled)
    {
        commandFlags |= OLECMDF_ENABLED;
    }

    pOleCmd->cmdf = commandFlags;
}

// ---------------------------------------------------------------------------
// Name:        CCodeXLVSPackagePackage::onMode
// Description: Called when a mode is selected from the menu
// Author:      Uri Shomroni
// Date:        14/5/2012
// ---------------------------------------------------------------------------
void CodeXLVSPackage::onModeClicked(CommandHandler* pSender, DWORD flags, VARIANT* pIn, VARIANT* pOut)
{
    GT_UNREFERENCED_PARAMETER(flags);
    GT_UNREFERENCED_PARAMETER(pIn);
    GT_UNREFERENCED_PARAMETER(pOut);

    if (nullptr != pSender)
    {
        DWORD commandId = pSender->GetId().GetId();

        // Invoke core logic.
        VSCORE(vsc_OnModeClicked(commandId));
    }
}

// ---------------------------------------------------------------------------
// Name:        CCodeXLVSPackagePackage::openMultiWatchView
// Description: Opens a multi watch view with the requested index
// Arguments:   int multiWatchWindowIndex
//              variableName - the requested variable name
// Return Val:  void
// Author:      Sigal Algranaty
// Date:        2/3/2011
// ---------------------------------------------------------------------------
void CodeXLVSPackage::openMultiWatchView(int multiWatchWindowIndex, const std::wstring& variableName /*= AF_STR_Empty*/)
{
    verifyBaseViewsCreated();

    // Find the available multi watch window:
    if (multiWatchWindowIndex >= VSP_AMOUNT_OF_MULTIWATCH_VIEWS)
    {
        // We've already used all the MultiWatch windows, and we should reuse:
        multiWatchWindowIndex = 0;
    }

    VSP_ASSERT(multiWatchWindowIndex < VSP_AMOUNT_OF_MULTIWATCH_VIEWS);

    if (multiWatchWindowIndex < VSP_AMOUNT_OF_MULTIWATCH_VIEWS)
    {
        // Get the available multi watch window:
        vspToolWindow* pMultiWatchWindow = _pMultiWatchToolWindows[multiWatchWindowIndex];
        VSP_ASSERT(pMultiWatchWindow != nullptr);

        if (pMultiWatchWindow != nullptr)
        {
            int viewID = VSCORE(vsc_GetFirstMultiWatchViewId()) + multiWatchWindowIndex;
            pMultiWatchWindow->setMyWindowCommandID(viewID);
            pMultiWatchWindow->CreateAndShow();

            // If there was a selected text for the variable name:
            if (!variableName.empty())
            {
                VSCORE(vsc_OpenMultiWatchViewId(viewID, variableName.c_str()));
            }
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        CCodeXLVSPackagePackage::onOpenCLBreakpoints
// Description: Command handler for CodeXL->New Breakpoint->Breakpoint command
// Author:      Sigal Algranaty
// Date:        25/1/2011
// ---------------------------------------------------------------------------
void CodeXLVSPackage::onOpenCLBreakpoints(CommandHandler* pSender, DWORD flags, VARIANT* pIn, VARIANT* pOut)
{
    GT_UNREFERENCED_PARAMETER(pSender);
    GT_UNREFERENCED_PARAMETER(flags);
    GT_UNREFERENCED_PARAMETER(pIn);
    GT_UNREFERENCED_PARAMETER(pOut);

    // make sure that there is a solution before opening the breakpoint:
    bool solutionExists = vspPackageWrapper::instance().isVSUIContextActive(UICONTEXT_SolutionExists);

    if (solutionExists)
    {
        // Invoke the core logic.
        VSCORE(vsc_OpenBreakpointsDialog());
    }
}

// ---------------------------------------------------------------------------
// Name:        CCodeXLVSPackagePackage::onUpdateOpenCLBreakpoints
// Description: Called to update the breakpoints command's visibility
// Author:      Uri Shomroni
// Date:        9/3/2011
// ---------------------------------------------------------------------------
void CodeXLVSPackage::onUpdateOpenCLBreakpoints(CommandHandler& handler, OLECMD* pOleCmd, OLECMDTEXT* pOleText)
{
    GT_UNREFERENCED_PARAMETER(&handler);
    GT_UNREFERENCED_PARAMETER(pOleText);

    if (pOleCmd != nullptr)
    {
        // Check if a solution exists through VS interfaces:
        bool shouldShow = vspPackageWrapper::instance().isVSUIContextActive(UICONTEXT_SolutionExists) && VSCORE(vsc_OnUpdateOpenCLBreakpoints());
        bool shouldEnable = shouldShow;

        pOleCmd->cmdf = OLECMDF_SUPPORTED;

        if (shouldEnable)
        {
            pOleCmd->cmdf |= OLECMDF_ENABLED;
        }

        if (!shouldShow)
        {
            pOleCmd->cmdf |= OLECMDF_INVISIBLE;
        }
    }
}
// ---------------------------------------------------------------------------
// Name:        CCodeXLVSPackagePackage::LoadUILibrary::ExtendedErrorInfo& GetLoadUILibraryErrorInfo
// Description: This function provides the error information if it is not possible to load
//              the UI dll. It is for this reason that the resource IDS_E_BADINSTALL must
//              be defined inside this dll's resources.
// Return Val:  static const
// Author:      Sigal Algranaty
// Date:        25/1/2011
// ---------------------------------------------------------------------------
const CodeXLVSPackage::LoadUILibrary::ExtendedErrorInfo& CodeXLVSPackage::GetLoadUILibraryErrorInfo()
{
    static LoadUILibrary::ExtendedErrorInfo errorInfo(IDS_E_BADINSTALL);
    return errorInfo;
}

// ---------------------------------------------------------------------------
// Name:        CCodeXLVSPackagePackage::onLaunchOpenCLDebugging
// Description: Command handler for "Launch OpenCL debugging" command
// Arguments:    CommandHandler* pSender
//              DWORD flags
//              VARIANT* pIn
//              VARIANT* pOut
// Author:      Uri Shomroni
// Date:        25/1/2011
// ---------------------------------------------------------------------------
void CodeXLVSPackage::OnStartButton(CommandHandler* pSender, DWORD flags, VARIANT* pIn, VARIANT* pOut)
{
    GT_UNREFERENCED_PARAMETER(pSender);
    GT_UNREFERENCED_PARAMETER(flags);
    GT_UNREFERENCED_PARAMETER(pIn);
    GT_UNREFERENCED_PARAMETER(pOut);

    // Check the VS project settings, and update the infrastructure:
    bool shouldDebug = false;
    bool shouldProfile = false;
    bool shouldFrameAnalyze = false;
    bool isProjectTypeValid = true;
    updateProjectSettingsFromVS(shouldDebug, shouldProfile, shouldFrameAnalyze, isProjectTypeValid);

    vspDTEConnector::instance().SaveAllCommand();

    if (shouldDebug)
    {
        // Execute a debug session:
        executeDebugSession(isProjectTypeValid);
    }
    else if (shouldProfile)
    {
        executeProfileSession();
    }
    else if (shouldFrameAnalyze)
    {
        executeFrameAnalysis();
    }
    else
    {
        buildOpenCLFile();
    }

    // Update the tree root text:
    VSCORE(vsc_OnLaunchOpenCLDebugging());
}


// ---------------------------------------------------------------------------
// Name:        CCodeXLVSPackagePackage::onUpdateLaunchOpenCLDebugging
// Description: Command handler for CodeXL launch debug session command update
// Author:      Uri Shomroni
// Date:        25/1/2011
// ---------------------------------------------------------------------------
void CodeXLVSPackage::onUpdateLaunchOpenCLDebugging(CommandHandler& handler, OLECMD* pOleCmd, OLECMDTEXT* pOleText)
{
    GT_UNREFERENCED_PARAMETER(&handler);

    bool isActionEnabled = true, isActionChecked = false, isActionVisible = true;

    wchar_t* pCurrentMode = nullptr;
    VSCORE(vsc_GetCurrentModeStr(pCurrentMode));
    VSP_ASSERT(pCurrentMode != nullptr);
    std::wstring currentMode((pCurrentMode != nullptr) ? pCurrentMode : L"");
    VSCORE(vscDeleteWcharString(pCurrentMode));

    if (currentMode.empty())
    {
        const wchar_t* pModeStr = VSCORE(vsc_Get_GD_STR_executionMode());
        currentMode = pModeStr;
    }

    if (currentMode == VSCORE(vsc_GetProfileModeStr()))
    {
        onUpdateLaunchProfileAction(isActionEnabled, isActionChecked);
    }
    else if (currentMode == VSCORE(vsc_GetExecutionModeStr()))
    {
        onUpdateLaunchDebugAction(isActionEnabled, isActionChecked);
    }
    else if (currentMode == VSCORE(vsc_GetFrameAnalysisModeStr()))
    {
        onUpdateLaunchFrameAnalysisAction(isActionEnabled, isActionChecked);
    }
    else
    {
        isActionEnabled = false;
        isActionVisible = false;
    }

    if (pOleCmd != nullptr)
    {
        DWORD commandFlags = OLECMDF_SUPPORTED;

        if (isActionEnabled)
        {
            commandFlags |= OLECMDF_ENABLED;
        }

        if (isActionChecked)
        {
            commandFlags |= OLECMDF_LATCHED;
        }

        if (!isActionVisible)
        {
            commandFlags |= OLECMDF_INVISIBLE;
        }

        pOleCmd->cmdf = commandFlags;
    }

    if (pOleText != nullptr)
    {
        std::wstring commandName;
        std::wstring verbName;
        vspGetStartActionCommandName(verbName, commandName, true);

        // Truncate the string to the buffer's size:
        int bufferSize = static_cast<int>(pOleText->cwBuf);

        if (static_cast<unsigned int>(bufferSize) < commandName.length())
        {
            commandName = commandName.substr(0, bufferSize);
        }

        // Copy the characters to the buffer:
        lstrcpy(pOleText->rgwz, commandName.c_str());
        pOleText->cwActual = (ULONG)(commandName.length() + 1);
    }
}

// ---------------------------------------------------------------------------
// Name:        CCodeXLVSPackagePackage::onStepInto
// Description: Handle CodeXL step into command
// Arguments:   CommandHandler* pSender
//              DWORD flags
//              VARIANT* pIn
//              VARIANT* pOut
// Return Val:  void
// Author:      Sigal Algranaty
// Date:        8/5/2011
// ---------------------------------------------------------------------------
void CodeXLVSPackage::onStepInto(CommandHandler* pSender, DWORD flags, VARIANT* pIn, VARIANT* pOut)
{
    GT_UNREFERENCED_PARAMETER(pSender);
    GT_UNREFERENCED_PARAMETER(flags);
    GT_UNREFERENCED_PARAMETER(pIn);
    GT_UNREFERENCED_PARAMETER(pOut);

    bool rc;

    // Call the DTE connector step into function:
    rc = vspDTEConnector::instance().stepInto();
    VSP_ASSERT(rc);
}

// ---------------------------------------------------------------------------
// Name:        CCodeXLVSPackagePackage::onUpdateStepInto
// Description: Command handler for CodeXL step into command update
// Arguments:   CommandHandler& handler
//              OLECMD* pOleCmd
//              OLECMDTEXT* pOleText
// Author:      Sigal Algranaty
// Date:        8/5/2011
// ---------------------------------------------------------------------------
void CodeXLVSPackage::onUpdateStepInto(CommandHandler& handler, OLECMD* pOleCmd, OLECMDTEXT* pOleText)
{
    GT_UNREFERENCED_PARAMETER(&handler);
    GT_UNREFERENCED_PARAMETER(pOleText);

    bool isStepIntoEnabled = false;
    bool shouldShow = false;

    // Invoke core logic.
    VSCORE(vsc_OnUpdateStepInfo(isStepIntoEnabled, shouldShow));

    if (pOleCmd != nullptr)
    {
        DWORD commandFlags = OLECMDF_SUPPORTED;

        if (isStepIntoEnabled)
        {
            commandFlags |= OLECMDF_ENABLED;
        }

        if (!shouldShow)
        {
            commandFlags |= OLECMDF_INVISIBLE;
        }

        pOleCmd->cmdf = commandFlags;
    }
}

// ---------------------------------------------------------------------------
// Name:        CCodeXLVSPackagePackage::onStepOver
// Description: Handle CodeXL step over command
// Arguments:   CommandHandler* pSender
//              DWORD flags
//              VARIANT* pIn
//              VARIANT* pOut
// Return Val:  void
// Author:      Sigal Algranaty
// Date:        8/5/2011
// ---------------------------------------------------------------------------
void CodeXLVSPackage::onStepOver(CommandHandler* pSender, DWORD flags, VARIANT* pIn, VARIANT* pOut)
{
    GT_UNREFERENCED_PARAMETER(pSender);
    GT_UNREFERENCED_PARAMETER(flags);
    GT_UNREFERENCED_PARAMETER(pIn);
    GT_UNREFERENCED_PARAMETER(pOut);

    bool rc;

    // Call the DTE connector step over function:
    rc = vspDTEConnector::instance().stepOver();
    VSP_ASSERT(rc);
}


void CodeXLVSPackage::onStepOut(CommandHandler* pSender, DWORD flags, VARIANT* pIn, VARIANT* pOut)
{
    GT_UNREFERENCED_PARAMETER(pSender);
    GT_UNREFERENCED_PARAMETER(flags);
    GT_UNREFERENCED_PARAMETER(pIn);
    GT_UNREFERENCED_PARAMETER(pOut);

    bool rc;

    // Call the DTE connector step over function:
    rc = vspDTEConnector::instance().stepOut();
    VSP_ASSERT(rc);
}

// ---------------------------------------------------------------------------
// Name:        CCodeXLVSPackagePackage::onStepOver
// Description: Handle CodeXL step over command
// Arguments:   CommandHandler* pSender
//              DWORD flags
//              VARIANT* pIn
//              VARIANT* pOut
// Return Val:  void
// Author:      Sigal Algranaty
// Date:        8/5/2011
// ---------------------------------------------------------------------------
void CodeXLVSPackage::onAPIStep(CommandHandler* pSender, DWORD flags, VARIANT* pIn, VARIANT* pOut)
{
    bool isResumeDebugging = VSCORE(vsc_OnAPIStep_OnUpdateMode_IsResumeDebuggingRequired());

    if (isResumeDebugging)
    {
        // Call the DTE connector resume function:
        vspDTEConnector::instance().resumeDebugging();
    }
    else
    {
        // Call the launch process function"
        OnStartButton(pSender, flags, pIn, pOut);
    }
}

void CodeXLVSPackage::onFrameStep(CommandHandler* pSender, DWORD flags, VARIANT* pIn, VARIANT* pOut)
{
    bool isResumeDebugging = VSCORE(vsc_OnFrameStep_OnUpdateMode_IsResumeDebuggingRequired());

    if (isResumeDebugging)
    {
        // Call the DTE connector resume function:
        vspDTEConnector::instance().resumeDebugging();
    }
    else
    {
        // Call the launch process function"
        OnStartButton(pSender, flags, pIn, pOut);
    }
}

// ---------------------------------------------------------------------------
// Name:        CCodeXLVSPackagePackage::onStepOver
// Description: Handle CodeXL step over command
// Arguments:   CommandHandler* pSender
//              DWORD flags
//              VARIANT* pIn
//              VARIANT* pOut
// Return Val:  void
// Author:      Sigal Algranaty
// Date:        8/5/2011
// ---------------------------------------------------------------------------
void CodeXLVSPackage::onDrawStep(CommandHandler* pSender, DWORD flags, VARIANT* pIn, VARIANT* pOut)
{
    bool isDebuggedProcExists = VSCORE(vsc_OnFrameStep_OnUpdateMode_IsDebuggedProcExists());

    if (isDebuggedProcExists)
    {
        // Call the DTE connector resume function:
        vspDTEConnector::instance().resumeDebugging();
    }
    else
    {
        // Call the launch process function"
        OnStartButton(pSender, flags, pIn, pOut);
    }
}


// ---------------------------------------------------------------------------
// Name:        CCodeXLVSPackagePackage::onUpdateFrameStep
// Description: Command handler for CodeXL frame step command update
// Arguments:   CommandHandler& handler
//              OLECMD* pOleCmd
//              OLECMDTEXT* pOleText
// Author:      Sigal Algranaty
// Date:        8/5/2011
// ---------------------------------------------------------------------------
void CodeXLVSPackage::onUpdateFrameStep(CommandHandler& handler, OLECMD* pOleCmd, OLECMDTEXT* pOleText)
{
    GT_UNREFERENCED_PARAMETER(&handler);
    GT_UNREFERENCED_PARAMETER(pOleText);

    bool isProcessSuspended = false;
    bool isProcessExists = false;
    bool shouldShow = false;
    VSCORE(vsc_OnUpdateFrameStep(isProcessSuspended, isProcessExists, shouldShow));

    if (pOleCmd != nullptr)
    {
        DWORD commandFlags = OLECMDF_SUPPORTED;

        // Enable the command if the debugged process exists and suspended:
        bool enableCommand = true;

        if (isProcessExists)
        {
            enableCommand = isProcessSuspended;
        }

        if (enableCommand)
        {
            commandFlags |= OLECMDF_ENABLED;
        }

        if (!shouldShow)
        {
            commandFlags |= OLECMDF_INVISIBLE;
        }

        pOleCmd->cmdf = commandFlags;
    }
}


// ---------------------------------------------------------------------------
// Name:        CCodeXLVSPackagePackage::onUpdateStepOver
// Description: Command handler for CodeXL step over command update
// Arguments:   CommandHandler& handler
//              OLECMD* pOleCmd
//              OLECMDTEXT* pOleText
// Author:      Sigal Algranaty
// Date:        8/5/2011
// ---------------------------------------------------------------------------
void CodeXLVSPackage::onUpdateStepOver(CommandHandler& handler, OLECMD* pOleCmd, OLECMDTEXT* pOleText)
{
    GT_UNREFERENCED_PARAMETER(&handler);
    GT_UNREFERENCED_PARAMETER(pOleText);

    bool isProcessSuspended = false;
    bool isProcessExists = false;
    bool shouldShow = false;
    VSCORE(vsc_OnUpdateStepOver(isProcessSuspended, isProcessExists, shouldShow));

    if (pOleCmd != nullptr)
    {
        DWORD commandFlags = OLECMDF_SUPPORTED;

        // Enable the command if the debugged process exists and suspended:
        bool enableCommand = isProcessExists && isProcessSuspended;

        if (enableCommand)
        {
            commandFlags |= OLECMDF_ENABLED;
        }

        if (!shouldShow)
        {
            commandFlags |= OLECMDF_INVISIBLE;
        }

        pOleCmd->cmdf = commandFlags;
    }
}

void CodeXLVSPackage::onUpdateStepOut(CommandHandler& handler, OLECMD* pOleCmd, OLECMDTEXT* pOleText)
{
    GT_UNREFERENCED_PARAMETER(&handler);
    GT_UNREFERENCED_PARAMETER(pOleText);

    bool isStepOutEnabled = false;
    bool shouldShow = false;

    // Invoke core logic.
    VSCORE(vsc_OnUpdateStepOut(isStepOutEnabled, shouldShow));

    if (pOleCmd != nullptr)
    {
        DWORD commandFlags = OLECMDF_SUPPORTED;

        if (isStepOutEnabled)
        {
            commandFlags |= OLECMDF_ENABLED;
        }

        if (!shouldShow)
        {
            commandFlags |= OLECMDF_INVISIBLE;
        }

        pOleCmd->cmdf = commandFlags;
    }
}

// ---------------------------------------------------------------------------
// Name:        CCodeXLVSPackagePackage::setToolWindowCaption
// Description: Sets a tool window caption
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        27/1/2011
// ---------------------------------------------------------------------------
bool CodeXLVSPackage::setToolWindowCaption(int windowId, const std::wstring& windowCaption)
{
    bool retVal = false;

    vspToolWindow* pToolWindow = nullptr;
    const int callsHistoryId = VSCORE(vsc_GetCallsHistoryListId());
    const int propertiesViewId = VSCORE(vsc_GetPropertiesViewId());
    const int objNavigationViewId = VSCORE(vsc_GetObjectNavigationTreeId());
    const int statisticsViewId = VSCORE(vsc_GetStatisticsViewId());
    const int memAnalysisViewId = VSCORE(vsc_GetMemoryAnalysisViewerId());
    const int stateVarsViewId = VSCORE(vsc_GetStateVariablesViewId());

    /*switch (windowId)*/

    if (windowId == callsHistoryId)
    {
        pToolWindow = _pCallsHistoryToolWindow;
    }
    else if (windowId == propertiesViewId)
    {
        pToolWindow = _pPropertiesToolWindow;
    }
    else if (windowId == objNavigationViewId)
    {
        pToolWindow = _pObjectsExplorerToolWindow;
    }
    else if (windowId == statisticsViewId)
    {
        pToolWindow = _pStatisticsToolWindow;
    }
    else if (windowId == stateVarsViewId)
    {
        pToolWindow = _pStateVariablesToolWindow;
    }
    else if (windowId == memAnalysisViewId)
    {
        pToolWindow = _pMemoryToolWindow;
    }
    else
    {
        pToolWindow = nullptr;
    }

    VSP_ASSERT(pToolWindow != nullptr);

    if (pToolWindow != nullptr)
    {
        // Set the tool window caption:
        retVal = pToolWindow->setCaption(windowCaption);
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        CCodeXLVSPackagePackage::getVSRegistryRootPath
// Description: Gets a WCHAR string with the root of the registry
// Author:      Uri Shomroni
// Date:        3/2/2011
// ---------------------------------------------------------------------------
const WCHAR* CodeXLVSPackage::getVSRegistryRootPath(bool tempRegPath)
{
    const WCHAR* retVal = nullptr;
    bool isDeallocRequired = false;
    WCHAR* pValueFromCore = VSCORE(CCodeXLVSPackagePackage_GetVSRegistryRootPath)(tempRegPath, isDeallocRequired);

    if (nullptr == pValueFromCore)
    {
        static WCHAR vsDefaultRegistryRoot[] = VSP_STR_VisualStudioDefaultRegistryRootPath;
        static WCHAR vsDefaultRegistryRootConfig[] = VSP_STR_VisualStudioDefaultRegistryRootPath2;
        retVal = tempRegPath ? vsDefaultRegistryRootConfig : vsDefaultRegistryRoot;
    }
    else // nullptr != pValueFromCore
    {
        // Create a temporary copy.
        static std::wstring wTmpStr;
        static std::wstring wRegStr;
        std::wstring& wUseStr = tempRegPath ? wTmpStr : wRegStr; // Do not overwrite one buffer with the other
        wUseStr = pValueFromCore;
        retVal = wUseStr.c_str();

        if (isDeallocRequired)
        {
            // Delete the original.
            VSCORE(vscDeleteWcharString)(pValueFromCore);
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        CCodeXLVSPackagePackage::validateDebugSettings
// Description: Verifies that the
// Return Val:  bool - Success / failure.
// Author:      Uri Shomroni
// Date:        8/2/2011
// ---------------------------------------------------------------------------
bool CodeXLVSPackage::validateDebugSettings(const std::wstring& executableFilePath, const std::wstring& workingDirectoryPath, bool isProjectTypeValid)
{
    // Invoke the core logic.
    return VSCORE(vsc_ValidateDebugSettings)(executableFilePath.c_str(), workingDirectoryPath.c_str(), isProjectTypeValid);
}

// ---------------------------------------------------------------------------
// Name:        CCodeXLVSPackagePackage::validateStartDebugAfterBuild
// Description: Checks if the projects needs build and if build fails what to do
// Return Val:  true - continue debugging
// Author:      Gilad Yarnitzky
// Date:        24/3/2011
// ---------------------------------------------------------------------------
bool CodeXLVSPackage::validateStartDebugAfterBuild()
{
    bool continueDebug = true;
    bool buildProject = false;

    std::vector<std::wstring> projectNamesToBuild;
    std::vector<VxDTE::Project*> projectsList;

    vspDTEConnector& theDTEConnector = vspDTEConnector::instance();
    int onRunWhenOutOfDateAction, onRunOrPreviewAction, onRunWhenErrorsAction;
    theDTEConnector.getBuildOptions(onRunWhenOutOfDateAction, onRunOrPreviewAction, onRunWhenErrorsAction);

    // Handle save files before build:
    if (onRunOrPreviewAction == VSP_PROMPT_TO_SAVE)
    {
        bool isChangedFilesSaveRequired = false;
        VSCORE(vsc_ShowSaveListDialog)(isChangedFilesSaveRequired, continueDebug, buildProject);

        if (isChangedFilesSaveRequired)
        {
            theDTEConnector.saveChangedFiles(true);
        }
    }
    else if (onRunOrPreviewAction == VSP_SAVE_ALL)
    {
        theDTEConnector.saveChangedFiles(true);
    }
    else if (onRunOrPreviewAction == VSP_SAVE_CHANGE_TO_OPEN_ONLY)
    {
        theDTEConnector.saveChangedFiles(false);
    }

    theDTEConnector.getBuildDependencies(projectsList);

    // If the user selected to continue (had the save prompt option on), or nothing to save:
    if (continueDebug)
    {
        // If Never build, done here:
        if (onRunWhenOutOfDateAction == VSP_NEVER_BUILD)
        {
            buildProject = false;
        }

        // Check If we need to build project:
        if (onRunWhenOutOfDateAction == VSP_ALWAYS_BUILD)
        {
            buildProject = true;
        }
        else if (!projectsList.empty())
        {
            // Show what is needed to build:
            theDTEConnector.constructProjectsNames(projectsList, projectNamesToBuild);
            wchar_t** pProjNamesArray = nullptr;
            int pProjNamesArrSize = 0;
            vspVsUtilsWstrVectorToWstrArray(projectNamesToBuild, pProjNamesArray, pProjNamesArrSize);
            VSCORE(CCodeXLVSPackagePackage_ValidateStartDebugAfterBuild)(pProjNamesArray, pProjNamesArrSize, buildProject, continueDebug);
        }
    }

    if (buildProject)
    {
        // Build project:
        if (!theDTEConnector.verifyStartupProjectBuilt())
        {
            // The user set in the options to launch old version in case of failure:
            if (onRunWhenErrorsAction == VSP_LAUNCH_OLD_VERSION)
            {
                continueDebug = true;
            }
            // The user set in the options not to continue in case of failure:
            else if (onRunWhenErrorsAction == VSP_DO_NOT_LAUNCH)
            {
                continueDebug = false;
            }
            else if (onRunWhenErrorsAction == VSP_PROMPT_TO_LAUNCH)
            {
                // Notify the user the build failed:
                IVsUIShell* piUIShell = nullptr;
                HRESULT hr = GetVsSiteCache().QueryService(SID_SVsUIShell, &piUIShell);

                if (SUCCEEDED(hr) && (piUIShell != nullptr))
                {
                    IID tempID; // not used
                    BSTR messageCaption(VSP_STR_BuildFailedCaption);
                    BSTR messageBody(VSP_STR_BuildFailedBody);
                    long result;

                    hr = piUIShell->ShowMessageBox(0, tempID, messageCaption, messageBody, nullptr, 0, OLEMSGBUTTON_YESNO, OLEMSGDEFBUTTON_SECOND, OLEMSGICON_INFO, false, &result);

                    if (SUCCEEDED(hr))
                    {
                        if (result == IDNO)
                        {
                            continueDebug  = false;
                        }
                    }

                    piUIShell->Release();
                }
            }
            else
            {
                //  Option setting for failure is wrong it should be checked what is defined in the options.
                VSP_ASSERT(false);
            }
        } // if (!theDTEConnector.verifyStartupProjectBuilt())
    } // if (buildProject)

#if defined VSP_VS11BUILD || defined VSP_VS12BUILD || defined VSP_VS14BUILD

    if (continueDebug)
    {
        std::wstring appUserModelId;
        continueDebug = theDTEConnector.deployStartupProject(appUserModelId);
        VSCORE(vsc_SetWindowsStoreAppUserModelID)(appUserModelId.c_str());
    }

#endif // VSP_VS11BUILD || VSP_VS12BUILD || defined VSP_VS14BUILD

    // Release the projects from the list:
    for (int nProject = 0 ; nProject < (int)projectsList.size(); nProject++)
    {
        projectsList[nProject]->Release();
    }

    return continueDebug;
}

// ---------------------------------------------------------------------------
// Name:        CCodeXLVSPackagePackage::registerLanguageServices
// Description: register the language services for .cl and .glsl files on Visual Studio 2010.
// Author:      Uri Shomroni
// Date:        20/01/2014
// ---------------------------------------------------------------------------
bool CodeXLVSPackage::registerLanguageServices(const std::wstring& registryRoot)
{
    bool retVal = false;

#if defined VSP_VS11BUILD || defined VSP_VS12BUILD || defined VSP_VS14BUILD

    GT_UNREFERENCED_PARAMETER(registryRoot);

    // On Visual Studio 11.0, 11.1, 11.2, 11.4 and 12.0, this causes crashes in the code editor when using smart tabs.
    // In addition, this method does not work for those VS versions.
    // So instead, we do nothing here.
    // See also:
    // http://msdn.microsoft.com/en-us/library/microsoft.visualstudio.shell.providelanguageextensionattribute.aspx
    // http://support.microsoft.com/kb/2911573/en (under "C++")
    retVal = true;

#else // ndefined VSP_VS11BUILD && ndefined VSP_VS12BUILD && ndefined VSP_VS14BUILD

    // Register our languages:
    // *.cl
    std::wstring extensionMappingKeyPath = registryRoot;
    vspVsUtilsRemoveTrailing(extensionMappingKeyPath, '\\');
    extensionMappingKeyPath.append(L"\\");
    extensionMappingKeyPath.append(L"FileExtensionMapping\\cl\\");
    HKEY extensionMappingKey;
    DWORD keyStatus = 0;
    LRESULT rcKey = RegCreateKeyEx(HKEY_CURRENT_USER, extensionMappingKeyPath.c_str(), 0, nullptr, 0, KEY_WRITE, nullptr, &extensionMappingKey, &keyStatus);

    if (rcKey == ERROR_SUCCESS)
    {
        // If the key did not exist before:
        if (keyStatus == REG_CREATED_NEW_KEY)
        {
            // Register this file extension to the C/C++ language service and the code editor.
            // This is equivalent to adding it in Tools > Options > Text Editor > File Extension.
            static const std::wstring codeEditorGUID = L"{9A634DE0-9841-4F52-A1B4-5ACA1608298D}";
            static const WCHAR* pLogViewID = L"LogViewID";
            static const std::wstring c_cppLanguageServiceGUID = L"{B2F072B0-ABC1-11D0-9D62-00C04FD9DFD9}";

            RegSetValueEx(extensionMappingKey, nullptr, 0, REG_SZ, (const BYTE*)codeEditorGUID.c_str(), (codeEditorGUID.length() + 1) * sizeof(wchar_t));
            RegSetValueEx(extensionMappingKey, pLogViewID, 0, REG_SZ, (const BYTE*)c_cppLanguageServiceGUID.c_str(), (c_cppLanguageServiceGUID.length() + 1) * sizeof(wchar_t));
        }

        // Close the key handle:
        RegCloseKey(extensionMappingKey);
        extensionMappingKey = nullptr;
    }

    // *.glsl
    extensionMappingKeyPath = registryRoot;
    vspVsUtilsRemoveTrailing(extensionMappingKeyPath, '\\');
    extensionMappingKeyPath.append(L"\\");
    extensionMappingKeyPath.append(L"FileExtensionMapping\\glsl\\");
    keyStatus = 0;
    rcKey = RegCreateKeyEx(HKEY_CURRENT_USER, extensionMappingKeyPath.c_str(), 0, nullptr, 0, KEY_WRITE, nullptr, &extensionMappingKey, &keyStatus);

    if (rcKey == ERROR_SUCCESS)
    {
        // If the key did not exist before:
        if (keyStatus == REG_CREATED_NEW_KEY)
        {
            // Register this file extension to the C/C++ language service and the code editor.
            // This is equivalent to adding it in Tools > Options > Text Editor > File Extension.
            static const std::wstring codeEditorGUID = L"{9A634DE0-9841-4F52-A1B4-5ACA1608298D}";
            static const WCHAR* pLogViewID = L"LogViewID";
            static const std::wstring c_cppLanguageServiceGUID = L"{B2F072B0-ABC1-11D0-9D62-00C04FD9DFD9}";

            RegSetValueEx(extensionMappingKey, nullptr, 0, REG_SZ, (const BYTE*)codeEditorGUID.c_str(), (codeEditorGUID.length() + 1) * sizeof(wchar_t));
            RegSetValueEx(extensionMappingKey, pLogViewID, 0, REG_SZ, (const BYTE*)c_cppLanguageServiceGUID.c_str(), (c_cppLanguageServiceGUID.length() + 1) * sizeof(wchar_t));
        }

        // Close the key handle:
        RegCloseKey(extensionMappingKey);
        extensionMappingKey = nullptr;
        keyStatus = 0;
    }

#endif

    return retVal;
}
// ---------------------------------------------------------------------------
// Name:        CCodeXLVSPackagePackage::verifyBaseViewsCreated
// Description: Some of the views are needed for creation or updating of other
//              views. This function makes sure these views are created:
// Return Val:  void
// Author:      Uri Shomroni
// Date:        14/2/2011
// ---------------------------------------------------------------------------
void CodeXLVSPackage::verifyBaseViewsCreated()
{
    // The process explorer tree and the properties views are used by other CodeXL views, therefor they
    // should be created when the package is loaded:
    static bool alreadyCreated = false;

    if (!alreadyCreated)
    {
        alreadyCreated = true;
        VSP_ASSERT(_pObjectsExplorerToolWindow != nullptr);

        if (_pObjectsExplorerToolWindow != nullptr)
        {
            _pObjectsExplorerToolWindow->setMyWindowCommandID(VSCORE(vsc_GetObjectNavigationTreeId)());
            _pObjectsExplorerToolWindow->Create();

            // Initialize the debug tree handler:
            VSCORE(vsc_InitDebugTreeHandler)();
        }

        VSP_ASSERT(_pPropertiesToolWindow != nullptr);

        if (_pPropertiesToolWindow != nullptr)
        {
            _pPropertiesToolWindow->setMyWindowCommandID(VSCORE(vsc_GetPropertiesViewId)());
            _pPropertiesToolWindow->Create();
        }

        // make sure the solution view is in the front:
        vspDTEConnector::instance().ExecuteCommand(L"View.SolutionExplorer");
    }
}


// ---------------------------------------------------------------------------
// Name:        CCodeXLVSPackagePackage::createSendErrorReportDialog
// Description: Creates the "Send Error Report" dialog.
// Author:      Sigal Algranaty
// Date:        21/6/2011
// ---------------------------------------------------------------------------
void CodeXLVSPackage::createSendErrorReportDialog()
{
    VSCORE(vsc_CreateSendErrorReportDialog)(m_pCoreImpl);
}


// ---------------------------------------------------------------------------
// Name:        CCodeXLVSPackagePackage::initializeDebuggerPlugin
// Description: Initialize the debugger plugin
// Author:      Sigal Algranaty
// Date:        20/5/2012
// ---------------------------------------------------------------------------
void CodeXLVSPackage::initializeDebuggerPlugin()
{
    VSCORE(vsc_InitDebuggerPlugin)();
}


// ---------------------------------------------------------------------------
// Name:        CCodeXLVSPackagePackage::onNewDebugEngine
// Description: Called when a new debug engine is created
// Author:      Uri Shomroni
// Date:        28/7/2013
// ---------------------------------------------------------------------------
void CodeXLVSPackage::onNewDebugEngine(void* pNewDebugEngine)
{
    VSCORE(vsc_OnNewDebugEngine)(m_pCoreImpl, pNewDebugEngine);
}


// ---------------------------------------------------------------------------
// Name:        CCodeXLVSPackagePackage::checkgDEBBugerInstallation
// Description: Checks if gDEBugger is installed and warn the user if it is
//              This assumes that the directory exists and that the user can not just disable gDEBugger, This is correct
//              since gDEBugger is installed with msi and not vsix so it can not be disabled through the extension manager
// Return Val:  void
// Author:      Gilad Yarnitzky
// Date:        6/11/2012
// ---------------------------------------------------------------------------
void CodeXLVSPackage::checkgDEBBugerInstallation()
{
    VSCORE(vsc_CheckDebuggerInstallation)();
}

// ---------------------------------------------------------------------------
// Name:        CCodeXLVSPackagePackage::registerDebugEngineInTemporaryHive
// Description: Adds the debug engine to the temporary registry hive
// Author:      Uri Shomroni
// Date:        6/11/2014
// ---------------------------------------------------------------------------
void CodeXLVSPackage::registerDebugEngineInTemporaryHive()
{
    // Register our custom debug engine using the appropriate registry entries:
    // (See http://msdn.microsoft.com/en-us/library/bb146733.aspx for more possible values)
    // We are passing fUserSpecific = TRUE, since we want to only modify the local registry.
    // Writing to the global registry causes it to be marked as dirty and causes the whole VS hive to be reloaded.
    const WCHAR* registryRoot = getVSRegistryRootPath(false);
    std::wstring engineName(VSP_STR_DebugEngineName);
    static const CLSID nativeProgramProviderCLSID = { 0x4ff9def4, 0x8922, 0x4d02, {0x93, 0x79, 0x3f, 0xfa, 0x64, 0xd1, 0xd6, 0x39} };
    HRESULT hrSetDebugEngineReg1 = SetMetric(metrictypeEngine, CLSID_vspDebugEngine, metricName, (unsigned short*)engineName.c_str(), TRUE, (unsigned short*)registryRoot);
    HRESULT hrSetDebugEngineReg2 = SetMetric(metrictypeEngine, CLSID_vspDebugEngine, metricCLSID, CLSID_vspDebugEngine, TRUE, (unsigned short*)registryRoot);
    HRESULT hrSetDebugEngineReg3 = SetMetric(metrictypeEngine, CLSID_vspDebugEngine, metricProgramProvider, nativeProgramProviderCLSID, TRUE, (unsigned short*)registryRoot);
    HRESULT hrSetDebugEngineReg4 = SetMetric(metrictypeEngine, CLSID_vspDebugEngine, metricAlwaysLoadLocal, (DWORD)1, TRUE, (unsigned short*)registryRoot);
    HRESULT hrSetDebugEngineReg5 = SetMetric(metrictypeEngine, CLSID_vspDebugEngine, metricAlwaysLoadProgramProviderLocal, (DWORD)1, TRUE, (unsigned short*)registryRoot);
    registryRoot = getVSRegistryRootPath(true);
    HRESULT hrSetDebugEngineReg6 = SetMetric(metrictypeEngine, CLSID_vspDebugEngine, metricName, (unsigned short*)engineName.c_str(), TRUE, (unsigned short*)registryRoot);
    HRESULT hrSetDebugEngineReg7 = SetMetric(metrictypeEngine, CLSID_vspDebugEngine, metricCLSID, CLSID_vspDebugEngine, TRUE, (unsigned short*)registryRoot);
    HRESULT hrSetDebugEngineReg8 = SetMetric(metrictypeEngine, CLSID_vspDebugEngine, metricProgramProvider, nativeProgramProviderCLSID, TRUE, (unsigned short*)registryRoot);
    HRESULT hrSetDebugEngineReg9 = SetMetric(metrictypeEngine, CLSID_vspDebugEngine, metricAlwaysLoadLocal, (DWORD)1, TRUE, (unsigned short*)registryRoot);
    HRESULT hrSetDebugEngineReg10 = SetMetric(metrictypeEngine, CLSID_vspDebugEngine, metricAlwaysLoadProgramProviderLocal, (DWORD)1, TRUE, (unsigned short*)registryRoot);
    bool constRegSuccess = SUCCEEDED(hrSetDebugEngineReg1) && SUCCEEDED(hrSetDebugEngineReg2) && SUCCEEDED(hrSetDebugEngineReg3) && SUCCEEDED(hrSetDebugEngineReg4) && SUCCEEDED(hrSetDebugEngineReg5);
    bool tempRegSuccess = SUCCEEDED(hrSetDebugEngineReg6) && SUCCEEDED(hrSetDebugEngineReg7) && SUCCEEDED(hrSetDebugEngineReg8) && SUCCEEDED(hrSetDebugEngineReg9) && SUCCEEDED(hrSetDebugEngineReg10);

    if (!constRegSuccess || !tempRegSuccess)
    {
        VSP_ASSERT(false);
    }
}

// ---------------------------------------------------------------------------
// Name:        CCodeXLVSPackagePackage::unregisterDebugEngineFromTemporaryHive
// Description: Removes the debug engine from the temporary registry hive
// Author:      Uri Shomroni
// Date:        6/11/2014
// ---------------------------------------------------------------------------
void CodeXLVSPackage::unregisterDebugEngineFromTemporaryHive()
{
    // Un-register our custom debug engine by removing the registry entries related to it:
    const WCHAR* vsRegistryRoot = getVSRegistryRootPath(false);
    REFGUID deGUID = VSCORE(vsc_GetDebugEngineGuid)();
    HRESULT hrUnSetDebugEngineReg1 = RemoveMetric(metrictypeEngine, deGUID, metricName, (unsigned short*)vsRegistryRoot);
    HRESULT hrUnSetDebugEngineReg2 = RemoveMetric(metrictypeEngine, deGUID, metricCLSID, (unsigned short*)vsRegistryRoot);
    HRESULT hrUnSetDebugEngineReg3 = RemoveMetric(metrictypeEngine, deGUID, metricProgramProvider, (unsigned short*)vsRegistryRoot);
    HRESULT hrUnSetDebugEngineReg4 = RemoveMetric(metrictypeEngine, deGUID, metricAlwaysLoadLocal, (unsigned short*)vsRegistryRoot);
    vsRegistryRoot = getVSRegistryRootPath(true);
    HRESULT hrUnSetDebugEngineReg5 = RemoveMetric(metrictypeEngine, deGUID, metricName, (unsigned short*)vsRegistryRoot);
    HRESULT hrUnSetDebugEngineReg6 = RemoveMetric(metrictypeEngine, deGUID, metricCLSID, (unsigned short*)vsRegistryRoot);
    HRESULT hrUnSetDebugEngineReg7 = RemoveMetric(metrictypeEngine, deGUID, metricProgramProvider, (unsigned short*)vsRegistryRoot);
    HRESULT hrUnSetDebugEngineReg8 = RemoveMetric(metrictypeEngine, deGUID, metricAlwaysLoadLocal, (unsigned short*)vsRegistryRoot);
    bool constRegSuccess = SUCCEEDED(hrUnSetDebugEngineReg1) && SUCCEEDED(hrUnSetDebugEngineReg2) && SUCCEEDED(hrUnSetDebugEngineReg3) && SUCCEEDED(hrUnSetDebugEngineReg4);
    bool tempRegSuccess = SUCCEEDED(hrUnSetDebugEngineReg5) && SUCCEEDED(hrUnSetDebugEngineReg6) && SUCCEEDED(hrUnSetDebugEngineReg7) && SUCCEEDED(hrUnSetDebugEngineReg8);

    if (!constRegSuccess || !tempRegSuccess)
    {
        VSP_ASSERT(false);
    }
}

void CodeXLVSPackage::updateProjectSettingsFromVS(bool& shouldDebug, bool& shouldProfile, bool& shouldFrameAnalyze, bool& isProjectTypeValid)
{
    // Get the project details we need to debug/profile:
    std::wstring executableFilePath;
    std::wstring workingDirectoryPath;
    std::wstring commandLineArguments;
    std::wstring environment;
    bool isProjectOpened = false;
    bool isNonNativeProject = false;
    vspDTEConnector& theDTEConnector = vspDTEConnector::instance();
    theDTEConnector.getStartupProjectDebugInformation(executableFilePath, workingDirectoryPath, commandLineArguments, environment, isProjectOpened, isProjectTypeValid, isNonNativeProject);

    // Invoke the core logic.
    VSCORE(vsc_UpdateProjectSettingsFromVS)(executableFilePath.c_str(), workingDirectoryPath.c_str(),
                                            commandLineArguments.c_str(), environment.c_str(), shouldDebug, shouldProfile, shouldFrameAnalyze);
}

void CodeXLVSPackage::executeDebugSession(bool isProjectTypeValid)
{
    if (!VSCORE(vsc_IsDebuggedProcessExists)())
    {
        // Get the visual studio debugger:
        IVsDebugger* piDebugger = nullptr;
        HRESULT hr = GetVsSiteCache().QueryService(SID_SVsShellDebugger, &piDebugger);
        VSP_ASSERT(SUCCEEDED(hr));
        VSP_ASSERT(piDebugger != nullptr);

        if (piDebugger != nullptr)
        {
            // Validate driver and GPU.
            VSCORE(vsc_ValidateDriverAndGpu)();

            if (validateStartDebugAfterBuild())
            {
                // Clear any old edit and continue information:
                vspDTEConnector::instance().clearOpenedFiles();

                // Validate the debug settings and display any required messages:
                wchar_t* pExecutableFilePathBuffer = nullptr;
                wchar_t* pWorkingDirectoryPath = nullptr;
                wchar_t* pCommandLineArguments = nullptr;
                wchar_t* pEnvironment = nullptr;

                // Extract the current settings.
                VSCORE(vsc_GetCurrentSettings)(pExecutableFilePathBuffer, pWorkingDirectoryPath, pCommandLineArguments, pEnvironment);
                std::wstring executableFilePath      = (pExecutableFilePathBuffer != nullptr) ? pExecutableFilePathBuffer : L"";
                std::wstring workingDirectoryPath    = (pWorkingDirectoryPath != nullptr) ? pWorkingDirectoryPath : L"";;
                std::wstring commandLineArguments    = (pCommandLineArguments != nullptr) ? pCommandLineArguments : L"";;
                std::wstring environment             = (pEnvironment != nullptr) ? pEnvironment : L"";;

                bool rcDbg = validateDebugSettings(executableFilePath, workingDirectoryPath, isProjectTypeValid);

                if (rcDbg)
                {
                    // Set the parameters:
                    // Note: the target info is released by the IVsDebugger, so we need to allocate it:
                    VsDebugTargetInfo* pDebugTargetInfo = new VsDebugTargetInfo();
                    ::memset(pDebugTargetInfo, 0, sizeof(VsDebugTargetInfo));

                    pDebugTargetInfo->cbSize = sizeof(VsDebugTargetInfo);
                    pDebugTargetInfo->dlo = DLO_CreateProcess;
                    pDebugTargetInfo->fSendStdoutToOutputWindow = 0; // Let stdout stay with the application.
                    pDebugTargetInfo->clsidCustom = CLSID_vspDebugEngine; // Set the launching engine the sample engine guid
                    pDebugTargetInfo->grfLaunch = 0;
                    pDebugTargetInfo->bstrRemoteMachine = nullptr; // debug locally
                    pDebugTargetInfo->bstrExe = SysAllocString(executableFilePath.c_str());
                    pDebugTargetInfo->bstrCurDir = SysAllocString(workingDirectoryPath.c_str());
                    BSTR argumentsAsBSTR = nullptr;

                    if (!commandLineArguments.empty())
                    {
                        // If there are no command line arguments, there is no need to allocate an empty string:
                        argumentsAsBSTR = SysAllocString(commandLineArguments.c_str());
                    }

                    pDebugTargetInfo->bstrArg = argumentsAsBSTR;
                    BSTR environmentAsBSTR = nullptr;

                    if (!environment.empty())
                    {
                        // While the information from the DTE is newline-separated, we need semicolons as the separators:
                        vspVsUtilsWReplaceAllOccurrences(environment, L"\n", L";");

                        // An environment variable name cannot start with a space:
                        vspVsUtilsWReplaceAllOccurrences(environment, L"; ", L";");

                        // If there are no environment variables defined, there is no need to allocate an empty string:
                        environmentAsBSTR = SysAllocString(environment.c_str());
                    }

                    pDebugTargetInfo->bstrEnv = environmentAsBSTR;

                    // Clear the old debug engine before creating a new one:
                    vspPackageWrapper::instance().uninformPackageOfDebugEngine();

                    // BUG458281 - an external application (regedit or another instance of Visual Studio) may have removed
                    // The debug engine entry from the temporary registry hive. Make sure the values are still there:
                    registerDebugEngineInTemporaryHive();

                    // For 64-bit debugging, we need to set the Dll directory here, since we want it to be inherited
                    // by Visual Studio's msvsmon.exe remote process, which is launched as soon as LaunchDebugTargets is called:
                    const wchar_t* pDllDir = nullptr;
                    VSCORE(vsc_GetProcessDllDirectoryFromCreationData)(executableFilePath.c_str(), pDllDir);
                    wchar_t oldDllDir[MAX_PATH + 1] = { 0 };
                    DWORD oldDllDirLen = 0;

                    if (nullptr != pDllDir)
                    {
                        oldDllDirLen = ::GetDllDirectory(MAX_PATH, oldDllDir);
                        oldDllDir[oldDllDirLen] = (wchar_t)0;

                        ::SetDllDirectory(pDllDir);
                    }

                    // Launch the debugged application by having the SDM call our debug engine:
                    hr = piDebugger->LaunchDebugTargets(1, pDebugTargetInfo);

                    if (nullptr != pDllDir)
                    {
                        if (0 == oldDllDirLen)
                        {
                            ::SetDllDirectory(nullptr);
                        }
                        else
                        {
                            ::SetDllDirectory(oldDllDir);
                        }
                    }

                    if (!SUCCEEDED(hr))
                    {
                        VSP_ASSERT(SUCCEEDED(hr));

                        // Display a message to the user.
                        VSCORE(vsc_DisplayDebugProcessLaunchFailureMessage)(hr);
                    }

                    // Release the allocated strings.
                    VSCORE(vscDeleteWcharString)(pExecutableFilePathBuffer);
                    VSCORE(vscDeleteWcharString)(pWorkingDirectoryPath);
                    VSCORE(vscDeleteWcharString)(pCommandLineArguments);
                    VSCORE(vscDeleteWcharString)(pEnvironment);
                }
            }

            // Release the debugger interface:
            piDebugger->Release();
        }
    }
    else // gaDebuggedProcessExists()
    {
        // Call the VS facilities "continue" command, so it will call the debug engine back to resume:
        vspDTEConnector::instance().resumeDebugging();
    }
}

void CodeXLVSPackage::executeProfileSession()
{
    if (validateStartDebugAfterBuild())
    {
        // Load the current project file if not loaded:
        VSP_ASSERT(nullptr != m_pTimer);

        if (nullptr != m_pTimer)
        {
            m_pTimer->onClockTick();
        }

        // Run the profile action:
        VSCORE(vsc_ExecuteProfileSession)();
    }
}

void CodeXLVSPackage::executeFrameAnalysis()
{
    if (validateStartDebugAfterBuild())
    {
        VSCORE(vsc_ExecuteFrameAnalysisSession)();
    }
}

void CodeXLVSPackage::buildOpenCLFile()
{
    std::wstring pFilePathAsStr;
    bool hasActiveDocument = vspDTEConnector::instance().getActiveDocumentFileFullPath(pFilePathAsStr);

    if (hasActiveDocument && !pFilePathAsStr.empty())
    {
        VSCORE(vscDTEConnector_BuildOpenCLFile)(pFilePathAsStr.c_str());
    }

}

void CodeXLVSPackage::onUpdateLaunchProfileAction(bool& isActionEnabled, bool& isActionChecked)
{
    isActionChecked = false;
    isActionEnabled = vspDTEConnector::instance().isSolutionLoaded();
    VSCORE(vsc_OnUpdateLaunchProfileAction)(isActionEnabled, isActionChecked);
}

void CodeXLVSPackage::onUpdateLaunchDebugAction(bool& isActionEnabled, bool& isActionChecked)
{
    isActionChecked = false;
    bool processExists = false;
    bool processRunning = false;
    VSCORE(vsc_UpdateLaunchDebugAction)(processExists, processRunning);

    bool solutionLoaded = vspDTEConnector::instance().isSolutionLoaded();


    // Check if our debugger or native debugger is running:
    bool isDebuggerRunning = vspDTEConnector::instance().isDebuggingOn();

    // Check if our debugger is running (in this case we want to leave the run command on, and use it as continue command):
    bool isNativeDebuggerRunning = isDebuggerRunning && !(VSCORE(vsc_IsAnyDebugEngineAvailable)());

    // Enable the command if we are not running, the solution is loaded, and no debug engine is running:
    isActionEnabled = solutionLoaded && (!processRunning) && !isNativeDebuggerRunning;

    // This means profiler is not running
    isActionEnabled = isActionEnabled && !VSCORE(vsc_CanStopCurrentRun)();
}

void CodeXLVSPackage::onUpdateLaunchFrameAnalysisAction(bool& isActionEnabled, bool& isActionChecked)
{
    isActionChecked = false;
    isActionEnabled = vspDTEConnector::instance().isSolutionLoaded();
    VSCORE(vsc_OnUpdateLaunchFrameAnalysisAction)(isActionEnabled, isActionChecked);
}

void CodeXLVSPackage::SetProgressInfo(const wchar_t* pProgBarLabel, bool isInProgress, unsigned long progBarComplete, unsigned long progBarRange)
{
    if (pProgBarLabel != nullptr)
    {
        CodeXLVSPackage* pPackage = vspPackageWrapper::instance().getPackage();

        if (pPackage != nullptr)
        {
            // Update all the commands after the focused windows are set:
            CComPtr<IVsStatusbar> spStatusBar;
            CHKHR(pPackage->GetVsSiteCache().QueryService(SID_SVsStatusbar, &spStatusBar));
            VSP_ASSERT(spStatusBar != nullptr);

            if (spStatusBar != nullptr)
            {
                BSTR progressText = SysAllocString(pProgBarLabel);
                spStatusBar->Progress(&_userCookie, isInProgress, progressText, progBarComplete, progBarRange);
                SysFreeString(progressText);
            }
        }
    }
}

void CodeXLVSPackage::CloseDocumentsOfDeletedFiles() const
{
    vspDTEConnector::instance().closeDocumentsOfDeletedFiles();
}

bool CodeXLVSPackage::SaveFileWithPath(const wchar_t* pFilePathStr) const
{
    return vspDTEConnector::instance().saveFileWithPath(pFilePathStr);
}

bool CodeXLVSPackage::GetFunctionBreakpoints(wchar_t**& pEnabledFunctionBreakpointsBuffer, int& enabledFuncBreakpointsCount,
                                             wchar_t**& pDisabledFunctionBreakpointsBuffer, int& disabledFuncBreakpointsCount) const
{
    std::vector<std::wstring> enabledBreakpointNames, disabledBreakpointNames;
    bool isOk = vspDTEConnector::instance().getFunctionBreakpoints(enabledBreakpointNames, disabledBreakpointNames);
    VSP_ASSERT(isOk);

    if (isOk)
    {
        vspVsUtilsWstrVectorToWstrArray(enabledBreakpointNames, pEnabledFunctionBreakpointsBuffer, enabledFuncBreakpointsCount);
        vspVsUtilsWstrVectorToWstrArray(disabledBreakpointNames, pDisabledFunctionBreakpointsBuffer, disabledFuncBreakpointsCount);
    }

    return isOk;
}

bool CodeXLVSPackage::CloseFile(const wchar_t* pFilePath) const
{
    void* piWindow = nullptr;
    bool retVal = vspDTEConnector::instance().getWindowFromFilePath(pFilePath, piWindow);

    if (retVal)
    {
        retVal = vspDTEConnector::instance().closeAndReleaseWindow(piWindow);
    }

    return retVal;
}

bool CodeXLVSPackage::OpenFileAtPosition(const wchar_t* pFilePath, int lineNumber /*= 0*/, int columnNumber /*= 0*/, bool selectLine /*= true*/) const
{
    GT_UNREFERENCED_PARAMETER(columnNumber);

    return vspDTEConnector::instance().openFileAtPosition((pFilePath != nullptr ? pFilePath : L""), lineNumber, selectLine);
}

void CodeXLVSPackage::DeleteWcharStrBuffers(wchar_t**& pBuffersArray, int buffersArraySize) const
{
    VSP_ASSERT((0 < buffersArraySize) || (nullptr == pBuffersArray));

    if (nullptr != pBuffersArray)
    {
        vspVsUtilsDeleteWcharBuffersArray(pBuffersArray, buffersArraySize);
    }
}

bool CodeXLVSPackage::scvOwnerGetWindowFromFilePath(const wchar_t* pFilePath, void*& pWindowBuffer) const
{
    vspDTEConnector& theDTEConnector = vspDTEConnector::instance();
    return theDTEConnector.getWindowFromFilePath(pFilePath, pWindowBuffer);
}

bool CodeXLVSPackage::scvOwnerCloseAndReleaseWindow(void*& pWindow, bool closeWindow /*= true*/) const
{
    vspDTEConnector& theDTEConnector = vspDTEConnector::instance();
    return theDTEConnector.closeAndReleaseWindow(pWindow, closeWindow);
}

bool CodeXLVSPackage::scvOwnerOpenFileAtPosition(const wchar_t* pFilePath, int lineNumber /*= 0*/, int columnNumber /*= 0*/, bool selectLine /*= true*/) const
{
    return this->OpenFileAtPosition(pFilePath, lineNumber, columnNumber, selectLine);
}

bool CodeXLVSPackage::IsAnyOpenedFileModified() const
{
    return vspDTEConnector::instance().wasAnyOpenedFileModified();
}

void CodeXLVSPackage::ClearOpenFiles() const
{
    vspDTEConnector::instance().clearOpenedFiles();
}

bool CodeXLVSPackage::IsSrcLocationBreakpointEnabled(const wchar_t* filePath, int lineNumber, bool& isEnabled) const
{
    return vspDTEConnector::instance().isSourceLocationBreakpointEnabled(filePath, lineNumber, isEnabled);
}

bool CodeXLVSPackage::IsFuncBreakpointEnabled(const wchar_t* functionName, bool& isEnabled) const
{
    return vspDTEConnector::instance().isFunctionBreakpointEnabled(functionName, isEnabled);
}

bool CodeXLVSPackage::DisableFuncBreakpoint(const wchar_t* functionName) const
{
    return vspDTEConnector::instance().disableFunctionBreakpoint(functionName);
}

bool CodeXLVSPackage::AddBreakpointInSourceLocation(const wchar_t* filePath, int lineNumber, bool enabled) const
{
    return vspDTEConnector::instance().addBreakpointInSourceLocation(filePath, lineNumber, enabled);
}

bool CodeXLVSPackage::RemoveBreakpointsInSourceLocation(const wchar_t* filePath, int lineNumber) const
{
    return vspDTEConnector::instance().removeBreakpointsInSourceLocation(filePath, lineNumber);
}

void CodeXLVSPackage::CloseDisassemblyWindow() const
{
    vspDTEConnector::instance().CloseDisassemblyWindow();
}

void CodeXLVSPackage::ForceVariablesReevaluation() const
{
    vspDTEConnector::instance().forceVariablesReevaluation();
}

bool CodeXLVSPackage::RaiseStatisticsView()
{
    // Open the statistics view:
    onStatisticsView(nullptr, 0, nullptr, nullptr);
    return true;
}

bool CodeXLVSPackage::RaiseMemoryView()
{
    onMemoryView(nullptr, 0, nullptr, nullptr);
    return true;
}

void CodeXLVSPackage::SetToolWindowCaption(int windowId, const wchar_t* windowCaption)
{
    setToolWindowCaption(windowId, windowCaption);
}

void CodeXLVSPackage::SaveChangedFiles(bool saveSolutionAndProject) const
{
    vspDTEConnector::instance().saveChangedFiles(saveSolutionAndProject);
}

bool CodeXLVSPackage::OpenSolution(const wchar_t* solutionName) const
{
    return vspDTEConnector::instance().openSolution(solutionName);
}

bool CodeXLVSPackage::ivwmoVerifyBaseViewsCreated()
{
    bool retVal = false;

    if (m_isSited)
    {
        verifyBaseViewsCreated();
        retVal = true;
    }

    return retVal;
}

bool CodeXLVSPackage::ShouldUpdateProgress()
{
    bool ret = false;
    // Update all the commands after the focused windows are set:
    CComPtr<IVsStatusbar> spStatusBar;
    CHKHR(GetVsSiteCache().QueryService(SID_SVsStatusbar, &spStatusBar));
    VSP_ASSERT(spStatusBar != nullptr);

    if (spStatusBar != nullptr)
    {
        ret = true;
    }

    return ret;
}

void CodeXLVSPackage::ClearMessagePane() const
{
    vspPackageWrapper::instance().clearMessagePane();
}

void CodeXLVSPackage::OutputMessage(const wchar_t* pMsgBuffer, bool outputOnlyToLog)
{
    std::wstring tmpBuffer = pMsgBuffer;
    vspPackageWrapper::instance().outputMessage(tmpBuffer, outputOnlyToLog);
}

void CodeXLVSPackage::DeleteWCharStringBuffer(wchar_t*& pBuffer)
{
    delete[] pBuffer;
    pBuffer = nullptr;
}

void CodeXLVSPackage::ClearBuildPane()
{
    vspPackageWrapper::instance().clearBuildPane();
}

void CodeXLVSPackage::OutputBuildMessage(const wchar_t* pMsgToPrint, bool outputOnlyToLog, const wchar_t* pfile, int line)
{
    std::wstring msgBuf(pMsgToPrint != nullptr ? pMsgToPrint : L"");
    std::wstring fileBuf(pMsgToPrint != nullptr ? pfile : L"");
    vspPackageWrapper::instance().outputBuildMessage(msgBuf, outputOnlyToLog, fileBuf, line);
}

void CodeXLVSPackage::ivacoOwnerDeleteWCharStr(wchar_t*& pStr)
{
    delete[] pStr;
    pStr = nullptr;
}

void CodeXLVSPackage::UpdateProjectSettingsFromStartupProject()
{
    vspUpdateProjectSettingsFromStartupProject();
}

void CodeXLVSPackage::InformPackageOfNewDebugEngine(void* pNewEngine) const
{
    vspPackageWrapper::instance().informPackageOfNewDebugEngine(pNewEngine);
}

void CodeXLVSPackage::SetHexDisplayMode()
{
    vspDTEConnector::instance().setHexDisplayMode();
}

bool CodeXLVSPackage::GetVsWindowsManagementMode(VsWindowsManagementMode& buffer) const
{
    bool ret = false;
    buffer = VS_WMM_UNKNOWN;

#ifdef VSP_VS10BUILD
    buffer = VS_WMM_VS10;
    ret = true;
#elif defined VSP_VS11BUILD
    buffer = VS_WMM_VS11;
    ret = true;
#elif defined VSP_VS12BUILD
    buffer = VS_WMM_VS12;
    ret = true;
#elif defined VSP_VS14BUILD
    buffer = VS_WMM_VS14;
    ret = true;
#else
#error Unknown MSVC version
#endif

    return ret;
}


void CodeXLVSPackage::OnUpdateStartButton(CommandHandler& handler, OLECMD* pOleCmd, OLECMDTEXT* pOleText)
{
    GT_UNREFERENCED_PARAMETER(handler);
    GT_UNREFERENCED_PARAMETER(pOleCmd);
    GT_UNREFERENCED_PARAMETER(pOleText);


    bool isActionEnabled = true, isActionChecked = false, isActionVisible = true;

    wchar_t* pCurrentMode = nullptr;
    VSCORE(vsc_GetCurrentModeStr)(pCurrentMode);
    VSP_ASSERT(pCurrentMode != nullptr);
    std::wstring currentMode((pCurrentMode != nullptr) ? pCurrentMode : L"");
    VSCORE(vscDeleteWcharString)(pCurrentMode);

    if (currentMode.empty())
    {
        currentMode = VSCORE(vsc_Get_GD_STR_executionMode)();
    }

    if (currentMode == VSCORE(vsc_GetProfileModeStr)())
    {
        onUpdateLaunchProfileAction(isActionEnabled, isActionChecked);
    }
    else if (currentMode == VSCORE(vsc_GetExecutionModeStr)())
    {
        onUpdateLaunchDebugAction(isActionEnabled, isActionChecked);
    }
    else if (currentMode == VSCORE(vsc_GetFrameAnalysisModeStr)())
    {
        onUpdateLaunchFrameAnalysisAction(isActionEnabled, isActionChecked);
    }
    else
    {
        isActionEnabled = false;
        isActionVisible = false;
    }

    if (pOleCmd != nullptr)
    {
        DWORD commandFlags = OLECMDF_SUPPORTED;

        if (isActionEnabled)
        {
            commandFlags |= OLECMDF_ENABLED;
        }

        if (isActionChecked)
        {
            commandFlags |= OLECMDF_LATCHED;
        }

        if (!isActionVisible)
        {
            commandFlags |= OLECMDF_INVISIBLE;
        }

        pOleCmd->cmdf = commandFlags;
    }

    int bufferSize = (int)pOleText->cwBuf;
    int commandStrLength = 0;
    wchar_t* pCmdNameBuf = nullptr;
    VSCORE(vsc_OnUpdateStartButtonGetText)(bufferSize, pCmdNameBuf, commandStrLength);
    VSP_ASSERT(pCmdNameBuf != nullptr);

    if (pCmdNameBuf != nullptr)
    {
        // Copy the characters to the buffer:
        lstrcpy(pOleText->rgwz, pCmdNameBuf);
        pOleText->cwActual = (ULONG)(commandStrLength + 1);
        VSCORE(vscDeleteWcharString)(pCmdNameBuf);
    }

    if (pOleCmd != nullptr)
    {
        DWORD commandFlags = OLECMDF_SUPPORTED;

        if (isActionEnabled)
        {
            commandFlags |= OLECMDF_ENABLED;
        }

        pOleCmd->cmdf = commandFlags;
    }
}

void CodeXLVSPackage::OnConfigureRemoteHost(CommandHandler* pSender, DWORD flags, VARIANT* pIn, VARIANT* pOut)
{
    GT_UNREFERENCED_PARAMETER(pSender);
    GT_UNREFERENCED_PARAMETER(flags);
    GT_UNREFERENCED_PARAMETER(pIn);
    GT_UNREFERENCED_PARAMETER(pOut);

    VSCORE(vsc_OnConfigureRemoteHost)();
}

void CodeXLVSPackage::OnUpdateConfigureRemoteHost(CommandHandler& handler, OLECMD* pOleCmd, OLECMDTEXT* pOleText)
{
    GT_UNREFERENCED_PARAMETER(handler);
    GT_UNREFERENCED_PARAMETER(pOleCmd);
    GT_UNREFERENCED_PARAMETER(pOleText);

    bool isActionEnabled = false;

    VSCORE(vsc_OnUpdateConfigureRemoteHost)(isActionEnabled);

    if (pOleCmd != nullptr)
    {
        DWORD commandFlags = OLECMDF_SUPPORTED;

        if (isActionEnabled)
        {
            commandFlags |= OLECMDF_ENABLED;
        }

        pOleCmd->cmdf = commandFlags;
    }
}

