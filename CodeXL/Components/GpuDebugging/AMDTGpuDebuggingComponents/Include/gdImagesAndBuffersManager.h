//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gdImagesAndBuffersManager.h
///
//==================================================================================

//------------------------------ gdImagesAndBuffersManager.h ------------------------------

#ifndef __GDIMAGESANDBUFFERSMANAGER
#define __GDIMAGESANDBUFFERSMANAGER

// AMDTApplicationFramework:
#include <AMDTApplicationFramework/Include/views/afApplicationTreeItemData.h>

// AMDTGpuDebuggingComponents:
#include <AMDTGpuDebuggingComponents/Include/views/gdThumbnailView.h>
#include <AMDTGpuDebuggingComponents/Include/views/gdImageAndBufferViewsController.h>
#include <AMDTGpuDebuggingComponents/Include/gdDebugApplicationTreeHandler.h>
#include <AMDTGpuDebuggingComponents/Include/gdGDebuggerAppDLLBuild.h>


// "Images and Buffers" Toolbar
// NOTICE: These actions should remain adjacent. If another action is added to the toolbar,
// it must be handled in application implement, and ID_IMAGES_AND_BUFFERS_VIEWER_TOOLBAR_INVERT should be updated:
enum gdImageActionId
{
    ID_IMAGES_AND_BUFFERS_VIEWER_TOOLBAR_ACTION_UNKNOWN = -1,
    ID_IMAGES_AND_BUFFERS_VIEWER_TOOLBAR_NORMAL = 0,
    ID_IMAGES_AND_BUFFERS_VIEWER_TOOLBAR_FIRST_ACTION = ID_IMAGES_AND_BUFFERS_VIEWER_TOOLBAR_NORMAL,
    ID_IMAGES_AND_BUFFERS_VIEWER_TOOLBAR_PAN,
    ID_IMAGES_AND_BUFFERS_VIEWER_TOOLBAR_ZOOMIN,
    ID_IMAGES_AND_BUFFERS_VIEWER_TOOLBAR_ZOOMOUT,
    ID_IMAGES_AND_BUFFERS_VIEWER_ORIGINAL_SIZE,
    ID_IMAGES_AND_BUFFERS_VIEWER_TOOLBAR_BEST_FIT,
    ID_IMAGES_AND_BUFFERS_VIEWER_TOOLBAR_ROTATE_LEFT,
    ID_IMAGES_AND_BUFFERS_VIEWER_TOOLBAR_ROTATE_RIGHT,
    ID_IMAGES_AND_BUFFERS_VIEWER_TOOLBAR_RED_CHANNEL,
    ID_IMAGES_AND_BUFFERS_VIEWER_TOOLBAR_GREEN_CHANNEL,
    ID_IMAGES_AND_BUFFERS_VIEWER_TOOLBAR_BLUE_CHANNEL,
    ID_IMAGES_AND_BUFFERS_VIEWER_TOOLBAR_ALPHA_CHANNEL,
    ID_IMAGES_AND_BUFFERS_VIEWER_TOOLBAR_INVERT,
    ID_IMAGES_AND_BUFFERS_VIEWER_TOOLBAR_GRAYSCALE,
    ID_IMAGES_AND_BUFFERS_VIEWER_TOOLBAR_LAST_ACTION = ID_IMAGES_AND_BUFFERS_VIEWER_TOOLBAR_GRAYSCALE,
    ID_IMAGES_AND_BUFFERS_VIEWER_COPY,
    ID_IMAGES_AND_BUFFERS_VIEWER_SELECT_ALL,
    ID_IMAGES_AND_BUFFERS_VIEWER_SELECT_IMAGE_VIEW,
    ID_IMAGES_AND_BUFFERS_VIEWER_SELECT_DATA_VIEW,
    ID_IMAGES_AND_BUFFERS_VIEWER_ZOOM_COMBOBOX
};


// ----------------------------------------------------------------------------------
// Class Name:           gdImagesAndBuffersManager
// General Description:  A single instance object which is managing the display of
//                       images and buffers. The object is also handling toolbars
//                       and actions supported for images and buffers
// Author:               Sigal Algranaty
// Creation Date:        15/12/2010
// ----------------------------------------------------------------------------------
class GD_API gdImagesAndBuffersManager
{
    friend class vspSingletonsDelete;
public:
    // Get my single instance:
    static gdImagesAndBuffersManager& instance();

    // Destructor:
    virtual ~gdImagesAndBuffersManager();

    // Clears the image and data viewers:
    void resetImageAndDataViews(bool resetFilters = true);

    // Is used for controlling the images and buffer views:
    gdImageAndBufferViewsController& controller() {return _imagesAndBuffersController;};

    // Create a view for a new image / buffer object:
    gdImageAndBufferView* createNewImageAndBufferView(QWidget* pParentWindow, const QSize& viewSize);
    gdThumbnailView* createNewThumbnailView(QWidget* pParentWindow, const QSize& viewSize);
    void setFocusedViews(gdImageAndBufferView* pImageBufferView, gdThumbnailView* pThumbnailView, bool isDebuggedProcessSuspended) {_imagesAndBuffersController.setFocusedViews(pImageBufferView, pThumbnailView, isDebuggedProcessSuspended);};
    gdImageAndBufferView* focusedImageBufferView() { return _imagesAndBuffersController.focusedImageBufferView();}

    // Display an item:
    bool displayItem(gdImageAndBufferView* pImageBufferView, const gtString& itemString);
    bool displayItem(gdImageAndBufferView* pImageBufferView, afApplicationTreeItemData* pItemData);
    bool displayThumbnailItem(gdThumbnailView* pThumbnailView, const gtString& itemString);
    bool displayThumbnailItem(gdThumbnailView* pThumbnailView, afApplicationTreeItemData* pItemData);

    // Event handling all toolbar events:
    void onToolbarEvent(int commandId);
    void toggleToolsForCommandEvent(int commandId);
    void updateOpenedViewsOnEvent(bool isSuspended);

    // Enabling / Disabling ToolBar buttons:
    bool isToolbarCommandEnabled(int commandId);
    bool isToolbarCheckedCommandEnabled(int commandId, bool& isChecked);

    // OnSize event:
    void onImageBufferSize(gdImageAndBufferView* pImageBufferView, const QSize& windowSize);

    // OnClosePane event:
    void onPaneClose(gdImageAndBufferView* pImageBufferView) {_imagesAndBuffersController.closeImageBufferView(pImageBufferView);}
    void onPaneClose(gdThumbnailView* pThumbnailView) {_imagesAndBuffersController.closeThumbnailView(pThumbnailView);}

    // Update hex mode on all image and buffer view:
    void updateOpenedViewsHexMode(bool hexMode) {_imagesAndBuffersController.updateOpenedViewsHexDisplayMode(hexMode);}

    // Update active view focus:
    void updateActiveViewFocus(QWidget* activeView);

private:

    // Only my instance() method should create my single instance:
    gdImagesAndBuffersManager();

private:

    // Single instance static member:
    static gdImagesAndBuffersManager* _pMySingleInstance;

private:
    // Apply the last viewed item properties:
    virtual void applyLastViewedItemProperties(bool applyZoom = true);

    // Loads an item (texture / buffer) into the image and data viewers
    bool loadItemIntoViewer(afApplicationTreeItemData* pViewerItem);

    // Adjust the textures and buffers viewer after loading an item (texture / buffer)
    void adjustViewerAfterItemLoading(afApplicationTreeItemData* pLoadedItem);

    // Updates the RGBA channels buttons enabled status and linked status
    void updateRGBChannelsStatus(afApplicationTreeItemData* pLoadedItemData);

    // Reset active tools and filters:
    void resetActiveTools();

protected:

    // Thumbnail view:
    gdThumbnailView* _pThumbnailView;

    // Buffer / Image view:
    gdImageAndBufferView* _pImageBufferView;

    // Is used for controlling the images and buffer views:
    gdImageAndBufferViewsController _imagesAndBuffersController;

};

#endif // __gdImagesAndBuffersManager
