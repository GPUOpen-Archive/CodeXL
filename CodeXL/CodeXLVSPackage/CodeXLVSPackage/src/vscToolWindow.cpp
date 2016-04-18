//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file vscToolWindow.cpp
///
//==================================================================================

#include "StdAfx.h"

// Qt:
#include <QtWidgets>

#include <Include/Public/vscToolWindow.h>

// Local (core):
#include <Include/vscCoreInternalUtils.h>

// VS:
#include <src/vspQTWindowPaneImpl.h>
#include <Include/../../CodeXLVSPackageUi/CommandIds.h>
#include <CodeXLVSPackage/Include/vspCommandIDs.h>
#include <Include/vspStringConstants.h>
#include <stdidcmd.h>

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTApplicationComponents/Include/acFunctions.h>
#include <AMDTOSWrappers/Include/osDebuggingFunctions.h>
#include <AMDTOSWrappers/Include/osDebugLog.h>
#include <AMDTApplicationFramework/Include/afGlobalVariablesManager.h>
#include <AMDTApplicationFramework/Include/views/afPropertiesView.h>
#include <AMDTGpuDebuggingComponents/Include/gdDebugApplicationTreeHandler.h>
#include <AMDTGpuDebuggingComponents/Include/views/gdAPICallsHistoryView.h>
#include <AMDTGpuDebuggingComponents/Include/views/gdMemoryView.h>
#include <AMDTGpuDebuggingComponents/Include/views/gdStateVariablesView.h>
#include <AMDTGpuDebuggingComponents/Include/views/gdStatisticsView.h>
#include <AMDTGpuDebuggingComponents/Include/views/gdImageAndBufferView.h>
#include <AMDTGpuDebuggingComponents/Include/views/gdMultiWatchView.h>
#include <AMDTGpuDebuggingComponents/Include/gdStatisticsPanel.h>
#include <src/vspWindowsManager.h>

typedef void(*SHOWCALLBACKPROC)(void*);

gtVector<vscToolWindowData*> g_WindowData;

void vscToolWindow::OnFrameShow(bool isFrameShown, int gdWindowCommandID)
{

    // If the memory viewer is closed, update the tree that no memory update should be performed:
    if (gdWindowCommandID == ID_MEMORY_ANALYSIS_VIEWER)
    {
        // Get the memory view object:
        gdMemoryView* pMemoryView = vspWindowsManager::instance().memoryView(nullptr, QSize(-1, -1));
        GT_IF_WITH_ASSERT(pMemoryView != nullptr)
        {
            // Update the view:
            bool rc = pMemoryView->updateView(isFrameShown);
            GT_ASSERT(rc);
        }
    }

    // If the statistics view is closed, update the tree that no memory update should be performed:
    if (gdWindowCommandID == ID_STATISTICS_VIEW)
    {
        // Get the statistics view object:
        gdStatisticsPanel* pStatisticsPanel = vspWindowsManager::instance().statisticsPanel(nullptr, QSize(-1, -1));
        GT_IF_WITH_ASSERT(pStatisticsPanel != nullptr)
        {
            // Get the currently selected context:
            apContextID currentContext;
            gdDebugApplicationTreeHandler::instance()->selectedContext(currentContext);

            gdStatisticsView* pStatisticsView = pStatisticsPanel->statisticsView();
            GT_IF_WITH_ASSERT(pStatisticsView != nullptr)
            {
                // Display the context in the statistics view:
                pStatisticsView->displayContext(currentContext, isFrameShown);
            }
        }
    }

    // If it is a multi watch view, update its shown / hidden status:
    else if ((gdWindowCommandID >= ID_MULTIWATCH_VIEW_FIRST) && (gdWindowCommandID <= ID_MULTIWATCH_VIEW3))
    {
        vspWindowsManager::instance().updateMultiwatchViewShowStatus(gdWindowCommandID, isFrameShown);
    }

}

void vscToolWindow::OnExecuteEditCommand(int senderId)
{
    // Sanity check:
    GT_IF_WITH_ASSERT(_pImpl != nullptr)
    {
        // Get the base view pointer:
        afBaseView* pBaseView = _pImpl->baseView();
        GT_IF_WITH_ASSERT(pBaseView != nullptr)
        {
            // Sanity check:
            GT_IF_WITH_ASSERT(_pImpl != nullptr)
            {
                // Get the base view pointer:
                afBaseView* pBaseView = _pImpl->baseView();

                // Sanity check:
                GT_IF_WITH_ASSERT(pBaseView != nullptr)
                {
                    // Perform the operation according to the command id:
                    switch (senderId)
                    {
                        case cmdidCopy:
                            pBaseView->onEdit_Copy();
                            break;

                        case cmdidSelectAll:
                            pBaseView->onEdit_SelectAll();
                            break;

                        default:
                            GT_ASSERT_EX(false, L"Got into onExecuteEditCommand with unsupported command id");
                            break;
                    }
                }
            }
        }
    }
}

void vscToolWindow::OnUpdateEditCommand(bool& isEnabled, bool& isFoundCommandHandler, int cmdId)
{
    // Sanity check:
    GT_IF_WITH_ASSERT(_pImpl != nullptr)
    {
        // Get the base view pointer:
        afBaseView* pBaseView = _pImpl->baseView();
        GT_IF_WITH_ASSERT(pBaseView != nullptr)
        {
            isEnabled = false;
            isFoundCommandHandler = true;

            // Perform the operation according to the command id:
            switch (cmdId)
            {
                case cmdidCopy:
                    pBaseView->onUpdateEdit_Copy(isEnabled);
                    break;

                case cmdidSelectAll:
                    pBaseView->onUpdateEdit_SelectAll(isEnabled);
                    break;

                case cmdidFind:
                    pBaseView->onUpdateEdit_Find(isEnabled);
                    break;

                case cmdidFindNext:
                    pBaseView->onUpdateEdit_FindNext(isEnabled);
                    break;

                default:
                    isFoundCommandHandler = false;
                    break;
            }
        }
    }
}

void vscToolWindow::vscToolWindow_CreatePaneWindow(HWND hwndParent, int x, int y, int cx, int cy, HWND* phWND, int gdWindowCommandID)
{
    // Check if this window should be top window:
    bool needsTimer = (gdWindowCommandID == ID_OBJECT_NAVIGATION_TREE ||
                       gdWindowCommandID == ID_STATISTICS_VIEW ||
                       gdWindowCommandID == ID_CALLS_HISTORY_LIST);


    // Create the QT widget implementation:
    _pImpl = new vspQTWindowPaneImpl(hwndParent, x, y, cx, cy, needsTimer);

    // Create a QT window using our pane impl:
    bool rcHandleCreateWindow = CreateQtPaneWindow(gdWindowCommandID);
    GT_ASSERT(rcHandleCreateWindow);

    // Get the created widget from the window implementation:
    QWidget* pWidget = _pImpl->widget();
    GT_IF_WITH_ASSERT(pWidget != nullptr)
    {
        // Set the HWND handle:
        *phWND = (HWND)pWidget->winId();
    }

    // Add a connection member
    // so all keyboard events will be received by it (BUG 78)
    QWidget* pCreated = _pImpl->createdQTWidget();
    // Check that the created widget is not already in the global window data
    bool foundData = false;

    for (unsigned int nData = 0; nData < g_WindowData.size(); nData++)
    {
        vscToolWindowData* pCurrentData = g_WindowData[nData];
        GT_IF_WITH_ASSERT(nullptr != pCurrentData)
        {
            if (pCurrentData->m_pActivationWidget == pCreated)
            {
                foundData = true;
            }
        }
    }

    // If not found add a new data
    if (!foundData)
    {
        vscToolWindowData* pNewData = new vscToolWindowData;
        pNewData->m_pActivationWidget = pCreated;
        pNewData->m_pWrapper = pWidget;
        pNewData->m_pToolWindow = (void*)this;
        pNewData->m_pVsWindowFrame = nullptr;
        pNewData->m_pShowFunction = nullptr;
        g_WindowData.push_back(pNewData);
    }
}

bool vscToolWindow::CreateQtPaneWindow(int gdWindowCommandID)
{
    bool retVal = false;
    // Sanity check:
    GT_IF_WITH_ASSERT(_pImpl != nullptr)
    {
        // Get the parent widget:
        QWidget* pParentWidget = _pImpl->widget();
        GT_IF_WITH_ASSERT(pParentWidget != nullptr)
        {
            // Get the parent size:
            QSize parentWidgetSize = pParentWidget->size();

            switch (gdWindowCommandID)
            {

                case ID_MULTIWATCH_VIEW1:
                case ID_MULTIWATCH_VIEW2:
                case ID_MULTIWATCH_VIEW3:
                {
                    // Create the properties view:
                    gdMultiWatchView* pMultiWatchView = vspWindowsManager::instance().multiwatchView(pParentWidget, parentWidgetSize, gdWindowCommandID);
                    GT_IF_WITH_ASSERT(pMultiWatchView != nullptr)
                    {
                        // Set the created window:
                        _pImpl->setQTCreateWindow(pMultiWatchView);
                        retVal = true;
                    }
                    break;
                }

                case ID_OBJECT_NAVIGATION_TREE:
                {
                    // Create a monitored object tree:
                    afApplicationTree* pObjectsTree = vspWindowsManager::instance().monitoredObjectsTree(pParentWidget, parentWidgetSize);
                    GT_IF_WITH_ASSERT(pObjectsTree)
                    {
                        // Set the created window:
                        _pImpl->setQTCreateWindow(pObjectsTree);
                        retVal = true;
                    }

                    break;
                }


                case ID_MEMORY_ANALYSIS_VIEWER:
                {
                    // Create the properties view:
                    gdMemoryView* pMemoryView = vspWindowsManager::instance().memoryView(pParentWidget, parentWidgetSize);
                    GT_IF_WITH_ASSERT(pMemoryView != nullptr)
                    {
                        // Set the created window:
                        _pImpl->setQTCreateWindow((QWidget*)pMemoryView);
                        retVal = true;
                    }
                    break;
                }

                case ID_STATE_VARIABLES_VIEW:
                {
                    // Create the properties view:
                    gdStateVariablesView* pStateVariablesView = vspWindowsManager::instance().stateVariablesView(pParentWidget, parentWidgetSize);
                    GT_IF_WITH_ASSERT(pStateVariablesView != nullptr)
                    {
                        // Set the created window:
                        _pImpl->setQTCreateWindow((QWidget*)pStateVariablesView);
                        retVal = true;
                    }
                    break;
                }


                case ID_PROPERTIES_VIEW:
                {
                    // Create the properties view:
                    afPropertiesView* pPropertiesView = vspWindowsManager::instance().propertiesView(pParentWidget, parentWidgetSize);
                    GT_IF_WITH_ASSERT(pPropertiesView != nullptr)
                    {
                        // Set the created window:
                        _pImpl->setQTCreateWindow((QWidget*)pPropertiesView);
                        retVal = true;
                    }
                    break;
                }

                case ID_STATISTICS_VIEW:
                {
                    // Create the properties view:
                    gdStatisticsPanel* pStatisticsPanel = vspWindowsManager::instance().statisticsPanel(pParentWidget, parentWidgetSize);
                    GT_IF_WITH_ASSERT(pStatisticsPanel != nullptr)
                    {
                        // Set the created window:
                        _pImpl->setQTCreateWindow((QWidget*)pStatisticsPanel);
                        retVal = true;
                    }
                    break;
                }

                case ID_CALLS_HISTORY_LIST:
                {
                    // Create the calls history view:
                    gdAPICallsHistoryPanel* pCallsHistoryPanel = vspWindowsManager::instance().callsHistoryPanel(pParentWidget, parentWidgetSize);
                    GT_IF_WITH_ASSERT(pCallsHistoryPanel != nullptr)
                    {
                        // Set the created window:
                        _pImpl->setQTCreateWindow((QWidget*)pCallsHistoryPanel);
                        retVal = true;
                    }
                    break;
                }

                default:
                    GT_ASSERT_EX(false, VSP_STR_UnsupportedGDWindowCommandID);
                    break;
            }
        }
    }

    return retVal;
}

void vscToolWindow::SetToolShowFunction(void* pToolWindow, void* pVspToolWindow, void* pShowFunction)
{
    GT_IF_WITH_ASSERT(pToolWindow != nullptr && pVspToolWindow != nullptr && pShowFunction != nullptr)
    {
        for (unsigned int nData = 0; nData < g_WindowData.size(); nData++)
        {
            vscToolWindowData* pCurrentData = g_WindowData[nData];
            GT_IF_WITH_ASSERT(nullptr != pCurrentData)
            {
                if (pCurrentData->m_pToolWindow == pToolWindow)
                {
                    pCurrentData->m_pVsWindowFrame = pVspToolWindow;
                    pCurrentData->m_pShowFunction = pShowFunction;
                    break;
                }
            }
        }
    }
}

void vscToolWindow::CallToolShowFunction(QWidget* pActiveWidget)
{
    for (unsigned int nData = 0; nData < g_WindowData.size(); nData++)
    {
        vscToolWindowData* pCurrentData = g_WindowData[nData];
        GT_IF_WITH_ASSERT(nullptr != pCurrentData)
        {
            QWidget* pCurrent = pActiveWidget;

            while (pCurrent != nullptr)
            {
                if (pCurrentData->m_pWrapper == pCurrent)
                {
                    if (pCurrentData->m_pVsWindowFrame != nullptr)
                    {
                        if (nullptr != pCurrentData->m_pShowFunction)
                        {
                            SHOWCALLBACKPROC pCallBackFunction = (SHOWCALLBACKPROC)pCurrentData->m_pShowFunction;
                            pCallBackFunction(pCurrentData->m_pVsWindowFrame);
                        }
                    }

                    break;
                }

                pCurrent = pCurrent->parentWidget();
            }
        }
    }
}

bool vscToolWindow_CreateQTPaneWindow(void* pVscInstance, int gdWindowCommandID)
{
    bool ret = false;
    vscToolWindow* pInstance = (vscToolWindow*)pVscInstance;
    GT_IF_WITH_ASSERT(pInstance != nullptr)
    {
        ret = pInstance->CreateQtPaneWindow(gdWindowCommandID);
    }
    return ret;
}

void vscToolWindow_CreatePaneWindow(void* pVscInstance, HWND hwndParent, int x, int y, int cx, int cy, HWND* phWND, int gdWindowCommandID)
{
    vscToolWindow* pInstance = (vscToolWindow*)pVscInstance;
    GT_IF_WITH_ASSERT(pInstance != nullptr)
    {
        pInstance->vscToolWindow_CreatePaneWindow(hwndParent, x, y, cx, cy, phWND, gdWindowCommandID);
    }
}

void* vscToolWindow__CreateInstance()
{
    return new vscToolWindow();
}

void vscToolWindow__DestroyInstance(void*& pInstance)
{
    delete pInstance;
    pInstance = nullptr;
}

void vscToolWindow_OnUpdateEditCommand(void* pVscInstance, bool& isEnabled, bool& isFoundCommandHandler, int cmdId)
{
    vscToolWindow* pInstance = (vscToolWindow*)pVscInstance;
    GT_IF_WITH_ASSERT(pInstance != nullptr)
    {
        pInstance->OnUpdateEditCommand(isEnabled, isFoundCommandHandler, cmdId);
    }
}

void vscToolWindow_OnExecuteEditCommand(void* pVscInstance, int senderId)
{
    vscToolWindow* pInstance = (vscToolWindow*)pVscInstance;
    GT_IF_WITH_ASSERT(pInstance != nullptr)
    {
        pInstance->OnExecuteEditCommand(senderId);
    }
}

void vscToolWindow_OnFrameShow(void* pVscInstance, bool isFrameShown, int gdWindowCommandID)
{
    vscToolWindow* pInstance = (vscToolWindow*)pVscInstance;
    GT_IF_WITH_ASSERT(pInstance != nullptr)
    {
        pInstance->OnFrameShow(isFrameShown, gdWindowCommandID);
    }
}

void vscToolWindow_GetVersionCaption(wchar_t*& pCaptionBuffer)
{
    gtString tmp = afGlobalVariablesManager::instance().versionCaption();
    pCaptionBuffer = vscAllocateAndCopy(tmp);
}

void vscToolWindow_SetToolShowFunction(void* pToolWindow, void* pVspToolWindow, void* pShowFunction)
{
    vscToolWindow::SetToolShowFunction(pToolWindow, pVspToolWindow, pShowFunction);
}

