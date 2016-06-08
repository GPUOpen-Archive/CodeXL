//=====================================================================
// Copyright 2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file oaRawFileSeralizer.cpp
///
//=====================================================================

//------------------------------ oaRawFileSeralizer.cpp ------------------------------

/*

Raw file format
===============

The raw file is a binary file.
Version 1.2 has the following format:

<Raw File>                          ::= <File Header> <File Data> <End of file>
|---<File Header>                   ::= <File Version> <Components Per Pixel> <Component Data Type>
|   |---<File Version>              ::= <integer>
|   |---<Data Format>               ::= <unsigned int> - Data format (for example: RGB)
|   +---<Component Data Type>       ::= <unsigned int> - Component data type (for example: UNSIGNED BYTE)
|
|---<File Data>                     ::= <Data size> <Amount of Pages> <Raw data>
|   |---<Data Size>                 ::= <Data width> <Data height>
|   |   |---<Data Width>            ::= <integer>
|   |   +---<Data Height>           ::= <integer>
|   |
|   |---<Amount of pages>
|   |   +---<Amount of pages>       ::= <integer>
|   |
|   +---<Raw Data>                  ::= "Data width" X "Data height" of "Components per pixel" x "Component Data Type"
|       +---<Raw Data Item>         ::= An array of bits of the raw data defined by "Data Format" and "Component Data Type"
|
+---<End File>                      ::= EOF

*/

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTBaseTools/Include/AMDTDefinitions.h>
#include <AMDTOSWrappers/Include/osDebugLog.h>
#include <AMDTOSWrappers/Include/osFile.h>
#include <AMDTOSWrappers/Include/osStringConstants.h>

// Standard C:
#include <stdlib.h>

// ZLib:
#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
    // Uri, 14/11/10 - this is required for linking with zlib 1.2.5:
    #define ZLIB_WINAPI
#endif
#include <zlib.h>

// Local:
#include <AMDTOSAPIWrappers/Include/oaRawFileSeralizer.h>
#include <AMDTOSAPIWrappers/Include/oaStringConstants.h>

// Buffer output file version
#define OA_RAW_DATA_FILE_VERSION 3                     // (3) is Version 1.2

// Data chunk size (for reading / writing files).
#define OA_RAW_DATA_DATA_CHUNK_SIZE 256 * 1024         // 256 KB


// ---------------------------------------------------------------------------
// Name:        oaRawFileSeralizer::oaRawFileSeralizer
// Description: Constructor
// Author:      AMD Developer Tools Team
// Date:        7/8/2007
// ---------------------------------------------------------------------------
oaRawFileSeralizer::oaRawFileSeralizer()
    : _dataType(OA_UNSIGNED_BYTE), _texelDataFormat(OA_TEXEL_FORMAT_UNKNOWN),
      _width(-1), _height(-1), _pRawData(NULL), _amountOfPages(-1)
{
}


// ---------------------------------------------------------------------------
// Name:        oaRawFileSeralizer::~oaRawFileSeralizer
// Description: Destructor
// Author:      AMD Developer Tools Team
// Date:        7/8/2007
// ---------------------------------------------------------------------------
oaRawFileSeralizer::~oaRawFileSeralizer()
{
    // Release allocated raw memory, if exists
    releaseRawMemory();
}

// ---------------------------------------------------------------------------
// Name:        oaRawFileSeralizer::writeFileToDisk
// Description: Writes the raw data into the output file. Make the output file
//              a compressed zlib file.
// Arguments:   pRawMemoryStream - The memory stream to write to the file
//              filePath - The output raw data file path.
//              zlibCompressionLevel - The zlib compression level (0..9)
// Return Val:  bool  - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        19/12/2007
// ---------------------------------------------------------------------------
bool oaRawFileSeralizer::writeFileToDisk(osRawMemoryStream* pRawMemoryStream, const osFilePath& filePath, int zlibCompressionLevel)
{
    bool retVal = false;

    // Sanity Check:
    GT_IF_WITH_ASSERT(pRawMemoryStream != NULL)
    {
        // Calculate the raw memory buffer size:
        size_t writePos = pRawMemoryStream->currentWritePosition();
        size_t readPos = pRawMemoryStream->currentReadPosition();
        size_t srcBufferSize = writePos - readPos;

        // Initialize The zlib stream
        z_stream zlibStream;
        zlibStream.zalloc = Z_NULL;
        zlibStream.zfree = Z_NULL;
        zlibStream.opaque = Z_NULL;

        // Initialize deflation process
        int rc0 = deflateInit(&zlibStream, zlibCompressionLevel);
        GT_IF_WITH_ASSERT(rc0 == Z_OK)
        {
            // Open output file for writing
            osFile outputFile;
            bool rc1 = outputFile.open(filePath, osChannel::OS_BINARY_CHANNEL, osFile::OS_OPEN_TO_WRITE);
            GT_IF_WITH_ASSERT(rc1)
            {
                // Get raw memory buffer data pointer
                gtByte* pSrcBufferData = pRawMemoryStream->getRawBufferPointer();
                GT_IF_WITH_ASSERT(pSrcBufferData != NULL)
                {
                    // Indicate the data that should be compressed (size and pointer to data)
                    zlibStream.avail_in = (uInt)srcBufferSize;
                    zlibStream.next_in = (Bytef*)pSrcBufferData;

                    // Create compressed data output buffer
                    Bytef* pOutputCompressBuffer = (Bytef*) malloc(OA_RAW_DATA_DATA_CHUNK_SIZE);


                    retVal = true;

                    // Deflate() on input until output buffer not full, or until all source has been read
                    do
                    {
                        // The compressed data that should be written (size and buffer)
                        zlibStream.avail_out = OA_RAW_DATA_DATA_CHUNK_SIZE;
                        zlibStream.next_out = pOutputCompressBuffer;

                        // Compressed the data
                        int rc3 = deflate(&zlibStream, Z_FULL_FLUSH);
                        GT_IF_WITH_ASSERT(rc3 == Z_OK)
                        {
                            // Calculate the amount of bytes to write to destination file and write them
                            unsigned long bytesToWrite = OA_RAW_DATA_DATA_CHUNK_SIZE - zlibStream.avail_out;
                            bool rc4 = outputFile.write((gtByte*)pOutputCompressBuffer, bytesToWrite);

                            if (!rc4)
                            {
                                retVal = false;
                                GT_ASSERT_EX(false, L"Writing to output file has failed!");
                            }
                        }
                    }
                    while ((zlibStream.avail_out == 0) && (retVal));

                    // Free data chunk
                    free(pOutputCompressBuffer);
                }

                // Close output file
                outputFile.close();
            }
        }

        // Close deflate process and return
        deflateEnd(&zlibStream);
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        oaRawFileSeralizer::saveBufferToFile
// Description: Saves a raw data to a file
// Arguments:   filePath - The file name to save the raw data into
//              zlibCompressionLevel - The zlib compression level to
//              save the file with (0..9), -1 is default
// Return Val:  Success / Failure
// Author:      AMD Developer Tools Team
// Date:        7/8/2007
// ---------------------------------------------------------------------------
bool oaRawFileSeralizer::saveToFile(const osFilePath& filePath, int zlibCompressionLevel)
{
    bool retVal = false;

    // Output debug log message:
    static gtString messageHeader = OA_STR_savingRawDataFile;
    writeHeaderToDebugLogFile(messageHeader, filePath);

    // Initialize the size with the width:
    gtSizeType estimatedOutputBufferSize = _width;
    bool rcSizeEstimated = calculateRawDataSize(estimatedOutputBufferSize);
    GT_IF_WITH_ASSERT(rcSizeEstimated)
    {
        // add a 1kb overhead for header
        estimatedOutputBufferSize += 1024;

        // Allocate a memory stream:
        osRawMemoryStream* pRawMemoryStream  = new osRawMemoryStream(estimatedOutputBufferSize);


        // Generate the file header:
        bool rc1 = writeHeaderToBuffer(pRawMemoryStream);

        // Generate the buffer data:
        bool rc2 = writeDataToBuffer(pRawMemoryStream);

        // If all data was generated successfully
        GT_IF_WITH_ASSERT(rc1 && rc2)
        {
            // Write buffer raw memory stream to a file on the disk
            retVal = writeFileToDisk(pRawMemoryStream, filePath, zlibCompressionLevel);
        }

        // We are done, release the memory stream
        delete pRawMemoryStream;
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        oaRawFileSeralizer::writeHeaderToBuffer
// Description: Generates the file header.
//              See "Raw file format" at the top of this file for more
//              details.
// Arguments:   pRawMemoryStream - The memory stream to write to
// Return Val:  bool  - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        8/8/2007
// ---------------------------------------------------------------------------
bool oaRawFileSeralizer::writeHeaderToBuffer(osRawMemoryStream* pRawMemoryStream)
{
    bool retVal = false;

    // Sanity Check:
    GT_IF_WITH_ASSERT(pRawMemoryStream != NULL)
    {
        // Write the file version:
        gtInt32 fileVersion = (gtInt32)OA_RAW_DATA_FILE_VERSION;
        *pRawMemoryStream << fileVersion;

        // Convert data format to unsigned int and write it:
        gtUInt32 uintTexelDataFormat = (gtUInt32) _texelDataFormat;
        *pRawMemoryStream << uintTexelDataFormat;

        // Convert data type to unsigned int and write it:
        gtUInt32 uintComponentDataType = (gtUInt32) _dataType;
        *pRawMemoryStream << uintComponentDataType;

        // Write the buffer width and height:
        *pRawMemoryStream << (gtInt32)_width;
        *pRawMemoryStream << (gtInt32)_height;

        // Write the amount of pages in the raw data:
        *pRawMemoryStream << (gtInt32)_amountOfPages;

        retVal = true;
    }

    // End of file header
    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        oaRawFileSeralizer::writeHeaderToDebugLogFile
// Description: Writes the header information to the debug log file.
// Arguments: debugMessageHeader - A string that will be presented at beginning of the debug printout.
//            associatedFilePath - The path of the file associated with the logged action.
// Author:      AMD Developer Tools Team
// Date:        8/8/2007
// ---------------------------------------------------------------------------
void oaRawFileSeralizer::writeHeaderToDebugLogFile(const gtString& debugMessageHeader, const osFilePath& associatedFilePath)
{
    // If the current debug log severity is "Debug":
    osDebugLogSeverity loggedSeverity = osDebugLog::instance().loggedSeverity();

    if (OS_DEBUG_LOG_DEBUG <= loggedSeverity)
    {
        // Initialize the debug log message to contain the input header:
        gtString debugLogMsg = debugMessageHeader;

        // Add the associated file path:
        debugLogMsg += L" - ";
        debugLogMsg += OA_STR_associatedFilePath;
        debugLogMsg += associatedFilePath.asString();

        // Add the raw file version:
        debugLogMsg += L", ";
        debugLogMsg.appendFormattedString(OA_STR_rawFileVersion, (int)OA_RAW_DATA_FILE_VERSION);

        // Add the texel data format:
        gtString texelDataFormatAsString;
        bool rc1 = oaTexelDataFormatAsString(_texelDataFormat, texelDataFormatAsString);

        if (!rc1)
        {
            texelDataFormatAsString = OS_STR_unknown;
        }

        debugLogMsg += L", ";
        debugLogMsg += OA_STR_texelDataFormat;
        debugLogMsg += texelDataFormatAsString;

        // Add the texel data type:
        gtString texelDataTypeAsString;
        bool rc2 = oaDataTypeAsString(_dataType, texelDataTypeAsString);

        if (!rc2)
        {
            texelDataTypeAsString = OS_STR_unknown;
        }

        debugLogMsg += L", ";
        debugLogMsg += OA_STR_texelDataFormat;
        debugLogMsg += texelDataTypeAsString;

        // Add the buffer width and height:
        debugLogMsg += L", ";
        debugLogMsg.appendFormattedString(OA_STR_pageDimensions, _width, _height);

        // Add the pages amount:
        debugLogMsg += L", ";
        debugLogMsg.appendFormattedString(OA_STR_pagesAmount, _amountOfPages);

        // Output the debug log message:
        OS_OUTPUT_DEBUG_LOG(debugLogMsg.asCharArray(), OS_DEBUG_LOG_DEBUG);
    }
}


// ---------------------------------------------------------------------------
// Name:        oaRawFileSeralizer::calculateRawDataSize
// Description: Calculate the memory space that will receive the raw data.
// Arguments:   rawDataSize - Output raw data size
// Return Val:  bool - success / failure
// Author:      AMD Developer Tools Team
// Date:        14/8/2007
// ---------------------------------------------------------------------------
bool oaRawFileSeralizer::calculateRawDataSize(gtSizeType& rawDataSize)
{
    bool retVal = false;

    // Reset variable
    rawDataSize = 0;
    int rawDataPixelSize = 1;

    // For VBO texel types, the size is simply the width:
    bool isVBO = oaIsBufferTexelFormat(_texelDataFormat);

    if (isVBO)
    {
        rawDataPixelSize = 1;
    }
    else
    {
        // Calculate raw data pixel size
        rawDataPixelSize = oaCalculatePixelUnitByteSize(_texelDataFormat, _dataType);
    }

    GT_IF_WITH_ASSERT(rawDataPixelSize != -1)
    {
        // Do we have valid dimensions?
        GT_IF_WITH_ASSERT((_width >= 0) && (_height >= 0))
        {
            // Do we have a valid amount of pages?
            GT_IF_WITH_ASSERT(_amountOfPages > 0)
            {
                // Calculate the required buffer size (in bytes):
                rawDataSize = _width * _height * rawDataPixelSize * _amountOfPages;

                retVal = true;
            }
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        oaRawFileSeralizer::writeDataToBuffer
// Description: Generates the file raw data
//              See "Raw file format" at the top of this file for more
//              details.
// Arguments:   pRawMemoryStream - The memory stream to write to
// Return Val:  bool  - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        8/8/2007
// ---------------------------------------------------------------------------
bool oaRawFileSeralizer::writeDataToBuffer(osRawMemoryStream* pRawMemoryStream)
{
    bool retVal = false;

    // Sanity Check:
    GT_IF_WITH_ASSERT(pRawMemoryStream != NULL)
    {
        // Is raw data ok?
        GT_IF_WITH_ASSERT(_pRawData != NULL)
        {
            // Calculate raw data size:
            gtSizeType rawDataSize = 0;
            bool rc1 = calculateRawDataSize(rawDataSize);
            GT_IF_WITH_ASSERT(rc1)
            {
                retVal = true;

                // Do we have anything to write?
                if (rawDataSize > 0)
                {
                    // Write the raw data into the file
                    retVal = pRawMemoryStream->write(_pRawData, rawDataSize);
                }
            }
        }
    }

    // End of file data
    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        oaRawFileSeralizer::getRawDataPointer
// Description: Returns a pointer to the raw data of the buffer. If takeOwnership
//              is true, we give the pointer ownership to the requester.
// Arguments:   takeOwnership - If flagged true, the object that requested
//              the buffer data will be responsible for releasing the memory
//              allocated by the buffer raw data.
// Return Val:  The raw data pointer
// Author:      AMD Developer Tools Team
// Date:        18/8/2007
// ---------------------------------------------------------------------------
gtByte* oaRawFileSeralizer::getRawDataPointer(bool takeOwnership)
{
    // Get data pointer
    gtByte* pData = _pRawData;

    // Should we give ownership?
    if (takeOwnership)
    {
        // Make sure the memory allocated by the buffer raw data will not be released by us
        _pRawData = NULL;
    }

    return pData;
}

// ---------------------------------------------------------------------------
// Name:        oaRawFileSeralizer::getRawDataSize
// Description: Gets the total size of the raw data.
// Author:      AMD Developer Tools Team
// Date:        13/1/2010
// ---------------------------------------------------------------------------
gtSizeType oaRawFileSeralizer::getRawDataSize()
{
    gtSizeType retVal = 0;

    gtSizeType rawDataSize = 0;
    bool rcCalc = calculateRawDataSize(rawDataSize);
    GT_IF_WITH_ASSERT(rcCalc)
    {
        retVal = rawDataSize;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        acBufferFileHandler::loadFileToMemoryStream
// Description: Loads a data file to a memory stream
// Arguments:   filePath - The file path
// Return Val:  A pointer to the osRawMemoryStream object. NULL if error occurred
// Author:      AMD Developer Tools Team
// Date:        12/8/2007
// ---------------------------------------------------------------------------
osRawMemoryStream* oaRawFileSeralizer::extractFileToMemoryStream(const osFilePath& filePath)
{
    osRawMemoryStream* pRawMemoryStream = NULL;

    // Initialize zlib stream
    z_stream zlibStream;
    zlibStream.zalloc = Z_NULL;
    zlibStream.zfree = Z_NULL;
    zlibStream.opaque = Z_NULL;
    zlibStream.avail_in = 0;
    zlibStream.next_in = Z_NULL;

    int rc1 = inflateInit(&zlibStream);
    GT_IF_WITH_ASSERT(rc1 == Z_OK)
    {
        gtByte* pMemory = NULL;
        unsigned long fileSize = 0;

        // Read the whole source file to memory
        bool rc2 = readFileToMemory(filePath, pMemory, fileSize);
        GT_IF_WITH_ASSERT(rc2)
        {
            // This is the data that should be decompressed
            zlibStream.next_in = (Bytef*)pMemory;
            zlibStream.avail_in = fileSize;

            // Create output buffer
            Bytef* pOutputUncompressBuffer = (Bytef*) malloc(OA_RAW_DATA_DATA_CHUNK_SIZE);
            GT_IF_WITH_ASSERT(pOutputUncompressBuffer != NULL)
            {
                // Create output stream
                pRawMemoryStream = new osRawMemoryStream;
                GT_IF_WITH_ASSERT(pRawMemoryStream != NULL)
                {
                    // Read chunks of uncompressed data from source
                    do
                    {
                        zlibStream.avail_out = OA_RAW_DATA_DATA_CHUNK_SIZE;
                        zlibStream.next_out = pOutputUncompressBuffer;

                        // Decompress source data
                        int rc3 = inflate(&zlibStream, Z_NO_FLUSH);
                        GT_IF_WITH_ASSERT(rc3 == Z_OK)
                        {
                            // Write the uncompressed data the raw memory stream
                            unsigned long uncompressSize = OA_RAW_DATA_DATA_CHUNK_SIZE - zlibStream.avail_out;
                            bool rc4 = pRawMemoryStream->write((gtByte*)pOutputUncompressBuffer, uncompressSize);
                            GT_ASSERT(rc4);
                        }
                    }
                    while (zlibStream.avail_out == 0);
                }

                // Free data chunk
                free(pOutputUncompressBuffer);
            }

            // Release the file contents memory
            free(pMemory);
        }
    }

    // Clean up and return
    inflateEnd(&zlibStream);

    return pRawMemoryStream;
}

// ---------------------------------------------------------------------------
// Name:        oaRawFileSeralizer::readFileToMemory
// Description: Reads a whole file (as is) to memory and return a pointer
//              the memory (which holds the file content) and also, return
//              the size of the file content.
// Arguments:   filePath - The file to load the data from
//              pFileContent - Output pointer to the file data
//              fileSize - Output content size
// Return Val:  bool - Success / Failure
// Author:      AMD Developer Tools Team
// Date:        24/8/2007
// ---------------------------------------------------------------------------
bool oaRawFileSeralizer::readFileToMemory(const osFilePath filePath, gtByte*& pFileContent, unsigned long& fileSize)
{
    bool retVal = false;
    pFileContent = NULL;

    // Is source file ok?
    GT_IF_WITH_ASSERT(filePath.isRegularFile())
    {
        // Open source file
        osFile inputFile;
        bool rc1 = inputFile.open(filePath, osChannel::OS_BINARY_CHANNEL, osFile::OS_OPEN_TO_READ);
        GT_IF_WITH_ASSERT(rc1)
        {
            // Get file size
            GT_IF_WITH_ASSERT(inputFile.getSize(fileSize))
            {
                // Allocate memory for reading the whole file
                pFileContent = (gtByte*)malloc(fileSize);
                GT_IF_WITH_ASSERT(pFileContent != NULL)
                {
                    // Read whole data from file
                    gtSize_t actualDataRead = 0;
                    bool rc2 = inputFile.readAvailableData(pFileContent, fileSize, actualDataRead);
                    GT_IF_WITH_ASSERT(rc2)
                    {
                        // Was data read successfully?
                        GT_IF_WITH_ASSERT(actualDataRead == fileSize)
                        {
                            retVal = true;
                        }
                    }
                }
            }

            // Close input file
            inputFile.close();
        }
    }

    // If file was not loaded successfully, release the allocated memory
    if (!retVal)
    {
        if (pFileContent)
        {
            free(pFileContent);
            pFileContent = NULL;
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        acBufferFileHandler::readHeaderFromBuffer
// Description: Loads the raw data header from the memory stream
// Arguments:   pMemoryStream - The memory stream to read the header from
// Return Val:  bool - Success / Failure
// Author:      AMD Developer Tools Team
// Date:        1/9/2007
// ---------------------------------------------------------------------------
bool oaRawFileSeralizer::readHeaderFromBuffer(osRawMemoryStream* pMemoryStream)
{
    bool retVal = false;

    // Sanity Check:
    GT_IF_WITH_ASSERT(pMemoryStream != NULL)
    {
        // Get file version
        gtInt32 fileVersion = 0;
        *pMemoryStream >> fileVersion;

        // Do we support this version?
        GT_IF_WITH_ASSERT(fileVersion == (gtInt32) OA_RAW_DATA_FILE_VERSION)
        {
            // Get texel data format
            gtUInt32 uint32TexelDataFormat = 0;
            *pMemoryStream >> uint32TexelDataFormat;
            _texelDataFormat = (oaTexelDataFormat) uint32TexelDataFormat;

            // Get buffer data type:
            gtUInt32 uint32ComponentDataType = 0;
            *pMemoryStream >> uint32ComponentDataType;
            _dataType = (oaDataType) uint32ComponentDataType;

            // Get the buffer width and height:
            gtInt32 widthAsInt32 = 0;
            *pMemoryStream >> widthAsInt32;
            _width = (int)widthAsInt32;
            gtInt32 heightAsInt32 = 0;
            *pMemoryStream >> heightAsInt32;
            _height = (int)heightAsInt32;

            // Get amount of pages in the raw data:
            gtInt32 amountOfPagesAsInt32 = 0;
            *pMemoryStream >> amountOfPagesAsInt32;
            _amountOfPages = (int)amountOfPagesAsInt32;

            // Check that we got valid values?
            GT_IF_WITH_ASSERT((_width > 0) && (_height > 0))
            {
                GT_IF_WITH_ASSERT(_amountOfPages > 0)
                {
                    retVal = true;
                }
            }
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        acBufferFileHandler::releaseRawMemory
// Description: Release allocated raw memory, if exists
// Author:      AMD Developer Tools Team
// Date:        1/9/2007
// ---------------------------------------------------------------------------
void oaRawFileSeralizer::releaseRawMemory()
{
    // Release raw data allocated memory
    if (_pRawData)
    {
        free(_pRawData);
        _pRawData = NULL;
    }
}

// ---------------------------------------------------------------------------
// Name:        acBufferFileHandler::readRawDataFromBuffer
// Description: Loads the raw data content from the memory stream
// Arguments:   pMemoryStream - The memory stream to read the data content from
// Return Val:  bool - Success / Failure
// Author:      AMD Developer Tools Team
// Date:        1/9/2007
// ---------------------------------------------------------------------------
bool oaRawFileSeralizer::readRawDataFromBuffer(osRawMemoryStream* pMemoryStream)
{
    bool retVal = false;

    // Sanity Check:
    GT_IF_WITH_ASSERT(pMemoryStream != NULL)
    {
        // Calculate raw data size
        gtSizeType rawDataSize = 0;
        bool rc1 = calculateRawDataSize(rawDataSize);
        GT_IF_WITH_ASSERT(rc1)
        {
            // Release old raw memory, if exists
            releaseRawMemory();

            // Allocate the new buffer:
            _pRawData = (gtByte*)malloc(rawDataSize);
            GT_IF_WITH_ASSERT(_pRawData != NULL)
            {
                retVal = true;

                // Do we have anything to read?
                if (rawDataSize > 0)
                {
                    // Read from the memory stream
                    retVal = pMemoryStream->read(_pRawData, rawDataSize);
                }
            }
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        oaRawFileSeralizer::loadFromFile
// Description: Loads a raw data file
// Arguments:   fileToLoad - The file name to load the raw data from
// Return Val:  bool - Success / Failure
// Author:      AMD Developer Tools Team
// Date:        1/9/2007
// ---------------------------------------------------------------------------
bool oaRawFileSeralizer::loadFromFile(const osFilePath& fileToLoad)
{
    bool retVal = false;

    // Does file exist?
    GT_IF_WITH_ASSERT(fileToLoad.isRegularFile())
    {
        // Load the file into a memory stream
        osRawMemoryStream* pRawMemoryStream = extractFileToMemoryStream(fileToLoad);
        GT_IF_WITH_ASSERT(pRawMemoryStream != NULL)
        {
            // Read the file header:
            bool rc1 = readHeaderFromBuffer(pRawMemoryStream);
            GT_IF_WITH_ASSERT(rc1)
            {
                // Output debug log message:
                static gtString messageHeader = OA_STR_loadingRawDataFile;
                writeHeaderToDebugLogFile(messageHeader, fileToLoad);

                // Read the raw data buffer:
                bool rc2 = readRawDataFromBuffer(pRawMemoryStream);
                GT_IF_WITH_ASSERT(rc2)
                {
                    retVal = true;
                }
            }

            // We are done, release the memory stream
            delete pRawMemoryStream;
        }
    }

    return retVal;
}








