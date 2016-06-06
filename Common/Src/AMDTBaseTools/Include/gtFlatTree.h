//=====================================================================
// Copyright 2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gtFlatTree.h
///
//=====================================================================
#ifndef __GTFLATTREE
#define __GTFLATTREE

// Local:
#include "gtVector.h"
#include "gtAlgorithms.h"

template<class _Kty, // Key type
         class _VTy> // Value type
struct gtKeyAsValueBegin
{
    const _Kty& operator()(const _VTy& val) const
    {
        return *reinterpret_cast<const _Kty*>(&val);
    }
};

template<class _Kty, // Key type
         class _Ty, // Value type
         class _Pr, // Comparator predicate type
         class _Alloc, // Actual allocator type (should be value allocator)
         class _Kfv = gtKeyAsValueBegin<_Kty, _Ty> > // Extract key from element value
class gtFlatTree
{
protected:
    typedef gtVector<_Ty, _Alloc> vector_type;

    vector_type m_vec;

public:
    typedef _Kty key_type;
    typedef _Ty value_type;
    typedef _Pr key_compare;
    typedef _Alloc allocator_type;
    typedef _Kfv extract_key;

    typedef typename vector_type::size_type                 size_type;
    typedef typename vector_type::difference_type           difference_type;
    typedef typename vector_type::pointer                   pointer;
    typedef typename vector_type::const_pointer             const_pointer;
    typedef typename vector_type::reference                 reference;
    typedef typename vector_type::const_reference           const_reference;

    typedef typename vector_type::iterator                  iterator;
    typedef typename vector_type::const_iterator            const_iterator;
    typedef typename vector_type::reverse_iterator          reverse_iterator;
    typedef typename vector_type::const_reverse_iterator    const_reverse_iterator;


    typedef std::pair<iterator, bool> _Pairib;
    typedef std::pair<iterator, iterator> _Pairii;
    typedef std::pair<const_iterator, const_iterator> _Paircc;


    gtFlatTree() {}

    gtFlatTree(const gtFlatTree& other) : m_vec(other.m_vec) {}

    gtFlatTree& operator=(const gtFlatTree& other)
    {
        m_vec = other.m_vec;
        return *this;
    }

#if AMDT_HAS_CPP0X
    gtFlatTree(gtFlatTree&& other) : m_vec(move(other.m_vec)) {}

    gtFlatTree& operator=(gtFlatTree&& other)
    {
        m_vec = move(other.m_vec);
        return *this;
    }

    const_iterator cbegin() const
    {
        return m_vec.cbegin();
    }

    const_iterator cend() const
    {
        return m_vec.cend();
    }

    const_reverse_iterator crbegin() const
    {
        return m_vec.crbegin();
    }

    const_reverse_iterator crend() const
    {
        return m_vec.crend();
    }
#endif // AMDT_HAS_CPP0X

    iterator begin()
    {
        return m_vec.begin();
    }

    const_iterator begin() const
    {
        return m_vec.begin();
    }

    iterator end()
    {
        return m_vec.end();
    }

    const_iterator end() const
    {
        return m_vec.end();
    }

    reverse_iterator rbegin()
    {
        return m_vec.rbegin();
    }

    const_reverse_iterator rbegin() const
    {
        return m_vec.rbegin();
    }

    reverse_iterator rend()
    {
        return m_vec.rend();
    }

    const_reverse_iterator rend() const
    {
        return m_vec.rend();
    }

    bool empty() const
    {
        return m_vec.empty();
    }

    size_type size() const
    {
        return m_vec.size();
    }

    size_type max_size() const
    {
        return m_vec.max_size();
    }

    size_type capacity() const
    {
        return m_vec.capacity();
    }

    void reserve(size_type cnt)
    {
        m_vec.reserve(cnt);
    }

    void swap(gtFlatTree& other)
    {
        m_vec.swap(other.m_vec);
    }

    iterator erase(const_iterator pos)
    {
        return m_vec.erase(pos);
    }

    size_type erase(const key_type& k)
    {
        _Pairii range = equal_range(k);
        size_type ret = static_cast<size_type>(range.second - range.first);

        if (0 != ret)
        {
            m_vec.erase(range.first, range.second);
        }

        return ret;
    }

    iterator erase(const_iterator first, const_iterator last)
    {
        return m_vec.erase(first, last);
    }

    void clear()
    {
        m_vec.clear();
    }

    iterator find(const key_type& k)
    {
        iterator itEnd = end();
        iterator it = _Lower_bound(begin(), itEnd, k);

        if (it != itEnd && key_compare()(k, extract_key()(*it)))
        {
            it = itEnd;
        }

        return it;
    }

    const_iterator find(const key_type& k) const
    {
        const_iterator itEnd = end();
        const_iterator it = _Lower_bound(begin(), itEnd, k);

        if (it != itEnd && key_compare()(k, extract_key()(*it)))
        {
            it = itEnd;
        }

        return it;
    }

    size_type count(const key_type& k) const
    {
#if 0
        _Paircc range = equal_range(k);
        return static_cast<size_type>(range.second - range.first);
#else
        const_iterator first = begin(), last = end();
        first = _Lower_bound(first, last, k);
        return (first != last && !key_compare()(k, extract_key()(*first))) ? 1 : 0;
#endif
    }

    iterator lower_bound(const key_type& k)
    {
        return _Lower_bound(begin(), end(), k);
    }

    const_iterator lower_bound(const key_type& k) const
    {
        return _Lower_bound(begin(), end(), k);
    }

    iterator upper_bound(const key_type& k)
    {
        return _Upper_bound(begin(), end(), k);
    }

    const_iterator upper_bound(const key_type& k) const
    {
        return _Upper_bound(begin(), end(), k);
    }

    _Pairii equal_range(const key_type& k)
    {
        return _Equal_range(begin(), end(), k);
    }

    _Paircc equal_range(const key_type& k) const
    {
        return _Equal_range(begin(), end(), k);
    }

    _Pairib insert(const value_type& val)
    {
        iterator first = begin(), last = end();
        key_type k = extract_key()(val);
        iterator pos = _Lower_bound(first, last, k);

        _Pairib ret;
        ret.second = (pos == last || key_compare()(k, extract_key()(*pos)));
        ret.first = ret.second ? m_vec.insert(pos, val) : pos;
        return ret;
    }

#if AMDT_HAS_CPP0X
    _Pairib insert(value_type&& val)
    {
        iterator first = begin(), last = end();
        key_type k = extract_key()(val);
        iterator pos = _Lower_bound(first, last, k);

        _Pairib ret;
        ret.second = (pos == last || key_compare()(k, extract_key()(*pos)));
        ret.first = ret.second ? m_vec.insert(pos, std::move(val)) : pos;
        return ret;
    }
#endif // AMDT_HAS_CPP0X

    template<class _Iter>
    void insert(_Iter first, _Iter last)
    {
        for (; first != last; ++first)
        {
            insert(*first);
        }
    }

protected:
    template <class _FwdIt>
    _FwdIt _Lower_bound(_FwdIt first, _FwdIt last, const key_type& key) const
    {
        size_type len = static_cast<size_type>(last - first);

        while (0 != len)
        {
            size_type half = len >> 1;
            _FwdIt mid = first;
            mid += half;

            if (key_compare()(extract_key()(*mid), key))
            {
                first = ++mid;
                len -= half + 1;
            }
            else
            {
                len = half;
            }
        }

        return first;
    }

    template <class _FwdIt>
    _FwdIt _Upper_bound(_FwdIt first, _FwdIt last, const key_type& key) const
    {
        size_type len = static_cast<size_type>(last - first);

        while (0 != len)
        {
            size_type half = len >> 1;
            _FwdIt mid = first;
            mid += half;

            if (key_compare()(key, extract_key()(*mid)))
            {
                len = half;
            }
            else
            {
                first = ++mid;
                len -= half + 1;
            }
        }

        return first;
    }

    template <class _FwdIt>
    std::pair<_FwdIt, _FwdIt> _Equal_range(_FwdIt first, _FwdIt last, const key_type& key) const
    {
        size_type len = static_cast<size_type>(last - first);

        while (0 != len)
        {
            size_type half = len >> 1;
            _FwdIt mid = first;
            mid += half;

            if (key_compare()(extract_key()(*mid), key))
            {
                first = ++mid;
                len -= half + 1;
            }
            else if (key_compare()(key, extract_key()(*mid)))
            {
                len = half;
            }
            else
            {
                last = first;
                last += len;
                first = _Lower_bound(first, mid, key);
                last = _Lower_bound(++mid, last, key);
                return std::pair<_FwdIt, _FwdIt>(first, last);
            }
        }

        return std::pair<_FwdIt, _FwdIt>(first, first);
    }
};

#endif // __GTFLATTREE
