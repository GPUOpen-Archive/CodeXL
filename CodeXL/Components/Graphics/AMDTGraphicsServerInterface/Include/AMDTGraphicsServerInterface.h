//=============================================================
// (c) 2015 Advanced Micro Devices, Inc.
//
/// \author Thomas Chiu
/// \version $Revision: #2 $
/// \brief  The CodeXL component Handle communication to GPS server
//
//=============================================================
// $Id: //devtools/main/CodeXL/Components/Graphics/AMDTGraphicsServerInterface/Include/AMDTGraphicsServerInterface.h#2 $
// Last checkin:   $DateTime: 2015/07/11 13:27:17 $
// Last edited by: $Author: swsahoo $
// Change list:    $Change: 533728 $
//=============================================================

#ifndef _AMDT_GRAPHICS_SERVER_INTERFACE_H_
#define _AMDT_GRAPHICS_SERVER_INTERFACE_H_

// Foreward declarations:
class osPortAddress;

// Infra:
#include <AMDTBaseTools/Include/gtASCIIString.h>
#include <AMDTBaseTools/Include/gtVector.h>
#include <AMDTOSWrappers/Include/osHTTPClient.h>

// Local:
#include <AMDTGraphicsServerInterface/Include/GraphicsServerInterfaceDLLBuild.h>


// ----------------------------------------------------------------------------------
// Class Name:           GraphicsServerCommunication
// General Description:
//   Handle communication to GPS server
//
// Author:               Thomas Chiu
// Creation Date:        24/08/2015
// ----------------------------------------------------------------------------------
class GSI_API GraphicsServerCommunication
{
public:
    // This enum mimics the GRAPHICS_SERVER_STATE enum in GraphicsServerState.h
    enum TargetAppState
    {
        APP_ACTIVE = 0,
        APP_NOT_RENDERING = 1,
        APP_NOT_RUNNING = 2
    };

    GraphicsServerCommunication(const gtASCIIString& ServerURL, unsigned short iPort);
    virtual ~GraphicsServerCommunication();

    bool QueryServers(gtASCIIString& strServers);

    // connections
    bool ConnectServer(const gtASCIIString strServer);
    bool ConnectProcess(const gtASCIIString strPid, const gtASCIIString& apiType);
    bool Disconnect();

    // commands
    bool PushLogger(gtASCIIString& strWebResponse);
    bool PushObjectProcessor(gtASCIIString& strWebResponse);
    bool GetLinkedTrace(gtASCIIString& returnedPage);
    bool GetObjectTree(gtASCIIString& strObjectTreeXML);
    bool GetObjectDatabase(gtASCIIString& strObjectDatabaseXML);
    bool RunProfile(gtASCIIString& returnedPage);
    bool GetTimingLog(gtASCIIString& returnedPage);
    bool GetProcesses(gtASCIIString& strWebResponse);
    bool PopLayer();
    bool ShutDown();

    bool GetCounters(gtASCIIString& returnedPage);
    bool SetCounters(const gtVector<int>& selectedCounters, gtASCIIString& returnedPage);

    // frame related commands
    bool GetCurrentFrame(unsigned char*& pImageBuffer, unsigned long& imageSize);
    bool GetFrameThumbnail(unsigned char*& pImageBuffer, unsigned long& imageSize, int frameIndex);
    bool GetNumCapturedFrames(gtASCIIString& executable, int& numFrames);

    /// Get the current frame info. This function is a heartbeat function, that retrieves the current frame info and thumbnail from the server
    /// \param frameInfoAsXML[out] frameInfoAsXML the frame info XML
    /// \param pImageBuffer[out] the image thumbnail buffer
    /// \param imageSize[out] the size of the image thumbnail
    /// \return true if the data was received successfully
    bool GetCurrentFrameInfo(gtASCIIString& frameInfoAsXML, unsigned char*& pImageBuffer, unsigned long& imageSize);

    /// Send a capture request to the server, and get the current frame info and thumbnail
    /// \param numberFramesToCapture number of frames to capture
    /// \param frameInfoXML[out] the captured frame info as XML
    /// \return true for success
    bool CaptureFrame(int numberFramesToCapture, gtASCIIString& frameInfoAsXML);

    /// Stub function until the get captured frame functionality is resolved
    bool CaptureFrameStub(gtASCIIString& frameInfoAsXML, unsigned char*& pImageBuffer, unsigned long& imageSize);

    ///  Set the current session name
    /// \param sessionName the session name
    /// \return true for success
    bool SetSessionName(const gtASCIIString& sessionName);

    ///  Set the current project name
    /// \param projectName the project name
    /// \return true for success
    bool SetProjectName(const gtASCIIString& projectName);

    void SetStopSignal() {m_isStopSignaled = true;}

    void sleepBetweenResendAttempts();

private:
    // Do not allow the use of my default constructor:
    GraphicsServerCommunication();

    /// Send a command PID to the server, and receive the server response as string
    bool SendCommandPid(const gtASCIIString& strCommand, gtASCIIString& strWebResponse, const gtASCIIString& strPid, bool isResendAllowed = true);

    /// Request for a string data from server
    bool RequestData(const gtASCIIString& strRequest, gtASCIIString& strRtnPage, bool isResendAllowed = true);

    /// Send a command PID to the server, and receive the server response as binary data
    /// \param strCommand the command as string
    /// \param pReturnDataBuffer[out] the server response as binary buffer
    /// \param dataBufferSize[out] the size of the output buffer
    /// \return true for success
    bool SendCommandWithBinaryData(const gtASCIIString& strCommand, unsigned char*& pReturnDataBuffer, unsigned long& dataBufferSize);

    /// Request for a binary data from server
    /// \param strRequest the command as string
    /// \param pReturnDataBuffer[out] the server response as binary buffer
    /// \param dataBufferSize[out] the size of the output buffer
    /// \return true for success
    bool RequestData(const gtASCIIString& strRequest, unsigned char*& pReturnData, unsigned long& dataBufferSize);

    // Temp function from the stub functions
    bool LoadImageFromDisk(gtString& imageName, unsigned char*& pImageBuffer, unsigned long& imageSize);

    bool SendCachedMessages(gtASCIIString& lastMessageReply, TargetAppState& appState);
    void CheckServerStatus(const gtASCIIString& httpRequestResult, TargetAppState& appState);
    void CheckServerStatus(unsigned char* pBuffer, unsigned long bufferSize, TargetAppState& appState);

private:
    enum { SLEEP_INTERVAL_BETWEEN_CONSECUTIVE_RESEND_ATTEMPTS_IN_MILLISECS = 500};
    osPortAddress   m_GPSServer;
    osHTTPClient    m_httpClient;

    // The server URL:
    gtASCIIString   m_serverURL;

    // Port for connection, default to 80
    unsigned short  m_Port = 80;

    // current process ID
    gtASCIIString   m_strPid;
    
    /// current API http command for example : /DX12 or /Vulkan
    gtASCIIString   m_strApiHttpCommand;

    bool m_isStopSignaled;
};

#endif //__GraphicsServerCommunication_H
