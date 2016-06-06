#ifndef _COMPAT_H_
#define _COMPAT_H_ 1

#include <sys/cdefs.h>
#include <stdio.h>
#include <stdlib.h>

typedef signed   __int8  int8_t;
typedef unsigned __int8  uint8_t;
typedef signed   __int16 int16_t;
typedef unsigned __int16 uint16_t;
typedef signed   __int32 int32_t;
typedef unsigned __int32 uint32_t;
typedef signed   __int64 int64_t;
typedef unsigned __int64 uint64_t;

typedef uint8_t  u_int8_t;
typedef uint16_t u_int16_t;
typedef uint32_t u_int32_t;
typedef uint64_t u_int64_t;

#ifndef _WIN64
typedef __int32 ssize_t;
#else // _WIN64
typedef __int64 ssize_t;
#endif // _WIN64

#if __STDC_VERSION__ < 199901L
# if _MSC_VER >= 1600
#  define __func__ __FUNCTION__
# else
#  define __func__ "<unknown>"
# endif
#endif


typedef long uid_t;
typedef long gid_t;
typedef long mode_t;
typedef long off_t;

#endif