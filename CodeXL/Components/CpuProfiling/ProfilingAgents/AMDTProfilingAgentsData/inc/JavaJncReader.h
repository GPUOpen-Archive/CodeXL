//==================================================================================
// Copyright (c) 2013-2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file JavaJncReader.h
/// \brief This file contains an interface to read JCL file for Java code profiling.
///
//==================================================================================

#ifndef _JAVAJNCREADER_H_
#define _JAVAJNCREADER_H_

#ifdef _WIN32
    #include "../src/Windows/JavaJncReader_Win.h"
#else
    #include "../src/Linux/JavaJncReader_Lin.h"
#endif

#endif // _JAVAJNCREADER_H_
