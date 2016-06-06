//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file acRawDataExporter.cpp
///
//==================================================================================

//------------------------------ acRawDataExporter.cpp ------------------------------

// Qt
#include <QtWidgets>

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTBaseTools/Include/gtString.h>
#include <AMDTOSWrappers/Include/osTime.h>
#include <AMDTOSWrappers/Include/osFile.h>

// Local:
#include <AMDTApplicationComponents/Include/acRawDataExporter.h>
#include <AMDTApplicationComponents/Include/acImageDataProxy.h>

// ---------------------------------------------------------------------------
// Name:        acRawDataExporter::acRawDataExporter
// Description: Constructor
// Arguments:   pRawDataHandler - Raw data to be written
// Author:      Eran Zinman
// Date:        27/1/2008
// ---------------------------------------------------------------------------
acRawDataExporter::acRawDataExporter(acRawFileHandler* pRawDataHandler)
    : _pRawDataHandler(pRawDataHandler), _outputFileType(AP_PNG_FILE)
{

}

// ---------------------------------------------------------------------------
// Name:        acRawDataExporter::~acRawDataExporter
// Description: Destructor
// Author:      Eran Zinman
// Date:        27/1/2008
// ---------------------------------------------------------------------------
acRawDataExporter::~acRawDataExporter()
{

}

// ---------------------------------------------------------------------------
// Name:        acRawDataExporter::setOutputFormat
// Description: Sets the output format to save the raw data in
// Arguments:   fileType - File type to save the data as.
// Return Val:  bool - Success / Failure
// Author:      Eran Zinman
// Date:        27/1/2008
// ---------------------------------------------------------------------------
bool acRawDataExporter::setOutputFormat(apFileType fileType)
{
    bool retVal = false;

    // Sanity check:
    GT_IF_WITH_ASSERT(_pRawDataHandler != NULL)
    {
        // Let check if raw data is multipage. If it does - only supported graphic format is TIFF
        int amountOfPages = _pRawDataHandler->amountOfPages();
        bool isMultipage = amountOfPages > 1;

        // Now we do a check to see if file format is supported:
        switch (fileType)
        {
            case AP_JPEG_FILE:
            case AP_PNG_FILE:
            case AP_BMP_FILE:
            {
                // We can only single page files in these formats
                retVal = !isMultipage;
            }
            break;

            case AP_TIFF_FILE:
            case AP_CSV_FILE:
            {
                // These formats are supported for single and multipage files
                retVal = true;
            }
            break;

            default:
            {
                retVal = false;
                GT_ASSERT_EX(false, L"Output file format is not supported!");
            }
            break;
        }

        // If output format is valid, save the new output format settings
        if (retVal)
        {
            _outputFileType = fileType;
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        acRawDataExporter::exportToFile
// Description: Export raw data to file
// Arguments:   filePath - Output file path
// Return Val:  bool - Success / Failure
// Author:      Eran Zinman
// Date:        27/1/2008
// ---------------------------------------------------------------------------
bool acRawDataExporter::exportToFile(const osFilePath& filePath)
{
    bool retVal = false;

    // Sanity check:
    GT_IF_WITH_ASSERT(_pRawDataHandler != NULL)
    {
        // Let check if raw data is multipage.
        int amountOfPages = _pRawDataHandler->amountOfPages();
        bool isMultipage = amountOfPages > 1;

        // Export file according to the defined export file type
        switch (_outputFileType)
        {
            case AP_JPEG_FILE:
            case AP_PNG_FILE:
            case AP_BMP_FILE:
            {
                // Let's just verify that raw data is not multipage
                GT_IF_WITH_ASSERT(!isMultipage)
                {
                    // Export single bitmap to disk
                    retVal = exportSingleBitmap(filePath);
                }
            }
            break;

            case AP_TIFF_FILE:
            {
                // Check if we need to write bitmap as multipage or not
                if (isMultipage)
                {
                    // Export multi-page bitmap to disk
                    retVal = exportMultiPageBitmap(filePath);
                }
                else
                {
                    // Export single bitmap to disk
                    retVal = exportSingleBitmap(filePath);
                }
            }
            break;

            case AP_CSV_FILE:
            {
                // Export spreadsheet data to disk
                retVal = exportSpreadSheet(filePath);
            }
            break;

            default:
            {
                retVal = false;
                GT_ASSERT_EX(false, L"Output file format is not supported!");
            }
            break;
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        acRawDataExporter::exportSingleBitmap
// Description: Export raw data to a single bitmap file
// Arguments:   filePath - Output file path
// Return Val:  bool - Success / Failure
// Author:      Eran Zinman
// Date:        27/1/2008
// ---------------------------------------------------------------------------
bool acRawDataExporter::exportSingleBitmap(const osFilePath& filePath)
{
    bool retVal = false;

    // Sanity check:
    GT_IF_WITH_ASSERT(_pRawDataHandler != NULL)
    {
        // If raw data was loaded successfully:
        GT_IF_WITH_ASSERT(_pRawDataHandler->isOk())
        {
            // Convert file type to free image format
            gtString fileExtension = L"png";
            bool rc1 = convertFileTypeToImageFormat(_outputFileType, fileExtension);
            GT_IF_WITH_ASSERT(rc1)
            {
                osFilePath imagePath = filePath;
                imagePath.setFileExtension(fileExtension);

                // Convert the raw data to Qt:
                QImage* pImage = _pRawDataHandler->convertToQImage();
                GT_IF_WITH_ASSERT(pImage != NULL)
                {
                    // Save the image to disk:
                    retVal = pImage->save(imagePath.asString().asASCIICharArray());

                    // Delete bitmap from memory:
                    delete pImage;
                }
            }
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        acRawDataExporter::convertFileTypeToImageFormat
// Description: Convert a file type to a file extension of the format
// Arguments:   apFileType fileType
//              gtString& fileFormat
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        25/7/2012
// ---------------------------------------------------------------------------
bool acRawDataExporter::convertFileTypeToImageFormat(apFileType fileType, gtString& fileFormat)
{
    bool retVal = true;

    // Get the selected file type
    switch (fileType)
    {
        case AP_BMP_FILE:
            fileFormat = L"bmp";
            break;

        case AP_TIFF_FILE:
            fileFormat = L"tiff";
            break;

        case AP_JPEG_FILE:
            fileFormat = L"jpg";
            break;

        case AP_PNG_FILE:
            fileFormat = L"png";
            break;

        default:
            // Unknown file type:
            retVal = false;
            break;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        acRawDataExporter::exportMultiPageBitmap
// Description: Export multipage raw data to a multipage bitmap file
// Arguments:   filePath - Output file path
// Return Val:  bool - Success / Failure
// Author:      Eran Zinman
// Date:        27/1/2008
// ---------------------------------------------------------------------------
bool acRawDataExporter::exportMultiPageBitmap(const osFilePath& filePath)
{
    GT_UNREFERENCED_PARAMETER(filePath);

    bool retVal = false;

    // Sanity check:
    GT_IF_WITH_ASSERT(_pRawDataHandler != NULL)
    {
        // If raw data was loaded successfully:
        GT_IF_WITH_ASSERT(_pRawDataHandler->isOk())
        {
            // Convert file type to free image format
            gtString fileExtension = L"png";
            bool rc1 = convertFileTypeToImageFormat(_outputFileType, fileExtension);
            GT_IF_WITH_ASSERT(rc1)
            {
                // Make sure output format is TIFF (as only TIFF support multipage bitmap)
                GT_IF_WITH_ASSERT(fileExtension == L"tiff")
                {
#pragma message ("TODO: freeimage: multipage")
                    /*// Allocate a free image multi map:
                    FIMULTIBITMAP* pFreeImageMultiBitmap = FreeImage_OpenMultiBitmap(fif, filePath.asString().asASCIICharArray(), TRUE, FALSE, FALSE);
                    GT_IF_WITH_ASSERT (pFreeImageMultiBitmap != NULL)
                    {
                        // Fill the multipage bitmap
                        retVal = fillMultiPageBitmap(pFreeImageMultiBitmap);
                        GT_ASSERT(retVal);

                        // Close output multipage image
                        BOOL rc2 = FreeImage_CloseMultiBitmap(pFreeImageMultiBitmap);
                        GT_ASSERT (rc2);
                    }*/
                }
            }
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        acRawDataExporter::fillMultiPageBitmap
// Description: Fills a multi page bitmap will raw data handler content
// Arguments:   pQMultiBitmap - Multi page bitmap to fill
// Return Val:  bool - Success / Failure
// Author:      Eran Zinman
// Date:        27/1/2008
// ---------------------------------------------------------------------------
bool acRawDataExporter::fillMultiPageBitmap(QImage* pQMultiBitmap)
{
    bool retVal = false;

    // Sanity check:
    GT_IF_WITH_ASSERT(_pRawDataHandler && pQMultiBitmap)
    {
        retVal = true;

        // Loop through all the pages in the raw data:
        int amountOfPages = _pRawDataHandler->amountOfPages();

        for (int i = 0; i < amountOfPages; i++)
        {
            // This flag indicates of current iteration was successful
            bool isSuccess = false;

            // Set current page as active page
            bool rc1 = _pRawDataHandler->setActivePage(i);
            GT_IF_WITH_ASSERT(rc1)
            {
                // Convert current page in raw data to FreeImage format
                QImage* pCurrentImage = _pRawDataHandler->convertToQImage();
                GT_IF_WITH_ASSERT(pCurrentImage != NULL)
                {
#pragma message ("TODO: freeimage: Multipaget bitmaps!")
                    /*// Fill bitmap vertically (as images are being held upside down in FreeImage):
                    BOOL rc1 = FreeImage_FlipVertical(pDib);
                    GT_IF_WITH_ASSERT (rc1 != FALSE)
                    {
                        // Replace RGBA to BGRA on windows before adding image to multipage bitmap
                        bool rc2 = acImageDataProxy::replaceWindowsRGBAValues(pDib);
                        GT_IF_WITH_ASSERT (rc2)
                        {
                            // Add the current 2D bitmap to the multi bitmap:
                            FreeImage_AppendPage(pQMultiBitmap, pDib);

                            // Flag that current iteration was successful:
                            isSuccess = true;
                        }
                    }*/
                }
            }

            // We return true only if all iterations were successful
            retVal = retVal && isSuccess;
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        acRawDataExporter::exportSingleSpreadSheet
// Description: Export raw data to a single spreadsheet file
// Arguments:   filePath - Output file path
// Return Val:  bool - Success / Failure
// Author:      Eran Zinman
// Date:        27/1/2008
// ---------------------------------------------------------------------------
bool acRawDataExporter::exportSpreadSheet(const osFilePath& filePath)
{
    bool retVal = false;

    // Sanity check:
    GT_IF_WITH_ASSERT(_pRawDataHandler != NULL)
    {
        // Open output file for writing:
        osFile csvFile;
        bool rcOpenCSVFile = csvFile.open(filePath, osChannel::OS_ASCII_TEXT_CHANNEL, osFile::OS_OPEN_TO_WRITE);
        GT_IF_WITH_ASSERT(rcOpenCSVFile)
        {
            retVal = true;

            // Write CSV header to file:
            csvFile << _csvHeader;
            csvFile.flush();

            // Loop through the raw data pages:
            int amountOfPages = _pRawDataHandler->amountOfPages();

            for (int i = 0; i < amountOfPages; i++)
            {
                // Set current page as active page:
                bool rcGetActivePage = _pRawDataHandler->setActivePage(i);
                GT_IF_WITH_ASSERT(rcGetActivePage)
                {
                    // Write current CSV page (only relevant if we have more than one page):
                    if (amountOfPages > 1)
                    {
                        gtString pageSeperator;
                        pageSeperator.append(L"\n*****************************\n");
                        pageSeperator.appendFormattedString(L"Data page: %d / %d", i + 1, amountOfPages);
                        pageSeperator.append(L"\n*****************************\n");

                        csvFile << pageSeperator;
                    }

                    // Write Current raw data page CSV content
                    bool rcCSVPageContent = writeCSVPageContent(csvFile);
                    GT_ASSERT(rcCSVPageContent);

                    retVal = retVal && rcCSVPageContent;
                }
            }

            // We are done - close the CSV output file
            csvFile.close();
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        acRawDataExporter::writeCSVPageContent
// Description: Generates current page content in CSV format
// Arguments:   csvFile - Csv file to write the content into
// Return Val:  bool - Success / Failure
// Author:      Eran Zinman
// Date:        27/1/2008
// ---------------------------------------------------------------------------
bool acRawDataExporter::writeCSVPageContent(osFile& csvFile)
{
    bool retVal = false;

    // Sanity check:
    GT_IF_WITH_ASSERT(_pRawDataHandler != NULL)
    {
        retVal = true;

        // Check if this is a buffer raw data handler:
        if (oaIsBufferTexelFormat(_pRawDataHandler->dataFormat()))
        {
            writeBufferCSVPageContent(csvFile);
        }
        else
        {
            writeNonBufferCSVPageContent(csvFile);
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        acRawDataExporter::writeNonBufferCSVPageContent
// Description: Writes a non buffer CSV page
// Arguments:   osFile& csvFile
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        20/5/2010
// ---------------------------------------------------------------------------
bool acRawDataExporter::writeNonBufferCSVPageContent(osFile& csvFile)
{
    bool retVal = false;

    GT_IF_WITH_ASSERT(_pRawDataHandler != NULL)
    {
        // Get raw data texel format:
        oaTexelDataFormat dataFormat = OA_TEXEL_FORMAT_UNKNOWN;
        oaDataType dataType = OA_BYTE;
        _pRawDataHandler->getDataTypeAndFormat(dataFormat, dataType);

        // Get raw data dimensions:
        int width = 0;
        int height = 0;
        _pRawDataHandler->getSize(width, height);

        // Generate the CSV file columns:
        gtString csvColumns;
        // Add an empty cell at top left corner
        csvColumns.append(L",");

        // First generate the first line - table columns:
        for (int i = 0; i < width; i++)
        {
            csvColumns.appendFormattedString(L"[%d],", i);
        }

        // Write columns to file
        csvFile << csvColumns;

        // Get amount of components in data format
        int amountOfComponents = oaAmountOfTexelFormatComponents(dataFormat);
        GT_IF_WITH_ASSERT(amountOfComponents != -1)
        {
            // Calculate number of rows (which is the data height * amount of components)
            int rowsNum = height * amountOfComponents;

            // Start pouring the data
            for (int y = 0; y < rowsNum; y++)
            {
                // Start a new line
                csvFile << "\n";

                // Write row label which is component name and y position in raw data
                int yPosition = y / amountOfComponents;
                int channelIndex = y % amountOfComponents;

                // Get current channel type
                oaTexelDataFormat channelType = oaGetTexelFormatComponentType(dataFormat, channelIndex);
                GT_IF_WITH_ASSERT(channelType != OA_TEXEL_FORMAT_STENCIL)
                {
                    // Get current channel name
                    gtString channelName;
                    bool rcGetChannelName = oaGetTexelDataFormatName(channelType, channelName);
                    GT_IF_WITH_ASSERT(rcGetChannelName)
                    {
                        // Write channel name and y position as row label
                        gtString csvRowLabel;
                        csvRowLabel.appendFormattedString(L"%ls [%d]: ,", channelName.asCharArray(), yPosition);
                        csvFile << csvRowLabel;

                        for (int x = 0; x < width; x++)
                        {
                            // Get cell value
                            apPixelValueParameter* pParameter;
                            bool isValueAvailable = false;
                            bool rc2 = _pRawDataHandler->getRawDataComponentValue(x, yPosition, channelIndex, pParameter, isValueAvailable);

                            if (rc2)
                            {
                                if (pParameter)
                                {
                                    // Get cell value as string
                                    gtString cellValue;
                                    pParameter->valueAsString(cellValue);

                                    // Write cell value
                                    gtString csvCellValue;
                                    csvCellValue.appendFormattedString(L"%ls,", cellValue.asCharArray());

                                    // Write csv cell value to file
                                    csvFile << csvCellValue;
                                }
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
// Name:        acRawDataExporter::writeBufferCSVPageContent
// Description: Writes a buffer CSV page
// Arguments:   osFile& csvFile
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        20/5/2010
// ---------------------------------------------------------------------------
bool acRawDataExporter::writeBufferCSVPageContent(osFile& csvFile)
{
    bool retVal = false;

    GT_IF_WITH_ASSERT(_pRawDataHandler != NULL)
    {
        // Get raw data texel format:
        oaTexelDataFormat dataFormat = OA_TEXEL_FORMAT_UNKNOWN;
        oaDataType dataType = OA_BYTE;
        _pRawDataHandler->getDataTypeAndFormat(dataFormat, dataType);

        // Get raw data dimensions:
        int width = 0;
        int height = 0;
        _pRawDataHandler->getSize(width, height);

        // Get the amount of cells:
        int amountOfCells = oaAmountOfTexelFormatComponents(dataFormat);

        // Calculate the size of one raw:
        int oneRowSize = 0;

        for (int x = 0; x < amountOfCells; x++)
        {
            // Get current channel type
            oaTexelDataFormat channelType = oaGetTexelFormatComponentType(dataFormat, x);
            GT_IF_WITH_ASSERT(channelType != OA_TEXEL_FORMAT_STENCIL)
            {
                // Get the current data type
                dataType = oaGetTexelFormatDataType(dataFormat, x);
                oneRowSize += oaSizeOfDataType(dataType);
            }
        }

        // Calculate the amount of rows:
        float amountOfRowsAsFloat = (float)width / (float)oneRowSize;
        int amountOfRows = (int)(ceill(amountOfRowsAsFloat));

        // Generate the CSV file columns:
        gtString csvColumns;
        // Add an empty cell at top left corner
        csvColumns.append(L",");

        // First generate the first line - table columns:
        for (int i = 0; i < amountOfCells; i++)
        {
            csvColumns.appendFormattedString(L"[%d],", i);
        }

        // Write columns to file
        csvFile << csvColumns;

        // Start pouring the data:
        for (int y = 0; y < amountOfRows; y++)
        {
            // Start a new line
            csvFile << "\n";

            gtString csvRowLabel;
            csvRowLabel.appendFormattedString(L"[%d]: ,", y);
            csvFile << csvRowLabel;

            for (int x = 0; x < amountOfCells; x++)
            {
                // Get current channel type
                oaTexelDataFormat channelType = oaGetTexelFormatComponentType(dataFormat, x);
                GT_IF_WITH_ASSERT(channelType != OA_TEXEL_FORMAT_STENCIL)
                {
                    // Get the current data type
                    dataType = oaGetTexelFormatDataType(dataFormat, x);

                    // Get the raw data value:
                    apPixelValueParameter* pParameter = NULL;
                    bool rcRawDataValue = _pRawDataHandler->getBufferRawDataComponentValue(y, x, pParameter);

                    if (rcRawDataValue)
                    {
                        if (pParameter)
                        {
                            // Get cell value as string
                            gtString cellValue;
                            pParameter->valueAsString(cellValue);

                            // Write cell value
                            gtString csvCellValue;
                            csvCellValue.appendFormattedString(L"%ls,", cellValue.asCharArray());

                            // Write csv cell value to file
                            csvFile << csvCellValue;
                        }
                    }
                }
            }
        }
    }

    return retVal;
}



