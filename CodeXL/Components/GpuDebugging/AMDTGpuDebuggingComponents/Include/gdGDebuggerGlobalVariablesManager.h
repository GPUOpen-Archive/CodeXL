//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gdGDebuggerGlobalVariablesManager.h
///
//==================================================================================

//------------------------------ gdGDebuggerGlobalVariablesManager.h ------------------------------

#ifndef __gdGDebuggerGlobalVariablesManager
#define __gdGDebuggerGlobalVariablesManager

// Qt:
#include <QtWidgets>

// Forward declaration:
class apBreakpointHitEvent;
class gdCodeXLGlobalVariableChangeEvent;
class TiXmlHandle;
class TiXmlNode;
class TiXmlDocument;

// Infra:
#include <AMDTBaseTools/Include/gtList.h>
#include <AMDTAPIClasses/Include/apFunctionType.h>
#include <AMDTOSWrappers/Include/osFilePath.h>
#include <AMDTOSWrappers/Include/osPortAddress.h>
#include <AMDTAPIClasses/Include/apDebugProjectSettings.h>
#include <AMDTAPIClasses/Include/apContextID.h>
#include <AMDTAPIClasses/Include/Events/apIEventsObserver.h>

// Local:
#include <AMDTGpuDebuggingComponents/Include/gdGDebuggerAppDLLBuild.h>

#define GD_DEFAULT_DEBUG_OBJECTS_TREE_MAX_ITEMS_PER_TYPE 200

// ----------------------------------------------------------------------------------
// Class Name:           gdGDebuggerGlobalVariablesManager
// General Description:
//   Stores and managed the application global variables.
//   Supplies notification mechanism for changes done in these variables.
// Author:               Yaki Tebeka
// Creation Date:        24/5/2004
// ----------------------------------------------------------------------------------
class GD_API gdGDebuggerGlobalVariablesManager : apIEventsObserver
{
public:
    static gdGDebuggerGlobalVariablesManager& instance();
    virtual ~gdGDebuggerGlobalVariablesManager();

    // Set / get global variable values functions:
    void setChosenContext(apContextID contextId);
    apContextID chosenContext() const { return _chosenContextId; };
    apContextType chosenContextType() const { return _chosenContextId._contextType; };

    void setChosenThread(int threadIndex, bool kernelDebuggingThread);
    int chosenThread() const { return _chosenThreadIndex; };
    bool isKernelDebuggingThreadChosen() const {return m_chosenThreadIsKernelDebugging;};

    unsigned int maxTreeItemsPerType() const { return m_maxTreeItemsPerType; };
    void setMaxTreeItemsPerType(unsigned int maxItems) { m_maxTreeItemsPerType = maxItems; };

    void startRecording();
    void stopRecording();
    bool recording() const { return _recording; };

    bool setCurrentDebugProjectSettings(const apDebugProjectSettings& projectSettings);
    const apDebugProjectSettings& currentDebugProjectSettings() const { return m_currentProjectSettings; };

    // Get the directory that contains the OpenGL ES implementation DLLs:
    const osFilePath& getOpenGLESDLLsDirectory() const { return _openGLESProjectDLLsDirectory; };

    // Overrides apIEventsObserver:
    virtual void onEvent(const apEvent& eve, bool& vetoEvent);
    virtual const wchar_t* eventObserverName() const { return L"CodeXLGlobalVariablesManager"; };

public:

    //////////////////////////////////////////////////////////////////////////
    // Set / get Options data functions:
    //////////////////////////////////////////////////////////////////////////
    void setImagesFileFormat(apFileType texturesFileFormat);
    apFileType imagesFileFormat() const;

    void setLoggingLimits(unsigned int maxOpenGLCallsPerContext, unsigned int maxOpenCLCallsPerContext, unsigned int maxOpenCLCommandPerQueue);
    void getLoggingLimits(unsigned int& maxOpenGLCallsPerContext, unsigned int& maxOpenCLCallsPerContext, unsigned int& maxOpenCLCommandPerQueue) const;

private:

    // Set (defaults) hard coded options data - if the options file does not exist
    void setDefaultsHardCodedOptionsData();

    int getThreadIndex(const osThreadId& threadId) const;

    void onProcessCreatedEvent();
    void onBreakpointHitEvent(apBreakpointHitEvent& eve);

    // Only my instance() function can create me:
    gdGDebuggerGlobalVariablesManager();

    // Only gdSingletonsDelete should delete me:
    friend class gdSingletonsDelete;

private:
    // The single instance of this class:
    static gdGDebuggerGlobalVariablesManager* _pMySingleInstance;

    // General
    osFilePath _optionsDataFilePath;

    // The Id of the context that the application GUI currently displays.
    apContextID _chosenContextId;

    // The Id of the thread that the application GUI currently displays.
    int _chosenThreadIndex;
    bool m_chosenThreadIsKernelDebugging;

    // Contains true iff the current call to setChosenContext was triggered by a call
    // to setChosenThread and vice versa:
    bool _triggeredByTheThread;
    bool _triggeredByTheContext;

    // The maximum number of items of each type we show in the monitored objects tree
    unsigned int m_maxTreeItemsPerType;

    // keep the status of the OpenGL calls log recorder
    bool _recording;

    // The current process creation data:
    apDebugProjectSettings m_currentProjectSettings;

    // Contains OpenGL projects spies directory:
    // TO_DO: OpenCL - do we need a separate spies directory for OpenCL? Using file path interception
    // and allowing OpenGL/CL-only project would make it seem so...
    osFilePath _openGLProjectSpiesDirectory;

    // Contains OpenGL ES DLLs directory:
    osFilePath _openGLESProjectDLLsDirectory;

};

#endif  // __gdGDebuggerGlobalVariablesManager
