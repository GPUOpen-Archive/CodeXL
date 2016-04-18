//==================================================================================
// Copyright (c) 2013-2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file AMDTProfileDALDataTypes.h
///
//==================================================================================

#ifndef _AMDTPROFILEDALDATATYPES_H_
#define _AMDTPROFILEDALDATATYPES_H_

// This is only used between DAL & Adapter
struct PPSampleData
{
    int      m_quantizedTime;
    int      m_counterID;
    double   m_sampleValue;

    PPSampleData(int quantizedTime, int counterID, double sampleValue) :
        m_quantizedTime(quantizedTime), m_counterID(counterID), m_sampleValue(sampleValue) {}
};

#endif //_AMDTPROFILEDALDATATYPES_H_