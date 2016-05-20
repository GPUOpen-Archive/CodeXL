//==============================================================================
// Copyright (c) 2013-2016 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  Support for an error image that can be returned to a client when an image cannot be created
//==============================================================================

#include "Logger.h"
#include "SharedGlobal.h"
#include "CommandProcessor.h"
#include "ErrorImage.h"

//--------------------------------------------------------------------------
/// Constructor
//--------------------------------------------------------------------------
ErrorImageBase::ErrorImageBase()
    : m_pImageData(nullptr),
    m_nDataLength(0),
    m_bReady(false)
{
}

//--------------------------------------------------------------------------
/// Destructor
//--------------------------------------------------------------------------
ErrorImageBase::~ErrorImageBase()
{
}

//--------------------------------------------------------------------------
/// Sends the current image data through the command response back to the client.
/// \param pR The Command response
//--------------------------------------------------------------------------
void ErrorImageBase::Send(CommandResponse* pR)
{
    PsAssert(pR != nullptr);

    // Check to see if we were initialized OK
    if (m_bReady == false)
    {
        // if not, try to load the image
        Load();

        // Check to see if we were initialized OK
        if (m_bReady == false)
        {
            // send an error back to the requestor (this will also be put in the log)
            pR->SendError("The %s was not initialized successfully\n", m_strFilename.asCharArray());
            return;
        }
    }

    // Send the image data
    pR->Send(m_pImageData, m_nDataLength);
}

//--------------------------------------------------------------------------
/// Sends the current image data through the command response back to the client.
/// \param pR The Command response
//--------------------------------------------------------------------------
void ErrorImageBase::SendDDS(CommandResponse* pR)
{
    PsAssert(pR != nullptr);
    // Send a single byte. Client will detect this and ignore it.
    pR->Send("0", 1);
}

//--------------------------------------------------------------------------
/// Loads the specified file into memory.
/// \param pFilename filename of an image to load into memory
//--------------------------------------------------------------------------
void ErrorImageBase::Load()
{
    // Get the server path
    const char* strServerPath;
    strServerPath = SG_GET_PATH(ServerPath);

    if (!strServerPath || strlen(strServerPath)  < 1)
    {
        Log(logERROR, "Cannot find the server executable directory name.\n");
        return;
    }

    // Use an gtASCIIString to easily do string stuff.
    gtASCIIString strTmp;
    strTmp = strServerPath;
    strTmp.append(m_strFilename);

    // Try to open the file we are looking for
    FILE* f = nullptr;
    fopen_s(&f, strTmp.asCharArray(), "rb");

    if (f != nullptr)
    {
        // Get the length of the data
        fseek(f, 0, SEEK_END);
        long length = ftell(f);
        m_nDataLength = (unsigned int)length;

        // reset position back to beginning
        fseek(f, 0, SEEK_SET);

        // Make room for it
        m_pImageData = (char*)(malloc(sizeof(char) * m_nDataLength));

        if (m_pImageData != nullptr)
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

//-----------------------------------------------------------------------------
/// Singleton class to handle Empty images.
/// This should be sent back if a requested texture is not available because the
/// application is not using it or nothing is bound to the requested slot.
//-----------------------------------------------------------------------------

/// Constructor sets up the desired image
EmptyImage::EmptyImage()
{
    m_strFilename = "Images/EmptyImage.png";
}

//-----------------------------------------------------------------------------
/// Singleton class to handle error images.
/// This should be sent back if a requested texture is available but the server
/// encountered an error while trying to send it back to the client. This can
/// also be used for textures which are not currently supported.
//-----------------------------------------------------------------------------

/// Constructor sets up the desired image
ErrorImage::ErrorImage()
{
    m_strFilename = "Images/ErrorImage.png";
}
