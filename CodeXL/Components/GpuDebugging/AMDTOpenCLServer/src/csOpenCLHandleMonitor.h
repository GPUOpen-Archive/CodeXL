//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file csOpenCLHandleMonitor.h
///
//==================================================================================

//------------------------------ csOpenCLHandleMonitor.h ------------------------------

#ifndef __CSOPENCLHANDLEMONITOR_H
#define __CSOPENCLHANDLEMONITOR_H

// Infra:
#include <AMDTBaseTools/Include/gtMap.h>
#include <AMDTOSWrappers/Include/osCriticalSection.h>
#include <AMDTOSAPIWrappers/Include/oaOSAPIDefinitions.h>
#include <AMDTAPIClasses/Include/apCLObjectID.h>


// ----------------------------------------------------------------------------------
// Class Name:           csOpenCLHandleMonitor
// General Description: Monitors OpenCL handle objects. (Is used for cl parameters display
// Author:               Sigal Algranaty
// Creation Date:        8/12/2009
// ----------------------------------------------------------------------------------
class csOpenCLHandleMonitor
{
public:
    csOpenCLHandleMonitor();
    virtual ~csOpenCLHandleMonitor();

    // Get an OpenCL handle object:
    apCLObjectID* getCLHandleObjectDetails(oaCLHandle ptr) const;
    void registerOpenCLHandle(oaCLHandle ptr, int contextId, int objectId, osTransferableObjectType objectType, int ownerObjectId = -1, int objectDisplayId = -1);
    void nameHandledObject(oaCLHandle handle, const gtString& objectName);

    bool validateLivingHandle(oaCLHandle handle, osTransferableObjectType type) const;

private:
    // Do not allow use of the = operator for this class. Use reference or pointer transferral instead
    csOpenCLHandleMonitor& operator=(const csOpenCLHandleMonitor& otherMonitor) = delete;
    csOpenCLHandleMonitor& operator=(csOpenCLHandleMonitor&& otherMonitor) = delete;
    csOpenCLHandleMonitor(const csOpenCLHandleMonitor& otherMonitor) = delete;
    csOpenCLHandleMonitor(csOpenCLHandleMonitor&& otherMonitor) = delete;

    // A vector containing devices objects:
    gtMap<oaCLHandle, apCLObjectID*> _clHandleObjectsMap;
    osCriticalSection m_clHandleObjectsMapAccessCS;
};


#endif //__CSOPENCLHANDLEMONITOR_H

