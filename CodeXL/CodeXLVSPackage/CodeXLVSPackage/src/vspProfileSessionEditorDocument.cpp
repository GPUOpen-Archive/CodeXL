//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file vspProfileSessionEditorDocument.cpp
///
//==================================================================================

//------------------------------ vspProfileSessionEditorDocument.cpp ------------------------------

#include "stdafx.h"
#include <src/Guids.h>
#include <../CodeXLVSPackageUi/Resource.h>

// Local:
#include <src/Package.h>
#include <src/vspCoreAPI.h>
#include <src/vspDTEConnector.h>
#include <src/vspPackageWrapper.h>
#include <src/vspProfileSessionEditorDocument.h>
#include <Include/vspStringConstants.h>
#include <src/vscVsUtils.h>


vspProfileSessionEditorDocument::vspProfileSessionEditorDocument() : vspEditorDocument()
{
    m_pEditorDocumentCoreImpl = VSCORE(vscProfileSessionEditorDocument_CreateInstance)();
    VSP_ASSERT(m_pEditorDocumentCoreImpl != nullptr);

    if (m_pEditorDocumentCoreImpl != NULL)
    {
        // Register as the owner to get notifications.
        VSCORE(vscProfileSessionEditorDocument_SetVSCOwner)(m_pEditorDocumentCoreImpl, this);
    }
}
vspProfileSessionEditorDocument::~vspProfileSessionEditorDocument()
{

}

// Called by VSL::DocumentPersistanceBase::GetClassID, which is also called by
// VSL::DocumentPersistanceBase::GetGuidEditorType)
// RELOCATE TO CORE: No
const GUID& vspProfileSessionEditorDocument::GetEditorTypeGuid() const
{
    // The GUID for the factory is the one to return from IPersist::GetClassID and
    // IVsPersistDocData::GetGuidEditorType
    return CLSID_CodeXLVSPackageProfileSessionEditorFactory;
}

void vspProfileSessionEditorDocument::ceOnAfterSessionRenamed(wchar_t* oldSessionFilePath)
{
    // Close the opened MDI in VS:
    vspDTEConnector::instance().closeOpenedFile(oldSessionFilePath);

    // Ask the core's runtime to release the memory.
    VSCORE(vscDeleteWcharString)(oldSessionFilePath);
}

void vspProfileSessionEditorDocument::ceOnExplorerReadyForSessions()
{
    // Do not trigger the CodeLX extension clock tick from this location because this function may be called when
    // the ClockTick() is executed, leading to an endless recursion.

    // Invoke the function that implements the actual core logic.
    VSCORE(vscProfileSessionEditorDocument_LoadSession)(m_pEditorDocumentCoreImpl);

    // check if there was a cancel load in the middle
    if (m_pEditorDocumentCoreImpl != nullptr)
    {
        SetEditorCaption(m_filePathStr);
    }
}
