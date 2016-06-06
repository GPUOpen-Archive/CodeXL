//==============================================================================
// Copyright (c) 2012-2015 Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools
/// \file
/// \brief  Definition of the Debug Information dwarf parser
//==============================================================================
#ifndef DBGINFODWARFPARSER_H_
#define DBGINFODWARFPARSER_H_

//@{
/// Elf and Dwarf related typedefs
typedef struct _Elf Elf;
typedef struct _Dwarf_Debug* Dwarf_Debug;
typedef struct _Dwarf_Die* Dwarf_Die;
//@}

/// Dwarf:
#include <libelf.h>
#include <libdwarf.h>
#include <dwarf.h>

/// STL:
#include <string>
#include <vector>

/// Local:
#include <DbgInfoLines.h>
#include <DbgInfoData.h>
#include <DbgInfoDefinitions.h>
/// \note: this is temporarily here due to \a DwarfLocationResolver which will be placed in a different class
#include <DbgInfoConsumerImpl.h>
#include <FacilitiesInterface.h>



namespace HwDbg
{
/// Sets the dwarf address type to be uint64
typedef HwDbgUInt64 DwarfAddrType;

/// -----------------------------------------------------------------------------------------------
/// \struct KernelBinary
/// \brief Description: A simple structure for holding a chunk of memory and its size
/// -----------------------------------------------------------------------------------------------
struct DBGINF_API KernelBinary
{
public:
    KernelBinary(const void* pBinaryData, size_t binarySize);
    KernelBinary(const KernelBinary& other);
    ~KernelBinary();

    KernelBinary& operator=(const KernelBinary& other);

    /// Move semantics
#ifdef HWDBGINFO_MOVE_SEMANTICS
    KernelBinary(KernelBinary&& other);
    KernelBinary& operator=(KernelBinary&& other);
#endif

    void setBinary(const void* pBinaryData, size_t binarySize);

    /// Check conformance to ELF classes:
    bool isElf32Binary() const;
    /// Check conformance to ELF classes:
    bool isElf64Binary() const;

    /// Extract a sub-buffer as a binary itself:
    bool getSubBufferAsBinary(size_t offset, size_t size, KernelBinary& o_bufferAsBinary) const;
    /// Shorthand for a subbuffer trimming headers:
    bool getTrimmedBufferAsBinary(size_t start_trim, size_t end_trim, KernelBinary& o_bufferAsBinary) const;
    /// Extract an ELF section as a binary itself:
    bool getElfSectionAsBinary(int sectionIndex, KernelBinary& o_sectionAsBinary) const;
    /// Extract an ELF section as a binary itself:
    bool getElfSectionAsBinary(const std::string& sectionName, KernelBinary& o_sectionAsBinary, int* o_pSectionLinkIndex = nullptr) const;
    /// Extract an ELF symbol as a binary itself:
    bool getElfSymbolAsBinary(const std::string& symbol, KernelBinary& o_symbolAsBinary) const;

    /// List the section names:
    void listELFSectionNames(std::vector<std::string>& o_sectionNames) const;
    /// List the symbol names:
    void listELFSymbolNames(std::vector<std::string>& o_symbolNames) const;

    const void* m_pBinaryData; ///< A buffer containing the elf binary data
    size_t m_binarySize; ///< The size of the buffer
};

/// -----------------------------------------------------------------------------------------------
/// \struct DwarfVariableLocation
/// \brief Description: Represents a dwarf variable location
/// -----------------------------------------------------------------------------------------------
struct DBGINF_API DwarfVariableLocation
{
public:
    /// Which register holds the value for this location:
    enum LocationRegister
    {
        LOC_REG_REGISTER,   ///< A register holds the value
        LOC_REG_STACK,      ///< The frame pointer holds the value
        LOC_REG_NONE,       ///< No registers are to be used in getting the value
        LOC_REG_UNINIT,     ///< Default / max value
    };

public:
    /// Helper function for the DbgInfoDumper - converts the DwarfVariableLocationto a string
    static void AsString(const DwarfVariableLocation& loc, std::string& o_outputString);
    /// Translate the ValueLocationType enum to a string
    static void LocRegToStr(const LocationRegister& locType, std::string& o_outputString);
    /// Initialize the structure - we need this because of the way we use the location as a template argument we cannot have the init inside the constructor
    void Initialize();

public:
    ////////////////////////////////////////////////////////////////////////////////////
    // The following flow is needed to get a variable value from this location object:
    // 1. What is m_locationRegister:
    //  a. REG_REGISTER:
    //     set cLoc = GetRegister(m_registerNumber)
    //  b. REG_STACK:
    //     set cLoc = GetRegister(framePointer)
    //  c. REG_NONE:
    //     set cLoc = 0
    // 2. Is m_shouldDerefValue:
    //  a. false:
    //     set cVal = cLoc >> m_locationOffset
    //  b. true:
    //     set cLoc = cLoc + m_locationOffset
    //     set cVal = GetMemory(cLoc)
    // 3. set cVal = cVal >> m_pieceOffset
    // 4. set cVal = cVal && ((8 ^ m_pieceSize) - 1)
    // 5. set cVal = cVal + m_constAddition
    // 6. return cVal
    //
    // (m_locationResource is used only if cVal is itself a global memory pointer)
    //////////////////////////////////////////////////////////////////////////////////////
    /// Where is the base value located?
    LocationRegister m_locationRegister;
    /// The register number, if applicable:
    unsigned int m_registerNumber;
    /// Is the value direct or indirect?
    bool m_shouldDerefValue;
    /// Is the value offset?
    unsigned int m_locationOffset;
    /// For ORCA Global memory, the ALU mapping:
    HwDbgUInt64 m_locationResource;
    /// Memory region used in isa dwarf
    unsigned int m_isaMemoryRegion;
    /// For piece operations, the offset of the piece:
    unsigned int m_pieceOffset;
    /// For piece operations, the size of the piece:
    unsigned int m_pieceSize;
    /// Added constant value:
    int m_constAddition;

    /*
    /// ORCA stack debug info:
    /// whether this is a register or a stack offset
    enum ValueLocationType
    {
        LOC_TYPE_REGISTER, ///< treat as a value, offset is inside the register
        LOC_TYPE_INDIRECT_REGISTER, ///< treat the register value as a pointer, offset is added to the register value
        LOC_TYPE_STACK_OFFSET, ///< treat the value as an offset, offset is set to be the same as the value
    };

    HwDbgUInt64 m_location; ///< The location - value if register, pointer if indirect register, offset if stack offset
    ValueLocationType m_locationType; ///< Register, Indirect Register or Stack offset
    unsigned int m_locationOffset; ///< The offset of the variable, used primarily for member variables
    HwDbgUInt64 m_locationResource; ///< The ALU which this variable is mapped to
    */
};

///////////////////////////////////////////////////////////////////////////////////////////////////
/// \class DbgInfoDwarfParser
/// \brief Description: Used to convert Dwarf to DbgInfo Structures, all methods are static, this class should only be used to parse the data
/// Afterwards all logic to query the data is in the consumer
/// This class has the following instantiation:
/// - LineType    - DbgInfoLines::FileLocation
/// - AddressType - DwarfAddrType
/// - VarLocationType - DwarfVariableLocation
/// It allows us to parse dwarf to our dbgInfo classes
/// The main function is InitializeWithBinary
/// All methods are static and no initialization is required
///////////////////////////////////////////////////////////////////////////////////////////////////

class DBGINF_API DbgInfoDwarfParser
{
public:
    //@{
    /// Some typedefs to make my life easier:
    typedef LineNumberMapping<DwarfAddrType, FileLocation> DwarfLineMapping;
    typedef CodeScope<DwarfAddrType, FileLocation, DwarfVariableLocation> DwarfCodeScope;
    typedef DwarfCodeScope::ScopeType DwarfCodeScopeType;
    typedef DwarfCodeScope::AddressRange DwarfCodeScopeAddressRange;
    typedef VariableInfo<DwarfAddrType, DwarfVariableLocation> DwarfVariableInfo;
    typedef HwDbgInfo_indirection DwarfVariableIndirectionType;
    //@}
    /// The main function - fills the scope and line mapping from the binary, the filepath, if provided, is the path to the file from which the source was taken
    static bool InitializeWithBinary(const KernelBinary& kernelBinary, DwarfCodeScope& o_scope, DwarfLineMapping& o_lineNumberMapping, const std::string& firstSourceFileRealPath = "");
    /// Given a scope, return all the locations of the variables whose type is REGISTER in the scope
    static bool ListVariableRegisterLocations(const DwarfCodeScope* pTopScope, std::vector<DwarfAddrType>& variableLocations);

private:
    /// Fills the variable info from DWARF, also returns a vector of additional location in case of several instances of the variable
    static void FillVariableWithInformationFromDIE(Dwarf_Die variableDIE, Dwarf_Debug pDwarf, bool isMember, DwarfVariableInfo& o_variableData, std::vector<DwarfVariableLocation>& o_variableAdditionalLocations);
    /// Fills various fields of a variable
    static void FillTypeNameAndDetailsFromTypeDIE(Dwarf_Die typeDIE, Dwarf_Debug pDwarf, bool expandIndirectMembers, bool isRegisterParamter, DwarfVariableInfo& o_variable);
    /// Fills the LineNumberMapping from DWARF
    static bool FillLineMappingFromDwarf(Dwarf_Die cuDIE, const std::string& firstSourceFileRealPath, Dwarf_Debug pDwarf, DwarfLineMapping& o_lineNumberMapping);
    /// Fills the Scope from DWARF
    static void FillCodeScopeFromDwarf(Dwarf_Die programDIE, const std::string& firstSourceFileRealPath, Dwarf_Debug pDwarf, DwarfCodeScope* pParentScope, const DwarfCodeScopeType& scopeType, DwarfCodeScope& o_scope);
    /// Fills the address ranges from DWARF
    static void FillAddressRanges(Dwarf_Die programDIE, Dwarf_Debug pDwarf, DwarfCodeScope& o_scope);
    /// Fills the scope name from DWARF
    static void FillScopeName(Dwarf_Die programDIE, Dwarf_Debug pDwarf, std::string& o_scopeName);
    /// Fills the scope hsa data from DWARF
    static void FillScopeHsaData(Dwarf_Die programDIE, Dwarf_Debug pDwarf, DwarfCodeScope& o_scope);
    /// Fills the FrameBase - which is the base variable location of the scope - from DWARF
    static void FillFrameBase(Dwarf_Die programDIE, Dwarf_Debug pDwarf, DwarfCodeScope& o_scope);
    /// In case this is an inlined function, fill the Inlined data which is the line in which the function is defined, from DWARF
    static void FillInlinedFunctionData(Dwarf_Die programDIE, const std::string& firstSourceFileRealPath, Dwarf_Debug pDwarf, DwarfCodeScope& o_scope);
    /// Fills the child scopes recursively
    static void FillChildren(Dwarf_Die programDIE, const std::string& firstSourceFileRealPath, Dwarf_Debug pDwarf, DwarfCodeScope& o_scope);
    /// Fills the Indirection of a variable from DWARF
    static void FillVarIndirectionDetails(Dwarf_Die variableDIE, DwarfVariableInfo& o_variable);
    /// Fills the Encoding of a variable from DWARF
    static void FillVarEncoding(Dwarf_Die variableDIE, DwarfVariableInfo& o_variable);
    /// Fills the Name of a variable from DWARF
    static void FillVarName(Dwarf_Die variableDIE, Dwarf_Debug pDwarf, DwarfVariableInfo& o_variable);
    /// Fills the const value of a variable from DWARF (if it is const)
    static void FillConstValue(Dwarf_Die variableDIE, Dwarf_Debug pDwarf, DwarfVariableInfo& o_variable);
    /// Create the name of a variable from type from DWARF
    static void CreateVarNameFromType(Dwarf_Die typeDIE, DwarfVariableInfo& o_variable);
    /// Add a child scope from DWARF
    static void AddChildScope(Dwarf_Die childDIE, const std::string& firstSourceFileRealPath, Dwarf_Debug pDwarf, const DwarfCodeScopeType& childScopeType, DwarfCodeScope& o_scope);
    /// Get the variable type from DWARF
    static void GetVariableValueTypeFromTAG(int dwarfTAG, bool& isConst, bool& isParam);
    /// Get the scope type from DWARF tag
    static DwarfCodeScopeType GetScopeTypeFromTAG(int dwarfTAG);
    /// Get the location of a variable from DWARF
    static void UpdateLocationWithDWARFData(const Dwarf_Loc& locationRegister, DwarfVariableLocation& io_location, bool isMember);
    /// retrieves a die according to a reference attribute
    static int GetDwarfFormRefDie(Dwarf_Attribute attr, Dwarf_Die* pReturnDIE, Dwarf_Error* pError, Dwarf_Debug dbg);

    /// Helper function to be used in compound consumer - is here temporarily because it contains DWARF specific logic:
    static bool DwarfLocationResolver(const DwarfVariableLocation& hVarLoc, const HwDbgUInt64& lAddr, const DbgInfoConsumerImpl<HwDbgUInt64, DwarfAddrType, DwarfVariableLocation>& lConsumer, DwarfVariableLocation& o_lVarLocation, void* userData = nullptr);
};
}; // end of HwDbg namespace


#endif // DBGINFODWARFPARSER_H_
