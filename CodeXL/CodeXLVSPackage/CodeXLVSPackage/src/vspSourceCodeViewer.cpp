//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file vspSourceCodeViewer.cpp
///
//==================================================================================

#include "stdafx.h"

// Infra:
#include <AMDTBaseTools/Include/gtASCIIString.h>
#include <AMDTBaseTools/Include/gtString.h>
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTOSWrappers/Include/osFile.h>
#include <AMDTAPIClasses/Include/apCLKernel.h>
#include <AMDTAPIClasses/Include/apCLProgram.h>
#include <AMDTAPIClasses/Include/apGLShaderObject.h>
#include <AMDTApiFunctions/Include/gaGRApiFunctions.h>
#include <AMDTApplicationFramework/Include/afAppStringConstants.h>
#include <AMDTGpuDebuggingComponents/Include/gdAidFunctions.h>
#include <AMDTGpuDebuggingComponents/Include/gdCommandIDs.h>
#include <AMDTGpuDebuggingComponents/Include/gdGDebuggerGlobalVariablesManager.h>
#include <AMDTGpuDebuggingComponents/Include/gdStringConstants.h>
#include <AMDTGpuDebuggingComponents/Include/gdDebugApplicationTreeHandler.h>

// Local:
#include <src/vspSourceCodeViewer.h>

// Static members initializations:
vspSourceCodeViewer* vspSourceCodeViewer::_pMySingleInstance = NULL;

// ---------------------------------------------------------------------------
// Name:        vspSourceCodeViewer::vspSourceCodeViewer
// Description: Constructor
// Author:      Sigal Algranaty
// Date:        11/10/2010
// ---------------------------------------------------------------------------
vspSourceCodeViewer::vspSourceCodeViewer() : m_pOwner(NULL)
{
}

// ---------------------------------------------------------------------------
// Name:        vspWindowsManager::~vspWindowsManager
// Description: Destructor
// Author:      Sigal Algranaty
// Date:        11/10/2010
// ---------------------------------------------------------------------------
vspSourceCodeViewer::~vspSourceCodeViewer()
{
    // Clear the windows vector:
    closeAllOpenedSourceWindows();
}

// ---------------------------------------------------------------------------
// Name:        vspDTEConnector::instance
// Description: Returns the single instance of this class. Creates it on
//              the first call to this function.
// Author:      Uri Shomroni
// Date:        10/3/2011
// ---------------------------------------------------------------------------
vspSourceCodeViewer& vspSourceCodeViewer::instance()
{
    if (_pMySingleInstance == NULL)
    {
        _pMySingleInstance = new vspSourceCodeViewer;

    }

    return *_pMySingleInstance;
}

// ---------------------------------------------------------------------------
// Name:        vspSourceCodeViewer::displayOpenCLKernelSourceCode
// Description: Displays an OpenCL program or kernel source code in Visual Studio
// Author:      Sigal Algranaty
// Date:        11/10/2010
// ---------------------------------------------------------------------------
void vspSourceCodeViewer::displayOpenCLProgramSourceCode(const afApplicationTreeItemData* pItemData)
{
    // Sanity check:
    GT_IF_WITH_ASSERT(pItemData != NULL)
    {
        gdDebugApplicationTreeData* pGDData = qobject_cast<gdDebugApplicationTreeData*>(pItemData->extendedItemData());
        GT_IF_WITH_ASSERT(pGDData != NULL)
        {
            int contextIndex = pGDData->_contextId._contextId;
            int progIndex = -1;
            gtString kernelFuncName;

            // Check with what object type were we called:
            afTreeItemType objType = pItemData->m_itemType;

            switch (objType)
            {
                case AF_TREE_ITEM_CL_KERNEL:
                {
                    // Get the OpenCL kernel details:
                    apCLKernel kernelDetails(OA_CL_NULL_HANDLE, -1, OA_CL_NULL_HANDLE, AF_STR_Empty);
                    bool rc = gaGetOpenCLKernelObjectDetails(contextIndex, pGDData->_clKernelHandle, kernelDetails);

                    if (rc)
                    {
                        // Get the kernel name:
                        kernelFuncName = kernelDetails.kernelFunctionName();

                        // The program index is the kernel's program index:
                        progIndex = kernelDetails.programIndex();
                    }
                }
                break;

                case AF_TREE_ITEM_CL_PROGRAM:
                {
                    // The program index is simply the object index:
                    progIndex = pGDData->_objectOpenCLIndex;
                }
                break;

                default:
                {
                    // We should not get here with other types:
                    GT_ASSERT(false);
                }
                break;
            }

            // If we have a program:
            if (progIndex > -1)
            {
                osFilePath sourceCodeFilePath;

                // Get the OpenCL program details:
                apCLProgram programDetails(OA_CL_NULL_HANDLE);
                bool rcProg = gaGetOpenCLProgramObjectDetails(contextIndex, progIndex, programDetails);
                GT_ASSERT(rcProg);

                // Get the program file path:
                sourceCodeFilePath = programDetails.sourceCodeFilePath();

                // Localize the path as needed:
                gaRemoteToLocalFile(sourceCodeFilePath, true);

                // If we found a file to open:
                if (sourceCodeFilePath.exists())
                {
                    // If we have a kernel function name, try and find it in the source:
                    int fileLineNumber = 0;

                    if (!kernelFuncName.isEmpty())
                    {
                        fileLineNumber = gdSearchForKernelDeclarationInSourceFile(sourceCodeFilePath, kernelFuncName);
                    }

                    // Open the file in Visual Studio. Select the line if this is a kernel (i.e.: we have a line number):
                    bool selectLine = (fileLineNumber > 0);
                    GT_IF_WITH_ASSERT(m_pOwner != NULL)
                    {
                        m_pOwner->scvOwnerOpenFileAtPosition(sourceCodeFilePath.asString().asCharArray(), fileLineNumber, selectLine, 0);
                    }
                }
            }
        }
    }
}


// ---------------------------------------------------------------------------
// Name:        vspSourceCodeViewer::displayOpenCLKernelSourceCode
// Description: Displays a GLSL sh program or kernel source code in Visual Studio
// Author:      Sigal Algranaty
// Date:        11/10/2010
// ---------------------------------------------------------------------------
void vspSourceCodeViewer::displayOpenGLSLShaderCode(const afApplicationTreeItemData* pItemData)
{
    // Sanity check:
    GT_IF_WITH_ASSERT(pItemData != NULL)
    {
        gdDebugApplicationTreeData* pGDData = qobject_cast<gdDebugApplicationTreeData*>(pItemData->extendedItemData());
        GT_IF_WITH_ASSERT(pGDData != NULL)
        {
            // Get the shader object details:
            gtAutoPtr<apGLShaderObject> aptrShaderDetails = NULL;
            bool rcGetShader = gaGetShaderObjectDetails(pGDData->_contextId._contextId, pGDData->_objectOpenGLName, aptrShaderDetails);
            GT_IF_WITH_ASSERT(rcGetShader)
            {
                // Get the shader file path:
                osFilePath sourceCodeFilePath;
                sourceCodeFilePath = aptrShaderDetails->sourceCodeFilePath();

                // Localize the path as needed:
                gaRemoteToLocalFile(sourceCodeFilePath, true);

                // If we found a file to open:
                if (sourceCodeFilePath.exists())
                {
                    int fileLineNumber = 0;

                    // Open the file in Visual Studio. Select the line if this is a kernel (i.e.: we have a line number):
                    bool selectLine = (fileLineNumber > 0);
                    GT_IF_WITH_ASSERT(m_pOwner != NULL)
                    {
                        m_pOwner->scvOwnerOpenFileAtPosition(sourceCodeFilePath.asString().asCharArray(), fileLineNumber, selectLine, 0);
                    }
                }
            }
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        vspSourceCodeViewer::addDocumentToControlledVector
// Description: Adds a document to our vector if it does not exist there yet.
// Author:      Uri Shomroni
// Date:        22/3/2011
// ---------------------------------------------------------------------------
void vspSourceCodeViewer::addDocumentToControlledVector(const osFilePath& documentPath)
{
    static const osFilePath tempDirectory(osFilePath::OS_TEMP_DIRECTORY);

    if (documentPath.asString().startsWith(tempDirectory.asString()))
    {
        // Go over the windows we already have in our vector:
        int numberOfDocuments = (int)_controlledDocuments.size();
        bool docFound = false;

        for (int i = 0; i < numberOfDocuments; i++)
        {
            // Check if this is our window:
            if (_controlledDocuments[i] == documentPath)
            {
                docFound = true;
                break;
            }
        }

        if (!docFound)
        {
            // We don't have this window, add it to our vector:
            _controlledDocuments.push_back(documentPath);
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        vspSourceCodeViewer::closeAllOpenedSourceWindows
// Description: Closes all the document windows that were opened by this class
// Author:      Uri Shomroni
// Date:        10/3/2011
// ---------------------------------------------------------------------------
void vspSourceCodeViewer::closeAllOpenedSourceWindows()
{
    // First open all the windows from the controlled documents vector to make sure we
    // have their window handles:
    int numberOfDocuments = (int)_controlledDocuments.size();

    GT_IF_WITH_ASSERT(m_pOwner != NULL)
    {
        for (int i = 0; i < numberOfDocuments; i++)
        {
            // Get the window:
            void* piCurrentWindow = NULL;
            m_pOwner->scvOwnerGetWindowFromFilePath(_controlledDocuments[i].asString().asCharArray(), piCurrentWindow);

            // Close the window and release the interface. This will also set the pointer
            // back to NULL:
            m_pOwner->scvOwnerCloseAndReleaseWindow(piCurrentWindow);
        }
    }
    // Clear the vector:
    _controlledDocuments.clear();
}

void vspSourceCodeViewer::setOwner(const IVscSourceCodeViewerOwner* pOwner)
{
    GT_ASSERT(pOwner != NULL);
    m_pOwner = pOwner;
}


