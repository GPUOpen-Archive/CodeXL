//=====================================================================
// Copyright 2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file oaRawFileSeralizer.h
///
//=====================================================================

//------------------------------ oaRawFileSeralizer.h ------------------------------

#ifndef __OARAWFILESERALIZER
#define __OARAWFILESERALIZER

// Infra:
#include <AMDTBaseTools/Include/AMDTDefinitions.h>
#include <AMDTOSWrappers/Include/osFilePath.h>
#include <AMDTOSWrappers/Include/osRawMemoryStream.h>

// Local:
#include <AMDTOSAPIWrappers/Include/oaOSAPIWrappersDLLBuild.h>
#include <AMDTOSAPIWrappers/Include/oaDataType.h>
#include <AMDTOSAPIWrappers/Include/oaTexelDataFormat.h>

#define OA_RAW_FILE_DEFAULT_COMPRESSION -1

// ----------------------------------------------------------------------------------
// Class Name:           oaRawFileSeralizer
// General Description:  Writes / reads raw data into / from a file.
// Author:      AMD Developer Tools Team
// Creation Date:        1/9/2007
// ----------------------------------------------------------------------------------
class OA_API oaRawFileSeralizer
{
public:
    // Constructor:
    oaRawFileSeralizer();

    // Destructor:
    virtual ~oaRawFileSeralizer();

public:
    // Save raw data to a file:
    bool saveToFile(const osFilePath& bufferFilePath, int zlibCompressionLevel = OA_RAW_FILE_DEFAULT_COMPRESSION);

    // Load raw data data from a file:
    bool loadFromFile(const osFilePath& bufferFilePath);

public:
    // Set raw data pointer
    void setRawData(gtByte* pRawData) { _pRawData = pRawData; };

    // Set the raw data dimensions
    void setRawDataDimensions(int width, int height) { _width = width; _height = height; };

    // Set the raw data texel data format and data type:
    void setRawDataFormat(oaTexelDataFormat texelDataFormat, oaDataType dataType) { _texelDataFormat = texelDataFormat; _dataType = dataType; };

    // Set the amount of pages in the raw data
    void setAmountOfPages(int amountOfPages) { _amountOfPages = amountOfPages; };

public:
    // Get raw data pointer (optional: take ownership over it)
    gtByte* getRawDataPointer(bool takeOwnership = false);

    // Get the total size, in bytes, of the data:
    gtSizeType getRawDataSize();

    // Get raw data Dimensions
    void getRawDataDimensions(int& width, int& height) { width = _width; height = _height; };

    // Get the raw data texel data format and data type:
    void getRawDataFormat(oaTexelDataFormat& texelDataFormat, oaDataType& dataType) { texelDataFormat = _texelDataFormat; dataType = _dataType; };

    // Return amount of pages in the raw data
    int getAmountOfPages() { return _amountOfPages; };

private:
    // Generate the file header for the output buffer:
    bool writeHeaderToBuffer(osRawMemoryStream* pRawMemoryStream);

    // Writes the header into the debug log file:
    void writeHeaderToDebugLogFile(const gtString& debugMessageHeader, const osFilePath& associatedFilePath);

    // Generate the file data for the output buffer:
    bool writeDataToBuffer(osRawMemoryStream* pRawMemoryStream);

    // Writes the raw data into the output file
    bool writeFileToDisk(osRawMemoryStream* pRawMemoryStream, const osFilePath& filePath, int zlibCompressionLevel);

private:
    // Extract a raw data file to osRawMemoryStream object
    osRawMemoryStream* extractFileToMemoryStream(const osFilePath& filePath);

    // Reads a whole file (as is) to memory
    bool readFileToMemory(const osFilePath filePath, gtByte*& pFileContent, unsigned long& fileSize);

    // Loads the raw data header from the memory stream
    bool readHeaderFromBuffer(osRawMemoryStream* pMemoryStream);

    // Loads the raw data content from the memory stream
    bool readRawDataFromBuffer(osRawMemoryStream* pMemoryStream);

private:
    // Calculate the memory space that will receive the raw data
    bool calculateRawDataSize(gtSizeType& rawDataSize);

    // Release allocated raw memory, if exists
    void releaseRawMemory();

private:
    // Raw data data and component format
    oaDataType _dataType;

    // Amount of components per pixel
    oaTexelDataFormat _texelDataFormat;

    // Raw data width and height
    int _width, _height;

    // The Raw data array
    gtByte* _pRawData;

    // The amount of pages in the raw data
    int _amountOfPages;
};


#endif  // __OARAWFILESERALIZER
