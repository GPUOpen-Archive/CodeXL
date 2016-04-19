//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file csDWARFParser.cpp
///
//==================================================================================

//------------------------------ csDWARFParser.cpp ------------------------------

// Infra:
#include <AMDTBaseTools/Include/AMDTDefinitions.h>
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTBaseTools/Include/gtQueue.h>
#include <AMDTBaseTools/Include/gtStringTokenizer.h>
#include <AMDTOSWrappers/Include/osFilePath.h>
#include <AMDTOSAPIWrappers/Include/oaDataType.h>
#include <AMDTOSAPIWrappers/Include/oaTexelDataFormat.h>

// Local:
#include <src/csDWARFParser.h>

// GRLibDWARF:
#include <libelf.h>
#include <libdwarf.h>
#include <dwarf.h>
// Uri, 16/11/10: libelf and winnt.h define this macro as two different thing.
// Since we do not use the libelf macro, we undefine it before include gtAssert.h,
// which contains windows headers, to avoid a redundant warning.
#undef SLIST_ENTRY

// Uri, 18/11/10: our implementation of DWARF is missing some values in the header file, added them here:
#define DW_DLA_STRING 0x01
#define DW_DLA_LOCDESC 0x03
#define DW_DLA_BLOCK 0x06
#define DW_DLA_DIE 0x08
#define DW_DLA_LINE 0x09
#define DW_DLA_ATTR 0x0a
#define DW_DLA_LIST 0x0f
#define DW_DLA_LOC_BLOCK 0x16

// Uri, 1/2/12: private AMD values for DWARF:
#define DW_AT_AMDIL_address_space 0x3ff1
#define DW_AT_AMDIL_resource 0x3ff2

// Define this as a macro rather than a function to have the GT_ASSERT show the correct file / line:
#define CS_DW_REPORT_ERROR(dwErr, cond)             \
    {                                                   \
        gtString errMsg;                                \
        gtString dwErrMsg;                              \
        if (NULL != dwErr.err_msg)                      \
        {                                               \
            dwErrMsg.fromASCIIString(dwErr.err_msg);    \
        }                                               \
        gtString dwErrFunc;                             \
        if (NULL != dwErr.err_func)                     \
        {                                               \
            dwErrFunc.fromASCIIString(dwErr.err_func);  \
        }                                               \
        \
        errMsg.appendFormattedString(L"Dwarf Error #%d (ELF #%d) at ", dwErr.err_error, dwErr.err_elferror).append(dwErrFunc).appendFormattedString(L" (line %d):\n", dwErr.err_line).append(dwErrMsg); \
        GT_ASSERT_EX(cond, errMsg.asCharArray());       \
    }


int dwarf_formref_die(Dwarf_Attribute attr, Dwarf_Die* return_die, Dwarf_Error* error, Dwarf_Debug dbg /* This added parameter cannot be avoided since we can't get attr's attr->at_die->die_dbg member */)
{
    int retVal = DW_DLV_OK;

    // We expect the attribute to be a reference or a global reference. Try each of those:
    Dwarf_Error err;
    memset((void*)&err, 0, sizeof(Dwarf_Error));
    Dwarf_Off offset = 0;
    int rc = dwarf_global_formref(attr, &offset, &err);

    if (rc == DW_DLV_OK)
    {
        // Get the DIE:
        retVal = dwarf_offdie(dbg, offset, return_die, error);
    }
    else // rc != DW_DLV_OK
    {
        // Not a global reference, try local (global offset should cover what we're doing manually here):
        rc = dwarf_formref(attr, &offset, &err);

        if (rc == DW_DLV_OK)
        {
            // Get the compilation unit's offset:
            Dwarf_Die cuDIE = NULL;
            dwarf_siblingof(dbg, NULL, &cuDIE, NULL);
            Dwarf_Off cuOff = 0;
            dwarf_dieoffset(cuDIE, &cuOff, NULL);
            dwarf_dealloc(dbg, (Dwarf_Ptr)cuDIE, DW_DLA_DIE);
            cuDIE = NULL;

            // Add the CU offset to this offset, and get the DIE:
            retVal = dwarf_offdie(dbg, offset + cuOff, return_die, error);
        }
        else // rc != DW_DLV_OK
        {
            // Return the error and retVal from the form function:
            retVal = rc;

            if (error != NULL)
            {
                *error = err;
            }
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        csDWARFVariable::valueTypeFromTAG
// Description: Translates a DWARF DIE TAG to the value type enumeration
// Author:      Uri Shomroni
// Date:        25/11/2010
// ---------------------------------------------------------------------------
csDWARFVariable::ValueType csDWARFVariable::valueTypeFromTAG(int dwarfTAG)
{
    ValueType retVal = CS_UNKNOWN_VARIABLE_VALUE;

    switch (dwarfTAG)
    {
        case DW_TAG_formal_parameter:
            retVal = CS_PARAMETER_VALUE;
            break;

        case DW_TAG_constant:
            retVal = CS_CONSTANT_VALUE;
            break;

        case DW_TAG_enumerator:
        case DW_TAG_variable:
            retVal = CS_VARIABLE_VALUE;
            break;

        default:
            // Unexpected value:
            GT_ASSERT(false);
            break;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        csDWARFVariable::parseValueFromPointer
// Description: Uses this variable's information to parse pData into a string
// Author:      Uri Shomroni
// Date:        25/1/2011
// ---------------------------------------------------------------------------
void csDWARFVariable::parseValueFromPointer(const void* pData, gtString& valueString, bool useHexStrings, int pointerSize, bool indirectValue) const
{
    const void* pUseData = pData;

    // If this variable has a constant value, use that instead:
    if (_variableConstantValueExists)
    {
        pUseData = (const void*)&_variableConstantValue;
    }

    bool showDirectPointerValue = _valueIsPointer && (!indirectValue);

    if (showDirectPointerValue)
    {
        valueString.appendFormattedString((4 < pointerSize) ? GT_64_BIT_POINTER_FORMAT_LOWERCASE : GT_32_BIT_POINTER_FORMAT_LOWERCASE, *(const void**)pUseData);
    }
    else // !showDirectPointerValue
    {
        switch (_valueEncoding)
        {
            case CS_POINTER_ENCODING:
                valueString.appendFormattedString((4 < pointerSize) ? GT_64_BIT_POINTER_FORMAT_LOWERCASE : GT_32_BIT_POINTER_FORMAT_LOWERCASE, *(const void**)pUseData);
                break;

            case CS_BOOL_ENCODING:
                valueString.append(*(const bool*)pUseData ? L"true" : L"false");
                break;

            case CS_FLOAT_ENCODING:
            {
                // Print with scientific notation if the value is too small:
                if (_valueSize < 8)
                {
                    float valueAsFloat = *(const float*)pUseData;
                    bool shouldUseG = ((valueAsFloat != 0.0f) && (valueAsFloat < 0.000001f) && (valueAsFloat > -0.000001f));
                    valueString.appendFormattedString(shouldUseG ? L"%g" : L"%f", valueAsFloat);
                }
                else
                {
                    double valueAsDouble = *(const double*)pUseData;
                    bool shouldUseG = ((valueAsDouble != 0.0) && (valueAsDouble < 0.000001) && (valueAsDouble > -0.000001));
                    valueString.appendFormattedString(shouldUseG ? L"%g" : L"%f", valueAsDouble);
                }
            }
            break;

            case CS_INT_ENCODING:
            {
                if (_valueSize < 4)
                {
                    if (useHexStrings)
                    {
                        valueString.appendFormattedString(GT_UNSIGNED_INT_HEXADECIMAL_4_CHAR_FORMAT, *(const gtUInt16*)pUseData);
                    }
                    else
                    {
                        valueString.appendFormattedString(L"%d", *(const gtInt16*)pUseData);
                    }
                }
                else if (_valueSize < 8)
                {
                    if (useHexStrings)
                    {
                        valueString.appendFormattedString(GT_UNSIGNED_INT_HEXADECIMAL_8_CHAR_FORMAT, *(const gtUInt32*)pUseData);
                    }
                    else
                    {
                        valueString.appendFormattedString(L"%d", *(const gtInt32*)pUseData);
                    }
                }
                else
                {
                    if (useHexStrings)
                    {
                        valueString.appendFormattedString(GT_UNSIGNED_INT_HEXADECIMAL_16_CHAR_FORMAT, *(const gtUInt64*)pUseData);
                    }
                    else
                    {
                        valueString.appendFormattedString(L"%lld", *(const gtInt64*)pUseData);
                    }
                }
            }
            break;

            case CS_CHAR_ENCODING:
            {
                int valueAsChar = (*(const gtByte*)pUseData) & 0xff;
                valueString.appendFormattedString(useHexStrings ? GT_UNSIGNED_INT_HEXADECIMAL_2_CHAR_FORMAT : L"%d", valueAsChar).appendFormattedString(L" \'%c\'", (valueAsChar < 128) ? valueAsChar : 63);
            }
            break;

            case CS_UINT_ENCODING:
            {
                if (_valueSize < 4)
                {
                    valueString.appendFormattedString(useHexStrings ? GT_UNSIGNED_INT_HEXADECIMAL_4_CHAR_FORMAT : L"%u", *(const gtUInt16*)pUseData);
                }
                else if (_valueSize < 8)
                {
                    valueString.appendFormattedString(useHexStrings ? GT_UNSIGNED_INT_HEXADECIMAL_8_CHAR_FORMAT : L"%u", *(const gtUInt32*)pUseData);
                }
                else
                {
                    valueString.appendFormattedString(useHexStrings ? GT_UNSIGNED_INT_HEXADECIMAL_16_CHAR_FORMAT : L"%llu", *(const gtUInt64*)pUseData);
                }
            }
            break;

            case CS_UCHAR_ENCODING:
            {
                unsigned int valueAsUChar = (*(const gtUByte*)pUseData) & 0xff;
                valueString.appendFormattedString(useHexStrings ? GT_UNSIGNED_INT_HEXADECIMAL_2_CHAR_FORMAT : L"%u", valueAsUChar).appendFormattedString(L" \'%c\'", (valueAsUChar < 128) ? valueAsUChar : 63);
            }
            break;

            case CS_NO_ENCODING:
                break;

            case CS_UNKNOWN_ENCODING:
                valueString = L"Unknown value";
                break;
        }
    }

    // If this type has any members, display them:
    int numberOfMembers = (int)_variableMembers.size();

    if ((numberOfMembers > 0) && (_valueEncoding != CS_POINTER_ENCODING) && (!showDirectPointerValue))
    {
        if (!valueString.isEmpty())
        {
            valueString.append(' ');
        }

        valueString.append('{');

        // Add all the members:
        for (int i = 0; i < numberOfMembers; i++)
        {
            if (i > 0)
            {
                valueString.append(',').append(' ');
            }

            // Add the current member to the string:
            const csDWARFVariable& currentMember = _variableMembers[i];

            // Get the member's data pointer by adding the offset:
            const void* pCurrentMemberData = (const void*)((gtSize_t)pUseData + (gtSize_t)currentMember._variableLocation._variableLocationOffset);
            gtString currentMemberAsString;

            // Get the member as a string:
            currentMember.parseValueFromPointer(pCurrentMemberData, currentMemberAsString, useHexStrings, pointerSize);
            valueString.append(currentMemberAsString);
        }

        valueString.append('}');
    }

    // TO_DO: Uri, 11/3/12 - Some address spaces are not currently (CodeXL 6.2) supported for dereferencing.
    // If this is a local or constant pointer, don't show its value:
    if (_valueIsPointer || _valueIsArray || (-1 != _variableType.find('&')))
    {
        if (_valuePointerAddressSpace == CS_CONSTANT_POINTER)
        {
            valueString = L"viewing constant / __constant memory is not currently supported";
        }
        else if (_valuePointerAddressSpace == CS_LOCAL_POINTER)
        {
            valueString = L"viewing local / __local memory is not currently supported";
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        csDWARFVariable::getDataFormat
// Description: Gets the variable's data format and type
// Author:      Uri Shomroni
// Date:        3/3/2011
// ---------------------------------------------------------------------------
void csDWARFVariable::getDataFormat(oaTexelDataFormat& dataFormat, oaDataType& dataType) const
{
    // Set the data format:
    dataFormat = OA_TEXEL_FORMAT_VARIABLE_VALUE;
    dataType = OA_UNKNOWN_DATA_TYPE;

    if (_valueIsPointer)
    {
        dataType = (_valueSize >= 8) ? OA_UNSIGNED_LONG : OA_UNSIGNED_INT;
    }
    else
    {
        switch (_valueEncoding)
        {
            case CS_POINTER_ENCODING:
                dataType = (_valueSize >= 8) ? OA_UNSIGNED_LONG : OA_UNSIGNED_INT;
                break;

            case CS_BOOL_ENCODING:
                dataType = OA_UNSIGNED_BYTE;
                break;

            case CS_FLOAT_ENCODING:
                dataType = (_valueSize >= 8) ? OA_DOUBLE :
                           (_valueSize >= 4) ? OA_FLOAT :
                           // (_valueSize >= 2) ? OS_HALF :
                           OA_UNKNOWN_DATA_TYPE;
                break;

            case CS_INT_ENCODING:
                dataType = (_valueSize >= 8) ? OA_LONG :
                           (_valueSize >= 4) ? OA_INT :
                           (_valueSize >= 2) ? OA_SHORT :
                           (_valueSize >= 1) ? OA_BYTE :
                           OA_UNKNOWN_DATA_TYPE;
                break;

            case CS_CHAR_ENCODING:
                dataType = OA_BYTE;
                break;

            case CS_UINT_ENCODING:
                dataType = (_valueSize >= 8) ? OA_UNSIGNED_LONG :
                           (_valueSize >= 4) ? OA_UNSIGNED_INT :
                           (_valueSize >= 2) ? OA_UNSIGNED_SHORT :
                           (_valueSize >= 1) ? OA_UNSIGNED_BYTE :
                           OA_UNKNOWN_DATA_TYPE;
                break;

            case CS_UCHAR_ENCODING:
                dataType = OA_UNSIGNED_BYTE;
                break;

            case CS_NO_ENCODING:
            case CS_UNKNOWN_ENCODING:
                dataType = OA_UNKNOWN_DATA_TYPE;
                dataFormat = OA_TEXEL_FORMAT_UNKNOWN;
                break;

            default:
                GT_ASSERT(false);
                break;
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        csDWARFVariable::canMatchMemberName
// Description: Returns true iff memberFullName describes a variable or member
//              name that this variable can match.
//              e.g. if the string is "a.b.c" then this variable must have the
//              name a, and it must have a member called b of a class which has
//              a member called c.
// Return Val:  bool - Success / failure.
// Author:      Uri Shomroni
// Date:        29/3/2011
// ---------------------------------------------------------------------------
bool csDWARFVariable::canMatchMemberName(const gtString& memberFullName, const csDWARFVariable*& pFoundMember) const
{
    // Break the name to members of members of members:
    bool retVal = false;
    gtStringTokenizer variableMemberTokenizer(memberFullName, '.');
    gtString baseVariableName;
    variableMemberTokenizer.getNextToken(baseVariableName);

    if (_variableName == baseVariableName)
    {
        // The base variable was found, start looking in its member list:
        retVal = true;
        gtString currentMemberName;
        const csDWARFVariable* pCurrentMemberDetails = this;

        while (variableMemberTokenizer.getNextToken(currentMemberName))
        {
            // Check if this is a member of the current struct:
            const gtVector<csDWARFVariable>& currentMemberSubitems = pCurrentMemberDetails->_variableMembers;
            int numberOfSubitems = (int)currentMemberSubitems.size();
            bool foundSubitem = false;

            for (int i = 0; i < numberOfSubitems; i++)
            {
                // Check if this is the requested subitem:
                const csDWARFVariable& currentSubitem = currentMemberSubitems[i];

                if (currentSubitem._variableName == currentMemberName)
                {
                    // Iterate into it:
                    pCurrentMemberDetails = &currentSubitem;

                    // Stop looking here:
                    foundSubitem = true;
                    break;
                }
            }

            // If this member doesn't exist:
            if (!foundSubitem)
            {
                // Stop looking for this value, the expression is not valid:
                retVal = false;
                break;
            }
        }

        // Return the found member:
        if (retVal)
        {
            pFoundMember = pCurrentMemberDetails;
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        csDWARFCodeLocation::csDWARFCodeLocation
// Description: Constructor
// Author:      Uri Shomroni
// Date:        29/11/2011
// ---------------------------------------------------------------------------
csDWARFCodeLocation::csDWARFCodeLocation(const gtString& sourceFileFullPath, int lineNumber)
    : _sourceFileFullPath(sourceFileFullPath), _lineNumber(lineNumber)
{
    // Normalize the path separators. Note we only need to do this when initializing
    // from a string, not in operator= or in the copy constructor:
    osFilePath::adjustStringToCurrentOS(_sourceFileFullPath);
}

// ---------------------------------------------------------------------------
// Name:        csDWARFProgram::programScopeTypeFromTAG
// Description: Translates a DWARF DIE TAG to the scope type enumeration
// Author:      Uri Shomroni
// Date:        24/11/2010
// ---------------------------------------------------------------------------
csDWARFProgram::ScopeType csDWARFProgram::programScopeTypeFromTAG(int dwarfTAG)
{
    ScopeType retVal = CS_UNKNOWN_SCOPE;

    switch (dwarfTAG)
    {
        case DW_TAG_entry_point:
        case DW_TAG_subprogram:
            retVal = CS_KERNEL_FUNCTION_SCOPE;
            break;

        case DW_TAG_inlined_subroutine:
            retVal = CS_INLINE_FUNCTION_SCOPE;
            break;

        case DW_TAG_lexical_block:
            retVal = CS_CODE_SCOPE;
            break;

        case DW_TAG_compile_unit:
            retVal = CS_COMPILATION_UNIT_SCOPE;
            break;

        default:
            // Unexpected value:
            GT_ASSERT(false);
            break;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        csDWARFParser::csDWARFParser
// Description: Constructor
// Author:      Uri Shomroni
// Date:        16/11/2010
// ---------------------------------------------------------------------------
csDWARFParser::csDWARFParser()
    : _isInitialized(false), _binaryData(NULL), _binarySize(0), _pElf(NULL), _pDwarf(NULL), _pCUProgram(NULL), m_dwarfAddressSize(-1)
{

}

// ---------------------------------------------------------------------------
// Name:        csDWARFParser::~csDWARFParser
// Description: Destructor
// Author:      Uri Shomroni
// Date:        16/11/2010
// ---------------------------------------------------------------------------
csDWARFParser::~csDWARFParser()
{
    // Verify no DWARF session is in progress:
    terminate();
}

// ---------------------------------------------------------------------------
// Name:        csDWARFParser::initializeWithBinary
// Description: Looks for DWARF sections in the supplied ELF binary and initialzes
//              The reading from it.
// Arguments: binaryData - the pointer to a nonmutable ELF binary
//            binarySize - the binary size
// Return Val:  bool - Success / failure.
// Author:      Uri Shomroni
// Date:        16/11/2010
// ---------------------------------------------------------------------------
bool csDWARFParser::initializeWithBinary(const void* binaryData, gtSize_t binarySize)
{
    bool retVal = false;

    // We do not currently support parsing two binaries:
    if (!_isInitialized)
    {
        // Copy the data pointer:
        _binaryData = binaryData;
        _binarySize = binarySize;

        // Initialize an Elf object with this memory:
        GT_ASSERT(_pElf == NULL);
        elf_version(EV_CURRENT);
        _pElf = elf_memory((char*)_binaryData, _binarySize);
        GT_IF_WITH_ASSERT(_pElf != NULL)
        {
            // Initialize a D with this Elf object:
            Dwarf_Error initErr;
            memset((void*)&initErr, 0, sizeof(Dwarf_Error));

            int rcDW = dwarf_elf_init(_pElf, DW_DLC_READ, NULL, NULL, &_pDwarf, &initErr);

            if ((rcDW == DW_DLV_OK) && (_pDwarf != NULL))
            {
                // Mark that we succeeded:
                retVal = true;
                _isInitialized = true;

                // Get the offset for the compilation unit. Note that we expect OpenCL kernels
                // to only have one CU. Check that we have one at all:
                Dwarf_Unsigned cuHeaderOffset = 0;
                Dwarf_Error err;
                memset((void*)&err, 0, sizeof(Dwarf_Error));

                int rc = dwarf_next_cu_header(_pDwarf, NULL, NULL, NULL, NULL, &cuHeaderOffset, &err);

                if (rc == DW_DLV_OK)
                {
                    // Get the DIE for the first CU by calling for the sibling of NULL:
                    Dwarf_Die cuDIE = NULL;
                    rc = dwarf_siblingof(_pDwarf, NULL, &cuDIE, &err);

                    if (rc == DW_DLV_OK)
                    {
                        // Create a program object for the CU and start filling it:
                        GT_ASSERT(_pCUProgram == NULL);
                        _pCUProgram = new csDWARFProgram;
                        _pCUProgram->_programScopeType = csDWARFProgram::CS_COMPILATION_UNIT_SCOPE;
                        fillProgramWithInformationFromDIE(*_pCUProgram, NULL, cuDIE);

                        // Use the CU DIE to get the line number information. This needs to happen after the programs are
                        // initialized, since each entry must be associated with a program:
                        bool rcLn = initializeLineNumberInformation(cuDIE);
                        GT_ASSERT(rcLn);

                        // Release the CU DIE:
                        dwarf_dealloc(_pDwarf, (Dwarf_Ptr)cuDIE, DW_DLA_DIE);
                    }
                }

                // Get the pointer size:
                Dwarf_Half ptrSz = 0;
                rc = dwarf_get_address_size(_pDwarf, &ptrSz, &err);
                GT_IF_WITH_ASSERT(DW_DLV_OK == rc)
                {
                    m_dwarfAddressSize = (int)ptrSz;
                }
            }
            else
            {
                // Report initialization errors:
                CS_DW_REPORT_ERROR(initErr, false);
            }
        }
    }

    // If we failed, clean up:
    if (!retVal)
    {
        _binaryData = NULL;
        _binarySize = 0;

        if (_pDwarf != NULL)
        {
            Dwarf_Error finishErr;
            memset((void*)&finishErr, 0, sizeof(Dwarf_Error));

            int rcDF = dwarf_finish(_pDwarf, &finishErr);
            GT_ASSERT(DW_DLV_OK == rcDF);
        }

        if (_pElf != NULL)
        {
            int rcEF = elf_end(_pElf);
            GT_ASSERT(rcEF == 0);
            _pElf = NULL;
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        csDWARFParser::terminate
// Description: Terminates the reading from the current binary and gets ready
//              to read from another one.
// Return Val:  bool - Success / failure.
// Author:      Uri Shomroni
// Date:        16/11/2010
// ---------------------------------------------------------------------------
bool csDWARFParser::terminate()
{
    bool retVal = !_isInitialized;

    if (_isInitialized)
    {
        // Mark we are no longer initialized:
        _isInitialized = false;

        // Clear the data pointer:
        _binaryData = NULL;
        _binarySize = 0;

        // Clear the lines information:
        clearLineInformation();

        // Delete the program / variable tree (the destructors should clear the hierarchy):
        delete _pCUProgram;
        _pCUProgram = NULL;

        // Finish the DWARF session:
        if (_pDwarf != NULL)
        {
            Dwarf_Error finishErr;
            memset((void*)&finishErr, 0, sizeof(Dwarf_Error));
            int rcDF = dwarf_finish(_pDwarf, &finishErr);
            GT_ASSERT(rcDF == DW_DLV_OK);
        }

        // End the ELF session:
        if (_pElf != NULL)
        {
            int rcEF = elf_end(_pElf);
            GT_ASSERT(rcEF == 0);
            _pElf = NULL;
        }

        retVal = true;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        csDWARFParser::dwarfPointerSize
// Description: Returns the DWARF pointer size
// Return Val:  bool - Success / failure.
// Author:      Uri Shomroni
// Date:        24/11/2014
// ---------------------------------------------------------------------------
int csDWARFParser::dwarfPointerSize()
{
    int retVal = -1;

    if (_isInitialized)
    {
        retVal = m_dwarfAddressSize;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        csDWARFParser::lineNumberFromAddress
// Description: Returns the lineNumber to which addr is mapped
// Return Val:  bool - Success / failure.
// Author:      Uri Shomroni
// Date:        21/11/2010
// ---------------------------------------------------------------------------
bool csDWARFParser::lineNumberFromAddress(csDwarfAddressType addr, csDWARFCodeLocation& codeLoc)
{
    bool retVal = false;
    codeLoc._sourceFileFullPath.makeEmpty();
    codeLoc._lineNumber = -1;

    // Find the address:
    gtMap<csDwarfAddressType, csDWARFCodeLocation>::const_iterator findIter = _programCounterToCodeLocation.find(addr);
    gtMap<csDwarfAddressType, csDWARFCodeLocation>::const_iterator endIter = _programCounterToCodeLocation.end();

    if (findIter != endIter)
    {
        retVal = true;

        // Return the line number:
        codeLoc = (*findIter).second;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        csDWARFParser::addressesFromLineNumber
// Description: Copies the vector of addresses mapped to lineNum into addrs.
// Return Val:  bool - Success / failure.
// Author:      Uri Shomroni
// Date:        21/11/2010
// ---------------------------------------------------------------------------
bool csDWARFParser::addressesFromLineNumber(const csDWARFCodeLocation& codeLoc, gtVector<csDwarfAddressType>& addrs)
{
    bool retVal = false;
    addrs.clear();

    // Find the line number:
    gtMap<csDWARFCodeLocation, gtVector<csDwarfAddressType> >::const_iterator findIter = _codeLocationToProgramCounters.find(codeLoc);
    gtMap<csDWARFCodeLocation, gtVector<csDwarfAddressType> >::const_iterator endIter = _codeLocationToProgramCounters.end();

    if (findIter != endIter)
    {
        retVal = true;

        // Copy the vector:
        const gtVector<csDwarfAddressType>& foundAddrs = (*findIter).second;
        int numOfAddrs = (int)foundAddrs.size();

        for (int i = 0 ; i < numOfAddrs; i++)
        {
            addrs.push_back(foundAddrs[i]);
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        csDWARFParser::getMappedAddresses
// Description: Returns all addresses that are mapped to any line number
// Return Val:  bool - Success / failure.
// Author:      Uri Shomroni
// Date:        20/6/2011
// ---------------------------------------------------------------------------
bool csDWARFParser::getMappedAddresses(gtVector<csDwarfAddressType>& addrs)
{
    bool retVal = false;
    addrs.clear();

    // If we have debug information
    if (_isInitialized)
    {
        retVal = true;

        // Copy the addresses:
        int numberOfPCs = (int)_programCountersMappedToLineNumbers.size();

        for (int i = 0; i < numberOfPCs; i++)
        {
            addrs.push_back(_programCountersMappedToLineNumbers[i]);
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        csDWARFParser::closestUsedLineNumber
// Description: If there are any addresses mapped to lineNum, returns it.
//              If not and there are any addresses mapped to lines after lineNum,
//              returns the closest one.
//              If not and there are any addresses mapped at all, returns the closest
//              one before lineNum.
//              Otherwise, returns false and -1
// Return Val:  bool - Success / failure.
// Author:      Uri Shomroni
// Date:        21/11/2010
// ---------------------------------------------------------------------------
bool csDWARFParser::closestUsedLineNumber(const csDWARFCodeLocation& codeLoc, int& usedLineNum)
{
    (void)(codeLoc); // unused
    (void)(usedLineNum); // unused
    // TO_DO

    return false;
}

// ---------------------------------------------------------------------------
// Name:        csDWARFParser::listVariablesFromAddress
// Description: Gets a list of all variables defined in addr's scope and all
//              containing scopes
// Return Val:  bool - Success / failure.
// Author:      Uri Shomroni
// Date:        21/2/2011
// ---------------------------------------------------------------------------
bool csDWARFParser::listVariablesFromAddress(csDwarfAddressType addr, gtVector<gtString>& variableNames, bool getLeaves, int stackFrameDepth)
{
    bool retVal = false;

    // Get the address's scope:
    const csDWARFProgram* pAddrScope = findAddressScope(addr);

    if (pAddrScope != NULL)
    {
        retVal = true;

        int currentStackDepth = 0;
        int totalStackDepth = addressStackDepth(addr);

        // Go up in the hierarchy, adding all variables in each scope:
        const csDWARFProgram* pCurrentScope = pAddrScope;

        while (pCurrentScope != NULL)
        {
            // Get the variables vector:
            const gtPtrVector<csDWARFVariable*>& currentScopeVariables = pCurrentScope->_programVariables;

            // We add the variables in three cases:
            // 1. The parameter is -1, signifying the caller wants all the locals from all levels.
            // 2. This is the requested scope
            // 3. This is the global scope (outside the kernel)
            if ((stackFrameDepth == -1) || (stackFrameDepth == currentStackDepth) || (currentStackDepth == totalStackDepth))
            {
                // Get all the variables:
                int numberOfVariables = (int)currentScopeVariables.size();

                for (int i = 0; i < numberOfVariables; i++)
                {
                    // Sanity check:
                    const csDWARFVariable* pCurrentVariable = currentScopeVariables[i];
                    GT_IF_WITH_ASSERT(pCurrentVariable != NULL)
                    {
                        // Check the variable's scope:
                        if ((pCurrentVariable->_variableLocation._variableHighestPC >= addr) && (pCurrentVariable->_variableLocation._variableLowestPC <= addr))
                        {
                            if (getLeaves)
                            {
                                // Add the variable's leaf descendants into the vector:
                                gtString namesBase = pCurrentVariable->_variableName;
                                addLeafMembersToVector(*pCurrentVariable, variableNames, namesBase);
                            }
                            else // !getLeaves
                            {
                                // Add the variable to the vector:
                                variableNames.push_back(pCurrentVariable->_variableName);
                            }
                        }
                    }
                }
            }

            // If this is a function, the next frame will be a higher level:
            if ((pCurrentScope->_programScopeType == csDWARFProgram::CS_INLINE_FUNCTION_SCOPE) ||
                (pCurrentScope->_programScopeType == csDWARFProgram::CS_KERNEL_FUNCTION_SCOPE))
            {
                currentStackDepth++;
            }

            // Go to this scope's parent scope:
            pCurrentScope = pCurrentScope->_pParentProgram;
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        csDWARFParser::listVariableLocations
// Description: Returns all the variable locations that are used by any variable
//              at any time in the DWARF table.
// Return Val:  bool - Success / failure.
// Author:      Uri Shomroni
// Date:        17/4/2011
// ---------------------------------------------------------------------------
bool csDWARFParser::listVariableRegisterLocations(gtVector<gtUInt64>& variableLocations)
{
    bool retVal = false;

    // Clear the output vector:
    variableLocations.clear();

    if (_pCUProgram != NULL)
    {
        retVal = true;

        // Start with the CU program:
        gtQueue<const csDWARFProgram*> programsToCheck;
        programsToCheck.push(_pCUProgram);

        // Go over all the programs in the tree:
        while (!programsToCheck.empty())
        {
            // Get the current program:
            const csDWARFProgram* pCurrentProgram = programsToCheck.front();
            GT_IF_WITH_ASSERT(pCurrentProgram != NULL)
            {
                // Add this program's variables to the locations list:
                const gtPtrVector<csDWARFVariable*>& currentProgramVariables = pCurrentProgram->_programVariables;
                int numberOfVariables = (int)currentProgramVariables.size();

                for (int i = 0; i < numberOfVariables; i++)
                {
                    // Sanity check:
                    const csDWARFVariable* pCurrentVariable = currentProgramVariables[i];

                    if (pCurrentVariable != NULL)
                    {
                        // If this variable is placed inside a register:
                        if (pCurrentVariable->_variableLocation._variableLocationType == csDWARFVariable::CS_REGISTER)
                        {
                            // If we've got a location:
                            gtUInt64 currentVarLocation = pCurrentVariable->_variableLocation._variableLocation;

                            if (currentVarLocation != GT_UINT64_MAX)
                            {
                                // Add it to the vector:
                                variableLocations.push_back(currentVarLocation);
                            }
                        }
                    }
                }

                // If this program has a stack pointer:
                if (pCurrentProgram->_framePointerRegister != GT_UINT64_MAX)
                {
                    // Add it to the vector:
                    variableLocations.push_back(pCurrentProgram->_framePointerRegister);
                }

                // Add this program's children to the queue:
                const gtPtrVector<csDWARFProgram*>& currentProgramChildren = pCurrentProgram->_childPrograms;
                int numberOfChildren = (int)currentProgramChildren.size();

                for (int i = 0; i < numberOfChildren; i++)
                {
                    programsToCheck.push(currentProgramChildren[i]);
                }
            }

            // Dispose of the used program:
            programsToCheck.pop();
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        csDWARFParser::findAddressScope
// Description: Finds the smallest scope that contains addr
// Author:      Uri Shomroni
// Date:        20/12/2010
// ---------------------------------------------------------------------------
const csDWARFProgram* csDWARFParser::findAddressScope(csDwarfAddressType addr)
{
    const csDWARFProgram* retVal = NULL;

    // Start from the entire program:
    bool goOn = true;
    const csDWARFProgram* pCurrentScope = _pCUProgram;
    const csDWARFProgram::ProgramPCRange* pCurrentHitRange = NULL;

    while (goOn && (pCurrentScope != NULL))
    {
        // We assume all child scopes are distinct, so we look for one that has the address:
        // TO_DO: check if the scopes are indeed distinct
        const gtPtrVector<csDWARFProgram*>& childPrograms = pCurrentScope->_childPrograms;
        int numOfChildScopes = (int)childPrograms.size();
        const csDWARFProgram* pFoundScope = NULL;

        for (int i = 0; i < numOfChildScopes; i++)
        {
            // Check if this child program contains the address:
            const csDWARFProgram* pChildScope = childPrograms[i];
            GT_IF_WITH_ASSERT(pChildScope != NULL)
            {
                int numberOfPCRanges = (int)pChildScope->_programPCRanges.size();

                if (numberOfPCRanges > 0)
                {
                    bool foundStrictSubset = false;

                    // Try to find one that contains the address:
                    for (int j = 0; j < numberOfPCRanges; j++)
                    {
                        const csDWARFProgram::ProgramPCRange& currentPCRange = pChildScope->_programPCRanges[j];

                        // If this range is a match:
                        if ((currentPCRange._startPC <= addr) && (currentPCRange._endPC > addr))
                        {
                            // If we have scopes that have no size of their own, they might be garbage, so keep looking.
                            // However, if the scope is really smaller, it's sure to be a subset:
                            if (pCurrentHitRange == NULL)
                            {
                                foundStrictSubset = true;
                            }
                            else if ((pCurrentHitRange->_startPC != currentPCRange._startPC) ||
                                     (pCurrentHitRange->_endPC != currentPCRange._endPC))
                            {
                                foundStrictSubset = true;
                            }

                            // Stop looking in this (child) program:
                            pFoundScope = pChildScope;
                            pCurrentHitRange = &currentPCRange;
                            break; // for (j)
                        }
                    }

                    // Unless we found a strict subset (i.e. a subset which is a smaller range), continue looking for a better match:
                    if (foundStrictSubset)
                    {
                        break; // for (i)
                    }
                }
                else if (pFoundScope == NULL) // && (numberOfPCRanges == 0)
                {
                    // If we only found this sort of scopes, we can only guess which one of them is the correct one:
                    pFoundScope = pChildScope;
                }
                else if ((pFoundScope->_childPrograms.size() == 0) && (pChildScope->_childPrograms.size() > 0)) // && (pFoundScope == NULL) && (numberOfPCRanges == 0)
                {
                    // Scopes with children are less likely to be inlined functions virtual sources:
                    pFoundScope = pChildScope;
                }
            }
        }

        if (pFoundScope != NULL)
        {
            // One of the child scopes has the address, move to the next iteration:
            pCurrentScope = pFoundScope;
        }
        else // pFoundScope == NULL
        {
            // None of the child scopes has the address, so the current scope is the smallest containing it:
            goOn = false;
        }
    }

    // Use the smallest scope we found:
    retVal = pCurrentScope;

    // If no scope contains this address, return NULL instead:
    if (retVal != NULL)
    {
        if (pCurrentHitRange == NULL)
        {
            // This will happen if no numeric range contains the address, the retVal will be the main CU program
            // or one of its abstract children:
            retVal = NULL;
        }
        else if ((pCurrentHitRange->_startPC > addr) || (pCurrentHitRange->_endPC < addr))
        {
            // This is just a sanity check:
            retVal = NULL;
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        csDWARFParser::findClosestScopeContainingVariable
// Description: Finds the smallest scope that contains startAddr and defines a
//              variable named varName. If ppVar is not NULL and the variable is
//              found, returns it.
// Author:      Uri Shomroni
// Date:        20/12/2010
// ---------------------------------------------------------------------------
const csDWARFProgram* csDWARFParser::findClosestScopeContainingVariable(csDwarfAddressType startAddr, const gtString& varName, const csDWARFVariable** ppVar)
{
    const csDWARFProgram* retVal = NULL;

    // Find the smallest scope that contains startAddr:
    const csDWARFProgram* pCurrentScope = findAddressScope(startAddr);

    while (pCurrentScope != NULL)
    {
        // See if this scope contains a variable named correctly:
        const gtPtrVector<csDWARFVariable*>& currentScopeVars = pCurrentScope->_programVariables;
        int numberOfVars = (int)currentScopeVars.size();
        bool foundVar = false;

        for (int i = 0; i < numberOfVars; i++)
        {
            // Check if this variable is the correct one:
            const csDWARFVariable* pCurrentVar = currentScopeVars[i];
            GT_IF_WITH_ASSERT(pCurrentVar != NULL)
            {
                const csDWARFVariable* pFoundMember = NULL;

                if (pCurrentVar->canMatchMemberName(varName, pFoundMember))
                {
                    // If the address is in the variable's scope:
                    if ((pCurrentVar->_variableLocation._variableHighestPC >= startAddr) && (pCurrentVar->_variableLocation._variableLowestPC <= startAddr))
                    {
                        // We found the variable!
                        foundVar = true;

                        if (ppVar != NULL)
                        {
                            // Return it if required:
                            *ppVar = pFoundMember;
                        }

                        // If this value does not have a location, try to look for one that does:
                        if (pCurrentVar->_variableLocation._variableLocation != GT_UINT64_MAX)
                        {
                            break;
                        }
                    }
                }
            }
        }

        if (!foundVar)
        {
            // Rise up in the hierarchy until you find the variable:
            pCurrentScope = pCurrentScope->_pParentProgram;
        }
        else // foundVar
        {
            // This scope is the smallest that contains a matching variable, return it:
            retVal = pCurrentScope;
            break;
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        csDWARFParser::getFramePointerRegister
// Description: Gets the reigster that holds the frame pointer for varName at startAddr
// Author:      Uri Shomroni
// Date:        26/7/2011
// ---------------------------------------------------------------------------
gtUInt64 csDWARFParser::getFramePointerRegister(csDwarfAddressType startAddr, const gtString& varName)
{
    gtUInt64 retVal = GT_UINT64_MAX;

    // Get the variable's scope:
    const csDWARFProgram* pCurrentProgram = findClosestScopeContainingVariable(startAddr, varName, NULL);

    while (pCurrentProgram != NULL)
    {
        // If we have a frame pointer:
        if (pCurrentProgram->_framePointerRegister != GT_UINT64_MAX)
        {
            // return it:
            retVal = pCurrentProgram->_framePointerRegister;
            break;
        }

        // Continue to the next parent:
        pCurrentProgram = pCurrentProgram->_pParentProgram;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        csDWARFParser::addressStackDepth
// Description: Checks how many functions are invoked by getting into the address's
//              scope
// Author:      Uri Shomroni
// Date:        20/4/2011
// ---------------------------------------------------------------------------
int csDWARFParser::addressStackDepth(csDwarfAddressType addr)
{
    int retVal = 0;

    // Go up the frame hierarchy looking for functions:
    const csDWARFProgram* pCurrentProgram = findAddressScope(addr);

    while (pCurrentProgram != NULL)
    {
        // If this is a function, increment the counter:
        if ((csDWARFProgram::CS_KERNEL_FUNCTION_SCOPE == pCurrentProgram->_programScopeType) || (csDWARFProgram::CS_INLINE_FUNCTION_SCOPE == pCurrentProgram->_programScopeType))
        {
            retVal++;
        }

        // Go up one level:
        pCurrentProgram = pCurrentProgram->_pParentProgram;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        csDWARFParser::initializeLineNumberInformation
// Description: After successfully initializing a DWARF session, extracts the line
//              information from it, and stores it for usage:
// Return Val:  bool - Success / failure.
// Author:      Uri Shomroni
// Date:        18/11/2010
// ---------------------------------------------------------------------------
bool csDWARFParser::initializeLineNumberInformation(Dwarf_Die cuDIE)
{
    bool retVal = false;

    // Get the source lines data:
    Dwarf_Error err;
    memset((void*)&err, 0, sizeof(Dwarf_Error));
    Dwarf_Line* pLines = NULL;
    Dwarf_Signed numberOfLines = 0;
    int rc = dwarf_srclines(cuDIE, &pLines, &numberOfLines, &err);

    if (rc == DW_DLV_OK)
    {
        retVal = true;

        // Iterate the lines:
        for (Dwarf_Signed i = 0; i < numberOfLines; i++)
        {
            // Ignore end sequence "lines", since they simply mark the line number for the end of a scope:
            Dwarf_Bool isESEQ = 0;
            rc = dwarf_lineendsequence(pLines[i], &isESEQ, &err);

            if ((rc == DW_DLV_OK) && (isESEQ == 0))
            {
                // The first file is the kernel main source. We get its path from the OpenCL spy,
                // so we do not need the temp file path from the compiler:
                Dwarf_Unsigned fileIndex = 0;
                gtString sourceFilePathAsString = _firstSourceFileRealPath;
                rc = dwarf_line_srcfileno(pLines[i], &fileIndex, &err);

                if ((rc == DW_DLV_OK) && (fileIndex > 1))
                {
                    // Get the source file path:
                    char* fileNameAsCharArray = NULL;
                    rc = dwarf_linesrc(pLines[i], &fileNameAsCharArray, &err);

                    if ((rc == DW_DLV_OK) && (fileNameAsCharArray != NULL))
                    {
                        // Convert it to a gtString:
                        sourceFilePathAsString.fromASCIIString(fileNameAsCharArray);
                        dwarf_dealloc(_pDwarf, (Dwarf_Ptr)fileNameAsCharArray, DW_DLA_STRING);
                    }
                }

                // Get the current line number:
                Dwarf_Unsigned lineNum = 0;
                rc = dwarf_lineno(pLines[i], &lineNum, &err);

                if (rc == DW_DLV_OK)
                {
                    // Get the current line address:
                    Dwarf_Addr lineAddress = 0;
                    rc = dwarf_lineaddr(pLines[i], &lineAddress, &err);

                    if (rc == DW_DLV_OK)
                    {
                        // Mark the connections between these two values:
                        csDWARFCodeLocation currentCodeLocation(sourceFilePathAsString, (int)lineNum);
                        connectLineNumberWithProgramCounter(currentCodeLocation, lineAddress);
                    }
                }
            }

            // Release the line struct:
            dwarf_dealloc(_pDwarf, (Dwarf_Ptr)pLines[i], DW_DLA_LINE);
        }

        // Release the lines buffer:
        dwarf_dealloc(_pDwarf, (Dwarf_Ptr)pLines, DW_DLA_LIST);
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        csDWARFParser::fillProgramWithInformationFromDIE
// Description: Recursively creates the program / subprogram tree and fills it with
//              the variables defined in each scope. Note that we expect the
//              _programScopeType member to be set by the parent, so we won't
//              need to check that attribute here.
// Author:      Uri Shomroni
// Date:        24/11/2010
// ---------------------------------------------------------------------------
void csDWARFParser::fillProgramWithInformationFromDIE(csDWARFProgram& programData, csDWARFProgram* pParentProgram, Dwarf_Die programDIE)
{
    Dwarf_Error err ;
    memset((void*)&err, 0, sizeof(Dwarf_Error));

    // Copy the parent pointer:
    programData._pParentProgram = pParentProgram;

    // Fill the current program's details with information from the DIE:
    // Get the high and low program counters:
    // Dwarf_Unsigned lowPCVal = 0, highPCVal = 0;
    // int rcLVal = dwarf_attrval_unsigned(programDIE, DW_AT_low_pc, &lowPCVal, &err);
    // int rcHVal = dwarf_attrval_unsigned(programDIE, DW_AT_high_pc, &highPCVal, &err);
    Dwarf_Addr lowPCVal = 0;
    Dwarf_Addr highPCVal = 0;
    int rcLVal = dwarf_lowpc(programDIE, &lowPCVal, &err);
    int rcHVal = dwarf_highpc(programDIE, &highPCVal, &err);

    if ((rcLVal == DW_DLV_OK) && (rcHVal == DW_DLV_OK) && (lowPCVal <= highPCVal))
    {
        // If we got both values and the are ordered logically, set them to the class:
        csDWARFProgram::ProgramPCRange pcRange((csDwarfAddressType)lowPCVal, (csDwarfAddressType)highPCVal);
        programData._programPCRanges.push_back(pcRange);
    }
    else if (rcLVal == DW_DLV_OK)
    {
        // DWARF also allows only the low value to exist - this means a one-instruction range:
        csDWARFProgram::ProgramPCRange pcRange((csDwarfAddressType)lowPCVal, (csDwarfAddressType)lowPCVal);
        programData._programPCRanges.push_back(pcRange);
    }

    // Look for the ranges attribute, which supplies multiple ranges:
    Dwarf_Attribute rangesAsAttr = NULL;
    int rcRAtt = dwarf_attr(programDIE, DW_AT_ranges, &rangesAsAttr, &err);

    if ((rcRAtt == DW_DLV_OK) && (rangesAsAttr != NULL))
    {
        // Form it into a section offset:
        Dwarf_Off rangesOffset = 0;
        int rcROff = dwarf_global_formref(rangesAsAttr, &rangesOffset, &err);

        if (rcROff != DW_DLV_OK)
        {
            // Try reading it as a non-global address:
            rcROff = dwarf_formref(rangesAsAttr, &rangesOffset, &err);

            if (rcROff != DW_DLV_OK)
            {
                // Prior to DWARF version 3, offsets were encoded as U4 or U8:
                Dwarf_Unsigned rangesOffsetAsUnsigned = 0;
                rcROff = dwarf_formudata(rangesAsAttr, &rangesOffsetAsUnsigned, &err);

                if (rcROff == DW_DLV_OK)
                {
                    rangesOffset = (Dwarf_Off)rangesOffsetAsUnsigned;
                }
            }
        }

        if (rcROff == DW_DLV_OK)
        {
            Dwarf_Ranges* pRangesList = NULL;
            Dwarf_Signed rangesCount = 0;
            int rcRng = dwarf_get_ranges(_pDwarf, rangesOffset, &pRangesList, &rangesCount, NULL, &err);

            if ((rcRng == DW_DLV_OK) && (pRangesList != NULL) && (rangesCount > 0))
            {
                // Iterate the ranges:
                for (Dwarf_Signed i = 0; i < rangesCount; i++)
                {
                    switch (pRangesList[i].dwr_type)
                    {
                        case DW_RANGES_ENTRY:
                        {
                            csDwarfAddressType loAddr = (csDwarfAddressType)pRangesList[i].dwr_addr1;
                            csDwarfAddressType hiAddr = (csDwarfAddressType)pRangesList[i].dwr_addr2;

                            if ((loAddr != 0) && (hiAddr != 0) && (hiAddr >= loAddr))
                            {
                                // Add the additional ranges:
                                csDWARFProgram::ProgramPCRange pcRange(loAddr, hiAddr);
                                programData._programPCRanges.push_back(pcRange);
                            }
                        }
                        break;

                        case DW_RANGES_ADDRESS_SELECTION:
                            // We currently don't handle direct addresses, and these shouldn't be generated:
                            GT_ASSERT(false);
                            break;

                        case DW_RANGES_END:
                            // Ignore the end marker
                            break;

                        default:
                            // Unexpected value!
                            GT_ASSERT(false);
                            break;
                    }
                }

                // Release the range list:
                dwarf_ranges_dealloc(_pDwarf, pRangesList, rangesCount);
            }
        }

        // Release the attribute:
        dwarf_dealloc(_pDwarf, (Dwarf_Ptr)rangesAsAttr, DW_DLA_ATTR);
    }

    // Get the name (if available):
    const char* programNameAsCharArray = NULL;
    // int rcNm = dwarf_attrval_string(programDIE, DW_AT_name, &programNameAsCharArray, &err);
    int rcNm = dwarf_diename(programDIE, (char**)&programNameAsCharArray, &err);

    if ((rcNm == DW_DLV_OK) && (programNameAsCharArray != NULL))
    {
        // Copy the name:
        programData._programName.fromASCIIString(programNameAsCharArray);

        // Release the string:
        dwarf_dealloc(_pDwarf, (Dwarf_Ptr)programNameAsCharArray, DW_DLA_STRING);
    }

    // Get the frame pointer (if available):
    Dwarf_Attribute fbLocDescAsAttribute = NULL;
    int rcAddr = dwarf_attr(programDIE, DW_AT_frame_base, &fbLocDescAsAttribute, &err);

    if ((rcAddr == DW_DLV_OK) && (fbLocDescAsAttribute != NULL))
    {
        // Get the location list from the attribute:
        Dwarf_Locdesc* pFramePointerLocation = NULL;
        Dwarf_Signed numberOfLocations = 0;

        // Before calling the loclist function, make sure the attribute is the correct format (one of the block formats).
        // See BUG365690 - the compiler sometime emits numbers instead of data blocks here, and our DWARF implementation does not
        // verify the input of dwarf_loclist.
        Dwarf_Half attrFormat = DW_FORM_block;
        rcAddr = dwarf_whatform(fbLocDescAsAttribute, &attrFormat, &err);

        if ((DW_DLV_OK == rcAddr) &&
            ((DW_FORM_block == attrFormat) || (DW_FORM_block1 == attrFormat) || (DW_FORM_block2 == attrFormat) || (DW_FORM_block4 == attrFormat)))
        {
            rcAddr = dwarf_loclist(fbLocDescAsAttribute, &pFramePointerLocation, &numberOfLocations, &err);
        }

        if ((rcAddr == DW_DLV_OK) && (pFramePointerLocation != NULL) && (0 < numberOfLocations))
        {
            if (pFramePointerLocation->ld_s != NULL)
            {
                csDWARFVariable::ValueLocationType locationType = csDWARFVariable::CS_UNKNOWN_VARIABLE_LOCATION;
                programData._framePointerRegister = locationFromDWARFData(pFramePointerLocation->ld_s->lr_atom, pFramePointerLocation->ld_s->lr_number, locationType);

                // We expect the frame pointer to be stored in a register with no added offset::
                GT_ASSERT(locationType == csDWARFVariable::CS_REGISTER);
                GT_ASSERT(pFramePointerLocation->ld_s->lr_offset == 0);

                // Release the location block:
                dwarf_dealloc(_pDwarf, (Dwarf_Ptr)(pFramePointerLocation->ld_s), DW_DLA_LOC_BLOCK);
            }

            // Release the other locations blocks:
            for (Dwarf_Signed i = 1; numberOfLocations > i; i++)
            {
                if (NULL != pFramePointerLocation[i].ld_s)
                {
                    dwarf_dealloc(_pDwarf, (Dwarf_Ptr)(pFramePointerLocation[i].ld_s), DW_DLA_LOC_BLOCK);
                }
            }

            // Release the location list:
            GT_ASSERT(numberOfLocations == 1);
            dwarf_dealloc(_pDwarf, (Dwarf_Ptr)pFramePointerLocation, DW_DLA_LOCDESC);
        }
    }

    // Iterate this DIE's Children, and create program and variable data objects for them:
    Dwarf_Die currentChild = NULL;
    int rc = dwarf_child(programDIE, &currentChild, &err);
    bool goOn = ((rc == DW_DLV_OK) && (currentChild != NULL));

    while (goOn)
    {
        // Get the current child's DWARF TAG:
        Dwarf_Half currentChildTag = 0;
        rc = dwarf_tag(currentChild, &currentChildTag, &err);
        GT_IF_WITH_ASSERT(rc == DW_DLV_OK)
        {
            // Check what kind of DIE this is:
            switch (currentChildTag)
            {
                case DW_TAG_array_type:
                case DW_TAG_class_type:
                case DW_TAG_enumeration_type:
                case DW_TAG_member:
                case DW_TAG_pointer_type:
                case DW_TAG_string_type:
                case DW_TAG_structure_type:
                case DW_TAG_typedef:
                case DW_TAG_union_type:
                case DW_TAG_base_type:
                case DW_TAG_const_type:
                {
                    // TO_DO: decide on how to handle types.
                }
                break;

                case DW_TAG_entry_point:
                case DW_TAG_lexical_block:
                case DW_TAG_inlined_subroutine:
                case DW_TAG_subprogram:
                {
                    // This is a sub-program, create a program object for it:
                    csDWARFProgram* pChildProgram = new csDWARFProgram;
                    pChildProgram->_programScopeType = csDWARFProgram::programScopeTypeFromTAG(currentChildTag);
                    bool shouldAddSubprogram = true;

                    // If this is an inlined function, get its function details
                    if (pChildProgram->_programScopeType == csDWARFProgram::CS_INLINE_FUNCTION_SCOPE)
                    {
                        fillInlinedFunctinoProgramWithInformationFromDIE(*pChildProgram, currentChild);
                    }
                    else // pChildProgram->_programScopeType != csDWARFProgram::CS_INLINE_FUNCTION_SCOPE
                    {
                        // If this is a subprogram DIE, make sure it is not the abstract representation of
                        // an inlined function:
                        Dwarf_Bool isInlined = 0;
                        rc = dwarf_attrval_flag(currentChild, DW_AT_inline, &isInlined, &err);

                        if ((rc == DW_DLV_OK) && (isInlined == 1))
                        {
                            shouldAddSubprogram = false;
                        }
                    }

                    if (shouldAddSubprogram)
                    {
                        // Recursively fill the program objects:
                        fillProgramWithInformationFromDIE(*pChildProgram, &programData, currentChild);

                        // Add it to our children vector:
                        programData._childPrograms.push_back(pChildProgram);
                    }
                    else // !shouldAddSubprogram
                    {
                        // Dispose of it:
                        delete pChildProgram;
                    }
                }
                break;

                case DW_TAG_compile_unit:
                {
                    // We do not expect compilation units to be children of any other DIE type!
                    GT_ASSERT(false);
                }
                break;

                case DW_TAG_formal_parameter:
                case DW_TAG_constant:
                case DW_TAG_enumerator:
                case DW_TAG_variable:
                {
                    // This is a variable, create a variable object:
                    csDWARFVariable* pVariable = new csDWARFVariable;
                    pVariable->_valueType = csDWARFVariable::valueTypeFromTAG(currentChildTag);
                    gtVector<csDWARFVariable::csDWARFVariableLocation> variableAdditionalLocations;
                    fillVariableWithInformationFromDIE(*pVariable, currentChild, variableAdditionalLocations, false);

                    // Add it to our variables vector:
                    programData._programVariables.push_back(pVariable);

                    // Also add any duplicates it has with other locations:
                    int numberOfAdditionalLocations = (int)variableAdditionalLocations.size();

                    for (int i = 0; i < numberOfAdditionalLocations; i++)
                    {
                        // Copy the name and other metadata:
                        csDWARFVariable* pVariableAdditionalLocation = new csDWARFVariable;
                        *pVariableAdditionalLocation = *pVariable;

                        // Copy the different location:
                        pVariableAdditionalLocation->_variableLocation = variableAdditionalLocations[i];

                        // Add the variable to the vector:
                        programData._programVariables.push_back(pVariableAdditionalLocation);
                    }
                }
                break;

                default:
                {
                    // DW_TAG_imported_declaration, DW_TAG_label, DW_TAG_reference_type, DW_TAG_subroutine_type, DW_TAG_unspecified_parameters
                    // DW_TAG_variant, DW_TAG_common_block, DW_TAG_common_inclusion, DW_TAG_inheritance, DW_TAG_module, DW_TAG_ptr_to_member_type
                    // DW_TAG_set_type, DW_TAG_subrange_type, DW_TAG_with_stmt, DW_TAG_access_declaration, DW_TAG_catch_block, DW_TAG_friend
                    // DW_TAG_namelist, DW_TAG_namelist_item, DW_TAG_packed_type, DW_TAG_template_type_parameter = DW_TAG_template_type_param,
                    // DW_TAG_template_value_parameter = DW_TAG_template_value_param, DW_TAG_thrown_type, DW_TAG_try_block, DW_TAG_variant_part,
                    // DW_TAG_volatile_type, DW_TAG_dwarf_procedure, DW_TAG_restrict_type, DW_TAG_interface_type, DW_TAG_namespace,
                    // DW_TAG_imported_module, DW_TAG_unspecified_type, DW_TAG_partial_unit, DW_TAG_imported_unit, DW_TAG_condition,
                    // DW_TAG_shared_type, DW_TAG_type_unit, DW_TAG_rvalue_reference_type, DW_TAG_template_alias
                    // and user types are not currently handled.
                }
                break;
            }
        }

        // Get the next child:
        Dwarf_Die nextChild = NULL;
        rc = dwarf_siblingof(_pDwarf, currentChild, &nextChild, &err);

        // Release the current child:
        dwarf_dealloc(_pDwarf, (Dwarf_Ptr)currentChild, DW_DLA_DIE);

        // Move to the next iteration:
        currentChild = nextChild;
        goOn = ((rc == DW_DLV_OK) && (currentChild != NULL));
    }

    // Intersect the variables in this program:
    intersectVariablesInProgram(programData);

    // Make sure we didn't stop on an error:
    GT_ASSERT(rc == DW_DLV_NO_ENTRY);
}

// ---------------------------------------------------------------------------
// Name:        csDWARFParser::fillInlinedFunctinoProgramWithInformationFromDIE
// Description: Fills an inlined function special parameters from the DIE
// Author:      Uri Shomroni
// Date:        20/4/2011
// ---------------------------------------------------------------------------
void csDWARFParser::fillInlinedFunctinoProgramWithInformationFromDIE(csDWARFProgram& programData, Dwarf_Die programDIE)
{
    // Get the line number:
    Dwarf_Error err;
    memset((void*)&err, 0, sizeof(Dwarf_Error));
    Dwarf_Unsigned callLineNumber = 0;
    int rc = dwarf_attrval_unsigned(programDIE, DW_AT_call_line, &callLineNumber, &err);

    if (rc == DW_DLV_OK)
    {
        programData._inlinedFunctionCodeLocation._lineNumber = (int)callLineNumber;
    }

    // Get the file name:
    Dwarf_Unsigned callFileNumber = 0;
    rc = dwarf_attrval_unsigned(programDIE, DW_AT_call_file, &callFileNumber, &err);

    if ((rc == DW_DLV_OK) && (callFileNumber > 0))
    {
        // The first file is the kernel main source. We get its path from the OpenCL spy,
        // so we do not need the temp file path from the compiler:
        programData._inlinedFunctionCodeLocation._sourceFileFullPath = _firstSourceFileRealPath;

        if (callFileNumber > 1)
        {
            // Get the CU DIE:
            Dwarf_Die cuDIE = NULL;
            rc = dwarf_siblingof(_pDwarf, NULL, &cuDIE, NULL);

            if ((rc == DW_DLV_OK) && (cuDIE != NULL))
            {
                // Get the file names:
                char** pSourceFilesAsCharArrays = NULL;
                Dwarf_Signed numberOfSourceFiles = -1;
                rc = dwarf_srcfiles(cuDIE, &pSourceFilesAsCharArrays, &numberOfSourceFiles, &err);

                if ((rc == DW_DLV_OK) && (pSourceFilesAsCharArrays != NULL) && (numberOfSourceFiles > 0))
                {
                    if (numberOfSourceFiles >= (Dwarf_Signed)callFileNumber)
                    {
                        // The index given by the DIE is 1-based:
                        programData._inlinedFunctionCodeLocation._sourceFileFullPath.fromASCIIString(pSourceFilesAsCharArrays[callFileNumber - 1]);
                    }

                    // Release the strings:
                    for (Dwarf_Signed i = 0; i < numberOfSourceFiles; i++)
                    {
                        dwarf_dealloc(_pDwarf, (Dwarf_Ptr)(pSourceFilesAsCharArrays[i]), DW_DLA_STRING);
                        pSourceFilesAsCharArrays[i] = NULL;
                    }

                    // Release the string list:
                    dwarf_dealloc(_pDwarf, (Dwarf_Ptr)pSourceFilesAsCharArrays, DW_DLA_LIST);
                }

                // Release the CU DIE:
                dwarf_dealloc(_pDwarf, (Dwarf_Ptr)cuDIE, DW_DLA_DIE);
            }
        }
    }

    // Get the function name and variables:
    Dwarf_Attribute functionAbstractOriginAsAttribute = NULL;
    rc = dwarf_attr(programDIE, DW_AT_abstract_origin, &functionAbstractOriginAsAttribute, &err);

    if ((rc == DW_DLV_OK) && (functionAbstractOriginAsAttribute != NULL))
    {
        // Get the function abstract origin DIE:
        Dwarf_Die functionAbstractOriginDIE = NULL;
        rc = dwarf_formref_die(functionAbstractOriginAsAttribute, &functionAbstractOriginDIE, &err, _pDwarf);

        if ((rc == DW_DLV_OK) && (functionAbstractOriginDIE != NULL))
        {
            // Get the function name:
            char* pFunctionNameAsString = NULL;
            rc = dwarf_diename(functionAbstractOriginDIE, &pFunctionNameAsString, &err);

            if ((rc == DW_DLV_OK) && (pFunctionNameAsString != NULL))
            {
                // Copy the function name:
                programData._inlinedFunctionName.fromASCIIString(pFunctionNameAsString);

                // Release the string:
                dwarf_dealloc(_pDwarf, (Dwarf_Ptr)pFunctionNameAsString, DW_DLA_STRING);
            }

            // Release the DIE:
            dwarf_dealloc(_pDwarf, (Dwarf_Ptr)functionAbstractOriginDIE, DW_DLA_DIE);
        }

        // Release the attribute:
        dwarf_dealloc(_pDwarf, (Dwarf_Ptr)functionAbstractOriginAsAttribute, DW_DLA_ATTR);
    }
}

// ---------------------------------------------------------------------------
// Name:        csDWARFParser::fillVariableWithInformationFromDIE
// Description: Fills variableData with the details from the DWARF DIE.
//              Note that we expect the _programScopeType member to be set by
//              the parent, so we won't need to check that attribute here.
// Author:      Uri Shomroni
// Date:        25/11/2010
// ---------------------------------------------------------------------------
void csDWARFParser::fillVariableWithInformationFromDIE(csDWARFVariable& variableData, Dwarf_Die variableDIE, gtVector<csDWARFVariable::csDWARFVariableLocation>& variableAdditionalLocations, bool isMember)
{
    // Get the variable's location attribute:
    variableAdditionalLocations.clear();
    Dwarf_Attribute varLocDescAsAttribute = NULL;
    Dwarf_Error err;
    memset((void*)&err, 0, sizeof(Dwarf_Error));
    int rc = dwarf_attr(variableDIE, isMember ? DW_AT_data_member_location : DW_AT_location, &varLocDescAsAttribute, &err);

    if ((rc == DW_DLV_OK) && (varLocDescAsAttribute != NULL))
    {
        // Get the location list from the attribute:
        Dwarf_Locdesc* pVarLocationDescriptions = NULL;
        Dwarf_Signed locationsCount = 0;

        // Before calling the loclist function, make sure the attribute is the correct format (one of the block formats).
        // See BUG365690 - the compiler sometime emits numbers instead of data blocks here, and our DWARF implementation does not
        // verify the input of dwarf_loclist.
        Dwarf_Half attrFormat = DW_FORM_block;
        rc = dwarf_whatform(varLocDescAsAttribute, &attrFormat, &err);

        if ((DW_DLV_OK == rc) &&
            ((DW_FORM_block == attrFormat) || (DW_FORM_block1 == attrFormat) || (DW_FORM_block2 == attrFormat) || (DW_FORM_block4 == attrFormat)))
        {
            rc = dwarf_loclist(varLocDescAsAttribute, &pVarLocationDescriptions, &locationsCount, &err);
        }

        if ((rc == DW_DLV_OK) && (pVarLocationDescriptions != NULL))
        {
            // If this variable has at least one location:
            bool isFirstLocation = true;

            for (int i = 0; i < locationsCount; i++)
            {
                // Get the details and fill the variable data with them:
                csDWARFVariable::csDWARFVariableLocation variableCurrentLocation = variableData._variableLocation;

                if (!isMember)
                {
                    variableCurrentLocation._variableLowestPC = (csDwarfAddressType)(pVarLocationDescriptions[i].ld_lopc);
                    variableCurrentLocation._variableHighestPC = (csDwarfAddressType)(pVarLocationDescriptions[i].ld_hipc);
                    variableCurrentLocation._variableHighestPCValid = true;
                }

                Dwarf_Loc* pLocationRecord = pVarLocationDescriptions[i].ld_s;

                if (pLocationRecord != NULL)
                {
                    // Read all the operations:
                    int numberOfLocationsOperations = (int)pVarLocationDescriptions[i].ld_cents;
                    int newValueSize = -1;
                    bool locationFound = false;

                    for (int j = 0; j < numberOfLocationsOperations; j++)
                    {
                        Dwarf_Loc& rCurrentLocationOperation = pLocationRecord[j];

                        switch (rCurrentLocationOperation.lr_atom)
                        {
                            case DW_OP_piece:
                            {
                                newValueSize = (int)rCurrentLocationOperation.lr_number;
                            }
                            break;

                            case DW_OP_bit_piece:
                            {
                                newValueSize = ((int)rCurrentLocationOperation.lr_number + 7) / 8;
                                variableCurrentLocation._variableLocationOffset += (((gtInt32)rCurrentLocationOperation.lr_number2) + 7) / 8;
                            }
                            break;

                            case DW_OP_dup:
                            case DW_OP_drop:
                            case DW_OP_over:
                            case DW_OP_pick:
                            case DW_OP_swap:
                            case DW_OP_rot:
                            case DW_OP_abs:
                            case DW_OP_and:
                            case DW_OP_div:
                            case DW_OP_minus:
                            case DW_OP_mod:
                            case DW_OP_mul:
                            case DW_OP_neg:
                            case DW_OP_not:
                            case DW_OP_or:
                            case DW_OP_plus:
                            case DW_OP_xor:
                            case DW_OP_eq:
                            case DW_OP_ge:
                            case DW_OP_gt:
                            case DW_OP_le:
                            case DW_OP_lt:
                            case DW_OP_ne:
                            case DW_OP_skip:
                                // We do not currently support these operations, which require maintaining an expression stack:
                                GT_ASSERT(false);
                                break;

                            default:
                            {
                                // Get the location (currently, a simple register number or a frame pointer offset):
                                if ((!isMember) || (variableCurrentLocation._variableLocation == GT_UINT64_MAX))
                                {
                                    variableCurrentLocation._variableLocation = locationFromDWARFData((gtUByte)rCurrentLocationOperation.lr_atom, (gtUInt64)rCurrentLocationOperation.lr_number, variableCurrentLocation._variableLocationType);

                                    // We do not expect to get here twice:
                                    GT_ASSERT(!locationFound);
                                    locationFound = true;
                                }

                                // Get the offset:
                                // For DW_OP_plus_uconst location and DW_OP_breg1 - DW_OP_breg31, the offset is the main operand plus the offset value
                                // For DW_OP_bregx location, the offset is the second operand plus the offset value
                                // For all other locations, the offset is the offset value.
                                variableCurrentLocation._variableLocationOffset = (((rCurrentLocationOperation.lr_atom == DW_OP_plus_uconst) || (variableCurrentLocation._variableLocation == csDWARFVariable::CS_INDIRECT_REGISTER)) ? (gtInt32)rCurrentLocationOperation.lr_number :
                                                                                   (rCurrentLocationOperation.lr_atom == DW_OP_bregx) ? (gtInt32)rCurrentLocationOperation.lr_number2 :
                                                                                   0)
                                                                                  + (gtInt32)rCurrentLocationOperation.lr_offset;
                            }
                            break;
                        }
                    }

                    if (newValueSize > -1)
                    {
                        variableData._valueSize = newValueSize;
                    }

                    // Sum up the accumulated offset:
                    variableCurrentLocation._variableLocationAccumulatedOffset += variableCurrentLocation._variableLocationOffset;
                }
                else // pLocationRecord == NULL
                {
                    // The last location in every list longer than 1 is a NULL entry used to mark the end of the list:
                    GT_ASSERT((locationsCount > 1) && (i == (locationsCount - 1)));
                }

                if (isFirstLocation)
                {
                    variableData._variableLocation = variableCurrentLocation;
                    isFirstLocation = false;
                }
                else // !isFirstLocation
                {
                    variableAdditionalLocations.push_back(variableCurrentLocation);
                }

                dwarf_dealloc(_pDwarf, (Dwarf_Ptr)(pVarLocationDescriptions[i].ld_s), DW_DLA_LOC_BLOCK);
                // The locdesc-s are allocated as a single block - released below:
                // dwarf_dealloc(_pDwarf, (Dwarf_Ptr)(&pVarLocationDescriptions[i]), DW_DLA_LOCDESC);
            }

            dwarf_dealloc(_pDwarf, (Dwarf_Ptr)pVarLocationDescriptions, DW_DLA_LOCDESC);
        }

        // Release the attribute:
        dwarf_dealloc(_pDwarf, (Dwarf_Ptr)varLocDescAsAttribute, DW_DLA_ATTR);
    }

    // Get the address space attribute, if it is available:
    Dwarf_Unsigned addressSpaceAsDwarfUnsigned = 0;
    rc = dwarf_attrval_unsigned(variableDIE, DW_AT_AMDIL_address_space, &addressSpaceAsDwarfUnsigned, &err);

    if (rc == DW_DLV_OK)
    {
        switch (addressSpaceAsDwarfUnsigned)
        {
            case 1:
                variableData._valuePointerAddressSpace = csDWARFVariable::CS_GLOBAL_POINTER;
                break;

            case 2:
                variableData._valuePointerAddressSpace = csDWARFVariable::CS_CONSTANT_POINTER;
                break;

            case 3:
                variableData._valuePointerAddressSpace = csDWARFVariable::CS_LOCAL_POINTER;
                break;

            default:
                // The attribute is present, but has an unknown value:
                variableData._valuePointerAddressSpace = csDWARFVariable::CS_UNKNOWN_POINTER;
                break;
        }
    }

    // Get the resource attribute, if it is available:
    Dwarf_Unsigned resourceAsDwarfUnsigned = 0;
    rc = dwarf_attrval_unsigned(variableDIE, DW_AT_AMDIL_resource, &resourceAsDwarfUnsigned, &err);

    if (rc == DW_DLV_OK)
    {
        // Note that board older than SI might not produce this attribute at all:
        variableData._variableLocation._variableLocationResource = (gtUInt64)resourceAsDwarfUnsigned;
    }

    // Get the start scope attribute, if it is available:
    Dwarf_Unsigned startScopeAsDwarfUnsigned = 0;
    rc = dwarf_attrval_unsigned(variableDIE, DW_AT_start_scope, &startScopeAsDwarfUnsigned, &err);

    if (rc == DW_DLV_OK)
    {
        // Make sure this start scope is between the given addresses:
        csDwarfAddressType startScope = (csDwarfAddressType)startScopeAsDwarfUnsigned;

        if ((variableData._variableLocation._variableHighestPC >= startScope) && (variableData._variableLocation._variableLowestPC < startScope))
        {
            // This is more detailed, use it instead:
            variableData._variableLocation._variableLowestPC = startScope;
            variableData._variableLocation._variableHighestPCValid = false;
        }
    }

    // Get the variable name:
    const char* varNameAsCharArray = NULL;
    // int rcNm = dwarf_attrval_string(variableDIE, DW_AT_name, &varNameAsCharArray, &err);
    int rcNm = dwarf_diename(variableDIE, (char**)&varNameAsCharArray, &err);

    if ((rcNm == DW_DLV_OK) && (varNameAsCharArray != NULL))
    {
        // Copy the name:
        variableData._variableName.fromASCIIString(varNameAsCharArray);

        // Release the string:
        dwarf_dealloc(_pDwarf, (Dwarf_Ptr)varNameAsCharArray, DW_DLA_STRING);
    }

    // Uri, 13/11/11 - struct parameters passed by-value are shown as the reference data to their value rather than
    // the value location itself - but DWARF still shows this as a register type. While the change is passed in the
    // address space attribute, it is shown as a __private myStruct rather than a __private myStruct& in DWARF.
    // See fogbugz case 7526.
    bool isRegisterParameter = false;

    if ((variableData._variableLocation._variableLocationType == csDWARFVariable::CS_REGISTER) && (variableData._valueType == csDWARFVariable::CS_PARAMETER_VALUE))
    {
        isRegisterParameter = true;
    }

    // Get the variable type:
    Dwarf_Attribute varTypeRefAsAttribute = NULL;
    int rcTp = dwarf_attr(variableDIE, DW_AT_type, &varTypeRefAsAttribute, &err);

    if ((rcTp == DW_DLV_OK) && (varTypeRefAsAttribute != NULL))
    {
        // Get the type DIE
        Dwarf_Die typeDIE = NULL;
        rcTp = dwarf_formref_die(varTypeRefAsAttribute, &typeDIE, &err, _pDwarf);

        if ((rcTp == DW_DLV_OK) && (typeDIE != NULL))
        {
            typeNameAndDetailsFromTypeDIE(variableData._variableType, variableData._valueSize, variableData._valueEncoding, variableData._valueIsPointer, variableData._valueIsArray, variableData._variableMembers, !isMember, variableData._variableLocation._variableLocation, variableData._variableLocation._variableLocationType, variableData._variableLocation._variableLocationResource, variableData._variableLocation._variableLocationAccumulatedOffset, typeDIE, isRegisterParameter);

            // Do not allow members to have no name:
            if (isMember && (variableData._variableName.isEmpty()))
            {
                variableData._variableName = L"unnamed_";
                Dwarf_Half currentTypeTag = 0;
                rc = dwarf_tag(typeDIE, &currentTypeTag, &err);

                if (DW_DLV_OK == rc)
                {
                    // See what kind of type this is:
                    switch (currentTypeTag)
                    {
                        case DW_TAG_array_type:
                            variableData._variableName.append(L"array_");
                            break;

                        case DW_TAG_enumeration_type:
                            variableData._variableName.append(L"enum_");
                            break;

                        case DW_TAG_pointer_type:
                            variableData._variableName.append(L"ptr_");
                            break;

                        case DW_TAG_reference_type:
                            variableData._variableName.append(L"ref_");
                            break;

                        case DW_TAG_structure_type:
                            variableData._variableName.append(L"struct_");
                            break;

                        case DW_TAG_union_type:
                            variableData._variableName.append(L"union_");
                            break;

                        case DW_TAG_class_type:
                            variableData._variableName.append(L"class_");
                            break;

                        case DW_TAG_typedef:
                        case DW_TAG_base_type:
                        case DW_TAG_const_type:
                        case DW_TAG_volatile_type:
                        case DW_TAG_string_type:
                        case DW_TAG_subroutine_type:
                        case DW_TAG_ptr_to_member_type:
                        case DW_TAG_set_type:
                        case DW_TAG_subrange_type:
                        case DW_TAG_packed_type:
                        case DW_TAG_thrown_type:
                        case DW_TAG_restrict_type:
                        case DW_TAG_interface_type:
                        case DW_TAG_unspecified_type:
                        case DW_TAG_shared_type:
                        default:
                            variableData._variableName.append(L"member_");
                            break;
                    }
                }
                else // DW_DLV_OK != rc
                {
                    variableData._variableName.append(L"member_");
                }

                // Add a serial number (spanning between types, etc):
                static int unknownMemberIndex = 0;
                variableData._variableName.appendFormattedString(L"%d", unknownMemberIndex++);
            }

            // Release the DIE:
            dwarf_dealloc(_pDwarf, (Dwarf_Ptr)typeDIE, DW_DLA_DIE);
        }

        // Release the attribute:
        dwarf_dealloc(_pDwarf, (Dwarf_Ptr)varTypeRefAsAttribute, DW_DLA_ATTR);
    }

    // As per the comment above, if the value of a parameter is a struct, it's actually an indirect value:
    if ((isRegisterParameter) && (variableData._valueEncoding == csDWARFVariable::CS_NO_ENCODING))
    {
        variableData._variableLocation._variableLocationType = csDWARFVariable::CS_INDIRECT_REGISTER;
    }

    // In OpenCL C, an unqualified pointer is a private pointer. This is not shown in the ORCA compiler:
    if (variableData._valueIsPointer &&
        ((csDWARFVariable::CS_NOT_A_POINTER == variableData._valuePointerAddressSpace) ||
         (csDWARFVariable::CS_UNKNOWN_POINTER == variableData._valuePointerAddressSpace)))
    {
        variableData._valuePointerAddressSpace = csDWARFVariable::CS_PRIVATE_POINTER;
    }

    // If the variable has a constant value, get it:
    Dwarf_Attribute constValueAttribute = NULL;
    int rcCV = dwarf_attr(variableDIE, DW_AT_const_value, &constValueAttribute, &err);

    if (rcCV == DW_DLV_OK)
    {
        Dwarf_Block* constValueBlock = NULL;
        rcCV = dwarf_formblock(constValueAttribute, &constValueBlock, &err);

        if (rcCV == DW_DLV_OK)
        {
            // Copy it into the struct:
            variableData._variableConstantValue = 0;
            ::memcpy(&variableData._variableConstantValue, constValueBlock->bl_data, min((gtSize_t)constValueBlock->bl_len, sizeof(gtUInt64)));
            variableData._variableConstantValueExists = true;

            // Release the block:
            dwarf_dealloc(_pDwarf, (Dwarf_Ptr)constValueBlock, DW_DLA_BLOCK);
        }

        // Release the attribute:
        dwarf_dealloc(_pDwarf, (Dwarf_Ptr)constValueAttribute, DW_DLA_ATTR);
    }

    // Get the function name and variables:
    Dwarf_Attribute variableAbstractOriginAsAttribute = NULL;
    rc = dwarf_attr(variableDIE, DW_AT_abstract_origin, &variableAbstractOriginAsAttribute, &err);

    if ((rc == DW_DLV_OK) && (variableAbstractOriginAsAttribute != NULL))
    {
        // Get the function abstract origin DIE:
        Dwarf_Die variableAbstractOriginDIE = NULL;
        rc = dwarf_formref_die(variableAbstractOriginAsAttribute, &variableAbstractOriginDIE, &err, _pDwarf);

        if ((rc == DW_DLV_OK) && (variableAbstractOriginDIE != NULL))
        {
            // Fill any data you can from the abstract origin. We do not need locations, since the abstract origin has none:
            gtVector<csDWARFVariable::csDWARFVariableLocation> ignoredAdditionalLocations;
            fillVariableWithInformationFromDIE(variableData, variableAbstractOriginDIE, ignoredAdditionalLocations, isMember);
            GT_ASSERT(ignoredAdditionalLocations.size() == 0);

            // Release the DIE:
            dwarf_dealloc(_pDwarf, (Dwarf_Ptr)variableAbstractOriginDIE, DW_DLA_DIE);
        }

        // Release the attribute:
        dwarf_dealloc(_pDwarf, (Dwarf_Ptr)variableAbstractOriginAsAttribute, DW_DLA_ATTR);
    }
}

// ---------------------------------------------------------------------------
// Name:        csDWARFParser::connectLineNumberWithProgramCounter
// Description: Maps lineNum to progCounter and vice-versa.
// Return Val:  bool - Success / failure.
// Author:      Uri Shomroni
// Date:        18/11/2010
// ---------------------------------------------------------------------------
bool csDWARFParser::connectLineNumberWithProgramCounter(const csDWARFCodeLocation& codeLoc, csDwarfAddressType progCounter)
{
    bool retVal = true;
    bool newPC = false;

    // Map the PC to a line number:
    _programCounterToCodeLocation[progCounter] = codeLoc;
    _programCountersMappedToLineNumbers.push_back(progCounter);

    // Add the PC to the vector of PCs mapped to the line number:
    gtMap<csDWARFCodeLocation, gtVector<csDwarfAddressType> >::iterator findIter = _codeLocationToProgramCounters.find(codeLoc);
    gtMap<csDWARFCodeLocation, gtVector<csDwarfAddressType> >::iterator endIter = _codeLocationToProgramCounters.end();

    if (findIter == endIter)
    {
        // No PCs were mapped to this line number yet, add a new vector:
        gtVector<csDwarfAddressType> programCounters;

        // Add the current PC to this vector:
        programCounters.push_back(progCounter);

        // Set it in the map:
        _codeLocationToProgramCounters[codeLoc] = programCounters;
        newPC = true;
    }
    else // findIter != endIter
    {
        // Get the vector of PCs already mapped to this line number:
        gtVector<csDwarfAddressType>& programCounters = (*findIter).second;

        // Make sure we don't add the same PC twice:
        int numberOfPCs = (int)programCounters.size();
        bool foundPC = false;

        for (int i = 0; i < numberOfPCs; i++)
        {
            // If this is the PC we are about to add:
            if (programCounters[i] == progCounter)
            {
                foundPC = true;
                break;
            }
        }

        // If this is a new PC:
        if (!foundPC)
        {
            // Add it to the vector:
            programCounters.push_back(progCounter);
            newPC = true;
        }
    }

    // If this is the first time this PC has appeared:
    if (newPC)
    {
        // Find which is the smallest-ranged program containing this address:
        csDWARFProgram* pContainingProgram = getAddressScope(progCounter);

        if (pContainingProgram != NULL)
        {
            // Add it to the program's PCs vector:
            pContainingProgram->_programMappedPCs.push_back(progCounter);
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        csDWARFParser::locationFromDWARFData
// Description: Parses the data from the Dwarf_Loc struct to a location to be
//              used by the kernel debugging manager.
// Author:      Uri Shomroni
// Date:        19/1/2011
// ---------------------------------------------------------------------------
gtUInt64 csDWARFParser::locationFromDWARFData(gtUByte atom, gtUInt64 number1, csDWARFVariable::ValueLocationType& locationType)
{
    gtUInt64 retVal = GT_UINT64_MAX;

    switch (atom)
    {
        // Handle basic registers:
        case DW_OP_reg0:
        case DW_OP_breg0:
            retVal = 0;
            locationType = (atom == DW_OP_breg0) ? csDWARFVariable::CS_INDIRECT_REGISTER : csDWARFVariable::CS_REGISTER;
            break;

        case DW_OP_reg1:
        case DW_OP_breg1:
            retVal = 1;
            locationType = (atom == DW_OP_breg1) ? csDWARFVariable::CS_INDIRECT_REGISTER : csDWARFVariable::CS_REGISTER;
            break;

        case DW_OP_reg2:
        case DW_OP_breg2:
            retVal = 2;
            locationType = (atom == DW_OP_breg2) ? csDWARFVariable::CS_INDIRECT_REGISTER : csDWARFVariable::CS_REGISTER;
            break;

        case DW_OP_reg3:
        case DW_OP_breg3:
            retVal = 3;
            locationType = (atom == DW_OP_breg3) ? csDWARFVariable::CS_INDIRECT_REGISTER : csDWARFVariable::CS_REGISTER;
            break;

        case DW_OP_reg4:
        case DW_OP_breg4:
            retVal = 4;
            locationType = (atom == DW_OP_breg4) ? csDWARFVariable::CS_INDIRECT_REGISTER : csDWARFVariable::CS_REGISTER;
            break;

        case DW_OP_reg5:
        case DW_OP_breg5:
            retVal = 5;
            locationType = (atom == DW_OP_breg5) ? csDWARFVariable::CS_INDIRECT_REGISTER : csDWARFVariable::CS_REGISTER;
            break;

        case DW_OP_reg6:
        case DW_OP_breg6:
            retVal = 6;
            locationType = (atom == DW_OP_breg6) ? csDWARFVariable::CS_INDIRECT_REGISTER : csDWARFVariable::CS_REGISTER;
            break;

        case DW_OP_reg7:
        case DW_OP_breg7:
            retVal = 7;
            locationType = (atom == DW_OP_breg7) ? csDWARFVariable::CS_INDIRECT_REGISTER : csDWARFVariable::CS_REGISTER;
            break;

        case DW_OP_reg8:
        case DW_OP_breg8:
            retVal = 8;
            locationType = (atom == DW_OP_breg8) ? csDWARFVariable::CS_INDIRECT_REGISTER : csDWARFVariable::CS_REGISTER;
            break;

        case DW_OP_reg9:
        case DW_OP_breg9:
            retVal = 9;
            locationType = (atom == DW_OP_breg9) ? csDWARFVariable::CS_INDIRECT_REGISTER : csDWARFVariable::CS_REGISTER;
            break;

        case DW_OP_reg10:
        case DW_OP_breg10:
            retVal = 10;
            locationType = (atom == DW_OP_breg10) ? csDWARFVariable::CS_INDIRECT_REGISTER : csDWARFVariable::CS_REGISTER;
            break;

        case DW_OP_reg11:
        case DW_OP_breg11:
            retVal = 11;
            locationType = (atom == DW_OP_breg11) ? csDWARFVariable::CS_INDIRECT_REGISTER : csDWARFVariable::CS_REGISTER;
            break;

        case DW_OP_reg12:
        case DW_OP_breg12:
            retVal = 12;
            locationType = (atom == DW_OP_breg12) ? csDWARFVariable::CS_INDIRECT_REGISTER : csDWARFVariable::CS_REGISTER;
            break;

        case DW_OP_reg13:
        case DW_OP_breg13:
            retVal = 13;
            locationType = (atom == DW_OP_breg13) ? csDWARFVariable::CS_INDIRECT_REGISTER : csDWARFVariable::CS_REGISTER;
            break;

        case DW_OP_reg14:
        case DW_OP_breg14:
            retVal = 14;
            locationType = (atom == DW_OP_breg14) ? csDWARFVariable::CS_INDIRECT_REGISTER : csDWARFVariable::CS_REGISTER;
            break;

        case DW_OP_reg15:
        case DW_OP_breg15:
            retVal = 15;
            locationType = (atom == DW_OP_breg15) ? csDWARFVariable::CS_INDIRECT_REGISTER : csDWARFVariable::CS_REGISTER;
            break;

        case DW_OP_reg16:
        case DW_OP_breg16:
            retVal = 16;
            locationType = (atom == DW_OP_breg16) ? csDWARFVariable::CS_INDIRECT_REGISTER : csDWARFVariable::CS_REGISTER;
            break;

        case DW_OP_reg17:
        case DW_OP_breg17:
            retVal = 17;
            locationType = (atom == DW_OP_breg17) ? csDWARFVariable::CS_INDIRECT_REGISTER : csDWARFVariable::CS_REGISTER;
            break;

        case DW_OP_reg18:
        case DW_OP_breg18:
            retVal = 18;
            locationType = (atom == DW_OP_breg18) ? csDWARFVariable::CS_INDIRECT_REGISTER : csDWARFVariable::CS_REGISTER;
            break;

        case DW_OP_reg19:
        case DW_OP_breg19:
            retVal = 19;
            locationType = (atom == DW_OP_breg19) ? csDWARFVariable::CS_INDIRECT_REGISTER : csDWARFVariable::CS_REGISTER;
            break;

        case DW_OP_reg20:
        case DW_OP_breg20:
            retVal = 20;
            locationType = (atom == DW_OP_breg20) ? csDWARFVariable::CS_INDIRECT_REGISTER : csDWARFVariable::CS_REGISTER;
            break;

        case DW_OP_reg21:
        case DW_OP_breg21:
            retVal = 21;
            locationType = (atom == DW_OP_breg21) ? csDWARFVariable::CS_INDIRECT_REGISTER : csDWARFVariable::CS_REGISTER;
            break;

        case DW_OP_reg22:
        case DW_OP_breg22:
            retVal = 22;
            locationType = (atom == DW_OP_breg22) ? csDWARFVariable::CS_INDIRECT_REGISTER : csDWARFVariable::CS_REGISTER;
            break;

        case DW_OP_reg23:
        case DW_OP_breg23:
            retVal = 23;
            locationType = (atom == DW_OP_breg23) ? csDWARFVariable::CS_INDIRECT_REGISTER : csDWARFVariable::CS_REGISTER;
            break;

        case DW_OP_reg24:
        case DW_OP_breg24:
            retVal = 24;
            locationType = (atom == DW_OP_breg24) ? csDWARFVariable::CS_INDIRECT_REGISTER : csDWARFVariable::CS_REGISTER;
            break;

        case DW_OP_reg25:
        case DW_OP_breg25:
            retVal = 25;
            locationType = (atom == DW_OP_breg25) ? csDWARFVariable::CS_INDIRECT_REGISTER : csDWARFVariable::CS_REGISTER;
            break;

        case DW_OP_reg26:
        case DW_OP_breg26:
            retVal = 26;
            locationType = (atom == DW_OP_breg26) ? csDWARFVariable::CS_INDIRECT_REGISTER : csDWARFVariable::CS_REGISTER;
            break;

        case DW_OP_reg27:
        case DW_OP_breg27:
            retVal = 27;
            locationType = (atom == DW_OP_breg27) ? csDWARFVariable::CS_INDIRECT_REGISTER : csDWARFVariable::CS_REGISTER;
            break;

        case DW_OP_reg28:
        case DW_OP_breg28:
            retVal = 28;
            locationType = (atom == DW_OP_breg28) ? csDWARFVariable::CS_INDIRECT_REGISTER : csDWARFVariable::CS_REGISTER;
            break;

        case DW_OP_reg29:
        case DW_OP_breg29:
            retVal = 29;
            locationType = (atom == DW_OP_breg29) ? csDWARFVariable::CS_INDIRECT_REGISTER : csDWARFVariable::CS_REGISTER;
            break;

        case DW_OP_reg30:
        case DW_OP_breg30:
            retVal = 30;
            locationType = (atom == DW_OP_breg30) ? csDWARFVariable::CS_INDIRECT_REGISTER : csDWARFVariable::CS_REGISTER;
            break;

        case DW_OP_reg31:
        case DW_OP_breg31:
            retVal = 31;
            locationType = (atom == DW_OP_breg31) ? csDWARFVariable::CS_INDIRECT_REGISTER : csDWARFVariable::CS_REGISTER;
            break;

        // Handle exteneded register numbers:
        case DW_OP_regx:
        case DW_OP_bregx:
            retVal = number1;
            locationType = (atom == DW_OP_bregx) ? csDWARFVariable::CS_INDIRECT_REGISTER : csDWARFVariable::CS_REGISTER;
            break;

        // Handle offset locations:
        case DW_OP_plus_uconst:
            break;

        // Handle stack location:
        case DW_OP_fbreg:
            retVal = number1;
            locationType = csDWARFVariable::CS_STACK_OFFSET;
            break;

        default:
            // Unexpected value!
            GT_ASSERT(false);
            break;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        csDWARFParser::intersectVariablesInProgram
// Description: Goes over the variables list in the program, intersecting the
//              ranges of like-named variables that have the DW_AT_start_scope
//              attribute as their only address range.
// Author:      Uri Shomroni
// Date:        3/5/2011
// ---------------------------------------------------------------------------
void csDWARFParser::intersectVariablesInProgram(csDWARFProgram& programData)
{
    // Iterate all the variables:
    gtPtrVector<csDWARFVariable*>& programVariables = programData._programVariables;
    int numberOfVariables = (int)programVariables.size();

    for (int i = 0; i < numberOfVariables; i++)
    {
        // Sanity check:
        csDWARFVariable* pCurrentVariable = programVariables[i];
        GT_IF_WITH_ASSERT(pCurrentVariable != NULL)
        {
            // Get all the other variables:
            for (int j = (i + 1); j < numberOfVariables; j++)
            {
                // No need to assert twice for the same variable:
                csDWARFVariable* pOtherVariable = programVariables[j];

                if (pOtherVariable != NULL)
                {
                    // If these two variables have the same name:
                    if (pCurrentVariable->_variableName == pOtherVariable->_variableName)
                    {
                        // Check if the current variable needs to be truncated:
                        csDWARFVariable::csDWARFVariableLocation& currentVariableLocation = pCurrentVariable->_variableLocation;
                        csDWARFVariable::csDWARFVariableLocation& otherVariableLocation = pOtherVariable->_variableLocation;

                        if (!currentVariableLocation._variableHighestPCValid)
                        {
                            if ((currentVariableLocation._variableHighestPC > otherVariableLocation._variableLowestPC) &&
                                (currentVariableLocation._variableLowestPC < otherVariableLocation._variableLowestPC))
                            {
                                // Truncate it:
                                currentVariableLocation._variableHighestPC = otherVariableLocation._variableLowestPC - 1;
                            }
                        }

                        // Check if the other variable needs to be truncated:
                        if (!otherVariableLocation._variableHighestPCValid)
                        {
                            if ((otherVariableLocation._variableHighestPC > currentVariableLocation._variableLowestPC) &&
                                (otherVariableLocation._variableLowestPC < currentVariableLocation._variableLowestPC))
                            {
                                // Truncate it:
                                otherVariableLocation._variableHighestPC = currentVariableLocation._variableLowestPC - 1;
                            }
                        }
                    }
                }
            }
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        csDWARFParser::typeNameAndDetailsFromTypeDIE
// Description: Gets a type name and appropriate format string from the type DIE
// Author:      Uri Shomroni
// Date:        20/1/2011
// ---------------------------------------------------------------------------
void csDWARFParser::typeNameAndDetailsFromTypeDIE(gtString& typeName, gtUInt32& typeSize, csDWARFVariable::ValueEncoding& typeEncoding, bool& isPointerType, bool& isArrayType, gtVector<csDWARFVariable>& typeMembers, bool expandIndirectMembers, gtUInt64 membersLocation, csDWARFVariable::ValueLocationType membersLocationType, gtUInt64 membersLocationResource, gtUInt32 membersAccumulatedOffset, Dwarf_Die typeDIE, bool isRegisterParamter)
{
    typeName.makeEmpty();
    gtString genericTypeName = L"unnamed_type_";
    typeEncoding = csDWARFVariable::CS_UNKNOWN_ENCODING;
    isPointerType = false;
    isArrayType = false;
    bool goOn = true;
    bool getSibling = false;
    Dwarf_Die currentType = typeDIE;
    Dwarf_Die typeForName = currentType;
    bool foundName = false;
    bool hasMembers = false;
    Dwarf_Error err;
    memset((void*)&err, 0, sizeof(Dwarf_Error));

    while (goOn)
    {
        // Get the current type's TAG:
        Dwarf_Half currentTypeTag = 0;
        int rc = dwarf_tag(currentType, &currentTypeTag, &err);

        if (rc == DW_DLV_OK)
        {
            // See what kind of type this is:
            switch (currentTypeTag)
            {
                case DW_TAG_array_type:

                    // Array, add brackets:
                    if (!foundName) { typeName.prepend(L"[]"); }

                    genericTypeName = L"unnamed_type_array_";
                    isArrayType = true;
                    break;

                case DW_TAG_enumeration_type:

                    // This is an enumeration, show this in the name and stop looking
                    if (!foundName) { typeName.prepend(L" enum"); }

                    genericTypeName = L"unnamed_enum_type_";
                    goOn = false;
                    break;

                case DW_TAG_pointer_type:

                    // Pointer, add an asterisk:
                    if (!foundName) { typeName.prepend('*'); }

                    genericTypeName = L"unnamed_type_pointer_";
                    isPointerType = true;
                    break;

                case DW_TAG_reference_type:

                    // Reference, add an ampersand:
                    if (!foundName) { typeName.prepend('&'); }

                    genericTypeName = L"unnamed_type_reference_";
                    break;

                case DW_TAG_structure_type:
                case DW_TAG_union_type:
                case DW_TAG_class_type:

                    // This is a struct, it will not point to another type, and we'll not show its "value":
                    if (typeEncoding == csDWARFVariable::CS_UNKNOWN_ENCODING)
                    {
                        typeEncoding = csDWARFVariable::CS_NO_ENCODING;
                    }

                    hasMembers = true;

                    if (DW_TAG_union_type == currentTypeTag)
                    {
                        genericTypeName = L"unnamed_union_type_";
                    }
                    else if (DW_TAG_structure_type == currentTypeTag)
                    {
                        genericTypeName = L"unnamed_struct_type_";
                    }
                    else if (DW_TAG_class_type == currentTypeTag)
                    {
                        genericTypeName = L"unnamed_class_type_";
                    }

                    if (foundName)
                    {
                        goOn = false;
                    }
                    else
                    {
                        // Try and see if this struct has a typedef sibling:
                        getSibling = true;
                    }

                    break;

                case DW_TAG_typedef:
                    // This is a typedef, we need to get the name from here but the rest of the details from the pointed value:
                    typeForName = currentType;
                    foundName = true;
                    break;

                case DW_TAG_base_type:
                    // This is a basic type, stop searching and get this type's name:
                    goOn = false;
                    break;

                case DW_TAG_const_type:
                case DW_TAG_volatile_type:
                    // This is a modified type, we currently don't reflect this in the type name
                    // TO_DO: We should add the modifier word (e.g. "const") between the next type element (e.g. int *const*)
                    break;

                case DW_TAG_string_type:
                case DW_TAG_subroutine_type:
                case DW_TAG_ptr_to_member_type:
                case DW_TAG_set_type:
                case DW_TAG_subrange_type:
                case DW_TAG_packed_type:
                case DW_TAG_thrown_type:
                case DW_TAG_restrict_type:
                case DW_TAG_interface_type:
                case DW_TAG_unspecified_type:
                case DW_TAG_shared_type:
                    // Unsupported type:
                    GT_ASSERT_EX(false, L"Unsupported kernel variable type");
                    goOn = false;
                    break;

                default:
                    // Unexpected value:
                    GT_ASSERT(false);
                    goOn = false;
                    break;
            }

            // If we need to continue
            if (goOn)
            {
                // We want to stop if we fail to iterate:
                goOn = false;

                // Get the next type DIE by this:
                int rcTp = DW_DLV_ERROR;
                Dwarf_Die nextTypeDIE = NULL;
                bool isValidDIE = false;

                // Avoid using
                if (getSibling)
                {
                    // Get the sibling and verify it is a typedef:
                    rcTp = dwarf_siblingof(_pDwarf, currentType, &nextTypeDIE, &err);

                    if ((DW_DLV_OK == rcTp) && (NULL != nextTypeDIE))
                    {
                        Dwarf_Half nextTypeTag = 0;
                        int rcNT = dwarf_tag(nextTypeDIE, &nextTypeTag, &err);

                        if (rcNT == DW_DLV_OK)
                        {
                            // We need to make sure it's a typedef, otherwise we might
                            // get stuck in a loop:
                            isValidDIE = (nextTypeTag == DW_TAG_typedef);
                        }
                    }
                }
                else // !getSibling
                {
                    Dwarf_Attribute varTypeRefAsAttribute = NULL;
                    rcTp = dwarf_attr(currentType, DW_AT_type, &varTypeRefAsAttribute, &err);

                    if ((rcTp == DW_DLV_OK) && (varTypeRefAsAttribute != NULL))
                    {
                        // Get the type DIE
                        rcTp = dwarf_formref_die(varTypeRefAsAttribute, &nextTypeDIE, &err, _pDwarf);

                        if ((rcTp == DW_DLV_OK) && (nextTypeDIE != NULL))
                        {
                            isValidDIE = true;
                        }

                        // Release the attribute:
                        dwarf_dealloc(_pDwarf, (Dwarf_Ptr)varTypeRefAsAttribute, DW_DLA_ATTR);
                    }
                }

                // If we didn't go to the sibling or the sibling is proper:
                if (isValidDIE)
                {
                    // Release the previous DIE:
                    if ((currentType != typeDIE) && (currentType != typeForName))
                    {
                        dwarf_dealloc(_pDwarf, (Dwarf_Ptr)currentType, DW_DLA_DIE);
                    }

                    // Iterate to the next one:
                    currentType = nextTypeDIE;
                    goOn = true;
                }
                else // !isValidDIE
                {
                    // Release the next DIE:
                    if ((nextTypeDIE != typeDIE) && (nextTypeDIE != typeForName) && (NULL != nextTypeDIE))
                    {
                        dwarf_dealloc(_pDwarf, (Dwarf_Ptr)nextTypeDIE, DW_DLA_DIE);
                    }
                }

                // Only attempt to get the sibling once:
                getSibling = false;
            }
        }
        else // rc != DW_DLV_OK
        {
            // We encountered an error:
            goOn = false;
        }
    }

    // If we did not find anything more detailed, consider a pointer as pointer type:
    if ((isPointerType || isArrayType) && ((typeEncoding == csDWARFVariable::CS_UNKNOWN_ENCODING) || (typeEncoding == csDWARFVariable::CS_NO_ENCODING)))
    {
        typeEncoding = csDWARFVariable::CS_POINTER_ENCODING;
    }

    if (!foundName)
    {
        typeForName = currentType;
    }

    // We do not want to expand indirect members in some cases, to avoid recursive loops
    // in this parsing code:
    if ((typeEncoding == csDWARFVariable::CS_POINTER_ENCODING) && (!expandIndirectMembers))
    {
        hasMembers = false;
    }

    // Get the name of this type:
    const char* typeNameAsCharArray = NULL;
    int rcNm = dwarf_diename(typeForName, (char**)&typeNameAsCharArray, &err);

    if ((rcNm == DW_DLV_OK) && (typeNameAsCharArray != NULL))
    {
        // Copy the name:
        gtString baseTypeName;
        baseTypeName.fromASCIIString(typeNameAsCharArray);
        typeName.prepend(baseTypeName);

        // Release the string:
        dwarf_dealloc(_pDwarf, (Dwarf_Ptr)typeNameAsCharArray, DW_DLA_STRING);
    }
    else if (!foundName)
    {
        // Use a generic name:
        static int runningTypeIndex = 0;
        genericTypeName.appendFormattedString(L"%d", runningTypeIndex++);
        typeName = genericTypeName;
    }

    // We can now deallocate the type for name DIE:
    if ((currentType != typeForName) && (typeDIE != typeForName))
    {
        dwarf_dealloc(_pDwarf, (Dwarf_Ptr)typeForName, DW_DLA_DIE);
    }

    // Get the data size:
    Dwarf_Unsigned typeSizeAsDWARFUnsigned = 0;
    int rcSz = dwarf_attrval_unsigned(currentType, DW_AT_byte_size, &typeSizeAsDWARFUnsigned, &err);

    if (rcSz == DW_DLV_OK)
    {
        typeSize = (gtUInt32)typeSizeAsDWARFUnsigned;
    }

    // Get the encoding for types we want to know and for pointers, for dereferencing:
    if (typeEncoding == csDWARFVariable::CS_UNKNOWN_ENCODING || isPointerType || isArrayType)
    {
        // Get the encoding type:
        Dwarf_Unsigned typeEncodingAsDWARFUnsigned = 0;
        int rcFm = dwarf_attrval_unsigned(currentType, DW_AT_encoding, &typeEncodingAsDWARFUnsigned, &err);

        if (rcFm == DW_DLV_OK)
        {
            switch (typeEncodingAsDWARFUnsigned)
            {
                case DW_ATE_address:
                    typeEncoding = csDWARFVariable::CS_POINTER_ENCODING;
                    break;

                case DW_ATE_boolean:
                    typeEncoding = csDWARFVariable::CS_BOOL_ENCODING;
                    break;

                case DW_ATE_float:
                    typeEncoding = csDWARFVariable::CS_FLOAT_ENCODING;
                    break;

                case DW_ATE_signed:
                    typeEncoding = csDWARFVariable::CS_INT_ENCODING;
                    break;

                case DW_ATE_signed_char:
                    typeEncoding = csDWARFVariable::CS_CHAR_ENCODING;
                    break;

                case DW_ATE_unsigned:
                    typeEncoding = csDWARFVariable::CS_UINT_ENCODING;
                    break;

                case DW_ATE_unsigned_char:
                    typeEncoding = csDWARFVariable::CS_UCHAR_ENCODING;
                    break;

                case DW_ATE_complex_float:
                case DW_ATE_imaginary_float:
                case DW_ATE_packed_decimal:
                case DW_ATE_numeric_string:
                case DW_ATE_edited:
                case DW_ATE_signed_fixed:
                case DW_ATE_unsigned_fixed:
                case DW_ATE_decimal_float:
                    // Unsupported type:
                    GT_ASSERT_EX(false, L"Unsupported kernel variable type");
                    break;

                default:
                    // Unexpected value:
                    GT_ASSERT(false);
                    break;
            }
        }
    }

    // If this (struct) type has children (members), get them:
    if (hasMembers)
    {
        // See comment in fillVariableWithInformationFromDIE before the call to this function:
        // When a struct is passed by-value, its data is actually referenced and not direct.
        csDWARFVariable::ValueLocationType realLocationType = membersLocationType;

        if (isRegisterParamter)
        {
            realLocationType = csDWARFVariable::CS_INDIRECT_REGISTER;
        }

        // Get the first child:
        Dwarf_Die currentChild = NULL;
        int rcCh = dwarf_child(currentType, &currentChild, &err);

        if (rcCh == DW_DLV_OK)
        {
            // Iterate all the children:
            while (currentChild != NULL)
            {
                // Make sure it is a member:
                Dwarf_Half childTag = DW_TAG_member;
                int rcTg = dwarf_tag(currentChild, &childTag, &err);

                if (!((DW_DLV_OK == rcTg) && (DW_TAG_member != childTag)))
                {
                    // Get the current member's data:
                    csDWARFVariable currentMember;
                    currentMember._variableLocation._variableLocationAccumulatedOffset = membersAccumulatedOffset;
                    currentMember._variableLocation._variableLocation = membersLocation;
                    currentMember._variableLocation._variableLocationType = realLocationType;
                    currentMember._variableLocation._variableLocationResource = membersLocationResource;
                    // Variables do not have locations that are scoped, so ignore them for this:
                    gtVector<csDWARFVariable::csDWARFVariableLocation> ignoredAdditionalLocations;
                    fillVariableWithInformationFromDIE(currentMember, currentChild, ignoredAdditionalLocations, true);
                    GT_ASSERT(ignoredAdditionalLocations.size() == 0);

                    // Add the member to the vector:
                    typeMembers.push_back(currentMember);
                }

                // Move to the next member DIE:
                Dwarf_Die nextChild = NULL;
                int rcSib = dwarf_siblingof(_pDwarf, currentChild, &nextChild, &err);

                // Release the current child:
                dwarf_dealloc(_pDwarf, (Dwarf_Ptr)currentChild, DW_DLA_DIE);

                if (DW_DLV_OK == rcSib)
                {
                    // Move to the next iteration:
                    currentChild = nextChild;
                }
                else
                {
                    // Stop on failure:
                    currentChild = NULL;
                }
            }
        }
    }

    // Release the base type DIE:
    if (currentType != typeDIE)
    {
        dwarf_dealloc(_pDwarf, (Dwarf_Ptr)currentType, DW_DLA_ATTR);
    }
}

// ---------------------------------------------------------------------------
// Name:        csDWARFParser::clearLineInformation
// Description: Clears all source line information we currently have:
// Author:      Uri Shomroni
// Date:        18/11/2010
// ---------------------------------------------------------------------------
void csDWARFParser::clearLineInformation()
{
    // Clear both maps:
    _programCounterToCodeLocation.clear();
    _codeLocationToProgramCounters.clear();
    _programCountersMappedToLineNumbers.clear();
}

// ---------------------------------------------------------------------------
// Name:        csDWARFParser::getAddressScope
// Description: Mutable version of findAddressScope
// Author:      Uri Shomroni
// Date:        14/7/2011
// ---------------------------------------------------------------------------
csDWARFProgram* csDWARFParser::getAddressScope(csDwarfAddressType addr)
{
    csDWARFProgram* retVal = NULL;
    const csDWARFProgram* pAddressScope = findAddressScope(addr);
    GT_IF_WITH_ASSERT(pAddressScope != NULL)
    {
        retVal = (csDWARFProgram*)pAddressScope;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        csDWARFParser::addLeafMembersToVector
// Description: Recursively finds the leaf members and adds them to the vector
// Author:      Uri Shomroni
// Date:        25/5/2011
// ---------------------------------------------------------------------------
void csDWARFParser::addLeafMembersToVector(const csDWARFVariable& variableData, gtVector<gtString>& variableNames, gtString& namesBase)
{
    // Currently we do not support the local and constant memory spaces, so do not show them here:
    if ((csDWARFVariable::CS_LOCAL_POINTER != variableData._valuePointerAddressSpace) && (csDWARFVariable::CS_CONSTANT_POINTER != variableData._valuePointerAddressSpace))
    {
        // If the current variable is a pointer:
        if ((variableData._valueIsPointer) || (variableData._valueIsArray) || (variableData._valueEncoding == csDWARFVariable::CS_POINTER_ENCODING))
        {
            // Pointers should be added as reference and dereferenced:
            variableNames.push_back(namesBase);
            namesBase.prepend('*').append(')').prepend('(');
        }

        const gtVector<csDWARFVariable>& variableMembers = variableData._variableMembers;
        int numberOfMembers = (int)variableMembers.size();

        if (numberOfMembers < 1)
        {
            // This is a leaf, add it:
            variableNames.push_back(namesBase);
        }
        else // numberOfMembers >= 1
        {
            // Iterate the members:
            gtString memberNamesBase;

            for (int i = 0; i < numberOfMembers; i++)
            {
                // Get the member data:
                const csDWARFVariable& currentMember = variableMembers[i];
                memberNamesBase = namesBase;
                memberNamesBase.append('.').append(currentMember._variableName);

                // Recurse into the member:
                addLeafMembersToVector(currentMember, variableNames, memberNamesBase);
            }
        }
    }
}

