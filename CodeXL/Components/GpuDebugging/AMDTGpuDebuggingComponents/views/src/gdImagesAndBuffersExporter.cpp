//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gdImagesAndBuffersExporter.cpp
///
//==================================================================================

//------------------------------ gdImagesAndBuffersExporter.cpp ------------------------------

// Qt
#include <QtWidgets>




// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTBaseTools/Include/gtString.h>
#include <AMDTOSWrappers/Include/osDebuggingFunctions.h>
#include <AMDTOSWrappers/Include/osFile.h>
#include <AMDTOSWrappers/Include/osOSDefinitions.h>
#include <AMDTOSWrappers/Include/osStopWatch.h>
#include <AMDTOSWrappers/Include/osTime.h>
#include <AMDTAPIClasses/Include/apGLTexture.h>
#include <AMDTAPIClasses/Include/apGLVBO.h>
#include <AMDTAPIClasses/Include/apCLImage.h>
#include <AMDTAPIClasses/Include/apCLBuffer.h>
#include <AMDTAPIClasses/Include/apCLSubBuffer.h>
#include <AMDTAPIClasses/Include/apStaticBuffer.h>
#include <AMDTApiFunctions/Include/gaGRApiFunctions.h>
#include <AMDTApplicationComponents/Include/acRawDataExporter.h>
#include <AMDTApplicationComponents/Include/acRawFileHandler.h>

// AMDTApplicationFramework:
#include <AMDTApplicationFramework/Include/afAppStringConstants.h>
#include <AMDTApplicationFramework/Include/afProgressBarWrapper.h>
#include <AMDTApplicationFramework/Include/afProjectManager.h>

// Local:
#include <AMDTGpuDebuggingComponents/Include/gdAidFunctions.h>
#include <AMDTGpuDebuggingComponents/Include/gdGDebuggerGlobalVariablesManager.h>
#include <AMDTGpuDebuggingComponents/Include/gdStringConstants.h>
#include <AMDTGpuDebuggingComponents/Include/views/gdImagesAndBuffersExporter.h>

// Texture cube map name extension
static gtString cubeMapNameExtension[apGLTextureMipLevel::AP_MAX_AMOUNT_OF_TEXTURE_FACES] = {L"_PositiveX", L"_NegativeX", L"_PositiveY", L"_NegativeY", L"_PositiveZ", L"_NegativeZ"};

// ---------------------------------------------------------------------------
// Name:        gdImagesAndBuffersExporter::gdImagesAndBuffersExporter
// Description: Constructor
// Author:      Eran Zinman
// Date:        27/1/2007
// ---------------------------------------------------------------------------
gdImagesAndBuffersExporter::gdImagesAndBuffersExporter(apContextID contextID)
    : _shouldOverwrite(true), _outputDir(osFilePath(AF_STR_Empty)), _outputFileName(AF_STR_Empty), _fileType(AP_PNG_FILE), _activeContext(contextID),
      _isInGLBeginEndBlock(false), _pDoubleProgressDialog(NULL)
{
    // Check if the debugged process was stopped in a glBegin-glEnd block
    _isInGLBeginEndBlock = gaIsInOpenGLBeginEndBlock(_activeContext._contextId);
}

// ---------------------------------------------------------------------------
// Name:        gdImagesAndBuffersExporter::~gdImagesAndBuffersExporter
// Description: Destructor
// Author:      Eran Zinman
// Date:        27/1/2007
// ---------------------------------------------------------------------------
gdImagesAndBuffersExporter::~gdImagesAndBuffersExporter()
{

}

// ---------------------------------------------------------------------------
// Name:        gdImagesAndBuffersExporter::setOutputDirectory
// Description: Sets the output directory to save the files in
// Arguments:   outputDir - Output directory to save the files in
// Return Val:  bool - Success / Failure
// Author:      Eran Zinman
// Date:        27/1/2007
// ---------------------------------------------------------------------------
bool gdImagesAndBuffersExporter::setOutputDirectory(const osDirectory& outputDir)
{
    bool retVal = false;

    // Check if output directory exists:
    bool rc1 = outputDir.exists();
    GT_IF_WITH_ASSERT(rc1)
    {
        // Set the new output directory
        _outputDir = outputDir;

        retVal = true;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gdImagesAndBuffersExporter::exportGLTexture
// Description: Exports a single texture
// Arguments:   gdExportedTextureID exportedTextureID
//              apFileType fileType
//              bool useDirtyMechanism
// Return Val:  bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        1/6/2010
// ---------------------------------------------------------------------------
bool gdImagesAndBuffersExporter::exportTexture(gdExportedTextureID exportedTextureID, apFileType fileType, bool useDirtyMechanism)
{
    bool retVal = false;

    if (exportedTextureID._contextID.isOpenGLContext())
    {
        // Export the OpenGL texture:
        retVal = exportGLTexture(exportedTextureID, fileType, useDirtyMechanism);
    }

    else if (exportedTextureID._contextID.isOpenCLContext())
    {
        // Export the OpenCL image:
        retVal = exportCLImage(exportedTextureID, fileType);
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gdImagesAndBuffersExporter::exportGLTexture
// Description: Exports an OpenGL texture
// Arguments: gdExportedTextureID exportedTextureID
//            apFileType fileType
//            bool useDirtyMechanism
// Return Val: bool  - Success / failure.
// Author:      Yaki Tebeka
// Date:        1/6/2010
// ---------------------------------------------------------------------------
bool gdImagesAndBuffersExporter::exportGLTexture(gdExportedTextureID exportedTextureID, apFileType fileType, bool useDirtyMechanism)
{
    bool retVal = false;

    // Clear result vector
    _outputResult.deleteElementsAndClear();

    // Extract the texture raw data to disk
    // TO_DO: texture mip levels - optimization
    gtVector<apGLTextureMipLevelID> texturesVector;
    texturesVector.push_back(exportedTextureID._textureId);

    // Update the texture raw data:
    bool rc1 = gaUpdateTextureRawData(exportedTextureID._contextID._contextId, texturesVector);
    GT_IF_WITH_ASSERT(rc1)
    {
        // Update current item progress to 25%:
        afProgressBarWrapper::instance().incrementProgressBar();

        // Get the texture details:
        apGLTexture textureDetails;
        bool rc2 = gaGetTextureObjectDetails(exportedTextureID._contextID._contextId, exportedTextureID._textureId._textureName, textureDetails);
        GT_IF_WITH_ASSERT(rc2)
        {
            // If texture is 3D texture, and we want to save texture as image,
            // make sure we are saving the file as a tiff
            apTextureType textureType = textureDetails.textureType();

            if ((textureType == AP_3D_TEXTURE) || (textureType == AP_1D_ARRAY_TEXTURE) || (textureType == AP_2D_ARRAY_TEXTURE))
            {
                // Are we trying to save texture as an image?
                if ((fileType == AP_JPEG_FILE) || (fileType == AP_PNG_FILE) || (fileType == AP_BMP_FILE))
                {
                    // 3D textures images are always stored as tiff format
                    fileType = AP_TIFF_FILE;
                }
            }

            // Save the export file type
            _fileType = fileType;

            // Get amount of texture elements:
            int amountOfIndices = textureDetails.amountOfTextureDataFiles();

            // This flag will indicate if *all* textures images were exported successfuly or not:
            bool allExportSuccess = true;

            // Loop through the texture elements:
            for (int i = 0; i < amountOfIndices; i++)
            {
                // This flag indicates if current texture element was exported successfully:
                bool isSuccess = false;

                // Get texture element file name:
                // Get the current texture data file path:
                osFilePath textureFile;
                bool rc0 = textureDetails.getTextureDataFilePath(textureFile, i);
                GT_ASSERT(rc0);

                // Localize the path as needed:
                gaRemoteToLocalFile(textureFile, false);

                // Sets the output exported file name:
                gtString outputFileName = _outputFileName;

                // If we didn't chose a specific filename
                rc1 = true;

                if (outputFileName.isEmpty())
                {
                    rc1 = textureFile.getFileName(outputFileName);
                }
                else
                {
                    // For cube map textures - add Cube pane information
                    if (textureType == AP_CUBE_MAP_TEXTURE)
                    {
                        // Add cube map name extension
                        outputFileName.append(cubeMapNameExtension[i]);
                    }
                }

                // If we didn't specify an output directory, just use the texture default output directory
                osFilePath outputDir = _outputDir.directoryPath();

                if (outputDir.asString().isEmpty())
                {
                    textureFile.getFileDirectory(_outputDir);
                }

                GT_IF_WITH_ASSERT(rc1)
                {
                    // Generate full output file path (directory + filename + extension)
                    osFilePath outputFilePath;
                    rc2 = generateOutputFilePath(outputFilePath, outputFileName);
                    GT_IF_WITH_ASSERT(rc2)
                    {
                        // This flag will indicate if we need to update the texture image or not
                        bool updatedIsRequired = true;

                        // If we can use the textures dirty mechanism, and not texture element is dirty, we don't need to update
                        if (useDirtyMechanism)
                        {
                            if (!textureDetails.dirtyTextureImageExists(0))
                            {
                                // Let's just verify finally that we actually have the file that we already generated
                                if (outputFilePath.isRegularFile())
                                {
                                    updatedIsRequired = false;
                                    retVal = true;
                                }
                            }
                        }

                        // Do we need to update the textures images?
                        if (updatedIsRequired)
                        {
                            // Load raw file into memory
                            acRawFileHandler pRawFileHandler;
                            bool rc3 = pRawFileHandler.loadFromFile(textureFile);
                            GT_IF_WITH_ASSERT(rc3)
                            {
                                // If raw data was loaded successfully
                                GT_IF_WITH_ASSERT(pRawFileHandler.isOk())
                                {
                                    // Export file to disk, and add export result to result vector
                                    isSuccess = exportFileToDisk(&pRawFileHandler, outputFilePath);
                                }
                            }

                            // If file was not exported successfully
                            if (!isSuccess)
                            {
                                // Add file to the result vector with a flag that it wasn't exported successfully
                                addFileToOutputVector(outputFilePath, GD_EXPORT_FILE_FAILED);

                                // Flag that not all image export went smoothly
                                allExportSuccess = false;
                            }

                            // We return true even if we failed, because output result vector will contain the results
                            retVal = true;
                        }
                    }
                }
            }

            // Only if all images exports were done successfully, we need to flag all textures images as not dirty
            if (useDirtyMechanism)
            {
                if (allExportSuccess)
                {
                    bool rc4 = gaMarkAllTextureImagesAsUpdated(exportedTextureID._contextID._contextId, exportedTextureID._textureId._textureName);
                    GT_ASSERT(rc4);
                }
            }
        }
    }

    afProgressBarWrapper::instance().incrementProgressBar();

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gdImagesAndBuffersExporter::exportCLImage
// Description: Exports an OpenCL image
// Arguments:   gdExportedTextureID exportedTextureID
//              apFileType fileType
//              bool useDirtyMechanism
// Return Val:  bool  - Success / failure.
// Author:      Yaki Tebeka
// Date:        1/6/2010
// ---------------------------------------------------------------------------
bool gdImagesAndBuffersExporter::exportCLImage(gdExportedTextureID exportedTextureID, apFileType fileType)
{
    bool retVal = false;

    // Clear result vector
    _outputResult.deleteElementsAndClear();

    // Extract the texture raw data to disk:
    gtVector<int> texturesVector;
    texturesVector.push_back(exportedTextureID._textureId._textureName);

    // Update the texture raw data:
    bool rc1 = gaUpdateOpenCLImageRawData(exportedTextureID._contextID._contextId, texturesVector);
    GT_IF_WITH_ASSERT(rc1)
    {
        afProgressBarWrapper::instance().incrementProgressBar();

        // Get the texture details:
        apCLImage textureDetails;
        bool rc2 = gaGetOpenCLImageObjectDetails(exportedTextureID._contextID._contextId, exportedTextureID._textureId._textureName, textureDetails);
        GT_IF_WITH_ASSERT(rc2)
        {
            // If texture is 3D texture, and we want to save texture as image,
            // make sure we are saving the file as a tiff
            apTextureType textureType = textureDetails.imageType();

            if ((textureType == AP_3D_TEXTURE) || (textureType == AP_1D_ARRAY_TEXTURE) || (textureType == AP_2D_ARRAY_TEXTURE))
            {
                // Are we trying to save texture as an image?
                if ((fileType == AP_JPEG_FILE) || (fileType == AP_PNG_FILE) || (fileType == AP_BMP_FILE))
                {
                    // 3D textures images are always stored as tiff format
                    fileType = AP_TIFF_FILE;
                }
            }

            // Save the export file type:
            _fileType = fileType;

            // This flag indicates if current texture element was exported successfully:
            bool isSuccess = false;

            // Get the texture file path:
            osFilePath textureFile;
            textureDetails.imageFilePath(textureFile);

            // Localize the path as needed:
            gaRemoteToLocalFile(textureFile, false);

            // Sets the output exported file name:
            gtString outputFileName = _outputFileName;

            // If we didn't choose a specific filename:
            rc1 = true;

            if (outputFileName.isEmpty())
            {
                rc1 = textureFile.getFileName(outputFileName);
            }

            // If we didn't specify an output directory, just use the texture default output directory
            osFilePath outputDir = _outputDir.directoryPath();

            if (outputDir.asString().isEmpty())
            {
                textureFile.getFileDirectory(_outputDir);
            }

            GT_IF_WITH_ASSERT(rc1)
            {
                // Generate full output file path (directory + filename + extension)
                osFilePath outputFilePath;
                rc2 = generateOutputFilePath(outputFilePath, outputFileName);
                GT_IF_WITH_ASSERT(rc2)
                {
                    // This flag will indicate if we need to update the texture image or not:
                    bool updatedIsRequired = true;

                    // Do we need to update the textures images?
                    if (updatedIsRequired)
                    {
                        // Load raw file into memory
                        acRawFileHandler pRawFileHandler;
                        bool rc3 = pRawFileHandler.loadFromFile(textureFile);
                        GT_IF_WITH_ASSERT(rc3)
                        {
                            // If raw data was loaded successfully
                            GT_IF_WITH_ASSERT(pRawFileHandler.isOk())
                            {
                                // Export file to disk, and add export result to result vector:
                                isSuccess = exportFileToDisk(&pRawFileHandler, outputFilePath);
                            }
                        }

                        // If file was not exported successfully
                        if (!isSuccess)
                        {
                            // Add file to the result vector with a flag that it wasn't exported successfully
                            addFileToOutputVector(outputFilePath, GD_EXPORT_FILE_FAILED);
                        }

                        // We return true even if we failed, because output result vector will contain the results
                        retVal = true;
                    }
                }
            }
        }
    }

    afProgressBarWrapper::instance().incrementProgressBar();

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gdImagesAndBuffersExporter::getExportResult
// Description: Return a copy of the vector containing all exported files
//              and their export result
// Arguments:   exportResult - Output copied export result vector
// Author:      Eran Zinman
// Date:        27/1/2007
// ---------------------------------------------------------------------------
void gdImagesAndBuffersExporter::getExportResult(gtPtrVector<gdFileExporterOutputResult*>& exportResult)
{
    // First clear output vector
    exportResult.deleteElementsAndClear();

    // Get amount of indicies
    int amountOfIndicies = _outputResult.size();

    // Loop through the source vector items
    for (int i = 0; i < amountOfIndicies; i++)
    {
        // Get output result object
        gdFileExporterOutputResult* pOutputResult = _outputResult[i];

        if (pOutputResult != NULL)
        {
            // Add output result object to output vector
            exportResult.push_back(pOutputResult);
        }
    }

    // Clear the output result vector, but don't delete the pointers!
    _outputResult.clear();
}

// ---------------------------------------------------------------------------
// Name:        gdImagesAndBuffersExporter::exportStaticBuffer
// Description: Export a static buffer using the given file type
// Arguments:   bufferType - The static buffer to export
//              fileType - Export output file type
// Return Val:  bool - Success / Failure
// Author:      Eran Zinman
// Date:        27/1/2007
// ---------------------------------------------------------------------------
bool gdImagesAndBuffersExporter::exportStaticBuffer(apDisplayBuffer bufferType, apFileType fileType)
{
    bool retVal = false;

    // Clear result vector
    _outputResult.deleteElementsAndClear();

    // Save export file type
    _fileType = fileType;

    // Extract the static buffer raw data to disk
    bool rc1 = gaUpdateStaticBufferRawData(_activeContext._contextId, bufferType);
    GT_IF_WITH_ASSERT(rc1)
    {
        // Get the static buffer details:
        apStaticBuffer staticBufferDetails;
        rc1 = gaGetStaticBufferObjectDetails(_activeContext._contextId, bufferType, staticBufferDetails);
        GT_IF_WITH_ASSERT(rc1)
        {
            // Export static buffer to disk
            retVal = exportStaticBufferToDisk(staticBufferDetails);
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gdImagesAndBuffersExporter::Export static buffer to disk
// Description: Exports a static buffer to disk
// Arguments:   staticBufferDertails - static buffer details
// Return Val:  bool - Success / Failure
// Author:      Eran Zinman
// Date:        27/1/2007
// ---------------------------------------------------------------------------
bool gdImagesAndBuffersExporter::exportStaticBufferToDisk(const apStaticBuffer& staticBufferDetails)
{
    bool retVal = false;

    afProgressBarWrapper::instance().incrementProgressBar();

    // Get static buffer type
    apDisplayBuffer bufferType = staticBufferDetails.bufferType();

    // Get the static buffer filename
    osFilePath bufferFile;
    staticBufferDetails.getBufferFilePath(bufferFile);

    // Localize the path as needed:
    gaRemoteToLocalFile(bufferFile, false);

    // Sets the output exported file name
    gtString outputFileName = _outputFileName;

    // If we didn't chose a specific filename
    bool rc1 = true;

    if (outputFileName.isEmpty())
    {
        rc1 = bufferFile.getFileName(outputFileName);
    }

    GT_IF_WITH_ASSERT(rc1)
    {
        // Generate output file name
        osFilePath outputFilePath;
        bool rc2 = generateOutputFilePath(outputFilePath, outputFileName);
        GT_IF_WITH_ASSERT(rc2)
        {
            // This flag indicates if file was exported successfully
            bool isSuccess = false;

            // Load raw file into memory
            acRawFileHandler pRawFileHandler;
            rc2 = pRawFileHandler.loadFromFile(bufferFile);
            GT_IF_WITH_ASSERT(rc2)
            {
                // If raw data was loaded successfully
                GT_IF_WITH_ASSERT(pRawFileHandler.isOk())
                {
                    // Get buffer data type:
                    oaTexelDataFormat bufferDataFormat;
                    oaDataType bufferDataType;
                    staticBufferDetails.getBufferFormat(bufferDataFormat, bufferDataType);

                    // For now we normalize the values only for DEPTH and STENCIL buffers:
                    bool normalizeValues = ((bufferType == AP_DEPTH_BUFFER) || (bufferType == AP_STENCIL_BUFFER));

                    if (normalizeValues)
                    {
                        bool rc3 = pRawFileHandler.normalizeValues(bufferDataFormat);
                        GT_ASSERT(rc3);
                    }

                    // Export file to disk, and add export result to result vector
                    isSuccess = exportFileToDisk(&pRawFileHandler, outputFilePath);
                }
            }

            // If file was not exported successfully
            if (!isSuccess)
            {
                // Add file to the result vector with a flag that it wasn't exported successfully
                addFileToOutputVector(outputFilePath, GD_EXPORT_FILE_FAILED);
            }

            // We return true even if we failed, because output result vector will contain the results
            retVal = true;
        }
    }

    afProgressBarWrapper::instance().incrementProgressBar();
    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gdImagesAndBuffersExporter::exportVBOToDisk
// Description: Exports a VBO to disk
// Arguments:   const apGLVBO& vboDetails
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        17/5/2010
// ---------------------------------------------------------------------------
bool gdImagesAndBuffersExporter::exportVBOToDisk(const apGLVBO& vboDetails)
{
    bool retVal = false;

    // Get the VBO filename:
    osFilePath bufferFile;
    vboDetails.getBufferFilePath(bufferFile);

    // Localize the path as needed:
    gaRemoteToLocalFile(bufferFile, false);

    afProgressBarWrapper::instance().incrementProgressBar();

    // Sets the output exported file name:
    gtString outputFileName = _outputFileName;

    // If we didn't chose a specific filename
    bool rc1 = true;

    if (outputFileName.isEmpty())
    {
        rc1 = bufferFile.getFileName(outputFileName);
    }

    GT_IF_WITH_ASSERT(rc1)
    {
        // Generate output file name
        osFilePath outputFilePath;
        bool rc2 = generateOutputFilePath(outputFilePath, outputFileName);
        GT_IF_WITH_ASSERT(rc2)
        {
            // This flag indicates if file was exported successfully
            bool isSuccess = false;

            // Load raw file into memory
            acRawFileHandler rawFileHandler;
            rc2 = rawFileHandler.loadFromFile(bufferFile);
            GT_IF_WITH_ASSERT(rc2)
            {
                // If raw data was loaded successfully
                GT_IF_WITH_ASSERT(rawFileHandler.isOk())
                {
                    // Set the buffer display properties:
                    rawFileHandler.setBufferDataFormat(vboDetails.displayFormat());

                    // Export file to disk, and add export result to result vector
                    isSuccess = exportFileToDisk(&rawFileHandler, outputFilePath);
                }
            }

            // If file was not exported successfully
            if (!isSuccess)
            {
                // Add file to the result vector with a flag that it wasn't exported successfully
                addFileToOutputVector(outputFilePath, GD_EXPORT_FILE_FAILED);
            }

            // We return true even if we failed, because output result vector will contain the results
            retVal = true;
        }
    }

    afProgressBarWrapper::instance().incrementProgressBar();

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gdImagesAndBuffersExporter::exportPBuffer
// Description: Export a PBuffer using the given file type
// Arguments:   pbufferID - The PBuffer ID
//              bufferType - The static buffer to export
//              pbufferContextID - The PBuffer spy context id
//              fileType - Export output file type
// Return Val:  bool - Success / Failure
// Author:      Eran Zinman
// Date:        27/1/2007
// ---------------------------------------------------------------------------
bool gdImagesAndBuffersExporter::exportPBuffer(int pbufferID, apDisplayBuffer bufferType, int pbufferContextID, apFileType fileType)
{
    bool retVal = false;

    // Clear result vector
    _outputResult.deleteElementsAndClear();

    // Sanity check:
    GT_IF_WITH_ASSERT(pbufferContextID > 0)
    {
        // Save export file type
        _fileType = fileType;

        // Extract the PBuffer raw data to disk
        bool rc1 = gaUpdatePBufferStaticBufferRawData(pbufferContextID, pbufferID, bufferType);
        GT_IF_WITH_ASSERT(rc1)
        {
            // Get the PBuffer static buffer details:
            apStaticBuffer staticBufferDetails;
            bool rc2 = gaGetPBufferStaticBufferObjectDetails(pbufferID, bufferType, staticBufferDetails);
            GT_IF_WITH_ASSERT(rc2)
            {
                // Export static buffer to disk
                retVal = exportStaticBufferToDisk(staticBufferDetails);
            }
        }
    }

    return retVal;
}
// ---------------------------------------------------------------------------
// Name:        gdImagesAndBuffersExporter::exportFileToDisk
// Description: Export raw data file to disk and record result in output vector
// Arguments:   pRawDataHandler - Raw data to export
//              outputFilePath - Output file path
// Author:      Eran Zinman
// Date:        27/1/2007
// ---------------------------------------------------------------------------
bool gdImagesAndBuffersExporter::exportFileToDisk(acRawFileHandler* pRawDataHandler, const osFilePath& outputFilePath)
{
    bool retVal = false;

    // Sanity check:
    GT_IF_WITH_ASSERT(pRawDataHandler != NULL)
    {
        // If we are not allowed to overwrite files, check if file exists:
        if ((!_shouldOverwrite) && (outputFilePath.isRegularFile()))
        {
            // File exist and won't be exported
            addFileToOutputVector(outputFilePath, GD_EXPORT_FILE_EXIST);

            retVal = true;
        }
        else
        {
            // This flag indicates if export was successful
            bool isSuccess = false;

            // Export file to disk
            acRawDataExporter rawDataExporter(pRawDataHandler);

            // If we are exporting a csv file, generate an header for it
            if (_fileType == AP_CSV_FILE)
            {
                // Generate a CSV header
                gtString CSVHeader;
                generateCSVFileHeader(pRawDataHandler, CSVHeader);

                // Set CSV for the file exporter
                rawDataExporter.setCSVHeader(CSVHeader);
            }

            // Set exporter output format
            bool rc3 = rawDataExporter.setOutputFormat(_fileType);
            GT_IF_WITH_ASSERT(rc3)
            {
                // Export file to disk
                isSuccess = rawDataExporter.exportToFile(outputFilePath);

                if (isSuccess)
                {
                    // File was exported successfully
                    addFileToOutputVector(outputFilePath, GD_EXPORT_FILE_SUCCESS);

                    retVal = true;
                }
            }
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gdImagesAndBuffersExporter::generateCSVFileHeader
// Description: Generates a CSV file header
// Arguments:   pRawDataHandler - Raw data to generate the CSV header for
//              fileHeader - Output file header
// Author:      Eran Zinman
// Date:        27/1/2008
// ---------------------------------------------------------------------------
void gdImagesAndBuffersExporter::generateCSVFileHeader(acRawFileHandler* pRawDataHandler, gtString& fileHeader)
{
    // Empty file header
    fileHeader.makeEmpty();

    // Sanity check:
    GT_IF_WITH_ASSERT(pRawDataHandler != NULL)
    {
        // File save time and date
        gtString fileSavedDate;
        gtString fileSavedTime;
        {
            // Get current date and time
            osTime fileSavedDateAndTime;
            fileSavedDateAndTime.setFromCurrentTime();

            // Store date and time values
            fileSavedDateAndTime.dateAsString(fileSavedDate, osTime::UNDERSCORE_SAPERATOR, osTime::LOCAL);
            fileSavedDateAndTime.timeAsString(fileSavedTime, osTime::WINDOWS_STYLE, osTime::LOCAL);
        }

        // Get the current project name:
        gtString projectName = afProjectManager::instance().currentProjectSettings().projectName();

        // Get the raw data dimensions:
        gtString rawDataDimensions;
        {
            // Get raw data size
            int width = 0;
            int height = 0;
            pRawDataHandler->getSize(width, height);

            // Generate raw data dimensions string
            rawDataDimensions.appendFormattedString(GD_STR_RawDataExportFileDataDimensions, width, height);
        }

        // Get raw data data format and data type:
        gtString rawDataFormat;
        gtString rawDataType;
        {
            // Get raw data properties:
            oaTexelDataFormat dataFormat = OA_TEXEL_FORMAT_UNKNOWN;
            oaDataType dataType = OA_BYTE;
            pRawDataHandler->getDataTypeAndFormat(dataFormat, dataType);

            // Get texture format and texel type in OpenGL format:
            GLenum pixelFormat = oaTexelDataFormatToGLEnum(dataFormat);
            GLenum texelsType = oaDataTypeToGLEnum(dataType);
            GT_IF_WITH_ASSERT((texelsType != GL_NONE) && (pixelFormat != GL_NONE))
            {
                // Convert openGL enum to string
                apGLenumParameter dataFormatParameter(pixelFormat);
                apGLenumParameter dataTypeParameter(texelsType);

                gtString strPixelFormat;
                gtString strDataType;
                dataFormatParameter.valueAsString(strPixelFormat);
                dataTypeParameter.valueAsString(strDataType);

                // Create final strings:
                rawDataFormat.appendFormattedString(GD_STR_RawDataExportFileDataFormat, strPixelFormat.asCharArray());
                rawDataFormat.appendFormattedString(GD_STR_RawDataExportFileDataType, strDataType.asCharArray());
            }
        }

        // Generate amount of raw data pages:
        gtString rawDataAmountOfPages;
        {
            int amountOfPages = pRawDataHandler->amountOfPages();
            rawDataAmountOfPages.appendFormattedString(GD_STR_RawDataExportFileAmountOfDataPages, amountOfPages);
        }

        // ***********************
        // Generate the CSV header
        // ***********************
        fileHeader.append(GD_STR_StatisticsExportFileHeaderSeperator);
        fileHeader.append(GD_STR_RawDataExportFileHeaderTitle);
        fileHeader.append(GD_STR_StatisticsExportFileHeaderProjectName) += projectName += L"\n//\n";
        fileHeader.append(rawDataDimensions);
        fileHeader.append(rawDataFormat);
        fileHeader.append(rawDataType);
        fileHeader.append(rawDataAmountOfPages);
        fileHeader.append(GD_STR_StatisticsExportFileHeaderGenerationDate) += fileSavedDate += AF_STR_NewLine;
        fileHeader.append(GD_STR_StatisticsExportFileHeaderGenerationTime) += fileSavedTime += L"\n//\n";
        fileHeader.append(GD_STR_StatisticsExportFileHeaderGeneratedBy);
        fileHeader.append(GD_STR_StatisticsExportFileHeaderWebSite);
        fileHeader.append(GD_STR_StatisticsExportFileHeaderSeperator) += AF_STR_NewLine;
    }
}

// ---------------------------------------------------------------------------
// Name:        gdImagesAndBuffersExporter::setOutputFileName
// Description: Sets the output filename to save the file in
// Arguments:   fileName - sets output filename
// Author:      Eran Zinman
// Date:        27/1/2007
// ---------------------------------------------------------------------------
void gdImagesAndBuffersExporter::setOutputFileName(const gtString& fileName)
{
    // Save output file name
    _outputFileName = fileName;
}

// ---------------------------------------------------------------------------
// Name:        gdImagesAndBuffersExporter::addFileToOutputVector
// Description: Adds a file to the output results vector
// Arguments:   filePath - Output file that was exported
//              status - Export file status
// Author:      Eran Zinman
// Date:        27/1/2007
// ---------------------------------------------------------------------------
void gdImagesAndBuffersExporter::addFileToOutputVector(const osFilePath& filePath, gdExportedFileStatus status)
{
    // Create a new output file result struct
    gdFileExporterOutputResult* pOutputResult = new gdFileExporterOutputResult;
    GT_IF_WITH_ASSERT(pOutputResult != NULL)
    {
        // Set the output result values
        pOutputResult->filePath = filePath;
        pOutputResult->fileStatus = status;

        // Add file to the output results vector
        _outputResult.push_back(pOutputResult);
    }
}

// ---------------------------------------------------------------------------
// Name:        gdImagesAndBuffersExporter::generateOutputFilePath
// Description: Generate output file path
// Arguments:   outputFilePath - Outputs the file path to be exported
//              fileName - File name of the file to be exported
// Return Val:  bool - Success / Failure
// Author:      Eran Zinman
// Date:        27/1/2007
// ---------------------------------------------------------------------------
bool gdImagesAndBuffersExporter::generateOutputFilePath(osFilePath& outputFilePath, gtString fileName)
{
    bool retVal = false;

    // Set the output directory
    outputFilePath.setFileDirectory(_outputDir);

    // Set the output file name
    outputFilePath.setFileName(fileName);

    // Generate a file extension
    gtString fileExtension;
    bool rc1 = apFileTypeToFileExtensionString(_fileType, fileExtension);
    GT_IF_WITH_ASSERT(rc1)
    {
        // Set the new extension to the filename
        outputFilePath.setFileExtension(fileExtension);

        retVal = true;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gdImagesAndBuffersExporter::exportPBuffer
// Description: Export a PBuffer using the given file type
// Arguments:   pbufferID - The PBuffer ID
//              bufferType - The static buffer to export
//              pbufferContextID - The PBuffer spy context id
//              fileType - Export output file type
// Return Val:  bool - Success / Failure
// Author:      Eran Zinman
// Date:        27/1/2007
// ---------------------------------------------------------------------------
bool gdImagesAndBuffersExporter::exportVBO(GLuint vboName, apFileType fileType)
{
    bool retVal = false;

    // Clear result vector
    _outputResult.deleteElementsAndClear();

    // Save export file type
    _fileType = fileType;

    // Extract the PBuffer raw data to disk
    gtVector<GLuint> vboNames;
    vboNames.push_back(vboName);
    bool rcUpdateVBOs = gaUpdateVBORawData(_activeContext._contextId, vboNames);
    GT_IF_WITH_ASSERT(rcUpdateVBOs)
    {
        // Get the VBO details:
        apGLVBO vboDetails;
        bool rcGetVBODetails = gaGetVBODetails(_activeContext._contextId, vboName, vboDetails);
        GT_IF_WITH_ASSERT(rcGetVBODetails)
        {
            // Export VBO to disk
            retVal = exportVBOToDisk(vboDetails);
        }
    }
    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gdImagesAndBuffersExporter::exportCLBuffer
// Description: Export a CL buffer object
// Arguments:   int clBufferIndex
//              apFileType fileType
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        11/1/2011
// ---------------------------------------------------------------------------
bool gdImagesAndBuffersExporter::exportCLBuffer(int clBufferIndex, apFileType fileType)
{
    bool retVal = false;

    // Clear result vector
    _outputResult.deleteElementsAndClear();

    // Save export file type
    _fileType = fileType;

    // Extract the PBuffer raw data to disk
    gtVector<int> bufferIdsVec;
    bufferIdsVec.push_back(clBufferIndex);
    bool rcUpdateBuffer = gaUpdateOpenCLBufferRawData(_activeContext._contextId, bufferIdsVec);
    GT_IF_WITH_ASSERT(rcUpdateBuffer)
    {
        // Get the OpenCL buffer details:
        apCLBuffer bufferDetails;
        bool rcGetBufferDetails = gaGetOpenCLBufferObjectDetails(_activeContext._contextId, clBufferIndex, bufferDetails);
        GT_IF_WITH_ASSERT(rcGetBufferDetails)
        {
            // Export OpenCL buffer to disk:
            retVal = exportCLBufferToDisk(bufferDetails);
        }
    }
    return retVal;

}


// ---------------------------------------------------------------------------
// Name:        gdImagesAndBuffersExporter::exportCLSubBuffer
// Description: Export an OpenCL sub buffer object
// Arguments:   int clSubBufferIndex
//              apFileType fileType
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        11/1/2011
// ---------------------------------------------------------------------------
bool gdImagesAndBuffersExporter::exportCLSubBuffer(int clSubBufferIndex, apFileType fileType)
{
    bool retVal = false;

    // Clear result vector
    _outputResult.deleteElementsAndClear();

    // Save export file type
    _fileType = fileType;

    // Extract the PBuffer raw data to disk
    gtVector<int> subBufferIdsVec;
    subBufferIdsVec.push_back(clSubBufferIndex);
    bool rcUpdateSubBuffer = gaUpdateOpenCLSubBufferRawData(_activeContext._contextId, subBufferIdsVec);
    GT_IF_WITH_ASSERT(rcUpdateSubBuffer)
    {
        // Get the OpenCL sub-buffer details:
        apCLSubBuffer subBufferDetails;
        bool rcGetBufferDetails = gaGetOpenCLSubBufferObjectDetails(_activeContext._contextId, clSubBufferIndex, subBufferDetails);
        GT_IF_WITH_ASSERT(rcGetBufferDetails)
        {
            // Export OpenCL sub-buffer to disk:
            retVal = exportCLSubBufferToDisk(subBufferDetails);
        }
    }
    return retVal;

}

// ---------------------------------------------------------------------------
// Name:        gdImagesAndBuffersExporter::exportCLBufferToDisk
// Description: Exports an OpenCL buffer to disk
// Arguments:   const apCLBuffer& bufferDetails
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        20/5/2010
// ---------------------------------------------------------------------------
bool gdImagesAndBuffersExporter::exportCLBufferToDisk(const apCLBuffer& bufferDetails)
{
    bool retVal = false;

    // Get the buffer filename:
    osFilePath bufferFile;
    bufferDetails.getBufferFilePath(bufferFile);

    // Localize the path as needed:
    gaRemoteToLocalFile(bufferFile, false);

    // Sets the output exported file name:
    gtString outputFileName = _outputFileName;

    // If we didn't chose a specific filename
    bool rc1 = true;

    if (outputFileName.isEmpty())
    {
        rc1 = bufferFile.getFileName(outputFileName);
    }

    afProgressBarWrapper::instance().incrementProgressBar();

    GT_IF_WITH_ASSERT(rc1)
    {
        // Generate output file name
        osFilePath outputFilePath;
        bool rc2 = generateOutputFilePath(outputFilePath, outputFileName);
        GT_IF_WITH_ASSERT(rc2)
        {
            // This flag indicates if file was exported successfully
            bool isSuccess = false;

            // Load raw file into memory
            acRawFileHandler rawFileHandler;
            rc2 = rawFileHandler.loadFromFile(bufferFile);
            GT_IF_WITH_ASSERT(rc2)
            {
                // If raw data was loaded successfully
                GT_IF_WITH_ASSERT(rawFileHandler.isOk())
                {
                    // Set the buffer display properties:
                    rawFileHandler.setBufferDataFormat(bufferDetails.displayFormat());

                    // Export file to disk, and add export result to result vector
                    isSuccess = exportFileToDisk(&rawFileHandler, outputFilePath);

                }
            }

            // If file was not exported successfully
            if (!isSuccess)
            {
                // Add file to the result vector with a flag that it wasn't exported successfully
                addFileToOutputVector(outputFilePath, GD_EXPORT_FILE_FAILED);
            }

            // We return true even if we failed, because output result vector will contain the results
            retVal = true;
        }
    }

    afProgressBarWrapper::instance().incrementProgressBar();

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gdImagesAndBuffersExporter::exportCLSubBufferToDisk
// Description: Exports an OpenCL sub-buffer to disk
// Arguments:   const apCLSubBuffer& subBufferDetails
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        11/1/2011
// ---------------------------------------------------------------------------
bool gdImagesAndBuffersExporter::exportCLSubBufferToDisk(const apCLSubBuffer& subBufferDetails)
{
    bool retVal = false;

    // Get the buffer filename:
    osFilePath subBufferFile;
    subBufferDetails.getSubBufferFilePath(subBufferFile);

    // Localize the path as needed:
    gaRemoteToLocalFile(subBufferFile, false);

    // Sets the output exported file name:
    gtString outputFileName = _outputFileName;

    // If we didn't chose a specific filename
    bool rc1 = true;

    if (outputFileName.isEmpty())
    {
        rc1 = subBufferFile.getFileName(outputFileName);
    }

    afProgressBarWrapper::instance().incrementProgressBar();

    GT_IF_WITH_ASSERT(rc1)
    {
        // Generate output file name
        osFilePath outputFilePath;
        bool rc2 = generateOutputFilePath(outputFilePath, outputFileName);
        GT_IF_WITH_ASSERT(rc2)
        {
            // This flag indicates if file was exported successfully
            bool isSuccess = false;

            // Load raw file into memory
            acRawFileHandler rawFileHandler;
            rc2 = rawFileHandler.loadFromFile(subBufferFile);
            GT_IF_WITH_ASSERT(rc2)
            {
                // If raw data was loaded successfully
                GT_IF_WITH_ASSERT(rawFileHandler.isOk())
                {

                    // Set the buffer display properties:
                    rawFileHandler.setBufferDataFormat(subBufferDetails.displayFormat());

                    // Export file to disk, and add export result to result vector
                    isSuccess = exportFileToDisk(&rawFileHandler, outputFilePath);

                }
            }

            // If file was not exported successfully
            if (!isSuccess)
            {
                // Add file to the result vector with a flag that it wasn't exported successfully
                addFileToOutputVector(outputFilePath, GD_EXPORT_FILE_FAILED);
            }

            // We return true even if we failed, because output result vector will contain the results
            retVal = true;
        }
    }

    afProgressBarWrapper::instance().incrementProgressBar();

    return retVal;
}
