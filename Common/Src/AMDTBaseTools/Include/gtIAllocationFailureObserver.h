//=====================================================================
// Copyright 2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gtIAllocationFailureObserver.h
///
//=====================================================================

//------------------------------ gtIAllocationFailureObserver.h ------------------------------

#ifndef __GTIALLOCATIONFAILUREOBSERVER_H
#define __GTIALLOCATIONFAILUREOBSERVER_H

// Local:
#include <AMDTBaseTools/Include/gtGRBaseToolsDLLBuild.h>


// ----------------------------------------------------------------------------------
// Class Name:           gtIAllocationFailureObserver
// General Description:
//   An interface that can be implemented by classes that would like to receive
//   notifications about memory allocation failures.
//
// Author:      AMD Developer Tools Team
// Creation Date:        31/1/2009
// ----------------------------------------------------------------------------------
class GT_API gtIAllocationFailureObserver
{
public:
    virtual void onAllocationFailure() = 0;

    virtual ~gtIAllocationFailureObserver();
};


#endif //__GTIALLOCATIONFAILUREOBSERVER_H

