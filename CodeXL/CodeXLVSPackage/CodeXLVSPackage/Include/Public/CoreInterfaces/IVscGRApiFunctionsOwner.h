//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file IVscGRApiFunctionsOwner.h
///
//==================================================================================

#ifndef IVscGRApiFunctionsOwner_h__
#define IVscGRApiFunctionsOwner_h__

class IVscGRApiFunctionsOwner
{
public:
    virtual ~IVscGRApiFunctionsOwner() {}
    virtual void SetHexDisplayMode() = 0;
};

#endif // IVscGRApiFunctionsOwner_h__
