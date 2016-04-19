//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gdSourceCodeWindow.cpp
///
//==================================================================================

//------------------------------ gdSourceCodeWindow.cpp ------------------------------

// Ignore compiler warnings:
#include <AMDTBaseTools/Include/gtIgnoreCompilerWarnings.h>

// For compilers that support precompilation, includes <wx/wx.h>.
#include <wx/wxprec.h>

// for all others, include the necessary headers (this file is usually all you
// need because it includes almost all 'standard' wxWidgets headers)
#ifndef WX_PRECOMP
    #include <wx/wx.h>
#endif

// wxWidgets headers
#include <wx/window.h>   // for wxWindow
#include <wx/config.h>   // configuration support
#include <wx/filedlg.h>  // file dialog support
#include <wx/filename.h> // filename support
#include <wx/notebook.h> // notebook support
#include <wx/settings.h> // system settings
#include <wx/string.h>   // strings support
#include <wx/image.h>    // images support

// Infra
#include <AMDTBaseTools/Include/gtString.h>
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTBaseTools/Include/gtStringTokenizer.h>
#include <AMDTOSWrappers/Include/osOSDefinitions.h>
#include <AMDTOSWrappers/Include/osDirectory.h>

// AMDTApplicationFramework:
#include <AMDTApplicationFramework/Include/afAppStringConstants.h>

// Local:
#include <AMDTGpuDebuggingComponents/Include/gdStringConstants.h>
#include <AMDTGpuDebuggingComponents/Include/gdCodeXLGlobalVariablesManager.h>
#include <AMDTGpuDebuggingComponents/Include/views/gdSourceCodeWindowDefsext.h>     // Additional definitions
#include <AMDTGpuDebuggingComponents/Include/views/gdSourceCodeWindowTextCtrl.h>    // gdSourceCodeWindowTextCtrl module
#include <AMDTGpuDebuggingComponents/Include/views/gdSourceCodeWindowTextPref.h>        // Prefs
#include <AMDTGpuDebuggingComponents/Include/views/gdSourceCodeWindow.h>
#include <AMDTGpuDebuggingComponents/Include/gdCommandIDs.h>



// ----- global variables -----
// The use of global variables ensures that these variables data is kept
// during the session and among class instances.

//! global print data, to remember settings during the session
wxPrintData* g_printData = (wxPrintData*) NULL;
wxPageSetupData* g_pageSetupData = (wxPageSetupData*) NULL;


//----------------------------------------------------------------------------
// wxWindows events table
//----------------------------------------------------------------------------
BEGIN_EVENT_TABLE(gdSourceCodeWindow, wxWindow)
    EVT_SIZE(gdSourceCodeWindow::onSizeChanged)
END_EVENT_TABLE()


gdSourceCodeWindow::gdSourceCodeWindow(wxWindow* parent, wxWindowID id,
                                       const wxPoint& pos, const wxSize& size,
                                       long style, const wxString& name)
    : wxWindow(parent, id, pos, size, style, name),
      m_sourceCodeViewerTextCtrl(NULL)
{
    // Create the text control:
    m_sourceCodeViewerTextCtrl = new gdSourceCodeWindowTextCtrl(this, ID_SHADERS_SOURCE_CODE_VIEW_SCINTILLA_EDITOR);
    GT_ASSERT_ALLOCATION(m_sourceCodeViewerTextCtrl);

    // Set the background to be white:
    SetBackgroundColour(_T("WHITE"));

    // initialize print data
    if (g_printData == NULL)
    {
        g_printData = new wxPrintData;
        GT_ASSERT_ALLOCATION(g_printData);
    }

    // Initialize the page setup data:
    if (g_pageSetupData == NULL)
    {
        g_pageSetupData = new wxPageSetupDialogData;
        GT_ASSERT_ALLOCATION(g_pageSetupData);
    }
}

gdSourceCodeWindow::~gdSourceCodeWindow()
{
    // Delete the page setup data:
    if (g_pageSetupData != NULL)
    {
        delete g_pageSetupData;
        g_pageSetupData = NULL;
    }

    if (g_printData != NULL)
    {
        delete g_printData;
        g_printData = NULL;
    }
}


// ---------------------------------------------------------------------------
// Name:        gdSourceCodeWindow::clearSourceCodeData
// Description: Clears the content of this window.
// Author:      Yaki Tebeka
// Date:        28/4/2005
// ---------------------------------------------------------------------------
void gdSourceCodeWindow::clearSourceCodeData()
{
    m_sourceCodeViewerTextCtrl->SetFilename(wxEmptyString);
    m_sourceCodeViewerTextCtrl->ClearAll();
    m_sourceCodeViewerTextCtrl->SetSavePoint();
}


// ---------------------------------------------------------------------------
// Name:        gdSourceCodeWindow::openFileAtLine
// Description: Open the file at a specific line.
// Arguments:  filePath - The path of the file to be opened.
//             foundFilePath - The path where the file was found.
//             lineNumber - The line number to which the source code window
//                          will be scrolled.
// Return Val:  bool - Success / failure.
// Author:      Avi Shapira
// Date:        14/10/2004
// ---------------------------------------------------------------------------
bool gdSourceCodeWindow::openFileAtLine(const osFilePath& filePath, osFilePath& foundFilePath, int lineNumber)
{
    bool retVal = false;

    osFilePath filePathToBeLoaded = filePath;
    bool isNewFile;
    bool fileExistsOnDisk = false;

    // If the path that we got is not empty:
    if (!(filePathToBeLoaded.asString().isEmpty()))
    {
        // Make sure that the file exists on disk:
        retVal = ensureFileExistsOnDisk(filePathToBeLoaded, fileExistsOnDisk, isNewFile);

        // If the file exists on disk:
        if (fileExistsOnDisk)
        {
            // If we need to open the file:
            if (isNewFile)
            {
                // Open the file:
                retVal = loadFile(filePathToBeLoaded.asString().asCharArray());
            }

            if (retVal)
            {
                // Log the path where we found the file:
                foundFilePath = filePathToBeLoaded;

                // Add the file name and path to the hash table:
                gtString fileNameForHash = AF_STR_Empty;
                osDirectory filePathForHash;
                foundFilePath.getFileDirectory(filePathForHash);
                foundFilePath.getFileName(fileNameForHash);
                fileNameForHash.toLowerCase();
                _sourceCodeFileNameToPath[fileNameForHash] = filePathForHash.directoryPath().asString();

                // Remove the previous marker
                m_sourceCodeViewerTextCtrl->MarkerDeleteAll(1);

                // Set a marker at the input line
                m_sourceCodeViewerTextCtrl->MarkerDefine(1, wxSTC_MARK_ARROW, wxNullColour, "yellow");

                // We are adding the marker to line-1 because it seems that the marker is adding it to line +1
                int markLine = 0;

                if (lineNumber > 0)
                {
                    markLine = lineNumber - 1;
                }

                m_sourceCodeViewerTextCtrl->MarkerAdd(markLine, 1);

                // Get the number of visible lines
                int linesOnScreen = m_sourceCodeViewerTextCtrl->LinesOnScreen();
                int scrollToLine = markLine - (linesOnScreen / 2);

                // Scroll to the line (put the line in the middle of the screen)
                m_sourceCodeViewerTextCtrl->ScrollToLine(scrollToLine);
            }
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gdSourceCodeWindow::openFile
// Description: Opens an input source code file.
// Arguments:   filePath - The input source code file path.
// Return Val:  bool - Success / failure.
// Author:      Yaki Tebeka
// Date:        28/4/2005
// ---------------------------------------------------------------------------
bool gdSourceCodeWindow::openFile(const osFilePath& filePath)
{
    bool retVal = false;

    retVal = loadFile(filePath.asString().asCharArray());

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gdSourceCodeWindow::openedFilePath
// Description: Returns the currently opened file path.
// Author:      Yaki Tebeka
// Date:        28/4/2005
// ---------------------------------------------------------------------------
void gdSourceCodeWindow::openedFilePath(osFilePath& filePath)
{
    gtString filePathAsString(m_sourceCodeViewerTextCtrl->GetFilename().GetData());
    filePath = filePathAsString;
}


// ---------------------------------------------------------------------------
// Name:        gdSourceCodeWindow::isSourceCodeModified
// Description: Returns true iff the contained source code is modified.
// Author:      Yaki Tebeka
// Date:        28/4/2005
// ---------------------------------------------------------------------------
bool gdSourceCodeWindow::isSourceCodeModified() const
{
    bool retVal = m_sourceCodeViewerTextCtrl->Modified();
    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gdSourceCodeWindow::saveSourceCodeToFile
// Description: Saves the contained source code into a given file.
// Return Val:  bool - Success / failure.
// Author:      Yaki Tebeka
// Date:        28/4/2005
// ---------------------------------------------------------------------------
bool gdSourceCodeWindow::saveSourceCodeToFile(const wxString& filename)
{
    bool retVal = m_sourceCodeViewerTextCtrl->SaveFile(filename);
    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gdSourceCodeWindow::saveSourceCodeToItsOriginFile
// Description: Saves the contained source code into its origin file.
// Return Val:  bool - Success / failure.
// Author:      Yaki Tebeka
// Date:        28/4/2005
// ---------------------------------------------------------------------------
bool gdSourceCodeWindow::saveSourceCodeToItsOriginFile()
{
    bool retVal = m_sourceCodeViewerTextCtrl->SaveFile();
    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gdSourceCodeWindow::openSourceCodePropertiesDialog
// Description: Opens the source code properties dialog.
// Author:      Yaki Tebeka
// Date:        28/4/2005
// ---------------------------------------------------------------------------
void gdSourceCodeWindow::openSourceCodePropertiesDialog()
{
    gdSourceCodeWindowTextCtrlProperties(m_sourceCodeViewerTextCtrl, 0);
}

// ---------------------------------------------------------------------------
// Name:        gdSourceCodeWindow::onSizeChanged
// Description: Is called when the window size is changed
// Author:      Yaki Tebeka
// Date:        28/4/2005
// ---------------------------------------------------------------------------
void gdSourceCodeWindow::onSizeChanged(wxSizeEvent& WXUNUSED(event))
{
    // Get my client size:
    wxSize clientSize = GetClientSize();

    // Set the source code window to have this size:
    if (m_sourceCodeViewerTextCtrl)
    {
        m_sourceCodeViewerTextCtrl->SetSize(clientSize);
    }
}


// ---------------------------------------------------------------------------
// Name:        gdSourceCodeWindow::onScintillaTextCtrlEvent
// Description: Handles Scintilla text control native events.
// Arguments:   event - The native Scintilla text control event.
// Author:      Yaki Tebeka
// Date:        28/4/2005
// ---------------------------------------------------------------------------
void gdSourceCodeWindow::onScintillaTextCtrlEvent(wxCommandEvent& event)
{
    if (m_sourceCodeViewerTextCtrl)
    {
        // Pass the event to the Scintilla text control for processing:
        m_sourceCodeViewerTextCtrl->ProcessWindowEvent(event);
    }
}


// ---------------------------------------------------------------------------
// Name:        gdSourceCodeWindow::onScintillaTextCtrlUIEvent
// Description: Enables passing GUI enabler events to the Scintilla text control.
// Arguments:   event - A class that contains thee input event, and output GUI
//                      element status.
// Author:      Yaki Tebeka
// Date:        28/4/2005
// ---------------------------------------------------------------------------
void gdSourceCodeWindow::onScintillaTextCtrlUIEvent(wxUpdateUIEvent& event)
{
    if (m_sourceCodeViewerTextCtrl)
    {
        // Pass the event to the Scintilla text control for processing:
        // (It will call event.Enable(X) with the appropriate value)
        m_sourceCodeViewerTextCtrl->ProcessWindowEvent(event);
    }
    else
    {
        // Disable the GUI element.
        event.Enable(false);
    }
}


bool gdSourceCodeWindow::loadFile(wxString fname)
{
    bool retVal = false;

    wxFileName w(fname);
    w.Normalize();
    fname = w.GetFullPath();

    retVal = m_sourceCodeViewerTextCtrl->LoadFile(fname);

    return retVal;
}

wxRect gdSourceCodeWindow::determinePrintSize()
{

    wxSize scr = wxGetDisplaySize();

    // determine position and size (shifting 16 left and down)
    wxRect rect = GetRect();
    rect.x += 16;
    rect.y += 16;
    rect.width = wxMin(rect.width, (scr.x - rect.x));
    rect.height = wxMin(rect.height, (scr.x - rect.y));

    return rect;
}


// ---------------------------------------------------------------------------
// Name:        gdSourceCodeWindow::getFileStoredPath
// Description: Inputs a file name and returns its stored path (if exists).
// Arguments:   fileName - The input file name
//              storedFilePath - Will get the stored file path (if exists).
// Return Val:  bool - Success / failure.
// Author:      Yaki Tebeka
// Date:        28/4/2005
// ---------------------------------------------------------------------------
bool gdSourceCodeWindow::getFileStoredPath(const gtString& fileName, osFilePath& storedFilePath) const
{
    bool retVal = false;

    // The hash holds lower case file name. Use a lower case search string:
    gtString searchStr = fileName;
    searchStr.toLowerCase();

    gtMap<gtString, gtString>::const_iterator iter = _sourceCodeFileNameToPath.find(searchStr);

    if (iter != _sourceCodeFileNameToPath.end())
    {
        storedFilePath = iter->second;
        retVal = true;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gdSourceCodeWindow::deleteAllMarkers
// Description: Delete all the markers in the source code viewer.
// Author:      Avi Shapira
// Date:        19/6/2005
// ---------------------------------------------------------------------------
void gdSourceCodeWindow::deleteAllMarkers()
{
    // Delete all markers
    m_sourceCodeViewerTextCtrl->MarkerDeleteAll(1);
}

// ---------------------------------------------------------------------------
// Name:        gdSourceCodeWindow::setText
// Description: Set editor text
// Arguments: const gtString& text
// Return Val: void
// Author:      Sigal Algranaty
// Date:        17/11/2009
// ---------------------------------------------------------------------------
void gdSourceCodeWindow::setText(const gtString& text)
{
    m_sourceCodeViewerTextCtrl->SetText(text.asCharArray());
}



// ---------------------------------------------------------------------------
// Name:        gdSourceCodeWindow::markOpenCLKernel
// Description: Mark an OpenCL kernel within the source and scroll to the kernel
//              source location
// Arguments: kernelName - The kernel's name.
// Author:      Sigal Algranaty
// Date:        22/11/2009
// ---------------------------------------------------------------------------
void gdSourceCodeWindow::markOpenCLKernel(const gtString& kernelName)
{
    // Remove the previous marker (if exists):
    m_sourceCodeViewerTextCtrl->MarkerDeleteAll(1);

    m_sourceCodeViewerTextCtrl->SetTargetStart(0);
    m_sourceCodeViewerTextCtrl->SetTargetEnd(0);

    int kernelNamePos = -1;

    // Get the text length:
    int textLength = m_sourceCodeViewerTextCtrl->GetTextLength();

    // Search for the kernels within the file, and for each, check if this is the requested one:
    int findPos = 0;
    findPos = m_sourceCodeViewerTextCtrl->FindText(findPos, textLength, AF_STR_CLKernelDeclaration, SCV_FR_DOWN);

    while (findPos >= 0)
    {
        // Search for the first "(" after the kernel declaration:
        int parenthesesPos = m_sourceCodeViewerTextCtrl->FindText(findPos, textLength, "(", SCV_FR_DOWN);

        if (parenthesesPos >= 0)
        {
            // Search for the kernel name between the "__kernel" declaration and the first "(":
            kernelNamePos = m_sourceCodeViewerTextCtrl->FindText(findPos, parenthesesPos, kernelName.asCharArray(), SCV_FR_DOWN);

            if (kernelNamePos >= 0)
            {
                break;
            }

            // Keep looking:
            findPos = m_sourceCodeViewerTextCtrl->FindText(parenthesesPos, textLength, AF_STR_CLKernelDeclaration, SCV_FR_DOWN);
        }
        else
        {
            // No more kernels:
            break;
        }
    }

    // If the kernel was found:
    if (kernelNamePos >= 0)
    {
        // m_sourceCodeViewerTextCtrl->GotoPos(kernelPosition);

        // Set a marker at the kernel's line
        int kernelLine = m_sourceCodeViewerTextCtrl->LineFromPosition(kernelNamePos);
        m_sourceCodeViewerTextCtrl->MarkerDefine(1, wxSTC_MARK_ARROW, wxNullColour, "yellow");
        m_sourceCodeViewerTextCtrl->MarkerAdd(kernelLine, 1);
        // m_sourceCodeViewerTextCtrl->MarkerDefine(1, wxSTC_MARK_ARROW, wxNullColour, "yellow");

        // Get the number of visible lines:
        int linesOnScreen = m_sourceCodeViewerTextCtrl->LinesOnScreen();
        int scrollToLine = kernelLine - (linesOnScreen / 2);

        // Scroll to the line (put the line in the middle of the screen):
        m_sourceCodeViewerTextCtrl->ScrollToLine(scrollToLine);
    }
}



// ---------------------------------------------------------------------------
// Name:        gdSourceCodeWindow::ensureFileExistsOnDisk
// Description: Ensuring a source code file exist on disk, and storing the file
//              real path
//              NOTICE: the code was part of openFileAtLine - I am just separating
//              into functions
// Arguments:   filePathToBeLoaded - the file to be loaded
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        13/1/2011
// ---------------------------------------------------------------------------
bool gdSourceCodeWindow::ensureFileExistsOnDisk(osFilePath& filePathToBeLoaded, bool& fileExistsOnDisk, bool& isNewFile)
{
    bool retVal = false;
    // Verify that we don't load again the already loaded file:
    gtString filePathToBeLoadedString = filePathToBeLoaded.asString();
    gtString openedFilePathString(m_sourceCodeViewerTextCtrl->GetFilename().GetData());
    filePathToBeLoadedString.toLowerCase();
    openedFilePathString.toLowerCase();

    // If the requested file is already loaded:
    if (filePathToBeLoadedString == openedFilePathString)
    {
        isNewFile = false;
        fileExistsOnDisk = true;
        retVal = true;
    }
    else
    {
        // If the file exist on disk:
        if (filePathToBeLoaded.isRegularFile())
        {
            fileExistsOnDisk = true;
        }
        else
        {
            // Get the file name:
            gtString fileName;
            filePathToBeLoaded.getFileName(fileName);

            // Look for a stored file path:

            // If the file path is already stored in this class:
            osFilePath storedFilePath;
            bool storedFilePathExists = getFileStoredPath(fileName, storedFilePath);

            if (storedFilePathExists)
            {
                // Set the file path to the stored file path:
                filePathToBeLoaded.setFileDirectory(storedFilePath);
                fileExistsOnDisk = true;
            }
            else
            {
                // Look in the Additional files directories:
                // ----------------------------------------

                // Get the Additional directory:
                gdCodeXLGlobalVariablesManager& globalVarsManager = gdCodeXLGlobalVariablesManager::instance();
                bool searchInAdditionalDirectories = globalVarsManager.searchInAdditionalDirectoriesBool();

                if (searchInAdditionalDirectories)
                {
                    gtString additionalDirectories = globalVarsManager.additionalSourceCodeDirectories();
                    osFilePath newFilePath;

                    // The Additional Directories are ';' separated:
                    gtStringTokenizer strTokenizer(additionalDirectories, L";");
                    gtString currentAdditionalDirectory;

                    // Iterate over the Additional Directories strings to find the source file:
                    while (strTokenizer.getNextToken(currentAdditionalDirectory))
                    {
                        newFilePath = filePathToBeLoaded;
                        newFilePath.setFileDirectory(currentAdditionalDirectory);

                        if (newFilePath.isRegularFile())
                        {
                            filePathToBeLoaded = newFilePath;
                            fileExistsOnDisk = true;
                            break;
                        }
                    }
                }

                if (!fileExistsOnDisk)
                {
                    // Check with a new source code root directory
                    // -------------------------------------------
                    // Get the Additional directory:
                    gdCodeXLGlobalVariablesManager& globalVarsManager = gdCodeXLGlobalVariablesManager::instance();
                    bool  sourceCodeRootDirBool = globalVarsManager.setSourceCodeRootLocationBool();

                    if (sourceCodeRootDirBool)
                    {
                        // Set the source code root dir into the var
                        gtString sourceCodeRootDir = globalVarsManager.sourceCodeRootLocation();

                        // Append the file path to the source code root dir
                        sourceCodeRootDir.append(filePathToBeLoaded.asString());

                        // Set the new file path into the appropriate var format
                        osFilePath newFilePath;
                        newFilePath = sourceCodeRootDir;

                        if (newFilePath.isRegularFile())
                        {
                            filePathToBeLoaded = newFilePath;
                            fileExistsOnDisk = true;
                        }
                    }
                }
            }
        }
    }

    return retVal;
}

