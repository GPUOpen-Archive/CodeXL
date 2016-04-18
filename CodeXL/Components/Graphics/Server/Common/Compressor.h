//==============================================================================
// Copyright (c) 2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief Experimental code to compress data returned to the client (currently unused)
//==============================================================================

#include <stdio.h>
#include <sstream>

#ifdef USE_GZIP

#include "zlib.h"

#define CHUNK 0x4000
#define windowBits 15
#define GZIP_ENCODING 16

#define CALL_ZLIB(x) {                                                  \
        int status;                                                     \
        status = x;                                                     \
        if (status < 0) {                                               \
            fprintf (stderr,                                            \
                     "%s:%d: %s returned a bad status of %d.\n",        \
                     __FILE__, __LINE__, #x, status);                   \
            exit (EXIT_FAILURE);                                        \
        }                                                               \
    }


unsigned int GetCompressedSize();

unsigned char* GetCompressedData();

void Compress(const char* pMessage);

void CleanupCompressedData();

/*class Compressor
{
public:

   Compressor::Compressor()
      :  m_pData(NULL),
         m_nLength(0)
   {
   }

   Compressor::~Compressor()
   {
      if ( m_pData != NULL )
      {
         free(m_pData);
      }
   }

   void Compress(const char* pMessage);


   unsigned int GetCompressedSize()
   {
      return m_nLength;
   }

   unsigned char *GetCompressedData()
   {
      return m_pData;
   }

   void Cleanup()
   {
      if ( m_pData != NULL )
      {
         free(m_pData);
      }

      m_nLength = 0;
   }



private:

   unsigned char* m_pData;

   unsigned int m_nLength;

};*/

#endif
