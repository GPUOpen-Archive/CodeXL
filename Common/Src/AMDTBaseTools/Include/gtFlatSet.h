//=====================================================================
// Copyright 2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gtFlatSet.h
///
//=====================================================================
#ifndef __GTFLATSET
#define __GTFLATSET

// Local:
#include "gtFlatTree.h"

template<class _Kty, class _Pr = std::less<_Kty>, class _A = std::allocator<_Kty> >
class gtFlatSet : public gtFlatTree<_Kty, _Kty, _Pr, _A>
{
public:
    typedef gtFlatTree<_Kty, _Kty, _Pr, _A> _Mybase;

    gtFlatSet() {}
    gtFlatSet(const gtFlatSet& other) : _Mybase(other) {}

    gtFlatSet& operator=(const gtFlatSet& other)
    {
        static_cast<_Mybase*>(this)->operator=(other);
        return *this;
    }

#if AMDT_HAS_CPP0X
    gtFlatSet(gtFlatSet&& other) : _Mybase(move(other)) {}

    gtFlatSet& operator=(gtFlatSet&& other)
    {
        static_cast<_Mybase*>(this)->operator=(move(other));
        return *this;
    }
#endif
};

#endif // __GTFLATSET
