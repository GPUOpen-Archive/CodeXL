//=====================================================================
// Copyright 2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gtSet.h
///
//=====================================================================

//------------------------------ gtSet.h ------------------------------

#ifndef __GTSET
#define __GTSET

// STL:
#include <set>

// Local:
#include <AMDTBaseTools/Include/AMDTDefinitions.h>
#include <AMDTBaseTools/Include/gtIgnoreCompilerWarnings.h>

// Allow using types from the std namespace:
using namespace std;


// ----------------------------------------------------------------------------------
// Class Name:           gtSet
// General Description:
//  A class representing a set of unique Keys.
//
// Author:      AMD Developer Tools Team
// Creation Date:        6/8/2013
// ----------------------------------------------------------------------------------

#if (AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT)

// Ehud - Same issue as with gtMap (full comment is in gtMap.h).
#define gtSet std::set

#else

template<class _Kty, class _Pr = less<_Kty>, class _Alloc = allocator<_Kty> >
class gtSet : public set<_Kty, _Pr, _Alloc>
{
public:
    typedef set<_Kty, _Pr, _Alloc> StdSet;

    gtSet() {}
    gtSet(const gtSet& other) : StdSet(other) {}

    gtSet& operator=(const gtSet& other)
    {
        static_cast<StdSet*>(this)->operator=(other);
        return *this;
    }

#if AMDT_HAS_CPP0X
    gtSet(gtSet&& other) : StdSet(move(other)) {}

    gtSet& operator=(gtSet&& other)
    {
        static_cast<StdSet*>(this)->operator=(move(other));
        return *this;
    }
#endif
};
#endif

#endif  // __GTSET
