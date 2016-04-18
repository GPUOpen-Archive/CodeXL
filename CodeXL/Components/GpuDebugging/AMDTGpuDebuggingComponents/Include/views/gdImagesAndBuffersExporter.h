//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gdImagesAndBuffersExporter.h
///
//==================================================================================

//------------------------------ gdImagesAndBuffersExporter.h ------------------------------

#ifndef __GDIMAGESANDBUFFERSEXPORTER
#define __GDIMAGESANDBUFFERSEXPORTER

// Forward decelerations:
struct FIBITMAP;

// Infra:
#include <AMDTApplicationComponents/Include/acRawFileHandler.h>
#include <AMDTBaseTools/Include/gtPtrVector.h>
#include <AMDTOSWrappers/Include/osDirectory.h>
#include <AMDTAPIClasses/Include/apGLTexture.h>
#include <AMDTAPIClasses/Include/apContextID.h>
#include <AMDTAPIClasses/Include/apFileType.h>

// Local:
#include <AMDTGpuDebuggingComponents/Include/dialogs/gdDoubleProgressBarDialog.h>


class apStaticBuffer;
class apGLVBO;
class apCLBuffer;
class apCLSubBuffer;
class apGLTexture;

// Indicates for each file if it was written successfully, failed or already exist
enum gdExportedFileStatus
{
    GD_EXPORT_FILE_SUCCESS,   // File was written successfully
    GD_EXPORT_FILE_FAILED,    // File was failed to be saved
    GD_EXPORT_FILE_EXIST      // File already exists - no change was made
};


// This struct holds information about each file that was written:
struct gdFileExporterOutputResult
{
    // Output file full path (directory + file name + extension):
    osFilePath filePath;

    // Output file status (success / failed / already exist):
    gdExportedFileStatus fileStatus;
};

// This struct holds information about single texture identifier:
struct gdExportedTextureID
{
    apContextID _contextID;
    apGLTextureMipLevelID _textureId;
};
// ----------------------------------------------------------------------------------
// Class Name:           gdImagesAndBuffersExporter
// General Description:  This class writes texture data to disk in an image or
//                       spreadsheet format
// Author:               Eran Zinman
// Creation Date:        25/1/2008
// ----------------------------------------------------------------------------------
class GD_API gdImagesAndBuffersExporter
{
public:
    // Constructor:
    gdImagesAndBuffersExporter(apContextID contextID);

    // Destructor:
    ~gdImagesAndBuffersExporter();

public:
    // Sets the output directory to save the files in:
    bool setOutputDirectory(const osDirectory& outputDir);

    // Sets the output filename to save the file in:
    void setOutputFileName(const gtString& fileName);

    // Enable / Disable file overwrite:
    void enableFileOverwrite(bool isAllowed) { _shouldOverwrite = isAllowed; };

public:
    // Export functions:
    bool exportTexture(gdExportedTextureID exportedTextureID, apFileType fileType, bool useDirtyMechanism = false);
    bool exportStaticBuffer(apDisplayBuffer bufferType, apFileType fileType);
    bool exportPBuffer(int pbufferID, apDisplayBuffer bufferType, int pbufferContextID, apFileType fileType);
    bool exportVBO(GLuint vboName, apFileType fileType);
    bool exportCLBuffer(int clBufferIndex, apFileType fileType);
    bool exportCLSubBuffer(int clBufferIndex, apFileType fileType);

public:
    // Return a copy of the vector containing all exported files and their export result:
    void getExportResult(gtPtrVector<gdFileExporterOutputResult*>& exportResult);

private:
    // Generate output file path:
    bool generateOutputFilePath(osFilePath& outputFilePath, gtString fileName);

    // Adds a file to the output results vector:
    void addFileToOutputVector(const osFilePath& filePath, gdExportedFileStatus status);

    // Export raw data file to disk and record result in output vector:
    bool exportFileToDisk(acRawFileHandler* pRawDataHandler, const osFilePath& outputFilePath);

    // Export static buffer to disk
    bool exportStaticBufferToDisk(const apStaticBuffer& staticBufferDetails);

    // Export VBO to disk:
    bool exportVBOToDisk(const apGLVBO& vboDetails);

    // Export OpenCL buffer to disk:
    bool exportCLBufferToDisk(const apCLBuffer& bufferDetails);

    // Export OpenCL sub-buffer to disk:
    bool exportCLSubBufferToDisk(const apCLSubBuffer& subBufferDetails);

    // Generates a CSV file header:
    void generateCSVFileHeader(acRawFileHandler* pRawDataHandler, gtString& fileHeader);

    bool exportGLTexture(gdExportedTextureID exportedTextureID, apFileType fileType, bool useDirtyMechanism);
    bool exportCLImage(gdExportedTextureID exportedTextureID, apFileType fileType);

private:
    // Should we overwrite existing files?
    bool _shouldOverwrite;

    // Default output directory:
    osDirectory _outputDir;

    // Output file name:
    gtString _outputFileName;

    // Export file type:
    apFileType _fileType;

    // Currently active context:
    apContextID _activeContext;

    // Are we in GL begin end block:
    bool _isInGLBeginEndBlock;

    // Holds information of all files that were written
    gtPtrVector<gdFileExporterOutputResult*> _outputResult;

    // We can hold either a double slider or single slider dialogs which we will update
    gdDoubleProgressBarDialog* _pDoubleProgressDialog;
};


#endif  // __gdTexturesAndBuffersExporter
