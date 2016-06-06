//=====================================================================
// Copyright 2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file osBugReporter.cpp
///
//=====================================================================

//------------------------------ osBugReporter.cpp ------------------------------

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>

// Local:
#include <AMDTOSWrappers/Include/osBugReporter.h>
#include <AMDTOSWrappers/Include/osHTTPClient.h>

// An aid buffer length:
#define OS_HTTP_REQUEST_BUFF_LEN 1024


// ---------------------------------------------------------------------------
// Name:        osBugReporter::osBugReporter
// Description: Constructor
// Arguments: CRMServerURL - The CRM server URL (Example: "support.gremedy.com")
//            bugSubmissionCGIRelativeURL - The relative URL (under the CRM server) of the
//                                          bug submission script (Example: "/scoutsubmit.asp").
//            projectName - The name of the project into which the bug will be reported.
//            bugArea - The Area into which the bug will be reported.
//            bugTitle - The bug title. Notice that this title uniquely identifies the bug.
//            bugDescription - The bug description.
//            additionalInformation - Any additional information that should be attached
//                                    to the bug report.
//            reporterUserName - The bug reporter user name.
//            reporterEmail - The bug reporter email.
//            createNewBugIfTitleExists - true - a new bug will be opened even if bug with the same
//                                               title exists in the CRM system.
//                                        false - If a bug with the same title already exist in the CRM
//                                                system, this bug information will be appended into it.
// Author:      AMD Developer Tools Team
// Date:        24/11/2005
// ---------------------------------------------------------------------------
osBugReporter::osBugReporter(const gtASCIIString& CRMServerURL, const gtASCIIString& bugSubmissionCGIRelativeURL,
                             const gtASCIIString& projectName, const gtASCIIString& bugArea,
                             const gtASCIIString& bugTitle, const gtASCIIString& bugDescription,
                             const gtASCIIString& reporterUserName, const gtASCIIString& reporterEmail,
                             bool createNewBugIfTitleExists)
    : _CRMServerURL(CRMServerURL), _bugSubmissionCGIRelativeURL(bugSubmissionCGIRelativeURL),
      _projectName(projectName), _bugArea(bugArea), _bugTitle(bugTitle), _bugDescription(bugDescription),
      _reporterUserName(reporterUserName), _reporterEmail(reporterEmail), _createNewBugIfTitleExists(createNewBugIfTitleExists)
{
}


// ---------------------------------------------------------------------------
// Name:        osBugReporter::~osBugReporter
// Description: Destructor
// Author:      AMD Developer Tools Team
// Date:        24/11/2005
// ---------------------------------------------------------------------------
osBugReporter::~osBugReporter()
{
}


// ---------------------------------------------------------------------------
// Name:        osBugReporter::reportBug
// Description: Reports the bug.
// Arguments: Will get the CRM system returned message.
// Return Val: bool  - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        24/11/2005
// Implementation notes:
//   To post a bug into a FogBugz system, we send an HTTP POST request to the
//   FogBugz server scoutsubmit.asp page. Example of such a request can be:
//   http://support.gremedy.com/scoutsubmit.asp?Description=Test description&Extra=Test extra
//   &Email=foo@goo.com&ScoutUserName=Yaki Tebeka&ScoutProject=CodeXL&ScoutArea=Support
//   &ScoutDefaultMessage=Test default mgs&ForceNewBug=0
//
//   The server send us back an XML page that contains a message from the FogBugz server.
//   Success message example:
//   <?xml version="1.0" ?><Success>Test default mgs</Success>
//
//   Failure message example:
//   <?xml version="1.0" ?><Error>User not valid: Moo</Error>
// ---------------------------------------------------------------------------
bool osBugReporter::reportBug(gtASCIIString& CRMSystemReturnedMessage, bool isUsingProxy, const osPortAddress& proxyServer)
{
    bool retVal = false;

    // Build an HTTP "POST" request string that will perform the bug submission:
    gtASCIIString httpPOSTRequestString;
    buildHTTPRequestString(httpPOSTRequestString);

    // Send the request to the CRM system:
    gtASCIIString returnedPage;
    bool rc1 = sendRequestToCRMSystem(httpPOSTRequestString, returnedPage, isUsingProxy, proxyServer);

    if (rc1)
    {
        // Parse the page returned from the CRM system:
        retVal = parseReturnedPage(returnedPage, CRMSystemReturnedMessage);
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        osBugReporter::buildHTTPRequestString
// Description:
//  Builds an HTTP "POST" request string that performs the bug submission.
//
// Implementation notes:
//   The bug submission is of the following form:
//   scoutsubmit.asp?Description=Test description&Extra=Test extra
//   &Email=foo@goo.com&ScoutUserName=Yaki Tebeka&ScoutProject=CodeXL&ScoutArea=Support
//   &ScoutDefaultMessage=Test default mgs&ForceNewBug=0
//
// Author:      AMD Developer Tools Team
// Date:        27/11/2005
// ---------------------------------------------------------------------------
void osBugReporter::buildHTTPRequestString(gtASCIIString& requestString)
{
    requestString.makeEmpty();

    // Add the bug arguments:
    requestString += "Description=";
    requestString += _bugTitle;

    if (!_bugDescription.isEmpty())
    {
        requestString += "&Extra=";
        requestString += _bugDescription;
    }

    if (!_reporterEmail.isEmpty())
    {
        requestString += "&Email=";
        requestString += _reporterEmail;
    }

    if (!_reporterUserName.isEmpty())
    {
        requestString += "&ScoutUserName=";
        requestString += _reporterUserName;
    }

    if (!_projectName.isEmpty())
    {
        requestString += "&ScoutProject=";
        requestString += _projectName;
    }

    if (!_bugArea.isEmpty())
    {
        requestString += "&ScoutArea=";
        requestString += _bugArea;
    }

    requestString += "&ScoutDefaultMessage=";
    requestString += "Thanks for the bug report";

    requestString += "&ForceNewBug=";

    if (_createNewBugIfTitleExists)
    {
        requestString += "1";
    }
    else
    {
        requestString += "0";
    }

    // Adjust the request string to the "application/x-www-form-urlencoded" content type:
    adjustStringToContentType(requestString);
}


// ---------------------------------------------------------------------------
// Name:        osBugReporter::adjustStringToContentType
// Description:
//  Inputs a string and adjust it to the "application/x-www-form-urlencoded"
//  content type.
//
// Implementation notes:
//
//  From http://www.w3.org/TR/html4/interact/forms.html#h-17.13.4.1 :
//  Forms submitted with this content type must be encoded as follows:
//  1. Control names and values are escaped. Space characters are replaced by `+',
//     and then reserved characters are escaped as described in [RFC1738],
//     section 2.2: Non-alphanumeric characters are replaced by `%HH',
//     a percent sign and two hexadecimal digits representing the ASCII code of the character.
//     Line breaks are represented as "CR LF" pairs (i.e., `%0D%0A').
//  2. The control names/values are listed in the order they appear in the document.
//     The name is separated from the value by `=' and name/value pairs are separated
//     from each other by `&'.
//                                                                                                                                                                                                                                                                                               2. The control names/values are listed in the order they appear in the document. The name is separated from the value by `=' and name/value pairs are separated from each other by `&'.
// Author:      AMD Developer Tools Team
// Date:        5/7/2007
// ---------------------------------------------------------------------------
void osBugReporter::adjustStringToContentType(gtASCIIString& requestString)
{
    // Replace % with its ASCII hexadecimally representation:
    requestString.replace("%", "%25", true);

    // Replace # with its ASCII hexadecimally representation:
    requestString.replace("#", "%23", true);

    // Replace quotes (") with its ASCII hexadecimally representation:
    requestString.replace("\"", "%22", true);

    // Replace spaces with +:
    requestString.replace(" ", "%20", true);

    // Replace "\n" by "CR LF" pairs:
    requestString.replace("\n", "%0D%0A", true);
}


// ---------------------------------------------------------------------------
// Name:        osBugReporter::sendRequestToCRMSystem
// Description: Sends the HTTP POST request to the CRM system.
// Arguments: requestString - The POST request string.
//            returnedPage - The result page returned from the CRM system.
// Return Val: bool  - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        5/7/2007
// ---------------------------------------------------------------------------
bool osBugReporter::sendRequestToCRMSystem(const gtASCIIString& requestString, gtASCIIString& returnedPage, bool isUsingProxy, const osPortAddress& proxyServer)
{
    bool retVal = false;

    // Setup an HTTP client that will perform our HTTP request:
    osPortAddress bugReportServer(_CRMServerURL, 80);

    if (isUsingProxy)
    {
        bugReportServer = proxyServer;
    }

    osHTTPClient httpClient(bugReportServer);

    // Set the POST request data:
    httpClient.setPostBuffer(requestString);

    // Set the CRM Web server address:
    bool rc1 = httpClient.connect();
    GT_IF_WITH_ASSERT(rc1)
    {
        // Execute the HTTP submission:
        gtASCIIString httpRequestResult;
        bool rc2 = httpClient.RequestPagePost(_bugSubmissionCGIRelativeURL, httpRequestResult, isUsingProxy, _CRMServerURL.asCharArray());
        GT_IF_WITH_ASSERT(rc2)
        {
            // Set the returned page:
            returnedPage = httpRequestResult;

            // Convert the given string from HTML to ASCII:
            returnedPage.decodeHTML();

            retVal = true;
        }
        else
        {
            gtString errorCode;
            errorCode.fromASCIIString(httpClient.getLastErrorCode().asCharArray());
            GT_ASSERT_EX(rc2, errorCode.asCharArray());
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        osBugReporter::parseReturnedPage
// Description: Parses the page returned from the CRM server.
// Arguments: returnedPage - The returned page as a string.
//            CRMSystemReturnedMessage - Will get the CRM system "human readable"
//                                       returned string.
// Return Val: bool  - true iff the bug submission was successful.
// Author:      AMD Developer Tools Team
// Date:        28/11/2005
// Implementation Notes:
//   The FogBugz server send us back an XML page that contains:
//   a. Success / Error - as an XML tag.
//   b. A human readable string associated with the bug report.
//
//   Success message example:
//   <?xml version="1.0" ?><Success>Test default mgs</Success>
//
//   Failure message example:
//   <?xml version="1.0" ?><Error>User not valid: Moo</Error>
// ---------------------------------------------------------------------------
bool osBugReporter::parseReturnedPage(const gtASCIIString& returnedPage, gtASCIIString& CRMSystemReturnedMessage)
{
    bool retVal = false;
    CRMSystemReturnedMessage.makeEmpty();

    // Get the returned message in a lower case representation:
    gtASCIIString returnedPageLowerCase = returnedPage;
    returnedPageLowerCase.toLowerCase();

    // Search for a "<success>" tag:
    gtASCIIString successTag("<success>");
    int successTagPos = returnedPageLowerCase.find(successTag);

    // If there is a "<success>" tag:
    if (successTagPos != -1)
    {
        // Search for the matching "</success>" tag:
        int successTagSize = successTag.length();
        int endSuccessTagPos = returnedPageLowerCase.find("</success>", successTagPos + successTagSize);

        // If we found the matching "</success>" tag:
        if (endSuccessTagPos)
        {
            // Output the CRM system returned "Human readable" string:
            returnedPage.getSubString(successTagPos + successTagSize, endSuccessTagPos - 1, CRMSystemReturnedMessage);
            retVal = true;
        }
    }
    else
    {
        // We didn't find a success tag.
        // Look for an "<error>" tag:
        gtASCIIString errorTag("<error>");
        int errorTagPos = returnedPageLowerCase.find(errorTag);

        // If there is an "<error>" tag:
        if (errorTagPos != -1)
        {
            int errorTagSize = errorTag.length();
            int endErrorTagPos = returnedPageLowerCase.find("</error>", errorTagPos + errorTagSize);

            // If we found the matching "</error>" tag:
            if (endErrorTagPos)
            {
                // Output the CRM system returned "Human readable" string:
                returnedPage.getSubString(errorTagPos + errorTagSize, endErrorTagPos - 1, CRMSystemReturnedMessage);
            }
        }
    }

    return retVal;
}

