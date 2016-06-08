//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apCLProgram.h
///
//==================================================================================

//------------------------------ apCLProgram.h ------------------------------

#ifndef __APCLPROGRAM_H
#define __APCLPROGRAM_H

// Infra:
#include <AMDTBaseTools/Include/gtASCIIString.h>
#include <AMDTBaseTools/Include/gtString.h>
#include <AMDTBaseTools/Include/gtVector.h>
#include <AMDTOSWrappers/Include/osFilePath.h>
#include <AMDTOSAPIWrappers/Include/oaOpenCLIncludes.h>
#include <AMDTOSAPIWrappers/Include/oaOSAPIDefinitions.h>

// Local:
#include <AMDTAPIClasses/Include/apAPIClassesDLLBuild.h>
#include <AMDTAPIClasses/Include/apAllocatedObject.h>


void AP_API apProgramBuildStatusAsString(cl_build_status buildStatus, gtString& statusAsString);

// ----------------------------------------------------------------------------------
// Class Name:           AP_API apCLProgram : public apAllocatedObject
// General Description:
//   Represents an OpenCL program.
//
// Author:  AMD Developer Tools Team
// Creation Date:        15/11/2009
// ----------------------------------------------------------------------------------
class AP_API apCLProgram : public apAllocatedObject
{
public:
    // Held per device to describe the build information:
    struct programBuildData
    {
        // Contains the program build status:
        cl_build_status _buildStatus;

        // Contains the program build log:
        gtString _buildLog;

        // Contains the program build options:
        gtString _buildOptions;

        // Contains the program global variables' total size:
        gtUInt64 m_buildGlobalVariablesTotalSize;

    public:
        programBuildData()
            : _buildStatus(CL_BUILD_NONE), m_buildGlobalVariablesTotalSize(0) {};
        programBuildData(cl_build_status buildStatus, const gtString& buildLog, const gtString& buildOptions, gtUInt64 buildGlobalVariablesTotalSize)
            : _buildStatus(buildStatus), _buildLog(buildLog), _buildOptions(buildOptions), m_buildGlobalVariablesTotalSize(buildGlobalVariablesTotalSize) {};
    };

    // Self functions:
    apCLProgram(oaCLProgramHandle programHandle);
    virtual ~apCLProgram();

    // Source code:
    const osFilePath& sourceCodeFilePath() const { return _programSourceCodeFilePath; };
    void setSourceCodeFilePath(const osFilePath& filePath) { _programSourceCodeFilePath = filePath; };

    // Program kernelHandles:
    const gtVector<oaCLKernelHandle>& kernelHandles() const {return _programKernelHandles;};
    void setProgramKernels(const gtASCIIString& kernelsStr);
    const gtVector<gtString>& kernelNames() const {return _kernelNames;};

    // Program devices:
    const gtVector<int>& devices() const {return _programDevices;};
    void addDevice(int deviceId);
    void clearDevices();
    gtVector<programBuildData>& devicesBuildData() {return _programDevicesBuildData;};
    const gtVector<programBuildData>& devicesBuildData() const {return _programDevicesBuildData;};
    bool wasProgramBuiltSuccessfully() const;

    // Program binary type:
    gtString programBinaryTypeAsString() const ;
    void setProgramBinaryType(cl_program_binary_type binaryType) {_programBinaryType = binaryType;};

    // Program Handle:
    oaCLProgramHandle programHandle() const { return _hProgram; };

    // Handles to related programs:
    void addRelatedProgram(oaCLProgramHandle relatedProgram) {m_relatedPrograms.push_back(relatedProgram);};
    const gtVector<oaCLProgramHandle>& relatedProgramHandles() const { return m_relatedPrograms; };

    // Deletion status:
    bool wasMarkedForDeletion() const {return _wasMarkedForDeletion;};
    void onProgramMarkedForDeletion() {_wasMarkedForDeletion = true;};

    // Reference count:
    int referenceCount() const {return _referenceCount;};
    void setReferenceCount(int rCount) {_referenceCount = rCount;};

    // Devices amount:
    int numDevices() const {return (int)_programDevices.size();};

    // Can the program be debugged?
    bool canDebugProgram(gtString& reason) const {reason = m_notDebuggableReason; return m_isDebuggable;};

    // Can the program be queried for declared kernels? (See CODEXL-2756, SWDEV-90408)
    bool canUpdateProgramKernels() const { return m_canUpdateKernels; };

    // cl_gremedy_object_naming:
    const gtString& programName() const {return _programName;};
    void setProgramName(const gtString& name) {_programName = name;};

    // On event functions:
    void onProgramBuild(bool wasProgramBuiltSuccessful, const gtString& buildLog);
    void onKernelCreation(oaCLKernelHandle kernelHandle);
    void onKernelDeletion(oaCLKernelHandle kernelHandle);
    void setProgramNotDebuggable(const gtString& reason) {m_isDebuggable = false; m_notDebuggableReason = reason;};
    void setCanUpdateProgramKernels(bool canUpdate) { m_canUpdateKernels = canUpdate; };

    // Overrides osTransferableObject:
    virtual osTransferableObjectType type() const;
    virtual bool writeSelfIntoChannel(osChannel& ipcChannel) const;
    virtual bool readSelfFromChannel(osChannel& ipcChannel);

private:
    // Do not allow the use of my default constructor:
    apCLProgram();

private:
    // Handle to the OpenCL program:
    oaCLProgramHandle _hProgram;

    // Handles to related programs:
    gtVector<oaCLProgramHandle> m_relatedPrograms;

    // Was this program marked for deletion?
    bool _wasMarkedForDeletion;

    // The program source code file path:
    osFilePath _programSourceCodeFilePath;

    // Contain a vector of the program's kernelHandles:
    gtVector<oaCLKernelHandle> _programKernelHandles;

    // Contain a vector of the program's devices API Ids:
    gtVector<int> _programDevices;

    // The build information for each device (this vector must be the same size and order
    // as the _programDevices vector):
    gtVector<programBuildData> _programDevicesBuildData;

    // The program reference count:
    int _referenceCount;

    // cl_gremedy_object_naming:
    gtString _programName;

    // Contain the program kernel names:
    gtVector<gtString> _kernelNames;

    // Program binary type:
    cl_program_binary_type _programBinaryType;

    // Can the program kernels be debugged?
    bool m_isDebuggable;
    gtString m_notDebuggableReason;

    // Can the program be queried for declared kernels? (See CODEXL-2756, SWDEV-90408)
    bool m_canUpdateKernels;
};


#endif //__APCLPROGRAM_H
