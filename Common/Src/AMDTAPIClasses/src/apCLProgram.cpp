//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apCLProgram.cpp
///
//==================================================================================

//------------------------------ apCLProgram.cpp ------------------------------

// Infra:
#include <AMDTOSWrappers/Include/osChannel.h>
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTBaseTools/Include/gtASCIIStringTokenizer.h>

// Local:
#include <AMDTAPIClasses/Include/apCLProgram.h>
#include <AMDTAPIClasses/Include/apStringConstants.h>

// ---------------------------------------------------------------------------
// Name:        apProgramBuildStatusAsString
// Description: Return the program build status as string
// Author:  AMD Developer Tools Team
// Date:        4/12/2009
// ---------------------------------------------------------------------------
void apProgramBuildStatusAsString(cl_build_status buildStatus, gtString& statusAsString)
{
    switch (buildStatus)
    {
        case CL_BUILD_NONE:
            statusAsString = L"None";
            break;

        case CL_BUILD_IN_PROGRESS:
            statusAsString = L"In progress";
            break;

        case CL_BUILD_ERROR:
            statusAsString = L"Error";
            break;

        case CL_BUILD_SUCCESS:
            statusAsString = L"Success";
            break;

        default:
            statusAsString = AP_STR_NotAvailable;
            GT_ASSERT_EX(false, L"Unrecognized build status");
            break;
    }
}

// ---------------------------------------------------------------------------
// Name:        apCLProgram::apCLProgram
// Description: Constructor
// Arguments: programHandle - The program's OpenCL handle.
// Author:  AMD Developer Tools Team
// Date:        15/11/2009
// ---------------------------------------------------------------------------
apCLProgram::apCLProgram(oaCLProgramHandle programHandle)
    : _hProgram(programHandle), _wasMarkedForDeletion(false), _referenceCount(0), _programBinaryType(CL_PROGRAM_BINARY_TYPE_NONE), m_isDebuggable(true), m_canUpdateKernels(true)
{
}


// ---------------------------------------------------------------------------
// Name:        apCLProgram::~apCLProgram
// Description: Destructor.
// Author:  AMD Developer Tools Team
// Date:        15/11/2009
// ---------------------------------------------------------------------------
apCLProgram::~apCLProgram()
{
}

// ---------------------------------------------------------------------------
// Name:        apCLProgram::addDevice
// Description: Adds a device for this program
// Arguments - deviceId - The device's API ID.
// Author:  AMD Developer Tools Team
// Date:        17/1/2010
// ---------------------------------------------------------------------------
void apCLProgram::addDevice(int deviceId)
{
    _programDevices.push_back(deviceId);
    programBuildData emptyBuildData;
    _programDevicesBuildData.push_back(emptyBuildData);
}

// ---------------------------------------------------------------------------
// Name:        apCLProgram::clearDevices
// Description: Clears the devices vectors for this program
// Author:  AMD Developer Tools Team
// Date:        17/1/2010
// ---------------------------------------------------------------------------
void apCLProgram::clearDevices()
{
    _programDevices.clear();
    _programDevicesBuildData.clear();
}

// ---------------------------------------------------------------------------
// Name:        apCLProgram::wasProgramBuiltSuccessfully
// Description: Returns true iff the program was built successfully for at least one device
// Author:  AMD Developer Tools Team
// Date:        26/7/2015
// ---------------------------------------------------------------------------
bool apCLProgram::wasProgramBuiltSuccessfully() const
{
    bool retVal = false;

    int buildDataCount = (int)_programDevicesBuildData.size();

    // Check all devices for a successful build:
    for (int i = 0; !retVal && buildDataCount > i; ++i)
    {
        retVal = (CL_BUILD_SUCCESS == _programDevicesBuildData[i]._buildStatus);
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        apCLProgram::onProgramBuild
// Description: Is called when a program is built.
// Arguments: wasProgramBuiltSuccessful - was the program built successfully.
//            buildLog - The program build log.
// Author:  AMD Developer Tools Team
// Date:        15/11/2009
// ---------------------------------------------------------------------------
void apCLProgram::onProgramBuild(bool wasProgramBuiltSuccessful, const gtString& buildLog)
{
    (void)(wasProgramBuiltSuccessful); // unused
    (void)(buildLog); // unused
}

// ---------------------------------------------------------------------------
// Name:        apCLProgram::onKernelCreation
// Description: Is called when a kernelHandle is built from this program.
// Arguments: kernelHandle - The kernel's gandle
// Author:  AMD Developer Tools Team
// Date:        22/11/2009
// ---------------------------------------------------------------------------
void apCLProgram::onKernelCreation(oaCLKernelHandle kernelHandle)
{
    _programKernelHandles.push_back(kernelHandle);
}

// ---------------------------------------------------------------------------
// Name:        apCLProgram::onKernelDeletion
// Description: Is called when kernelHandle, belonging to this program, is deleted
// Author:  AMD Developer Tools Team
// Date:        29/7/2010
// ---------------------------------------------------------------------------
void apCLProgram::onKernelDeletion(oaCLKernelHandle kernelHandle)
{
    // Iterate the kernels:
    int amountOfKernels = (int)_programKernelHandles.size();
    bool foundKernel = false;

    for (int i = 0; i < amountOfKernels; i++)
    {
        if (foundKernel)
        {
            // Note that foundKernel can only be true if we have already looped once,
            // so i has to be greater than 0, and i - 1 is then a valid index:
            _programKernelHandles[i - 1] = _programKernelHandles[i];
        }
        else // !foundKernel
        {
            if (_programKernelHandles[i] == kernelHandle)
            {
                // This is the kernel we searched for, mark that we found it:
                foundKernel = true;
            }
        }
    }

    GT_IF_WITH_ASSERT(foundKernel)
    {
        // The last item is now either our deleted kernel, or a duplicate of the last kernel, so remove it:
        _programKernelHandles.pop_back();
    }
}

// ---------------------------------------------------------------------------
// Name:        apCLProgram::type
// Description: Returns this transferable object type.
// Author:  AMD Developer Tools Team
// Date:        15/11/2009
// ---------------------------------------------------------------------------
osTransferableObjectType apCLProgram::type() const
{
    return OS_TOBJ_ID_CL_PROGRAM;
}


// ---------------------------------------------------------------------------
// Name:        apCLProgram::writeSelfIntoChannel
// Description: Writes this apCLProgram into an IPC Channel.
// Arguments: ipcChannel - The IPC Channel.
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        15/11/2009
// ---------------------------------------------------------------------------
bool apCLProgram::writeSelfIntoChannel(osChannel& ipcChannel) const
{
    ipcChannel << (gtUInt64)_hProgram;

    // Write the related programs:
    gtUInt32 numberOfRelatedPrograms = (gtUInt32)m_relatedPrograms.size();
    ipcChannel << numberOfRelatedPrograms;

    for (gtUInt32 i = 0 ; i < numberOfRelatedPrograms; i++)
    {
        ipcChannel << (gtUInt64)m_relatedPrograms[i];
    }

    ipcChannel << _wasMarkedForDeletion;
    ipcChannel << _programSourceCodeFilePath.asString();

    // Write the program kernels:
    gtUInt32 amountOfKernels = (gtUInt32)_programKernelHandles.size();
    ipcChannel << amountOfKernels;

    for (gtUInt32 i = 0; i < amountOfKernels; i++)
    {
        ipcChannel << (gtUInt64)_programKernelHandles[i];
    }

    // Write the program devices:
    gtUInt32 amountOfDevices = (gtUInt32)_programDevices.size();
    ipcChannel << amountOfDevices;

    for (gtUInt32 i = 0; i < amountOfDevices; i++)
    {
        ipcChannel << (gtInt32)_programDevices[i];
        const programBuildData& currentBuildData = _programDevicesBuildData[i];
        ipcChannel << currentBuildData._buildLog;
        ipcChannel << currentBuildData._buildOptions;
        ipcChannel << (gtInt32)currentBuildData._buildStatus;
        ipcChannel << currentBuildData.m_buildGlobalVariablesTotalSize;
    }

    ipcChannel << (gtInt32)_referenceCount;
    ipcChannel << _programName;

    ipcChannel << (gtInt32)_kernelNames.size();

    for (int i = 0 ; i < (int)_kernelNames.size(); i++)
    {
        ipcChannel << _kernelNames[i];
    }

    // Write the binary type:
    ipcChannel << (gtInt32)_programBinaryType;

    // Write the debuggable data:
    ipcChannel << m_isDebuggable;
    ipcChannel << m_notDebuggableReason;

    // Write the kernel update data:
    ipcChannel << m_canUpdateKernels;

    // Write the allocated object Info:
    apAllocatedObject::writeSelfIntoChannel(ipcChannel);

    return true;
}


// ---------------------------------------------------------------------------
// Name:        apCLProgram::readSelfFromChannel
// Description: Reads this apCLProgram from an IPC Channel.
// Arguments: ipcChannel - The IPC Channel.
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        15/11/2009
// ---------------------------------------------------------------------------
bool apCLProgram::readSelfFromChannel(osChannel& ipcChannel)
{
    // Clear the vectors:
    clearDevices();
    _programKernelHandles.clear();

    // Read the program handle:
    gtUInt64 hProgramAsUint64 = 0;
    ipcChannel >> hProgramAsUint64;
    _hProgram = (oaCLProgramHandle)hProgramAsUint64;

    // Read the related programs:
    m_relatedPrograms.clear();
    gtUInt32 numberOfRelatedPrograms = 0;
    ipcChannel >> numberOfRelatedPrograms;

    for (gtUInt32 i = 0 ; i < numberOfRelatedPrograms; i++)
    {
        gtUInt64 currentRelatedProgramAsUInt64 = 0;
        ipcChannel >> currentRelatedProgramAsUInt64;
        m_relatedPrograms.push_back((oaCLProgramHandle)currentRelatedProgramAsUInt64);
    }

    // Read the deletion status:
    ipcChannel >> _wasMarkedForDeletion;

    // Read the source code file path:
    gtString sourceCodeFilePathAsStr;
    ipcChannel >> sourceCodeFilePathAsStr;
    _programSourceCodeFilePath = sourceCodeFilePathAsStr;

    // Read the program kernels:
    _programKernelHandles.clear();
    gtUInt32 amountOfKernels = 0;
    ipcChannel >> amountOfKernels;

    for (gtUInt32 i = 0; i < amountOfKernels; i++)
    {
        gtUInt64 kernelAsUInt64 = 0;
        ipcChannel >> kernelAsUInt64;
        oaCLKernelHandle currentKernel = (oaCLKernelHandle)kernelAsUInt64;
        _programKernelHandles.push_back(currentKernel);
    }

    // Read the program devices and the build data for each:
    _programDevices.clear();
    _programDevicesBuildData.clear();
    gtUInt32 amountOfDevices = 0;
    ipcChannel >> amountOfDevices;

    for (gtUInt32 i = 0; i < amountOfDevices; i++)
    {
        gtInt32 deviceAsInt32 = 0;
        ipcChannel >> deviceAsInt32;
        int currentDevice = (int)deviceAsInt32;
        _programDevices.push_back(currentDevice);

        programBuildData currentBuildData;
        ipcChannel >> currentBuildData._buildLog;
        ipcChannel >> currentBuildData._buildOptions;
        gtInt32 currentBuildStatusAsInt32 = (gtInt32)CL_BUILD_NONE;
        ipcChannel >> currentBuildStatusAsInt32;
        currentBuildData._buildStatus = (cl_build_status)currentBuildStatusAsInt32;
        ipcChannel >> currentBuildData.m_buildGlobalVariablesTotalSize;
        _programDevicesBuildData.push_back(currentBuildData);
    }

    // Read the reference count:
    gtInt32 refCountAsInt32 = 0;
    ipcChannel >> refCountAsInt32;
    _referenceCount = (int)refCountAsInt32;

    // Read the program name:
    ipcChannel >> _programName;

    gtInt32 numKernels = 0;
    ipcChannel >> numKernels;
    _kernelNames.clear();

    for (int i = 0; i < (int)numKernels; i++)
    {
        gtString kernelName;
        ipcChannel >> kernelName;
        _kernelNames.push_back(kernelName);
    }

    gtInt32 binaryType;
    ipcChannel >> binaryType;
    _programBinaryType = (cl_program_binary_type)binaryType;

    ipcChannel >> m_isDebuggable;
    ipcChannel >> m_notDebuggableReason;

    ipcChannel >> m_canUpdateKernels;

    // Read the allocated object Info:
    apAllocatedObject::readSelfFromChannel(ipcChannel);

    return true;
}


// ---------------------------------------------------------------------------
// Name:        apCLProgram::setProgramKernels
// Description: Sets the program kernel names and amount
// Arguments:   const gtASCIIString& kernelsStr
//              int kernelsAmount
// Author:  AMD Developer Tools Team
// Date:        11/1/2012
// ---------------------------------------------------------------------------
void apCLProgram::setProgramKernels(const gtASCIIString& kernelsStr)
{
    // Set the kernels amount:
    _kernelNames.clear();

    // Parse the kernel names and add to the vector:
    gtASCIIStringTokenizer programsKernelsTokenizer(kernelsStr, ";");
    gtASCIIString kernelName;

    while (programsKernelsTokenizer.getNextToken(kernelName))
    {
        gtString kernelNameStr;
        kernelNameStr.fromASCIIString(kernelName.asCharArray());
        _kernelNames.push_back(kernelNameStr);
    }
}


// ---------------------------------------------------------------------------
// Name:        apCLProgram::programBinaryTypeAsString
// Description: Get the program binary type as string
// Return Val:  gtString
// Author:  AMD Developer Tools Team
// Date:        11/1/2012
// ---------------------------------------------------------------------------
gtString apCLProgram::programBinaryTypeAsString() const
{
    gtString retVal;

    switch (_programBinaryType)
    {
        case CL_PROGRAM_BINARY_TYPE_NONE:
        {
            retVal = AP_STR_clProgramBinaryTypeNone;
            break;
        }

        case CL_PROGRAM_BINARY_TYPE_COMPILED_OBJECT:
        {
            retVal = AP_STR_clProgramBinaryTypeCompiledObject;
            break;
        }

        case CL_PROGRAM_BINARY_TYPE_LIBRARY:
        {
            retVal = AP_STR_clProgramBinaryTypeLibrary;
            break;
        }

        case CL_PROGRAM_BINARY_TYPE_EXECUTABLE:
        {
            retVal = AP_STR_clProgramBinaryTypeExecutable;
            break;
        }

        default:
        {
            GT_ASSERT_EX(false, L"Unknown program binary type");
            break;
        }
    }

    return retVal;
}

