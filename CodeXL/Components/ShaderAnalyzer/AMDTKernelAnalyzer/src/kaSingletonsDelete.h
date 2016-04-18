//------------------------------ kaSingletonsDelete.h ------------------------------

#ifndef __KASINGLETONSDELETE_H
#define __KASINGLETONSDELETE_H

// ----------------------------------------------------------------------------------
// Class Name           kaSingletonsDelete
// General Description:
//  Deletes all the singleton objects.
//  Thus removes redundant memory leak reports.
//
//  Implementation notes:
//    We hold a single instance of this class. Its destructor is called when the
//    process that holds it terminates. This is an excellent timing to delete all the
//    existing singleton objects.
//
// Author:              Gilad Yarnitzky
// Creation Date:       30/7/2013
// ----------------------------------------------------------------------------------
class kaSingletonsDelete
{
public:
    ~kaSingletonsDelete();
};


#endif  // __KASINGLETONSDELETE_H
