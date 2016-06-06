//=====================================================================
// Copyright 2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file
///
//=====================================================================

//------------------------------ osChannelEncryptor.cpp ------------------------------

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>

// Local:
#include <AMDTOSWrappers/Include/osChannelEncryptor.h>


// ---------------------------------------------------------------------------
// Name:        osChannelEncryptor::osChannelEncryptor
// Description: Constructor.
// Arguments: wrappedChannel - The channel that this class wrap and encrypts.
//            encryptionKey - The encryption key.
// Author:      AMD Developer Tools Team
// Date:        11/10/2006
// ---------------------------------------------------------------------------
osChannelEncryptor::osChannelEncryptor(osChannel& wrappedChannel, const crBlowfishEncryptionKey& encryptionKey)
    : _wrappedChannel(wrappedChannel), _blowfishEncryptor(encryptionKey),
      _pReadBuffer(NULL), _readBufferSize(0), _pWriteBuffer(NULL), _writeBufferSize(0)
{
}


// ---------------------------------------------------------------------------
// Name:        osChannelEncryptor::~osChannelEncryptor
// Description: Destructor.
// Author:      AMD Developer Tools Team
// Date:        11/10/2006
// ---------------------------------------------------------------------------
osChannelEncryptor::~osChannelEncryptor()
{
    // Clean up:
    delete[] _pWriteBuffer;
    _pWriteBuffer = NULL;
    _writeBufferSize = 0;

    delete[] _pReadBuffer;
    _pReadBuffer = NULL;
    _readBufferSize = 0;
}


// ---------------------------------------------------------------------------
// Name:        osChannelEncryptor::restartEncryptionStream
// Description: Restarts the encryption stream.
//              See crBlowfishEncryptor::restartEncryptionStream documentation
//              for more details.
// Author:      AMD Developer Tools Team
// Date:        11/10/2006
// ---------------------------------------------------------------------------
void osChannelEncryptor::restartEncryptionStream()
{
    _blowfishEncryptor.restartEncryptionStream();
}


// ---------------------------------------------------------------------------
// Name:        osChannelEncryptor::channelType
// Description: Returns the wrapped channel type.
// Author:      AMD Developer Tools Team
// Date:        11/10/2006
// ---------------------------------------------------------------------------
osChannel::osChannelType osChannelEncryptor::channelType() const
{
    osChannel::osChannelType wrappedChannelType = _wrappedChannel.channelType();
    return wrappedChannelType;
}


// ---------------------------------------------------------------------------
// Name:        osChannelEncryptor::write
// Description: Writes data into the wrapped channel. The data will be encrypted
//              before it is written into the channel.
// Arguments: pDataBuffer - The input data buffer.
//            dataSize - The input data size.
// Return Val: bool  - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        11/10/2006
// ---------------------------------------------------------------------------
bool osChannelEncryptor::write(const gtByte* pDataBuffer, gtSize_t dataSize)
{
    bool retVal = false;

    // Allocate buffer for the encrypted data:
    bool rc1 = allocateBuffer(_pWriteBuffer, _writeBufferSize, dataSize);
    GT_IF_WITH_ASSERT(rc1)
    {
        // Encrypt the data into the write buffer:
        bool rc2 = _blowfishEncryptor.encrypt((const gtUByte*)pDataBuffer, dataSize, (gtUByte*)_pWriteBuffer);
        GT_IF_WITH_ASSERT(rc2)
        {
            // Write the encrypted data:
            bool rc3 = _wrappedChannel.write(_pWriteBuffer, dataSize);
            GT_IF_WITH_ASSERT(rc3)
            {
                retVal = true;
            }
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        osChannelEncryptor::read
// Description: Reads encrypted data from the wrapped channel. The data will
//              be encrypted before it is written into the output buffer.
// Arguments: pDataBuffer - The output buffer.
//            dataSize - Amount of data to be read.
// Return Val: bool  - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        11/10/2006
// ---------------------------------------------------------------------------
bool osChannelEncryptor::read(gtByte* pDataBuffer, gtSize_t dataSize)
{
    bool retVal = false;

    // Allocate buffer for the read cipher data:
    bool rc1 = allocateBuffer(_pReadBuffer, _readBufferSize, dataSize);
    GT_IF_WITH_ASSERT(rc1)
    {
        // Read the cipher data into the read buffer:
        bool rc2 = _wrappedChannel.read(_pReadBuffer, dataSize);
        GT_IF_WITH_ASSERT(rc2)
        {
            // Decrypt the cipher data into the output buffer:
            bool rc3 = _blowfishEncryptor.decrypt((const gtUByte*)_pReadBuffer, dataSize, (gtUByte*)pDataBuffer);
            GT_IF_WITH_ASSERT(rc3)
            {
                retVal = true;
            }
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        osChannelEncryptor::readAvailableData
// Description:
//  Reads as much encrypted data as possible from the wrapped channel.
//  The data will be encrypted before it is written into the output buffer.
//
// Arguments:   pDataBuffer - A buffer that will receive the data.
//              bufferSize - The buffer size.
//              amountOfDataRead - The amount of data actually read from the file.
// Return Val:  bool - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        24/8/2005
// ---------------------------------------------------------------------------
bool osChannelEncryptor::readAvailableData(gtByte* pDataBuffer, gtSize_t bufferSize, gtSize_t& amountOfDataRead)
{
    // This function is not implemented yet.
    return false;
}


// ---------------------------------------------------------------------------
// Name:        osChannelEncryptor::allocateBuffer
// Description:
//   Inputs an existing buffer and a data size.
//   If the buffer is not big enough to hold the data size, allocates a buffer
//   that will be sufficient to hold the data and replaces the exiting buffer with it.
//
// Arguments: pBuffer - An existing buffer (that may not be big enough for
//                         the input data size).
//            bufferSize - The existing buffer size.
//            dataSize - The data size.
// Return Val: bool  - Success / failure.
y// Author:      AMD Developer Tools Team
// Date:        11/10/2006
// ---------------------------------------------------------------------------
bool osChannelEncryptor::allocateBuffer(gtByte*& pBuffer, gtSize_t& bufferSize, gtSize_t dataSize)
{
    bool retVal = false;

    // If the old buffer is big enough for the data:
    if (dataSize <= bufferSize)
    {
        retVal = true;
    }
    else
    {
        // Delete the existing buffer:
        delete[] pBuffer;
        pBuffer = NULL;
        bufferSize = 0;

        // Allocate a new buffer:
        pBuffer = new gtByte[dataSize];
        GT_IF_WITH_ASSERT(pBuffer != NULL)
        {
            bufferSize = dataSize;
            retVal = true;
        }
    }

    return retVal;
}


