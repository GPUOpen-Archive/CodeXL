//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gsSyncObjectsMonitor.h
///
//==================================================================================

//------------------------------ gsSyncObjectsMonitor.h ------------------------------

#ifndef __GSSYNCOBJECTSMONITOR
#define __GSSYNCOBJECTSMONITOR

// OpenGL:
#include <AMDTOSAPIWrappers/Include/oaOpenGLIncludes.h>

// Infra:
#include <AMDTBaseTools/Include/gtPtrVector.h>
#include <AMDTAPIClasses/Include/apGLSync.h>


// ----------------------------------------------------------------------------------
// Class Name:           gsSyncObjectsMonitor
//
// General Description:
//   Monitors sync objects allocated in a given application.
// Author:               Sigal Algranaty
// Creation Date:        28/10/2009
// ----------------------------------------------------------------------------------
class gsSyncObjectsMonitor
{
public:
    gsSyncObjectsMonitor();
    ~gsSyncObjectsMonitor();

public:
    // Sync objects actions:
    bool onSyncObjectDeletion(GLsync sync);
    bool onSyncObjectCreation(GLsync sync, GLenum condition);

    int amountOfSyncObjects() const { return (int)_syncObjects.size(); };
    const apGLSync* getSyncObjectDetails(int syncObjectIndex) const;

private:
    // Do not allow use of the = operator for this class. Use reference or pointer transferral instead
    gsSyncObjectsMonitor& operator=(const gsSyncObjectsMonitor& otherMonitor);
    gsSyncObjectsMonitor(const gsSyncObjectsMonitor& otherMonitor);

private:
    // Hold the sync object details:
    gtPtrVector<apGLSync*> _syncObjects;

    // Assign the sync names:
    int _nextFreeSyncID;
};


#endif  // __GSSYNCOBJECTSMONITOR
