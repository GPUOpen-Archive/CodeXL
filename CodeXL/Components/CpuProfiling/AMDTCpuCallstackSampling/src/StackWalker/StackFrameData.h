//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file StackFrameData.h
///
//==================================================================================

#ifndef _STACKFRAMEDATA_H_
#define _STACKFRAMEDATA_H_

#include <AMDTOSWrappers/Include/osOSDefinitions.h>

#ifndef FALSE
    #define FALSE 0
#endif

#ifndef TRUE
    #define TRUE 1
#endif

#if AMDT_CPP_COMPILER == AMDT_VISUAL_CPP_COMPILER
    #pragma warning(push)
    #pragma warning(disable : 4201) // nameless struct/union
#endif
struct StackFrameData
{
    StackFrameData()
    {
        m_valid.value = 0UL;
    }

    unsigned m_type;
    gtUInt32 m_size;

    gtVAddr m_base;

    gtVAddr m_returnAddress;
    gtVAddr m_localsBase;
    gtUInt32 m_lengthLocals;
    gtUInt32 m_lengthParams;
    gtUInt32 m_lengthProlog;
    gtUInt32 m_lengthSavedRegisters;
    gtUInt32 m_systemExceptionHandling : 1;
    gtUInt32 m_cplusplusExceptionHandling : 1;
    gtUInt32 m_functionStart : 1;
    gtUInt32 m_allocatesBasePointer : 1;
    gtUInt32 m_maxStack;

    union
    {
        struct
        {
            gtUInt32 type : 1;
            gtUInt32 base : 1;
            gtUInt32 size : 1;
            gtUInt32 returnAddress : 1;
            gtUInt32 localsBase : 1;
            gtUInt32 lengthLocals : 1;
            gtUInt32 lengthParams : 1;
            gtUInt32 lengthProlog : 1;
            gtUInt32 lengthSavedRegisters : 1;
            gtUInt32 systemExceptionHandling : 1;
            gtUInt32 cplusplusExceptionHandling : 1;
            gtUInt32 functionStart : 1;
            gtUInt32 allocatesBasePointer : 1;
            gtUInt32 maxStack : 1;
        };

        gtUInt32 value;
    } m_valid;
};


struct StackFrameControlData
{
    gtVAddr m_programCounter;
    gtVAddr m_stackPtr;
    gtVAddr m_framePtr;
};

#if AMDT_CPP_COMPILER == AMDT_VISUAL_CPP_COMPILER
    #pragma warning(pop)
#endif

#endif // _STACKFRAMEDATA_H_
