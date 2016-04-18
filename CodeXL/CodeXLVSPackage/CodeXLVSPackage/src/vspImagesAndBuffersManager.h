//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file vspImagesAndBuffersManager.h
///
//==================================================================================

//------------------------------ vspImagesAndBuffersManager.h ------------------------------

#ifndef __VSPIMAGESANDBUFFERSMANAGER
#define __VSPIMAGESANDBUFFERSMANAGER

// Forward declarations:
class afPropertiesView;

// Infra:
#include <AMDTGpuDebuggingComponents/Include/gdImagesAndBuffersManager.h>
#include <AMDTGpuDebuggingComponents/Include/gdDebugApplicationTreeData.h>
#include <AMDTGpuDebuggingComponents/Include/gdDebugApplicationTreeHandler.h>
#include <AMDTGpuDebuggingComponents/Include/views/gdThumbnailView.h>
#include <AMDTGpuDebuggingComponents/Include/views/gdImageAndBufferViewsController.h>


// ----------------------------------------------------------------------------------
// Class Name:           vspImagesAndBuffersManager
// General Description:  A single instance object which is managing the display of
//                       images and buffers. The object is also handling toolbars
//                       and actions supported for images and buffers
// Author:               Sigal Algranaty
// Creation Date:        15/12/2010
// ----------------------------------------------------------------------------------
class vspImagesAndBuffersManager
{
    friend class vspSingletonsDelete;
public:
    // Get my single instance:
    static vspImagesAndBuffersManager& instance();

    // Destructor:
    ~vspImagesAndBuffersManager();

    // Clears the image and data viewers:
    void resetImageAndDataViews(bool resetFilters = true);

    // Create a view for a new image / buffer object:
    gdImageAndBufferView* createNewImageAndBufferView(QWidget* pParentWindow, QSize viewSize);
    gdThumbnailView* createNewThumbnailView(QWidget* pParentWindow, QSize viewSize);
    void setFocusedViews(gdImageAndBufferView* pImageBufferView, gdThumbnailView* pThumbnailView, bool isDebuggedProcessSuspended) {_imagesAndBuffersController.setFocusedViews(pImageBufferView, pThumbnailView, isDebuggedProcessSuspended);};
    gdImageAndBufferView* focusedImageBufferView() { return _imagesAndBuffersController.focusedImageBufferView();}

    // Display an item:
    bool displayItem(gdImageAndBufferView* pImageBufferView, const gtString& itemString);
    bool displayThumbnailItem(gdThumbnailView* pThumbnailView, const gtString& itemString);


    // Event handling all toolbar events:
    void onToolbarEvent(gdImageActionId commandId);
    void toggleToolsForCommandEvent(int commandId);
    void updateOpenedViewsOnEvent(bool isSuspended);

    // Enabling / Disabling ToolBar buttons:
    bool isToolbarCommandEnabled(int commandId);
    bool isToolbarCheckedCommandEnabled(int commandId, bool& isChecked);

    // OnSize event:
    void onImageBufferSize(gdImageAndBufferView* pImageBufferView, QSize windowSize);

    // OnClosePane event:
    void onPaneClose(gdImageAndBufferView* pImageBufferView) {_imagesAndBuffersController.closeImageBufferView(pImageBufferView);}
    void onPaneClose(gdThumbnailView* pThumbnailView) {_imagesAndBuffersController.closeThumbnailView(pThumbnailView);}

    // Update hex mode on all image and buffer view:
    void updateOpenedViewsHexMode(bool hexMode) {_imagesAndBuffersController.updateOpenedViewsHexDisplayMode(hexMode);}

private:

    // Only my instance() method should create my single instance:
    vspImagesAndBuffersManager();

private:

    // Single instance static member:
    static vspImagesAndBuffersManager* _pMySingleInstance;

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

    // Monitored items explorer:
    afApplicationTree* _pMonitoredObjectsExplorer;

    // Properties view:
    afPropertiesView* _pPropertiesView;

    // Thumbnail view:
    gdThumbnailView* _pThumbnailView;

    // Buffer / Image view:
    gdImageAndBufferView* _pImageBufferView;

    // Is used for controlling the images and buffer views:
    gdImageAndBufferViewsController _imagesAndBuffersController;

};

#endif // __VSPIMAGESANDBUFFERSMANAGER
