//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apSingeltonsDelete.h
///
//==================================================================================

//------------------------------ apSingeltonsDelete.h ------------------------------

#ifndef __APSINGELTONSDELETE
#define __APSINGELTONSDELETE

// ----------------------------------------------------------------------------------
// Class Name:           apSingeltonsDelete
// General Description:
//  Deletes all the singleton objects.
//  Thus removes redundant memory leak reports.
//
//  Implementation notes:
//    We hold a single instance of this class. Its destructor is called when the
//    process that holds it terminates. This is an excellent timing to delete all the
//    existing singleton objects.
//
// Author:  AMD Developer Tools Team
// Creation Date:        24/4/2004
// ----------------------------------------------------------------------------------
class apSingeltonsDelete
{
public:
    apSingeltonsDelete() {};
    ~apSingeltonsDelete();
};

#endif  // __APSINGELTONSDELETE
