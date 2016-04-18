//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file csDWARFParser.h
///
//==================================================================================

//------------------------------ csDWARFParser.h ------------------------------

#ifndef __CSDWARFPARSER_H
#define __CSDWARFPARSER_H

// Forward declarations:
typedef struct _Elf Elf;
typedef struct _Dwarf_Debug* Dwarf_Debug;
typedef struct _Dwarf_Die* Dwarf_Die;
enum oaTexelDataFormat;
enum oaDataType;

// Infra:
#include <AMDTBaseTools/Include/AMDTDefinitions.h>
#include <AMDTBaseTools/Include/gtMap.h>
#include <AMDTBaseTools/Include/gtPtrVector.h>
#include <AMDTBaseTools/Include/gtString.h>
#include <AMDTBaseTools/Include/gtVector.h>

// This matches Dwarf_Addr in size and signedness:
#define csDwarfAddressType gtUInt64


// ----------------------------------------------------------------------------------
// Class Name:           csDWARFVariable
// General Description: Represents a variable or parameter TAG DIE in DWARF, and contains
//                      the data we extracted from it
// Author:               Uri Shomroni
// Creation Date:        23/11/2010
// ----------------------------------------------------------------------------------
struct csDWARFVariable
{
public:
    enum ValueType
    {
        CS_PARAMETER_VALUE, // A function parameter
        CS_CONSTANT_VALUE,  // A constant
        CS_VARIABLE_VALUE,  // A variable
        CS_UNKNOWN_VARIABLE_VALUE
    };

    enum ValueEncoding
    {
        CS_POINTER_ENCODING,
        CS_BOOL_ENCODING,
        CS_FLOAT_ENCODING,
        CS_INT_ENCODING,
        CS_UINT_ENCODING,
        CS_CHAR_ENCODING,
        CS_UCHAR_ENCODING,
        CS_NO_ENCODING,
        CS_UNKNOWN_ENCODING,
    };

    enum ValueLocationType
    {
        CS_REGISTER,
        CS_INDIRECT_REGISTER,
        CS_STACK_OFFSET,
        CS_UNKNOWN_VARIABLE_LOCATION,
    };

    enum PointerAddressSpace
    {
        CS_NOT_A_POINTER,
        CS_GLOBAL_POINTER,
        CS_REGION_POINTER,
        CS_LOCAL_POINTER,
        CS_PRIVATE_POINTER,
        CS_CONSTANT_POINTER,
        CS_UNKNOWN_POINTER
    };

    struct csDWARFVariableLocation
    {
    public:
        csDWARFVariableLocation() : _variableLocation(GT_UINT64_MAX), _variableLocationType(CS_UNKNOWN_VARIABLE_LOCATION), _variableLocationOffset(0), _variableLocationAccumulatedOffset(0), _variableLocationResource(0), _variableLowestPC((csDwarfAddressType)0x0000000000000000LL), _variableHighestPCValid(false), _variableHighestPC((csDwarfAddressType)0xFFFFFFFFFFFFFFFFLL) {};
        ~csDWARFVariableLocation() {};

    public:
        gtUInt64 _variableLocation;
        ValueLocationType _variableLocationType;
        gtInt32 _variableLocationOffset;
        gtInt32 _variableLocationAccumulatedOffset;
        gtUInt64 _variableLocationResource;
        csDwarfAddressType _variableLowestPC;
        bool _variableHighestPCValid;
        csDwarfAddressType _variableHighestPC;
    };

public:
    csDWARFVariable() : _valueType(CS_UNKNOWN_VARIABLE_VALUE), _variableConstantValue(0), _variableConstantValueExists(false), _valueSize(0), _valueEncoding(CS_UNKNOWN_ENCODING), _valueIsPointer(false), _valuePointerAddressSpace(CS_NOT_A_POINTER), _valueIsArray(false) {};
    ~csDWARFVariable() {};

    static ValueType valueTypeFromTAG(int dwarfTAG);
    void parseValueFromPointer(const void* pData, gtString& valueString, bool useHexStrings, int pointerSize, bool indirectValue = false) const;
    void getDataFormat(oaTexelDataFormat& dataFormat, oaDataType& dataType) const;
    bool canMatchMemberName(const gtString& memberFullName, const csDWARFVariable*& pFoundMember) const;

public:
    gtString _variableName;
    ValueType _valueType;
    gtString _variableType;
    gtUInt64 _variableConstantValue;
    bool _variableConstantValueExists;
    gtUInt32 _valueSize;
    csDWARFVariableLocation _variableLocation;
    ValueEncoding _valueEncoding;
    bool _valueIsPointer;
    PointerAddressSpace _valuePointerAddressSpace;
    bool _valueIsArray;
    gtVector<csDWARFVariable> _variableMembers;
};

// ----------------------------------------------------------------------------------
// Class Name:          csDWARFCodeLocation
// General Description: Represents a code location - a combination of a file path and
//                      line number.
// Author:              Uri Shomroni
// Creation Date:       29/11/2011
// ----------------------------------------------------------------------------------
struct csDWARFCodeLocation
{
public:
    csDWARFCodeLocation(const gtString& sourceFileFullPath, int lineNumber);
    csDWARFCodeLocation() : _lineNumber(-1) {};
    ~csDWARFCodeLocation() {};

    bool operator< (const csDWARFCodeLocation& other) const {return ((_sourceFileFullPath < other._sourceFileFullPath) || ((_sourceFileFullPath == other._sourceFileFullPath) && (_lineNumber < other._lineNumber)));};

public:
    gtString _sourceFileFullPath;
    int _lineNumber;
};

// ----------------------------------------------------------------------------------
// Class Name:           csDWARFProgram
// General Description: Represents a program or subprogram (scope) in DWARF
// Author:               Uri Shomroni
// Creation Date:        24/11/2010
// ----------------------------------------------------------------------------------
struct csDWARFProgram
{
public:
    enum ScopeType
    {
        CS_COMPILATION_UNIT_SCOPE,  // A compilation unit = source file
        CS_KERNEL_FUNCTION_SCOPE,   // A kernel / main function
        CS_INLINE_FUNCTION_SCOPE,   // An inlined function
        CS_CODE_SCOPE,              // A code scope / { } block
        CS_UNKNOWN_SCOPE
    };

    struct ProgramPCRange
    {
    public:
        ProgramPCRange() : _startPC((csDwarfAddressType)0x0000000000000000LL), _endPC((csDwarfAddressType)0xFFFFFFFFFFFFFFFFLL) {};
        ProgramPCRange(csDwarfAddressType startPC, csDwarfAddressType endPC) : _startPC(startPC), _endPC(endPC) {};
        ~ProgramPCRange() {};

    public:
        csDwarfAddressType _startPC;
        csDwarfAddressType _endPC;
    };

    csDWARFProgram() : _programScopeType(CS_UNKNOWN_SCOPE), _pParentProgram(NULL), _framePointerRegister(GT_UINT64_MAX), _inlinedFunctionCodeLocation(L"", -1) {};
    ~csDWARFProgram() {_pParentProgram = NULL;};

    static ScopeType programScopeTypeFromTAG(int dwarfTAG);

public:
    gtString _programName;
    ScopeType _programScopeType;
    csDWARFProgram* _pParentProgram;
    gtUInt64 _framePointerRegister;
    gtPtrVector<csDWARFProgram*> _childPrograms;
    gtPtrVector<csDWARFVariable*> _programVariables;
    gtVector<ProgramPCRange> _programPCRanges;
    gtVector<csDwarfAddressType> _programMappedPCs;
    gtString _inlinedFunctionName;
    csDWARFCodeLocation _inlinedFunctionCodeLocation;
};

// ----------------------------------------------------------------------------------
// Class Name:           csDWARFParser
// General Description: Uses libdwarf to parse and return information about debugged
//                      OpenCL Kernels
// Author:               Uri Shomroni
// Creation Date:        15/11/2010
// ----------------------------------------------------------------------------------
class csDWARFParser
{
public:
    csDWARFParser();
    ~csDWARFParser();

    bool initializeWithBinary(const void* binaryData, gtSize_t binarySize);
    bool isInitialized() {return _isInitialized;};
    bool terminate();

    int dwarfPointerSize();

    void setFirstSourceFileRealPath(const gtString& pathAsString) {_firstSourceFileRealPath = pathAsString;};

    bool lineNumberFromAddress(csDwarfAddressType addr, csDWARFCodeLocation& codeLoc);
    bool addressesFromLineNumber(const csDWARFCodeLocation& codeLoc, gtVector<csDwarfAddressType>& addrs);
    bool getMappedAddresses(gtVector<csDwarfAddressType>& addrs);
    bool closestUsedLineNumber(const csDWARFCodeLocation& codeLoc, int& usedLineNum);
    bool listVariablesFromAddress(csDwarfAddressType addr, gtVector<gtString>& variableNames, bool getLeaves, int stackFrameDepth);
    bool listVariableRegisterLocations(gtVector<gtUInt64>& variableLocations);

    const csDWARFProgram* findAddressScope(csDwarfAddressType addr);
    const csDWARFProgram* findClosestScopeContainingVariable(csDwarfAddressType startAddr, const gtString& varName, const csDWARFVariable** ppVar);
    gtUInt64 getFramePointerRegister(csDwarfAddressType startAddr, const gtString& varName);
    int addressStackDepth(csDwarfAddressType addr);

private:
    bool initializeLineNumberInformation(Dwarf_Die cuDIE);
    void fillProgramWithInformationFromDIE(csDWARFProgram& programData, csDWARFProgram* pParentProgram, Dwarf_Die programDIE);
    void fillInlinedFunctinoProgramWithInformationFromDIE(csDWARFProgram& programData, Dwarf_Die programDIE);
    void fillVariableWithInformationFromDIE(csDWARFVariable& variableData, Dwarf_Die variableDIE, gtVector<csDWARFVariable::csDWARFVariableLocation>& variableAdditionalLocations, bool isMember);
    bool connectLineNumberWithProgramCounter(const csDWARFCodeLocation& codeLoc, csDwarfAddressType progCounter);
    gtUInt64 locationFromDWARFData(gtUByte atom, gtUInt64 number1, csDWARFVariable::ValueLocationType& locationType);
    void intersectVariablesInProgram(csDWARFProgram& programData);
    void typeNameAndDetailsFromTypeDIE(gtString& typeName, gtUInt32& typeSize, csDWARFVariable::ValueEncoding& typeEncoding, bool& isPointerType, bool& isArrayType, gtVector<csDWARFVariable>& typeMembers, bool expandIndirectMembers, gtUInt64 membersLocation, csDWARFVariable::ValueLocationType membersLocationType, gtUInt64 membersLocationResource, gtUInt32 membersAccumulatedOffset, Dwarf_Die typeDIE, bool isRegisterParamter);
    void clearLineInformation();
    csDWARFProgram* getAddressScope(csDwarfAddressType addr);

    void addLeafMembersToVector(const csDWARFVariable& variableData, gtVector<gtString>& variableNames, gtString& namesBase);

private:
    bool _isInitialized;
    const void* _binaryData;
    gtSize_t _binarySize;
    Elf* _pElf;
    Dwarf_Debug _pDwarf;
    gtString _firstSourceFileRealPath;
    gtMap<csDwarfAddressType, csDWARFCodeLocation> _programCounterToCodeLocation;
    gtMap<csDWARFCodeLocation, gtVector<csDwarfAddressType> > _codeLocationToProgramCounters;
    gtVector<csDwarfAddressType> _programCountersMappedToLineNumbers;
    csDWARFProgram* _pCUProgram;
    int m_dwarfAddressSize;
};

#endif //__CSDWARFPARSER_H

