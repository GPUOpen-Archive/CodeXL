//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file csOpenCLHandleMonitor.cpp
///
//==================================================================================

//------------------------------ csOpenCLHandleMonitor.cpp ------------------------------

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTOSWrappers/Include/osDebugLog.h>
#include <AMDTOSWrappers/Include/osCriticalSectionLocker.h>


// Local:
#include <src/csOpenCLHandleMonitor.h>


// ---------------------------------------------------------------------------
// Name:        csOpenCLHandleMonitor::csOpenCLHandleMonitor
// Description: Constructor.
// Arguments:
// Author:      Sigal Algranaty
// Date:        8/12/2009
// ---------------------------------------------------------------------------
csOpenCLHandleMonitor::csOpenCLHandleMonitor()
{
}

// ---------------------------------------------------------------------------
// Name:        csOpenCLHandleMonitor::~csOpenCLHandleMonitor
// Description: Destructor.
// Author:      Sigal Algranaty
// Date:        8/12/2009
// ---------------------------------------------------------------------------
csOpenCLHandleMonitor::~csOpenCLHandleMonitor()
{
    osCriticalSectionLocker mapCSLocker(m_clHandleObjectsMapAccessCS);

    // Clear the object IDs:
    gtMap<oaCLHandle, apCLObjectID*>::iterator iter = _clHandleObjectsMap.begin();
    gtMap<oaCLHandle, apCLObjectID*>::iterator endIter = _clHandleObjectsMap.end();

    for (; endIter != iter; iter++)
    {
        delete(*iter).second;
    }

    // Clear the map:
    _clHandleObjectsMap.clear();
}

// ---------------------------------------------------------------------------
// Name:        csOpenCLHandleMonitor::getCLHandleObjectDetails
// Description: Return an OpenCL object by its handle
// Arguments: void* ptr
// Return Val: apCLObjectID*
// Author:      Sigal Algranaty
// Date:        8/12/2009
// ---------------------------------------------------------------------------
apCLObjectID* csOpenCLHandleMonitor::getCLHandleObjectDetails(oaCLHandle ptr) const
{
    apCLObjectID* pRetVal = NULL;

    // Do not attempt this if the critical section is locked:
    if (((osCriticalSection&)m_clHandleObjectsMapAccessCS).tryEntering())
    {
        // Find the handle within the map:
        gtMap<oaCLHandle, apCLObjectID*>::const_iterator iterFind = _clHandleObjectsMap.find(ptr);

        if (iterFind != _clHandleObjectsMap.end())
        {
            pRetVal = (*iterFind).second;
        }

        ((osCriticalSection&)m_clHandleObjectsMapAccessCS).leave();
    }

    return pRetVal;
}

// ---------------------------------------------------------------------------
// Name:        csOpenCLHandleMonitor::registerOpenCLHandle
// Description: Adds an openCL handle mapping
// Arguments: void* ptr
//            int contextId
//            int objectId
//            osTransferableObjectType objectType
//            int ownerObjectId - represent the object that owns the object  -
//                                for example - program for kernels. This parameter is optional
//            int objectDisplayId - when the object name is different then it's index (object that
//                                  are released), use this parameter for the object 'real' display name
// Return Val: void
// Author:      Sigal Algranaty
// Date:        8/12/2009
// ---------------------------------------------------------------------------
void csOpenCLHandleMonitor::registerOpenCLHandle(oaCLHandle ptr, int contextId, int objectId, osTransferableObjectType objectType, int ownerObjectId, int objectDisplayId)
{
    osCriticalSectionLocker mapCSLocker(m_clHandleObjectsMapAccessCS);

    apCLObjectID* pNewObj = getCLHandleObjectDetails(ptr);

    if (pNewObj == NULL)
    {
        pNewObj = new apCLObjectID;

        // Insert the new object to the map:
        _clHandleObjectsMap[ptr] = pNewObj;
    }

    // Set the object details:
    pNewObj->_contextId = contextId;
    pNewObj->_objectId = objectId;
    pNewObj->_objectType = objectType;
    pNewObj->_ownerObjectId = ownerObjectId;
    pNewObj->_objectDisplayName = objectDisplayId;
}

// ---------------------------------------------------------------------------
// Name:        csOpenCLHandleMonitor::nameHandledObject
// Description: Sets the name of a handled object, to match a call to a
//              clNameXxxxGREMEDY() function.
// Author:      Uri Shomroni
// Date:        22/7/2010
// ---------------------------------------------------------------------------
void csOpenCLHandleMonitor::nameHandledObject(oaCLHandle handle, const gtString& objectName)
{
    osCriticalSectionLocker mapCSLocker(m_clHandleObjectsMapAccessCS);

    gtMap<oaCLHandle, apCLObjectID*>::iterator findIter = _clHandleObjectsMap.find(handle);

    if (findIter != _clHandleObjectsMap.end())
    {
        apCLObjectID* pObjectId = (*findIter).second;
        GT_IF_WITH_ASSERT(pObjectId != NULL)
        {
            pObjectId->_objectName = objectName;
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        csOpenCLHandleMonitor::validateLivingHandle
// Description: Returns true iff the handle is registered as a living object of the given type.
// Author:      Uri Shomroni
// Date:        28/7/2015
// ---------------------------------------------------------------------------
bool csOpenCLHandleMonitor::validateLivingHandle(oaCLHandle handle, osTransferableObjectType type) const
{
    bool retVal = false;

    if (OA_CL_NULL_HANDLE != handle)
    {
        // HERE BE DRAGONS!
        // Do not attempt to update an object we "know" is dead (that is to say, its handle has been released, but it is not yet reused).
        apCLObjectID* pObj = getCLHandleObjectDetails(handle);
        retVal = (nullptr != pObj);

        if (retVal)
        {
            retVal = (-1 < pObj->_objectId);

            // Devices are not context-bound:
            if (OS_TOBJ_ID_CL_DEVICE != type)
            {
                retVal = retVal && (0 < pObj->_contextId);
            }
            else // OS_TOBJ_ID_CL_DEVICE == type
            {
                retVal = retVal && (0 == pObj->_contextId);
            }

            retVal = retVal && (type == pObj->_objectType);
        }
    }

    return retVal;
}

