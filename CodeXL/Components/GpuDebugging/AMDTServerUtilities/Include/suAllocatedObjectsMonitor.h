//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file suAllocatedObjectsMonitor.h
///
//==================================================================================

//------------------------------ suAllocatedObjectsMonitor.h ------------------------------

#ifndef __SUALLOCATEDOBJECTSMONITOR_H
#define __SUALLOCATEDOBJECTSMONITOR_H

// Forward declarations:
class apAllocatedObject;

// Infra:
#include <AMDTOSWrappers/Include/osCriticalSection.h>
#include <AMDTOSWrappers/Include/osCallStack.h>

// Local:
#include <AMDTServerUtilities/Include/suSpiesUtilitiesDLLBuild.h>

// ----------------------------------------------------------------------------------
// Class Name:           suAllocatedObjectsMonitor
// General Description:
//   Holds the creation call stack of "allocated objects" (apAllocatedObject objects).
// Author:               Uri Shomroni
// Creation Date:        25/11/2008
// ----------------------------------------------------------------------------------
class SU_API suAllocatedObjectsMonitor
{
public:
    static suAllocatedObjectsMonitor& instance();
    ~suAllocatedObjectsMonitor();

    unsigned int numberOfAllocatedObjects() const {return (unsigned int)_allocatedObjectsCreationCallStacks.size();};
    bool registerAllocatedObject(apAllocatedObject& allocObj);
    bool registerAllocatedObjects(gtVector<apAllocatedObject*>& allocObjs);
    bool getAllocatedObjectCreationCallStack(int index, const osCallStack*& o_pCallsStack) const;

    void collectAllocatedObjectsCreationCallsStacks(bool collectCreationStacks);

    void clearObjects();

private:
    // Do not allow use of the = operator for this class. Use reference or pointer transferral instead
    suAllocatedObjectsMonitor();
    suAllocatedObjectsMonitor& operator=(const suAllocatedObjectsMonitor& otherMonitor);
    suAllocatedObjectsMonitor(const suAllocatedObjectsMonitor& otherMonitor);

private:
    // Holds allocated objects creation call stack:
    gtVector<osCallStack*> _allocatedObjectsCreationCallStacks;

    // A critical section that control the access to _allocatedObjectsCreationCallStacks:
    osCriticalSection _allocatedObjectsCreationCallStacksCS;

    // Are we collecting allocated objects' creation calls stacks?
    bool _collectingAllocatedObjectsCreationCallsStacks;

    static suAllocatedObjectsMonitor* _pMySingleInstance;
};

#endif //__SUALLOCATEDOBJECTSMONITOR_H

