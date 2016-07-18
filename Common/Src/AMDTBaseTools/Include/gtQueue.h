//=====================================================================
// Copyright 2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gtQueue.h
///
//=====================================================================

//------------------------------ gtQueue.h ------------------------------

#ifndef __GTQUEUE
#define __GTQUEUE

// STL:
#include <deque>
#include <queue>

// Local:
#include <AMDTBaseTools/Include/gtIgnoreCompilerWarnings.h>


// ----------------------------------------------------------------------------------
// Class Name:           gtQueue
// General Description:
//   A class representing a queue of elements. Items insertion and extraction is done
//   in FIFO (First in first out) order.
//
// Author:      AMD Developer Tools Team
// Creation Date:        11/5/2003
// ----------------------------------------------------------------------------------
template<class Type, class Container = std::deque<Type> >
class gtQueue : public std::queue<Type, Container>
{
public:
    typedef std::queue<Type, Container> StdQueue;

    gtQueue() {};
    gtQueue(const gtQueue& other) : StdQueue(other) {};

    gtQueue& operator=(const gtQueue& other)
    {
        static_cast<StdQueue*>(this)->operator=(other);
        return *this;
    }

#if AMDT_HAS_CPP0X
    gtQueue(gtQueue&& other) : StdQueue(std::move(other)) {}

    gtQueue& operator=(gtQueue&& other)
    {
        static_cast<StdQueue*>(this)->operator=(std::move(other));
        return *this;
    }
#endif
};


#endif  // __GTQUEUE
