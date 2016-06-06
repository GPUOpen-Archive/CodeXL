//=====================================================================
// Copyright 2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gtAlgorithms.h
///
//=====================================================================

//------------------------------ gtAlgorithms.h ------------------------------

#ifndef __GTALGORITHMS
#define __GTALGORITHMS

// STL:
#include <algorithm>

// Local:
#include <AMDTBaseTools/Include/gtIgnoreCompilerWarnings.h>

// Allow using types from the std namespace:
using namespace std;


// ---------------------------------------------------------------------------
// Name:        gtFind
// Description: Returns the first iterator i in the range [first, last)
//              such that *i == value. Returns last if no such iterator exists.
// Arguments:   first - The search start iterator.
//              last - The search end iterator.
//              value - The value to be searched.
// Return Val:  InputIterator - The
// Author:      AMD Developer Tools Team
// Date:        5/4/2005
// Usage example:
//  gtList<int> myList;
//  myList.push_back(3);
//  myList.push_back(1);
//  myList.push_back(7);
//
//  gtList<int>::iterator result = gtFind(myList.begin(), myList.end(), 7);
//  gtAssert(result == myList.end() || *result != 7);
//
// ---------------------------------------------------------------------------
template<class InputIterator, class EqualityComparable>
InputIterator gtFind(InputIterator first, InputIterator last, const EqualityComparable& value)
{
    return std::find(first, last, value);
}


// ---------------------------------------------------------------------------
// Name:        gtSort
// Description:
//   Sort the elements in [first, last) into ascending order.
// Arguments: first - The first item in the collection to be sorted.
//            last - The last item in the collection to be sorted.
//            comp - A comparison functor (that tells the sort algorithm which
//                   item is bigger.
// Author:      AMD Developer Tools Team
// Date:        2/10/2005
// ---------------------------------------------------------------------------
template <class RandomAccessIterator, class StrictWeakOrdering>
void gtSort(RandomAccessIterator first, RandomAccessIterator last, StrictWeakOrdering comp)
{
    return std::sort(first, last, comp);
}

// ---------------------------------------------------------------------------
// Name:        gtSort
// Description:
//   Sort the elements in [first, last) into ascending order.
// Arguments: first - The first item in the collection to be sorted.
//            last - The last item in the collection to be sorted.
// Implementation Notes: The < operator must be implemented for the data.
// Author:      AMD Developer Tools Team
// Date:        10/1/2010
// ---------------------------------------------------------------------------
template <class RandomAccessIterator>
void gtSort(RandomAccessIterator first, RandomAccessIterator last)
{
    return std::sort(first, last);
}

template<class ForwardIterator, class ValType>
inline ForwardIterator gtLowerBound(ForwardIterator first, ForwardIterator last, const ValType& val)
{
    return std::lower_bound(first, last, val);
}

template<class ForwardIterator, class ValType>
inline ForwardIterator gtUpperBound(ForwardIterator first, ForwardIterator last, const ValType& val)
{
    return std::upper_bound(first, last, val);
}

template<class ForwardIterator, class ValType>
inline ForwardIterator gtBinarySearch(ForwardIterator first, ForwardIterator last, const ValType& val, ForwardIterator fail)
{
    // test if \a val equivalent to some element, using operator<
    first = gtLowerBound(first, last, val);
    return (first != last && !(val < *first)) ? first : fail;
}

template<class ForwardIterator, class ValType>
inline ForwardIterator gtBinarySearch(ForwardIterator first, ForwardIterator last, const ValType& val)
{
    return gtBinarySearch(first, last, val, last);
}


// Compute log2(N)
template <unsigned N>
struct Log2
{
    enum { VALUE = Log2 < N / 2 >::VALUE + 1 };
};

// Break the recursion
template <>
struct Log2<1>
{
    enum { VALUE = 0 };
};

// Break the recursion
template <>
struct Log2<0>
{
    enum { VALUE = 0 };
};


#define GT_ALIGN_DOWN(value, alignment)  ((value) & ~(alignment - 1))
#define   GT_ALIGN_UP(value, alignment)  (GT_ALIGN_DOWN(((value) + alignment - 1), alignment))


template <typename T>
inline T gtAlignDown(T value, gtSize_t alignment)
{
    return value & ~((T)(alignment - 1));
}

template <typename T>
inline T* gtAlignDown(T* value, gtSize_t alignment)
{
    return (T*) gtAlignDown((gtUIntPtr) value, alignment);
}

template <typename T>
inline T gtAlignUp(T value, gtSize_t alignment)
{
    return gtAlignDown(value + (T)(alignment - 1), alignment);
}

template <typename T>
inline T* gtAlignUp(T* value, gtSize_t alignment)
{
    return (T*) gtAlignUp((gtUIntPtr) value, alignment);
}


// Using de Bruijn sequence
inline gtUInt32 CountTrailingZeros(gtUInt32 val)
{
#ifndef _WIN32
    static const gtUByte multiplyDeBruijnBitPosition[32] =
    {
        0, 1, 28, 2, 29, 14, 24, 3, 30, 22, 20, 15, 25, 17, 4, 8,
        31, 27, 13, 23, 21, 19, 16, 7, 26, 12, 18, 6, 11, 5, 10, 9
    };
    return static_cast<gtUInt32>(multiplyDeBruijnBitPosition[static_cast<gtUInt32>((val & -val) * 0x077CB531U) >> 27]);
#else
    unsigned long index;
    _BitScanForward(&index, val);
    return static_cast<gtUInt32>(index);
#endif
}

#endif  // __GTALGORITHMS
