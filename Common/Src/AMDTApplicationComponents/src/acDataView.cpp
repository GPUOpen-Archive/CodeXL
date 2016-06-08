//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file acDataView.cpp
///
//==================================================================================

//------------------------------ acDataView.cpp ------------------------------

// Qt:
#include <QtWidgets>

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTBaseTools/Include/gtString.h>
#include <AMDTOSWrappers/Include/osDebuggingFunctions.h>
#include <AMDTOSWrappers/Include/osOSDefinitions.h>
#include <AMDTOSAPIWrappers/Include/oaDataType.h>

// Local:
#include <AMDTApplicationComponents/Include/acCommandIDs.h>
#include <AMDTApplicationComponents/Include/acDataView.h>
#include <AMDTApplicationComponents/Include/acDataViewGridTable.h>
#include <AMDTApplicationComponents/Include/acFunctions.h>
#include <AMDTApplicationComponents/Include/acVirtualListCtrl.h>
#include <inc/acStringConstants.h>

#define AC_DATA_VIEW_MIN_GRID_ZOOM_LEVEL       -2 // Minimum grid zoom level
#define AC_DATA_VIEW_DEFAULT_GRID_ZOOM_LEVEL  -1 // Default grid zoom level
#define AC_DATA_VIEW_MAX_GRID_ZOOM_LEVEL       6 // Maximum grid zoom level
#define AC_DATA_VIEW_CELL_HMARGIN         10
#define AC_DATA_VIEW_CELL_VMARGIN        10


const int SLIDER_PAGE_STEP = 1;

// ---------------------------------------------------------------------------
// Name:        acDataView::acDataView
// Description: Constructor - Creates the data view
// Arguments:   same as QWidget
// Author:      Eran Zinman
// Date:        27/7/2007
// ---------------------------------------------------------------------------
acDataView::acDataView(QWidget* pParent, const gtVector<QString>& notebookPagesNames)
    : QWidget(pParent),
      m_pTablesNotebook(NULL), m_pSelectAllAction(NULL), m_pShowHexAction(NULL),
      m_pLinkCheckBox(NULL), m_pNormalizeValuesCheckBox(NULL), m_pShowHexValuesCheckBox(NULL),
      m_pMainLayout(NULL), m_pGridZoomSlider(NULL), m_defaultGridFontSize(-1),
      m_activeTableCanvasID(-1), m_lastGridPixelPosition(AC_DATA_VIEW_PIXEL_POSITION_NOT_IN_GRID),
      m_lastImageSelectedPixelPosition(AC_DATA_VIEW_PIXEL_POSITION_NOT_IN_GRID), m_lastPageAddedIndex(-1), m_lastSelectedGridPixelPosition(AC_DATA_VIEW_PIXEL_POSITION_NOT_IN_GRID)

{
    // Create the layout of the acDataView:
    createViewLayout(notebookPagesNames);
}


// ---------------------------------------------------------------------------
// Name:        acDataView::~acDataView
// Description: Destructor
// Author:      Eran Zinman
// Date:        27/7/2007
// ---------------------------------------------------------------------------
acDataView::~acDataView()
{
    // Release grid from memory
    clearGrid();
}

// ---------------------------------------------------------------------------
// Name         : acDataView::UpdateOnlyVisibleCells
// Description  : This function resizes only the grid cells that are visible
// Return Val   : void
// Author       : Jeganathan Swaminathan
// Date         : 25/9/2012
// ---------------------------------------------------------------------------
void acDataView::updateOnlyVisibleCells()
{
    // Sanity check:
    // Get the current grid object:
    acVirtualListCtrl* pCurrentGrid = currentDataGrid();
    GT_IF_WITH_ASSERT(pCurrentGrid != NULL)
    {
        int minVisibleRow, maxVisibleRow, minVisibleCol, maxVisibleCol;

        minVisibleCol = pCurrentGrid->horizontalHeader()->visualIndexAt(0);
        minVisibleRow = pCurrentGrid->verticalHeader()->visualIndexAt(0);

        QWidget* pWidget = pCurrentGrid->childAt(minVisibleRow, minVisibleCol);

        if (NULL == pWidget)
        {
            return;
        }

        int singleCellWidth = pWidget->width();
        int singleCellHeight = pWidget->height();

        if ((singleCellWidth <= 0) || (singleCellHeight <= 0))
        {
            return;
        }

        int tableWidth  = pCurrentGrid->width();
        int tableHeight = pCurrentGrid->height();

        maxVisibleRow = minVisibleRow + tableHeight / singleCellHeight + 20;
        maxVisibleCol = minVisibleCol + tableWidth / singleCellWidth + 15;

        //Ensure the max row visible doesn't exceed the maximum available
        //rows in table.
        int nMaxRow = gridRowCount();
        int nMaxCol = gridColumnCount();

        if (maxVisibleRow > nMaxRow)
        {
            maxVisibleRow = nMaxRow - 1;
        }

        //Ensure the max col visible doesn't exceed the maximum available
        //columns in table.
        if (maxVisibleCol > nMaxCol)
        {
            maxVisibleCol = nMaxCol - 1;
        }

        //Here you go, now resize only visible rows and columns here
        for (int col = minVisibleCol; col <= maxVisibleCol; ++col)
        {
            pCurrentGrid->resizeColumnToContents(col);
        }

        for (int row = minVisibleRow; row <= maxVisibleRow; ++row)
        {
            pCurrentGrid->resizeRowToContents(row);
        }
    }
}


// ---------------------------------------------------------------------------
// Name:        acDataView::onGridItemHovered
// Description: Is responding to the grid mouse enter signal
// Arguments:   const QModelIndex& index
// Return Val:  void
// Author:      Sigal Algranaty
// Date:        9/5/2012
// ---------------------------------------------------------------------------
void acDataView::onGridItemHovered(const QModelIndex& index)
{
    // Get the current grid object:
    acVirtualListCtrl* pCurrentGrid = currentDataGrid();

    // If grid exists:
    if (pCurrentGrid)
    {
        QPoint gridPoint(index.column(), index.row());

        // check if gridPoint is valid
        if (gridPoint != AC_DATA_VIEW_PIXEL_POSITION_NOT_IN_GRID)
        {
            // Save the last grid pixel position by converting grid position to pixel position:
            m_lastGridPixelPosition = gridPositionToPixelPosition(gridPoint);
            // Send the event to my parent, letting him know mouse position have changed
            emit pixelPositionChanged(m_activeTableCanvasID, m_lastGridPixelPosition, false, false);
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        acDataView::getRawDataHandler
// Description: Return the raw data handler from the data view
// Return Val:  Pointer to the raw file handler object
// Author:      Eran Zinman
// Date:        3/8/2007
// ---------------------------------------------------------------------------
acRawFileHandler* acDataView::getRawDataHandler()
{
    acRawFileHandler* pRawDataHandler = NULL;

    // Get amount of data view items
    int amountOfIndices = m_rawDataItem.size();

    // Make sure we have at least one item loaded:
    if (amountOfIndices > 0)
    {
        // Get first item
        acDataViewItem* pDataItem = m_rawDataItem[0];
        GT_IF_WITH_ASSERT(pDataItem != NULL)
        {
            pRawDataHandler = pDataItem->getRawDataHandler();
        }
    }

    return pRawDataHandler;
}

// ---------------------------------------------------------------------------
// Name:        acDataView::getRawDataHandler
// Description: Return the data view rotation angle
// Arguments:   rotateAngle - Output rotation angle
// Return Val:  bool - Success / Failure
// Author:      Eran Zinman
// Date:        10/1/2008
// ---------------------------------------------------------------------------
bool acDataView::getRotationAngle(double& rotateAngle)
{
    bool retVal = false;

    // Get amount of data view items:
    int amountOfIndices = m_rawDataItem.size();

    // Make sure we have at least one item loaded:
    GT_IF_WITH_ASSERT(amountOfIndices > 0)
    {
        // Get first item:
        acDataViewItem* pDataItem = m_rawDataItem[0];
        GT_IF_WITH_ASSERT(pDataItem != NULL)
        {
            rotateAngle = pDataItem->rotationAngle();

            retVal = true;
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        acDataView::rotateDataView
// Description: Rotates the data view
// Arguments:   double - Amount of degrees to rotate by
// Author:      Eran Zinman
// Date:        8/1/2008
// ---------------------------------------------------------------------------
void acDataView::rotateDataView(int value)
{
    // Get amount of raw data items
    int amountOfIndices = m_rawDataItem.size();

    // Loop through all the raw data items
    for (int i = 0; i < amountOfIndices; i++)
    {
        if (m_rawDataItem[i] != NULL)
        {
            // Rotate the raw data item
            bool rc1 = m_rawDataItem[i]->rotateRawDataByAngle((int)value);
            GT_ASSERT(rc1);

            // Update the raw data table
            updateDataTable(m_rawDataItem[i]);
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        acDataView::enableChannel
// Description: Enable / Disable channel from the raw data
// Arguments:   dataFormat - The channel to enable / disable
//              isEnabled - Should we enable / disable
// Author:      Eran Zinman
// Date:        8/1/2008
// ---------------------------------------------------------------------------
void acDataView::enableChannel(oaTexelDataFormat dataFormat, bool isEnabled)
{
    // Loop through all the raw data items
    int amountOfIndices = m_rawDataItem.size();

    for (int i = 0; i < amountOfIndices; i++)
    {
        if (m_rawDataItem[i] != NULL)
        {
            // Enable / Disable channel in the raw data table
            m_rawDataItem[i]->enableRawDataChannel(dataFormat, isEnabled);

            // Update the raw data table
            updateDataTable(m_rawDataItem[i]);
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        acDataView::enablePrimaryChannel
// Description: Enable / Disable primary channel from the raw data
// Arguments:   isEnabled - Should we enable / disable
// Author:      Eran Zinman
// Date:        9/1/2008
// ---------------------------------------------------------------------------
void acDataView::enablePrimaryChannel(bool isEnabled)
{
    // Loop through all the raw data items
    int amountOfIndices = m_rawDataItem.size();

    for (int i = 0; i < amountOfIndices; i++)
    {
        if (m_rawDataItem[i] != NULL)
        {
            // Enable / Disable primary channel in the raw data table
            m_rawDataItem[i]->enablePrimaryRawDataChannel(isEnabled);

            // Update the raw data table
            updateDataTable(m_rawDataItem[i]);
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        acDataView::updateDataTable
// Description: This function tells the acDataViewItem* object to update
//              it's internal table. The reason for calling this function, instead
//              of directly telling the acDataViewItem to udpate himself, is
//              because we might have the item table displaying at the moment.
//
//              If the table is being displayed at the moment, we remove the table,
//              update it and put it back in.
//
// Arguments:   tpDataViewItem - The raw data to be displayed in the table
// Return Val:  bool - Success / Failure
// Author:      Eran Zinman
// Date:        3/8/2007
// ---------------------------------------------------------------------------
bool acDataView::updateDataTable(acDataViewItem* pDataViewItem)
{
    bool retVal = false;

    // Sanity check:
    GT_IF_WITH_ASSERT(pDataViewItem != NULL)
    {
        // Get the data view item data grid table
        acDataViewGridTable* pGridTable = pDataViewItem->gridTable();
        GT_IF_WITH_ASSERT(pGridTable != NULL)
        {
            // Is this table the currently displayed table?
            bool isActiveTable = false;

            // Get the current grid object:
            acVirtualListCtrl* pCurrentGrid = currentDataGrid();
            GT_IF_WITH_ASSERT(pCurrentGrid != NULL)
            {
                // Are the pointers equal? (Are we using this table at the moment)
                if (pCurrentGrid->model() == pGridTable)
                {
                    // Flag that this is the currently displayed table
                    isActiveTable = true;

                    // Remove table from the grid:
                    pCurrentGrid->setModel(NULL);
                }

                // Should we insert the table back into the grid?
                if (isActiveTable)
                {
                    // Get new table pointer
                    pGridTable = pDataViewItem->gridTable();
                    GT_IF_WITH_ASSERT(pGridTable != NULL)
                    {
                        // Set the table back into the acDataViewItem object
                        pCurrentGrid->setModel(pGridTable);

                        // Update the grid data:
                        pCurrentGrid->updateData();

                        QItemSelectionModel* pModel = currentDataGrid()->selectionModel();

                        if (pModel != NULL)
                        {
                            bool rc = connect(pModel, SIGNAL(currentChanged(const QModelIndex&, const QModelIndex&)), this, SLOT(onCurrentItemChanged(const QModelIndex&, const QModelIndex&)));
                            GT_ASSERT(rc);
                            rc = connect(pModel, SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection&)), this, SLOT(onSelectedItemChanged(const QItemSelection&, const QItemSelection&)));
                            GT_ASSERT(rc);
                        }
                    }
                }

                retVal = true;
            }
        }
    }
    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        acDataView::onNormalizeCheckBox
// Description: Occurs when normalized values CheckBox is clicked
// Arguments:   event - The event details
// Author:      Eran Zinman
// Date:        7/1/2008
// ---------------------------------------------------------------------------
void acDataView::onNormalizeCheckBox(int state)
{
    bool isEnabled = (state == Qt::Checked);

    // Loop through all the data items and set their normalized values
    int amountOfIndices = m_rawDataItem.size();

    // Loop through all the raw data items
    for (int i = 0; i < amountOfIndices; i++)
    {
        if (m_rawDataItem[i] != NULL)
        {
            // Enable / Disable normalized values show
            m_rawDataItem[i]->enableNormalizedValues(isEnabled);
        }
    }

    // Get the current grid object:
    acVirtualListCtrl* pCurrentGrid = currentDataGrid();
    GT_IF_WITH_ASSERT(pCurrentGrid != NULL)
    {
        // Update the data:
        pCurrentGrid->updateData();
    }
}

// ---------------------------------------------------------------------------
// Name:        acDataView::onShowHexValues
// Description: Occurs when "Show Hexadecimal values"  CheckBox is clicked
// Arguments:   event - The event details
// Author:      Sigal Algranaty
// Date:        14/4/2011
// ---------------------------------------------------------------------------
void acDataView::onShowHexValues(int state)
{
    bool isChecked = (state == Qt::Checked);

    onShowHexValues(isChecked);
}


// ---------------------------------------------------------------------------
// Name:        acDataView::onShowHexValues
// Description: Occurs when "Show Hexadecimal values"  CheckBox is clicked
// Arguments:   event - The event details
// Author:      Sigal Algranaty
// Date:        10/7/2012
// ---------------------------------------------------------------------------
void acDataView::onShowHexValues(bool isChecked)
{
    // Set show hex mode:
    acDataViewItem::showHexadecimalValues(isChecked);

    // Auto size the grid
    setBestFitGridCellSizes();

    // Sanity check
    // Get the current grid object:
    acVirtualListCtrl* pCurrentGrid = currentDataGrid();
    GT_IF_WITH_ASSERT(pCurrentGrid != NULL)
    {
        // Update the data:
        pCurrentGrid->updateData();
    }

    emit hexChanged(isChecked);
}

// ---------------------------------------------------------------------------
// Name:        acDataView::onUpdateShowHexValues
// Description:
// Author:      Sigal Algranaty
// Date:        17/4/2011
// ---------------------------------------------------------------------------
void acDataView::onUpdateShowHexValues(bool& isEnabled, bool& isChecked)
{
    if (m_rawDataItem.size() > 0)
    {
        // If first raw data item exist:
        GT_IF_WITH_ASSERT(m_rawDataItem[0] != NULL)
        {
            // Check if currently hex values should be shown:
            isChecked = m_rawDataItem[0]->shouldShowHexadecimalValues();

            // Get the texel format:
            acRawFileHandler* pRawFileHandler = m_rawDataItem[0]->getRawDataHandler();
            GT_IF_WITH_ASSERT(pRawFileHandler != NULL)
            {
                // Get the data format:
                oaTexelDataFormat  texelDataFormat = pRawFileHandler->dataFormat();

                // Get the displayed data type:
                gtVector<oaDataType> dataTypesVector;
                bool rc1 = oaGetTexelFormatContainedDataTypes(texelDataFormat, dataTypesVector);

                if (!rc1)
                {
                    // Get the data type from the raw file handler;
                    oaDataType dataType = pRawFileHandler->dataType();
                    dataTypesVector.push_back(dataType);
                }

                // Check if one of the current data type can be displayed as hex values:
                for (int i = 0; i < (int)dataTypesVector.size(); i++)
                {
                    isEnabled = oaCanBeDisplayedAsHexadecimal(dataTypesVector[i]);

                    if (isEnabled)
                    {
                        break;
                    }
                }
            }
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        acDataView::onTabWidgetCurrentPageChanged
// Description: Occurs when notebook page had changed. Reparent the data grid
//              to the dummy window displayed
// Arguments:   event - The event details
// Author:      Eran Zinman
// Date:        1/8/2007
// ---------------------------------------------------------------------------
void acDataView::onTabWidgetCurrentPageChanged(int currentPage)
{
    // Get the current grid object:
    acVirtualListCtrl* pCurrentGrid = currentDataGrid();
    GT_IF_WITH_ASSERT(pCurrentGrid != NULL)
    {
        // Get canvas ID matching to this page:
        acImageItemID canvasItemID = -1;
        GT_IF_WITH_ASSERT((currentPage >= 0) && (currentPage < (int)m_rawDataItem.size()) && (m_rawDataItem.size() > 0))
        {
            GT_IF_WITH_ASSERT(m_rawDataItem[currentPage] != NULL)
            {
                // Set the pointer to the current item
                canvasItemID = m_rawDataItem[currentPage]->getCanvasID();

                if (canvasItemID != -1)
                {
                    // Set the active data table
                    bool rc1 = setActiveTable(canvasItemID);
                    GT_ASSERT(rc1);
                }
            }
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        acDataView::onGridZoomValueChanged
// Description: Handle grid zoom level change
// Arguments:   int currentValue
// Author:      Sigal Algranaty
// Date:        13/6/2012
// ---------------------------------------------------------------------------
void acDataView::onGridZoomValueChanged(int currentValue)
{
    // Get the current grid object:
    acVirtualListCtrl* pCurrentGrid = currentDataGrid();
    GT_IF_WITH_ASSERT(pCurrentGrid != NULL)
    {
        // Get the current slider zoom level
        static int curVal = AC_DATA_VIEW_DEFAULT_GRID_ZOOM_LEVEL;

        // Was the zoom level changed?
        if (curVal != currentValue)
        {
            // Set the new value:
            curVal = currentValue;
            // Set the new font size
            setGridFontSize(m_defaultGridFontSize + curVal);
            // Best fit the grid font cell sizes
            setBestFitGridCellSizes();
            // Save current grid cell position
            int cellRow = -1, cellCol = -1;

            if (isDataAndImageViewsLinked() && m_lastImageSelectedPixelPosition != AC_DATA_VIEW_PIXEL_POSITION_NOT_IN_GRID)
            {
                m_lastImageSelectedPixelPosition.y();
                m_lastImageSelectedPixelPosition.x();
                QAbstractItemModel* pModel = pCurrentGrid->model();

                if (pModel != NULL)
                {
                    // If inside grid area
                    bool rc1 = (m_lastImageSelectedPixelPosition.y() <= pModel->rowCount() && m_lastImageSelectedPixelPosition.x() <= pModel->columnCount());
                    GT_IF_WITH_ASSERT(rc1)
                    {
                        // Set the cursor at the new grid position:
                        pCurrentGrid->clearSelection();
                        // Center the new grid position in the grid view
                        centerCellInGridView(m_lastImageSelectedPixelPosition.y(), m_lastImageSelectedPixelPosition.x());

                        // Get currently loaded data view item
                        acDataViewItem* pDataViewItem = getDataItemByCanvasID(m_activeTableCanvasID);
                        GT_IF_WITH_ASSERT(pDataViewItem != NULL)
                        {
                            // Select the pixel block:
                            QModelIndex index = pModel->index(m_lastImageSelectedPixelPosition.y(), m_lastImageSelectedPixelPosition.x());
                            pCurrentGrid->setCurrentIndex(index);
                            pCurrentGrid->selectionModel()->select(index, QItemSelectionModel::SelectCurrent | QItemSelectionModel::ClearAndSelect);

                            if (isVisible())
                            {
                                pCurrentGrid->setFocus();
                            }

                            pCurrentGrid->scrollTo(index);
                        }
                    }
                }
            }
            else if (pCurrentGrid->selectedIndexes().count() > 0)
            {
                //Retrieve the first visual column and row as opposed to logical column and row
                cellRow = pCurrentGrid->verticalHeader()->visualIndexAt(0);
                cellCol = pCurrentGrid->horizontalHeader()->visualIndexAt(0);
                // Center the table on the last cell position
                centerCellInGridView(cellRow, cellCol);
            }
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        acDataView::isDataAndImageViewsLinked
// Description: Checks if the data and image views are linked
// Return Val:  True - linked, False - otherwise
// Author:      Eran Zinman
// Date:        27/7/2007
// ---------------------------------------------------------------------------
bool acDataView::isDataAndImageViewsLinked()const
{
    // Are views linked?
    bool retVal = false;

    // Sanity Check:
    GT_IF_WITH_ASSERT(m_pLinkCheckBox != NULL)
    {
        retVal = (m_pLinkCheckBox->checkState() == Qt::Checked);
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        acDataView::getDataItemByCanvasID
// Description: Checks if an item with the required canvasID exists.
//              If such an item exists, return the appropriate acDataViewItem
//              object.
// Return Val:  The acDataViewItem object with the canvasID if found,
//              else return NULL.
// Author:      Eran Zinman
// Date:        31/7/2007
// ---------------------------------------------------------------------------
acDataViewItem* acDataView::getDataItemByCanvasID(int canvasItemID)
{
    acDataViewItem* pDataViewItem = NULL;

    // Get amount of raw data items
    int amountOfIndices = m_rawDataItem.size();

    // Loop through all the raw data items
    for (int i = 0; i < amountOfIndices; i++)
    {
        if (m_rawDataItem[i] != NULL)
        {
            // Does the item canvasID matching the one we are looking for?
            if (m_rawDataItem[i]->getCanvasID() == canvasItemID)
            {
                // Set the pointer to the current item
                pDataViewItem = m_rawDataItem[i];

                // Break the loop
                break;
            }
        }
    }

    // Return the data view items object
    return pDataViewItem;
}

// ---------------------------------------------------------------------------
// Name:        acDataView::pixelPositionToGridPosition
// Description: Converts pixel position to grid position
// Arguments:   pixelPos - (row, col) position in pixels
// Return Val:  Position in grid
// Author:      Eran Zinman
// Date:        29/7/2007
// ---------------------------------------------------------------------------
QPoint acDataView::pixelPositionToGridPosition(QPoint pixelPos)
{
    QPoint gridPos(0, 0);

    // Get currently loaded data item
    acDataViewItem* pDataViewItem = getDataItemByCanvasID(m_activeTableCanvasID);

    // pDataViewItem maybe NULL since m_activeTableCanvasID maybe -1
    if (pDataViewItem != NULL)
    {
        // Get number of active channels
        int activeChannels = pDataViewItem->amountOfActiveChannels();

        // Calculate grid position
        gridPos.setX(pixelPos.x());
        gridPos.setY(pixelPos.y() * activeChannels);
    }

    // Return grid position
    return gridPos;
}


// ---------------------------------------------------------------------------
// Name:        acDataView::gridPositionToPixelPosition
// Description: Converts grid position to pixel position
// Arguments:   gridPos - (row, col) position in grid
// Return Val:  Position in pixels
// Author:      Eran Zinman
// Date:        29/7/2007
// ---------------------------------------------------------------------------
QPoint acDataView::gridPositionToPixelPosition(QPoint gridPos)
{
    QPoint pixelPos(0, 0);

    // Get currently loaded data item:
    acDataViewItem* pDataViewItem = getDataItemByCanvasID(m_activeTableCanvasID);
    GT_IF_WITH_ASSERT(pDataViewItem != NULL)
    {
        // Get number of active channels
        int activeChannels = pDataViewItem->amountOfActiveChannels();

        if (activeChannels > 0)
        {
            // Calculate pixel position of the currently loaded item
            pixelPos.setX(gridPos.x());
            pixelPos.setY(gridPos.y() / activeChannels);
        }
    }

    // Return grid position
    return pixelPos;
}

// ---------------------------------------------------------------------------
// Name:        acDataView::highlightPixelPosition
// Description: Highlight the cells in the pixel position
// Arguments:   canvasItemID - The item with the canvasID to highlight
//              pixelPos - The pixel position
// Author:      Eran Zinman
// Date:        29/7/2007
// ---------------------------------------------------------------------------
void acDataView::highlightPixelPosition(acImageItemID canvasItemID, QPoint pixelPos, bool bSelectedFromImage)
{
    acDataViewItem* pDataViewItem = getDataItemByCanvasID(canvasItemID);

    // pDataViewItem can be NULL if there is no image (for example in multi watch view when no variable was selected)
    if (pDataViewItem != NULL)
    {
        // If view are linked
        if (pDataViewItem->amountOfActiveChannels() > 0 && isDataAndImageViewsLinked())
        {
            // Get the current grid object:
            acVirtualListCtrl* pCurrentGrid = currentDataGrid();
            GT_IF_WITH_ASSERT(pCurrentGrid != NULL)
            {
                // Convert pixel position to grid position
                QPoint gridPos = pixelPositionToGridPosition(pixelPos);

                if (bSelectedFromImage)
                {
                    m_lastImageSelectedPixelPosition = gridPos;
                }

                QAbstractItemModel* pModel = pCurrentGrid->model();

                if (pModel != NULL)
                {
                    // If inside grid area
                    bool rc1 = (gridPos.y() <= pModel->rowCount() && gridPos.x() <= pModel->columnCount());
                    GT_IF_WITH_ASSERT(rc1)
                    {
                        // Change the grid table, if necessary.
                        rc1 = setActiveTable(canvasItemID);
                        GT_IF_WITH_ASSERT(rc1)
                        {
                            blockSignals(true);// to prevent emitting selectedItemChanged
                            // Set the cursor at the new grid position:
                            pCurrentGrid->clearSelection();
                            blockSignals(false);

                            // Center the new grid position in the grid view
                            centerCellInGridView(gridPos.y(), gridPos.x());

                            // Get currently loaded data view item
                            pDataViewItem = getDataItemByCanvasID(m_activeTableCanvasID);
                            GT_IF_WITH_ASSERT(pDataViewItem != NULL)
                            {
                                // Select the pixel block:
                                QModelIndex index = pModel->index(gridPos.y(), gridPos.x());
                                blockSignals(true);
                                pCurrentGrid->blockSignals(true);
                                pCurrentGrid->setCurrentIndex(index);
                                pCurrentGrid->blockSignals(false);
                                blockSignals(false);


                                QItemSelectionModel* pSelectionModel = pCurrentGrid->selectionModel();
                                GT_IF_WITH_ASSERT(NULL != pSelectionModel)
                                {
                                    pSelectionModel->blockSignals(true);
                                    int activeChannels = pDataViewItem->amountOfActiveChannels();

                                    if (activeChannels > 1)
                                    {
                                        QItemSelection rowSelection;
                                        QModelIndex lastIndex = pModel->index(gridPos.y() + (activeChannels - 1), gridPos.x());
                                        rowSelection.select(index, lastIndex);
                                        pSelectionModel->select(rowSelection, QItemSelectionModel::Select | QItemSelectionModel::ClearAndSelect);
                                    }
                                    else
                                    {
                                        pSelectionModel->select(index, QItemSelectionModel::SelectCurrent | QItemSelectionModel::ClearAndSelect);
                                    }

                                    pSelectionModel->blockSignals(false);
                                    blockSignals(false);
                                }

                                if (isVisible())
                                {
                                    pCurrentGrid->setFocus();
                                }

                                pCurrentGrid->scrollTo(index, QAbstractItemView::PositionAtCenter);
                            }
                        }
                    }
                }
            }
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        acDataView::centerCellInGridView
// Description: Centers the cell in the view
// Arguments:   Row - cell row, Col - cell column
// Author:      Eran Zinman
// Date:        31/7/2007
// ---------------------------------------------------------------------------
void acDataView::centerCellInGridView(int row, int col)
{
    if ((row >= 0) && (col >= 0))
    {
        // Sanity check:
        // Get the current grid object:
        acVirtualListCtrl* pCurrentGrid = currentDataGrid();
        GT_IF_WITH_ASSERT(pCurrentGrid != NULL)
        {
            GT_IF_WITH_ASSERT(pCurrentGrid->model() != NULL)
            {
                QModelIndex lastFunctionModelIndex = pCurrentGrid->model()->index(row, col);
                pCurrentGrid->scrollTo(lastFunctionModelIndex, QAbstractItemView::PositionAtCenter);
            }
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        acDataView::setGridFontSize
// Description: Sets the grid font size
// Arguments:   fontPointSize - The new font size
// Author:      Eran Zinman
// Date:        30/7/2007
// ---------------------------------------------------------------------------
void acDataView::setGridFontSize(int fontPointSize)
{
    // Get the current grid object:
    acVirtualListCtrl* pCurrentGrid = currentDataGrid();
    GT_IF_WITH_ASSERT(pCurrentGrid != NULL)
    {
        // Reduce the grid font size by one point
        QFont cellFont = pCurrentGrid->font();

        cellFont.setPointSize(fontPointSize);
        pCurrentGrid->setFont(cellFont);
    }
}


// ---------------------------------------------------------------------------
// Name:        acDataView::adjustDataGridLayout
// Description: Adjusts the data grid layout (fonts, colors, etc...)
// Author:      Eran Zinman
// Date:        30/7/2007
// ---------------------------------------------------------------------------
void acDataView::adjustDataGridLayout()
{
    // Sanity check:
    acVirtualListCtrl* pCurrentGrid = currentDataGrid();
    GT_IF_WITH_ASSERT(pCurrentGrid != NULL)
    {
        // By default, set the grid font size to be default - 1
        setGridFontSize(m_defaultGridFontSize - 1);
    }
}

// ---------------------------------------------------------------------------
// Name:        acDataView::createViewLayout
// Description: Create the layout of the data view
// Author:      Eran Zinman
// Date:        27/7/2007
// ---------------------------------------------------------------------------
void acDataView::createViewLayout(const gtVector<QString>& notebookPagesNames)
{
    m_pMainLayout = new QGridLayout;


    // Create the "Link" CheckBox
    m_pLinkCheckBox = new QCheckBox(AC_STR_DataViewLinkImageAndDataViews);


    // Set value to "Checked":
    m_pLinkCheckBox->setChecked(true);

    // Add the CheckBox to the layout:
    m_pMainLayout->addWidget(m_pLinkCheckBox, 0, 0, 1, 3);

    // Create a normalized values CheckBox:
    m_pNormalizeValuesCheckBox = new QCheckBox(AC_STR_DataViewShowNormalizedValues);


    // Set value to "UnChecked":
    m_pNormalizeValuesCheckBox->setChecked(false);

    // Create a normalized values CheckBox:
    m_pShowHexValuesCheckBox = new QCheckBox(AC_STR_DataViewShowHexValues);


    // Set value to "UnChecked"
    m_pShowHexValuesCheckBox->setChecked(false);

    // Connect the check boxes click signals:
    bool rc = connect(m_pShowHexValuesCheckBox, SIGNAL(stateChanged(int)), this, SLOT(onShowHexValues(int)));
    GT_ASSERT(rc);

    rc = connect(m_pNormalizeValuesCheckBox, SIGNAL(stateChanged(int)), this, SLOT(onNormalizeCheckBox(int)));
    GT_ASSERT(rc);

    // Add the CheckBox to the sizer:
    m_pMainLayout->addWidget(m_pNormalizeValuesCheckBox, 1, 0, 1, 1);
    m_pMainLayout->addWidget(m_pShowHexValuesCheckBox, 2, 0, 1, 3);

    // Create the font size slider
    m_pGridZoomSlider = new QSlider(Qt::Horizontal);

    m_pGridZoomSlider->setRange(AC_DATA_VIEW_MIN_GRID_ZOOM_LEVEL, AC_DATA_VIEW_MAX_GRID_ZOOM_LEVEL);
    m_pGridZoomSlider->setPageStep(SLIDER_PAGE_STEP);

    //Disabling tracking ensures the slider value changed signal is triggered only when the mouse is released.
    //This improves performance, this is the reason why QHeaderView::ResizeToContents performance appeared
    //to be terribly bad while the root cause being Tracking was enabled which triggers the event multiples times
    //as opposed to signaling the event only when the mouse is released.
    m_pGridZoomSlider->setTracking(false);

    // Set the slider tick frequency:
    m_pGridZoomSlider->setTickInterval(1);

    // Connect the slider value changed signal:
    rc = connect(m_pGridZoomSlider, SIGNAL(valueChanged(int)), this, SLOT(onGridZoomValueChanged(int)));
    GT_ASSERT(rc);

    // Create the zoom level slider text
    QLabel* pZoomSliderText = new QLabel(AC_STR_DataViewGridZoom);


    // Add it to the sizer
    m_pMainLayout->addWidget(pZoomSliderText, 1, 1, 1, 1, Qt::AlignRight);
    m_pMainLayout->addWidget(m_pGridZoomSlider, 1, 2, 1, 1);
    m_pMainLayout->setColumnStretch(0, 20);
    m_pMainLayout->setColumnStretch(1, 10);

    m_pTablesNotebook = new acTabWidget;


    m_pTablesNotebook->setTabsClosable(false);

    m_pMainLayout->addWidget(m_pTablesNotebook, 3, 0, 1, 3);

    // Connect the page changed event:
    rc = connect(m_pTablesNotebook, SIGNAL(currentChanged(int)), this, SLOT(onTabWidgetCurrentPageChanged(int)));
    GT_ASSERT(rc);

    // Create notebook page holder pages and add them to the table:
    m_pTablesNotebook->blockSignals(true);

    for (int i = 0 ; i < (int)notebookPagesNames.size() ; i++)
    {
        // Create the grid for this
        acVirtualListCtrl* pCurrentGrid = new acVirtualListCtrl(this, NULL, false);


        m_dataGrids.push_back(pCurrentGrid);

        pCurrentGrid->setMouseTracking(true);

        // Set the grid bg color:
        QPalette p = pCurrentGrid->palette();
        p.setColor(pCurrentGrid->backgroundRole(), Qt::white);
        p.setColor(QPalette::Base, Qt::white);
        pCurrentGrid->setAutoFillBackground(true);
        pCurrentGrid->setPalette(p);

        // Connect the grid to the mouse move event:
        rc = connect(pCurrentGrid, SIGNAL(entered(const QModelIndex&)), this, SLOT(onGridItemHovered(const QModelIndex&)));
        GT_ASSERT(rc);

        rc = connect(pCurrentGrid, SIGNAL(itemHovered(const QModelIndex&)), this, SLOT(onGridItemHovered(const QModelIndex&)));
        GT_ASSERT(rc);

        // Connect the grid to the mouse click and double click slots:
        rc = connect(pCurrentGrid, SIGNAL(clicked(const QModelIndex&)), this, SLOT(onGridItemClicked(const QModelIndex&)));
        GT_ASSERT(rc);

        rc = connect(pCurrentGrid, SIGNAL(pressed(const QModelIndex&)), this, SLOT(onGridItemPressed(const QModelIndex&)));
        GT_ASSERT(rc);

        rc = connect(pCurrentGrid, SIGNAL(activated(const QModelIndex&)), this, SLOT(onGridItemDoubleClicked(const QModelIndex&)));
        GT_ASSERT(rc);

        rc = connect(pCurrentGrid->verticalScrollBar(), SIGNAL(valueChanged(int)), this, SLOT(onScroll()));
        GT_ASSERT(rc);

        rc = connect(pCurrentGrid->horizontalScrollBar(), SIGNAL(valueChanged(int)), this, SLOT(onScroll()));
        GT_ASSERT(rc);

        pCurrentGrid->resize(pCurrentGrid->size());

        m_defaultGridFontSize = pZoomSliderText->font().pointSize();

        // Initialize the font default size:
        QFont font = pCurrentGrid->font();
        font.setPointSize(m_defaultGridFontSize);
        pCurrentGrid->setFont(font);

        // Add the page to the notebook:
        m_pTablesNotebook->addTab(pCurrentGrid, notebookPagesNames[i]);

        pCurrentGrid->setVisible(true);

    }

    m_pTablesNotebook->blockSignals(false);
    m_pTablesNotebook->setCurrentIndex(0);

    acVirtualListCtrl* pCurrentGrid = currentDataGrid();

    // If grid exists:
    GT_IF_WITH_ASSERT(pCurrentGrid != NULL)
    {
        // Word wrap is enabled by default in QTableView, this must be disabled to ensure
        // resize of cells when container resizes
        pCurrentGrid->setWordWrap(false);

        // Default Text Elide Property is ElideRight this must be modified to None to avoid "..."
        pCurrentGrid->setTextElideMode(Qt::ElideNone);

        pCurrentGrid->setHorizontalScrollMode(QAbstractItemView::ScrollPerItem);
        pCurrentGrid->setVerticalScrollMode(QAbstractItemView::ScrollPerItem);

        //This ensures the cell width resizes as per content
        pCurrentGrid->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);

        //This ensures the cell height resizes as per content
        pCurrentGrid->verticalHeader()->setSectionResizeMode(QHeaderView::Interactive);

    }

    // Initialize the grid context menu:
    extendGridContextMenu();

    // Adjust grid settings (fonts, colors, etc...)
    adjustDataGridLayout();

    // Set the background color:
    QColor bgColor = acGetSystemDefaultBackgroundColor();
    QPalette p = palette();
    p.setColor(backgroundRole(), bgColor);
    p.setColor(QPalette::Base, bgColor);
    setAutoFillBackground(true);
    setPalette(p);

    // Set the sizer and fit it
    setLayout(m_pMainLayout);
}

// ---------------------------------------------------------------------------
// Name:        acDataView::getCellBestFit
// Description: A smart calculation of the cell size (Width and Height)
//              without looping through all the cells. This function
//              calculates the size according to the expected cell data.
// Arguments:   minWidth - Output cell minimum width
//              minHeight - Output cell minimum height
// Author:      Eran Zinman
// Date:        13/8/2007
// ---------------------------------------------------------------------------
void acDataView::getCellBestFit(int& minWidth, int& minHeight, oaDataType dataType)
{
    // Reset variables
    minHeight = 0;
    minWidth = 0;

    // Check if hexadecimal display is on:
    bool showHexadecimalValues = acDataViewItem::shouldShowHexadecimalValues();

    // Determine the largest cell possible value
    QString strLargestValue = "MMMMMMMMMM";

    switch (dataType)
    {
        case OA_FLOAT:
        case OA_DOUBLE:
        {
            // For FLOAT we have a digit, point, and another maximum 9 digits. Use wide digits like "8"
            strLargestValue = "8.888888888";
        }
        break;

        case OA_BYTE:
        case OA_UNSIGNED_BYTE:
        case OA_UNSIGNED_BYTE_3_3_2:
        case OA_UNSIGNED_BYTE_2_3_3_REV:
        case OA_UNSIGNED_CHAR:
        {
            if (showHexadecimalValues)
            {
                strLargestValue = "0x88";
            }
            else
            {
                // For BYTE we have 3 digits (highest number is 256). Use wide digits like 8
                strLargestValue = "888";
            }
        }
        break;

        case OA_CHAR:
        {
            if (showHexadecimalValues)
            {
                strLargestValue = "0x88";
            }
            else
            {
                strLargestValue = "8888";
            }
        }
        break;

        case OA_SHORT:
        case OA_UNSIGNED_SHORT:
        case OA_UNSIGNED_SHORT_5_6_5:
        case OA_UNSIGNED_SHORT_5_6_5_REV:
        case OA_UNSIGNED_SHORT_4_4_4_4:
        case OA_UNSIGNED_SHORT_4_4_4_4_REV:
        case OA_UNSIGNED_SHORT_5_5_5_1:
        case OA_UNSIGNED_SHORT_1_5_5_5_REV:
        {
            if (showHexadecimalValues)
            {
                strLargestValue = "0x8888";
            }
            else
            {
                // For SHORT we have 5 digits (highest number is 32767). Use wide digits like 8
                strLargestValue = "88888";
            }
        }
        break;

        case OA_INT:
        case OA_UNSIGNED_INT:
        case OA_UNSIGNED_INT_8_8_8_8:
        case OA_UNSIGNED_INT_8_8_8_8_REV:
        case OA_UNSIGNED_INT_10_10_10_2:
        case OA_UNSIGNED_INT_2_10_10_10_REV:
        {
            if (showHexadecimalValues)
            {
                strLargestValue = "0x88888888";
            }
            else
            {
                // For INT we have 10 digits (highest number is 2147483647). Use wide digits like 8
                strLargestValue = "8888888888";
            }
        }
        break;

        case OA_LONG:
        case OA_UNSIGNED_LONG:
        {
            if (showHexadecimalValues)
            {
                strLargestValue = "0x8888888888888888";
            }
            else
            {
                strLargestValue = "88888888888888888888";
            }
        }
        break;

        default:
        {
            GT_ASSERT_EX(false, L"Unknown data type!");
        }
        break;
    }

    // Add some extra "spacing"; on Linux and Mac we have bigger fonts - use some more spacing
#if AMDT_BUILD_TARGET == AMDT_LINUX_OS
    {
        // Add two more wide letters (like 'M') to the string for some extra spacing
        strLargestValue.append("MM");
    }
#else
    {
        // Add one more wide letter (like 'M') to the string for some extra spacing
        strLargestValue.append("M");
    }
#endif

    // Get the current grid object:
    acVirtualListCtrl* pCurrentGrid = currentDataGrid();
    GT_IF_WITH_ASSERT(pCurrentGrid != NULL)
    {
        // Calculate the necessary text size:
        QFont font = pCurrentGrid->font();
        QRect fontBoundingRect = QFontMetrics(font).boundingRect(strLargestValue);
        minWidth = fontBoundingRect.width();
        minHeight = fontBoundingRect.height();

        // Add margins:
        minWidth += AC_DATA_VIEW_CELL_HMARGIN;
        minHeight += AC_DATA_VIEW_CELL_VMARGIN;
    }
}



// ---------------------------------------------------------------------------
// Name:        acDataView::getCellBestFit
// Description: A smart calculation of the cell size (Width and Height)
//              without looping through all the cells. This function
//              calculates the size according to the expected cell data.
// Arguments:   minWidth - Output cell minimum width
//              minHeight - Output cell minimum height
// Author:      Sigal Algranaty
// Date:        13/8/2007
// ---------------------------------------------------------------------------
void acDataView::getCellBestFit(int& minWidth, int& minHeight)
{
    // Reset variables
    minHeight = 0;
    minWidth = 0;

    // Get the current grid object:
    acVirtualListCtrl* pCurrentGrid = currentDataGrid();
    GT_IF_WITH_ASSERT(pCurrentGrid != NULL)
    {
        // If the grid isn't empty:
        if (m_rawDataItem.size() > 0)
        {
            // If first raw data item exist:
            GT_IF_WITH_ASSERT(m_rawDataItem[0] != NULL)
            {
                // Get number of columns:
                int largestNumber = gridColumnCount();

                // Determine what is the largest possible label width
                QString labelStr;
                labelStr.append(largestNumber);

                // Calculate the largest possible label width (in pixels)
                int minLabelWidth = 0, minLabelHeight = 0;

                // Calculate the necessary text size:
                QFont font = pCurrentGrid->font();
                QRect fontBoundingRect = QFontMetrics(font).boundingRect(labelStr);
                minLabelWidth = fontBoundingRect.width();
                minLabelHeight = fontBoundingRect.height();

                // Get the texel format:
                acRawFileHandler* pRawFileHandler = m_rawDataItem[0]->getRawDataHandler();
                GT_IF_WITH_ASSERT(pRawFileHandler != NULL)
                {
                    // Get the data format:
                    oaTexelDataFormat  texelDataFormat = pRawFileHandler->dataFormat();

                    // Define a vector of data types, for all the table data types:
                    gtVector<oaDataType> dataTypesVector;
                    bool rc1 = oaGetTexelFormatContainedDataTypes(texelDataFormat, dataTypesVector);

                    if (!rc1)
                    {
                        // Get the data type from the raw file handler;
                        oaDataType dataType = pRawFileHandler->dataType();
                        dataTypesVector.push_back(dataType);
                    }

                    int minCellWidth = 0;
                    int minCellHeight = 0;

                    // Iterate the data types, and calculate min dimension for each of them:
                    for (int i = 0; i < (int)dataTypesVector.size(); i++)
                    {
                        int currentMinWidth = 0;
                        int currentMinHeight = 0;
                        // Get the current data type:
                        oaDataType currentDataType = dataTypesVector[i];

                        // Get cell best fit for this data type:
                        getCellBestFit(currentMinWidth, currentMinHeight, currentDataType);

                        minCellWidth = max(currentMinWidth, minCellWidth);
                        minCellHeight = max(currentMinHeight, minCellHeight);
                    }

                    // Now give the best fit size the largest among the largest cell size and largest label size
                    minWidth = max(minCellWidth, minLabelWidth);
                    minHeight = max(minCellHeight, minLabelHeight);
                }
            }
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        acDataView::getGridLabelBestFit
// Description: A smart calculation of the grid label size (Width and Height)
//              without looping through all the labels. This function
//              calculates the size according to the expected label data.
// Arguments:   minWidth - Output grid label minimum width
//              minHeight - Output grid label minimum height
// Author:      Eran Zinman
// Date:        13/8/2007
// ---------------------------------------------------------------------------
void acDataView::getGridLabelBestFit(int& minLabelWidth, int& minLabelHeight)
{
    // Reset variables
    minLabelHeight = 0;
    minLabelWidth = 0;

    // Calculate the row label width (green + highest row number)
    int highestRow = gridRowCount();

    // Get the longest label (channel) text
    gtString strLargestRowLabel;

    // Get the current grid object:
    acVirtualListCtrl* pCurrentGrid = currentDataGrid();

    if ((pCurrentGrid != NULL) && (m_rawDataItem.size() > 0))
    {
        // If first data item exist
        GT_IF_WITH_ASSERT(m_rawDataItem[0] != NULL)
        {
            // Loop through the channels of the first data item and find the longest channel text
            int amountOfIndices = m_rawDataItem[0]->amountOfActiveChannels();

            for (int i = 0; i < amountOfIndices; i++)
            {
                // Get channel
                acRawDataChannel* pChannel = m_rawDataItem[0]->getChannel(i);
                GT_IF_WITH_ASSERT(pChannel != NULL)
                {
                    // Get channel type:
                    oaTexelDataFormat channelType = pChannel->_channelType;

                    // Get channel name:
                    // Do not assert for this function failure cause VBO formats do not have names:
                    gtString strChannel;
                    oaGetTexelDataFormatName(channelType, strChannel);

                    // Add the number of the longest row to the channel name.
                    // Also add a nice wide char to the end like "M", for some extra space
                    strChannel.appendFormattedString(L": %uM", highestRow);

                    // Get the channel name text size
                    int labelWidth = 0 , labelHeight = 0;

                    // Calculate the necessary text size:
                    QFont font = pCurrentGrid->font();
                    QString qstrChannel = QString::fromStdWString(strChannel.asCharArray());
                    QRect fontBoundingRect = QFontMetrics(font).boundingRect(qstrChannel);
                    labelWidth = fontBoundingRect.width();
                    labelHeight = fontBoundingRect.height();

                    // Did we find a largest label width or height?
                    if (labelWidth > minLabelWidth)
                    {
                        minLabelWidth = labelWidth;
                    }

                    if (labelHeight > minLabelHeight)
                    {
                        minLabelHeight = labelHeight;
                    }
                }
            }
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        acDataView::setBestFitGridCellSizes
// Description: Best fits grid cell sizes
// Author:      Eran Zinman
// Date:        29/7/2007
// ---------------------------------------------------------------------------
void acDataView::setBestFitGridCellSizes()
{
    // Get the current grid object:
    acVirtualListCtrl* pCurrentGrid = currentDataGrid();
    GT_IF_WITH_ASSERT(pCurrentGrid != NULL)
    {
        // Get cell best fit width and height:
        int minWidth = 0, minHeight = 0;
        getCellBestFit(minWidth, minHeight);

        // Get the labels best fit width and height
        int minLabelWidth, minLabelHeight;
        getGridLabelBestFit(minLabelWidth, minLabelHeight);

        // Find the currently visible sections:
        int firstVisibleVerticalSection = pCurrentGrid->verticalScrollBar()->value();
        int firstVisibleHorizontalSection = pCurrentGrid->horizontalScrollBar()->value();

        int vSectionsCount = pCurrentGrid->verticalHeader()->count();
        int hSectionsCount = pCurrentGrid->horizontalHeader()->count();

        for (int i = firstVisibleHorizontalSection; (i < firstVisibleHorizontalSection + 100) && (i < hSectionsCount); i++)
        {
            pCurrentGrid->horizontalHeader()->resizeSection(i, minWidth);
        }

        for (int i = firstVisibleVerticalSection; (i < firstVisibleVerticalSection + 100) && (i < vSectionsCount) ; i++)
        {
            pCurrentGrid->verticalHeader()->resizeSection(i, minHeight);
        }

        // Set the model best fit size:
        acDataViewGridTable* pTableGrid =  qobject_cast<acDataViewGridTable*>(pCurrentGrid->model());

        if (pTableGrid != NULL)
        {
            pTableGrid->setCellBestFitSize(QSize(minWidth, minHeight));
        }
    }
    updateOnlyVisibleCells();
}

// ---------------------------------------------------------------------------
// Name:        acDataView::loadDataIntoGridView
// Description: Loads a texture / buffer into the data view
// Arguments:   canvasItemID - The item ID in the image manager canvas.
//              pRawDataHandler - The raw data handler.
//              isImageViewShown - is the image view shown
//              doesItemContainMultiplePages - should we hide / show the tab titles
//              viewInfo - additional informatino for the data view item
// Return Val:  bool - Success / Failure
// Author:      Eran Zinman
// Date:        27/7/2007
// ---------------------------------------------------------------------------
bool acDataView::loadDataIntoGridView(int canvasItemID, acRawFileHandler* pRawDataHandler, bool isImageViewShown,
                                      bool doesItemContainMultiplePages, const acDataViewItem::acDataViewInfo& viewInfo)
{
    bool retVal = false;

    // If raw data handler is ok
    GT_IF_WITH_ASSERT(pRawDataHandler != NULL)
    {
        // Get the window for the current item:
        int currentAddedPage = ++m_lastPageAddedIndex;
        GT_IF_WITH_ASSERT((currentAddedPage >= 0) && (currentAddedPage < (int)m_dataGrids.size()))
        {
            // Create a window for tabs that will hold the data grid
            acVirtualListCtrl* pGrid = m_dataGrids[currentAddedPage];
            GT_IF_WITH_ASSERT(pGrid != NULL)
            {
                // Create the data view item. Make the dummy window his parent:
                acDataViewItem* pDataViewItem = new acDataViewItem(pGrid, canvasItemID, pRawDataHandler, viewInfo);


                GT_IF_WITH_ASSERT(pDataViewItem->isOk())
                {
                    // Show we show normalized values?
                    GT_IF_WITH_ASSERT(m_pNormalizeValuesCheckBox != NULL)
                    {
                        bool enable = (m_pNormalizeValuesCheckBox->checkState() == Qt::Checked);
                        pDataViewItem->enableNormalizedValues(enable);
                    }

                    // Get the current grid object:
                    acVirtualListCtrl* pCurrentGrid = currentDataGrid();
                    GT_IF_WITH_ASSERT((pCurrentGrid != NULL) && (m_pTablesNotebook != NULL))
                    {
                        // If there is only one page to display, hide the tabs:
                        m_pTablesNotebook->setTabBarVisible(doesItemContainMultiplePages);

                        // Hide / Show each of the pages:
                        for (int i = 1; i < (int)m_dataGrids.size(); i++)
                        {
                            QWidget* pCurrentWindow = m_dataGrids[i];

                            if (pCurrentWindow != NULL)
                            {
                                pCurrentWindow->setVisible(false);
                            }
                        }

                        // Update the table:
                        m_pTablesNotebook->update();

                        // Save the new item into the raw data items data vector:
                        m_rawDataItem.push_back(pDataViewItem);

                        // Hide / Show the checkboxes:
                        m_pLinkCheckBox->setVisible(isImageViewShown);
                        m_pNormalizeValuesCheckBox->setVisible(isImageViewShown);

                        layout();
                        retVal = true;
                    }
                }
            }
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        acDataView::getTableName
// Description: Return a tables name, according to a given canvas ID
// Arguments:   canvasItemID - The data item canvas item ID
//              tableName - Output table name
// Return Val:  bool - Success / Failure
// Author:      Eran Zinman
// Date:        12/2/2008
// ---------------------------------------------------------------------------
bool acDataView::getTableName(int canvasItemID, gtString& tableName)
{
    bool retVal = true;

    // Get the data view item object
    acDataViewItem* pDataViewItem = getDataItemByCanvasID(canvasItemID);
    GT_IF_WITH_ASSERT(pDataViewItem != NULL)
    {
        // Get the data item parent object
        QWidget* pParent = pDataViewItem->parent();
        GT_IF_WITH_ASSERT(pParent != NULL)
        {
            // Get the parent index in the notebook
            int pageIndex = m_pTablesNotebook->indexOf(pParent);

            if (pageIndex >= 0)
            {
                // Get table name
                tableName.fromASCIIString(m_pTablesNotebook->tabText(pageIndex).toLatin1().data());

                retVal = true;
            }
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        acDataView::setActiveTable
// Description: Sets the data item with the canvasItemID the currently
//              active table
// Arguments:   canvasItemID - The data item canvas item ID
// Return Val:  bool - Success / Failure
// Author:      Eran Zinman
// Date:        31/7/2007
// ---------------------------------------------------------------------------
bool acDataView::setActiveTable(acImageItemID canvasItemID)
{
    bool retVal = true;

    // Do we need to change?
    if (canvasItemID != m_activeTableCanvasID)
    {
        retVal = false;

        // Get the data view item object
        acDataViewItem* pDataViewItem = getDataItemByCanvasID(canvasItemID);
        GT_IF_WITH_ASSERT(pDataViewItem != NULL)
        {
            // Get the item grid table
            acDataViewGridTable* pGridTable = pDataViewItem->gridTable();
            GT_IF_WITH_ASSERT(pGridTable != NULL)
            {
                // Get the current grid object:
                acVirtualListCtrl* pCurrentGrid = currentDataGrid();
                GT_IF_WITH_ASSERT(pCurrentGrid != NULL)
                {
                    // Set the table as the active table
                    pCurrentGrid->setModel(pGridTable);

                    bool rcConnect = connect(pGridTable, SIGNAL(rowsInserted(const QModelIndex&, int, int)), pCurrentGrid->verticalHeader(), SLOT(doItemsLayout()));
                    GT_ASSERT(rcConnect);
                    rcConnect = connect(pGridTable, SIGNAL(rowsInserted(const QModelIndex&, int, int)), pCurrentGrid->horizontalHeader(), SLOT(doItemsLayout()));
                    GT_ASSERT(rcConnect);

                    // Auto size the grid
                    setBestFitGridCellSizes();

                    pCurrentGrid->verticalScrollBar()->update();
                    pCurrentGrid->horizontalScrollBar()->update();

                    // Save the current active data item
                    m_activeTableCanvasID = canvasItemID;

                    // Get the data item parent object
                    QWidget* pParent = pDataViewItem->parent();
                    GT_IF_WITH_ASSERT(pParent != NULL)
                    {
                        // Get the parent index in the notebook
                        int pageIndex = m_pTablesNotebook->indexOf(pParent);

                        if (pageIndex >= 0)
                        {
                            // Select the page
                            if (m_pTablesNotebook != NULL)
                            {
                                if (m_pTablesNotebook->currentIndex() != pageIndex)
                                {
                                    // Set selection:
                                    m_pTablesNotebook->setCurrentIndex(pageIndex);
                                }

                                retVal = true;
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
// Name:        acDataView::clearGrid
// Description: Clears the grid and releases all it's data from the memory
// Author:      Eran Zinman
// Date:        31/7/2007
// ---------------------------------------------------------------------------
void acDataView::clearGrid()
{
    // Get the current grid object:
    acVirtualListCtrl* pCurrentGrid = currentDataGrid();
    GT_IF_WITH_ASSERT(pCurrentGrid != NULL)
    {
        // Set the current grid table to be null, and temporarily re-parent the data grid to the view:
        pCurrentGrid->setModel(NULL);
    }

    // Reset the last added page index:
    m_lastPageAddedIndex = -1;

    // Release all the data from the data view items vector
    m_rawDataItem.deleteElementsAndClear();

    // Flag that we are outside of the grid
    m_lastGridPixelPosition = AC_DATA_VIEW_PIXEL_POSITION_NOT_IN_GRID;

    // Reset the currently active object
    m_activeTableCanvasID = AC_IMAGE_MANAGER_ID_NONE;
}



// ---------------------------------------------------------------------------
// Name:        acDataView::updateDataItem
// Description: This function is called when the raw file handler attributes are
//              changed.
// Return Val: bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        21/4/2009
// ---------------------------------------------------------------------------
bool acDataView::updateDataItem()
{
    bool retVal = false;

    // Get the first data item:
    // Get amount of data view items
    int amountOfIndices = m_rawDataItem.size();

    // Make sure we have at least one item loaded:
    if (amountOfIndices > 0)
    {
        // Get first item:
        acDataViewItem* pDataItem = m_rawDataItem[0];
        GT_IF_WITH_ASSERT(pDataItem != NULL)
        {
            // Update the items' attributes:
            pDataItem->updateAttributes();
        }

        // Update the data table:
        updateDataTable(pDataItem);

        // Auto size the grid
        setBestFitGridCellSizes();

    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        acDataView::onEdit_SelectAll
// Description: Select all the cells in the grid
// Author:      Sigal Algranaty
// Date:        9/9/2009
// ---------------------------------------------------------------------------
void acDataView::onEdit_SelectAll()
{
    // Get the current grid object:
    acVirtualListCtrl* pCurrentGrid = currentDataGrid();
    GT_IF_WITH_ASSERT(pCurrentGrid != NULL)
    {
        // Select all cells in data grid:
        pCurrentGrid->selectAll();
    }
}

// ---------------------------------------------------------------------------
// Name:        acDataView::onEdit_Copy
// Description: Copy text from the grid selected cells to the  clipboard
// Author:      Sigal Algranaty
// Date:        9/9/2009
// ---------------------------------------------------------------------------
void acDataView::onEdit_Copy()
{
    // Get the current grid object:
    acVirtualListCtrl* pCurrentGrid = currentDataGrid();
    GT_IF_WITH_ASSERT(pCurrentGrid != NULL)
    {
        // Get the grid selected cells:
        int amountOfCols = gridColumnCount();
        int amountOfRows = gridRowCount();

        // The string contain the copied data:
        gtASCIIString copyText;

        // Iterate grid items and add the selected items to the copy string:
        for (int row = 0; row < amountOfRows; row++)
        {
            bool areItemsSelectedInRow = false;

            for (int col = 0; col < amountOfCols; col++)
            {
                bool isCellSelected = pCurrentGrid->isItemSelected(row, col);

                if (isCellSelected)
                {
                    // Get value for this selected item:
                    QModelIndex index = pCurrentGrid->model()->index(row, col);
                    QVariant selectedValue = pCurrentGrid->model()->data(index);
                    QString str = selectedValue.toString();
                    copyText.appendFormattedString("%s, ", str.toLatin1().data());
                    areItemsSelectedInRow = true;
                }
            }

            if (areItemsSelectedInRow)
            {
                copyText.append("\n");
            }
        }

        // if nothing was selected copy text is cursor position:
        if (copyText.isEmpty())
        {
            // Get the cursor position:
            QPoint cursorPos = pCurrentGrid->cursor().pos();
            QModelIndex index = pCurrentGrid->indexAt(cursorPos);

            if (index.isValid())
            {
                copyText = pCurrentGrid->model()->data(index).toString().toLatin1().data();
            }
        }

        // If there are items copied:
        if (copyText.length() > 0)
        {
            // Get the clipboard single instance:
            QClipboard* pTheClipboard = qApp->clipboard();
            GT_IF_WITH_ASSERT(pTheClipboard != NULL)
            {
                pTheClipboard->setText(copyText.asCharArray());
            }
        }
    }
}


// ---------------------------------------------------------------------------
// Name:        acDataView::areItemsSelected
// Description: Are items currently selected?
// Return Val: bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        9/9/2009
// ---------------------------------------------------------------------------
bool acDataView::areItemsSelected()
{
    bool retVal = false;
    // Get the current grid object:
    acVirtualListCtrl* pCurrentGrid = currentDataGrid();

    if (pCurrentGrid != NULL)
    {
        retVal = !pCurrentGrid->selectedIndexes().isEmpty();
    }

    return retVal;
}



// ---------------------------------------------------------------------------
// Name:        acDataView::applyImageActions
// Description: Apply a filter of actions
// Arguments:   unsigned int actions - bitwise made of acImageItemAction's
// Return Val:  void
// Author:      Sigal Algranaty
// Date:        22/4/2010
// ---------------------------------------------------------------------------
void acDataView::applyImageActions(unsigned int actions)
{
    //.Check if a channel is checked for each of the channels:
    bool isRedChannelChecked = ((actions & AC_IMAGE_CHANNEL_RED) != 0);
    bool isGreenChannelChecked = ((actions & AC_IMAGE_CHANNEL_GREEN) != 0);
    bool isBlueChannelChecked = ((actions & AC_IMAGE_CHANNEL_BLUE) != 0);
    bool isAlphaChannelChecked = ((actions & AC_IMAGE_CHANNEL_ALPHA) != 0);

    // Enable / Disable each of the channels:
    enableChannel(OA_TEXEL_FORMAT_RED, isRedChannelChecked);
    enableChannel(OA_TEXEL_FORMAT_GREEN, isGreenChannelChecked);
    enableChannel(OA_TEXEL_FORMAT_BLUE, isBlueChannelChecked);
    enableChannel(OA_TEXEL_FORMAT_ALPHA, isAlphaChannelChecked);
    // Highlight pixel position if there is at least one channel:
    // obtain a pointer to acDataViewItem and check number of active channels
    acDataViewItem* pDataViewItem = getDataItemByCanvasID(m_activeTableCanvasID);

    if (NULL != pDataViewItem && m_lastSelectedGridPixelPosition != AC_DATA_VIEW_PIXEL_POSITION_NOT_IN_GRID)
    {
        int activeChannels = pDataViewItem->amountOfActiveChannels();

        if (activeChannels > 0)
        {
            highlightPixelPosition(m_activeTableCanvasID, m_lastSelectedGridPixelPosition);
        }
    }
}


// ---------------------------------------------------------------------------
// Name:        acDataView::setHexDisplayMode
// Description: Sets the view hexadecimal display mode
// Arguments:   bool hexMode
// Author:      Gilad Yarnitzky
// Date:        26/5/2011
// ---------------------------------------------------------------------------
void acDataView::setHexDisplayMode(bool hexMode)
{
    // Set the checkbox state:
    if (m_pShowHexValuesCheckBox != NULL)
    {
        m_pShowHexValuesCheckBox->setChecked(hexMode);
    }

    // Set the static flag for the hex display:
    acDataViewItem::showHexadecimalValues(hexMode);

    // If there are items in the grid update the grid
    if (m_rawDataItem.size() > 0)
    {
        // Auto size the grid
        setBestFitGridCellSizes();

        // Refresh view
        update();
    }
}


// ---------------------------------------------------------------------------
// Name:        acDataView::enableHexCheckbox
// Description: Enable / disable hexadecimal check box
// Arguments:   bool enable
// Author:      Sigal Algranaty
// Date:        16/9/2011
// ---------------------------------------------------------------------------
void acDataView::enableHexCheckbox(bool enable)
{
    // Sanity check:
    GT_IF_WITH_ASSERT(m_pShowHexValuesCheckBox != NULL)
    {
        m_pShowHexValuesCheckBox->setEnabled(enable);
    }
}

// ---------------------------------------------------------------------------
// Name:        acDataView::updateGrid
// Description: Update the grid view
// Author:      Sigal Algranaty
// Date:        15/11/2011
// ---------------------------------------------------------------------------
void acDataView::updateGrid()
{
    // Get the current grid object:
    acVirtualListCtrl* pCurrentGrid = currentDataGrid();
    GT_IF_WITH_ASSERT(pCurrentGrid != NULL)
    {
        pCurrentGrid->update();
    }
}

// ---------------------------------------------------------------------------
// Name:        acDataView::extendGridContextMenu
// Description: Extend the grid context menu
// Author:      Sigal Algranaty
// Date:        10/7/2012
// ---------------------------------------------------------------------------
void acDataView::extendGridContextMenu()
{
    for (int i = 0; i < (int)m_dataGrids.size(); i++)
    {
        // Get the current grid:
        acVirtualListCtrl* pCurrentGrid = m_dataGrids[i];
        QMenu* pMenu = pCurrentGrid->contextMenu();
        GT_IF_WITH_ASSERT(pMenu != NULL)
        {
            pMenu->addSeparator();
            // Insert Show Hex action:
            m_pShowHexAction = pMenu->addAction(AC_STR_Hexadecimaldisplay);
            GT_ASSERT(m_pShowHexAction != NULL);
            m_pShowHexAction->setCheckable(true);

            // NOTICE: For some reason, if the action's checked and enable state are not initialized here, afterwards it can't be initialized.
            m_pShowHexAction->setEnabled(true);
            m_pShowHexAction->setChecked(false);

            bool rcConnect = connect(m_pShowHexAction, SIGNAL(triggered(bool)), this, SLOT(onShowHexValues(bool)));
            GT_ASSERT(rcConnect);

            // Connect the menu to its slots:
            rcConnect = connect(pMenu, SIGNAL(aboutToShow()), this, SLOT(onAboutToShowContextMenu()));
            GT_ASSERT(rcConnect);
        }
    }
}


// ---------------------------------------------------------------------------
// Name:        acDataView::gridRowCount
// Description: Returns the grid row count
// Return Val:  int
// Author:      Sigal Algranaty
// Date:        11/6/2012
// ---------------------------------------------------------------------------
int acDataView::gridRowCount()
{
    int retVal = 0;
    // Get the current grid object:
    acVirtualListCtrl* pCurrentGrid = currentDataGrid();

    if (pCurrentGrid != NULL)
    {
        if (pCurrentGrid->model() != NULL)
        {
            retVal = pCurrentGrid->model()->rowCount();
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        acDataView::gridColumnCount
// Description: Returns the grid column count
// Return Val:  int
// Author:      Sigal Algranaty
// Date:        11/6/2012
// ---------------------------------------------------------------------------
int acDataView::gridColumnCount()
{
    int retVal = 0;
    // Get the current grid object:
    acVirtualListCtrl* pCurrentGrid = currentDataGrid();

    if (pCurrentGrid != NULL)
    {
        if (pCurrentGrid->model() != NULL)
        {
            retVal = pCurrentGrid->model()->columnCount();
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        acDataView::onGridItemClicked
// Description: IS called when a grid item was clicked
// Arguments:   const QModelIndex& clickedItem
// Author:      Sigal Algranaty
// Date:        13/6/2012
// ---------------------------------------------------------------------------
void acDataView::onGridItemClicked(const QModelIndex& clickedItem)
{
    QPoint gridPoint(clickedItem.column(), clickedItem.row());

    // Save the last grid pixel position by converting grid position to pixel position:
    m_lastGridPixelPosition = gridPositionToPixelPosition(gridPoint);

    m_lastSelectedGridPixelPosition = m_lastGridPixelPosition;
    // Get the texel format:
    acRawFileHandler* pRawFileHandler = m_rawDataItem[0]->getRawDataHandler();
    GT_IF_WITH_ASSERT(pRawFileHandler != NULL)
    {
        // Get the data format:
        oaTexelDataFormat  texelDataFormat = pRawFileHandler->dataFormat();
        // don't select all active channels for non rgb data
        bool selectDataChannels = !oaIsBufferTexelFormat(texelDataFormat);

        if (selectDataChannels)
        {
            acDataViewItem* pDataViewItem = getDataItemByCanvasID(m_activeTableCanvasID);
            GT_IF_WITH_ASSERT(pDataViewItem != NULL)
            {
                int activeChannels = pDataViewItem->amountOfActiveChannels();

                if (activeChannels > 1)
                {
                    // Get the current grid object:
                    acVirtualListCtrl* pCurrentGrid = currentDataGrid();
                    GT_IF_WITH_ASSERT(pCurrentGrid != NULL)
                    {
                        // Convert pixel position to grid position
                        QPoint gridPos = pixelPositionToGridPosition(m_lastSelectedGridPixelPosition);
                        QAbstractItemModel* pModel = pCurrentGrid->model();

                        if (pModel != NULL)
                        {
                            // Get currently loaded data view item
                            pDataViewItem = getDataItemByCanvasID(m_activeTableCanvasID);
                            GT_IF_WITH_ASSERT(pDataViewItem != NULL)
                            {
                                // Select the pixel block:
                                QItemSelectionModel* pSelectionModel = pCurrentGrid->selectionModel();
                                GT_IF_WITH_ASSERT(NULL != pSelectionModel)
                                {
                                    QItemSelection rowSelection;
                                    QModelIndex firstIndex = pModel->index(gridPos.y(), gridPos.x());
                                    QModelIndex lastIndex = pModel->index(gridPos.y() + (activeChannels - 1), gridPos.x());
                                    rowSelection.select(firstIndex, lastIndex);
                                    pSelectionModel->select(rowSelection, QItemSelectionModel::ClearAndSelect);
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    // Send the event to my parent, letting him know mouse position have changed, and mouse was left clicked
    emit pixelPositionChanged(m_activeTableCanvasID, m_lastSelectedGridPixelPosition, true, false);
}

void acDataView::onGridItemPressed(const QModelIndex& clickedItem)
{
    QPoint gridPoint(clickedItem.column(), clickedItem.row());

    // Save the last grid pixel position by converting grid position to pixel position:
    m_lastGridPixelPosition = gridPositionToPixelPosition(gridPoint);

    m_lastSelectedGridPixelPosition = m_lastGridPixelPosition;

    // Send the event to my parent, letting him know mouse position have changed, and mouse was left clicked
    emit pixelPositionChanged(m_activeTableCanvasID, m_lastSelectedGridPixelPosition, true, false);
}

void acDataView::onSelectedItemChanged(const QItemSelection& selected, const QItemSelection& deselected)
{
    Q_UNUSED(deselected);

    const QModelIndexList& selectedIndexes = selected.indexes();

    if (!selectedIndexes.isEmpty())
    {
        QPoint gridPoint(selectedIndexes.first().column(), selectedIndexes.first().row());

        // Save the last grid pixel position by converting grid position to pixel position:
        m_lastGridPixelPosition = gridPositionToPixelPosition(gridPoint);

        m_lastSelectedGridPixelPosition = m_lastGridPixelPosition;

        // Send the event to my parent, letting him know mouse position have changed, and mouse was left clicked
        emit pixelPositionChanged(m_activeTableCanvasID, m_lastSelectedGridPixelPosition, true, false);
    }
}

void acDataView::onCurrentItemChanged(const QModelIndex& current, const QModelIndex& previous)
{
    Q_UNUSED(previous);

    QPoint gridPoint(current.column(), current.row());

    // Save the last grid pixel position by converting grid position to pixel position:
    m_lastGridPixelPosition = gridPositionToPixelPosition(gridPoint);

    // Send the event to my parent, letting him know mouse position have changed, and mouse was left clicked
    emit pixelPositionChanged(m_activeTableCanvasID, m_lastGridPixelPosition, true, false);
}

// ---------------------------------------------------------------------------
// Name:        acDataView::onGridItemDoubleClicked
// Description: IS called when a grid item was clicked
// Arguments:   const QModelIndex& clickedItem
// Author:      Sigal Algranaty
// Date:        13/6/2012
// ---------------------------------------------------------------------------
void acDataView::onGridItemDoubleClicked(const QModelIndex& clickedItem)
{
    QPoint gridPoint(clickedItem.column(), clickedItem.row());

    // Save the last grid pixel position by converting grid position to pixel position:
    m_lastGridPixelPosition = gridPositionToPixelPosition(gridPoint);
    m_lastSelectedGridPixelPosition = m_lastGridPixelPosition;

    // Send the event to my parent, letting him know mouse position have changed, and mouse was left clicked:
    emit pixelPositionChanged(m_activeTableCanvasID, m_lastSelectedGridPixelPosition, false, true);
}

// ---------------------------------------------------------------------------
// Name:        acDataView::currentDataGrid
// Description: Get the current selected data grid
// Return Val:  acVirtualListCtrl*
// Author:      Sigal Algranaty
// Date:        8/7/2012
// ---------------------------------------------------------------------------
acVirtualListCtrl* acDataView::currentDataGrid() const
{
    acVirtualListCtrl* pRetVal = NULL;

    // Sanity check:
    GT_IF_WITH_ASSERT(m_pTablesNotebook != NULL)
    {
        // Get the table current index:
        int currentIndex = m_pTablesNotebook->currentIndex();
        GT_IF_WITH_ASSERT((currentIndex >= 0) && (currentIndex < (int)m_dataGrids.size()))
        {
            pRetVal = m_dataGrids[currentIndex];
        }

    }
    return pRetVal;
}


// ---------------------------------------------------------------------------
// Name:        acDataView::onSelectAll
// Description: Select all cells in current grid
// Return Val:  acVirtualListCtrl*
// Author:      Sigal Algranaty
// Date:        10/7/2012
// ---------------------------------------------------------------------------
void acDataView::onSelectAll()
{
    acVirtualListCtrl* pCurrentGrid = currentDataGrid();
    GT_IF_WITH_ASSERT(pCurrentGrid != NULL)
    {
        pCurrentGrid->selectAll();
    }
}

// ---------------------------------------------------------------------------
// Name:        acDataView::onAboutToShowContextMenu
// Description: Is called when the context menu is about to be shown
// Author:      Sigal Algranaty
// Date:        10/7/2012
// ---------------------------------------------------------------------------
void acDataView::onAboutToShowContextMenu()
{
    // Get the current grid table:
    acVirtualListCtrl* pCurrentGrid = currentDataGrid();
    GT_IF_WITH_ASSERT(pCurrentGrid != NULL)
    {
        QMenu* pMenu = pCurrentGrid->contextMenu();
        GT_IF_WITH_ASSERT(pMenu != NULL)
        {
            bool isEnabled = false, isChecked = false;
            onUpdateShowHexValues(isEnabled, isChecked);

            if (m_pShowHexAction != NULL)
            {
                m_pShowHexAction->setEnabled(isEnabled);
                m_pShowHexAction->setChecked(isChecked);
            }

            if (m_pSelectAllAction != NULL)
            {
                isEnabled = false;

                if (pCurrentGrid->model() != NULL)
                {
                    isEnabled = (pCurrentGrid->model()->rowCount() > 0);
                }

                m_pSelectAllAction->setEnabled(isEnabled);
            }
        }
    }
}

/// -----------------------------------------------------------------------------------------------
/// \brief Name:        onScroll
/// \brief Description: Is handling the vertical and horizontal scroll
/// \return void
/// -----------------------------------------------------------------------------------------------
void acDataView::onScroll()
{
    setBestFitGridCellSizes();
}

