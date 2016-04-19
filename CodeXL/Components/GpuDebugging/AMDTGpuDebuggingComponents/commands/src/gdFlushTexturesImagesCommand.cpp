//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gdFlushTexturesImagesCommand.cpp
///
//==================================================================================

//------------------------------ gdFlushTexturesImagesCommand.cpp ------------------------------

// Qt:
#include <QtWidgets>

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTAPIClasses/Include/apAPIConnectionType.h>
#include <AMDTApiFunctions/Include/gaGRApiFunctions.h>

// AMDTApplicationFramework:
#include <AMDTApplicationFramework/Include/afMainAppWindow.h>
#include <AMDTApplicationFramework/Include/afProgressBarWrapper.h>

// Local:
#include <AMDTGpuDebuggingComponents/Include/views/gdImagesAndBuffersExporter.h>
#include <AMDTGpuDebuggingComponents/Include/gdGDebuggerGlobalVariablesManager.h>
#include <AMDTGpuDebuggingComponents/Include/commands/gdFlushTexturesImagesCommand.h>
#include <AMDTGpuDebuggingComponents/Include/gdStringConstants.h>

// ---------------------------------------------------------------------------
// Name:        gdFlushTexturesImagesCommand::gdFlushTexturesImagesCommand
// Description: Constructor
// Arguments:   pParent - The double slider progress bar parent
// Author:      Eran Zinman
// Date:        31/1/2008
// ---------------------------------------------------------------------------
gdFlushTexturesImagesCommand::gdFlushTexturesImagesCommand(QWidget* pParent)
    : m_pMyParent(pParent), m_amountOfTexturesAlreadyExported(0)
{

}

// ---------------------------------------------------------------------------
// Name:        gdFlushTexturesImagesCommand::~gdFlushTexturesImagesCommand
// Description: Destructor.
// Author:      Eran Zinman
// Date:        31/1/2008
// ---------------------------------------------------------------------------
gdFlushTexturesImagesCommand::~gdFlushTexturesImagesCommand()
{

}

// ---------------------------------------------------------------------------
// Name:        gdFlushTexturesImagesCommand::canExecuteSpecificCommand
// Description: Answers the question - can we stop debugging the currently
//              debugged process.
// Author:      Eran Zinman
// Date:        31/1/2008
// Implementation Notes:
//   We can stop debugging iff there is currently a debugged process and
//   openGL recording was set to "ON".
// ---------------------------------------------------------------------------
bool gdFlushTexturesImagesCommand::canExecuteSpecificCommand()
{
    bool retVal = false;

    // Check if there is already a debugged process and it's suspended:
    bool debuggedProcessSuspended = gaIsDebuggedProcessSuspended();

    // Check if we need to flush all textures since openGL log was recorded
    bool wasRecorded = gaWasOpenGLDataRecordedInDebugSession();

    // Check if texture images recording is enabled
    bool recordingEnabled = false;
    bool rc1 = gaIsImagesDataLogged(recordingEnabled);
    GT_IF_WITH_ASSERT(rc1)
    {
        // We allow execution of this function only if debugged process was suspended
        // and openGL data was recorded and textures images data logging is enabled
        retVal = debuggedProcessSuspended && wasRecorded && recordingEnabled;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gdFlushTexturesImagesCommand::executeSpecificCommand
// Description: Terminates the debugged process.
// Return Val:  bool - Success / failure.
// Author:      Eran Zinman
// Date:        31/1/2008
// ---------------------------------------------------------------------------
bool gdFlushTexturesImagesCommand::executeSpecificCommand()
{
    bool retVal = false;

    // Generate a vector containing all textures to be exported:
    bool rc1 = generateAllTexturesExportList(m_dirtyTexturesVector);
    GT_IF_WITH_ASSERT(rc1)
    {
        // Show the progress bar only if we have any textures to export:
        if (m_dirtyTexturesVector.size() > 0)
        {
            // Export all textures to disk:
            exportAllTexturesToDisk(m_dirtyTexturesVector);

            // Notify the user that the export process was done:
            afProgressBarWrapper::instance().hideProgressBar();

        }

        retVal = true;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gdFlushTexturesImagesCommand::generateAllTexturesExportList
// Description: Generate a vector containing all textures that will be
//              exported (from all render contexts)
// Arguments:   contextTexturesVector - Output vector. The vector will be in the size
//              of the amount of render context. For each entry in the vector,
//              the numeric value will indicate the number of textures in the
//              specific render context.
//              totalAmountOfTexturesToExport - Output the total amount of dirty
//              textures to be exported from all render contexts
// Return Val:  bool - Success / Failure
// Author:      Eran Zinman
// Date:        29/1/2008
// ---------------------------------------------------------------------------
bool gdFlushTexturesImagesCommand::generateAllTexturesExportList(gtVector<gdExportedTextureID>& dirtyTexturesVector)
{
    bool retVal = false;

    // Make sure output vector is clean:
    dirtyTexturesVector.clear();

    bool rcOpenGLTextures = true;
    bool rcOpenCLTextures = true;

    if (gaIsAPIConnectionActive(AP_OPENGL_API_CONNECTION))
    {
        // Generate the OpenGL textures list for update:
        rcOpenGLTextures = generateAllOpenGLTexturesExportList(dirtyTexturesVector);
        GT_ASSERT(rcOpenGLTextures);
    }

    if (gaIsAPIConnectionActive(AP_OPENCL_API_CONNECTION))
    {
        // Generate the OpenCL textures list for update:
        rcOpenCLTextures = generateAllOpenCLImagesExportList(dirtyTexturesVector);
        GT_ASSERT(rcOpenCLTextures);
    }

    retVal = rcOpenCLTextures && rcOpenGLTextures;
    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gdFlushTexturesImagesCommand::exportAllTexturesToDisk
// Description: Export all textures to disk according to the contexts vector
// Arguments:   contextTexturesVector - Contexts textures vector.
//              See the "generateAllTexturesExportList" function for details
// Return Val:  bool - Success / Failure
// Author:      Eran Zinman
// Date:        29/1/2008
// ---------------------------------------------------------------------------
bool gdFlushTexturesImagesCommand::exportAllTexturesToDisk(const gtVector<gdExportedTextureID>& dirtyTexturesVector)
{
    bool retVal = false;

    // Sanity check:
    retVal = true;

    // Get default image export format (defined by the user) using the gdGDebuggerGlobalVariablesManager.
    gdGDebuggerGlobalVariablesManager& globalVarsManager = gdGDebuggerGlobalVariablesManager::instance();
    apFileType exportFileType = globalVarsManager.imagesFileFormat();

    int progressSteps = dirtyTexturesVector.size() * 3 + 5;
    afProgressBarWrapper::instance().setProgressDetails(GD_STR_FlushTexturesAndBuffersCommandWaitMessage, progressSteps);

    afMainAppWindow* pMainWindow = afMainAppWindow::instance();

    if (pMainWindow != NULL)
    {
        pMainWindow->setCursor(Qt::WaitCursor);
        pMainWindow->setFocus();
    }

    // Loop through the render contexts and export textures from each render context
    for (int i = 0; i < (int) dirtyTexturesVector.size(); i++)
    {
        // Get current context dirty textures vector
        gdExportedTextureID dirtyTextureID = dirtyTexturesVector[i];

        // Export the current texture:
        bool rc1 = exportTextureToDisk(dirtyTextureID, exportFileType);
        GT_ASSERT(rc1);

        // Update the progress:
        afProgressBarWrapper::instance().incrementProgressBar();
    }

    if (pMainWindow != NULL)
    {
        pMainWindow->setCursor(Qt::ArrowCursor);
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gdFlushTexturesImagesCommand::exportTextureToDisk
// Description: Export a texture to disk
// Arguments:   contextID - The context to flush all of his textures to disk
//              pContextDirtyTexturesVecotr - A vector containing all
//              dirty textures id that need to be exported
//              fileType - The file type format to save the texture as
// Return Val:  bool - Success / Failure
// Author:      Eran Zinman
// Date:        29/1/2008
// ---------------------------------------------------------------------------
bool gdFlushTexturesImagesCommand::exportTextureToDisk(gdExportedTextureID textureID, apFileType fileType)
{
    bool retVal = true;

    // Generate the currently exported object name:
    gtString currentObjectName;

    if (textureID._contextID.isOpenGLContext())
    {
        currentObjectName.appendFormattedString(GD_STR_FlushTexturesAndBuffersCommandExportedGLObjectName, textureID._textureId._textureName, textureID._contextID._contextId);
    }
    else if (textureID._contextID.isOpenCLContext())
    {
        currentObjectName.appendFormattedString(GD_STR_FlushTexturesAndBuffersCommandExportedCLObjectName, textureID._textureId._textureName, textureID._contextID._contextId);
    }

    gtString updateText;
    updateText.appendFormattedString(GD_STR_ImagesAndBuffersViewerExportingItemMessage, currentObjectName.asCharArray(), m_amountOfTexturesAlreadyExported, m_dirtyTexturesVector.size());
    afProgressBarWrapper::instance().setProgressText(updateText);

    // Sanity check:
    GT_IF_WITH_ASSERT(retVal)
    {
        // Update the double progress bar item text:
        gtString exportingMessage;
        exportingMessage.appendFormattedString(GD_STR_FlushTexturesAndBuffersCommandExportingSuffix, currentObjectName.asCharArray());
        afProgressBarWrapper::instance().setProgressText(exportingMessage);

        // Initialize textures exporter class and attach the double progress dialog
        gdImagesAndBuffersExporter texturesExporter(textureID._contextID);

        // Write texture image context to disk
        (void) texturesExporter.exportTexture(textureID, fileType, true);

        // Increase the total amount of textures already exported
        m_amountOfTexturesAlreadyExported++;

        // Update total progress bar:
        afProgressBarWrapper::instance().incrementProgressBar();
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gdFlushTexturesImagesCommand::generateAllOpenGLTexturesExportList
// Description: Generates the vector of OpenGL texture for export
// Arguments:   gtVector<gdExportedTextureID>& dirtyTexturesVector
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        31/5/2010
// ---------------------------------------------------------------------------
bool gdFlushTexturesImagesCommand::generateAllOpenGLTexturesExportList(gtVector<gdExportedTextureID>& dirtyTexturesVector)
{
    bool retVal = false;

    // Get the amount of allocated render contexts:
    int contextsAmount = 0;
    bool rc1 = gaGetAmountOfRenderContexts(contextsAmount);
    GT_IF_WITH_ASSERT(rc1)
    {
        retVal = true;

        // Loop through the render contexts (first context is contextID = 1)
        for (int contextID = 1; contextID < contextsAmount; contextID++)
        {
            // Calculate the amount of dirty textures in current render context
            int amountOfDirtyTextures = 0;

            // Check if context was deleted or not
            bool isContextDeleted = gaWasContextDeleted(apContextID(AP_OPENGL_CONTEXT, contextID));

            // If context was not deleted
            if (!isContextDeleted)
            {
                // Get amount of textures in current context
                int amountOfTextures = 0;
                bool rc3 = gaGetAmountOfTextureObjects(contextID, amountOfTextures);
                GT_IF_WITH_ASSERT(rc3)
                {
                    // Loop through all the textures in the current render context
                    for (int i = 0; i < amountOfTextures; i++)
                    {
                        // Get the current texture object ID
                        GLuint textureName = 0;
                        bool rc4 = gaGetTextureObjectName(contextID, i, textureName);

                        if (rc4)
                        {
                            // Check if texture is dirty or not
                            bool dirtyImageExists = true, dirtyRawDataExists = true;

                            // TO_DO: texture mip levels - optimization
                            gdExportedTextureID textureID;
                            textureID._textureId._textureName = textureName;
                            textureID._textureId._textureMipLevel = 0;
                            textureID._contextID._contextId = contextID;
                            textureID._contextID._contextType = AP_OPENGL_CONTEXT;
                            bool rc5 = gaIsTextureImageDirty(contextID, textureID._textureId, dirtyImageExists, dirtyRawDataExists);
                            GT_IF_WITH_ASSERT(rc5)
                            {
                                // Add only dirty textures to the export vector
                                if (dirtyImageExists)
                                {
                                    // Add texture to the export vector
                                    dirtyTexturesVector.push_back(textureID);

                                    // Increase amount of dirty textures
                                    amountOfDirtyTextures++;
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

// ---------------------------------------------------------------------------
// Name:        gdFlushTexturesImagesCommand::generateAllOpenCLImagesExportList
// Description: Generates the vector of OpenCL texture for export
// Arguments:   gtVector<gdExportedTextureID>& dirtyTexturesVector
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        1/6/2010
// ---------------------------------------------------------------------------
bool gdFlushTexturesImagesCommand::generateAllOpenCLImagesExportList(gtVector<gdExportedTextureID>& dirtyTexturesVector)
{
    bool retVal = false;

    // Get the amount of OpenCL contexts:
    int contextsAmount = 0;
    bool rcGetContexts = gaGetAmountOfOpenCLContexts(contextsAmount);
    GT_IF_WITH_ASSERT(rcGetContexts)
    {
        retVal = true;

        // Loop through the contexts (first context is contextID = 1):
        for (int contextID = 1; contextID < contextsAmount; contextID++)
        {
            // Check if context was deleted or not:
            bool wasContextDeleted = gaWasContextDeleted(apContextID(AP_OPENCL_CONTEXT, contextID));

            // If context was not deleted:
            if (!wasContextDeleted)
            {
                // Get amount of textures in current context:
                int amountOfTextures = 0;
                bool rcAmount = gaGetAmountOfOpenCLImageObjects(contextID, amountOfTextures);
                GT_IF_WITH_ASSERT(rcAmount)
                {
                    // Loop through all the textures in the current render context:
                    for (int textureIndex = 0; textureIndex < amountOfTextures; textureIndex++)
                    {
                        // Define the texture ID:
                        gdExportedTextureID textureID;
                        textureID._textureId._textureName = textureIndex;
                        textureID._textureId._textureMipLevel = 0;
                        textureID._contextID._contextId = contextID;
                        textureID._contextID._contextType = AP_OPENCL_CONTEXT;

                        // Add texture to the export vector:
                        dirtyTexturesVector.push_back(textureID);
                    }
                }
            }
        }
    }
    return retVal;
}

