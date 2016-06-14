//------------------------------ AMDTGraphicsServerInterface.cpp ------------------------------

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTOSWrappers/Include/osFilePath.h>
#include <AMDTOSWrappers/Include/osThread.h>

// Local:
#include <AMDTGraphicsServerInterface/Include/AMDTGraphicsServerInterface.h>
<<<<<<< c380a1742d0b0b4df6a10c73dbe9028771ba731f
#include <Server/Common/Tracing/CaptureTypes.h>
=======
>>>>>>> Cpu\Gpu only without UI

// CodeXL uses INFINITE timeout when waiting for graphics server replies because the web server responsiveness
// may be delayed when games go through their heavy load initialization period.
// There is no point in setting a short timeout because as long as the web server is alive we want to get any response it generates
#define GRAPHIC_SERVER_READ_TIMEOUT OS_CHANNEL_INFINIT_TIME_OUT

#define GP_GRAPHICS_SERVER_STATE_STALLED "GRAPHICS_SERVER_STATE_STALLED"
#define GP_GRAPHICS_SERVER_STATE_PROCESS_NOT_RUNNING "GRAPHICS_SERVER_STATE_PROCESS_NOT_RUNNING"

#define GP_GRAPHICS_SERVER_DX12_API_TYPE  "DX12"
#define GP_GRAPHICS_SERVER_VULKAN_API_TYPE  "Vulkan"


/// Convert a CaptureType value to a string
/// \param eType The input type to convert
/// \return A string version of the capture type's value.
gtASCIIString CaptureTypeString(CaptureType eType)
{
    switch (eType)
    {
    case CaptureType_APITrace:
        return gtASCIIString("1");
    case CaptureType_GPUTrace:
        return gtASCIIString("2");
    case CaptureType_LinkedTrace:
        return gtASCIIString("3");
    case CaptureType_FullFrameCapture:
        return gtASCIIString("4");
    default:
        return gtASCIIString("3");
    }
}
// ---------------------------------------------------------------------------
// Name:        GraphicsServerCommunication::GraphicsServerCommunication
// Description: Constructor
// Arguments:   ServerURL - The  server URL (Example: "127.0.0.1")
//              iPort - Communication port
// ---------------------------------------------------------------------------
GraphicsServerCommunication::GraphicsServerCommunication(const gtASCIIString& ServerURL, unsigned short iPort)
    : m_serverURL(ServerURL), m_Port(iPort)
{
}

// ---------------------------------------------------------------------------
// Name:        GraphicsServerCommunication::~GraphicsServerCommunication
// Description: Destructor
// ---------------------------------------------------------------------------
GraphicsServerCommunication::~GraphicsServerCommunication()
{

}

// ---------------------------------------------------------------------------
// Name:        GraphicsServerCommunication::QueryServers
// Description: TO BE IMPLEMENTED - locate available Graphics servers
// Arguments:   strServers(return) - List of available servers
// Return Val:  bool  - Success / failure
// ---------------------------------------------------------------------------
bool GraphicsServerCommunication::QueryServers(gtASCIIString& strServers)
{
    GT_UNREFERENCED_PARAMETER(strServers);

    bool retVal = false;

    // TO BE IMPLEMENTED, not sure about usage scenario

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        GraphicsServerCommunication::ConnectServer
// Description: Connect to server with specified Address or existing address
// Arguments:   strServer - optional address to override and update existing information
// Return Val:  bool  - Success / failure
// ---------------------------------------------------------------------------
bool GraphicsServerCommunication::ConnectServer(const gtASCIIString strServer)
{
    bool retVal = false;
    m_isStopSignaled = false;

    if (0 < strServer.length())
    {
        m_serverURL = strServer;
    }

    // Set the server address and port
    m_GPSServer = osPortAddress(m_serverURL, m_Port);

    m_httpClient = osHTTPClient(m_GPSServer);
    m_httpClient.GetTCPSocket().setReadOperationTimeOut(GRAPHIC_SERVER_READ_TIMEOUT);

    if (true == m_httpClient.connect())
    {
        retVal = true;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        GraphicsServerCommunication::ConnectProcess
// Description: Connect to Graphics server's specific process
// Arguments:   strPid - Process ID to connect to and update current PID, (optional) can be "", function will then connect to known process
// Return Val:  bool  - Success / failure
// ---------------------------------------------------------------------------
bool GraphicsServerCommunication::ConnectProcess(const gtASCIIString strPid, const gtASCIIString& apiType)
{
    gtASCIIString strWebResponse;

    bool retVal = false;

    if ((0 < strPid.length()) && (true == strPid.isIntegerNumber()))
    {
        m_strPid = strPid;
    }

    if (apiType.isEmpty() == false)
    {
        if (apiType == GP_GRAPHICS_SERVER_DX12_API_TYPE || apiType == GP_GRAPHICS_SERVER_VULKAN_API_TYPE)
        {
            m_strApiHttpCommand = "/";
            m_strApiHttpCommand.append(apiType);
        }
        else
        {
            gtString msg = L"Wrong API given : ";
            msg.append(gtString().fromASCIIString(apiType.asCharArray()));
            GT_ASSERT_EX(false, msg.asCharArray());
            m_strApiHttpCommand = "";
        }
    }
    if (m_strPid.isEmpty() == false && m_strApiHttpCommand.isEmpty() == false)
    {
        // Connect
        gtASCIIString showStack = m_strApiHttpCommand;
        retVal = SendCommandPid(showStack.append("/ShowStack"), strWebResponse, "");

        if (retVal)
        {
            gtASCIIString timeControl = m_strApiHttpCommand;
            retVal = SendCommandPid(timeControl.append("/PushLayer=TimeControl") , strWebResponse, "");
        }

        if (retVal)
        {
            gtASCIIString tcSettings = m_strApiHttpCommand;
            retVal = SendCommandPid(tcSettings.append("/TC/Settings.xml"), strWebResponse, "");
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        GraphicsServerCommunication::Disconnect()
// Description: Disconnect web client
// Return Val:  bool  - Success / failure
// ---------------------------------------------------------------------------
bool GraphicsServerCommunication::Disconnect()
{
    bool retVal = false;

    // object handle connection checks internally
    retVal = m_httpClient.disconnect();

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        GraphicsServerCommunication::sleepBetweenResendAttempts()
// Description: Sleep between attempts to resend a message to the graphics server
// Return Val:  void
// ---------------------------------------------------------------------------
void GraphicsServerCommunication::sleepBetweenResendAttempts()
{
    osSleep(SLEEP_INTERVAL_BETWEEN_CONSECUTIVE_RESEND_ATTEMPTS_IN_MILLISECS);
}

// ---------------------------------------------------------------------------
// Name:        GraphicsServerCommunication::RequestData
// Description: Sends the HTTP request to the server, obtain returned page
// Arguments:   requestString - The URL request string.
//              returnedPage - The result page returned from the system.
// Return Val:  bool  - Success / failure
// ---------------------------------------------------------------------------
bool GraphicsServerCommunication::RequestData(const gtASCIIString& requestString, gtASCIIString& returnedPage, bool isResendAllowed)
{
    bool retVal = false;
    TargetAppState appState = APP_ACTIVE;

    if (true == m_httpClient.connect())
    {
        do
        {
            gtASCIIString httpReply;
            bool rc = m_httpClient.requestPage(requestString, returnedPage, false, m_serverURL.asCharArray(), true);

            if (rc)
            {
                CheckServerStatus(returnedPage, appState);

                if (APP_ACTIVE == appState)
                {
                    // The message was successfully handled by the server
                    retVal = true;
                    break;
                }
                else if (APP_NOT_RENDERING == appState)
                {
                    // The message was rejected by the server
                    if (false == m_isStopSignaled)
                    {
                        sleepBetweenResendAttempts();
                    }
                }
                else if (APP_NOT_RUNNING == appState)
                {
                    // The target application is no longer running
                    break;
                }
            }
            else
            {
                gtString errorCode;
                errorCode.fromASCIIString(m_httpClient.getLastErrorCode().asCharArray());
                GT_ASSERT_EX(rc, errorCode.asCharArray());
                break;
            }
        }
        while (isResendAllowed && false == m_isStopSignaled);
    }

    return retVal;
}

bool GraphicsServerCommunication::RequestData(const gtASCIIString& strRequest, unsigned char*& pReturnData, unsigned long& dataBufferSize)
{
    bool retVal = false;
    TargetAppState appState = APP_ACTIVE;

    if (true == m_httpClient.connect())
    {
        do
        {
            gtASCIIString httpReply;
            // Execute the HTTP submission:
            pReturnData = nullptr;
            dataBufferSize = 0;
            bool rc = m_httpClient.RequestPageWithBinaryData(strRequest, pReturnData, dataBufferSize, false, m_serverURL.asCharArray(), true);

            if (rc)
            {
                CheckServerStatus(pReturnData, dataBufferSize, appState);

                if (APP_ACTIVE == appState)
                {
                    // The message was successfully handled by the server
                    retVal = true;
                    break;
                }
                else if (APP_NOT_RENDERING == appState)
                {
                    // The message was rejected by the server
                    if (false == m_isStopSignaled)
                    {
                        sleepBetweenResendAttempts();
                    }

                }
                else if (APP_NOT_RUNNING == appState)
                {
                    // The target application is no longer running
                    break;
                }
            }
            else
            {
                // communication failure
                dataBufferSize = 0;
                delete[] pReturnData;

                gtString errorCode;
                errorCode.fromASCIIString(m_httpClient.getLastErrorCode().asCharArray());
                GT_ASSERT_EX(rc, errorCode.asCharArray());
                break;
            }
        }
        while (false == m_isStopSignaled);
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        GraphicsServerCommunication::SendCommandPid
// Description: locate available Graphics server
// Arguments:   strWebResponse(return) - web string returned.
//              strCommand - URL command to send to server
//              strPid - optional Process ID string, leave empty, "", to use existing Process ID
// Return Val:  bool  - Success / failure
// ---------------------------------------------------------------------------
bool GraphicsServerCommunication::SendCommandPid(const gtASCIIString& strCommand, gtASCIIString& strWebResponse, const gtASCIIString& strPid, bool isResendAllowed)
{
    bool retVal = false;
    gtASCIIString strConnectPid;
    gtASCIIString strQueryUrl;

    if ((0 < strPid.length()) && (true == strPid.isIntegerNumber()))
    {
        strConnectPid = strPid;
    }
    else if ((0 < m_strPid.length()) && (true == m_strPid.isIntegerNumber()))
    {
        strConnectPid = m_strPid;
    }

    strQueryUrl = "/";
    strQueryUrl.append(strConnectPid);
    strQueryUrl.append(strCommand);

    retVal = RequestData(strQueryUrl, strWebResponse, isResendAllowed);

    return retVal;
}

bool GraphicsServerCommunication::SendCommandWithBinaryData(const gtASCIIString& strCommand, unsigned char*& pReturnDataBuffer, unsigned long& dataBufferSize)
{
    bool retVal = false;

    gtASCIIString strConnectPid;
    gtASCIIString strQueryUrl;

    if ((0 < m_strPid.length()) && (true == m_strPid.isIntegerNumber()))
    {
        strConnectPid = m_strPid;
    }

    strQueryUrl = "/";
    strQueryUrl.append(strConnectPid);
    strQueryUrl.append(strCommand);


    retVal = RequestData(strQueryUrl, pReturnDataBuffer, dataBufferSize);

    return retVal;
}

bool GraphicsServerCommunication::GetProcesses(gtASCIIString& strWebResponse)
{
    bool retVal = false;
    OS_DEBUG_LOG_TRACER_WITH_RETVAL(retVal);

    // The ConnectToAPI dialog sends this message periodically directly from the main UI thread.
    // To keep the UI responsive we set the isResendAllowed flag to false to avoid getting stuck in a
    // resend loop
    bool isResendAllowed = false;
    retVal = RequestData("/Process.xml", strWebResponse, isResendAllowed);

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        GraphicsServerCommunication::PushLogger
// Description: Enable logger layer on Graphics server, allow subsequent log retrieval, i.e. LinkedTrace
// Arguments:   strWebResponse(return) - web response from server
// Return Val:  bool  - Success / failure
// ---------------------------------------------------------------------------
bool GraphicsServerCommunication::PushLogger(gtASCIIString& strWebResponse)
{
    bool retVal = false;
    gtASCIIString command = m_strApiHttpCommand;
    retVal = SendCommandPid(command.append("/PushLayer=Logger"), strWebResponse, "");

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        GraphicsServerCommunication::PushObjectProcessor
// Description: Enable logger layer on Graphics server, allow subsequent log retrieval, i.e. LinkedTrace
// Arguments:   strWebResponse(return) - web response from server
// Return Val:  bool  - Success / failure
// ---------------------------------------------------------------------------
bool GraphicsServerCommunication::PushObjectProcessor(gtASCIIString& strWebResponse)
{
    bool retVal = false;
    gtASCIIString command = m_strApiHttpCommand;
    retVal = SendCommandPid(command.append("/PushLayer=ObjectDatabase"), strWebResponse, "");

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        GraphicsServerCommunication::GetLinkedTrace
// Description: Retrieve captured Linked Trace from Graphics server
// Arguments:   strLinkedTrace(return) - collection of linked trace from current 3D application
// Return Val:  bool  - Success / failure
// ---------------------------------------------------------------------------
bool GraphicsServerCommunication::GetLinkedTrace(gtASCIIString& strLinkedTrace)
{
    bool retVal = false;
    gtASCIIString httpRtnString;
    retVal = PushLogger(httpRtnString);

    if (true == retVal)
    {
        // linked trace, always
        gtASCIIString linkedTrace = m_strApiHttpCommand;
        retVal = SendCommandPid(linkedTrace.append("/LOG/LinkedTrace.txt"), strLinkedTrace, "");

        if (retVal)
        {
            PopLayer();
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        GraphicsServerCommunication::GetObjectTree
// Description: Retrieve captured Objects from Graphics server
// Arguments:   strObjects(return) - collection of Objects from current 3D application
// Return Val:  bool  - Success / failure
// ---------------------------------------------------------------------------
bool GraphicsServerCommunication::GetObjectTree(gtASCIIString& strObjectTree)
{
    bool retVal = false;
    OS_DEBUG_LOG_TRACER_WITH_RETVAL(retVal);
    gtASCIIString httpRtnString;

    // Object tree
    gtASCIIString command = m_strApiHttpCommand;
    retVal = SendCommandPid(command.append("/PushLayer=ObjectDatabase"), httpRtnString, "");

    if (true == retVal)
    {
        command = m_strApiHttpCommand;
        retVal = SendCommandPid(command.append("/DB/ObjectTree.xml"), httpRtnString, "");

        if (true == retVal)
        {
            strObjectTree = httpRtnString;
            PopLayer();
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        GraphicsServerCommunication::GetObjectDatabase
// Description: Retrieve captured object database from Graphics server
// Arguments:   strFullObjDBase(return) - Object database from current 3D application
// Return Val:  bool  - Success / failure
// ---------------------------------------------------------------------------
bool GraphicsServerCommunication::GetObjectDatabase(gtASCIIString& strFullObjDBase)
{
    bool retVal = false;
    OS_DEBUG_LOG_TRACER_WITH_RETVAL(retVal);
    gtASCIIString httpRtnString;

    // Get database
    gtASCIIString objDBPushlayer = m_strApiHttpCommand;
    retVal = SendCommandPid(objDBPushlayer.append("/PushLayer=ObjectDatabase"), httpRtnString, "");

    if (true == retVal)
    {
        gtASCIIString dbObjDBPushlayer = m_strApiHttpCommand;
        retVal = SendCommandPid(dbObjDBPushlayer.append("/DB/ObjectDatabase.xml"), httpRtnString, "");

        if (true == retVal)
        {
            // assign object string
            strFullObjDBase = httpRtnString;
            PopLayer();
        }
    }

    return retVal;
}

bool GraphicsServerCommunication::RunProfile(gtASCIIString& strLinkedTrace)
{
    bool retVal = false;
    gtASCIIString httpRtnString;

    // linked trace, always
    retVal = SendCommandPid("/FP/Profiler.xml", httpRtnString, "");

    if (true == retVal)
    {
        // assign only the linked trace string
        strLinkedTrace = httpRtnString;
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        GraphicsServerCommunication::GetTimingLog
// Description: Retrieve DX11 Timing Log
// Arguments:   strTimingLog(return) - List of available servers, sample string "345, 8923, 7737, ", consider changing to array of strings.
// Return Val:  bool  - Success / failure
// ---------------------------------------------------------------------------
bool GraphicsServerCommunication::GetTimingLog(gtASCIIString& strTimingLog)
{
    bool retVal = false;
    OS_DEBUG_LOG_TRACER_WITH_RETVAL(retVal);
    gtASCIIString httpRtnString;

    //TODO use DX11 as parameter
    retVal = SendCommandPid("/DX11/LOG/TimingLog.txt", httpRtnString, "");

    if (true == retVal)
    {
        // assign the Timing Log string
        strTimingLog = httpRtnString;
    }

    return retVal;
}

bool GraphicsServerCommunication::PopLayer()
{
    bool retVal = false;
    gtASCIIString httpRtnString;

    gtASCIIString poplayer = m_strApiHttpCommand;
    retVal = SendCommandPid(poplayer.append("/PopLayer"), httpRtnString, "");

    return retVal;
}

bool GraphicsServerCommunication::GetCounters(gtASCIIString& returnedPage)
{
    bool retVal = false;
    OS_DEBUG_LOG_TRACER_WITH_RETVAL(retVal);
    gtASCIIString httpRtnString;

    gtASCIIString commandPrefix = m_strApiHttpCommand;
    retVal = SendCommandPid(commandPrefix.append("/PushLayer=FrameProfiler"), httpRtnString, "");

    if (retVal)
    {
	    commandPrefix = m_strApiHttpCommand;
        retVal = SendCommandPid(commandPrefix.append("/FP/CounterInfo.xml"), httpRtnString, "");

        if (retVal)
        {
            returnedPage = httpRtnString;
            PopLayer();
        }
    }

    return retVal;
}

bool GraphicsServerCommunication::SetCounters(const gtVector<int>& selectedCounters, gtASCIIString& returnedPage)
{
    bool retVal = false;
    gtASCIIString httpRtnString;

    gtASCIIString commandPrefix = m_strApiHttpCommand;
    retVal = SendCommandPid(commandPrefix.append("/PushLayer=FrameProfiler"), httpRtnString, "");

    if (retVal)
    {
	    commandPrefix = m_strApiHttpCommand;
        gtASCIIString sendCommand(commandPrefix.append("/FP/CounterSelect.txt="));

        int numCounters = (int)selectedCounters.size();

        for (int nCounter = 0; nCounter < numCounters; nCounter++)
        {
            sendCommand.appendFormattedString("%d", selectedCounters[nCounter]);

            if (nCounter != numCounters - 1)
            {
                sendCommand.append(",");
            }
        }

        retVal = SendCommandPid(sendCommand.asCharArray(), httpRtnString, "");

        if (retVal)
        {
            returnedPage = httpRtnString;
            PopLayer();
        }
    }

    return retVal;
}

bool GraphicsServerCommunication::ShutDown()
{
    bool retVal = false;
    m_isStopSignaled = true;

    gtASCIIString httpRtnString;

    retVal = RequestData("/ShutDown", httpRtnString);

    return retVal;
}

bool GraphicsServerCommunication::GetCurrentFrame(unsigned char*& pImageBuffer, unsigned long& imageSize)
{
    bool retVal = false;
    OS_DEBUG_LOG_TRACER_WITH_RETVAL(retVal);

    // stub implementation that cycle through 12 images
    static int currentImage = 1;
    gtString imageName;
    imageName.appendFormattedString(L"cxl_stub_image%d", currentImage);

    retVal = LoadImageFromDisk(imageName, pImageBuffer, imageSize);

    currentImage = (currentImage + 1) % 12 + 1;

    return retVal;
}

bool GraphicsServerCommunication::GetFrameThumbnail(unsigned char*& pImageBuffer, unsigned long& imageSize, int frameIndex)
{
    bool retVal = false;
    OS_DEBUG_LOG_TRACER_WITH_RETVAL(retVal);

    // stub implementation that cycle through 12 images
    int currentImage = frameIndex % 12 + 1;
    gtString imageName;
    imageName.appendFormattedString(L"cxl_stub_image%d", currentImage);

    retVal = LoadImageFromDisk(imageName, pImageBuffer, imageSize);

    currentImage = (currentImage + 1) % 12 + 1;

    return retVal;
}

bool GraphicsServerCommunication::GetNumCapturedFrames(gtASCIIString& executable, int& numFrames)
{
    bool retVal = true;
    OS_DEBUG_LOG_TRACER_WITH_RETVAL(retVal);

    numFrames = 3;

    if (executable.length() > 6)
    {
        numFrames = 4;
    }

    return retVal;
}

bool GraphicsServerCommunication::GetCurrentFrameInfo(gtASCIIString& frameInfoAsXML, unsigned char*& pImageBuffer, unsigned long& imageSize)
{
    bool retVal = true;
    OS_DEBUG_LOG_TRACER_WITH_RETVAL(retVal);

    gtASCIIString httpRtnString;
    gtASCIIString commandPrefix = m_strApiHttpCommand;
    retVal = SendCommandPid(commandPrefix.append("/PushLayer=FrameDebugger"), httpRtnString, "");
    GT_ASSERT(retVal);

    if (retVal)
    {
        commandPrefix = m_strApiHttpCommand;
        // Send a request to get the current frame info
        retVal = SendCommandPid(commandPrefix.append("/FD/GetCurrentFrameInfo.xml"), frameInfoAsXML, "");
        GT_ASSERT(retVal);

        if (retVal)
        {
            commandPrefix = m_strApiHttpCommand;
            // Send a request to get the current frame thumbnail
            retVal = SendCommandWithBinaryData(commandPrefix.append("/FD/GetFrameBufferImage.png?width=512?height=512"), pImageBuffer, imageSize);

            if (retVal)
            {
                PopLayer();
            }
        }
    }

    return retVal;
}

// CodeXL's capture frame function
// ---------------------------------------------------------------------------
// Name:        GraphicsServerCommunication::CaptureFrame
// Description: Capture all available frame information to disk, trace and objects
// Arguments:   numberFramesToCapture number of frames to capture
//              frameInfoAsXML(return) - path that contains captured metadata file
// Return Val:  bool  - Success / failure
// ---------------------------------------------------------------------------
bool GraphicsServerCommunication::CaptureFrame(int numberFramesToCapture, gtASCIIString& frameInfoAsXML, CaptureType captuteTypeVal)
{
    bool retVal = false;
    OS_DEBUG_LOG_TRACER_WITH_RETVAL(retVal);

    gtASCIIString httpRtnString;
    retVal = PushLogger(httpRtnString);

    if (retVal == true)
    {
        // Capture the frame, =3 is includes the save XML frame data, need object database active to include the object database.
        gtASCIIString commandToSend = m_strApiHttpCommand;
        commandToSend.append("/FrameCaptureWithSave?CaptureType=");

        // Use CaptureType to define what type of capture to do
        //commandToSend.append(CaptureTypeString(CaptureType_APITrace));
        //commandToSend.append(CaptureTypeString(CaptureType_GPUTrace));
        //commandToSend.append(CaptureTypeString(CaptureType_LinkedTrace));
        commandToSend.append(CaptureTypeString(captuteTypeVal));

        // Add the number of frames to capture
        commandToSend.appendFormattedString("&CaptureCount=%d", numberFramesToCapture);
        retVal = SendCommandPid(commandToSend, frameInfoAsXML, "");

        // Pop the logger layer
        PopLayer();
    }

    return retVal;
}

bool GraphicsServerCommunication::CaptureFrameStub(gtASCIIString& frameInfoAsXML, unsigned char*& pImageBuffer, unsigned long& imageSize)
{
    bool retVal = true;
#pragma message ("TODO: FA: remove this function")

    static gtASCIIString serverResponseFormat = "<Root><Location></Location>"\
                                                "<FrameNumber>%d</FrameNumber><Contents>"\
                                                "<LinkedTrace>linkedtrace_20150915_091403.ltr</LinkedTrace>"\
                                                "<FrameBufferImage></FrameBufferImage>"\
                                                "<ElapsedTime>%d</ElapsedTime>"\
                                                "<FPS>%d</FPS>"\
                                                "<CPUFrameDuration>%f</CPUFrameDuration>"\
                                                "<APICallCount>%d</APICallCount>"\
                                                "<DrawCallCount>%d</DrawCallCount>"\
                                                "</Contents></Root>";

    static int frameIndex = 1345;
    frameIndex += rand() % 50;
    int elapsedTimeMS = rand() % 1000000;
    int fps = rand() % 250;
    double frameDuration = (double)rand() / 2.345;
    int apiCalls = rand() % 500000;
    int drawCalls = rand() % apiCalls;

    frameInfoAsXML.appendFormattedString(serverResponseFormat.asCharArray(), frameIndex, elapsedTimeMS, fps, frameDuration, apiCalls, drawCalls);

    GetFrameThumbnail(pImageBuffer, imageSize, frameIndex % 12);
    return retVal;
}

bool GraphicsServerCommunication::LoadImageFromDisk(gtString& imageName, unsigned char*& pImageBuffer, unsigned long& imageSize)
{
    bool retVal = false;

    pImageBuffer = nullptr;
    osFilePath imageFilePath(osFilePath::OS_USER_DOCUMENTS);

    imageFilePath.setFileName(imageName);
    imageFilePath.setFileExtension(L"png");

    // load the image and return it in the buffer:
    if (imageFilePath.exists())
    {
        osFile imageFile(imageFilePath);
        bool rc = imageFile.getSize(imageSize);
        GT_IF_WITH_ASSERT(rc)
        {
            rc = imageFile.open(osChannel::OS_BINARY_CHANNEL);
            GT_IF_WITH_ASSERT(rc)
            {
                pImageBuffer = new unsigned char[imageSize];
                rc = imageFile.read((char*)pImageBuffer, imageSize);
                GT_IF_WITH_ASSERT(rc)
                {
                    retVal = true;
                }
                imageFile.close();
            }
        }
    }
    else
    {
        GT_ASSERT_EX(false, L"Place images cxl_stub_image1-cxl_stub_image12 in user user documents. You can get it from devtools\\main\\CodeXL\\Documents\\Graphics\\StubImages\\");
    }

    return retVal;
}

void GraphicsServerCommunication::CheckServerStatus(const gtASCIIString& httpRequestResult, TargetAppState& appState)
{
    if (httpRequestResult.find(GP_GRAPHICS_SERVER_STATE_PROCESS_NOT_RUNNING) >= 0)
    {
        appState = APP_NOT_RUNNING;
    }
    else if (httpRequestResult.find(GP_GRAPHICS_SERVER_STATE_STALLED) >= 0)
    {
        appState = APP_NOT_RENDERING;
    }
    else
    {
        appState = APP_ACTIVE;
    }
}

void GraphicsServerCommunication::CheckServerStatus(unsigned char* pBuffer, unsigned long bufferSize, TargetAppState& appState)
{
    GT_ASSERT(bufferSize > 0);

    if (bufferSize > 1)
    {
        appState = APP_ACTIVE;
    }

    if (1 == bufferSize && pBuffer != nullptr)
    {
        int numericValue = pBuffer[0];
        appState = static_cast<TargetAppState>(numericValue);
    }
}

/// Replace any special HTTP characters
/// \param ioCommandText the command string to modify. This string is fixed up in place.
void GraphicsServerCommunication::ReplaceSpecialHTTPCharacters(gtASCIIString& ioCommandText)
{
    ioCommandText.replace(" ", "%20");
    ioCommandText.replace("\"", "%22");
    ioCommandText.replace("\\", "%5C");
    ioCommandText.replace("-", "%E2%80%93");
    ioCommandText.replace("&", "%26");
    ioCommandText.replace("'", "%27");
    ioCommandText.replace("`", "%60");
    ioCommandText.replace("`", "%E2%80%98");
}

bool GraphicsServerCommunication::SetSessionName(const gtASCIIString& sessionName)
{
    bool retVal = false;
    // Connect
    gtASCIIString commandText = m_strApiHttpCommand;
    gtASCIIString strWebResponse;
    commandText.appendFormattedString("/SetSessionName.txt=%s", sessionName.asCharArray());
    ReplaceSpecialHTTPCharacters(commandText);
    retVal = SendCommandPid(commandText, strWebResponse, "");
    return retVal;
}

bool GraphicsServerCommunication::SetProjectName(const gtASCIIString& projectName)
{
    bool retVal = false;

    // Connect
    gtASCIIString commandText = m_strApiHttpCommand;
    gtASCIIString strWebResponse;
    commandText.appendFormattedString("/SetProjectName.txt=%s", projectName.asCharArray());
    ReplaceSpecialHTTPCharacters(commandText);
    retVal = SendCommandPid(commandText, strWebResponse, "");
    return retVal;
}
