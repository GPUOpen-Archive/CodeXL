//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file vspEditorDocument.cpp
///
//==================================================================================

//------------------------------ vspEditorDocument.cpp ------------------------------

#include "stdafx.h"

#include <atlsafe.h>

// Infra:
#include <AMDTBaseTools/Include/AMDTDefinitions.h>

// Local:
#include <src/Package.h>
#include <src/vspCoreAPI.h>
#include <src/vspEditorDocument.h>
#include <src/vspPackageWrapper.h>

vspEditorDocument::vspEditorDocument() : m_bClosed(false), m_filePathStr(nullptr)
{
}


vspEditorDocument::~vspEditorDocument()
{
    VSL_ASSERT(m_bClosed);

    if (!m_bClosed)
    {
        // Paranoid clean-up.  Ignore return value, nothing to do if this fails,
        // and execution should not have arrived here anyway.
        ClosePane();
    }

    // Destroy the core instance.
    VSCORE(vsc_DestroyInstance)(m_pEditorDocumentCoreImpl);
}

STDMETHODIMP vspEditorDocument::CreatePaneWindow(_In_ HWND hWndParent, _In_ int x, _In_ int y, _In_ int cx, _In_ int cy, _Out_ HWND* phWnd)
{
    VSL_STDMETHODTRY
    {
        VSL_CHECKPOINTER(phWnd, E_INVALIDARG);

        // Invoke the core logic.
        VSCORE(vscEditorDocument_CreatePaneWindow)(m_pEditorDocumentCoreImpl, hWndParent, x, y, cx, cy, phWnd);

    } VSL_STDMETHODCATCH()

    return VSL_GET_STDMETHOD_HRESULT();
}

// ---------------------------------------------------------------------------
// Name:        vspEditorDocument::LoadDocData
// Description: This callback is called when VS loads the document data into
//              editor
// Arguments:   _In_ LPCOLESTR pszMkDocument
// Return Val:  STDMETHODIMP
// Author:      Gilad Yarnitzky
// Date:        27/12/2010
// ---------------------------------------------------------------------------
STDMETHODIMP vspEditorDocument::LoadDocData(_In_ LPCOLESTR pszMkDocument)
{
    VSL_STDMETHODTRY
    {
        VSL_CHECKPOINTER(pszMkDocument, E_INVALIDARG);
        CodeXLVSPackage* pPackage = vspPackageWrapper::instance().getPackage();

        if (pPackage != nullptr)
        {
            pPackage->TriggerClockTick();
        }

        VSL_CHECKPOINTER(pszMkDocument, E_INVALIDARG);

        // This is needed since we're not opening the file via the persistence manager:
        SetFileName(pszMkDocument);

        m_filePathStr = SysAllocString(pszMkDocument);

        // Invoke the core logic:
        VSCORE(vscEditorDocument_LoadDocData)(m_pEditorDocumentCoreImpl, pszMkDocument);

        // Set the file path for this session:
        SetEditorCaption(pszMkDocument);

    } VSL_STDMETHODCATCH()

    return VSL_GET_STDMETHOD_HRESULT();
}


// ---------------------------------------------------------------------------
// Name:        vspEditorDocument::SetEditorCaption
// Description: Sets the editor caption, according to the displayed object
// Return Val:  bool - Success / failure.
// Author:      Gilad Yarnitzky
// Date:        27/12/2010
// ---------------------------------------------------------------------------
bool vspEditorDocument::SetEditorCaption(const wchar_t* filePath)
{
    bool retVal = false;

    wchar_t* itemNameStr = NULL;
    // Get the IVsWindowFrame pointer from the editor service provider:
    CComPtr<IVsWindowFrame> spIVsWindowFrame;
    CHKHR(GetVsSiteCache().QueryService(SID_SVsWindowFrame, &spIVsWindowFrame));
    assert(spIVsWindowFrame != NULL);

    if (spIVsWindowFrame != NULL)
    {
        VARIANT caption = {0};
        V_VT(&caption) = VT_BSTR;

        if (filePath != nullptr)
        {
            VSCORE(vscEditorDocument_SetEditorCaption)(m_pEditorDocumentCoreImpl, filePath, itemNameStr);
        }

        // Set the editor caption to an empty string:
        int rcSetProperty = spIVsWindowFrame->SetProperty(VSFPROPID_EditorCaption, caption);
        assert(rcSetProperty == 0);

        // Copy the string to the VARIANT structure:
        V_BSTR(&caption) = SysAllocString(itemNameStr);

        // Release the string.
        VSCORE(vscDeleteWcharString)(itemNameStr);

        // Set the object string as the owner caption:
        rcSetProperty = spIVsWindowFrame->SetProperty(VSFPROPID_OwnerCaption, caption);
        assert(rcSetProperty == 0);

        // Release the caption string memory:
        VariantClear(&caption);
    }

    return retVal;
}

// **DONE**
STDMETHODIMP vspEditorDocument::GetDefaultSize(_Out_ SIZE* psize)
{
    VSL_STDMETHODTRY
    {

        VSL_CHECKPOINTER(psize, E_INVALIDARG);

        psize->cx = 50;
        psize->cy = 50;

    } VSL_STDMETHODCATCH()

    return VSL_GET_STDMETHOD_HRESULT();
}

// **DONE**
STDMETHODIMP vspEditorDocument::ClosePane()
{
    if (m_bClosed)
    {
        // recursion guard
        return E_UNEXPECTED;
    }

    VSL_STDMETHODTRY
    {
        // Notify the persistence base class that the document is closing
        OnDocumentClose();

        // Invoke the core logic.
        VSCORE(vscEditorDocument_ClosePane)(m_pEditorDocumentCoreImpl);

        m_bClosed = true;

        m_pEditorDocumentCoreImpl = nullptr;
    } VSL_STDMETHODCATCH()

    return VSL_GET_STDMETHOD_HRESULT();
}


// ---------------------------------------------------------------------------
// Name:        vspEditorDocument::OnSize
// Description: Is overriding IVsWindowFrameNotify OnSize
// Arguments:   int x, y - the window new position
//              int w, h - the window size
// Return Val:  STDMETHODIMP
// Author:      Gilad Yarnitzky
// Date:        5/1/2011
// ---------------------------------------------------------------------------
// **DONE**
STDMETHODIMP vspEditorDocument::OnSize(int x, int y, int w, int h)
{
    GT_UNREFERENCED_PARAMETER(x);
    GT_UNREFERENCED_PARAMETER(y);
    GT_UNREFERENCED_PARAMETER(w);
    GT_UNREFERENCED_PARAMETER(h);

    return S_OK;
}



// ---------------------------------------------------------------------------
// Name:        vspEditorDocument::OnSize
// Description: Is overriding IVsWindowFrameNotify OnShow
// Arguments:   fShow - is the window shown
// Return Val:  STDMETHODIMP
// Author:      Gilad Yarnitzky
// Date:        5/1/2011
// ---------------------------------------------------------------------------
// **DONE**
STDMETHODIMP vspEditorDocument::OnShow(FRAMESHOW fShow)
{
    HRESULT retVal = S_OK;

    if (fShow == TRUE)
    {
        // Invoke the core logic:
        VSCORE(vscEditorDocument_OnShow)(m_pEditorDocumentCoreImpl);

        // Update all the commands after the focused windows are set:
        CComPtr<IVsUIShell> spUiShell = this->GetVsSiteCache().GetCachedService<IVsUIShell, SID_SVsUIShell>();
        spUiShell->UpdateCommandUI(TRUE);
    }

    return retVal;
}

//////////////////////////////////////////////////////////////////////////
// EditorEvents.inl
//////////////////////////////////////////////////////////////////////////

/*
NOTE - pOleCmd is not ensured against NULL in the OnQuery* methods, as
VSL::CommandHandlerBase::QueryStatus, which is the called of the OnQuery* method implemented
here, already does so.  Additionally, VSL::IOleCommandTargetImpl::QueryStatus checks
pOleCmd as well before calling the query status handler.

The OnQuery* are all private, so the only way to access them is via calling QueryStatus.
*/
// **DONE**
LRESULT vspEditorDocument::OnUserInteractionEvent(int /*wParam*/, _In_ LPNMHDR pHeader, BOOL& /*bHandled*/)
{
    CHKPTR(pHeader, E_FAIL);
    MSGFILTER* pMsgFilter = reinterpret_cast<MSGFILTER*>(pHeader);

    // Non-zero indicates the RTF control should not handle the message
    // Use this as the default and let the default cases set it to 0
    // for events not handled here
    LRESULT iRet = 1;

    switch (pMsgFilter->msg)
    {
        case WM_RBUTTONUP:
        {
            // Convert to screen coordinates
            POINT pt;
            POINTSTOPOINT(pt, MAKEPOINTS(pMsgFilter->lParam));
            // GetControl().ClientToScreen(&pt);
            CHK(pt.x <= SHRT_MAX && pt.x > 0, E_FAIL);
            CHK(pt.y <= SHRT_MAX && pt.y > 0, E_FAIL);

            POINTS ptsTemp = {static_cast<short>(pt.x) , static_cast<short>(pt.y)};

            // Now show the context menu
            CComPtr<IOleComponentUIManager> spIOleComponentUIManager;
            CHKHR(GetVsSiteCache().QueryService(SID_SOleComponentUIManager, &spIOleComponentUIManager));
            CHKHR(spIOleComponentUIManager->ShowContextMenu(OLEROLE_TOPLEVELCOMPONENT,
                                                            CLSID_CodeXLVSPackageCmdSet,
                                                            IDMX_RTF,
                                                            ptsTemp,
                                                            (IOleCommandTarget*) this));
        }
        break;

        case WM_CHAR:
        {

            // CTRL+A/"Select All" is handled separately from the others, since this command does
            // does not modify the content
            if (1 == pMsgFilter->wParam)
            {
                iRet = 0;
                break;
            }

            iRet = 0;

            break;
        }

        default:
            // Return value of 0 indicates that the control should process the event.
            // Since this message isn't handled here, let the control handle it.
            iRet = 0;
            break;
    }


    return iRet;
}

// **DONE**
LRESULT vspEditorDocument::OnSetFocus(UINT /*uMsg*/, _In_ WPARAM /*wParam*/, _In_ LPARAM /*lParam*/, BOOL& bHandled)
{
    // Just hook this message, don't handle it.
    bHandled = FALSE;

    // Update the status bar
    // SetInfo();

    // The results pane is very aggressive in updating the status bar
    // and will update it even after it has lost focus, so set a short
    // timer to update the status bar again, in case this occurs
    //  VSL_CHECKBOOL_GLE(0 != SetTimer(WDELAYSTATUSBARUPDATETIMERID, 100, NULL));

    return 0;  // Ignored, since this is not the final handler of the message
}

// ---------------------------------------------------------------------------
// Name:        vspEditorDocument::onQueryCopyAction
// Description: enables the copy command in the VS edit menu
// Arguments:   const CommandHandler& rSender
//              _Inout_ OLECMD* pOleCmd
//              _Inout_ OLECMDTEXT* pOleText
// Return Val:  void
// Author:      Gilad Yarnitzky
// Date:        7/2/2011
// ---------------------------------------------------------------------------
// **DONE**
void vspEditorDocument::onQueryCopyAction(const CommandHandler& rSender, _Inout_ OLECMD* pOleCmd, _Inout_ OLECMDTEXT* pOleText)
{
    GT_UNREFERENCED_PARAMETER(&rSender);
    GT_UNREFERENCED_PARAMETER(pOleText);

    pOleCmd[0].cmdf = (OLECMDF_SUPPORTED);
    bool canCopy = true;

    // Invoke the core logic:
    VSCORE(vscEditorDocument_OnUpdateEdit_Copy)(m_pEditorDocumentCoreImpl, canCopy);

    if (canCopy)
    {
        pOleCmd[0].cmdf |= (OLECMDF_ENABLED);
    }
}

// ---------------------------------------------------------------------------
// Name:        vspEditorDocument::onQuerySelectAllAction
// Description: enables the select all command in the VS edit menu
// Arguments:   const CommandHandler& rSender
//              _Inout_ OLECMD* pOleCmd
//              _Inout_ OLECMDTEXT* pOleText
// Return Val:  void
// Author:      Gilad Yarnitzky
// Date:        7/2/2011
// ---------------------------------------------------------------------------
// **DONE**
void vspEditorDocument::onQuerySelectAllAction(const CommandHandler& rSender, _Inout_ OLECMD* pOleCmd, _Inout_ OLECMDTEXT* pOleText)
{
    GT_UNREFERENCED_PARAMETER(&rSender);
    GT_UNREFERENCED_PARAMETER(pOleText);

    pOleCmd[0].cmdf = (OLECMDF_SUPPORTED);
    bool canSelectAll = true;

    if (canSelectAll)
    {
        pOleCmd[0].cmdf |= (OLECMDF_ENABLED);
    }
}


// ---------------------------------------------------------------------------
// Name:        vspEditorDocument::onCopyAction
// Description: execute the copy command in the VS edit menu
// Arguments:   _In_ CommandHandler* pSender
//              DWORD flags
//              _In_ VARIANT* pIn
//              _Out_ VARIANT* pOut
// Return Val:  void
// Author:      Gilad Yarnitzky
// Date:        7/2/2011
// ---------------------------------------------------------------------------
// **DONE**
void vspEditorDocument::onCopyAction(_In_ CommandHandler* pSender, DWORD flags, _In_ VARIANT* pIn, _Out_ VARIANT* pOut)
{
    GT_UNREFERENCED_PARAMETER(pSender);
    GT_UNREFERENCED_PARAMETER(flags);
    GT_UNREFERENCED_PARAMETER(pIn);
    GT_UNREFERENCED_PARAMETER(pOut);

    VSCORE(vscEditorDocument_Copy)(m_pEditorDocumentCoreImpl);
}

// ---------------------------------------------------------------------------
// Name:        vspEditorDocument::onSelectAllAction
// Description: execute the select all command in the VS edit menu
// Arguments:   _In_ CommandHandler* pSender
//              DWORD flags
//              _In_ VARIANT* pIn
//              _Out_ VARIANT* pOut
// Return Val:  void
// Author:      Gilad Yarnitzky
// Date:        7/2/2011
// ---------------------------------------------------------------------------
// **DONE**
void vspEditorDocument::onSelectAllAction(_In_ CommandHandler* pSender, DWORD flags, _In_ VARIANT* pIn, _Out_ VARIANT* pOut)
{
    GT_UNREFERENCED_PARAMETER(pSender);
    GT_UNREFERENCED_PARAMETER(flags);
    GT_UNREFERENCED_PARAMETER(pIn);
    GT_UNREFERENCED_PARAMETER(pOut);

    VSCORE(vscEditorDocument_SelectAll)(m_pEditorDocumentCoreImpl);
}

//////////////////////////////////////////////////////////////////////////
// EditorFindAndReplace.inl - no longer needed, we do not support find
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
// EditorPersistance.inl
//////////////////////////////////////////////////////////////////////////

// Called by the IPersistFileFormat::Load implementation on DocumentPersistanceBase
// **DONE**
HRESULT vspEditorDocument::ReadData(
    File& rFile,
    BOOL bInsert,
    DWORD& rdwFormatIndex) throw()
{
    GT_UNREFERENCED_PARAMETER(bInsert);

    VSL_STDMETHODTRY
    {

        // Only the default format is supported by this editor
        rdwFormatIndex = DEF_FORMAT_INDEX;

        {

            //  Control::SuspendDrawAndNotifications suspend(GetControl());

            // Figure out format of file being read by examining the start of
            // the file for the RTF signature.
            const char szRTFSignature[] = "{\\rtf";
            char szHeader[_countof(szRTFSignature)];
            const DWORD dwBytesToRead = _countof(szRTFSignature) - 1; // -1 as the last spot is for the NULL terminator
            DWORD dwBytesRead;
            rFile.Read(szHeader, dwBytesToRead, dwBytesRead);

            // NULL terminate so this is a proper string
            szHeader[_countof(szHeader) - 1] = '\0';

            // If the signature isn't RTF, then assume text
            DWORD dwFormat = SF_TEXT;

            if (dwBytesToRead == dwBytesRead && 0 == ::_strnicmp(szRTFSignature, szHeader, dwBytesToRead))
            {
                dwFormat = SF_RTF;
            }

            // Move back to the beginning of the file
            rFile.Seek(0L, FILE_BEGIN);

            // Now tell the control to load the file
            EDITSTREAM editStream =
            {
                reinterpret_cast<DWORD_PTR>(&rFile),
                S_OK,
                &EditStreamCallback<true>
            };

            // This message will result in EditStreamInCallback being called
            //  GetControl().SendMessage(
            //      EM_STREAMIN,
            //      (bInsert ? dwFormat | SFF_SELECTION : dwFormat),
            //      &editStream);

            VSL_SET_STDMETHOD_HRESULT(*(reinterpret_cast<HRESULT*>(&editStream.dwError)));

        } // Suspend needs to be destroyed here

        if (SUCCEEDED(VSL_GET_STDMETHOD_HRESULT()))
        {
            // Redraw so that the new content is reflected on screen
            //      GetControl().InvalidateRect(NULL, TRUE);
            //      GetControl().UpdateWindow();
            // Update the status bar, since the content is being loaded for the first time
            // SetInfo();
        }

    } VSL_STDMETHODCATCH()

    return VSL_GET_STDMETHOD_HRESULT();
}

// Called indirectly by IPersistFileFormat::Save and IVsFileBackup::BackupFile implementations
// on DocumentPersistanceBase
// **DONE**
void vspEditorDocument::WriteData(File& rFile, DWORD /*dwFormatIndex*/)
{
    // Don't need to check the parameters, the caller ensures they are valid

    EDITSTREAM editStream =
    {
        reinterpret_cast<DWORD_PTR>(&rFile),
        S_OK,
        &EditStreamCallback<false>
    };

    // This message will result in EditStreamCallback being called
    //  GetControl().SendMessage(EM_STREAMOUT, SF_RTF, &editStream);

    CHKHR(*(reinterpret_cast<HRESULT*>(&editStream.dwError)));
}

// Called by the Rich Edit control during the processing of EM_STREAMOUT and EM_STREAMIN
template <bool bRead_T> // method template
DWORD CALLBACK vspEditorDocument::EditStreamCallback(
    _In_ DWORD_PTR dwpFile,
    _Inout_bytecap_(iBufferByteSize) LPBYTE pBuffer,
    LONG iBufferByteSize,
    _Out_ LONG* piBytesWritenOrRead)
{
    DWORD dwBytesWritten = 0;

    DWORD dwBufferByteSize = iBufferByteSize;

    VSL_STDMETHODTRY
    {

        if (NULL != pBuffer &&
        0 != dwBufferByteSize &&
        LONG_MAX >= dwBufferByteSize &&
        NULL != piBytesWritenOrRead)
        {
            File* pFile = reinterpret_cast<File*>(dwpFile);

            if (NULL != pFile)
            {
                if (bRead_T)
                {
                    pFile->Read(pBuffer, dwBufferByteSize, dwBytesWritten);
                }
                else
                {
                    pFile->Write(pBuffer, dwBufferByteSize, &dwBytesWritten);
                }

                VSL_ASSERT(dwBytesWritten <= iBufferByteSize);
            }

            if (dwBytesWritten <= dwBufferByteSize)
            {
                *piBytesWritenOrRead = dwBytesWritten;
            }
        }

    } VSL_STDMETHODCATCH()

    HRESULT hr = (dwBytesWritten >= 0 && (bRead_T ? true : dwBytesWritten == dwBufferByteSize)) ?
                 VSL_GET_STDMETHOD_HRESULT() :
                 (FAILED(VSL_GET_STDMETHOD_HRESULT()) ? VSL_GET_STDMETHOD_HRESULT() : E_FAIL);

    // If the return value is non-Zero (i.e. not S_OK), the return value will
    // be put into the dwError member of the EDITSTREAM instance passed with
    // the EM_STREAMOUT message in WriteData
    return *(reinterpret_cast<DWORD*>(&hr));
}

// Called by VSL::DocumentPersistanceBase::InitNew and VSL::DocumentPersistanceBase::Save
// **DONE**
bool vspEditorDocument::IsValidFormat(DWORD dwFormatIndex)
{
    // Only one format, the default, is supported
    return DEF_FORMAT_INDEX == dwFormatIndex;
}

// Called by VSL::DocumentPersistanceBase::FilesChanged
// **DONE**
void vspEditorDocument::OnFileChangedSetTimer()
{
    // 500 milliseconds is an arbitrary time to delay
    // See vspEditorDocument::OnTimer and
    // VSL::DocumentPersistanceBase::FilesChanged for details
    //VSL_CHECKBOOL_GLE(0 != SetTimer(WFILECHANGEDTIMERID, 500, NULL));
}

// Called by VSL::DocumentPersistanceBase::GetClassID, which is also called by
// VSL::DocumentPersistanceBase::GetGuidEditorType)
// **DONE**
const GUID& vspEditorDocument::GetEditorTypeGuid() const
{
    // The GUID for the factory is the one to return from IPersist::GetClassID and
    // IVsPersistDocData::GetGuidEditorType
    return CLSID_CodeXLVSPackageKernelAnalyzerEditorFactory;
}

// Called by VSL::DocumentPersistanceBase::GetFormatList
// **DONE**
void vspEditorDocument::GetFormatListString(ATL::CStringW& rstrFormatList)
{
    // Load the file format list string from the resource DLL
    VSL_CHECKBOOL_GLE(rstrFormatList.LoadString(IDS_FORMATSTR));
}

// Called indirectly by VSL::DocumentPersistanceBase::Load and VSL::DocumentPersistanceBase::Save
// **DONE**
void vspEditorDocument::PostSetDirty()
{
    // Notify the Rich Edit control of the current dirty state
    //GetControl().SetModified(IsFileDirty());
}

void vspEditorDocument::onQueryCommandUpdate(const CommandHandler& rSender, _Inout_ OLECMD* pOleCmd, _Inout_ OLECMDTEXT* pOleText)
{
    GetCommandUpdate(rSender, pOleCmd, pOleText);
}

void vspEditorDocument::onCommandAction(_In_ CommandHandler* pSender, DWORD flags, _In_ VARIANT* pIn, _Out_ VARIANT* pOut)
{
    ExecuteCommandAction(pSender, flags, pIn, pOut);
}

void vspEditorDocument::GetCommandUpdate(const CommandHandler& rSender, _Inout_ OLECMD* pOleCmd, _Inout_ OLECMDTEXT* pOleText)
{
    // by default the commands are disabled. A derived document can inherit the function and change behavior
    GT_UNREFERENCED_PARAMETER(rSender);
    GT_UNREFERENCED_PARAMETER(pOleText);
    pOleCmd[0].cmdf = OLECMDSTATE_DISABLED;
}

void vspEditorDocument::ExecuteCommandAction(_In_ CommandHandler* pSender, DWORD flags, _In_ VARIANT* pIn, _Out_ VARIANT* pOut)
{
    // by default the commands does nothing. A derived document can inherit the function and change behavior
    GT_UNREFERENCED_PARAMETER(pSender);
    GT_UNREFERENCED_PARAMETER(flags);
    GT_UNREFERENCED_PARAMETER(pIn);
    GT_UNREFERENCED_PARAMETER(pOut);
}