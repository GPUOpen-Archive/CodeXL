#ifndef __METRIC_H__
#define __METRIC_H__

// ------------------------------------------------------------------
// Predefined metric names

// "CLSID"
extern const unsigned short* metricCLSID;
// "Name"
extern const unsigned short* metricName;
// "Language"
extern const unsigned short* metricLanguage;


#ifndef NO_DBGMETRIC // if NO_DBGMETIC is defined, don't include functions

    // ------------------------------------------------------------------
    // General purpose metric routines

    //HRESULT GetMetric(const unsigned short* pszMachine, const unsigned short* pszType, REFGUID guidSection, const unsigned short* pszMetric, VARIANT* pvarValue, const unsigned short* pszAltRoot);
    HRESULT __stdcall GetMetric(const unsigned short* pszMachine, const unsigned short* pszType, REFGUID guidSection, const unsigned short* pszMetric, _Out_ DWORD* pdwValue, const unsigned short* pszAltRoot);
    HRESULT __stdcall GetMetric(const unsigned short* pszMachine, const unsigned short* pszType, REFGUID guidSection, const unsigned short* pszMetric, BSTR* pbstrValue, const unsigned short* pszAltRoot);
    HRESULT __stdcall GetMetric(const unsigned short* pszMachine, const unsigned short* pszType, REFGUID guidSection, const unsigned short* pszMetric, _Out_ GUID* pguidValue, const unsigned short* pszAltRoot);
    HRESULT __stdcall GetMetric(const unsigned short* pszMachine, const unsigned short* pszType, REFGUID guidSection, const unsigned short* pszMetric, _Out_opt_cap_post_count_(*pdwSize, *pdwSize) GUID* rgguidValues, _Inout_ DWORD* pdwSize, const unsigned short* pszAltRoot);

    //HRESULT SetMetric(const unsigned short* pszType, REFGUID guidSection, const unsigned short* pszMetric, const VARIANT varValue);
    HRESULT __stdcall SetMetric(const unsigned short* pszType, REFGUID guidSection, const unsigned short* pszMetric, const DWORD dwValue, bool fUserSpecific, const unsigned short* pszAltRoot);
    HRESULT __stdcall SetMetric(const unsigned short* pszType, REFGUID guidSection, const unsigned short* pszMetric, const unsigned short* pszValue, bool fUserSpecific, const unsigned short* pszAltRoot);
    HRESULT __stdcall SetMetric(const unsigned short* pszType, REFGUID guidSection, const unsigned short* pszMetric, REFGUID guidValue, bool fUserSpecific, const unsigned short* pszAltRoot);
    HRESULT __stdcall SetMetric(const unsigned short* pszType, REFGUID guidSection, const unsigned short* pszMetric, _In_count_(dwSize) const GUID* rgguidValues, DWORD dwSize, bool fUserSpecific, const unsigned short* pszAltRoot);

    HRESULT __stdcall EnumMetricSections(const unsigned short* pszMachine, const unsigned short* pszType, _Out_opt_cap_post_count_(*pdwSize, *pdwSize) GUID* rgguidSections, _Inout_ DWORD* pdwSize, const unsigned short* pszAltRoot);

    HRESULT __stdcall RemoveMetric(const unsigned short* pszType, REFGUID guidSection, const unsigned short* pszMetric, const unsigned short* pszAltRoot);

    HRESULT __stdcall SetMetricLocale(WORD wLangId);
    WORD __stdcall GetMetricLocale();

    HRESULT ReadTextFileAsBstr(const unsigned short* szFileName, BSTR* pbstrFileContent);
#endif // end ifndef NO_DBGMETRIC



// Predefined metric types
// "Engine"
extern const unsigned short* metrictypeEngine;
// "PortSupplier"
extern const unsigned short* metrictypePortSupplier;
// "Exception"
extern const unsigned short* metrictypeException;
// "EEExtensions"
extern const unsigned short* metricttypeEEExtension;

// Predefined engine metric names
// AddressBP
extern const unsigned short* metricAddressBP;
// AlwaysLoadLocal
extern const unsigned short* metricAlwaysLoadLocal;
// LoadInDebuggeeSession
extern const unsigned short* metricLoadInDebuggeeSession;
// LoadedByDebuggee
extern const unsigned short* metricLoadedByDebuggee;
// Attach
extern const unsigned short* metricAttach;
// CallStackBP
extern const unsigned short* metricCallStackBP;
// ConditionalBP
extern const unsigned short* metricConditionalBP;
// DataBP
extern const unsigned short* metricDataBP;
// Disassembly
extern const unsigned short* metricDisassembly;
// Dump writing
extern const unsigned short* metricDumpWriting;
// ENC
extern const unsigned short* metricENC;
// Exceptions
extern const unsigned short* metricExceptions;
// FunctionBP
extern const unsigned short* metricFunctionBP;
// HitCountBP
extern const unsigned short* metricHitCountBP;
// JITDebug
extern const unsigned short* metricJITDebug;
// Memory
extern const unsigned short* metricMemory;
// Port supplier
extern const unsigned short* metricPortSupplier;
// Registers
extern const unsigned short* metricRegisters;
// SetNextStatement
extern const unsigned short* metricSetNextStatement;
// SuspendThread
extern const unsigned short* metricSuspendThread;
// WarnIfNoSymbols
extern const unsigned short* metricWarnIfNoSymbols;
// Filtering non-user frames
extern const unsigned short* metricShowNonUserCode;
// What CLSID provides program nodes?
extern const unsigned short* metricProgramProvider;
// Always load the program provider locally?
extern const unsigned short* metricAlwaysLoadProgramProviderLocal;
// Use engine to watch for process events instead of program provider?
extern const unsigned short* metricEngineCanWatchProcess;
// Engines will be placed in the SDM's engine filter in descending order of priority. This determines the order in which WatchForProviderEvents/etc is called during multi-engine launch
extern const unsigned short* metricEnginePriority;
// Can we do remote debugging?
extern const unsigned short* metricRemoteDebugging;
// Does the program provider support returning process information for a process that we are not debugging?
// This should _NOT_ be set to true unlesss the program provider can return information without the use
// of DCOM. This implies that 'metricAlwaysLoadProgramProviderLocal' should be set as well.
extern const unsigned short* metricRemoteProcessListing;
// Should the encmgr use native's encbuild.dll to build for enc?
extern const unsigned short* metricEncUseNativeBuilder;
// When debugging a 64-bit process under WOW, should we load the engine 'remotely'
// or in the devenv process (which is running under WOW64)
extern const unsigned short* metricLoadUnderWOW64;
// When debugging a 64-bit process under WOW, should we load the program provider
// 'remotely' or in the devenv process (which is running under WOW64)
extern const unsigned short* metricLoadProgramProviderUnderWOW64;
// Stop on unhandled exceptions thrown across app domain boundaries
extern const unsigned short* metricStopOnExceptionCrossingManagedBoundary;
// Warn user if there is no "user" code on launch
extern const unsigned short* metricWarnIfNoUserCodeOnLaunch;
// Priority for engine automatic selection (preference given to higher)
extern const unsigned short* metricAutoSelectPriority;
// engines not compatible with this engine (only for automatic engine selection)
extern const unsigned short* metricAutoSelectIncompatibleList;
// engines not compatible with this engine
extern const unsigned short* metricIncompatibleList;
// Disable JIT optimizations while debugging
extern const unsigned short* metricDisableJITOptimization;
// Default memory organization 0=little endian (most typical), 1=big endian
extern const unsigned short* metricBigEndian;
// Allow multiple debuggers (no causality)
extern const unsigned short* metricAllowMultipleDebuggers;
// Ignore GPU race hazards if the data didn't change the value
extern const unsigned short* metricGpuRaceHazardsAllowSame;
// Is this engine an IntelliTrace engine?
extern const unsigned short* metricIntellitraceEngine;

// Filtering non-user frames
extern const unsigned short* metricShowNonUserCode;

// Stepping in "user" code only
extern const unsigned short* metricJustMyCodeStepping;
// Allow all threads to run when doing a funceval
extern const unsigned short* metricAllThreadsRunOnFuncEval;
// Use Shim API to get ICorDebug
extern const unsigned short* metricUseShimAPI;
// Attempt to map breakpoints in client-side script
extern const unsigned short* metricMapClientBreakpoints;
// Enable funceval quick abort
extern const unsigned short* metricEnableFuncEvalQuickAbort;
// Specify detour dll names for funceval quick abort
extern const unsigned short* metricFuncEvalQuickAbortDlls;
// Specify EXEs for which we shouldn't do FEQA
extern const unsigned short* metricFuncEvalQuickAbortExcludeList;
// Trace settings.
extern const unsigned short* metricTracing;
extern const unsigned short* metricEnableTracing;
// Enable/disable crossthread dependency notifications
extern const unsigned short* metricCrossThreadDependencyNotification;

// Managed engine activation
extern const unsigned short* metricEngineClass;
extern const unsigned short* metricEngineAssembly;
extern const unsigned short* metricProgramProviderClass;
extern const unsigned short* metricProgramProviderAssembly;

// Predefined EE metric names
// Engine
extern const unsigned short* metricEngine;
// Preload Modules
extern const unsigned short* metricPreloadModules;
// ThisObjectName
extern const unsigned short* metricThisObjectName;
// HideGoToSource
extern const unsigned short* metricHideGoToSource;
// HideGoToDisassembly
extern const unsigned short* metricHideGoToDisassembly;
// HideRunToCursor
extern const unsigned short* metricHideRunToCursor;
// HideBreakpointCommands
extern const unsigned short* metricHideCallStackBreakpoints;

// Predefined EE Extension metric names
// ExtensionDll
extern const unsigned short* metricExtensionDll;
// RegistersSupported
extern const unsigned short* metricExtensionRegistersSupported;
// RegistersEntryPoint
extern const unsigned short* metricExtensionRegistersEntryPoint;
// TypesSupported
extern const unsigned short* metricExtensionTypesSupported;
// TypesEntryPoint
extern const unsigned short* metricExtensionTypesEntryPoint;

// Predefined PortSupplier metric names
// PortPickerCLSID
extern const unsigned short* metricPortPickerCLSID;
// DisallowUserEnteredPorts
extern const unsigned short* metricDisallowUserEnteredPorts;
// PidBase
extern const unsigned short* metricPidBase;


#ifndef NO_DBGMETRIC // if NO_DBGMETIC is defined, don't include functions

    // ------------------------------------------------------------------
    // Engine-specific metric routines

    HRESULT __stdcall EnumDebugEngines(const unsigned short* pszMachine, REFGUID guidPortSupplier, BOOL fRequireRemoteDebugging, _Out_opt_cap_post_count_(*pdwSize, *pdwSize) GUID* rgguidEngines, _Inout_ DWORD* pdwSize, const unsigned short* pszAltRoot);

#endif // end ifndef NO_DBGMETRIC



#ifndef NO_DBGMETRIC // if NO_DBGMETIC is defined, don't include functions

    // ------------------------------------------------------------------
    // EE-specific metric routines

    HRESULT __stdcall GetEEMetric(REFGUID guidLang, REFGUID guidVendor, const unsigned short* pszMetric, _Out_ DWORD* pdwValue, const unsigned short* pszAltRoot);
    HRESULT __stdcall GetEEMetric(REFGUID guidLang, REFGUID guidVendor, const unsigned short* pszMetric, BSTR* pbstrValue, const unsigned short* pszAltRoot);
    HRESULT __stdcall GetEEMetric(REFGUID guidLang, REFGUID guidVendor, const unsigned short* pszMetric, _Out_ GUID* pguidValue, const unsigned short* pszAltRoot);
    HRESULT __stdcall GetEEMetric(REFGUID guidLang, REFGUID guidVendor, const unsigned short* pszMetric, _Out_opt_cap_post_count_(*pdwSize, *pdwSize) GUID* rgguidValues, _Inout_ DWORD* pdwSize, const unsigned short* pszAltRoot);

    HRESULT __stdcall SetEEMetric(REFGUID guidLang, REFGUID guidVendor, const unsigned short* pszMetric, DWORD dwValue, const unsigned short* pszAltRoot);
    HRESULT __stdcall SetEEMetric(REFGUID guidLang, REFGUID guidVendor, const unsigned short* pszMetric, const unsigned short* pszValue, const unsigned short* pszAltRoot);
    HRESULT __stdcall SetEEMetric(REFGUID guidLang, REFGUID guidVendor, const unsigned short* pszMetric, REFGUID guidValue, const unsigned short* pszAltRoot);
    HRESULT __stdcall SetEEMetric(REFGUID guidLang, REFGUID guidVendor, const unsigned short* pszMetric, _In_count_(dwSize) const GUID* rgguidValues, DWORD dwSize, const unsigned short* pszAltRoot);

    HRESULT __stdcall EnumEEs(_Out_opt_cap_post_count_(*pdwSize, *pdwSize) GUID* rgguidLang, _Out_opt_cap_post_count_(*pdwSize, *pdwSize) GUID* rgguidVendor, _Inout_ DWORD* pdwSize, const unsigned short* pszAltRoot);

    HRESULT __stdcall RemoveEEMetric(REFGUID guidLang, REFGUID guidVendor, const unsigned short* pszMetric, const unsigned short* pszAltRoot);

    HRESULT __stdcall GetEEMetricFile(REFGUID guidLang, REFGUID guidVendor, const unsigned short* pszMetric, BSTR* pbstrValue, const unsigned short* pszAltRoot);

#endif // end ifndef NO_DBGMETRIC



#ifndef NO_DBGMETRIC // if NO_DBGMETIC is defined, don't include functions

    // ------------------------------------------------------------------
    // SP-specific metric routines

    HRESULT __stdcall GetSPMetric(REFGUID guidSymbolType, const unsigned short* pszStoreType, const unsigned short* pszMetric, BSTR* pbstrValue, const unsigned short* pszAltRoot);
    HRESULT __stdcall GetSPMetric(REFGUID guidSymbolType, const unsigned short* pszStoreType, const unsigned short* pszMetric, _Out_ GUID* pguidValue, const unsigned short* pszAltRoot);

    HRESULT __stdcall SetSPMetric(REFGUID guidSymbolType, const unsigned short* pszStoreType, const unsigned short* pszMetric, const unsigned short* pszValue, const unsigned short* pszAltRoot);
    HRESULT __stdcall SetSPMetric(REFGUID guidSymbolType, const unsigned short* pszStoreType, const unsigned short* pszMetric, REFGUID guidValue, const unsigned short* pszAltRoot);

    HRESULT __stdcall RemoveSPMetric(REFGUID guidSymbolType, const unsigned short* pszStoreType, const unsigned short* pszMetric, const unsigned short* pszAltRoot);

#endif // end ifndef NO_DBGMETRIC



// Predefined SP store types
// "file"
extern const unsigned short* storetypeFile;
// "metadata"
extern const unsigned short* storetypeMetadata;


#ifndef NO_DBGMETRIC // if NO_DBGMETIC is defined, don't include functions

// ------------------------------------------------------------------
// Exception metric routines

struct EXCEPTION_DEFAULT_STOP_STATE
{
    BSTR bstrExceptionName;
    DWORD dwCode;
    DWORD /*EXCEPTION_STATE*/ dwState; // EXCEPTION_STOP_FIRST_CHANCE, EXCEPTION_STOP_USER_FIRST_CHANCE, and/or EXCEPTION_STOP_USER_UNCAUGHT
    GUID guidType; // This is the guid category
};

// Enumerate the default stop settings for a particular exception type (ex: guidNativeOnlyEng,
// guidMDANotification) from the registry. Any exception not is this list has a default stop state of
// EXCEPTION_STOP_SECOND_CHANCE.
HRESULT __stdcall EnumExceptionDefaultStopMetrics(REFGUID guidType, _Out_opt_ DWORD* pdwDefaultState, _Out_opt_cap_post_count_(*pdwSize, *pdwSize) EXCEPTION_DEFAULT_STOP_STATE* rgSettings, _Inout_ DWORD* pdwSize, const unsigned short* pszAltRoot);

HRESULT __stdcall GetExceptionDefaultStopState(REFGUID guidType, _In_opt_ const unsigned short* szExceptionName, DWORD dwCode, _Out_ DWORD* pdwState, _In_z_ const unsigned short* pszAltRoot);

HRESULT __stdcall GetExceptionMetric(REFGUID guidType, const unsigned short* pszException, _Out_opt_ DWORD* pdwState, _Out_opt_ DWORD* pdwCode, const unsigned short* pszAltRoot);

HRESULT __stdcall SetExceptionMetric(REFGUID guidType, const unsigned short* pszParent, const unsigned short* pszException, DWORD dwState, DWORD dwCode, const unsigned short* pszAltRoot);

HRESULT __stdcall EnumExceptionMetrics(REFGUID guidType, const unsigned short* pszParent, _Out_opt_cap_post_count_(*pdwSize, *pdwSize) BSTR* rgbstrExceptions, _Out_opt_cap_post_count_(*pdwSize, *pdwSize) DWORD* rgdwState, _Out_opt_cap_post_count_(*pdwSize, *pdwSize) DWORD* rgdwCode, _Inout_ DWORD* pdwSize, const unsigned short* pszAltRoot);

HRESULT __stdcall RemoveExceptionMetric(REFGUID guidType, const unsigned short* pszParent, const unsigned short* pszException, const unsigned short* pszAltRoot);
HRESULT __stdcall RemoveAllExceptionMetrics(REFGUID guidType, const unsigned short* pszAltRoot);

#endif // end ifndef NO_DBGMETRIC

#endif // __METRIC_H__
