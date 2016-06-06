//=====================================================================
// Copyright 2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file osDebuggingFunctions.cpp
///
//=====================================================================

//------------------------------ osDebuggingFunctions.cpp ------------------------------

// Windows:
#define WIN32_LEAN_AND_MEAN 1
#include <Windows.h>
#include <atlbase.h>

// Import EnvDTE (Dev Studio 8 Automation):
#pragma warning(disable : 4278)
#pragma warning(disable : 4146)
#import "libid:80cc9f66-e7d8-4ddd-85b6-d9e6cd0e93e2" version("8.0") lcid("0") raw_interfaces_only named_guids
#pragma warning(default : 4146)
#pragma warning(default : 4278)

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>

// Local:
#include <AMDTOSWrappers/Include/osDebugLog.h>
#include <AMDTOSWrappers/Include/osDebuggingFunctions.h>


// ---------------------------------------------------------------------------
// Name:        osOutputDebugString
// Description:
//   a. If the calling process is running under a debugger, sends the input debug string
//      to the debugger. Debuggers will usually display this string.
//   b. Writes the debug string into the debug log file.
//
// Arguments:   debugString - The string to be sent to the debugger.
// Return Val:  bool - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        5/10/2004
// ---------------------------------------------------------------------------
void osOutputDebugString(const gtString& debugString)
{
    // Send a debug string event to my debugger:
    OutputDebugString(debugString.asCharArray());

    // Output log message
    gtString logMessage = OS_STR_DebugStringOutputPrefix;
    logMessage += debugString;

    osDebugLog& theDebugLog = osDebugLog::instance();
    theDebugLog.addPrintout(_T(__FUNCTION__), _T(__FILE__) , __LINE__ , logMessage.asCharArray() , OS_DEBUG_LOG_INFO);
}


// ---------------------------------------------------------------------------
// Name:        osWPerror
// Description:
//   Writes an error message to the console.
//
// Arguments:   pErrorMessage - The message to be printed
// Author:      AMD Developer Tools Team
// Date:        19/6/2011
// ---------------------------------------------------------------------------
void osWPerror(const wchar_t* pErrorMessage)
{
    if (pErrorMessage != NULL)
    {
        _wperror(pErrorMessage);
    }
}


// ---------------------------------------------------------------------------
// Name:        osThrowBreakpointException
// Description: Throws a breakpoint exception.
// Author:      AMD Developer Tools Team
// Date:        5/10/2004
// ---------------------------------------------------------------------------
void osThrowBreakpointException()
{
    // Throw a breakpoint exception:
    DebugBreak();
}


// ---------------------------------------------------------------------------
// Name:        osIsRunningUnderDebugger
// Description: Returns true iff the calling process runs under a debugger.
// Author:      AMD Developer Tools Team
// Date:        5/10/2004
// ---------------------------------------------------------------------------
bool osIsRunningUnderDebugger()
{
    bool retVal = false;

    // Get a handle to the kernel32.dll:
    HINSTANCE  hInstKernelr32 = LoadLibrary(L"kernel32.dll");

    if (hInstKernelr32)
    {
        // Define the IsDebuggerPresent function type:
        typedef BOOL (CALLBACK * IsDebuggerPresentPROC)(void);

        // Get the address of the IsDebuggerPresent function
        IsDebuggerPresentPROC IsDebuggerPresent = (IsDebuggerPresentPROC)GetProcAddress(hInstKernelr32, "IsDebuggerPresent");

        if (IsDebuggerPresent != NULL)
        {
            // Check if we are running under a debugger:
            BOOL isUnderDebugger = IsDebuggerPresent();
            retVal = (isUnderDebugger != 0);
        }

        // Clean up:
        FreeLibrary(hInstKernelr32);
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        osOpenFileInSourceCodeEditor
// Description: Opens the input source code location in an editor.
// Arguments: filePath - The path of the file to be opened.
//            lineNumber - The line, in the above file that the editor should display.
// Return Val: bool  - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        4/2/2009
// ---------------------------------------------------------------------------
bool osOpenFileInSourceCodeEditor(const osFilePath& filePath, int lineNumber)
{
    bool retVal = false;

    // Get the Visual Studio Automation Com class id:
    CLSID clsid;
    HRESULT result = ::CLSIDFromProgID(L"VisualStudio.DTE", &clsid);

    if (SUCCEEDED(result))
    {
        // Get the active object's IUnknown interface:
        CComPtr<IUnknown> iActiveObjectAsIUnknown;
        result = ::GetActiveObject(clsid, NULL, &iActiveObjectAsIUnknown);

        if (SUCCEEDED(result))
        {
            // Get it's _DTE interface:
            CComPtr<EnvDTE::_DTE> iDTE;
            iDTE = iActiveObjectAsIUnknown;

            // Get it's ItemOperations object interface:
            CComPtr<EnvDTE::ItemOperations> iItemOperations;
            result = iDTE->get_ItemOperations(&iItemOperations);

            if (SUCCEEDED(result))
            {
                // Open the input file in the Dev Studio IDE:
                CComBSTR bstrFileName(filePath.asString().asCharArray());
                CComBSTR bstrKind(EnvDTE::vsViewKindTextView);
                CComPtr<EnvDTE::Window> iWindow;
                result = iItemOperations->OpenFile(bstrFileName, bstrKind, &iWindow);

                if (SUCCEEDED(result))
                {
                    // Get DevStudio's active document (which should be the document that displays the input file):
                    CComPtr<EnvDTE::Document> iActiveDocument;
                    result = iDTE->get_ActiveDocument(&iActiveDocument);

                    if (SUCCEEDED(result))
                    {
                        // Get the document's iTextSelection, as an IDispatch interface:
                        CComPtr<IDispatch> iSelectionAsIDispatch;
                        result = iActiveDocument->get_Selection(&iSelectionAsIDispatch);

                        if (SUCCEEDED(result))
                        {
                            // Convert the IDispatch interface into a EnvDTE::TextSelection interface:
                            CComPtr<EnvDTE::TextSelection> iTextSelection;
                            result = iSelectionAsIDispatch->QueryInterface(&iTextSelection);

                            if (SUCCEEDED(result))
                            {
                                // Select the input line:
                                result = iTextSelection->GotoLine(lineNumber, TRUE);

                                if (SUCCEEDED(result))
                                {
                                    // Get DevStudio's main window:
                                    CComPtr<EnvDTE::Window> iMainWindow;
                                    result = iDTE->get_MainWindow(&iMainWindow);

                                    if (SUCCEEDED(result))
                                    {
                                        // Make it visible:
                                        iMainWindow->Activate();

                                        retVal = true;

                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    return retVal;
}

