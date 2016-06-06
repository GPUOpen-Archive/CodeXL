//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apOpenCLParameters.h
///
//==================================================================================

//------------------------------ apOpenCLParameters.h ------------------------------

#ifndef __APOPENCLPARAMETERS
#define __APOPENCLPARAMETERS

// OpenCL and OpenGL:
#include <AMDTOSAPIWrappers/Include/oaOpenCLIncludes.h>
//#include <AMDTOSAPIWrappers/Include/oaOpenGLIncludes.h>

// Infra:
#include <AMDTBaseTools/Include/gtVector.h>
#include <AMDTOSAPIWrappers/Include/oaOSAPIDefinitions.h>

// Local:
#include <AMDTAPIClasses/Include/apBasicParameters.h>
#include <AMDTAPIClasses/Include/apOpenGLParameters.h>


// ------------------- OpenCL parameters ---------------------

// Translate enumerator values:
bool AP_API apCLEnumValueToString(cl_uint value, gtString& valueAsString);

// Global function for bitfield parameters:
void apAddCLBitfieldBitToValueString(cl_bitfield value, cl_bitfield bit, const wchar_t* bitAsString, gtString& valueString, bool& wasBitEncountered, bool pipeAsSeparator = true);

// ----------------------------------------------------------------------------------
// Class Name:           AP_API apCLuintParameter : public apParameter
// General Description: Represents a cl_uint parameter (OpencL unsigned integer).
// Author:  AMD Developer Tools Team
// Creation Date:        29/10/2009
// ----------------------------------------------------------------------------------
class AP_API apOpenCLPixelValueParameter : public apPixelValueParameter
{
    // Uri, 5/4/15 - add error checking here
    void setValueFromInt(GLint) {};
    void setValueFromFloat(GLfloat) {};
};

// ----------------------------------------------------------------------------------
// Class Name:           AP_API apCLuintParameter : public apParameter
// General Description: Represents a cl_uint parameter (OpencL unsigned integer).
// Author:  AMD Developer Tools Team
// Creation Date:        29/10/2009
// ----------------------------------------------------------------------------------
class AP_API apCLuintParameter : public apOpenCLPixelValueParameter
{
public:
    // Self functions:
    apCLuintParameter(cl_uint value = 0) : _value(value) { };
    virtual ~apCLuintParameter() {};
    cl_uint value() const { return _value; };

    // Overrides osTransferableObject:
    virtual osTransferableObjectType type() const;
    virtual bool writeSelfIntoChannel(osChannel& ipcChannel) const;
    virtual bool readSelfFromChannel(osChannel& ipcChannel);

    // Overrides apParameter:
    virtual void readValueFromArgumentsList(va_list& pArgumentList);
    virtual void readValueFromPointer(void* pValue);
    virtual gtSizeType sizeofData();
    virtual void valueAsString(gtString& valueString) const;
    virtual void valueAsHexString(gtString& valueString) const;
    virtual bool compareToOther(const apParameter& other)const;

    // Overrides apPixelValueParameter:
    virtual char asPixelValue() const;
    virtual void shiftAndMaskBits(unsigned int bitsMask, int amountOfBitsToShiftRight);
    virtual void setValueFromDouble(double value);
    virtual double valueAsDouble();

private:
    // The parameter value:
    cl_uint _value;
};

// ----------------------------------------------------------------------------------
// Class Name:           AP_API apCLintParameter : public apPixelValueParameter
// General Description: Represents a cl_int parameter (OpencL integer).
// Author:  AMD Developer Tools Team
// Creation Date:        29/10/2009
// ----------------------------------------------------------------------------------
class AP_API apCLintParameter : public apOpenCLPixelValueParameter
{
public:
    // Self functions:
    apCLintParameter(cl_int value = 0) : _value(value) { };
    virtual ~apCLintParameter() {};
    cl_int value() const { return _value; };

    // Overrides osTransferableObject:
    virtual osTransferableObjectType type() const;
    virtual bool writeSelfIntoChannel(osChannel& ipcChannel) const;
    virtual bool readSelfFromChannel(osChannel& ipcChannel);

    // Overrides apParameter:
    virtual void readValueFromArgumentsList(va_list& pArgumentList);
    virtual void readValueFromPointer(void* pValue);
    virtual gtSizeType sizeofData();
    virtual void valueAsString(gtString& valueString) const;
    virtual void valueAsHexString(gtString& valueString) const;
    virtual bool compareToOther(const apParameter& other)const;

    // Overrides apPixelValueParameter:
    virtual char asPixelValue() const;
    virtual void shiftAndMaskBits(unsigned int bitsMask, int amountOfBitsToShiftRight);
    virtual void setValueFromDouble(double value);
    virtual double valueAsDouble();

private:
    // The parameter value:
    cl_int _value;
};


// ----------------------------------------------------------------------------------
// Class Name:           AP_API apCLulongParameter : public apPixelValueParameter
// General Description: Represents a cl_ulong parameter (OpencL unsigned long).
// Author:  AMD Developer Tools Team
// Creation Date:        27/3/2011
// ----------------------------------------------------------------------------------
class AP_API apCLulongParameter : public apOpenCLPixelValueParameter
{
public:
    // Self functions:
    apCLulongParameter(cl_ulong value = 0) : _value(value) { };
    virtual ~apCLulongParameter() {};
    cl_ulong value() const { return _value; };

    // Overrides osTransferableObject:
    virtual osTransferableObjectType type() const;
    virtual bool writeSelfIntoChannel(osChannel& ipcChannel) const;
    virtual bool readSelfFromChannel(osChannel& ipcChannel);

    // Overrides apParameter:
    virtual void readValueFromArgumentsList(va_list& pArgumentList);
    virtual void readValueFromPointer(void* pValue);
    virtual gtSizeType sizeofData();
    virtual void valueAsString(gtString& valueString) const;
    virtual void valueAsHexString(gtString& valueString) const;
    virtual bool compareToOther(const apParameter& other)const;

    // Overrides apPixelValueParameter:
    virtual char asPixelValue() const;
    virtual void shiftAndMaskBits(unsigned int bitsMask, int amountOfBitsToShiftRight);
    virtual void setValueFromDouble(double value);
    virtual double valueAsDouble();

private:
    // The parameter value:
    cl_ulong _value;
};


// ----------------------------------------------------------------------------------
// Class Name:           AP_API apCLlongParameter : public apParameter
// General Description: Represents a cl_long parameter (OpencL long).
// Author:  AMD Developer Tools Team
// Creation Date:        27/3/2011
// ----------------------------------------------------------------------------------
class AP_API apCLlongParameter : public apOpenCLPixelValueParameter
{
public:
    // Self functions:
    apCLlongParameter(cl_long value = 0) : _value(value) { };
    virtual ~apCLlongParameter() {};
    cl_long value() const { return _value; };

    // Overrides osTransferableObject:
    virtual osTransferableObjectType type() const;
    virtual bool writeSelfIntoChannel(osChannel& ipcChannel) const;
    virtual bool readSelfFromChannel(osChannel& ipcChannel);

    // Overrides apParameter:
    virtual void readValueFromArgumentsList(va_list& pArgumentList);
    virtual void readValueFromPointer(void* pValue);
    virtual gtSizeType sizeofData();
    virtual void valueAsString(gtString& valueString) const;
    virtual void valueAsHexString(gtString& valueString) const;
    virtual bool compareToOther(const apParameter& other)const;

    // Overrides apPixelValueParameter:
    virtual char asPixelValue() const;
    virtual void shiftAndMaskBits(unsigned int bitsMask, int amountOfBitsToShiftRight);
    virtual void setValueFromDouble(double value);
    virtual double valueAsDouble();

private:
    // The parameter value:
    cl_long _value;
};


// ----------------------------------------------------------------------------------
// Class Name:           AP_API apCLucharParameter : public apPixelValueParameter
// General Description: Represents a cl_ulong parameter (OpencL unsigned char).
// Author:  AMD Developer Tools Team
// Creation Date:        27/3/2011
// ----------------------------------------------------------------------------------
class AP_API apCLucharParameter : public apOpenCLPixelValueParameter
{
public:
    // Self functions:
    apCLucharParameter(cl_uchar value = 0) : _value(value) { };
    virtual ~apCLucharParameter() {};
    cl_uchar value() const { return _value; };

    // Overrides osTransferableObject:
    virtual osTransferableObjectType type() const;
    virtual bool writeSelfIntoChannel(osChannel& ipcChannel) const;
    virtual bool readSelfFromChannel(osChannel& ipcChannel);

    // Overrides apParameter:
    virtual void readValueFromArgumentsList(va_list& pArgumentList);
    virtual void readValueFromPointer(void* pValue);
    virtual gtSizeType sizeofData();
    virtual void valueAsString(gtString& valueString) const;
    virtual void valueAsHexString(gtString& valueString) const;
    virtual bool compareToOther(const apParameter& other)const;

    // Overrides apPixelValueParameter:
    virtual char asPixelValue() const;
    virtual void shiftAndMaskBits(unsigned int bitsMask, int amountOfBitsToShiftRight);
    virtual void setValueFromDouble(double value);
    virtual double valueAsDouble();

private:
    // The parameter value:
    cl_uchar _value;
};


// ----------------------------------------------------------------------------------
// Class Name:           AP_API apCLcharParameter : public apParameter
// General Description: Represents a cl_long parameter (OpenCL char).
// Author:  AMD Developer Tools Team
// Creation Date:        27/3/2011
// ----------------------------------------------------------------------------------
class AP_API apCLcharParameter : public apOpenCLPixelValueParameter
{
public:
    // Self functions:
    apCLcharParameter(cl_char value = 0) : _value(value) { };
    virtual ~apCLcharParameter() {};
    cl_char value() const { return _value; };

    // Overrides osTransferableObject:
    virtual osTransferableObjectType type() const;
    virtual bool writeSelfIntoChannel(osChannel& ipcChannel) const;
    virtual bool readSelfFromChannel(osChannel& ipcChannel);

    // Overrides apParameter:
    virtual void readValueFromArgumentsList(va_list& pArgumentList);
    virtual void readValueFromPointer(void* pValue);
    virtual gtSizeType sizeofData();
    virtual void valueAsString(gtString& valueString) const;
    virtual void valueAsHexString(gtString& valueString) const;
    virtual bool compareToOther(const apParameter& other)const;

    // Overrides apPixelValueParameter:
    virtual char asPixelValue() const;
    virtual void shiftAndMaskBits(unsigned int bitsMask, int amountOfBitsToShiftRight);
    virtual void setValueFromDouble(double value);
    virtual double valueAsDouble();

private:
    // The parameter value:
    cl_char _value;
};


// ----------------------------------------------------------------------------------
// Class Name:           AP_API apCLBoolParameter : public apParameter
// General Description: Represents a cl_bool parameter (OpenCL bool).
// Author:  AMD Developer Tools Team
// Creation Date:        8/11/2009
// ----------------------------------------------------------------------------------
class AP_API apCLBoolParameter : public apParameter
{
public:
    // Self functions:
    apCLBoolParameter(cl_bool value = 0) : _value(value) { };
    virtual ~apCLBoolParameter() {};
    cl_bool value() const { return _value; };

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
    cl_bool _value;
};


// ----------------------------------------------------------------------------------
// Class Name:           AP_API apCLMemFlags : public apParameter
// General Description:
//   Represents OpenCL memory flags.
//
// Author:  AMD Developer Tools Team
// Creation Date:        18/11/2009
// ----------------------------------------------------------------------------------
class AP_API apCLMemFlags : public apBitfieldParameter
{
public:
    apCLMemFlags(cl_mem_flags memoryFlags = 0);
    virtual ~apCLMemFlags();

    // Overrides osTransferableObject:
    virtual osTransferableObjectType type() const;
    virtual bool writeSelfIntoChannel(osChannel& ipcChannel) const;
    virtual bool readSelfFromChannel(osChannel& ipcChannel);

    // Flags:
    void setFlags(cl_mem_flags flags) {_value = flags;};
    cl_mem_flags getFlags() {return _value;};

    // Overrides apParameter:
    virtual void readValueFromArgumentsList(va_list& pArgumentList);
    virtual void readValueFromPointer(void* pValue);
    virtual gtSizeType sizeofData();
    // virtual void valueAsString(gtString& valueString) const; // Handled by apBitfieldParameter
    virtual bool compareToOther(const apParameter& other)const;

protected:
    // Overrides apBitfieldParameter
    virtual bool getSpecialValue(gtString& o_string) const;
    virtual gtUInt64 getBitfieldAsUInt64() const;
    virtual bool getBitName(int bitIndex, gtString& o_string) const;

private:
    // The OpenCL memory flags bitfield:
    cl_mem_flags _value;
};

// ----------------------------------------------------------------------------------
// Class Name:           AP_API apCLSVMMemFlagsParameter : public apCLMemFlags
// General Description:
//   Represents OpenCL SVM memory flags.
//
// Author:  AMD Developer Tools Team
// Creation Date:        28/9/2014
// ----------------------------------------------------------------------------------
class AP_API apCLSVMMemFlagsParameter : public apCLMemFlags
{
public:
    apCLSVMMemFlagsParameter(cl_svm_mem_flags memoryFlags = 0);
    virtual ~apCLSVMMemFlagsParameter();

    // Overrides apCLMemFlags:
    virtual osTransferableObjectType type() const;
    virtual bool getBitName(int bitIndex, gtString& o_string) const;
};


// ----------------------------------------------------------------------------------
// Class Name:           AP_API apCLGLObjectTypeParameter : public apParameter
// General Description: Represents a cl_gl_object_type parameter (OpenGL object type).
// Author:  AMD Developer Tools Team
// Creation Date:        11/11/2009
// ----------------------------------------------------------------------------------
class AP_API apCLGLObjectTypeParameter : public apParameter
{
public:
    // Self functions:
    apCLGLObjectTypeParameter(cl_gl_object_type value = 0) : _value(value) { };
    virtual ~apCLGLObjectTypeParameter() {};
    cl_gl_object_type value() const { return _value; };

    // Overrides osTransferableObject:
    virtual osTransferableObjectType type() const;
    virtual bool writeSelfIntoChannel(osChannel& ipcChannel) const;
    virtual bool readSelfFromChannel(osChannel& ipcChannel);

    // Overrides apParameter:
    virtual void readValueFromArgumentsList(va_list& pArgumentList);
    virtual void readValueFromPointer(void* pValue);
    virtual gtSizeType sizeofData();
    virtual void valueAsString(gtString& valueString) const;
    virtual void valueAsHexString(gtString& valueString) const;
    virtual bool compareToOther(const apParameter& other)const;

private:
    // The parameter value:
    cl_gl_object_type _value;
};


// ----------------------------------------------------------------------------------
// Class Name:           AP_API apCLGLTextureInfoParameter : public apParameter
// General Description: Represents a cl_gl_texture_info parameter (OpenGL texture info).
// Author:  AMD Developer Tools Team
// Creation Date:        11/11/2009
// ----------------------------------------------------------------------------------
class AP_API apCLGLTextureInfoParameter : public apParameter
{
public:
    // Self functions:
    apCLGLTextureInfoParameter(cl_gl_texture_info value = 0) : _value(value) { };
    virtual ~apCLGLTextureInfoParameter() {};
    cl_gl_texture_info value() const { return _value; };

    // Overrides osTransferableObject:
    virtual osTransferableObjectType type() const;
    virtual bool writeSelfIntoChannel(osChannel& ipcChannel) const;
    virtual bool readSelfFromChannel(osChannel& ipcChannel);

    // Overrides apParameter:
    virtual void readValueFromArgumentsList(va_list& pArgumentList);
    virtual void readValueFromPointer(void* pValue);
    virtual gtSizeType sizeofData();
    virtual void valueAsString(gtString& valueString) const;
    virtual void valueAsHexString(gtString& valueString) const;
    virtual bool compareToOther(const apParameter& other)const;

private:
    // The parameter value:
    cl_gl_texture_info _value;
};


// ----------------------------------------------------------------------------------
// Class Name:           AP_API apCLMultiStringParameter : public apParameter
// General Description:
//   Represents an OpenCL multi string parameter: an array of strings.
//   This parameter type is used by OpenCL to load program source codes, etc.
//
// Author:  AMD Developer Tools Team
// Creation Date:        16/11/2009
// ----------------------------------------------------------------------------------
class AP_API apCLMultiStringParameter : public apParameter
{
public:
    // Self functions:
    apCLMultiStringParameter() {};
    apCLMultiStringParameter(cl_uint count, const char** strings, const size_t* lengths);
    virtual ~apCLMultiStringParameter() {};
    const gtVector<gtASCIIString>& value() const { return _value; };

    // Overrides osTransferableObject:
    virtual osTransferableObjectType type() const;
    virtual bool writeSelfIntoChannel(osChannel& ipcChannel) const;
    virtual bool readSelfFromChannel(osChannel& ipcChannel);

    // Overrides apParameter:
    virtual void readValueFromArgumentsList(va_list& pArgumentList);
    virtual void readValueFromPointer(void* pValue);
    virtual gtSizeType sizeofData();
    virtual void valueAsString(gtString& valueString) const;
    virtual void valueAsString(gtASCIIString& valueString) const;
    virtual bool compareToOther(const apParameter& other)const;

private:
    void setCLStrings(cl_uint count, const char** strings, const size_t* lengths);

private:
    // The parameter value:
    gtVector<gtASCIIString> _value;
};


// ----------------------------------------------------------------------------------
// Class Name:           AP_API apCLHandleParameter : public apParameter
// General Description: Represents an OpenCL handle parameter.
// Author:  AMD Developer Tools Team
// Creation Date:        8/12/2009
// ----------------------------------------------------------------------------------
class AP_API apCLHandleParameter : public apParameter
{
public:
    // Self functions:
    apCLHandleParameter(oaCLHandle ptrValue = OA_CL_NULL_HANDLE, osTransferableObjectType pointedObjectType = OS_TOBJ_ID_INT_PARAMETER);
    virtual ~apCLHandleParameter() {};
    osTransferableObjectType pointedObjectType() const { return _pointedObjectType; };
    oaCLHandle pointerValue() const { return _ptrValue; };

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
    void setValue(oaCLHandle ptrValue, osTransferableObjectType pointedObjectType) {_ptrValue = ptrValue; _pointedObjectType = pointedObjectType;};

private:
    // Is the debugged app 64-bit?
    bool _is64BitPointer;

    // The pointer value:
    oaCLHandle _ptrValue;

    // The parameter type:
    osTransferableObjectType _pointedObjectType;
};


// ----------------------------------------------------------------------------------
// Class Name:           AP_API apCLDeviceTypeParameter : public apParameter
// General Description: Represents a cl_device_type parameter
// Author:  AMD Developer Tools Team
// Creation Date:        20/1/2010
// ----------------------------------------------------------------------------------
class AP_API apCLDeviceTypeParameter : public apBitfieldParameter
{
public:
    // Self functions:
    apCLDeviceTypeParameter(cl_device_type value = 0) : _value(value) {};
    virtual ~apCLDeviceTypeParameter() {};
    cl_device_type value() const { return _value; };
    void setValue(const cl_device_type& newValue);

    // Overrides osTransferableObject:
    virtual osTransferableObjectType type() const;
    virtual bool writeSelfIntoChannel(osChannel& ipcChannel) const;
    virtual bool readSelfFromChannel(osChannel& ipcChannel);

    // Overrides apParameter:
    virtual void readValueFromArgumentsList(va_list& pArgumentList);
    virtual void readValueFromPointer(void* pValue);
    virtual gtSizeType sizeofData();
    // virtual void valueAsString(gtString& valueString) const;
    virtual bool compareToOther(const apParameter& other)const;

protected:
    // Overrides apBitfieldParameter
    virtual bool getSpecialValue(gtString& o_string) const;
    virtual gtUInt64 getBitfieldAsUInt64() const;
    virtual bool getBitName(int bitIndex, gtString& o_string) const;

private:
    // The parameter value:
    cl_device_type _value;
};

// ----------------------------------------------------------------------------------
// Class Name:          AP_API apCLDeviceExecutionCapabilitiesParameter : public apParameter
// General Description: Represents a cl_device_exec_capabilities parameter
// Author:  AMD Developer Tools Team
// Creation Date:       25/3/2010
// ----------------------------------------------------------------------------------
class AP_API apCLDeviceExecutionCapabilitiesParameter : public apBitfieldParameter
{
public:
    // Self functions:
    apCLDeviceExecutionCapabilitiesParameter(cl_device_exec_capabilities value = 0) : _value(value) {};
    virtual ~apCLDeviceExecutionCapabilitiesParameter() {};
    cl_device_exec_capabilities value() const { return _value; };
    void setValue(const cl_device_exec_capabilities& newValue);

    // Overrides osTransferableObject:
    virtual osTransferableObjectType type() const;
    virtual bool writeSelfIntoChannel(osChannel& ipcChannel) const;
    virtual bool readSelfFromChannel(osChannel& ipcChannel);

    // Overrides apParameter:
    virtual void readValueFromArgumentsList(va_list& pArgumentList);
    virtual void readValueFromPointer(void* pValue);
    virtual gtSizeType sizeofData();
    // virtual void valueAsString(gtString& valueString) const;
    virtual bool compareToOther(const apParameter& other)const;

protected:
    // Overrides apBitfieldParameter
    virtual bool getSpecialValue(gtString& o_string) const;
    virtual gtUInt64 getBitfieldAsUInt64() const;
    virtual bool getBitName(int bitIndex, gtString& o_string) const;

private:
    // The parameter value:
    cl_device_exec_capabilities _value;
};

// ----------------------------------------------------------------------------------
// Class Name:          AP_API apCLDeviceFloatingPointConfigParameter : public apParameter
// General Description: Represents a cl_device_fp_config parameter
// Author:  AMD Developer Tools Team
// Creation Date:       25/3/2010
// ----------------------------------------------------------------------------------
class AP_API apCLDeviceFloatingPointConfigParameter : public apBitfieldParameter
{
public:
    // Self functions:
    apCLDeviceFloatingPointConfigParameter(cl_device_fp_config value = 0) : _value(value) {};
    virtual ~apCLDeviceFloatingPointConfigParameter() {};
    cl_device_fp_config value() const { return _value; };
    void setValue(const cl_device_fp_config& newValue);

    // Overrides osTransferableObject:
    virtual osTransferableObjectType type() const;
    virtual bool writeSelfIntoChannel(osChannel& ipcChannel) const;
    virtual bool readSelfFromChannel(osChannel& ipcChannel);

    // Overrides apParameter:
    virtual void readValueFromArgumentsList(va_list& pArgumentList);
    virtual void readValueFromPointer(void* pValue);
    virtual gtSizeType sizeofData();
    // virtual void valueAsString(gtString& valueString) const;
    virtual bool compareToOther(const apParameter& other)const;

protected:
    // Overrides apBitfieldParameter
    virtual bool getSpecialValue(gtString& o_string) const;
    virtual gtUInt64 getBitfieldAsUInt64() const;
    virtual bool getBitName(int bitIndex, gtString& o_string) const;

private:
    // The parameter value:
    cl_device_fp_config _value;
};

// ----------------------------------------------------------------------------------
// Class Name:           AP_API apCLCommandQueuePropertiesParameter : public apParameter
// General Description: Represents a cl_command_queue_properties parameter
// Author:  AMD Developer Tools Team
// Creation Date:        20/1/2010
// ----------------------------------------------------------------------------------
class AP_API apCLCommandQueuePropertiesParameter : public apBitfieldParameter
{
public:
    // Self functions:
    apCLCommandQueuePropertiesParameter(cl_device_type value = 0) : _value(value) {};
    virtual ~apCLCommandQueuePropertiesParameter() {};
    cl_command_queue_properties value() const { return _value; };

    // Overrides osTransferableObject:
    virtual osTransferableObjectType type() const;
    virtual bool writeSelfIntoChannel(osChannel& ipcChannel) const;
    virtual bool readSelfFromChannel(osChannel& ipcChannel);

    // Overrides apParameter:
    virtual void readValueFromArgumentsList(va_list& pArgumentList);
    virtual void readValueFromPointer(void* pValue);
    virtual gtSizeType sizeofData();
    // virtual void valueAsString(gtString& valueString) const;
    virtual bool compareToOther(const apParameter& other)const;

protected:
    // Overrides apBitfieldParameter
    virtual bool getSpecialValue(gtString& o_string) const;
    virtual gtUInt64 getBitfieldAsUInt64() const;
    virtual bool getBitName(int bitIndex, gtString& o_string) const;

private:
    // The parameter value:
    cl_command_queue_properties _value;
};

// ----------------------------------------------------------------------------------
// Class Name:           AP_API apCLMapFlagsParameter : public apParameter
// General Description: Represents a cl_map_flags parameter
// Author:  AMD Developer Tools Team
// Creation Date:        20/1/2010
// ----------------------------------------------------------------------------------
class AP_API apCLMapFlagsParameter : public apBitfieldParameter
{
public:
    // Self functions:
    apCLMapFlagsParameter(cl_map_flags value = 0) : _value(value) {};
    virtual ~apCLMapFlagsParameter() {};
    cl_map_flags value() const { return _value; };

    // Overrides osTransferableObject:
    virtual osTransferableObjectType type() const;
    virtual bool writeSelfIntoChannel(osChannel& ipcChannel) const;
    virtual bool readSelfFromChannel(osChannel& ipcChannel);

    // Overrides apParameter:
    virtual void readValueFromArgumentsList(va_list& pArgumentList);
    virtual void readValueFromPointer(void* pValue);
    virtual gtSizeType sizeofData();
    // virtual void valueAsString(gtString& valueString) const;
    virtual bool compareToOther(const apParameter& other)const;

protected:
    // Overrides apBitfieldParameter
    virtual bool getSpecialValue(gtString& o_string) const;
    virtual gtUInt64 getBitfieldAsUInt64() const;
    virtual bool getBitName(int bitIndex, gtString& o_string) const;

private:
    // The parameter value:
    cl_map_flags _value;
};

// ----------------------------------------------------------------------------------
// Class Name:           AP_API apCLMapFlagsParameter : public apParameter
// General Description:  Represents a cl_mem_migration_flags parameter
// Author:  AMD Developer Tools Team
// Creation Date:        10/1/2012
// ----------------------------------------------------------------------------------
class AP_API apCLMemoryMigrationFlagsParameter : public apBitfieldParameter
{
public:
    // Self functions:
    apCLMemoryMigrationFlagsParameter(cl_mem_migration_flags value = 0) : _value(value) {};
    virtual ~apCLMemoryMigrationFlagsParameter() {};
    cl_mem_migration_flags value() const { return _value; };

    // Overrides osTransferableObject:
    virtual osTransferableObjectType type() const;
    virtual bool writeSelfIntoChannel(osChannel& ipcChannel) const;
    virtual bool readSelfFromChannel(osChannel& ipcChannel);

    // Overrides apParameter:
    virtual void readValueFromArgumentsList(va_list& pArgumentList);
    virtual void readValueFromPointer(void* pValue);
    virtual gtSizeType sizeofData();
    // virtual void valueAsString(gtString& valueString) const;
    virtual bool compareToOther(const apParameter& other)const;

protected:
    // Overrides apBitfieldParameter
    virtual bool getSpecialValue(gtString& o_string) const;
    virtual gtUInt64 getBitfieldAsUInt64() const;
    virtual bool getBitName(int bitIndex, gtString& o_string) const;

private:
    // The parameter value:
    cl_mem_migration_flags _value;
};

// ----------------------------------------------------------------------------------
// Class Name:           AP_API apCLDeviceAffinityDomainParameter : public apBitfieldParameter
// General Description:  Represents a cl_device_affinity_domain parameter
// Author:  AMD Developer Tools Team
// Creation Date:        29/9/2014
// ----------------------------------------------------------------------------------
class AP_API apCLDeviceAffinityDomainParameter : public apBitfieldParameter
{
public:
    // Self functions:
    apCLDeviceAffinityDomainParameter(cl_device_affinity_domain value = 0) : _value(value) {};
    virtual ~apCLDeviceAffinityDomainParameter() {};
    cl_device_affinity_domain value() const { return _value; };

    // Overrides osTransferableObject:
    virtual osTransferableObjectType type() const;
    virtual bool writeSelfIntoChannel(osChannel& ipcChannel) const;
    virtual bool readSelfFromChannel(osChannel& ipcChannel);

    // Overrides apParameter:
    virtual void readValueFromArgumentsList(va_list& pArgumentList);
    virtual void readValueFromPointer(void* pValue);
    virtual gtSizeType sizeofData();
    // virtual void valueAsString(gtString& valueString) const;
    virtual bool compareToOther(const apParameter& other)const;

protected:
    // Overrides apBitfieldParameter
    virtual bool getSpecialValue(gtString& o_string) const;
    virtual gtUInt64 getBitfieldAsUInt64() const;
    virtual bool getBitName(int bitIndex, gtString& o_string) const;

private:
    // The parameter value:
    cl_device_affinity_domain _value;
};

// ----------------------------------------------------------------------------------
// Class Name:           AP_API apCLDeviceSVMCapabilitiesParameter : public apBitfieldParameter
// General Description:  Represents a cl_device_svm_capabilities parameter
// Author:  AMD Developer Tools Team
// Creation Date:        29/9/2014
// ----------------------------------------------------------------------------------
class AP_API apCLDeviceSVMCapabilitiesParameter : public apBitfieldParameter
{
public:
    // Self functions:
    apCLDeviceSVMCapabilitiesParameter(cl_device_svm_capabilities value = 0) : _value(value) {};
    virtual ~apCLDeviceSVMCapabilitiesParameter() {};
    cl_device_svm_capabilities value() const { return _value; };

    // Overrides osTransferableObject:
    virtual osTransferableObjectType type() const;
    virtual bool writeSelfIntoChannel(osChannel& ipcChannel) const;
    virtual bool readSelfFromChannel(osChannel& ipcChannel);

    // Overrides apParameter:
    virtual void readValueFromArgumentsList(va_list& pArgumentList);
    virtual void readValueFromPointer(void* pValue);
    virtual gtSizeType sizeofData();
    // virtual void valueAsString(gtString& valueString) const;
    virtual bool compareToOther(const apParameter& other)const;

protected:
    // Overrides apBitfieldParameter
    virtual bool getSpecialValue(gtString& o_string) const;
    virtual gtUInt64 getBitfieldAsUInt64() const;
    virtual bool getBitName(int bitIndex, gtString& o_string) const;

private:
    // The parameter value:
    cl_device_svm_capabilities _value;
};

// ----------------------------------------------------------------------------------
// Class Name:           AP_API apCLKernelArgTypeQualifierParameter : public apBitfieldParameter
// General Description:  Represents a cl_kernel_arg_type_qualifier parameter
// Author:  AMD Developer Tools Team
// Creation Date:        29/9/2014
// ----------------------------------------------------------------------------------
class AP_API apCLKernelArgTypeQualifierParameter : public apBitfieldParameter
{
public:
    // Self functions:
    apCLKernelArgTypeQualifierParameter(cl_kernel_arg_type_qualifier value = 0) : _value(value) {};
    virtual ~apCLKernelArgTypeQualifierParameter() {};
    cl_kernel_arg_type_qualifier value() const { return _value; };

    // Overrides osTransferableObject:
    virtual osTransferableObjectType type() const;
    virtual bool writeSelfIntoChannel(osChannel& ipcChannel) const;
    virtual bool readSelfFromChannel(osChannel& ipcChannel);

    // Overrides apParameter:
    virtual void readValueFromArgumentsList(va_list& pArgumentList);
    virtual void readValueFromPointer(void* pValue);
    virtual gtSizeType sizeofData();
    // virtual void valueAsString(gtString& valueString) const;
    virtual bool compareToOther(const apParameter& other)const;

protected:
    // Overrides apBitfieldParameter
    virtual bool getSpecialValue(gtString& o_string) const;
    virtual gtUInt64 getBitfieldAsUInt64() const;
    virtual bool getBitName(int bitIndex, gtString& o_string) const;

private:
    // The parameter value:
    cl_kernel_arg_type_qualifier _value;
};


// ----------------------------------------------------------------------------------
// Class Name:           AP_API apCLEnumParameter : public apParameter
// General Description: Represents an OpenCL enum parameter. The parameter is used for
//                      all cl objects that are behaving as enumerations
//                      cl_context_info, cl_command_queue_info, cl_channel_order, cl_channel_type
//                      cl_mem_object_type, cl_mem_info, cl_image_info, cl_addressing_mode,
//                      cl_filter_mode, cl_sampler_info, cl_program_info, cl_program_build_info
//                      cl_build_status, cl_kernel_info, cl_kernel_work_group_info, cl_event_info
//                      cl_command_type, cl_profiling_info, cl_platform_info cl_device_info
//                      cl_device_mem_cache_type, cl_device_local_mem_type
// Author:  AMD Developer Tools Team
// Creation Date:        20/1/2010
// ----------------------------------------------------------------------------------
class AP_API apCLEnumParameter : public apParameter
{
public:
    // Self functions:
    apCLEnumParameter(cl_uint value = 0) : _value(value) {};
    cl_uint value() const { return _value; };

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
    cl_uint _value;
};


// ----------------------------------------------------------------------------------
// Class Name:           AP_API apCLBufferRegionParameter : public apParameter
// General Description: Represents a cl_buffer_region parameter
// Author:  AMD Developer Tools Team
// Creation Date:        26/10/2010
// ----------------------------------------------------------------------------------
class AP_API apCLBufferRegionParameter : public apParameter
{
public:
    // Self functions:
    apCLBufferRegionParameter(const cl_buffer_region& bufferRegion) { _value.origin = bufferRegion.origin; _value.size = bufferRegion.size; };
    cl_buffer_region value() const { return _value; };

    // Overrides osTransferableObject:
    virtual osTransferableObjectType type() const;
    virtual bool writeSelfIntoChannel(osChannel& ipcChannel) const;
    virtual bool readSelfFromChannel(osChannel& ipcChannel);

    // Overrides apParameter:
    virtual void readValueFromArgumentsList(va_list& pArgumentList);
    virtual void readValueFromPointer(void* pValue);
    virtual gtSizeType sizeofData();
    virtual void valueAsString(gtString& valueString) const;
    virtual void valueAsHexString(gtString& valueString) const;
    virtual bool compareToOther(const apParameter& other)const;

private:
    // The parameter value:
    cl_buffer_region _value;
};



// ----------------------------------------------------------------------------------
// Class Name:          AP_API apCLImageDescriptionParameter : public apParameter
// General Description: Represents a cl_image_desc data
// Author:  AMD Developer Tools Team
// Creation Date:       9/1/2012
// ----------------------------------------------------------------------------------
class AP_API apCLImageDescriptionParameter : public apParameter
{
public:
    // Self functions:
    apCLImageDescriptionParameter(const cl_image_desc& imageDesc);
    apCLImageDescriptionParameter() {};
    cl_image_desc value() const { return _value; };

    // Overrides osTransferableObject:
    virtual osTransferableObjectType type() const;
    virtual bool writeSelfIntoChannel(osChannel& ipcChannel) const;
    virtual bool readSelfFromChannel(osChannel& ipcChannel);

    // Overrides apParameter:
    virtual void readValueFromArgumentsList(va_list& pArgumentList);
    virtual void readValueFromPointer(void* pValue);
    virtual gtSizeType sizeofData();
    virtual void valueAsString(gtString& valueString) const;
    virtual void valueAsHexString(gtString& valueString) const;
    virtual bool compareToOther(const apParameter& other)const;

private:
    // The parameter value:
    cl_image_desc _value;
};

// ----------------------------------------------------------------------------------
// Class Name:          AP_API apCLContextPropertyListParameter : public apZeroTerminatedListParameter
// General Description: Represents a cl_context_properties list
// Author:  AMD Developer Tools Team
// Creation Date:       28/9/2014
// ----------------------------------------------------------------------------------
class AP_API apCLContextPropertyListParameter : public apZeroTerminatedListParameter
{
public:
    apCLContextPropertyListParameter();
    virtual ~apCLContextPropertyListParameter();

    // Overrides osTransferableObject
    virtual osTransferableObjectType type() const;

    // Overrides apZeroTerminatedListParameter:
    virtual unsigned int sizeOfArrayElement() const;
    virtual bool fillParamNameAndValue(const void* i_pArrayItem1, const void* i_pArrayItem2, gtAutoPtr<apParameter>& o_aptrArrayItem1, gtAutoPtr<apParameter>& o_aptrArrayItem2) const;
};

// ----------------------------------------------------------------------------------
// Class Name:          AP_API apCLCommandQueuePropertyListParameter : public apZeroTerminatedListParameter
// General Description: Represents a cl_command_queue_properties list
// Author:  AMD Developer Tools Team
// Creation Date:       29/9/2014
// ----------------------------------------------------------------------------------
class AP_API apCLCommandQueuePropertyListParameter : public apZeroTerminatedListParameter
{
public:
    apCLCommandQueuePropertyListParameter();
    virtual ~apCLCommandQueuePropertyListParameter();

    // Overrides osTransferableObject
    virtual osTransferableObjectType type() const;

    // Overrides apZeroTerminatedListParameter:
    virtual unsigned int sizeOfArrayElement() const;
    virtual bool fillParamNameAndValue(const void* i_pArrayItem1, const void* i_pArrayItem2, gtAutoPtr<apParameter>& o_aptrArrayItem1, gtAutoPtr<apParameter>& o_aptrArrayItem2) const;
};

// ----------------------------------------------------------------------------------
// Class Name:          AP_API apCLPipePropertyListParameter : public apZeroTerminatedListParameter
// General Description: Represents a cl_pipe_properties list
// Author:  AMD Developer Tools Team
// Creation Date:       29/9/2014
// ----------------------------------------------------------------------------------
class AP_API apCLPipePropertyListParameter : public apZeroTerminatedListParameter
{
public:
    apCLPipePropertyListParameter();
    virtual ~apCLPipePropertyListParameter();

    // Overrides osTransferableObject
    virtual osTransferableObjectType type() const;

    // Overrides apZeroTerminatedListParameter:
    virtual unsigned int sizeOfArrayElement() const;
    virtual bool fillParamNameAndValue(const void* i_pArrayItem1, const void* i_pArrayItem2, gtAutoPtr<apParameter>& o_aptrArrayItem1, gtAutoPtr<apParameter>& o_aptrArrayItem2) const;
};

// ----------------------------------------------------------------------------------
// Class Name:          AP_API apCLSamplerPropertyListParameter : public apZeroTerminatedListParameter
// General Description: Represents a cl_sampler_properties list
// Author:  AMD Developer Tools Team
// Creation Date:       29/9/2014
// ----------------------------------------------------------------------------------
class AP_API apCLSamplerPropertyListParameter : public apZeroTerminatedListParameter
{
public:
    apCLSamplerPropertyListParameter();
    virtual ~apCLSamplerPropertyListParameter();

    // Overrides osTransferableObject
    virtual osTransferableObjectType type() const;

    // Overrides apZeroTerminatedListParameter:
    virtual unsigned int sizeOfArrayElement() const;
    virtual bool fillParamNameAndValue(const void* i_pArrayItem1, const void* i_pArrayItem2, gtAutoPtr<apParameter>& o_aptrArrayItem1, gtAutoPtr<apParameter>& o_aptrArrayItem2) const;
};



#endif  // __APOPENCLPARAMETERS
