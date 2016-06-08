//=====================================================================
// Copyright 2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file oaSingeltonsDelete.h
///
//=====================================================================

//------------------------------ oaSingeltonsDelete.h ------------------------------

#ifndef __OASINGELTONSDELETE
#define __OASINGELTONSDELETE


// ----------------------------------------------------------------------------------
// Class Name:           oaSingeltonsDelete
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
class oaSingeltonsDelete
{
public:
    oaSingeltonsDelete() {};
    ~oaSingeltonsDelete();
};


#endif  // __OASINGELTONSDELETE
