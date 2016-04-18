//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file vspPackageCommandHandler.h
///
//==================================================================================

//------------------------------ vspPackageCommandHandler.h ------------------------------

#ifndef __VSPPACKAGECOMMANDHANDLER_H
#define __VSPPACKAGECOMMANDHANDLER_H


#pragma once

// VS includes:
//#include <VSLCommandTarget.h>
#include <VsDbgCmd.h>

// Infra:
//#include <AMDTGpuDebuggingComponents/Include/gdStringConstants.h>

// Local:
#include <src/vspToolWindow.h>
#include <src/vspUnknown.h>
//#include <src/vspWindowsManager.h>
#include <src/vscCommandTarget.h>
#include <src/Package.h>

#include "src/resource.h"       // main symbols
#include "src/Guids.h"
#include "..\CodeXLVSPackageUI\Resource.h"

#include "..\CodeXLVSPackageUI\CommandIds.h"


using namespace VSL;

// ----------------------------------------------------------------------------------
// Class Name:          vspPackageCommandHandler :
// General Description: This class is used to add command handling for CodeXL package
//                      class.
//                      VS compiler has a weird limitation of 31 max amount of static
//                      variables in a block, and VSP_COMMAND_MAP_ENTRY macro uses a static
//                      variables -> max 31 command handlers for a class...
//                      The purpose of this class is to expand the command handlers amount
//                      for CodeXL package.
// Author:              Sigal Algranaty
// Creation Date:       29/5/2011
// ----------------------------------------------------------------------------------
class vspPackageCommandHandler :
    public CComObjectRootEx<CComSingleThreadModel>,
    public CComPtrBase<vspPackageCommandHandler >,
    public IOleCommandTargetVSPImpl<vspPackageCommandHandler >,
    public vspCUnknown
{
public:

    // COM objects typically should not be cloned, and this prevents cloning by declaring the
    // copy constructor and assignment operator private (NOTE:  this macro includes the deceleration of
    // a private section, so everything following this macro and preceding a public or protected
    // section will be private).
    VSL_DECLARE_NOT_COPYABLE(vspPackageCommandHandler)


public:
    vspPackageCommandHandler();
    ~vspPackageCommandHandler();

    ////////////////////////////////////////////////////////////
    // IUnknown methods
    STDMETHOD_(ULONG, AddRef)(void);
    STDMETHOD_(ULONG, Release)(void);
    STDMETHOD(QueryInterface)(REFIID riid, LPVOID* ppvObj);

    static vspCommandHandler* GetCommand(const vscCommandId& rId);

    // the following functions are command handlers called when the user selects the command to show the one of our tool windows:
    HRESULT onProjectSettings(vspCommandHandler* pSender, DWORD flags, VARIANT* pIn, VARIANT* pOut);
    HRESULT onUpdateProjectSettings(vspCommandHandler& handler, OLECMD* pOleCmd, OLECMDTEXT* pOleText);
    HRESULT onUpdateOptions(vspCommandHandler& handler, OLECMD* pOleCmd, OLECMDTEXT* pOleText);
    HRESULT onOptions(vspCommandHandler* pSender, DWORD flags, VARIANT* pIn, VARIANT* pOut);
    HRESULT onSystemInformation(vspCommandHandler* pSender, DWORD flags, VARIANT* pIn, VARIANT* pOut);
    HRESULT onDisabledFeatures(vspCommandHandler* pSender, DWORD flags, VARIANT* pIn, VARIANT* pOut);
    HRESULT onHexDisplayMode(vspCommandHandler* pSender, DWORD flags, VARIANT* pIn, VARIANT* pOut);
    HRESULT onUpdateHexDisplayMode(vspCommandHandler& handler, OLECMD* pOleCmd, OLECMDTEXT* pOleText);
    HRESULT onUpdateProfileMode(vspCommandHandler& handler, OLECMD* pOleCmd, OLECMDTEXT* pOleText);
    HRESULT onUpdateRefreshSessions(vspCommandHandler& handler, OLECMD* pOleCmd, OLECMDTEXT* pOleText);
    HRESULT onProfileMode(vspCommandHandler* pSender, DWORD flags, VARIANT* pIn, VARIANT* pOut);
    HRESULT onPPSelectCounters(vspCommandHandler* pSender, DWORD flags, VARIANT* pIn, VARIANT* pOut);
    HRESULT onUpdatePPSelectCounters(vspCommandHandler& handler, OLECMD* pOleCmd, OLECMDTEXT* pOleText);

    HRESULT onAttach(vspCommandHandler* pSender, DWORD flags, VARIANT* pIn, VARIANT* pOut);

    HRESULT onStop(vspCommandHandler* pSender, DWORD flags, VARIANT* pIn, VARIANT* pOut);
    HRESULT onBreak(vspCommandHandler* pSender, DWORD flags, VARIANT* pIn, VARIANT* pOut);

    HRESULT onRefreshSessions(vspCommandHandler* pSender, DWORD flags, VARIANT* pIn, VARIANT* pOut);
    HRESULT onCapture(vspCommandHandler* pSender, DWORD flags, VARIANT* pIn, VARIANT* pOut);

    // KA command functions:
    HRESULT onUpdateOpenCLBuild(vspCommandHandler& handler, OLECMD* pOleCmd, OLECMDTEXT* pOleText);
    HRESULT onOpenCLBuild(vspCommandHandler* pSender, DWORD flags, VARIANT* pIn, VARIANT* pOut);
    HRESULT onUpdateAddOpenCLFile(vspCommandHandler& handler, OLECMD* pOleCmd, OLECMDTEXT* pOleText);
    HRESULT onAddOpenCLFile(vspCommandHandler* pSender, DWORD flags, VARIANT* pIn, VARIANT* pOut);
    HRESULT onUpdateCreateSourceFile(vspCommandHandler& handler, OLECMD* pOleCmd, OLECMDTEXT* pOleText);
    HRESULT onCreateSourceFile(vspCommandHandler* pSender, DWORD flags, VARIANT* pIn, VARIANT* pOut);
    HRESULT onDeviceOptions(vspCommandHandler* pSender, DWORD flags, VARIANT* pIn, VARIANT* pOut);
    HRESULT onKernelSettingOptions(vspCommandHandler* pSender, DWORD flags, VARIANT* pIn, VARIANT* pOut);
    HRESULT OnUpdateonDeviceOptions(vspCommandHandler& handler, OLECMD* pOleCmd, OLECMDTEXT* pOleText);

    /// handling cancel build command
    HRESULT onCancelBuild(vspCommandHandler* pSender, DWORD flags, VARIANT* pIn, VARIANT* pOut);

    /// update UI of cancel build command
    HRESULT onUpdateCancelBuild(vspCommandHandler& handler, OLECMD* pOleCmd, OLECMDTEXT* pOleText);



    // work item combo:
    HRESULT onUpdateWorkItemCoord(const vspCommandHandler& rSender, _Inout_ OLECMD* pOleCmd, _Inout_ OLECMDTEXT* pOleText);
    HRESULT onWorkItemCoordGetList(_In_ vspCommandHandler* pSender, DWORD flags, _In_ VARIANT* pIn, _Out_ VARIANT* pVarOut);
    HRESULT onWorkItemCoordChanged(_In_ vspCommandHandler* pSender, DWORD flags, _In_ VARIANT* pIn, _Out_ VARIANT* pOut);

    // Hex display:

    // Disabled features:
    HRESULT onUpdateDisabledFeatures(vspCommandHandler& handler, OLECMD* pOleCmd, OLECMDTEXT* pOleText);

    // Kernel Analyzer Toolbar
    // the target list combo functions:
    HRESULT onUpdateFolderModelCombo(const vspCommandHandler& rSender, _Inout_ OLECMD* pOleCmd, _Inout_ OLECMDTEXT* pOleText);
    HRESULT onShaderModelComboGetList(_In_ vspCommandHandler* pSender, DWORD flags, _In_ VARIANT* pIn, _Out_ VARIANT* pVarOut);
    HRESULT onShaderModelComboChanged(_In_ vspCommandHandler* pSender, DWORD flags, _In_ VARIANT* pIn, _Out_ VARIANT* pOut);
    // the open gl shader type list combo functions:
    HRESULT onUpdateTypeCombo(const vspCommandHandler& rSender, _Inout_ OLECMD* pOleCmd, _Inout_ OLECMDTEXT* pOleText);
    HRESULT onTypeComboGetList(_In_ vspCommandHandler* pSender, DWORD flags, _In_ VARIANT* pIn, _Out_ VARIANT* pVarOut);
    HRESULT onTypeComboChanged(_In_ vspCommandHandler* pSender, DWORD flags, _In_ VARIANT* pIn, _Out_ VARIANT* pOut);
    // the functions list combo functions:
    HRESULT onUpdateFunctionName(const vspCommandHandler& rSender, _Inout_ OLECMD* pOleCmd, _Inout_ OLECMDTEXT* pOleText);
    HRESULT onFunctionNameGetList(_In_ vspCommandHandler* pSender, DWORD flags, _In_ VARIANT* pIn, _Out_ VARIANT* pVarOut);
    HRESULT onFunctionNameChanged(_In_ vspCommandHandler* pSender, DWORD flags, _In_ VARIANT* pIn, _Out_ VARIANT* pOut);

    HRESULT onUpdateBuildOptions(const vspCommandHandler& rSender, _Inout_ OLECMD* pOleCmd, _Inout_ OLECMDTEXT* pOleText);
    HRESULT onBuildOptionsChanged(_In_ vspCommandHandler* pSender, DWORD flags, _In_ VARIANT* pIn, _Out_ VARIANT* pOut);

    // the Bitness combo functions:
    HRESULT onUpdateBitnessCombo(const vspCommandHandler& rSender, _Inout_ OLECMD* pOleCmd, _Inout_ OLECMDTEXT* pOleText);
    HRESULT onBitnessComboGetList(_In_ vspCommandHandler* pSender, DWORD flags, _In_ VARIANT* pIn, _Out_ VARIANT* pVarOut);
    HRESULT onBitnessComboChanged(_In_ vspCommandHandler* pSender, DWORD flags, _In_ VARIANT* pIn, _Out_ VARIANT* pOut);

    // Help commands
    void onHelpOpenURL(vspCommandHandler* pSender, DWORD flags, VARIANT* pIn, VARIANT* pOut);
    void OnUpdateSample(vspCommandHandler& handler, OLECMD* pOleCmd, OLECMDTEXT* pOleText);
    void OnHelpOpenTeapotSample(vspCommandHandler* pSender, DWORD flags, VARIANT* pIn, VARIANT* pOut);
    void OnHelpOpenMatMulSample(vspCommandHandler* pSender, DWORD flags, VARIANT* pIn, VARIANT* pOut);
    void OnHelpOpenD3D12MultithreadingSample(vspCommandHandler* pSender, DWORD flags, VARIANT* pIn, VARIANT* pOut);
    void onViewHelp(vspCommandHandler* pSender, DWORD flags, VARIANT* pIn, VARIANT* pOut);
    void onViewQuickStart(vspCommandHandler* pSender, DWORD flags, VARIANT* pIn, VARIANT* pOut);
    void onCheckForUpdates(vspCommandHandler* pSender, DWORD flags, VARIANT* pIn, VARIANT* pOut);
    void onAboutDialog(vspCommandHandler* pSender, DWORD flags, VARIANT* pIn, VARIANT* pOut);

    /// Get the project settings from VS, and update it in the AF, so that the project settings dialog will reflect the user selection
    void UpdateProjectSettingsFromVS(bool& shouldDebug, bool& shouldProfile, bool& shouldFrameAnalyze, bool& isProjectTypeValid);


    // Region: IVscPackageCommandHandler - Begin.

    virtual bool IsSolutionLoaded() const;

    // Region: IVscPackageCommandHandler - End.

};



#endif //__VSPPACKAGECOMMANDHANDLER_H

