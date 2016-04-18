//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gdStateVariablesCompareFunctor.h
///
//==================================================================================

//------------------------------ gdStateVariablesCompareFunctor.h ------------------------------

#ifndef __GDSTATEVARIABLESCOMPAREFUNCTOR
#define __GDSTATEVARIABLESCOMPAREFUNCTOR

// Infra:
#include <AMDTBaseTools/Include/gtVector.h>


// ----------------------------------------------------------------------------------
// Class Name:           gdStateVariablesCompareFunctor
// General Description:
//  Sorts state variables according to the alphabetic order of their names.
//
// Author:               Yaki Tebeka
// Creation Date:        12/4/2005
// ----------------------------------------------------------------------------------
class gdStateVariablesCompareFunctor
{
public:
    bool operator()(const int& x, const int& y);
};


#endif // __GDSTATEVARIABLESCOMPAREFUNCTOR
