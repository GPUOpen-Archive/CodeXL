//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gaSingletonsDelete.h
///
//==================================================================================

//------------------------------ gaSingletonsDelete.h ------------------------------

#ifndef __GASINGLETONSDELETE
#define __GASINGLETONSDELETE


// ----------------------------------------------------------------------------------
// Class Name:           gaSingletonsDelete
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
// Creation Date:        21/04/2004
// ----------------------------------------------------------------------------------
class gaSingletonsDelete
{
public:
    gaSingletonsDelete() {};
    ~gaSingletonsDelete();
};


#endif  // __GASINGLETONSDELETE
