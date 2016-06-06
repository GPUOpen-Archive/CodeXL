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

inline gtInt32 AtomicAdd(volatile gtInt32& target, gtInt32 value)
{
    return _InterlockedExchangeAdd(reinterpret_cast<volatile long*>(&target), value);
}

inline gtInt64 AtomicAdd(volatile gtInt64& target, gtInt64 value)
{
#if (AMDT_ADDRESS_SPACE_TYPE == AMDT_64_BIT_ADDRESS_SPACE)
    return _InterlockedExchangeAdd64(&target, value);
#else
    gtInt64 oldVal;

    do
    {
        oldVal = target;
    }
    while (_InterlockedCompareExchange64(&target, oldVal + value, oldVal) != oldVal);

    return oldVal;
#endif
}


inline gtInt32 AtomicAnd(volatile gtInt32& target, gtInt32 value)
{
    return _InterlockedAnd(reinterpret_cast<volatile long*>(&target), value);
}

inline gtInt64 AtomicAnd(volatile gtInt64& target, gtInt64 value)
{
#if (AMDT_ADDRESS_SPACE_TYPE == AMDT_64_BIT_ADDRESS_SPACE)
    return _InterlockedAnd64(&target, value);
#else
    gtInt64 oldVal;

    do
    {
        oldVal = target;
    }
    while (_InterlockedCompareExchange64(&target, oldVal & value, oldVal) != oldVal);

    return oldVal;
#endif
}


inline gtInt32 AtomicOr(volatile gtInt32& target, gtInt32 value)
{
    return _InterlockedOr(reinterpret_cast<volatile long*>(&target), value);
}

inline gtInt64 AtomicOr(volatile gtInt64& target, gtInt64 value)
{
#if (AMDT_ADDRESS_SPACE_TYPE == AMDT_64_BIT_ADDRESS_SPACE)
    return _InterlockedOr64(&target, value);
#else
    gtInt64 oldVal;

    do
    {
        oldVal = target;
    }
    while (_InterlockedCompareExchange64(&target, oldVal | value, oldVal) != oldVal);

    return oldVal;
#endif
}


inline gtInt32 AtomicXor(volatile gtInt32& target, gtInt32 value)
{
    return _InterlockedXor(reinterpret_cast<volatile long*>(&target), value);
}

inline gtInt64 AtomicXor(volatile gtInt64& target, gtInt64 value)
{
#if (AMDT_ADDRESS_SPACE_TYPE == AMDT_64_BIT_ADDRESS_SPACE)
    return _InterlockedXor64(&target, value);
#else
    gtInt64 oldVal;

    do
    {
        oldVal = target;
    }
    while (_InterlockedCompareExchange64(&target, oldVal ^ value, oldVal) != oldVal);

    return oldVal;
#endif
}


inline gtInt32 AtomicSwap(volatile gtInt32& target, gtInt32 value)
{
    return _InterlockedExchange(reinterpret_cast<volatile long*>(&target), value);
}

inline gtInt64 AtomicSwap(volatile gtInt64& target, gtInt64 value)
{
#if (AMDT_ADDRESS_SPACE_TYPE == AMDT_64_BIT_ADDRESS_SPACE)
    return _InterlockedExchange64(&target, value);
#else
    gtInt64 oldVal;

    do
    {
        oldVal = target;
    }
    while (_InterlockedCompareExchange64(&target, value, oldVal) != oldVal);

    return oldVal;
#endif
}


inline bool AtomicCompareAndSwap(volatile gtInt32& target, gtInt32 oldValue, gtInt32 newValue)
{
    return oldValue == _InterlockedCompareExchange(reinterpret_cast<volatile long*>(&target), newValue, oldValue);
}

inline bool AtomicCompareAndSwap(volatile gtInt64& target, gtInt64 oldValue, gtInt64 newValue)
{
    return oldValue == _InterlockedCompareExchange64(&target, newValue, oldValue);
}

#endif // __OSATOMICIMPL
