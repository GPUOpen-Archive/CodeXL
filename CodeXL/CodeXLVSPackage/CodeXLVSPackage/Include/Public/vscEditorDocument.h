//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file vscEditorDocument.h
///
//==================================================================================

#ifndef vscEditorDocument_h__
#define vscEditorDocument_h__

// Infra:
#include <AMDTOSWrappers/Include/osFilePath.h>
#include <QVector>

#include "src\CodeXLVSPackageCoreDefs.h"

class vspQTWindowPaneImpl;
class QWidget;

/// ----------------------------------------------------------------------------------
/// Class Name:    vscEditorDocument
///                This class is base class for the vscEditorDocument
///                vsc: the code implementations. EditorDocument: the terminology used for classes implementing the MDI creation
///                and management in VS. This class is inherited for each plug-in, and anything specific for the plug ins, should
///                be implemented in derived classes
/// ----------------------------------------------------------------------------------
class vscEditorDocument
{
public:
    vscEditorDocument();
    virtual ~vscEditorDocument();

    /// Creates a Qt window, and put into the VS wrapping:
    virtual void CreatePaneWindow(HWND hWndParent, int x, int y, int cx, int cy, HWND* phWnd);
    virtual void LoadDocData(const wchar_t* filePathStr);
    virtual void SetEditorCaption(const wchar_t* filePathStr, wchar_t*& itemNameStrBuffer);
    virtual void ClosePane();
    virtual void OnShow();
    virtual void OnUpdateEdit_Copy(bool& isCopyPossible);
    virtual void OnUpdateEdit_SelectAll(bool& isSelectAllPossible);
    virtual void OnSelectAll();
    virtual void OnCopy();

    /// return the inner widget
    QWidget* InnerWidget() { return m_pInnerView; }

    /// returns the implementation pane
    vspQTWindowPaneImpl* ImplPane() { return m_pImpl; }

    /// static function to get the open documents
    static QVector<vscEditorDocument*>& OpenDocuments() { return m_stDocumentsVector; }

    /// reference counter for correct delete
    void Retain() { m_refCount++; }
    void Release();

protected:

    /// This function is called when the content of the window is created. It should be implemented by the specific classes:
    virtual QWidget* CreateView();

protected:

    // Contain the setMyWindowCommandID instance (is used for integrating CodeXL window within a VS pane:
    vspQTWindowPaneImpl* m_pImpl;

    QWidget* m_pInnerView;

    osFilePath m_filePath;

    // map that holds the created vscEditorDocument for fast access when looking a specific document
    static QVector<vscEditorDocument*> m_stDocumentsVector;

    /// reference counter for delete
    int m_refCount;
};

void vscEditorDocument_CreatePaneWindow(void* pVscInstance, HWND hWndParent, int x, int y, int cx, int cy, HWND* phWnd);
void vscEditorDocument_LoadDocData(void* pVscInstance, const wchar_t* filePathStr);
void vscEditorDocument_SetEditorCaption(void* pInstance, const wchar_t* filePathStr, wchar_t*& itemNameStrBuffer);
void vscEditorDocument_ClosePane(void* pVscInstance);
void vscEditorDocument_OnShow(void* pVscInstance);
void vscEditorDocument_OnUpdateEdit_Copy(void* pVscInstance, bool& isCopyPossilble);
void vscEditorDocument_Copy(void* pVscInstance);
void vscEditorDocument_OnUpdateEdit_SelectAll(void* pVscInstance, bool& isSelectAllPossible);
void vscEditorDocument_SelectAll(void* pVscInstance);

#endif // vscEditorDocument_h__