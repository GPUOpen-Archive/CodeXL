//==============================================================================
// Copyright (c) 2011-2016 Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools
/// \file
/// \brief This class manages the dynamic loading of aticalcl, aticaldd, aticalrt.
//==============================================================================

#ifndef _CAL_MODULE_H_
#define _CAL_MODULE_H_

#include <cal.h>
#include <calcl.h>
#include <cal_ext.h>
#include <private/calddi.h>

// These are extracted from a structure definition in calddi.h.
// They are then wrapped up here.
namespace myddi
{
typedef CALresult(CALAPIENTRY* ddiifInit)(void);
typedef CALresult(CALAPIENTRY* ddiifGetVersion)(unsigned int* major, unsigned int* minor, unsigned int* imp);
typedef CALresult(CALAPIENTRY* ddiifShutdown)(void);
typedef CALresult(CALAPIENTRY* ddiifDeviceGetCount)(CALuint* count);
typedef CALresult(CALAPIENTRY* ddiifDeviceGetInfo)(CALdeviceinfo* info, CALuint ordinal);
typedef CALresult(CALAPIENTRY* ddiifDeviceGetAttribs)(CALdeviceattribs* attribs, CALuint ordinal);
typedef CALresult(CALAPIENTRY* ddiifDeviceGetStatus)(CALdevicestatus* status, CALdevice device);
typedef CALresult(CALAPIENTRY* ddiifDeviceOpen)(CALdevice* dev, CALuint ordinal);
typedef CALresult(CALAPIENTRY* ddiifDeviceClose)(CALdevice dev);
typedef CALresult(CALAPIENTRY* ddiifResAllocLocal2D)(CALresource* res, CALdevice dev, CALuint width, CALuint height, CALformat format, CALuint flags);
typedef CALresult(CALAPIENTRY* ddiifResAllocRemote2D)(CALresource* res, CALdevice* dev, CALuint deviceCount, CALuint width, CALuint height, CALformat format, CALuint flags);
typedef CALresult(CALAPIENTRY* ddiifResAllocLocal1D)(CALresource* res, CALdevice dev, CALuint width, CALformat format, CALuint flags);
typedef CALresult(CALAPIENTRY* ddiifResAllocRemote1D)(CALresource* res, CALdevice* dev, CALuint deviceCount, CALuint width, CALformat format, CALuint flags);
typedef CALresult(CALAPIENTRY* ddiifResFree)(CALresource res);
typedef CALresult(CALAPIENTRY* ddiifResMap)(CALvoid** pPtr, CALuint* pitch, CALresource res, CALuint flags);
typedef CALresult(CALAPIENTRY* ddiifResUnmap)(CALresource res);
typedef CALresult(CALAPIENTRY* ddiifCtxCreate)(CALcontext* ctx, CALdevice dev);
typedef CALresult(CALAPIENTRY* ddiifCtxDestroy)(CALcontext ctx);
typedef CALresult(CALAPIENTRY* ddiifCtxGetMem)(CALmem* mem, CALcontext ctx, CALresource res);
typedef CALresult(CALAPIENTRY* ddiifCtxReleaseMem)(CALcontext ctx, CALmem mem);
typedef CALresult(CALAPIENTRY* ddiifCtxSetMem)(CALcontext ctx, CALname name, CALmem mem);
typedef CALresult(CALAPIENTRY* ddiifCtxRunProgram)(CALevent* event, CALcontext ctx, CALfunc func, const CALdomain* domain);
typedef CALresult(CALAPIENTRY* ddiifCtxIsEventDone)(CALcontext ctx, CALevent event);
typedef CALresult(CALAPIENTRY* ddiifCtxFlush)(CALcontext ctx);
typedef CALresult(CALAPIENTRY* ddiifMemCopy)(CALevent* event, CALcontext ctx, CALmem srcMem, CALmem dstMem, CALuint flags);
typedef CALresult(CALAPIENTRY* ddiifImageRead)(CALimage* image, const CALvoid* buffer, CALuint size);
typedef CALresult(CALAPIENTRY* ddiifImageFree)(CALimage image);
typedef CALresult(CALAPIENTRY* ddiifModuleLoad)(CALmodule* module, CALcontext ctx, CALimage image);
typedef CALresult(CALAPIENTRY* ddiifModuleUnload)(CALcontext ctx, CALmodule module);
typedef CALresult(CALAPIENTRY* ddiifModuleGetEntry)(CALfunc* func, CALcontext ctx, CALmodule module, const CALchar* procName);
typedef CALresult(CALAPIENTRY* ddiifModuleGetName)(CALname* name, CALcontext ctx, CALmodule module, const CALchar* varName);
typedef const char* (CALAPIENTRY* ddiifGetErrorString)();
typedef CALresult(CALAPIENTRY* ddiifCtxRunProgramGrid)(CALevent* event, CALcontext ctx, CALprogramGrid* pProgramGrid);
typedef CALresult(CALAPIENTRY* ddiifModuleGetFuncInfo)(CALfuncInfo* pInfo, CALcontext   ctx, CALmodule    module, CALfunc      func);
typedef CALresult(CALAPIENTRY* ddiifCtxRunProgramGridArray)(CALevent* event, CALcontext ctx, CALprogramGridArray* pGridArray);
typedef CALresult(CALAPIENTRY* ddiifExtSupported)(CALextid extid);
typedef CALresult(CALAPIENTRY* ddiifExtGetVersion)(CALuint* major, CALuint* minor, CALextid extid);
typedef CALresult(CALAPIENTRY* ddiifExtGetProc)(CALextproc* proc, CALextid extid, const CALchar* procname);
typedef CALresult(CALAPIENTRY* ddiifCompile)(CALobject*  obj,  CALlanguage lanEnum, const CALchar*    source, CALtarget   target);
typedef CALresult(CALAPIENTRY* ddiifLink)(CALimage*  image, CALobject* obj, CALuint objCount);
typedef CALresult(CALAPIENTRY* ddiifFreeObject)(CALobject obj);
typedef CALresult(CALAPIENTRY* ddiifFreeImage)(CALimage image);
typedef void (CALAPIENTRY* ddiifDisassembleImage)(const CALimage image, CALLogFunction logfunc);
typedef CALresult(CALAPIENTRY* ddiifAssembleObject)(CALobject* obj, CALCLprogramType programType, const CALchar* source,  CALtarget target);
typedef void (CALAPIENTRY* ddiifDisassembleObject)(const CALobject* obj, CALLogFunction logfunc);
typedef CALresult(CALAPIENTRY* ddiifImageGetSize)(CALuint* size, CALimage image);
typedef CALresult(CALAPIENTRY* ddiifImageWrite)(CALvoid* buffer, CALuint size, CALimage image);
typedef const char* (CALAPIENTRY* ddiifclGetErrorString)();
typedef CALresult(CALAPIENTRY* ddiifConfig)(const CALchar* key, const CALchar* value);
typedef CALvoid(CALAPIENTRY* ddiifClearConfig)();
typedef CALresult(CALAPIENTRY* ddiifAssemble)(CALobject* obj, CALlanguage language, CALCLprogramType programType, const CALchar* source, CALtarget target);
typedef void (CALAPIENTRY* ddiifDisassemble)(const CALobject* obj, CALlanguage language, CALLogFunction logfunc);
typedef CALresult(CALAPIENTRY* ddiifclExtGetProc)(CALCLextproc* proc, CALCLextid extid, const CALchar* procname);
typedef CALresult(CALAPIENTRY* ddiifclExtSupported)(CALCLextid extid);
typedef CALresult(CALAPIENTRY* ddiifDeviceClockUp)(CALdevice dev, CALuint flag);
typedef CALresult(CALAPIENTRY* ddiifGetFuncInfoFromImage)(CALimage image, CALfuncInfo* pFuncInfo);

// from aticaldd.dll/so
typedef void*       (CALAPIENTRY* ddiifGetExport)(unsigned int export_num);
};

#include "DynamicLibraryModule.h"

// Table of entry point, type of entry point.
#define CALCL_INTERFACE_TABLE \
    X(AssembleObject) \
    X(Compile) \
    X(DisassembleImage) \
    X(DisassembleObject) \
    X(ExtGetProc) \
    X(ExtSupported) \
    X(FreeImage) \
    X(FreeObject) \
    X(GetErrorString) \
    X(GetVersion) \
    X(ImageGetSize) \
    X(ImageWrite) \
    X(Link)

/// This class handles the dynamic loading of aticalcl.dll/libaticalcl.so.
/// \note There will typically be one of these objects.
///       That instance will be global.
///       There is a trap for the unwary.
///       The order of global ctors is only defined within a single compilation unit.
///       So, one should not use these interfaces before "main" is reached.
///       This is different than calling these functions when the .dll/.so is linked against.
class CALCLModule
{
public:

    /// Default name to use for construction.
    /// This is usually aticalcl.dll or libaticalcl.so.
    static const char* s_DefaultModuleName;

    /// Constructor
    /// \param module to load.
    CALCLModule(const std::string& moduleName);

    /// destructor
    ~CALCLModule();

    /// Load module.
    /// \param[in] name The module name (aticalcl[64].dll or libaticalcl.so).
    /// \return         true if successful, false otherwise
    bool LoadModule(const std::string& name = s_DefaultModuleName);

    /// Unload the calcl shared image.
    void UnloadModule();

    /// Have we sucessfully loaded the cal module?
    /// \returns enumeration value to answer query.
    bool IsLoaded() { return m_ModuleLoaded; }

#define X(SYM) myddi::ddiif##SYM SYM;
    CALCL_INTERFACE_TABLE;
#undef X

private:
    /// Initialize the internal data
    void Initialize();

    /// Have we loaded the CAL module?
    bool                 m_ModuleLoaded;

    /// Helper.
    DynamicLibraryModule m_DynamicLibraryHelper;
};

#define CALRT_INTERFACE_TABLE \
    X(CtxCreate) \
    X(CtxDestroy) \
    X(CtxFlush) \
    X(CtxGetMem) \
    X(CtxIsEventDone) \
    X(CtxReleaseMem) \
    X(CtxRunProgram) \
    X(CtxRunProgramGrid) \
    X(CtxRunProgramGridArray) \
    X(CtxSetMem) \
    X(DeviceClose) \
    X(DeviceGetAttribs) \
    X(DeviceGetCount) \
    X(DeviceGetInfo) \
    X(DeviceGetStatus) \
    X(DeviceOpen) \
    X(ExtGetProc) \
    X(ExtGetVersion) \
    X(ExtSupported) \
    X(GetErrorString) \
    X(GetVersion) \
    X(ImageFree) \
    X(ImageRead) \
    X(Init) \
    X(MemCopy) \
    X(ModuleGetEntry) \
    X(ModuleGetFuncInfo) \
    X(ModuleGetName) \
    X(ModuleLoad) \
    X(ModuleUnload) \
    X(ResAllocLocal1D) \
    X(ResAllocLocal2D) \
    X(ResAllocRemote1D) \
    X(ResAllocRemote2D) \
    X(ResFree) \
    X(ResMap) \
    X(ResUnmap) \
    X(Shutdown)

/// This class handles the dynamic loading of aticalrt.dll/libaticalrt.so.
/// \note There will typically be one of these objects.
///       That instance will be global.
///       There is a trap for the unwary.
///       The order of global ctors is only defined within a single compilation unit.
///       So, one should not use these interfaces before "main" is reached.
///       This is different than calling these functions when the .dll/.so is linked against.
class CALRTModule
{
public:

    /// Default name to use for construction.
    /// This is usually aticalrt.dll or libaticalrt.so.
    static const char* s_DefaultModuleName;

    /// Constructor
    /// \param module to load.
    CALRTModule(const std::string& moduleName);

    /// destructor
    ~CALRTModule();

    /// Load module.
    /// \param[in] name The module name (aticalrt[64].dll or libaticalrt.so).
    /// \return         true if successful, false otherwise
    bool LoadModule(const std::string& name = s_DefaultModuleName);

    /// Unload the calrt shared image.
    void UnloadModule();

    /// Have we sucessfully loaded the cal module?
    /// \returns enumeration value to answer query.
    bool IsLoaded() { return m_ModuleLoaded; }

#define X(SYM) myddi::ddiif##SYM SYM;
    CALRT_INTERFACE_TABLE;
#undef X

private:
    /// Initialize the internal data
    void Initialize();

    /// Have we loaded the CAL module?
    bool                 m_ModuleLoaded;

    /// Helper.
    DynamicLibraryModule m_DynamicLibraryHelper;
};


#define CALDD_INTERFACE_TABLE \
    X(GetExport) \
    X(GetVersion) \
    X(Init)

/// This class handles the dynamic loading of aticaldd.dll/libaticaldd.so.
/// \note There will typically be one of these objects.
///       That instance will be global.
///       There is a trap for the unwary.
///       The order of global ctors is only defined within a single compilation unit.
///       So, one should not use these interfaces before "main" is reached.
///       This is different than calling these functions when the .dll/.so is linked against.
class CALDDModule
{
public:

    /// Default name to use for construction.
    /// This is usually aticaldd.dll or libaticaldd.so.
    static const char* s_DefaultModuleName;

    /// Constructor
    /// \param module to load.
    CALDDModule(const std::string& moduleName);

    /// destructor
    ~CALDDModule();

    /// Load module.
    /// \param[in] name The module name (aticaldd[64].dll or libaticaldd.so).
    ///                 Linux installs play games with the placement of libatcaldd.
    ///                 Extra code (e.g. a full path) will be needed.
    /// \return         true if successful, false otherwise
    bool LoadModule(const std::string& name = s_DefaultModuleName);

    /// Unload the caldd shared image.
    void UnloadModule();

    /// Have we sucessfully loaded the cal module?
    /// \returns enumeration value to answer query.
    bool IsLoaded() { return m_ModuleLoaded; }

#define X(SYM) myddi::ddiif##SYM SYM;
    CALDD_INTERFACE_TABLE;
#undef X

private:
    /// Initialize the internal data
    void Initialize();

    /// Have we loaded the CAL module?
    bool                 m_ModuleLoaded;

    /// Helper.
    DynamicLibraryModule m_DynamicLibraryHelper;
};


#endif
