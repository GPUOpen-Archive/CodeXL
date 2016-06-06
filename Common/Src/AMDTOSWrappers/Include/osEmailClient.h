//=====================================================================
// Copyright 2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file osEmailClient.h
///
//=====================================================================

//------------------------------ osEmailClient.h ------------------------------

#ifndef __OSEMAILCLIENT_H
#define __OSEMAILCLIENT_H

// Infra:
#include <AMDTBaseTools/Include/gtVector.h>

// Local:
#include <AMDTOSWrappers/Include/osOSDefinitions.h>
#include <AMDTOSWrappers/Include/osTCPSocketClient.h>
#include <AMDTOSWrappers/Include/osPortAddress.h>


// ----------------------------------------------------------------------------------
// Class Name:           OS_API osEmailClient
//
// General Description:
//   A simple email client that enables sending emails through a mail server using
//   the SMTP protocol.
//
// Author:      AMD Developer Tools Team
// Creation Date:        3/6/2008
// ----------------------------------------------------------------------------------
class OS_API osEmailClient
{
public:
    osEmailClient();
    osEmailClient(const osPortAddress& serverPortAddress);
    virtual ~osEmailClient();

    void setServerAndPort(const osPortAddress& portAddress) {_smtpServerAddress = portAddress; } ;
    const osPortAddress& serverAndPort() const {return _smtpServerAddress;};
    bool connect();
    bool disconnect();
    bool sendEmail(const gtASCIIString& subject, const gtASCIIString& messageBody, const gtASCIIString& fromAddress, const gtASCIIString& fromName, const gtVector<gtASCIIString>& toList,
                   const gtVector<gtASCIIString>& ccList = osEmptyRecipientsList, const gtVector<gtASCIIString>& bccList = osEmptyRecipientsList);
    gtASCIIString getLastErrorCode() const { return _errorCode; };
    void setAuthenticationData(const gtASCIIString& userName, const gtASCIIString& password) {_userName = userName; _password = password; };

    // An empty recipients list:
    static const gtVector<gtASCIIString> osEmptyRecipientsList;

private:
    // Add a recipient to the TO_SMTP list:
    bool addEmailRecipient(const gtASCIIString& emailAddress);

    // Reads the server's response
    bool readServerResponse(gtASCIIString& response);

    // Gets the reply code from a string
    static bool verifyReplyCode(const gtASCIIString& serverResponse, const gtASCIIString& expectedReplyCode);

    // Convert information to Base64
    static bool convertStringToBase64(const gtASCIIString& src, gtASCIIString& dest);
    static bool convertBase64ToString(const gtASCIIString&, gtASCIIString&);

    // Send authentication data and return whether it worked
    bool authenticateSession();


private:
    // A TCP IP client, used to perform SMTP requests:
    osTCPSocketClient _tcpClient;

    // Contains the SMTP server address:
    osPortAddress _smtpServerAddress;

    // Contains the error code, returned from the SMTP server:
    gtASCIIString _errorCode;

    // Contains the authentication information (username and password), or empty strings if it's unneeded:
    // _userName should be in the format "foo@bar.org".
    gtASCIIString _userName;
    gtASCIIString _password;
};


#endif //__OSEMAILCLIENT_H

