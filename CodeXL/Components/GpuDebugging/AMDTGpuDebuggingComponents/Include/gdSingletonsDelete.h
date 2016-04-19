//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gdSingletonsDelete.h
///
//==================================================================================

//------------------------------ gdSingletonsDelete.h ------------------------------

#ifndef __GDSINGLETONSDELETE
#define __GDSINGLETONSDELETE


// ----------------------------------------------------------------------------------
// Class Name:           gdSingletonsDelete
// General Description:
//  Deletes all the singleton objects.
//  Thus removes redundant memory leak reports.
//
//  Implementation notes:
//    We hold a single instance of this class. Its destructor is called when the
//    process that holds it terminates. This is an excellent timing to delete all the
//    existing singleton objects.
//
// Author:               Yaki Tebeka
// Creation Date:        7/12/2003
// ----------------------------------------------------------------------------------
class gdSingletonsDelete
{
public:
    ~gdSingletonsDelete();
};


#endif  // __GDSINGLETONSDELETE
