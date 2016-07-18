//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apCLContextProperties.h
///
//==================================================================================

//------------------------------ apCLContextProperties.h ----------------------------

#ifndef __APCLCONTEXT_PROPERTIES_H
#define __APCLCONTEXT_PROPERTIES_H

// Infra:
#include <AMDTBaseTools/Include/gtPtrVector.h>
#include <AMDTBaseTools/Include/gtString.h>
#include <AMDTOSAPIWrappers/Include/oaOpenCLIncludes.h>
#include <AMDTOSAPIWrappers/Include/oaOSAPIDefinitions.h>

// Local:
#include <AMDTAPIClasses/Include/apAPIClassesDLLBuild.h>
#include <AMDTAPIClasses/Include/apAllocatedObject.h>


// ----------------------------------------------------------------------------------
// Class Name:           AP_API apCLContextProperties
// General Description:  Holds the information of an OpenCL Context creation properties.
// Author:  AMD Developer Tools Team
// Creation Date:        8/4/2010
// ----------------------------------------------------------------------------------
class AP_API apCLContextProperties : public apAllocatedObject
{
public:
    apCLContextProperties(cl_context_properties* pContextProperties = NULL);
    apCLContextProperties(const apCLContextProperties& other);
    virtual ~apCLContextProperties();
    apCLContextProperties& operator=(const apCLContextProperties& other);

    // Amount of properties in struct:
    int amountOfProperties() const {return (int)_properties.size();};

    // Return a single property as string:
    bool clPropertyAsString(int propertyIndex, gtString& propertyName, gtString& propertyValue) const ;

    // Return single property value:
    oaCLContextProperty clPropertyValue(cl_uint propertyName) const ;

    // Init from cl_context_properties:
    bool initFromCLContextProperties(cl_context_properties* pContextProperties, bool hideForcedProperties = true);

    // Overrides osTransferableObject:
    virtual osTransferableObjectType type() const { return OS_TOBJ_ID_CL_CONTEXT_PROPERTIES; };
    virtual bool writeSelfIntoChannel(osChannel& ipcChannel) const;
    virtual bool readSelfFromChannel(osChannel& ipcChannel);

private:
    typedef std::pair<cl_uint, oaCLContextProperty> apCLProperty;
    gtVector<apCLProperty> _properties;
};


#endif //__APCLCONTEXT_PROPERTIES_H

