//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file vscToolWindow.h
///
//==================================================================================

#ifndef vscToolWindow_h__
#define vscToolWindow_h__
#include "CodeXLVSPackageCoreDefs.h"
#include <Windef.h>

void* vscToolWindow__CreateInstance();

void vscToolWindow__DestroyInstance(void*& pInstance);

bool vscToolWindow_CreateQTPaneWindow(void* pVscInstance, int gdWindowCommandID);

void vscToolWindow_CreatePaneWindow(void* pVscInstance, HWND hwndParent, int x, int y, int cx, int cy, HWND* phWND, int gdWindowCommandID);

void vscToolWindow_OnUpdateEditCommand(void* pVscInstance, bool& isEnabled, bool& isFoundCommandHandler, int cmdId);

void vscToolWindow_OnExecuteEditCommand(void* pVscInstance, int senderId);

void vscToolWindow_OnFrameShow(void* pVscInstance, bool isFrameShown, int gdWindowCommandID);

void vscToolWindow_GetVersionCaption(wchar_t*& pCaptionBuffer);

void vscToolWindow_SetToolShowFunction(void* pToolWindow, void* pVspToolWindow, void* pShowFunction);

class QWidget;

struct vscToolWindowData
{
public:
    QWidget* m_pActivationWidget;
    QWidget* m_pWrapper;
    void*    m_pToolWindow;
    void*    m_pVsWindowFrame;
    void*    m_pShowFunction;
};

class vspQTWindowPaneImpl;

class vscToolWindow
{
public:
    vscToolWindow() : _pImpl(NULL) {}
    ~vscToolWindow() {}

    void OnFrameShow(bool isFrameShown, int gdWindowCommandID);

    void OnExecuteEditCommand(int senderId);

    void OnUpdateEditCommand(bool& isEnabled, bool& isFoundCommandHandler, int cmdId);

    void vscToolWindow_CreatePaneWindow(HWND hwndParent, int x, int y, int cx, int cy, HWND* phWND, int gdWindowCommandID);

    bool CreateQtPaneWindow(int gdWindowCommandID);

    /// Sets the show call back function for a specific pane window
    static void SetToolShowFunction(void* pToolWindow, void* pVspToolWindow, void* pShowFunction);

    /// Call the show function from the vsptoolwindow associated with the pActiveWidget if there is one
    static void CallToolShowFunction(QWidget* pActiveWidget);

    // Contain the setMyWindowCommandID instance (is used for integrating CodeXL window
    // within a VS pane:
    vspQTWindowPaneImpl* _pImpl;
};

#endif // vscToolWindow_h__
