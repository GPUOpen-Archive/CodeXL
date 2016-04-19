//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gdImageAndBufferViewsController.h
///
//==================================================================================

//------------------------------ gdImageAndBufferViewsController.h -----------------------------

#ifndef __GDIMAGEANDBUFFERVIEWSCONTROLLER
#define __GDIMAGEANDBUFFERVIEWSCONTROLLER

// Infra:
#include <AMDTBaseTools/Include/gtString.h>
#include <AMDTBaseTools/Include/gtVector.h>
#include <AMDTBaseTools/Include/gtPtrVector.h>
#include <AMDTOSWrappers/Include/osFilePath.h>
#include <AMDTAPIClasses/Include/Events/apIEventsObserver.h>
#include <AMDTAPIClasses/Include/apFileType.h>
#include <AMDTApplicationComponents/Include/acImageManager.h>

// Local:
#include <AMDTGpuDebuggingComponents/Include/views/gdImagesAndBuffersExporter.h>
#include <AMDTGpuDebuggingComponents/Include/gdDebugApplicationTreeData.h>
#include <AMDTGpuDebuggingComponents/Include/gdGDebuggerAppDLLBuild.h>
#include <AMDTGpuDebuggingComponents/Include/gdStringConstants.h>

// Pre declarations:
class gdDebugApplicationTreeData;
class apDebuggedProcessRunSuspendedEvent;
class gdDebugApplicationTreeHandler;
class gdPropertiesEventObserver;
class gdThumbnailView;
class gdImageAndBufferView;

// ----------------------------------------------------------------------------------
// Class Name:           gdImageAndBufferViewsController: public apIEventsObserver
// General Description:  This class is used for managing all the GUI elements used
//                       for displaying and controlling images and buffers.
//                       The class is observing events that are relevant for the display
//                       Of images and buffers, and negotiate between the GUI elements
//                       for the display of the current displayed elements.
// Author:               Sigal Algranaty
// Creation Date:        15/11/2010
// ----------------------------------------------------------------------------------
class GD_API gdImageAndBufferViewsController
{

public:
    // Constructor:
    gdImageAndBufferViewsController();

    // Destructor:
    virtual ~gdImageAndBufferViewsController();

    // Clears the image and data viewers
    void resetImageAndDataViews(bool resetFilters = true);

    // Accessors:
    apContextID activeContext() const {return _activeContext;};
    void setActiveContext(const apContextID& activeContext)  {_activeContext = activeContext;};
    bool isInGLBeginEndBlock() const {return _isInGLBeginEndBlock;};
    void setIsInGLBeginEndBlock(bool isInGLBeginEndBlock) {_isInGLBeginEndBlock = isInGLBeginEndBlock;};
    acDisplayedItemProperties lastViewedItemProperties() const {return _lastViewedItemProperties;}

    void resetActiveFilters() { _imageActionsEnabled = AC_IMAGE_INVERT_FILTER | AC_IMAGE_GRAYSCALE_FILTER; _lastViewedItemProperties._arePropertiesValid = false;}
    void resetDisplayedProperties() { _lastViewedItemProperties._arePropertiesValid = false;}

    // Return true iff there is an open images / buffer views:
    bool isImageBufferItemDisplayed() const {return ((_displayedThumbnailViews.size() > 0) || (_displayedImageBufferViews.size() > 0));};

    // Indicates if the RGBA buttons are enabled or not:
    unsigned int imageActionsEnabled() const {return _imageActionsEnabled;};
    void setImageActionsEnabled(unsigned int actionsEnabled) {_imageActionsEnabled = actionsEnabled;};

    // Saves the last viewed item properties:
    virtual void saveLastViewedItemProperties();

    // Set my views:
    void openImageBufferView(gdImageAndBufferView* pImageBufferView);
    void closeImageBufferView(gdImageAndBufferView* pImageBufferView);

    void openThumbnailView(gdThumbnailView* pThumbnailView);
    void closeThumbnailView(gdThumbnailView* pThumbnailView);

    void setFocusedViews(gdImageAndBufferView* pImageBufferView, gdThumbnailView* pThumbnailView, bool isDebuggedProcessSuspended);
    gdImageAndBufferView* focusedImageBufferView() {return _pFocusedImageBufferView;}
    gdThumbnailView* focusedThumbnailView() {return _pFocusedThumbnailView;}

    // Thumbnail view:
    bool isThumbnailViewActive() const {return _isThumbnailViewCurrentlyActive;};

    // Retrieves the current selected viewer item
    afApplicationTreeItemData* getCurrentlySelectedViewerItem();

    // Export all textures and buffers viewer functions:
    bool generateAllItemsExportList(gtVector<afApplicationTreeItemData*>& exportListVector);
    void exportAllTexturesAndBuffers(const gtString& outputDir, apFileType fileType, bool shouldOverwriteFiles);
    void showSaveAllItemsDialog(const gtVector<apFileType>& fileTypesVector);

    // Internal aid utilities for exporting files
    bool generateExportItemName(afApplicationTreeItemData* pViewerItem, gtString& exportName, gtString& exportFileName);

    // wxEvent handlers (Internal viewer components events):
    void onStandardPointer();

    // ToolBars event handlers:
    void onZoomIn();
    void onZoomOut();
    void onPan();
    void onColorChannel(acImageItemAction colorAction);
    void onGrayscale();
    void onInvert();
    void onBestFit();
    void onRotateLeft();
    void onRotateRight();

    // Menu Commands:
    void onSaveImageAs(afApplicationTreeItemData* pSelectedItemData);
    void onSaveRawDataAs(afApplicationTreeItemData* pViewerItem);
    void onSaveAllAsImage();
    void onSaveAllAsRawData();
    void onMenuImageViewSelected();
    void onMenuDataViewSelected();
    void onEditCopy();
    void onEditSelectAll();
    void onOriginalSize();

    // Update command for the tools:
    bool shouldCommandBeEnabled(bool& isChecked, int commandId) const;

    /// \returns should command be enabled
    bool shouldEnableImageViewingCommand() const;
    /// \brief checks if command is checkable, calls shouldEnableImageViewingCommand() to determine if command is enabled
    /// \param[out] isChecked command is checkable
    /// \return should command be enabled
    bool shouldEnableImageViewingCommand(bool& isChecked, int commandId) const;
    bool shouldEnableZoom(int commandId) const;
    bool shouldEnableEdit() const;
    bool shouldEnableChannelTool(bool& isChannelChecked, int colorFilter, bool checkByPassFilters = true) const;
    bool shouldEnableImageViewCommand(bool& isChecked) const;
    bool shouldEnableDataViewCommand(bool& isChecked) const;
    bool shouldEnableSaveAllCommand() const;
    bool shouldEnableSaveRawDataCommand() const;

    // Updates the RGBA channels buttons enabled status and linked status:
    void enableImageActionsByItemType(afTreeItemType itemType);
    bool checkWhichImageActionsAreEnabled(const acRawFileHandler* pRawDataHandler);

    // Update the opened images / buffers / thumbnail views:
    void updateOpenedViewsOnEvent(bool isSuspended);

    // Update hex mode on all image and buffer view:
    void updateOpenedViewsHexDisplayMode(bool hexMode);

    // Return an existing view:
    gdImageAndBufferView* getExistingImageBufferView(const osFilePath& objectFilePath);
    gdThumbnailView* getExistingThumbnailView(const osFilePath& objectFilePath);
    bool removeExistingView(QWidget* pExistingView);

    // Progress utilities:
    void initProgress(int amountOfItems);

protected:

    // Current displayed thumbnail views:
    gtVector<gdThumbnailView*> _displayedThumbnailViews;

    // Thumbnail view:
    gdThumbnailView* _pFocusedThumbnailView;

    // Currently displayed image / buffer views:
    gtVector<gdImageAndBufferView*> _displayedImageBufferViews;

    // Focused image / buffer view:
    gdImageAndBufferView* _pFocusedImageBufferView;

    // Currently active context:
    apContextID _activeContext;

    // Flags that we are in a glBegin - glEnd block:
    bool _isInGLBeginEndBlock;

    // Indicates if the RGBA buttons are enabled or not:
    unsigned int _imageActionsEnabled;

    // Contain display properties for the last viewed item:
    acDisplayedItemProperties _lastViewedItemProperties;

    // Have the viewer's contents been update since the last debug break:
    bool _isInfoUpdated;

    // Contain true iff the thumbnail view is currently active:
    bool _isThumbnailViewCurrentlyActive;

    // Contain true if the current focused view display a "real" loaded item:
    bool _isItemLoaded;

    // Contain true if the debugged process is currently suspended:
    bool _isDebuggedProcessSuspended;
};

#endif // __gdImageAndBufferViewsController
