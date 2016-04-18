//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file vscProfileSessionEditorDocument.h
///
//==================================================================================

#ifndef __vscProfileSessionEditorDocument_h
#define __vscProfileSessionEditorDocument_h
#include "CodeXLVSPackageCoreDefs.h"

// Core interfaces:
#include "Include/Public/CoreInterfaces/IProfileSessionEditorDocVsCoreImplOwner.h"
#include <Windows.h>

// Local:
#include <Include/Public/vscEditorDocument.h>


class vscProfileSessionEditorDocument: public vscEditorDocument
{
public:

    friend class ProfileMDIWindowHelper;
    friend class vspPowerProfilingEditorManager;
    friend class vspPowerProfilingEditorManagerHelper;
    vscProfileSessionEditorDocument();

    void vscSetOwner(IProfileSessionEditorDocVsCoreImplOwner* pOwner) { m_pOwner = pOwner; }
    virtual void LoadDocData(const wchar_t* filePathStr);

    virtual void SetEditorCaption(const wchar_t* filePathStr, wchar_t*& itemNameStrBuffer);
    virtual void ClosePane();

    bool GetEditorCaption(wchar_t*& pOutBuffer);

    void LoadSession();

    /// Overrides base class implementation:
    virtual void OnShow();
protected:

    /// This function is called when the content of the window is created. It should be implemented by the specific classes:
    virtual QWidget* CreateView();
protected:

    // As for now there is one and only one event handler per vscProfileSessionEditorDocument instance
    // (only one owner), so there is no need for a vector.
    IProfileSessionEditorDocVsCoreImplOwner* m_pOwner;
};

void* vscProfileSessionEditorDocument_CreateInstance();
void vscProfileSessionEditorDocument_SetVSCOwner(void* pVscInstance, IProfileSessionEditorDocVsCoreImplOwner* handler);
void vscProfileSessionEditorDocument_LoadSession(void* pVscInstance);
// void vscProfileSessionEditorDocument_GetEditorCaption(void* pVscInstance, wchar_t*& pOutBuffer);

#endif // __vscProfileSessionEditorDocument_h
