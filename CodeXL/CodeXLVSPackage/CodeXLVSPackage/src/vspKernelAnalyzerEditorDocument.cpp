//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file vspKernelAnalyzerEditorDocument.cpp
///
//==================================================================================

//------------------------------ vspKernelAnalyzerEditorDocument.cpp ------------------------------

#include "stdafx.h"

#include <atlsafe.h>

// Infra:
#include <AMDTBaseTools/Include/AMDTDefinitions.h>

// Local:
#include <src/vspCoreAPI.h>
#include <src/vspKernelAnalyzerEditorDocument.h>
#include <src/vspTimer.h>




vspKernelAnalyzerEditorDocument::vspKernelAnalyzerEditorDocument() : vspEditorDocument()
{
    m_pEditorDocumentCoreImpl = VSCORE(vscKernelAnalyzerEditorDocument_CreateInstance)();
    assert(m_pEditorDocumentCoreImpl != NULL);
}

vspKernelAnalyzerEditorDocument::~vspKernelAnalyzerEditorDocument()
{

}


// Called by VSL::DocumentPersistanceBase::GetClassID, which is also called by
// VSL::DocumentPersistanceBase::GetGuidEditorType)
const GUID& vspKernelAnalyzerEditorDocument::GetEditorTypeGuid() const
{
    // The GUID for the factory is the one to return from IPersist::GetClassID and
    // IVsPersistDocData::GetGuidEditorType
    return CLSID_CodeXLVSPackageKernelAnalyzerEditorFactory;
}
