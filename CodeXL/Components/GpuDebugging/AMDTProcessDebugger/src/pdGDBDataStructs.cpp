//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file pdGDBDataStructs.cpp
///
//==================================================================================

//------------------------------ pdGDBDataStructs.cpp ------------------------------

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>

// Local:
#include <src/pdGDBDataStructs.h>

// ------------------------------- pdGDBData -------------------------------

// ---------------------------------------------------------------------------
// Name:        pdGDBData::executeGDBCommand
// Description: Destructor.
// Author:      Yaki Tebeka
// Date:        7/1/2007
// ---------------------------------------------------------------------------
pdGDBData::~pdGDBData()
{
}


// ------------------------------- pdGDBThreadData -------------------------------

// ---------------------------------------------------------------------------
// Name:        pdGDBThreadData::pdGDBThreadData
// Description: Consturcor.
// Author:      Yaki Tebeka
// Date:        7/1/2007
// ---------------------------------------------------------------------------
pdGDBThreadData::pdGDBThreadData()
    : _OSThreadId(OS_NO_THREAD_ID),
      _gdbThreadId(-1),
      _threadIPLocation(0),
      _isGDBsActiveThread(false),
      _isDriverThread(false)
{
}


// ---------------------------------------------------------------------------
// Name:        pdGDBThreadData::~pdGDBThreadData
// Description: Destructor.
// Author:      Yaki Tebeka
// Date:        7/1/2007
// ---------------------------------------------------------------------------
pdGDBThreadData::~pdGDBThreadData()
{
}


// ---------------------------------------------------------------------------
// Name:        pdGDBThreadData::type
// Description: Returns my GDB data type.
// Author:      Yaki Tebeka
// Date:        7/1/2007
// ---------------------------------------------------------------------------
pdGDBData::pdGDBDataType pdGDBThreadData::type() const
{
    return pdGDBData::PD_GDB_THREAD_DATA;
}


// ------------------------------- pdGDBThreadDataList -------------------------------

// ---------------------------------------------------------------------------
// Name:        pdGDBThreadDataList::pdGDBThreadDataList
// Description: Consturcor.
// Author:      Yaki Tebeka
// Date:        7/1/2007
// ---------------------------------------------------------------------------
pdGDBThreadDataList::pdGDBThreadDataList()
{
}


// ---------------------------------------------------------------------------
// Name:        pdGDBThreadDataList::~pdGDBThreadDataList
// Description: Destructor.
// Author:      Yaki Tebeka
// Date:        7/1/2007
// ---------------------------------------------------------------------------
pdGDBThreadDataList::~pdGDBThreadDataList()
{
}


// ---------------------------------------------------------------------------
// Name:        pdGDBThreadDataList::type
// Description: Returns my GDB data type.
// Author:      Yaki Tebeka
// Date:        7/1/2007
// ---------------------------------------------------------------------------
pdGDBData::pdGDBDataType pdGDBThreadDataList::type() const
{
    return pdGDBData::PD_GDB_THREAD_DATA_LIST;
}


// ------------------------------- pdGDBCallStack -------------------------------

// ---------------------------------------------------------------------------
// Name:        pdGDBCallStack::pdGDBCallStack
// Description: Constructor.
// Author:      Yaki Tebeka
// Date:        14/3/2007
// ---------------------------------------------------------------------------
pdGDBCallStack::pdGDBCallStack()
{
}


// ---------------------------------------------------------------------------
// Name:        pdGDBCallStack::~pdGDBCallStack
// Description: Destructor.
// Author:      Yaki Tebeka
// Date:        14/3/2007
// ---------------------------------------------------------------------------
pdGDBCallStack::~pdGDBCallStack()
{
}


// ---------------------------------------------------------------------------
// Name:        pdGDBCallStack::type
// Description: Returns my GDB data type.
// Author:      Yaki Tebeka
// Date:        14/3/2007
// ---------------------------------------------------------------------------
pdGDBData::pdGDBDataType pdGDBCallStack::type() const
{
    return pdGDBData::PD_GDB_CALL_STACK_DATA;
}


// ------------------------------- pdGDBProcessId -------------------------------


// ---------------------------------------------------------------------------
// Name:        pdGDBProcessId::pdGDBProcessId
// Description: Constructor.
// Author:      Yaki Tebeka
// Date:        29/8/2007
// ---------------------------------------------------------------------------
pdGDBProcessId::pdGDBProcessId()
    : _processPid(0)
{
}


// ---------------------------------------------------------------------------
// Name:        pdGDBProcessId::~pdGDBProcessId
// Description: Destructor
// Author:      Yaki Tebeka
// Date:        29/8/2007
// ---------------------------------------------------------------------------
pdGDBProcessId::~pdGDBProcessId()
{
}


// ---------------------------------------------------------------------------
// Name:        pdGDBProcessId::type
// Description: Returns my GDB data type.
// Author:      Yaki Tebeka
// Date:        29/8/2007
// ---------------------------------------------------------------------------
pdGDBData::pdGDBDataType pdGDBProcessId::type() const
{
    return pdGDBData::PD_GDB_PROCESS_ID;
}

// ----------------------------- pdGDBSourceCodeData -----------------------------

// ---------------------------------------------------------------------------
// Name:        pdGDBSourceCodeData::pdGDBSourceCodeData
// Description: Constructor
// Author:      Uri Shomroni
// Date:        17/2/2009
// ---------------------------------------------------------------------------
pdGDBSourceCodeData::pdGDBSourceCodeData(const osFilePath& sourceCodeFilePath, unsigned int lineNumber)
    : _sourceCodeFilePath(sourceCodeFilePath), _lineNumber(lineNumber)
{

}

// ---------------------------------------------------------------------------
// Name:        pdGDBSourceCodeData::~pdGDBSourceCodeData
// Description: Destructor
// Author:      Uri Shomroni
// Date:        17/2/2009
// ---------------------------------------------------------------------------
pdGDBSourceCodeData::~pdGDBSourceCodeData()
{

}

// ---------------------------------------------------------------------------
// Name:        pdGDBSourceCodeData::type
// Description: Returns my GDB data type.
// Author:      Uri Shomroni
// Date:        17/2/2009
// ---------------------------------------------------------------------------
pdGDBData::pdGDBDataType pdGDBSourceCodeData::type() const
{
    return pdGDBData::PD_GDB_SOURCE_DATA;
}

// ------------------------------- pdGDBLibraryData ------------------------------

// ---------------------------------------------------------------------------
// Name:        pdGDBLibraryData::pdGDBLibraryData
// Description: Constructor
// Author:      Uri Shomroni
// Date:        22/2/2009
// ---------------------------------------------------------------------------
pdGDBLibraryData::pdGDBLibraryData()
{

}

// ---------------------------------------------------------------------------
// Name:        pdGDBLibraryData::~pdGDBLibraryData
// Description: Destructor
// Author:      Uri Shomroni
// Date:        22/2/2009
// ---------------------------------------------------------------------------
pdGDBLibraryData::~pdGDBLibraryData()
{

}

// ---------------------------------------------------------------------------
// Name:        pdGDBLibraryData::type
// Description: Returns my GDB data type.
// Author:      Uri Shomroni
// Date:        22/2/2009
// ---------------------------------------------------------------------------
pdGDBData::pdGDBDataType pdGDBLibraryData::type() const
{
    return pdGDBData::PD_GDB_LIBRARY_DATA;
}

// ------------------------------- pdGDBStringData -------------------------------


// ---------------------------------------------------------------------------
// Name:        pdGDBStringData::pdGDBStringData
// Description: Constructor.
// Author:      Yaki Tebeka
// Date:        9/1/2008
// ---------------------------------------------------------------------------
pdGDBStringData::pdGDBStringData(const gtASCIIString& gdbOutputString)
    : _gdbOutputString(gdbOutputString)
{
}


// ---------------------------------------------------------------------------
// Name:        pdGDBStringData::~pdGDBStringData
// Description: Destructor
// Author:      Yaki Tebeka
// Date:        9/1/2008
// ---------------------------------------------------------------------------
pdGDBStringData::~pdGDBStringData()
{
}


// ---------------------------------------------------------------------------
// Name:        pdGDBStringData::type
// Description: Returns my GDB data type.
// Author:      Yaki Tebeka
// Date:        9/1/2008
// ---------------------------------------------------------------------------
pdGDBData::pdGDBDataType pdGDBStringData::type() const
{
    return pdGDBData::PD_GDB_STRING_DATA;
}

///////////////////////////////////////////////
/// \brief Standard constructor (unavailable)
///
pdGDBFrameLocalsData::pdGDBFrameLocalsData() {}

///////////////////////////////////////////////
/// \brief Constructor
///
/// \param[in]  locals a list of "local variable name" <-> "local variable value" pairs
pdGDBFrameLocalsData::pdGDBFrameLocalsData(const gtList < std::pair<gtString, gtString> >& locals): _localsVariables(locals) {}

///////////////////////////////////////////////
/// \brief Standard virtual destructor
///
pdGDBFrameLocalsData::~pdGDBFrameLocalsData() {}

///////////////////////////////////////////////
/// \brief Get current instance type
///
/// \return data type of the pdGDBFrameLocalVariableValue structure
pdGDBData::pdGDBDataType pdGDBFrameLocalsData::type() const
{
    return pdGDBData::PD_LOCALS_VARIABLES_LIST_DATA;
}

////////////////////////////////////////////
/// \brief Standard constructor (unavailable)
///
pdGDBFrameLocalVariableValue::pdGDBFrameLocalVariableValue() {}


///////////////////////////////////////////////
/// \brief Constructor
///
/// \param[in]  variableName a name of requested local variable
/// \param[in]  variableValue a ASCII presentation of local variable value
pdGDBFrameLocalVariableValue::pdGDBFrameLocalVariableValue(const gtString& variableName, const gtString&  variableValue):
    _variableName(variableName), _variableValue(variableValue)
{
}

///////////////////////////////////////////////
/// \brief Standard virtual destructor
///
pdGDBFrameLocalVariableValue::~pdGDBFrameLocalVariableValue() {}

///////////////////////////////////////////////
/// \brief Get current instance type
///
/// \return data type of the pdGDBFrameLocalVariableValue structure
pdGDBData::pdGDBDataType pdGDBFrameLocalVariableValue::type() const
{
    return pdGDBData::PD_LOCAL_VARIABLE_VALUE_DATA;
}

///////////////////////////////////////////////
/// \brief Constructor
///
/// \param index a new gdb breakpoint index
pdGDBBreakpointIndex::pdGDBBreakpointIndex(int index): m_gdbBreakpointIndex(index)
{
}

///////////////////////////////////////////////
/// \brief Get current instance type
///
/// \return data type of the pdGDBFrameLocalVariableValue structure
pdGDBData::pdGDBDataType pdGDBBreakpointIndex::type() const
{
    return pdGDBData::PD_GDB_BREAKPOINT_INDEX_DATA;
}

///////////////////////////////////////////////
/// \brief Standard virtual destructor
///
pdGDBBreakpointIndex::~pdGDBBreakpointIndex()
{

}

///////////////////////////////////////////////
/// \brief Constructor
///
/// \param[in]  variableType a ASCII presentation of variable type
pdGDBFVariableType::pdGDBFVariableType(const gtString&  variableType) : _variableType(variableType) {};

///////////////////////////////////////////////
/// \brief Get current instance type
///
/// \return data type of the pdGDBFrameLocalVariableValue structure
pdGDBData::pdGDBDataType pdGDBFVariableType::type() const { return pdGDBData::PD_GDB_VARIABLE_TYPE_DATA; };

///////////////////////////////////////////////
/// \brief Standard virtual destructor
///
pdGDBFVariableType::~pdGDBFVariableType() {};

///////////////////////////////////////////////
/// \brief Standard constructor (unavailable)
///
pdGDBFVariableType::pdGDBFVariableType() {};

///////////////////////////////////////////////
/// \brief Constructor
///
/// \param errorInfo a gdb error string
pdGDBHostStepErrorInfoIndex::pdGDBHostStepErrorInfoIndex(const gtASCIIString& errorInfo) : m_gdbErrorInfo(errorInfo) {};

///////////////////////////////////////////////
/// \brief Get current instance type
///
/// \return data type of the pdGDBFrameLocalVariableValue structure
pdGDBData::pdGDBDataType pdGDBHostStepErrorInfoIndex::type() const { return pdGDBData::PD_GDB_HOST_STEP_RESULT_DATA; }

///////////////////////////////////////////////
/// \brief Standard virtual destructor
///
pdGDBHostStepErrorInfoIndex::~pdGDBHostStepErrorInfoIndex() {};


