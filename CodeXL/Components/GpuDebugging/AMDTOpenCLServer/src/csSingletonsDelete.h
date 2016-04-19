//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file csSingletonsDelete.h
///
//==================================================================================

//------------------------------ sSingletonsDelete.h ------------------------------

#ifndef __CSSINGLETONSDELETE
#define __CSSINGLETONSDELETE


// ----------------------------------------------------------------------------------
// Class Name:           csSingletonsDelete
// General Description:
//  Deletes all the singleton objects.
//  Thus removes redundant memory leak reports.
//
//  Implementation notes:
//    We hold a single instance of this class. Its destructor is called when the
//    process that holds it terminates. This is an excellent timing to delete all the
//    existing singleton objects.
//
// Author:               Uri Shomroni
// Creation Date:        16/11/2010
// ----------------------------------------------------------------------------------
class csSingletonsDelete
{
public:
    csSingletonsDelete();
    ~csSingletonsDelete();
};


#endif  // __CSSINGLETONSDELETE
