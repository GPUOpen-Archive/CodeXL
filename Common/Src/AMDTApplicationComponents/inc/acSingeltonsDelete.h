//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file acSingeltonsDelete.h
///
//==================================================================================

#ifndef __ACSINGELTONSDELETE
#define __ACSINGELTONSDELETE

// ----------------------------------------------------------------------------------
// Class Name:         acSingeltonsDelete
// General Description: Deletes all the singleton objects.
//                      Thus removes redundant memory leak reports.
//                      Implementation notes:
//                      We hold a single instance of this class. Its destructor is called when the
//                      process that holds it terminates. This is an excellent timing to delete all the
//                      existing singleton objects.
// Author:              Sigal Algranaty
// Creation Date:       9/8/2011
// ----------------------------------------------------------------------------------
class acSingeltonsDelete
{
public:
    acSingeltonsDelete() {};
    ~acSingeltonsDelete();
};

#endif  // __ACSINGELTONSDELETE
