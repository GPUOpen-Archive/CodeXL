//==============================================================================
// Copyright (c) 2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief Experimental code to compress data returned to the client (currently unused)
//==============================================================================

#include "Compressor.h"

#ifdef USE_GZIP

static void strm_init(z_stream* strm)
{
    strm->zalloc = Z_NULL;
    strm->zfree  = Z_NULL;
    strm->opaque = Z_NULL;
    CALL_ZLIB(deflateInit2(strm, Z_DEFAULT_COMPRESSION, Z_DEFLATED,
                           windowBits | GZIP_ENCODING, 8,
                           Z_DEFAULT_STRATEGY));
}

unsigned char* m_pData;
unsigned int m_nLength;

unsigned int GetCompressedSize()
{
    return m_nLength;
}

unsigned char* GetCompressedData()
{
    return m_pData;
}

void CleanupCompressedData()
{
    if (m_pData != NULL)
    {
        free(m_pData);
    }

    m_nLength = 0;
}

void Compress(const char* pMessage)
{
    // Cleanup any previous data
    CleanupCompressedData();

    unsigned int memSize = (unsigned int)strlen(pMessage) * sizeof(unsigned char);

    m_pData = (unsigned char*) malloc(strlen(pMessage) * sizeof(unsigned char));

    unsigned char out[CHUNK];
    z_stream strm;
    strm_init(& strm);
    strm.next_in = (unsigned char*) pMessage;
    strm.avail_in = memSize;

    do
    {
        int have;
        strm.avail_out = CHUNK;
        strm.next_out = out;
        CALL_ZLIB(deflate(& strm, Z_FINISH));
        have = CHUNK - strm.avail_out;
        memcpy(&m_pData[m_nLength], out, have);
        m_nLength += have;
    }
    while (strm.avail_out == 0);

    deflateEnd(& strm);
}

#endif
