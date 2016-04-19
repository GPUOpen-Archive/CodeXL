//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file afHTMLUtils.h
///
//==================================================================================

#ifndef __AFHTMLUTILS_H
#define __AFHTMLUTILS_H

class apDebugProjectSettings;
class apDebuggedProcessRunStartedEvent;
class apDebuggedProcessTerminatedEvent;
class apDebuggedProcessCreatedEvent;

// Local:
#include <AMDTApplicationFramework/Include/afApplicationFrameworkDLLBuild.h>

/// -----------------------------------------------------------------------------------------------
/// \class Name: AF_API afHTMLUtils
/// \brief Description:  Used for creation of HTML properties strings
/// -----------------------------------------------------------------------------------------------
class AF_API afHTMLUtils
{

public:
    afHTMLUtils();

    void buildProcessRunStartedEventPropertiesString(const apDebugProjectSettings& processStartedData , const apDebuggedProcessRunStartedEvent& processRunStartedEvent, gtString& propertiesHTMLMessage);
    void buildProcessTerminationEventPropertiesString(const apDebugProjectSettings& processStartedData, const apDebuggedProcessTerminatedEvent& processTerminationEvent, gtString& propertiesHTMLMessage);
    void buildProcessCreatedEventPropertiesString(const apDebugProjectSettings& processCreationData, const apDebuggedProcessCreatedEvent& processCreatedEvent, gtString& propertiesHTMLMessage);

};


#endif //__AFHTMLUTILS_H

