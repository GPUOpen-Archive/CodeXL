//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file acRawFileHandler.cpp
///
//==================================================================================

//------------------------------ acRawFileHandler.cpp ------------------------------

// Qt
#include <AMDTApplicationComponents/Include/acQtIncludes.h>

// Standard C:
#include <string.h>
#include <math.h>
#include <stdlib.h>

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTAPIClasses/Include/apInternalFormat.h>
#include <AMDTAPIClasses/Include/apGLTexture.h>
#include <AMDTOSWrappers/Include/osDebugLog.h>
#include <AMDTOSWrappers/Include/osFile.h>
#include <AMDTOSWrappers/Include/osStream.h>
#include <AMDTOSWrappers/Include/osOSDefinitions.h>
#include <AMDTOSAPIWrappers/Include/oaRawFileSeralizer.h>

// Local:
#include <AMDTApplicationComponents/Include/acMessageBox.h>
#include <AMDTApplicationComponents/Include/acColours.h>
#include <AMDTApplicationComponents/Include/acRawFileHandler.h>
#include <inc/acStringConstants.h>

// Defines the default output QImage bytes per pixel format
#define AC_OUTPUT_IMAGE_BYTES_PER_PIXEL 4
#define AC_CONVERT_IMAGE_LIMIT 1073741824
// Static members:
apGLfloatParameter* acRawFileHandler::_pStaticGLFloatParamter = new apGLfloatParameter;
apGLbyteParameter* acRawFileHandler::_pStaticGLByteParamter = new apGLbyteParameter;
apGLubyteParameter* acRawFileHandler::_pStaticGLuByteParamter = new apGLubyteParameter;
apCLcharParameter* acRawFileHandler::_pStaticCLCharParamter = new apCLcharParameter;
apCLucharParameter* acRawFileHandler::_pStaticCLUCharParamter = new apCLucharParameter;
apGLdoubleParameter* acRawFileHandler::_pStaticGLDoubleParamter = new apGLdoubleParameter;
apGLshortParameter* acRawFileHandler::_pStaticGLShortParamter = new apGLshortParameter;
apGLushortParameter* acRawFileHandler::_pStaticGLUShortParamter = new apGLushortParameter;
apGLintParameter* acRawFileHandler::_pStaticGLIntParamter = new apGLintParameter;
apGLuintParameter* acRawFileHandler::_pStaticGLUIntParamter = new apGLuintParameter;
apCLlongParameter* acRawFileHandler::_pStaticCLLongParamter = new apCLlongParameter;
apCLulongParameter* acRawFileHandler::_pStaticCLULongParamter = new apCLulongParameter;

// This is the pixel byte order (little endian ARGB32 image format):
#define QT_BITMAP_ALPHA_CHANNEL_INDEX 3
#define QT_BITMAP_RED_CHANNEL_INDEX 2
#define QT_BITMAP_GREEN_CHANNEL_INDEX 1
#define QT_BITMAP_BLUE_CHANNEL_INDEX 0

// ---------------------------------------------------------------------------
// Name:        acRawFileHandler::acRawFileHandler
// Description: Constructor - loads the raw data file
// Arguments:   rawDataFile - The raw data file to load
//              isStaticData - Is data static or not
// Author:      Eran Zinman
// Date:        11/8/2007
// ---------------------------------------------------------------------------
acRawFileHandler::acRawFileHandler(bool yFlipImage)
    : _isLoadedSuccesfully(false), _width(0), _height(0), m_yFlipImage(yFlipImage),
      _componentDataType(OA_UNSIGNED_BYTE), _texelDataFormat(OA_TEXEL_FORMAT_UNKNOWN),
      _rawDataPixelSize(0), _dataTypeSize(0), _amountOfPixelComponents(0),
      _amountOfComponentsInDataType(0), _pRawData(NULL), _rawDataSize(0), _amountOfPages(-1),
      m_pageStride(1), m_pageOffset(0), _activePage(0), _activePageRawDataOffset(0), _offset(0),
      _stride(0), _valuesNormazlied(false), _pPixelValueDataParameter(NULL), _minValueParameter(0),
      _maxValueParameter(0), _valueMultiplier(-1)
{
    for (int i = 0; i < AC_MAX_RAW_FILE_FILTER_HANDLERS; i++)
    {
        _pFilterRawFileHandler[i] = NULL;
    }
}

// ---------------------------------------------------------------------------
// Name:        acRawFileHandler::acRawFileHandler
// Description: Constructor - loads the raw data file
// Arguments:
// Author:      Eran Zinman
// Date:        11/8/2007
// ---------------------------------------------------------------------------
acRawFileHandler::acRawFileHandler(gtUByte* pRawData, gtSizeType rawDataSize, int width, int height, oaTexelDataFormat dataFormat, oaDataType dataType, int amountOfPages)
    : _isLoadedSuccesfully(false), _width(width), _height(height), m_yFlipImage(true),
      _componentDataType(dataType), _texelDataFormat(dataFormat),
      _rawDataPixelSize(0), _dataTypeSize(0), _amountOfPixelComponents(0),
      _amountOfComponentsInDataType(0), _pRawData(pRawData), _rawDataSize(rawDataSize), _amountOfPages(amountOfPages),
      m_pageStride(1), m_pageOffset(0), _activePage(0), _activePageRawDataOffset(0), _valuesNormazlied(false),
      _pPixelValueDataParameter(NULL), _minValueParameter(0), _maxValueParameter(0),
      _valueMultiplier(-1)
{
    for (int i = 0; i < AC_MAX_RAW_FILE_FILTER_HANDLERS; i++)
    {
        _pFilterRawFileHandler[i] = NULL;
    }

    // Sanity check:
    GT_IF_WITH_ASSERT(pRawData != NULL)
    {
        // Dimensions check:
        bool rc1 = ((_width >= 0) && (_height >= 0));
        GT_IF_WITH_ASSERT(rc1)
        {
            // Do we have any pages?
            bool rc2 = (amountOfPages > 0);
            GT_IF_WITH_ASSERT(rc2)
            {
                // Initializes the raw file handler
                initHandler();
            }
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        acRawFileHandler::~acRawFileHandler
// Description: Destructor
// Author:      Eran Zinman
// Date:        11/8/2007
// ---------------------------------------------------------------------------
acRawFileHandler::~acRawFileHandler()
{
    // Release raw data memory
    if (_pRawData)
    {
        free(_pRawData);
        _pRawData = NULL;
    }

    // Clear the GL parameter object:
    clearDataParameter();
}


// ---------------------------------------------------------------------------
// Name:        acRawFileHandler::clearDataParameter
// Description: Clear the openGL parameter object used for reading values
// Return Val: void
// Author:      Sigal Algranaty
// Date:        5/8/2009
// ---------------------------------------------------------------------------
void acRawFileHandler::clearDataParameter()
{
    // Delete the GL data parameter
    if (_pPixelValueDataParameter)
    {
        if ((_pPixelValueDataParameter != _pStaticGLByteParamter) && (_pPixelValueDataParameter != _pStaticGLFloatParamter)
            && (_pPixelValueDataParameter != _pStaticGLuByteParamter) && (_pPixelValueDataParameter != _pStaticGLIntParamter)
            && (_pPixelValueDataParameter != _pStaticGLUShortParamter) && (_pPixelValueDataParameter != _pStaticGLDoubleParamter)
            && (_pPixelValueDataParameter != _pStaticGLShortParamter) && (_pPixelValueDataParameter != _pStaticGLUIntParamter)
            && (_pPixelValueDataParameter != _pStaticCLCharParamter) && (_pPixelValueDataParameter != _pStaticCLUCharParamter)
            && (_pPixelValueDataParameter != _pStaticCLLongParamter) && (_pPixelValueDataParameter != _pStaticCLULongParamter))

        {
            delete _pPixelValueDataParameter;
            _pPixelValueDataParameter = NULL;
        }
    }
}
// ---------------------------------------------------------------------------
// Name:        acRawFileHandler::initHandler
// Description: Initializes the raw file handler after loading data. This
//              function must be called after setting / loading new data
//              to the raw file handler.
// Author:      Eran Zinman
// Date:        16/1/2008
// ---------------------------------------------------------------------------
void acRawFileHandler::initHandler()
{
    // We do a bunch of tests and afterwards we'll determine if data was loaded ok.
    _isLoadedSuccesfully = false;

    // Calculate raw data pixel size:
    _rawDataPixelSize = oaCalculatePixelUnitByteSize(_texelDataFormat, _componentDataType);
    GT_IF_WITH_ASSERT(_rawDataPixelSize != -1)
    {
        // Calculate amount of components per pixel
        _amountOfPixelComponents = oaAmountOfTexelFormatComponents(_texelDataFormat);
        GT_IF_WITH_ASSERT(_amountOfPixelComponents != -1)
        {
            // Calculate data type size
            _dataTypeSize = oaSizeOfDataType(_componentDataType);
            GT_IF_WITH_ASSERT(_dataTypeSize != -1)
            {
                // Get amount of components *inside* the data type
                _amountOfComponentsInDataType = oaAmountComponentsInDataType(_componentDataType);
                GT_IF_WITH_ASSERT(_amountOfComponentsInDataType != -1)
                {
                    // Generate openGL read parameters
                    _pPixelValueDataParameter = createPixelValueDataParameter();
                    GT_IF_WITH_ASSERT(_pPixelValueDataParameter != NULL)
                    {
                        _isLoadedSuccesfully = true;
                    }
                }
            }
        }
    }

    // Alert if data wasn't loaded successfully
    GT_ASSERT(_isLoadedSuccesfully);
}

// ---------------------------------------------------------------------------
// Name:        acRawFileHandler::loadFromFile
// Description: Loads raw data from file into the raw file handler
// Arguments:   rawDataFile - The raw data file to load
// Return Val:  bool - Success / Failure
// Author:      Eran Zinman
// Date:        16/1/2008
// ---------------------------------------------------------------------------
bool acRawFileHandler::loadFromFile(const osFilePath& rawDataFile)
{
    bool retVal = false;

    // Load the raw data from file
    if (rawDataFile.isRegularFile())
    {
        // Load Raw data file
        oaRawFileSeralizer rawFileSeralizer;
        bool rc = rawFileSeralizer.loadFromFile(rawDataFile);
        GT_IF_WITH_ASSERT(rc)
        {
            // Set pointer to the new memory. *Take ownership* over releasing the memory from oaRawFileSeralizer
            _rawDataSize = rawFileSeralizer.getRawDataSize();
            _pRawData = (gtUByte*)rawFileSeralizer.getRawDataPointer(true /* Take Ownership */);
            GT_IF_WITH_ASSERT(_pRawData != NULL)
            {
                // Get raw data dimensions
                rawFileSeralizer.getRawDataDimensions(_width, _height);

                // Get raw data data type
                rawFileSeralizer.getRawDataFormat(_texelDataFormat, _componentDataType);

                // Get amount of pages in raw data
                _amountOfPages = rawFileSeralizer.getAmountOfPages();

                // Initializes the raw file hander
                initHandler();

                // If data was loaded successfully
                if (_isLoadedSuccesfully)
                {
                    retVal = true;
                }
            }
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        acRawFileHandler::createPixelValueDataParameter
// Description: Generates an OpenGL parameter which will be used to retrieve
//              data from the raw data according to the raw data type.
// Return Val:  A pointer of apParameter on the parameter type.
// Author:      Eran Zinman
// Date:        11/8/2007
// ---------------------------------------------------------------------------
apPixelValueParameter* acRawFileHandler::createPixelValueDataParameter()
{
    apPixelValueParameter* pParameter = NULL;

    // Now read the data according to it's type
    switch (_componentDataType)
    {
        case OA_UNSIGNED_BYTE:
        case OA_UNSIGNED_BYTE_3_3_2:
        case OA_UNSIGNED_BYTE_2_3_3_REV:
        {
            // Unsigned byte type
            pParameter = new apGLubyteParameter;
        }
        break;

        case OA_BYTE:
        {
            // Byte type
            pParameter = new apGLbyteParameter;
        }
        break;

        case OA_UNSIGNED_SHORT:
        case OA_UNSIGNED_SHORT_5_6_5:
        case OA_UNSIGNED_SHORT_5_6_5_REV:
        case OA_UNSIGNED_SHORT_4_4_4_4:
        case OA_UNSIGNED_SHORT_4_4_4_4_REV:
        case OA_UNSIGNED_SHORT_5_5_5_1:
        case OA_UNSIGNED_SHORT_1_5_5_5_REV:
        {
            // Unsigned short type
            pParameter = new apGLushortParameter;
        }
        break;

        case OA_SHORT:
        {
            // Short type
            pParameter = new apGLshortParameter;
        }
        break;

        case OA_UNSIGNED_INT:
        case OA_UNSIGNED_INT_8_8_8_8:
        case OA_UNSIGNED_INT_8_8_8_8_REV:
        case OA_UNSIGNED_INT_10_10_10_2:
        case OA_UNSIGNED_INT_2_10_10_10_REV:
        {
            // Unsigned int type
            pParameter = new apGLuintParameter;
        }
        break;

        case OA_INT:
        {
            // Int type
            pParameter = new apGLintParameter;
        }
        break;

        case OA_LONG:
        {
            // Long type
            pParameter = new apCLlongParameter;
        }
        break;

        case OA_UNSIGNED_LONG:
        {
            // Int type
            pParameter = new apCLulongParameter;
        }
        break;

        case OA_FLOAT:
        {
            // Float type
            pParameter = new apGLfloatParameter;
        }
        break;

        case OA_DOUBLE:
        {
            // Float type
            pParameter = new apGLdoubleParameter;
        }
        break;

        default:
        {
            // Unknown buffer data format:
            gtString errString = L"Unsupported data type";
            errString.appendFormattedString(L": %d", _componentDataType);

            GT_ASSERT_EX(false, errString.asCharArray());
        }
        break;
    }

    return pParameter;
}

// ---------------------------------------------------------------------------
// Name:        acRawFileHandler::convertToQImage
// Description: Converts the raw data to QImage format on the currently
// Return Val:  QImage*
// Author:      Sigal Algranaty
// Date:        24/7/2012
// ---------------------------------------------------------------------------
QImage* acRawFileHandler::convertToQImage()
{
    QImage* pRetVal = NULL;

    // If raw data file was loaded successfully
    if (_isLoadedSuccesfully)
    {
        // Allocate a memory buffer that will hold the pixels in raw format:
        int bytesPerPixel = 4;

        // make sure 1G limit is not reached:
        if (_width * _height * bytesPerPixel < AC_CONVERT_IMAGE_LIMIT)
        {
            uchar* pImageBuffer = new uchar[_width * _height * bytesPerPixel];


            // Add the offset to get current page:
            gtUByte* pSrcRawData = (gtUByte*)(_pRawData + _activePageRawDataOffset);

            int qimagePixelSize = 4 * sizeof(uchar);
            GT_IF_WITH_ASSERT(pSrcRawData != NULL)
            {
                // Calculate the line and pixel sizes:
                int qimageLineSize = qimagePixelSize * _width;

                // Fill the free image bitmap with the raw data converted image:
                for (int y = 0; y < _height; y++)
                {
                    // Get the current line index:
                    int lineIndex = m_yFlipImage ? _height - (y + 1) : y;

                    for (int x = 0; x < _width; x++)
                    {
                        // Before calculating the pixel value, check if the pixel should be filtered:
                        bool shouldPixelBeDisplayed = true;
                        bool rcCheckFilter = checkPixelFilter(y, x, shouldPixelBeDisplayed);
                        GT_ASSERT(rcCheckFilter);

                        int currentPixelOffset = lineIndex * qimageLineSize + x * qimagePixelSize;
                        uchar* pPixelAddress = pImageBuffer + currentPixelOffset;

                        // If values are normalized, fill normalized values, else fill with default multiplier
                        if (_valuesNormazlied)
                        {
                            // Fill normalized pixel value according to the raw data type
                            calculateNormalizedPixelValue(pSrcRawData, pPixelAddress, shouldPixelBeDisplayed);
                        }
                        else
                        {
                            // Fill pixel value according to the raw data type:
                            calculateQImagePixelValue(pSrcRawData, pPixelAddress, shouldPixelBeDisplayed);
                        }

                        // Advance to the next source pixel position:
                        pSrcRawData += _rawDataPixelSize;
                    }
                }

                // Make sure width and height are in valid range before trying to create the qimage or it might crash the application:
                if (_width * _height * bytesPerPixel < AC_CONVERT_IMAGE_LIMIT)
                {
                    // Allocate the required bitmap:
                    pRetVal = new QImage(pImageBuffer, _width, _height, _width * 4, QImage::Format_ARGB32);

                }
            }
        }
        else
        {
            gtString imageSizeLimitReached;
            imageSizeLimitReached.appendFormattedString(L"%d %d %ls", _width, _height, AC_STR_ImageLimitReached);
            OS_OUTPUT_DEBUG_LOG(imageSizeLimitReached.asCharArray(), OS_DEBUG_LOG_ERROR);
            QString titleToUser(QString::fromWCharArray(AC_STR_ErrorMessageError));
            QString messageToUser(QString::fromWCharArray(AC_STR_ImageLimitReached));
            acMessageBox::instance().critical(titleToUser, messageToUser);
        }
    }

    return pRetVal;
}

// ---------------------------------------------------------------------------
// Name:        acRawFileHandler::findLowestAndHighestValuesFloat
// Description: Finds the lowest and highest values in a float raw data
// Arguments:   shouldNegativeValuesBeNormalized - tells how to handle negative value
//              true - normalize to 0
//              false - remain as is
// Return Val:  bool - success / failure
// Author:      Eran Zinman
// Date:        13/8/2007
// ---------------------------------------------------------------------------
bool acRawFileHandler::findMinMaxValuesFromRawData(bool shouldNegativeValuesBeNormalized)
{
    bool retVal = false;

    GT_IF_WITH_ASSERT(_isLoadedSuccesfully)
    {
        // Sanity check
        GT_IF_WITH_ASSERT(_pPixelValueDataParameter != NULL)
        {
            // Get pointer to the raw data (with the active page offset):
            gtUByte* pRawDataSrc = (gtUByte*)(_pRawData + _activePageRawDataOffset);
            GT_IF_WITH_ASSERT(pRawDataSrc != NULL)
            {
                // Get initial lowest and highest values from the first pixel in array
                _pPixelValueDataParameter->readValueFromPointer(pRawDataSrc);

                _minValueParameter = _pPixelValueDataParameter->valueAsDouble();
                _maxValueParameter = _minValueParameter;

                // Fill the free image bitmap with the converted raw data image:
                for (int y = 0; y < _height; y++)
                {
                    for (int x = 0; x < _width; x++)
                    {
                        // Get the current pixel value
                        _pPixelValueDataParameter->readValueFromPointer(pRawDataSrc);

                        // Get value as double
                        double curValue = _pPixelValueDataParameter->valueAsDouble();

                        // Check if we found a new maximum or minimum
                        if (curValue < _minValueParameter)
                        {
                            if (!shouldNegativeValuesBeNormalized)
                            {
                                _minValueParameter = curValue;
                            }
                            else if (curValue >= 0)
                            {
                                _minValueParameter = curValue;
                            }
                        }
                        else if (curValue > _maxValueParameter)
                        {
                            _maxValueParameter = curValue;
                        }

                        // Advance to the next source pixel:
                        pRawDataSrc += _rawDataPixelSize;
                    }
                }

                retVal = true;
            }
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        acRawFileHandler::getValueMultiplier
// Description: Return the value multiplier (if values are normalized)
// Arguments:   valueMultiplier - Output value multiplier
// Return Val:  bool - Success / Failure
// Author:      Eran Zinman
// Date:        7/1/2008
// ---------------------------------------------------------------------------
bool acRawFileHandler::getValueMultiplier(double& valueMultiplier)
{
    bool retVal = false;

    if (_valuesNormazlied)
    {
        // Return the value multiplier
        valueMultiplier = _valueMultiplier;

        retVal = true;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        acRawFileHandler::calculateValueMultiplier
// Description: Calculates the value multiplier according to the minimum and
//              maximum values of the raw data
// Arguments:   minValue, maxValue - Raw data minimum and maximum values
// Author:      Eran Zinman
// Date:        30/12/2007
// ---------------------------------------------------------------------------
void acRawFileHandler::calculateValueMultiplier(double minValue, double maxValue)
{
    // Calculate the amount of possible values between minimum and maximum values
    double amountOfValues = maxValue - minValue;

    // These are the amount of values we need to normalize our values to
    double mapToValue = (double)255.0f;

    // When data type is not float (regular numbers) we add one extra to the amount of possible values.
    // For example: If minValue is 5 and maxValue is 9 we got 9 - 5 + 1 = 5 Possible values
    if ((OA_FLOAT != _componentDataType) && (OA_DOUBLE != _componentDataType))
    {
        amountOfValues++;
        mapToValue = (double)256.0f;
    }

    // Recalculate the value multiplier according to the new min and max values
    _valueMultiplier = mapToValue / amountOfValues;

    // Flag that the raw data is normalized
    _valuesNormazlied = true;
}

// ---------------------------------------------------------------------------
// Name:        acRawFileHandler::setMinMaxValues
// Description: Manually sets the minimum and maximum values for the raw data
//              (for example - using the double slider component)
// Arguments:   minValue, maxValue - Raw data minimum and maximum values
// Return Val:  bool - Success / Failure
// Author:      Eran Zinman
// Date:        16/11/2007
// ---------------------------------------------------------------------------
bool acRawFileHandler::setMinMaxValues(double minValue, double maxValue)
{
    bool retVal = false;

    // Do we have reasonable values?
    GT_IF_WITH_ASSERT(maxValue >= minValue)
    {
        // Set the updated minimum and maximum values
        _minValueParameter = minValue;
        _maxValueParameter = maxValue;

        // Recalculate the value multiplier according to the new min and max values
        calculateValueMultiplier(minValue, maxValue);

        retVal = true;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        acRawFileHandler::getMinMaxValues
// Description: Return the minimum and maximum values
// Arguments:   leftValue - Output minimum value
//              rightValue - Output maximum value
// Author:      Eran Zinman
// Date:        11/8/2007
// ---------------------------------------------------------------------------
void acRawFileHandler::getMinMaxValues(double& minValue, double& maxValue)
{
    // Return minimum and maximum values
    minValue = _minValueParameter;
    maxValue = _maxValueParameter;
}


// ---------------------------------------------------------------------------
// Name:        acRawFileHandler::shouldNormalizeDataNegativeValues
// Description: The function returns true if negative value should be normalized, and false
//              if the data format supports negative values
// Arguments: oaTexelDataFormat texelFormat
// Return Val: bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        22/6/2008
// ---------------------------------------------------------------------------
bool acRawFileHandler::shouldNormalizeDataNegativeValues(oaTexelDataFormat texelFormat)
{
    bool retVal = false;

    switch (texelFormat)
    {

        case OA_TEXEL_FORMAT_DEPTH:
        case OA_TEXEL_FORMAT_DEPTH_COMPONENT16:
        case OA_TEXEL_FORMAT_DEPTH_COMPONENT24:
        case OA_TEXEL_FORMAT_DEPTH_COMPONENT32:
        case OA_TEXEL_FORMAT_DEPTH_EXT:
            retVal = true;
            break;


        default:
        {
            retVal = false;
        }
    }

    return retVal;
}
// ---------------------------------------------------------------------------
// Name:        acRawFileHandler::normalizeValues
// Description: Normalizing the raw data values before converting it to
//              QImage object
// Arguments:   bool force - should we force re-normalization if already normalized
// Return Val:  bool  - Success / failure.
// Author:      Eran Zinman
// Date:        11/8/2007
// ---------------------------------------------------------------------------
bool acRawFileHandler::normalizeValues(oaTexelDataFormat texelFormat, bool force)
{
    bool retVal = true;

    // If values weren't normalized before:
    if (!_valuesNormazlied || force)
    {
        retVal = false;

        // We only know how to normalize one component data formats:
        GT_IF_WITH_ASSERT(_amountOfPixelComponents == 1)
        {
            // Check if negative values should be normalized:
            bool shouldNegativeValuesBeNormalized = shouldNormalizeDataNegativeValues(texelFormat);

            // Find highest and lowest:
            bool rc1 = findMinMaxValuesFromRawData(shouldNegativeValuesBeNormalized);
            GT_IF_WITH_ASSERT(rc1)
            {
                // Do we have reasonable values?
                bool rc2 = (_maxValueParameter >= _minValueParameter);
                GT_IF_WITH_ASSERT(rc2)
                {
                    // Recalculate the value multiplier according to the new min and max values
                    calculateValueMultiplier(_minValueParameter, _maxValueParameter);

                    retVal = true;
                }
            }
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        acRawFileHandler::getRawDataValue
// Description: Retrieves a value from the raw data
// Arguments:   x,y - (x, y) position in the raw data
//              componentIndex - Index of the desired component (0..amount-1)
//              value - Output raw data value
// Return Val:  bool  - Success / failure.
// Author:      Eran Zinman
// Date:        8/12/2007
// ---------------------------------------------------------------------------
bool acRawFileHandler::getRawDataComponentValue(int x, int y, int componentIndex, apPixelValueParameter*& pParameter, bool& isValueAvailable)
{
    bool retVal = false;

    // Sanity check:
    GT_IF_WITH_ASSERT(_isLoadedSuccesfully)
    {
        if (oaIsBufferTexelFormat(_texelDataFormat))
        {
            retVal = getBufferRawDataComponentValue(x, y, pParameter);
        }
        else
        {
            // First check if the component is filtered:
            bool rcCheckFilter = checkPixelFilter(y, x, isValueAvailable);
            GT_ASSERT(rcCheckFilter);

            if (isValueAvailable)
            {
                // Components range check:
                bool rc1 = ((componentIndex >= 0) && (componentIndex < _amountOfPixelComponents));
                GT_IF_WITH_ASSERT(rc1)
                {
                    // Before we access the memory do a (x, y) position range check:
                    bool rc2 = ((x >= 0) && (y >= 0) && (x < _width) && (y < _height));

                    if (rc2)
                    {
                        // Go to raw data pixel position
                        unsigned long pageOffset = _activePageRawDataOffset + (y * _width + x) * _rawDataPixelSize;
                        gtUByte* pDataOffset = (gtUByte*)(_pRawData + pageOffset);

                        // Get raw data component value
                        bool rc3 = getPixelChannelValue(pDataOffset, componentIndex, pParameter);
                        GT_IF_WITH_ASSERT(rc3)
                        {
                            retVal = true;
                        }
                    }
                }
            }
            else
            {
                retVal = true;
            }

        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        acRawFileHandler::getRawDataValue
// Description: Retrieves a value from the raw data
// Arguments:   x - the chunk number within the raw data
//              y - the item index within the chunk
//              value - Output raw data value
// Return Val:  bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        19/4/2009
// ---------------------------------------------------------------------------
bool acRawFileHandler::getBufferRawDataComponentValue(int x, int y, apPixelValueParameter*& pParameter)
{
    bool retVal = false;

    // Set the offset to start with the buffer offset:
    unsigned long pageOffset = _offset;

    // Get the chunk size according to the texel format:
    int chunkSize = oaCalculateChunkByteSize(_texelDataFormat);

    // Check what should be the offset for this coordinate:
    pageOffset += chunkSize * x;

    // Add the offset within the chunk:
    for (int i = 0; i < y; i++)
    {
        // Get the current pixel data type:
        oaDataType currentDataType = oaGetTexelFormatDataType(_texelDataFormat, i);

        // Calculate the size of pixel unit, for the current pixel:
        int sizeOfCurrentPixel = oaSizeOfDataType(currentDataType);

        // Add the current pixel size to the data offset:
        pageOffset += sizeOfCurrentPixel;
    }

    // Add the stride offsets for each row (one stride for each row - chunk of VBO data):
    pageOffset += x * _stride;

    // Get the requested pixel data type:
    oaDataType dataType = oaGetTexelFormatDataType(_texelDataFormat, y);

    // Get the requested data type size:
    int dataTypeSize = oaSizeOfDataType(dataType);

    // Get the parameter for the requested pixel:

    // Before we access the memory do a x position range check:
    bool rc2 = ((pageOffset + dataTypeSize) <= (unsigned int)_width);

    if (rc2)
    {
        // Go to raw data pixel position:
        gtUByte* pDataOffset = (gtUByte*)(_pRawData + pageOffset);

        bool rc3 = true;

        switch (dataType)
        {
            case OA_FLOAT:
                _pPixelValueDataParameter = _pStaticGLFloatParamter;
                break;

            case OA_DOUBLE:
                _pPixelValueDataParameter = _pStaticGLDoubleParamter;
                break;

            case OA_BYTE:
                _pPixelValueDataParameter = _pStaticGLByteParamter;
                break;

            case OA_UNSIGNED_BYTE:
                _pPixelValueDataParameter = _pStaticGLuByteParamter;
                break;

            case OA_CHAR:
                _pPixelValueDataParameter = _pStaticCLCharParamter;
                break;

            case OA_UNSIGNED_CHAR:
                _pPixelValueDataParameter = _pStaticCLUCharParamter;
                break;

            case OA_UNSIGNED_SHORT:
                _pPixelValueDataParameter = _pStaticGLUShortParamter;
                break;

            case OA_SHORT:
                _pPixelValueDataParameter = _pStaticGLShortParamter;
                break;

            case OA_INT:
                _pPixelValueDataParameter = _pStaticGLIntParamter;
                break;

            case OA_UNSIGNED_INT:
                _pPixelValueDataParameter = _pStaticGLUIntParamter;
                break;

            case OA_LONG:
                _pPixelValueDataParameter = _pStaticCLLongParamter;
                break;

            case OA_UNSIGNED_LONG:
                _pPixelValueDataParameter = _pStaticCLULongParamter;
                break;

            default:
                GT_ASSERT(false);
                rc3 = false;
                break;
        }

        GT_IF_WITH_ASSERT(rc3)
        {
            _componentDataType = dataType;

            // Get raw data component value:
            rc3 = getPixelChannelValue(pDataOffset, 0, pParameter);
            GT_IF_WITH_ASSERT(rc3)
            {
                retVal = true;
            }

            _componentDataType = OA_BYTE;
        }
    }

    return retVal;
}
// ---------------------------------------------------------------------------
// Name:        acRawFileHandler::getNormalizedPixelValue
// Description: Generates a normalized pixel value, given an OpenGL parameter
// Arguments:   pParameter - Input OpenGL parameter
//              channelIndex - Current channel index
//              pixelValue - Output pixel value
// Return Val:  bool  - Success / failure.
// Author:      Eran Zinman
// Date:        7/1/2008
// ---------------------------------------------------------------------------
bool acRawFileHandler::getNormalizedPixelValue(apPixelValueParameter* pParameter, int channelIndex, GLubyte& pixelValue)
{
    bool retVal = false;

    // Sanity check:
    GT_IF_WITH_ASSERT(pParameter != NULL)
    {
        // If raw data was normalized and we are dealing with the primary channel (channel index 0)
        if (_valuesNormazlied && (channelIndex == 0))
        {
            // Get current, minimum and maximum values as double
            double currentVal = pParameter->valueAsDouble();

            // If value is out of range
            if ((currentVal < _minValueParameter) || (currentVal > _maxValueParameter))
            {
                pixelValue = 0;
            }
            else
            {
                // Reduce the min value from current value
                currentVal -= _minValueParameter;

                // Convert channel value to pixel value
                pixelValue = (GLubyte)(currentVal * _valueMultiplier);
            }
        }
        else
        {
            // Just convert parameter to pixel value
            pixelValue = pParameter->asPixelValue();
        }

        retVal = true;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        acRawFileHandler::calculateNormalizedPixelValue
// Description: Fill normalized pixel value according to the raw data type
// Arguments:   pSrcRawData - The raw data pixel offset
//              pPixelData - The QImage destination pixel offset
// Author:      Eran Zinman
// Date:        29/12/2007
// ---------------------------------------------------------------------------
void acRawFileHandler::calculateNormalizedPixelValue(gtUByte* pSrcRawData, uchar* pPixelData, bool shouldPixelBeDisplayed)
{
    // Pixel value that will be written into the image
    GLubyte pixelValue[AC_OUTPUT_IMAGE_BYTES_PER_PIXEL] = {0, 0, 0, 0xff};

    if (!shouldPixelBeDisplayed)
    {
        // Fill the pixel value with non available color:
        pixelValue[QT_BITMAP_RED_CHANNEL_INDEX] = acQRAW_FILE_NOT_IN_SCOPE_COLOR.red();
        pixelValue[QT_BITMAP_GREEN_CHANNEL_INDEX] = acQRAW_FILE_NOT_IN_SCOPE_COLOR.green();
        pixelValue[QT_BITMAP_BLUE_CHANNEL_INDEX] = acQRAW_FILE_NOT_IN_SCOPE_COLOR.blue();
    }
    else
    {
        // Get first component value:
        apPixelValueParameter* pParameter = NULL;
        bool rc = getPixelChannelValue(pSrcRawData, 0, pParameter);
        GT_IF_WITH_ASSERT(rc)
        {
            // Get current, minimum and maximum values as double
            double currentVal = pParameter->valueAsDouble();

            // If we are above range show red
            if (currentVal > _maxValueParameter)
            {
                // If we are above range, fill red channel
                pixelValue[QT_BITMAP_RED_CHANNEL_INDEX] = acQRAW_FILE_ABOVE_RANGE_COLOR.red();
                pixelValue[QT_BITMAP_GREEN_CHANNEL_INDEX] = acQRAW_FILE_ABOVE_RANGE_COLOR.green();
                pixelValue[QT_BITMAP_BLUE_CHANNEL_INDEX] = acQRAW_FILE_ABOVE_RANGE_COLOR.blue();
            }
            else if (currentVal < _minValueParameter)
            {
                // If we are below range, fill green channel
                pixelValue[QT_BITMAP_RED_CHANNEL_INDEX] = acQRAW_FILE_BELOW_RANGE_COLOR.red();
                pixelValue[QT_BITMAP_GREEN_CHANNEL_INDEX] = acQRAW_FILE_BELOW_RANGE_COLOR.green();
                pixelValue[QT_BITMAP_BLUE_CHANNEL_INDEX] = acQRAW_FILE_BELOW_RANGE_COLOR.blue();
            }
            else
            {
                // Reduce the min value from current value:
                currentVal -= _minValueParameter;

                if ((_maxValueParameter - _minValueParameter) > 0)
                {
                    currentVal /= _maxValueParameter - _minValueParameter;
                }

                // For these types - Fill the result into the R, G, B values:
                pixelValue[QT_BITMAP_RED_CHANNEL_INDEX] = currentVal * acQRAW_FILE_TOP_RANGE_COLOR.red();
                pixelValue[QT_BITMAP_GREEN_CHANNEL_INDEX] = currentVal * acQRAW_FILE_TOP_RANGE_COLOR.green();
                pixelValue[QT_BITMAP_BLUE_CHANNEL_INDEX] = currentVal * acQRAW_FILE_TOP_RANGE_COLOR.blue();
            }
        }

        // We got a special format which can be normalized (LUMINANCE_ALPHA)
        if ((_texelDataFormat == OA_TEXEL_FORMAT_LUMINANCEALPHA) || (_texelDataFormat == OA_TEXEL_FORMAT_LUMINANCEALPHA_COMPRESSED))
        {
            // Get alpha channel value
            rc = getPixelChannelValue(pSrcRawData, 1, pParameter);
            GT_IF_WITH_ASSERT(rc)
            {
                // Convert channel value to pixel value
                GLubyte channelValue = pParameter->asPixelValue();

                // Set alpha channel value
                pixelValue[QT_BITMAP_ALPHA_CHANNEL_INDEX] = channelValue;
            }
        }
    }

    // Write RGBA values:
    memcpy(pPixelData, &pixelValue, AC_OUTPUT_IMAGE_BYTES_PER_PIXEL);
}

// ---------------------------------------------------------------------------
// Name:        acRawFileHandler::calculateQImagePixelValue
// Description: Calculate the raw data type and then fills pixel value
//              according to the raw data type
// Arguments:   pSrcRawData - The raw data pixel offset
//              pPixelData - The QImage destination pixel offset
// Author:      Eran Zinman
// Date:        9/12/2007
// ---------------------------------------------------------------------------
void acRawFileHandler::calculateQImagePixelValue(gtUByte* pSrcRawData, uchar* pPixelData, bool shouldPixelBeDisplayed)
{
    // Pixel value that will be written into the image
    GLubyte pixelValue[AC_OUTPUT_IMAGE_BYTES_PER_PIXEL] = {0, 0, 0, 0xff};

    if (!shouldPixelBeDisplayed)
    {
        // Fill the pixel value with non available color:
        pixelValue[QT_BITMAP_RED_CHANNEL_INDEX] = acQRAW_FILE_NOT_IN_SCOPE_COLOR.red();
        pixelValue[QT_BITMAP_GREEN_CHANNEL_INDEX] = acQRAW_FILE_NOT_IN_SCOPE_COLOR.green();
        pixelValue[QT_BITMAP_BLUE_CHANNEL_INDEX] = acQRAW_FILE_NOT_IN_SCOPE_COLOR.blue();
    }
    else
    {
        // Loop through the channels
        for (int i = 0; i < _amountOfPixelComponents; i++)
        {
            // Get component value
            apPixelValueParameter* pParameter = NULL;
            bool rc = getPixelChannelValue(pSrcRawData, i, pParameter);
            GT_IF_WITH_ASSERT(rc)
            {
                // Convert channel value to pixel value
                GLubyte channelValue = pParameter->asPixelValue();

                // Fill the appropriate channel(s) with the pixel value
                oaTexelDataFormat componentFormat = oaGetTexelFormatComponentType(_texelDataFormat, i);

                switch (componentFormat)
                {
                    case OA_TEXEL_FORMAT_STENCIL:
                    case OA_TEXEL_FORMAT_DEPTH:
                    case OA_TEXEL_FORMAT_LUMINANCE:
                    case OA_TEXEL_FORMAT_COLORINDEX:
                    case OA_TEXEL_FORMAT_VARIABLE_VALUE:
                    {
                        // For these types - Fill the result into the R, G, B values:
                        memset(pixelValue, channelValue, 3 * sizeof(GLubyte));
                    }
                    break;

                    case OA_TEXEL_FORMAT_RED:
                        pixelValue[QT_BITMAP_RED_CHANNEL_INDEX] = channelValue;
                        break;

                    case OA_TEXEL_FORMAT_GREEN:
                        pixelValue[QT_BITMAP_GREEN_CHANNEL_INDEX] = channelValue;
                        break;

                    case OA_TEXEL_FORMAT_BLUE:
                        pixelValue[QT_BITMAP_BLUE_CHANNEL_INDEX] = channelValue;
                        break;

                    case OA_TEXEL_FORMAT_ALPHA:
                        pixelValue[QT_BITMAP_ALPHA_CHANNEL_INDEX] = channelValue;
                        break;

                    default:
                        GT_ASSERT(false);
                        break;
                }
            }
        }
    }

    // Write RGBA values:
    memcpy(pPixelData, &pixelValue, AC_OUTPUT_IMAGE_BYTES_PER_PIXEL);
}

// ---------------------------------------------------------------------------
// Name:        acRawFileHandler::getPixelChannelValue
// Description: Return the channel value (according to the channel index) from
//              the current offset in the raw data pointer
// Arguments:   pDataOffset - The raw data offset, pointing to the current pixel
//              componentIndex - The index of the channel to retrieve it's value
//              pParameter - The apParameter which will be read from the raw data
// Return Val:  bool - Success / Failure
// Author:      Eran Zinman
// Date:        9/12/2007
// ---------------------------------------------------------------------------
bool acRawFileHandler::getPixelChannelValue(gtUByte* pDataOffset, int componentIndex, apPixelValueParameter*& pParameter)
{
    bool retVal = false;

    // Sanity check:
    if (_pPixelValueDataParameter)
    {
        retVal = true;

        // On regular data types, just move offset to the current channel index
        if (_amountOfComponentsInDataType == 1)
        {
            pDataOffset += componentIndex * _dataTypeSize;
        }

        // Read data from raw data pointer into the apParameter
        _pPixelValueDataParameter->readValueFromPointer((void*)pDataOffset);

        // If we got a few components in the data type, we need to take only the relevant channel bits
        if (_amountOfComponentsInDataType > 1)
        {
            retVal = false;

            if (getDataTypeComponentValue(_componentDataType, componentIndex, _pPixelValueDataParameter))
            {
                // Of many bytes enter into this bit mask
                int amountOfComponentBits = oaAmountOfComponentBits(_componentDataType, componentIndex);
                GT_IF_WITH_ASSERT(amountOfComponentBits != -1)
                {
                    // Set the component value multiplier which is basically 256 / (2^amount of component bits)
                    double valueMultiplier = (double)0xff / (double)(1 << amountOfComponentBits);
                    _pPixelValueDataParameter->setValueMultiplier(valueMultiplier);

                    retVal = true;
                }
            }
        }

        // Return the GLDataParameter
        pParameter = _pPixelValueDataParameter;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        acRawFileHandler::calculatePageOffset
// Description: Sets the current active page in the raw data
// Arguments:   pageIndex - The index of the page of which we need to
//              calculate it's offset.
// Return Val:  bool - Success / Failure
// Author:      Eran Zinman
// Date:        8/12/2007
// ---------------------------------------------------------------------------
bool acRawFileHandler::setActivePage(int pageIndex)
{
    bool retVal = false;

    // If all data was loaded successfully
    if (_isLoadedSuccesfully)
    {
        int realPageIndex = (pageIndex * m_pageStride) + m_pageOffset;

        // Range check
        GT_IF_WITH_ASSERT((realPageIndex >= 0) && (realPageIndex < _amountOfPages))
        {
            // Calculate the offset in the raw data
            _activePageRawDataOffset = _rawDataPixelSize * _width * _height * realPageIndex;

            // Save the currently active page
            _activePage = realPageIndex;

            retVal = true;
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        acRawFileHandler::setMiddlePageAsActivePage
// Description: Sets the currently active page to be the middle page in
//              the raw data
// Author:      Eran Zinman
// Date:        4/1/2008
// ---------------------------------------------------------------------------
void acRawFileHandler::setMiddlePageAsActivePage()
{
    // If image have several pages, use middle page
    if (amountOfExternalPages() > 1)
    {
        // Calculate middle page position
        int middlePage = (amountOfExternalPages() - 1) / 2;

        // Set active page to be middle page
        bool rc = setActivePage(middlePage);
        GT_ASSERT(rc);
    }
}


// ---------------------------------------------------------------------------
// Name:        acRawFileHandler::getDataTypeComponentValue
// Description: Inputs a multiple component data type and return the value
//              of a specific component.
// Arguments:   dataType - The dataType we need to handle.
//              componentIndex - The component to take the value from.
//              srcValue - The original compacted bit array.
//              dstValue - Output value of the specific bits channel.
// Return Val:  bool - Success / Failure
// Author:      Eran Zinman
// Date:        8/12/2007
// ---------------------------------------------------------------------------
bool acRawFileHandler::getDataTypeComponentValue(oaDataType dataType, int componentIndex, apPixelValueParameter* pParameter)
{
    // This is a bit mask for a given amount of bits. Make it static so it won't initialize on every call
    static int bitsMask[11] = {0,      /* 0000000000 binary mask */
                               0x1,    /* 0000000001 binary mask */
                               0x3,    /* 0000000011 binary mask */
                               0x7,    /* 0000000111 binary mask */
                               0xF,    /* 0000001111 binary mask */
                               0x1F,   /* 0000011111 binary mask */
                               0x3F,   /* 0000111111 binary mask */
                               0x7F,   /* 0001111111 binary mask */
                               0xFF,   /* 0011111111 binary mask */
                               0x1FF,  /* 0111111111 binary mask */
                               0x3FF
                              }; /* 1111111111 binary mask */
    bool retVal = true;

    // Sanity check
    GT_IF_WITH_ASSERT(pParameter != NULL)
    {
        // Get total amount of data type components
        int amountOfDataTypeComponents = oaAmountComponentsInDataType(dataType);
        GT_IF_WITH_ASSERT(amountOfDataTypeComponents != -1)
        {
            // Sum up all the bits from right of the desired value in the dataType
            int sumOfBitsToDesiredValue = 0;

            for (int i = amountOfDataTypeComponents - 1; i > componentIndex; i--)
            {
                int amountOfComponentBits = oaAmountOfComponentBits(dataType, i);
                GT_IF_WITH_ASSERT(amountOfComponentBits != -1)
                {
                    sumOfBitsToDesiredValue += amountOfComponentBits;
                }
                else
                {
                    retVal = false;
                }
            }

            if (retVal)
            {
                retVal = false;

                // Get the desired byte bits number
                int amountOfDesiredValueBits = oaAmountOfComponentBits(dataType, componentIndex);
                GT_IF_WITH_ASSERT(amountOfDesiredValueBits != -1)
                {
                    pParameter->shiftAndMaskBits(bitsMask[amountOfDesiredValueBits], sumOfBitsToDesiredValue);

                    retVal = true;
                }
            }
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        acRawFileHandler::setBufferDataFormat
// Description: Sets the file handler data format.
//              Notice: This function can only be called for VBO file handler
// Arguments: oaTexelDataFormat texelDataFormat
// Return Val: void
// Author:      Sigal Algranaty
// Date:        16/4/2009
// ---------------------------------------------------------------------------
void acRawFileHandler::setBufferDataFormat(oaTexelDataFormat texelDataFormat)
{
    // Check if the current format is either VBO formats, or an OpenCL buffer format:
    bool isCurrentFormatVBO = ((_texelDataFormat >= OA_TEXEL_FORMAT_FIRST_GL_BUFFER_FORMAT) && (_texelDataFormat <= OA_TEXEL_FORMAT_LAST_GL_BUFFER_FORMAT));
    isCurrentFormatVBO = isCurrentFormatVBO || ((_texelDataFormat >= OA_TEXEL_FORMAT_FIRST_CL_BUFFER_FORMAT) && (_texelDataFormat <= OA_TEXEL_FORMAT_LAST_CL_BUFFER_FORMAT));

    // Check if the new format is either VBO formats, or an OpenCL buffer format:
    bool isNewFormatVBO = (texelDataFormat >= OA_TEXEL_FORMAT_FIRST_GL_BUFFER_FORMAT) && (texelDataFormat <= OA_TEXEL_FORMAT_LAST_GL_BUFFER_FORMAT);
    isNewFormatVBO = isNewFormatVBO || ((texelDataFormat >= OA_TEXEL_FORMAT_FIRST_CL_BUFFER_FORMAT) && (texelDataFormat <= OA_TEXEL_FORMAT_LAST_CL_BUFFER_FORMAT));

    GT_IF_WITH_ASSERT(isNewFormatVBO && isCurrentFormatVBO)
    {
        _texelDataFormat = texelDataFormat;
    }
}


// ---------------------------------------------------------------------------
// Name:        acRawFileHandler::setDataFormatAndAdaptSize
// Description: Sets the raw file handler data format and size.
// Arguments: oaTexelDataFormat texelDataFormat
// Return Val: void
// Author:      Sigal Algranaty
// Date:        5/8/2009
// ---------------------------------------------------------------------------
bool acRawFileHandler::setDataFormatAndAdaptSize(GLenum openGLDataFormat)
{
    bool retVal = false;
    GLuint pixelSize = 0;
    bool rc1 = apGetPixelSizeInBitsByInternalFormat(openGLDataFormat, pixelSize);
    GT_IF_WITH_ASSERT(rc1)
    {
        // Override the raw file handler with the texture buffer format:
        oaTexelDataFormat dataFormat = OA_TEXEL_FORMAT_UNKNOWN;
        bool rc2 = oaGLEnumToTexelDataFormat(openGLDataFormat, dataFormat);
        GT_IF_WITH_ASSERT(rc2)
        {
            // Set the texel data format:
            _texelDataFormat = dataFormat;
        }

        // Initialize the amount of pixel components according to the new format:
        _amountOfPixelComponents = oaAmountOfTexelFormatComponents(_texelDataFormat);

        // Convert the format to data type:
        bool rc3 = apGLTexture::textureBufferFormatToDataType(openGLDataFormat, _componentDataType);
        GT_IF_WITH_ASSERT(rc3)
        {
            _rawDataPixelSize = oaCalculatePixelUnitByteSize(_texelDataFormat, _componentDataType);
            GT_IF_WITH_ASSERT(_rawDataPixelSize != -1)
            {
                // Override the data type size:
                _dataTypeSize = oaSizeOfDataType(_componentDataType);

                // Override the parameter according to new component type:
                if (_pPixelValueDataParameter != NULL)
                {
                    clearDataParameter();
                }

                _pPixelValueDataParameter = createPixelValueDataParameter();
                GT_IF_WITH_ASSERT(_pPixelValueDataParameter != NULL)
                {
                    // Adjust the size:
                    float newSize = (float)_width * (float)_height;
                    newSize = (newSize / (float)pixelSize);
                    _width = (int)floorf(newSize);
                    _height = 1;
                    retVal = true;
                }
            }
        }
    }
    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        acRawFileHandler::setPageStride
// Description: Sets the page stride and offset. This only works properly
//              If the data is loaded but not yet accessed.
// Author:      Uri Shomroni
// Date:        21/7/2015
// ---------------------------------------------------------------------------
void acRawFileHandler::setPageStride(int pageStride, int pageOffset)
{
    GT_IF_WITH_ASSERT((0 < pageStride) && (-1 < pageOffset) && (pageOffset < pageStride))
    {
        int activePageIndex = activePage();

        m_pageStride = pageStride;
        m_pageOffset = pageOffset;

        // Fix the offset with this new data
        if (activePageIndex < amountOfExternalPages())
        {
            setActivePage(activePageIndex);
        }
        else
        {
            setMiddlePageAsActivePage();
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        acRawFileHandler::checkPixelFilter
// Description: Checks is a pixel should / shouldn't be displayed according to the
//              raw file handler containing the filter
// Arguments:   gtUByte* pSrcRawData
//              bool& shouldDisplay
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        9/3/2011
// ---------------------------------------------------------------------------
bool acRawFileHandler::checkPixelFilter(int row, int col, bool& shouldDisplayPixel)
{
    bool retVal = true;

    // Pixel is displayed by default:
    shouldDisplayPixel = true;

    for (int i = 0; i < AC_MAX_RAW_FILE_FILTER_HANDLERS; i++)
    {
        bool passesCurrentFilter = false;
        bool filterSuccess = false;

        if (_pFilterRawFileHandler[i] != NULL)
        {
            apPixelValueParameter* pFilterParameter = NULL;

            // Calculate real y position:
            int yPos = row / _amountOfPixelComponents;
            int xPos = col;

            // Get the filter raw data value:
            bool test = false;
            bool rcFilterDataValue = _pFilterRawFileHandler[i]->getRawDataComponentValue(xPos, yPos, 0, pFilterParameter, test);
            GT_IF_WITH_ASSERT(rcFilterDataValue)
            {
                // Get the filter parameter pixel value:
                GLubyte pixelValue = 0;
                bool rcGetPixelValue = _pFilterRawFileHandler[i]->getNormalizedPixelValue(pFilterParameter, 0, pixelValue);
                GT_IF_WITH_ASSERT(rcGetPixelValue)
                {
                    passesCurrentFilter = (pixelValue != 0);
                    filterSuccess = true;
                }
            }
        }
        else
        {
            // No filter:
            passesCurrentFilter = true;
            filterSuccess = true;
        }

        shouldDisplayPixel = shouldDisplayPixel && passesCurrentFilter;
        retVal = retVal && filterSuccess;
    }

    return retVal;
}
