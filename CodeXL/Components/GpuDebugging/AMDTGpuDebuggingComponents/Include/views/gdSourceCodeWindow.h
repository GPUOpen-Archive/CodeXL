//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gdSourceCodeWindow.h
///
//==================================================================================

//------------------------------ gdSourceCodeWindow.h ------------------------------

#ifndef __GDSOURCECODEWINDOW
#define __GDSOURCECODEWINDOW


// Infra:
#include <AMDTOSWrappers/Include/osFilePath.h>
#include <AMDTBaseTools/Include/gtMap.h>

// Local:
#include <AMDTGpuDebuggingComponents/Include/views/gdSourceCodeWindowTextCtrl.h>


// ----------------------------------------------------------------------------------
// Class Name:           gdSourceCodeWindow: public wxWindow
// General Description:
//  A window that displays source code in a styled text manner.
//
// Author:               Avi Shapira
// Creation Date:        20/10/2004
// ----------------------------------------------------------------------------------
class GD_API gdSourceCodeWindow: public wxWindow
{
public:
    gdSourceCodeWindow(wxWindow* parent, wxWindowID id, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = 0, const wxString& name = wxPanelNameStr);
    ~gdSourceCodeWindow();

    void clearSourceCodeData();
    bool openFileAtLine(const osFilePath& filePath, osFilePath& foundFilePath, int lineNumber);
    bool openFile(const osFilePath& filePath);
    void openedFilePath(osFilePath& filePath);
    bool isSourceCodeModified() const;
    bool saveSourceCodeToFile(const wxString& filename);
    bool saveSourceCodeToItsOriginFile();
    void setText(const gtString& text);

    // Open dialog functions:
    void openSourceCodePropertiesDialog();

    // Remove the markers
    void deleteAllMarkers();

    // wxWindows events:
    void onSizeChanged(wxSizeEvent& event);

    // Scintilla native events handlers:
    void onScintillaTextCtrlEvent(wxCommandEvent& event);
    void onScintillaTextCtrlUIEvent(wxUpdateUIEvent& event);

    void markOpenCLKernel(const gtString& kernelName);

private:
    bool loadFile(wxString fname);
    bool ensureFileExistsOnDisk(osFilePath& filePathToBeLoaded, bool& fileExistsOnDisk, bool& isNewFile);
    wxRect determinePrintSize();
    bool getFileStoredPath(const gtString& fileName, osFilePath& storedFilePath) const;


private:
    // Do not allow the use of my default constructor:
    gdSourceCodeWindow();

    // sourceCodeViewerTextCtrl object
    gdSourceCodeWindowTextCtrl* m_sourceCodeViewerTextCtrl;

    // Maps source code file name to source code file path:
    gtMap<gtString, gtString> _sourceCodeFileNameToPath;

    DECLARE_EVENT_TABLE()
};


#endif  // __GDSOURCECODEWINDOW
