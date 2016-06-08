//==============================================================================
// Copyright (c) 2013-2015 Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools
/// \file
/// \brief  Definition of dumper for Debug Information
//==============================================================================
#ifndef DBGINFODUMPER_H_
#define DBGINFODUMPER_H_

// Infra:
#include <string>
#include <vector>
#include <map>

// Local:
#include <DbgInfoData.h>
#include <DbgInfoLines.h>
#include <DbgInfoDefinitions.h>

namespace HwDbg
{

/// -----------------------------------------------------------------------------------------------
/// \class DbgInfoDumper
/// \brief Description: Dumps the dbg information to a string
/// \tparam AddrType: Address type
/// \tparam  LineType: Line type
/// \tparam  VarLocationType: Variable location type
/// -----------------------------------------------------------------------------------------------
template<typename AddrType, typename LineType, typename VarLocationType> class DbgInfoDumper
{
public:
    /// These are definitions for Dumpers which need to be defined:
    /// The reason this is done with function pointers is that the addrType and lineType can be primitives:
    typedef void (*AddrTypeDumper)(const AddrType& addr, std::string& addrAsString);
    /// Line to string:
    typedef void (*LineTypeDumper)(const LineType& line, std::string& lineAsString);
    /// Location to string:
    typedef void (*VarLocationTypeDumper)(const VarLocationType& varLocation, std::string& lineAsString);

public:
    /// Explicit constructor
    DbgInfoDumper(AddrTypeDumper addrTypeDumper, LineTypeDumper lineTypeDumper, VarLocationTypeDumper varLocationTypeDumper, bool useIndent = true)
        : m_addrTypeDumper(addrTypeDumper), m_lineTypeDumper(lineTypeDumper), m_varLocTypeDumper(varLocationTypeDumper), m_useIndent(useIndent) {}
    /// Default constructor:
    DbgInfoDumper(): m_addrTypeDumper(0), m_lineTypeDumper(0), m_varLocTypeDumper(0), m_useIndent(true) {}
    /// Dtor:
    virtual ~DbgInfoDumper() {}
    /// Setter:
    void SetUseIndent(bool useIndent) {m_useIndent = useIndent;}
    /// Setter:
    void SetAddrTypeDumper(const AddrTypeDumper& addrTypeDumper) {m_addrTypeDumper = addrTypeDumper;}
    /// Setter:
    void SetLineTypeDumper(const LineTypeDumper& lineTypeDumper) {m_lineTypeDumper = lineTypeDumper;}
    /// Setter:
    void SetVarLocationTypeDumper(const VarLocationTypeDumper& varLocationTypeDumper) {m_varLocTypeDumper = varLocationTypeDumper;}
    /// Dumps LineNumberMapping to string:
    void PrintLineInformation(const LineNumberMapping<AddrType, LineType>& lineMapping, std::string& o_outputString);
    /// Dumps Variable Information to string:
    void PrintVariableInformation(const VariableInfo<AddrType, VarLocationType>& varInfo, const std::string& indent, std::string& o_appendOutputString);
    /// Dumps CodeScope to string:
    void PrintScope(const CodeScope<AddrType, LineType, VarLocationType>& scopeInfo, std::string& o_outputString);

protected:
    /// Dumps Variable Information to string:
    void InternalPrintVariableInformation(const VariableInfo<AddrType, VarLocationType>& varInfo, std::string& o_appendOutputString, const std::string& indent, int depth = 0);
    /// Dumps Variable IndirectionType to string:
    void PrintVariableIndirectionType(const typename VariableInfo<AddrType, VarLocationType>::VariableIndirection& indirectionType, std::string& o_appendedOutputString);
    /// Dumps ScopeType to string:
    void PrintScopeType(const typename CodeScope<AddrType, LineType, VarLocationType>::ScopeType& scopeType, std::string& o_appendedOutputString);
    /// Dumps Address range to string:
    void PrintAddressRange(const typename CodeScope< AddrType, LineType, VarLocationType>::AddressRange& addressRange, std::string& o_appendedOutputString);
    /// Dumps the inline information to string:
    void PrintInlineLocations(const typename CodeScope<AddrType, LineType, VarLocationType>::InlineInformation& inlineInformation, std::string& o_appendedOutputString);
    /// Dumps CodeScope to string:
    void InternalPrintScope(const CodeScope<AddrType, LineType, VarLocationType>& scopeInfo, std::string& o_appendedOutputString, std::string indent = "", int depth = 0);

    /// Members:
    /// Function pointer to a function which knows how to print an AddrType:
    AddrTypeDumper m_addrTypeDumper;
    /// Function pointer to a function which knows how to print an LineType:
    LineTypeDumper m_lineTypeDumper;
    /// Function pointer to a function which knows how to print an VarLocationType:
    VarLocationTypeDumper m_varLocTypeDumper;
    /// Whether to add indentation to the output strings or not:
    bool m_useIndent;
}; //class DbgInfoDumper

/// ---------------------------------------------------------------------------
/// PrintLineInformation
/// \brief Description: Prints a line number mapping in the following format:
///                line1 -> { addr1, addr2, addr3 }
///                line2 -> { addr5, addr6, addr8 }
/// \param[in] lineMapping - a mapping between lines and addresses
/// \param[out] o_outputString - The output string which contains the line to addresses mapping
/// \return void
/// ---------------------------------------------------------------------------
template<typename AddrType, typename LineType, typename VarLocationType>
void DbgInfoDumper<AddrType, LineType, VarLocationType>::PrintLineInformation(const LineNumberMapping<AddrType, LineType>& lineMapping, std::string& o_outputString)
{
    assert(m_addrTypeDumper != nullptr && m_lineTypeDumper != nullptr);

    if (m_addrTypeDumper != nullptr && m_lineTypeDumper != nullptr)
    {
        typedef std::vector<AddrType> AddressVec;
        typedef std::vector<LineType> LineVec;

        o_outputString.clear();
        LineVec lines;
        AddressVec addressesPerLine;

        // Get all mapped lines:
        lineMapping.GetMappedLines(lines);

        // Loop over all lines:
        for (typename LineVec::iterator lineIt = lines.begin(); lineIt != lines.end(); ++ lineIt)
        {
            LineType line = *lineIt;
            // Print the line to lineStr:
            std::string lineStr;
            m_lineTypeDumper(line, lineStr);
            o_outputString += lineStr;
            o_outputString += " -> {";

            // Get the addresses for this line:
            if (lineMapping.GetAddressesFromLine(line, addressesPerLine))
            {
                std::string addrStr;

                // Print the addresses in the following format: { addr1, addr2 }
                for (typename AddressVec::iterator addrIt = addressesPerLine.begin(); addrIt != addressesPerLine.end(); ++addrIt)
                {
                    addrStr.clear();
                    AddrType addr = *addrIt;
                    m_addrTypeDumper(addr, addrStr);
                    o_outputString += ((addrIt == addressesPerLine.begin()) ? " " : ", ");
                    o_outputString += addrStr;
                }
            }

            // Close the address string:
            o_outputString += " }\n";
        }
    }
}

/// -----------------------------------------------------------------------------------------------
/// PrintVariableInformation
/// \brief Description: prints all the information about a variable in the following format:
/// {VarInfo [Depth]    //depth is the relative depth of the variable - variables can contain other variables
///     Name, TypeName, Size, Encoding, Value, Indirection=Direct|Pointer|Reference|Array, IndirectionDetail
/// }VarInfo [Depth]
/// \param[in]          varInfo - the variable information struct to print
/// \param[in]          indent - Whether to print with indentation
/// \param[out]         o_appendOutputString - The output string
/// \return void
/// -----------------------------------------------------------------------------------------------
template<typename AddrType, typename LineType, typename VarLocationType>
void DbgInfoDumper<AddrType, LineType, VarLocationType>::PrintVariableInformation(const VariableInfo<AddrType, VarLocationType>& varInfo, const std::string& indent, std::string& o_appendOutputString)
{
    assert(m_varLocTypeDumper != nullptr);

    if (m_varLocTypeDumper != nullptr)
    {
        // Recursive call:
        InternalPrintVariableInformation(varInfo, o_appendOutputString, indent, 0);
    }
}

/// ---------------------------------------------------------------------------
/// PrintScope
/// \brief Description:
/// \param[in] scopeInfo - The topmost scope object, template args are:
/// \param[out] o_outputString - The output string returned by the method
/// \return void
/// ---------------------------------------------------------------------------
template<typename AddrType, typename LineType, typename VarLocationType>
void DbgInfoDumper<AddrType, LineType, VarLocationType>::PrintScope(const CodeScope<AddrType, LineType, VarLocationType>& scopeInfo, std::string& o_outputString)
{
    o_outputString.clear();
    assert(m_addrTypeDumper != nullptr && m_lineTypeDumper != nullptr && m_varLocTypeDumper != nullptr);

    if (m_addrTypeDumper != nullptr && m_lineTypeDumper != nullptr && m_varLocTypeDumper != nullptr)
    {
        InternalPrintScope(scopeInfo, o_outputString);
    }
}

/// -----------------------------------------------------------------------------------------------
/// InternalPrintVariableInformation
/// \brief Description: prints all the information about a variable in the following format:
/// {Start Variable [Depth]    //depth is the relative depth of the variable - variables can contain other variables
///     Name, TypeName, Size, Encoding, Value, Indirection=Direct|Pointer|Reference|Array, IndirectionDetail, VarLocation=[Const|VarLocation]
/// }End Variable [Depth]
/// \param[in]          varInfo - the variable information struct to print
/// \param[out]         o_appendOutputString - The output string
/// \param[in]          indent - whether to print with indentation
/// \param[in]          depth - the depth of the current variable
/// \return void
/// -----------------------------------------------------------------------------------------------
template<typename AddrType, typename LineType, typename VarLocationType>
void DbgInfoDumper<AddrType, LineType, VarLocationType>::InternalPrintVariableInformation(const VariableInfo<AddrType, VarLocationType>& varInfo, std::string& o_appendOutputString, const std::string& indent, int depth)
{
    std::string varLocation;
    std::string valueType;

    std::string indirectionType;
    std::string lowVariablePC;
    std::string highVariablePC;
    std::string varIndent = (m_useIndent ? indent : "");
    std::string isOutParam = varInfo.m_isOutParam ? "OutputParam" : "Not OutputParam";

    (o_appendOutputString += varIndent) += string_format("{Start Variable [%d]\n", depth);

    if (varInfo.VarValueType() == VariableInfo<AddrType, VarLocationType>::DID_VAR_VARIABLE_VALUE)
    {
        m_varLocTypeDumper(varInfo.m_varValue.m_varValueLocation, varLocation);
        valueType = "Variable";
    }
    else
    {
        valueType = "Const";

        for (HwDbgUInt64 i = 0; i < varInfo.m_varSize; i++)
        {
            varLocation += string_format("%02lx", (int)varInfo.m_varValue.m_varConstantValue[i]);
        }
    }

    PrintVariableIndirectionType(varInfo.m_varIndirection, indirectionType);

    m_addrTypeDumper(varInfo.m_lowVariablePC, lowVariablePC);
    m_addrTypeDumper(varInfo.m_highVariablePC, highVariablePC);

    std::string innerVarIndent = (m_useIndent ? varIndent += "\t" : "");

    (((((((((((((((((((((((
                              o_appendOutputString += innerVarIndent) +=
                              "Name=") += varInfo.m_varName) +=
                            ", TypeName=") += varInfo.m_typeName) +=
                          string_format(", Encoding=%u", varInfo.m_varEncoding)) +=
                         ", Value=") += valueType) +=
                       ", Indirection=") += indirectionType) +=
                     string_format(", IndirectionDetail=%u", varInfo.m_varIndirectionDetail)) +=
                    ", AddressRange=[") += lowVariablePC) += ", ") += highVariablePC) += "]") +=
               ", ") += varLocation) +=
             ", ") += isOutParam) +=
           string_format(", brigOffset=%u", varInfo.m_brigOffset)) +=
          "\n") += varIndent) +=
              string_format("}End Variable [%d]\n", depth);

    //Print each child
    for (typename std::vector<VariableInfo<AddrType, VarLocationType> >::const_iterator varInfoIt = varInfo.m_varMembers.begin(); varInfoIt != varInfo.m_varMembers.end(); ++varInfoIt)
    {
        InternalPrintVariableInformation((*varInfoIt), o_appendOutputString, innerVarIndent, depth + 1);
    }
}

/// ---------------------------------------------------------------------------
/// PrintVariableIndirectionType
/// \brief Description: helper function
/// \param[in] indirectionType
/// \param[out] o_appendedOutputString
/// \return void
/// ---------------------------------------------------------------------------
template<typename AddrType, typename LineType, typename VarLocationType>
void DbgInfoDumper<AddrType, LineType, VarLocationType>::PrintVariableIndirectionType(const typename VariableInfo<AddrType, VarLocationType>::VariableIndirection& indirectionType, std::string& o_appendedOutputString)
{
    switch (indirectionType)
    {
        case VariableInfo<AddrType, VarLocationType>::DID_IND_DIRECT:
            o_appendedOutputString += "Direct";
            break;

        case VariableInfo<AddrType, VarLocationType>::DID_IND_POINTER:
            o_appendedOutputString += "Pointer";
            break;

        case VariableInfo<AddrType, VarLocationType>::DID_IND_REFERENCE:
            o_appendedOutputString += "Reference";
            break;

        case VariableInfo<AddrType, VarLocationType>::DID_IND_ARRAY:
            o_appendedOutputString += "Array";
            break;

        default:
            o_appendedOutputString += "Unknown";
            break;
    };
}

/// ---------------------------------------------------------------------------
/// PrintAddressRange
/// \brief Description: Helper function to print an address rand in the following format:
/// (MinAddr, MaxAddr)
/// \param[in] addressRange - the address range to print
/// \param[out] o_appendedOutputString - The appended output string
/// \return void
/// ---------------------------------------------------------------------------
template<typename AddrType, typename LineType, typename VarLocationType>
void DbgInfoDumper<AddrType, LineType, VarLocationType>::PrintAddressRange(const typename CodeScope< AddrType, LineType, VarLocationType>::AddressRange& addressRange, std::string& o_appendedOutputString)
{
    assert(m_addrTypeDumper != nullptr);

    if (m_addrTypeDumper != nullptr)
    {
        std::string minAddr, maxAddr;
        m_addrTypeDumper(addressRange.m_minAddr, minAddr);
        m_addrTypeDumper(addressRange.m_maxAddr, maxAddr);
        o_appendedOutputString += "(";
        o_appendedOutputString += minAddr;
        o_appendedOutputString += ", ";
        o_appendedOutputString += maxAddr;
        o_appendedOutputString += ") ";
    }
}

/// ---------------------------------------------------------------------------
/// PrintInlineLocations
/// \brief Description: prints the inline location
/// \param[in] inlineInformation - contains a LineType
/// \param[out] o_appendedOutputString
/// \return void
/// ---------------------------------------------------------------------------
template<typename AddrType, typename LineType, typename VarLocationType>
void DbgInfoDumper<AddrType, LineType, VarLocationType>::PrintInlineLocations(const typename CodeScope<AddrType, LineType, VarLocationType>::InlineInformation& inlineInformation, std::string& o_appendedOutputString)
{
    assert(m_lineTypeDumper != nullptr);

    if (m_lineTypeDumper != nullptr)
    {
        o_appendedOutputString += "InlineLocations=";
        std::string line;
        m_lineTypeDumper(inlineInformation.m_inlinedAt, line);
        o_appendedOutputString += line;
    }
}

/// ---------------------------------------------------------------------------
/// PrintScopeType
/// \brief Description: Helper function to print the scope type
/// \param[in] scopeType
/// \param[out] o_appendedOutputString
/// \return void
/// ---------------------------------------------------------------------------
template<typename AddrType, typename LineType, typename VarLocationType>
void DbgInfoDumper<AddrType, LineType, VarLocationType>::PrintScopeType(const typename CodeScope<AddrType, LineType, VarLocationType>::ScopeType& scopeType, std::string& o_appendedOutputString)
{
    switch (scopeType)
    {
        case CodeScope<AddrType, LineType, VarLocationType>::DID_SCT_CODE_SCOPE:
            o_appendedOutputString += "Code_Scope";
            break;

        case CodeScope<AddrType, LineType, VarLocationType>::DID_SCT_COMPILATION_UNIT:
            o_appendedOutputString += "Compilation_Unit";
            break;

        case CodeScope<AddrType, LineType, VarLocationType>::DID_SCT_FUNCTION:
            o_appendedOutputString += "Function";
            break;

        case CodeScope<AddrType, LineType, VarLocationType>::DID_SCT_GLOBAL_SCOPE:
            o_appendedOutputString += "Global_Scope";
            break;

        case CodeScope<AddrType, LineType, VarLocationType>::DID_SCT_INLINED_FUNCTION:
            o_appendedOutputString += "Inlined_Function";
            break;

        case CodeScope<AddrType, LineType, VarLocationType>::DID_SCT_HSA_ARGUMENT_SCOPE:
            o_appendedOutputString += "HSA Argument Scope";
            break;

        default:
            o_appendedOutputString += "Unknown";
    };
}

/// -----------------------------------------------------------------------------------------------
/// InternalPrintScope
/// \brief Description: prints all the information about a scope in the following format:
///     {Start Scope [Depth]    //depth is the relative depth of the Scope - Scopes can contain other Scopes
///         Name=Name, Type=Type[, Inline=InlineLocation], AddressRanges=[(low1,high1), ...,  (lowN, highN), FrameBase=]
///         Variables:
///         {Variables...}
///     }End Scope [Depth]
///     //    Children... recursively called
///     {Start Scope [Depth+1]    //depth of child
///         ...
///     }End Scope [Depth+1]    //End of child
/// \param[in]          scopeInfo - The scope
/// \param[out]         o_appendedOutputString - the output string
/// \param[in]          indent - whether to print with indentation
/// \param[in]          depth - the depth to print
/// \return void
/// -----------------------------------------------------------------------------------------------
template<typename AddrType, typename LineType, typename VarLocationType>
void DbgInfoDumper<AddrType, LineType, VarLocationType>::InternalPrintScope(const CodeScope<AddrType, LineType, VarLocationType>& scopeInfo, std::string& o_appendedOutputString, std::string indent, int depth)
{
    std::string scopeIndent = (m_useIndent ? indent : "");
    std::string isKernel = scopeInfo.m_isKernel ? "Kernel" : "Not Kernel";

    (o_appendedOutputString += scopeIndent) += string_format("{Start Scope [%d]\n", depth);
    std::string innerScopeIndent = scopeIndent;

    if (m_useIndent)
    {
        innerScopeIndent += "\t";
    }

    // Scope Type:
    std::string scopeType;
    PrintScopeType(scopeInfo.m_scopeType, scopeType);

    std::string strInlineInfo;

    // Inline information, only print if scope type is INLINED_FUNCTION:
    if (scopeInfo.m_scopeType == CodeScope<AddrType, LineType, VarLocationType>::DID_SCT_INLINED_FUNCTION)
    {
        PrintInlineLocations(scopeInfo.m_inlineInfo, strInlineInfo);
        strInlineInfo += ", ";
    }

    std::string addressRanges;

    for (typename std::vector<typename CodeScope<AddrType, LineType, VarLocationType>::AddressRange>::const_iterator addrIt = scopeInfo.m_scopeAddressRanges.begin(); addrIt != scopeInfo.m_scopeAddressRanges.end(); ++addrIt)
    {
        PrintAddressRange(*addrIt, addressRanges);
    }

    addressRanges += scopeInfo.m_scopeHasNonTrivialAddressRanges ? "- Non-Trivial" : "- Trivial";

    std::string frameBase;

    if (scopeInfo.m_pFrameBase != nullptr)
    {
        m_varLocTypeDumper(*(scopeInfo.m_pFrameBase), frameBase);
    }

    string_prepend(frameBase, "{") += "}";
    std::string cachedAddresses = "CachedAddresses=[";

    for (typename std::set<AddrType>::const_iterator it = scopeInfo.m_addressCache.begin(); it != scopeInfo.m_addressCache.end(); ++it)
    {
        std::string cachedAddr;
        m_addrTypeDumper(*it, cachedAddr);
        cachedAddresses += cachedAddr;
        cachedAddresses += ", ";
    }

    if (!scopeInfo.m_addressCache.empty())
    {
        string_remove_trailing(cachedAddresses, ' ');
        string_remove_trailing(cachedAddresses, ',');
    }

    cachedAddresses += ']';

    std::string workitemOffset;

    if (nullptr != scopeInfo.m_pWorkitemOffset)
    {
        m_varLocTypeDumper(*scopeInfo.m_pWorkitemOffset, workitemOffset);
    }

    ((((((((((((((((((((
                           o_appendedOutputString += innerScopeIndent) +=
                           "Name=") += scopeInfo.m_scopeName) +=
                         ", Type=") += scopeType) +=
                       ", ") += strInlineInfo) +=
                     "AddressRanges=[") += addressRanges) += "]\n") +=
                  innerScopeIndent) +=
                 "FrameBase=") += frameBase) += "\n") +=
              innerScopeIndent) +=
             cachedAddresses) += " ") +=
           isKernel) += " ") +=
         workitemOffset) += "\n";

    // Print each variable child:
    if (scopeInfo.m_scopeVars.size() > 0)
    {
        (o_appendedOutputString += innerScopeIndent) += "Variables:\n";

        for (typename std::vector<VariableInfo<AddrType, VarLocationType>*>::const_iterator scopeVarsIt = scopeInfo.m_scopeVars.begin(); scopeVarsIt != scopeInfo.m_scopeVars.end(); ++scopeVarsIt)
        {
            PrintVariableInformation((**scopeVarsIt), innerScopeIndent, o_appendedOutputString);
        }
    }

    // Child scopes:
    int childScopeDepth = depth + 1;

    for (typename std::vector<CodeScope<AddrType, LineType, VarLocationType>*>::const_iterator scopesIt = scopeInfo.m_children.begin(); scopesIt != scopeInfo.m_children.end(); ++scopesIt)
    {
        InternalPrintScope((**scopesIt), o_appendedOutputString, innerScopeIndent, childScopeDepth);
    }

    // End scope:
    (o_appendedOutputString += scopeIndent) += string_format("}End Scope [%d]\n", depth);
}

/// Deafult / helper print functions:

/// -----------------------------------------------------------------------------------------------
/// HelperFunctionDumpInt
/// \brief Description: helper function which is passed to the dumper - prints int
/// \param[in]          var - the variable to dump
/// \param[in]          outString - the output string
/// \return void
/// -----------------------------------------------------------------------------------------------
template < typename T /* = int */ >
void HelperFunctionDumpInt(const T& var, std::string& outString)
{
    outString = string_format("%d", var);
}

/// -----------------------------------------------------------------------------------------------
/// HelperFunctionDumpUInt
/// \brief Description: helper function which is passed to the dumper - prints uint
/// \param[in]          var - the variable to dump
/// \param[in]          outString - the output string
/// \return void
/// -----------------------------------------------------------------------------------------------
template < typename T /* = unsigned int */ >
void HelperFunctionDumpUInt(const T& var, std::string& outString)
{
    outString = string_format("%u", var);
}

/// -----------------------------------------------------------------------------------------------
/// HelperFunctionDumpLong
/// \brief Description: helper function which is passed to the dumper - prints long
/// \param[in]          var - the variable to dump
/// \param[in]          outString - the output string
/// \return void
/// -----------------------------------------------------------------------------------------------
template < typename T /* = long */ >
void HelperFunctionDumpLong(const T& var, std::string& outString)
{
    outString = string_format("%ld", var);
}

/// -----------------------------------------------------------------------------------------------
/// HelperFunctionDumpULong
/// \brief Description: helper function which is passed to the dumper - prints ulong
/// \param[in]          var - the variable to dump
/// \param[in]          outString - the output string
/// \return void
/// -----------------------------------------------------------------------------------------------
template < typename T /* = unsigned long */ >
void HelperFunctionDumpULong(const T& var, std::string& outString)
{
    outString = string_format("%lu", var);
}
/// -----------------------------------------------------------------------------------------------
/// HelperFunctionDumpULongHex
/// \brief Description: helper function which is passed to the dumper - prints ulong
/// \param[in]          var - the variable to dump
/// \param[in]          outString - the output string
/// \return void
/// -----------------------------------------------------------------------------------------------
template < typename T /* = unsigned long */ >
void HelperFunctionDumpULongHex(const T& var, std::string& outString)
{
    outString = string_format("%#lx", var);
}

/// -----------------------------------------------------------------------------------------------
/// HelperFunctionDumpString
/// \brief Description: helper function which is passed to the dumper - prints string
/// \param[in]          var - the variable to dump
/// \param[in]          outString - the output string
/// \return void
/// -----------------------------------------------------------------------------------------------
template < typename T /* = std::string */ >
void HelperFunctionDumpString(const T& var, std::string& outString)
{
    outString = var;
}

/// -----------------------------------------------------------------------------------------------
/// HelperFunctionDumpHwDbgUInt64AsMemAddr
/// \brief Description: helper function which is passed to the dumper - prints HwDbgUInt64 as memory address
/// \param[in]          var - the variable to dump
/// \param[in]          outString - the output string
/// \return void
/// -----------------------------------------------------------------------------------------------
template < typename T /* = HwDbgUInt64 */ >
void HelperFunctionDumpUnsignedLongLongHex(const T& var, std::string& outString)
{
    outString = string_format("%#llx", var);
}

/// -----------------------------------------------------------------------------------------------
/// HelperFunctionDumpUint64AsEmptyString
/// \brief Description: helper function which is passed to the dumper - prints HwDbgUInt64 as memory address
/// \param[in]          outString - the output string
/// \return void
/// -----------------------------------------------------------------------------------------------
template<typename T>
void HelperFunctionEmptyStringDumper(const T&, std::string& outString)
{
    outString.clear();
}

} //namespace HwDbg

#endif // DBGINFODUMPER_H_
