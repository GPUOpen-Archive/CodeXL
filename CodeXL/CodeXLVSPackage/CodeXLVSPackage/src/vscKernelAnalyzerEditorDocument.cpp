//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file vscKernelAnalyzerEditorDocument.cpp
///
//==================================================================================

#include "stdafx.h"
#include <Include/Public/vscKernelAnalyzerEditorDocument.h>
#include <Include/vscCoreInternalUtils.h>

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

// Kernel analyzer
#include <AMDTKernelAnalyzer/src/kaApplicationTreeHandler.h>
#include <AMDTKernelAnalyzer/src/kaOverviewView.h>
#include <AMDTKernelAnalyzer/src/kaKernelView.h>
#include <AMDTKernelAnalyzer/src/kaProjectDataManager.h>
#include <AMDTKernelAnalyzer/src/kaTreeDataExtension.h>
#include <AMDTKernelAnalyzer/include/kaStringConstants.h>

// Local:
#include <Include/Public/vscKernelAnalyzerEditorDocument.h>
#include <src/vspKernelAnalyzerEditorManager.h>
#include <src/vspQTWindowPaneImpl.h>

vscKernelAnalyzerEditorDocument::vscKernelAnalyzerEditorDocument() : vscEditorDocument()
{

}

vscKernelAnalyzerEditorDocument::~vscKernelAnalyzerEditorDocument()
{

}

void vscKernelAnalyzerEditorDocument::ClosePane()
{
    GT_IF_WITH_ASSERT(m_pImpl != nullptr)
    {
        kaKernelView* pView = qobject_cast<kaKernelView*>(m_pImpl->createdQTWidget());

        if (pView != NULL)
        {
            pView->CloseAllTabs();

            vspKernelAnalyzerEditorManager::instance().removeKernelView(pView);

            // The view is not a child of the VS view so it needs to be deleted:
            pView->deleteLater();
        }
    }
}

void vscKernelAnalyzerEditorDocument::OnShow()
{
    GT_IF_WITH_ASSERT(m_pImpl != nullptr)
    {
        vspKernelAnalyzerEditorManager::instance().setActiveView(m_pImpl->createdQTWidget());
    }
}

QWidget* vscKernelAnalyzerEditorDocument::CreateView()
{
    kaKernelView* pRetVal = nullptr;
    GT_IF_WITH_ASSERT(m_pImpl != nullptr)
    {

        pRetVal = new kaKernelView(m_pImpl->widget());

        if (pRetVal != NULL)
        {
            vspKernelAnalyzerEditorManager::instance().addKernelView(pRetVal);
            vspKernelAnalyzerEditorManager::instance().setActiveView(pRetVal);
        }
    }
    return pRetVal;
}

void vscKernelAnalyzerEditorDocument::SetEditorCaption(const wchar_t* filePathStr, wchar_t*& itemNameStrBuffer)
{
    itemNameStrBuffer = NULL;

    osFilePath filePath(filePathStr);
    osFilePath dataFilePath;
    osFile dataFile;
    dataFile.open(filePath, osChannel::OS_UNICODE_TEXT_CHANNEL, osFile::OS_OPEN_TO_READ);
    gtString dataFileAsString;
    dataFile.readIntoString(dataFileAsString);
    dataFile.close();
    // get the first line from the file
    gtString firstLine;
    gtStringTokenizer firstLineTokenizer(dataFileAsString, L"\n");
    firstLineTokenizer.getNextToken(firstLine);
    dataFilePath.setFullPathFromString(firstLine);

    gtString itemNameStr;
    // all captions have the file name and extension:
    dataFilePath.getFileNameAndExtension(itemNameStr);

    gtString fileTypeExt;
    filePath.getFileExtension(fileTypeExt);

    if (fileTypeExt == KA_STR_overviewExtension)
    {
        itemNameStr.append(KA_STR_overViewTitle);
    }
    else
    {
        itemNameStr.append(L" ");
        // get kernel name:
        osDirectory filePathDir;
        filePath.getFileDirectory(filePathDir);
        gtString filePathDirAsStr = filePathDir.directoryPath().asString();
        int lastPath = filePathDirAsStr.reverseFind(osFilePath::osPathSeparator);
        gtString kernelName;
        filePathDirAsStr.getSubString(lastPath + 1, filePathDirAsStr.length(), kernelName);
        itemNameStr.append(kernelName);

        // Allocate and copy the output string.
        itemNameStrBuffer = vscAllocateAndCopy(itemNameStr);
    }

    kaKernelView* pKernelView = qobject_cast<kaKernelView*>(vspKernelAnalyzerEditorManager::instance().activeView());

    if (pKernelView != NULL)
    {
        pKernelView->setDataFile(filePath);

        if (fileTypeExt == KA_STR_overviewExtension)
        {
            pKernelView->displayFile(filePath, filePath, AF_TREE_ITEM_KA_OVERVIEW);
        }
        else if (fileTypeExt == KA_STR_kernelViewExtension)
        {
            // Load the file path to the view. If the data for the project is not ready yet, vspKernelAnalyzerEditorManager will
            // add the view for late load list of views
            vspKernelAnalyzerEditorManager::instance().LoadDataForFilePath(pKernelView, filePath);
        }
    }
}

void* vscKernelAnalyzerEditorDocument_CreateInstance()
{
    return new vscKernelAnalyzerEditorDocument();
}



