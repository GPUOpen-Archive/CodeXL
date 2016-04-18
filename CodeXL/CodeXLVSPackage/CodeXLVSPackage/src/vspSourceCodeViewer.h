//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file vspSourceCodeViewer.h
///
//==================================================================================

//------------------------------ vspSourceCodeViewer.h ------------------------------

#ifndef __VSPSOURCECODEVIEWER_H
#define __VSPSOURCECODEVIEWER_H

// Forward declarations:
class gtString;
class gdDebugApplicationTreeData;

// Infra:
#include <AMDTBaseTools/Include/gtVector.h>
#include <AMDTOSWrappers/Include/osFilePath.h>
#include <Include/Public/CoreInterfaces/IVscSourceCodeViewerOwner.h>
#include <AMDTApplicationFramework/Include/views/afApplicationTreeItemData.h>

// ----------------------------------------------------------------------------------
// Class Name:          vspSourceCodeViewer
// General Description: An object handling source code view in Visual Studio
// Author:              Sigal Algranaty
// Creation Date:       11/10/2010
// ----------------------------------------------------------------------------------
class vspSourceCodeViewer
{
public:
    vspSourceCodeViewer();
    virtual ~vspSourceCodeViewer();
    static vspSourceCodeViewer& instance();

    void displayOpenCLProgramSourceCode(const afApplicationTreeItemData* pItemData);
    void displayOpenGLSLShaderCode(const afApplicationTreeItemData* pItemData);
    void addDocumentToControlledVector(const osFilePath& documentPath);
    void closeAllOpenedSourceWindows();
    void setOwner(const IVscSourceCodeViewerOwner* pOwner);

private:
    friend class vspSingletonsDelete;

private:
    static vspSourceCodeViewer* _pMySingleInstance;
    gtVector<osFilePath> _controlledDocuments;
    const IVscSourceCodeViewerOwner* m_pOwner;
};


#endif //__VSPSOURCECODEVIEWER_H

