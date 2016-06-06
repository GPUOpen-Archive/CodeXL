//=====================================================================
// Copyright 2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file
///
//=====================================================================

//------------------------------ osChannelEncryptor.h ------------------------------

#ifndef __OSCHANNELENCRYPTOR_H
#define __OSCHANNELENCRYPTOR_H

// Infra:
#include <AMDTBaseTools/Include/gtIgnoreCompilerWarnings.h>
#include <GRCryptographicLibrary/crBlowfishEncryptor.h>

// Local:
#include <AMDTOSWrappers/Include/osOSDefinitions.h>
#include <AMDTOSWrappers/Include/osChannel.h>


// ----------------------------------------------------------------------------------
// Class Name:           osChannelEncryptor
// General Description:
//   A wrapper for an osChannel that encrypts its communication stream.
//
// Author:      AMD Developer Tools Team
// Creation Date:        10/10/2006
// Implementation notes:
//   - We hold the osChannel that we wrap as a member.
//   - We inherit osChannel to be able to imitate it fully (operators, etc)
// ----------------------------------------------------------------------------------
class OS_API osChannelEncryptor : public osChannel
{
public:
    osChannelEncryptor(osChannel& wrappedChannel, const crBlowfishEncryptionKey& encryptionKey);
    virtual ~osChannelEncryptor();

    void restartEncryptionStream();

    // Overrides osChannel
    virtual osChannelType channelType() const;
    virtual bool write(const gtByte* pDataBuffer, gtSize_t dataSize);
    virtual bool read(gtByte* pDataBuffer, gtSize_t dataSize);
    virtual bool readAvailableData(gtByte* pDataBuffer, gtSize_t bufferSize, gtSize_t& amountOfDataRead);

private:
    bool allocateBuffer(gtByte*& pBuffer, gtSize_t& bufferSize, gtSize_t dataSize);

    // Do not allow the use of my default constructor:
    osChannelEncryptor();

private:
    // The wrapped channel who's communication we encrypt:
    osChannel& _wrappedChannel;

    // The blow fish encryptor:
    crBlowfishEncryptor _blowfishEncryptor;

    // The read buffer:
    gtByte* _pReadBuffer;
    gtSize_t _readBufferSize;

    // The write buffer:
    gtByte* _pWriteBuffer;
    gtSize_t _writeBufferSize;
};


#endif //__OSCHANNELENCRYPTOR_H
