//=====================================================================
// Copyright 2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file osEmailClient.cpp
///
//=====================================================================

//------------------------------ osEmailClient.cpp ------------------------------


// SMTP protocol examples and docs can be found under:
// - http://www.codeguru.com/forum/showthread.php?t=300530
// - http://www.codeguru.com/forum/showthread.php?t=339589
// - http://en.wikipedia.org/wiki/Simple_Mail_Transfer_Protocol
// - http://forums.msdn.microsoft.com/en-US/vclanguage/thread/12345906-8a17-41c2-846f-fd3e1a135238/

// Local:
#include <AMDTOSWrappers/Include/osStringConstants.h>
#include <AMDTOSWrappers/Include/osDebugLog.h>
#include <AMDTOSWrappers/Include/osMachine.h>
#include <AMDTOSWrappers/Include/osTime.h>
#include <AMDTOSWrappers/Include/osThread.h>
#include <AMDTOSWrappers/Include/osEmailClient.h>

// The size of a read aid buffer:
#define READ_BUFFER_SIZE 1024

// How many times to try loggin in:
#define OS_SMTP_AUTHENTICATION_ATTEMPTS 3

// Static members initializations:
const gtVector<gtASCIIString> osEmailClient::osEmptyRecipientsList;


// ---------------------------------------------------------------------------
// Name:        osEmailClient::osEmailClient
// Description: Constructor
// Author:      AMD Developer Tools Team
// Date:        3/6/2008
// ---------------------------------------------------------------------------
osEmailClient::osEmailClient()
{
}


// ---------------------------------------------------------------------------
// Name:        osEmailClient::osEmailClient
// Description: Constructor
// Arguments: portAddress - The SMTP server address and port number.
// Author:      AMD Developer Tools Team
// Date:        3/6/2008
// ---------------------------------------------------------------------------
osEmailClient::osEmailClient(const osPortAddress& portAddress)
    : _smtpServerAddress(portAddress)
{
}


// ---------------------------------------------------------------------------
// Name:        osEmailClient::~osEmailClient
// Description: Destructor
// Author:      AMD Developer Tools Team
// Date:        3/6/2008
// ---------------------------------------------------------------------------
osEmailClient::~osEmailClient()
{
    if (_tcpClient.isOpen())
    {
        disconnect();
    }
}


// ---------------------------------------------------------------------------
// Name:        osEmailClient::connect
// Description: Connects to the server specified in _smtpServerAddress.
// Return Val: bool  - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        3/6/2008
// ---------------------------------------------------------------------------
bool osEmailClient::connect()
{
    bool retVal = false;
    bool disconnectionSuccessful = true;

    // If we are already connected:
    if (_tcpClient.isOpen())
    {
        // We want to d/c and reconnect since the user may have changed the server since last connecting.
        disconnectionSuccessful = disconnect();
    }

    // If we didn't manage to disconnect:
    if (!disconnectionSuccessful)
    {
        _errorCode = OS_STR_cannotEndTCPSession;
    }
    else
    {
        // Open the socket:
        bool tcpSocketOpen = _tcpClient.open();

        if (!tcpSocketOpen)
        {
            _errorCode = OS_STR_cannotOpenTCPSocket;
        }
        else
        {
            // Connect the socket to the SMTP server:
            bool rc = _tcpClient.connect(_smtpServerAddress);

            if (rc)
            {
                // Output debug log message:
                gtString logMessage;
                logMessage.appendFormattedString(OS_STR_connectedToSMTPServer, _smtpServerAddress.hostName().asCharArray(), _smtpServerAddress.portNumber());
                OS_OUTPUT_DEBUG_LOG(logMessage.asCharArray(), OS_DEBUG_LOG_DEBUG);

                // Read server respond and verify it:
                gtASCIIString serverResponse;
                retVal = readServerResponse(serverResponse);
                retVal = retVal && verifyReplyCode(serverResponse, "220");

                if (retVal)
                {
                    // First try connecting with ESMTP. if this fails, try with SMTP:
                    // Create and send the EHLO string:
                    gtString localMachineNameString;
                    bool rcClientName = osGetLocalMachineName(localMachineNameString);

                    if (rcClientName)
                    {
                        gtASCIIString heloString = "EHLO ";
                        heloString.append(localMachineNameString.asASCIICharArray());
                        heloString.append("\r\n");
                        bool rcEhlo = _tcpClient.write((gtByte*)heloString.asCharArray(), heloString.length());

                        if (rcEhlo)
                        {
                            // Verify we got an "250 Ok" response:
                            retVal = readServerResponse(serverResponse);
                            retVal = retVal && verifyReplyCode(serverResponse, "250");
                        }
                    }

                    if (!retVal)
                    {
                        // if we failed since we got the "550 not implemented" response or the "500 command unrecognized" response:
                        bool esmtpNotAvailable = verifyReplyCode(serverResponse, "550") || verifyReplyCode(serverResponse, "500");

                        if (esmtpNotAvailable)
                        {
                            // Create and send the HELO string:
                            gtASCIIString localMachineName;
                            rcClientName = osGetLocalMachineName(localMachineName);

                            if (rcClientName)
                            {
                                gtASCIIString heloString = "HELO ";
                                heloString.append(localMachineName);
                                heloString.append("\r\n");
                                bool rcHelo = _tcpClient.write((gtByte*)heloString.asCharArray(), heloString.length());

                                if (rcHelo)
                                {
                                    // Verify we got an "250 Ok" response:
                                    retVal = readServerResponse(serverResponse);
                                    retVal = retVal && verifyReplyCode(serverResponse, "250");
                                }
                            }
                        }
                    }

                    if (retVal)
                    {
                        if (!(_userName.isEmpty()) && !(_password.isEmpty()))
                        {
                            // if we recieved a username and password, use them now:
                            retVal = authenticateSession();

                            if (!retVal)
                            {
                                // if we tried to authenticate but failed (eg wrong username / pass), disconnect
                                retVal = disconnect() && retVal;
                            }
                        }
                    }
                }
                else
                {
                    _errorCode = serverResponse;
                }
            }
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        osEmailClient::disconnect
// Description: Disconnects current session.
// Return Val: bool  - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        3/6/2008
// ---------------------------------------------------------------------------
bool osEmailClient::disconnect()
{
    bool retVal = false;

    // If the socket is not open:
    if (!_tcpClient.isOpen())
    {
        // We have nothing to do:
        retVal = true;
    }
    else
    {
        // Let the server know that we are quitting the session:
        bool rcQuit = _tcpClient.write((gtByte*)"QUIT\r\n", 6);

        if (rcQuit)
        {
            // Verify we got a "221 goodbye" response:
            gtASCIIString serverResponse;
            rcQuit = readServerResponse(serverResponse);
            rcQuit = rcQuit && verifyReplyCode(serverResponse, "221");
        }

        // If we didn't manage to quit normally, try and close the TCP connection:
        retVal = _tcpClient.close() || rcQuit;
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        osEmailClient::sendEmail
// Description: Sends an email message.
// Arguments:                           example
//  subject - The message subject.      "Assertion failure"
//  messageBody - The message body.
//  fromAddress - Sender email.         assertbot@gremedy.com
//  fromName - Sender name.             GRemedy Assert bot
//  toList - A list of to recipients.   yaki@gremedy.com
//  ccList - A list of to recipients.       -"-
//  bccList - A list of to recipients.      -"-
//      Note that this does not allow using recipient name and instead uses the
//      recipient's email address as their name.
//
// Return Val: bool  - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        3/6/2008
// ---------------------------------------------------------------------------
bool osEmailClient::sendEmail(const gtASCIIString& subject, const gtASCIIString& messageBody, const gtASCIIString& fromAddress,
                              const gtASCIIString& fromName, const gtVector<gtASCIIString>& toList,
                              const gtVector<gtASCIIString>& ccList, const gtVector<gtASCIIString>& bccList)
{
    bool retVal = true;

    // If we lost the connection, try to reconnect:
    bool isConnected = true;

    if (!_tcpClient.isOpen())
    {
        isConnected = connect();
    }

    // Verify that we are connected:
    if (isConnected)
    {
        gtASCIIString serverResponse;

        // Start making the headers for the email's data:
        gtASCIIString dataString = "From: \"";
        dataString.append(fromName);
        dataString.append("\" <");
        dataString.append(fromAddress);
        dataString.append(">");

        // Generate and send the MAIL FROM string:
        gtASCIIString fromString = "MAIL FROM:<";
        fromString.append(fromAddress);
        fromString.append(">\r\n");
        bool rcFrom = _tcpClient.write((gtByte*)fromString.asCharArray(), fromString.length());

        if (rcFrom)
        {
            // Verify we got an "250 Ok" reply
            rcFrom = readServerResponse(serverResponse);
            rcFrom = rcFrom && verifyReplyCode(serverResponse, "250");
        }

        retVal = retVal && rcFrom;

        // Add the recipients (To, CC, BCC) as RCPT TO as well as adding them
        // to the data string
        int sizeOfToList = (int)toList.size();
        bool isFirstOfRecipientType = true;
        bool rcRCPT = false;

        for (int i = 0; i < sizeOfToList ; i++)
        {
            rcRCPT = addEmailRecipient(toList[i]);
            retVal = retVal && rcRCPT;

            if (isFirstOfRecipientType)
            {
                dataString.append("\r\nTo: ");
                isFirstOfRecipientType = false;
            }
            else
            {
                dataString.append(", \r\n");
            }

            dataString.append("\"");
            dataString.append(toList[i]);
            dataString.append("\" <");
            dataString.append(toList[i]);
            dataString.append(">");
        }

        int sizeOfCcList = (int)ccList.size();
        isFirstOfRecipientType = true;

        for (int i = 0; i < sizeOfCcList ; i++)
        {
            rcRCPT = addEmailRecipient(ccList[i]);
            retVal = retVal && rcRCPT;

            if (isFirstOfRecipientType)
            {
                dataString.append("\r\nCc: ");
                isFirstOfRecipientType = false;
            }
            else
            {
                dataString.append(", \r\n");
            }

            dataString.append("\"");
            dataString.append(ccList[i]);
            dataString.append("\" <");
            dataString.append(ccList[i]);
            dataString.append(">");
        }

        int sizeOfBccList = (int)bccList.size();

        for (int i = 0; i < sizeOfBccList ; i++)
        {
            rcRCPT = addEmailRecipient(bccList[i]);
            retVal = retVal && rcRCPT;

            // Don't add to the data string since this is a BCC
        }

        bool rcData = _tcpClient.write((gtByte*)"DATA\r\n", 6);

        if (rcData)
        {
            rcData = readServerResponse(serverResponse);
            rcData = rcData && verifyReplyCode(serverResponse, "354");

            bool readyToRecieve = false;
            gtASCIIString dataTerminator;

            if (rcData)
            {
                readyToRecieve = true;
                dataTerminator = "\r\n.\r\n";
            }

            if (readyToRecieve)
            {
                // Add the subject and local date to the data string:
                dataString.append("\r\nSubject: ");
                dataString.append(subject);
                dataString.append("\r\nDate: ");
                gtASCIIString dateAsString;
                osTime now;
                now.setFromCurrentTime();
                now.dateAsString(dateAsString, osTime::FOR_EMAIL, osTime::LOCAL);
                dataString.append(dateAsString);

                // Add the message body
                dataString.append("\r\n\r\n");
                dataString.append(messageBody);
                dataString.append(dataTerminator);

                // send the data string
                bool rcDataSent = _tcpClient.write((gtByte*)dataString.asCharArray(), dataString.length());

                if (rcDataSent)
                {
                    // verify we got a "250 Ok" reply
                    rcDataSent = readServerResponse(serverResponse);
                    rcDataSent = rcDataSent && verifyReplyCode(serverResponse, "250");
                    retVal = rcDataSent;
                }
                else
                {
                    _errorCode = "Problem writing data to server";
                }
            }
            else
            {
                _errorCode = serverResponse;
            }
        }


        retVal = retVal && rcData;
    }
    else
    {
        _errorCode = "Problem connecting to server by TCP";
    }

    retVal = retVal && isConnected;

    if (!retVal)
    {
        gtString errorCode;
        errorCode.fromASCIIString(_errorCode.asCharArray());
        OS_OUTPUT_DEBUG_LOG(errorCode.asCharArray(), OS_DEBUG_LOG_ERROR);
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        osEmailClient::addEmailRecipient
// Description: Adds an SMTP_TO recipient to the next sent email. Note that
//              All CC and BCC recipients must be added this way as well.
// Arguments: emailAddress - the email address of the recipient.
// Return Val: bool  - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        3/6/2008
// ---------------------------------------------------------------------------
bool osEmailClient::addEmailRecipient(const gtASCIIString& emailAddress)
{
    bool retVal = false;
    bool isConnected = true;

    if (!_tcpClient.isOpen()) // If we lost the connection, try to reconnect:
    {
        isConnected = connect();
    }

    if (isConnected)
    {
        // send the RCPT TO string
        gtASCIIString recipientRequest = "RCPT TO:<";
        recipientRequest.append(emailAddress);
        recipientRequest.append(">\r\n");

        int stringLength = recipientRequest.length();
        retVal = _tcpClient.write((gtByte*)recipientRequest.asCharArray(), stringLength);
    }

    if (retVal)
    {
        // Verify that we got a "250 Ok" reply
        gtASCIIString serverResponse;
        retVal = readServerResponse(serverResponse);
        retVal = retVal && verifyReplyCode(serverResponse, "250");

        if (!retVal)
        {
            _errorCode = serverResponse;
        }
    }
    else
    {
        _errorCode = "TCP Error";
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        osEmailClient::readServerResponse
// Description: Reads the SMTP server's response
// Arguments: response - the response goes here
// Return Val: bool - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        3/6/2008
// ---------------------------------------------------------------------------
bool osEmailClient::readServerResponse(gtASCIIString& response)
{
    bool retVal = false;
    response.makeEmpty();
    gtByte buff[READ_BUFFER_SIZE + 1];

    // Wait for the server's response:
    osSleep(1);

    gtSize_t amountOfDataRead = 0;
    bool moreToRead = true;
    bool lastBufferEmpty = false;
    bool packetLoss = false;

    while (moreToRead)
    {
        // Try to read fromAddress the TCP socket
        bool rc2 = _tcpClient.readAvailableData(buff, READ_BUFFER_SIZE, amountOfDataRead);

        if (!rc2)
        {
            if (!lastBufferEmpty)
            {
                // We tried to read, but there is no data available:
                lastBufferEmpty = true;
            }
            else
            {
                // We already waited for data, but didn't get it after the sleep period:
                moreToRead = false;
                packetLoss = true;
            }
        }
        else
        {
            // We did manage to read fromAddress the socket:
            lastBufferEmpty = false;

            if (amountOfDataRead == 0)
            {
                // The socket was closed
                moreToRead = false;
            }
            else
            {
                // We got data:
                buff[amountOfDataRead] = 0;
                gtASCIIString interimTextBuffer = (const char*)buff;
                response.append(interimTextBuffer);

                // We assume we got everything since the SMTP server responses are typically short
                moreToRead = false;
                retVal = true;
            }
        }
    }

    if (packetLoss)
    {
        _errorCode = OS_STR_packetLoss;
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        osEmailClient::verifyReplyCode
// Description: Verifies that serverResponse is a legal SMTP server response and
//              that the reply code is expectedReplycode
// Return Val: bool - is it the right kind of response?
// Author:      AMD Developer Tools Team
// Date:        3/6/2008
// ---------------------------------------------------------------------------
bool osEmailClient::verifyReplyCode(const gtASCIIString& serverResponse, const gtASCIIString& expectedReplyCode)
{
    bool retVal = false;
    int respLength = serverResponse.length();

    if (respLength >= 3)
    {
        gtASCIIString replyCode;
        serverResponse.getSubString(0, 2, replyCode);
        retVal = (replyCode == expectedReplyCode);
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        osEmailClient::convertStringToBase64
// Description: Converts the string in src to a Base64 string in dest
// Return Val: bool  - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        26/6/2008
// ---------------------------------------------------------------------------
bool osEmailClient::convertStringToBase64(const gtASCIIString& src, gtASCIIString& dest)
{
    bool retVal = true;

    const gtASCIIString base64Dictionary = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

    unsigned int strLenBy3 = src.length() / 3;
    unsigned int strLenMod3 = src.length() % 3;

    unsigned long buffer;

    for (unsigned int i = 0; i < strLenBy3; i++)
    {
        char c1 = src[3 * i];
        char c2 = src[3 * i + 1];
        char c3 = src[3 * i + 2];

        buffer = (c1 << 16) | (c2 << 8) | (c3);

        unsigned int a1 = buffer / (64 * 64 * 64);
        unsigned int a2 = (buffer % (64 * 64 * 64)) / (64 * 64);
        unsigned int a3 = (buffer % (64 * 64)) / 64;
        unsigned int a4 = buffer % 64;

        dest.appendFormattedString("%c%c%c%c", base64Dictionary[a1], base64Dictionary[a2], base64Dictionary[a3], base64Dictionary[a4]);
    }

    if (strLenMod3 == 1)
    {
        char c1 = src[3 * strLenBy3];
        buffer = (c1 << 16);

        unsigned int a1 = (char)(buffer / (64 * 64 * 64));
        unsigned int a2 = (char)((buffer % (64 * 64 * 64)) / (64 * 64));
        dest.appendFormattedString("%c%c==", base64Dictionary[a1], base64Dictionary[a2]);
    }
    else if (strLenMod3 == 2)
    {
        char c1 = src[3 * strLenBy3];
        char c2 = src[3 * strLenBy3 + 1];

        buffer = (c1 << 16) | (c2 << 8);

        char a1 = (char)(buffer / (64 * 64 * 64));
        char a2 = (char)((buffer % (64 * 64 * 64)) / (64 * 64));
        char a3 = (char)((buffer % (64 * 64)) / 64);

        dest.appendFormattedString("%c%c%c=", base64Dictionary[a1], base64Dictionary[a2], base64Dictionary[a3]);
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        osEmailClient::convertBase64ToString
// Description:  Converts the Base64 string in src to a string in dest
// Return Val: bool  - Success / failure. (will usually fail if src is not a
//              valid Base64 string.
// Author:      AMD Developer Tools Team
// Date:        26/6/2008
// ---------------------------------------------------------------------------
bool osEmailClient::convertBase64ToString(const gtASCIIString& src, gtASCIIString& dest)
{
    (void)(src); // unused
    (void)(dest); // unused
    bool retVal = false;

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        osEmailClient::authenticateSession
// Description: Use the preset authentication data to login to the server
// Return Val: bool  - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        26/6/2008
// ---------------------------------------------------------------------------
bool osEmailClient::authenticateSession()
{
    bool retVal = false;
    gtASCIIString userNameAsBase64;
    gtASCIIString passwordAsBase64;
    bool rcBase64 = convertStringToBase64(_userName, userNameAsBase64);
    rcBase64 = convertStringToBase64(_password, passwordAsBase64) && rcBase64;

    if (rcBase64)
    {
        userNameAsBase64.append("\r\n");
        passwordAsBase64.append("\r\n");

        if (_tcpClient.isOpen())
        {
            gtASCIIString authString = "AUTH LOGIN\r\n";
            bool rcAuth = _tcpClient.write((gtByte*)authString.asCharArray(), authString.length());

            if (rcAuth)
            {
                gtASCIIString serverResponse;

                for (int trysLeft = OS_SMTP_AUTHENTICATION_ATTEMPTS; trysLeft > 0; trysLeft--)
                {
                    // Check we got a "334 VXNlcm5hbWU6" ("334 Username:") response:
                    retVal = readServerResponse(serverResponse);
                    retVal = retVal && verifyReplyCode(serverResponse, "334");

                    if (retVal)
                    {
                        // Write the username:
                        retVal = _tcpClient.write((gtByte*)userNameAsBase64.asCharArray(), userNameAsBase64.length());

                        if (retVal)
                        {
                            // Check we got a "334 UGFzc3dvcmQ6" ("334 Password:") response:
                            retVal = readServerResponse(serverResponse);
                            retVal = retVal && verifyReplyCode(serverResponse, "334");

                            if (retVal)
                            {
                                // Write the password:
                                retVal = _tcpClient.write((gtByte*)passwordAsBase64.asCharArray(), passwordAsBase64.length());

                                if (retVal)
                                {
                                    retVal = readServerResponse(serverResponse);
                                    // See if we got a "235 Authentication successfu" response:
                                    retVal = retVal && verifyReplyCode(serverResponse, "235");

                                    if (retVal)
                                    {
                                        // We succeeded, break the loop.
                                        trysLeft = 0;
                                    }
                                    else
                                    {
                                        // This will probably be a "535 incorrect authentication data" response:
                                        _errorCode = serverResponse;
                                    }
                                }
                                else
                                {
                                    _errorCode = "Problem writing authentication data";
                                }
                            }
                            else
                            {
                                _errorCode = serverResponse;
                            }
                        }
                        else
                        {
                            _errorCode = "Problem writing authentication data";
                        }
                    }
                    else
                    {
                        _errorCode = serverResponse;
                    }
                }
            }
            else
            {
                _errorCode = "Problem writing authentication data";
            }
        }
    }
    else
    {
        _errorCode = "Problem converting authentication data to Base64.";
    }

    return retVal;
}
