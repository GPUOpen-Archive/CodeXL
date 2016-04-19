//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file vspSingletonsDelete.h
///
//==================================================================================

//------------------------------ vspSingletonsDelete.h ------------------------------

#ifndef __VSPSINGLETONSDELETE_H
#define __VSPSINGLETONSDELETE_H

// ----------------------------------------------------------------------------------
// Class Name:           vspSingletonsDelete
// General Description:
//  Deletes all the singleton objects.
//  Thus removes memory leak.
//
//  Implementation notes:
//    We hold a single instance of this class. Its destructor is called when the
//    process that holds it unloads the DLL. This is an excellent timing to delete all the
//    existing singleton objects.
// Author:               Uri Shomroni
// Creation Date:        12/10/2010
// ----------------------------------------------------------------------------------
class vspSingletonsDelete
{
public:
    vspSingletonsDelete();
    ~vspSingletonsDelete();
};

#endif //__VSPSINGLETONSDELETE_H

