//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file acDataViewItem.cpp
///
//==================================================================================

//------------------------------ acDataViewItem.cpp ------------------------------

// Qt:
#include <QtWidgets>

// Infra:
#include <AMDTBaseTools/Include/gtString.h>
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTAPIClasses/Include/apParameters.h>
#include <AMDTApplicationComponents/Include/acDataViewGridTable.h>
#include <AMDTApplicationComponents/Include/acDataViewItem.h>

// Local:
#include <inc/acStringConstants.h>

// Static members initialization:
bool acDataViewItem::m_sShowHexValues = false;

// ---------------------------------------------------------------------------
// Name:        acDataViewItem::acDataViewItem
// Description: Constructor - Creates the raw data item
// Arguments:   pParent - The parent of the raw data item
//              canvasItemID - The canvas ID connected with the item
//              pRawDataHandler - Raw data handler
// Author:      Eran Zinman
// Date:        5/8/2007
// ---------------------------------------------------------------------------
acDataViewItem::acDataViewItem(QWidget* pParent, int canvasItemID, acRawFileHandler* pRawDataHandler, const acDataViewInfo& dataInfo)
    : m_canvasItemID(canvasItemID), m_width(-1), m_height(-1), m_chunkSize(-1), m_pRawDataHandler(pRawDataHandler), m_amountOfActiveChannels(0),
      m_viewDataInfo(dataInfo), m_dataType(OA_UNSIGNED_BYTE), m_texelDataFormat(OA_TEXEL_FORMAT_UNKNOWN), m_rotationAngle(0), m_showNormalizedValues(false),
      m_pTableBase(NULL), m_pParent(pParent)
{
    GT_IF_WITH_ASSERT(pRawDataHandler != NULL)
    {
        GT_IF_WITH_ASSERT(pRawDataHandler->isOk())
        {
            // Get texel data format and data type:
            pRawDataHandler->getDataTypeAndFormat(m_texelDataFormat, m_dataType);

            // Get width and height:
            pRawDataHandler->getSize(m_width, m_height);

            // By default - set the number of active channels to be the same as the number of channels
            m_amountOfActiveChannels = oaAmountOfTexelFormatComponents(m_texelDataFormat);

            // Handle VBO differently:
            oaTexelDataFormat dataFormat = m_pRawDataHandler->dataFormat();

            if (oaIsBufferTexelFormat(dataFormat))
            {
                // Calculate VBO buffer attributes:
                calculateBufferAttributes();
            }

            // Generate the channels array:
            generateChannelsArray(m_texelDataFormat);

            // Generate the grid table:
            bool rc1 = updateDataTable();
            GT_ASSERT(rc1);
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        acDataViewItem::~acDataViewItem
// Description: Destructor
// Author:      Eran Zinman
// Date:        5/8/2007
// ---------------------------------------------------------------------------
acDataViewItem::~acDataViewItem()
{
    // Delete the table grid table from memory
    if (m_pTableBase != NULL)
    {
        delete m_pTableBase;
        m_pTableBase = NULL;
    }

    // Delete the raw data handler
    if (m_pRawDataHandler != NULL)
    {
        delete m_pRawDataHandler;
        m_pRawDataHandler = NULL;
    }

    m_channels.deleteElementsAndClear();
}

// ---------------------------------------------------------------------------
// Name:        acDataViewItem::generateChannelsArray
// Description: Generates the channels array
// Arguments:   channels - The object data channels description
// Author:      Eran Zinman
// Date:        5/8/2007
// ---------------------------------------------------------------------------
void acDataViewItem::generateChannelsArray(oaTexelDataFormat channels)
{
    // Get amount of components in the texel data format
    int amountOfComponents = oaAmountOfTexelFormatComponents(channels);
    GT_IF_WITH_ASSERT(amountOfComponents != -1)
    {
        // Loop through the data format components
        for (int i = 0; i < amountOfComponents; i++)
        {
            // Get channel type
            oaTexelDataFormat channelType = oaGetTexelFormatComponentType(channels, i);
            GT_IF_WITH_ASSERT(channelType != OA_TEXEL_FORMAT_UNKNOWN)
            {
                // Create the new channel object
                acRawDataChannel* pRawDataChannel = new acRawDataChannel;
                GT_IF_WITH_ASSERT(pRawDataChannel != NULL)
                {
                    // Set the channel type
                    pRawDataChannel->_channelType = channelType;

                    // Enable channel by default
                    pRawDataChannel->_isEnabled = true;

                    // Add the channel into the channels vector
                    m_channels.push_back(pRawDataChannel);
                }
            }
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        acDataViewItem::rotatRawDataByAngle
// Description: Rotate the raw data using a given angle. Currently supported
//              angles are only 90, 180 and 270 degrees.
// Arguments:   rotateAngle - The rotation angle.
// Return Val:  bool - Success / Failure
// Author:      Eran Zinman
// Date:        3/8/2007
// ---------------------------------------------------------------------------
bool acDataViewItem::rotateRawDataByAngle(int rotateAngle)
{
    bool retVal = false;

    // We only know how to rotate 0, 90, 180 and 270 degrees
    bool rc = ((rotateAngle == 0) || (rotateAngle == 90) || (rotateAngle == 180) || (rotateAngle == 270));
    GT_IF_WITH_ASSERT(rc)
    {
        // Save the new rotation angle
        m_rotationAngle += rotateAngle;

        // Stay in the 360deg circle
        m_rotationAngle %= 360;

        retVal = true;
    }

    // Return success / failure
    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        calculateRotationPosition
// Description: If raw data is rotated, calculate rotated (xPos, yPos) accordingly
// Arguments:   xPos, yPos - Output new xPos and yPos variables
// Author:      Eran Zinman
// Date:        12/8/2007
// ---------------------------------------------------------------------------
void acDataViewItem::calculateRotationPosition(int& xPos, int& yPos)
{
    // Convert rotation angle to int
    int rotateAngleAsInt = (int)m_rotationAngle;

    switch (rotateAngleAsInt)
    {
        case 0:
        {
            // Do nothing...
        }
        break;

        case 90:
        {
            // Save y position
            int val = yPos;

            // Change y position to be x position
            yPos = xPos;

            // Set x position to be the height - (previous y position + 1)
            xPos = m_height - (val + 1);
        }
        break;

        case 180:
        {
            // Change x position to be width - (x position + 1)
            xPos = m_width - (xPos + 1);

            // Change y position to be height - (y position + 1)
            yPos = m_height - (yPos + 1);
        }
        break;

        case 270:
        {
            // Save x position
            int val = xPos;

            // Change x position to be the previous x position
            xPos = yPos;

            // Set y position to be the width - (previous x position + 1)
            yPos = m_width - (val + 1);
        }
        break;

        default:
        {
            GT_ASSERT_EX(false, L"Unknown rotation angle!");
        }
        break;
    }
}

// ---------------------------------------------------------------------------
// Name:        acDataViewItem::getGridCellData
// Description: Get grid cell data at the grid (row, col) position
// Arguments:   row, col - Position to get cell data from
//              cellValue - Output cell value as string
// Return Val:  bool - Success / Failure
// Author:      Eran Zinman
// Date:        12/8/2007
// ---------------------------------------------------------------------------
bool acDataViewItem::getGridCellData(int row, int col, gtString& cellValue)
{
    bool retVal = false;

    // Is the value filtered?
    bool shouldValueBeDisplayed = true;

    for (int i = 0 ; i < AC_MAX_RAW_FILE_FILTER_HANDLERS; i++)
    {
        if (NULL != m_viewDataInfo._pFilterRawFileHandler[i])
        {
            apPixelValueParameter* pFilterParameter = NULL;

            int yPos = 0;

            if (m_amountOfActiveChannels > 0)
            {
                yPos = row / m_amountOfActiveChannels;
            }

            // Calculate real y position:
            int xPos = col;

            // Get the filter raw data value:
            bool isValueAvailable = false;
            bool rcFilterDataValue = m_viewDataInfo._pFilterRawFileHandler[i]->getRawDataComponentValue(xPos, yPos, 0, pFilterParameter, isValueAvailable);
            GT_IF_WITH_ASSERT(rcFilterDataValue && isValueAvailable)
            {
                // Get the filter parameter pixel value:
                GLubyte pixelValue = 0;
                bool rcGetPixelValue = m_viewDataInfo._pFilterRawFileHandler[i]->getNormalizedPixelValue(pFilterParameter, 0, pixelValue);
                GT_IF_WITH_ASSERT(rcGetPixelValue)
                {
                    shouldValueBeDisplayed = (pixelValue != 0) && shouldValueBeDisplayed;
                }
            }
        }
    }

    if (shouldValueBeDisplayed)
    {
        GT_IF_WITH_ASSERT(m_pRawDataHandler != NULL)
        {
            bool rcRawDataValue = false;
            apPixelValueParameter* pParameter = NULL;
            int realChannelIndex = 0;

            oaTexelDataFormat dataFormat = m_pRawDataHandler->dataFormat();

            if (oaIsBufferTexelFormat(dataFormat))
            {
                // Get the raw data value:
                rcRawDataValue = m_pRawDataHandler->getBufferRawDataComponentValue(row, col, pParameter);
            }
            else
            {
                // Get real channel index:
                int channelIndex = (m_amountOfActiveChannels - 1) - (row % m_amountOfActiveChannels);
                realChannelIndex = getEnabledChannelIndex(channelIndex);

                // Calculate real y position:
                int yPos = 0;

                if (m_amountOfActiveChannels > 0)
                {
                    yPos = row / m_amountOfActiveChannels;
                }

                int xPos = col;

                // If raw data is rotated, change (xPos, yPos) accordingly:
                calculateRotationPosition(xPos, yPos);

                // Get the raw data value:
                bool isValueAvailable;
                rcRawDataValue = m_pRawDataHandler->getRawDataComponentValue(xPos, yPos, realChannelIndex, pParameter, isValueAvailable);
            }

            if (rcRawDataValue)
            {
                if (pParameter)
                {
                    // Show we show normalized value or raw data value?
                    // Normalization is only valid for primary channel)
                    if (m_showNormalizedValues)
                    {
                        // Calculate normalized pixel value and convert it to string
                        GLubyte pixelValue = 0;
                        bool rc1 = m_pRawDataHandler->getNormalizedPixelValue(pParameter, realChannelIndex, pixelValue);

                        if (rc1)
                        {
                            cellValue.appendFormattedString(L"%d", pixelValue);
                        }
                    }
                    else
                    {
                        if (m_sShowHexValues)
                        {
                            pParameter->valueAsHexString(cellValue);
                        }
                        else
                        {
                            // Convert raw data value to string:
                            pParameter->valueAsString(cellValue);
                        }
                    }
                }
            }
            else
            {
                cellValue.append(AC_STR_NotAvailable);
            }

            retVal = true;
        }
    }
    else // !shouldValueBeDisplayed
    {
        retVal = true;
        cellValue = AC_STR_NotAvailable;
    }

    return retVal;
}
// ---------------------------------------------------------------------------
// Name:        acDataViewItem::setActivePage
// Description: Sets the current active page in the raw data
// Arguments:   pageIndex - The index of the page to be set active
// Return Val:  bool - Success / Failure
// Author:      Eran Zinman
// Date:        17/12/2007
// ---------------------------------------------------------------------------
bool acDataViewItem::setActivePage(int pageIndex)
{
    bool retVal = false;

    GT_IF_WITH_ASSERT(m_pRawDataHandler != NULL)
    {
        // Set active page
        retVal = m_pRawDataHandler->setActivePage(pageIndex);
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        acDataViewItem::updateDataTable
// Description: Updates the internal data table.
// Return Val:  bool - Success / Failure
// Author:      Eran Zinman
// Date:        3/8/2007
// ---------------------------------------------------------------------------
bool acDataViewItem::updateDataTable()
{
    bool retVal = false;

    // If old table exists, delete it
    if (m_pTableBase != NULL)
    {
        delete m_pTableBase;
        m_pTableBase = NULL;
    }

    // Recreate the grid table
    m_pTableBase = new acDataViewGridTable(this);


    // For VBOs, do not paint the cells backgrounds:
    GT_IF_WITH_ASSERT(m_pRawDataHandler != NULL)
    {
        oaTexelDataFormat dataFormat = m_pRawDataHandler->dataFormat();
        oaIsBufferTexelFormat(dataFormat);
    }

    retVal = true;

    // Return success / failure
    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        acDataViewItem::calculateNumberOfActiveChannels
// Description: Calculates the number of active channels
// Return Val:  Number of active channels
// Author:      Eran Zinman
// Date:        30/7/2007
// ---------------------------------------------------------------------------
int acDataViewItem::calculateNumberOfActiveChannels()
{
    // Total number of active channels
    int activeChannels = 0;

    // Get number of records in the channels vector
    int amountOfIndices = m_channels.size();

    // Loop through the channels and check for active channels
    for (int i = 0; i < amountOfIndices; i++)
    {
        if (m_channels[i] != NULL)
        {
            // If channel is enabled
            if (m_channels[i]->_isEnabled)
            {
                // Increase total number of channels
                activeChannels++;
            }
        }
    }

    return activeChannels;
}

// ---------------------------------------------------------------------------
// Name:        acDataViewItem::enableRawDataChannel
// Description: Enabled / Disables raw data channels
// Arguments:   channelType - The channel type,
//              isEnabled - Enable / Disable channel
// Return Val:  bool - Success / Failure
// Author:      Eran Zinman
// Date:        5/8/2007
// ---------------------------------------------------------------------------
bool acDataViewItem::enableRawDataChannel(oaTexelDataFormat channelType, bool isEnabled)
{
    bool retVal = false;

    // We only know how to enable / disable one channel
    int amountOfComponents = oaAmountOfTexelFormatComponents(channelType);
    GT_IF_WITH_ASSERT(amountOfComponents == 1)
    {
        // Get number of channels in the raw data
        int amountOfChannels = m_channels.size();

        // Loop through the channels and find the channel we are looking for
        for (int i = 0; i < amountOfChannels; i++)
        {
            if (m_channels[i] != NULL)
            {
                // Is this the channel we are looking for?
                if (m_channels[i]->_channelType == channelType)
                {
                    // Enable / Disable channels
                    m_channels[i]->_isEnabled = isEnabled;

                    // Recalculate amount of active channels
                    m_amountOfActiveChannels = calculateNumberOfActiveChannels();

                    // Channel was found, break the loop
                    retVal = true;
                    break;
                }
            }
        }
    }

    // Return success / failure
    return retVal;
}
// ---------------------------------------------------------------------------
// Name:        acDataViewItem::enablePrimaryRawDataChannel
// Description: Enabled / Disables raw data primary channel
// Arguments:   isEnabled - Enable / Disable channel
// Return Val:  Success / Failure
// Author:      Eran Zinman
// Date:        9/1/2008
// ---------------------------------------------------------------------------
bool acDataViewItem::enablePrimaryRawDataChannel(bool isEnabled)
{
    bool retVal = false;

    // Get first (primary) channel
    oaTexelDataFormat primaryChannel = oaGetTexelFormatComponentType(m_texelDataFormat, 0);
    GT_IF_WITH_ASSERT(primaryChannel != OA_TEXEL_FORMAT_UNKNOWN)
    {
        retVal = enableRawDataChannel(primaryChannel, isEnabled);
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        acDataViewItem::isOk
// Description: Checks if the raw data item is ok
// Return Val:  bool - True - item is ok. False - otherwise
// Author:      Eran Zinman
// Date:        5/8/2007
// ---------------------------------------------------------------------------
bool acDataViewItem::isOk()
{
    bool retVal = false;

    // If we got correct dimensions
    if (m_width >= 0 && m_height >= 0)
    {
        // Do we have a parent?
        if (m_pParent)
        {
            // Did we allocated all necessary objects?
            if (m_pRawDataHandler)
            {
                if (m_pRawDataHandler->isOk())
                {
                    if (m_pTableBase)
                    {
                        retVal = true;
                    }
                }
            }
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        acDataViewItem::getEnabledChannelIndex
// Description: Get channel real index by suppling channelIndex.
//              We will look only in active channels
// Arguments:   channelIndex - The channelIndex ID
// Return Val:  Real channel index in the channels vector
// Author:      Eran Zinman
// Date:        30/7/2007
// ---------------------------------------------------------------------------
int acDataViewItem::getEnabledChannelIndex(int channelIndex)
{
    int realChannelIndex = 0;
    int channelIter = 0;

    // Get number of channels in the raw data
    int amountOfChannels = m_channels.size();

    // Loop through the channels and check for active channels
    for (int i = 0; i < amountOfChannels; i++)
    {
        if (m_channels[i] != NULL)
        {
            // If channel is enabled
            if (m_channels[i]->_isEnabled)
            {
                // Is this the channel we were looking for?
                if (channelIter == channelIndex)
                {
                    // Get channel real index
                    realChannelIndex = i;

                    // We found channel name - break the loop
                    break;
                }

                // Increase total number of channels
                channelIter++;
            }
        }
    }

    // Return channel real index
    return realChannelIndex;
}

// ---------------------------------------------------------------------------
// Name:        acDataViewItem::getChannel
// Description: Return a pointer to the raw data channel, give a channel index
// Arguments:   channelIndex - The channelIndex
// Return Val:  A pointer to the channel object
// Author:      Eran Zinman
// Date:        5/8/2007
// ---------------------------------------------------------------------------
acRawDataChannel* acDataViewItem::getChannel(int channelIndex)
{
    acRawDataChannel* pRawDataChannel = NULL;

    // Get number of channels in the raw data
    int amountOfChannels = m_channels.size();

    // Check that we are in range
    GT_IF_WITH_ASSERT(channelIndex >= 0 && channelIndex < amountOfChannels)
    {
        // Return the pointer to the channel
        pRawDataChannel = m_channels[channelIndex];
    }

    return pRawDataChannel;
}

// ---------------------------------------------------------------------------
// Name:        acDataViewItem::size
// Description: Return the data view item size.
// Return Val: wxSize
// Author:      Sigal Algranaty
// Date:        19/4/2009
// ---------------------------------------------------------------------------
QSize acDataViewItem::size()
{
    return QSize(m_width, m_height);
}



// ---------------------------------------------------------------------------
// Name:        acDataViewItem::calculateBufferAttributes
// Description: Calculate a buffer size according to the VBO format
// Author:      Sigal Algranaty
// Date:        19/4/2009
// ---------------------------------------------------------------------------
void acDataViewItem::calculateBufferAttributes()
{
    // The VBO is displayed one row for a chunk, and one column for each item in a chink:

    // Get the VBO buffer size of chunk according to type:
    m_chunkSize = oaCalculateChunkByteSize(m_texelDataFormat);

    // Get the number of chunks:
    m_height = (int)ceilf((float)m_width / (float)m_chunkSize);
    m_width = m_amountOfActiveChannels;
}


// ---------------------------------------------------------------------------
// Name:        acDataViewItem::updateAttributes
// Description: Update the data item attributes
// Return Val: void
// Author:      Sigal Algranaty
// Date:        21/4/2009
// ---------------------------------------------------------------------------
void acDataViewItem::updateAttributes()
{
    // Get texel data format and data type:
    m_pRawDataHandler->getDataTypeAndFormat(m_texelDataFormat, m_dataType);

    // Get width and height:
    m_pRawDataHandler->getSize(m_width, m_height);

    // Calculate the number of channels:
    m_amountOfActiveChannels = oaAmountOfTexelFormatComponents(m_texelDataFormat);

    // Handle VBO differently:
    oaTexelDataFormat dataFormat = m_pRawDataHandler->dataFormat();

    if (oaIsBufferTexelFormat(dataFormat))
    {
        // Calculate VBO buffer attributes:
        calculateBufferAttributes();
    }

    // Generate the channels array:
    generateChannelsArray(m_texelDataFormat);

}

