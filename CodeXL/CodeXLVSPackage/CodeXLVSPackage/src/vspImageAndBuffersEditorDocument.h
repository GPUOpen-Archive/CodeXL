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

    /// Define the toolbar actions for the images and buffers MDI windows:
    // IOleCommandTarget.
    VSL_BEGIN_COMMAND_MAP()

    // Every command is identified by the shell using a GUID/DWORD pair, so every the definition of
    // commands must contain this information.

    // The following command map entries define a GUID/DWORD pair to identify the command and a
    // callback for the command execution and status queries.
    VSL_COMMAND_MAP_ENTRY(CMDSETID_StandardCommandSet97, cmdidZoomIn, &onQueryImageAndBufferAction, &onImageAndBufferAction)
    VSL_COMMAND_MAP_ENTRY(CMDSETID_StandardCommandSet97, cmdidZoomOut, &onQueryImageAndBufferAction, &onImageAndBufferAction)
    VSL_COMMAND_MAP_ENTRY(CLSID_CodeXLVSPackageCmdSet, commandIDBestFit, &onQueryImageAndBufferAction, &onImageAndBufferAction)
    VSL_COMMAND_MAP_ENTRY(CLSID_CodeXLVSPackageCmdSet, commandIDOrigSize, &onQueryImageAndBufferAction, &onImageAndBufferAction)
    VSL_COMMAND_MAP_ENTRY(CLSID_CodeXLVSPackageCmdSet, commandIDPan, &onQueryImageAndBufferAction, &onImageAndBufferAction)
    VSL_COMMAND_MAP_ENTRY(CLSID_CodeXLVSPackageCmdSet, commandIDSelect, &onQueryImageAndBufferAction, &onImageAndBufferAction)
    VSL_COMMAND_MAP_ENTRY(CLSID_CodeXLVSPackageCmdSet, commandIDRotateLeft, &onQueryImageAndBufferCheckedAction, &onImageAndBufferAction)
    VSL_COMMAND_MAP_ENTRY(CLSID_CodeXLVSPackageCmdSet, commandIDRotateRight, &onQueryImageAndBufferCheckedAction, &onImageAndBufferAction)
    VSL_COMMAND_MAP_ENTRY(CLSID_CodeXLVSPackageCmdSet, commandIDChannelRed, &onQueryImageAndBufferCheckedAction, &onImageAndBufferAction)
    VSL_COMMAND_MAP_ENTRY(CLSID_CodeXLVSPackageCmdSet, commandIDChannelGreen, &onQueryImageAndBufferCheckedAction, &onImageAndBufferAction)
    VSL_COMMAND_MAP_ENTRY(CLSID_CodeXLVSPackageCmdSet, commandIDChannelBlue, &onQueryImageAndBufferCheckedAction, &onImageAndBufferAction)
    VSL_COMMAND_MAP_ENTRY(CLSID_CodeXLVSPackageCmdSet, commandIDChannelAlpha, &onQueryImageAndBufferCheckedAction, &onImageAndBufferAction)
    VSL_COMMAND_MAP_ENTRY(CLSID_CodeXLVSPackageCmdSet, commandIDChannelInvert, &onQueryImageAndBufferCheckedAction, &onImageAndBufferAction)
    VSL_COMMAND_MAP_ENTRY(CLSID_CodeXLVSPackageCmdSet, commandIDChannelGrayscale, &onQueryImageAndBufferCheckedAction, &onImageAndBufferAction)

    VSL_COMMAND_MAP_ENTRY(CLSID_CodeXLVSPackageCmdSet, commandIDImageSizeComboGetList, &onQueryImageSizeGetList, &onImageSizeGetList)
    VSL_COMMAND_MAP_ENTRY(CLSID_CodeXLVSPackageCmdSet, commandIDImageSizeCombo, &onQueryImageSizeChanged, &onImageSizeChanged)

    VSL_COMMAND_MAP_ENTRY(CMDSETID_StandardCommandSet97, cmdidCopy, &onQueryCopyAction, &onCopyAction)
    VSL_COMMAND_MAP_ENTRY(CMDSETID_StandardCommandSet97, cmdidSelectAll, &onQuerySelectAllAction, &onSelectAllAction)

    // Terminate the definition of the command map
    VSL_END_VSCOMMAND_MAP()

    // Called by VSL::DocumentPersistanceBase::GetClassID, which is also called by
    // VSL::DocumentPersistanceBase::GetGuidEditorType)
    const GUID& GetEditorTypeGuid() const;

protected:

    typedef vspImageAndBuffersEditorDocument This;

    vspImageAndBuffersEditorDocument();

    ~vspImageAndBuffersEditorDocument();


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

