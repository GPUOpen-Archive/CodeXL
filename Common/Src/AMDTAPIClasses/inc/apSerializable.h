//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apSerializable.h
///
//==================================================================================

//------------------------------ apSerializable.h ------------------------------

#ifndef __APSERIALIZABLE
#define __APSERIALIZABLE
#include <AMDTBaseTools/Include/gtString.h>
// ----------------------------------------------------------------------------------
// Class Name:           apSerializable
// General Description:
//  Interface for serilizable objects
//
// Author:               AMD Developer Tools Team
// Creation Date:        27/12/2015
// ----------------------------------------------------------------------------------
class TiXmlElement;
class apXmlSerializable
{
public:
    virtual ~apXmlSerializable() {}
    virtual void Serialize(gtString& outString) = 0;
    virtual void DeSerialize(TiXmlElement* pNode) = 0;
};

#endif  // __APSERIALIZABLE
