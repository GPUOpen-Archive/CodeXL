//=====================================================================
// Copyright 2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file oaDriver.cpp
///
//=====================================================================

//------------------------------ oaDriver.cpp ------------------------------

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTOSWrappers/Include/osModule.h>

// Local:
#include <AMDTOSAPIWrappers/Include/oaDriver.h>
#include <AMDTOSAPIWrappers/Include/oaStringConstants.h>


#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
// This function is implemented in ADLUtil.cpp on Windows and in linux/oaDriver.cpp on Linux:
void* __stdcall ADL_Main_Memory_Alloc(int iSize);
// ADLUtil:
#include <ADLUtil.h>
#elif AMDT_BUILD_TARGET == AMDT_LINUX_OS
// This function is implemented in ADLUtil.cpp on Windows and in linux/oaDriver.cpp on Linux:
void* ADL_Main_Memory_Alloc(int iSize);

// ADLUtil.h brings a lot of baggage on Linux, just take the relevant definitions:
#define ADL_MAX_PATH 256
typedef struct ADLVersionsInfo
{
    char strDriverVer[ADL_MAX_PATH];
    char strCatalystVersion[ADL_MAX_PATH];
    char strCatalystWebLink[ADL_MAX_PATH];
} ADLVersionsInfo, *LPADLVersionsInfo;
typedef void* ADL_CONTEXT_HANDLE;
typedef void* (*ADL_MAIN_MALLOC_CALLBACK)(int);
typedef int(*ADL_Main_Control_Create_fn)(ADL_MAIN_MALLOC_CALLBACK, int);
typedef int(*ADL_Main_Control_Destroy_fn)();
typedef int(*ADL2_Main_Control_Create_fn)(ADL_MAIN_MALLOC_CALLBACK, int, ADL_CONTEXT_HANDLE*);
typedef int(*ADL2_Main_Control_Destroy_fn)(ADL_CONTEXT_HANDLE);
typedef int(*ADL_Graphics_Versions_Get_fn)(struct ADLVersionsInfo*);
typedef int(*ADL2_Graphics_Versions_Get_fn)(ADL_CONTEXT_HANDLE, struct ADLVersionsInfo*);
#else
#error Unknown Build Target!
#endif

// TO_DO: we need to update the Common/Lib/AMD/ADL folder to have these values from there:
typedef struct ADLVersionsInfoX2
{
    char strDriverVer[ADL_MAX_PATH];
    char strCatalystVersion[ADL_MAX_PATH];
    char strCrimsomVersion[ADL_MAX_PATH];
    char strCatalystWebLink[ADL_MAX_PATH];
} ADLVersionsInfoX2, *LPADLVersionsInfoX2;

typedef int(*ADL2_Graphics_VersionsX2_Get_fn)(ADL_CONTEXT_HANDLE, ADLVersionsInfoX2*);


// ---------------------------------------------------------------------------
// Name:        oaGetDriverVersionfromADLModule
// Description: Gets the driver version from
// Return Val:  oaDriverError
// Author:      AMD Developer Tools Team
// Date:        6/4/2016
// ---------------------------------------------------------------------------
oaDriverError oaGetDriverVersionfromADLModule(osModuleHandle adlModule, gtString& driverVersion)
{
    oaDriverError driverError = OA_DRIVER_UNKNOWN;
    GT_IF_WITH_ASSERT(OS_NO_MODULE_HANDLE != adlModule)
    {
        ADLVersionsInfo driverInfo;
        ADLVersionsInfoX2 driverInfoX2;
        bool gotVersionsInfo = false;
        bool gotVersionsInfoX2 = false;

        // Get the procedure for the adl functions:
        osProcedureAddress vp_adl2MainControlCreate = nullptr;
        osProcedureAddress vp_adl2GraphicsVersionsGet = nullptr;
        osProcedureAddress vp_adl2GraphicsVersionsX2Get = nullptr;
        osProcedureAddress vp_adl2MainControlDestroy = nullptr;

        bool rc1 = osGetProcedureAddress(adlModule, OA_STR_ADL2_DRIVER_CREATE_FUNCTION, vp_adl2MainControlCreate, false);
        bool rc2 = osGetProcedureAddress(adlModule, OA_STR_ADL2_DRIVER_VERSION_FUNCTION, vp_adl2GraphicsVersionsX2Get, false);
        bool rc3 = osGetProcedureAddress(adlModule, OA_STR_ADL2_DRIVER_VERSION_FUNCTION_LEGACY, vp_adl2GraphicsVersionsGet, false);
        bool rc4 = osGetProcedureAddress(adlModule, OA_STR_ADL2_DRIVER_DESTROY_FUNCTION, vp_adl2MainControlDestroy, false);

        if (rc1 && (rc2 || rc3) && rc4 &&
            (nullptr != vp_adl2MainControlCreate) &&
            ((nullptr != vp_adl2GraphicsVersionsX2Get) || (nullptr != vp_adl2GraphicsVersionsGet)) &&
            (nullptr != vp_adl2MainControlDestroy))
        {
            // Init ADL2:
            ADL_CONTEXT_HANDLE hADL = nullptr;
            ADL2_Main_Control_Create_fn pfn_adl2MainControlCreate = (ADL2_Main_Control_Create_fn)vp_adl2MainControlCreate;
            pfn_adl2MainControlCreate(ADL_Main_Memory_Alloc, 1, &hADL);

            // Check the version:
            if (nullptr != vp_adl2GraphicsVersionsX2Get)
            {
                ADL2_Graphics_VersionsX2_Get_fn pfn_adl2GraphicsVersionsX2Get = (ADL2_Graphics_VersionsX2_Get_fn)vp_adl2GraphicsVersionsX2Get;
                unsigned int result = pfn_adl2GraphicsVersionsX2Get(hADL, &driverInfoX2);

                // When the result is 1, it means that we have a warning, but we can ignore it:
                GT_IF_WITH_ASSERT((result == 0) || (result == 1))
                {
                    gotVersionsInfoX2 = true;
                }
            }

            if (nullptr != vp_adl2GraphicsVersionsGet)
            {
                ADL2_Graphics_Versions_Get_fn pfn_adl2GraphicsVersionsGet = (ADL2_Graphics_Versions_Get_fn)vp_adl2GraphicsVersionsGet;
                unsigned int result = pfn_adl2GraphicsVersionsGet(hADL, &driverInfo);

                // When the result is 1, it means that we have a warning, but we can ignore it:
                GT_IF_WITH_ASSERT((result == 0) || (result == 1))
                {
                    gotVersionsInfo = true;
                }
            }

            // Destroy ADL2:
            ADL2_Main_Control_Destroy_fn pfn_adl2MainControlDestroy = (ADL2_Main_Control_Destroy_fn)vp_adl2MainControlDestroy;
            pfn_adl2MainControlDestroy(hADL);
        }
        else
        {
            // Get the procedure for the adl functions:
            osProcedureAddress vp_adlMainControlCreate = nullptr;
            osProcedureAddress vp_adlGraphicsVersionsGet = nullptr;
            osProcedureAddress vp_adlMainControlDestroy = nullptr;

            bool rc5 = osGetProcedureAddress(adlModule, OA_STR_ADL_DRIVER_CREATE_FUNCTION, vp_adlMainControlCreate);
            bool rc6 = osGetProcedureAddress(adlModule, OA_STR_ADL_DRIVER_VERSION_FUNCTION, vp_adlGraphicsVersionsGet);
            bool rc7 = osGetProcedureAddress(adlModule, OA_STR_ADL_DRIVER_DESTROY_FUNCTION, vp_adlMainControlDestroy);
            GT_IF_WITH_ASSERT(rc5 && rc6 && rc7 && (nullptr != vp_adlGraphicsVersionsGet) && (nullptr != vp_adlMainControlCreate) && (nullptr != vp_adlMainControlDestroy))
            {
                // Init ADL:
                ADL_Main_Control_Create_fn pfn_adlMainControlCreate = (ADL_Main_Control_Create_fn)vp_adlMainControlCreate;
                pfn_adlMainControlCreate(ADL_Main_Memory_Alloc, 1);

                // Check the version:
                ADL_Graphics_Versions_Get_fn pfn_adlGraphicsVersionsGet = (ADL_Graphics_Versions_Get_fn)vp_adlGraphicsVersionsGet;
                unsigned int result = pfn_adlGraphicsVersionsGet(&driverInfo);

                // When the result is 1, it means that we have a warning, but we can ignore it:
                GT_IF_WITH_ASSERT((result == 0) || (result == 1))
                {
                    gotVersionsInfo = true;
                }

                // Destroy ADL:
                ADL_Main_Control_Destroy_fn pfn_adlMainControlDestroy = (ADL_Main_Control_Destroy_fn)vp_adlMainControlDestroy;
                pfn_adlMainControlDestroy();
            }
        }

        GT_IF_WITH_ASSERT(gotVersionsInfo || gotVersionsInfoX2)
        {
            if (gotVersionsInfoX2)
            {
                driverVersion.fromASCIIString(driverInfoX2.strCatalystVersion);

                if (driverVersion.isEmpty())
                {
                    driverVersion.fromASCIIString(driverInfoX2.strCrimsomVersion);
                }
            }

            if (driverVersion.isEmpty() && gotVersionsInfo)
            {
                driverVersion.fromASCIIString(driverInfo.strCatalystVersion);
            }

            if (driverVersion.isEmpty())
            {
                driverError = OA_DRIVER_VERSION_EMPTY;
            }
            else
            {
                driverError = OA_DRIVER_OK;
            }
        }
    }

    return driverError;
}
