//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file acRawFileHandler.h
///
//==================================================================================

//------------------------------ acRawFileHandler.h ------------------------------

#ifndef __ACRAWFILEHANDLER
#define __ACRAWFILEHANDLER

// Qt:
#include <QImage>

// Infra:
#include <AMDTOSWrappers/Include/osFilePath.h>
#include <AMDTOSWrappers/Include/osRawMemoryStream.h>
#include <AMDTOSAPIWrappers/Include/oaDataType.h>
#include <AMDTOSAPIWrappers/Include/oaTexelDataFormat.h>
#include <AMDTAPIClasses/Include/apOpenGLParameters.h>
#include <AMDTAPIClasses/Include/apOpenCLParameters.h>

// Local:
#include <AMDTApplicationComponents/Include/acApplicationComponentsDLLBuild.h>
#include <AMDTApplicationComponents/Include/acDefinitions.h>


// ----------------------------------------------------------------------------------
// Class Name:           acRawFileHandler
// General Description:  This class loads a raw file from the disk and basically
//                       allows these two main functions:
//
//                       1. Giving access to it's raw data (for the data viewer)
//                       2. Converting raw data into an image (for the image viewer)
//
//                       The acRawFileHandler also supports raw data files with
//                       multiple pages.
//
// Author:               Eran Zinman
// Creation Date:        11/8/2007
// ----------------------------------------------------------------------------------
class AC_API acRawFileHandler
{
public:
    // Constructors:
    acRawFileHandler(bool yFlipImage = true);
    acRawFileHandler(gtUByte* pRawData, gtSizeType rawDataSize, int width, int height, oaTexelDataFormat dataFormat, oaDataType dataType, int amountOfPages);

    // Destructor:
    ~acRawFileHandler();

    // Checks if the raw file handler was loaded successfully or not
    bool isOk() const  { return _isLoadedSuccesfully; };

    // Loads raw data from file
    bool loadFromFile(const osFilePath& rawDataFile);

    // Sets the current active page in the raw data, default is page 0
    bool setActivePage(int pageIndex);

    // Sets the currently active page to be the middle page in the raw data
    void setMiddlePageAsActivePage();

    // Set filter raw file handler:
    void setFilterRawFileHandler(acRawFileHandler* pFilterRawFileHandler, unsigned int index) {if (index < AC_MAX_RAW_FILE_FILTER_HANDLERS) {_pFilterRawFileHandler[index] = pFilterRawFileHandler;};};

    // Convert the currently active page in the raw data into a QImage object:
    QImage* convertToQImage();

    // Retrieves a raw data component value from a certain (xPos, yPos) position, from the currently active page
    bool getRawDataComponentValue(int x, int y, int componentIndex, apPixelValueParameter*& pParameter, bool& isValueAvailable);

    // Return a VBO raw data component value:
    bool getBufferRawDataComponentValue(int x, int y, apPixelValueParameter*& pParameter);

    // Ask the raw data to calculate it's best fit normal
    bool normalizeValues(oaTexelDataFormat texelFormat, bool force = false);

    // Set manually minimum and maximum range values
    bool setMinMaxValues(double minValue, double maxValue);

    // Return the minimum and maximum values
    void getMinMaxValues(double& leftValue, double& rightValue);

    // Return the value multiplier
    bool getValueMultiplier(double& valueMultiplier);

    // Return raw data normalized status
    bool isNormalized() { return _valuesNormazlied; };

public:
    // Returns the height and width:
    void getSize(int& width, int& height) { height = _height; width = _width; };

    // Returns the data type and data format:
    void getDataTypeAndFormat(oaTexelDataFormat& texelDataFormat, oaDataType& componentDataType)const { texelDataFormat = _texelDataFormat; componentDataType = _componentDataType; };

    // Data format:
    oaTexelDataFormat dataFormat() const { return _texelDataFormat;};
    void setBufferDataFormat(oaTexelDataFormat texelDataFormat);
    bool setDataFormatAndAdaptSize(GLenum openGLDataFormat);

    // Page stride:
    void setPageStride(int pageStride, int pageOffset);

    // Offset:
    int offset() const {return _offset;};
    void setOffset(int offset) {_offset = offset;};

    // Stride:
    GLsizei stride() const {return _stride;};
    void setStride(GLsizei stride) {_stride = stride;};

    // Buffer display properties:
    void setDisplayProperties(oaTexelDataFormat displayFormat, int offset, GLsizei stride) {_texelDataFormat = displayFormat; _offset = offset; _stride = stride;}
    void getDisplayProperties(oaTexelDataFormat& displayFormat, int& offset, GLsizei& stride) const { displayFormat = _texelDataFormat; offset = _offset; stride = _stride; };

    // Data Type:
    oaDataType dataType() const { return _componentDataType;}

    // Get amount of pages in raw data
    int amountOfPages() const { return _amountOfPages; };
    int amountOfExternalPages() const { return _amountOfPages / ((1 > m_pageStride) ? 1 : m_pageStride); }

    // Get currently active page
    int activePage() const { return (_activePage - m_pageOffset) / ((1 > m_pageStride) ? 1 : m_pageStride); };

    // Get normalized pixel value
    bool getNormalizedPixelValue(apPixelValueParameter* pParameter, int channelIndex, GLubyte& pixelValue);

    // Get the size, in bytes, of our data:
    gtSizeType getDataSize() const {return _rawDataSize;}

    // Get the Y-flip flag state
    bool shouldYFlipImage() const { return m_yFlipImage; }

private:
    // Initializes the raw file handler after loading data
    void initHandler();

    // Create an appropriate apPixelValueParameter which will be used to reading form raw data
    apPixelValueParameter* createPixelValueDataParameter();
    void clearDataParameter();

    // Fill pixel value according to the raw data type
    void calculateQImagePixelValue(gtUByte* pSrcRawData, uchar* pPixel, bool shouldPixelBeDisplayed);

    // Fill normalized pixel value according to the raw data type
    void calculateNormalizedPixelValue(gtUByte* pSrcRawData, uchar* pPixel, bool shouldPixelBeDisplayed);

    // Function checks if negative values are allowed by the input texel format:
    bool shouldNormalizeDataNegativeValues(oaTexelDataFormat texelFormat);

private:
    // Retrieves channel value given an offset in the raw data pointing to the current pixel
    bool getPixelChannelValue(gtUByte* pDataOffset, int componentIndex, apPixelValueParameter*& pParameter);

    // Checks if a pixel should be filtered:
    bool checkPixelFilter(int row, int col, bool& shouldDisplayPixel);

    // Finds the lowest and highest values in a raw data
    bool findMinMaxValuesFromRawData(bool shouldNegativeValuesBeNormalized);

    // Calculates the value multiplier according to the minimum and maximum values
    void calculateValueMultiplier(double minValue, double maxValue);

    // Inputs a multiple component data type and return the value of a specific component
    bool getDataTypeComponentValue(oaDataType dataType, int componentIndex, apPixelValueParameter* pParameter);

private:
    // This variable flags if the raw data file was loaded successfully
    bool _isLoadedSuccesfully;

    // Width and height
    int _width;
    int _height;

    // Y-flip the image?
    bool m_yFlipImage;

    // Data format and data type
    oaDataType _componentDataType;

    // Amount of components per pixel
    oaTexelDataFormat _texelDataFormat;

    // Bytes size a of raw data pixel and amount of components
    int _rawDataPixelSize;
    int _dataTypeSize;
    int _amountOfPixelComponents;
    int _amountOfComponentsInDataType;

    // Raw data
    gtUByte* _pRawData;

    // Total size, in bytes, of data held by us:
    gtSizeType _rawDataSize;

    // Amount of pages in the raw data
    int _amountOfPages;
    int m_pageStride;
    int m_pageOffset;

    // Current active page in raw data and the offset in the raw data buffer
    int _activePage;
    unsigned long _activePageRawDataOffset;

    // Buffer offset:
    int _offset;

    // Buffer stride:
    GLsizei _stride;

    // This variable flags if the pixel value multiplier used for altering the raw data value to pixel color was set
    bool _valuesNormazlied;

    // Pixel value read parameter
    apPixelValueParameter* _pPixelValueDataParameter;

    static apGLfloatParameter* _pStaticGLFloatParamter;
    static apGLbyteParameter* _pStaticGLByteParamter;
    static apGLubyteParameter* _pStaticGLuByteParamter;
    static apGLdoubleParameter* _pStaticGLDoubleParamter;
    static apGLshortParameter* _pStaticGLShortParamter;
    static apGLushortParameter* _pStaticGLUShortParamter;
    static apGLintParameter* _pStaticGLIntParamter;
    static apGLuintParameter* _pStaticGLUIntParamter;
    static apCLlongParameter* _pStaticCLLongParamter;
    static apCLulongParameter* _pStaticCLULongParamter;
    static apCLcharParameter* _pStaticCLCharParamter;
    static apCLucharParameter* _pStaticCLUCharParamter;
    static apNotAvailableParameter* _pStaticNotAvaoilableParamter;


    // The raw data minimum, maximum and value multiplier values:
    double _minValueParameter;
    double _maxValueParameter;
    double _valueMultiplier;

    // Contain a raw file handler with a filter for this raw data:
    acRawFileHandler* _pFilterRawFileHandler[AC_MAX_RAW_FILE_FILTER_HANDLERS];
};

#endif  // __ACRAWFILEHANDLER
