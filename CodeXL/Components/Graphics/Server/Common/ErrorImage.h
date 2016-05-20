//==============================================================================
// Copyright (c) 2013-2016 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  Support for an error image that can be returned to a client when an image cannot be created
//==============================================================================

#ifndef GPS2_ERROR_IMAGE_H
#define GPS2_ERROR_IMAGE_H

#include "TSingleton.h"

//-----------------------------------------------------------------------------
/// Base class to handle different types of error images
//-----------------------------------------------------------------------------
class ErrorImageBase
{
public:

    //--------------------------------------------------------------------------
    /// Sends the current image data through the command response back to the client.
    /// \param pR The Command response
    //--------------------------------------------------------------------------
    void Send(CommandResponse* pR);

    //--------------------------------------------------------------------------
    /// Sends the current image data through the command response back to the client.
    /// \param pR The Command response
    //--------------------------------------------------------------------------
    void SendDDS(CommandResponse* pR);

protected:

    //--------------------------------------------------------------------------
    /// Constructor
    //--------------------------------------------------------------------------
    ErrorImageBase();

    //--------------------------------------------------------------------------
    /// Destructor
    //--------------------------------------------------------------------------
    ~ErrorImageBase();

    //--------------------------------------------------------------------------
    /// Loads the specified file into memory.
    /// \param pFilename filename of an image to load into memory
    //--------------------------------------------------------------------------
    void Load();

    gtASCIIString m_strFilename;  ///< name of the file that this error image was created with
private:

    char*         m_pImageData;   ///< pointer to the data for the error image
    unsigned int  m_nDataLength;  ///< the size (number of bytes) of the error image
    bool          m_bReady;       ///< indicates that the image is loaded and ready for use

};

//-----------------------------------------------------------------------------
/// Singleton class to handle Empty images.
/// This should be sent back if a requested texture is not available because the
/// application is not using it or nothing is bound to the requested slot.
//-----------------------------------------------------------------------------
class EmptyImage : public ErrorImageBase, public TSingleton< EmptyImage >
{
    /// TSingleton needs to be able to use our constructor.
    friend class TSingleton<EmptyImage>;

protected:

    /// Constructor
    EmptyImage();
};

//-----------------------------------------------------------------------------
/// Singleton class to handle error images.
/// This should be sent back if a requested texture is available but the server
/// encountered an error while trying to send it back to the client. This can
/// also be used for textures which are not currently supported.
//-----------------------------------------------------------------------------
class ErrorImage : public ErrorImageBase, public TSingleton< ErrorImage >
{
    /// TSingleton needs to be able to use our constructor.
    friend class TSingleton<ErrorImage>;

protected:

    /// Constructor
    ErrorImage();
};

#endif // GPS2_ERROR_IMAGE_H
