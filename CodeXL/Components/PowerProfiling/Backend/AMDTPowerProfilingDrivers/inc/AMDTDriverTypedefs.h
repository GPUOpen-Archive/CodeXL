//===============================================================================
//
// Copyright(c) 2015 Advanced Micro Devices, Inc.All Rights Reserved
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files(the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and / or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions :
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//
//=================================================================================

#ifndef _DRIVER_TYPEDEFS_H_
#define _DRIVER_TYPEDEFS_H_
#define SMU_REG_ADDRESS_UNDEFINED 0xFFFFFF
#ifdef _WIN32
    typedef __int8  int8;
    typedef __int16 int16;
    typedef __int32 int32;
    typedef __int64 int64;

    typedef unsigned __int8  uint8;
    typedef unsigned __int16 uint16;
    typedef unsigned __int32 uint32;
    typedef unsigned __int64 uint64;

    typedef unsigned __int8  uint8_t;
    typedef unsigned __int16 uint16_t;
    typedef unsigned __int32 uint32_t;
    typedef unsigned __int64 uint64_t;
    typedef uint8 byte;
    typedef uint32 atomic;

    // Atomic Operations
    #define ATOMIC_SET(x, y) InterlockedExchange((volatile LONG*)x, y)
    #define ATOMIC_GET(x, y) InterlockedExchange((volatile LONG*)x, y)
    #define ATOMIC_CMPEXCHANGE(x, y, z) InterlockedCompareExchange((LONG volatile*)x, y, z)
    #define WRITE_DWORD(x,y)  *(uint32*)(x) = y
    #define READ_DWORD(x)     (*(uint32*)(x))

#endif // _WIN32

#ifdef _LINUX

#ifdef KERNEL_MODULE
    #include <linux/types.h>
    #include <linux/string.h>
    #include <linux/kernel.h>
    typedef int wchar_t;
    #define STATUS_SUCCESS 0
    #define STATUS_NO_MEMORY -1
    #define STATUS_INVALID_PARAMETER -2
    #define STATUS_ACCESS_DENIED -3
#else
    #include <stdint.h>
#endif

typedef int8_t  int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;

typedef uint8_t  uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;
typedef uint8_t  byte;
#ifdef __KERNEL__
    typedef atomic_t atomic;
#else
    typedef uint32 atomic;
#endif

// Atomic functions
#ifdef __KERNEL__
    #define ATOMIC_SET(x, y)    atomic_set(x, y)
#else
    #define ATOMIC_SET(x, y)    __sync_lock_test_and_set(x, y)
#endif

#define ATOMIC_GET(x, y)    *(int *)x  = atomic_read(&y)
// if current value of *x is z, then write y into *x
#define ATOMIC_CMPEXCHANGE(x, y, z) __sync_val_compare_and_swap((int*)&x, z, y)

#define WRITE_DWORD(x,y)    write_dword(x,y)
#define READ_DWORD(x)       read_dword(x)

#define INVALID_HANDLE_VALUE -1

// For Compatability with Windows types

typedef union _ULARGE_INTEGER
{

    struct
    {
        uint32_t LowPart;
        uint32_t HighPart;
    };
    struct
    {
        uint32_t LowPart;
        uint32_t HighPart;
    } u;
    uint64_t    QuadPart;

} ULARGE_INTEGER;

typedef union _LARGE_INTEGER
{
    struct
    {
        int32_t LowPart;
        int32_t  HighPart;
    };
    struct
    {
        int32_t LowPart;
        int32_t HighPart;
    } u;
    int64_t QuadPart;
} LARGE_INTEGER, *PLARGE_INTEGER;

typedef int HANDLE;

#endif // _LINUX

#endif //_DRIVER_TYPEDEFS_H_
