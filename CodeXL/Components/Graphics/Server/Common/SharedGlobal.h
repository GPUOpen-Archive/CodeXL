//==============================================================================
// Copyright (c) 2007-2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  Shared Globals mechanism for PerfStudio.Used to send massages to the
/// graphics server plugins via shared memory.
//==============================================================================

#ifndef GPS_SHAREDGLOBAL_H
#define GPS_SHAREDGLOBAL_H

#include <stddef.h>
#include "Logger.h"
#include "defines.h"
#include "SharedMemory.h"
#include <AMDTOSWrappers/Include/osMutex.h>

#define SG_GET_PATH(data) SharedGlobal::Instance()->GetPath(offsetof(struct PsSharedGlobal, data)) ///< Shared Memory helper macro
#define SG_SET_PATH(data, path) SharedGlobal::Instance()->SetPath(offsetof(struct PsSharedGlobal, data), path) ///< Shared Memory helper macro

#define SG_GET_BOOL(data) SharedGlobal::Instance()->GetValue<bool>(offsetof(struct PsSharedGlobal, data)) ///< Shared Memory helper macro
#define SG_SET_BOOL(data, value) SharedGlobal::Instance()->SetValue<bool>(offsetof(struct PsSharedGlobal, data), value) ///< Shared Memory helper macro

#define SG_GET_INT(data) SharedGlobal::Instance()->GetValue<int32>(offsetof(struct PsSharedGlobal, data)) ///< Shared Memory helper macro
#define SG_SET_INT(data, value) SharedGlobal::Instance()->SetValue<int>(offsetof(struct PsSharedGlobal, data), value) ///< Shared Memory helper macro

#define SG_GET_UINT(data) SharedGlobal::Instance()->GetValue<uint32>(offsetof(struct PsSharedGlobal, data)) ///< Shared Memory helper macro
#define SG_SET_UINT(data, value) SharedGlobal::Instance()->SetValue<unsigned int>(offsetof(struct PsSharedGlobal, data), value) ///< Shared Memory helper macro

#define SG_GET_FLOAT(data) SharedGlobal::Instance()->GetValue<float>(offsetof(struct PsSharedGlobal, data)) ///< Shared Memory helper macro
#define SG_SET_FLOAT(data, value) SharedGlobal::Instance()->SetValue<float>(offsetof(struct PsSharedGlobal, data), value) ///< Shared Memory helper macro

#define SG_GET_DOUBLE(data) SharedGlobal::Instance()->GetValue<double>(offsetof(struct PsSharedGlobal, data)) ///< Shared Memory helper macro
#define SG_SET_DOUBLE(data, value) SharedGlobal::Instance()->SetValue<double>(offsetof(struct PsSharedGlobal, data), value) ///< Shared Memory helper macro

#define FLAG_BUILD_INTERNAL   0x01 ///< Server flag
#define FLAG_BUILD_32BIT      0x02 ///< Server flag

/// Holds all the data values that should be shared between the main server and the plugins. Add data values
/// to this structure to have them included in the data that is shared between processes.
struct PsSharedGlobal
{
    char ServerPath[ PS_MAX_PATH ];     ///< Path that the server was launched from
    char LogfilePath[ PS_MAX_PATH ];    ///< Path to the log file
    char PluginsPath[ PS_MAX_PATH ];    ///< Path that plugins should be loaded from
#ifdef _WIN32
    char MicroDLLPath[ PS_MAX_PATH ];   ///< Path to the MicroDLL.dll file
    char AppInitDllFileList[PS_MAX_PATH];   ///< List of files to inject MicroDll into if using AppInit_DLLs reg. key
#endif
    char GPUPerfAPIPath[ PS_MAX_PATH ]; ///< Path to GPA dll files
    char CounterFile[ PS_MAX_PATH ];    ///< Path to the default counter definition file (I don't think this is an active code path)
    int32 OptionLogLevel;               ///< Indicates what level the logging is set at
    int32 OptionWireFrameColor;         ///< Indicates color to use for wireframe drawing
    int32 OptionHopCountMax;            ///< The number of process hops allowed
    int32 OptionHopCount;               ///< Current number of hops
    int32 OptionCaptureFrame;            ///< Target frame to capture. App will run to this frame and a capture will be performed automatically
    int32 OptionTimeOverrideMode;       ///< Duplicate of client setting for autocapture
    int32 OptionFilterDrawCalls;        ///< Duplicate of "filter non Draw/Dispatch draw calls" client setting for autocapture
    int32 OptionTraceType;              ///< Mantle/DX12 Only: Specifies which trace type to collect during the set CaptureFrame.
    uint32 OptionLayerFlag;             ///< Allows a GPS developer to turn on or off certain layers in a Plugin
    uint32 OptionNumTracedFrames;       ///< The number of sequential frames to trace/capture in a row.
    uint32 OptionStatsDuration;         ///< Sets the target millisecond duration for which to collect frame statistics.
    uint32 OptionStatsTrigger;          ///< Customize the trigger (any valid Virtual-Key Code) used to start collection of frame statistics.
    float OptionSpeed;                  ///< Overrides the default speed setting in the TimeControlLayer
    bool OptionBreak;                   ///< Allows a GPS developer to attach to the 3D app as it is being launched (immediately after MicroDLL is injected)
    bool OptionNoLogfile;               ///< Disables generation of a log file
    bool OptionRealPause;               ///< Forces RealPause on in the TimeControlLayer
    bool OptionForceRefRast;            ///< Forces the ref rast
    bool OptionDebugRuntime;            ///< Indicates that the D3D Debug Runtime should be enabled
    bool OptionWARPDevice;              ///< Indicates that the D3D WARP Device should be enabled
    bool OptionSDDisableDepthCopy;      ///< Disable copying the depth buffer with the shader debugger.
    bool OptionNoProcessTrack;          ///< Disable process tracking
    bool OptionCopyMappedBuffersUsingCPU;     ///< Duplicate of client setting for autocapture
    bool OptionWireFrameOverlay;        ///<enable/disable wireframe overlay
    bool OptionFlattenCommandLists;     ///< Duplicate of client setting for autocapture
    bool OptionLiquidVR;                ///< Enable when launching a LiquidVR application with PerfStudio. This delays the interception of the Device and DeviceContext until after LVR has wrapped them.
    bool OptionCollectFrameStats;       ///< Enable the collection of frame statistics with a keypress.
#ifdef _WIN32
    bool SteamInjected;                 ///< Was Steam used to launch the application, and was it injected with MicroDLL
#endif
    bool OptionDllReplacement;          ///< Is Dll Replacement being used
    bool OptionManualDllReplacement;    ///< Use manual DLL replacement; hooking and micro dll not used
    bool OptionAppInitDll;              ///< Use the AppInit_DLL registry setting to inject micro dll into applications
    uint32 OptionMdoMode;               ///< How MDO is being used
    uint8 BuildFlags;                   ///< Build flags (internal, 32 or 64 bit etc)
    double LastPresentTime;             ///< Sets time at which the last frame was rendered
    bool ForceRenderStallState;          ///< Used to tell the GPUPerfserver to block all incoming commands from the client
};

/// Manages a shared memory that used by both the main server and the 3D application processes. Provides
/// accessors to the data members which also handle locking and unlocking of the shared memory.
class SharedGlobal
{
public:
    /// Accessor to the singleton instance
    static SharedGlobal* Instance(void);

    /// Gets a string from the shared memory.
    /// This function should not be called directly. All access is via the SG_GET_PATH macro
    const char* GetPath(size_t offset);

    /// Sets a string in the shared memory.
    /// This function should not be called directly. All access is via the SG_SET_PATH macro
    bool SetPath(size_t offset, const char* path);

    //-----------------------------------------------------------------------------
    /// Get a type T value from the global data block.
    ///
    /// \param offset The offset from the start of the structure to
    /// the path of interest. It is calculated automatically at compile time using the
    /// offsetof functionality through the SG_GET_*TYPE* macro
    /// \return the value stored at the offset
    //-----------------------------------------------------------------------------
    template< class T > const T GetValue(size_t offset)
    {
        PsAssert(this != NULL);
        PsAssert(m_MapFile != NULL);
        PsAssert(m_MapFile->Get() != NULL);

        if (Lock())
        {
            T val = *(T*) & ((char*)m_MapFile->Get())[offset];
            Unlock();
            return (val);
        }

        return (0);
    }

    //-----------------------------------------------------------------------------
    /// Set a type T flag in the global data block.
    ///
    /// \param offset The offset from the start of the structure to
    /// the path of interest. It is calculated automatically at compile time using the
    /// offsetof functionality through the SG_SET_*TYPE* macro
    /// \param value the value to set at the offset
    /// \return true if the value could be set
    //-----------------------------------------------------------------------------
    template< class T > bool SetValue(size_t offset, const T value)
    {
        PsAssert(this != NULL);
        PsAssert(m_MapFile != NULL);
        PsAssert(m_MapFile->Get() != NULL);

        if (Lock())
        {
            *(T*)&((char*)m_MapFile->Get())[offset] = value;

            Unlock();
            return (true);
        }

        return (false);
    }

private:

    /// request exclusive access to global memory region
    /// \return true on success; false on failure
    bool Lock(void);

    /// release exclusive access to global memory region
    void Unlock(void);

    /// Private constructor to adhere to singleton pattern
    SharedGlobal(void);

    /// destructor
    ~SharedGlobal(void);

    /// Initializes the shared memory and any necessary mutexes
    /// \return true on success; false on error
    bool Initialize(void);

private:
    SharedMemory*    m_MapFile;     ///< Shared memory
    osMutex*       m_Mutex;         ///< Mutex object created once at initialization.

    bool   m_bInitialized;          ///< Has the object been initialized?
    ///< this should only be accessed after exclusive access granted.
    struct PsSharedGlobal m_Shadow;     ///< Shadow copy of global memory (not all fields updated - check
    ///< implementation of each accessor function)
};

#endif // GPS_SHAREDGLOBAL_H
