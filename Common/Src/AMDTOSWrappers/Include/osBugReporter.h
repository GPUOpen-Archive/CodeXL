//=====================================================================
// Copyright 2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file osBugReporter.h
///
//=====================================================================

//------------------------------ osBugReporter.h ------------------------------

#ifndef __OSBUGREPORTER_H
#define __OSBUGREPORTER_H

// Foreward declarations:
class osPortAddress;

// Infra:
#include <AMDTBaseTools/Include/gtASCIIString.h>

// Local:
#include <AMDTOSWrappers/Include/osOSWrappersDLLBuild.h>


// ----------------------------------------------------------------------------------
// Class Name:           osBugReporter
// General Description:
//   Reports bugs to a CRM Web server.
//   (CRM = Customer Relations Management)
//
// Author:      AMD Developer Tools Team
// Creation Date:        24/11/2005
// ----------------------------------------------------------------------------------
class OS_API osBugReporter
{
public:
    osBugReporter(const gtASCIIString& CRMServerURL, const gtASCIIString& bugSubmissionCGIRelativeURL,
                  const gtASCIIString& projectName, const gtASCIIString& bugArea,
                  const gtASCIIString& bugTitle, const gtASCIIString& bugDescription,
                  const gtASCIIString& reporterUserName, const gtASCIIString& reporterEmail,
                  bool createNewBugIfTitleExists);
    virtual ~osBugReporter();

    bool reportBug(gtASCIIString& CRMSystemReturnedMessage, bool isUsingProxy, const osPortAddress& proxyServer);

private:
    void buildHTTPRequestString(gtASCIIString& requestString);
    void adjustStringToContentType(gtASCIIString& requestString);
    bool sendRequestToCRMSystem(const gtASCIIString& requestString, gtASCIIString& returnedPage, bool isUsingProxy, const osPortAddress& proxyServer);
    bool parseReturnedPage(const gtASCIIString& returnedPage, gtASCIIString& CRMSystemReturnedMessage);

    // Do not allow the use of my default constructor:
    osBugReporter();

private:
    // The CRM server URL:
    gtASCIIString _CRMServerURL;

    // The relative URL (under the CRM server) of the bug submission script:
    gtASCIIString _bugSubmissionCGIRelativeURL;

    // The name of the project into which the bug will be reported:
    gtASCIIString _projectName;

    // The Area into which the bug will be reported:
    gtASCIIString _bugArea;

    // The bug title (uniquely identifies the bug):
    gtASCIIString _bugTitle;

    // The bug description:
    gtASCIIString _bugDescription;

    // The bug reporter user name:
    gtASCIIString _reporterUserName;

    // The bug reporter user email:
    gtASCIIString _reporterEmail;

    // Always create a new bug (even if a bug with the same title already exist):
    bool _createNewBugIfTitleExists;
};


#endif //__OSBUGREPORTER_H
