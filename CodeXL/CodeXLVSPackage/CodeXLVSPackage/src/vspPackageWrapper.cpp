//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file vspPackageWrapper.cpp
///
//==================================================================================

//------------------------------ vspPackageWrapper.cpp ------------------------------

#include "stdafx.h"

// C++:
#include <string>

// Local:
#include <src/CommonIncludes.h>
#include <src/Package.h>
#include <src/vspCoreAPI.h>
#include <src/vspPackageWrapper.h>
#include <src/vscVsUtils.h>


#define _CTC_GUIDS_
#include <vsdebugguids.h>
#undef _CTC_GUIDS_
#include <vsshell.h>

// Static members initializations:
vspPackageWrapper* vspPackageWrapper::_pMySingleInstance = NULL;


// ---------------------------------------------------------------------------
// Name:        vspPackageWrapper::vspPackageWrapper
// Description: Constructor
// Author:      Sigal Algranaty
// Date:        24/10/2010
// ---------------------------------------------------------------------------
vspPackageWrapper::vspPackageWrapper()
    : _pPackage(NULL), _pOutputWindowPane(NULL)
{

}

// ---------------------------------------------------------------------------
// Name:        vspPackageWrapper::~vspPackageWrapper
// Description: Destructor
// Author:      Sigal Algranaty
// Date:        24/10/2010
// ---------------------------------------------------------------------------
vspPackageWrapper::~vspPackageWrapper()
{
    clearPackage();
}

// ---------------------------------------------------------------------------
// Name:        vspPackageWrapper::instance
// Description: Returns the single instance of this class. Creates it on
//              the first call to this function.
// Author:      Sigal Algranaty
// Date:        24/10/2010
// ---------------------------------------------------------------------------
vspPackageWrapper& vspPackageWrapper::instance()
{
    if (_pMySingleInstance == NULL)
    {
        _pMySingleInstance = new vspPackageWrapper;

    }

    return *_pMySingleInstance;
}

// ---------------------------------------------------------------------------
// Name:        vspPackageWrapper::setPackage
// Description: Set my package
// Arguments:   CCodeXLVSPackagePackage* pPackage
// Return Val:  void
// Author:      Sigal Algranaty
// Date:        24/10/2010
// ---------------------------------------------------------------------------
void vspPackageWrapper::setPackage(CodeXLVSPackage* pPackage)
{
    // We shouldn't get here twice, but make sure that we didn't, anyway:
    if (_pPackage != NULL)
    {
        VSP_ASSERT(_pPackage == NULL);
        _pPackage->Release();
        _pPackage = NULL;
    }

    // Copy the pointer:
    _pPackage = pPackage;

    // If we got a pointer, retain it:
    if (_pPackage != NULL)
    {
        _pPackage->AddRef();
    }
}

// ---------------------------------------------------------------------------
// Name:        vspPackageWrapper::clearPackage
// Description: Clears our pointer to the package and other members taken from it
// Author:      Uri Shomroni
// Date:        24/10/2011
// ---------------------------------------------------------------------------
void vspPackageWrapper::clearPackage()
{
    // No access to the output panel window
    if (_pOutputWindowPane != NULL)
    {
        _pOutputWindowPane->Release();
        _pOutputWindowPane = NULL;
    }

    // Release the Package interface:
    if (_pPackage != NULL)
    {
        _pPackage->Release();
        _pPackage = NULL;
    }
}

// ---------------------------------------------------------------------------
// Name:        vspPackageWrapper::isVSUIContextActive
// Description: Check with the VS monitor, is the requested UI context is active
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        3/7/2011
// ---------------------------------------------------------------------------
bool vspPackageWrapper::isVSUIContextActive(REFGUID rguidCmdUI)
{
    bool retVal = false;

    // Get the VS site cache:
    CodeXLVSPackage* pPackage = vspPackageWrapper::instance().getPackage();
    VSP_ASSERT(pPackage != NULL);

    if (pPackage != NULL)
    {

        // Check if the "Debugging" UI context is active:
        IVsMonitorSelection* piMonitorSelection = NULL;
        HRESULT hr = pPackage->GetVsSiteCache().QueryService(SID_SVsShellMonitorSelection, &piMonitorSelection);

        if (SUCCEEDED(hr) && (piMonitorSelection != NULL))
        {
            // Get a cookie for this UI context:
            VSCOOKIE uiContextCookie = NULL;
            hr = piMonitorSelection->GetCmdUIContextCookie(rguidCmdUI, &uiContextCookie);

            if (SUCCEEDED(hr))
            {
                // Check if it is active:
                BOOL isUIContextActive = FALSE;
                hr = piMonitorSelection->IsCmdUIContextActive(uiContextCookie, &isUIContextActive);

                if (SUCCEEDED(hr))
                {
                    retVal = (isUIContextActive == TRUE);
                }
            }

            piMonitorSelection->Release();
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        vspPackageWrapper::outputMessage
// Description: Send a text message to the output pane of visual studio
// Arguments:   gtString messageString
// Return Val:  void
// Author:      Gilad Yarnitzky
// Date:        17/2/2011
// ---------------------------------------------------------------------------
void vspPackageWrapper::outputMessage(const std::wstring& messageString, bool outputOnlyToLog)
{
    // Add to log:
    VSCORE(vscPrintDebugMsgToDebugLog)(messageString.c_str());

    if (!outputOnlyToLog)
    {
        IVsOutputWindowPane* spOutputWindowPane = getDebugPane();
        VSP_ASSERT(spOutputWindowPane != NULL);

        if (spOutputWindowPane != NULL)
        {
            HRESULT rc = spOutputWindowPane->Activate();
            bool isOk = (rc == S_OK);
            VSP_ASSERT(isOk);

            if (isOk)
            {
                std::wstring messageForOutputPane = messageString;
                // handle a case where the string begins with \n\n\n\n- we don't want to have many "CodeXL-" with no messege
                std::size_t found1 = messageString.find(VS_STR_NewLine);

                if (found1 == 0)
                {
                    for (;;)
                    {
                        std::size_t found2 = messageString.find(VS_STR_NewLine, found1 + 2);

                        if ((found2 != std::string::npos) && (found1 + 2) == found2)
                        {
                            found1 = found2;
                            continue;
                        }
                        else
                        {
                            break;
                        }
                    }

                    messageForOutputPane.insert(found1 + 1, VS_STR_CodeXLPrefix);
                }
                else
                {
                    messageForOutputPane.insert(0, VS_STR_CodeXLPrefix);
                }

                messageForOutputPane += VS_STR_NewLine;

                // Add the message to the debug pane:
                BSTR oleMessageString = SysAllocString(messageForOutputPane.c_str());
                spOutputWindowPane->OutputString(oleMessageString);
                SysFreeString(oleMessageString);

                // Reduce the reference count
                spOutputWindowPane->Release();
            }
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        vspPackageWrapper::clearMessageList
// Description: Clear debug pane
// Return Val:  void
// Author:      Gilad Yarnitzky
// Date:        21/2/2011
// ---------------------------------------------------------------------------
void vspPackageWrapper::clearMessagePane()
{
    IVsOutputWindowPane* spOutputWindowPane = getDebugPane();
    VSP_ASSERT(spOutputWindowPane != NULL);

    if (spOutputWindowPane != NULL)
    {
        spOutputWindowPane->Clear();

        // Reduce the reference count:
        spOutputWindowPane->Release();
    }
}

// ---------------------------------------------------------------------------
// Name:        vspPackageWrapper::informPackageOfNewDebugEngine
// Description: When a new debug engine is created, inform the package it exists:
// Author:      Uri Shomroni
// Date:        28/7/2013
// ---------------------------------------------------------------------------
void vspPackageWrapper::informPackageOfNewDebugEngine(void* pNewEngine)
{
    VSP_ASSERT(NULL != _pPackage);

    if (NULL != _pPackage)
    {
        VSP_ASSERT(NULL != pNewEngine);

        if (NULL != pNewEngine)
        {
            _pPackage->onNewDebugEngine(pNewEngine);
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        vspPackageWrapper::informPackageOfNewDebugEngine
// Description: When a debug engine is no longer used, inform the package it
//              doesn't exist:
// Author:      Uri Shomroni
// Date:        20/6/2014
// ---------------------------------------------------------------------------
void vspPackageWrapper::uninformPackageOfDebugEngine()
{
    VSP_ASSERT(NULL != _pPackage);

    if (NULL != _pPackage)
    {
        _pPackage->onNewDebugEngine(NULL);
    }
}

// ---------------------------------------------------------------------------
// Name:        vspPackageWrapper::getDebugPane
// Description:
// Return Val:  IVsOutputWindowPane*
// Author:      Gilad Yarnitzky
// Date:        21/2/2011
// ---------------------------------------------------------------------------
IVsOutputWindowPane* vspPackageWrapper::getDebugPane()
{
    IVsOutputWindowPane* pReturnWindowPane = NULL;

    // Use cached pane if it is ready:
    if (_pOutputWindowPane == NULL)
    {
        GUID debugOutputPaneGUID = guidDebugOutputPane;

        // Get the interface of the output pane:
        CodeXLVSPackage* pPackage = vspPackageWrapper::instance().getPackage();
        VSP_ASSERT(pPackage != NULL);

        if (pPackage != NULL)
        {
            IVsOutputWindow* spOutputWindow = NULL;
            HRESULT hr = pPackage->GetVsSiteCache().QueryService(SID_SVsOutputWindow, &spOutputWindow);

            if (spOutputWindow != NULL)
            {
                IVsOutputWindowPane* spOutputWindowPane = NULL;
                hr = spOutputWindow->GetPane(debugOutputPaneGUID, &spOutputWindowPane);

                if (hr == S_OK)
                {
                    // Cache the output pane:
                    _pOutputWindowPane = spOutputWindowPane;
                }
                else
                {
                    VSP_ASSERT(FALSE);
                }
            }

            spOutputWindow->Release();
        }
    }

    // Return the pane:
    pReturnWindowPane = _pOutputWindowPane;
    VSP_ASSERT(pReturnWindowPane != NULL);

    if (pReturnWindowPane != NULL)
    {
        pReturnWindowPane->AddRef();
    }

    return pReturnWindowPane;
}

// ---------------------------------------------------------------------------
IVsOutputWindowPane* vspPackageWrapper::getBuildPane()
{
    IVsOutputWindowPane* pReturnWindowPane = NULL;

    // Use cached pane if it is ready:
    if (_pOutputWindowPane == NULL)
    {
        GUID debugOutputPaneGUID = GUID_BuildOutputWindowPane;

        // Get the interface of the output pane:
        CodeXLVSPackage* pPackage = vspPackageWrapper::instance().getPackage();

        if (pPackage != NULL)
        {
            IVsOutputWindow* spOutputWindow = NULL;
            HRESULT hr = pPackage->GetVsSiteCache().QueryService(SID_SVsOutputWindow, &spOutputWindow);

            if (spOutputWindow != NULL)
            {
                IVsOutputWindowPane* spOutputWindowPane = NULL;
                hr = spOutputWindow->GetPane(debugOutputPaneGUID, &spOutputWindowPane);

                if (hr == S_OK)
                {
                    // Cache the output pane:
                    _pOutputWindowPane = spOutputWindowPane;
                }
                else
                {
                    VSP_ASSERT(FALSE);
                }

                spOutputWindow->Release();
            }

        }
    }

    // Return the pane:
    pReturnWindowPane = _pOutputWindowPane;

    if (pReturnWindowPane != NULL)
    {
        pReturnWindowPane->AddRef();
    }

    return pReturnWindowPane;
}

// ---------------------------------------------------------------------------
void vspPackageWrapper::clearBuildPane()
{
    IVsOutputWindowPane* spOutputWindowPane = getBuildPane();

    if (spOutputWindowPane != NULL)
    {
        spOutputWindowPane->Clear();

        // Reduce the reference count:
        spOutputWindowPane->Release();
    }
}

// ---------------------------------------------------------------------------
void vspPackageWrapper::outputBuildMessage(std::wstring& messageString, bool outputOnlyToLog, std::wstring filePathAndName, int line)
{
    // Add to log:
    VSCORE(vscPrintDebugMsgToDebugLog)(messageString.c_str());

    if (!outputOnlyToLog)
    {
        IVsOutputWindowPane* spOutputWindowPane = getBuildPane();

        if (spOutputWindowPane != NULL)
        {
            HRESULT rc;

            rc = spOutputWindowPane->Activate();
            VSP_ASSERT(rc == S_OK);

            vspDTEConnector::instance().ShowOutputWindow();
            // Add CodeXL prefix to the message (so that the message is identified in the output pane as CodeXL's message)

            // handle a case where the string begins with \n\n\n\n- we don't want to have many "CodeXL-" with no messege
            std::size_t found1 = messageString.find(VS_STR_NewLine);

            if (found1 == 0)
            {
                for (;;)
                {
                    std::size_t found2 = messageString.find(VS_STR_NewLine, found1 + 2);

                    if ((found2 != std::string::npos) && (found1 + 2) == found2)
                    {
                        found1 = found2;
                        continue;
                    }
                    else
                    {
                        break;
                    }
                }

                messageString.insert(found1 + 1, VS_STR_CodeXLPrefix);
            }
            else
            {
                messageString.insert(0, VS_STR_CodeXLPrefix);
            }

            //  when we have a message that's few lines, align lines with codeXL prefix size
            std::size_t found = messageString.find(VS_STR_NewLine);
            std::size_t lastPos = messageString.length() - 1;

            while (found > 0 && found < lastPos)
            {
                messageString.insert(found + 1, VS_STR_CodeXLPrefixIndentation);
                found = messageString.find(VS_STR_NewLine, found + 1);
                lastPos = messageString.length() - 1;
            }

            // verify message ends with new line
            if (lastPos < messageString.length() && (messageString.compare(lastPos, 1, VS_STR_NewLine) != 0))
            {
                messageString += VS_STR_NewLine;
            }

            // Add the message to the debug pane:
            BSTR oleMessageString = SysAllocString(messageString.c_str());

            if (!filePathAndName.empty())
            {
                // message is of the format PATH_TO_FILE\file.cl, line 123:message and will be connected to the actual file and line number
                // connect message to it's location: file and line
                HRESULT res = spOutputWindowPane->OutputTaskItemString(oleMessageString, TP_NORMAL, CAT_BUILDCOMPILE, L"", BMP_COMPILE, filePathAndName.c_str(), line, L"");
                VSP_ASSERT(res == S_OK);

                res = spOutputWindowPane->FlushToTaskList();
                VSP_ASSERT(res == S_OK);
            }
            else
            {
                spOutputWindowPane->OutputString(oleMessageString);
            }

            SysFreeString(oleMessageString);

            // Reduce the reference count
            spOutputWindowPane->Release();
        }
    }
}

