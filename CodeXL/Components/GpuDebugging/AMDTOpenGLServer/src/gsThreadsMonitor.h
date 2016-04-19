//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gsThreadsMonitor.h
///
//==================================================================================

//------------------------------ gsThreadsMonitor.h ------------------------------

#ifndef __GSTHREADSMONITOR_H
#define __GSTHREADSMONITOR_H

// Infra:
#include <AMDTBaseTools/Include/gtMap.h>
#include <src/gsThreadLocalData.h>

// Local:

// ----------------------------------------------------------------------------------
// Class Name:          gsThreadsMonitor
// General Description: Used to maintain information relevant to threads and the OpenGL
//                      implementation and spy (Thread to context relations, etc)
// Author:              Uri Shomroni
// Creation Date:       2/11/2009
// ----------------------------------------------------------------------------------
class gsThreadsMonitor
{
public:
    gsThreadsMonitor();
    ~gsThreadsMonitor();

    // Thread <-> Context relation queries:
    int threadCurrentRenderContext(const osThreadId& threadId) const;
    osThreadId renderContextCurrentThread(int renderContextId) const;
    int currentThreadRenderContextSpyId() const;

    // Updating the information based on changes in the app (MakeCurrent calls):
    void removeRenderContextFromAllThreads(oaOpenGLRenderContextHandle pRenderContext);
    bool setCurrentThreadCurrentContext(oaDeviceContextHandle hDC, oaDrawableHandle drawSurface, oaDrawableHandle readSurface, int spyContextId);

private:
    // Maps a thread id to its current render context:
    gtMap<osThreadId, gsThreadLocalData*> _threadIdToThreadLocalData;

    // Critical section object that control the access to _threadIdToThreadLocalData:
    osCriticalSection _threadIdToThreadLocalDataCS;
};

#endif //__GSTHREADSMONITOR_H

