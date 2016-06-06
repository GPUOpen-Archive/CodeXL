//=====================================================================
// Copyright 2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file osFilePathByLastAccessDateCompareFunctor.h
///
//=====================================================================

//------------------------------ osFilePathByLastModifiedDateCompareFunctor.h ------------------------------

#ifndef __OSFILEPATHBYLASTACCESSDATECOMPAREFUNCTOR
#define __OSFILEPATHBYLASTACCESSDATECOMPAREFUNCTOR

// Local:
#include <AMDTOSWrappers/Include/osFile.h>

// ----------------------------------------------------------------------------------
// Class Name:           osFilePathByLastModifiedDateCompareFunctor
// General Description:
//   Compare functor of file paths, by the file's last access date.
//
// Author:      AMD Developer Tools Team
// Creation Date:        24/12/2007
// ----------------------------------------------------------------------------------
class osFilePathByLastModifiedDateCompareFunctor
{
public:
    bool operator()(const osFilePath& file1, const osFilePath& file2);
};


#endif  // __OSFILEPATHBYLASTACCESSDATECOMPAREFUNCTOR
