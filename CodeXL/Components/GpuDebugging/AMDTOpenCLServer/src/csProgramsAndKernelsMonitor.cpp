//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file csProgramsAndKernelsMonitor.cpp
///
//==================================================================================

//------------------------------ csProgramsAndKernelsMonitor.cpp ------------------------------

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTOSWrappers/Include/osApplication.h>
#include <AMDTOSWrappers/Include/osFile.h>
#include <AMDTOSWrappers/Include/osDebugLog.h>
#include <AMDTAPIClasses/Include/Events/apOpenCLProgramBuildEvent.h>
#include <AMDTServerUtilities/Include/suGlobalVariables.h>
#include <AMDTServerUtilities/Include/suIKernelDebuggingManager.h>
#include <AMDTServerUtilities/Include/suInterceptionMacros.h>
#include <AMDTServerUtilities/Include/suSpyAPIFunctions.h>
#include <AMDTServerUtilities/Include/suStringConstants.h>

// Local:
#include <src/csAMDKernelDebuggingFunctionPointers.h>
#include <src/csGlobalVariables.h>
#include <src/csMonitoredFunctionPointers.h>
#include <src/csOpenCLMonitor.h>
#include <src/csProgramsAndKernelsMonitor.h>
#include <Include/csPublicStringConstants.h>
#include <src/csStringConstants.h>


// ---------------------------------------------------------------------------
// Name:        csProgramsAndKernelsMonitor::csProgramsAndKernelsMonitor
// Description: Constructor.
// Arguments: spyContextId - The spy id of the OpenCL context who's programs and kernelHandles are monitored.
// Author:      Yaki Tebeka
// Date:        22/11/2009
// ---------------------------------------------------------------------------
csProgramsAndKernelsMonitor::csProgramsAndKernelsMonitor(int spyContextId): _spyContextId(spyContextId)
{
}


// ---------------------------------------------------------------------------
// Name:        csProgramsAndKernelsMonitor::~csProgramsAndKernelsMonitor
// Description: Destructor.
// Author:      Yaki Tebeka
// Date:        22/11/2009
// ---------------------------------------------------------------------------
csProgramsAndKernelsMonitor::~csProgramsAndKernelsMonitor()
{
}

// ---------------------------------------------------------------------------
// Name:        csProgramsAndKernelsMonitor::updateContextDataSnapshot
// Description: Updates each of the programs and kernel properties
// Return Val: bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        24/11/2009
// ---------------------------------------------------------------------------
bool csProgramsAndKernelsMonitor::updateContextDataSnapshot()
{
    bool retVal = false;

    // Update each of this monitor program objects' info:
    bool rc1 = updateProgramsInfo();
    GT_ASSERT(rc1);

    // Update the kernel objects:
    bool rc2 = updateKernelsInfo();
    GT_ASSERT(rc2);

    retVal = rc1 && rc2;
    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        csProgramsAndKernelsMonitor::onProgramCreation
// Description: Is called when a program is created with source code.
// Arguments: programHandle - The newly created program handle.
//            programSourceCode - The program's source code.
// Author:      Yaki Tebeka
// Date:        22/11/2009
// ---------------------------------------------------------------------------
void csProgramsAndKernelsMonitor::onProgramCreation(cl_program programHandle, const gtASCIIString& programSourceCode)
{
    // Create a program monitor that will represent the created OpenCL program:
    apCLProgram* pProgramMonitor = new apCLProgram((oaCLProgramHandle)programHandle);

    // Register this object:
    su_stat_theAllocatedObjectsMonitor.registerAllocatedObject(*pProgramMonitor);

    // Log the existence of this program:
    _programMonitors.push_back(pProgramMonitor);

    // Map this program to itself for internal and external use:
    _programAndKernelExternalToInternalHandle[(oaCLHandle)programHandle] = (oaCLHandle)programHandle;
    _programAndKernelInternalToExternalHandle[(oaCLHandle)programHandle] = (oaCLHandle)programHandle;

    // Get the handles monitor:
    csOpenCLHandleMonitor& handlesMonitor = cs_stat_openCLMonitorInstance.openCLHandleMonitor();
    int objectIndex = (int)_programMonitors.size() - 1;
    handlesMonitor.registerOpenCLHandle((oaCLHandle)programHandle, _spyContextId, objectIndex, OS_TOBJ_ID_CL_PROGRAM, -1, objectIndex + 1);

    // If this program was created with source (and not with binary):
    if (!programSourceCode.isEmpty())
    {
        // Set the program's source code:
        int programIndex = (int)_programMonitors.size() - 1;
        bool rc = logProgramSourceCodeUpdate(*pProgramMonitor, programIndex, programSourceCode);
        GT_ASSERT(rc);
    }
}

// ---------------------------------------------------------------------------
// Name:        csProgramsAndKernelsMonitor::onProgramMarkedForDeletion
// Description: Called when the program is marked for deletion
//              (calling clReleaseProgram with the reference count at 1)
// Author:      Uri Shomroni
// Date:        31/3/2010
// ---------------------------------------------------------------------------
void csProgramsAndKernelsMonitor::onProgramMarkedForDeletion(cl_program program)
{
    // Get the program monitor:
    apCLProgram* pProgram = programMonitor((oaCLProgramHandle)program);
    GT_IF_WITH_ASSERT(pProgram != NULL)
    {
        // Log the program handle deletion:
        csOpenCLMonitor& theOpenCLMonitor = csOpenCLMonitor::instance();
        csOpenCLHandleMonitor& handlesMonitor = theOpenCLMonitor.openCLHandleMonitor();
        handlesMonitor.registerOpenCLHandle((oaCLHandle)pProgram->programHandle(), _spyContextId, -1, OS_TOBJ_ID_CL_PROGRAM);

        // Mark it as deleted:
        pProgram->onProgramMarkedForDeletion();
    }
}

// ---------------------------------------------------------------------------
// Name:        csProgramsAndKernelsMonitor::onBeforeProgramBuild
// Description: Event called before an OpenCL program is about to be built
// Arguments:   cl_program programHandle - the built program cl handle
//              cl_uint numberOfDevices - amount of devices
//              const cl_device_id* devicesList - the devices list
//              const char* compileOptions - compile options
// Author:      Sigal Algranaty
// Date:        3/7/2011
// ---------------------------------------------------------------------------
void csProgramsAndKernelsMonitor::onBeforeProgramBuild(cl_program programHandle, cl_uint numberOfDevices, const cl_device_id* devicesList, const char* compileOptions)
{
    (void)(numberOfDevices); // unused
    (void)(devicesList); // unused
    (void)(compileOptions); // unused
    // Get the program object monitor:
    apCLProgram* pProgramObject = programMonitor((oaCLProgramHandle)programHandle);
    GT_IF_WITH_ASSERT(pProgramObject != NULL)
    {
        // Notify the debugger about the program build:
        // Get the program index:
        int programIndex = programIndexFromHandle((oaCLProgramHandle)programHandle);

        // Define a build program event:
        apOpenCLProgramBuildEvent buildProgramEvent(_spyContextId, programIndex, pProgramObject->programName(), pProgramObject->devicesBuildData(), false);

        // Send an event with the build log details:
        bool rcEve = suForwardEventToClient(buildProgramEvent);
        GT_ASSERT(rcEve);
    }
}


// ---------------------------------------------------------------------------
// Name:        csProgramsAndKernelsMonitor::onProgramsLinked
// Description: Called when a program is linked with other programs by:
//              clLinkProgram - input programs
//              clCompileProgram - include headers
//              Marks the programs as related to the "main" program.
// Author:      Uri Shomroni
// Date:        26/8/2013
// ---------------------------------------------------------------------------
void csProgramsAndKernelsMonitor::onProgramsLinked(cl_program program, cl_uint num_related_programs, const cl_program* related_programs)
{
    if (0 < num_related_programs)
    {
        GT_IF_WITH_ASSERT(NULL != related_programs)
        {
            apCLProgram* pProgramObject = programMonitor((oaCLProgramHandle)program);

            for (cl_uint i = 0; i < num_related_programs; i++)
            {
                pProgramObject->addRelatedProgram((oaCLProgramHandle)related_programs[i]);
            }
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        csProgramsAndKernelsMonitor::onProgramBuilt
// Description: Event called after an OpenCL program is built
// Arguments:   cl_program programHandle - the built program cl handle
//              cl_uint numberOfDevices - amount of devices
//              const cl_device_id* devicesList - the devices list
//              const char* compileOptions - compile options
// Author:      Sigal Algranaty
// Date:        3/7/2011
// ---------------------------------------------------------------------------
void csProgramsAndKernelsMonitor::onProgramBuilt(cl_program programHandle, cl_uint numberOfDevices, const cl_device_id* devicesList, const char* compileOptions, bool programDebuggable, const gtString& programNotDebuggableReason, bool canUpdateProgramKernels)
{
    (void)(compileOptions); // unused
    // Get the program object monitor:
    apCLProgram* pProgramObject = programMonitor((oaCLProgramHandle)programHandle);
    GT_IF_WITH_ASSERT(pProgramObject != NULL)
    {
        // Update the program devices:
        bool rc = updateProgramDevices(*pProgramObject, (oaCLProgramHandle)programHandle);
        GT_ASSERT(rc);

        // Set the kernel update flag:
        pProgramObject->setCanUpdateProgramKernels(canUpdateProgramKernels);

        // Update the kernel device:
        rc = updateProgramKernels(*pProgramObject, (oaCLProgramHandle)programHandle);
        GT_ASSERT(rc);

        // Look for the program internal handle:
        gtMap<oaCLHandle, oaCLHandle>::const_iterator findIter = _programAndKernelExternalToInternalHandle.find((oaCLProgramHandle)programHandle);
        gtMap<oaCLHandle, oaCLHandle>::const_iterator endIter = _programAndKernelExternalToInternalHandle.end();
        GT_IF_WITH_ASSERT(findIter != endIter)
        {
            // Iterate each of the input devices, and update the build log for each of them:
            for (int i = 0; i < (int)numberOfDevices; i++)
            {
                // Get the current device id and get the index within the program object:
                cl_device_id currentInputDeviceHandle = devicesList[i];

                // Get the device index from the device handle:
                int deviceIndex = deviceIndexFromDeviceHandle(*pProgramObject, (oaCLDeviceID)currentInputDeviceHandle);
                GT_IF_WITH_ASSERT(deviceIndex >= 0)
                {
                    // Update the build log from the OpenCL API:
                    bool rc1 = updateProgramBuildLog(*pProgramObject, (oaCLProgramHandle)programHandle, 0);
                    GT_IF_WITH_ASSERT(rc1)
                    {
                        // Notify the debugger about the program build:
                        // Get the program index:
                        int programIndex = programIndexFromHandle((oaCLProgramHandle)programHandle);

                        // Define a build program event:
                        apOpenCLProgramBuildEvent buildProgramEvent(_spyContextId, programIndex, pProgramObject->programName(), pProgramObject->devicesBuildData(), true);

                        // Send an event with the build log details:
                        bool rcEve = suForwardEventToClient(buildProgramEvent);
                        GT_ASSERT(rcEve);
                    }
                }
            }
        }

        // If this program is not debuggable, mark down the reason:
        if (!programDebuggable)
        {
            pProgramObject->setProgramNotDebuggable(programNotDebuggableReason);
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        csProgramsAndKernelsMonitor::deviceIndexFromDeviceHandle
// Description: Get device index from device handle for the input program
// Arguments:   const apCLProgram& programDetails
//              oaCLDeviceID deviceHandle
// Return Val:  int
// Author:      Sigal Algranaty
// Date:        3/7/2011
// ---------------------------------------------------------------------------
int csProgramsAndKernelsMonitor::deviceIndexFromDeviceHandle(const apCLProgram& programDetails, oaCLDeviceID deviceHandle)
{
    int retVal = -1;

    // Get the program devices:
    const gtVector<int> programDevices = programDetails.devices();

    // Get the devices monitor:
    csDevicesMonitor& theDevicesMonitor = csOpenCLMonitor::instance().devicesMonitor();
    int amountOfDevices = theDevicesMonitor.amountOfDevices();

    for (int deviceIndex = 0; deviceIndex < amountOfDevices; deviceIndex++)
    {
        // Get the requested device handle:
        const apCLDevice* pDevice = theDevicesMonitor.getDeviceObjectDetailsByIndex(deviceIndex);
        GT_IF_WITH_ASSERT(pDevice != NULL)
        {
            oaCLDeviceID curDeviceHandle = (oaCLDeviceID)(pDevice->deviceHandle());

            if (curDeviceHandle == deviceHandle)
            {
                retVal = deviceIndex;
                break;
            }
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        csProgramsAndKernelsMonitor::onKernelCreation
// Description: Is called when a kernel is created.
// Arguments: program - The OpenCL handle for the program containing the kernel.
//            kernel - The OpenCL handle for the kernel object.
//            kernelName - The name of the program's function that this kernel executed.
// Author:      Yaki Tebeka
// Date:        22/11/2009
// ---------------------------------------------------------------------------
void csProgramsAndKernelsMonitor::onKernelCreation(cl_program programHandle, cl_kernel kernelHandle, const gtString& kernelName)
{
    // Get the program index:
    int programIndex = programIndexFromHandle((oaCLProgramHandle)programHandle);

    // Log the created kernel object:
    csCLKernel* pNewKernel = new csCLKernel((oaCLProgramHandle)programHandle, programIndex, (oaCLKernelHandle)kernelHandle, kernelName);
    _kernelMonitors.push_back(pNewKernel);

    // Register this object in the allocated objects monitor:
    su_stat_theAllocatedObjectsMonitor.registerAllocatedObject(*pNewKernel);

    // Map the kernel name as the same for internal and external use:
    _programAndKernelExternalToInternalHandle[(oaCLHandle)kernelHandle] = (oaCLHandle)kernelHandle;
    _programAndKernelInternalToExternalHandle[(oaCLHandle)kernelHandle] = (oaCLHandle)kernelHandle;

    // Get the program object containing the kernel:
    apCLProgram* pProgramMonitor = programMonitor((oaCLProgramHandle)programHandle);
    GT_IF_WITH_ASSERT(pProgramMonitor != NULL)
    {
        // Mark that this is the kernel's program:
        pProgramMonitor->onKernelCreation((oaCLKernelHandle)kernelHandle);
    }

    // Get the handles monitor:
    csOpenCLHandleMonitor& handlesMonitor = cs_stat_openCLMonitorInstance.openCLHandleMonitor();

    // The object id is the kernel object within the program:
    int objectIndex = (int)pProgramMonitor->kernelHandles().size();
    int ownerObjectIndex = -1;

    // Get the program index:
    for (int i = 0; i < (int)_programMonitors.size(); i++)
    {
        if (_programMonitors[i] == pProgramMonitor)
        {
            ownerObjectIndex = i;
            break;
        }
    }

    // Register the kernel object:
    handlesMonitor.registerOpenCLHandle((oaCLHandle)kernelHandle, _spyContextId, objectIndex, OS_TOBJ_ID_CL_KERNEL, ownerObjectIndex, objectIndex);
}

// ---------------------------------------------------------------------------
// Name:        csProgramsAndKernelsMonitor::onKernelMarkedForDeletion
// Description: Called when the kernel is marked for deletion
//              (calling clReleaseKernel with the reference count at 1)
// Author:      Uri Shomroni
// Date:        31/3/2010
// ---------------------------------------------------------------------------
void csProgramsAndKernelsMonitor::onKernelMarkedForDeletion(cl_kernel kernel)
{
    // Get the kernel monitor index:
    oaCLKernelHandle kernelHandle = (oaCLKernelHandle)kernel;
    int kernelIndex = kernelMonitorIndex(kernelHandle);
    GT_IF_WITH_ASSERT(kernelIndex >= 0)
    {
        // Update the program containing this kernel of the deletion:
        csCLKernel* pKernel = kernelMonitor(kernelHandle);
        GT_IF_WITH_ASSERT(pKernel != NULL)
        {
            // Log the program handle deletion:
            csOpenCLMonitor& theOpenCLMonitor = csOpenCLMonitor::instance();
            csOpenCLHandleMonitor& handlesMonitor = theOpenCLMonitor.openCLHandleMonitor();
            handlesMonitor.registerOpenCLHandle((oaCLHandle)pKernel->kernelHandle(), _spyContextId, -1, OS_TOBJ_ID_CL_KERNEL);

            // Get the program:
            oaCLProgramHandle programHandle = pKernel->programHandle();
            apCLProgram* pProgram = programMonitor(programHandle);

            // Do not assert, the program might have been deleted by the beforeKernelMarkedForDeletion() function:
            if (pProgram != NULL)
            {
                // Remove this kernel from its kernelHandles list:
                pProgram->onKernelDeletion(kernelHandle);
            }

            // Delete the kernel object:
            _kernelMonitors.removeItem(kernelIndex);
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        csProgramsAndKernelsMonitor::onKernelArgumentSet
// Description: Called when kernel's arg_index-th argument is set
// Author:      Uri Shomroni
// Date:        18/1/2010
// ---------------------------------------------------------------------------
void csProgramsAndKernelsMonitor::onKernelArgumentSet(cl_kernel kernel, cl_uint arg_index, size_t arg_size, const void* arg_value, bool isSVMPointer)
{
    // Get the kernel object:
    csCLKernel* pKernel = kernelMonitor((oaCLKernelHandle)kernel);
    GT_IF_WITH_ASSERT(pKernel != NULL)
    {
        pKernel->onArgumentSet(arg_index, arg_size, arg_value, isSVMPointer);
    }
}

// ---------------------------------------------------------------------------
// Name:        csProgramsAndKernelsMonitor::checkForReleasedObjects
// Description: Checks if any of the objects monitored by this class have been released
// Author:      Uri Shomroni
// Date:        26/8/2013
// ---------------------------------------------------------------------------
void csProgramsAndKernelsMonitor::checkForReleasedObjects()
{
    // Kernels first, then programs, since released kernels might cause the program list to change:

    // Collect the object handles:
    gtVector<oaCLKernelHandle> kernelHandles;
    int numberOfKernels = (int)_kernelMonitors.size();

    for (int i = 0; i < numberOfKernels; i++)
    {
        const apCLKernel* pKernel = _kernelMonitors[i];
        GT_IF_WITH_ASSERT(NULL != pKernel)
        {
            kernelHandles.push_back(pKernel->kernelHandle());
        }
    }

    // Check each one. This is done separately, since finding an object
    // that was marked for deletion will release it:
    csOpenCLMonitor& theOpenCLMonitor = csOpenCLMonitor::instance();
    int kernelsFound = (int)kernelHandles.size();

    for (int i = 0; i < kernelsFound; i++)
    {
        theOpenCLMonitor.checkIfKernelWasDeleted((cl_kernel)kernelHandles[i], false);
    }

    // Collect the object handles:
    gtVector<oaCLProgramHandle> programHandles;
    int numberOfPrograms = (int)_programMonitors.size();

    for (int i = 0; i < numberOfPrograms; i++)
    {
        const apCLProgram* pProgram = _programMonitors[i];
        GT_IF_WITH_ASSERT(NULL != pProgram)
        {
            if (!pProgram->wasMarkedForDeletion())
            {
                programHandles.push_back(pProgram->programHandle());
            }
        }
    }

    // Check each one. This is done separately, since finding an object
    // that was marked for deletion will release it:
    int programsFound = (int)programHandles.size();

    for (int i = 0; i < programsFound; i++)
    {
        theOpenCLMonitor.checkIfProgramWasDeleted((cl_program)programHandles[i], false);
    }
}

// ---------------------------------------------------------------------------
// Name:        csProgramsAndKernelsMonitor::amountOfPrograms
// Description: Returns the amount of OpenCL programs associated with the
//              OpenCL context monitored by this class.
// Author:      Yaki Tebeka
// Date:        16/11/2009
// ---------------------------------------------------------------------------
int csProgramsAndKernelsMonitor::amountOfPrograms() const
{
    int retVal = (int)_programMonitors.size();
    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        csProgramsAndKernelsMonitor::amountOfNotDeletedPrograms
// Description: Returns the amount of OpenCL programs that was not marked for
//              deletion for the OpenCL context monitored by this class.
// Return Val:  int
// Author:      Sigal Algranaty
// Date:        18/7/2010
// ---------------------------------------------------------------------------
int csProgramsAndKernelsMonitor::amountOfNotDeletedPrograms() const
{
    int retVal = 0;

    for (int i = 0 ; i < (int)_programMonitors.size(); i++)
    {
        // Get the current program:
        apCLProgram* pCurrentProgram = _programMonitors[i];

        if (pCurrentProgram != NULL)
        {
            if (!pCurrentProgram->wasMarkedForDeletion())
            {
                retVal ++;
            }
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        csProgramsAndKernelsMonitor::programMonitor
// Description: Inputs a program index and return it's monitor.
// Author:      Yaki Tebeka
// Date:        16/11/2009
// ---------------------------------------------------------------------------
const apCLProgram* csProgramsAndKernelsMonitor::programMonitor(int programId) const
{
    const apCLProgram* retVal = NULL;

    // Index range check:
    int programsAmount = (int)_programMonitors.size();
    GT_IF_WITH_ASSERT((0 <= programId) && (programId < programsAmount))
    {
        retVal = _programMonitors[programId];
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        csProgramsAndKernelsMonitor::programMonitor
// Description: Inputs a program index and return it's monitor.
// Author:      Yaki Tebeka
// Date:        16/11/2009
// ---------------------------------------------------------------------------
apCLProgram* csProgramsAndKernelsMonitor::programMonitor(int programId)
{
    apCLProgram* retVal = NULL;

    // Index range check:
    int programsAmount = (int)_programMonitors.size();
    GT_IF_WITH_ASSERT((0 <= programId) && (programId < programsAmount))
    {
        retVal = _programMonitors[programId];
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        csProgramsAndKernelsMonitor::programMonitor
// Description:
//   Inputs a program's OpenCL handle and returns the program's monitor, or NULL if
//   such program was not recorded by our OpenGL server.
// Author:      Yaki Tebeka
// Date:        22/11/2009
// ---------------------------------------------------------------------------
const apCLProgram* csProgramsAndKernelsMonitor::programMonitor(oaCLProgramHandle programHandle) const
{
    const apCLProgram* retVal = NULL;

    // Iterate the existing program monitors:
    gtPtrVector<apCLProgram*>::const_iterator iter = _programMonitors.begin();
    gtPtrVector<apCLProgram*>::const_iterator endIter = _programMonitors.end();

    while (iter != endIter)
    {
        // Get the current program monitor:
        const apCLProgram* pCurrentMonitor = *iter;
        GT_IF_WITH_ASSERT(pCurrentMonitor != NULL)
        {
            // If the current program is the program we are looking for:
            oaCLProgramHandle currProgramHandle = pCurrentMonitor->programHandle();

            if (programHandle == currProgramHandle)
            {
                retVal = pCurrentMonitor;

                // If we found a deleted program, keep looking - there might be a living one which
                // reuses the handle:
                if (!pCurrentMonitor->wasMarkedForDeletion())
                {
                    break;
                }
            }
        }

        iter++;
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        csProgramsAndKernelsMonitor::programMonitor
// Description:
//   Inputs a program's OpenCL handle and returns the program's monitor, or NULL if
//   such program was not recorded by our OpenGL server.
// Author:      Yaki Tebeka
// Date:        22/11/2009
// ---------------------------------------------------------------------------
apCLProgram* csProgramsAndKernelsMonitor::programMonitor(oaCLProgramHandle programHandle)
{
    apCLProgram* retVal = NULL;

    // Iterate the existing program monitors:
    gtPtrVector<apCLProgram*>::const_iterator iter = _programMonitors.begin();
    gtPtrVector<apCLProgram*>::const_iterator endIter = _programMonitors.end();

    while (iter != endIter)
    {
        // Get the current program monitor:
        apCLProgram* pCurrentMonitor = *iter;
        GT_IF_WITH_ASSERT(pCurrentMonitor != NULL)
        {
            // If the current program is the program we are looking for:
            oaCLProgramHandle currProgramHandle = pCurrentMonitor->programHandle();

            if (programHandle == currProgramHandle)
            {
                retVal = pCurrentMonitor;

                // If we found a deleted program, keep looking - there might be a living one which
                // reuses the handle:
                if (!pCurrentMonitor->wasMarkedForDeletion())
                {
                    break;
                }
            }
        }

        iter++;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        csProgramsAndKernelsMonitor::programIndexFromHandle
// Description: Returns a program's index in this class's vector
// Author:      Uri Shomroni
// Date:        29/7/2010
// ---------------------------------------------------------------------------
int csProgramsAndKernelsMonitor::programIndexFromHandle(oaCLProgramHandle programHandle) const
{
    int retVal = -1;

    // Iterate the existing program monitors:
    int numberOfPrograms = (int)_programMonitors.size();

    for (int i = 0; i < numberOfPrograms; i++)
    {
        // Get the current program monitor:
        const apCLProgram* pCurrentMonitor = _programMonitors[i];
        GT_IF_WITH_ASSERT(pCurrentMonitor != NULL)
        {
            // If the current program is the program we are looking for:
            oaCLProgramHandle currProgramHandle = pCurrentMonitor->programHandle();

            if (programHandle == currProgramHandle)
            {
                // Return its index:
                retVal = i;
                break;
            }
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        csProgramsAndKernelsMonitor::getInternalProgramHandle
// Description: If the program source is forced, return the internal handle.
//              otherwise, return the external handle
// Author:      Uri Shomroni
// Date:        17/1/2010
// ---------------------------------------------------------------------------
oaCLProgramHandle csProgramsAndKernelsMonitor::getInternalProgramHandle(oaCLProgramHandle externalProgramHandle) const
{
    oaCLProgramHandle retVal = externalProgramHandle;

    gtMap<oaCLHandle, oaCLHandle>::const_iterator findIter = _programAndKernelExternalToInternalHandle.find(externalProgramHandle);
    gtMap<oaCLHandle, oaCLHandle>::const_iterator endIter = _programAndKernelExternalToInternalHandle.end();

    // The handle will appear in the map iff the code is forced:
    if (findIter != endIter)
    {
        // Set the real value:
        retVal = (oaCLProgramHandle)((*findIter).second);
    }
    else
    {
        // This program was created by a function not monitored by us, register it in the maps:
        csProgramsAndKernelsMonitor* pNonConstMe = (csProgramsAndKernelsMonitor*)this;
        cl_program handleAsCLProgram = (cl_program)externalProgramHandle;

        // Get the program source:
        SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_clGetProgramInfo);
        gtASCIIString programSource;
        gtSizeType codeLength = 0;
        cl_int retCode = cs_stat_realFunctionPointers.clGetProgramInfo(handleAsCLProgram, CL_PROGRAM_SOURCE, 0, NULL, &codeLength);

        // Do not assert here as this might be a program created with binaries and not sources:
        if ((retCode == CL_SUCCESS) && (codeLength > 0))
        {
            // Get the source code:
            char* pSourceCode = new char[codeLength + 1];
            retCode = cs_stat_realFunctionPointers.clGetProgramInfo(handleAsCLProgram, CL_PROGRAM_SOURCE, codeLength + 1, pSourceCode, NULL);
            pSourceCode[codeLength] = '\0';

            if (retCode == CL_SUCCESS)
            {
                programSource = pSourceCode;
            }

            delete[] pSourceCode;
        }

        SU_AFTER_EXECUTING_REAL_FUNCTION(ap_clGetProgramInfo);

        // Register this program in the vector and maps:
        pNonConstMe->onProgramCreation(handleAsCLProgram, programSource);
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        csProgramsAndKernelsMonitor::setProgramSourceCode
// Description: Sets the program's source code from a file
// Return Val:  bool - Success / failure.
// Author:      Uri Shomroni
// Date:        18/1/2010
// ---------------------------------------------------------------------------
bool csProgramsAndKernelsMonitor::setProgramSourceCode(oaCLProgramHandle externalProgramHandle, const osFilePath& inputFilePath)
{
    bool retVal = false;

    // Just note the new path, we will replace the program object only when building:
    apCLProgram* pProgram = programMonitor(externalProgramHandle);
    GT_IF_WITH_ASSERT(pProgram != NULL)
    {
        retVal = true;
        pProgram->setSourceCodeFilePath(inputFilePath);
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        csProgramsAndKernelsMonitor::buildProgram
// Description: Builds a problem and reconnects it and all its kernelHandles for use
//              with the external handles.
// Arguments:   pFailedProgramData - will get failure data if the build fails
//              and we know why. It is the caller's responsibility to release
//              this if allocated.
// Return Val:  bool - Success / failure.
// Author:      Uri Shomroni
// Date:        18/1/2010
// ---------------------------------------------------------------------------
bool csProgramsAndKernelsMonitor::buildProgram(oaCLProgramHandle externalProgramHandle, apCLProgram*& pFailedProgramData)
{
    bool retVal = false;

    // Get the program object:
    const apCLProgram* pProgram = programMonitor(externalProgramHandle);
    GT_IF_WITH_ASSERT(pProgram != NULL)
    {
        // Get the new source code:
        const osFilePath& newSourceCodeFilePath = pProgram->sourceCodeFilePath();
        osFile newSourceCodeFile;
        bool rc = newSourceCodeFile.open(newSourceCodeFilePath, osChannel::OS_ASCII_TEXT_CHANNEL, osFile::OS_OPEN_TO_READ);
        GT_IF_WITH_ASSERT(rc)
        {
            // Load the new source code into a string:
            gtString newSourceCode;
            rc = newSourceCodeFile.readIntoString(newSourceCode);
            newSourceCodeFile.close();
            GT_IF_WITH_ASSERT(rc)
            {
                // Get the old internal handle:
                gtMap<oaCLHandle, oaCLHandle>::iterator findExToInIter = _programAndKernelExternalToInternalHandle.find(externalProgramHandle);
                gtMap<oaCLHandle, oaCLHandle>::iterator endExToInIter = _programAndKernelExternalToInternalHandle.end();
                GT_IF_WITH_ASSERT(findExToInIter != endExToInIter)
                {
                    oaCLProgramHandle oldInternalHandle = (*findExToInIter).second;

                    // Get the old program's context, so we could create the new program under it:
                    cl_context programContext = NULL;
                    SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_clGetProgramInfo);
                    cl_int rcCtx = cs_stat_realFunctionPointers.clGetProgramInfo((cl_program)oldInternalHandle, CL_PROGRAM_CONTEXT, sizeof(cl_context), &programContext, NULL);
                    SU_AFTER_EXECUTING_REAL_FUNCTION(ap_clGetProgramInfo);

                    GT_IF_WITH_ASSERT((rcCtx == CL_SUCCESS) && (programContext != NULL))
                    {
                        // Create a new program from the source code:
                        const char* newSourceAsCharArray = newSourceCode.asASCIICharArray();
                        gtSizeType codeLength = newSourceCode.length();
                        cl_int errCode = CL_SUCCESS;
                        SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_clCreateProgramWithSource);
                        oaCLProgramHandle newInternalHandle = (oaCLProgramHandle)cs_stat_realFunctionPointers.clCreateProgramWithSource(programContext, 1, &newSourceAsCharArray, &codeLength, &errCode);
                        SU_AFTER_EXECUTING_REAL_FUNCTION(ap_clCreateProgramWithSource);

                        GT_IF_WITH_ASSERT(newInternalHandle != OA_CL_NULL_HANDLE)
                        {
                            retVal = true;

                            // Update the old program data:
                            bool rcUpd = updateProgramInfo((apCLProgram*)pProgram);
                            GT_ASSERT(rcUpd);

                            // For each binary that was attempted to be built on the old program, attempt to build
                            // on the same device:
                            const gtVector<int>& programDevices = pProgram->devices();
                            const gtVector<apCLProgram::programBuildData>& programDevicesBuildData = pProgram->devicesBuildData();
                            int amountOfDevicesData = (int)programDevicesBuildData.size();
                            int amountOfDevices = (int)programDevices.size();

                            if (amountOfDevices < amountOfDevicesData)
                            {
                                // This should never happen, but we check to avoid access violations:
                                GT_ASSERT(amountOfDevices == amountOfDevicesData);
                                amountOfDevicesData = amountOfDevices;
                            }

                            // Loop the devices to know which ones to build on:
                            SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_clBuildProgram);
                            csDevicesMonitor& theDevicesMonitor = csOpenCLMonitor::instance().devicesMonitor();

                            for (int i = 0; i < amountOfDevicesData; i++)
                            {
                                // If we or the user attempted to build this before:
                                const apCLProgram::programBuildData& currentBuildData = programDevicesBuildData[i];

                                if (currentBuildData._buildStatus != CL_BUILD_NONE)
                                {
                                    // Get the current device wrapper:
                                    int currDeviceID = programDevices[i];
                                    const apCLDevice* pDevice = theDevicesMonitor.getDeviceObjectDetailsByIndex(currDeviceID);
                                    GT_IF_WITH_ASSERT(pDevice != NULL)
                                    {
                                        // Try building:
                                        cl_device_id currentDevice = (cl_device_id)pDevice->deviceHandle();
                                        cl_int rcBuild = cs_stat_realFunctionPointers.clBuildProgram((cl_program)newInternalHandle, 1, &currentDevice, currentBuildData._buildOptions.asASCIICharArray(), NULL, NULL);
                                        retVal = retVal && (rcBuild == CL_SUCCESS);
                                    }
                                }
                            }

                            SU_AFTER_EXECUTING_REAL_FUNCTION(ap_clBuildProgram);

                            // Iterate the program's kernelHandles:
                            const gtVector<oaCLKernelHandle>& programKernels = pProgram->kernelHandles();
                            int numberOfKernels = (int)programKernels.size();
                            gtVector<oaCLKernelHandle> kernelsNewInternalHandles;

                            for (int i = 0; i < numberOfKernels; i++)
                            {
                                // Create a new kernel for each one in the old program:
                                const csCLKernel* pKernel = kernelMonitor(programKernels[i]);
                                oaCLKernelHandle kernelNewInternalHandle = OA_CL_NULL_HANDLE;

                                bool rcKern = relinkKernelAfterProgramBuild(pKernel, newInternalHandle, kernelNewInternalHandle);
                                GT_ASSERT(rcKern);
                                retVal = retVal && rcKern;

                                kernelsNewInternalHandles.push_back(kernelNewInternalHandle);
                            }

                            // Make sure we got the right amount of kernel handles:
                            retVal = retVal && (numberOfKernels == (int)kernelsNewInternalHandles.size());

                            if (retVal)
                            {
                                // Iterate the program's kernelHandles:
                                for (int i = 0; i < numberOfKernels; i++)
                                {
                                    // Map the old kernelHandles to the new ones:
                                    const csCLKernel* pKernel = kernelMonitor(programKernels[i]);

                                    bool rcKern = replaceKernelAfterProgramBuild(pKernel, kernelsNewInternalHandles[i]);
                                    GT_ASSERT(rcKern);
                                }

                                // Make the new program's reference count equal to the old one's, and release the old one's:
                                int refCount = pProgram->referenceCount();
                                SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_clRetainProgram);
                                SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_clReleaseProgram);

                                // The new program already has one reference from being created:
                                for (int i = 1; i < refCount; i++)
                                {
                                    cl_int rcRelease = cs_stat_realFunctionPointers.clRetainProgram((cl_program)newInternalHandle);
                                    GT_ASSERT(rcRelease == CL_SUCCESS);
                                    rcRelease = cs_stat_realFunctionPointers.clReleaseProgram((cl_program)oldInternalHandle);
                                    GT_ASSERT(rcRelease == CL_SUCCESS);
                                }

                                // Release the old program (it has an extra reference count from being created:
                                cl_int rcRelease2 = cs_stat_realFunctionPointers.clReleaseProgram((cl_program)oldInternalHandle);
                                GT_ASSERT(rcRelease2 == CL_SUCCESS);
                                SU_AFTER_EXECUTING_REAL_FUNCTION(ap_clRetainProgram);
                                SU_AFTER_EXECUTING_REAL_FUNCTION(ap_clReleaseProgram);

                                // Re-map the program in our name maps:
                                _programAndKernelExternalToInternalHandle[externalProgramHandle] = newInternalHandle;
                                _programAndKernelInternalToExternalHandle[newInternalHandle] = externalProgramHandle;
                                _programAndKernelInternalToExternalHandle[oldInternalHandle] = OA_CL_NULL_HANDLE;
                            }
                            else // ! retVal
                            {
                                // Get the reason why we failed and the logs for it:
                                pFailedProgramData = new apCLProgram(newInternalHandle);
                                bool rcFail = updateProgramInfoWithInternalHandle(*pFailedProgramData, newInternalHandle);

                                if (!rcFail)
                                {
                                    GT_ASSERT(rcFail);
                                    delete pFailedProgramData;
                                    pFailedProgramData = NULL;
                                }

                                // Release the new program and all its kernelHandles
                                SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_clReleaseProgram);
                                cl_int rcRelease = cs_stat_realFunctionPointers.clReleaseProgram((cl_program)newInternalHandle);
                                SU_AFTER_EXECUTING_REAL_FUNCTION(ap_clReleaseProgram);
                                GT_ASSERT(rcRelease == CL_SUCCESS);

                                // Iterate the kernelHandles we created:
                                SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_clReleaseKernel);
                                numberOfKernels = (int)kernelsNewInternalHandles.size();
                                bool isCSKernelDebuggingOn = cs_stat_pIKernelDebuggingManager->isAMDKernelDebuggingEnabled() && (suIKernelDebuggingManager::CS_OPENCL_SOFTWARE_KERNEL_DEBUGGER == cs_stat_pIKernelDebuggingManager->kernelDebuggerType());

                                for (int i = 0; i < numberOfKernels; i++)
                                {
                                    // Release them:
                                    if (isCSKernelDebuggingOn)
                                    {
                                        rcRelease = cs_stat_amdKernelDebuggingFunctionPointers.amdclInterceptReleaseKernel((cl_kernel)kernelsNewInternalHandles[i]);
                                        GT_ASSERT(rcRelease == CL_SUCCESS);
                                    }
                                    else // !isKernelDebuggingOn
                                    {
                                        rcRelease = cs_stat_realFunctionPointers.clReleaseKernel((cl_kernel)kernelsNewInternalHandles[i]);
                                        GT_ASSERT(rcRelease == CL_SUCCESS);
                                    }
                                }

                                SU_AFTER_EXECUTING_REAL_FUNCTION(ap_clReleaseKernel);
                            }
                        }
                    }
                }
            }
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        csProgramsAndKernelsMonitor::amountOfKernels
// Description: Returns the amount of kernelHandles created in the OpenCL context this class logs.
// Author:      Yaki Tebeka
// Date:        22/11/2009
// ---------------------------------------------------------------------------
int csProgramsAndKernelsMonitor::amountOfKernels() const
{
    int retVal = (int)_kernelMonitors.size();
    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        csProgramsAndKernelsMonitor::amountOfNotDeletedKernelsInProgram
// Description: Returns the amount of Kernels in the specified program that were
//              not yet marked for deletion.
// Author:      Uri Shomroni
// Date:        29/7/2010
// ---------------------------------------------------------------------------
int csProgramsAndKernelsMonitor::amountOfNotDeletedKernelsInProgram(int programId) const
{
    int retVal = 0;

    // Get the program object:
    const apCLProgram* pProgram = programMonitor(programId);
    GT_IF_WITH_ASSERT(pProgram != NULL)
    {
        // We remove kernelHandles from the program upon deletion, so no need to check
        // any more than the number noted in the program object:
        retVal = (int)pProgram->kernelHandles().size();
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        csProgramsAndKernelsMonitor::kernelMonitor
// Description: Inputs a kernel index and return it's monitor.
// Author:      Yaki Tebeka
// Date:        22/11/2009
// ---------------------------------------------------------------------------
const csCLKernel* csProgramsAndKernelsMonitor::kernelMonitor(int kernelId) const
{
    const csCLKernel* retVal = NULL;

    // Index range check:
    int kernelsAmount = (int)_kernelMonitors.size();
    GT_IF_WITH_ASSERT((0 <= kernelId) && (kernelId < kernelsAmount))
    {
        retVal = _kernelMonitors[kernelId];
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        csProgramsAndKernelsMonitor::kernelMonitor
// Description: Inputs a kernel index and return it's monitor.
// Author:      Yaki Tebeka
// Date:        22/11/2009
// ---------------------------------------------------------------------------
csCLKernel* csProgramsAndKernelsMonitor::kernelMonitor(int kernelId)
{
    csCLKernel* retVal = NULL;

    // Index range check:
    int kernelsAmount = (int)_kernelMonitors.size();
    GT_IF_WITH_ASSERT((0 <= kernelId) && (kernelId < kernelsAmount))
    {
        retVal = _kernelMonitors[kernelId];
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        csProgramsAndKernelsMonitor::kernelMonitor
//   Inputs a kernel's OpenCL handle and returns the kernel's monitor, or NULL if
//   such kernel was not recorded by our OpenGL server.
// Author:      Yaki Tebeka
// Date:        22/11/2009
// ---------------------------------------------------------------------------
const csCLKernel* csProgramsAndKernelsMonitor::kernelMonitor(oaCLKernelHandle kernelHandle) const
{
    const csCLKernel* retVal = NULL;

    // Iterate the existing kernel monitors:
    gtPtrVector<csCLKernel*>::const_iterator iter = _kernelMonitors.begin();
    gtPtrVector<csCLKernel*>::const_iterator endIter = _kernelMonitors.end();

    while (iter != endIter)
    {
        // Get the current kernel monitor:
        const csCLKernel* pCurrentMonitor = *iter;
        GT_IF_WITH_ASSERT(pCurrentMonitor != NULL)
        {
            // If the current kernel is the program we are looking for:
            oaCLKernelHandle currKernelHandle = pCurrentMonitor->kernelHandle();

            if (kernelHandle == currKernelHandle)
            {
                retVal = pCurrentMonitor;
                break;
            }
        }

        iter++;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        csProgramsAndKernelsMonitor::kernelMonitorIndex
// Description: Return a kernel monitor index for the kernel handle
// Arguments:   oaCLKernelHandle kernelHandle
// Return Val:  int
// Author:      Sigal Algranaty
// Date:        4/5/2010
// ---------------------------------------------------------------------------
int csProgramsAndKernelsMonitor::kernelMonitorIndex(oaCLKernelHandle kernelHandle) const
{
    int retVal = -1;

    // Iterate the existing kernel monitors:
    for (int i = 0; i < (int)_kernelMonitors.size(); i++)
    {
        // Get the current kernel monitor:
        const csCLKernel* pCurrentMonitor = _kernelMonitors[i];
        GT_IF_WITH_ASSERT(pCurrentMonitor != NULL)
        {
            // If the current kernel is the program we are looking for:
            oaCLKernelHandle currKernelHandle = pCurrentMonitor->kernelHandle();

            if (kernelHandle == currKernelHandle)
            {
                retVal = i;
                break;
            }
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        csProgramsAndKernelsMonitor::kernelMonitor
//   Inputs a kernel's OpenCL handle and returns the kernel's monitor, or NULL if
//   such kernel was not recorded by our OpenGL server.
// Author:      Yaki Tebeka
// Date:        22/11/2009
// ---------------------------------------------------------------------------
csCLKernel* csProgramsAndKernelsMonitor::kernelMonitor(oaCLKernelHandle kernelHandle)
{
    csCLKernel* retVal = NULL;

    // Iterate the existing kernel monitors:
    gtPtrVector<csCLKernel*>::const_iterator iter = _kernelMonitors.begin();
    gtPtrVector<csCLKernel*>::const_iterator endIter = _kernelMonitors.end();

    while (iter != endIter)
    {
        // Get the current kernel monitor:
        csCLKernel* pCurrentMonitor = *iter;
        GT_IF_WITH_ASSERT(pCurrentMonitor != NULL)
        {
            // If the current kernel is the program we are looking for:
            oaCLKernelHandle currKernelHandle = pCurrentMonitor->kernelHandle();

            if (kernelHandle == currKernelHandle)
            {
                retVal = pCurrentMonitor;
                break;
            }
        }

        iter++;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        csProgramsAndKernelsMonitor::getInternalKernelHandle
// Description: If the program containing the kernel's source is forced, return
//              the internal handle. otherwise, return the external handle
// Author:      Uri Shomroni
// Date:        17/1/2010
// ---------------------------------------------------------------------------
oaCLKernelHandle csProgramsAndKernelsMonitor::getInternalKernelHandle(oaCLKernelHandle externalKernelHandle) const
{
    oaCLKernelHandle retVal = externalKernelHandle;

    gtMap<oaCLHandle, oaCLHandle>::const_iterator findIter = _programAndKernelExternalToInternalHandle.find((oaCLHandle)externalKernelHandle);
    gtMap<oaCLHandle, oaCLHandle>::const_iterator endIter = _programAndKernelExternalToInternalHandle.end();

    // The handle will appear in the map iff the code is forced:
    if (findIter != endIter)
    {
        // Set the real value:
        retVal = (oaCLKernelHandle)((*findIter).second);
    }
    else
    {
        // This kernel was created by a function not monitored by us, register it:
        csProgramsAndKernelsMonitor* pNonConstMe = (csProgramsAndKernelsMonitor*)this;

        cl_kernel handleAsCLKernel = (cl_kernel)externalKernelHandle;

        // Get the kernel's name and controlling program:
        SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_clGetKernelInfo);
        cl_program kernelProgram = NULL;
        cl_int retCode = cs_stat_realFunctionPointers.clGetKernelInfo(handleAsCLKernel, CL_KERNEL_PROGRAM, sizeof(cl_program), &kernelProgram, NULL);

        if (retCode != CL_SUCCESS)
        {
            GT_ASSERT(retCode == CL_SUCCESS);
            kernelProgram = NULL;
        }

        gtSizeType kernelNameLength = 0;
        gtString kernelName;
        retCode = cs_stat_realFunctionPointers.clGetKernelInfo(handleAsCLKernel, CL_KERNEL_FUNCTION_NAME, 0, NULL, &kernelNameLength);
        GT_IF_WITH_ASSERT((retCode == CL_SUCCESS) && (kernelNameLength > 0))
        {
            char* pKernelName = new char[kernelNameLength + 1];
            retCode = cs_stat_realFunctionPointers.clGetKernelInfo(handleAsCLKernel, CL_KERNEL_FUNCTION_NAME, kernelNameLength + 1, pKernelName, NULL);
            pKernelName[kernelNameLength] = '\0';
            GT_IF_WITH_ASSERT(retCode == CL_SUCCESS)
            {
                kernelName.fromASCIIString(pKernelName);
            }

            delete[] pKernelName;
        }
        SU_AFTER_EXECUTING_REAL_FUNCTION(ap_clGetKernelInfo);

        // Register the kernel in the vector and maps:
        pNonConstMe->onKernelCreation(kernelProgram, handleAsCLKernel, kernelName);
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        csProgramsAndKernelsMonitor::logProgramSourceCodeUpdate
// Description: Is called after a program source code is changed.
//  Logs the new source code into a file and updates the object that represents
//  the program.
// Arguments: programObject - The object that represents the program.
//            programSourceCode - The program source code.
// Return Val: bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        18/11/2009
// ---------------------------------------------------------------------------
bool csProgramsAndKernelsMonitor::logProgramSourceCodeUpdate(apCLProgram& programObject, int programIndex, const gtASCIIString& programSourceCode)
{
    bool retVal = false;

    // Check if the source file is not already in the project files:
    bool programFound = false;
    gtVector<osFilePath> programsFilePaths = csOpenCLMonitor::instance().programsFilePath();
    int numPrograms = (int)programsFilePaths.size();
    gtVector<char> currentProgramSource;

    for (int nProgram = 0 ; nProgram < numPrograms ; nProgram++)
    {
        // Read the program:
        osFile programCodeFile;
        bool rc = programCodeFile.open(programsFilePaths[nProgram], osChannel::OS_BINARY_CHANNEL, osFile::OS_OPEN_TO_READ);
        GT_ASSERT(rc);

        // Get the size of the program:
        unsigned long programSize = 0;
        rc = programCodeFile.getSize(programSize);
        GT_ASSERT(rc);

        // allocate a buffer:
        currentProgramSource.resize(programSize);
        char* programBuffer = &(currentProgramSource[0]);

        // Read the program into the buffer:
        gtSize_t programRead;
        rc = programCodeFile.readAvailableData(programBuffer, programSize, programRead);
        GT_ASSERT(rc);

        // Close the file:
        programCodeFile.close();

        // convert to match input source:
        gtSize_t programSizeArray[1] = {programSize};
        gtASCIIString originalFileConverted;
        apCLMultiStringParameter originalFileAsString(1, (const char**)&programBuffer, programSizeArray);
        originalFileAsString.valueAsString(originalFileConverted);

        if (originalFileConverted == programSourceCode)
        {
            programObject.setSourceCodeFilePath(programsFilePaths[nProgram]);
            programFound = true;
            retVal = true;

            break;
        }
    }

    if (!programFound)
    {
        // Generate a file path for the input program's source code:
        osFilePath sourceCodeFilePath;
        generateProgramSourceCodeFilePath(programObject, programIndex, sourceCodeFilePath);

        // Save the shader source code into this file:
        osFile sourceCodeFile;
        bool rc = sourceCodeFile.open(sourceCodeFilePath, osChannel::OS_ASCII_TEXT_CHANNEL, osFile::OS_OPEN_TO_WRITE);

        if (rc)
        {
            // Write the shader source code into the file:
            sourceCodeFile << programSourceCode;

            // Close the file:
            sourceCodeFile.close();

            // Log the file path in the shader wrapper object:
            programObject.setSourceCodeFilePath(sourceCodeFilePath);

            retVal = true;
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        csProgramsAndKernelsMonitor::generateProgramSourceCodeFilePath
// Description: Generates a program source code file path.
// Arguments:   programObject - The program object used to generate the Path.
//              int programIndex - the program internal index
//              filePath - The output file path.
// Author:      Sigal Algranaty
// Date:        18/11/2009
// ---------------------------------------------------------------------------
void csProgramsAndKernelsMonitor::generateProgramSourceCodeFilePath(apCLProgram& programObject, int programIndex, osFilePath& filePath) const
{
    (void)(programObject); // unused
    // Get the current debugged application name:
    gtString applicationName;
    bool rc = osGetCurrentApplicationName(applicationName);
    GT_IF_WITH_ASSERT(rc && !applicationName.isEmpty())
    {
        // Build the log file name:
        gtString logFileName;
        logFileName.appendFormattedString(CS_STR_programFilePath, applicationName.asCharArray(), _spyContextId, programIndex);

        // Set the log file path:
        filePath = suCurrentProjectLogFilesDirectory();
        filePath.setFileName(logFileName);

        // Set the log file extension:
        // TO_DO: OpenCL : invent some other extension for OpenCL programs
        gtString extensionString = CS_STR_kernelSourceFileExtension;
        filePath.setFileExtension(extensionString);
    }
}

// ---------------------------------------------------------------------------
// Name:        csProgramsAndKernelsMonitor::relinkKernelAfterProgramBuild
// Description: Relinks the kernel after a program was rebuilt, and maps the
//              handles appropriately
// Return Val: bool  - Success / failure.
// Author:      Uri Shomroni
// Date:        18/1/2010
// ---------------------------------------------------------------------------
bool csProgramsAndKernelsMonitor::relinkKernelAfterProgramBuild(const csCLKernel* pKernel, oaCLProgramHandle newProgramInternalHandle, oaCLKernelHandle& newKernelInternalHandle)
{
    bool retVal = false;

    GT_IF_WITH_ASSERT(pKernel != NULL)
    {
        // Create a new kernel from the name:
        cl_int errCode = CL_SUCCESS;
        const char* kernelFunctionNameAsASCIICharArray = pKernel->kernelFunctionName().asASCIICharArray();
        SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_clCreateKernel);

        if (cs_stat_pIKernelDebuggingManager->isAMDKernelDebuggingEnabled() && (suIKernelDebuggingManager::CS_OPENCL_SOFTWARE_KERNEL_DEBUGGER == cs_stat_pIKernelDebuggingManager->kernelDebuggerType()))
        {
            newKernelInternalHandle = (oaCLKernelHandle)cs_stat_amdKernelDebuggingFunctionPointers.amdclInterceptCreateKernel((cl_program)newProgramInternalHandle, kernelFunctionNameAsASCIICharArray, &errCode);
        }
        else // !cs_stat_pIKernelDebuggingManager->isAMDKernelDebuggingEnabled() || (suIKernelDebuggingManager::CS_OPENCL_SOFTWARE_KERNEL_DEBUGGER != cs_stat_pIKernelDebuggingManager->kernelDebuggerType())
        {
            newKernelInternalHandle = (oaCLKernelHandle)cs_stat_realFunctionPointers.clCreateKernel((cl_program)newProgramInternalHandle, kernelFunctionNameAsASCIICharArray, &errCode);
        }

        SU_AFTER_EXECUTING_REAL_FUNCTION(ap_clCreateKernel);

        // If we managed to create the new kernel:
        GT_IF_WITH_ASSERT(newKernelInternalHandle != OA_CL_NULL_HANDLE)
        {
            // Set whichever arguments we can:
            retVal = pKernel->restoreKernelArguments(newKernelInternalHandle);
            GT_ASSERT(retVal);
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        csProgramsAndKernelsMonitor::replaceKernelAfterProgramBuild
// Description: Replace the kernel after the program was successfully built including
//              all kernelHandles
// Return Val:  bool - Success / failure.
// Author:      Uri Shomroni
// Date:        18/1/2010
// ---------------------------------------------------------------------------
bool csProgramsAndKernelsMonitor::replaceKernelAfterProgramBuild(const csCLKernel* pKernel, oaCLKernelHandle newKernelInternalHandle)
{
    bool retVal = false;

    GT_IF_WITH_ASSERT(pKernel != NULL)
    {
        oaCLKernelHandle externalKernelHandle = pKernel->kernelHandle();

        // Get the old internal handle:
        gtMap<oaCLHandle, oaCLHandle>::iterator findExToInIter = _programAndKernelExternalToInternalHandle.find(externalKernelHandle);
        gtMap<oaCLHandle, oaCLHandle>::iterator endExToInIter = _programAndKernelExternalToInternalHandle.end();
        GT_IF_WITH_ASSERT(findExToInIter != endExToInIter)
        {
            oaCLKernelHandle oldInternalHandle = (*findExToInIter).second;

            // Update the old kernel data:
            bool rcUpd = updateKernelInfo((csCLKernel*)pKernel);
            GT_ASSERT(rcUpd);

            // Make the new kernel's reference count equal to the old one's, and release the old one:
            int refCount = pKernel->referenceCount();
            SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_clRetainKernel);
            SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_clReleaseKernel);
            bool isCSKernelDebuggingOn = cs_stat_pIKernelDebuggingManager->isAMDKernelDebuggingEnabled() && (suIKernelDebuggingManager::CS_OPENCL_SOFTWARE_KERNEL_DEBUGGER == cs_stat_pIKernelDebuggingManager->kernelDebuggerType());

            // The new program already has one reference from being created:
            for (int i = 1; i < refCount; i++)
            {
                if (isCSKernelDebuggingOn)
                {
                    cl_int rcRelease = cs_stat_amdKernelDebuggingFunctionPointers.amdclInterceptRetainKernel((cl_kernel)newKernelInternalHandle);
                    GT_ASSERT(rcRelease == CL_SUCCESS);
                    rcRelease = cs_stat_amdKernelDebuggingFunctionPointers.amdclInterceptReleaseKernel((cl_kernel)oldInternalHandle);
                    GT_ASSERT(rcRelease == CL_SUCCESS);
                }
                else // !isKernelDebuggingOn
                {
                    cl_int rcRelease = cs_stat_realFunctionPointers.clRetainKernel((cl_kernel)newKernelInternalHandle);
                    GT_ASSERT(rcRelease == CL_SUCCESS);
                    rcRelease = cs_stat_realFunctionPointers.clReleaseKernel((cl_kernel)oldInternalHandle);
                    GT_ASSERT(rcRelease == CL_SUCCESS);
                }
            }

            // Release the old kernel (it has an extra reference count from being created:
            if (isCSKernelDebuggingOn)
            {
                cl_int rcRelease2 = cs_stat_amdKernelDebuggingFunctionPointers.amdclInterceptReleaseKernel((cl_kernel)oldInternalHandle);
                GT_ASSERT(rcRelease2 == CL_SUCCESS);
            }
            else // !isKernelDebuggingOn
            {
                cl_int rcRelease2 = cs_stat_realFunctionPointers.clReleaseKernel((cl_kernel)oldInternalHandle);
                GT_ASSERT(rcRelease2 == CL_SUCCESS);
            }

            SU_AFTER_EXECUTING_REAL_FUNCTION(ap_clRetainKernel);
            SU_AFTER_EXECUTING_REAL_FUNCTION(ap_clReleaseKernel);

            // Re-map the kernel in our name maps:
            _programAndKernelExternalToInternalHandle[externalKernelHandle] = newKernelInternalHandle;
            _programAndKernelInternalToExternalHandle[newKernelInternalHandle] = externalKernelHandle;
            _programAndKernelInternalToExternalHandle[oldInternalHandle] = OA_CL_NULL_HANDLE;

            retVal = true;
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        csProgramsAndKernelsMonitor::updateProgramsInfo
// Description: Update each of the program objects using clGetProgramInfo
// Return Val: bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        24/11/2009
// ---------------------------------------------------------------------------
bool csProgramsAndKernelsMonitor::updateProgramsInfo()
{
    bool retVal = true;

    int numberOfPrograms = (int)_programMonitors.size();

    for (int i = 0; i < numberOfPrograms; i++)
    {
        // Get current programs:
        apCLProgram* pProgram = _programMonitors[i];
        retVal = updateProgramInfo(pProgram) && retVal;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        csProgramsAndKernelsMonitor::updateProgramInfo
// Description: Updates one program's information
// Return Val:  bool - Success / failure.
// Author:      Uri Shomroni
// Date:        18/1/2010
// ---------------------------------------------------------------------------
bool csProgramsAndKernelsMonitor::updateProgramInfo(apCLProgram* pProgram)
{
    bool retVal = true;

    GT_IF_WITH_ASSERT(pProgram != NULL)
    {
        // If the program was not marked for deletion:
        if (!pProgram->wasMarkedForDeletion())
        {
            // Get the program handle:
            gtMap<oaCLHandle, oaCLHandle>::const_iterator findIter = _programAndKernelExternalToInternalHandle.find(pProgram->programHandle());
            gtMap<oaCLHandle, oaCLHandle>::const_iterator endIter = _programAndKernelExternalToInternalHandle.end();
            GT_IF_WITH_ASSERT(findIter != endIter)
            {
                retVal = updateProgramInfoWithInternalHandle(*pProgram, ((*findIter).second));
            }
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        csProgramsAndKernelsMonitor::updateProgramInfoWithInternalHandle
// Description: Performs the actual queries for updateProgramInfo.
//              Allowing to get a given internal handle allows us to get the build
//              status and log for failed builds caused by buildProgram.
// Return Val: bool  - Success / failure.
// Author:      Uri Shomroni
// Date:        19/1/2010
// ---------------------------------------------------------------------------
bool csProgramsAndKernelsMonitor::updateProgramInfoWithInternalHandle(apCLProgram& programDetails, oaCLProgramHandle programInternalHandle)
{
    bool retVal = false;

    // Update the program devices:
    bool rcUpdateDevices = updateProgramDevices(programDetails, programInternalHandle);
    GT_ASSERT(rcUpdateDevices);

    cl_program programInternalHandleAsCLProgram = (cl_program)(programInternalHandle);

    // Get the program reference count:
    SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_clGetProgramInfo);
    cl_uint refCount = 0;
    cl_uint cRetVal3 = cs_stat_realFunctionPointers.clGetProgramInfo(programInternalHandleAsCLProgram, CL_PROGRAM_REFERENCE_COUNT, sizeof(cl_uint), &refCount, NULL);
    GT_IF_WITH_ASSERT(cRetVal3 == CL_SUCCESS)
    {
        // Set the program reference count.
        // Subtract 1 for the reference that the debugger adds:
        programDetails.setReferenceCount((gtUInt32)((refCount > 0) ? (refCount - 1) : refCount));
    }
    SU_AFTER_EXECUTING_REAL_FUNCTION(ap_clGetProgramInfo);

    // Get the program build log:
    cl_uint cRetVal4 = CL_SUCCESS;
    bool rcBuildData = true;
    const gtVector<int>& programDevices = programDetails.devices();
    gtVector<apCLProgram::programBuildData>& programDevicesBuildData = programDetails.devicesBuildData();
    int numberOfDevices = (int)programDevices.size();
    SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_clGetProgramBuildInfo);

    gtVector<char> optionsStringBuf;

    for (int i = 0; i < numberOfDevices; i++)
    {
        // Get the build log:
        bool rcGetBuildLog = updateProgramBuildLog(programDetails, programInternalHandle, i);
        GT_ASSERT(rcGetBuildLog);

        // Get the build log:
        (void) updateProgramBinaryType(programDetails, programInternalHandle, i);

        // Get the requested device handle:
        csDevicesMonitor& theDevicesMonitor = csOpenCLMonitor::instance().devicesMonitor();
        const apCLDevice* pDevice = theDevicesMonitor.getDeviceObjectDetailsByIndex(programDevices[i]);
        cl_device_id curDeviceHandle = (cl_device_id)(pDevice->deviceHandle());

        // Get the current build data:
        apCLProgram::programBuildData& currentBuildData = programDevicesBuildData[i];

        // Get the program build status:
        cl_build_status buildStatus = CL_BUILD_NONE;
        cRetVal4 = cs_stat_realFunctionPointers.clGetProgramBuildInfo(programInternalHandleAsCLProgram, curDeviceHandle, CL_PROGRAM_BUILD_STATUS, sizeof(cl_build_status), &buildStatus, NULL);

        if (cRetVal4 == CL_SUCCESS)
        {
            currentBuildData._buildStatus = buildStatus;
        }

        // Get the program build options:
        gtSizeType buildOptionsLength = 0;
        cRetVal4 = cs_stat_realFunctionPointers.clGetProgramBuildInfo(programInternalHandleAsCLProgram, curDeviceHandle, CL_PROGRAM_BUILD_OPTIONS, 0, NULL, &buildOptionsLength);

        if ((cRetVal4 == CL_SUCCESS) && (buildOptionsLength > 0))
        {
            optionsStringBuf.resize(buildOptionsLength + 2);
            cRetVal4 = cs_stat_realFunctionPointers.clGetProgramBuildInfo(programInternalHandleAsCLProgram, curDeviceHandle, CL_PROGRAM_BUILD_OPTIONS, buildOptionsLength + 1, &(optionsStringBuf[0]), NULL);
            optionsStringBuf[buildOptionsLength + 1] = '\0';
            GT_IF_WITH_ASSERT(cRetVal4 == CL_SUCCESS)
            {
                currentBuildData._buildOptions.fromASCIIString(&(optionsStringBuf[0]));

                // If we are using the AMD kernel debugging API, we are appending " -g -cl-opt-disable" to the end:
                if (cs_stat_pIKernelDebuggingManager->isAMDKernelDebuggingEnabled() && (suIKernelDebuggingManager::CS_OPENCL_SOFTWARE_KERNEL_DEBUGGER == cs_stat_pIKernelDebuggingManager->kernelDebuggerType()))
                {
                    // Verify that this was indeed added at the end:
                    static const gtString forcedBuildOptions = SU_STR_kernelDebuggingForcedBuildOptions;
                    static const int forcedBuildOptionsLength = forcedBuildOptions.length();
                    static const gtString forcedBuildOptions2 = SU_STR_kernelDebuggingForcedBuildOptionLegacy;
                    static const int forcedBuildOptions2Length = forcedBuildOptions2.length();
                    int foundPosition = currentBuildData._buildOptions.reverseFind(forcedBuildOptions);

                    // We allow for the flags not to be present if the program was not yet built:
                    if (!((foundPosition > -1) || (CL_BUILD_NONE == currentBuildData._buildStatus)))
                    {
                        // Output a log message if the flags were not found:
                        gtString logMsg = currentBuildData._buildOptions;
                        logMsg.prepend(L"Program under OpenCL kernel debugger did not have the debugging flags added. Program build flags: ");
                        OS_OUTPUT_DEBUG_LOG(logMsg.asCharArray(), OS_DEBUG_LOG_DEBUG);
                    }

                    if (foundPosition > -1)
                    {
                        // If these are the only build options, clear this field entirely:
                        int buildOptionsLength2 = currentBuildData._buildOptions.length();
                        GT_ASSERT((foundPosition == (buildOptionsLength2 - forcedBuildOptionsLength)) ||
                                  (foundPosition == (buildOptionsLength2 - forcedBuildOptionsLength - forcedBuildOptions2Length)));

                        if (0 < foundPosition)
                        {
                            currentBuildData._buildOptions.truncate(0, foundPosition - 1);
                        }
                        else // 0 == foundPosition
                        {
                            currentBuildData._buildOptions.makeEmpty();
                        }
                    }
                }
            }
        }
    }

    // Update the program kernels:
    bool rcUpdateKernels = updateProgramKernels(programDetails, programInternalHandle);
    GT_ASSERT(rcUpdateKernels);

    SU_AFTER_EXECUTING_REAL_FUNCTION(ap_clGetProgramBuildInfo);

    retVal = (rcUpdateDevices && rcUpdateKernels && (cRetVal3 == CL_SUCCESS) && rcBuildData);

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        csProgramsAndKernelsMonitor::updateProgramDevices
// Description: Updates the program object with the details of the program devices
// Arguments:   apCLProgram& programDetails
//              oaCLProgramHandle programInternalHandle
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        3/7/2011
// ---------------------------------------------------------------------------
bool csProgramsAndKernelsMonitor::updateProgramDevices(apCLProgram& programDetails, oaCLProgramHandle programInternalHandle)
{
    bool retVal = false;

    // Get the program devices number:
    SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_clGetProgramInfo);
    cl_uint numDevices = 0;
    cl_uint cRetVal1 = cs_stat_realFunctionPointers.clGetProgramInfo((cl_program)programInternalHandle, CL_PROGRAM_NUM_DEVICES, sizeof(cl_uint), &numDevices, NULL);

    if (cRetVal1 == CL_SUCCESS)
    {
        GT_IF_WITH_ASSERT(numDevices > 0)
        {
            // Get the program devices:
            csDevicesMonitor& theDevicesMonitor = csOpenCLMonitor::instance().devicesMonitor();
            cl_device_id* pCLProgramDevices = new cl_device_id[numDevices];
            cl_uint cRetVal2 = cs_stat_realFunctionPointers.clGetProgramInfo((cl_program)programInternalHandle, CL_PROGRAM_DEVICES, numDevices * sizeof(cl_device_id), pCLProgramDevices, NULL);
            GT_IF_WITH_ASSERT(cRetVal2 == CL_SUCCESS)
            {
                retVal = true;

                // Update the program devices:
                programDetails.clearDevices();

                for (int i = 0; i < (int) numDevices; i++)
                {
                    // Get the current device id:
                    oaCLDeviceID deviceHandle = (oaCLDeviceID)pCLProgramDevices[i];
                    int deviceID = theDevicesMonitor.getDeviceObjectAPIID(deviceHandle);
                    GT_IF_WITH_ASSERT(deviceID != -1)
                    {
                        // Add the device to the program :
                        programDetails.addDevice(deviceID);
                    }
                }
            }

            // Delete the devices vector:
            delete[] pCLProgramDevices;
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        csProgramsAndKernelsMonitor::updateProgramKernels
// Description: Updates the program object with the details of the program kernel names
// Arguments:   apCLProgram& programDetails
//              oaCLProgramHandle programInternalHandle
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        11/1/2012
// ---------------------------------------------------------------------------
bool csProgramsAndKernelsMonitor::updateProgramKernels(apCLProgram& programDetails, oaCLProgramHandle programInternalHandle)
{
    bool retVal = true;

    // This operation is only relevant if the program was built for at least one device, and if this query was never performed before:
    if (programDetails.wasProgramBuiltSuccessfully() && programDetails.canUpdateProgramKernels() && programDetails.kernelNames().empty())
    {
        // Get the program devices number:
        SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_clGetProgramInfo);

        // Get the kernel count:
        gtSizeType numKernels = 0;
        cl_uint cRetVal1 = cs_stat_realFunctionPointers.clGetProgramInfo((cl_program)programInternalHandle, CL_PROGRAM_NUM_KERNELS, sizeof(gtSizeType), &numKernels, NULL);
        retVal = retVal && (cRetVal1 == CL_SUCCESS);

        // Get the kernel names:
        gtASCIIString kernelNamesStr;
        gtSizeType kernelNamesLength = 0;
        cl_int retCode = cs_stat_realFunctionPointers.clGetProgramInfo((cl_program)programInternalHandle, CL_PROGRAM_KERNEL_NAMES, 0, NULL, &kernelNamesLength);

        // Do not assert here as this might be a program created with binaries and not sources:
        if ((retCode == CL_SUCCESS) && (kernelNamesLength > 0))
        {
            // Get the source code:
            char* pKernelNames = new char[kernelNamesLength + 1];
            retCode = cs_stat_realFunctionPointers.clGetProgramInfo((cl_program)programInternalHandle, CL_PROGRAM_KERNEL_NAMES, kernelNamesLength + 1, pKernelNames, NULL);
            pKernelNames[kernelNamesLength] = '\0';

            if (retCode == CL_SUCCESS)
            {
                kernelNamesStr = pKernelNames;
                GT_ASSERT(kernelNamesStr.count(';') + 1 == (int)numKernels);

                // Set the program kernels:
                programDetails.setProgramKernels(kernelNamesStr);
            }

            delete[] pKernelNames;
        }

        SU_AFTER_EXECUTING_REAL_FUNCTION(ap_clGetProgramInfo);
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        csProgramsAndKernelsMonitor::updateKernelsInfo
// Description: Update each of the kernelHandles using clGetKernelInfo
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        24/11/2009
// ---------------------------------------------------------------------------
bool csProgramsAndKernelsMonitor::updateKernelsInfo()
{
    bool retVal = true;

    for (int i = 0; i < (int)_kernelMonitors.size(); i++)
    {
        // Get current kernel:
        csCLKernel* pKernel = _kernelMonitors[i];

        // Update the kernel general info:
        bool rc = updateKernelInfo(pKernel) && retVal;
        retVal = rc && retVal;

        // Update the kernel args info:
        rc = updateKernelArgsInfo(pKernel);
        retVal = rc && retVal;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        csProgramsAndKernelsMonitor::updateKernelInfo
// Description: Updates a single kernel's information
// Return Val:  bool - Success / failure.
// Author:      Uri Shomroni
// Date:        18/1/2010
// ---------------------------------------------------------------------------
bool csProgramsAndKernelsMonitor::updateKernelInfo(csCLKernel* pKernel)
{
    bool retVal = true;

    GT_IF_WITH_ASSERT(pKernel != NULL)
    {
        gtMap<oaCLHandle, oaCLHandle>::const_iterator findIter = _programAndKernelExternalToInternalHandle.find(pKernel->kernelHandle());
        gtMap<oaCLHandle, oaCLHandle>::const_iterator endIter = _programAndKernelExternalToInternalHandle.end();
        GT_IF_WITH_ASSERT(findIter != endIter)
        {
            // Get the kernel handle:
            cl_kernel kernelInternalHandle = (cl_kernel)((*findIter).second);

            // Get the kernel arguments number:
            SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_clGetKernelInfo);
            cl_uint numArgs = 0;
            cl_uint cRetVal1 = cs_stat_realFunctionPointers.clGetKernelInfo(kernelInternalHandle, CL_KERNEL_NUM_ARGS, sizeof(cl_uint), &numArgs, NULL);
            GT_IF_WITH_ASSERT(cRetVal1 == CL_SUCCESS)
            {
                // Set the kernel number of arguments:
                pKernel->setNumArgs(numArgs);
            }

            // Get the kernel reference count:
            cl_uint refCount = 0;
            cl_uint cRetVal2 = cs_stat_realFunctionPointers.clGetKernelInfo(kernelInternalHandle, CL_KERNEL_REFERENCE_COUNT, sizeof(cl_uint), &refCount, NULL);
            GT_IF_WITH_ASSERT(cRetVal2 == CL_SUCCESS)
            {
                // Set the kernel reference count.
                // Subtract 1 for the reference that the debugger adds:
                pKernel->setRefCount((gtUInt32)((refCount > 0) ? (refCount - 1) : refCount));
            }
            SU_AFTER_EXECUTING_REAL_FUNCTION(ap_clGetKernelInfo);

            // Get the kernel workgroup information:
            // Get the kernel associated program (in order to get the device id):
            const apCLProgram* pProgram = programMonitor(pKernel->programHandle());
            bool rc3 = true;

            if ((nullptr != pProgram) && (nullptr != cs_stat_realFunctionPointers.clGetKernelWorkGroupInfo))
            {
                pKernel->clearWorkgroupInfo();

                SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_clGetKernelWorkGroupInfo);
                const gtVector<int>& devices = pProgram->devices();
                int deviceCount = (int)devices.size();

                csDevicesMonitor& theDevicesMonitor = csOpenCLMonitor::instance().devicesMonitor();

                for (int i = 0; deviceCount > i; ++i)
                {
                    int deviceId = devices[i];
                    const apCLDevice* pDevice = theDevicesMonitor.getDeviceObjectDetailsByIndex(deviceId);
                    GT_IF_WITH_ASSERT(pDevice != NULL)
                    {
                        bool rcDev = true;
                        cl_device_id deviceHandle = (cl_device_id)(pDevice->deviceHandle());
                        size_t maxWorkGroupSize = 0;
                        cl_uint cRetVal3 = cs_stat_realFunctionPointers.clGetKernelWorkGroupInfo(kernelInternalHandle, deviceHandle, CL_KERNEL_WORK_GROUP_SIZE, sizeof(size_t), &maxWorkGroupSize, NULL);
                        GT_ASSERT(cRetVal3 == CL_SUCCESS);
                        rcDev = rcDev && (CL_SUCCESS == cRetVal3);

                        size_t requiredWorkGroupSize[3] = { 0 };
                        cRetVal3 = cs_stat_realFunctionPointers.clGetKernelWorkGroupInfo(kernelInternalHandle, deviceHandle, CL_KERNEL_COMPILE_WORK_GROUP_SIZE, 3 * sizeof(size_t), requiredWorkGroupSize, NULL);
                        GT_ASSERT(cRetVal3 == CL_SUCCESS);
                        rcDev = rcDev && (CL_SUCCESS == cRetVal3);

                        cl_ulong requiredLocalMemorySize = 0;
                        cRetVal3 = cs_stat_realFunctionPointers.clGetKernelWorkGroupInfo(kernelInternalHandle, deviceHandle, CL_KERNEL_LOCAL_MEM_SIZE, sizeof(cl_ulong), &requiredLocalMemorySize, NULL);
                        GT_ASSERT(cRetVal3 == CL_SUCCESS);
                        rcDev = rcDev && (CL_SUCCESS == cRetVal3);

                        // If we got the data for one device:
                        if (rcDev)
                        {
                            // Add it to the kernel object:
                            pKernel->addKernelWorkgroupInfo((gtInt32)deviceId, (gtUInt64)maxWorkGroupSize, (gtUInt64)requiredWorkGroupSize[0], (gtUInt64)requiredWorkGroupSize[1], (gtUInt64)requiredWorkGroupSize[2], (gtUInt64)requiredLocalMemorySize);
                        }

                        rc3 = rc3 && rcDev;
                    }
                }

                SU_AFTER_EXECUTING_REAL_FUNCTION(ap_clGetKernelWorkGroupInfo);
            }

            retVal = (cRetVal1 == CL_SUCCESS) && (cRetVal2 == CL_SUCCESS) && rc3;
        }
    }

    return retVal;
}



// ---------------------------------------------------------------------------
// Name:        csProgramsAndKernelsMonitor::updateKernelArgsInfo
// Description: Update the kernel arguments info
// Arguments:   csCLKernel* pKernel
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        11/1/2012
// ---------------------------------------------------------------------------
bool csProgramsAndKernelsMonitor::updateKernelArgsInfo(csCLKernel* pKernel)
{
    bool retVal = true;
    GT_IF_WITH_ASSERT(pKernel != NULL)
    {
        // If we support OpenCL 1.2:
        if (cs_stat_realFunctionPointers.clGetKernelArgInfo != NULL)
        {
            // Clear the kernel arguments info:
            pKernel->clearArgumentsInfo();

            // Get the internal handle:
            oaCLKernelHandle hKernel = pKernel->kernelHandle();
            gtMap<oaCLHandle, oaCLHandle>::const_iterator findIter = _programAndKernelExternalToInternalHandle.find(hKernel);
            gtMap<oaCLHandle, oaCLHandle>::const_iterator endIter = _programAndKernelExternalToInternalHandle.end();
            GT_IF_WITH_ASSERT(findIter != endIter)
            {
                // Get the kernel handle:
                cl_kernel kernelInternalHandle = (cl_kernel)((*findIter).second);

                // Get the kernel arguments number:
                gtASCIIString argTypeName;
                gtASCIIString argName;
                gtVector<char> outputStringBuf;

                // Get the kernel arguments amount:
                SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_clGetKernelInfo);
                cl_uint kernelArgNum = 0;
                cl_int rcNum = cs_stat_realFunctionPointers.clGetKernelInfo(kernelInternalHandle, CL_KERNEL_NUM_ARGS, sizeof(cl_uint), &kernelArgNum, NULL);
                SU_AFTER_EXECUTING_REAL_FUNCTION(ap_clGetKernelInfo);

                if ((CL_SUCCESS == rcNum) && (0 < kernelArgNum))
                {
                    SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_clGetKernelArgInfo);

                    for (int i = 0; i < (int)kernelArgNum; i++)
                    {
                        cl_kernel_arg_address_qualifier addressQualifier;
                        cl_uint cRetVal = cs_stat_realFunctionPointers.clGetKernelArgInfo(kernelInternalHandle, i, CL_KERNEL_ARG_ADDRESS_QUALIFIER, sizeof(cl_kernel_arg_address_qualifier), &addressQualifier, NULL);
                        GT_ASSERT(cRetVal == CL_SUCCESS);

                        cl_kernel_arg_access_qualifier accessQualifier;
                        cRetVal = cs_stat_realFunctionPointers.clGetKernelArgInfo(kernelInternalHandle, i, CL_KERNEL_ARG_ACCESS_QUALIFIER, sizeof(cl_kernel_arg_access_qualifier), &accessQualifier, NULL);
                        GT_ASSERT(cRetVal == CL_SUCCESS);

                        size_t argTypeNameLength = 0;
                        cRetVal = cs_stat_realFunctionPointers.clGetKernelArgInfo(kernelInternalHandle, i, CL_KERNEL_ARG_TYPE_NAME, 0, NULL, &argTypeNameLength);
                        GT_IF_WITH_ASSERT((cRetVal == CL_SUCCESS) && (argTypeNameLength > 0))
                        {
                            // Get the arg type name:
                            outputStringBuf.resize(argTypeNameLength + 2);
                            cRetVal = cs_stat_realFunctionPointers.clGetKernelArgInfo(kernelInternalHandle, i, CL_KERNEL_ARG_TYPE_NAME, argTypeNameLength + 1, &(outputStringBuf[0]), NULL);
                            outputStringBuf[argTypeNameLength + 1] = '\0';

                            if (cRetVal == CL_SUCCESS)
                            {
                                argTypeName = &(outputStringBuf[0]);
                            }
                        }

                        cl_kernel_arg_type_qualifier argTypeQualifier;
                        cRetVal = cs_stat_realFunctionPointers.clGetKernelArgInfo(kernelInternalHandle, i, CL_KERNEL_ARG_TYPE_QUALIFIER, sizeof(cl_kernel_arg_type_qualifier), &argTypeQualifier, NULL);
                        GT_ASSERT(cRetVal == CL_SUCCESS);

                        size_t argNameLength = 0;
                        cRetVal = cs_stat_realFunctionPointers.clGetKernelArgInfo(kernelInternalHandle, i, CL_KERNEL_ARG_NAME, 0, NULL, &argNameLength);
                        GT_IF_WITH_ASSERT((cRetVal == CL_SUCCESS) && (argNameLength > 0))
                        {
                            // Get the arg name:
                            outputStringBuf.resize(argNameLength + 2);
                            cRetVal = cs_stat_realFunctionPointers.clGetKernelArgInfo(kernelInternalHandle, i, CL_KERNEL_ARG_NAME, argNameLength + 1, &(outputStringBuf[0]), NULL);
                            outputStringBuf[argNameLength + 1] = '\0';

                            if (cRetVal == CL_SUCCESS)
                            {
                                argName = &(outputStringBuf[0]);
                            }
                        }

                        // Add a kernel argument info:
                        pKernel->addKernelArgumentInfo(addressQualifier, accessQualifier, argTypeQualifier, argTypeName, argName);
                    }

                    SU_AFTER_EXECUTING_REAL_FUNCTION(ap_clGetKernelArgInfo);
                }
            }
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        csProgramsAndKernelsMonitor::updateProgramBuildLog
// Description: Update the program build log
// Arguments:   apCLProgram& programDetails
//              oaCLProgramHandle programInternalHandle
//              int currentDeviceIndex
// Return Val:  bool - Success / failure.
// Author:      Gilad Yarnitzky
// Date:        3/7/2011
// ---------------------------------------------------------------------------
bool csProgramsAndKernelsMonitor::updateProgramBuildLog(apCLProgram& programDetails, oaCLProgramHandle programInternalHandle, int currentDeviceIndexInProgram)
{
    bool retVal = false;

    cl_program programInternalHandleAsCLProgram = (cl_program)(programInternalHandle);

    // Get the devices monitor:
    csDevicesMonitor& theDevicesMonitor = csOpenCLMonitor::instance().devicesMonitor();

    // Get the requested device handle:
    int currentDeviceAPIID = programDetails.devices()[currentDeviceIndexInProgram];
    const apCLDevice* pDevice = theDevicesMonitor.getDeviceObjectDetailsByIndex(currentDeviceAPIID);
    GT_IF_WITH_ASSERT(pDevice != NULL)
    {
        // Get the program device build data:
        gtVector<apCLProgram::programBuildData>& programDevicesBuildData = programDetails.devicesBuildData();

        // Get the current device id:
        apCLProgram::programBuildData& currentBuildData = programDevicesBuildData[currentDeviceIndexInProgram];

        // Get the device CL handle:
        cl_device_id curDeviceHandle = (cl_device_id)(pDevice->deviceHandle());

        // Get the build log:
        gtSizeType buildLogLength = 0;
        cl_int rcBuild = cs_stat_realFunctionPointers.clGetProgramBuildInfo(programInternalHandleAsCLProgram, curDeviceHandle, CL_PROGRAM_BUILD_LOG, 0, NULL, &buildLogLength);

        if ((rcBuild == CL_SUCCESS) && (buildLogLength > 0))
        {
            retVal = true;
            char* pBuildLog = new char[buildLogLength + 1];
            rcBuild = cs_stat_realFunctionPointers.clGetProgramBuildInfo(programInternalHandleAsCLProgram, curDeviceHandle, CL_PROGRAM_BUILD_LOG, buildLogLength + 1, pBuildLog, NULL);
            pBuildLog[buildLogLength] = '\0';
            GT_IF_WITH_ASSERT(rcBuild == CL_SUCCESS)
            {
                currentBuildData._buildLog.fromASCIIString(pBuildLog);
            }
            delete[] pBuildLog;

            // Get the global variables total size:
            gtSize_t totalGlobalVariablesSize = 0;
            rcBuild = cs_stat_realFunctionPointers.clGetProgramBuildInfo(programInternalHandleAsCLProgram, curDeviceHandle, CL_PROGRAM_BUILD_GLOBAL_VARIABLE_TOTAL_SIZE, sizeof(size_t), &totalGlobalVariablesSize, NULL);

            if (CL_SUCCESS == rcBuild)
            {
                currentBuildData.m_buildGlobalVariablesTotalSize = (gtUInt64)totalGlobalVariablesSize;
            }
        }
    }
    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        csProgramsAndKernelsMonitor::updateProgramBinaryType
// Description: Get the program binary type
// Arguments:   apCLProgram& programDetails
//              oaCLProgramHandle programInternalHandle
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        11/1/2012
// ---------------------------------------------------------------------------
bool csProgramsAndKernelsMonitor::updateProgramBinaryType(apCLProgram& programDetails, oaCLProgramHandle programInternalHandle, int currentDeviceIndexInProgram)
{
    bool retVal = false;

    cl_program programInternalHandleAsCLProgram = (cl_program)(programInternalHandle);

    // Get the devices monitor:
    csDevicesMonitor& theDevicesMonitor = csOpenCLMonitor::instance().devicesMonitor();

    // Get the requested device handle:
    int currentDeviceAPIID = programDetails.devices()[currentDeviceIndexInProgram];
    const apCLDevice* pDevice = theDevicesMonitor.getDeviceObjectDetailsByIndex(currentDeviceAPIID);
    GT_IF_WITH_ASSERT(pDevice != NULL)
    {
        // Get the program device build data:
        gtVector<apCLProgram::programBuildData>& programDevicesBuildData = programDetails.devicesBuildData();

        // Get the current device id:
        // apCLProgram::programBuildData& currentBuildData = programDevicesBuildData[currentDeviceIndexInProgram];
        (void) programDevicesBuildData[currentDeviceIndexInProgram];

        // Get the device CL handle:
        cl_device_id curDeviceHandle = (cl_device_id)(pDevice->deviceHandle());

        // Get the build log:
        cl_program_binary_type programBinaryType;
        cl_int cRetVal = cs_stat_realFunctionPointers.clGetProgramBuildInfo(programInternalHandleAsCLProgram, curDeviceHandle, CL_PROGRAM_BINARY_TYPE, sizeof(cl_program_binary_type), &programBinaryType, NULL);

        if (cRetVal == CL_SUCCESS)
        {
            programDetails.setProgramBinaryType(programBinaryType);
        }
    }
    return retVal;
}


