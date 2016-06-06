
#ifndef __CALDDI_H__
#define __CALDDI_H__

#include "cal.h"
#include "calcl.h"
#include "cal_ext.h"
#include "calcl_ext.h"

#ifdef __cplusplus
extern "C" {
#define CALAPI
#else
#define CALAPI extern
#endif

#ifdef _WIN32
#define CALAPIENTRY  __stdcall
#else
#define CALAPIENTRY
#endif

// DDI VERSION number
// Any time the exported api changes, this needs to be updated
// and the calddiInit needs to be updated to control backward
// compatibility
#define CALDDI_VERSION 1

CALAPI int CALAPIENTRY calddiInit(unsigned int ddi_version);

CALAPI void* CALAPIENTRY calddiGetExport(unsigned int export_num);

CALAPI unsigned int CALAPIENTRY calddiGetVersion();

typedef enum DDIresultEnum {
    DDI_RESULT_OK                = 0, /**< No error */
    DDI_RESULT_ERROR             = 1, /**< Operational error */
    DDI_RESULT_INVALID_PARAMETER = 2, /**< Parameter passed in is invalid */
    DDI_RESULT_NOT_SUPPORTED     = 3, /**< Function used properly but currently not supported */
    DDI_RESULT_ALREADY           = 4, /**< Stateful operation requested has already been performed */
    DDI_RESULT_NOT_INITIALIZED   = 5, /**< CAL function was called without CAL being initialized */
    DDI_RESULT_BAD_HANDLE        = 6, /**< A handle parameter is invalid */
    DDI_RESULT_BAD_NAME_TYPE     = 7, /**< A name parameter is invalid */
    DDI_RESULT_PENDING           = 8, /**< An asynchronous operation is still pending */
    DDI_RESULT_BUSY              = 9,  /**< The resource in question is still in use */
    DDI_RESULT_WARNING           = 10, /**< Compiler generated a warning */
    DDI_RESULT_INVALID_THREAD    = 11, /**< CAL context is in the incorrect thread*/
} DDIresult;

typedef struct calddi_ifRec {
    DDIresult (CALAPIENTRY *ddiifInit) (void);                                                              // export 1
    DDIresult (CALAPIENTRY *ddiifGetVersion)(unsigned int* major, unsigned int* minor, unsigned int* imp);  // export 2
    DDIresult (CALAPIENTRY *ddiifShutdown)(void);                                                           // export 3
    DDIresult (CALAPIENTRY *ddiifDeviceGetCount)(CALuint* count);                                           // export 4
    DDIresult (CALAPIENTRY *ddiifDeviceGetInfo)(CALdeviceinfo* info, CALuint ordinal);                      // export 5
    DDIresult (CALAPIENTRY *ddiifDeviceGetAttribs)(CALdeviceattribs* attribs, CALuint ordinal);             // export 6
    DDIresult (CALAPIENTRY *ddiifDeviceGetStatus)(CALdevicestatus* status, CALdevice device);               // export 7
    DDIresult (CALAPIENTRY *ddiifDeviceOpen)(CALdevice* dev, CALuint ordinal);                              // export 8
    DDIresult (CALAPIENTRY *ddiifDeviceClose)(CALdevice dev);                                               // export 9
    DDIresult (CALAPIENTRY *ddiifResAllocLocal2D)(CALresource* res, CALdevice dev, CALuint width, CALuint height, CALformat format, CALuint flags);  // export 10
    DDIresult (CALAPIENTRY *ddiifResAllocRemote2D)(CALresource* res, CALdevice *dev, CALuint deviceCount, CALuint width, CALuint height, CALformat format, CALuint flags);  // export 11
    DDIresult (CALAPIENTRY *ddiifResAllocLocal1D)(CALresource* res, CALdevice dev, CALuint width, CALformat format, CALuint flags);  // export 12
    DDIresult (CALAPIENTRY *ddiifResAllocRemote1D)(CALresource* res, CALdevice* dev, CALuint deviceCount, CALuint width, CALformat format, CALuint flags); // export 13
    DDIresult (CALAPIENTRY *ddiifResFree)(CALresource res);                                                 // export 14
    DDIresult (CALAPIENTRY *ddiifResMap)(CALvoid** pPtr, CALuint* pitch, CALresource res, CALuint flags);   // export 15
    DDIresult (CALAPIENTRY *ddiifResUnmap)(CALresource res);                                                // export 16
    DDIresult (CALAPIENTRY *ddiifCtxCreate)(CALcontext* ctx, CALdevice dev);                                // export 17
    DDIresult (CALAPIENTRY *ddiifCtxDestory)(CALcontext ctx);                                               // export 18
    DDIresult (CALAPIENTRY *ddiifCtxGetMem)(CALmem* mem, CALcontext ctx, CALresource res);                  // export 19
    DDIresult (CALAPIENTRY *ddiifCtxReleaseMem)(CALcontext ctx, CALmem mem);                                // export 20
    DDIresult (CALAPIENTRY *ddiifCtxSetMem)(CALcontext ctx, CALname name, CALmem mem);                      // export 21
    DDIresult (CALAPIENTRY *ddiifCtxRunProgram)(CALevent* event, CALcontext ctx, CALfunc func, const CALdomain* domain);  // export 22
    DDIresult (CALAPIENTRY *ddiifCtxIsEventDone)(CALcontext ctx, CALevent event);                           // export 23
    DDIresult (CALAPIENTRY *ddiifCtxFlush)(CALcontext ctx);                                                 // export 24
    DDIresult (CALAPIENTRY *ddiifMemCopy)(CALevent* event, CALcontext ctx, CALmem srcMem, CALmem dstMem, CALuint flags);  // export 25
    DDIresult (CALAPIENTRY *ddiifImageRead)(CALimage *image, const CALvoid* buffer, CALuint size);          // export 26
    DDIresult (CALAPIENTRY *ddiifImageFree)(CALimage image);                                                // export 27
    DDIresult (CALAPIENTRY *ddiifModuleLoad)(CALmodule* module, CALcontext ctx, CALimage image);            // export 28
    DDIresult (CALAPIENTRY *ddiifModuleUnload)(CALcontext ctx, CALmodule module);                           // export 29
    DDIresult (CALAPIENTRY *ddiifModuleGetEntry)(CALfunc* func, CALcontext ctx, CALmodule module, const CALchar* procName); // export 30
    DDIresult (CALAPIENTRY *ddiifModuleGetName)(CALname* name, CALcontext ctx, CALmodule module, const CALchar* varName);   // export 31
    const char*(CALAPIENTRY *ddiifGetErrorString)();                                                        // export 32
    DDIresult (CALAPIENTRY *ddiifCtxRunProgramGrid)(CALevent* event,CALcontext ctx,CALprogramGrid* pProgramGrid);   // export 33
    DDIresult (CALAPIENTRY *ddiifModuleGetFuncInfo)(CALfuncInfo* pInfo, CALcontext   ctx, CALmodule    module, CALfunc      func);   // export 34
    DDIresult (CALAPIENTRY *ddiifCtxRunProgramGridArray)(CALevent* event,CALcontext ctx,CALprogramGridArray* pGridArray);   // export 35
    DDIresult (CALAPIENTRY *ddiifExtSupported)(CALextid extid);                                             // export 36
    DDIresult (CALAPIENTRY *ddiifExtGetVersion)(CALuint* major, CALuint* minor, CALextid extid);            // export 37
    DDIresult (CALAPIENTRY *ddiifExtGetProc)(CALextproc* proc, CALextid extid, const CALchar* procname);    // export 38
    DDIresult (CALAPIENTRY *ddiifCompile)(CALobject*  obj,  CALlanguage lanEnum, const CALchar*    source, CALtarget   target);   // export 39
    DDIresult (CALAPIENTRY *ddiifLink)(CALimage*  image, CALobject* obj, CALuint objCount);                 // export 40
    DDIresult (CALAPIENTRY *ddiifFreeObject)(CALobject obj);                                                // export 41
    DDIresult (CALAPIENTRY *ddiifFreeImage)(CALimage image);                                                // export 42
    void      (CALAPIENTRY *ddiifDisassembleImage)(const CALimage image, CALLogFunction logfunc);           // export 43
    DDIresult (CALAPIENTRY *ddiifAssembleObject)(CALobject* obj, CALCLprogramType programType, const CALchar* source,  CALtarget target);   // export 44
    void      (CALAPIENTRY *ddiifDisassembleObject)(const CALobject* obj, CALLogFunction logfunc);          // export 45
    DDIresult (CALAPIENTRY *ddiifImageGetSize)(CALuint* size, CALimage image);                              // export 46
    DDIresult (CALAPIENTRY *ddiifImageWrite)(CALvoid* buffer, CALuint size, CALimage image);                // export 47
    const char* (CALAPIENTRY *ddiifclGetErrorString)();                                                     // export 48
    DDIresult (CALAPIENTRY *ddiifConfig)(const CALchar* key, const CALchar* value);                         // export 49
    CALvoid   (CALAPIENTRY *ddiifClearConfig)();                                                            // export 50
    DDIresult (CALAPIENTRY *ddiifAssemble)(CALobject* obj, CALlanguage language, CALCLprogramType programType, const CALchar* source, CALtarget target);   // export 51
    void      (CALAPIENTRY *ddiifDisassemble)(const CALobject* obj, CALlanguage language, CALLogFunction logfunc);   // export 52
    DDIresult (CALAPIENTRY *ddiifclExtGetProc)(CALCLextproc* proc, CALCLextid extid, const CALchar* procname);   // export 53
    DDIresult (CALAPIENTRY *ddiifclExtSupported)(CALCLextid extid);                                         // export 54
    DDIresult (CALAPIENTRY *ddiifDeviceClockUp)(CALdevice dev, CALuint flag);                                         // export 55
    DDIresult (CALAPIENTRY *ddiifGetFuncInfoFromImage)(CALimage image, CALfuncInfo *pFuncInfo);                                         // export 55
} calddi_if;

#ifdef __cplusplus
}      /* extern "C" { */
#endif

#endif // __CALDDI_H__

