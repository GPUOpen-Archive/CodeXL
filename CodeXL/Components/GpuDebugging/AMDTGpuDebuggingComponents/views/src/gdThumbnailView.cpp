//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gdThumbnailView.cpp
///
//==================================================================================

//------------------------------ gdThumbnailView.cpp ------------------------------

// Qt
#include <AMDTApplicationComponents/Include/acQtIncludes.h>

#include <AMDTGpuDebuggingComponents/Include/gdHTMLProperties.h>

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTAPIClasses/Include/apInternalFormat.h>
#include <AMDTAPIClasses/Include/apStaticBuffer.h>
#include <AMDTAPIClasses/Include/apCLImage.h>
#include <AMDTAPIClasses/Include/apCLBuffer.h>
#include <AMDTAPIClasses/Include/apGLRenderBuffer.h>
#include <AMDTAPIClasses/Include/apGLVBO.h>
#include <AMDTApiFunctions/Include/gaGRApiFunctions.h>
#include <AMDTApplicationComponents/Include/acImageManager.h>
#include <AMDTApplicationComponents/Include/acFunctions.h>

// AMDTApplicationFramework:
#include <AMDTApplicationFramework/Include/afApplicationCommands.h>
#include <AMDTApplicationFramework/Include/afAppStringConstants.h>
#include <AMDTApplicationFramework/Include/afProgressBarWrapper.h>
#include <AMDTApplicationFramework/Include/views/afApplicationTree.h>

// Local:
#include <AMDTGpuDebuggingComponents/Include/gdAidFunctions.h>
#include <AMDTGpuDebuggingComponents/Include/gdCommandIDs.h>
#include <AMDTGpuDebuggingComponents/Include/gdGDebuggerGlobalVariablesManager.h>
#include <AMDTGpuDebuggingComponents/Include/views/gdThumbnailView.h>
#include <AMDTGpuDebuggingComponents/Include/gdDebugApplicationTreeHandler.h>
#include <AMDTGpuDebuggingComponents/Include/views/gdPBufferImageProxy.h>
#include <AMDTGpuDebuggingComponents/Include/views/gdStaticBufferImageProxy.h>
#include <AMDTGpuDebuggingComponents/Include/views/gdRenderBufferImageProxy.h>
#include <AMDTGpuDebuggingComponents/Include/views/gdTextureImageProxy.h>
#include <AMDTGpuDebuggingComponents/Include/views/gdVBOImageProxy.h>


// Minimum objects for progress bar:
#define GD_MIN_THUMBNAIL_AMOUNT_FOR_PROGRESS_BAR 100

// ---------------------------------------------------------------------------
// Name:        gdThumbnailView::gdThumbnailView
// Description: Constructor
// Arguments:   pParent - The thumbnail view's parent
// Author:      Sigal Algranaty
// Date:        7/11/2010
// ---------------------------------------------------------------------------
gdThumbnailView::gdThumbnailView(QWidget* pParent, afProgressBarWrapper* pProgressBar, gdDebugApplicationTreeHandler* pObjectsTree)
    : acImageManager(pParent, AC_MANAGER_MODE_THUMBNAIL_VIEW), afBaseView(pProgressBar),
      _pDisplayedItemData(NULL), _isInGLBeginEndBlock(false)
{
    (void)(pObjectsTree);  // unused

}

// ---------------------------------------------------------------------------
// Name:        gdThumbnailView::~gdThumbnailView
// Description: Destructor
// Author:      Sigal Algranaty
// Date:        7/11/2010
// ---------------------------------------------------------------------------
gdThumbnailView::~gdThumbnailView()
{
}


// ---------------------------------------------------------------------------
// Name:        gdThumbnailView::clearView
// Description: Clears the thumbnail view
// Return Val:  void
// Author:      Sigal Algranaty
// Date:        10/11/2010
// ---------------------------------------------------------------------------
void gdThumbnailView::clearView()
{
    // Clear all my thumbnails:
    clearAllObjects();

    // Reset tooltip:
    setToolTip(AF_STR_EmptyA);

    // Reset the cursor to default:
    setCursor(Qt::ArrowCursor);
}


// ---------------------------------------------------------------------------
// Name:        gdThumbnailView::setFrameLayout
// Description:
// Arguments:   wxSize viewSize
// Return Val:  void
// Author:      Sigal Algranaty
// Date:        10/11/2010
// ---------------------------------------------------------------------------
bool gdThumbnailView::setFrameLayout(const QSize& viewSize)
{
    bool retVal = true;

    // Set my min size;
    resize(viewSize);

    layout();

    // Connect the pixel change signal of the data view:
    bool rc = connect(this, SIGNAL(pixelPositionChanged(acImageItemID, const QPoint&, bool, bool)), this, SLOT(onImageItemEvent(acImageItemID, const QPoint&, bool, bool)));
    GT_ASSERT(rc);

    return retVal;
}
// ---------------------------------------------------------------------------
// Name:        gdThumbnailView::displayThumbnailItem
// Description: Display a thumbnail item in the thumbnail view
// Arguments:   pThumbnailItemData
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        7/11/2010
// ---------------------------------------------------------------------------
bool gdThumbnailView::displayThumbnailItem(afApplicationTreeItemData* pThumbnailItemData)
{
    bool retVal = false;

    // Set the displayed item data:
    _pDisplayedItemData = pThumbnailItemData;

    // Sanity check:
    GT_IF_WITH_ASSERT(_pDisplayedItemData != NULL)
    {
        // Copy the item to an item data with the object id:
        _pDisplayedItemData->copyID(_displayedItemId);

        retVal = true;

        // Add the items according to the node type:
        if (_pDisplayedItemData->m_objectCount > 0)
        {
            switch (_pDisplayedItemData->m_itemType)
            {
                case AF_TREE_ITEM_GL_TEXTURES_NODE:
                case AF_TREE_ITEM_CL_IMAGES_NODE:
                    retVal = addTexturesToThumbnailView(_pDisplayedItemData);
                    break;

                case AF_TREE_ITEM_GL_RENDER_BUFFERS_NODE:
                    retVal = addRenderBufferToThumbnailView(_pDisplayedItemData);
                    break;

                case AF_TREE_ITEM_GL_STATIC_BUFFERS_NODE:
                    retVal = addStaticBuffersToThumbnailView(_pDisplayedItemData);
                    break;

                case AF_TREE_ITEM_GL_PBUFFERS_NODE:
                    retVal = addPBuffersToThumbnailView(_pDisplayedItemData);
                    break;

                case AF_TREE_ITEM_GL_PBUFFER_NODE:
                    retVal = addPBufferContentToThumbnailView(_pDisplayedItemData);
                    break;

                case AF_TREE_ITEM_GL_VBO_NODE:
                    retVal = addVBOsToThumbnailView(_pDisplayedItemData);
                    break;

                case AF_TREE_ITEM_GL_FBO_NODE:
                    retVal = addAllFBOsAttachmentToThumbnailView(_pDisplayedItemData);
                    break;

                case AF_TREE_ITEM_GL_FBO:
                    retVal = addFBOAttachmentsToThumbnailView(_pDisplayedItemData);
                    break;

                case AF_TREE_ITEM_CL_BUFFERS_NODE:
                    retVal = addCLBuffersToThumbnailView(_pDisplayedItemData);
                    break;

                default:
                    retVal = false;
                    GT_ASSERT_EX(false, L"Unsupported thumbnail requested");
                    break;
            }
        }
        else
        {
            // Display a "No objects" message in the thumbnail view:
            _pDisplayedItemData->setItemLoadStatus(AF_ITEM_NOT_LOADED, AF_ITEM_LOAD_EMPTY_THUMBS);
        }

        if (retVal)
        {
            // Set the thumbnail load status:
            _pDisplayedItemData->_itemLoadStatus._itemLoadStatusType = AF_ITEM_LOAD_SUCCESS;
        }

        // Check if a message should be displayed, and display it:
        displayTextMessageIfNeeded();

        // Adjust the manager layout after adding all the thumbnails
        forceImagesRepaint();
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gdThumbnailView::displayTextMessageIfNeeded
// Description: Checks if the object should be displayed as text message and
//              display it
// Author:      Sigal Algranaty
// Date:        7/4/2011
// ---------------------------------------------------------------------------
void gdThumbnailView::displayTextMessageIfNeeded()
{
    // Sanity check:
    GT_IF_WITH_ASSERT(_pDisplayedItemData != NULL)
    {
        // If the item load was failed, display the failure description:
        if (!_pDisplayedItemData->isItemDisplayed())
        {
            // Get the failure message as string:
            gtString failureMsg, failureTitle;
            _pDisplayedItemData->getItemStatusAsString(_pDisplayedItemData->m_itemType, failureTitle, failureMsg);

            // Write the text message in thumbnail view:
            // TO_DO: Sigal errors: check the color
            writeTextMessage(failureTitle, failureMsg, Qt::red);
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        gdThumbnailView::addPBuffersToThumbnailView
// Description: Add all pbuffers thumbnails to the view
// Arguments:   const afApplicationTreeItemData* pThumbnailItemData
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        7/11/2010
// ---------------------------------------------------------------------------
bool gdThumbnailView::addPBuffersToThumbnailView(const afApplicationTreeItemData* pThumbnailItemData)
{
    (void)(pThumbnailItemData);  // unused
    // TO_DO: VS textures and buffers viewer: implement
    bool retVal = false;
    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gdThumbnailView::addPBufferContentToThumbnailView
// Description: Generates the PBuffer content buffers to thumbnail view
// Arguments:   pThumbnailItemData - the item data for the PBuffer
// Return Val:  bool - Success / Failure
// Author:      Eran Zinman
// Date:        25/1/2008
// ---------------------------------------------------------------------------
bool gdThumbnailView::addPBufferContentToThumbnailView(const afApplicationTreeItemData* pThumbnailItemData)
{
    bool retVal = false;

    // Sanity check:
    GT_IF_WITH_ASSERT((gdDebugApplicationTreeHandler::instance() != NULL) && (pThumbnailItemData != NULL))
    {
        gdDebugApplicationTreeData* pGDData = qobject_cast<gdDebugApplicationTreeData*>(pThumbnailItemData->extendedItemData());
        GT_IF_WITH_ASSERT(pGDData != NULL)
        {

            // Get teh PBuffer id:
            int pbufferID = pGDData->_objectOpenGLName;

            // Get amount of displayed PBuffers:
            int amountOfDisplayedPBuffers = pThumbnailItemData->m_objectCount;

            // Get the PBuffers index:
            int pbufferIndex = -1;

            // Search for the index of the PBuffer with this PBuffer ID:
            for (int i = 0; i < amountOfDisplayedPBuffers; i++)
            {
                // Get the current PBuffer id:
                int currentPBufferID = 0;
                bool rc = gdDebugApplicationTreeHandler::instance()->getPBufferId(i, currentPBufferID);

                if (rc && (currentPBufferID == pbufferID))
                {
                    pbufferIndex = i;
                    break;
                }
            }

            GT_IF_WITH_ASSERT(pbufferIndex >= 0)
            {
                // Get the amount of buffers attached to this buffer:
                int amountOfPbufferAttahments = gdDebugApplicationTreeHandler::instance()->getAmountOfBuffersAttachedToPBuffer(pbufferIndex);

                for (int i = 0; i < amountOfPbufferAttahments; i++)
                {
                    // Get the current attached buffer item data:
                    afApplicationTreeItemData* pViewerItem = gdDebugApplicationTreeHandler::instance()->getPBufferAttachmentItemData(pbufferIndex, i);
                    GT_IF_WITH_ASSERT(pViewerItem != NULL)
                    {
                        // Is this the item type we are looking for?
                        if (pViewerItem->m_itemType == AF_TREE_ITEM_GL_PBUFFER_STATIC_BUFFER)
                        {
                            gdDebugApplicationTreeData* pGDViewerItem = qobject_cast<gdDebugApplicationTreeData*>(pViewerItem->extendedItemData());
                            GT_IF_WITH_ASSERT(pGDViewerItem != NULL)
                            {
                                // Does item belong to our PBuffer?
                                if (pGDViewerItem->_objectOpenGLName == (unsigned int)pbufferID)
                                {
                                    // Get the buffer type
                                    apDisplayBuffer bufferType = pGDViewerItem->_bufferType;

                                    // Get the PBuffer static buffer details:
                                    apStaticBuffer staticBufferDetails;
                                    bool rc1 = gaGetPBufferStaticBufferObjectDetails(pbufferID, bufferType, staticBufferDetails);
                                    GT_IF_WITH_ASSERT(rc1)
                                    {
                                        // Top label is PBuffer ID
                                        gtString topLabel;
                                        topLabel.appendFormattedString(GD_STR_ImagesAndBuffersViewerPBufferName, pbufferID);

                                        // Generate PBuffer static buffer bottom label
                                        gtString bottomLabel;
                                        bool rc2 = apGetBufferName(bufferType, bottomLabel);
                                        GT_IF_WITH_ASSERT(rc2)
                                        {
                                            // Create the PBuffer image data proxy:
                                            gdPBufferImageProxy* pImageDataProxy = new gdPBufferImageProxy(pbufferID, staticBufferDetails, pGDViewerItem->_contextId._contextId, _isInGLBeginEndBlock);


                                            // Add thumbnail to the image viewer:
                                            unsigned int imageActions = 0;
                                            bool rcImageActions = acImageManager::getItemActionsByFormat(staticBufferDetails.bufferFormat(), imageActions);
                                            GT_ASSERT(rcImageActions);

                                            bool rcAddThumb = addThumbnailItem(pImageDataProxy, imageActions, (void*)pViewerItem, topLabel, bottomLabel);
                                            GT_ASSERT(rcAddThumb);
                                        }
                                    }
                                    retVal = true;
                                }
                            }
                        }
                    }
                }
            }
        }
    }


    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gdThumbnailView::addTexturesToThumbnailView
// Description: Loads and display all available textures into the image viewer
// Arguments:   pThumbnailItemData - the item data representing the textures node
// Return Val:  bool - Success / Failure
// Author:      Eran Zinman
// Date:        24/5/2007
// ---------------------------------------------------------------------------
bool gdThumbnailView::addTexturesToThumbnailView(const afApplicationTreeItemData* pThumbnailItemData)
{
    bool retVal = false;

    // Sanity check
    GT_IF_WITH_ASSERT((pThumbnailItemData != NULL) && (gdDebugApplicationTreeHandler::instance() != NULL))
    {
        retVal = true;

        gdDebugApplicationTreeData* pGDThumbnailItemData = qobject_cast<gdDebugApplicationTreeData*>(pThumbnailItemData->extendedItemData());
        GT_IF_WITH_ASSERT(pGDThumbnailItemData != NULL)
        {

            // Get the context id:
            apContextID contextId = pGDThumbnailItemData->_contextId;

            bool isGLContext = contextId.isOpenGLContext();
            GT_ASSERT(contextId.isValid());

            // Get amount of texture items:
            int amountOfDisplayedTextures = gdDebugApplicationTreeHandler::instance()->amountOfDisplayedObjectForType(contextId, AF_TREE_ITEM_CL_IMAGE) +
                                            gdDebugApplicationTreeHandler::instance()->amountOfDisplayedObjectForType(contextId, AF_TREE_ITEM_GL_TEXTURE);

            // Initialize progress dialog:
            afProgressBarWrapper::instance().ShowProgressBar(AF_TREE_ITEM_GL_TEXTURE, L"Loading", amountOfDisplayedTextures);

            for (int i = 0; i < amountOfDisplayedTextures; i++)
            {
                // Make sure progress bar is set to its max value:
                updateProgressBar(i);

                // Get the texture item data:
                afApplicationTreeItemData* pTextureItemData = NULL;

                if (isGLContext)
                {
                    pTextureItemData = gdDebugApplicationTreeHandler::instance()->getItemDataByType(contextId, AF_TREE_ITEM_GL_TEXTURE, i);
                    GT_IF_WITH_ASSERT(pTextureItemData != NULL)
                    {
                        addSingleOpenGLTextureThumbnail(pTextureItemData);
                    }
                }
                else
                {
                    pTextureItemData = gdDebugApplicationTreeHandler::instance()->getItemDataByType(contextId, AF_TREE_ITEM_CL_IMAGE, i);
                    GT_IF_WITH_ASSERT(pTextureItemData != NULL)
                    {
                        addSingleOpenCLImageThumbnail(pTextureItemData);
                    }
                }
            }

            hideProgressBar();
        }
    }


    return retVal;
}



// ---------------------------------------------------------------------------
// Name:        gdThumbnailView::addSingleOpenGLTextureThumbnail
// Description: Loads an OpenGL texture thumbnail into thumbnail view
// Arguments:   pTextureItemData - the item data representing the texture object
// Return Val:  bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        2/12/2009
// ---------------------------------------------------------------------------
bool gdThumbnailView::addSingleOpenGLTextureThumbnail(afApplicationTreeItemData* pTextureItemData)
{
    bool retVal = false;
    gdDebugApplicationTreeData* pGDTextureItemData = qobject_cast<gdDebugApplicationTreeData*>(pTextureItemData->extendedItemData());
    GT_IF_WITH_ASSERT(pGDTextureItemData != NULL)
    {
        // Get texture name:
        GLuint textureName = pGDTextureItemData->_textureMiplevelID._textureName;

        // Get the item context id:
        apContextID contextId = pGDTextureItemData->_contextId;

        // Get texture thumbnail data details:
        apGLTextureMiplevelData textureThumbnailDetails;
        bool rc = gaGetTextureObjectThumbnailData(contextId._contextId, textureName, textureThumbnailDetails);
        GT_IF_WITH_ASSERT(rc)
        {
            // Generate texture top label and bottom label
            gtString topLabel;
            gtString bottomLabel;

            // Build the texture top label:
            generateGLTextureTopLabel(textureName, topLabel);

            // Generate the texture bottom label (dimension label):
            generateTextureThumbnailLabel(textureThumbnailDetails, bottomLabel, contextId);

            // Create the texture image data proxy:
            // Load the image data with level 0. We want the thumbnail to always show level 0, and not the currently selected level
            // by the user:
            gdTextureImageProxy* pImageDataProxy = new gdTextureImageProxy(pTextureItemData, _isInGLBeginEndBlock, AC_IMAGES_MANAGER_THUMBNAIL_SIZE, AC_IMAGES_MANAGER_THUMBNAIL_SIZE);


            // Get image possible actions by type:
            unsigned int imageActions = 0;
            bool rcImageActions = getItemActionsByFormat(textureThumbnailDetails.texelDataFormat(), imageActions);
            GT_ASSERT(rcImageActions);

            // Add thumbnail to the image viewer:
            retVal = addThumbnailItem(pImageDataProxy, imageActions, (void*)pTextureItemData, topLabel, bottomLabel);
            GT_ASSERT(retVal);
        }
    }
    return retVal;
}



// ---------------------------------------------------------------------------
// Name:        gdThumbnailView::addSingleOpenCLImageThumbnail
// Description: Loads an OpenGL texture thumbnail into thumbnail view
// Arguments:   pTextureItemData - the item data representing the texture object
// Return Val:  bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        2/12/2009
// ---------------------------------------------------------------------------
bool gdThumbnailView::addSingleOpenCLImageThumbnail(afApplicationTreeItemData* pTextureItemData)
{
    bool retVal = false;

    gdDebugApplicationTreeData* pGDTextureItemData = qobject_cast<gdDebugApplicationTreeData*>(pTextureItemData->extendedItemData());
    GT_IF_WITH_ASSERT(pGDTextureItemData != NULL)
    {
        // Get the item context id:
        apContextID contextId = pGDTextureItemData->_contextId;

        // Get texture details:
        apCLImage textureDetails;
        bool rc = gaGetOpenCLImageObjectDetails(contextId._contextId, pGDTextureItemData->_objectOpenCLIndex, textureDetails);
        GT_IF_WITH_ASSERT(rc)
        {
            // Generate texture top label and bottom label
            gtString topLabel;
            gtString bottomLabel;

            // Build the texture top label:
            generateCLImageTopLabel(textureDetails, topLabel);

            // Generate the texture bottom label (dimension label):
            generateCLImageThumbnailLabel(textureDetails, bottomLabel);

            // Create the texture image data proxy:
            // Load the image data with level 0. We want the thumbnail to always show level 0, and not the currently selected level
            // by the user:
            gdTextureImageProxy* pImageDataProxy = new gdTextureImageProxy(pTextureItemData, false, AC_IMAGES_MANAGER_THUMBNAIL_SIZE, AC_IMAGES_MANAGER_THUMBNAIL_SIZE);


            // Get the texture data format as oaTexelDataFormat:
            oaTexelDataFormat dataFormat = OA_TEXEL_FORMAT_UNKNOWN;

            // Get the data format as oaTexelDataFormat:
            bool rcTexelFormat = oaCLImageFormatToTexelFormat(textureDetails.dataFormat(), dataFormat);
            GT_ASSERT(rcTexelFormat);

            // Get image possible actions by type:
            unsigned int imageActions = 0;
            bool rcImageActions = getItemActionsByFormat(dataFormat, imageActions);
            GT_ASSERT(rcImageActions);


            // Add thumbnail to the image viewer:
            retVal = addThumbnailItem(pImageDataProxy, imageActions, (void*)pTextureItemData, topLabel, bottomLabel);
            GT_ASSERT(retVal);
        }
    }

    return retVal;
}



// ---------------------------------------------------------------------------
// Name:        gdThumbnailView::generateTextureTopLabel
// Description: Generates the texture top label
// Arguments: GLuint textureName
//            gtString& topLabel - the texture top label
// Return Val: void
// Author:      Sigal Algranaty
// Date:        9/8/2009
// ---------------------------------------------------------------------------
void gdThumbnailView::generateGLTextureTopLabel(GLuint textureName, gtString& topLabel)
{
    // Generate a texture name:
    apGLTextureMipLevelID textureID;
    textureID._textureName = textureName;
    textureID._textureMipLevel = 0;

    // Generate the object name:
    gdHTMLProperties htmlBuilder;
    htmlBuilder.getGLTextureName(textureID, -1, -1, topLabel, true);
}

// ---------------------------------------------------------------------------
// Name:        gdThumbnailView::generateTextureTopLabel
// Description: Generates the texture top label
// Arguments: GLuint textureName
//            gtString& topLabel - the texture top label
// Return Val: void
// Author:      Sigal Algranaty
// Date:        9/8/2009
// ---------------------------------------------------------------------------
void gdThumbnailView::generateCLImageTopLabel(const apCLImage& textureDetails, gtString& topLabel)
{
    // Generate the object name:
    gdHTMLProperties htmlBuilder;
    htmlBuilder.getCLImageName(textureDetails, topLabel, true);
}

// ---------------------------------------------------------------------------
// Name:        gdThumbnailView::addAllFBOsAttachmentToThumbnailView
// Description: Loads and display all available FBO buffers into the image viewer
// Arguments:   pThumbnailItemData - the FBO viewer item
// Return Val:  bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        5/6/2008
// ---------------------------------------------------------------------------
bool gdThumbnailView::addAllFBOsAttachmentToThumbnailView(const afApplicationTreeItemData* pThumbnailItemData)
{
    bool retVal = false;

    gdDebugApplicationTreeData* pGDThumbnailItemData = qobject_cast<gdDebugApplicationTreeData*>(pThumbnailItemData->extendedItemData());
    GT_IF_WITH_ASSERT(pGDThumbnailItemData != NULL)
    {
        // Get the item context id:
        apContextID contextId = pGDThumbnailItemData->_contextId;

        // Sanity check:
        GT_IF_WITH_ASSERT((pThumbnailItemData != NULL) && (gdDebugApplicationTreeHandler::instance() != NULL))
        {
            retVal = true;

            // Get amount of displayed FBOs:
            int amountOfDisplayedFBOs = gdDebugApplicationTreeHandler::instance()->amountOfDisplayedObjectForType(contextId, AF_TREE_ITEM_GL_FBO);

            // Loop through the FBOs:
            for (int i = 0; i < amountOfDisplayedFBOs; i++)
            {
                // Get the item data from the tree item id:
                afApplicationTreeItemData* pFBOViewerItem = gdDebugApplicationTreeHandler::instance()->getItemDataByType(contextId, AF_TREE_ITEM_GL_FBO_NODE, i);

                if (pFBOViewerItem != NULL)
                {
                    // Go through the FBO buffer items. Skip the others
                    if (pFBOViewerItem->m_itemType == AF_TREE_ITEM_GL_FBO_NODE)
                    {

                        // Go through the FBO attachments:
                        int amountOfFBOAttachments = gdDebugApplicationTreeHandler::instance()->amountOfFBOAttachments(contextId, i);

                        for (int j = 0; j < amountOfFBOAttachments; j++)
                        {

                            // Get the item data from the tree item id:
                            afApplicationTreeItemData* pFBOAttachmentViewerItem = gdDebugApplicationTreeHandler::instance()->getFBOAttachmentItemData(contextId, i, j);

                            if (pFBOAttachmentViewerItem != NULL)
                            {
                                gdDebugApplicationTreeData* pGDFBOAttachmentViewerItem = qobject_cast<gdDebugApplicationTreeData*>(pFBOAttachmentViewerItem->extendedItemData());
                                GT_IF_WITH_ASSERT(pGDFBOAttachmentViewerItem != NULL)
                                {
                                    // Go through the FBO buffer items. Skip the others
                                    if (pFBOAttachmentViewerItem->m_itemType == AF_TREE_ITEM_GL_FBO)
                                    {
                                        // Check if the buffer item is a texture or a render buffer:
                                        GLenum attachmentTarget = pGDFBOAttachmentViewerItem->_bufferAttachmentTarget;

                                        bool isTextureAttachment = false;
                                        bool rc = apGLFBO::isTextureAttachmentTarget(attachmentTarget, isTextureAttachment);
                                        GT_IF_WITH_ASSERT(rc)
                                        {
                                            if (isTextureAttachment)
                                            {
                                                // The attachment is a texture, load it as texture:
                                                // Get texture ID:
                                                GLuint textureName = pGDFBOAttachmentViewerItem->_textureMiplevelID._textureName;

                                                // Get texture thumbnail data details:
                                                apGLTextureMiplevelData textureThumbnailDetails;
                                                rc = gaGetTextureObjectThumbnailData(contextId._contextId, textureName, textureThumbnailDetails);
                                                GT_IF_WITH_ASSERT(rc)
                                                {
                                                    // Generate texture top label and bottom label
                                                    gtString topLabel;
                                                    gtString bottomLabel;
                                                    topLabel.appendFormattedString(GD_STR_TexturesViewerNameFBO, pGDFBOAttachmentViewerItem->_fboAttachmentFBOName);
                                                    topLabel.append(L": ");
                                                    topLabel.appendFormattedString(GD_STR_ImagesAndBuffersViewerExportItemNameShortTexture, textureName);
                                                    generateTextureThumbnailLabel(textureThumbnailDetails, bottomLabel, contextId);

                                                    // Create the texture image data proxy:
                                                    gdTextureImageProxy* pImageDataProxy = new gdTextureImageProxy(pFBOAttachmentViewerItem, _isInGLBeginEndBlock, AC_IMAGES_MANAGER_THUMBNAIL_SIZE, AC_IMAGES_MANAGER_THUMBNAIL_SIZE);
                                                    GT_IF_WITH_ASSERT(pImageDataProxy != NULL)
                                                    {
                                                        // Add thumbnail to the image viewer:
                                                        unsigned int imageActions = 0;
                                                        bool rcImageActions = acImageManager::getItemActionsByFormat(pGDFBOAttachmentViewerItem->_dataFormat, imageActions);
                                                        GT_ASSERT(rcImageActions);

                                                        // Add thumbnail to the image viewer:
                                                        bool rcAddThumb = addThumbnailItem(pImageDataProxy, imageActions, (void*)pFBOAttachmentViewerItem, topLabel, bottomLabel);
                                                        GT_ASSERT(rcAddThumb);
                                                    }
                                                }
                                            }
                                            else
                                            {
                                                // The attachment is a render buffer, load it as texture:
                                                // Get the render buffer ID:
                                                GLuint renderBufferID = pGDFBOAttachmentViewerItem->_objectOpenGLName;

                                                // Get the selected texture details
                                                apGLRenderBuffer renderBufferDetails(0);
                                                rc = gaGetRenderBufferObjectDetails(contextId._contextId, renderBufferID, renderBufferDetails);
                                                GT_IF_WITH_ASSERT(rc)
                                                {
                                                    // Generate texture top label and bottom label
                                                    gtString topLabel;
                                                    gtString bottomLabel;
                                                    topLabel.appendFormattedString(GD_STR_TexturesViewerNameFBO, pGDFBOAttachmentViewerItem->_fboAttachmentFBOName);
                                                    topLabel.append(L": ");
                                                    topLabel.appendFormattedString(GD_STR_ImagesAndBuffersViewerRenderBufferNameShortFormat, renderBufferID);
                                                    generateRenderBufferThumbnailLabel(renderBufferDetails, bottomLabel);

                                                    // Create the render buffer image data proxy
                                                    gdRenderBufferImageProxy* pImageDataProxy = new gdRenderBufferImageProxy(renderBufferID, renderBufferDetails, contextId._contextId, _isInGLBeginEndBlock);


                                                    // Add thumbnail to the image viewer:
                                                    unsigned int imageActions = 0;
                                                    bool rcImageActions = acImageManager::getItemActionsByFormat(pGDFBOAttachmentViewerItem->_dataFormat, imageActions);
                                                    GT_ASSERT(rcImageActions);

                                                    // Add thumbnail to the image viewer:
                                                    bool rcAddThumb = addThumbnailItem(pImageDataProxy, imageActions, (void*)pFBOAttachmentViewerItem, topLabel, bottomLabel);
                                                    GT_ASSERT(rcAddThumb);
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gdThumbnailView::addFBOAttachmentsToThumbnailView
// Description: Loads and display all available FBO buffers into the image viewer
// Arguments:   pThumbnailItemData - the FBO viewer item
// Return Val:  bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        5/6/2008
// ---------------------------------------------------------------------------
bool gdThumbnailView::addFBOAttachmentsToThumbnailView(const afApplicationTreeItemData* pThumbnailItemData)
{
    bool retVal = false;

    // Sanity check
    GT_IF_WITH_ASSERT(pThumbnailItemData && gdDebugApplicationTreeHandler::instance())
    {
        gdDebugApplicationTreeData* pGDThumbnailItemData = qobject_cast<gdDebugApplicationTreeData*>(pThumbnailItemData->extendedItemData());
        GT_IF_WITH_ASSERT(pGDThumbnailItemData != NULL)
        {
            // Get the item context id:
            apContextID contextId = pGDThumbnailItemData->_contextId;

            retVal = true;

            // Get the selected FBO:
            GLuint fboName = pGDThumbnailItemData->_objectOpenGLName;
            GT_IF_WITH_ASSERT(fboName != 0)
            {
                // Get the index of the FBO with the fbo name:
                int fboIndex = gdDebugApplicationTreeHandler::instance()->getFBOIndex(contextId, fboName);
                GT_IF_WITH_ASSERT(fboIndex >= 0)
                {
                    // Get amount of FBO attachments:
                    int amountOfFBODisplayedAttachments = gdDebugApplicationTreeHandler::instance()->amountOfFBOAttachments(contextId, fboIndex);

                    for (int i = 0; i < amountOfFBODisplayedAttachments; i++)
                    {
                        // Get the item data from the tree item id:
                        afApplicationTreeItemData* pFBOAttachmentItem = gdDebugApplicationTreeHandler::instance()->getFBOAttachmentItemData(contextId, fboIndex, i);

                        if (pFBOAttachmentItem != NULL)
                        {
                            gdDebugApplicationTreeData* pGDFBOAttachmentItem = qobject_cast<gdDebugApplicationTreeData*>(pFBOAttachmentItem->extendedItemData());
                            GT_IF_WITH_ASSERT(pGDFBOAttachmentItem != NULL)
                            {
                                // Go through the FBO buffer items. Skip the others
                                if (pFBOAttachmentItem->m_itemType == AF_TREE_ITEM_GL_FBO)
                                {
                                    // Check if the buffer item is a texture or a render buffer:
                                    GLenum attachmentTarget = pGDFBOAttachmentItem->_bufferAttachmentTarget;

                                    // Check if this attachment object is binded to the requested FBO:
                                    if (pGDFBOAttachmentItem->_fboAttachmentFBOName  != fboName)
                                    {
                                        continue;
                                    }

                                    bool isTextureAttachment = false;
                                    bool rc = apGLFBO::isTextureAttachmentTarget(attachmentTarget, isTextureAttachment);
                                    GT_IF_WITH_ASSERT(rc)
                                    {
                                        if (isTextureAttachment)
                                        {
                                            // The attachment is a texture, load it as texture:
                                            // Get texture ID:
                                            GLuint textureName = pGDFBOAttachmentItem->_textureMiplevelID._textureName;

                                            // Get texture thumbnail data details:
                                            apGLTextureMiplevelData textureThumbnailDetails;
                                            rc = gaGetTextureObjectThumbnailData(contextId._contextId, textureName, textureThumbnailDetails);
                                            GT_IF_WITH_ASSERT(rc)
                                            {
                                                // Generate texture top label and bottom label
                                                gtString topLabel;
                                                gtString bottomLabel;
                                                topLabel.appendFormattedString(GD_STR_PropertiesTextureNameFormat, textureName);
                                                generateTextureThumbnailLabel(textureThumbnailDetails, bottomLabel, contextId);

                                                // Create the texture image data proxy
                                                gdTextureImageProxy* pImageDataProxy = new gdTextureImageProxy(pFBOAttachmentItem, _isInGLBeginEndBlock, AC_IMAGES_MANAGER_THUMBNAIL_SIZE, AC_IMAGES_MANAGER_THUMBNAIL_SIZE);
                                                GT_IF_WITH_ASSERT(pImageDataProxy != NULL)
                                                {
                                                    unsigned int imageActions = 0;
                                                    bool rcImageActions = acImageManager::getItemActionsByFormat(textureThumbnailDetails.texelDataFormat(), imageActions);
                                                    GT_ASSERT(rcImageActions);

                                                    // Add thumbnail to the image viewer:
                                                    bool rcAddThumb = addThumbnailItem(pImageDataProxy, imageActions, (void*)pFBOAttachmentItem, topLabel, bottomLabel);
                                                    GT_ASSERT(rcAddThumb);
                                                }
                                            }
                                        }
                                        else
                                        {
                                            // The attachment is a render buffer, load it as texture:
                                            // Get the render buffer ID:
                                            GLuint renderBufferID = pGDFBOAttachmentItem->_objectOpenGLName;

                                            // Get the selected texture details
                                            apGLRenderBuffer renderBufferDetails(0);
                                            rc = gaGetRenderBufferObjectDetails(contextId._contextId, renderBufferID, renderBufferDetails);
                                            GT_IF_WITH_ASSERT(rc)
                                            {
                                                // Generate texture top label and bottom label
                                                gtString topLabel;
                                                gtString bottomLabel;
                                                topLabel.appendFormattedString(GD_STR_ImagesAndBuffersViewerRenderBufferNameFormat, renderBufferID);
                                                generateRenderBufferThumbnailLabel(renderBufferDetails, bottomLabel);

                                                // Create the render buffer image data proxy
                                                gdRenderBufferImageProxy* pImageDataProxy = new gdRenderBufferImageProxy(renderBufferID, renderBufferDetails, contextId._contextId, _isInGLBeginEndBlock);
                                                GT_IF_WITH_ASSERT(pImageDataProxy != NULL)
                                                {
                                                    unsigned int imageActions = 0;
                                                    bool rcImageActions = acImageManager::getItemActionsByFormat(renderBufferDetails.bufferFormat(), imageActions);
                                                    GT_ASSERT(rcImageActions);

                                                    // Add thumbnail to the image viewer:
                                                    bool rcAddThumb = addThumbnailItem(pImageDataProxy, imageActions, (void*)pFBOAttachmentItem, topLabel, bottomLabel);
                                                    GT_ASSERT(rcAddThumb);
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gdTexturesViewer::addRenderBufferToThumbnailView
// Description: Loads and display all available render buffers into the image viewer
// Return Val:  bool - Success / Failure
// Author:      Sigal Algranaty
// Date:        3/6/2008
// ---------------------------------------------------------------------------
bool gdThumbnailView::addRenderBufferToThumbnailView(const afApplicationTreeItemData* pThumbnailItemData)
{
    bool retVal = false;

    // Sanity check
    GT_IF_WITH_ASSERT((pThumbnailItemData != NULL) && (gdDebugApplicationTreeHandler::instance() != NULL))
    {
        gdDebugApplicationTreeData* pGDThumbnailItemData = qobject_cast<gdDebugApplicationTreeData*>(pThumbnailItemData->extendedItemData());
        GT_IF_WITH_ASSERT(pGDThumbnailItemData != NULL)
        {
            retVal = true;

            // Get the item context id:
            apContextID contextId = pGDThumbnailItemData->_contextId;

            // Loop through the render buffers:
            int amountOfDisplatedRenderBuffers = gdDebugApplicationTreeHandler::instance()->amountOfDisplayedObjectForType(contextId, AF_TREE_ITEM_GL_RENDER_BUFFER);

            // Check if progress bar is needed:
            bool shouldShowProgress = (amountOfDisplatedRenderBuffers > GD_MIN_THUMBNAIL_AMOUNT_FOR_PROGRESS_BAR);

            if (shouldShowProgress)
            {
                // Initialize progress dialog:
                afProgressBarWrapper::instance().ShowProgressBar(AF_TREE_ITEM_GL_RENDER_BUFFER, L"Loading", amountOfDisplatedRenderBuffers);
            }

            for (int i = 0; i < amountOfDisplatedRenderBuffers; i++)
            {
                // Get the item data from the tree item id:
                afApplicationTreeItemData* pViewerItem = gdDebugApplicationTreeHandler::instance()->getItemDataByType(contextId, AF_TREE_ITEM_GL_RENDER_BUFFER, i);

                if (pViewerItem != NULL)
                {
                    // Make sure item is a render buffer before loading it
                    if (pViewerItem->m_itemType == AF_TREE_ITEM_GL_RENDER_BUFFER)
                    {
                        gdDebugApplicationTreeData* pGDViewerItem = qobject_cast<gdDebugApplicationTreeData*>(pViewerItem->extendedItemData());
                        GT_IF_WITH_ASSERT(pGDViewerItem != NULL)
                        {
                            // Get the render buffer ID:
                            GLuint renderBufferID = pGDViewerItem->_objectOpenGLName;

                            // Get the selected texture details
                            apGLRenderBuffer renderBufferDetails(0);
                            bool rc = gaGetRenderBufferObjectDetails(contextId._contextId, renderBufferID, renderBufferDetails);
                            GT_IF_WITH_ASSERT(rc)
                            {
                                // Generate texture top label and bottom label
                                gtString topLabel;
                                gtString bottomLabel;
                                topLabel.appendFormattedString(GD_STR_ImagesAndBuffersViewerRenderBufferNameFormat, renderBufferID);
                                generateRenderBufferThumbnailLabel(renderBufferDetails, bottomLabel);

                                // Create the render buffer image data proxy
                                gdRenderBufferImageProxy* pImageDataProxy = new gdRenderBufferImageProxy(renderBufferID, renderBufferDetails, contextId._contextId, _isInGLBeginEndBlock);


                                unsigned int imageActions = 0;
                                bool rcImageActions = acImageManager::getItemActionsByFormat(renderBufferDetails.bufferFormat(), imageActions);
                                GT_ASSERT(rcImageActions);

                                // Add thumbnail to the image viewer:
                                bool rcAddThumb = addThumbnailItem(pImageDataProxy, imageActions, (void*)pViewerItem, topLabel, bottomLabel);
                                GT_ASSERT(rcAddThumb);
                            }
                        }
                    }

                    // Update progress if needed:
                    if (shouldShowProgress)
                    {
                        updateProgressBar(i);
                    }
                }

                // Kill progress if needed:
                if (shouldShowProgress)
                {
                    hideProgressBar();
                }
            }
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gdTexturesViewer::addStaticBuffersToThumbnailView
// Description: Loads and display all available static buffers into the
//              image viewer
//              pThumbnailItemData - the item data representing the static buffers node
// Return Val:  bool - Success / Failure
// Author:      Eran Zinman
// Date:        31/12/2007
// ---------------------------------------------------------------------------
bool gdThumbnailView::addStaticBuffersToThumbnailView(const afApplicationTreeItemData* pThumbnailItemData)
{
    bool retVal = false;

    // Sanity check
    GT_IF_WITH_ASSERT((pThumbnailItemData != NULL) && (gdDebugApplicationTreeHandler::instance() != NULL))
    {
        retVal = true;

        gdDebugApplicationTreeData* pGDThumbnailItemData = qobject_cast<gdDebugApplicationTreeData*>(pThumbnailItemData->extendedItemData());
        GT_IF_WITH_ASSERT(pGDThumbnailItemData != NULL)
        {
            // Get the item context id:
            apContextID contextId = pGDThumbnailItemData->_contextId;

            // Update the static buffer dimensions (this function also updated the static buffers types):
            bool rc = gaUpdateStaticBuffersDimension(contextId._contextId);
            GT_ASSERT(rc);

            // Get amount of displayed static buffers:
            int amountOfDisplayedStaticBuffers = gdDebugApplicationTreeHandler::instance()->amountOfDisplayedObjectForType(contextId, AF_TREE_ITEM_GL_STATIC_BUFFER);

            for (int i = 0; i < amountOfDisplayedStaticBuffers; i++)
            {
                // Get the item data from the tree item id:
                afApplicationTreeItemData* pViewerItem = gdDebugApplicationTreeHandler::instance()->getItemDataByType(contextId, AF_TREE_ITEM_GL_STATIC_BUFFER, i);

                if (pViewerItem != NULL)
                {
                    // Make sure item is a static buffer before loading it
                    if (pViewerItem->m_itemType == AF_TREE_ITEM_GL_STATIC_BUFFER)
                    {
                        gdDebugApplicationTreeData* pGDViewerItem = qobject_cast<gdDebugApplicationTreeData*>(pViewerItem->extendedItemData());
                        GT_IF_WITH_ASSERT(pGDViewerItem != NULL)
                        {
                            // Get the buffer type
                            apDisplayBuffer bufferType = pGDViewerItem->_bufferType;

                            // Get static buffer details
                            apStaticBuffer staticBufferDetails;
                            bool rc1 = gaGetStaticBufferObjectDetails(contextId._contextId, bufferType, staticBufferDetails);
                            GT_IF_WITH_ASSERT(rc1)
                            {
                                // Check if we have an active FBO bound:
                                GLuint activeFBOName = 0;
                                gaGetActiveFBO(contextId._contextId, activeFBOName);
                                bool isFBOBound = (activeFBOName != 0);

                                // Generate static buffer top label
                                gtString topLabel;
                                bool rc2 = apGetBufferName(bufferType, topLabel);
                                GT_IF_WITH_ASSERT(rc2)
                                {
                                    // Create the static buffer image data proxy
                                    gdStaticBufferImageProxy* pImageDataProxy = new gdStaticBufferImageProxy(staticBufferDetails, contextId._contextId, _isInGLBeginEndBlock, isFBOBound);
                                    GT_IF_WITH_ASSERT(pImageDataProxy != NULL)
                                    {
                                        // Check the enabled image actions:
                                        unsigned int imageActions = 0;
                                        bool rcImageActions = acImageManager::getItemActionsByFormat(staticBufferDetails.bufferFormat(), imageActions);
                                        GT_ASSERT(rcImageActions);
                                        // Add thumbnail to the image viewer:
                                        bool rcAddThumb = addThumbnailItem(pImageDataProxy, imageActions, (void*)pViewerItem, topLabel, L"");
                                        GT_ASSERT(rcAddThumb);
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    return retVal;
}



// ---------------------------------------------------------------------------
// Name:        gdTexturesViewer::addVBOsToThumbnailView
// Description: Loads and display all available VBOs into the image viewer
// Arguments:   pThumbnailItemData - the item data representing the VBOs node
// Return Val:  bool - Success / Failure
// Author:      Sigal Algranaty
// Date:        23/4/2009
// ---------------------------------------------------------------------------
bool gdThumbnailView::addVBOsToThumbnailView(const afApplicationTreeItemData* pThumbnailItemData)
{
    bool retVal = true;

    // Sanity check
    GT_IF_WITH_ASSERT((pThumbnailItemData != NULL) && (gdDebugApplicationTreeHandler::instance() != NULL))
    {
        gdDebugApplicationTreeData* pGDThumbnailItemData = qobject_cast<gdDebugApplicationTreeData*>(pThumbnailItemData->extendedItemData());
        GT_IF_WITH_ASSERT(pGDThumbnailItemData != NULL)
        {
            // Get the item context id:
            apContextID contextId = pGDThumbnailItemData->_contextId;

            // Get amount of VBOs:
            int amountOfCurrentVBOs = gdDebugApplicationTreeHandler::instance()->amountOfDisplayedObjectForType(contextId, AF_TREE_ITEM_GL_VBO);

            // Check if progress bar is needed:
            bool shouldShowProgress = (amountOfCurrentVBOs > GD_MIN_THUMBNAIL_AMOUNT_FOR_PROGRESS_BAR);

            if (shouldShowProgress)
            {
                // Initialize progress dialog:
                afProgressBarWrapper::instance().ShowProgressBar(AF_TREE_ITEM_GL_VBO, L"Loading", amountOfCurrentVBOs);
            }

            // Loop through the VBOs:
            for (int i = 0; i < amountOfCurrentVBOs; i++)
            {
                // Get the item data from the tree item id:
                afApplicationTreeItemData* pViewerItem = gdDebugApplicationTreeHandler::instance()->getItemDataByType(contextId, AF_TREE_ITEM_GL_VBO, i);

                if (pViewerItem != NULL)
                {
                    gdDebugApplicationTreeData* pGDViewerItem = qobject_cast<gdDebugApplicationTreeData*>(pViewerItem->extendedItemData());
                    GT_IF_WITH_ASSERT(pGDViewerItem != NULL)
                    {
                        // Is this the item type we are looking for?
                        if (pViewerItem->m_itemType == AF_TREE_ITEM_GL_VBO)
                        {
                            // Get the VBO name:
                            GLuint vboName = pGDViewerItem->_objectOpenGLName;

                            // Get the selected texture details
                            apGLVBO vboDetails;
                            bool rc = gaGetVBODetails(contextId._contextId, vboName, vboDetails);
                            GT_IF_WITH_ASSERT(rc)
                            {
                                // Generate VBO top label and bottom label:
                                gtString topLabel;
                                gtString bottomLabel;
                                topLabel.appendFormattedString(GD_STR_ImagesAndBuffersViewerVBONameFormat, vboName);
                                generateVBOThumbnailLabel(vboDetails, bottomLabel, contextId);

                                // Create the VBO image data proxy
                                gdVBOImageProxy* pImageDataProxy = new gdVBOImageProxy(contextId, vboName);
                                GT_IF_WITH_ASSERT(pImageDataProxy != NULL)
                                {
                                    // Add thumbnail to the image viewer:
                                    rc = addThumbnailItem(pImageDataProxy, 0, (void*)pViewerItem, topLabel, bottomLabel);
                                    GT_ASSERT(rc);
                                }
                            }
                        }
                    }

                    // Update progress bar if needed:
                    if (shouldShowProgress)
                    {
                        updateProgressBar(i);
                    }
                }

                // Kill progress bar if needed:
                if (shouldShowProgress)
                {
                    hideProgressBar();
                }

                retVal = true;
            }
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gdThumbnailView::generateRenderBufferThumbnailLabel
// Description: Generate a render buffer thumbnail bottom label
// Arguments: const apGLRenderBuffer& renderBufferDetails
//            gtString& bottomLabel
// Return Val: void
// Author:      Sigal Algranaty
// Date:        3/6/2008
// ---------------------------------------------------------------------------
void gdThumbnailView::generateRenderBufferThumbnailLabel(const apGLRenderBuffer& renderBufferDetails, gtString& bottomLabel)
{
    // Clear the bottom caption
    bottomLabel.makeEmpty();

    // Get the buffer type:
    apDisplayBuffer bufferType = renderBufferDetails.getBufferType();

    // Get the texture type string
    gtString bufferTypeAsString;
    bool rc = apGetBufferName(bufferType, bufferTypeAsString);

    if (!rc)
    {
        // Unbound render buffer:
        bufferTypeAsString = GD_STR_PropertiesRenderBufferUnAttached;
    }

    // Add the word "Render Buffer" to the texture type
    bottomLabel.append(GD_STR_ImagesAndBuffersViewerThumbnailViewRenderBufferBottomCaption);
    bottomLabel.append(AF_STR_NewLine);

    // Get the render buffer dimensions (width, height)
    GLsizei bufferWidth = 0;
    GLsizei bufferHeight = 0;

    // Retrieve values
    renderBufferDetails.getBufferDimensions(bufferWidth, bufferHeight);

    // Add the Texture width x Texture height string
    bottomLabel.appendFormattedString(GD_STR_ImagesAndBuffersViewerThumbnailViewTexture2DDimensions, bufferWidth, bufferHeight);
}

// ---------------------------------------------------------------------------
// Name:        gdThumbnailView::generateVBOThumbnailLabel
// Description: Generate a render buffer thumbnail bottom label
// Arguments: const apGLVBO& vboDetails - the VBO object
//            gtString& bottomLabel - the label (output)
// Return Val: void
// Author:      Sigal Algranaty
// Date:        3/6/2008
// ---------------------------------------------------------------------------
void gdThumbnailView::generateVBOThumbnailLabel(const apGLVBO& vboDetails, gtString& bottomLabel, const apContextID& contextId)
{
    // Clear the bottom caption
    bottomLabel.makeEmpty();

    // Get the VBO size:
    gtSize_t vboSize = vboDetails.size();

    if (vboSize > 0)
    {
        // We have the information in bits, convert it to kilobytes
        vboSize = (gtSize_t)ceil((float)vboSize / (1024.0F));

        if (vboSize == 0)
        {
            vboSize = 1;
        }
    }

    gtString sizeString;
    sizeString.makeEmpty();


    // Build the object size's string:
    if (vboSize > 0)
    {
        sizeString.appendFormattedString(L"%d", vboSize);
        sizeString.addThousandSeperators();
        sizeString.append(AF_STR_Space AF_STR_KilobytesShort);
    }
    else
    {
        sizeString.append(L"0 " AF_STR_KilobytesShort);
    }

    // Get the VBO attachment:
    GLenum vboAttachment;
    gtVector<GLenum> ignored;
    gaGetVBOAttachment(contextId._contextId, vboDetails.name(), vboAttachment, ignored);

    // Add the VBO last attachment:
    // Get attachment as string:
    gtString strBufferAttachment;

    if (vboAttachment == 0)
    {
        strBufferAttachment = GD_STR_PropertiesRenderBufferUnAttached;
    }
    else
    {
        // Translate the VBO attachment to string:
        apGLenumValueToString(vboAttachment, strBufferAttachment);
    }

    bottomLabel.append(strBufferAttachment);
    bottomLabel.append(AF_STR_NewLine);

    // Add the VBO size string:
    bottomLabel.append(sizeString);
}


// ---------------------------------------------------------------------------
// Name:        gdThumbnailView::generateCLBufferThumbnailLabel
// Description: Generate a buffer thumbnail bottom label
// Arguments: const apCLBuffer& bufferDetails - the buffer object
//            gtString& bottomLabel - the label (output)
// Return Val: void
// Author:      Sigal Algranaty
// Date:        2/12/2009
// ---------------------------------------------------------------------------
void gdThumbnailView::generateCLBufferThumbnailLabel(const apCLBuffer& bufferDetails, gtString& bottomLabel)
{
    // Clear the bottom caption
    bottomLabel.makeEmpty();

    // Get the buffer size:
    gtSize_t bufferSize = bufferDetails.bufferSize();

    if (bufferSize > 0)
    {
        // We have the information in bits, convert it to kilobytes
        bufferSize = (gtSize_t)ceil((float)bufferSize / (1024.0F));

        if (bufferSize == 0)
        {
            bufferSize = 1;
        }
    }

    gtString sizeString;
    sizeString.makeEmpty();

    // Build the object size's string:
    if (bufferSize > 0)
    {
        sizeString.appendFormattedString(L"%d", bufferSize);
        sizeString.addThousandSeperators();
        sizeString.append(AF_STR_Space AF_STR_KilobytesShort);
    }
    else
    {
        sizeString.append(L"0 " AF_STR_KilobytesShort);
    }

    // Add the buffer size string:
    bottomLabel.append(sizeString);
}

// ---------------------------------------------------------------------------
// Name:        gdThumbnailView::generateTextureThumbnailBottomLabel
// Description: Generate a texture thumbnail bottom label
// Arguments:   textureThumbnailDetails - The texture thumbnail details
//              bottomLabel - Output bottom label
// Author:      Eran Zinman
// Date:        20/12/2007
// ---------------------------------------------------------------------------
void gdThumbnailView::generateTextureThumbnailLabel(const apGLTextureMiplevelData& textureThumbnailDetails, gtString& botttomLabel, const apContextID& contextId)
{
    // Clear the bottom caption
    botttomLabel.makeEmpty();

    // Get the texture type (1D, 2D, 3D, CUBE, ...):
    apTextureType textureType = textureThumbnailDetails.textureType();

    // Get the texture type string
    apTextureTypeAsString(textureType, botttomLabel);

    // Add the word "Texture" to the texture type
    botttomLabel.append(GD_STR_ImagesAndBuffersViewerThumbnailViewGLBottomCaption);
    botttomLabel.append(AF_STR_NewLine);

    // Get the texture dimensions (width, height)
    GLsizei texWidth;
    GLsizei texHeight;
    GLsizei texDepth;
    GLsizei borderWidth;
    textureThumbnailDetails.getDimensions(texWidth, texHeight, texDepth, borderWidth);

    if (textureThumbnailDetails.textureType() == AP_BUFFER_TEXTURE)
    {
        // For buffer texture get the size from the attached VBO:
        GLuint bufferName = textureThumbnailDetails.bufferName();
        GLenum openGLDataFormat = textureThumbnailDetails.bufferInternalFormat();

        if (bufferName != 0)
        {
            // Get the VBO details:
            apGLVBO vboDetails;
            bool rc1 = gaGetVBODetails(contextId._contextId, bufferName, vboDetails);
            GT_IF_WITH_ASSERT(rc1)
            {
                texWidth = vboDetails.size();

                GLuint pixelSize = 0;
                bool rc2 = apGetPixelSizeInBitsByInternalFormat(openGLDataFormat, pixelSize);
                GT_IF_WITH_ASSERT(rc2)
                {
                    float newSize = (float)texWidth;
                    newSize = (newSize / (float)pixelSize);
                    texWidth = (GLsizei)floorf(newSize);
                }
                texHeight = 1;
            }
        }
    }

    apTextureType texType = textureThumbnailDetails.textureType();

    // Add the Texture width x Texture height string:
    if ((texType == AP_1D_TEXTURE) || (texType == AP_1D_ARRAY_TEXTURE))
    {
        botttomLabel.appendFormattedString(GD_STR_ImagesAndBuffersViewerThumbnailViewTexture1DDimensions, texWidth);
    }
    else if (texType == AP_3D_TEXTURE)
    {
        botttomLabel.appendFormattedString(GD_STR_ImagesAndBuffersViewerThumbnailViewTexture3DDimensions, texWidth, texHeight, texDepth);
    }
    else
    {
        botttomLabel.appendFormattedString(GD_STR_ImagesAndBuffersViewerThumbnailViewTexture2DDimensions, texWidth, texHeight);
    }
}




// ---------------------------------------------------------------------------
// Name:        gdThumbnailView::generateTextureThumbnailBottomLabel
// Description: Generate a texture thumbnail bottom label
// Arguments:   textureThumbnailDetails - The texture thumbnail details
//              bottomLabel - Output bottom label
// Author:      Sigal Algranaty
// Date:        2/12/2009
// ---------------------------------------------------------------------------
void gdThumbnailView::generateCLImageThumbnailLabel(const apCLImage& textureDetails, gtString& botttomLabel)
{
    // Clear the bottom caption
    botttomLabel.makeEmpty();

    // Get the texture type (2D / 3D):
    apTextureType textureType = textureDetails.imageType();

    // Get the texture type string:
    apTextureTypeAsString(textureType, botttomLabel);

    // Add the word "Texture" to the texture type:
    botttomLabel.append(GD_STR_ImagesAndBuffersViewerThumbnailViewCLBottomCaption);
    botttomLabel.append(AF_STR_NewLine);

    // Get the texture dimensions (width, height, depth):
    gtSize_t texWidth = 0, texHeight = 0, texDepth = 0;
    textureDetails.getDimensions(texWidth, texHeight, texDepth);

    // Add the Texture width x Texture height string:
    if (textureDetails.imageType() == AP_3D_TEXTURE)
    {
        botttomLabel.appendFormattedString(GD_STR_ImagesAndBuffersViewerThumbnailViewTexture3DDimensions, texWidth, texHeight, texDepth);
    }
    else
    {
        botttomLabel.appendFormattedString(GD_STR_ImagesAndBuffersViewerThumbnailViewTexture2DDimensions, texWidth, texHeight);
    }
}

// ---------------------------------------------------------------------------
// Name:        gdTexturesViewer::addCLBuffersToThumbnailView
// Description: Loads and display all available OpenCL buffers into the image viewer
// Return Val:  bool - Success / Failure
// Author:      Sigal Algranaty
// Date:        2/12/2009
// ---------------------------------------------------------------------------
bool gdThumbnailView::addCLBuffersToThumbnailView(const afApplicationTreeItemData* pThumbnailItemData)
{
    bool retVal = true;

    // Sanity check
    GT_IF_WITH_ASSERT((pThumbnailItemData != NULL) && (gdDebugApplicationTreeHandler::instance() != NULL))
    {
        gdDebugApplicationTreeData* pGDThumbnailItemData = qobject_cast<gdDebugApplicationTreeData*>(pThumbnailItemData->extendedItemData());
        GT_IF_WITH_ASSERT(pGDThumbnailItemData != NULL)
        {
            // Get the item context id:
            apContextID contextId = pGDThumbnailItemData->_contextId;

            // Get amount of buffers:
            int amountOfCurrentBuffers = gdDebugApplicationTreeHandler::instance()->amountOfDisplayedObjectForType(contextId, AF_TREE_ITEM_CL_BUFFER);

            // Loop through the VBOs:
            for (int i = 0; i < amountOfCurrentBuffers; i++)
            {
                // Get the item data from the tree item id:
                afApplicationTreeItemData* pViewerItem = gdDebugApplicationTreeHandler::instance()->getItemDataByType(contextId, AF_TREE_ITEM_CL_BUFFER, i);

                if (pViewerItem != NULL)
                {
                    if (pViewerItem != NULL)
                    {
                        gdDebugApplicationTreeData* pGDViewerItem = qobject_cast<gdDebugApplicationTreeData*>(pViewerItem->extendedItemData());
                        GT_IF_WITH_ASSERT(pGDViewerItem != NULL)
                        {
                            // Is this the item type we are looking for?
                            if (pViewerItem->m_itemType == AF_TREE_ITEM_CL_BUFFER)
                            {
                                // Get the buffer index:
                                int bufferIndex = pGDViewerItem->_objectOpenCLIndex;

                                // Get the selected buffer details:
                                apCLBuffer bufferDetails;
                                bool rc = gaGetOpenCLBufferObjectDetails(contextId._contextId, bufferIndex, bufferDetails);
                                GT_IF_WITH_ASSERT(rc)
                                {
                                    // Generate Buffer top label and bottom label
                                    gtString topLabel;
                                    gtString bottomLabel;
                                    gdGetBufferDisplayName(bufferDetails, topLabel, false);
                                    generateCLBufferThumbnailLabel(bufferDetails, bottomLabel);

                                    // Create the buffer image data proxy
                                    gdVBOImageProxy* pImageDataProxy = new gdVBOImageProxy(contextId, bufferIndex);


                                    // Add thumbnail to the image viewer:
                                    rc = addThumbnailItem(pImageDataProxy, 0, (void*)pViewerItem, topLabel, bottomLabel);
                                    GT_ASSERT(rc);
                                }
                            }

                            retVal = true;
                        }
                    }
                }
            }
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gdThumbnailView::applyLastViewedItemProperties
// Description: Apply the last item properties
// Return Val:  void
// Author:      Sigal Algranaty
// Date:        8/11/2010
// ---------------------------------------------------------------------------
void gdThumbnailView::applyLastViewedItemProperties(const acDisplayedItemProperties& lastViewedItemProperties)
{
    // Apply RGBA filters to the images:
    setFilterForAllImages(lastViewedItemProperties._actionsMask, true);
}

// ---------------------------------------------------------------------------
// Name:        gdThumbnailView::onImageItemEvent
// Description: Is implementing item clicked event
// Arguments:   acImageItemID imageItemID
//              const QPoint& posOnImage
//              bool mouseLeftDown
//              bool mouseDoubleClick
// Return Val:  void
// Author:      Sigal Algranaty
// Date:        21/6/2012
// ---------------------------------------------------------------------------
void gdThumbnailView::onImageItemEvent(acImageItemID imageItemID, const QPoint& posOnImage, bool mouseLeftDown, bool mouseDoubleClick)
{
    (void)(posOnImage);  // unused
    (void)(mouseDoubleClick);  // unused

    if (mouseLeftDown)
    {
        // Get the item data associated with the item:
        afApplicationTreeItemData* pViewerItem = (afApplicationTreeItemData*)getItemData(imageItemID);
        GT_IF_WITH_ASSERT((pViewerItem != NULL) && (gdDebugApplicationTreeHandler::instance() != NULL))
        {
            afApplicationCommands* pApplicationCommands = afApplicationCommands::instance();
            GT_IF_WITH_ASSERT(pApplicationCommands != NULL)
            {
                afApplicationTree* pApplicationTree = pApplicationCommands->applicationTree();
                GT_IF_WITH_ASSERT(pApplicationTree != NULL)
                {
                    // Announce the tree that the following click event is a user click:
                    // NOTICE: Sigal 28/12/2010
                    // There is no way to activate a tree item programmatically. Therefore, we select,
                    // and let the tree know that it should reacts as if the user activated.
                    // In VS there is a different reaction to selection and activation, therefore we should do this trick:
                    pApplicationTree->setTreatSelectionAsActivation(true);

                    // Select the item in the navigator tree:
                    bool rc = pApplicationTree->selectItem(pViewerItem, true);
                    GT_ASSERT(rc);

                    pApplicationTree->setTreatSelectionAsActivation(false);
                }
            }
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        gdThumbnailView::onProcessNotSuspended
// Description: Is called when process is running / created / resumed.
//              The view is displaying a message that the data is available only
//              on process suspension
// Return Val:  void
// Author:      Sigal Algranaty
// Date:        29/12/2010
// ---------------------------------------------------------------------------
void gdThumbnailView::onProcessNotSuspended()
{
    // Clear all the thumbnails:
    clearView();

    // Notify the user that the objects are displayed only on process suspension:
    if (gaDebuggedProcessExists())
    {
        writeTextMessage(AF_STR_ImagesAndBuffersViewImagesBuffersProcessIsRunningTitle, AF_STR_ImagesAndBuffersViewImagesBuffersProcessIsRunning);
    }
    else
    {
        writeTextMessage(AF_STR_ImagesAndBuffersViewImagesBuffersProcessIsNotRunningTitle, AF_STR_ImagesAndBuffersViewImagesBuffersProcessIsRunning);
    }

    // Reset the displayed item data:
    _pDisplayedItemData = NULL;
}

// ---------------------------------------------------------------------------
// Name:        gdThumbnailView::updateObjectDisplay
// Description: Is called on process run suspension. The function looks for the
//              object data in the updated monitored object tree, and loads the
//              object, or display a non existing object message
// Return Val:  void
// Author:      Sigal Algranaty
// Date:        29/12/2010
// ---------------------------------------------------------------------------
void gdThumbnailView::updateObjectDisplay()
{
    afApplicationTreeItemData* pDisplayedItemData = NULL;

    // Sanity check:
    if (gdDebugApplicationTreeHandler::instance() != NULL)
    {
        // Look for the displayed object item data:
        pDisplayedItemData = gdDebugApplicationTreeHandler::instance()->FindMatchingTreeItem(_displayedItemId);
    }

    if (pDisplayedItemData != NULL)
    {
        // Set the manager mode:
        setManagerMode(AC_MANAGER_MODE_THUMBNAIL_VIEW);

        // Clear all the thumbnails:
        clearView();

        // Display the existing item:
        displayThumbnailItem(pDisplayedItemData);
    }
    else
    {
        // Clear the thumbnail view:
        clearView();

        // Get the object id as string:
        gtString itemDisplayString;
        bool rc = gdHTMLProperties::itemIDAsString(_displayedItemId, itemDisplayString);
        GT_IF_WITH_ASSERT(rc)
        {
            // Build the object unavailable message:
            gtString msg;
            msg.appendFormattedString(AF_STR_ImagesAndBuffersViewObjectsNotAvailable, itemDisplayString.asCharArray());

            // Display the message:
            writeTextMessage(msg);
        }
    }
}

