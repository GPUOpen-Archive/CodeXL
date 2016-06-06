//==============================================================================
// Copyright (c) 2009-2016 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
//==============================================================================

#pragma once
#ifndef GDT_MEMUTILS_H
#define GDT_MEMUTILS_H

/// Safe delete macro.
/// \param[in] p     The pointer to delete.
#define GDT_SAFE_DELETE( p ) { if ( p != NULL ) { delete p; p = NULL; } }

/// Safe Release macro
/// \param[in] p     The pointer to release.
#define GDT_SAFE_RELEASE( p ) { if ( p != NULL ) if ( p->Release() == 0 ) p = NULL; }

/// Safe Release macro with assert if the ref-count is non-zero.
/// \param[in] p     The pointer to release.
#define GDT_SAFE_FINAL_RELEASE( p ) { if ( p != NULL ) { ULONG ulRefCount = p->Release(); GDT_Assert( ulRefCount  == 0 ); if( ulRefCount  == 0 ) p = NULL; } }

/// Safe free macro
/// \param[in] p     The pointer to free.
#define GDT_SAFE_FREE( p ) { if ( p != NULL ) { free(p); p = NULL; } }

/// Zero memory for an object.
/// \param[in] p     The object to zero.
#define GDT_ZERO_MEMORY(p) ZeroMemory(&p, sizeof(p))

/// Zero memory for an object pointer.
/// \param[in] p     The object pointer to zero.
#define GDT_ZERO_MEMORY_PTR(p) ZeroMemory(p, sizeof(*p))

/// Zero memory for an object pointer & size.
/// \param[in] p     The object pointer to zero.
/// \param[in] s     The size of memory to zero.
#define GDT_ZERO_MEMORY_RANGE(p, s) ZeroMemory(p, s)

#endif // GDT_MEMUTILS_H
