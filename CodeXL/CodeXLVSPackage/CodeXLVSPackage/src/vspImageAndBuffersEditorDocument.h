//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file vspImageAndBuffersEditorDocument.h
///
//==================================================================================

//------------------------------ vspImageAndBuffersEditorDocument.h ------------------------------

#ifndef __VSPIMAGEANDBUFFERSEDITORDOCUMENT_H
#define __VSPIMAGEANDBUFFERSEDITORDOCUMENT_H

// C++:
#include <cassert>

// Visual Studio:
#include <atlsafe.h>
#include <VSLExceptionHandlers.h>

// Local:
#include <../CodeXLVSPackageUI/CommandIds.h>
#include <src/vspCoreAPI.h>
#include <src/vspEditorDocument.h>
#include <src/vspToolWindow.h>

/***************************************************************************

EditorDocument provides the implementation of a single view editor (as opposed to a
multi-view editor that can display the same file in multiple modes like the HTML editor).

CodeXLVSPackage.pkgdef contains:

[$RootKey$\KeyBindingTables\{CF90A284-AB86-44CE-82AD-7F8971249635}]
@="#1"
"AllowNavKeyBinding"=dword:00000000
"Package"="{ecdfbaee-ad99-452d-874c-99fce5a48b8e}"

which is required for some, but not all, of the key bindings, which are located at the bottom of
CodeXLVSPackageUI.vsct, to work correctly so that the appropriate command handler below will be
called.

***************************************************************************/

class vspImageAndBuffersEditorDocument : public vspEditorDocument
{

    // COM objects typically should not be cloned, and this prevents cloning by declaring the
    // copy constructor and assignment operator private (NOTE:  this macro includes the deceleration of
    // a private section, so everything following this macro and preceding a public or protected
    // section will be private).
    VSL_DECLARE_NOT_COPYABLE(vspImageAndBuffersEditorDocument)

public:
    // Called by VSL::DocumentPersistanceBase::GetClassID, which is also called by
    // VSL::DocumentPersistanceBase::GetGuidEditorType)
    const GUID& GetEditorTypeGuid() const;

protected:

    typedef vspImageAndBuffersEditorDocument This;

    vspImageAndBuffersEditorDocument();

    ~vspImageAndBuffersEditorDocument();

    virtual void GetCommandUpdate(const CommandHandler& rSender, _Inout_ OLECMD* pOleCmd, _Inout_ OLECMDTEXT* pOleText);
    virtual void ExecuteCommandAction(_In_ CommandHandler* pSender, DWORD flags, _In_ VARIANT* pIn, _Out_ VARIANT* pOut);

    // Zoom action callbacks:
    void onImageAndBufferAction(_In_ CommandHandler* pSender, DWORD flags, _In_ VARIANT* pIn, _Out_ VARIANT* pOut);
    void onQueryImageAndBufferAction(const CommandHandler& rSender, _Inout_ OLECMD* pOleCmd, _Inout_ OLECMDTEXT* pOleText);
    void onQueryImageAndBufferCheckedAction(const CommandHandler& rSender, _Inout_ OLECMD* pOleCmd, _Inout_ OLECMDTEXT* pOleText);

    // Combo list:
    void onQueryImageSizeGetList(const CommandHandler& rSender, _Inout_ OLECMD* pOleCmd, _Inout_ OLECMDTEXT* pOleText);
    void onImageSizeGetList(_In_ CommandHandler* pSender, DWORD flags, _In_ VARIANT* pIn, _Out_ VARIANT* pOut);

    void onImageSizeChanged(_In_ CommandHandler* pSender, DWORD flags, _In_ VARIANT* pIn, _Out_ VARIANT* pOut);
    void onQueryImageSizeChanged(const CommandHandler& rSender, _Inout_ OLECMD* pOleCmd, _Inout_ OLECMDTEXT* pOleText);
};

#endif //__VSPIMAGEANDBUFFERSEDITORDOCUMENT_H

