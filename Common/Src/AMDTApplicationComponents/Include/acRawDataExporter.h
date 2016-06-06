//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file acRawDataExporter.h
///
//==================================================================================

//------------------------------ acRawDataExporter.h ------------------------------

#ifndef __ACRAWDATAEXPORTER
#define __ACRAWDATAEXPORTER

// Infra:
#include <AMDTAPIClasses/Include/apFileType.h>
#include <AMDTOSWrappers/Include/osFilePath.h>
#include <AMDTOSWrappers/Include/osFile.h>

// Local:
#include <AMDTApplicationComponents/Include/acRawFileHandler.h>


// ----------------------------------------------------------------------------------
// Class Name:           acRawDataExporter
// General Description:
//                       This class writes openGL raw data in two formats:
//                       1. Image format - either JPEG, TIFF, PNG or BMP;
//                       2. SpreadSheet format - CSV file.
//
// Author:               Eran Zinman
// Creation Date:        25/1/2008
// ----------------------------------------------------------------------------------
class AC_API acRawDataExporter
{
public:
    // Constructor:
    acRawDataExporter(acRawFileHandler* pRawDataHandler);

    // Destructor:
    virtual ~acRawDataExporter();

public:
    // Sets the output format to save the raw data in
    bool setOutputFormat(apFileType fileType);

    // Sets the header for the CSV file
    void setCSVHeader(const gtString& CSVHeader) { _csvHeader = CSVHeader; };

    // Export raw data to file
    bool exportToFile(const osFilePath& filePath);

public:
    // Converts an apFileType to image format:
    bool static convertFileTypeToImageFormat(apFileType fileType, gtString& fileFormat);

private:
    // Various export to disk functions:
    bool exportSingleBitmap(const osFilePath& filePath);
    bool exportMultiPageBitmap(const osFilePath& filePath);
    bool exportSpreadSheet(const osFilePath& filePath);

private:
    // Fills a multipage bitmap will raw data handler content:
    bool fillMultiPageBitmap(QImage* pImageMultiBitmap);

    // Generates current page content in csv format
    bool writeCSVPageContent(osFile& csvFile);
    bool writeBufferCSVPageContent(osFile& csvFile);
    bool writeNonBufferCSVPageContent(osFile& csvFile);


private:
    // Raw data that should be written
    acRawFileHandler* _pRawDataHandler;

    // The file format to save the file in
    apFileType _outputFileType;

    // The header for the CSV file
    gtString _csvHeader;


};


#endif  // __acRawDataExporter
