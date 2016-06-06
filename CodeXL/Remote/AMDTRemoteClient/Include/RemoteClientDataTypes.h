//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file RemoteClientDataTypes.h
///
//==================================================================================

#ifndef __RemoteClientDataTypes_h
#define __RemoteClientDataTypes_h
#include <AMDTBaseTools/Include/gtASCIIString.h>
#include <AMDTBaseTools/Include/gtString.h>

// This data type will be used to communicate errors which happened in the
// interaction between the remote client and the remote agent to the client front-end.
enum RemoteClientError
{
    rceNoError,
    rceUnknown,
    rceTargetAppNotFound,
    rceCommunicationFailure,
    rcePortUnavailable,
    rceTargetAppIsAlreadyRunning
};

struct FrameInfo
{
    virtual ~FrameInfo()
    {
        delete [] m_pImageBuffer;
        m_pImageBuffer = nullptr;
    }
    unsigned int   m_frameIndex = 0;
    double         m_elapsedTimeMS = 0.0;
    unsigned int   m_fps = 0;
    double         m_frameDuration = 0.0;
    unsigned int   m_apiCalls = 0;
    unsigned int   m_drawCalls = 0;
    unsigned int   m_framesCount = 1;

    unsigned char* m_pImageBuffer = nullptr;
    unsigned long  m_imageSize = 0;
    gtString       m_descriptionFileRemotePath;
    gtString       m_traceFileRemotePath;
    gtASCIIString  m_frameTrace;
    gtASCIIString  m_frameInfoXML;
    gtString       m_serverFolderPath;
};

const gtString FRAME_DESCRITPION_FILE_EXT = L"xml";
const gtString FRAME_DESCRITPION_FILE_PATTERN = L"description-";
const gtString FRAME_IMAGE_FILE_EXT = L"png";
const gtString FRAME_TRACE_FILE_EXT = L"ltr";

#endif // __RemoteClientDataTypes_h
