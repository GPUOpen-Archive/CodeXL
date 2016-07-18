//=====================================================================
// Copyright 2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gtStack.h
///
//=====================================================================

//------------------------------ gtStack.h ------------------------------

#ifndef __GTSTACK
#define __GTSTACK

// STL:
#include <deque>
#include <stack>

// Local:
#include <AMDTBaseTools/Include/gtIgnoreCompilerWarnings.h>


// ----------------------------------------------------------------------------------
// Class Name:           gtStack
// General Description:
//   A class representing a queue of elements. Items insertion and extraction is done
//   in FIFO (First in first out) order.
//
// Author:      AMD Developer Tools Team
// Creation Date:        11/5/2003
// ----------------------------------------------------------------------------------
template<class Type, class Container = std::deque<Type> >
class gtStack : public std::stack<Type, Container>
{
public:
    typedef std::stack<Type, Container> StdStack;

    gtStack() {};
    gtStack(const gtStack& other) : StdStack(other) {}

    gtStack& operator=(const gtStack& other)
    {
        static_cast<StdStack*>(this)->operator=(other);
        return *this;
    }

#if AMDT_HAS_CPP0X
    gtStack(gtStack&& other) : StdStack(std::move(other)) {}

    gtStack& operator=(gtStack&& other)
    {
        static_cast<StdStack*>(this)->operator=(std::move(other));
        return *this;
    }
#endif
};


#endif  // __GTSTACK
