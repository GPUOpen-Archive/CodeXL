//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file vspToolWindow.cpp
///
//==================================================================================
#include "stdafx.h"
#include "initguid.h"

// C++:
#include <cassert>
#include <string>
#include <sstream>

// Local:
#include <../CodeXLVSPackageUI/CommandIds.h>
#include <CodeXLVSPackage/Include/vspCommandIDs.h>
#include <src/vspCoreAPI.h>
#include <CodeXLVSPackage/Include/vspStringConstants.h>
#include <src/vspToolWindow.h>
#include <src/vspUtils.h>

// ---------------------------------------------------------------------------
// Name:        vspWindowPane::vspWindowPane
// Description: Constructor
// Author:      Sigal Algranaty
// Date:        18/5/2011
// ---------------------------------------------------------------------------
vspWindowPane::vspWindowPane()
    : _gdWindowCommandID(-1), m_pUnkFindState(NULL), _lastFoundTextLine(-1), VsWindowPaneFromResource(), m_pCoreImpl(VSCORE(vscToolWindow__CreateInstance)())
{
    assert(m_pCoreImpl != NULL);
}

// ---------------------------------------------------------------------------
// Name:        vspWindowPane::~vspWindowPane
// Description: Destructor
// Author:      Sigal Algranaty
// Date:        18/5/2011
// ---------------------------------------------------------------------------
vspWindowPane::~vspWindowPane()
{
    if (m_pUnkFindState != NULL)
    {
        m_pUnkFindState->Release();
        m_pUnkFindState = NULL;
    }

    // Destroy the core implementation object.
    VSCORE(vscToolWindow__DestroyInstance)(m_pCoreImpl);
}


// ---------------------------------------------------------------------------
// Name:        vspWindowPane::CreatePaneWindow
// Description: The function is called when the window pane is created.
//              In this function we 'steal' the window handle, and create a WX
//              pane with it
// Arguments:   HWND hwndParent - the parent window HWND handle
//              int x, y, cx, cy - the window position
//              HWND *phWND - output - the create window handle
// Return Val:  HRESULT
// Author:      Sigal Algranaty
// Date:        6/9/2010
// ---------------------------------------------------------------------------
HRESULT vspWindowPane::CreatePaneWindow(HWND hwndParent, int x, int y, int cx, int cy, HWND* phWND)
{
    VSL_STDMETHODTRY
    {
        VSL_CHECKPOINTER_DEFAULT(phWND);
        * phWND = NULL;

        // Invoke core logic.
        VSCORE(vscToolWindow_CreatePaneWindow)(m_pCoreImpl, hwndParent, x, y, cx, cy, phWND, _gdWindowCommandID);
    }
    VSL_STDMETHODCATCH()

    return VSL_GET_STDMETHOD_HRESULT();
}

// ---------------------------------------------------------------------------
// Name:        vspWindowPane::onUpdateEditCommand
// Description: Update function for VS edit commands
// Arguments:   CommandHandler& handler
//              OLECMD* pOleCmd
//              OLECMDTEXT* pOleText
// Return Val:  void
// Author:      Sigal Algranaty
// Date:        21/4/2011
// ---------------------------------------------------------------------------
void vspWindowPane::onUpdateEditCommand(CommandHandler& handler, OLECMD* pOleCmd, OLECMDTEXT* pOleText)
{
    GT_UNREFERENCED_PARAMETER(&handler);
    GT_UNREFERENCED_PARAMETER(pOleText);

    bool isEnabled = false;
    bool isFoundCommandHandler = false;

    assert(pOleCmd != NULL);

    if (pOleCmd != NULL)
    {
        int cmdId = pOleCmd->cmdID;

        // Invoke the core logic.
        VSCORE(vscToolWindow_OnUpdateEditCommand)(m_pCoreImpl, isEnabled, isFoundCommandHandler, cmdId);
        assert(isFoundCommandHandler);

        if (isFoundCommandHandler)
        {
            pOleCmd->cmdf = (OLECMDF_SUPPORTED);

            if (isEnabled)
            {
                pOleCmd->cmdf |= (OLECMDF_ENABLED);
            }
        }
    }
}


// ---------------------------------------------------------------------------
// Name:        vspWindowPane::onExecuteEditCommand
// Description: Command handler for VS edit commands
// Arguments:   CommandHandler* pSender
//              DWORD flags
//              VARIANT* pIn
//              VARIANT* pOut
// Return Val:  void
// Author:      Sigal Algranaty
// Date:        21/4/2011
// ---------------------------------------------------------------------------
void vspWindowPane::onExecuteEditCommand(CommandHandler* pSender, DWORD flags, VARIANT* pIn, VARIANT* pOut)
{
    GT_UNREFERENCED_PARAMETER(flags);
    GT_UNREFERENCED_PARAMETER(pIn);
    GT_UNREFERENCED_PARAMETER(pOut);

    int id = pSender->GetId().GetId();

    // Invoke core logic.
    VSCORE(vscToolWindow_OnExecuteEditCommand)(m_pCoreImpl, id);
}

// ---------------------------------------------------------------------------
// Name:        vspWindowPane::PostSited
// Description: Function called by VsVsWindowPaneFromResource at the end of SetSite; at this point the
//              window pane is constructed and sited and can be used, so this is where we can initialize
//              the event sink by siting it.
// Arguments:   IVsPackageEnums::SetSiteResult /*result*/
// Return Val:  void
// Author:      Sigal Algranaty
// Date:        21/9/2010
// ---------------------------------------------------------------------------
void vspWindowPane::PostSited(IVsPackageEnums::SetSiteResult /*result*/)
{
    VsWindowFrameEventSink<vspWindowPane>::SetSite(GetVsSiteCache());
}

// ---------------------------------------------------------------------------
// Name:        vspWindowPane::OnFrameSize
// Description: Callback function called by ToolWindowBase when the size of the window changes.
//              The function is setting the embedded WX window' size and position
// Arguments:   int x, y, w, h - the pane position and size
// Author:      Sigal Algranaty
// Date:        21/9/2010
// ---------------------------------------------------------------------------
void vspWindowPane::OnFrameSize(int x, int y, int w, int h)
{
    GT_UNREFERENCED_PARAMETER(x);
    GT_UNREFERENCED_PARAMETER(y);
    GT_UNREFERENCED_PARAMETER(w);
    GT_UNREFERENCED_PARAMETER(h);

    //if (_pImpl != NULL)
    //{
    // Update the window size for the requested window (call the relevant GD class size event):
    //        bool rcUpdateSize = vspWindowsManager::instance().updateViewSize(_gdWindowCommandID, QSize(w, h));
    //        GT_ASSERT(rcUpdateSize);
    //}
}

// ---------------------------------------------------------------------------
// Name:        vspWindowPane::OnFrameClose
// Description: Callback function called by ToolWindowBase when the frame is shown / hidden
// Return Val:  void
// Author:      Sigal Algranaty
// Date:        18/1/2011
// ---------------------------------------------------------------------------
void vspWindowPane::OnFrameShow(FRAMESHOW fShow)
{
    // Memory should be updated on explorer tree only when the statistics view is open:
    bool isFrameShown = ((fShow != FRAMESHOW_Hidden) && (fShow != FRAMESHOW_WinHidden) && (fShow != FRAMESHOW_WinClosed));
    VSCORE(vscToolWindow_OnFrameShow)(m_pCoreImpl, isFrameShown, _gdWindowCommandID);
}

// ---------------------------------------------------------------------------
// Name:        vspWindowPane::createQTPaneWindow
// Description: Create a QT window, using the implementation widget parent
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        1/12/2011
// ---------------------------------------------------------------------------
bool vspWindowPane::createQTPaneWindow()
{
    // Invoke core logic.
    return VSCORE(vscToolWindow_CreateQTPaneWindow)(m_pCoreImpl, _gdWindowCommandID);
}

// ---------------------------------------------------------------------------
// Name:        vspWindowPane::GetCapabilities
// Description: Return the capabilities for this view as a find target object
// Arguments:   __RPC__out BOOL *pfImage
//              __RPC__out VSFINDOPTIONS *pgrfOptions
// Return Val:  HRESULT STDMETHODCALLTYPE
// Author:      Sigal Algranaty
// Date:        17/5/2011
// ---------------------------------------------------------------------------
HRESULT STDMETHODCALLTYPE vspWindowPane::GetCapabilities(__RPC__out BOOL* pfImage, __RPC__out VSFINDOPTIONS* pgrfOptions)
{
    if (pfImage != NULL)
    {
        pfImage[0] = false;
    }

    if (pgrfOptions != NULL)
    {
        *pgrfOptions = FR_None;
        *pgrfOptions |= FR_Plain;
        *pgrfOptions |= FR_MatchCase;
        *pgrfOptions |= FR_WholeWord;
        *pgrfOptions |= FR_Find;
        *pgrfOptions |= FR_Document;
        *pgrfOptions |= FR_Backwards;
        *pgrfOptions |= FR_ResetPosition;

    }

    return S_OK;
}

// ---------------------------------------------------------------------------
// Name:        vspWindowPane::GetProperty
// Description:
// Arguments:    VSFTPROPID propid
//              __RPC__out VARIANT *pvar
// Return Val:  HRESULT STDMETHODCALLTYPE
// Author:      Sigal Algranaty
// Date:        18/5/2011
// ---------------------------------------------------------------------------
HRESULT STDMETHODCALLTYPE vspWindowPane::GetProperty(VSFTPROPID propid, __RPC__out VARIANT* pvar)
{
    GT_UNREFERENCED_PARAMETER(propid);
    GT_UNREFERENCED_PARAMETER(pvar);

    return S_OK;
}

// ---------------------------------------------------------------------------
// Name:        vspWindowPane::GetSearchImage
// Description:
// Arguments:    VSFINDOPTIONS grfOptions
//              __RPC__deref_out_opt IVsTextSpanSet **ppSpans
//              __RPC__deref_out_opt IVsTextImage **ppTextImage
// Return Val:  HRESULT STDMETHODCALLTYPE
// Author:      Sigal Algranaty
// Date:        18/5/2011
// ---------------------------------------------------------------------------
HRESULT STDMETHODCALLTYPE vspWindowPane::GetSearchImage(VSFINDOPTIONS grfOptions, __RPC__deref_out_opt IVsTextSpanSet** ppSpans, __RPC__deref_out_opt IVsTextImage** ppTextImage)
{
    GT_UNREFERENCED_PARAMETER(grfOptions);
    GT_UNREFERENCED_PARAMETER(ppSpans);
    GT_UNREFERENCED_PARAMETER(ppTextImage);

    return S_OK;
}

// ---------------------------------------------------------------------------
// Name:        vspWindowPane::Find
// Description: Implements the find operation when VS find dialog is used
// Arguments:    __RPC__in LPCOLESTR pszSearch
//              VSFINDOPTIONS grfOptions
//              BOOL fResetStartPoint
//              __RPC__in_opt IVsFindHelper *pHelper
//              __RPC__out VSFINDRESULT *pResult
// Return Val:  HRESULT STDMETHODCALLTYPE
// Author:      Sigal Algranaty
// Date:        17/5/2011
// ---------------------------------------------------------------------------
HRESULT STDMETHODCALLTYPE vspWindowPane::Find(__RPC__in LPCOLESTR pszSearch, VSFINDOPTIONS grfOptions, BOOL fResetStartPoint,
                                              __RPC__in_opt IVsFindHelper* pHelper, __RPC__out VSFINDRESULT* pResult)
{
    GT_UNREFERENCED_PARAMETER(pszSearch);
    GT_UNREFERENCED_PARAMETER(grfOptions);
    GT_UNREFERENCED_PARAMETER(fResetStartPoint);
    GT_UNREFERENCED_PARAMETER(pHelper);
    GT_UNREFERENCED_PARAMETER(pResult);

    return S_FALSE;
}

// ---------------------------------------------------------------------------
// Name:        vspWindowPane::Replace
// Description: Replace is not currently supported
// Arguments:   __RPC__in LPCOLESTR pszSearch
//              __RPC__in LPCOLESTR pszReplace
//              VSFINDOPTIONS grfOptions
//              BOOL fResetStartPoint
//              __RPC__in_opt IVsFindHelper *pHelper
//              __RPC__out BOOL *pfReplaced
// Return Val:  HRESULT STDMETHODCALLTYPE
// Author:      Sigal Algranaty
// Date:        18/5/2011
// ---------------------------------------------------------------------------
HRESULT STDMETHODCALLTYPE vspWindowPane::Replace(__RPC__in LPCOLESTR pszSearch, __RPC__in LPCOLESTR pszReplace, VSFINDOPTIONS grfOptions,
                                                 BOOL fResetStartPoint, __RPC__in_opt IVsFindHelper* pHelper, __RPC__out BOOL* pfReplaced)
{
    GT_UNREFERENCED_PARAMETER(pszSearch);
    GT_UNREFERENCED_PARAMETER(pszReplace);
    GT_UNREFERENCED_PARAMETER(grfOptions);
    GT_UNREFERENCED_PARAMETER(fResetStartPoint);
    GT_UNREFERENCED_PARAMETER(pHelper);
    GT_UNREFERENCED_PARAMETER(pfReplaced);

    return E_NOTIMPL;
}

// ---------------------------------------------------------------------------
// Name:        vspWindowPane::GetMatchRect
// Description: Empty implementation since we implement the find mechanism ourselves
// Arguments:   __RPC__out PRECT prc
// Return Val:  HRESULT STDMETHODCALLTYPE
// Author:      Sigal Algranaty
// Date:        18/5/2011
// ---------------------------------------------------------------------------
HRESULT STDMETHODCALLTYPE vspWindowPane::GetMatchRect(__RPC__out PRECT prc)
{
    GT_UNREFERENCED_PARAMETER(prc);

    return S_OK;
}

// ---------------------------------------------------------------------------
// Name:        vspWindowPane::NavigateTo
// Description: Empty implementation since we implement the find mechanism ourselves
// Arguments:   __RPC__in const TextSpan *pts
// Return Val:  HRESULT STDMETHODCALLTYPE
// Author:      Sigal Algranaty
// Date:        18/5/2011
// ---------------------------------------------------------------------------
HRESULT STDMETHODCALLTYPE vspWindowPane::NavigateTo(__RPC__in const TextSpan* pts)
{
    GT_UNREFERENCED_PARAMETER(pts);

    return S_OK;
}

// ---------------------------------------------------------------------------
// Name:        vspWindowPane::GetCurrentSpan
// Description: Empty implementation since we implement the find mechanism ourselves
// Arguments:   __RPC__out TextSpan *pts
// Return Val:  HRESULT STDMETHODCALLTYPE
// Author:      Sigal Algranaty
// Date:        18/5/2011
// ---------------------------------------------------------------------------
HRESULT STDMETHODCALLTYPE vspWindowPane::GetCurrentSpan(__RPC__out TextSpan* pts)
{
    GT_UNREFERENCED_PARAMETER(pts);

    return S_OK;
}

// ---------------------------------------------------------------------------
// Name:        vspWindowPane::SetFindState
// Description: The function sets and later return punk - an opaque pointer
//              that the VS find system uses
// Arguments:   __RPC__in_opt IUnknown *punk
// Return Val:  HRESULT STDMETHODCALLTYPE
// Author:      Sigal Algranaty
// Date:        18/5/2011
// ---------------------------------------------------------------------------
HRESULT STDMETHODCALLTYPE vspWindowPane::SetFindState(__RPC__in_opt IUnknown* punk)
{
    if (m_pUnkFindState != NULL)
    {
        m_pUnkFindState->Release();
        m_pUnkFindState = NULL;
    }

    if (punk)
    {
        punk->AddRef();
        m_pUnkFindState = punk;
    }

    return S_OK;
}

// ---------------------------------------------------------------------------
// Name:        vspWindowPane::GetFindState
// Description: The function gets punk - an opaque pointer that the VS find system uses
// Arguments:   __RPC__deref_out_opt IUnknown **ppunk
// Return Val:  HRESULT STDMETHODCALLTYPE
// Author:      Sigal Algranaty
// Date:        18/5/2011
// ---------------------------------------------------------------------------
HRESULT STDMETHODCALLTYPE vspWindowPane::GetFindState(__RPC__deref_out_opt IUnknown** ppunk)
{
    *ppunk = m_pUnkFindState;

    if (m_pUnkFindState)
    {
        m_pUnkFindState->AddRef();
    }

    return S_OK;
}

// ---------------------------------------------------------------------------
// Name:        vspWindowPane::NotifyFindTarget
// Description: Is called by the VS environment when the find target is changed
// Arguments:   VSFTNOTIFY notification
// Return Val:  HRESULT STDMETHODCALLTYPE
// Author:      Sigal Algranaty
// Date:        18/5/2011
// ---------------------------------------------------------------------------
HRESULT STDMETHODCALLTYPE vspWindowPane::NotifyFindTarget(VSFTNOTIFY notification)
{
    GT_UNREFERENCED_PARAMETER(notification);

    return S_OK;
}

// ---------------------------------------------------------------------------
// Name:        vspWindowPane::MarkSpan
// Description:
// Arguments:   __RPC__in const TextSpan *pts
// Return Val:  HRESULT STDMETHODCALLTYPE
// Author:      Sigal Algranaty
// Date:        18/5/2011
// ---------------------------------------------------------------------------
HRESULT STDMETHODCALLTYPE vspWindowPane::MarkSpan(__RPC__in const TextSpan* pts)
{
    GT_UNREFERENCED_PARAMETER(pts);

    return S_OK;
}

// ---------------------------------------------------------------------------
// Name:        vspWindowPane::GetLineSize
// Description: Returns the number of lines in the editor
// Arguments:   _Out_ LONG* pcLines
// Return Val:  STDMETHODIMP
// Author:      Sigal Algranaty
// Date:        18/5/2011
// ---------------------------------------------------------------------------
STDMETHODIMP vspWindowPane::GetLineSize(_Out_ LONG* pcLines)
{
    VSL_STDMETHODTRY
    {

        VSL_CHECKPOINTER(pcLines, E_INVALIDARG);

        return E_NOTIMPL;

    } VSL_STDMETHODCATCH()

    return VSL_GET_STDMETHOD_HRESULT();
}

// ---------------------------------------------------------------------------
// Name:        vspWindowPane::Replace
// Description: Replace is not implemented by our panes currently
// Return Val:  STDMETHODIMP
// Author:      Sigal Algranaty
// Date:        18/5/2011
// ---------------------------------------------------------------------------
STDMETHODIMP vspWindowPane::Replace(DWORD dwFlags, const TextSpan* pts, LONG cch, LPCOLESTR pchText, _Out_ TextSpan* ptsChanged)
{
    GT_UNREFERENCED_PARAMETER(dwFlags);
    GT_UNREFERENCED_PARAMETER(cch);
    GT_UNREFERENCED_PARAMETER(pchText);
    GT_UNREFERENCED_PARAMETER(ptsChanged);

    VSL_STDMETHODTRY
    {

        VSL_CHECKPOINTER(pts, E_INVALIDARG);

        return E_NOTIMPL;

    } VSL_STDMETHODCATCH()

    return VSL_GET_STDMETHOD_HRESULT();
}

// ---------------------------------------------------------------------------
// Name:        vspWindowPane::GetSpanLength
// Description:
// Arguments:    const TextSpan* pts
//              _Out_ LONG* pcch
// Return Val:  STDMETHODIMP
// Author:      Sigal Algranaty
// Date:        18/5/2011
// ---------------------------------------------------------------------------
STDMETHODIMP vspWindowPane::GetSpanLength(const TextSpan* pts, _Out_ LONG* pcch)
{

    VSL_STDMETHODTRY
    {

        VSL_CHECKPOINTER(pts, E_INVALIDARG);
        VSL_CHECKPOINTER(pcch, E_INVALIDARG);

        return E_NOTIMPL;

    } VSL_STDMETHODCATCH()

    return VSL_GET_STDMETHOD_HRESULT();
}

// ---------------------------------------------------------------------------
// Name:        vspWindowPane::GetLineLength
// Description:
// Arguments:    LONG iLine
//              _Out_ LONG* piLength
// Return Val:  STDMETHODIMP
// Author:      Sigal Algranaty
// Date:        18/5/2011
// ---------------------------------------------------------------------------
STDMETHODIMP vspWindowPane::GetLineLength(LONG iLine, _Out_ LONG* piLength)
{
    GT_UNREFERENCED_PARAMETER(iLine);

    VSL_STDMETHODTRY
    {

        VSL_CHECKPOINTER(piLength, E_INVALIDARG);

        return E_NOTIMPL;
    } VSL_STDMETHODCATCH()

    return VSL_GET_STDMETHOD_HRESULT();
}

// ---------------------------------------------------------------------------
// Name:        vspWindowPane::GetLine
// Description:
// Return Val:  STDMETHODIMP
// Author:      Sigal Algranaty
// Date:        18/5/2011
// ---------------------------------------------------------------------------
STDMETHODIMP vspWindowPane::GetLine(DWORD grfGet, LONG iLine, LONG iStartIndex, LONG iEndIndex, _Out_ LINEDATAEX* pLineData)
{
    GT_UNREFERENCED_PARAMETER(grfGet);
    GT_UNREFERENCED_PARAMETER(iLine);
    GT_UNREFERENCED_PARAMETER(iStartIndex);
    GT_UNREFERENCED_PARAMETER(iEndIndex);

    VSL_STDMETHODTRY
    {

        VSL_CHECKPOINTER(pLineData, E_INVALIDARG);

        return E_NOTIMPL;


    } VSL_STDMETHODCATCH()

    return VSL_GET_STDMETHOD_HRESULT();
}

void vspWindowPane::SetToolShowFunction(void* pVspToolWindow, void* pShowFunction)
{
    VSCORE(vscToolWindow_SetToolShowFunction)(m_pCoreImpl, pVspToolWindow, pShowFunction);
}

// ---------------------------------------------------------------------------
// Name:        const vspToolWindow::GetCaption
// Description: Return the caption of the tool window, according to its command it
// Return Val:  const wchar_t*
// Author:      Sigal Algranaty
// Date:        16/1/2011
// ---------------------------------------------------------------------------
const wchar_t* const vspToolWindow::GetCaption() const
{
    CStringW retVal;

    if (_myWindowCommandID == VSCORE(vsc_GetCallsHistoryListId)())
    {
        static CStringW strCallsHistoryCaption;

        // Avoid to load the string from the resources more that once.
        if (0 == strCallsHistoryCaption.GetLength())
        {
            VSL_CHECKBOOL_GLE(strCallsHistoryCaption.LoadStringW(_AtlBaseModule.GetResourceInstance(), IDS_FUNCTION_CALLS_HISTORY_WINDOW_TITLE));
        }

        retVal = strCallsHistoryCaption;
    }
    else if (_myWindowCommandID == VSCORE(vsc_GetPropertiesViewId)())
    {
        static CStringW strPropertiesCaption;

        // Avoid to load the string from the resources more that once.
        if (0 == strPropertiesCaption.GetLength())
        {
            VSL_CHECKBOOL_GLE(strPropertiesCaption.LoadStringW(_AtlBaseModule.GetResourceInstance(), IDS_PROPERTIES_WINDOW_TITLE));
        }

        retVal = strPropertiesCaption;
    }
    else if (_myWindowCommandID == VSCORE(vsc_GetObjectNavigationTreeId)())
    {
        static CStringW strExplorerCaption;

        // Avoid to load the string from the resources more that once.
        if (0 == strExplorerCaption.GetLength())
        {
            VSL_CHECKBOOL_GLE(strExplorerCaption.LoadStringW(_AtlBaseModule.GetResourceInstance(), IDS_EXPLORER_WINDOW_TITLE));

            std::wstring captionWithVersion = strExplorerCaption;
            wchar_t* pCaption = NULL;
            VSCORE(vscToolWindow_GetVersionCaption)(pCaption);
            assert(pCaption);

            if (pCaption != NULL)
            {
                std::wstring versionCaption = pCaption;

                // Release the allocated string.
                VSCORE(vscDeleteWcharString)(pCaption);

                if ((captionWithVersion.find(L"CodeXL") >= 0) && (!versionCaption.empty()))
                {
                    // Add the version string to the help caption:
                    std::wstring versionStr;
                    std::wstringstream converter;
                    converter << L"CodeXL Explorer (" << versionCaption.c_str() << L")";
                    versionStr.append(converter.str());
                    strExplorerCaption = versionStr.c_str();
                }
            }
        }

        retVal = strExplorerCaption;
    }
    else if (_myWindowCommandID == VSCORE(vsc_GetStatisticsViewId)())
    {
        static CStringW strStatisticsCaption;

        // Avoid to load the string from the resources more that once.
        if (0 == strStatisticsCaption.GetLength())
        {
            VSL_CHECKBOOL_GLE(strStatisticsCaption.LoadStringW(_AtlBaseModule.GetResourceInstance(), IDS_STATISTICS_WINDOW_TITLE));
        }

        retVal = strStatisticsCaption;
    }
    else if (_myWindowCommandID == VSCORE(vsc_GetCommandQueuesViewerId)())
    {
        assert(false);
        /*static CStringW strCommandQueuesCaption;
        // Avoid to load the string from the resources more that once.
        if (0 == strCommandQueuesCaption.GetLength())
        {
        VSL_CHECKBOOL_GLE(strCommandQueuesCaption.LoadStringW(_AtlBaseModule.GetResourceInstance(), IDS_COMMAND_QUEUES_WINDOW_TITLE));
        }
        retVal = strCommandQueuesCaption;*/
    }
    else if (_myWindowCommandID == VSCORE(vsc_GetFirstMultiWatchViewId)())
    {
        static CStringW strMultiwatchCaption;
        strMultiwatchCaption.Empty();

        // Avoid to load the string from the resources more that once.
        if (0 == strMultiwatchCaption.GetLength())
        {
            VSL_CHECKBOOL_GLE(strMultiwatchCaption.LoadStringW(_AtlBaseModule.GetResourceInstance(), IDS_MULTIWATCH_WINDOW_TITLE));
        }

        // Calculate the window index:
        int windowIndex = _myWindowCommandID - VSCORE(vsc_GetFirstMultiWatchViewId)() + 1;

        // Append the window index to the window caption:
        strMultiwatchCaption.AppendFormat(L"-%d", windowIndex);

        retVal = strMultiwatchCaption;
    }
    else if (_myWindowCommandID == VSCORE(vsc_GetSecondMultiWatchViewId)())
    {
        static CStringW strMultiwatchCaption;
        strMultiwatchCaption.Empty();

        // Avoid to load the string from the resources more that once.
        if (0 == strMultiwatchCaption.GetLength())
        {
            VSL_CHECKBOOL_GLE(strMultiwatchCaption.LoadStringW(_AtlBaseModule.GetResourceInstance(), IDS_MULTIWATCH_WINDOW_TITLE));
        }

        // Calculate the window index:
        int windowIndex = _myWindowCommandID - VSCORE(vsc_GetFirstMultiWatchViewId)() + 1;

        // Append the window index to the window caption:
        strMultiwatchCaption.AppendFormat(L"-%d", windowIndex);

        retVal = strMultiwatchCaption;
    }
    else if (_myWindowCommandID == VSCORE(vsc_GetThirdMultiWatchViewId)())
    {
        static CStringW strMultiwatchCaption;
        strMultiwatchCaption.Empty();

        // Avoid to load the string from the resources more that once.
        if (0 == strMultiwatchCaption.GetLength())
        {
            VSL_CHECKBOOL_GLE(strMultiwatchCaption.LoadStringW(_AtlBaseModule.GetResourceInstance(), IDS_MULTIWATCH_WINDOW_TITLE));
        }

        // Calculate the window index:
        int windowIndex = _myWindowCommandID - VSCORE(vsc_GetFirstMultiWatchViewId)() + 1;

        // Append the window index to the window caption:
        strMultiwatchCaption.AppendFormat(L"-%d", windowIndex);

        retVal = strMultiwatchCaption;
    }
    else if (_myWindowCommandID == VSCORE(vsc_GetMemoryAnalysisViewerId)())
    {
        static CStringW strMemoryCaption;

        // Avoid to load the string from the resources more that once.
        if (0 == strMemoryCaption.GetLength())
        {
            VSL_CHECKBOOL_GLE(strMemoryCaption.LoadStringW(_AtlBaseModule.GetResourceInstance(), IDS_MEMORY_WINDOW_TITLE));
        }

        retVal = strMemoryCaption;
    }
    else if (_myWindowCommandID == VSCORE(vsc_GetStateVariablesViewId)())
    {
        static CStringW strStateVariablesCaption;

        // Avoid to load the string from the resources more that once.
        if (0 == strStateVariablesCaption.GetLength())
        {
            VSL_CHECKBOOL_GLE(strStateVariablesCaption.LoadStringW(_AtlBaseModule.GetResourceInstance(), IDS_STATE_VARIABLES_WINDOW_TITLE));
        }

        retVal = strStateVariablesCaption;
    }
    else
    {
        static CStringW strEmptyCaption;
        retVal = strEmptyCaption;
    }

    return retVal;

}

// ---------------------------------------------------------------------------
// Name:        vspToolWindow::GetToolWindowGuid
// Description: Return the GUID of the persistent slot for this tool window.
// Return Val:  const GUID&
// Author:      Sigal Algranaty
// Date:        16/1/2011
// ---------------------------------------------------------------------------
const GUID& vspToolWindow::GetToolWindowGuid() const
{
    if (_myWindowCommandID == VSCORE(vsc_GetCallsHistoryListId)())
    {
        return CLSID_VSPCallsHistoryPersistanceId;
    }
    else if (_myWindowCommandID == VSCORE(vsc_GetPropertiesViewId)())
    {
        return CLSID_VSPPropertiesPersistanceId;
    }
    else if (_myWindowCommandID == VSCORE(vsc_GetObjectNavigationTreeId)())
    {
        return CLSID_VSPObjectsExplorerPersistanceId;
    }
    else if (_myWindowCommandID == VSCORE(vsc_GetStatisticsViewId)())
    {
        return CLSID_VSPStatisticsPersistanceId;
    }
    else if (_myWindowCommandID == VSCORE(vsc_GetCommandQueuesViewerId)())
    {
        assert(false); // return CLSID_VSPCommandQueuesPersistanceId;
    }
    else if (_myWindowCommandID == VSCORE(vsc_GetFirstMultiWatchViewId)())
    {
        return CLSID_VSPMultiwatch1PersistanceId;
    }
    else if (_myWindowCommandID == VSCORE(vsc_GetSecondMultiWatchViewId)())
    {
        return CLSID_VSPMultiwatch2PersistanceId;
    }
    else if (_myWindowCommandID == VSCORE(vsc_GetThirdMultiWatchViewId)())
    {
        return CLSID_VSPMultiwatch3PersistanceId;
    }
    else if (_myWindowCommandID == VSCORE(vsc_GetMemoryAnalysisViewerId)())
    {
        return CLSID_VSPMemoryPersistanceId;
    }
    else if (_myWindowCommandID == VSCORE(vsc_GetStateVariablesViewId)())
    {
        return CLSID_VSPStateVariablesPersistanceId;
    }

    return GUID_NULL;
}


// ---------------------------------------------------------------------------
// Name:        vspToolWindow::GetViewObject
// Description:
// Return Val:  IUnknown*
// Author:      Sigal Algranaty
// Date:        16/1/2011
// ---------------------------------------------------------------------------
IUnknown* vspToolWindow::GetViewObject()
{
    // Should only be called once per-instance
    VSL_CHECKBOOLEAN_EX(m_spView == NULL, E_UNEXPECTED, IDS_E_GETVIEWOBJECT_CALLED_AGAIN);

    // Create the object that implements the window pane for this tool window.
    CComObject<vspWindowPane>* pViewObject;
    VSL_CHECKHRESULT(CComObject<vspWindowPane>::CreateInstance(&pViewObject));

    IUnknown* pSPView = NULL;
    HRESULT hr = pViewObject->QueryInterface(IID_IUnknown, (void**)(&pSPView));

    if (FAILED(hr) || (pSPView == NULL))
    {
        // If QueryInterface failed, then there is something wrong with the object.
        // Delete it and throw an exception for the error.
        delete pViewObject;
        VSL_CHECKHRESULT(hr);
    }
    else
    {
        // test OLE interface
        IUnknown* pSPViewOle = NULL;
        pViewObject->QueryInterface(IID_IOleCommandTarget, (void**)(&pSPViewOle));

        m_spView = pSPView;
        vspWindowPane* pVSPackageWindowPage = (vspWindowPane*)((IVsWindowPane*)pSPView);
        assert(pVSPackageWindowPage != NULL);

        if (pVSPackageWindowPage != NULL)
        {
            // Set the pane window type:
            pVSPackageWindowPage->setWindowCommandID(_myWindowCommandID);
        }
    }

    return m_spView;
}


// ---------------------------------------------------------------------------
// Name:        vspToolWindow::setMyWindowCommandID
// Description:
// Arguments:   int windowCommandID
// Return Val:  void
// Author:      Sigal Algranaty
// Date:        16/1/2011
// ---------------------------------------------------------------------------
void vspToolWindow::setMyWindowCommandID(int windowCommandID)
{
    _myWindowCommandID = windowCommandID;
}

// ---------------------------------------------------------------------------
// Name:        vspToolWindow::PostCreate
// Description: This method is called by the base class after the tool window is created.
//              We use it to set the icon for this window.
// Return Val:  void
// Author:      Sigal Algranaty
// Date:        16/1/2011
// ---------------------------------------------------------------------------
void vspToolWindow::PostCreate()
{
    CComVariant srpvt;
    srpvt.vt = VT_I4;
    srpvt.intVal = IDB_IMAGES_MENU_COMMANDS_IMAGES;

    // We don't want to make the window creation fail only because we can not set
    // the icon, so we will not throw if SetProperty fails.
    if (SUCCEEDED(GetIVsWindowFrame()->SetProperty(VSFPROPID_BitmapResource, srpvt)))
    {
        srpvt.intVal = iconIndexFromWindowCommandID(_myWindowCommandID);
        GetIVsWindowFrame()->SetProperty(VSFPROPID_BitmapIndex, srpvt);
    }

    // Register the show function in the vsctoolwindow data manager
    IVsWindowFrame* pWindowFrame = GetIVsWindowFrame();

    // Get the vscToolWindow:
    IUnknown* pSPView = m_spView;
    vspWindowPane* pVSPackageWindowPane = (vspWindowPane*)((IVsWindowPane*)pSPView);

    // Pass the data to the window
    if (nullptr != pVSPackageWindowPane)
    {
        pVSPackageWindowPane->SetToolShowFunction((void*)pWindowFrame, (void*)&vscShowWindow);
    }
}


// ---------------------------------------------------------------------------
// Name:        vspToolWindow::iconIndexFromWindowCommandID
// Description: Translates a window's command ID to its index in the image map
// Author:      Uri Shomroni
// Date:        31/1/2011
// ---------------------------------------------------------------------------
int vspToolWindow::iconIndexFromWindowCommandID(int windowCommandID)
{
    int retVal = 0;

    if (windowCommandID == VSCORE(vsc_GetCallsHistoryListId)())
    {
        retVal = bmpHistory - 1;
    }
    else if (windowCommandID == VSCORE(vsc_GetPropertiesViewId)())
    {
        retVal = bmpProperties - 1;
    }
    else if (windowCommandID == VSCORE(vsc_GetObjectNavigationTreeId)())
    {
        retVal = bmpExplorer - 1;
    }
    else if (windowCommandID == VSCORE(vsc_GetStatisticsViewId)())
    {
        retVal = bmpStatistics - 1;
    }
    else if (windowCommandID == VSCORE(vsc_GetCommandQueuesViewerId)())
    {
        assert(false); // retVal = bmpCommandQueues - 1;
    }
    else if (windowCommandID == VSCORE(vsc_GetFirstMultiWatchViewId)()    ||
             windowCommandID == VSCORE(vsc_GetSecondMultiWatchViewId)()   ||
             windowCommandID == VSCORE(vsc_GetThirdMultiWatchViewId)())
    {
        retVal = bmpMultiWatch - 1;
    }
    else if (windowCommandID == VSCORE(vsc_GetMemoryAnalysisViewerId)())
    {
        retVal = bmpMemory - 1;
    }
    else if (windowCommandID == VSCORE(vsc_GetStateVariablesViewId)())
    {
        retVal = bmpStateVars - 1;
    }
    else if (windowCommandID == VSCORE(vsc_GetShadersSourceCodeViewerHtmlWindowId)())
    {
        retVal = bmpCommandQueues - 1;
    }
    else
    {
        assert(false);
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        vspToolWindow::setCaption
// Description: Set the tool window caption
// Arguments:   const gtString& caption
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        27/1/2011
// ---------------------------------------------------------------------------
bool vspToolWindow::setCaption(const std::wstring& caption)
{
    bool retVal = false;

    IVsWindowFrame* pWindowFrame = GetIVsWindowFrame();

    // NOTICE: Sometime the window frame is not yet initialized, so do not assert for this condition:
    if (pWindowFrame != NULL)
    {
        VARIANT captionVar = {0};
        V_VT(&captionVar) = VT_BSTR;

        // Copy the string to the VARIANT structure:
        V_BSTR(&captionVar) = SysAllocString(caption.c_str());

        // Set the object string as the owner caption:
        int rcSetProperty = pWindowFrame->SetProperty(VSFPROPID_Caption, captionVar);
        retVal = rcSetProperty == 0;

        // Release the caption string memory:
        VariantClear(&captionVar);
    }

    return retVal;
}

