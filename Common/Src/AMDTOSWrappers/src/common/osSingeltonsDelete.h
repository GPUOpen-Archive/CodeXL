//=====================================================================
// Copyright 2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file osSingeltonsDelete.h
///
//=====================================================================

//------------------------------ osSingeltonsDelete.h ------------------------------

#ifndef __OSSINGELTONSDELETE
#define __OSSINGELTONSDELETE


// ----------------------------------------------------------------------------------
// Class Name:           osSingeltonsDelete
// General Description:
//  Deletes all the GROSWrappers.dll singleton objects.
//  Thus removes redundant memory leak reports.
//
//  Implementation notes:
//    We hold a single instance of this class. Its destructor is called when the
//    process that holds it terminates. This is an excellent timing to delete all the
//    existing singleton objects.
//
// Author:      AMD Developer Tools Team
// Creation Date:        24/4/2004
// ----------------------------------------------------------------------------------
class osSingeltonsDelete
{
public:
    osSingeltonsDelete() {};
    ~osSingeltonsDelete();
};


#endif  // __OSSINGELTONSDELETE
