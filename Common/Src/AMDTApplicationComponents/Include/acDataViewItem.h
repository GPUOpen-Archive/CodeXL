//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file acDataViewItem.h
///
//==================================================================================

//------------------------------ acDataViewItem.h ------------------------------

#ifndef __ACDATAVIEWITEM
#define __ACDATAVIEWITEM

// Qt:
#include <QtWidgets>

// Infra:
#include <AMDTBaseTools/Include/gtAutoPtr.h>
#include <AMDTBaseTools/Include/gtPtrVector.h>
#include <AMDTBaseTools/Include/gtString.h>
#include <AMDTOSWrappers/Include/osOSDefinitions.h>
#include <AMDTOSAPIWrappers/Include/oaDataType.h>
#include <AMDTOSAPIWrappers/Include/oaTexelDataFormat.h>

// Local:
#include <AMDTApplicationComponents/Include/acImageItem.h>
#include <AMDTApplicationComponents/Include/acRawFileHandler.h>
#include <AMDTApplicationComponents/Include/acApplicationComponentsDLLBuild.h>
#include <AMDTApplicationComponents/Include/acDefinitions.h>

// Forward decelerations:
class acDataViewGridTable;

// This is a simple struct which holds all the channels in the raw data
struct AC_API acRawDataChannel
{
    oaTexelDataFormat _channelType;      // Channel type
    bool _isEnabled;                     // Is channel enabled?
};


// ----------------------------------------------------------------------------------
// Class Name:           acDataViewItem
// General Description:  This class represents a data view item.
//                       The item is associated with a raw data handler that
//                       eventually the grid table will take it's data from.
// Author:               Eran Zinman
// Creation Date:        5/8/2007
// ----------------------------------------------------------------------------------
class AC_API acDataViewItem
{
public:
    friend class acDataViewGridTable;
    struct acDataViewInfo
    {
        acRawFileHandler* _pFilterRawFileHandler[AC_MAX_RAW_FILE_FILTER_HANDLERS];
        int _yCoordGroupSize;
        int _xCoordGroupSize;

        int _xCoordOffset;
        int _yCoordOffset;

        acDataViewInfo(): _yCoordGroupSize(0), _xCoordGroupSize(0), _xCoordOffset(0), _yCoordOffset(0)
        {
            for (int i = 0; i < AC_MAX_RAW_FILE_FILTER_HANDLERS; i++)
            {
                _pFilterRawFileHandler[i] = NULL;
            }
        }
        acDataViewInfo(const acDataViewInfo& other): _yCoordGroupSize(other._yCoordGroupSize), _xCoordGroupSize(other._xCoordGroupSize),
            _xCoordOffset(other._xCoordOffset), _yCoordOffset(other._yCoordOffset)
        {
            for (int i = 0; i < AC_MAX_RAW_FILE_FILTER_HANDLERS; i++)
            {
                _pFilterRawFileHandler[i] = other._pFilterRawFileHandler[i];
            }
        }
    };

    // Default CTOR (required by Qt 5.2):
    acDataViewItem() {}

    // Constructor:
    acDataViewItem(QWidget* pParent, int canvasItemID, acRawFileHandler* pRawDataHandler, const acDataViewInfo& dataInfo);

    // Destructor:
    ~acDataViewItem();

    // Update my data table:
    bool updateDataTable();

    // Update my attributes:
    void updateAttributes();

    // Check if raw data is valid
    bool isOk();

public:
    // Rotate raw data by a specific angle (90, 180 or 270 degrees)
    bool rotateRawDataByAngle(int rotateAngle);

    // Get channel real index by suppling channelIndex
    int getEnabledChannelIndex(int channelIndex);

    // Enable / Disable channel
    bool enableRawDataChannel(oaTexelDataFormat channelType, bool isEnabled);
    bool enablePrimaryRawDataChannel(bool isEnabled);

    // Get grid cell data at the grid (row, col) position
    bool getGridCellData(int row, int col, gtString& cellValue);

    // Set the active page in the raw data
    bool setActivePage(int pageIndex);

    // Should we show normalized values?
    void enableNormalizedValues(bool enable) { m_showNormalizedValues = enable; };

    // Should we show hexadecimal values?
    static void showHexadecimalValues(bool showHex) { m_sShowHexValues = showHex;}
    static bool shouldShowHexadecimalValues() { return m_sShowHexValues;}

    // Return the current data item rotation angle
    double rotationAngle() { return m_rotationAngle; };

public:
    // Returns the pointer to the raw data grid table
    acDataViewGridTable* gridTable() { return m_pTableBase; };

    // Returns the size of the X/Y coordinate group size:
    int yCoordinateGroupSize() { return m_viewDataInfo._yCoordGroupSize; };
    int xCoordinateGroupSize() { return m_viewDataInfo._xCoordGroupSize; };

    // Returns the size of the X/Y coordinate offset:
    int yCoordinateOffset() { return m_viewDataInfo._yCoordOffset; };
    int xCoordinateOffset() { return m_viewDataInfo._xCoordOffset; };

    // Returns the item canvas ID
    acImageItemID getCanvasID() { return m_canvasItemID; };

    // Return the raw data item parent
    QWidget* parent() { return m_pParent; };

    // Return the raw data size:
    QSize size();

    // Return raw data channel object
    acRawDataChannel* getChannel(int channelIndex);

    // Return the raw data channel data type
    oaDataType dataType() { return m_dataType; };

    // Return the raw data format
    oaTexelDataFormat dataFormat() { return m_texelDataFormat; };

    // Return the raw data handler object
    acRawFileHandler* getRawDataHandler() { return m_pRawDataHandler; };

    // Amount of active channels:
    int amountOfActiveChannels() const {return m_amountOfActiveChannels;};

private:
    // Internal function that calculates how many channels are active from the total amount of channels
    int calculateNumberOfActiveChannels();

    // Generates the channels array
    void generateChannelsArray(oaTexelDataFormat channels);

    // If raw data is rotated, calculate rotated (xPos, yPos) accordingly
    void calculateRotationPosition(int& xPos, int& yPos);

    void calculateBufferAttributes();

private:
    // Canvas ID - For connection with the image viewer
    acImageItemID m_canvasItemID;

    // Raw data Properties:
    int m_width;
    int m_height;
    int m_chunkSize;
    acRawFileHandler* m_pRawDataHandler;

    // Amount of channels:
    int m_amountOfActiveChannels;

    // The size of each group in the X and Y coordinates:
    acDataViewInfo m_viewDataInfo;

    // Vector with the channel status:
    gtPtrVector<acRawDataChannel*> m_channels;

    // The data type:
    oaDataType m_dataType;

    // Raw data format:
    oaTexelDataFormat m_texelDataFormat;

    // Raw data Filters / Status:
    int m_rotationAngle;
    bool m_showNormalizedValues;
    static bool m_sShowHexValues;

    // Grid Data:
    acDataViewGridTable* m_pTableBase;
    QWidget* m_pParent;
};

#endif  // __ACDATAVIEWITEM
