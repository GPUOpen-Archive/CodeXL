//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apCLContextProperties.cpp
///
//==================================================================================

//------------------------------ apCLContextProperties.cpp -------------------

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTAPIClasses/Include/apOpenCLParameters.h>
#include <AMDTOSWrappers/Include/osChannel.h>
#include <AMDTOSWrappers/Include/osChannelOperators.h>
#include <AMDTOSWrappers/Include/osOSDefinitions.h>

// Local:
#include <AMDTAPIClasses/Include/apCLContextProperties.h>


// ---------------------------------------------------------------------------
// Name:        apCLContextProperties::apCLContextProperties
// Description: Constructor.
// Arguments:   The OpenCL pointer holding the context creation properties data
// Author:  AMD Developer Tools Team
// Date:        8/4/2010
// ---------------------------------------------------------------------------
apCLContextProperties::apCLContextProperties(cl_context_properties* pContextProperties)
{
    // Initialize the context properties from the data:
    initFromCLContextProperties(pContextProperties);
}


// ---------------------------------------------------------------------------
// Name:        apCLContextProperties::apCLContextProperties
// Description: Copy constructor
// Arguments: other - The other context information class from which I am created.
// Author:  AMD Developer Tools Team
// Date:        8/4/2010
// ---------------------------------------------------------------------------
apCLContextProperties::apCLContextProperties(const apCLContextProperties& other)
{
    this->operator=(other);
}


// ---------------------------------------------------------------------------
// Name:        apCLContextProperties::~apCLContext
// Description: Destructor.
// Author:  AMD Developer Tools Team
// Date:        8/4/2010
// ---------------------------------------------------------------------------
apCLContextProperties::~apCLContextProperties()
{
}


// ---------------------------------------------------------------------------
// Name:        apCLContextProperties::operator=
// Description: Copies information from another class instance into me.
// Author:  AMD Developer Tools Team
// Date:        8/4/2010
// ---------------------------------------------------------------------------
apCLContextProperties& apCLContextProperties::operator=(const apCLContextProperties& other)
{
    // Clear the current properties vector:
    _properties.clear();

    // Copy the properties:
    for (int i = 0 ; i < (int)other._properties.size(); i++)
    {
        // Copy current property:
        apCLProperty property = other._properties[i];
        _properties.push_back(property);
    }

    setAllocatedObjectId(other.getAllocatedObjectId(), true);
    return *this;
}


// ---------------------------------------------------------------------------
// Name:        apCLContextProperties::readSelfFromChannel
// Description: Writes this class data into ipcChannel.
// Author:  AMD Developer Tools Team
// Date:        8/4/2010
// ---------------------------------------------------------------------------
bool apCLContextProperties::writeSelfIntoChannel(osChannel& ipcChannel) const
{
    // Write the amount of properties:
    gtSize_t propListSize = _properties.size();
    ipcChannel << (gtUInt64)propListSize;

    // Write each of the properties:
    for (gtSize_t i = 0; i < propListSize; i++)
    {
        // Get the current property:
        apCLProperty property = _properties[i];

        // Write the property:
        ipcChannel << (gtUInt32)property.first;
        ipcChannel << (gtUInt64)property.second;
    }

    // Write base class data:
    bool retVal = apAllocatedObject::writeSelfIntoChannel(ipcChannel);

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        apCLContextProperties::readSelfFromChannel
// Description: Reads this class data from ipcChannel.
// Author:  AMD Developer Tools Team
// Date:        8/4/2010
// ---------------------------------------------------------------------------
bool apCLContextProperties::readSelfFromChannel(osChannel& ipcChannel)
{
    // Read the context properties list:
    _properties.clear();
    gtUInt64 propListSize = 0;
    ipcChannel >> propListSize;

    for (gtUInt64 i = 0; i < propListSize; i++)
    {
        gtUInt32 currPropertyNameAsUInt32 = 0;
        ipcChannel >> currPropertyNameAsUInt32;
        gtUInt64 currPropertyValueAsUInt64 = 0;
        ipcChannel >> currPropertyValueAsUInt64;

        apCLProperty currentProperty((cl_uint)currPropertyNameAsUInt32, (oaCLContextProperty)currPropertyValueAsUInt64);
        _properties.push_back(currentProperty);
    }

    // Read base class data:
    bool retVal = apAllocatedObject::readSelfFromChannel(ipcChannel);

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        apCLContextProperties::initFromCLContextProperties
// Description: Initializes the properties from OpenCL context properties structure
// Arguments:   cl_context_properties* pContextProperties
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        8/4/2010
// ---------------------------------------------------------------------------
bool apCLContextProperties::initFromCLContextProperties(cl_context_properties* pContextProperties, bool hideForcedProperties)
{
    bool retVal = true;

    if (pContextProperties != NULL)
    {
        // Clear the current properties:
        _properties.clear();

        // Read the properties one by one:
        cl_context_properties* pIter = pContextProperties;

        while (*pIter != 0)
        {
            // Read the property name:
            cl_uint propertyName = (cl_uint)(*pIter);

            // Read the property value:
            pIter ++;

            oaCLContextProperty propertyValue = (oaCLContextProperty)(*pIter);

            if ((!hideForcedProperties) ||
                ((CL_CONTEXT_COMMAND_INTERCEPT_CALLBACK_AMD != propertyName)
                 /* && (CL_CONTEXT_OFFLINE_DEVICES_AMD != propertyName) // Internal property, not currently forced by CodeXL debugger */
                ))
            {
                // Insert the current property to the vector:
                _properties.push_back(apCLProperty(propertyName, propertyValue));
            }

            // Increment the iterator:
            pIter ++;
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        apCLContextProperties::clPropertyAsString
// Description: Return the property i's strings (name + value)
// Arguments:   int propertyIndex
//            gtString& propertyName
//            gtString& propertyValue
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        11/4/2010
// ---------------------------------------------------------------------------
bool apCLContextProperties::clPropertyAsString(int propertyIndex, gtString& propertyName, gtString& propertyValue) const
{
    bool retVal = false;

    // Sanity check:
    GT_IF_WITH_ASSERT((propertyIndex >= 0) && (propertyIndex <= (int)_properties.size()))
    {
        retVal = true;
        // Get the requested property:
        apCLProperty property = _properties[propertyIndex];

        // Get the property name:
        cl_uint name = property.first;

        switch (name)
        {
            // Expected values, get the enum name:
            case CL_CONTEXT_PLATFORM: // Platform id as context property
            case CL_CONTEXT_INTEROP_USER_SYNC:
            case CL_GL_CONTEXT_KHR: // The OpenGL context

            // The OpenGL context device context
            case CL_EGL_DISPLAY_KHR: // Embedded
            case CL_GLX_DISPLAY_KHR: // Linux
            case CL_WGL_HDC_KHR: // Windows
            case CL_CGL_SHAREGROUP_KHR: // Mac
            {
                // Get the property name as string:
                apCLEnumValueToString(name, propertyName);
            }
            break;

            default:
            {
                propertyName.makeEmpty().appendFormattedString(L"Unknown context property (%#06x)", name);
            }
            break;
        }

        // The property value is always displayed as a number:
        oaCLContextProperty value = property.second;
        propertyValue.appendFormattedString(L"%#010llx", value);
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        apCLContextProperties::clPropertyValue
// Description: Return a context property value by the property name
// Arguments:   cl_uint propertyName
// Return Val:  oaCLContextProperty
// Author:  AMD Developer Tools Team
// Date:        21/7/2010
// ---------------------------------------------------------------------------
oaCLContextProperty apCLContextProperties::clPropertyValue(cl_uint propertyName) const
{
    oaCLContextProperty retVal = 0;

    for (int i = 0 ; i < (int)_properties.size(); i++)
    {
        // Get the current property:
        apCLProperty currentProperty = _properties[i];

        if (currentProperty.first == propertyName)
        {
            retVal = currentProperty.second;
        }
    }

    return retVal;
}

