//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file acDataView.h
///
//==================================================================================

//------------------------------ acDataView.h ------------------------------

#ifndef __ACDATAVIEW
#define __ACDATAVIEW

// Qt:
#include <QtWidgets>

QT_BEGIN_NAMESPACE
class QLineEdit;
class QBoxLayout;
QT_END_NAMESPACE

// Forward declaration:
class acVirtualListCtrl;

// Infra:
#include <AMDTBaseTools/Include/gtAutoPtr.h>
#include <AMDTBaseTools/Include/gtPtrVector.h>
#include <AMDTBaseTools/Include/gtString.h>
#include <AMDTOSAPIWrappers/Include/oaTexelDataFormat.h>
#include <AMDTApplicationComponents/Include/acDataViewItem.h>
#include <AMDTApplicationComponents/Include/acImageManager.h>
#include <AMDTApplicationComponents/Include/acRawFileHandler.h>
#include <AMDTApplicationComponents/Include/acTabWidget.h>

// Local:
#include <AMDTApplicationComponents/Include/acApplicationComponentsDLLBuild.h>

// need to undef Bool after all includes so the moc will compile in Linux
#undef Bool

#define AC_DATA_VIEW_PIXEL_POSITION_NOT_IN_GRID QPoint(-1 ,-1)

// Forward decelerations:
class acDataViewGridTable;

// ----------------------------------------------------------------------------------
// Class Name:           acDataView : public QWidget
// General Description:  Raw data view
// Author:               Eran Zinman
// Creation Date:        27/7/2007
// ----------------------------------------------------------------------------------
class AC_API acDataView : public QWidget
{
    Q_OBJECT

public:
    // Default CTOR (required by Qt 5.2):
    acDataView() {}

    // Constructor:
    acDataView(QWidget* pParent, const gtVector<QString>& notebookPagesNames);

    // Destructor:
    ~acDataView();

public:

    // Loads a image / buffer into the data view:
    bool loadDataIntoGridView(acImageItemID canvasItemID, acRawFileHandler* pRawDataHandler, bool isImageViewShown, bool doesItemContainMultiplePages, const acDataViewItem::acDataViewInfo& viewInfo);

    // Clears the grid and release the allocated memory:
    void clearGrid();

    // Updates a texture / buffer attributes:
    bool updateDataItem();

    // Clears the active table:
    void clearActiveTable() {m_activeTableCanvasID = AC_IMAGE_MANAGER_ID_NONE;};

    // Are items currently selected?
    bool areItemsSelected();
    void onEdit_Copy();
    void onEdit_SelectAll();

    int gridRowCount();
    int gridColumnCount();


    // Update the grid view:
    void updateGrid();

public:

    // Set table with the canvasItemId as the active table
    bool setActiveTable(acImageItemID canvasItemID);

    // Get active table canvas ID
    acImageItemID getActiveTableCanvasID() { return m_activeTableCanvasID; }

    // Highlight the image pixel in the grid
    void highlightPixelPosition(acImageItemID canvasItemID, QPoint pixelPos, bool bSelectedFromImage = false);

    // Return the last calculated grid pixel position
    QPoint getCurrentGridPixelPosition() { return m_lastGridPixelPosition; };

    /// Returns the last selected grid pixel position
    QPoint getLastSelectedGridPixelPosition() { return m_lastSelectedGridPixelPosition; }

    // Apply rotation for all items in the data view
    void rotateDataView(int value);

    // Apply image actions (bitwise input acImageItemAction):
    void applyImageActions(unsigned int actions);

    // Enable / Disable channel from the raw data
    void enableChannel(oaTexelDataFormat dataFormat, bool isEnabled);

    // Enable / Disable primary channel from the raw data
    void enablePrimaryChannel(bool isEnabled);

    // Return the raw data handler from the data view
    acRawFileHandler* getRawDataHandler();

    // Get data item by canvas ID
    acDataViewItem* getDataItemByCanvasID(acImageItemID canvasItemID);

    // Return the data view rotation angle
    bool getRotationAngle(double& rotateAngle);

    // Return a tables name, according to a given canvas ID
    bool getTableName(acImageItemID canvasItemID, gtString& tableName);

    // Change hex mode from outside:
    void setHexDisplayMode(bool hexMode);

    // Enable / disable hexadecimal check box:
    void enableHexCheckbox(bool enable);

    // Checks if there is a link between Data and Image views
    bool isDataAndImageViewsLinked()const;

    // Sets best grid cell size
    void setBestFitGridCellSizes();

protected slots:

    void onGridItemHovered(const QModelIndex& index);
    void onShowHexValues(int state);
    void onShowHexValues(bool checked);
    void onSelectAll();
    void onNormalizeCheckBox(int state);
    void onGridZoomValueChanged(int value);
    void onTabWidgetCurrentPageChanged(int current);
    void onGridItemDoubleClicked(const QModelIndex& clickedItem);
    void onGridItemClicked(const QModelIndex& clickedItem);
    void onGridItemPressed(const QModelIndex& clickedItem);
    void onCurrentItemChanged(const QModelIndex& current, const QModelIndex& previous);
    void onSelectedItemChanged(const QItemSelection& selected, const QItemSelection& deselected);
    void onAboutToShowContextMenu();
    void onScroll();
    void updateOnlyVisibleCells();
signals:
    // Pixel position changed signal:
    void pixelPositionChanged(acImageItemID imageItemID, const QPoint& posOnImage, bool mouseLeftDown, bool mouseDoubleClick);
    void hexChanged(bool displayHex);

protected:

    // Rotate raw data, given an angle
    void rotateRawDataByAngle(acDataViewItem* pDataViewItem, int rotateAngle);

    // Safe update function. Checks to see if data item is not being currently displayed
    bool updateDataTable(acDataViewItem* pDataViewItem);

    // Calculate the cell best fit size
    void getCellBestFit(int& minWidth, int& minHeight);
    void getCellBestFit(int& minWidth, int& minHeight, oaDataType dataType);


    // Calculate the labels best fit size
    void getGridLabelBestFit(int& minLabelWidth, int& minLabelHeight);

    // Right click menu:
    void extendGridContextMenu();

    void onUpdateShowHexValues(bool& isEnabled, bool& isChecked);

    // Creates the grid table
    void centerCellInGridView(int row, int col);

    // Matrix position convert functions
    QPoint pixelPositionToGridPosition(QPoint pixelPos);
    QPoint gridPositionToPixelPosition(QPoint gridPos);

    // Grid GUI functions:
    void setGridFontSize(int fontPointSize);
    void adjustDataGridLayout();


    // Clears all pages from the notebook
    void clearNoteBookPages();

    // View layout functions
    void createViewLayout(const gtVector<QString>& notebookPagesNames);


    acVirtualListCtrl* currentDataGrid() const ;

protected:

    // The data grid displaying the data
    gtVector<acVirtualListCtrl*> m_dataGrids;

    acTabWidget* m_pTablesNotebook;

    // Context menu additional actions:
    QAction* m_pSelectAllAction;
    QAction* m_pShowHexAction;

    // The "Link" CheckBox
    QCheckBox* m_pLinkCheckBox;

    // The show normalized values CheckBox
    QCheckBox* m_pNormalizeValuesCheckBox;

    // The show hex values CheckBox
    QCheckBox* m_pShowHexValuesCheckBox;

    // Sizers:
    QGridLayout* m_pMainLayout;

    // The grid zoom level slider
    QSlider* m_pGridZoomSlider;

    // Default grid font size
    int m_defaultGridFontSize;

    // Currently active table canvasID (The ID belongs to the canvasID of the data view parent)
    acImageItemID m_activeTableCanvasID;

    // Store the last pixel position to the mouse was hovering on
    QPoint m_lastGridPixelPosition;

    // Store pixel position selected on image view
    QPoint m_lastImageSelectedPixelPosition;

    // Contain the index for the last page that was added since the last clear grid:
    int m_lastPageAddedIndex;

    // Store the last pixel position where mouse was clicked on
    QPoint m_lastSelectedGridPixelPosition;


private:

    // A Vector containing all the raw data items:
    gtPtrVector<acDataViewItem*> m_rawDataItem;

};

#endif  // __acDataView
