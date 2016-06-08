**** AMD OpenCL Debug Library Release Notes ****

Libraries are supplied in release and debug builds for x86 and x86_64 for both
Windows and Linux. The Windows versions were compiled with Visual Studio 2010.
The Linux versions were compiled with GCC on CentOS 6.2.

The libraries are for internal AMD use and NDA'ed partner use only.

If you wish to distribute these libraries to other groups within AMD or
externally please contact callan.mcinally@amd.com.

The functionality of the libraries is described in AMDOpenCLDebug.doc.
Please email bug reports to gputools.support@amd.com.


**** System Requirements ****

- Tested on: Redwood, Juniper, Cypress, Cayman, Tahiti, Bonaire and Tonga GPUs.
             Loveland, BeaverCreek and Devastator APUs.
  Library should work on any EG/NI/SI/CI/VI GPU configuration supported by driver.

- Catalyst 11.11 or any more recent version.

**** Changes from Win Build 1711, Linux Build 1666 ****

- added amdclInterceptCreateCommandQueueWithProperties().
- amdclDebugEnqueueNDRangeKernel now validates that the queue has the command intercept flag.

**** Changes from Win Build 1559, Linux Build 1505 ****

- cl_icd_amd.h no longer included in zip file or tarball. Use the version from
  the most recent APPSDK version in Common\Lib\AMD\APPSDK

**** Changes from Win Build 1510, Linux Build 1458 ****

- SC Updates, Bug fixes

**** Changes from Win Build 1498, Linux Build 1446 ****

- Bug fixes

**** Changes from Win Build 1487, Linux Build 1435 ****

- Bug fixes, performance optimization

**** Changes from Win Build 1482, Linux Build 1430 ****

- Support Compiler lib for IL text and binary translation


**** Changes from Win Build 1465, Linux Build 1412 ****

- Support OpenCL image array, support 64 bit read atomic and bug fixes

**** Changes from Win Build 1445, Linux Build 1389 ****

- Support Bonaire, improve performance and bug fixes

**** Changes from Win Build 1409, Linux Build 1324 ****

- Support Catalyst 12.9 Beta, bug fixes

**** Changes from Win Build 1368, Linux Build 1316 ****

- Add amdclDebugUtilGetLastError().

**** Changes from Win Build 1323, Linux Build 1278 ****

- Various bug fixes.

**** Changes from Win Build 1246, Linux Build 1213 ****

- Added support for Scrapper and Devastator.


**** Changes from Win Build 1231, Linux Build 1196 ****

- Minor bug fixes.


**** Changes from Win Build 1176, Linux Build 1159 ****

- Improved logging.

- Various bug fixes.


**** Changes from Win Build 1175, Linux Build 1157 ****

- Metadata check ignores major and minor. Only revision respected.

**** Changes from Win Build 1151, No Linux Build ****

- Enhanced logging.


**** Changes from Win Build 1058, Linux Build 1066 ****

- Full SI support. amdclDebugGetGlobalMemoryValues() is now supported.
  Note extra argument for resource id.

- Added support for OpenCL kernel metadata version 3.0.104.


**** Changes from Build 1049 ****

- Initial SI support. amdclDebugGetGlobalMemoryValues() is not yet supported.

  
**** Changes from Build 1011 ****

- Added support for OpenCL kernel metadata version 3.0.98.

- Various bug fixes.


**** Changes from Build 1007 ****

- Atomic operations that return a value are explicitly not supported. Kernels that
  contain IL_OP_LDS_READ_*, IL_OP_GDS_READ_*, IL_OP_UAV_READ_* or IL_OP_APPEND_BUF_*
  cannot be debugged and an error will be returned.


**** Changes from Build 994 ****

- CL_KERNEL_NOT_DEBUGGABLE_AMD return code added to amdclDebugEnqueueNDRangeKernel().
  See API doc for more details.

- Added support for atomic counter kernel argument type.


**** Changes from Build 939 ****

- Added additional logging to AMDOpenCLDebugAPIUsage.log.

- Added support for OpenCL kernel metadata version 2.0.88.

- Various bug fixes.


**** Changes from Build 911 ****

- Performance optimizations when debugging kernels.

  
**** Changes from Build 880 ****

- Performance optimizations when debugging kernels built with -mem2reg=0.


**** Changes from Build 735 ****

- Updated API to support additional OpenCL memory spaces. See API doc for more details.

- Performance optimizations when debugging kernels built with -mem2reg=0.


**** Changes from Build 709 ****

- Various bug fixes.


**** Changes from Build 697 ****

- CAL functions are now used instead of linking to SC directly.

- Fixed tracking register bug when no tracking registers are set.


**** Changes from Build 676 ****

- Improved debugging performance when stepping through linear code.

- Added utility functions amdclDebugUtilStringToDebugRegisterLocator() and
  amdclDebugUtilDebugRegisterLocatorToString(). See API doc for more details.


**** Changes from Build 651 ****

- Improved out of memory handling.

- Reduced memory usage.

- Improved debugging performance when running to breakpoint.

- Breakpoints within loops are now correctly hit multiple times.

- Registers modified on both sides of if/else blocks now have correct values.


**** Changes from Build 612 ****

- Added amdclDebugGetGlobalMemory() function for retrieving values from global memory.
  See API doc for more details.

- Additional performance improvements and bug fixes.

- Improved multi device support.


**** Changes from Build 589 ****

- Removed all LGPL'ed code to allow distribution outside AMD.

- Improved performance of continue to breakpoint.


**** Changes from Build 578 ****

- CL_DEVICE_NOT_GPU_AMD return code added to amdclDebugEnqueueNDRangeKernel().
  See API doc for more details.

- Using image kernel arguments no longer causes all arguments to appear as zero.

- Added support for kernels that have a global data section.


**** Changes from Build 542 ****

- Added IL switch patching support.

- Added support for IL continue instruction.

- Improved multi device support.


**** Changes from Build 506 ****

- New event passed to user callback: AMDCLDEBUG_EVENT_ERROR. See API doc for more details.

- Fixed crash when kernels are created for a CPU device.

- Added support for kernel attribute reqd_work_group_size


**** Changes from Build 483 ****

- Multiple kernel programs now only recompile the kernel being debugged.


**** Changes from Build 459 ****

- Improved support for multiple GPU configurations.

- Additional function amdclDebugSetTrackingRegisters(). See API doc for more details.


**** Changes from Build 431 ****

- User event support implemented.

- Updated interface for setting breakpoints. See API doc for more details.


**** Changes from Build 360 ****

- Fast "step out of kernel" implemented. Remove all breakpoints and then do a Continue.

- Detection of AMD platform before attempting to debug.

- Additional intercept functions added: amdclInterceptCreateContext(),
  amdclInterceptCreateContextFromType() and amdclInterceptCreateCommandQueue(). These
  must be called in place of the respective CL functions.

- Additional functions amdclDebugGetNumberOfActiveRegisters() and
  amdclDebugGetActiveRegisters(). See API doc for more details.

  
**** Changes from Build 335 ****

- Breakpoints have been fixed.

- Debugging kernels using read/write buffers now produce correct results.


**** Changes from Build 297 ****

- Debugging on contexts that contain multiple devices is supported. Note: untested due to
  CL runtime instability on our multiple device system.

- Kernels that generate macros (such as those that use floating point division) supported.

- Kernel arguments other than buffers supported.

- Programs containing multiple kernels supported.

**** Changes from Build 209 ****

- .debugil section now used for patching.

- Kernels that generate macros (such as those that use floating point division)
  are no longer supported.

- Small changes to API doc.


**** Changes from last time ****

- Loops supported

- Nested flow control supported.

- Kernels that generate macros (such as those that use floating point division) supported.

- Execution mask for first couple of steps is now correct.

- Register locators should match the format used in DWARF.

- amdclDebugGetKernelBinary() now returns debug sections.


**** Known Issues ****

There are a number of known issues:

- OpenCL kernels that use local memory and either a) do not use barriers correctly; or
  b) contain conflicting writes to local memory will result in undefined debugger behaviour.

- Atomic operations that return a value are explicitly not supported. Kernels that
  contain IL_OP_LDS_READ_*, IL_OP_GDS_READ_*, IL_OP_UAV_READ_* or IL_OP_APPEND_BUF_*
  cannot be debugged and an error will be returned. Due to limitations of the current
  debugging method these instructions are unlikely to supported in the near future.
  
- Slightly strange program counter movement at very beginning of code, very end
  and near IL CALL statements.
  
  
Lihan Bin
June 10 2013
