//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file csProgramsAndKernelsMonitor.h
///
//==================================================================================

//------------------------------ csProgramsAndKernelsMonitor.h ------------------------------

#ifndef __CSPROGRAMSANDKERNELSMONITOR_H
#define __CSPROGRAMSANDKERNELSMONITOR_H

// Forward decelerations:
class osFilePath;

// Infra:
#include <AMDTBaseTools/Include/gtMap.h>
#include <AMDTBaseTools/Include/gtPtrVector.h>
#include <AMDTOSWrappers/Include/osOSDefinitions.h>
#include <AMDTOSAPIWrappers/Include/oaOpenCLIncludes.h>
#include <AMDTAPIClasses/Include/apCLProgram.h>

// Local:
#include <src/csCLKernel.h>


// ----------------------------------------------------------------------------------
// Class Name:           csProgramsAndKernelsMonitor
// General Description:
//  Monitors OpenCL programs and Kernels.
//
// Author:               Yaki Tebeka
// Creation Date:        22/11/2009
// ----------------------------------------------------------------------------------
class csProgramsAndKernelsMonitor
{
public:
    csProgramsAndKernelsMonitor(int spyContextId);
    virtual ~csProgramsAndKernelsMonitor();

    // Update context data snapshot:
    bool updateContextDataSnapshot();

    // Events:
    void onProgramCreation(cl_program programHandle, const gtASCIIString& programSourceCode);
    void onProgramMarkedForDeletion(cl_program program);
    void onProgramBuilt(cl_program programHandle, cl_uint numberOfDevices, const cl_device_id* devicesList, const char* compileOptions, bool programDebuggable, const gtString& programNotDebuggableReason, bool canUpdateProgramKernels);
    void onBeforeProgramBuild(cl_program programHandle, cl_uint numberOfDevices, const cl_device_id* devicesList, const char* compileOptions);
    void onProgramsLinked(cl_program program, cl_uint num_related_programs, const cl_program* related_programs);
    void onKernelCreation(cl_program programHandle, cl_kernel kernelHandle, const gtString& kernelName);
    void onKernelMarkedForDeletion(cl_kernel kernel);
    void onKernelArgumentSet(cl_kernel kernel, cl_uint arg_index, size_t arg_size, const void* arg_value, bool isSVMPointer);

    // Reference count checking:
    void checkForReleasedObjects();

    // Programs:
    int amountOfPrograms() const;
    int amountOfNotDeletedPrograms() const;
    const apCLProgram* programMonitor(int programId) const;
    apCLProgram* programMonitor(int programId);
    const apCLProgram* programMonitor(oaCLProgramHandle programHandle) const;
    apCLProgram* programMonitor(oaCLProgramHandle programHandle);
    int programIndexFromHandle(oaCLProgramHandle programHandle) const;
    oaCLProgramHandle getInternalProgramHandle(oaCLProgramHandle externalProgramHandle) const;
    bool setProgramSourceCode(oaCLProgramHandle externalProgramHandle, const osFilePath& inputFilePath);
    bool buildProgram(oaCLProgramHandle externalProgramHandle, apCLProgram*& pFailedProgramData);

    // Kernels:
    int amountOfKernels() const;
    int amountOfNotDeletedKernelsInProgram(int programId) const;
    const csCLKernel* kernelMonitor(int kernelId) const;
    csCLKernel* kernelMonitor(int kernelId);
    const csCLKernel* kernelMonitor(oaCLKernelHandle kernelHandle) const;
    csCLKernel* kernelMonitor(oaCLKernelHandle kernelHandle);
    int kernelMonitorIndex(oaCLKernelHandle kernelHandle) const;
    oaCLKernelHandle getInternalKernelHandle(oaCLKernelHandle externalKernelHandle) const;

private:
    bool logProgramSourceCodeUpdate(apCLProgram& programObject, int programIndex, const gtASCIIString& newSourceCode);
    void generateProgramSourceCodeFilePath(apCLProgram& programObject, int programIndex, osFilePath& filePath) const;
    bool updateProgramBuildLog(apCLProgram& programDetails, oaCLProgramHandle programInternalHandle, int currentDeviceIndexInProgram);
    bool updateProgramBinaryType(apCLProgram& programDetails, oaCLProgramHandle programInternalHandle, int currentDeviceIndexInProgram);
    bool updateProgramDevices(apCLProgram& programDetails, oaCLProgramHandle programInternalHandle);
    int deviceIndexFromDeviceHandle(const apCLProgram& programDetails, oaCLDeviceID deviceHandle);

    bool relinkKernelAfterProgramBuild(const csCLKernel* pKernel, oaCLProgramHandle newProgramInternalHandle, oaCLKernelHandle& newKernelInternalHandle);
    bool replaceKernelAfterProgramBuild(const csCLKernel* pKernel, oaCLKernelHandle newKernelInternalHandle);

    bool updateProgramsInfo();
    bool updateProgramInfo(apCLProgram* pProgram);
    bool updateProgramInfoWithInternalHandle(apCLProgram& program, oaCLProgramHandle programInternalHandle);
    bool updateKernelsInfo();
    bool updateKernelInfo(csCLKernel* pKernel);
    bool updateKernelArgsInfo(csCLKernel* pKernel);
    bool updateProgramKernels(apCLProgram& programDetails, oaCLProgramHandle programInternalHandle);

    // Do not allow the use of my default constructor:
    csProgramsAndKernelsMonitor();

    // Do not allow use of the = operator for this class. Use reference or pointer transferral instead
    csProgramsAndKernelsMonitor& operator=(const csProgramsAndKernelsMonitor& otherMonitor);
    csProgramsAndKernelsMonitor(const csProgramsAndKernelsMonitor& otherMonitor);

private:
    // The spy id of the OpenCL context who's programs and kernels are monitored:
    int _spyContextId;

    // A vector containing program monitors:
    gtPtrVector<apCLProgram*> _programMonitors;

    // A vector containing kernel monitors:
    gtPtrVector<csCLKernel*> _kernelMonitors;

    // Maps to help us translate the external and internal names of programs and kernels
    // after kernel edit-and-continue:
    gtMap<oaCLHandle, oaCLHandle> _programAndKernelExternalToInternalHandle;
    gtMap<oaCLHandle, oaCLHandle> _programAndKernelInternalToExternalHandle;
};


#endif //__CSPROGRAMSANDKERNELSMONITOR_H

