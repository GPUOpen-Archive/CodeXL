//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gsExtensionsManager.h
///
//==================================================================================

//------------------------------ gsExtensionsManager.h ------------------------------

#ifndef __GSEXTENSIONSMANAGER
#define __GSEXTENSIONSMANAGER

// Forward decelerations:
class osFilePath;

// Infra:
#include <AMDTBaseTools/Include/gtPtrVector.h>
#include <AMDTOSWrappers/Include/osOSDefinitions.h>
#include <AMDTAPIClasses/Include/apMonitoredFunctionId.h>
#include <AMDTAPIClasses/Include/apOpenGLExtensionsId.h>

// OpenGL:
#include <AMDTOSAPIWrappers/Include/oaOpenGLIncludes.h>

// Local:
#include <src/gsMonitoredFunctionPointers.h>
#include <src/gsRenderContextExtensionsData.h>


// ----------------------------------------------------------------------------------
// Class Name:   gsExtensionsManager
// General Description:
//   Manages the supported OpenGL, WGL and XGL extension functions.
//
//  Implementation notes:
//    Under Win32:
//    a. Different extensions are supported for each render context.
//    b. The extension function addresses are unique for each pixel format.
//       (All rendering contexts of a given pixel format share the same extension
//       function addresses).
//    Therefore, we maintain a list (structure) of extension function implementation
//    pointers per render context.
//
//    Under Linux and Mac:
//    Extensions support and extension pointers are global for all render contexts.
//    Therefore, we use render context 0 extensions data as the global extensions data.
//
// Author:               Yaki Tebeka
// Creation Date:        29/8/2003
// ----------------------------------------------------------------------------------
class gsExtensionsManager
{
public:
    static gsExtensionsManager& instance();
    ~gsExtensionsManager();
    bool initialize();

    const GLubyte* getSpyUnifiedExtensionsString(int contextSpyId) const;
    const GLubyte* getSpyExtensionString(int contextSpyId, GLuint extensionIndex) const;
    unsigned int getNumberOfSpyExtensionStrings(int contextSpyId) const;
    bool isExtensionSupported(int contextSpyId, apOpenGLExtensionsId extensionId) const;
    osProcedureAddress wrapperFunctionAddress(const gtASCIIString& functionName);
    osProcedureAddress spyImplementedExtensionAddress(const gtString& functionName) const;
    gsMonitoredFunctionPointers* extensionsRealImplementationPointers(int contextSpyId);
    gsMonitoredFunctionPointers* currentRenderContextExtensionsRealImplPointers();
    bool copyExtensionPointerFromOtherContexts(apMonitoredFunctionId funcId);
    bool getExtensionPointerFromSystem(apMonitoredFunctionId funcId);

    // OpenGL events:
    void onContextCreatedEvent(int contextSpyId);
    void onFirstTimeContextMadeCurrent(int contextSpyId, const int contextOpenGLVersion[2], bool isCompatibilityProfileActive);

private:
    int functionIndexFromMonitoredFunctionId(apMonitoredFunctionId funcId) const;
    bool initializeWrapperAddresses();
    void updateExtensionsSupport(int contextSpyId, const int contextOpenGLVersion[2]);
    void calculateRenderContextSpyUnifiedExtensionsString(int contextSpyId);
    void calculateRenderContextSpyExtensionStrings(int contextSpyId);
    void calculateCurrentRenderContextOpenGLSpyUnifiedExtensionsString(gtASCIIString& spyExtesnionsStr);
    void calculateCurrentRenderContextOpenGLESSpyExtensionsString(gtASCIIString& spyExtesnionsStr);
    osProcedureAddress getOGLBaseFunctionSpyAddress(const gtASCIIString& functionName);

    // Only my instance() method is allowed to create me:
    gsExtensionsManager();

private:
    // Maps extension id to our wrapper / implementation address:
    gtVector<osProcedureAddress> _extensionIdToWrapperAddress;

    // Holds render contexts extensions data:
    // - On Windows - index i contains render context i extensions data.
    // - On Linux - index 0 contains all render contexts extensions data.
    // (See "Implementation notes" at this class header documentation)
    gtPtrVector<gsRenderContextExtensionsData*> _renderContextsExtensionsData;

private:
    // A pointer to this class single instance:
    static gsExtensionsManager* _pMySingleInstance;

    // Allow gsSingletonsDelete to delete my single instance:
    friend class gsSingletonsDelete;
};


#endif  // __GSEXTENSIONSMANAGER
