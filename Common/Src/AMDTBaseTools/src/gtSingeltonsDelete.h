//=====================================================================
// Copyright 2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gtSingeltonsDelete.h
///
//=====================================================================

//------------------------------ gtSingeltonsDelete.h ------------------------------

#ifndef __GTSINGELTONSDELETE_H
#define __GTSINGELTONSDELETE_H


// ----------------------------------------------------------------------------------
// Class Name:           gtSingeltonsDelete
// General Description:
//  Deletes all the singleton objects.
//  Thus removes redundant memory leak reports.
//
//  Implementation notes:
//    We hold a single instance of this class. Its destructor is called when the
//    process that holds it terminates. This is an excellent timing to delete all the
//    existing singleton objects.
//
// Author:      AMD Developer Tools Team
// Creation Date:        20/8/2007
// ----------------------------------------------------------------------------------
class gtSingeltonsDelete
{
public:
    gtSingeltonsDelete() {};
    ~gtSingeltonsDelete();
};


#endif //__GTSINGELTONSDELETE_H

