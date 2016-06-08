//=====================================================================
// Copyright 2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gtMap.h
///
//=====================================================================

//------------------------------ gtMap.h ------------------------------

#ifndef __GTMAP
#define __GTMAP

// STL:
#include <map>

// Local:
#include <AMDTBaseTools/Include/AMDTDefinitions.h>
#include <AMDTBaseTools/Include/gtIgnoreCompilerWarnings.h>

// Allow using types from the std namespace:
using namespace std;


// ----------------------------------------------------------------------------------
// Class Name:           gtMap
// General Description:
//  A class representing a map that maps objects of type Key to objects of type Data.
//
// Author:      AMD Developer Tools Team
// Creation Date:        11/5/2003
// ----------------------------------------------------------------------------------

#if (AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT)

// Yaki - 10/11/2008 - I didn't manage to inherit gtMap on Mac without getting build problems.
// Instead, we use define.
#define gtMap std::map

#else

template<class _K, class _Tp, class _Pr = less<_K>, class _A = allocator<pair<const _K, _Tp> > >
class gtMap : public map< _K, _Tp, _Pr, _A>
{
public:
    typedef map< _K, _Tp, _Pr, _A> StdMap;

    gtMap() {}
    gtMap(const gtMap& other) : StdMap(other) {}

    gtMap& operator=(const gtMap& other)
    {
        static_cast<StdMap*>(this)->operator=(other);
        return *this;
    }

#if AMDT_HAS_CPP0X
    gtMap(gtMap&& other) : StdMap(move(other)) {}

    gtMap& operator=(gtMap&& other)
    {
        static_cast<StdMap*>(this)->operator=(move(other));
        return *this;
    }
#endif
};
#endif

#endif  // __GTMAP
