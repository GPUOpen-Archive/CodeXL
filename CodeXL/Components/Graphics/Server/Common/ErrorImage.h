//==============================================================================
// Copyright (c) 2013-2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  Support for an error image that can be returned to a client when an image cannot be created
//==============================================================================

#ifndef GPS2_ERROR_IMAGE_H
#define GPS2_ERROR_IMAGE_H

#include "Logger.h"
#include "SharedGlobal.h"
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
    void Send(CommandResponse* pR)
    {
        PsAssert(pR != NULL);

        // Check to see if we were initialized OK
        if (m_bReady == false)
        {
            // send an error back to the requestor (this will also be put in the log)
            pR->SendError("The %s was not initialized successfully\n", m_strFilename.asCharArray());
            return;
        }

        // Send the image data
        pR->Send(m_pImageData, m_nDataLength);
    }

    //--------------------------------------------------------------------------
    /// Sends the current image data through the command response back to the client.
    /// \param pR The Command response
    //--------------------------------------------------------------------------
    void SendDDS(CommandResponse* pR)
    {
        PsAssert(pR != NULL);
        // Send a single byte. Client will detect this and ignore it.
        pR->Send("0", 1);
    }

protected:

    //--------------------------------------------------------------------------
    /// Constructor
    //--------------------------------------------------------------------------
    ErrorImageBase()
        :  m_pImageData(NULL),
           m_nDataLength(0),
           m_bReady(false)
    {

    }

    //--------------------------------------------------------------------------
    /// Loads the specified file into memory.
    /// \param pFilename filename of an image to load into memory
    //--------------------------------------------------------------------------
    void Load(const char* pFilename)
    {
        m_strFilename = pFilename;

        // Get the server path
        const char* strServerPath;
        strServerPath = SG_GET_PATH(ServerPath) ;

        if (!strServerPath || strlen(strServerPath)  < 1)
        {
            Log(logERROR, "Cannot find the server executable directory name.\n");
            return;
        }

        // Use an gtASCIIString to easily do string stuff.
        gtASCIIString strTmp;
        strTmp = strServerPath;
        strTmp.append(pFilename);

        // Try to open the file we are looking for
        FILE* f = NULL;
        fopen_s(&f, strTmp.asCharArray(), "rb");

        if (f != NULL)
        {
            // Get the length of the data
            fseek(f, 0, SEEK_END);
            long length = ftell(f);
            m_nDataLength = (unsigned int) length;

            // reset position back to beginning
            fseek(f, 0, SEEK_SET);

            // Make room for it
            m_pImageData = (char*)(malloc(sizeof(char) * m_nDataLength));

            if (m_pImageData != NULL)
            {
                // Read the data
                size_t dataRead = fread(m_pImageData, sizeof(char), m_nDataLength, f);

                if (dataRead != m_nDataLength)
                {
                    Log(logWARNING, "Insufficient data read from image; image may be corrupted.\n");
                }
            }

            // clean up
            fclose(f);
        }
        else
        {
            Log(logERROR, "Cannot find the %s file at %s\n", m_strFilename.asCharArray(), strTmp.asCharArray());
            return;
        }

        // Mark ourselves as ready.
        m_bReady = true;
    }

private:

    gtASCIIString m_strFilename;  ///< name of the file that this error image was created with
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

    /// Constructor loads the desired image
    EmptyImage()
    {
        Load("Images/EmptyImage.png");
    }
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

    /// Constructor loads the desired image
    ErrorImage()
    {
        Load("Images/ErrorImage.png");
    }
};

#endif // GPS2_ERROR_IMAGE_H
