//==============================================================================
// Copyright (c) 2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief This file contains a function that will be called at startup which in
///        turn calls XInitThreads.
//==============================================================================

#ifdef _DEBUG
    #include <iostream>
#endif

#include <X11/Xlib.h>

extern "C"
{

    void CallXInitThreads()
    {
#ifdef _DEBUG
        std::cout << "Calling XInitThreads\n";
#endif
        XInitThreads();
    }

}
