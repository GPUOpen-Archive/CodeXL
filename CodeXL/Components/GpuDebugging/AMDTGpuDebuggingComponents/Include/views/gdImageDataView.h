//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gdImageDataView.h
///
//==================================================================================

//------------------------------ gdImageDataView.h ----------------------------

#ifndef __GDIMAGEDATAVIEW
#define __GDIMAGEDATAVIEW

// Forward declaration:
class afProgressBarWrapper;

// Qt:
#include <QtWidgets>

// Infra:
#include <AMDTApplicationComponents/Include/acImageManager.h>
#include <AMDTApplicationComponents/Include/acDataView.h>

// AMDTApplicationFramework:
#include <AMDTApplicationFramework/Include/afAppStringConstants.h>

// AMDTApplicationFramework:
#include <AMDTApplicationFramework/Include/views/afBaseView.h>

// Local:
#include <AMDTGpuDebuggingComponents/Include/gdGDebuggerAppDLLBuild.h>
#include <AMDTGpuDebuggingComponents/Include/views/gdImagesAndBuffersControlPanel.h>
#include <AMDTGpuDebuggingComponents/Include/views/gdImagesAndBuffersExporter.h>
#include <AMDTGpuDebuggingComponents/Include/gdDebugApplicationTreeData.h>


// Holds the display view state (which pages are displayed):
enum gdDisplayedViewsState
{
    GD_UNINITIALIZED_VIEW_STATE,

    // Text only:
    GD_TEXT_ONLY,

    // Data and image, image is open by default:
    GD_IMAGE_AND_DATA,

    // Data and image, data is open by default:
    GD_DATA_AND_IMAGE,

    // Only data view:
    GD_DATA_ONLY
};

// Holds displayed view properties:
struct gdDisplayedViewProperties
{
    gdDisplayedViewsState _viewState;
    int _lastSelectedPageIndex;
    QPoint _imageViewPosition;
    QPoint _dataViewPosition;
    bool _wasSplitProgrammaticallyDeleted;
    bool _forceControlPanelDisplay;
};


// Defines the zoom minimum and maximum values:
#define GD_IMAGES_AND_DATA_MIN_ZOOM_LEVEL 5             // Minimum zoom level: 5%
#define GD_IMAGES_AND_DATA_MAX_ZOOM_LEVEL 1000              // Maximum zoom level: 1000%
#define GD_IMAGES_AND_DATA_CONTROL_PANEL_HEIGHT 60
#define GD_IMAGES_AND_DATA_CONTROL_TEXT_CAPTION_X_MARGIN 20
#define GD_IMAGES_AND_DATA_CONTROL_VALUE_CAPTION_X_MARGIN 10

// ----------------------------------------------------------------------------------
// Class Name:          GD_API gdImageDataView : public QWidget, public afBaseView
// General Description:  Represents an object displayed both by data & image.
//                       This view is used to contain functionality both for
//                       images & buffer view, and for multi watch view
// Author:              Sigal Algranaty
// Creation Date:       10/6/2012
// ----------------------------------------------------------------------------------
class GD_API gdImageDataView : public QWidget, public afBaseView
{
    Q_OBJECT

public:
    // Constructor:
    gdImageDataView(QWidget* pParent, afProgressBarWrapper* pProgressBar);

    // Destructor:
    ~gdImageDataView();

    // Accessors:
    acDataView* dataView() const {return _pDataViewManager;};
    acImageManager* imageView() const {return _pImageViewManager;};
    const acDisplayedItemProperties& lastViewedItemProperties() const {return _lastViewedItemProperties;};
    afApplicationTreeItemData* displayedItemData() const {return _pDisplayedItemTreeData;};

    /// Virtual function which is used to update the currently displayed texture heading
    virtual void UpdateTextureHeading(gdDebugApplicationTreeData* pGDData, int newMipLevel);


    // Virtual functions:
    virtual void clearView();
    virtual bool setFrameLayout(const QSize& viewSize);
    virtual void displayCurrentTextureMiplevel(int miplevel, bool forceReload = false);
    virtual acRawFileHandler* getCurrentFilterRawDataHandler(unsigned int index);
    virtual bool handleDataCellDoubleClick(acImageItemID canvasId, acImageItem* pImageItem, QPoint posOnImage);
    bool zoomedImagePositionToImagePosition(acImageItem* pImageItem, const QPoint& posOnImage, QPoint& absolutePosOnImage, bool isHoveringImage, bool bFlipMatters = true);

    /*
    // Register canvas events:
    void registerToRecieveCanvasEvents(wxEvtHandler& eventHandler);
    void unregisterFromRecievingCanvasEvents(wxEvtHandler& eventHandler);
    */

    void applyLastViewedItemProperties(const acDisplayedItemProperties& lastViewedItemProperties, bool applyZoom);
    void saveLastViewedItemProperties(int currentZoomLevel);

    // Return true iff the image view is focused:
    bool isImageViewFocused() const;

    // Return true iff the image view is shown:
    bool isImageViewShown() const;

    // Select image / data view:
    void selectImageView(bool select);

    // Active tool:
    void setActiveTool(acImageManagerToolType tool);
    acImageManagerToolType getActiveTool() const;
    void resetActiveTool();

    int getBestFitPercentage();
    bool isNewZoomLevelPossible(int newZoomLevel) const;

    void adjust3dTextureLevel(int oldLevel, int newLevel);

    // Control panel accessors:
    void clearCurrentPixelInformation();

    // Image view accessors:
    void setFilterForAllImages(unsigned int filter, double value = 0.0);

    // Update the pixel position in the data / image / control views:
    void updatePixelPosition(acImageItemID canvasId, acImageItem* pImageItem, QPoint posOnImage, bool leftButtonDown);

    // Split & notebook:
    bool setImageViewMode(apTextureType textureType);
    void displayRelevantNotebookPages();

    void applyBestFit(int& newZoomLevel);

    void forceImageManagerRepaint();
    void calculateImagesLayout();

    // Edit menu commands
    virtual void onUpdateEdit_Copy(bool& isEnabled);
    virtual void onUpdateEdit_SelectAll(bool& isEnabled);

    virtual void onEdit_Copy();
    virtual void onEdit_SelectAll();

    // Export an item to disk:
    static bool exportSpyData(afApplicationTreeItemData* pViewerItem);

    // Save file dialog:
    void showSaveSingleFileDialog(const gtVector<apFileType>& fileTypesVector, afApplicationTreeItemData* pViewerItem);
    static bool exportViewerItemToDisk(afApplicationTreeItemData* pViewerItem, const osDirectory& outputDir, apFileType fileType, gtPtrVector<gdFileExporterOutputResult*>& exportResult, bool overwriteFiles = true, gtString suggestedFileName = AF_STR_Empty);

    // Generates a suggested file name for saving a file
    bool generateSuggestedExportFileName(afApplicationTreeItemData* pViewerItem, gtString& suggestedFileName);

    // Export the current viewed object (in the images and buffers viewer) to disk:
    bool exportCurrentlyViewedObjectToDisk(const osFilePath& outputFilePath, apFileType fileType);

    // Current raw data handler:
    acRawFileHandler* getCurrentRawDataHandler() const;

    void setViewStateAccordingToItemType(afTreeItemType itemType, const afItemLoadStatus& itemLoadStatus);
    void adjustViewerAfterItemLoading();

    // Write the requested message in the image manager:
    void displayItemMessageIfNeeded();

    // Zoom:
    void resetManagerZoomLevel() {_currentZoomLevel = 100;};
    void setTextureManagerZoomLevel(int zoomLevel);
    int getNextZoomLevel(unsigned int currentZoomLevel, bool upDirection = true) const;

    unsigned int currentZoomLevel() const { return _currentZoomLevel; };
    void onZoomIn();
    void onZoomOut();
    bool shouldEnableZoom(int commandId) const;
    static const gtVector<unsigned int>& availableZoomLevels() {return _stat_availableZoomLevelsVector;};
    bool tryToChangeZoomLevel(const gtString& enteredString);
    static bool isZoomComboBoxTextValid(const gtString& enteredString);

    // Hex display mode:
    void setHexDisplayMode(bool hexDisplayMode);

protected slots:

    virtual void onNoteBookPageChange(int currentPage);
    void onStandardPointerTool();
    void onImageItemEvent(acImageItemID imageItemID, const QPoint& posOnImage, bool mouseLeftDown, bool mouseDoubleClick);
    void onDataPixelMouseEvent(acImageItemID imageItemID, const QPoint& posOnImage, bool mouseLeftDown, bool mouseDoubleClick);
    void onHexChanged(bool displayHex);
    void onZoomChanged(int nNewZoomLevel);

protected:

    // General images and buffers loading functions:
    bool loadItemToImageAndDataViews(acRawFileHandler* pRawDataHandler, const gtString& objectName = L"", QPoint matrixPos = QPoint(0, 0));
    int loadRawDataToImageViewer(acRawFileHandler* pRawFileHandler, const gtString& objectName, QPoint matrixPos);
    bool loadRawDataToDataViewer(acRawFileHandler* pBuffer, int canvasID, bool isImageViewShown, bool doesItemContainMultiplePages);

    // Expand the image control panel:
    void setupControlPanel();

    // Normalize raw data:
    bool normalizeRawData(acRawFileHandler* pRawDataHandler);

    // Overrides QWidget:
    virtual void resizeEvent(QResizeEvent* pResizeEvent);
    void SaveSelectedPixelData(acImageItemID canvasId, acImageItem* pImageItem, QPoint posOnImage);

    // Overrides QWidget::wheelEvent
    virtual void wheelEvent(QWheelEvent* event);
protected:

    // The image view:
    acImageManager* _pImageViewManager;

    // The data view:
    acDataView* _pDataViewManager;

    // The center AUI notebook:
    acTabWidget* m_pTabWidget;

    // The main window sizer:
    QHBoxLayout* _pMainLayout;

    // Contain true if best fit was applied after the window size is truely set:
    QSize _previousSize;

    // The image control panel:
    gdImagesAndBuffersControlPanel* _pImageControlPanel;

    // True if we should skip selection events:
    bool _ignoreSelections;

    // Contain the names of the data view pages (for cube map textures):
    gtVector<QString> _dataViewPagesNames;

    // Hold the displayed pages configuration:
    gdDisplayedViewProperties _currentViewsDisplayProperties;

    // Properties for the last displayed item:
    acDisplayedItemProperties _lastViewedItemProperties;

    // The currently displayed item data:
    afApplicationTreeItemData* _pDisplayedItemTreeData;

    // Contain only information for the id of the displayed object.
    // This item data is used for redisplay of objects after process run:
    afApplicationTreeItemData _displayedItemId;

    // Current zoom level in the Image Manager:
    int _currentZoomLevel;

    // Available zoom levels:
    static gtVector<unsigned int> _stat_availableZoomLevelsVector;

    // Static item data used for items that display only text:
    static afApplicationTreeItemData* _pStaticEmptyItemData;

    // True if image should be flipped
    bool m_bFlipImage;

    // Selected pixel info
    acImageItemID   m_nSelectedPixelImageItemID;
    acImageItem*    m_pSelectedPixelImageItem;
    QPoint          m_pointSelectedPixelPosition;
    bool            m_bPixelInfoSelected;
};

#endif  // __GDIMAGEDATAVIEW
