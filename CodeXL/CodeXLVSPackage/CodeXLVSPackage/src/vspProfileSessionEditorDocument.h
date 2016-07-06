//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file vspProfileSessionEditorDocument.h
///
//==================================================================================

//------------------------------ vspProfileSessionEditorDocument.h ------------------------------

#ifndef __VSPPROFILESESSIONEDITORDOCUMENT_H
#define __VSPPROFILESESSIONEDITORDOCUMENT_H

#include "StdAfx.h"

// C++:
#include <string>

// Windows:
#include <WinBase.h>
#include <WinDef.h>
#include <WinUser.h>

// Infra:
#include <Include/Public/CoreInterfaces/IProfileSessionEditorDocVsCoreImplOwner.h>

// Local:
#include <../CodeXLVSPackageUI/CommandIds.h>
#include <src/vspCoreAPI.h>
#include <src/vspEditorDocument.h>

class ProfileMDIWindowHelper;
class SessionTreeNodeData;
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

class vspProfileSessionEditorDocument : public vspEditorDocument, public IProfileSessionEditorDocVsCoreImplOwner
{
    friend class ProfileMDIWindowHelper;

    // COM objects typically should not be cloned, and this prevents cloning by declaring the
    // copy constructor and assignment operator private (NOTE:  this macro includes the deceleration of
    // a private section, so everything following this macro and preceding a public or protected
    // section will be private).
    VSL_DECLARE_NOT_COPYABLE(vspProfileSessionEditorDocument)

public:

    // Called by VSL::DocumentPersistanceBase::GetClassID, which is also called by
    // VSL::DocumentPersistanceBase::GetGuidEditorType)
    const GUID& GetEditorTypeGuid() const;


protected:

    typedef vspProfileSessionEditorDocument This;

    vspProfileSessionEditorDocument();

    ~vspProfileSessionEditorDocument();

private:

    // Core event handlers:
    virtual void ceOnAfterSessionRenamed(wchar_t* oldSessionFilePath);
    virtual void ceOnExplorerReadyForSessions();

    // IVsWindowPane Data
    bool m_bClosed;
};





#endif //__VSPPROFILESESSIONEDITORDOCUMENT_H



