//=====================================================================
// Copyright 2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file osAtomicImpl.h
///
//=====================================================================
#ifndef __OSATOMICIMPL
#define __OSATOMICIMPL

#include <AMDTOSWrappers/Include/osOSDefinitions.h>

inline bool AtomicCompareAndSwap(volatile gtInt32& target, gtInt32 oldValue, gtInt32 newValue)
{
    bool result;
    __asm__ __volatile__("lock; cmpxchgl %3, %0; setz %1" :
                         "=m"(target), "=a"(result) :
                         "m"(target), "r"(newValue), "a"(oldValue) :
                         "memory");
    return result;
}

inline bool AtomicCompareAndSwap(volatile gtInt64& target, gtInt64 oldValue, gtInt64 newValue)
{
    bool result;
#if (AMDT_ADDRESS_SPACE_TYPE == AMDT_64_BIT_ADDRESS_SPACE)
    __asm__ __volatile__("lock; cmpxchgq %3, %0; setz %1" :
                         "=m"(target), "=a"(result) :
                         "m"(target), "r"(newValue), "a"(oldValue) :
                         "memory");
#else
    union
    {
        struct
        {
            gtInt32 lowPart;
            gtInt32 highPart;
        };

        gtInt64 quadPart;
    } oldLarge, newLarge;

    oldLarge.quadPart = oldValue;
    newLarge.quadPart = newValue;

    __asm__ __volatile__("lock; cmpxchg8b %0; setz %1" :
                         "=m"(target), "=a"(result) :
                         "m"(target), "d"(oldLarge.highPart), "a"(oldLarge.lowPart),
                         "c"(newLarge.highPart), "b"(newLarge.lowPart) :
                         "memory");
#endif
    return result;
}


inline gtInt32 AtomicAdd(volatile gtInt32& target, gtInt32 value)
{
    gtInt32 oldVal;

    __asm__ __volatile__("lock; xaddl %0, %1" :
                         "=r"(oldVal), "=m"(target) :
                         "0"(value), "m"(target) :
                         "memory");
    return oldVal;
}

inline gtInt64 AtomicAdd(volatile gtInt64& target, gtInt64 value)
{
    gtInt64 oldVal;

#if (AMDT_ADDRESS_SPACE_TYPE == AMDT_64_BIT_ADDRESS_SPACE)
    __asm__ __volatile__("lock; xaddq %0, %1" :
                         "=r"(oldVal), "=m"(target) :
                         "0"(value), "m"(target) :
                         "memory");
#else

    do
    {
        oldVal = target;
    }
    while (!AtomicCompareAndSwap(target, oldVal, oldVal + value));

#endif

    return oldVal;
}


inline gtInt32 AtomicAnd(volatile gtInt32& target, gtInt32 value)
{
    gtInt32 oldVal;

    do
    {
        oldVal = target;
    }
    while (!AtomicCompareAndSwap(target, oldVal, oldVal & value));

    return oldVal;
}

inline gtInt64 AtomicAnd(volatile gtInt64& target, gtInt64 value)
{
    gtInt64 oldVal;

    do
    {
        oldVal = target;
    }
    while (!AtomicCompareAndSwap(target, oldVal, oldVal & value));

    return oldVal;
}


inline gtInt32 AtomicOr(volatile gtInt32& target, gtInt32 value)
{
    gtInt32 oldVal;

    do
    {
        oldVal = target;
    }
    while (!AtomicCompareAndSwap(target, oldVal, oldVal | value));

    return oldVal;
}

inline gtInt64 AtomicOr(volatile gtInt64& target, gtInt64 value)
{
    gtInt64 oldVal;

    do
    {
        oldVal = target;
    }
    while (!AtomicCompareAndSwap(target, oldVal, oldVal | value));

    return oldVal;
}


inline gtInt32 AtomicXor(volatile gtInt32& target, gtInt32 value)
{
    gtInt32 oldVal;

    do
    {
        oldVal = target;
    }
    while (!AtomicCompareAndSwap(target, oldVal, oldVal ^ value));

    return oldVal;
}

inline gtInt64 AtomicXor(volatile gtInt64& target, gtInt64 value)
{
    gtInt64 oldVal;

    do
    {
        oldVal = target;
    }
    while (!AtomicCompareAndSwap(target, oldVal, oldVal ^ value));

    return oldVal;
}


inline gtInt32 AtomicSwap(volatile gtInt32& target, gtInt32 value)
{
    gtInt32 oldVal;

    __asm__ __volatile__("xchgl %0, %1" :
                         "=r"(oldVal), "=m"(target) :
                         "0"(value), "m"(target) :
                         "memory");
    return oldVal;
}

inline gtInt64 AtomicSwap(volatile gtInt64& target, gtInt64 value)
{
    gtInt64 oldVal;

#if (AMDT_ADDRESS_SPACE_TYPE == AMDT_64_BIT_ADDRESS_SPACE)
    __asm__ __volatile__("xchgq %0, %1" :
                         "=r"(oldVal), "=m"(target) :
                         "0"(value), "m"(target) :
                         "memory");
#else

    do
    {
        oldVal = target;
    }
    while (!AtomicCompareAndSwap(target, oldVal, value));

#endif

    return oldVal;
}

#endif // __OSATOMICIMPL
