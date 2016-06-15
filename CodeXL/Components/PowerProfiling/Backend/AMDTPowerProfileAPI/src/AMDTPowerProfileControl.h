//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file AMDTPowerProfileControl.h
///
//==================================================================================

#ifndef _AMDTPOWERPROFILECONTROL_H_
#define _AMDTPOWERPROFILECONTROL_H_
#include <AMDTPowerProfileInternal.h>
#include <AMDTPowerProfileDataTypes.h>

#include <wchar.h>

// Smu definitions
// SMU Message Commands used to START & SAMPLE the PM status logging
#define SMU_PM_STATUS_LOG_START    0x170
#define SMU_PM_STATUS_LOG_SAMPLE   0x171

// SMU channel-0 Message/Argument/Response registers for SMU/Driver communication
#define SMU7_SMC_MESSAGE_0         0x250
#define SMU7_SMC_RESPONSE_0        0x254
#define SMU7_SMC_MESSAGE_ARG_0     0x290

// Indirect access index/data pair address channel 2
#define SMC_IND_INDEX_2            0x210
#define SMC_IND_DATA_2             0x214

// SMU soft registers
#define SMU7_CU0_PWR               0x3FE58
#define SMU7_CU1_PWR               0x3FE88
#define SMU7_CU0_CALC_TEMP         0x3FE44
#define SMU7_CU1_CALC_TEMP         0x3FE74
#define SMU7_CU0_MEAS_TEMP         0x3FE38
#define SMU7_CU1_MEAS_TEMP         0x3FE64
#define SMU7_GPU_PWR               0x3FE2C
#define SMU7_PCIE_PWR              0x3FF4C
#define SMU7_DDR_PWR               0x3FF44
#define SMU7_DISPLAY_PWR           0x3FF48
#define SMU7_PACKAGE_PWR           0x3FF7C
#define SMU7_GPU_CALC_TEMP         0x3FE18
#define SMU7_GPU_MEAS_TEMP         0x3FE08
#define SMU7_SCLK                  0x3FF34
#define SMU7_VOLT_VDDC_LOAD        0x3FEA8   //SMU_PM_STATUS_42
#define SMU7_CURR_VDDC             0x3FEAC   //SMU_PM_STATUS_43
#define SMU7_VOLT_VDDC_LOAD_CALC   0x3FE9C   //SMU_PM_STATUS_39
#define SMU7_CURR_VDDC_CALC        0x3FE94   //SMU_PM_STATUS_37

#define MAPPED_BASE_OFFSET         0x608
#define IGPU_PCI_BASE_ADDRESS      0x80000824

#define SMU8_SMC_MESSAGE_0         0x724
#define SMU8_SMC_RESPONSE_0        0x764
#define SMU8_SMC_MESSAGE_ARG_0     0x7a4

// SMU8 AGM table access ids
#define SMU8_TESTSMC_MSG_RequestDataTable 0x2C
#define SMU8_TESTSMC_MSG_ReleaseDataTable 0x2D


#define MAX_BUS_CNT      256
#define MAX_PCIE_DEVICE_CNT   32
#define MAX_FUNCTION_CNT 8

/** AMDTPwrProfilingState: This enum represents the current state of the profiler.
    \ingroup profiling
*/
enum AMDTPwrProfilingState
{
    PowerProfilingUnavailable, /**< The profiler is not available */
    PowerProfilingInitialized, /**< The profiler is initialized */
    PowerProfilingStopped,     /**< The profiler is not currently profiling */
    PowerProfiling,            /**< The profiler is currently profiling */
    PowerProfilingPaused,      /**< The profiler is currently paused */
    PowerProfilingAborted      /**< The profiler encountered an error and aborted the profile*/
};

//AMDTPwrProfileInitParam: Initialization parameter
struct AMDTPwrProfileInitParam
{
    AMDTUInt32 isOnline: 1;      //Online or Offline mode
    AMDTUInt32 pad: 31;          //Reserved for future use
    AMDTUInt32 flag;             //Reserved for future use
};

//AMDTPwrAttributeProperty : Detailed property of each attributes
enum AMDTPwrAttributeProperty
{
    AMDTPWR_ATTRIBUTE_NAME,             // The attribute's name (should be a short string)
    AMDTPWR_ATTRIBUTE_DESCRIPTION,      // The attribute's description (should be a long string)
    AMDTPWR_ATTRIBUTE_TYPE,             // The attribute's data type
    AMDTPWR_ATTRIBUTE_UNIT_TYPE,        // The attribute's unit type
    AMDTPWR_ATTRIBUTE_CATEGORY,
    AMDTPWR_ATTRIBUTE_INSTANCE_TYPE     // per Core/CU/single attribute
};

//MemoryPool: create memory pool for internal use
typedef struct MemoryPool
{
    AMDTUInt8* m_pBase;
    AMDTUInt32 m_offset;
    AMDTUInt32 m_size;
} MemoryPool;

typedef struct
{
    AMDTUInt32 m_bus;
    AMDTUInt32 m_dev;
    AMDTUInt32 m_func;
} PciPortAddress;
/****************************************************************************/


/** This API will initialize the power profile according to the parameter passed by PowerProfileInitParam
    \ingroup profiling
    @param[in] pParam to set the parameters for initialization
    \return The success of enabling the profiler
    \retval AMDT_STATUS_OK Success
    \retval AMDT_ERROR_FAIL The profiling has already been enabled
    \retval AMDT_ERROR_ACCESSDENIED The profiler was not available
    \retval AMDT_ERROR_UNEXPECTED There was an unexpected error
*/
AMDTResult PwrProfileInitialize(
    AMDTPwrProfileInitParam* pParam);

/** This API will close the power profiler and unregister driver and cleanup all
      memory allocated during AMDTPwrProfileInitialize

     \ingroup profiling
    @param[in] pParam to set the parameters for initialization
    \return The success of enabling the profiler
    \retval AMDT_STATUS_OK Success
    \retval AMDT_ERROR_FAIL if the profile is not initialized.
    \retval AMDT_STATUS_PENDING if the profile is currently running
    \retval AMDT_ERROR_ACCESSDENIED The profiler was not available
    \retval AMDT_ERROR_UNEXPECTED There was an unexpected error
*/
AMDTResult PwrProfileClose();

/** This function provides provides the selected attribute's string paramenter supported.
       for example : AMDTPWR_ATTRIBUTE_NAME and AMDTPWR_ATTRIBUTE_DESCRIPTION
        \ingroup profiling
    @param[in] attributeIndex attribute index in the supported list.
    @param[in] attributeProperty attribute property.
    @param[out] ppPropertyValue attribute property value.
    be NULL also
    \retval AMDT_STATUS_OK Success
    \retval AMDT_ERROR_INVALIDARG if any of the parameters are invalid
    \retval AMDT_ERROR_UNEXPECTED an unexpected error occurred
*/
AMDTResult PwrGetAttributeStringParameter(
    /*in*/AMDTUInt32 attributeIndex,
    /*in*/AMDTPwrAttributeProperty attributeProperty,
    /*out*/wchar_t** ppPropertyValue);

AMDTResult PwrRegisterClient(AMDTUInt32* clientId);

/** This function set a list profile configuration selected by the user.
    \ingroup profiling
    @param[in] pConfig pointer to the power configuration list
    \return The success of setting the configuration
    \retval AMDT_STATUS_OK Success
    \retval AMDT_ERROR_FAIL \ref wrong/ unsupported selection of the profile configuration
    to set the requested config(s)
    \retval AMDT_ERROR_ACCESSDENIED if the profile was not enabled with \ref
    AMDTPwrProfileInitialize
    \retval AMDT_STATUS_PENDING if the profile is currently running
    \retval AMDT_ERROR_INVALIDARG if pConfig is NULL or invalid profile config value passed
    \retval AMDT_ERROR_UNEXPECTED an unexpected error occurred
*/

AMDTResult PwrSetProfilingConfiguration(
    /*in*/ AMDTPwrProfileConfig* pConfig);

/** If the profiler is not running, this will start the profiler, and cause the
    state to go to \ref ProfilingRunning.  It will start writing data onto the
    raw file/buffer as per the configuration set

    \ingroup profiling
    \return The success of running the profile
    \retval AMDT_STATUS_OK the profiling was successfully activated and running
    \retval AMDT_STATUS_PENDING the profiler was already running
    \retval AMDT_ERROR_UNEXPECTED an unexpected error occurred
*/
AMDTResult PwrStartProfiling();

/** If the profiler is running, this will stop the profiler, and cause the
    state to go to \ref ProfilingStopped.  If sampling, no further data will
    be written to the profile buffer/file.

    \ingroup profiling
    \return The success of stopping the profile
    \retval AMDT_STATUS_OK the profiling was successfully stopped
    \retval AMDT_ERROR_ACCESSDENIED the profiler was not running
    \retval AMDT_ERROR_UNEXPECTED an unexpected error occurred
*/
AMDTResult PwrStopProfiling();

/** If the profiler is running, this will pause the profiler, and cause the
    state to go to \ref ProfilingPaused.  If sampling, no further data will
    be written to the profile files/buffer until fnResumePowerProfile is called.
    Unlike AMDTPwrStopProfiling, AMDTPwrPauseProfiling will written the previous
    data in the buffer and star appending new data when AMDTPwrResumeProfiling.

    \ingroup profiling
    \return The success of pausing the profile
    \retval AMDT_STATUS_OK the profiling was successfully paused
    \retval AMDT_ERROR_ACCESSDENIED the profiler was not running
    \retval AMDT_ERROR_UNEXPECTED an unexpected error occurred
*/
AMDTResult PwrPauseProfiling();

/** If the profiler is in paused state, this will resume the profiler, and cause the
    state to go to \ref ProfilingRunning.  It will start writing data onto the
    raw file/buffer as per the configuration set. Unlike AMDTPwrStartProfiling,
    where a fresh buffer is taken, AMDTPwrResumeProfiling will start writting onto
    the buffer at the same location where it ends before calling AMDTPwrPauseProfiling.

    \ingroup profiling
    \return The success of resuming the profile
    \retval AMDT_STATUS_OK if success
    \retval AMDT_ERROR_ACCESSDENIED the profiler was not running
    \retval AMDT_ERROR_UNEXPECTED an unexpected error occurred
*/
AMDTResult PwrResumeProfiling();

/** This API provides the current state of the profile.

    \ingroup profiling
    \return The success of getting the profile state
    \retval AMDT_STATUS_OK is success
    \retval AMDT_ERROR_UNEXPECTED an unexpected error occurred
*/
AMDTResult  PwrGetProfilingState(
    /*out*/AMDTPwrProfilingState* pState);

/** This API set the current state of the profile.

    \ingroup profiling
    \return The success of getting the profile state
    \retval AMDT_STATUS_OK is success
    \retval AMDT_ERROR_UNEXPECTED an unexpected error occurred
*/
AMDTResult PwrSetProfilingState(
    /*in*/AMDTPwrProfilingState state);

//helper functions
bool IsIGPUAvailable();

AMDTResult PwrGetSupportedAttributeList(
    /*out*/AMDTPwrProfileAttributeList* pList);

/** This would provide the target system information where profile was run

    \ingroup profiling
    @param[out] pSystemInfo pointer to the target system information structure
    \return The success of setting the configuration
    \retval AMDT_STATUS_OK Success
    \retval AMDT_ERROR_INVALIDARG if pSystemInfo is NULL
    \retval AMDT_ERROR_UNEXPECTED an unexpected error occurred
*/
AMDTResult AMDTPwrGetTargetSystemInfo(
    /*in*/ AMDTPwrTargetSystemInfo* pSystemInfo);

// IsCefSupported: Check from cpuid if core effective frequency is supported
bool IsCefSupported(void);

// Create memory pool
AMDTResult CreateMemoryPool(MemoryPool* pPool, AMDTUInt32 size);
// Get buffer from the pool
AMDTUInt8* GetMemoryPoolBuffer(MemoryPool* pPool, AMDTUInt32 size);
// Delete the memory pool
AMDTResult ReleaseMemoryPool(MemoryPool* pPool);

// GetPciDeviceInfo: Device name, vendor id etc and port address
void GetPciDeviceInfo(AMDTUInt32 pkgId, PciDeviceInfo** ppDevInfo, PciPortAddress** ppAddress);


// GetApuPStateInfo: Function to access Apu pstate list information
AMDTPwrApuPstateList* GetApuPStateInfo();

// IsDgpuMMIOAccessible: For AMD dGPU get the command register and
// check MMIO access bit is set
bool IsDgpuMMIOAccessible(AMDTUInt32 bus, AMDTUInt32 dev, AMDTUInt32 func);

// PwrIsSVISupported: Check if SVI2 is supported
bool PwrIsSVISupported(const AMDTPwrTargetSystemInfo& sysInfo);

#endif //_AMDTPOWERPROFILECONTROL_H_

