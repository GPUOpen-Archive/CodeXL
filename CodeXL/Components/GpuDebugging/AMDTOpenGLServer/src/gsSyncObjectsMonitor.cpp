//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gsSyncObjectsMonitor.cpp
///
//==================================================================================

//------------------------------ gsSyncObjectsMonitor.cpp ------------------------------

// Infra:
#include <AMDTBaseTools/Include/AMDTDefinitions.h>
#include <AMDTBaseTools/Include/gtAssert.h>

// Spies Utilities:
#include <AMDTServerUtilities/Include/suGlobalVariables.h>

// Local:
#include <src/gsSyncObjectsMonitor.h>
#include <src/gsGlobalVariables.h>
#include <src/gsOpenGLMonitor.h>

#define GS_LAST_SYNC_NAME GT_INT32_MAX

// ---------------------------------------------------------------------------
// Name:        gsSyncObjectsMonitor::gsSyncObjectsMonitor
// Description: Constructor
// Author:      Sigal Algranaty
// Date:        28/10/2009
// ---------------------------------------------------------------------------
gsSyncObjectsMonitor::gsSyncObjectsMonitor()
    : _nextFreeSyncID(1)
{
}

// ---------------------------------------------------------------------------
// Name:        gsSyncObjectsMonitor::~gsSyncObjectsMonitor
// Description: Destructor
// Author:      Sigal Algranaty
// Date:        28/10/2009
// ---------------------------------------------------------------------------
gsSyncObjectsMonitor::~gsSyncObjectsMonitor()
{
}



// ---------------------------------------------------------------------------
// Name:        gsSyncObjectsMonitor::onSyncObjectCreation
// Description: Handle OpenGL sync object creation
// Arguments: GLsync sync
//            GLenum condition
// Return Val: bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        28/10/2009
// ---------------------------------------------------------------------------
bool gsSyncObjectsMonitor::onSyncObjectCreation(GLsync sync, GLenum condition)
{
    bool retVal = true;

    if (sync != NULL)
    {
        // Create a new sync object:
        apGLSync* pNewSyncObject = new apGLSync;


        // Set the new sync object OpenGL id:
        pNewSyncObject->setSyncID(_nextFreeSyncID);
        pNewSyncObject->setSyncHandle((oaGLSyncHandle)sync);
        pNewSyncObject->setSyncCodition(condition);

        // Set the next free sync ID:
        if ((_nextFreeSyncID >= GS_LAST_SYNC_NAME) || (_nextFreeSyncID < 0))
        {
            _nextFreeSyncID = 1;
        }
        else
        {
            _nextFreeSyncID++;
        }

        // Add the object to the vector:
        _syncObjects.push_back(pNewSyncObject);

        // Register this object in the allocated objects monitor:
        su_stat_theAllocatedObjectsMonitor.registerAllocatedObject(*pNewSyncObject);
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gsSyncObjectsMonitor::onSyncObjectDeletion
// Description: Handles OpenGL sync object deletion
// Arguments: GLsync sync
// Return Val: bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        28/10/2009
// ---------------------------------------------------------------------------
bool gsSyncObjectsMonitor::onSyncObjectDeletion(GLsync sync)
{
    bool retVal = false;

    // If this is a valid handle:
    oaGLSyncHandle deletedSyncHandle = (oaGLSyncHandle)sync;

    if (deletedSyncHandle != OA_GL_NULL_HANDLE)
    {
        // Search for the sync object index within the vector of objects:
        int numberOfSyncObjects = (int)_syncObjects.size();
        bool foundSync = false;

        for (int i = 0; i < numberOfSyncObjects; i++)
        {
            // Sanity check:
            apGLSync* pSyncObject = _syncObjects[i];
            GT_IF_WITH_ASSERT(pSyncObject != NULL)
            {
                if (foundSync)
                {
                    // Note that foundSync can only be true if we already passed at least one object,
                    // so accessing the (i-1) vector item is allowed:
                    _syncObjects[i - 1] = _syncObjects[i];
                }
                else
                {
                    if (pSyncObject->syncHandle() == deletedSyncHandle)
                    {
                        // Sync handles can be reused by the OpenGL implementation, if we have several objects with the same handle,
                        // delete the one that is still alive:
                        foundSync = true;

                        // Delete the monitor object:
                        delete pSyncObject;
                        _syncObjects[i] = NULL;
                    }
                }
            }
        }

        // If we managed to find and delete the sync object:
        if (foundSync)
        {
            // The sync handle exists (or once existed):
            retVal = true;

            // Remove the last sync in the vector, which is either our object or a duplicate of some other object:
            _syncObjects.pop_back();
        }
        else // !foundSync
        {
            // TO_DO: OpenGL 3.2 add detected error - sync object that does not exist is deleted:
        }
    }
    else // deletedSyncHandle == OA_GL_NULL_HANDLE
    {
        // This is an allowed operation:
        retVal = true;
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gsSyncObjectsMonitor::getSyncObjectDetails
// Description: Returns an apGLSync object according to the sync index
// Return Val:  apGLSync* - the sync Object
// Author:      Sigal Algranaty
// Date:        28/10/2009
// ---------------------------------------------------------------------------
const apGLSync* gsSyncObjectsMonitor::getSyncObjectDetails(int syncObjectIndex) const
{
    const apGLSync* pRetVal = NULL;

    // Sanity check:
    int numberOfSyncObjects = (int)_syncObjects.size();
    GT_IF_WITH_ASSERT((syncObjectIndex > -1) && (syncObjectIndex < numberOfSyncObjects))
    {
        pRetVal = _syncObjects[syncObjectIndex];
    }

    return pRetVal;
}
