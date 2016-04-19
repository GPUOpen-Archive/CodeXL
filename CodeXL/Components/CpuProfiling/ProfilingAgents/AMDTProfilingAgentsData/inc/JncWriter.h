//==================================================================================
// Copyright (c) 2013-2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file JncWriter.h
/// \brief This file contains an interface to write JIT Native Code file.
///
//==================================================================================

#ifndef _JNCWRITER_H_
#define _JNCWRITER_H_

#ifdef _WIN32

    #include "../src/Windows/PeJncWriter.h"
    typedef PeJncWriter JncWriter;

#else

    #include "../src/Linux/ElfJncWriter.h"
    typedef ElfJncWriter JncWriter;

#endif

#endif // _JNCWRITER_H_
