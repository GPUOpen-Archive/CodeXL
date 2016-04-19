//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file csBufferSerializer.h
///
//==================================================================================

//------------------------------ csBufferSerializer.h ------------------------------

#ifndef __CSBUFFERSERIALIZER
#define __CSBUFFERSERIALIZER

// Infra:
#include <AMDTAPIClasses/Include/apCLBuffer.h>
#include <AMDTOSWrappers/Include/osFilePath.h>
#include <AMDTAPIClasses/Include/apFileType.h>
#include <AMDTAPIClasses/Include/apCLBuffer.h>
#include <AMDTAPIClasses/Include/apCLSubBuffer.h>
#include <AMDTAPIClasses/Include/apCLImage.h>

// TO_DO: OpenCL buffers  change the class name after consulting with Yaki
// ----------------------------------------------------------------------------------
// Class Name:           csBufferSerializer
// General Description:  Stores an OpenCL buffer into a file.
// Author:               Sigal Algranaty
// Creation Date:        2/12/2009
// ----------------------------------------------------------------------------------
class csBufferSerializer
{
public:
    // Constructor:
    csBufferSerializer();

    // Destructor:
    virtual ~csBufferSerializer();

    // Save buffer data to a file:
    bool saveBufferToFile(apCLBuffer& bufferDetails, const osFilePath& bufferFilePath, oaCLCommandQueueHandle commandQueueHandle);

    // Save sub-buffer data to a file:
    bool saveSubBufferToFile(apCLSubBuffer& subBufferDetails, const apCLBuffer& ownerBuffer, const osFilePath& subBufferFilePath, oaCLCommandQueueHandle commandQueueHandle);

    // Save texture data to a file:
    bool saveTextureToFile(apCLImage& textureDetails, const osFilePath& textureFilePath, oaCLCommandQueueHandle commandQueueHandle);

};


#endif  // __CSBUFFERSERIALIZER
