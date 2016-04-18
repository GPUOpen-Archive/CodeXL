//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gdImageAndBufferView.cpp
///
//==================================================================================

//------------------------------ gdImageAndBufferView.cpp ------------------------------

// Qt
#include <AMDTApplicationComponents/Include/acQtIncludes.h>

// Infra:
#include <AMDTAPIClasses/Include/apCLBuffer.h>
#include <AMDTAPIClasses/Include/apCLImage.h>
#include <AMDTAPIClasses/Include/apCLSubBuffer.h>
#include <AMDTAPIClasses/Include/apGLFBO.h>
#include <AMDTAPIClasses/Include/apGLRenderBuffer.h>
#include <AMDTAPIClasses/Include/apGLVBO.h>
#include <AMDTAPIClasses/Include/apStaticBuffer.h>
#include <AMDTAPIClasses/Include/Events/apEventsHandler.h>
#include <AMDTApiFunctions/Include/gaGRApiFunctions.h>
#include <AMDTApplicationComponents/Include/acRawDataExporter.h>
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTOSWrappers/Include/osDebugLog.h>
#include <AMDTApplicationComponents/Include/acFunctions.h>

// AMDTApplicationFramework:
#include <AMDTApplicationFramework/Include/afAppStringConstants.h>
#include <AMDTApplicationFramework/Include/afProgressBarWrapper.h>

// Local:
#include <AMDTGpuDebuggingComponents/Include/gdAidFunctions.h>
#include <AMDTGpuDebuggingComponents/Include/gdCommandIDs.h>
#include <AMDTGpuDebuggingComponents/Include/gdGDebuggerGlobalVariablesManager.h>
#include <AMDTGpuDebuggingComponents/Include/gdHTMLProperties.h>
#include <AMDTGpuDebuggingComponents/Include/gdUpdateUIEvent.h>
#include <AMDTGpuDebuggingComponents/Include/views/gdImageAndBufferView.h>

// Image control panel default height:
#define GD_IMAGES_AND_BUFFERS_MAXIMUM_MANAGER_SIZE 10000

// ---------------------------------------------------------------------------
// Name:        gdImageAndBufferView::gdImageAndBufferView
// Description: Constructor
// Arguments:   pParent - The view's parent
// Author:      Sigal Algranaty
// Date:        8/11/2010
// ---------------------------------------------------------------------------
gdImageAndBufferView::gdImageAndBufferView(QWidget* pParent, afProgressBarWrapper* pProgressBar, gdDebugApplicationTreeHandler* pObjectsTree)
    : gdImageDataView(pParent, pProgressBar),
      _isInGLBeginEndBlock(false), _isInKernelDebugging(false), _isLastMipLevelFailed(false)
{
    (void)(pObjectsTree);  // unused
}

// ---------------------------------------------------------------------------
// Name:        gdImageAndBufferView::~gdImageAndBufferView
// Description: Destructor
// Author:      Sigal Algranaty
// Date:        8/11/2010
// ---------------------------------------------------------------------------
gdImageAndBufferView::~gdImageAndBufferView()
{
}

// ---------------------------------------------------------------------------
// Name:        gdImageAndBufferView::displayItem
// Description: Display a buffer / image item
// Arguments:   pThumbnailItemData
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        8/11/2010
// ---------------------------------------------------------------------------
bool gdImageAndBufferView::displayItem(afApplicationTreeItemData* pDisplayedItemData)
{
    bool retVal = false;

    // Set my item data:
    _pDisplayedItemTreeData = pDisplayedItemData;

    if (_pDisplayedItemTreeData != NULL)
    {
        _pDisplayedItemTreeData->setExtendedData(pDisplayedItemData->extendedItemData());

        gdDebugApplicationTreeData* pGDData = qobject_cast<gdDebugApplicationTreeData*>(pDisplayedItemData->extendedItemData());
        GT_IF_WITH_ASSERT(pGDData != NULL)
        {
            // Copy the data relevant for identification of the object:
            _pDisplayedItemTreeData->copyID(_displayedItemId);

            // Set the "Is in glBegin-end" flag:
            _isInGLBeginEndBlock = gaIsInOpenGLBeginEndBlock(pGDData->_contextId._contextId);

            // Set the "Is in kernel debugging" flag:
            _isInKernelDebugging = gaIsInKernelDebugging();

            // Check the current status of the debugged process:
            bool isDebuggedProcessSuspended = gaIsDebuggedProcessSuspended();

            if (!isDebuggedProcessSuspended)
            {
                // Get the appropriate failure description:
                afItemLoadFailureDescription failureDescription = gaDebuggedProcessExists() ? AF_ITEM_LOAD_PROCESS_IS_RUNNING : AF_ITEM_LOAD_PROCESS_IS_TERMINATED;

                pDisplayedItemData->setItemLoadStatus(AF_ITEM_LOAD_ERROR, failureDescription);
            }

            if (_pDisplayedItemTreeData->_itemLoadStatus._itemLoadStatusType != AF_ITEM_LOAD_ERROR)
            {
                // Clear all items:
                _pImageViewManager->setManagerMode(AC_MANAGER_MODE_STANDARD_ITEM);
                _pImageViewManager->clearAllObjects();

                switch (_pDisplayedItemTreeData->m_itemType)
                {

                    case AF_TREE_ITEM_GL_TEXTURE:
                    case AF_TREE_ITEM_CL_IMAGE:
                        retVal = loadTextureObject();
                        break;

                    case AF_TREE_ITEM_GL_STATIC_BUFFER:
                        retVal = loadStaticBuffer();
                        break;

                    case AF_TREE_ITEM_GL_RENDER_BUFFER:
                        retVal = loadRenderBuffer();
                        break;

                    case AF_TREE_ITEM_GL_FBO_ATTACHMENT:
                        retVal = loadFBOAttachmentBuffer();
                        break;

                    case AF_TREE_ITEM_GL_PBUFFER_STATIC_BUFFER:
                        retVal = loadPBufferStaticBuffer();
                        break;

                    case AF_TREE_ITEM_GL_VBO:
                        retVal = loadVBO();
                        break;

                    case AF_TREE_ITEM_CL_BUFFER:
                        retVal = loadCLBuffer();
                        break;

                    case AF_TREE_ITEM_CL_SUB_BUFFER:
                        retVal = loadCLSubBuffer();
                        break;

                    default:
                    {
                        GT_ASSERT_EX(false, L"Unknown item type!");
                        retVal = false;
                    }
                    break;
                }
            }

            // Setup the control panel with the loaded item attributes:
            setupControlPanel();

            // Write the failure message:
            displayItemMessageIfNeeded();
        }
    }
    else
    {
        _pDisplayedItemTreeData = _pStaticEmptyItemData;
        GT_IF_WITH_ASSERT(pDisplayedItemData != NULL)
        {
            _pDisplayedItemTreeData->setItemLoadStatus(_pDisplayedItemTreeData->_itemLoadStatus._itemLoadStatusType, _pDisplayedItemTreeData->_itemLoadStatus._loadStatusDescription);
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gdImageAndBufferView::loadTextureFile
// Description: Loads a texture raw data file into the image and data views,
//              inserting the image at a certain matrix position.
// Arguments:   filePath - Path of the texture to load
//              objectName - The object name
//              matrixInsertPos - Matrix position to insert texture
// Return Val:  bool - Success / Failure
// Author:      Eran Zinman
// Date:        20/12/2007
// ---------------------------------------------------------------------------
bool gdImageAndBufferView::loadTextureFile(const osFilePath& filePath, const gtString& objectName, QPoint matrixInsertPos, int index, int indexStride)
{
    bool retVal = true;

    // Sanity check:
    GT_IF_WITH_ASSERT(_pDisplayedItemTreeData != NULL)
    {
        gdDebugApplicationTreeData* pGDData = qobject_cast<gdDebugApplicationTreeData*>(_pDisplayedItemTreeData->extendedItemData());
        GT_IF_WITH_ASSERT(pGDData != NULL)
        {
            // Check if this item is a texture, or an FBO texture attachment:
            if ((pGDData->_textureType != AP_UNKNOWN_TEXTURE_TYPE) || (_pDisplayedItemTreeData->m_itemType == AF_TREE_ITEM_GL_FBO))
            {
                retVal = false;

                // Load the raw data into the textures and buffer viewer.
                // Important note: We are not responsible for releasing the raw file handler here,
                // the textures and buffers viewer will release the object later on.
                acRawFileHandler* pRawFileHandler = new acRawFileHandler;

                // Load raw data from file
                bool rc1 = pRawFileHandler->loadFromFile(filePath);
                GT_IF_WITH_ASSERT(rc1)
                {
                    // If raw data was loaded successfully
                    GT_IF_WITH_ASSERT(pRawFileHandler->isOk())
                    {
                        // Apply page stride and offset:
                        if (1 < indexStride)
                        {
                            pRawFileHandler->setPageStride(indexStride, index);
                        }

                        // Load the texture to image data views
                        retVal = loadItemToImageAndDataViews(pRawFileHandler, objectName, matrixInsertPos);
                    }
                }
            }
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gdImageAndBufferView::calcTextureElementMatrixPosAndLabel
// Description: Calculate the matrix position of a texture element and it's
//              top label (which will be displayed in the image manager).
// Arguments: apTextureType textureType
//            int elementIndex
//            QPoint& matrixPos
//            gtString& topLabel
// Return Val: bool  - Success / failure.
// Author:      Eran Zinman
// Date:        20/12/2007
// ---------------------------------------------------------------------------
bool gdImageAndBufferView::calcTextureElementMatrixPosAndLabel(apTextureType textureType, int elementIndex, QPoint& matrixPos, gtString& topLabel)
{
    bool retVal = false;

    // Range check:
    bool rc1 = ((elementIndex >= 0) && (elementIndex < apGLTextureMipLevel::AP_MAX_AMOUNT_OF_TEXTURE_FACES));
    GT_IF_WITH_ASSERT(rc1)
    {
        // Convert element index to apTextureFaceIndex enum
        apGLTextureMipLevel::apTextureFaceIndex textureFileIndex = (apGLTextureMipLevel::apTextureFaceIndex)elementIndex;

        // All texture types except from CubeMap textures are supposed to have only one element
        if ((textureType == AP_UNKNOWN_TEXTURE_TYPE) || (textureType == AP_1D_TEXTURE) ||
            (textureType == AP_2D_TEXTURE) || (textureType == AP_3D_TEXTURE) || (textureType == AP_1D_ARRAY_TEXTURE) || (textureType == AP_2D_ARRAY_TEXTURE) ||
            (textureType == AP_2D_TEXTURE_MULTISAMPLE) || (textureType == AP_2D_TEXTURE_MULTISAMPLE_ARRAY) ||
            (textureType == AP_TEXTURE_RECTANGLE) || (textureType == AP_BUFFER_TEXTURE))
        {
            // Make sure we are handling with only one element for these texture types
            if (textureFileIndex == apGLTextureMipLevel::AP_SINGLE_TEXTURE_FACE_INDEX)
            {
                // Just put the texture element in the [0, 0] position
                matrixPos = QPoint(0, 0);

                // These textures type don't have a top label, just make it empty
                topLabel.makeEmpty();

                retVal = true;
            }
        }
        else if ((AP_CUBE_MAP_TEXTURE == textureType) || (AP_CUBE_MAP_ARRAY_TEXTURE == textureType))
        {
            // Cube map texture layout
            // =======================
            // The Cube map textures are displayed as an open box, therefore they will
            // be organized in the following places in the texture matrix:
            //
            // [   0 1 2 3]             Legend (Row, Column):
            // [ |--------]             ---------------------
            // [0|   #    ]             (1, 0) - Positive X,  (1, 2) - Negative X
            // [1| # # # #]             (0, 1) - Positive Y,  (2, 1) - Negative Y
            // [2|   #    ]             (1, 1) - Positive Z,  (1, 3) - Negative Z

            retVal = true;

            // Set the top label:
            topLabel = acQStringToGTString(_dataViewPagesNames[textureFileIndex]);

            // Set the cube map element matrix position and top label
            switch (textureFileIndex)
            {
                case apGLTextureMipLevel::AP_TEXTURE_CUBE_MAP_POSITIVE_X_FACE_INDEX:
                {
                    matrixPos = QPoint(1, 0);
                }
                break;

                case apGLTextureMipLevel::AP_TEXTURE_CUBE_MAP_NEGATIVE_X_FACE_INDEX:
                {
                    matrixPos = QPoint(1, 2);
                }
                break;

                case apGLTextureMipLevel::AP_TEXTURE_CUBE_MAP_POSITIVE_Y_PANE_INDEX:
                {
                    matrixPos = QPoint(0, 1);
                }
                break;

                case (apGLTextureMipLevel::AP_TEXTURE_CUBE_MAP_NEGATIVE_Y_PANE_INDEX):
                {
                    matrixPos = QPoint(2, 1);
                }
                break;

                case apGLTextureMipLevel::AP_TEXTURE_CUBE_MAP_POSITIVE_Z_PANE_INDEX:
                {
                    matrixPos = QPoint(1, 1);
                }
                break;

                case (apGLTextureMipLevel::AP_TEXTURE_CUBE_MAP_NEGATIVE_Z_FACE_INDEX):
                {
                    matrixPos = QPoint(1, 3);
                }
                break;

                default:
                {
                    // Unknown cube map element
                    GT_ASSERT_EX(false, L"unknown cube map element!");
                    retVal = false;
                }
                break;
            }
        }
        else
        {
            // Unknown texture format
            GT_ASSERT_EX(false, L"Unknown texture format!");
            retVal = false;
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gdImageAndBufferView::loadTextureElements
// Description: Loads all texture elements into the image and data views.
//              Usually a texture consist only of one file, except from
//              cube map texture which consist of 6 files.
// Arguments:   textureID - the texture id
// Return Val:  bool - Success / Failure
// Author:      Eran Zinman
// Date:        20/12/2007
// ---------------------------------------------------------------------------
bool gdImageAndBufferView::loadTextureElements()
{
    bool retVal = false;

    // Sanity check:
    GT_IF_WITH_ASSERT(_pDisplayedItemTreeData != NULL)
    {
        gdDebugApplicationTreeData* pGDData = qobject_cast<gdDebugApplicationTreeData*>(_pDisplayedItemTreeData->extendedItemData());
        GT_IF_WITH_ASSERT(pGDData != NULL)
        {
            // Initialize the item status as not loaded:
            _pDisplayedItemTreeData->setItemLoadStatus(AF_ITEM_NOT_LOADED);

            // We cannot load a texture in glBegin-glEnd block
            if (!_isInGLBeginEndBlock)
            {
                retVal = false;

                if (pGDData->_contextId.isOpenGLContext())
                {
                    // Get texture thumbnail data details:
                    apGLTextureMiplevelData textureThumbnailDetails;
                    bool rc = gaGetTextureObjectThumbnailData(pGDData->_contextId._contextId, pGDData->_textureMiplevelID._textureName, textureThumbnailDetails);
                    GT_IF_WITH_ASSERT(rc)
                    {
                        // Get amount of texture elements:
                        int amountOfIndicies = textureThumbnailDetails.amountOfTextureDataFiles();
                        int indexStride = 1;

                        // Calculate how much every "texture" is worth in the progress dialog (we need to fill 60%)
                        float unitValue = (float)60.0f / (float)amountOfIndicies;

                        apTextureType texType = textureThumbnailDetails.textureType();

                        if (AP_CUBE_MAP_ARRAY_TEXTURE == texType)
                        {
                            GT_ASSERT(1 == amountOfIndicies);
                            amountOfIndicies = 6;
                            indexStride = 6;
                        }

                        // Loop through the texture elements:
                        for (int i = 0; i < amountOfIndicies; i++)
                        {
                            // Get texture raw data element file name:
                            osFilePath textureFilePath;
                            int j = (AP_CUBE_MAP_TEXTURE == texType) ? i : 0;
                            bool rc0 = gaGetTextureMiplevelDataFilePath(pGDData->_contextId._contextId, pGDData->_textureMiplevelID, j, textureFilePath);
                            GT_ASSERT(rc0);

                            // Localize the path as needed:
                            gaRemoteToLocalFile(textureFilePath, false);

                            // Calculate the texture element matrix position and top label:
                            QPoint matrixPos(0, 0);
                            gtString topLabel;
                            bool rc1 = calcTextureElementMatrixPosAndLabel(texType, i, matrixPos, topLabel);
                            GT_ASSERT(rc1);

                            // Load texture element
                            bool rc2 = loadTextureFile(textureFilePath, topLabel, matrixPos, i, indexStride);

                            if (rc2)
                            {
                                // Even if only one file was loaded successfully, return true
                                _pDisplayedItemTreeData->setItemLoadStatus(AF_ITEM_LOAD_SUCCESS);
                                retVal = true;
                            }

                            // Update the progress bar - we need to get from 30% to 90%
                            int progressValue = 30 + (int)((float)(i + 1) * unitValue);
                            updateProgressBar(progressValue);
                        }
                    }
                }
                // OpenCL:
                else if (pGDData->_contextId.isOpenCLContext())
                {
                    // Get the OpenCL texture object:
                    apCLImage textureDetails;
                    bool rc = gaGetOpenCLImageObjectDetails(pGDData->_contextId._contextId, pGDData->_objectOpenCLIndex, textureDetails);

                    // Get texture raw data element file name:
                    osFilePath textureFile;
                    textureDetails.imageFilePath(textureFile);

                    // Localize the path as needed:
                    gaRemoteToLocalFile(textureFile, false);

                    // Calculate the texture element matrix position and top label:
                    QPoint matrixPos(0, 0);
                    gtString topLabel;
                    rc = calcTextureElementMatrixPosAndLabel(textureDetails.imageType(), 0, matrixPos, topLabel);
                    GT_ASSERT(rc);

                    // Load texture element
                    bool rc2 = loadTextureFile(textureFile, topLabel, matrixPos, 0, 1);

                    if (rc2)
                    {
                        // Even if only one file was loaded successfully, return true
                        _pDisplayedItemTreeData->setItemLoadStatus(AF_ITEM_LOAD_SUCCESS);
                        retVal = true;
                    }

                    // Update the progress bar - we need to get from 30% to 90%
                    updateProgressBar(100);
                }
                else // pGDData->_contextId.isDefault()
                {
                    // There should be no texture elements in the NULL context:
                    GT_ASSERT_EX(false, L"Should not get here");
                }
            }
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gdImageDataView::displayCurrentTextureMiplevel
// Description: Displays currently displayed texture with another mip level:
// Arguments: int miplevel
// Return Val: void
// Author:      Sigal Algranaty
// Date:        12/1/2009
// ---------------------------------------------------------------------------
void gdImageAndBufferView::displayCurrentTextureMiplevel(int miplevel, bool forceReload)
{
    // Sanity check:
    GT_IF_WITH_ASSERT((_pDisplayedItemTreeData != NULL) && (m_pTabWidget != NULL))
    {
        gdDebugApplicationTreeData* pGDData = qobject_cast<gdDebugApplicationTreeData*>(_pDisplayedItemTreeData->extendedItemData());
        GT_IF_WITH_ASSERT(pGDData != NULL)
        {
            // Stop me from updating my display we are done:
            m_pTabWidget->setEnabled(false);

            int currentlyDisplayedMipLevel = pGDData->_textureMiplevelID._textureMipLevel;

            if ((miplevel != currentlyDisplayedMipLevel) || forceReload)
            {
                // Set the new mip level:
                pGDData->_textureMiplevelID._textureMipLevel = miplevel;

                // Override the data cache that was probably set to true while displaying other texture mip level:
                // This is not accurate. actually, the caching mechanism should be set for each mip id, but we carry viewer data for each texture
                pGDData->_isDataCached = false;

                bool bRes = LoadTexture(miplevel);

                if (!bRes)// failed to load texture for new miplevel
                {
                    clearView();

                    if (pGDData->_contextId.isOpenGLContext())
                    {
                        // if the item was not loaded successfully display the message:
                        gtString textureHeading;

                        if (miplevel > 0)
                        {
                            textureHeading.appendFormattedString(GD_STR_ImagesAndBuffersViewerTextureMipmapError, miplevel);
                            _isLastMipLevelFailed = true;
                        }

                        m_pTabWidget->setCurrentIndex(0);
                        _pImageViewManager->writeTextMessage(textureHeading);
                    }

                    return;
                }
                else
                {
                    if (_isLastMipLevelFailed)
                    {
                        updateObjectDisplay(bRes);
                        _isLastMipLevelFailed = false;
                    }
                }
            }

            // Apply a dummy event size:
            _pImageViewManager->layout();

            // Adjust the image and data views after loading the item
            adjustViewerAfterItemLoading();

            // Adjust the manager layout after adding the texture
            forceImageManagerRepaint();

            // Unfreeze me, update the view
            m_pTabWidget->setEnabled(true);

            // Apply the last item selected properties (to keep the filtering, rotations settings done for the
            // texture image):
            // Do not apply zoom settings, since the new mip level has other dimensions and we don't want to
            // override to best fit applied on it:
            applyLastViewedItemProperties(_lastViewedItemProperties, false);

            // Apply best fit for the current mip level:
            applyBestFit(_currentZoomLevel);

            // Set the combo zoom level:
            setTextureManagerZoomLevel(_currentZoomLevel);

            // Save the last viewed item properties:
            saveLastViewedItemProperties(_currentZoomLevel);

            // Trigger an update ui event:
            gdUpdateUIEvent updateUIEvent;
            apEventsHandler::instance().registerPendingDebugEvent(updateUIEvent);
        }
    }
}


// ---------------------------------------------------------------------------
// Name:        gdImageAndBufferView::loadTextureObject
// Description: Loads a texture object
// Arguments:   gdDebugApplicationTreeData* _pDisplayedItemTreeData
// Return Val: bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        9/8/2009
// ---------------------------------------------------------------------------
bool gdImageAndBufferView::loadTextureObject()
{
    bool retVal = false;
    GT_IF_WITH_ASSERT(_pDisplayedItemTreeData != NULL)
    {
        gdDebugApplicationTreeData* pGDData = qobject_cast<gdDebugApplicationTreeData*>(_pDisplayedItemTreeData->extendedItemData());
        GT_IF_WITH_ASSERT(pGDData != NULL)
        {
            if ((pGDData->_textureType == AP_BUFFER_TEXTURE) && (pGDData->_contextId.isOpenGLContext()))
            {
                retVal = loadTexBuffer();
            }
            else
            {
                retVal = LoadTexture(-1);
            }
        }
    }
    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gdImageAndBufferView::loadCLBuffer
// Description: Loads a CL buffer to the image viewer
// Arguments: gdDebugApplicationTreeData* _pDisplayedItemTreeData
// Return Val: bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        2/12/2009
// ---------------------------------------------------------------------------
bool gdImageAndBufferView::loadCLBuffer()
{
    bool retVal = false;
    bool isInteropObject = false;

    // Sanity check:
    GT_IF_WITH_ASSERT(_pDisplayedItemTreeData != NULL)
    {
        gdDebugApplicationTreeData* pGDData = qobject_cast<gdDebugApplicationTreeData*>(_pDisplayedItemTreeData->extendedItemData());
        GT_IF_WITH_ASSERT(pGDData != NULL)
        {
            // Initialize the VBO load status as not loaded:
            _pDisplayedItemTreeData->setItemLoadStatus(AF_ITEM_LOAD_ERROR);

            apCLBuffer clBufferDetails;

            // Initialize progress dialog:
            afProgressBarWrapper::instance().ShowProgressBar(AF_TREE_ITEM_CL_BUFFER, L"Loading", 1);

            // If the buffer is shared with GL do not display it:
            // NOTICE: This feature is not supported until the driver is fixed to support the retrieve of shared objects content:
            isInteropObject = (pGDData->_objectOpenGLName > 0);

            if (isInteropObject)
            {
                // Do not display CL shared items:
                _pDisplayedItemTreeData->setItemLoadStatus(AF_ITEM_NOT_LOADED, AF_ITEM_LOAD_CL_GL_INTEROP);
            }
            else
            {
                // Set progress bar at 20%:
                updateProgressBar(20);

                // Extract buffer raw data to disk:
                bool rc1 = exportSpyData(_pDisplayedItemTreeData);

                // Get the buffer details:
                bool rc2 = gaGetOpenCLBufferObjectDetails(pGDData->_contextId._contextId, pGDData->_objectOpenCLIndex, clBufferDetails);

                if (rc1)
                {
                    GT_IF_WITH_ASSERT(rc2)
                    {
                        // Set progress bar at 30%:
                        updateProgressBar(30);

                        // Get the buffer filename:
                        osFilePath bufferFile;
                        clBufferDetails.getBufferFilePath(bufferFile);

                        // Localize the path as needed:
                        gaRemoteToLocalFile(bufferFile, false);

                        // Get the buffer display properties:
                        oaTexelDataFormat bufferDataFormat = OA_TEXEL_FORMAT_V1F;
                        int offset = 0;
                        gtSize_t stride = 0;
                        clBufferDetails.getBufferDisplayProperties(bufferDataFormat, offset, stride);

                        // Load buffer file to data views:
                        retVal = loadBufferFile(bufferFile, bufferDataFormat, offset, stride);

                        if (retVal)
                        {
                            _pDisplayedItemTreeData->setItemLoadStatus(AF_ITEM_LOAD_SUCCESS);
                        }

                        // Set progress bar at 90%
                        updateProgressBar(90);
                    }
                }
                else
                {
                    OS_OUTPUT_DEBUG_LOG(GD_STR_LogMsg_couldNotExportSpyData, OS_DEBUG_LOG_DEBUG);
                }
            }

            // If buffer was not loaded successfully, display a message to the user
            if (!retVal)
            {
                if (!isInteropObject)
                {
                    setObjectNotLoadedStatus();
                }

                retVal = true;
            }

            // End progress bar operation
            hideProgressBar();
        }
    }

    return retVal;
}



// ---------------------------------------------------------------------------
// Name:        gdImageAndBufferView::loadCLSubBuffer
// Description: Loads a CL sub-buffer to the image viewer
// Return Val:  bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        11/1/2011
// ---------------------------------------------------------------------------
bool gdImageAndBufferView::loadCLSubBuffer()
{
    bool retVal = false;

    // Sanity check:
    GT_IF_WITH_ASSERT(_pDisplayedItemTreeData != NULL)
    {
        gdDebugApplicationTreeData* pGDData = qobject_cast<gdDebugApplicationTreeData*>(_pDisplayedItemTreeData->extendedItemData());
        GT_IF_WITH_ASSERT(pGDData != NULL)
        {
            // Initialize the VBO load status as not loaded:
            _pDisplayedItemTreeData->setItemLoadStatus(AF_ITEM_LOAD_ERROR);

            apCLSubBuffer clSubBufferDetails;

            // Initialize progress dialog:
            afProgressBarWrapper::instance().ShowProgressBar(AF_TREE_ITEM_CL_SUB_BUFFER, L"Loading", 1);

            // Set progress bar at 20%:
            updateProgressBar(20);

            // Extract buffer raw data to disk:
            bool rc1 = exportSpyData(_pDisplayedItemTreeData);

            // Get the buffer details:
            bool rc2 = gaGetOpenCLSubBufferObjectDetails(pGDData->_contextId._contextId, pGDData->_objectOpenCLIndex, clSubBufferDetails);

            if (rc1)
            {
                GT_IF_WITH_ASSERT(rc2)
                {
                    // Set progress bar at 30%:
                    updateProgressBar(30);

                    // Get the buffer filename:
                    osFilePath bufferFile;
                    clSubBufferDetails.getSubBufferFilePath(bufferFile);

                    // Localize the path as needed:
                    gaRemoteToLocalFile(bufferFile, false);

                    // Get the buffer display properties:
                    oaTexelDataFormat bufferDataFormat = OA_TEXEL_FORMAT_V1F;
                    int offset = 0;
                    gtSize_t stride = 0;
                    clSubBufferDetails.getSubBufferDisplayProperties(bufferDataFormat, offset, stride);

                    // Load buffer file to data views:
                    retVal = loadBufferFile(bufferFile, bufferDataFormat, offset, stride);

                    if (retVal)
                    {
                        _pDisplayedItemTreeData->setItemLoadStatus(AF_ITEM_LOAD_SUCCESS);
                    }

                    // Set progress bar at 90%
                    updateProgressBar(90);
                }
            }
            else
            {
                OS_OUTPUT_DEBUG_LOG(GD_STR_LogMsg_couldNotExportSpyData, OS_DEBUG_LOG_DEBUG);
            }

            // If buffer was not loaded successfully, display a message to the user
            if (!retVal)
            {
                setObjectNotLoadedStatus();
                retVal = true;
            }

            // End progress bar operation
            hideProgressBar();
        }
    }

    return retVal;
}

bool gdImageAndBufferView::LoadTexture(int newMipLevel)
{
    bool retVal = false;
    // Check if the object is a CL-GL shared object:
    bool isInteropObject = false;

    // Sanity check:
    GT_IF_WITH_ASSERT(_pDisplayedItemTreeData != NULL)
    {
        gdDebugApplicationTreeData* pGDData = qobject_cast<gdDebugApplicationTreeData*>(_pDisplayedItemTreeData->extendedItemData());
        GT_IF_WITH_ASSERT(pGDData != NULL)
        {
            // If the buffer is shared with GL do not display it:
            // NOTICE: This feature is not supported until the driver is fixed to support the retrieve of shared objects content:
            isInteropObject = (_pDisplayedItemTreeData->m_itemType == AF_TREE_ITEM_CL_IMAGE) && (pGDData->_objectOpenGLName > 0);
            isInteropObject = ((_pDisplayedItemTreeData->m_itemType == AF_TREE_ITEM_GL_TEXTURE) && (pGDData->_objectOpenCLName > 0)) || isInteropObject;

            if (isInteropObject)
            {
                // Do not display CL shared items:
                _pDisplayedItemTreeData->setItemLoadStatus(AF_ITEM_NOT_LOADED, AF_ITEM_LOAD_CL_GL_INTEROP);
            }
            else
            {
                apGLTextureMipLevelID textureID;

                // Initialize progress dialog:
                afProgressBarWrapper::instance().ShowProgressBar(AF_TREE_ITEM_GL_TEXTURE, L"Loading", 1);

                // Make sure texture data is cached to disk:
                bool rc1 = exportSpyData(_pDisplayedItemTreeData);

                if (!rc1)
                {
                    OS_OUTPUT_DEBUG_LOG(GD_STR_LogMsg_couldNotExportSpyData, OS_DEBUG_LOG_DEBUG);
                }

                // Update progress bar to 20%:
                updateProgressBar(20);

                // Get the texture ID:
                if (pGDData->_contextId.isOpenGLContext())
                {
                    textureID = pGDData->_textureMiplevelID;

                    // Update the texture parameters:
                    gtVector<apGLTextureMipLevelID> texturesVector;
                    texturesVector.push_back(textureID);
                    bool rc2 = gaUpdateTextureParameters(pGDData->_contextId._contextId, texturesVector, false);
                    GT_ASSERT(rc2);
                }
                else if (pGDData->_contextId.isOpenCLContext())
                {
                    // Get the texture name:
                    int textureIndex = pGDData->_objectOpenCLIndex;

                    // Set the texture ID:
                    textureID._textureMipLevel = 0;
                    textureID._textureName = textureIndex;
                }
                else // pGDData->_contextId.isDefault();
                {
                    GT_ASSERT(false);
                }

                // If the texture / image is shared with CL/GL do not display it:
                // NOTICE: This feature is not supported until the driver is fixed to support the retrieve of shared objects content:
                if (isInteropObject)
                {
                    // Do not display CL shared items:
                    _pDisplayedItemTreeData->setItemLoadStatus(AF_ITEM_NOT_LOADED, AF_ITEM_LOAD_CL_GL_INTEROP);
                }

                else
                {
                    // Adjust the image manager layout:
                    bool rc3 = setImageViewMode(pGDData->_textureType);
                    GT_IF_WITH_ASSERT(rc3)
                    {
                        // Set progress bar to 30%:
                        updateProgressBar(30);

                        // Loads all texture elements into the image and data views
                        retVal = loadTextureElements();

                        if (retVal)
                        {
                            // Update the texture heading
                            UpdateTextureHeading(pGDData, newMipLevel);
                        }
                    }

                    // Set progress bar to 95%:
                    updateProgressBar(95);
                }
            }

            // If texture was not loaded successfully, display a message to the user
            if (!retVal && !isInteropObject)
            {
                setObjectNotLoadedStatus();
                retVal = false;
            }

            // End progress bar operation
            hideProgressBar();
        }
    }

    return retVal;
}


void gdImageAndBufferView::UpdateTextureHeading(gdDebugApplicationTreeData* pGDData, int newMipLevel)
{
    // Sanity check:
    GT_IF_WITH_ASSERT((pGDData != nullptr) && (_pImageViewManager != nullptr))
    {
        apGLTextureMipLevelID textureID;
        gtString textureHeading;
        int textureLayer = pGDData->_textureLayer;

        // Get the texture type and update the texture heading
        apTextureType textureType = pGDData->_textureType;
        bool isTextureArray = (textureType == AP_2D_ARRAY_TEXTURE) || (textureType == AP_1D_ARRAY_TEXTURE);
        apContextID contextID = pGDData->_contextId;

        // Build the basis for the texture name (type + index)
        if (contextID.isOpenGLContext())
        {
            textureID = pGDData->_textureMiplevelID;

            gdHTMLProperties htmlProps;
            int clImageIndex = -1, clImageName = -1, clSpyID = -1;
            apGLTextureData textureData;
            bool rc = gaGetTextureDataObjectDetails(pGDData->_contextId._contextId, textureID._textureName, textureData);
            GT_IF_WITH_ASSERT(rc)
            {
                textureData.getCLDetails(clImageIndex, clImageName, clSpyID);
            }
            htmlProps.getGLTextureName(textureID, clImageName, clSpyID, textureHeading);
            textureHeading.prepend(AF_STR_Space);
            gtString contextStr;
            pGDData->_contextId.toString(contextStr);
            textureHeading.prepend(contextStr);
        }
        else if (contextID.isOpenCLContext())
        {
            // Get the texture name:
            int textureIndex = pGDData->_objectOpenCLIndex;

            // Set the texture ID:
            textureID._textureMipLevel = 0;
            textureID._textureName = textureIndex;
            apCLImage textureDetails;
            bool rc = gaGetOpenCLImageObjectDetails(pGDData->_contextId._contextId, textureIndex, textureDetails);
            GT_ASSERT(rc);

            // Build the OpenCL texture heading:
            gdHTMLProperties htmlProps;
            htmlProps.getCLImageName(textureDetails, textureHeading);
            textureHeading.prepend(AF_STR_Space);
            gtString contextStr;
            pGDData->_contextId.toString(contextStr);
            textureHeading.prepend(contextStr);
        }

        // Add the texture layer / index to the heading for 3d textures and texture arrays
        if ((textureType == AP_3D_TEXTURE) && (textureLayer >= 0))
        {
            textureHeading.appendFormattedString(GD_STR_ImagesAndBuffersViewerTextureHeadingLayerPostfix, textureLayer);
        }
        else if (isTextureArray && (textureLayer >= 0))
        {
            textureHeading.appendFormattedString(GD_STR_ImagesAndBuffersViewerTextureHeadingIndexPostfix, textureLayer);
        }

        // Only if the item was loaded successfully, set its title within the image viewer:
        if (newMipLevel > 0)
        {
            textureHeading.appendFormattedString(GD_STR_ImagesAndBuffersViewerTextureHeadingMipPostfix, newMipLevel);
        }

        _pImageViewManager->setItemHeading(textureHeading);
    }
}

// ---------------------------------------------------------------------------
// Name:        gdImageAndBufferView::loadPBufferStaticBuffer
// Description: Loads a PBuffer to the texture viewer
// Arguments:   _pDisplayedItemTreeData - The PBuffer item to load
// Return Val:  bool - Success / Failure
// Author:      Eran Zinman
// Date:        12/1/2008
// ---------------------------------------------------------------------------
bool gdImageAndBufferView::loadPBufferStaticBuffer()
{
    bool retVal = false;

    // Sanity check:
    GT_IF_WITH_ASSERT(_pDisplayedItemTreeData != NULL)
    {
        gdDebugApplicationTreeData* pGDData = qobject_cast<gdDebugApplicationTreeData*>(_pDisplayedItemTreeData->extendedItemData());
        GT_IF_WITH_ASSERT(pGDData != NULL)
        {
            // Initialize the buffer load status as not loaded:
            _pDisplayedItemTreeData->setItemLoadStatus(AF_ITEM_LOAD_ERROR);

            // Initialize progress dialog
            afProgressBarWrapper::instance().ShowProgressBar(AF_TREE_ITEM_GL_PBUFFER_STATIC_BUFFER, L"Loading", 1);

            // Extract buffer raw data to disk:
            bool rc1 = exportSpyData(_pDisplayedItemTreeData);

            if (rc1)
            {
                // Get PBuffer ID and static buffer type
                int pbufferID = pGDData->_objectOpenGLName;
                apDisplayBuffer bufferType = pGDData->_bufferType;

                // Get the PBuffer static buffer details:
                apStaticBuffer staticBufferDetails;
                bool rc2 = gaGetPBufferStaticBufferObjectDetails(pbufferID, bufferType, staticBufferDetails);
                GT_IF_WITH_ASSERT(rc2)
                {
                    // Set progress bar at 30%
                    updateProgressBar(30);

                    // Get the static buffer filename
                    osFilePath bufferFile;
                    staticBufferDetails.getBufferFilePath(bufferFile);

                    // Localize the path as needed:
                    gaRemoteToLocalFile(bufferFile, false);

                    // Load buffer file to image and data views
                    retVal = loadBufferFile(bufferFile);

                    if (retVal)
                    {
                        _pDisplayedItemTreeData->setItemLoadStatus(AF_ITEM_LOAD_SUCCESS);
                    }

                    // Build the texture heading:
                    // Top label is PBuffer ID
                    gtString topLabel;
                    topLabel.appendFormattedString(GD_STR_ImagesAndBuffersViewerPBufferName, bufferType);
                    _pImageViewManager->setItemHeading(topLabel);

                    // Set progress bar at 90%
                    updateProgressBar(90);

                }
            }
            else
            {
                OS_OUTPUT_DEBUG_LOG(GD_STR_LogMsg_couldNotExportSpyData, OS_DEBUG_LOG_DEBUG);
            }

            // If buffer was not loaded successfully, display a message to the user
            if (!retVal)
            {
                setObjectNotLoadedStatus();
                retVal = true;
            }

            hideProgressBar();
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gdImageAndBufferView::loadBufferFile
// Description: A generic function to loads a buffer to the image and data
//              views. Buffer can a static buffer or a PBuffer, as long
//              as the file is in standard raw data format.
// Arguments:   _pDisplayedItemTreeData - Viewer item associated with the buffer
//              bufferType - Static buffer type
//              bufferFile - Buffer file to load
// Return Val:  bool - Success / Failure
// Author:      Eran Zinman
// Date:        25/1/2008
// ---------------------------------------------------------------------------
bool gdImageAndBufferView::loadBufferFile(const osFilePath& bufferFile)
{
    bool retVal = false;

    // Sanity check:
    GT_IF_WITH_ASSERT(_pDisplayedItemTreeData != NULL)
    {
        gdDebugApplicationTreeData* pGDData = qobject_cast<gdDebugApplicationTreeData*>(_pDisplayedItemTreeData->extendedItemData());
        GT_IF_WITH_ASSERT(pGDData != NULL)
        {
            // Initialize item load status:
            _pDisplayedItemTreeData->setItemLoadStatus(AF_ITEM_LOAD_ERROR);

            // We cannot load a buffer in glBegin-glEnd block
            if (!_isInGLBeginEndBlock)
            {
                // Load buffer raw data into textures and buffer viewer
                acRawFileHandler* pRawFileHandler = new acRawFileHandler;


                // Load raw data from file
                bool rc1 = pRawFileHandler->loadFromFile(bufferFile);
                GT_IF_WITH_ASSERT(rc1)
                {
                    // If data was loaded successfully:
                    GT_IF_WITH_ASSERT(pRawFileHandler->isOk())
                    {
                        // Set progress bar at 70%
                        updateProgressBar(70);

                        // Load the buffer to image and data views.
                        int canvasID = loadItemToImageAndDataViews(pRawFileHandler);
                        GT_IF_WITH_ASSERT(canvasID != -1)
                        {
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
// Name:        gdImageAndBufferView::loadBufferFile
// Description:
// Arguments: gdDebugApplicationTreeData* _pDisplayedItemTreeData
//            const osFilePath& bufferFile
//            oaTexelDataFormat bufferDisplayFormat
//            int offset
//            GLsizei stride
// Return Val: bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        6/4/2009
// ---------------------------------------------------------------------------
bool gdImageAndBufferView::loadBufferFile(const osFilePath& bufferFile, oaTexelDataFormat bufferDisplayFormat, int offset, GLsizei stride)
{
    bool retVal = false;

    // Sanity check:
    GT_IF_WITH_ASSERT((_pDisplayedItemTreeData != NULL) && (_pImageControlPanel != NULL))
    {
        gdDebugApplicationTreeData* pGDData = qobject_cast<gdDebugApplicationTreeData*>(_pDisplayedItemTreeData->extendedItemData());
        GT_IF_WITH_ASSERT(pGDData != NULL)
        {
            // We cannot load a buffer in glBegin-glEnd block:
            if (!_isInGLBeginEndBlock)
            {
                // Load buffer raw data into textures and buffer viewer
                acRawFileHandler* pRawFileHandler = new acRawFileHandler;


                // Load raw data from file
                bool rc1 = pRawFileHandler->loadFromFile(bufferFile);
                GT_IF_WITH_ASSERT(rc1)
                {
                    // If data was loaded successfully:
                    GT_IF_WITH_ASSERT(pRawFileHandler->isOk())
                    {
                        // Set the raw file handler display properties:
                        pRawFileHandler->setDisplayProperties(bufferDisplayFormat, offset, stride);

                        // Set progress bar at 70%
                        updateProgressBar(70);

                        // Loads the VBO into the data view:
                        // (We do not load the VBO to the image viewer)
                        int canvasID = 0;
                        retVal = loadRawDataToDataViewer(pRawFileHandler, canvasID, false, false);

                        // Set the new view state:
                        _currentViewsDisplayProperties._viewState = GD_DATA_ONLY;

                        // Select the data view:
                        displayRelevantNotebookPages();
                    }
                }
            }
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gdImageAndBufferView::loadFBOAttachmentBuffer
// Description: Loads a render buffer to the texture viewer
// Arguments:   _pDisplayedItemTreeData - The render buffer item to load
// Return Val:  bool - Success / Failure
// Author:      Sigal Algranaty
// Date:        28/5/2008
// ---------------------------------------------------------------------------
bool gdImageAndBufferView::loadFBOAttachmentBuffer()
{
    bool retVal = false;

    // Sanity check:
    GT_IF_WITH_ASSERT(_pDisplayedItemTreeData != NULL)
    {
        gdDebugApplicationTreeData* pGDData = qobject_cast<gdDebugApplicationTreeData*>(_pDisplayedItemTreeData->extendedItemData());
        GT_IF_WITH_ASSERT(pGDData != NULL)
        {
            // Initialize the item load status as error:
            _pDisplayedItemTreeData->setItemLoadStatus(AF_ITEM_LOAD_ERROR);
            bool isTextureAttachment = false;
            bool rc = apGLFBO::isTextureAttachmentTarget(pGDData->_bufferAttachmentTarget, isTextureAttachment);
            GT_IF_WITH_ASSERT(rc)
            {
                if (isTextureAttachment)
                {
                    retVal = LoadTexture(-1);
                }
                else
                {
                    retVal = loadRenderBuffer();
                }

            }

            // If buffer was not loaded successfully, display a message to the user
            if (retVal)
            {
                _pDisplayedItemTreeData->setItemLoadStatus(AF_ITEM_LOAD_SUCCESS);
            }
            else
            {
                setObjectNotLoadedStatus();
                retVal = true;
            }

            // End progress bar operation
            hideProgressBar();
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gdImageAndBufferView::loadRenderBuffer
// Description: Loads a render buffer to the texture viewer
// Arguments:   _pDisplayedItemTreeData - The render buffer item to load
// Return Val:  bool - Success / Failure
// Author:      Sigal Algranaty
// Date:        28/5/2008
// ---------------------------------------------------------------------------
bool gdImageAndBufferView::loadRenderBuffer()
{
    bool retVal = false;
    // Render buffer details should be use also when spy export is not succeeded, therefore it is declared here:
    bool isRenderBufferDetailsTaken = false;
    bool isInteropObject = false;

    // Get the render buffer details:
    apGLRenderBuffer renderBufferDetails(0);

    // Sanity check:
    GT_IF_WITH_ASSERT(_pDisplayedItemTreeData != NULL)
    {
        gdDebugApplicationTreeData* pGDData = qobject_cast<gdDebugApplicationTreeData*>(_pDisplayedItemTreeData->extendedItemData());
        GT_IF_WITH_ASSERT(pGDData != NULL)
        {
            // Initialize progress dialog
            afProgressBarWrapper::instance().ShowProgressBar(AF_TREE_ITEM_GL_RENDER_BUFFER, L"Loading", 1);

            // Set progress bar at 20%
            updateProgressBar(20);

            gtString bufferUnLoadMessage;
            // If the buffer is shared with CL do not display it:
            // NOTICE: This feature is not supported until the driver is fixed to support the retrieve of shared objects content:
            isInteropObject = (pGDData->_objectOpenCLName > 0);

            if (isInteropObject)
            {
                // Do not display CL shared items:
                _pDisplayedItemTreeData->setItemLoadStatus(AF_ITEM_NOT_LOADED, AF_ITEM_LOAD_CL_GL_INTEROP);
            }
            else
            {
                // Extract buffer raw data to disk:
                bool rc1 = exportSpyData(_pDisplayedItemTreeData);

                if (rc1)
                {
                    isRenderBufferDetailsTaken = gaGetRenderBufferObjectDetails(pGDData->_contextId._contextId, pGDData->_objectOpenGLName, renderBufferDetails);
                    GT_IF_WITH_ASSERT(isRenderBufferDetailsTaken)
                    {
                        // Set progress bar at 30%
                        updateProgressBar(30);

                        // Get the render buffer filename:
                        osFilePath bufferFile;
                        renderBufferDetails.getBufferFilePath(bufferFile);

                        // Localize the path as needed:
                        gaRemoteToLocalFile(bufferFile, false);

                        // Load buffer file to image and data views
                        retVal = loadBufferFile(bufferFile);

                        if (retVal)
                        {
                            _pDisplayedItemTreeData->setItemLoadStatus(AF_ITEM_LOAD_SUCCESS);
                        }

                        // Build the texture heading:
                        gtString renderBufferHeading;
                        pGDData->_contextId.toString(renderBufferHeading);
                        renderBufferHeading.append(AF_STR_Space);
                        renderBufferHeading.appendFormattedString(GD_STR_ImagesAndBuffersViewerRenderBufferNameFormat, pGDData->_objectOpenGLName);

                        _pImageViewManager->setItemHeading(renderBufferHeading);

                        // Set progress bar at 90%
                        updateProgressBar(90);
                    }
                }
                else
                {
                    OS_OUTPUT_DEBUG_LOG(GD_STR_LogMsg_couldNotExportSpyData, OS_DEBUG_LOG_DEBUG);
                }

                if (!isRenderBufferDetailsTaken)
                {
                    // If the export process was failed, try to get the render buffer details anyway, and show in properties window:
                    isRenderBufferDetailsTaken = gaGetRenderBufferObjectDetails(pGDData->_contextId._contextId, pGDData->_objectOpenGLName, renderBufferDetails);
                }

                // If buffer was not loaded successfully, display a message to the user
                if (!retVal)
                {
                    setObjectNotLoadedStatus();
                    retVal = true;
                }
            }
        }

        // End progress bar operation
        hideProgressBar();
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gdImageAndBufferView::loadVBO
// Description: Loads a VBO to the texture viewer
// Arguments:   _pDisplayedItemTreeData - The render buffer item to load
// Return Val:  bool - Success / Failure
// Author:      Sigal Algranaty
// Date:        6/4/2009
// ---------------------------------------------------------------------------
bool gdImageAndBufferView::loadVBO()
{
    bool retVal = false;
    bool isInteropObject = false;

    // Sanity check:
    GT_IF_WITH_ASSERT(_pDisplayedItemTreeData != NULL)
    {
        gdDebugApplicationTreeData* pGDData = qobject_cast<gdDebugApplicationTreeData*>(_pDisplayedItemTreeData->extendedItemData());
        GT_IF_WITH_ASSERT(pGDData != NULL)
        {
            // Initialize the VBO load status as not loaded:
            _pDisplayedItemTreeData->setItemLoadStatus(AF_ITEM_LOAD_ERROR);

            apGLVBO vboDetails;

            // Initialize progress dialog:
            afProgressBarWrapper::instance().ShowProgressBar(AF_TREE_ITEM_GL_VBO, L"Loading", 1);

            // Set progress bar at 20%:
            updateProgressBar(20);

            // Get the VBO details:
            bool rc1 = gaGetVBODetails(pGDData->_contextId._contextId, pGDData->_objectOpenGLName, vboDetails);
            GT_ASSERT(rc1);

            // If the buffer is shared with CL do not display it:
            // NOTICE: This feature is not supported until the driver is fixed to support the retrieve of shared objects content:
            isInteropObject = (pGDData->_objectOpenCLName > 0);

            if (isInteropObject)
            {
                // Do not display CL shared items:
                _pDisplayedItemTreeData->setItemLoadStatus(AF_ITEM_NOT_LOADED, AF_ITEM_LOAD_CL_GL_INTEROP);
            }
            else
            {
                // Extract buffer raw data to disk:
                bool rc2 = exportSpyData(_pDisplayedItemTreeData);

                if (rc2)
                {
                    // Get the VBO details:
                    // NOTICE: we read the details once again on purpose, since we want the VBO path
                    // to be included in the VBO details:
                    apGLVBO vboDetails2;
                    bool rc3 = gaGetVBODetails(pGDData->_contextId._contextId, pGDData->_objectOpenGLName, vboDetails2);
                    GT_IF_WITH_ASSERT(rc3)
                    {
                        // Set progress bar at 30%:
                        updateProgressBar(30);

                        // Get the VBO filename:
                        osFilePath bufferFile;
                        vboDetails2.getBufferFilePath(bufferFile);

                        // Localize the path as needed:
                        gaRemoteToLocalFile(bufferFile, false);

                        // Get the VBO display properties:
                        oaTexelDataFormat bufferDisplayFormat = OA_TEXEL_FORMAT_UNKNOWN;
                        int offset = 0;
                        GLsizei stride = 0;
                        vboDetails2.getBufferDisplayProperties(bufferDisplayFormat, offset, stride);

                        // Load buffer file to data views:
                        retVal = loadBufferFile(bufferFile, bufferDisplayFormat, offset, stride);

                        if (retVal)
                        {
                            _pDisplayedItemTreeData->setItemLoadStatus(AF_ITEM_LOAD_SUCCESS);
                        }

                        // Set progress bar at 90%
                        updateProgressBar(90);
                    }
                }
                else
                {
                    OS_OUTPUT_DEBUG_LOG(GD_STR_LogMsg_couldNotExportSpyData, OS_DEBUG_LOG_DEBUG);
                }
            }

            // If buffer was not loaded successfully, display a message to the user
            if (!retVal)
            {
                if (!isInteropObject)
                {
                    setObjectNotLoadedStatus();
                }

                retVal = true;
            }

            // End progress bar operation
            hideProgressBar();
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gdImageAndBufferView::loadStaticBuffer
// Description: Loads a static buffer to the texture viewer
// Arguments:   _pDisplayedItemTreeData - The static buffer item to load
// Return Val:  bool - Success / Failure
// Author:      Eran Zinman
// Date:        25/11/2007
// ---------------------------------------------------------------------------
bool gdImageAndBufferView::loadStaticBuffer()
{
    bool retVal = false;


    // Sanity check:
    GT_IF_WITH_ASSERT(_pDisplayedItemTreeData != NULL)
    {
        gdDebugApplicationTreeData* pGDData = qobject_cast<gdDebugApplicationTreeData*>(_pDisplayedItemTreeData->extendedItemData());
        GT_IF_WITH_ASSERT(pGDData != NULL)
        {
            _pDisplayedItemTreeData->setItemLoadStatus(AF_ITEM_LOAD_ERROR);
            // Initialize progress dialog
            afProgressBarWrapper::instance().ShowProgressBar(AF_TREE_ITEM_GL_STATIC_BUFFER, L"Loading", 1);

            // Check if we have an active FBO bound in this context:
            GLuint activeFBOName = 0;
            gaGetActiveFBO(pGDData->_contextId._contextId, activeFBOName);
            bool isFBOBound = (activeFBOName != 0);

            if (!isFBOBound)
            {
                // Extract buffer raw data to disk:
                bool rc1 = exportSpyData(_pDisplayedItemTreeData);

                if (rc1)
                {
                    // Set progress bar at 20%
                    updateProgressBar(20);

                    // Get buffer type
                    apDisplayBuffer bufferType = pGDData->_bufferType;

                    // Get the static buffer details:
                    apStaticBuffer staticBufferDetails;
                    bool rc2 = gaGetStaticBufferObjectDetails(pGDData->_contextId._contextId, bufferType, staticBufferDetails);
                    GT_IF_WITH_ASSERT(rc2)
                    {
                        // Set progress bar at 30%
                        updateProgressBar(30);

                        // Get the static buffer filename
                        osFilePath bufferFile;
                        staticBufferDetails.getBufferFilePath(bufferFile);

                        // Localize the path as needed:
                        gaRemoteToLocalFile(bufferFile, false);

                        // Load buffer file to image and data views
                        retVal = loadBufferFile(bufferFile);

                        if (retVal)
                        {
                            _pDisplayedItemTreeData->setItemLoadStatus(AF_ITEM_LOAD_SUCCESS);

                            // Build the buffer heading:
                            // Generate PBuffer static buffer bottom label
                            gtString bufferHeading;
                            pGDData->_contextId.toString(bufferHeading);
                            bufferHeading.append(AF_STR_Space);
                            rc2 = apGetBufferName(bufferType, bufferHeading);
                            GT_ASSERT(rc2);
                            _pImageViewManager->setItemHeading(bufferHeading);
                        }

                        // Set progress bar at 90%
                        updateProgressBar(90);
                    }
                }
                else
                {
                    OS_OUTPUT_DEBUG_LOG(GD_STR_LogMsg_couldNotExportSpyData, OS_DEBUG_LOG_DEBUG);
                }
            }

            // If buffer was not loaded successfully, display a message to the user
            if (!retVal)
            {
                setObjectNotLoadedStatus(isFBOBound);
                retVal = true;
            }

            // End progress bar operation
            hideProgressBar();
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gdImageAndBufferView::loadTexBuffer
// Description: Loads a texture buffer to the texture viewer
// Arguments:   _pDisplayedItemTreeData - The render buffer item to load
// Return Val:  bool - Success / Failure
// Author:      Sigal Algranaty
// Date:        4/8/2009
// ---------------------------------------------------------------------------
bool gdImageAndBufferView::loadTexBuffer()
{
    bool retVal = false;

    // Sanity check:
    GT_IF_WITH_ASSERT(_pDisplayedItemTreeData != NULL)
    {
        gdDebugApplicationTreeData* pGDData = qobject_cast<gdDebugApplicationTreeData*>(_pDisplayedItemTreeData->extendedItemData());
        GT_IF_WITH_ASSERT(pGDData != NULL)
        {
            // Initialize progress dialog:
            afProgressBarWrapper::instance().ShowProgressBar(AF_TREE_ITEM_GL_TEXTURE, L"Loading", 1);


            apGLTextureMiplevelData textureThumbnailDetails;

            // Make sure texture data is cached to disk:
            bool rc1 = exportSpyData(_pDisplayedItemTreeData);

            if (rc1)
            {
                // Update progress bar to 20%:
                updateProgressBar(20);

                // Get the texture ID:
                apGLTextureMipLevelID textureID = pGDData->_textureMiplevelID;

                // Get texture thumbnail data details:
                bool rc3 = gaGetTextureObjectThumbnailData(pGDData->_contextId._contextId, textureID._textureName, textureThumbnailDetails);
                GT_IF_WITH_ASSERT(rc3)
                {
                    // Adjust the image manager layout:
                    rc3 = setImageViewMode(textureThumbnailDetails.textureType());
                    GT_IF_WITH_ASSERT(rc3)
                    {
                        // Set progress bar to 30%:
                        updateProgressBar(30);

                        // Build the texture buffer heading:
                        gtString textureHeading;
                        pGDData->_contextId.toString(textureHeading);
                        textureHeading.append(AF_STR_Space);
                        textureHeading.appendFormattedString(GD_STR_ImagesAndBuffersViewerTexBufferHeading, textureID._textureName);

                        // Set the item heading within the image manager:
                        _pImageViewManager->setItemHeading(textureHeading);

                        // We cannot load a texture in glBegin-glEnd block
                        if (!_isInGLBeginEndBlock)
                        {
                            retVal = false;

                            // Get texture raw data element file name:
                            osFilePath textureFile;
                            bool rc0 = gaGetTextureMiplevelDataFilePath(pGDData->_contextId._contextId, pGDData->_textureMiplevelID, 0, textureFile);
                            GT_ASSERT(rc0);

                            // Localize the path as needed:
                            gaRemoteToLocalFile(textureFile, false);

                            // Calculate the texture element matrix position and top label:
                            QPoint matrixPos(0, 0);
                            gtString topLabel;
                            rc1 = calcTextureElementMatrixPosAndLabel(textureThumbnailDetails.textureType(), 0, matrixPos, topLabel);
                            GT_ASSERT(rc1);

                            // Load the raw data into the textures and buffer viewer.
                            // Important note: We are not responsible for releasing the raw file handler here,
                            // the textures and buffers viewer will release the object later on.
                            acRawFileHandler* pRawFileHandler = new acRawFileHandler;


                            // Load raw data from file
                            bool rc2 = pRawFileHandler->loadFromFile(textureFile);
                            GT_IF_WITH_ASSERT(rc2)
                            {
                                // Set the raw file handler data format:
                                rc3 = pRawFileHandler->setDataFormatAndAdaptSize(textureThumbnailDetails.bufferInternalFormat());
                                GT_IF_WITH_ASSERT(rc3)
                                {
                                    // If raw data was loaded successfully
                                    GT_IF_WITH_ASSERT(pRawFileHandler->isOk())
                                    {

                                        // Load the texture to image data views
                                        retVal = loadItemToImageAndDataViews(pRawFileHandler, textureHeading, matrixPos);
                                    }
                                }
                            }
                        }
                    }

                    // Set progress bar to 95%:
                    updateProgressBar(95);

                }
            }
            else
            {
                OS_OUTPUT_DEBUG_LOG(GD_STR_LogMsg_couldNotExportSpyData, OS_DEBUG_LOG_DEBUG);
            }
        }


        // If texture was not loaded successfully, display a message to the user
        if (!retVal)
        {
            setObjectNotLoadedStatus();
            retVal = true;
        }

        // End progress bar operation
        hideProgressBar();
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gdImageAndBufferView::setObjectNotLoadedStatus
// Description: Set the buffer not loaded failure status
// Arguments:   isFBOBound - is the active FBO bound
// Author:      Sigal Algranaty
// Date:        6/4/2011
// ---------------------------------------------------------------------------
void gdImageAndBufferView::setObjectNotLoadedStatus(bool isFBOBound)
{
    // Sanity check:
    GT_IF_WITH_ASSERT(_pDisplayedItemTreeData != NULL)
    {
        gdDebugApplicationTreeData* pGDData = qobject_cast<gdDebugApplicationTreeData*>(_pDisplayedItemTreeData->extendedItemData());
        GT_IF_WITH_ASSERT(pGDData != NULL)
        {
            // Set default statuses:
            if (_pDisplayedItemTreeData->m_itemType == AF_TREE_ITEM_GL_TEXTURE)
            {
                _pDisplayedItemTreeData->setItemLoadStatus(AF_ITEM_NOT_LOADED, AF_ITEM_LOAD_TEXTURE_TYPE_UNKNOWN);
            }
            else if (_pDisplayedItemTreeData->m_itemType == AF_TREE_ITEM_CL_IMAGE)
            {
                _pDisplayedItemTreeData->setItemLoadStatus(AF_ITEM_NOT_LOADED, AF_ITEM_LOAD_IMAGE_TYPE_UNKNOWN);
            }

            // Are we in glBegin-glEnd block?
            if (_isInGLBeginEndBlock)
            {
                _pDisplayedItemTreeData->setItemLoadStatus(AF_ITEM_NOT_LOADED, AF_ITEM_LOAD_GLBEGIN_END_BLOCK);
            }
            else if (isFBOBound)
            {
                _pDisplayedItemTreeData->setItemLoadStatus(AF_ITEM_NOT_LOADED, AF_ITEM_LOAD_STATIC_BUFFER_BOUND_TO_FBO);
            }
        }
    }
}


// ---------------------------------------------------------------------------
// Name:        gdImageAndBufferView::updateObjectDisplay
// Description: Is called on process run suspension. The function looks for the
//              object data in the updated monitored object tree, and loads the
//              object, or display a non existing object message
// Return Val:  void
// Author:      Sigal Algranaty
// Date:        29/12/2010
// ---------------------------------------------------------------------------
void gdImageAndBufferView::updateObjectDisplay(bool& doesObjectExist)
{
    doesObjectExist = false;

    // Check if the process exists, and if it is suspended:
    bool doesProcessExist = gaDebuggedProcessExists();
    bool isProcessSuspended = gaIsDebuggedProcessSuspended();

    _pDisplayedItemTreeData = _pStaticEmptyItemData;

    // Sanity check:
    if (gdDebugApplicationTreeHandler::instance() != NULL)
    {
        // If we know in advance that the item display is about to fail, use the dummy item data:
        if (!_displayedItemId.isItemDisplayed())
        {
            _pDisplayedItemTreeData = _pStaticEmptyItemData;
            _pDisplayedItemTreeData->setItemLoadStatus(_displayedItemId._itemLoadStatus._itemLoadStatusType, _displayedItemId._itemLoadStatus._loadStatusDescription);
        }

        if (isProcessSuspended && doesProcessExist)
        {
            // Look for the displayed object item data (only if the debugged process already exist):
            _pDisplayedItemTreeData = gdDebugApplicationTreeHandler::instance()->FindMatchingTreeItem(_displayedItemId);
        }
    }

    GT_IF_WITH_ASSERT(_pDisplayedItemTreeData != NULL)
    {

        // Clear the view:
        clearView();

        // Set the manager mode:
        _pImageViewManager->setManagerMode(AC_MANAGER_MODE_STANDARD_ITEM);


        if (isProcessSuspended)
        {
            // The object was found:
            doesObjectExist = true;

            // Display the existing item:
            displayItem(_pDisplayedItemTreeData);

            // Apply the last item properties:
            applyLastViewedItemProperties(_lastViewedItemProperties, true);

            // Adjust the item after load:
            adjustViewerAfterItemLoading();

            // Adjust the manager layout after adding the texture
            forceImageManagerRepaint();

            m_pTabWidget->setVisible(true);
            _pImageControlPanel->setVisible(true);

            // This is the first time that the window size is set, so apply best fit:
            applyBestFit(_currentZoomLevel);

            // Set the texture manager new zoom level
            setTextureManagerZoomLevel(_currentZoomLevel);

            // If the item is not displayed for some reason, display a text message describing the situation:
            displayItemMessageIfNeeded();
        }
        else
        {
            // Copy my id to the displayed data, so that the "process is running" message would be displayed with the correct type:
            _displayedItemId.copyID(*_pDisplayedItemTreeData);

            // Get the appropriate failure description:
            afItemLoadFailureDescription failureDescription = gaDebuggedProcessExists() ? AF_ITEM_LOAD_PROCESS_IS_RUNNING : AF_ITEM_LOAD_PROCESS_IS_TERMINATED;

            // Set the load status (item cannot be loaded while process is running):
            _pDisplayedItemTreeData->setItemLoadStatus(AF_ITEM_NOT_LOADED, failureDescription);

            // Display process is not running message:
            displayItemMessageIfNeeded();
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        gdImageAndBufferView::initializeObjectNotLoadedStatus
// Description: If an object cannot be displayed and we know if in advance, use
//              this function to set the situation
// Arguments:   afItemLoadStatusType itemStatus
//              afItemLoadFailureDescription itemStatusDescription
// Author:      Sigal Algranaty
// Date:        7/4/2011
// ---------------------------------------------------------------------------
void gdImageAndBufferView::initializeObjectNotLoadedStatus(afItemLoadStatusType itemStatus, afItemLoadFailureDescription itemStatusDescription)
{
    // If the displayed object item data is set, set the item status on it, otherwise use the object id for that:
    if (_pDisplayedItemTreeData != NULL)
    {
        _pDisplayedItemTreeData->setItemLoadStatus(itemStatus, itemStatusDescription);
    }
    else
    {
        _displayedItemId.setItemLoadStatus(itemStatus, itemStatusDescription);
    }
}



