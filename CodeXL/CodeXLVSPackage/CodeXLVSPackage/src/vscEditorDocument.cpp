//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file vscEditorDocument.cpp
///
//==================================================================================

#include "stdafx.h"

// Qt:
//#include <QtGui>
#include <QtWidgets>

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTBaseTools/Include/gtStringTokenizer.h>
#include <AMDTOSWrappers/Include/osDebuggingFunctions.h>
#include <AMDTApiFunctions/Include/gaGRApiFunctions.h>
#include <AMDTApplicationFramework/Include/afAppStringConstants.h>
#include <AMDTApplicationFramework/Include/afTreeItemType.h>
#include <AMDTApplicationFramework/Include/views/afApplicationTreeItemData.h>
#include <AMDTApplicationFramework/Include/views/afBaseView.h>
#include <AMDTApplicationComponents/Include/acFunctions.h>
#include <AMDTKernelAnalyzer/src/kaApplicationTreeHandler.h>
#include <AMDTKernelAnalyzer/src/kaOverviewView.h>
#include <AMDTKernelAnalyzer/src/kaKernelView.h>
#include <AMDTKernelAnalyzer/src/kaProjectDataManager.h>
#include <AMDTKernelAnalyzer/src/kaTreeDataExtension.h>
#include <AMDTKernelAnalyzer/include/kaStringConstants.h>

// Local:
#include <Include/Public/vscEditorDocument.h>
#include <Include/vscCoreInternalUtils.h>
#include <Include/Public/vscKernelAnalyzerEditorDocument.h>
#include <src/vspKernelAnalyzerEditorManager.h>
#include <src/vspQTWindowPaneImpl.h>

QVector<vscEditorDocument*> vscEditorDocument::m_stDocumentsVector;

vscEditorDocument::vscEditorDocument() :
    m_pImpl(NULL), m_pInnerView(NULL), m_refCount(1)
{
    m_stDocumentsVector.push_back(this);
}

vscEditorDocument::~vscEditorDocument()
{
    int index = m_stDocumentsVector.indexOf(this);
    GT_IF_WITH_ASSERT(index != -1)
    {
        m_stDocumentsVector.removeAt(index);
    }
}


void vscEditorDocument::CreatePaneWindow(HWND hWndParent, int x, int y, int cx, int cy, HWND* phWnd)
{
    // Create the WX window implementation:
    m_pImpl = new vspQTWindowPaneImpl(hWndParent, x, y, cx, cy, true);

    // Return the output HWND:
    QWidget* pParentWidget = m_pImpl->widget();
    QWidget* pCreatedWindow = NULL;
    GT_IF_WITH_ASSERT(NULL != pParentWidget)
    {
        // Set my output window handle:
        *phWnd = reinterpret_cast<HWND>(pParentWidget->winId());

        pCreatedWindow = CreateView();

        if (pCreatedWindow != NULL)
        {
            // Set the created window:
            m_pImpl->setQTCreateWindow(pCreatedWindow);

            // Set my size:
            QSize parentPanelSize = pParentWidget->size();
            pCreatedWindow->resize(parentPanelSize);

            pParentWidget->update();

            *phWnd = reinterpret_cast<HWND>(pParentWidget->winId());
            pParentWidget->update();
        }
    }
}

void vscEditorDocument::ClosePane()
{
    if (m_pImpl != nullptr)
    {
        delete m_pImpl;
        m_pImpl = nullptr;
    }
}

void vscEditorDocument::OnShow()
{
    // Placeholder - nothing to do here
}

void vscEditorDocument::OnUpdateEdit_Copy(bool& isCopyPossible)
{
    GT_IF_WITH_ASSERT((m_pImpl != nullptr) && (m_pImpl->baseView() != nullptr))
    {
        m_pImpl->baseView()->onUpdateEdit_Copy(isCopyPossible);
    }
}

void vscEditorDocument::OnUpdateEdit_SelectAll(bool& isSelectAllPossible)
{
    GT_IF_WITH_ASSERT((m_pImpl != nullptr) && (m_pImpl->baseView() != nullptr))
    {
        m_pImpl->baseView()->onUpdateEdit_SelectAll(isSelectAllPossible);
    }
}

void vscEditorDocument::OnSelectAll()
{
    GT_IF_WITH_ASSERT((m_pImpl != nullptr) && (m_pImpl->baseView() != nullptr))
    {
        m_pImpl->baseView()->onEdit_SelectAll();
    }
}

void vscEditorDocument::OnCopy()
{
    GT_IF_WITH_ASSERT((m_pImpl != nullptr) && (m_pImpl->baseView() != nullptr))
    {
        m_pImpl->baseView()->onEdit_Copy();
    }
}

QWidget* vscEditorDocument::CreateView()
{
    GT_ASSERT_EX(false, L"Should not get here");
    return nullptr;
}

void vscEditorDocument::LoadDocData(const wchar_t* filePathStr)
{
    GT_UNREFERENCED_PARAMETER(filePathStr);
}

void vscEditorDocument::SetEditorCaption(const wchar_t* filePathStr, wchar_t*& itemNameStrBuffer)
{
    GT_UNREFERENCED_PARAMETER(filePathStr);
    GT_UNREFERENCED_PARAMETER(itemNameStrBuffer);
}

void vscEditorDocument::Release()
{
    m_refCount--;

    if (m_refCount <= 0)
    {
        delete this;
    }
}

void vscEditorDocument_CreatePaneWindow(void* pVscInstance, HWND hWndParent, int x, int y, int cx, int cy, HWND* phWnd)
{
    vscEditorDocument* pInstance = (vscEditorDocument*)pVscInstance;
    GT_IF_WITH_ASSERT(pInstance != NULL)
    {
        pInstance->CreatePaneWindow(hWndParent, x, y, cx, cy, phWnd);
    }
}

void vscEditorDocument_LoadDocData(void* pVscInstance, const wchar_t* filePathStr)
{
    vscEditorDocument* pInstance = (vscEditorDocument*)pVscInstance;
    GT_IF_WITH_ASSERT(pInstance != NULL)
    {
        pInstance->LoadDocData(filePathStr);
    }
}

void vscEditorDocument_SetEditorCaption(void* pVscInstance, const wchar_t* filePathStr, wchar_t*& itemNameStrBuffer)
{
    vscEditorDocument* pInstance = (vscEditorDocument*)pVscInstance;
    GT_IF_WITH_ASSERT(pInstance != NULL)
    {
        pInstance->SetEditorCaption(filePathStr, itemNameStrBuffer);
    }
}

void vscEditorDocument_ClosePane(void* pVscInstance)
{
    vscEditorDocument* pInstance = (vscEditorDocument*)pVscInstance;
    GT_IF_WITH_ASSERT(pInstance != NULL)
    {
        pInstance->ClosePane();
        pInstance->vscEditorDocument::ClosePane();
        // once it is done closing delete it
        pInstance->Release();
    }
}

void vscEditorDocument_OnShow(void* pVscInstance)
{
    vscEditorDocument* pInstance = (vscEditorDocument*)pVscInstance;
    GT_IF_WITH_ASSERT(pInstance != NULL)
    {
        pInstance->OnShow();
    }
}

void vscEditorDocument_OnUpdateEdit_Copy(void* pVscInstance, bool& isCopyPossilble)
{
    vscEditorDocument* pInstance = (vscEditorDocument*)pVscInstance;
    GT_IF_WITH_ASSERT(pInstance != NULL)
    {
        pInstance->OnUpdateEdit_Copy(isCopyPossilble);
    }
}

void vscEditorDocument_OnUpdateEdit_SelectAll(void* pVscInstance, bool& isSelectAllPossible)
{
    vscEditorDocument* pInstance = (vscEditorDocument*)pVscInstance;
    GT_IF_WITH_ASSERT(pInstance != NULL)
    {
        pInstance->OnUpdateEdit_SelectAll(isSelectAllPossible);
    }
}

void vscEditorDocument_SelectAll(void* pVscInstance)
{
    vscEditorDocument* pInstance = (vscEditorDocument*)pVscInstance;
    GT_IF_WITH_ASSERT(pInstance != NULL)
    {
        pInstance->OnSelectAll();
    }
}


void vscEditorDocument_Copy(void* pVscInstance)
{
    vscEditorDocument* pInstance = (vscEditorDocument*)pVscInstance;
    GT_IF_WITH_ASSERT(pInstance != NULL)
    {
        pInstance->OnCopy();
    }
}


