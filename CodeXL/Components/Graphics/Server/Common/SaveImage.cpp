//==============================================================================
// Copyright (c) 2007-2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  Saving images to JPG for PerfStudio
//==============================================================================

#undef NOMINMAX
#if defined (_WIN32)
    #include <windows.h>
    #include <fstream>
    #include "objidl.h"
    #include "wtypes.h"
    #pragma warning( push )
    #pragma warning( disable : 4458)
    #include "gdiplus.h"
    #pragma warning( pop )
#elif defined (_LINUX)
    #include <stdio.h>
    #include "WinDefs.h"
    #include "png.h"

    #define XMD_H       // INT32 etc already defined
    //extern "C"
    //{
    #include "jpeglib.h"
    #include "jerror.h"
    #include "jpegint.h"
    //}

    static const int IMAGE_QUALITY = 90;        // quality of output image (100 best, 0 worst)
    static const int MIN_IMAGE_DIMENSION = 64;  // minimum dimension of image for space allocation

#endif
#include "SaveImage.h"
#include "Logger.h"

// Windows-specific implementation using the GDI API
#if defined (_WIN32)

using namespace Gdiplus;

static int GetEncoderClsid(const WCHAR* format, CLSID* pClsid)
{
    UINT  num = 0;          // number of image encoders
    UINT  size = 0;         // size of the image encoder array
    // in bytes

    ImageCodecInfo* pImageCodecInfo = nullptr;

    GetImageEncodersSize(&num, &size);

    if (size == 0)
    {
        return -1;
    }

    pImageCodecInfo = (ImageCodecInfo*)(malloc(size));
    PsAssert(pImageCodecInfo != nullptr)

    if (pImageCodecInfo == nullptr)
    {
        return -1;
    }

    GetImageEncoders(num, size, pImageCodecInfo);

    for (UINT j = 0; j < num; ++j)
    {
        if (wcscmp(pImageCodecInfo[j].MimeType, format) == 0)
        {
            *pClsid = pImageCodecInfo[j].Clsid;
            free(pImageCodecInfo);
            return j;  // Success
        }
    }

    free(pImageCodecInfo);
    return -1;
}

static bool EncodeBitmap(Bitmap& image, const WCHAR* pcszEncodeFormat, UINT32* ulSize, unsigned char** pData)
{
    // Setup encoder parameters
    // Create stream with 0 size
    IStream* pIStream = nullptr;

    if (CreateStreamOnHGlobal(nullptr, TRUE, (LPSTREAM*)&pIStream) != S_OK)
    {

        Log(logERROR, "Failed to create stream on global memory!\n");
        return false;
    }

    CLSID pngClsid;
    GetEncoderClsid(pcszEncodeFormat, &pngClsid);

    // Setup encoder parameters
    EncoderParameters encoderParameters;
    EncoderParameters* pEncoderParameters = &encoderParameters;
    ULONG quality = 50; // setup compression level for jpeg

    if (wcscmp(pcszEncodeFormat, L"image/jpeg") == 0)
    {
        encoderParameters.Count = 1;
        encoderParameters.Parameter[0].Guid = EncoderQuality;
        encoderParameters.Parameter[0].Type = EncoderParameterValueTypeLong;
        encoderParameters.Parameter[0].NumberOfValues = 1;

        // setup compression level
        encoderParameters.Parameter[0].Value = &quality;
    }
    else if (wcscmp(pcszEncodeFormat, L"image/png") == 0)
    {
        pEncoderParameters = nullptr;
    }
    else
    {
        Log(logERROR, "Failed to save image: Unrecognized format.");
        return false;
    }

    //  Save the image to the stream
    Status SaveStatus = image.Save(pIStream, &pngClsid, pEncoderParameters);

    if (SaveStatus != Ok)
    {
        // this should free global memory used by the stream

        // according to MSDN

        pIStream->Release();
        Log(logERROR, "Failed to save to stream!\n");
        return false;
    }

    // get the size of the stream
    ULARGE_INTEGER ulnSize;
    LARGE_INTEGER lnOffset;
    lnOffset.QuadPart = 0;

    if (pIStream->Seek(lnOffset, STREAM_SEEK_END, &ulnSize) != S_OK)
    {
        pIStream->Release();
        Log(logERROR, "Failed to get the size of the stream!\n");
        return false;
    }

    // now move the pointer to the beginning of the file
    if (pIStream->Seek(lnOffset, STREAM_SEEK_SET, nullptr) != S_OK)
    {
        pIStream->Release();
        Log(logERROR, "Failed to move the file pointer to the beginning of the stream!\n");
        return false;
    }

    unsigned char* pBuff = (unsigned char*)malloc((size_t)ulnSize.QuadPart);
    PsAssert(pBuff != nullptr)

    if (pBuff == nullptr)
    {
        return false;
    }

    ULONG ulBytesRead;

    if (pIStream->Read(pBuff, (ULONG)ulnSize.QuadPart, &ulBytesRead) != S_OK)
    {
        pIStream->Release();
        free(pBuff);
        return false;
    }

    *pData  = pBuff;
    *ulSize  = ulBytesRead;
    /*
       // I am going to save it to the file just so we can
       // load the jpg to a gfx program
       FILE *fFile;
       fFile = fopen("c:\\test.jpg", "w");
       if(fFile)
       {
           char *pBuff = new char[ulnSize.QuadPart];

           // Read the stream directly into the buffer

           ULONG ulBytesRead;
           if(pIStream->Read(pBuff, ulnSize.QuadPart, &ulBytesRead) != S_OK)
           {
               pIStream->Release();
               delete pBuff;
               return false;
           }

           fwrite(pBuff, ulBytesRead, 1, fFile);
           fclose(fFile);
           delete pBuff;
       }
       else printf("Failed to save data to the disk!");

       // Free memory used by the stream
    */
    pIStream->Release();

    return true;
}

/// Simple structure that holds r, g, b values as unsigned chars
struct RGBPixel
{
    unsigned char r; ///< byte to represent red channel
    unsigned char g; ///< byte to represent green channel
    unsigned char b; ///< byte to represent blue channel
};

static bool _RGBtoJpeg(unsigned char* pDIB, int iWidth, int iHeight, UINT32* ulSize, unsigned char** pData)
{
    // Create the dest image
    Bitmap DestBmp(iWidth, iHeight, PixelFormat24bppRGB);
    Rect rect1(0, 0, iWidth, iHeight);

    BitmapData bitmapData;
    memset(&bitmapData, 0, sizeof(bitmapData));
    DestBmp.LockBits(
        &rect1,
        ImageLockModeRead,
        PixelFormat24bppRGB,
        &bitmapData);

    RGBPixel* DestPixels = (RGBPixel*)bitmapData.Scan0;

    if (!pDIB)
    {
        return false;
    }

    for (UINT row = 0; row < bitmapData.Height; ++row)
    {
        RGBPixel* pP = &DestPixels[(bitmapData.Height - row - 1) * bitmapData.Width ];

        for (UINT col = 0; col < bitmapData.Width; ++col)
        {
            pP->b = *(pDIB++);
            pP->g = *(pDIB++);
            pP->r = *(pDIB++);
            pDIB++;
            pP++;
        }
    }

    DestBmp.UnlockBits(&bitmapData);

    return (EncodeBitmap(DestBmp, L"image/jpeg", ulSize, pData));
}

static bool _RGBAtoPNG(unsigned char* pDIB, int iWidth, int iHeight, UINT32* ulSize, unsigned char** pData)
{
    if (pDIB == nullptr || ulSize == nullptr || pData == nullptr)
    {
        return false;
    }

    // Create the dest image
    Bitmap DestBmp(iWidth, iHeight, PixelFormat32bppARGB);
    Rect rect1(0, 0, iWidth, iHeight);

    BitmapData bitmapData;
    memset(&bitmapData, 0, sizeof(bitmapData));
    DestBmp.LockBits(
        &rect1,
        ImageLockModeRead,
        PixelFormat32bppARGB,
        &bitmapData);

    int nStride1 = bitmapData.Stride;

    if (nStride1 < 0)
    {
        nStride1 = -nStride1;
    }

    char* pDestData = (char*)bitmapData.Scan0;

    // copy the data, inverting the image top->bottom, and RGBA -> BGRA
    unsigned char* pSrcData = nullptr;

    for (INT row = bitmapData.Height - 1; row >= 0; --row)
    {
        pSrcData = &(pDIB[row * bitmapData.Stride]);

        for (UINT col = 0; col < bitmapData.Width; ++col)
        {
            char cRed = *pSrcData++;
            char cGreen = *pSrcData++;
            char cBlue = *pSrcData++;
            char cAlpha = *pSrcData++;

            *pDestData++ = cBlue;
            *pDestData++ = cGreen;
            *pDestData++ = cRed;
            *pDestData++ = cAlpha;
        }
    }

    DestBmp.UnlockBits(&bitmapData);

    return (EncodeBitmap(DestBmp, L"image/png", ulSize, pData));
}
#endif // def WIN32

////////////////////////////////////////////////////////////////////////////////////
/// Helper function to convert an image format string (e.g. "PNG") to a type.
/// \param strFormat The input string format.
/// \return The image format type
////////////////////////////////////////////////////////////////////////////////////
OGL_IMAGE_FILE_FORMAT GetOGLFileFormat(std::string strFormat)
{
    if (strstr(strFormat.c_str(), "PNG") != nullptr)
    {
        return OGL_IFF_PNG;
    }
    else if (strstr(strFormat.c_str(), "JPG") != nullptr)
    {
        return OGL_IFF_JPG;
    }
    // Uncomment this when we have support for saving a texture as DDS.
    //else if ( strstr ( strFormat.c_str(), "DDS" ) != nullptr )
    //{
    //   return OGL_IFF_DDS;
    //}
    else if (strstr(strFormat.c_str(), "BMP") != nullptr)
    {
        return OGL_IFF_BMP;
    }
    else
    {
        // Default to PNG
        return OGL_IFF_PNG;
    }
}

#if defined (_LINUX)

static const int INPUT_BUF_SIZE = 4096;

// -----------------------------------------------------------------------
// JPEG error handling
// Will "longjmp" on error_exit.
// -----------------------------------------------------------------------

struct ErrorHandler
{
    // "subclass" of jpeg_error_mgr
    struct jpeg_error_mgr errorMgr;
    jmp_buf setjmpBuffer;

    ErrorHandler(j_decompress_ptr cinfo)
    {
        Init((j_common_ptr)cinfo);
    }

    ErrorHandler(j_compress_ptr cinfo)
    {
        Init((j_common_ptr)cinfo);
    }

    void Init(j_common_ptr cinfo)
    {
        // setup the standard error handling.
        cinfo->err = jpeg_std_error(&errorMgr);

        // then hook up our error_exit function.
        errorMgr.error_exit = &ErrorHandler::OnErrorExit;
    }

    static void OnErrorExit(j_common_ptr cinfo)
    {
        // recover the pointer to "derived class" instance
        ErrorHandler* errorHandler = (ErrorHandler*)cinfo->err;

        // use the default error message output.
        (*cinfo->err->output_message)(cinfo);

        // return control to the setjmp point.
        longjmp(errorHandler->setjmpBuffer, 1);
    }
};

// Duplicate of struct in jdatadst.c. Needed to get size of JPEG image size
typedef struct
{
    struct jpeg_destination_mgr pub; /* public fields */

    unsigned char** outbuffer;   /* target buffer */
    unsigned long* outsize;
    unsigned char* newbuffer;    /* newly allocated buffer */
    JOCTET* buffer;      /* start of buffer */
    size_t bufsize;
} my_mem_destination_mgr;

typedef my_mem_destination_mgr* my_mem_dest_ptr;

struct State
{
    unsigned char* m_WriteBuffer;
    unsigned long  m_Size;
};

static  void    pngWriteData(png_structp png_ptr, png_bytep data, png_size_t length)
{
    if (png_ptr == nullptr)
    {
        return;
    }

    State*   state = (State*)(png_get_io_ptr(png_ptr));
    unsigned char*   ptr = state->m_WriteBuffer;
    ptr += state->m_Size;
    memcpy(ptr, data, length);
    state->m_Size += length;
}

static  void    pngFlush(__attribute__((unused)) png_structp png_ptr)
{
}

/*
// Invert the image top to bottom
static  void    FlipImage(unsigned char* pImageData, int width, int height)
{
   int* topPtr = (int*)pImageData;
   int* bottomPtr = topPtr + (width * (height-1));
   int lineCount = height / 2;
   for (int h = 0; h < lineCount; h++)
   {
      for (int w = 0; w < width; w++)
      {
         int tmp = *topPtr;
         *topPtr++ = *bottomPtr;
         *bottomPtr++ = tmp;
      }
      bottomPtr -= (width*2);
   }
}
*/

// Use a wrapper to encapsulate the setjmp call to suppress Linux compile warning (-Wclobbered)
static bool SetjmpWrapper(ErrorHandler& err)
{
    if (setjmp(err.setjmpBuffer))
    {
        return true;
    }
    return false;
}

static  bool    _RGBtoJpeg(unsigned char* pImageData, int width, int height, UINT32* outSize, unsigned char** outBuffer)
{
    unsigned char* writeBuffer = nullptr;
    static const int bytes_per_pixel = 4;

    struct jpeg_compress_struct cinfo;
    ErrorHandler err(&cinfo);

    if (SetjmpWrapper(err))
    {
        /* If we get here, the JPEG code has signaled an error.
        * We need to clean up the JPEG object, close the input file, and return.
        */
        jpeg_destroy_compress(&cinfo);
        return false;
    }

    // allocate enough memory for the resultant image. Ensure very small images
    // have more than enough memory; sometimes the compressed image is larger
    // than the source image leading to buffer overruns
    int minWidth = std::max(MIN_IMAGE_DIMENSION, width);
    int minHeight = std::max(MIN_IMAGE_DIMENSION, height);
    writeBuffer = (unsigned char*)malloc(minWidth * minHeight * bytes_per_pixel);
    jpeg_create_compress(&cinfo);

    //jpeg_set
    cinfo.image_width = width;
    cinfo.image_height = height;
    cinfo.input_components = 3;
    cinfo.in_color_space = JCS_RGB;
    jpeg_set_defaults(&cinfo);
    jpeg_quality_scaling(IMAGE_QUALITY);
    jpeg_set_quality(&cinfo, IMAGE_QUALITY, FALSE);
    cinfo.dct_method = JDCT_FLOAT;

    jpeg_mem_dest(&cinfo, &writeBuffer, (unsigned long*)outSize);

    jpeg_start_compress(&cinfo, TRUE);

    // use a larger buffer size for small images
    int row_stride = minWidth * 3;

    unsigned char* aTempBuffer = new unsigned char[row_stride];

    //   FlipImage( pImageData, width, height);
    int* aSrcPtr = (int*)pImageData;

    for (int aRow = 0; aRow < height; aRow++)
    {
        unsigned char* aDest = aTempBuffer;

        for (int aCol = 0; aCol < width; aCol++)
        {
            unsigned long src = *(aSrcPtr++);
            *aDest++ = (src) & 0xFF;
            *aDest++ = (src >>  8) & 0xFF;
            *aDest++ = (src >> 16) & 0xFF;
        }

        jpeg_write_scanlines(&cinfo, &aTempBuffer, 1);
    }

    //   FlipImage( pImageData, width, height);

    delete [] aTempBuffer;

    jpeg_finish_compress(&cinfo);

    // get the size of the compressed buffer
    my_mem_dest_ptr dest = (my_mem_dest_ptr) cinfo.dest;
    *outSize = *dest->outsize;

    jpeg_destroy_compress(&cinfo);

    // Allocate a buffer the size of the jpg image and copy the data to it
    // There's no way of knowing how much space will be needed until the
    // image has been compressed
    unsigned char* outBuff = (unsigned char*)malloc(*outSize);
    memcpy(outBuff, writeBuffer, *outSize);
    free(writeBuffer);

    *outBuffer = outBuff;

    return true;
}

static   bool    _RGBAtoPNG(unsigned char* pImageData, int width, int height, UINT32* outSize, unsigned char** outBuffer)
{
    static State state;
    png_structp png_ptr;
    png_infop info_ptr;
    static const int bytes_per_pixel = 4;

    // initialize stuff
    png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);

    if (!png_ptr)
    {
        Log(logERROR, "[write_png_file] png_create_write_struct failed");
        return false;
    }

    info_ptr = png_create_info_struct(png_ptr);

    if (!info_ptr)
    {
        png_destroy_write_struct(&png_ptr, nullptr);
        Log(logERROR, "[write_png_file] png_create_info_struct failed");
        return false;
    }

    if (setjmp(png_jmpbuf(png_ptr)))
    {
        png_destroy_write_struct(&png_ptr, &info_ptr);
        Log(logERROR, "[write_png_file] Error during init_io");
        return false;
    }

    // allocate enough memory for the resultant image. Ensure very small images
    // have more than enough memory; sometimes the compressed image is larger
    // than the source image leading to buffer overruns
    int minWidth = std::max(MIN_IMAGE_DIMENSION, width);
    int minHeight = std::max(MIN_IMAGE_DIMENSION, height);
    state.m_WriteBuffer = (unsigned char*)malloc(minWidth * minHeight * bytes_per_pixel);
    state.m_Size = 0;

    // set the write function to write to a buffer rather than a file
    png_set_write_fn(png_ptr, &state, pngWriteData, pngFlush);

    // write header
    if (setjmp(png_jmpbuf(png_ptr)))
    {
        png_destroy_write_struct(&png_ptr, &info_ptr);
        Log(logERROR, "[write_png_file] Error during writing header");
        return false;
    }

    png_set_IHDR(png_ptr, info_ptr, width, height,
                 8, PNG_COLOR_TYPE_RGB_ALPHA, PNG_INTERLACE_NONE,
                 PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);

    png_write_info(png_ptr, info_ptr);

    // write bytes
    if (setjmp(png_jmpbuf(png_ptr)))
    {
        png_destroy_write_struct(&png_ptr, &info_ptr);
        Log(logERROR, "[write_png_file] Error during writing bytes");
        return false;
    }

    //   FlipImage( pImageData, width, height );

    png_bytepp row_pointers = new png_bytep[height];

    int maxHeight = PNG_UINT_32_MAX / sizeof(png_bytep);

    if (height > maxHeight)
    {
        Log(logERROR, "Image is too tall to process in memory");
    }

    for (int k = 0; k < height; k++)
    {
        row_pointers[k] = pImageData + k * width * bytes_per_pixel;
    }

    png_write_image(png_ptr, row_pointers);

    //   FlipImage( pImageData, width, height);

    // end write
    if (setjmp(png_jmpbuf(png_ptr)))
    {
        png_destroy_write_struct(&png_ptr, &info_ptr);
        Log(logERROR, "[write_png_file] Error during end of write");
        return false;
    }

    png_write_end(png_ptr, info_ptr);

    png_destroy_write_struct(&png_ptr, &info_ptr);

    // cleanup heap allocation
    delete [] row_pointers;

    // Allocate a buffer the size of the jpg image and copy the data to it
    // There's no way of knowing how much space will be needed until the
    // image has been compressed
    unsigned char* outBuff = (unsigned char*)malloc(state.m_Size);
    memcpy(outBuff, state.m_WriteBuffer, state.m_Size);
    free(state.m_WriteBuffer);

    *outSize = state.m_Size;
    *outBuffer = outBuff;

    return true;
}

#endif   // _WIN32

bool RGBtoJpeg(unsigned char* pDIB, int iWidth, int iHeight, UINT32* ulSize, unsigned char** pData)
{
#ifdef _WIN32
    GdiplusStartupInput gdiplusStartupInput;
    ULONG_PTR gdiplusToken;
    GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, nullptr);

    bool res = _RGBtoJpeg(pDIB, iWidth, iHeight, ulSize, pData);

    GdiplusShutdown(gdiplusToken);
#else
    bool res = _RGBtoJpeg(pDIB, iWidth, iHeight, ulSize, pData);
#endif

    return res;
}

bool RGBAtoPNG(unsigned char* pDIB, int iWidth, int iHeight, UINT32* ulSize, unsigned char** pData)
{
#ifdef _WIN32
    GdiplusStartupInput gdiplusStartupInput;
    ULONG_PTR gdiplusToken;
    GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, nullptr);

    bool res = _RGBAtoPNG(pDIB, iWidth, iHeight, ulSize, pData);

    GdiplusShutdown(gdiplusToken);
#else
    bool res = _RGBAtoPNG(pDIB, iWidth, iHeight, ulSize, pData);
#endif

    return res;
}

bool RGBAtoBMP(unsigned char* pImageData, int iWidth, int iHeight, UINT32* pulSize, unsigned char** ppOutData)
{
    if (pImageData == nullptr ||
        iWidth == 0 ||
        iHeight == 0 ||
        pulSize == nullptr ||
        ppOutData == nullptr)
    {
        return false;
    }

    DWORD dwBytesPerPixel = 4;

    DWORD dwBMPWidth = (DWORD)iWidth;
    DWORD dwBMPHeight = (DWORD)iHeight;
    DWORD dwDataSize = dwBytesPerPixel * dwBMPWidth * dwBMPHeight;

    DWORD dwSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + dwDataSize;

    char* pDataBuffer = (char*) malloc(dwSize);

    if (pDataBuffer == nullptr)
    {
        Log(logERROR, "Failed to allocate memory in %s\n", __FUNCTION__);
        return false;
    }

    // now setup the bitmap header

    BITMAPFILEHEADER* pBMFH = (BITMAPFILEHEADER*) pDataBuffer;
    pBMFH->bfType = ((USHORT)(BYTE)('B') | ((USHORT)(BYTE)('M') << 8));
    pBMFH->bfSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + dwDataSize;
    pBMFH->bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
    pBMFH->bfReserved1 = 0;
    pBMFH->bfReserved2 = 0;

    BITMAPINFOHEADER* pBMINFO = (BITMAPINFOHEADER*)(pDataBuffer + sizeof(BITMAPFILEHEADER));
    pBMINFO->biSize = sizeof(BITMAPINFOHEADER);
    pBMINFO->biWidth = dwBMPWidth;
    pBMINFO->biHeight = dwBMPHeight;
    pBMINFO->biCompression = 0;
    pBMINFO->biSizeImage = dwDataSize;
    pBMINFO->biXPelsPerMeter = 0;
    pBMINFO->biYPelsPerMeter = 0;
    pBMINFO->biClrUsed = 0;
    pBMINFO->biClrImportant = 0;

    pBMINFO->biPlanes = 1;
    pBMINFO->biBitCount = 32;

    // fill buffer data
    char* pDestData = pDataBuffer + sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);

    unsigned char* pSrcData = pImageData;

    for (unsigned long ulRow = 0; ulRow < dwBMPHeight; ulRow++)
    {
        for (unsigned long ulCol = 0; ulCol < dwBMPWidth; ulCol++)
        {
            char cRed = *pSrcData++;
            char cGreen = *pSrcData++;
            char cBlue = *pSrcData++;
            char cAlpha = *pSrcData++;

            *pDestData++ = cBlue;
            *pDestData++ = cGreen;
            *pDestData++ = cRed;
            *pDestData++ = cAlpha;
        }

        // at the end of each row, skip the dest pointer ahead by stride
        //      pDestData += ulDestStride;
    } // end for each row

    *ppOutData = (unsigned char*)pDataBuffer;
    *pulSize = dwSize;
    return true;
}

#if USE_LIBPNG
//-----------------------------------------------------------------------------
/// Callback function used in PNG dumps.
//-----------------------------------------------------------------------------
void CustomPngWriteCb(png_structp png_ptr, png_bytep data, png_size_t length)
{
    std::vector<UCHAR>* pVec = (std::vector<UCHAR>*)png_get_io_ptr(png_ptr);
    pVec->insert(pVec->end(), data, data + length);
}

//-----------------------------------------------------------------------------
/// Take an RGBA CpuImage ptr and put it in PNG form resident in system memory.
/// Memory inside ppImage is allocated on behalf of the caller, so it is their responsibility to free it.
/// \param  pData  CPU data containing PNG pixel info
/// \param  pitch  Image pitch
/// \param  width  Image width
/// \param  height Image height
/// \return True   If successful
//-----------------------------------------------------------------------------
bool CpuImageToPngMem(
    CpuImage*          pImage,
    unsigned char**    ppPngMem,
    unsigned int*      pMemSize,
    const std::string& fileName) // Optional
{
    bool success = false;

    if ((pImage != nullptr) && (ppPngMem != nullptr) && (pMemSize != nullptr))
    {
        png_structp png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);

        if (png_ptr)
        {
            png_infop info_ptr = png_create_info_struct(png_ptr);
            png_set_IHDR(
                png_ptr,
                info_ptr,
                pImage->width,
                pImage->height,
                8,
                PNG_COLOR_TYPE_RGBA,
                PNG_INTERLACE_NONE,
                PNG_COMPRESSION_TYPE_DEFAULT,
                PNG_FILTER_TYPE_DEFAULT);

            std::vector<UCHAR*> rows(pImage->height);

            for (UINT i = 0; i < pImage->height; i++)
            {
                rows[i] = (UCHAR*)pImage->pData + i * pImage->width * 4;
            }

            png_set_rows(png_ptr, info_ptr, &rows[0]);

            std::vector<UCHAR> pngData;
            png_set_write_fn(png_ptr, &pngData, CustomPngWriteCb, nullptr);

            png_write_png(png_ptr, info_ptr, PNG_TRANSFORM_IDENTITY, nullptr);
            png_destroy_write_struct(&png_ptr, nullptr);

            const UINT memSize = (UINT)pngData.size();

            if (memSize != 0)
            {
                UCHAR* pOut = new UCHAR[memSize];

                if (pOut != nullptr)
                {
                    memcpy(pOut, &pngData[0], memSize);

                    *ppPngMem = pOut;
                    *pMemSize = memSize;

                    success = true;

                    // Optionally, dump it out to file
                    if (fileName != "")
                    {
                        std::ofstream outfile(fileName.c_str(), std::ios::out | std::ios::binary);
                        outfile.write((const char*)pOut, memSize);
                        outfile.close();
                    }
                }
            }
        }
    }

    return success;
}

#endif // USE_LIBPNG
