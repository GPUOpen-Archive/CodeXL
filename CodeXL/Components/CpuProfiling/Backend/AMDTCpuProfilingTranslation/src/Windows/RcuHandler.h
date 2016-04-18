//==================================================================================
// Copyright (c) 2014-2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file RcuHandler.h
///
//==================================================================================

#ifndef _RCUHANDLER_H_
#define _RCUHANDLER_H_

#include <AMDTOSWrappers/Include/osOSDefinitions.h>

struct RcuData : SLIST_ENTRY
{
    // The size of this array is 1 although in reality it serves as an access point to the entire buffer that
    // is allocated during runtime with a size that will hold the CSS data read from the PRD file.
    gtUByte m_buffer[1];
};

/// This is an abstract class. Classes that derive from this class should be created by a factory
/// that derives from the RcuHandlerAbstractFactory class (see below).
class RcuHandler
{
public:
    virtual ~RcuHandler() {}

    // The Read operation is guaranteed to be thread safe.
    virtual bool Read(RcuData& data) = 0;

    // The Copy operation is not thread safe.
    virtual void Copy(RcuData*& pSrc, RcuData*& pDest) = 0;

    // The Update operation is guaranteed to be thread safe.
    virtual void Update(RcuData& data) = 0;
};

/// This class implements the Abstract Factory design pattern
class RcuHandlerAbstractFactory
{
public:
    virtual ~RcuHandlerAbstractFactory() {}

    virtual RcuHandler* Create() = 0;
};

#endif // _RCUHANDLER_H_
