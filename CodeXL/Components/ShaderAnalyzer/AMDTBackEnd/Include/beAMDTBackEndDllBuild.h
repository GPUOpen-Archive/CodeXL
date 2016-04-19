// -*- C++ -*-
//=====================================================================
// Copyright 2012 Advanced Micro Devices, Inc. All rights reserved.
//
/// \author GPU Developer Tools
/// \file $File: //devtools/main/CodeXL/Components/ShaderAnalyzer/AMDTBackEnd/Include/beAMDTBackEndDllBuild.h $
/// \version $Revision: #1 $
/// \brief  Thing to decorate exported/imported backend functions.
//
//=====================================================================
// $Id: //devtools/main/CodeXL/Components/ShaderAnalyzer/AMDTBackEnd/Include/beAMDTBackEndDllBuild.h#1 $
// Last checkin:   $DateTime: 2016/02/28 16:32:28 $
// Last edited by: $Author: igal $
// Change list:    $Change: 561710 $
//=====================================================================

// TODO do we need another header for this?  I have duplicates.


#ifdef _WIN32
#pragma warning(disable:4005)
#if defined(AMDTBACKEND_EXPORTS)
    #define KA_BACKEND_DECLDIR __declspec(dllexport)
#elif defined(AMDTANALYSISBACKEND_STATIC)
    #define KA_BACKEND_DECLDIR
#else
    #define KA_BACKEND_DECLDIR __declspec(dllimport)
#endif
#else
// TODO We could use g++ __attribute syntax to control symbol visibility.
#  define KA_BACKEND_DECLDIR
#endif
