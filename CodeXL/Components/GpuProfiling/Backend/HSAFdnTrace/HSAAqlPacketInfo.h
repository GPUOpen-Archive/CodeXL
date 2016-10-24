//==============================================================================
// Copyright (c) 2016 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  This file contains classes for tracing HSA AQL Packets
//==============================================================================

#ifndef _HSA_AQL_PACKET_INFO_H_
#define _HSA_AQL_PACKET_INFO_H_

#include <ostream>
#include "hsa.h"

/// HSAAqlPacketBase base class
class HSAAqlPacketBase
{
public:
    /// Constructor
    HSAAqlPacketBase(hsa_packet_type_t type);

    /// Virtual destructor
    virtual ~HSAAqlPacketBase();

    /// Write timestamp entry
    /// \param sout output stream
    /// \param bTimeout a flag indicating output mode
    /// \return True if timestamps are ready
    virtual bool WritePacketEntry(std::ostream& sout, bool bTimeout);

    hsa_packet_type_t m_type;     ///< aql packet type
    uint64_t          m_packetId; ///< packet id of the packet
    hsa_agent_t       m_agent;    ///< agent associated with packet
    hsa_queue_t*      m_pQueue;   ///< queue a packet is dispatched to
    bool              m_isReady;  ///< flag indicating if this packet is ready to be written

private:
    /// Disable copy constructor
    /// \param[in] obj  the input object
    HSAAqlPacketBase(const HSAAqlPacketBase& obj);

    /// Disable assignment operator
    /// \param[in] obj  the input object
    /// \return a reference of the object
    HSAAqlPacketBase& operator=(const HSAAqlPacketBase& obj);
};

/// HSAAqlPacketBase descendant that handles kernel dispatch packets
class HSAAqlKernelDispatchPacket : public HSAAqlPacketBase
{
public:
    /// Constructor
    HSAAqlKernelDispatchPacket(hsa_kernel_dispatch_packet_t kernelDispatchPacket);

    /// Write timestamp entry
    /// \param sout output stream
    /// \param bTimeout a flag indicating output mode
    /// \return True if timestamps are ready
    virtual bool WritePacketEntry(std::ostream& sout, bool bTimeout);

    /// Sets the start/end timestamps
    /// \param start the start timestamp
    /// \param end the end timestamp
    void SetTimestamps(uint64_t start, uint64_t end);

private:
    uint64_t                     m_start;  ///< start time of packet
    uint64_t                     m_end;    ///< start time of packet
    hsa_kernel_dispatch_packet_t m_packet; ///< the kernel dispatch packet
};

/// HSAAqlPacketBase descendant that handles agent dispatch packets
class HSAAqlAgentDispatchPacket : public HSAAqlPacketBase
{
public:
    /// Constructor
    HSAAqlAgentDispatchPacket(hsa_agent_dispatch_packet_t agentDispatchPacket);

    /// Write timestamp entry
    /// \param sout output stream
    /// \param bTimeout a flag indicating output mode
    /// \return True if timestamps are ready
    virtual bool WritePacketEntry(std::ostream& sout, bool bTimeout);

private:
    hsa_agent_dispatch_packet_t m_packet; ///< the agent dispatch packet
};

/// HSAAqlPacketBase descendant that handles barrier-and packets
class HSAAqlBarrierAndPacket : public HSAAqlPacketBase
{
public:
    /// Constructor
    HSAAqlBarrierAndPacket(hsa_barrier_and_packet_t barrierAndPacket);

    /// Write timestamp entry
    /// \param sout output stream
    /// \param bTimeout a flag indicating output mode
    /// \return True if timestamps are ready
    virtual bool WritePacketEntry(std::ostream& sout, bool bTimeout);

private:
    hsa_barrier_and_packet_t m_packet; ///< the barrier and packet
};

/// HSAAqlPacketBase descendant that handles barrier-or packets
class HSAAqlBarrierOrPacket : public HSAAqlPacketBase
{
public:
    /// Constructor
    HSAAqlBarrierOrPacket(hsa_barrier_or_packet_t barrierOrPacket);

    /// Write timestamp entry
    /// \param sout output stream
    /// \param bTimeout a flag indicating output mode
    /// \return True if timestamps are ready
    virtual bool WritePacketEntry(std::ostream& sout, bool bTimeout);

private:
    hsa_barrier_or_packet_t m_packet; ///< the barrier or packet
};

#endif // _HSA_AQL_PACKET_INFO_H_
