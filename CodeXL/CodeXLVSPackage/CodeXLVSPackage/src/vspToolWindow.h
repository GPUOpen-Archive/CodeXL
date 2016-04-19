//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file vspToolWindow.h
///
//==================================================================================

//
// MyToolWindow.h
//
// This file contains the implementation of a tool window that hosts a .NET user control
//

#pragma once

#include <AtlWin.h>
#include <VSLWindows.h>
#include "guids.h"
#include <../CodeXLVSPackageUI/Resource.h>

// Forward declaration:
class vspWindowPaneImpl;
class vspWXWindowPaneImpl;

class vspWindowPane :
    public CComObjectRootEx<CComSingleThreadModel>,
    public VsWindowPaneFromResource<vspWindowPane, IDD_CodeXLVSPackage_DLG>,
    public VsWindowFrameEventSink<vspWindowPane>,
// IVsFindTarget, IVsTextImage, and IVsTextSpanSet are required for find and replace to work correctly
    public IVsFindTarget,
    public IVsTextImageImpl<>,
    public IVsTextSpanSetImpl,

    public VSL::ISupportErrorInfoImpl <
    InterfaceSupportsErrorInfoList<IVsWindowPane,
    InterfaceSupportsErrorInfoList<IVsWindowFrameNotify,
    InterfaceSupportsErrorInfoList<IVsWindowFrameNotify3 > > > >,
    public IOleCommandTargetImpl<vspWindowPane>
{
    VSL_DECLARE_NOT_COPYABLE(vspWindowPane)

public:
    void setWindowCommandID(int winCommandID) {_gdWindowCommandID = winCommandID;};

    void SetToolShowFunction(void* pVspToolWindow, void* pShowFunction);
protected:

    // Protected constructor called by CComObject<vspWindowPane>::CreateInstance.
    vspWindowPane();

    ~vspWindowPane();


public:

    BEGIN_COM_MAP(vspWindowPane)
    COM_INTERFACE_ENTRY(IVsWindowPane)
    COM_INTERFACE_ENTRY(IVsWindowFrameNotify)
    COM_INTERFACE_ENTRY(IVsWindowFrameNotify3)
    COM_INTERFACE_ENTRY(ISupportErrorInfo)
    COM_INTERFACE_ENTRY(IOleCommandTarget)
    COM_INTERFACE_ENTRY(IVsFindTarget)
    COM_INTERFACE_ENTRY(IVsTextImage)
    COM_INTERFACE_ENTRY(IVsTextSpanSet)
    END_COM_MAP()


    VSL_BEGIN_COMMAND_MAP()
    VSL_COMMAND_MAP_ENTRY(guidVSStd97, cmdidCopy, CommandHandler::QueryStatusHandler(&onUpdateEditCommand), CommandHandler::ExecHandler(&onExecuteEditCommand))
    VSL_COMMAND_MAP_ENTRY(guidVSStd97, cmdidSelectAll, CommandHandler::QueryStatusHandler(&onUpdateEditCommand), CommandHandler::ExecHandler(&onExecuteEditCommand))
    VSL_END_VSCOMMAND_MAP()

    // Update & execute exit commands:
    void onUpdateEditCommand(CommandHandler& handler, OLECMD* pOleCmd, OLECMDTEXT* pOleText);
    void onExecuteEditCommand(CommandHandler* pSender, DWORD flags, VARIANT* pIn, VARIANT* pOut);

    STDMETHOD(CreatePaneWindow)(HWND hwndParent, int x, int y, int cx, int cy, HWND* phwnd);

    // Function called by VsVsWindowPaneFromResource at the end of SetSite; at this point the
    // window pane is constructed and sited and can be used, so this is where we can initialize
    // the event sink by siting it:
    void PostSited(IVsPackageEnums::SetSiteResult /*result*/);

    // Callback function called by ToolWindowBase when the size of the window changes:
    void OnFrameSize(int x, int y, int w, int h);

    // Callback function called by ToolWindowBase when the frame is closed:
    void OnFrameShow(FRAMESHOW fShow);

    // IVsFindTarget overrides:
    virtual HRESULT STDMETHODCALLTYPE GetCapabilities(__RPC__out BOOL* pfImage, __RPC__out VSFINDOPTIONS* pgrfOptions);
    virtual HRESULT STDMETHODCALLTYPE GetProperty(VSFTPROPID propid, __RPC__out VARIANT* pvar);
    virtual HRESULT STDMETHODCALLTYPE GetSearchImage(VSFINDOPTIONS grfOptions, __RPC__deref_out_opt IVsTextSpanSet** ppSpans, __RPC__deref_out_opt IVsTextImage** ppTextImage);
    virtual HRESULT STDMETHODCALLTYPE Find(__RPC__in LPCOLESTR pszSearch, VSFINDOPTIONS grfOptions, BOOL fResetStartPoint, __RPC__in_opt IVsFindHelper* pHelper, __RPC__out VSFINDRESULT* pResult);
    virtual HRESULT STDMETHODCALLTYPE Replace(__RPC__in LPCOLESTR pszSearch, __RPC__in LPCOLESTR pszReplace, VSFINDOPTIONS grfOptions, BOOL fResetStartPoint, __RPC__in_opt IVsFindHelper* pHelper, __RPC__out BOOL* pfReplaced);
    virtual HRESULT STDMETHODCALLTYPE GetMatchRect(__RPC__out PRECT prc);
    virtual HRESULT STDMETHODCALLTYPE NavigateTo(__RPC__in const TextSpan* pts);
    virtual HRESULT STDMETHODCALLTYPE GetCurrentSpan(__RPC__out TextSpan* pts);
    virtual HRESULT STDMETHODCALLTYPE SetFindState(__RPC__in_opt IUnknown* punk);
    virtual HRESULT STDMETHODCALLTYPE GetFindState(__RPC__deref_out_opt IUnknown** ppunk);
    virtual HRESULT STDMETHODCALLTYPE NotifyFindTarget(VSFTNOTIFY notification);
    virtual HRESULT STDMETHODCALLTYPE MarkSpan(__RPC__in const TextSpan* pts);

    // IVsTextImageImpl overrides:
    STDMETHOD(GetLineSize)(_Out_ LONG* pcLines);
    STDMETHOD(Replace)(DWORD dwFlags, const TextSpan* pts, LONG cch, LPCOLESTR pchText, _Out_ TextSpan* ptsChanged);
    STDMETHOD(GetSpanLength)(const TextSpan* pts, _Out_ LONG* pcch);
    STDMETHOD(GetLineLength)(LONG iLine, _Out_ LONG* piLength);
    STDMETHOD(GetLine)(DWORD grfGet, LONG iLine, LONG iStartIndex, LONG iEndIndex, _Out_ LINEDATAEX* pLineData);

private:

    bool createQTPaneWindow();

    // A handle to the core implementation object.
    void* m_pCoreImpl;

    // Contain the create window command id:
    int _gdWindowCommandID;

    // Find state is an opaque object held on behalf of the find engine.
    IUnknown* m_pUnkFindState;

    // Contain the last line that the text was found in the current find session:
    int _lastFoundTextLine;

};

static void vscShowWindow(void* pWindowAsVoid)
{
    ((IVsWindowFrame*)pWindowAsVoid)->Show();
}

class vspToolWindow :
    public VSL::ToolWindowBase<vspToolWindow >
{
public:
    // Constructor of the tool window object.
    // The goal of this constructor is to initialize the base class with the site cache
    // of the owner package.
    vspToolWindow(const PackageVsSiteCache& rPackageVsSiteCache):
        ToolWindowBase(rPackageVsSiteCache), _myWindowCommandID(-1), m_spView(NULL) {}

    vspToolWindow::~vspToolWindow()
    {

    }

    // Caption of the tool window.
    const wchar_t* const GetCaption() const;
    bool setCaption(const std::wstring& caption);

    // Creation flags for this tool window.
    VSCREATETOOLWIN GetCreationFlags() const
    {
        return CTW_fInitNew | CTW_fForceCreate;
    }

    // Return the GUID of the persistent slot for this tool window.
    const GUID& GetToolWindowGuid() const;

    IUnknown* GetViewObject();

    void setMyWindowCommandID(int windowCommandID);

    // This method is called by the base class after the tool window is created.
    // We use it to set the icon for this window.
    void PostCreate();

private:
    int iconIndexFromWindowCommandID(int windowCommandID);

private:
    CComPtr<IUnknown> m_spView;

    int _myWindowCommandID;

};
