//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file csMacOSXInterception.cpp
///
//==================================================================================

//------------------------------ csMacOSXInterception.cpp ------------------------------

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTBaseTools/Include/AMDTDefinitions.h>
#include <AMDTAPIClasses/Include/apMonitoredFunctionsManager.h>

// Spies utilities:
#include <AMDTServerUtilities/Include/suMacOSXInterception.h>

// Local:
#include <src/csExtensionsManager.h>
#include <src/csMacOSXInterception.h>
#include <src/csMonitoredFunctionPointers.h>
#include <src/csGlobalVariables.h>

// ---------------------------------------------------------------------------
// Name:        csInitializeMacOSXOpenCLInterception
// Description: Initializes the interception of the OpenCL function indicated by funcId
// Return Val:  bool - Success / failure.
// Author:      Uri Shomroni
// Date:        18/1/2009
// ---------------------------------------------------------------------------
bool csInitializeMacOSXOpenCLInterception(apMonitoredFunctionId funcId)
{
    bool retVal = false;

    //////////////////////////////////////////////////////////////////////////
    // TO_DO: Uri, 10/8/10 - on Mac 64-bit, the OpenCL driver image has
    // clUnloadCompiler implemented as 8 bytes:
    //
    // 55       push %rbp
    // 48 89 e5 REX.W mov %rbp, %rsp
    // 31 c0    xor %eax, %eax          (note 32-bit addressing for 32-bit return value)
    // c9       leave                   (equivalent to mov %rsp, %rbp; pop %rp)
    // c3       ret (near)
    // As RAX is the int return register in AMD64, and XORing with itself
    // yields 0, this is effectively the same as the C code "return 0;" or
    // more precisely "return CL_SUCCESS;".
    //
    // clUnloadCompiler is directly followed by clReleaseProgram. Since our
    // 64-bit interception instructions are longer than 8 bytes, they will
    // overwrite eachother, causing the beginning of clReleaseProgram to be
    // overwritten by the end of the clUnloadCompiler (or vice-versa), causing
    // one of the to crash when called if we try to intercept both.
    //
    // Since clUnloadCompiler is far rarer (as well as doing nothing in this
    // implementation), we currently do not intercept it, to avoid crashes in
    // clReleaseProgram
    //////////////////////////////////////////////////////////////////////////
#if (AMDT_ADDRESS_SPACE_TYPE == AMDT_64_BIT_ADDRESS_SPACE)

    if (funcId == ap_clUnloadCompiler)
    {
        // Check if the function is indeed too close to clReleaseProgram:
        int clUnloadCompilerIndex = cs_stat_extensionsManager.functionIndexFromMonitoredFunctionId(ap_clUnloadCompiler);
        int clReleaseProgramIndex = cs_stat_extensionsManager.functionIndexFromMonitoredFunctionId(ap_clReleaseProgram);
        gtSize_t clUnloadCompilerAddress = (gtSize_t)(((osProcedureAddress*)(&cs_stat_realFunctionPointers))[clUnloadCompilerIndex]);
        gtSize_t clReleaseProgramAddress = (gtSize_t)(((osProcedureAddress*)(&cs_stat_realFunctionPointers))[clReleaseProgramIndex]);

        // Calculate the distance between the two functions
        gtSize_t addressesDifference = (clReleaseProgramAddress > clUnloadCompilerAddress) ? (clReleaseProgramAddress - clUnloadCompilerAddress) : (clUnloadCompilerAddress - clReleaseProgramAddress);

        // If it is too small:
        if (addressesDifference < SU_LENGTH_OF_JMP_COMMAND_IN_BYTES)
        {
            // Do not intercept clUnloadCompiler:
            return true;
        }
    }

#endif // AMDT_ADDRESS_SPACE_TYPE == AMDT_64_BIT_ADDRESS_SPACE

    // Sanity check
    GT_IF_WITH_ASSERT((funcId >= apFirstOpenCLFunctionIndex) && (funcId <= apLastOpenCLBaseFunctionIndex))
    {
        // We use the function name for getting the wrapper function address, as well as for debug log printouts
        gtString funcName = apMonitoredFunctionsManager::instance().monitoredFunctionName(funcId);

        // Get the real and wrapper addresses:
        osProcedureAddress pWrapperFunction = cs_stat_extensionsManager.wrapperFunctionAddress(funcName);

        // The csMonitoredFunctionPointers struct only contains OpenCL functions, so we need to shift the indices
        // down to access the pointers:
        int funcIndex = cs_stat_extensionsManager.functionIndexFromMonitoredFunctionId(funcId);

        // We need to get the real pointer second, since calling the above function also initializes the address
        // for extension functions.
        osProcedureAddress pRealFunction = ((osProcedureAddress*)(&cs_stat_realFunctionPointers))[funcIndex];

        // Will get the new function start address if needed:
        osProcedureAddress pNewRealFunction = pRealFunction;

        // Call the GRSpiesUtilities to intercept the function:
        retVal = suInitializeMacOSXInterception(funcId, funcName, pRealFunction, pWrapperFunction, pNewRealFunction);

        // If the interception created a new function base pointer, set it into the struct:
        if (pRealFunction != pNewRealFunction)
        {
            ((void**)&cs_stat_realFunctionPointers)[funcId] = pNewRealFunction;
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        csInitializeMacOSXOpenCLInterception
// Description: Initializes the Interception for all monitored OpenCL functions.
// Return Val:  bool - Success / failure.
// Author:      Uri Shomroni
// Date:        22/1/2009
// ---------------------------------------------------------------------------
bool csInitializeMacOSXOpenCLInterception()
{
    bool retVal = true;

    // Iterate all the OpenGLfunctions by their index:
    for (unsigned int i = apFirstOpenCLFunctionIndex; i <= apLastOpenCLBaseFunctionIndex; i++)
    {
        bool functionOk = csInitializeMacOSXOpenCLInterception((apMonitoredFunctionId)i);
        retVal = retVal && functionOk;
    }

    GT_RETURN_WITH_ASSERT(retVal);
}

