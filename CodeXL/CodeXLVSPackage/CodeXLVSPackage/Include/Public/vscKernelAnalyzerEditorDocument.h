//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file vscKernelAnalyzerEditorDocument.h
///
//==================================================================================

#ifndef vscKernelAnalyzerEditorDocument_h__
#define vscKernelAnalyzerEditorDocument_h__

// Local:
#include <Include/Public/vscEditorDocument.h>
#include "src\CodeXLVSPackageCoreDefs.h"

// Infra:
#include <AMDTOSWrappers/Include/osFilePath.h>

class vspQTWindowPaneImpl;
class QWidget;

/// ----------------------------------------------------------------------------------
/// Class Name:    vscKernelAnalyzerEditorDocument
///                This class is used to implement the core side of KA MDI windows created by CodeXL VS package.
///                The class is implementing Kernel Analyzer specific MDI needs. The functions will be called when the VS
///                extension will call vspEditorDocument functionality, which will call the appropriate vsc_ function, which
///                will call this class
/// ----------------------------------------------------------------------------------
class vscKernelAnalyzerEditorDocument : public vscEditorDocument
{
public:
    vscKernelAnalyzerEditorDocument();
    virtual ~vscKernelAnalyzerEditorDocument();

    virtual void ClosePane();
    virtual void OnShow();
    virtual void SetEditorCaption(const wchar_t* filePathStr, wchar_t*& itemNameStrBuffer);

protected:

    /// This function is called when the content of the window is created. It should be implemented by the specific classes:
    virtual QWidget* CreateView();
};

void* vscKernelAnalyzerEditorDocument_CreateInstance();

#endif // vscKernelAnalyzerEditorDocument_h__