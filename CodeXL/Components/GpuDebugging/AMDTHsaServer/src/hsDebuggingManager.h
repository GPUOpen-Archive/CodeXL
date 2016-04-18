//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file hsDebuggingManager.h
///
//==================================================================================

#ifndef __HSDEBUGGINGMANAGER_H
#define __HSDEBUGGINGMANAGER_H

// Forward declarations:
class hsDebugInfo;

// Infra:
#include <AMDTBaseTools/Include/AMDTDefinitions.h>
#include <AMDTBaseTools/Include/gtVector.h>
#include <AMDTOSWrappers/Include/osCriticalSection.h>
#include <AMDTOSWrappers/Include/osThread.h>
#include <AMDTAPIClasses/Include/apKernelDebuggingCommand.h>

// Should be equal to HWDBG_WAVEFRONT_SIZE:
#define HS_WAVEFRONT_SIZE 64

class hsDebuggingManager
{
public:
    static hsDebuggingManager& instance();

    bool InitializeInterception();
    bool UninitializeInterception();
    bool ShouldDebugKernel(const gtString& kernelName);
    bool StartDebugging(void* hDebugContext, const gtString& kernelName, const void* kernelArgs, const gtUInt32 gs[3], const gtUInt32 wgs[3]);
    bool IsDebuggingInProgress() const;
    bool IsInHSAKernelBreakpoint() const;

    gtUInt64 GetCurrentAddress() const;
    bool SetNextDebuggingCommand(apKernelDebuggingCommand cmd);
    bool SetBreakpoint(const gtString& kernelName, gtUInt64 lineNumber);
    void StopKernelRun();

    const hsDebugInfo* GetCurrentDebugInfo() const { return m_pCurrentDebugInfo; };
    void* GetDebugContextHandle();
    bool SetActiveWavefront(gtUInt32 waveIndex, gtUByte threadIndex);
    bool GetWorkItemId(gtUInt32 waveIndex, gtUByte threadIndex, gtUInt32 o_gid[3], gtUInt32 o_lid[3], gtUInt32 o_wgid[3], bool& o_active) const;
    bool GetActiveWorkItem(gtUInt32 o_gid[3], gtUInt32 o_lid[3], gtUInt32 o_wgid[3], bool& o_active) const;
    gtUInt32 GetWavefrontCount() const;
    gtUByte GetWorkDimensions() const;
    const gtString& GetKernelName() const;

private:
    class hsDebugEventThread: public osThread
    {
    public:
        hsDebugEventThread(void* hDebugContext, const gtString& kernelName, osCriticalSection& debuggingCS, const gtUInt32 gs[3], const gtUInt32 wgs[3]);
        virtual ~hsDebugEventThread();

        // Overrides osThread:
        int entryPoint() override;
        void beforeTermination() override;

        void StopDebugging();

        bool SetupStep(apKernelDebuggingCommand cmd);
        void UpdateBreakpoints();
        void UpdateWavefrontData();

        bool SetActiveWavefront(gtUInt32 waveIndex, gtUByte threadIndex);
        void GetActiveWavefront(gtUInt32& waveIndex, gtUByte& threadIndex) const { waveIndex = m_activeWaveIndex; threadIndex = m_activeWorkItemIndex; };
        bool GetWorkItemId(gtUInt32 waveIndex, gtUByte threadIndex, gtUInt32 o_gid[3], gtUInt32 o_lid[3], gtUInt32 o_wgid[3], bool& o_active) const;
        bool GetActiveWorkItem(gtUInt32 o_gid[3], gtUInt32 o_lid[3], gtUInt32 o_wgid[3], bool& o_active) const { return GetWorkItemId(m_activeWaveIndex, m_activeWorkItemIndex, o_gid, o_lid, o_wgid, o_active); };

        gtUInt64 GetActiveWavefrontPC() const;
        gtUInt32 GetWavefrontCount() const { return (gtUInt32)m_wavefrontData.size(); };
        gtUByte GetWorkDims() const { return (m_gridSize[0] < 1 ? 0 : ((m_gridSize[1] < 2) ? 1 : ((m_gridSize[2] < 2) ? 2 : 3))); };
        void* GetDebugContextHandle() const { return m_hDebugContext; };
        const gtString& GetKernelName() const { return m_kernelName; };
        bool IsSuspendedAtBreakpoint() const { return m_isSuspendedAtBreakpoint; };

    private:
        struct hsDebugWavefrontData
        {
        public:
            struct hsDebugWorkItemData
            {
                gtUInt32 m_wiId[3];
                bool m_active;
            };

            gtUInt32 m_wgId[3];
            hsDebugWorkItemData m_wis[HS_WAVEFRONT_SIZE];
            gtUInt32 m_waveAddress;
            gtUInt64 m_wavePC;
        };

    private:
        void* m_hDebugContext;
        const gtString m_kernelName;
        gtUInt32 m_gridSize[3];
        gtUInt32 m_workGroupSize[3];
        // gtVector<void*> m_currentBreakpoints; - This is not needed with "delete all breakpoints".
        gtVector<hsDebugWavefrontData> m_wavefrontData;
        gtUInt32 m_activeWaveIndex;
        gtUByte m_activeWorkItemIndex;
        bool m_isSuspendedAtBreakpoint;
        apKernelDebuggingCommand m_nextCommand;
        osCriticalSection& m_debuggingCS;
        bool m_goOn;
    };

private:
    hsDebuggingManager();
    ~hsDebuggingManager();
    friend class hsSingletonsDelete;

    hsDebuggingManager(const hsDebuggingManager&) = delete;
    hsDebuggingManager(hsDebuggingManager&&) = delete;
    hsDebuggingManager& operator=(const hsDebuggingManager&) = delete;
    hsDebuggingManager& operator=(hsDebuggingManager&&) = delete;

    void Cleanup();

    osCriticalSection m_debuggingCS;

    hsDebugInfo* m_pCurrentDebugInfo;
    hsDebugEventThread* m_pCurrentDebuggingThread;

    bool m_isDuringCleanup;

    static hsDebuggingManager* ms_pMySingleInstance;
};

#endif // __HSDEBUGGINGMANAGER_H
