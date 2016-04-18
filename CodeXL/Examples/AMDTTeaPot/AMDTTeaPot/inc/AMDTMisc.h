//==================================================================================
// Copyright (c) 2004 - 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file AMDTMisc.h
///
//==================================================================================

//------------------------------ AMDTMisc.h ----------------------------

#ifndef __AMDTMISC_H
#define __AMDTMISC_H

#include <stdio.h>
#include <stdlib.h>
#include <wchar.h>

/*
 * Some basic macros
 */
#define RET_EXIT_SUCCESS                (0)
#define RET_EXIT_ERR_GLX                (1)
#define RET_EXIT_ERR_ASSERTION          (2)
#define RET_EXIT_ERR_UNKNOWN            (-1)

#define MAX_PATH_LEN                    (255)


#if defined(WIN32)
#include "AMDTTeaPot.h"
#include <string.h>
#include <stdio.h>
#include <windows.h>
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>

#define ASSERT_MAX_STRING_LENGTH        (1024)

#define ASSERT(cond, fmt, ...)                                                                              \
    do {                                                                                                        \
        if (!(cond)) {                                                                                            \
            const int len = ASSERT_MAX_STRING_LENGTH;                                                               \
            char str[len];                                                                                          \
            wchar_t wstr[len];                                                                                      \
            _snprintf(str, len, "At File %s (line %3d)\n"                                                           \
                      "In function %s\n"                                                                  \
                      fmt "\n"                                                                            \
                      , __FILE__, __LINE__, __PRETTY_FUNCTION__, __VA_ARGS__);                            \
            ::mbstowcs(wstr, str, len);                                                                             \
            wstr[len] = '\0';                                                                                       \
            ::MessageBox( NULL, wstr, L"ASSERT", MB_OK | MB_ICONERROR);                                             \
            ::exit(RET_EXIT_ERR_ASSERTION);                                                                         \
        }                                                                                                         \
        __pragma(warning(push))                                                                              \
        __pragma(warning(disable : 4127))                                                                    \
    } while (0)                                                                                             \
        __pragma(warning(pop))

#define CONVERT_WX2GTK_MNEMONIC(VAR)                                                                        \
    do {                                                                                                        \
        for (size_t i = 0 ; i < strlen(VAR) ; i++) {                                                            \
            if (VAR[i] == '&')  VAR[i] = '_';                                                                   \
        }                                                                               \
        __pragma(warning(push))                                                                              \
        __pragma(warning(disable : 4127))                                                                    \
    } while (0)                                                                                             \
        __pragma(warning(pop))

#elif defined(__linux__)    //#if defined(WIN32)
#define ASSERT(cond, fmt, args...)                                                                          \
    do {                                                                                                        \
        if (!(cond)) {                                                                                            \
            fprintf(stderr, "ASSERT: %s:%3d:%s: " fmt "\n" , __FILE__, __LINE__, __PRETTY_FUNCTION__, ##args);      \
            ::exit(RET_EXIT_ERR_ASSERTION);                                                                         \
        }                                                                                                         \
    } while (0)

#define CONVERT_WX2GTK_MNEMONIC(VAR)                                                                        \
    do {                                                                                                        \
        for (size_t i = 0 ; i < strlen(VAR) ; i++) {                                                            \
            if (VAR[i] == '&')  VAR[i] = '_';                                                                   \
        }                                                                                                       \
    } while (0)
#endif // #elif defined(__linux__)


#define NEW_CS_FROM_WCS(VAR)                                                                                \
    char _##VAR[wcslen(VAR)];                                                                                   \
    memset(_##VAR, 0, wcslen(VAR));                                                                             \
    /*ASSERT((wcstombs(_##VAR, VAR, wcslen(VAR)) != (size_t)-1), "Invalid wide string conversion");      */     \
    if((wcstombs(_##VAR, VAR, wcslen(VAR)) == (size_t)-1)) {                                                    \
        for (unsigned int rd = 0, wr = 0; rd < wcslen(VAR) ; rd++) {                                            \
            switch (wctomb(&_##VAR[wr], VAR[rd])) {                                                             \
                case -1:            /* some unconvertable wchar encountered */                                  \
                    break;                                                                                      \
                case 0:             /* '\0' encountered */                                                      \
                    break;                                                                                      \
                default:                                                                                        \
                    wr++;                                                                                       \
                    break;                                                                                      \
            }                                                                                                   \
        }                                                                                                       \
    }                                                                                                           \
    _##VAR[wcslen(VAR)] = '\0';


#endif // #ifndef __AMDTMISC_H
