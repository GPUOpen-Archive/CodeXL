//=====================================================================
// Copyright 2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file osWin32DebugInfoReader.cpp
///
//=====================================================================

//------------------------------ osWin32DebugInfoReader.cpp ------------------------------

// Windows:
#define WIN32_LEAN_AND_MEAN 1
#include <Windows.h>

// Windows Debug Help library:
#pragma warning( push )
#pragma warning( disable : 4091)
#include <dbghelp.h>
#pragma warning( pop )

// Local:
#include <AMDTOSWrappers/Include/osFilePath.h>
#include <AMDTOSWrappers/Include/osWin32DebugInfoReader.h>
#include <AMDTOSWrappers/Include/osCallStackFrame.h>
#include <AMDTBaseTools/Include/gtAssert.h>


// Defines the size of a buffer that receives symbols information:
#define OS_SYM_BUFF_SIZE 512

// Changes from Win7 kit to Win8 kit:
typedef struct _IMAGEHLP_MODULEW64_WIN7
{
    DWORD    SizeOfStruct;           // set to sizeof(IMAGEHLP_MODULE64)
    DWORD64  BaseOfImage;            // base load address of module
    DWORD    ImageSize;              // virtual size of the loaded module
    DWORD    TimeDateStamp;          // date/time stamp from pe header
    DWORD    CheckSum;               // checksum from the pe header
    DWORD    NumSyms;                // number of symbols in the symbol table
    SYM_TYPE SymType;                // type of symbols loaded
    WCHAR    ModuleName[32];         // module name
    WCHAR    ImageName[256];         // image name
    // new elements: 07-Jun-2002
    WCHAR    LoadedImageName[256];   // symbol file name
    WCHAR    LoadedPdbName[256];     // pdb file name
    DWORD    CVSig;                  // Signature of the CV record in the debug directories
    WCHAR        CVData[MAX_PATH * 3];   // Contents of the CV record
    DWORD    PdbSig;                 // Signature of PDB
    GUID     PdbSig70;               // Signature of PDB (VC 7 and up)
    DWORD    PdbAge;                 // DBI age of pdb
    BOOL     PdbUnmatched;           // loaded an unmatched pdb
    BOOL     DbgUnmatched;           // loaded an unmatched dbg
    BOOL     LineNumbers;            // we have line number information
    BOOL     GlobalSymbols;          // we have internal symbol information
    BOOL     TypeInfo;               // we have type information
    // new elements: 17-Dec-2003
    BOOL     SourceIndexed;          // pdb supports source server
    BOOL     Publics;                // contains public symbols
} IMAGEHLP_MODULEW64_WIN7, *PIMAGEHLP_MODULEW64_WIN7;




// ---------------------------------------------------------------------------
// Name:        osWin32DebugInfoReader::osWin32DebugInfoReader
// Description: Constructor
// Arguments:   hProcess - The handle of process whose debug info will be queried.
// Author:      AMD Developer Tools Team
// Date:        9/5/2005
// ---------------------------------------------------------------------------
osWin32DebugInfoReader::osWin32DebugInfoReader(const osProcessHandle& hProcess)
    : _hProcess(hProcess)
{
}


// ---------------------------------------------------------------------------
// Name:        osWin32DebugInfoReader::getModuleFromAddress
// Description: Inputs an address in the queried process virtual address space
//              and returns the details of the module in which it resides.
// Arguments:   address - The input address.
//              moduleName - The output module name.
//              moduleStartAddr - the module base address in memory
// Return Val:  bool - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        9/5/2005
// ---------------------------------------------------------------------------
bool osWin32DebugInfoReader::getModuleFromAddress(DWORD64 address, osFilePath& moduleFilePath, osInstructionPointer& moduleStartAddr) const
{
    bool retVal = false;

    // A struct that will get the module info:
    IMAGEHLP_MODULE64 moduleInfo;
    moduleInfo.SizeOfStruct = sizeof(IMAGEHLP_MODULE64);

    // Get the module info:
    BOOL rc = SymGetModuleInfo64(_hProcess, address, &moduleInfo);

    if (rc)
    {
        // Output the module file name:
        gtString moduleFileName = moduleInfo.LoadedImageName;
        osFilePath modulePath(moduleFileName);
        moduleFilePath = modulePath;

        moduleStartAddr = (osInstructionPointer)moduleInfo.BaseOfImage;

        retVal = true;
    }
    else
    {
        // The problem might be that dbghelp.dll being used is older than the one we used to build this file.
        // Try an older version:
        if (ERROR_INVALID_PARAMETER == GetLastError())
        {
            // A struct that will get the module info:
            IMAGEHLP_MODULEW64_WIN7 moduleInfoWin7;
            moduleInfoWin7.SizeOfStruct = sizeof(IMAGEHLP_MODULEW64_WIN7);

            // Get the module info:
            rc = SymGetModuleInfo64(_hProcess, address, (PIMAGEHLP_MODULE64)&moduleInfoWin7);

            if (rc)
            {
                // Output the module file name:
                gtString moduleFileName = moduleInfoWin7.LoadedImageName;
                osFilePath modulePath(moduleFileName);
                moduleFilePath = modulePath;

                moduleStartAddr = (osInstructionPointer)moduleInfoWin7.BaseOfImage;

                retVal = true;
            }
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        osWin32DebugInfoReader::getFunctionFromAddress
/// Description: Inputs an address in the queried process virtual address space
//               and returns the details of function in which this address resides.
// Arguments:   address - The input address.
//              functionStartAddress - Will get the function start address.
//              functionName - Will get the function name.
// Return Val:  bool - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        9/5/2005
// ---------------------------------------------------------------------------
bool osWin32DebugInfoReader::getFunctionFromAddress(DWORD64 address, DWORD64& functionStartAddress,
                                                    gtString& functionName) const
{
    bool retVal = false;

    // A buffer that will hold the symbol information:
    BYTE symbolBuff[OS_SYM_BUFF_SIZE];
    PIMAGEHLP_SYMBOL64 pSym = (PIMAGEHLP_SYMBOL64)&symbolBuff;
    ZeroMemory(pSym , OS_SYM_BUFF_SIZE) ;
    pSym->SizeOfStruct = sizeof(IMAGEHLP_SYMBOL64) ;
    pSym->MaxNameLength = OS_SYM_BUFF_SIZE - sizeof(IMAGEHLP_SYMBOL64);

    // Will get the displacement of the input address from the address where the
    // found symbol begins:
    DWORD64 displacementFromSybmolBeginAddress = 0;

    // Get the function symbol information:
    if (SymGetSymFromAddr64(_hProcess, address,
                            &displacementFromSybmolBeginAddress, pSym) == TRUE)
    {
        // Output the function name:
        functionName.fromASCIIString(pSym->Name);

        // Output the function start address:
        functionStartAddress = address - displacementFromSybmolBeginAddress;

        retVal = true;
    }

    return retVal;
}



// ---------------------------------------------------------------------------
// Name:        osWin32DebugInfoReader::getSourceCodeFromAddress
// Description: Inputs an address in the queried process virtual address space
//              and returns the details of source code in which this address resides.
// Arguments:   address - The input address.
//              sourceCodeFile, lineNumber - Will get the source code file path and source code
//                                           line number that generated the code in that is loaded
//                                           into a place in which the input address resides.
// Return Val:  bool - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        9/5/2005
// ---------------------------------------------------------------------------
bool osWin32DebugInfoReader::getSourceCodeFromAddress(DWORD64 address, osFilePath& sourceCodeFile,
                                                      int& lineNumber) const
{
    bool retVal = false;

    // Get the source code file and line data:
    IMAGEHLP_LINE64 lineInfo = { sizeof(IMAGEHLP_LINE64) };
    DWORD lineDisplacement = 0;

    if (SymGetLineFromAddr64(_hProcess, address, &lineDisplacement, &lineInfo) == TRUE)
    {
        // Output the line number:
        lineNumber = lineInfo.LineNumber;

        // Output source file path:
        gtString fileName = lineInfo.FileName;
        osFilePath filePath(fileName);
        sourceCodeFile = filePath;

        retVal = true;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        osWin32DebugInfoReader::fillStackFrame
// Description: Fill a stack frame which only has an instruction counter address
//              and fills it with the rest of the debug information.
//                  consolidated from the functions whose headers follow.
// Return Val: bool  - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        26/10/2008
// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------
// Name:        osWin32CallStackReader::fillStackFrameModuleDetails
// Description: Inputs a windows stack frame and uses it to fill the associated
//              module file path of an osCallStackFrame object.
// Author:      AMD Developer Tools Team
// Date:        12/10/2004
// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------
// Name:        osWin32CallStackReader::fillStackFrameFunctionDetails
// Description:
//   Inputs a windows stack frame and uses it to fill the function details
//   of an osCallStackFrame object:
//   - Function name.
//   - Function start address.
//   - Instructing counter address.
//
//   If the function does not appear in the process symbols server:
//   - The function start address will not be filled.
//   - The function name will get the DLL name and the offset from the beginning
//     of the dll virtual address.
// Author:      AMD Developer Tools Team
// Date:        12/10/2004
// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------
// Name:        osWin32CallStackReader::fillStackFrameSourceCodeDetails
// Description: Inputs a windows stack frame and uses it to fill the associated
//              source file and line number details of an osCallStackFrame object.
// Author:      AMD Developer Tools Team
// Date:        12/10/2004
// ---------------------------------------------------------------------------
bool osWin32DebugInfoReader::fillStackFrame(osCallStackFrame& stackFrame)
{
    bool retVal = false;

    DWORD64 instructionAddress = stackFrame.instructionCounterAddress();

    // Get the module file path:
    osFilePath moduleFilePath;
    osInstructionPointer moduleStartAddr = (osInstructionPointer)NULL;
    bool rc1 = getModuleFromAddress(instructionAddress, moduleFilePath, moduleStartAddr);

    if (rc1)
    {
        // Fill the module file name and base address:
        stackFrame.setModuleFilePath(moduleFilePath);
        stackFrame.setModuleStartAddress(moduleStartAddr);

        // Check if this is a spy frame:
        // Get the file name of the current stack frame module:
        const osFilePath& modulePath = stackFrame.moduleFilePath();
        gtString modulePathLowerCase = modulePath.asString();
        modulePathLowerCase.toLowerCase();

        // If this is an OpenGL / OpenGL ES implementation function:
        if ((modulePathLowerCase.find(OS_GREMEDY_OPENCL_SERVER_MODULE_NAME) != -1) ||
            (modulePathLowerCase.find(OS_GREMEDY_OPENGL_SERVER_MODULE_NAME) != -1) ||
            (modulePathLowerCase.find(OS_OPENGL_ES_COMMON_DLL_NAME) != -1) ||
            (modulePathLowerCase.find(OS_OPENGL_ES_COMMON_LITE_DLL_NAME) != -1))
        {
            // If this is NOT the system's OpenGL dll:
            if ((-1 == modulePathLowerCase.find(OS_SYSTEM_32_FOLDER_NAME)) &&
                (-1 == modulePathLowerCase.find(OS_SYSTEM_WOW64_FOLDER_NAME)))
            {
                // Mark the stack frame as containing "spy" function:
                stackFrame.markAsSpyFunction();
            }
        }

        // If this is an OpenCL implementation function:
        if (modulePathLowerCase.find(OS_GREMEDY_OPENCL_SERVER_MODULE_NAME) != -1)
        {
            // If this is NOT the system's OpenCL dll:
            if ((-1 == modulePathLowerCase.find(OS_SYSTEM_32_FOLDER_NAME)) &&
                (-1 == modulePathLowerCase.find(OS_SYSTEM_WOW64_FOLDER_NAME)))
            {
                // Mark the stack frame as containing "spy" function:
                stackFrame.markAsSpyFunction();
            }
        }

        // If this is an NVIDIA OGL driver implementation function:
        gtString nvidiaOGLModuleName = OS_NVIDIA_OGL_DRIVER_DLL_NAME;
        nvidiaOGLModuleName += L".dll";

        if (modulePathLowerCase.find(nvidiaOGLModuleName) != -1)
        {
            // Mark the stack frame as containing "spy" function:
            // (This removes NVIDIA OGL driver call stack frames from
            //  the call stack when breaking on GLExpert messages)
            stackFrame.markAsSpyFunction();
        }
    }

    // Get the function details:
    DWORD64 functionStartAddress = 0;
    gtString functionName;
    bool rc2 = getFunctionFromAddress(instructionAddress, functionStartAddress, functionName);

    if (rc2)
    {
        // Fill the function name:
        stackFrame.setFunctionName(functionName);

        // Fill the function start address:
        stackFrame.setFunctionStartAddress(functionStartAddress);
    }

    // Get the source code file and line:
    osFilePath sourceCodeFile;
    int lineNumber = 0;
    bool rc3 = getSourceCodeFromAddress(instructionAddress, sourceCodeFile, lineNumber);

    if (rc3)
    {
        // Fill the line number:
        stackFrame.setSourceCodeFileLineNumber(lineNumber);

        // Fill the output source file path:
        stackFrame.setSourceCodeFilePath(sourceCodeFile);
    }

    retVal = rc1 && rc2 && rc3;

    return retVal;
}

