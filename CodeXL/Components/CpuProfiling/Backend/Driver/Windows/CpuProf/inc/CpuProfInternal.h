//=============================================================
// (c) 2012 Advanced Micro Devices, Inc.
//
/// \author franksw
/// \version $Revision: $
/// \brief  The internal definition file for the driver.
//
//=============================================================
// $Id: $
// Last checkin:   $DateTime: $
// Last edited by: $Author: $
// Change list:    $Change: $
//=============================================================

#ifndef _CPUPROF_INTERNAL_H_
#define _CPUPROF_INTERNAL_H_

#pragma warning(push)
#pragma warning(disable:4200) // zero length array
#pragma warning(disable:4201) // nameless struct/union
#pragma warning(disable:4214) // bit field types other than int

#include "CpuProfCommon.hpp"

/// \def CPUPROF_DEVICE_NAME The device name string
#define CPUPROF_DEVICE_NAME L"CpuProf0"
/// \def NT_DEVICE_NAME The device object name string
#define NT_DEVICE_NAME L"\\Device\\CpuProf0"
/// \def DOS_DEVICE_NAME The symbolic link name string
#define DOS_DEVICE_NAME L"\\??\\CpuProf0"

/// \def CPU_PROF_SHARED_OBJ_BASE The shared object that various instances of
/// Cpu profilers will use to signal a pause state during sampling
#define CPU_PROF_SHARED_OBJ_BASE L"\\BaseNamedObjects\\Global\\AMD_CPUPROF_SHARED_OBJ"

/// \def CLIENT_BUFFER_COUNT The default number of sampling data buffers that
/// are allocated
#define CLIENT_BUFFER_COUNT 128

/// \def MSR_CTL_SAMPLE_BIT This bit (EventSelect [20] Int: enable APIC
/// interrupt) is set in the configuration.msrControlValue if the event
/// configuration is for sampling
#define MSR_CTL_SAMPLE_BIT 0x00100000

/// \def MSR_IBS_OPS_DISPATCH_BIT This bit (IbsOpCtl [19] IbsOpCntCtl) is set
/// in the configuration.msrControlValue if the IBS_OP configuration uses
/// dispatched ops instead of cycle counts
#define MSR_IBS_OPS_DISPATCH_BIT (1 << 19)

/// EventSelect source bits [7:5] are used internally to encode the TLM that
/// controls the performance counter.
typedef enum
{
    /// Floating point events
    FP_SRC_BITS = 0,
    /// Load/Store events
    LS_SRC_BITS = 0x10,
    /// Data cache events
    DC_SRC_BITS = 0x10,
    /// L2 Cache and system interface events
    CU_SRC_BITS = 0x10,
    /// Instruction cache events
    IC_SRC_BITS = 0x10,
    /// Microcode engine and decode engine events
    ME_SRC_BITS = 0x10,
    /// Execution unit events
    EX_SRC_BITS = 0x10,
    /// Northbridge events
    NB_SRC_BITS = 0xE0
};


/// \def NB_DATA_MASK Bits [5:0] of the IBS_OP value[VALUE_OP_DATA_2] used to
/// hold NorthBridge data
#define NB_DATA_MASK 0x3F

/// \def FETCH_CTL_EXTD_MASK Bits [15:0] of MSRC001_103C
#define FETCH_CTL_EXTD_MASK 0xFFFF

/// \def OP_DATA_4_MASK Bit [0] of MSRC001_103D
#define OP_DATA_4_MASK 0x1


/// \def EFLAGS_RX_BIT_MASK The RX bit for 64-bit mode EFLAGS
#define EFLAGS_RX_BIT_MASK      0x80000000
/// \def EFLAGS_VM_BIT_MASK The VM bit for 64-bit mode EFLAGS
#define EFLAGS_VM_BIT_MASK      0x00020000
/// \def DESCRIPTOR_L_BIT_MASK The L bit for 64-bit mode rCS
#define DESCRIPTOR_L_BIT_MASK   0x0020000000000000
/// \def DESCRIPTOR_D_BIT_MASK The D bit for 64-bit mode rCS
#define DESCRIPTOR_D_BIT_MASK   0x0040000000000000

// ***************************************************************************
// [MajorFunction] Type Function Prototypes

//  ==========================================================================
/// CpuProf driver unloaded.  All allocated memory and resources are freed.
///
/// \param[in] pDriverObject Pointer to the driver object.
///
// ===========================================================================
DRIVER_UNLOAD CpuProfUnload;

//  ==========================================================================
/// CpuProf device creation.  Unused
///
/// \param[in] pDeviceObject Pointer to the device object.
/// \param[in] Irp Pointer to the current IRP.
///
/// \return STATUS_SUCCESS
///
// ===========================================================================
__drv_dispatchType(IRP_MJ_CREATE) DRIVER_DISPATCH CpuProfCreate;

//  ==========================================================================
/// CpuProf device closing.  Don't need to do anything because \ref
/// CpuProfCleanup will handle any client registrations left open
///
/// \param[in] pDeviceObject Pointer to the device object.
/// \param[in] Irp Pointer to the current IRP.
///
/// \return STATUS_SUCCESS
///
// ===========================================================================
__drv_dispatchType(IRP_MJ_CLOSE) DRIVER_DISPATCH CpuProfClose;

//  ==========================================================================
/// CpuProf IOCTL handler.  Handles all IOCTLS specified in \ref cadddef.h
///
/// \param[in] pDeviceObject Pointer to the device object.
/// \param[in] Irp Pointer to the current IRP.
///
/// \return Return status of the IOCTL handler
///
// ===========================================================================
__drv_dispatchType(IRP_MJ_DEVICE_CONTROL) DRIVER_DISPATCH CpuProfDispatch;

//  ==========================================================================
/// CpuProf cleanup function.  Called when the user application closes the
/// driver or it crashes
///
/// \param[in] pDeviceObject Pointer to the device object.
/// \param[in] Irp Pointer to the current IRP.
///
/// \return Return status of the IOCTL handler
///
// ===========================================================================
__drv_dispatchType(IRP_MJ_CLEANUP) DRIVER_DISPATCH CpuProfCleanup;


#pragma warning(pop)

#endif // _CPUPROF_INTERNAL_H_
