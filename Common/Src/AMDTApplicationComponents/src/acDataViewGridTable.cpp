//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file acDataViewGridTable.cpp
///
//==================================================================================

//------------------------------ acDataViewGridTable.cpp ------------------------------



// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>

// Local:
#include <AMDTApplicationComponents/Include/acColours.h>
#include <AMDTApplicationComponents/Include/acDataViewGridTable.h>


// ---------------------------------------------------------------------------
// Name:        acDataViewGridTable::acDataViewGridTable
// Description: Constructor - Creates the grid table
// Arguments:   pDataViewItem - The item to display in the raw data table
// Author:      Eran Zinman
// Date:        29/7/2007
// ---------------------------------------------------------------------------
acDataViewGridTable::acDataViewGridTable(acDataViewItem* pDataViewItem) :
    m_pDataViewItem(pDataViewItem), m_currentBestFitSize(-1, -1)
{

}

// ---------------------------------------------------------------------------
// Name:        acDataViewGridTable::~acDataViewGridTable
// Description: Destructor
// Author:      Eran Zinman
// Date:        29/7/2007
// ---------------------------------------------------------------------------
acDataViewGridTable::~acDataViewGridTable()
{

}

// ---------------------------------------------------------------------------
// Name:        acDataViewGridTable::GetNumberRows
// Description: Returns the number of rows in table
// Return Val:  Number of rows in table
// Author:      Eran Zinman
// Date:        30/7/2007
// ---------------------------------------------------------------------------
int acDataViewGridTable::rowCount(const QModelIndex& parent) const
{
    GT_UNREFERENCED_PARAMETER(parent);

    int rowsNum = 0;

    // Sanity check:
    GT_IF_WITH_ASSERT(m_pDataViewItem != NULL)
    {
        // Get number of active channels
        int activeChannels = m_pDataViewItem->amountOfActiveChannels();

        // Get the raw data height:
        rowsNum = m_pDataViewItem->size().height();

        // Calculate number of rows (data height * number of active channels):
        oaTexelDataFormat dataFormat = m_pDataViewItem->getRawDataHandler()->dataFormat();

        if (!oaIsBufferTexelFormat(dataFormat))
        {
            rowsNum *= activeChannels;
        }
    }

    // Return number of rows:
    return rowsNum;
}

// ---------------------------------------------------------------------------
// Name:        acDataViewGridTable::columnCount
// Description: Returns the number of columns in table
// Return Val:  Number of columns in table
// Author:      Eran Zinman
// Date:        30/7/2007
// ---------------------------------------------------------------------------
int acDataViewGridTable::columnCount(const QModelIndex& parent)const
{
    GT_UNREFERENCED_PARAMETER(parent);

    // Number of columns:
    int colsNum = 0;

    GT_IF_WITH_ASSERT(m_pDataViewItem != NULL)
    {
        // Get number of active channels:
        int activeChannels = m_pDataViewItem->amountOfActiveChannels();

        // Only show columns if we got channels enabled:
        if (activeChannels > 0)
        {
            // Calculate number of rows (data width):
            colsNum = m_pDataViewItem->size().width();
        }
    }

    // Return number of rows
    return colsNum;
}


// ---------------------------------------------------------------------------
// Name:        acDataViewGridTable::GetValue
// Description: Returns the value at a specific location
// Arguments:   Cell row and column
// Return Val:  Cell value as wxString
// Author:      Eran Zinman
// Date:        30/7/2007
// ---------------------------------------------------------------------------
QVariant acDataViewGridTable::data(const QModelIndex& index, int role)const
{
    QVariant retVal;

    if (index.isValid())
    {
        if ((role == Qt::DisplayRole) || (role == Qt::ToolTipRole))
        {
            int row = index.row();
            int col = index.column();

            QString outputStr;

            // Sanity check:
            GT_IF_WITH_ASSERT(m_pDataViewItem != NULL)
            {
                // If we display the data from bottom to top:
                if (oaIsTypeDisplayedUpsideDown(m_pDataViewItem->dataFormat()))
                {
                    // openGL origin (0, 0) is bottom left so flip the vertical row position:
                    row = rowCount() - row - 1;
                }

                // Get channel value at grid position:
                GT_IF_WITH_ASSERT(m_pDataViewItem != NULL)
                {
                    gtString retValAsGTString;
                    bool rc = m_pDataViewItem->getGridCellData(row, col, retValAsGTString);
                    GT_IF_WITH_ASSERT(rc)
                    {
                        outputStr = QString::fromStdWString(retValAsGTString.asCharArray());
                    }
                }
            }
            retVal = outputStr;
        }
        else if (role == Qt::BackgroundColorRole)
        {
            retVal = acGRAY_BG_COLOR;

            GT_IF_WITH_ASSERT(m_pDataViewItem != NULL)
            {
                int amountOfActiveChannels = m_pDataViewItem->m_amountOfActiveChannels;
                bool isBuffer = oaIsBufferTexelFormat(m_pDataViewItem->dataFormat());
                // Check if the current item is odd or even:

                bool isYEven = true, isXEven = true;

                if (m_pDataViewItem->m_viewDataInfo._yCoordGroupSize > 0 && amountOfActiveChannels != 0)
                {
                    // Calculate pixel Y position:
                    int pixelRow = -1;

                    if (isBuffer)
                    {
                        pixelRow = index.row();
                    }
                    else
                    {
                        pixelRow = index.row() / amountOfActiveChannels;
                    }

                    // If it's an even position
                    if ((pixelRow % 2) == 0)
                    {
                        isYEven = true;
                    }
                    else
                    {
                        isYEven = false;
                    }
                }

                if (m_pDataViewItem->m_viewDataInfo._xCoordGroupSize > 0 && amountOfActiveChannels != 0)
                {
                    // Calculate pixel X position:
                    int pixelCol = -1;

                    if (isBuffer)
                    {
                        pixelCol = index.column();
                    }
                    else
                    {
                        pixelCol = index.column() / amountOfActiveChannels;
                    }

                    // If it's an even position
                    if ((pixelCol % 2) == 0)
                    {
                        isXEven = true;
                    }
                    else
                    {
                        isXEven = false;
                    }
                }

                bool isEven = (isYEven != isXEven);

                if (isEven)
                {
                    retVal = QColor(255, 255, 203, 0xff);
                }
                else
                {
                    retVal = QColor(Qt::white);
                }
            }
        }
        else if (role == Qt::TextAlignmentRole)
        {
            retVal = Qt::AlignCenter;
        }
        else if (role == Qt::SizeHintRole)
        {
            retVal = m_currentBestFitSize;
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        acDataViewGridTable::headerData
// Description: Return the table headers data
// Arguments:   int section
//              Qt::Orientation orientation
//              int role
// Return Val:  QVariant
// Author:      Sigal Algranaty
// Date:        9/5/2012
// ---------------------------------------------------------------------------
QVariant acDataViewGridTable::headerData(int section, Qt::Orientation orientation, int role) const
{
    QVariant retVal;

    if (role == Qt::DisplayRole)
    {
        QString label;

        // Get the column title from the config:
        if (orientation == Qt::Horizontal)
        {
            gtString gtlabel = getColumnText(section).asCharArray();
            label = QString::fromWCharArray(gtlabel.asCharArray(), gtlabel.length());
            retVal = label;

        }
        else if (orientation == Qt::Vertical)
        {
            if (section < rowCount())
            {
                gtString gtlabel = getRowText(section).asCharArray();
                label = QString::fromWCharArray(gtlabel.asCharArray(), gtlabel.length());
                retVal = label;
            }
        }

        else if (role == Qt::SizeHintRole)
        {
            retVal = QHeaderView::ResizeToContents;
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        acDataViewGridTable::GetColLabelValue
// Description: Returns the column label for a specific row
// Arguments:   Cell column
// Return Val:  Column label
// Author:      Eran Zinman
// Date:        30/7/2007
// ---------------------------------------------------------------------------
gtString acDataViewGridTable::getRowText(int row) const
{
    gtString retVal;

    // Sanity check:
    GT_IF_WITH_ASSERT(m_pDataViewItem != NULL)
    {
        // Get number of active channels
        int activeChannels = m_pDataViewItem->amountOfActiveChannels();
        GT_IF_WITH_ASSERT(m_pDataViewItem->getRawDataHandler() != NULL)
        {
            oaTexelDataFormat dataFormat = m_pDataViewItem->getRawDataHandler()->dataFormat();

            if (oaIsBufferTexelFormat(dataFormat))
            {
                retVal.appendFormattedString(L"%u", row);
            }
            else
            {
                // Get channel index
                int channelIndex = 0;

                if (activeChannels > 0)
                {
                    channelIndex = (row % activeChannels);
                }

                // Add the row offset:
                row += m_pDataViewItem->yCoordinateOffset();

                if (oaIsTypeDisplayedUpsideDown(dataFormat))
                {
                    // openGL origin (0, 0) is bottom left so flip the vertical row position
                    row = rowCount() - row - 1;
                }

                // Generate channel text - Find the "channelIndex" channel number *only* from the active channels
                int realChannelIndex = m_pDataViewItem->getEnabledChannelIndex(channelIndex);

                // Get raw data channel
                acRawDataChannel* pRawDataChannel = m_pDataViewItem->getChannel(realChannelIndex);
                GT_IF_WITH_ASSERT(pRawDataChannel != NULL)
                {
                    // Get channel type
                    oaTexelDataFormat channelType = pRawDataChannel->_channelType;

                    // Get channel name
                    gtString strChannel;
                    GT_ASSERT(oaGetTexelDataFormatName(channelType, strChannel));

                    // Calculate pixel Y position
                    int pixelHeight = row / activeChannels;

                    // Create the column label (pixelHeight - 1) + Channel Index
                    retVal.appendFormattedString(L"%ls: %u", strChannel.asCharArray(), pixelHeight);
                }
            }
        }
    }

    // Return the column label
    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        acDataViewGridTable::getColumnText
// Description: Return the text for a requested column
// Arguments:   int section
//              QVariant retVal
// Return Val:  gtString
// Author:      Sigal Algranaty
// Date:        9/5/2012
// ---------------------------------------------------------------------------
gtString acDataViewGridTable::getColumnText(int col) const
{
    gtString retVal;

    acRawFileHandler* pRawFileHandler = m_pDataViewItem->getRawDataHandler();
    GT_IF_WITH_ASSERT(pRawFileHandler != NULL)
    {
        gtString columnLabel;
        oaTexelDataFormat dataFormat = m_pDataViewItem->getRawDataHandler()->dataFormat();

        if (!oaIsBufferTexelFormat(dataFormat))
        {
            // Add the row offset:
            col += m_pDataViewItem->xCoordinateOffset();

            // Create the column label:
            retVal.appendFormattedString(L"%u", col);

            if (dataFormat == OA_TEXEL_FORMAT_VARIABLE_VALUE)
            {
                retVal.prepend(L"X: ");
            }
        }
        else
        {
            // Get the column data format:
            oaTexelDataFormat rawFileFormat = pRawFileHandler->dataFormat();

            // Get the column data texel format:
            oaTexelDataFormat currentColFormat;
            int currentColLocalIndex = 0;
            bool rc = oaGetTexelFormatBufferComponentType(rawFileFormat, col, currentColFormat, currentColLocalIndex);
            GT_IF_WITH_ASSERT(rc)
            {
                // Get channel name:
                gtString strChannel;
                rc = oaGetTexelDataBufferFormatName(currentColFormat, strChannel, currentColLocalIndex);
                GT_ASSERT(rc);

                // Create the column label (pixelHeight - 1) + Channel Index:
                retVal.appendFormattedString(L"%ls", strChannel.asCharArray());
            }
        }
    }
    return retVal;

}

// ---------------------------------------------------------------------------
// Name:        acDataViewGridTable::flags
// Description: Item flags
// Arguments:   const QModelIndex &index
// Return Val:  Qt::ItemFlags
// Author:      Sigal Algranaty
// Date:        9/5/2012
// ---------------------------------------------------------------------------
Qt::ItemFlags acDataViewGridTable::flags(const QModelIndex& index) const
{
    Qt::ItemFlags retVal = Qt::ItemIsEnabled;

    if (index.isValid())
    {
        retVal = QAbstractTableModel::flags(index);
    }

    return retVal;
}
