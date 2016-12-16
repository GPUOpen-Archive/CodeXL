//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file CpuProfVersion.h
///
//==================================================================================

#ifndef _CPUPROF_VERSION_H_
#define _CPUPROF_VERSION_H_
#pragma once

#define CPUPROF_MAJOR_VERSION  2
#define CPUPROF_MINOR_VERSION  4
#define CPUPROF_BUILD_VERSION  0

/// \def DRIVER_VERSION The version format is:
/// [31-24]: cpu prof major, [23-16]: cpu prof minor, [15-0]: cpu prof build
#define DRIVER_VERSION  ((CPUPROF_MAJOR_VERSION << 24) | (CPUPROF_MINOR_VERSION << 16) | CPUPROF_BUILD_VERSION)

#endif // _CPUPROF_VERSION_H_
