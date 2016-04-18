//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gsStaticBuffersMonitor.h
///
//==================================================================================

//------------------------------ gsStaticBuffersMonitor.h ------------------------------

#ifndef __GSSTATICBUFFERSMONITOR
#define __GSSTATICBUFFERSMONITOR

// OpenGL:
#include <AMDTOSAPIWrappers/Include/oaOpenGLIncludes.h>

// Infra:
#include <AMDTBaseTools/Include/gtPtrVector.h>
#include <AMDTOSWrappers/Include/osOSDefinitions.h>
#include <AMDTOSAPIWrappers/Include/oaPixelFormat.h>
#include <AMDTAPIClasses/Include/apDisplayBuffer.h>
#include <AMDTAPIClasses/Include/apStaticBuffer.h>

// Pre-decleration
class gsRenderContextMonitor;
// ----------------------------------------------------------------------------------
// Class Name:           gsStaticBuffersMonitor
// General Description:  Monitors static buffers objects.
// Author:               Eran Zinman
// Creation Date:        27/08/2007
// ----------------------------------------------------------------------------------
class gsStaticBuffersMonitor
{
public:
    // Constructor:
    gsStaticBuffersMonitor(int spyContextId);

    // Destructor:
    virtual ~gsStaticBuffersMonitor();

public:
    // Context event functions:
    void onContextDeletion();
    void onFirstTimeContextMadeCurrent(const int contextOGLVersion[2]);

    // On context change functions:
    void updateContextDataSnapshot(bool canQueryContext);
    void clearContextDataSnapshot();

    // Updates a given buffer's raw data:
    bool updateBufferRawData(apDisplayBuffer bufferType);

    // Updates the buffer size according to the current view port:
    bool updateStaticBuffersDimensions();

    // Set pixel format:
    void setRenderContextPixelFormat(const oaPixelFormat* pPixelFormat) {_pRenderContextPixelFormat = pPixelFormat;};

    // Calculates the size of hDC linked with current render context
    static bool getCurrentThreadHDCSize(int& bufferWidth, int& bufferHeight);

    // Get buffer data format and data type:
    static bool getBufferDataFormatAndDataType(apDisplayBuffer bufferType, oaTexelDataFormat& bufferDataFormat, oaDataType& componentDataType);

public:
    // Public query functions:
    int amountOfStaticBuffers() const;

    // Get static buffer object details:
    apStaticBuffer* getStaticBufferObjectDetails(apDisplayBuffer bufferType) const;
    apStaticBuffer* getStaticBufferObjectDetails(int bufferID) const;

protected:
    // Update all of the context static buffer:
    void updateAllStaticBuffers(bool canQueryContext);

    // Clear all the static buffers from the static buffer vector
    void clearAllBuffers();

    // Generates the buffer file path
    virtual bool generateBufferFilePath(apDisplayBuffer bufferType, osFilePath& bufferFilePath) const;

private:
    // Do not allow use of the = operator for this class. Use reference or pointer transferral instead
    gsStaticBuffersMonitor& operator=(const gsStaticBuffersMonitor& otherMonitor);
    gsStaticBuffersMonitor(const gsStaticBuffersMonitor& otherMonitor);

private:
    // Update an individual static buffer:
    void updateStaticBuffer(apDisplayBuffer bufferType);

private:
    // Checks if a specific buffer exists or not
    bool isBufferExists(apDisplayBuffer bufferType);

    // Checks if interactive break mode is enabled.
    bool isInteractiveBreakOn() const;

private:
    // Store the currently active buffer
    void storeCurrentlyActiveBuffer();

    // Sets a color index buffer as the active buffer
    bool setBufferAsActive(apDisplayBuffer bufferType);

    // Restore the currently active buffer
    void restoreCurrentlyActiveBuffer();

    // Updated a given buffer's raw data:
    bool updateBufferRawData(apStaticBuffer& staticBuffer);

    // Set the active FBO, as FBOs prevent us from reading static buffers:
    void changeActiveFBOInAPI(GLuint fboToBind) const;

private:
    // The Spy id of my monitored render context:
    int _spyContextId;

    // A pointer to the render context pixel format:
    const oaPixelFormat* _pRenderContextPixelFormat;

    // Hold the parameters of the static buffers that reside in this render context:
    gtPtrVector<apStaticBuffer*> _staticBuffers;

    // Holds the real value of GL_READ_BUFFER while we read the pixels:
    GLenum _activeReadBuffer;

    // Contains true iff our controlling context can answer framebuffer size queries, i.e. glGetIntegerv(GL_*_BITS):
    bool _areFramebufferSizeQueriesSupported;
};


#endif  // __GSSTATICBUFFERSMONITOR
