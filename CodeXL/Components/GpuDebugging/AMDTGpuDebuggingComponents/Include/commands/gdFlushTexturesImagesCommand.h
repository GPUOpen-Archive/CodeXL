//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gdFlushTexturesImagesCommand.h
///
//==================================================================================

//------------------------------ gdFlushTexturesImagesCommand.h ------------------------------

#ifndef __GDFLUSHTEXTURESIMAGESCOMMAND
#define __GDFLUSHTEXTURESIMAGESCOMMAND

// AMDTApplicationFramework:
#include <AMDTApplicationFramework/Include/afCommand.h>


// Local:
#include <AMDTGpuDebuggingComponents/Include/dialogs/gdDoubleProgressBarDialog.h>
#include <AMDTGpuDebuggingComponents/Include/gdGDebuggerAppDLLBuild.h>
#include <AMDTGpuDebuggingComponents/Include/views/gdImagesAndBuffersExporter.h>
// ----------------------------------------------------------------------------------
// Class Name:           gdFlushTexturesImagesCommand : public afCommand
// General Description:
//   Flushes all textures images (from all render context) to disk.
//   This is used mainly for openGL html log.
//
// Author:               Eran Zinman
// Creation Date:        31/1/2008
// ----------------------------------------------------------------------------------
class GD_API gdFlushTexturesImagesCommand : public afCommand
{
public:
    gdFlushTexturesImagesCommand(QWidget* pParent);
    virtual ~gdFlushTexturesImagesCommand();

    // Overrides afCommand:
    virtual bool canExecuteSpecificCommand();
    virtual bool executeSpecificCommand();

private:

    // Generate a vector containing all textures that will be exported (from all render contexts):
    bool generateAllTexturesExportList(gtVector<gdExportedTextureID>& dirtyTexturesVector);

    // Adds all OpenGL textures to the vector:
    bool generateAllOpenGLTexturesExportList(gtVector<gdExportedTextureID>& dirtyTexturesVector);

    // Adds all OpenCL images to the vector:
    bool generateAllOpenCLImagesExportList(gtVector<gdExportedTextureID>& dirtyTexturesVector);

    // Export all textures to disk according to the contexts vector:
    bool exportAllTexturesToDisk(const gtVector<gdExportedTextureID>& dirtyTexturesVector);

    // Export one texture:
    bool exportTextureToDisk(gdExportedTextureID textureID, apFileType fileType);

private:

    // The parent which the double progress dialog will be the child of
    QWidget* m_pMyParent;

    // This variable will hold the total amount of textures already exported
    long m_amountOfTexturesAlreadyExported;

    gtVector<gdExportedTextureID> m_dirtyTexturesVector;
};



#endif  // __GDFLUSHTEXTURESIMAGESCOMMAND
