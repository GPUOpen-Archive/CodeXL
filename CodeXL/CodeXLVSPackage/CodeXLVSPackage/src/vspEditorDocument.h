//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file vspEditorDocument.h
///
//==================================================================================

//------------------------------ vspEditorDocument.h ------------------------------

#ifndef __VSPEDITORDOCUMENT_H
#define __VSPEDITORDOCUMENT_H

#include "StdAfx.h"

// General
#include <cassert>

// Windows:
#include <WinBase.h>
#include <WinDef.h>
#include <WinUser.h>

// Local:
#include <../CodeXLVSPackageUI/CommandIds.h>
#include <src/vspCoreAPI.h>
#include <src/vspToolWindow.h>

/// ----------------------------------------------------------------------------------
/// Class Name:    vspEditorDocument
///                This class is used to implement the infrastructure for MDI windows created by CodeXL
///                VS package. The class implements some of the functions
///                needed by our MDI windows.
///                vspEditorDocument inherits from the following VS classes:
///                CComObjectRootEx<CComSingleThreadModel>: Use ATL to take care of common COM infrastructure
///                IVsWindowPaneImpl<vspEditorDocument>: is required to be a document
///                IOleCommandTargetImpl<vspEditorDocument>: IOleCommandTarget is required to be a document that handles commands (as any editor will need to do)
///                DocumentPersistanceBase<vspEditorDocument, VSL::File>:
///                                   DocumentPersistanceBase provides the persistence related interfaces: IVsPersistDocData, IVsDocDataFileChangeContro, IVsFileChangeEvents, IPersistFileFormat, and IVsFileBackup.
///                IVsWindowFrameNotify, public IVsWindowFrameNotify3: Needed to catch resize events:
/// ----------------------------------------------------------------------------------
class vspEditorDocument :

    public CComObjectRootEx<CComSingleThreadModel>,
    public IVsWindowPaneImpl<vspEditorDocument>,
    public IOleCommandTargetImpl<vspEditorDocument >,
    public DocumentPersistanceBase<vspEditorDocument, VSL::File>,
    public IVsWindowFrameNotify, public IVsWindowFrameNotify3
{
    // COM objects typically should not be cloned, and this prevents cloning by declaring the
    // copy constructor and assignment operator private (NOTE:  this macro includes the deceleration of
    // a private section, so everything following this macro and preceding a public or protected
    // section will be private).
    VSL_DECLARE_NOT_COPYABLE(vspEditorDocument)

public:
    typedef VSL::File File;

    // Provides a portion of the implementation of IUnknown, in particular the list of interfaces
    // the EditorDocument object will support via QueryInterface
    BEGIN_COM_MAP(vspEditorDocument)
    COM_INTERFACE_ENTRY(IVsWindowPane)
    COM_INTERFACE_ENTRY(IOleCommandTarget)
    COM_INTERFACE_ENTRY(IPersistFileFormat)
    COM_INTERFACE_ENTRY(IPersist)
    COM_INTERFACE_ENTRY(IVsPersistDocData)
    COM_INTERFACE_ENTRY(IVsFileChangeEvents)
    COM_INTERFACE_ENTRY(IVsDocDataFileChangeControl)
    COM_INTERFACE_ENTRY(IVsFileBackup)
    COM_INTERFACE_ENTRY(IVsWindowFrameNotify)
    COM_INTERFACE_ENTRY(IVsWindowFrameNotify3)
    END_COM_MAP()

    // Defines the command handlers. IOleCommandTargetImpl will use these handlers to implement
    // IOleCommandTarget.
    VSL_BEGIN_COMMAND_MAP()

    // Every command is identified by the shell using a GUID/DWORD pair, so every the definition of
    // commands must contain this information.

    // The following command map entries define a GUID/DWORD pair to identify the command and a
    // callback for the command execution and status queries.
    VSL_COMMAND_MAP_ENTRY(CMDSETID_StandardCommandSet97, cmdidCopy, &onQueryCopyAction, &onCopyAction)
    VSL_COMMAND_MAP_ENTRY(CMDSETID_StandardCommandSet97, cmdidSelectAll, &onQuerySelectAllAction, &onSelectAllAction)

    VSL_COMMAND_MAP_ENTRY(CLSID_CodeXLVSPackageCmdSet, commandIDBestFit, &onQueryCommandUpdate, &onCommandAction)
    VSL_COMMAND_MAP_ENTRY(CLSID_CodeXLVSPackageCmdSet, commandIDOrigSize, &onQueryCommandUpdate, &onCommandAction)
    VSL_COMMAND_MAP_ENTRY(CLSID_CodeXLVSPackageCmdSet, commandIDPan, &onQueryCommandUpdate, &onCommandAction)
    VSL_COMMAND_MAP_ENTRY(CLSID_CodeXLVSPackageCmdSet, commandIDSelect, &onQueryCommandUpdate, &onCommandAction)
    VSL_COMMAND_MAP_ENTRY(CLSID_CodeXLVSPackageCmdSet, commandIDRotateLeft, &onQueryCommandUpdate, &onCommandAction)
    VSL_COMMAND_MAP_ENTRY(CLSID_CodeXLVSPackageCmdSet, commandIDRotateRight, &onQueryCommandUpdate, &onCommandAction)
    VSL_COMMAND_MAP_ENTRY(CLSID_CodeXLVSPackageCmdSet, commandIDChannelRed, &onQueryCommandUpdate, &onCommandAction)
    VSL_COMMAND_MAP_ENTRY(CLSID_CodeXLVSPackageCmdSet, commandIDChannelGreen, &onQueryCommandUpdate, &onCommandAction)
    VSL_COMMAND_MAP_ENTRY(CLSID_CodeXLVSPackageCmdSet, commandIDChannelBlue, &onQueryCommandUpdate, &onCommandAction)
    VSL_COMMAND_MAP_ENTRY(CLSID_CodeXLVSPackageCmdSet, commandIDChannelAlpha, &onQueryCommandUpdate, &onCommandAction)
    VSL_COMMAND_MAP_ENTRY(CLSID_CodeXLVSPackageCmdSet, commandIDChannelInvert, &onQueryCommandUpdate, &onCommandAction)
    VSL_COMMAND_MAP_ENTRY(CLSID_CodeXLVSPackageCmdSet, commandIDChannelGrayscale, &onQueryCommandUpdate, &onCommandAction)

    VSL_COMMAND_MAP_ENTRY(CLSID_CodeXLVSPackageCmdSet, commandIDImageSizeComboGetList, &onQueryCommandUpdate, &onCommandAction)
    VSL_COMMAND_MAP_ENTRY(CLSID_CodeXLVSPackageCmdSet, commandIDImageSizeCombo, &onQueryCommandUpdate, &onCommandAction)
    VSL_COMMAND_MAP_ENTRY(CMDSETID_StandardCommandSet97, cmdidZoomIn, &onQueryCommandUpdate, &onCommandAction)
    VSL_COMMAND_MAP_ENTRY(CMDSETID_StandardCommandSet97, cmdidZoomOut, &onQueryCommandUpdate, &onCommandAction)

    // Terminate the definition of the command map
    VSL_END_VSCOMMAND_MAP()

    VSL_BEGIN_MSG_MAP(vspEditorDocument)
    // On this event the context menu will be shown if needed, and some keyboard commands will be
    // dealt with.
    MESSAGE_HANDLER(WM_SETFOCUS, OnSetFocus)
    VSL_END_MSG_MAP()

    // IVsWindowPane methods overridden or not provided by IVsWindowPaneImpl

    STDMETHOD(CreatePaneWindow)(_In_ HWND hwndParent, _In_ int x, _In_ int y, _In_ int cx, _In_ int cy, _Out_ HWND* hwnd);

    // Override IPersistDocData
    STDMETHOD(LoadDocData)(LPCOLESTR pszMkDocument);

    STDMETHOD(GetDefaultSize)(
        /*[out]*/ _Out_ SIZE* psize);
    STDMETHOD(ClosePane)();

    // IVsWindowFrameNotify, IVsWindowFrameNotify3
    STDMETHOD(OnShow)(FRAMESHOW fShow);
    STDMETHOD(OnMove)(void) {return E_NOTIMPL;}
    STDMETHOD(OnSize)(void) {return E_NOTIMPL;}
    STDMETHOD(OnDockableChange)(BOOL fDockable)
    {
        UNREFERENCED_PARAMETER(fDockable);
        return E_NOTIMPL;
    }

    STDMETHOD(OnMove)(int x, int y, int w, int h) {return OnSize(x, y, w, h);}
    STDMETHOD(OnSize)(int x, int y, int w, int h);
    STDMETHOD(OnDockableChange)(BOOL fDockable, int x, int y, int w, int h)
    {
        UNREFERENCED_PARAMETER(fDockable);
        return OnSize(x, y, w, h);
    }
    STDMETHOD(OnClose)(FRAMECLOSE* pgrfSaveOptions)
    {
        UNREFERENCED_PARAMETER(pgrfSaveOptions);
        return S_OK;
    }

    // VSL base class statically bound call back methods

    // Called by IExtensibleObjectImpl::GetAutomationObject
    IDispatch* GetNamedAutomationObject(_In_z_ BSTR bstrName);

    // Called by VSL::DocumentPersistanceBase::InitNew and VSL::DocumentPersistanceBase::Save
    bool IsValidFormat(DWORD dwFormatIndex);

    // Called by VSL::DocumentPersistanceBase::FilesChanged
    void OnFileChangedSetTimer();

    // Called by VSL::DocumentPersistanceBase::GetClassID, which is also called by
    // VSL::DocumentPersistanceBase::GetGuidEditorType)
    const GUID& GetEditorTypeGuid() const;

    // Called by VSL::DocumentPersistanceBase::GetFormatList
    void GetFormatListString(ATL::CStringW& rstrFormatList);

    // Called indirectly by VSL::DocumentPersistanceBase::Load and VSL::DocumentPersistanceBase::Save
    void PostSetDirty();

    void PostSetReadOnly() {}

    // Called by the IPersistFileFormat::Load implementation on DocumentPersistanceBase
    STDMETHOD(ReadData)(File& rFile, BOOL bInsert, DWORD& rdwFormatIndex);

    // Called indirectly by IPersistFileFormat::Save and IVsFileBackup::BackupFile implementations
    // on DocumentPersistanceBase
    void WriteData(File& rFile, DWORD dwFormatIndex);

    // Called by VSL::IVsFindTargetImpl::GetCapabilities and
    // VSL::IVsFindTargetImpl::GetSearchImage
    DWORD GetCapabilityOptions();

protected:
    typedef IVsWindowPaneImpl<vspEditorDocument>::VsSiteCache VsSiteCache;

    typedef vspEditorDocument This;

    vspEditorDocument();

    ~vspEditorDocument();

    // Called by the Rich Edit control during the processing of EM_STREAMOUT and EM_STREAMIN
    template <bool bRead_T>
    static DWORD CALLBACK EditStreamCallback(
        _In_ DWORD_PTR dwpFile,
        _Inout_bytecap_(iBufferByteSize) LPBYTE pBuffer,
        LONG iBufferByteSize,
        _Out_ LONG* piBytesWritten);

    // Window Proc for the rich edit control.  Necessary to implement macro recording.
    static LRESULT CALLBACK RichEditWindowProc(
        _In_ HWND hWnd,
        UINT msg,
        _In_ WPARAM wParam,
        _In_ LPARAM lParam);

#pragma warning(push)
#pragma warning(disable : 4480) // // warning C4480: nonstandard extension used: specifying underlying type for enum
    enum TimerID : WPARAM
    {
        // ID of timer message sent from OnFileChangedSetTimer
        WFILECHANGEDTIMERID = 1,
        // ID of timer message sent from OnSetFocus
        WDELAYSTATUSBARUPDATETIMERID = 2,
    };
#pragma warning(pop)

protected:

    // A handle to the core implementation instance.
    void* m_pEditorDocumentCoreImpl;

    BSTR m_filePathStr;

    // Windows message handlers
    LRESULT OnSelectionChange(int /*wParam*/, _In_ LPNMHDR /*pHeader*/, BOOL& /*bHandled*/);
    LRESULT OnUserInteractionEvent(int /*wParam*/, _In_ LPNMHDR pHeader, BOOL& /*bHandled*/);
    LRESULT OnSetFocus(UINT /*uMsg*/, _In_ WPARAM wParam, _In_ LPARAM /*lParam*/, BOOL& bHandled);

    // Copy & select all commands
    void onQueryCopyAction(const CommandHandler& rSender, _Inout_ OLECMD* pOleCmd, _Inout_ OLECMDTEXT* pOleText);
    void onQuerySelectAllAction(const CommandHandler& rSender, _Inout_ OLECMD* pOleCmd, _Inout_ OLECMDTEXT* pOleText);
    void onCopyAction(_In_ CommandHandler* pSender, DWORD flags, _In_ VARIANT* pIn, _Out_ VARIANT* pOut);
    void onSelectAllAction(_In_ CommandHandler* pSender, DWORD flags, _In_ VARIANT* pIn, _Out_ VARIANT* pOut);

    void onQueryCommandUpdate(const CommandHandler& rSender, _Inout_ OLECMD* pOleCmd, _Inout_ OLECMDTEXT* pOleText);
    void onCommandAction(_In_ CommandHandler* pSender, DWORD flags, _In_ VARIANT* pIn, _Out_ VARIANT* pOut);

    virtual void GetCommandUpdate(const CommandHandler& rSender, _Inout_ OLECMD* pOleCmd, _Inout_ OLECMDTEXT* pOleText);
    virtual void ExecuteCommandAction(_In_ CommandHandler* pSender, DWORD flags, _In_ VARIANT* pIn, _Out_ VARIANT* pOut);

    // Utilities:
    bool SetEditorCaption(const wchar_t* filePath);

    // IVsWindowPane Data
    bool m_bClosed;
};

#endif //__VSPEDITORDOCUMENT_H

