//=====================================================================
// Copyright 2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gtFlatMap.h
///
//=====================================================================
#ifndef __GTFLATMAP
#define __GTFLATMAP

// Local:
#include "gtFlatTree.h"

template<class _K, class _Tp, class _Pr = std::less<_K>, class _A = std::allocator<std::pair<_K, _Tp> > >
class gtFlatMap : public gtFlatTree<_K, std::pair<_K, _Tp>, _Pr, _A>
{
public:
    typedef gtFlatTree<_K, std::pair<_K, _Tp>, _Pr, _A> _Mybase;
    typedef _Tp mapped_type;

    gtFlatMap() {}
    gtFlatMap(const gtFlatMap& other) : _Mybase(other) {}

    gtFlatMap& operator=(const gtFlatMap& other)
    {
        static_cast<_Mybase*>(this)->operator=(other);
        return *this;
    }

#if AMDT_HAS_CPP0X
    gtFlatMap(gtFlatMap&& other) : _Mybase(move(other)) {}

    gtFlatMap& operator=(gtFlatMap&& other)
    {
        static_cast<_Mybase*>(this)->operator=(move(other));
        return *this;
    }
#endif

    mapped_type& operator[](const _K& k)
    {
        typename _Mybase::iterator first = _Mybase::begin(), last = _Mybase::end();
        typename _Mybase::iterator pos = _Mybase::_Lower_bound(first, last, k);

        if (pos == last || typename _Mybase::key_compare()(k, typename _Mybase::extract_key()(*pos)))
        {
            pos = _Mybase::m_vec.insert(pos, typename _Mybase::value_type(k, mapped_type()));
        }

        return pos->second;
    }
};

#endif // __GTFLATMAP
