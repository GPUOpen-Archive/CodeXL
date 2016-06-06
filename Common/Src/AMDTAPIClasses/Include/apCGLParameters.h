//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apCGLParameters.h
///
//==================================================================================

//------------------------------ apCGLParameters.h ------------------------------

#ifndef __APCGLPARAMETERS_H
#define __APCGLPARAMETERS_H

// Infra:
#include <Carbon/Carbon.h>
#include <AMDTOSAPIWrappers/Include/oaOpenGLIncludes.h>

// Local:
#include <AMDTAPIClasses/Include/apBasicParameters.h>

// ----------------------------------------------------------------------------------
// Class Name:           AP_API apCGLPixelFormatAttributeParameter : public apParameter
// General Description:  Represents a CGLPixelFormatAttribute parameter (CGL enumeration).
// Author:  AMD Developer Tools Team
// Creation Date:        14/12/2008
// ----------------------------------------------------------------------------------
class AP_API apCGLPixelFormatAttributeParameter : public apParameter
{
public:
    // Self functions:
    apCGLPixelFormatAttributeParameter(CGLPixelFormatAttribute value = kCGLPFAAllRenderers) : _value(value) {};
    CGLPixelFormatAttribute value() const { return _value; };

    // Overrides osTransferableObject:
    virtual osTransferableObjectType type() const;
    virtual bool writeSelfIntoChannel(osChannel& ipcChannel) const;
    virtual bool readSelfFromChannel(osChannel& ipcChannel);

    // Overrides apParameter:
    virtual void readValueFromArgumentsList(va_list& pArgumentList);
    virtual void readValueFromPointer(void* pValue);
    virtual gtSizeType sizeofData();
    virtual void valueAsString(gtString& valueString) const;
    virtual bool compareToOther(const apParameter& other)const;

private:
    // The parameter value:
    CGLPixelFormatAttribute _value;
};

// ----------------------------------------------------------------------------------
// Class Name:           AP_API apCGLContextEnableParameter : public apParameter
// General Description:  Represents a apCGLContextEnableParameter parameter (CGL enumeration).
// Author:  AMD Developer Tools Team
// Creation Date:        14/12/2008
// ----------------------------------------------------------------------------------
class AP_API apCGLContextEnableParameter : public apParameter
{
public:
    // Self functions:
    apCGLContextEnableParameter(CGLContextEnable value =  kCGLCESwapRectangle) : _value(value) {};
    CGLContextEnable value() const { return _value; };

    // Overrides osTransferableObject:
    virtual osTransferableObjectType type() const;
    virtual bool writeSelfIntoChannel(osChannel& ipcChannel) const;
    virtual bool readSelfFromChannel(osChannel& ipcChannel);

    // Overrides apParameter:
    virtual void readValueFromArgumentsList(va_list& pArgumentList);
    virtual void readValueFromPointer(void* pValue);
    virtual gtSizeType sizeofData();
    virtual void valueAsString(gtString& valueString) const;
    virtual bool compareToOther(const apParameter& other)const;

private:
    // The parameter value:
    CGLContextEnable _value;
};


// ----------------------------------------------------------------------------------
// Class Name:           AP_API apCGLContextParameterParameter : public apParameter
// General Description:  Represents a CGLContextParameter parameter (CGL enumeration).
// Author:  AMD Developer Tools Team
// Creation Date:        14/12/2008
// ----------------------------------------------------------------------------------
class AP_API apCGLContextParameterParameter : public apParameter
{
public:
    // Self functions:
    apCGLContextParameterParameter(CGLContextParameter value =   kCGLCPSwapRectangle) : _value(value) {};
    CGLContextParameter value() const { return _value; };

    // Overrides osTransferableObject:
    virtual osTransferableObjectType type() const;
    virtual bool writeSelfIntoChannel(osChannel& ipcChannel) const;
    virtual bool readSelfFromChannel(osChannel& ipcChannel);

    // Overrides apParameter:
    virtual void readValueFromArgumentsList(va_list& pArgumentList);
    virtual void readValueFromPointer(void* pValue);
    virtual gtSizeType sizeofData();
    virtual void valueAsString(gtString& valueString) const;
    virtual bool compareToOther(const apParameter& other)const;

private:
    // The parameter value:
    CGLContextParameter _value;
};

// ----------------------------------------------------------------------------------
// Class Name:           AP_API apCGLGlobalOptionParameter : public apParameter
// General Description:  Represents a apCGLGlobalOptionParameter parameter (CGL enumeration).
// Author:  AMD Developer Tools Team
// Creation Date:        14/12/2008
// ----------------------------------------------------------------------------------
class AP_API apCGLGlobalOptionParameter : public apParameter
{
public:
    // Self functions:
    apCGLGlobalOptionParameter(CGLGlobalOption value = kCGLGOFormatCacheSize) : _value(value) {};
    CGLGlobalOption value() const { return _value; };

    // Overrides osTransferableObject:
    virtual osTransferableObjectType type() const;
    virtual bool writeSelfIntoChannel(osChannel& ipcChannel) const;
    virtual bool readSelfFromChannel(osChannel& ipcChannel);

    // Overrides apParameter:
    virtual void readValueFromArgumentsList(va_list& pArgumentList);
    virtual void readValueFromPointer(void* pValue);
    virtual gtSizeType sizeofData();
    virtual void valueAsString(gtString& valueString) const;
    virtual bool compareToOther(const apParameter& other)const;

private:
    // The parameter value:
    CGLGlobalOption _value;
};

// ----------------------------------------------------------------------------------
// Class Name:           AP_API apCGLRendererPropertyParameter : public apParameter
// General Description:  Represents a apCGLRendererPropertyParameter parameter (CGL enumeration).
// Author:  AMD Developer Tools Team
// Creation Date:        14/12/2008
// ----------------------------------------------------------------------------------
class AP_API apCGLRendererPropertyParameter : public apParameter
{
public:
    // Self functions:
    apCGLRendererPropertyParameter(CGLRendererProperty value = kCGLRPOffScreen) : _value(value) {};
    CGLRendererProperty value() const { return _value; };

    // Overrides osTransferableObject:
    virtual osTransferableObjectType type() const;
    virtual bool writeSelfIntoChannel(osChannel& ipcChannel) const;
    virtual bool readSelfFromChannel(osChannel& ipcChannel);

    // Overrides apParameter:
    virtual void readValueFromArgumentsList(va_list& pArgumentList);
    virtual void readValueFromPointer(void* pValue);
    virtual gtSizeType sizeofData();
    virtual void valueAsString(gtString& valueString) const;
    virtual bool compareToOther(const apParameter& other)const;

private:
    // The parameter value:
    CGLRendererProperty _value;
};

// ----------------------------------------------------------------------------------
// Class Name:           AP_API apCGLBufferModeMaskParameter : public apParameter
// General Description:  Represents a apCGLBufferModeMaskParameter parameter (CGL mask).
// Author:  AMD Developer Tools Team
// Creation Date:        14/12/2008
// ----------------------------------------------------------------------------------
class AP_API apCGLBufferModeMaskParameter : public apParameter
{
public:
    // Self functions:
    apCGLBufferModeMaskParameter(unsigned int value = 0) : _value(value) {};
    apCGLBufferModeMaskParameter value() const { return _value; };

    // Overrides osTransferableObject:
    virtual osTransferableObjectType type() const;
    virtual bool writeSelfIntoChannel(osChannel& ipcChannel) const;
    virtual bool readSelfFromChannel(osChannel& ipcChannel);

    // Overrides apParameter:
    virtual void readValueFromArgumentsList(va_list& pArgumentList);
    virtual void readValueFromPointer(void* pValue);
    virtual gtSizeType sizeofData();
    virtual void valueAsString(gtString& valueString) const;
    virtual bool compareToOther(const apParameter& other)const;

private:
    // The parameter value:
    unsigned int _value;
};


// ----------------------------------------------------------------------------------
// Class Name:           AP_API apCGLColorBufferFormatMaskParameter : public apParameter
// General Description:  Represents a apCGLColorBufferFormatMaskParameter parameter (CGL mask).
// Author:  AMD Developer Tools Team
// Creation Date:        15/12/2008
// ----------------------------------------------------------------------------------
class AP_API apCGLColorBufferFormatMaskParameter : public apParameter
{
public:
    // Self functions:
    apCGLColorBufferFormatMaskParameter(unsigned int value = 0) : _value(value) {};
    apCGLColorBufferFormatMaskParameter value() const { return _value; };

    // Overrides osTransferableObject:
    virtual osTransferableObjectType type() const;
    virtual bool writeSelfIntoChannel(osChannel& ipcChannel) const;
    virtual bool readSelfFromChannel(osChannel& ipcChannel);

    // Overrides apParameter:
    virtual void readValueFromArgumentsList(va_list& pArgumentList);
    virtual void readValueFromPointer(void* pValue);
    virtual gtSizeType sizeofData();
    virtual void valueAsString(gtString& valueString) const;
    virtual bool compareToOther(const apParameter& other)const;

private:
    // The parameter value:
    unsigned int _value;
};

// ----------------------------------------------------------------------------------
// Class Name:           AP_API apCGLRendererIDParameter : public apParameter
// General Description:  Represents a apCGLRendererIDParameter parameter (CGL mask).
// Author:  AMD Developer Tools Team
// Creation Date:        15/12/2008
// ----------------------------------------------------------------------------------
class AP_API apCGLRendererIDParameter : public apParameter
{
public:
    // Self functions:
    apCGLRendererIDParameter(unsigned int value = 0) : _value(value) {};
    apCGLRendererIDParameter value() const { return _value; };

    // Overrides osTransferableObject:
    virtual osTransferableObjectType type() const;
    virtual bool writeSelfIntoChannel(osChannel& ipcChannel) const;
    virtual bool readSelfFromChannel(osChannel& ipcChannel);

    // Overrides apParameter:
    virtual void readValueFromArgumentsList(va_list& pArgumentList);
    virtual void readValueFromPointer(void* pValue);
    virtual gtSizeType sizeofData();
    virtual void valueAsString(gtString& valueString) const;
    virtual bool compareToOther(const apParameter& other)const;

private:
    // The parameter value:
    unsigned int _value;
};

// ----------------------------------------------------------------------------------
// Class Name:           AP_API apCGLSamplingModeMaskParameter : public apParameter
// General Description:  Represents a apCGLSamplingModeMaskParameter parameter (CGL mask).
// Author:  AMD Developer Tools Team
// Creation Date:        15/12/2008
// ----------------------------------------------------------------------------------
class AP_API apCGLSamplingModeMaskParameter : public apParameter
{
public:
    // Self functions:
    apCGLSamplingModeMaskParameter(unsigned int value = 0) : _value(value) {};
    apCGLSamplingModeMaskParameter value() const { return _value; };

    // Overrides osTransferableObject:
    virtual osTransferableObjectType type() const;
    virtual bool writeSelfIntoChannel(osChannel& ipcChannel) const;
    virtual bool readSelfFromChannel(osChannel& ipcChannel);

    // Overrides apParameter:
    virtual void readValueFromArgumentsList(va_list& pArgumentList);
    virtual void readValueFromPointer(void* pValue);
    virtual gtSizeType sizeofData();
    virtual void valueAsString(gtString& valueString) const;
    virtual bool compareToOther(const apParameter& other)const;

private:
    // The parameter value:
    unsigned int _value;
};


// ----------------------------------------------------------------------------------
// Class Name:           AP_API apCGLStencilAndDepthModeMaskParameter : public apParameter
// General Description:  Represents a apCGLStencilAndDepthModeMaskParameter parameter (CGL mask).
// Author:  AMD Developer Tools Team
// Creation Date:        15/12/2008
// ----------------------------------------------------------------------------------
class AP_API apCGLStencilAndDepthModeMaskParameter : public apParameter
{
public:
    // Self functions:
    apCGLStencilAndDepthModeMaskParameter(unsigned int value = 0) : _value(value) {};
    apCGLStencilAndDepthModeMaskParameter value() const { return _value; };

    // Overrides osTransferableObject:
    virtual osTransferableObjectType type() const;
    virtual bool writeSelfIntoChannel(osChannel& ipcChannel) const;
    virtual bool readSelfFromChannel(osChannel& ipcChannel);

    // Overrides apParameter:
    virtual void readValueFromArgumentsList(va_list& pArgumentList);
    virtual void readValueFromPointer(void* pValue);
    virtual gtSizeType sizeofData();
    virtual void valueAsString(gtString& valueString) const;
    virtual bool compareToOther(const apParameter& other)const;

private:
    // The parameter value:
    unsigned int _value;
};


#endif //__APCGLPARAMETERS_H

