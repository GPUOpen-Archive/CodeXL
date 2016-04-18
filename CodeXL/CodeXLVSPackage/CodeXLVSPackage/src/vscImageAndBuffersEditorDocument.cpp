//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file vscImageAndBuffersEditorDocument.cpp
///
//==================================================================================

#include "stdafx.h"
#include <Include/Public/vscImageAndBuffersEditorDocument.h>
#include <Include/vscCoreInternalUtils.h>
#include "vspQTWindowPaneImpl.h"
#include "vspImagesAndBuffersManager.h"
#include "vspWindowsManager.h"

// Consider putting these header files in a place visible both to the VS code and to the core code
// (maybe in Include/Public)
#include "guids.h"
#include <CodeXLVSPackageUi/CommandIds.h>

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTOSWrappers/Include/osDebuggingFunctions.h>
#include <AMDTApiFunctions/Include/gaGRApiFunctions.h>

// AMDTApplicationFramework:
#include <AMDTApplicationFramework/Include/views/afApplicationTree.h>

// AMDTGpuDebuggingComponents:
#include <AMDTGpuDebuggingComponents/Include/gdHTMLProperties.h>
#include <AMDTGpuDebuggingComponents/Include/gdCommandIds.h>
#include <AMDTGpuDebuggingComponents/Include/gdImagesAndBuffersManager.h>
#include <AMDTGpuDebuggingComponents/Include/views/gdImageAndBufferView.h>



vscImageAndBuffersEditorDocument::vscImageAndBuffersEditorDocument() :
    m_pImageBufferView(NULL),
    m_pThumbnailView(NULL)
{

}

QWidget* vscImageAndBuffersEditorDocument::CreateView()
{
    // Will hold the created CodeXL window:
    QWidget* pRetVal = CreateWindowByObjectType(AF_TREE_ITEM_ITEM_NONE);
    return pRetVal;
}

QWidget* vscImageAndBuffersEditorDocument::CreateWindowByObjectType(afTreeItemType itemType)
{
    QWidget* pRetVal = NULL;

    // Sanity check:
    GT_IF_WITH_ASSERT(m_pImpl != NULL)
    {
        QWidget* pParentWidget = m_pImpl->widget();

        // Get the panel from the implementation window:
        GT_IF_WITH_ASSERT(pParentWidget != NULL)
        {
            // If we do not have an object type, get the activated object from tree:
            if (itemType == AF_TREE_ITEM_ITEM_NONE)
            {
                // Get the monitored object tree:
                afApplicationTree* pObjectsTree = vspWindowsManager::instance().monitoredObjectsTree(NULL, QSize(-1, -1));

                if (pObjectsTree != NULL)
                {
                    // Get the last activated item data:
                    afApplicationTreeItemData* pActivatedItemData = gdDebugApplicationTreeHandler::instance()->activatedItemData();

                    if (pActivatedItemData != NULL)
                    {
                        // Get the activated item type:
                        itemType = pActivatedItemData->m_itemType;
                    }
                }
            }

            if (itemType != AF_TREE_ITEM_ITEM_NONE)
            {
                // Get the parent panel size:
                QSize parentSize = pParentWidget->size();

                if (afApplicationTreeItemData::isItemThumbnail(itemType))
                {
                    // Create a thumbnail view:
                    m_pThumbnailView = vspImagesAndBuffersManager::instance().createNewThumbnailView(pParentWidget, parentSize);
                    GT_IF_WITH_ASSERT(m_pThumbnailView)
                    {
                        pRetVal = (QWidget*)m_pThumbnailView;
                    }

                    // Clear the view:
                    m_pThumbnailView->clearView();

                }
                else if (afApplicationTreeItemData::isItemImageOrBuffer(itemType))
                {
                    // Create a image / buffer view:
                    m_pImageBufferView = vspImagesAndBuffersManager::instance().createNewImageAndBufferView(pParentWidget, parentSize);
                    GT_IF_WITH_ASSERT(m_pImageBufferView)
                    {
                        pRetVal = (QWidget*)m_pImageBufferView;
                    }

                    // Clear the view:
                    m_pImageBufferView->clearView();

                    // Display empty item:
                    m_pImageBufferView->displayItem(NULL);
                }
            }
        }

    }
    return pRetVal;
}

void vscImageAndBuffersEditorDocument::LoadDocData(const wchar_t* pszMkDocument)
{
    // Get the file path:
    osFilePath filePath(pszMkDocument);

    // Get the file name:
    gtString fileName;
    filePath.getFileName(fileName);

    // Check if the view was already created:

    // NOTICE: There are 2 scenarios:
    // 1. We get to CreatePaneWindow when the tree is initialized and updated, we get the item type from the tree selected item,
    //    and that's how we initialize the view.
    // 2. We get to CreatePaneWindow on solution load, and the file name for the editor is remembered by VS as loaded for this solution.
    //    In this case, we wait for LoadDocData to identify the file name and to deduce the view to create.
    bool wasViewCreated = ((m_pImageBufferView != NULL) || (m_pThumbnailView != NULL));

    if (!wasViewCreated)
    {
        // Get the item type from the item name:
        afApplicationTreeItemData objectID;
        gdDebugApplicationTreeData* pGDData = new gdDebugApplicationTreeData;

        objectID.setExtendedData(pGDData);
        int additionalParameter = -1;
        bool rcGetObjectDetails = gdHTMLProperties::htmlLinkToObjectDetails(fileName, objectID, additionalParameter);
        GT_IF_WITH_ASSERT(rcGetObjectDetails)
        {
            QWidget* pParentWidget = m_pImpl->widget();
            QWidget* pCreatedWindow = CreateWindowByObjectType(objectID.m_itemType);
            GT_IF_WITH_ASSERT((pCreatedWindow != NULL) && (pParentWidget != NULL))
            {
                // Set the created window:
                m_pImpl->setQTCreateWindow(pCreatedWindow);

                // Create a main sizer for the panel:
                QHBoxLayout* pSizer = new QHBoxLayout;


                pSizer->addWidget(pCreatedWindow);

                // Set the sizer for the panel, fit and layout:
                pParentWidget->setLayout(pSizer);

                // Set my size:
                QSize parentPanelSize = pParentWidget->size();
                pCreatedWindow->resize(parentPanelSize);

                pParentWidget->update();
            }
        }
    }

    // If this is an image / buffer single item:
    if (m_pImageBufferView != NULL)
    {
        // Display the item:
        bool rcDisplayItem = vspImagesAndBuffersManager::instance().displayItem(m_pImageBufferView, fileName);
        GT_ASSERT(rcDisplayItem);
    }
    else if (m_pThumbnailView != NULL)
    {
        // Display the item:
        bool rcDisplayItem = vspImagesAndBuffersManager::instance().displayThumbnailItem(m_pThumbnailView, fileName);
        GT_ASSERT(rcDisplayItem);
    }
}


void vscImageAndBuffersEditorDocument::SetEditorCaption(const wchar_t* filePathStr, wchar_t*& itemNameStrBuffer)
{
    itemNameStrBuffer = NULL;

    // Set the file path:
    m_filePath.setFullPathFromString(filePathStr);

    // Get the objects monitored tree object:
    afApplicationTree* pMonitoredTree = vspWindowsManager::instance().monitoredObjectsTree(NULL, QSize(-1, -1));

    // Look for the displayed item data:
    const afApplicationTreeItemData* pDisplayedItemData = NULL;

    if (m_pThumbnailView != NULL)
    {
        // Thumbnail item:
        pDisplayedItemData = m_pThumbnailView->displayedItemData();
    }
    else if (m_pImageBufferView != NULL)
    {
        // Image / buffer item:
        pDisplayedItemData = m_pImageBufferView->displayedItemData();
    }

    gtString itemNameStr;

    bool itemExistInTree = false;

    // If the tree is already updated, get the tree item text:
    if ((pDisplayedItemData != NULL) && (pMonitoredTree != NULL))
    {
        // Check if the item exist in tree:
        itemExistInTree = (pDisplayedItemData->m_pTreeWidgetItem != NULL);
    }

    if (itemExistInTree)
    {
        // Get the item tree name:
        itemNameStr = pMonitoredTree->getTreeItemText(pDisplayedItemData->m_pTreeWidgetItem);
        gdDebugApplicationTreeData* pGDData = qobject_cast<gdDebugApplicationTreeData*>(pDisplayedItemData->extendedItemData());
        GT_IF_WITH_ASSERT(pGDData != NULL)
        {
            // Get the item context as string:
            gtString contextStr;
            pGDData->_contextId.toString(contextStr);

            // Append the context as string to the item name:
            itemNameStr.prependFormattedString(L"%ls ", contextStr.asCharArray());
        }
    }
    else
    {
        // The object does not exist in the tree, deduce the object name from the object name:
        gtString itemName;
        osFilePath filePath(filePathStr);
        filePath.getFileName(itemName);

        // Translate the item name to a display string:
        gdHTMLProperties::itemLinkToDisplayString(itemName, itemNameStr);
    }

    if (!itemNameStr.isEmpty())
    {
        // Allocate and copy the returned string.
        itemNameStrBuffer = vscAllocateAndCopy(itemNameStr);
    }
}

void vscImageAndBuffersEditorDocument::OnSize(int x, int y, int w, int h)
{
    GT_UNREFERENCED_PARAMETER(x);
    GT_UNREFERENCED_PARAMETER(y);

    if (m_pImpl != NULL)
    {
        // Get the created viewer's panel:
        QWidget* pWidget = m_pImpl->widget();

        if (pWidget != NULL)
        {
            QSize windowSize(w, h);
            // Set the panel position and size:

            // Update the created WX window's size:
            QWidget* pCreatedWindow = m_pImpl->createdQTWidget();

            if (pCreatedWindow != NULL)
            {
                pCreatedWindow->resize(windowSize);
            }

            // Handle the image / buffer size issues:
            if (m_pImageBufferView != NULL)
            {
                vspImagesAndBuffersManager::instance().onImageBufferSize(m_pImageBufferView, windowSize);
            }

            // Update the panel:
            pWidget->update();
        }
    }
}

void vscImageAndBuffersEditorDocument::ClosePane()
{
    if (m_pImageBufferView != NULL)
    {
        // Notify the manager that the pane is closed:
        vspImagesAndBuffersManager::instance().onPaneClose(m_pImageBufferView);
    }

    if (m_pThumbnailView != NULL)
    {
        // Notify the manager that the pane is closed:
        vspImagesAndBuffersManager::instance().onPaneClose(m_pThumbnailView);
    }
}

void vscImageAndBuffersEditorDocument::OnShow()
{
    // Update the images and buffers manager that I am the active editor:
    bool isDebuggedProcessSuspended = gaIsDebuggedProcessSuspended();
    vspImagesAndBuffersManager::instance().setFocusedViews(m_pImageBufferView, m_pThumbnailView, isDebuggedProcessSuspended);
}

void* vscImageAndBuffersEditorDocument_CreateInstance()
{
    return new vscImageAndBuffersEditorDocument();
}

void* vscImageAndBuffersEditorDocument_GetImageBufferViewHandle(void* pVscInstance)
{
    gdImageAndBufferView* ret = NULL;
    vscImageAndBuffersEditorDocument* pInstance = (vscImageAndBuffersEditorDocument*)pVscInstance;
    GT_IF_WITH_ASSERT(pInstance != NULL)
    {
        ret = pInstance->ImageBufferView();
    }
    return ret;
}

void* vscImageAndBuffersEditorDocument_GetThumbnailViewHandle(void* pVscInstance)
{
    gdThumbnailView* ret = NULL;
    vscImageAndBuffersEditorDocument* pInstance = (vscImageAndBuffersEditorDocument*)pVscInstance;
    GT_IF_WITH_ASSERT(pInstance != NULL)
    {
        ret = pInstance->ThumbnailView();
    }
    return ret;
}

bool vscCodeXLImageAndBufferCommandIDFromVSCommandId(const GUID& cmdGuid, DWORD cmdId, long& cmdIdBuffer)
{
    bool retVal = true;

    // Visual studio standard command ids:
    if (cmdGuid == CMDSETID_StandardCommandSet97)
    {
        if (cmdId == cmdidZoomIn)
        {
            cmdIdBuffer = ID_IMAGES_AND_BUFFERS_VIEWER_TOOLBAR_ZOOMIN;
        }
        else if (cmdId == cmdidZoomOut)
        {
            cmdIdBuffer = ID_IMAGES_AND_BUFFERS_VIEWER_TOOLBAR_ZOOMOUT;
        }
        else
        {
            retVal = false;
        }
    }
    else if (cmdGuid == CLSID_CodeXLVSPackageCmdSet)
    {
        // Get the CodeXL command id:
        switch (cmdId)
        {

            case  commandIDBestFit:
                cmdIdBuffer = ID_IMAGES_AND_BUFFERS_VIEWER_TOOLBAR_BEST_FIT;
                break;

            case commandIDOrigSize:
                cmdIdBuffer = ID_IMAGES_AND_BUFFERS_VIEWER_ORIGINAL_SIZE;
                break;

            case commandIDSelect:
                cmdIdBuffer = ID_IMAGES_AND_BUFFERS_VIEWER_TOOLBAR_NORMAL;
                break;

            case commandIDPan:
                cmdIdBuffer = ID_IMAGES_AND_BUFFERS_VIEWER_TOOLBAR_PAN;
                break;

            case commandIDRotateLeft:
                cmdIdBuffer = ID_IMAGES_AND_BUFFERS_VIEWER_TOOLBAR_ROTATE_LEFT;
                break;

            case commandIDRotateRight:
                cmdIdBuffer = ID_IMAGES_AND_BUFFERS_VIEWER_TOOLBAR_ROTATE_RIGHT;
                break;

            case commandIDChannelRed:
                cmdIdBuffer = ID_IMAGES_AND_BUFFERS_VIEWER_TOOLBAR_RED_CHANNEL;
                break;

            case commandIDChannelGreen:
                cmdIdBuffer = ID_IMAGES_AND_BUFFERS_VIEWER_TOOLBAR_GREEN_CHANNEL;
                break;

            case commandIDChannelBlue:
                cmdIdBuffer = ID_IMAGES_AND_BUFFERS_VIEWER_TOOLBAR_BLUE_CHANNEL;
                break;

            case commandIDChannelAlpha:
                cmdIdBuffer = ID_IMAGES_AND_BUFFERS_VIEWER_TOOLBAR_ALPHA_CHANNEL;
                break;

            case commandIDChannelInvert:
                cmdIdBuffer = ID_IMAGES_AND_BUFFERS_VIEWER_TOOLBAR_INVERT;
                break;

            case commandIDChannelGrayscale:
                cmdIdBuffer = ID_IMAGES_AND_BUFFERS_VIEWER_TOOLBAR_GRAYSCALE;
                break;

            default:
                retVal = false;
        }
    }
    else
    {
        retVal = false;
    }

    return retVal;
}

void vscImageAndBuffersEditorDocument_OnImageAndBuffersAction(const GUID& cmdGuid, DWORD cmdId)
{
    // Translate the VS command id to CodeXL command id:
    gdImageActionId gdCommandId = ID_IMAGES_AND_BUFFERS_VIEWER_TOOLBAR_ACTION_UNKNOWN;
    long gdCommandIdVal = 0;
    bool rcCommandID = vscCodeXLImageAndBufferCommandIDFromVSCommandId(cmdGuid, cmdId, gdCommandIdVal);
    gdCommandId = (gdImageActionId)gdCommandIdVal;

    GT_IF_WITH_ASSERT(rcCommandID)
    {
        // Perform the command:
        vspImagesAndBuffersManager::instance().onToolbarEvent(gdCommandId);
    }
}

bool vscImageAndBuffersEditorDocument_OnQueryImageAndBufferAction_IsActionRequired(const GUID& cmdGuid, DWORD cmdId)
{
    bool ret = false;
    // Translate the VS command id to CodeXL command id:
    gdImageActionId gdCommandId = ID_IMAGES_AND_BUFFERS_VIEWER_TOOLBAR_ACTION_UNKNOWN;
    long gdCommandIdVal = 0;
    bool rcCommandID = vscCodeXLImageAndBufferCommandIDFromVSCommandId(cmdGuid, cmdId, gdCommandIdVal);
    gdCommandId = (gdImageActionId)gdCommandIdVal;

    GT_IF_WITH_ASSERT(rcCommandID)
    {
        // Check if this action should be checked:
        ret = vspImagesAndBuffersManager::instance().isToolbarCommandEnabled(gdCommandId);
    }
    return ret;
}

void vscImageAndBuffersEditorDocument_OnQueryImageAndBufferCheckedAction_IsActionRequired(const GUID& cmdGuid, DWORD cmdId, bool& shouldEnableBuffer, bool& shouldCheckBuffer)
{
    // Translate the VS command id to CodeXL command id:
    gdImageActionId gdCommandId = ID_IMAGES_AND_BUFFERS_VIEWER_TOOLBAR_ACTION_UNKNOWN;
    long gdCommandIdVal = 0;
    bool rcCommandID = vscCodeXLImageAndBufferCommandIDFromVSCommandId(cmdGuid, cmdId, gdCommandIdVal);
    gdCommandId = (gdImageActionId)gdCommandIdVal;

    GT_IF_WITH_ASSERT(rcCommandID)
    {
        // Check if this action should be checked:
        shouldEnableBuffer = false;
        shouldCheckBuffer = false;

        shouldEnableBuffer = vspImagesAndBuffersManager::instance().isToolbarCheckedCommandEnabled(gdCommandId, shouldCheckBuffer);
    }
}

void vscImageAndBuffersEditorDocument_OnQueryImageSizeChanged_IsActionRequired(bool& shouldEnableBuffer)
{
    shouldEnableBuffer = vspImagesAndBuffersManager::instance().isToolbarCommandEnabled(ID_IMAGES_AND_BUFFERS_VIEWER_ZOOM_COMBOBOX);
}

bool vscImageAndBuffersEditorDocument_GetAvailableZoomLevels(unsigned int*& pOutBuffer, size_t& sizeBuffer)
{
    bool ret = false;
    // Get the list of available zoom strings from the image manager:
    const gtVector<unsigned int>& availableZoomLevels = gdImageAndBufferView::availableZoomLevels();

    // Fill the buffers:
    pOutBuffer = NULL;
    sizeBuffer = availableZoomLevels.size();
    GT_IF_WITH_ASSERT(sizeBuffer > 0)
    {
        pOutBuffer = new unsigned int[sizeBuffer];
        std::copy(availableZoomLevels.begin(), availableZoomLevels.end(), pOutBuffer);
        ret = true;
    }
    return ret;
}

bool vscImageAndBuffersEditorDocument_ChangeZoomLevel(const wchar_t* pZoomText, int& currentZoomLevelBuffer)
{
    bool ret = false;
    currentZoomLevelBuffer = 100;
    gdImageAndBufferView* pFocusedImageBufferView = vspImagesAndBuffersManager::instance().focusedImageBufferView();

    // Sanity check:
    GT_IF_WITH_ASSERT(pFocusedImageBufferView != NULL)
    {
        GT_IF_WITH_ASSERT(pZoomText != NULL)
        {
            // Check if the string is a valid integer value:
            ret = pFocusedImageBufferView->tryToChangeZoomLevel(pZoomText);

            if (ret)
            {
                currentZoomLevelBuffer = pFocusedImageBufferView->currentZoomLevel();
            }
        }
    }
    return ret;
}
