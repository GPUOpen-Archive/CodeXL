//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gsPBuffer.h
///
//==================================================================================

//------------------------------ gsPBuffer.h ------------------------------

#ifndef __GSPBUFFER
#define __GSPBUFFER

// OpenGL:
#include <AMDTOSAPIWrappers/Include/oaOpenGLIncludes.h>

// Infra:
#include <AMDTOSWrappers/Include/osOSDefinitions.h>
#include <AMDTAPIClasses/Include/apPBuffer.h>

// Local:
#include <src/gsStaticBuffersMonitor.h>


// ----------------------------------------------------------------------------------
// Class Name:           gsPBuffer : public apPBuffer
// General Description:
//   Represents an OpenGL PBuffer.
//   Adds spy related data to apPBuffer
//
// Author:               Eran Zinman
// Creation Date:        28/8/2007
// ----------------------------------------------------------------------------------
class gsPBuffer : public apPBuffer
{
public:
    // Constructor:
    gsPBuffer(const oaPBufferHandle& pbufferHandler, int pbufferID);

    // Destructor:
    virtual ~gsPBuffer();

public:
    // Update the PBuffer data snapshot:
    void updateDataSnapshot();

    // Get a static buffer object from the PBuffer objects
    apStaticBuffer* getPBufferStaticBufferObject(int staticBufferIter) const;

    // Updates the pPuffer static buffer raw data
    bool updatePBufferStaticBufferRawData(apDisplayBuffer bufferType);

    // On PBuffer deletion event handler
    void onDeletion();

    // Updates the static buffers list in PBuffer
    void updateStaticBuffersList();

    // Retrieves the PBuffer render context handle
    bool getRenderContextOSHandle(oaOpenGLRenderContextHandle& pbufferRenderContext) const;

protected:
    // Overrides gsStaticBuffersMonitor: we need to generate our own name convention for PBuffers
    virtual bool generateBufferFilePath(apDisplayBuffer bufferType, osFilePath& bufferFilePath) const;

private:
    // The PBuffer Id
    int _pbufferID;
};


#endif  // __GSPBUFFER
