//==============================================================================
/// Copyright (c) 2016 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file   vktLayerManager.h
/// \brief  Header file for Vulkan layer manager.
///         This is a high level class which maintains a stack of available
///         and enabled layers in the Vulkan server. It also provides a means to
///         perform custom work at create, destroy, and begin-frame time.
//==============================================================================

#ifndef __VKT_LAYER_MANAGER_H__
#define __VKT_LAYER_MANAGER_H__

#include "../../Common/ModernAPILayerManager.h"
#include "../../Common/defines.h"

class VktInterceptManager;
class FrameInfo;

//-----------------------------------------------------------------------------
///  A LayerManager with the special task of managing layers specific to Vulkan's runtime stack.
//-----------------------------------------------------------------------------
class VktLayerManager : public ModernAPILayerManager
{
public:
    static VktLayerManager* GetLayerManager();

    MultithreadedTraceAnalyzerLayer* GetTraceAnalyzerLayer();
    ModernAPIFrameDebuggerLayer* GetFrameDebuggerLayer();
    ModernAPIFrameProfilerLayer* GetFrameProfilerLayer();
    ObjectDatabaseProcessor* GetObjectDatabaseProcessor();

    VktLayerManager();

    /// Nothing special to destroy here
    virtual ~VktLayerManager() {}

    virtual bool OnCreate(CREATION_TYPE type, void* pPtr);
    virtual bool OnDestroy(CREATION_TYPE type, void* pPtr);
    virtual void BeginFrame();
    virtual void AutocaptureTriggered();

    bool InitializeLayerManager();
    bool ShutdownLayerManager();
    bool ProcessRequestFromCommId(CommunicationID inRequestId);

    /// Return whether this layer manager has been initialized
    bool HasBeenInitialized() const { return m_isInitialized; }

    /// Return a pointer to the interception manager
    VktInterceptManager* GetInterceptMgr() const { return m_pInterceptMgr; }

private:
    /// Our interception manager
    VktInterceptManager* m_pInterceptMgr;

    /// Track whether this layer manage is initialized already
    bool m_isInitialized;
};

#endif //__VKT_LAYER_MANAGER_H__