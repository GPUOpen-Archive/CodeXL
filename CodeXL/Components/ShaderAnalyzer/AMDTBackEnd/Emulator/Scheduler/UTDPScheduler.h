//=============================================================
// Copyright (c) 2013 Advanced Micro Devices, Inc.
//=============================================================

#ifndef __UTDPSCHEDULER_H
#define __UTDPSCHEDULER_H


#include <vector>
#include "CUScheduler.h"
#include "WorkGroup.h"


/// -----------------------------------------------------------------------------------------------
/// \class Name: UTDPScheduler
/// \brief Description:  The ultra-threaded dispatch processor [UTDP]accepts
///  commands from the command processor and distributes work across the array
///  of compute units.
/// -----------------------------------------------------------------------------------------------

class UTDPScheduler
{
public:
    /// SI Device types
    enum DeviceType
    {
        Tahiti,
        CapeVerde,
        Pitcrain
    };

    /// Scheduler status
    enum StatusSchedule
    {
        Status_ScheduleInvalidWorkDim,
        Status_ScheduleInvalidWorkGroupSize,
        Status_ScheduleInvalidWorkItemSize,
        Status_ScheduleSuccess
    };

    /// The number of compute units per device.
    static const size_t NumCUTahiti;
    static const size_t NumCUCapeVerde;
    static const size_t NumCUPitcrain;

    /// The number of work-items in the wavefront per device.
    static const size_t WaveFrontWINumTahiti;
    static const size_t WaveFrontWINumCapeVerde;
    static const size_t WaveFrontWINumPitcrain;

    /// The value of clock (in nano-seconds) per device.
    static const size_t ClkTahiti;
    static const size_t ClkCapeVerde;
    static const size_t ClkPitcrain;

    /// The maximum number of work-items per device.
    static const size_t DeviceMaxWorkGoupSizeTahiti;
    static const size_t DeviceMaxWorkGoupSizeCapeVerde;
    static const size_t DeviceMaxWorkGoupSizePitcrain;

    /// -----------------------------------------------------------------------------------------------
    /// \brief Name:        UTDPScheduler
    /// \brief Description: c`tor
    /// \return
    /// -----------------------------------------------------------------------------------------------
    UTDPScheduler(): m_wavefronts(0), m_throughput(0) {};

    /// -----------------------------------------------------------------------------------------------
    /// \brief Name:        UTDPScheduler
    /// \brief Description: c`tor
    /// \param[in]          instructions
    /// \param[in]          deviceType
    /// \param[in]          workDim
    /// \param[in]          globalWorkSize
    /// \param[in]          localWorkSize
    /// \param[in]          branchRate
    /// \return
    /// -----------------------------------------------------------------------------------------------
    UTDPScheduler(const std::vector<Instruction*>& instructions, DeviceType deviceType, size_t workDim, const std::vector<size_t>& globalWorkSize, const std::vector<size_t>& localWorkSize, double branchRate);
    /// -----------------------------------------------------------------------------------------------
    /// \brief Name:        ~UTDPScheduler
    /// \brief Description: d`tor.
    /// \return
    /// -----------------------------------------------------------------------------------------------
    ~UTDPScheduler() {}
    /// -----------------------------------------------------------------------------------------------
    /// \brief Name:        Schedule
    /// \brief Description:
    /// \param[in]          exeClkNum
    /// \return StatusSchedule Schedule the program kernel(s) and updated execution time.
    /// -----------------------------------------------------------------------------------------------
    StatusSchedule Schedule(size_t& exeClkNum);
    /// -----------------------------------------------------------------------------------------------
    /// \brief Name:        Schedule
    /// \brief Description:
    /// \param[in]          workGroup
    /// \param[in]          isScheduleProgress
    /// \return void Schedule work group #workGroup
    /// -----------------------------------------------------------------------------------------------
    void Schedule(size_t workGroup, bool& isScheduleProgress);

    /// -----------------------------------------------------------------------------------------------
    /// \brief Name:        IsWorkDimValid
    /// \brief Description: Checks if m_workDim is has a valid value (i.e. a value between 1 and 3).
    /// \return True : If yes.
    /// \return False: If no
    /// -----------------------------------------------------------------------------------------------
    bool IsWorkDimValid();
    /// -----------------------------------------------------------------------------------------------
    /// \brief Name:        IsGlobalWorkSizeValid
    /// \brief Description: Checks m_workGroupSize = m_globalWorkSize[0]*...*m_globalWorkSize[m_workDim - 1] is not greater than CL_DEVICE_MAX_WORK_GROUP_SIZE for the specified device.
    /// \return True : If yes.
    /// \return False: If no.
    /// -----------------------------------------------------------------------------------------------
    bool IsGlobalWorkSizeValid();
    /// -----------------------------------------------------------------------------------------------
    /// \brief Name:        IsLocalWorkSizeValid
    /// \brief Description: Checks if m_globalWorkSize values specified in m_globalWorkSize[0],... m_globalWorkSize[m_workDim - 1] are evenly divisable by the corresponding values specified in m_localWorkSize[0],... m_localWorkSize[m_workDim - 1].
    /// \return True : If yes.
    /// \return False: If no.
    /// -----------------------------------------------------------------------------------------------
    bool IsLocalWorkSizeValid();

    /// -----------------------------------------------------------------------------------------------
    /// \brief Name:        GetExeTime
    /// \brief Description: Get the execution time of the program in seconds
    /// \param[in]          exeClkNum
    /// \return size_t
    /// -----------------------------------------------------------------------------------------------
    size_t GetExeTime(size_t exeClkNum) const;

    /// -----------------------------------------------------------------------------------------------
    /// \brief Name:        UpdateInstructionCounters
    /// \brief Description: Update Instruction Counters
    /// \param[in]          instructions
    /// \return void
    /// -----------------------------------------------------------------------------------------------
    void UpdateInstructionCounters(const std::vector<Instruction*>& instructions, double branchRate);

    /// -----------------------------------------------------------------------------------------------
    /// \brief Name:        IsBranchTaken
    /// \brief Description: Estimates if branch should be taken
    /// \param[in]          itNumber
    /// \return True : If Branch is taken
    /// \return False: If Branch is not taken
    /// -----------------------------------------------------------------------------------------------
    bool IsBranchTaken(unsigned int itNumber, double branchRate, bool isForward = true);


    /// -----------------------------------------------------------------------------------------------
    /// \brief Name:        UpdateWavefrontsNum
    /// \brief Description: Calculate the total number of wavefronts executing a kernel
    /// \param[in]          waveFrontWINum
    /// \param[in]          globalWorkSize
    /// \param[in]          localWorkSize
    /// \return void
    /// -----------------------------------------------------------------------------------------------
    void UpdateWavefrontsNum(unsigned int waveFrontWINum, int* globalWorkSize, int* localWorkSize);

    /// -----------------------------------------------------------------------------------------------
    /// \brief Name:        GetScalarMemoryReadInstCount
    /// \brief Description: Returns Scalar Memory Read Instructions Count
    /// \return unsigned int
    /// -----------------------------------------------------------------------------------------------
    unsigned int GetScalarMemoryReadInstCount() const { return m_instructionCounters.m_scalarMemoryReadInstCount; }

    /// -----------------------------------------------------------------------------------------------
    /// \brief Name:        GetScalarALUInstCount
    /// \brief Description: Returns Scalar ALU Instructions Count
    /// \return unsigned int
    /// -----------------------------------------------------------------------------------------------
    unsigned int GetScalarALUInstCount() const { return m_instructionCounters.m_scalarALUInstCount; }

    /// -----------------------------------------------------------------------------------------------
    /// \brief Name:        GetVectorMemoryReadInstCount
    /// \brief Description: Returns Vector Memory Read Instructions Count
    /// \return unsigned int
    /// -----------------------------------------------------------------------------------------------
    unsigned int GetVectorMemoryReadInstCount() const { return m_instructionCounters.m_vectorMemoryReadInstCount; }

    /// -----------------------------------------------------------------------------------------------
    /// \brief Name:        GetVectorMemoryWriteInstCount
    /// \brief Description: GetVectorMemoryWriteInstCount
    /// \return unsigned int
    /// -----------------------------------------------------------------------------------------------
    unsigned int GetVectorMemoryWriteInstCount() const { return m_instructionCounters.m_vectorMemoryWriteInstCount; }

    /// -----------------------------------------------------------------------------------------------
    /// \brief Name:        GetVectorALUInstCount
    /// \brief Description: Returns Vector ALU Instructions Count
    /// \return unsigned int
    /// -----------------------------------------------------------------------------------------------
    unsigned int GetVectorALUInstCount() const { return m_instructionCounters.m_vectorALUInstCount; }

    /// -----------------------------------------------------------------------------------------------
    /// \brief Name:        GetLDSInstCount
    /// \brief Description: Returns LDS Instructions Count
    /// \return unsigned int
    /// -----------------------------------------------------------------------------------------------
    unsigned int GetLDSInstCount() const { return m_instructionCounters.m_LDSInstCount; }
    /// -----------------------------------------------------------------------------------------------
    /// \brief Name:        GetGDSInstCount
    /// \brief Description: Returns GDS Instructions Count
    /// \return unsigned int
    /// -----------------------------------------------------------------------------------------------
    unsigned int GetGDSInstCount() const { return m_instructionCounters.m_GDSInstCount; }

    /// -----------------------------------------------------------------------------------------------
    /// \brief Name:        GetExportInstCount
    /// \brief Description: Returns GDS Instructions Count
    /// \return unsigned int
    /// -----------------------------------------------------------------------------------------------
    unsigned int GetExportInstCount() const { return m_instructionCounters.m_exportInstCount; }

    /// -----------------------------------------------------------------------------------------------
    /// \brief Name:        GetAtomicsInstCount
    /// \brief Description: Returns Atomics Instructions Count
    /// \return unsigned int
    /// -----------------------------------------------------------------------------------------------
    unsigned int GetAtomicsInstCount() const { return m_instructionCounters.m_atomicsInstCount; }

    /// -----------------------------------------------------------------------------------------------
    /// \brief Name:        GetTakenBranchInstCount
    /// \brief Description: Returns Taken Branchees Instructions Count
    /// \return unsigned int
    /// -----------------------------------------------------------------------------------------------
    unsigned int GetTakenBranchInstCount() const { return m_instructionCounters.m_branchInstCount; }

    /// -----------------------------------------------------------------------------------------------
    /// \brief Name:        GetWaveFrontsNum
    /// \brief Description: Returns the total number of wavefronts for kernels execution
    /// \return unsigned int
    /// -----------------------------------------------------------------------------------------------
    unsigned int GetWaveFrontsNum() const { return m_wavefronts; }

    /// -----------------------------------------------------------------------------------------------
    /// \brief Name:        GetThroughput
    /// \brief Description: Returns the throughput
    /// \return double
    /// -----------------------------------------------------------------------------------------------
    double GetThroughput() const { return m_throughput; }

private:
    /// The device type.
    DeviceType m_deviceType;

    /// The number of compute units in the device.
    size_t m_numCU;

    /// The duration of the device clock in seconds
    size_t m_clkToSec;

    /// The maximum Number of work items in the device.
    size_t m_maxWorkGoupSize;

    /// The number of work-items per wavefront in the device.
    size_t m_waveFrontSize;

    /// The number of wavefronts per work group in the device for the program.
    size_t m_waveFrontNum;

    /// The number of work groups defined in the program`s kernel(s)
    size_t m_workGroupNum;

    /// The number of work in the work group.
    size_t m_workGroupSize;

    /// The number of dimensions of the program`s kernel(s)
    size_t m_workDim;

    /// The number of global work-items in each dimension of the program`s input.
    std::vector<size_t> m_globalWorkSize;

    /// The number of work-items in each dimension of the work-group.
    std::vector<size_t> m_localWorkSize;

    /// The array of schedulers for all compute unit.
    std::vector<CUScheduler> m_vCUScheduler;

    /// The array of all work groups.
    std::vector<WorkGroup> m_workGroups;

    // The branchRate
    double m_branchRate;


    /// Auxiliary multiplier struct used for accumulation for the local and global work sizes.
    struct Mul
    {
        size_t operator()(size_t l, size_t r) { return l * r; }

    } m_mul;

    /// The number of instruction in each category
    struct InstructionCounters
    {
        unsigned int m_scalarMemoryReadInstCount;
        unsigned int m_scalarMemoryWriteInstCount;
        unsigned int m_scalarALUInstCount;
        unsigned int m_vectorMemoryReadInstCount;
        unsigned int m_vectorMemoryWriteInstCount;
        unsigned int m_vectorALUInstCount;
        unsigned int m_LDSInstCount;
        unsigned int m_GDSInstCount;
        unsigned int m_exportInstCount;
        unsigned int m_atomicsInstCount;
        unsigned int m_branchInstCount;
        InstructionCounters() : m_scalarMemoryReadInstCount(0), m_scalarMemoryWriteInstCount(0), m_scalarALUInstCount(0), m_vectorMemoryReadInstCount(0), m_vectorMemoryWriteInstCount(0), m_vectorALUInstCount(0),
            m_LDSInstCount(0), m_GDSInstCount(0), m_exportInstCount(0), m_atomicsInstCount(0), m_branchInstCount(0) {}
    } m_instructionCounters;

    /// The total number of wavefronts executing a kernel
    unsigned int m_wavefronts;

    /// The throughput
    double m_throughput;

    void ResetInstructionsCounters();
};

#endif //__UTDPSCHEDULER_H

