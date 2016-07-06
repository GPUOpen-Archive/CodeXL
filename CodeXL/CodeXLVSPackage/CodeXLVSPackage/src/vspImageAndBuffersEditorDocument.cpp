//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file vspImageAndBuffersEditorDocument.cpp
///
//==================================================================================

//------------------------------ vspImageAndBuffersEditorDocument.cpp ------------------------------

#include "stdafx.h"

#include <string>
#include <sstream>

// Infra:
#include <AMDTBaseTools/Include/AMDTDefinitions.h>

// Local:
#include <src/vscVsUtils.h>
#include <src/vspCoreAPI.h>
#include <src/vspImageAndBuffersEditorDocument.h>




vspImageAndBuffersEditorDocument::vspImageAndBuffersEditorDocument() : vspEditorDocument()
{
    m_pEditorDocumentCoreImpl = VSCORE(vscImageAndBuffersEditorDocument_CreateInstance)();
    VSP_ASSERT(m_pEditorDocumentCoreImpl != NULL);
}

vspImageAndBuffersEditorDocument::~vspImageAndBuffersEditorDocument()
{

}


// Called by VSL::DocumentPersistanceBase::GetClassID, which is also called by
// VSL::DocumentPersistanceBase::GetGuidEditorType)
const GUID& vspImageAndBuffersEditorDocument::GetEditorTypeGuid() const
{
    // The GUID for the factory is the one to return from IPersist::GetClassID and
    // IVsPersistDocData::GetGuidEditorType
    return CLSID_CodeXLVSPackageEditorFactory;

}


// ---------------------------------------------------------------------------
// Name:        vspImageAndBuffersEditorDocument::onImageAndBufferAction
// Description: General event handling all the images and buffers toolbar commands
// Arguments:   _In_ CommandHandler* /*pSender*/
//              DWORD flags
//              _In_ VARIANT* pIn
//              _Out_ VARIANT* pOut
// Author:      Sigal Algranaty
// Date:        19/12/2010
// ---------------------------------------------------------------------------
void vspImageAndBuffersEditorDocument::onImageAndBufferAction(_In_ CommandHandler* pSender, DWORD flags, _In_ VARIANT* pIn, _Out_ VARIANT* pOut)
{
    GT_UNREFERENCED_PARAMETER(flags);
    GT_UNREFERENCED_PARAMETER(pIn);
    GT_UNREFERENCED_PARAMETER(pOut);

    VSP_ASSERT(pSender != NULL);

    if (pSender != NULL)
    {
        VSCORE(vscImageAndBuffersEditorDocument_OnImageAndBuffersAction)(pSender->GetId().GetGuid(), pSender->GetId().GetId());
    }
}

// ---------------------------------------------------------------------------
// Name:        vspImageAndBuffersEditorDocument::onQueryImageAndBufferAction
// Description: General event handling the Query event for all images and buffers toolbar commands
// Arguments:   const CommandHandler& rSender
//              _Inout_ OLECMD* pOleCmd
//              _Inout_ OLECMDTEXT* pOleText
// Return Val:  void
// Author:      Sigal Algranaty
// Date:        19/12/2010
// ---------------------------------------------------------------------------
void vspImageAndBuffersEditorDocument::onQueryImageAndBufferAction(const CommandHandler& rSender, _Inout_ OLECMD* pOleCmd, _Inout_ OLECMDTEXT* pOleText)
{
    GT_UNREFERENCED_PARAMETER(pOleText);

    bool shouldEnable = VSCORE(vscImageAndBuffersEditorDocument_OnQueryImageAndBufferAction_IsActionRequired)(rSender.GetId().GetGuid(), rSender.GetId().GetId());

    if (shouldEnable)
    {
        pOleCmd[0].cmdf = OLECMDSTATE_UP;
    }
    else
    {
        pOleCmd[0].cmdf = OLECMDSTATE_DISABLED;
    }
}

// ---------------------------------------------------------------------------
// Name:        vspImageAndBuffersEditorDocument::onQueryImageAndBufferAction
// Description: General event handling the Query event for all images and buffers toolbar commands
// Arguments:   const CommandHandler& rSender
//              _Inout_ OLECMD* pOleCmd
//              _Inout_ OLECMDTEXT* pOleText
// Return Val:  void
// Author:      Sigal Algranaty
// Date:        19/12/2010
// ---------------------------------------------------------------------------
void vspImageAndBuffersEditorDocument::onQueryImageAndBufferCheckedAction(const CommandHandler& rSender, _Inout_ OLECMD* pOleCmd, _Inout_ OLECMDTEXT* pOleText)
{
    GT_UNREFERENCED_PARAMETER(pOleText);

    pOleCmd[0].cmdf = OLECMDF_SUPPORTED;

    bool shouldEnable = false;
    bool shouldCheck = false;
    VSCORE(vscImageAndBuffersEditorDocument_OnQueryImageAndBufferCheckedAction_IsActionRequired)(rSender.GetId().GetGuid(), rSender.GetId().GetId(), shouldEnable, shouldCheck);

    if (shouldEnable)
    {
        pOleCmd[0].cmdf |= OLECMDF_ENABLED;

        if (shouldCheck)
        {
            pOleCmd[0].cmdf |= OLECMDF_LATCHED;
        }
    }
    else
    {
        pOleCmd[0].cmdf = OLECMDSTATE_DISABLED;
    }
}

// ---------------------------------------------------------------------------
// Name:        vspImageAndBuffersEditorDocument::onQueryImageSizeGetList
// Description: Used to update image list for the size combo
// Arguments:   const CommandHandler& rSender
//              _Inout_ OLECMD* pOleCmd
//              _Inout_ OLECMDTEXT* pOleText
// Return Val:  void
// Author:      Sigal Algranaty
// Date:        19/12/2010
// ---------------------------------------------------------------------------
void vspImageAndBuffersEditorDocument::onQueryImageSizeGetList(const CommandHandler& rSender, _Inout_ OLECMD* pOleCmd, _Inout_ OLECMDTEXT* pOleText)
{
    GT_UNREFERENCED_PARAMETER(&rSender);
    GT_UNREFERENCED_PARAMETER(pOleText);

    // Since this is a drop down, rather then a button, it is always up:
    pOleCmd[0].cmdf = OLECMDSTATE_UP;
}

// ---------------------------------------------------------------------------
// Name:        vspImageAndBuffersEditorDocument::onImageSizeGetList
// Description: Allocate and return the list of strings for the zoom combo
// Arguments:   _In_ CommandHandler* pSender
//              DWORD flags
//              _In_ VARIANT* pIn
//              _Out_ VARIANT* pvarOut
// Return Val:  void
// Author:      Sigal Algranaty
// Date:        20/12/2010
// ---------------------------------------------------------------------------
void vspImageAndBuffersEditorDocument::onImageSizeGetList(_In_ CommandHandler* pSender, DWORD flags, _In_ VARIANT* pIn, _Out_ VARIANT* pvarOut)
{
    GT_UNREFERENCED_PARAMETER(pSender);
    GT_UNREFERENCED_PARAMETER(flags);
    GT_UNREFERENCED_PARAMETER(pIn);

    unsigned int* pAvailableZoomLevelsBuffer = NULL;
    size_t bufSize = 0;

    bool isOk = VSCORE(vscImageAndBuffersEditorDocument_GetAvailableZoomLevels)(pAvailableZoomLevelsBuffer, bufSize) &&
                (pAvailableZoomLevelsBuffer != NULL);
    VSP_ASSERT(isOk);

    if (isOk)
    {
        VSL_CHECKPOINTER(pvarOut, E_INVALIDARG);

        // Clear the out the value here in case of failure
        ::VariantClear(pvarOut);

        VSL_CHECKBOOLEAN(bufSize > 0, E_FAIL);

        ATL::CComSafeArray<BSTR> zoomStringsArray(static_cast<ULONG>(bufSize));

        for (int i = 0; i < (int)bufSize; i++)
        {
            // Build the representation string:
            std::wstringstream tmpStream;
            tmpStream << ((int)pAvailableZoomLevelsBuffer[i]) << L"%";
            const wchar_t* pTmpStr = tmpStream.str().c_str();

            // Copy the strings to a BSTR:
            BSTR zoomValueAsBSTR = SysAllocString(pTmpStr);
            zoomStringsArray.SetAt(i, zoomValueAsBSTR);
        }

        V_ARRAY(pvarOut) = zoomStringsArray.Detach();
        V_VT(pvarOut) = VT_ARRAY | VT_BSTR;
    }

    VSCORE(vscDeleteUintBuffer)(pAvailableZoomLevelsBuffer);
}

// ---------------------------------------------------------------------------
// Name:        vspImageAndBuffersEditorDocument::onImageSizeChanged
// Description: Is called when the combo box value is changed, and also when the combo box
//              selected value if requested
// Arguments:   _In_ CommandHandler* pSender
//              DWORD flags
//              _In_ VARIANT* pIn
//              _Out_ VARIANT* pOut
// Return Val:  void
// Author:      Sigal Algranaty
// Date:        20/12/2010
// ---------------------------------------------------------------------------
void vspImageAndBuffersEditorDocument::onImageSizeChanged(_In_ CommandHandler* pSender, DWORD flags, _In_ VARIANT* pIn, _Out_ VARIANT* pOut)
{
    GT_UNREFERENCED_PARAMETER(pSender);
    GT_UNREFERENCED_PARAMETER(flags);

    bool isValid = false;
    int currentZoomLevel = 100;

    // The user typed a zoom as text:
    if (pIn != NULL)
    {
        if (pIn->vt == VT_BSTR)
        {
            // Get the user string as gtString:
            const wchar_t* zoomText = pIn->bstrVal;

            // Get the current focused image buffer view:
            isValid = VSCORE(vscImageAndBuffersEditorDocument_ChangeZoomLevel)(zoomText, currentZoomLevel);

            if (isValid)
            {
                if (pOut != NULL)
                {
                    // Copy the string to the VARIANT structure:
                    V_VT(pOut) = VT_BSTR;
                    V_BSTR(pOut) = SysAllocString(pIn->bstrVal);
                }
            }
        }
    }

    if (!isValid)
    {
        // Build the zoom representation string:
        std::wstring currentZoomStr;
        std::wstringstream tmpStream;
        tmpStream << currentZoomLevel << L"%";
        const wchar_t* tmpStr = tmpStream.str().c_str();

        // Copy the current zoom level to the VARIANT:
        // Allocate a new VARIANT:
        if (pOut != NULL)
        {
            // Copy the string to the VARIANT structure:
            V_VT(pOut) = VT_BSTR;
            V_BSTR(pOut) = SysAllocString(tmpStr);
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        vspImageAndBuffersEditorDocument::onQueryImageSizeChanged
// Description: Query function for the combo value change event
// Arguments:   const CommandHandler& rSender
//              _Inout_ OLECMD* pOleCmd
//              _Inout_ OLECMDTEXT* pOleText
// Return Val:  void
// Author:      Sigal Algranaty
// Date:        20/12/2010
// ---------------------------------------------------------------------------
void vspImageAndBuffersEditorDocument::onQueryImageSizeChanged(const CommandHandler& rSender, _Inout_ OLECMD* pOleCmd, _Inout_ OLECMDTEXT* pOleText)
{
    GT_UNREFERENCED_PARAMETER(&rSender);
    GT_UNREFERENCED_PARAMETER(pOleText);

    // Check if this action should be checked:
    bool shouldEnable = false;
    VSCORE(vscImageAndBuffersEditorDocument_OnQueryImageSizeChanged_IsActionRequired)(shouldEnable);

    if (shouldEnable)
    {
        pOleCmd[0].cmdf = OLECMDSTATE_UP;
    }
    else
    {
        pOleCmd[0].cmdf = OLECMDSTATE_DISABLED;
    }
}

void vspImageAndBuffersEditorDocument::GetCommandUpdate(const CommandHandler& rSender, _Inout_ OLECMD* pOleCmd, _Inout_ OLECMDTEXT* pOleText)
{
    DWORD commandId = rSender.GetId().GetId();
    switch (commandId)
    {
    case cmdidZoomIn:
    case cmdidZoomOut:
    case commandIDBestFit:
    case commandIDOrigSize:
    case commandIDPan:
    case commandIDSelect:
        onQueryImageAndBufferAction(rSender, pOleCmd, pOleText);
        break;

    case commandIDRotateLeft:
    case commandIDRotateRight:
    case commandIDChannelRed:
    case commandIDChannelGreen:
    case commandIDChannelBlue:
    case commandIDChannelAlpha:
    case commandIDChannelInvert:
    case commandIDChannelGrayscale:
        onQueryImageAndBufferCheckedAction(rSender, pOleCmd, pOleText);
        break;

    case commandIDImageSizeComboGetList:
    case commandIDImageSizeCombo:
        onQueryImageSizeGetList(rSender, pOleCmd, pOleText);
        break;
    }
}

void vspImageAndBuffersEditorDocument::ExecuteCommandAction(_In_ CommandHandler* pSender, DWORD flags, _In_ VARIANT* pIn, _Out_ VARIANT* pOut)
{
    DWORD commandId = pSender->GetId().GetId();

    switch (commandId)
    {
    case cmdidZoomIn:
    case cmdidZoomOut:
    case commandIDBestFit:
    case commandIDOrigSize:
    case commandIDPan:
    case commandIDSelect:
        onImageAndBufferAction(pSender, flags, pIn, pOut);
        break;

    case commandIDRotateLeft:
    case commandIDRotateRight:
    case commandIDChannelRed:
    case commandIDChannelGreen:
    case commandIDChannelBlue:
    case commandIDChannelAlpha:
    case commandIDChannelInvert:
    case commandIDChannelGrayscale:
        onImageAndBufferAction(pSender, flags, pIn, pOut);
        break;

    case commandIDImageSizeComboGetList:
    case commandIDImageSizeCombo:
        onImageSizeGetList(pSender, flags, pIn, pOut);
        break;
    }
}
