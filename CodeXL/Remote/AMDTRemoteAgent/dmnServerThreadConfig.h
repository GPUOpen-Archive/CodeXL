//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file dmnServerThreadConfig.h
///
//==================================================================================

#ifndef __dmnServerThreadConfig_h
#define __dmnServerThreadConfig_h

#include <AMDTRemoteAgent/Public Include/dmnDefinitions.h>
#include <AMDTRemoteAgent/dmnUtils.h>
#include <AMDTBaseTools/Include/AMDTDefinitions.h>
#include <AMDTBaseTools/Include/gtString.h>

// Container for the configuration parameters
// for the Daemon's server thread.
class dmnServerThreadConfig
{
public:
    dmnServerThreadConfig(unsigned int backlog, unsigned int daemonPortNumber, long readTimeout, long writeTimeout, const gtString& userForcedIpStr);
    ~dmnServerThreadConfig(void);

    unsigned int getBacklog() const { return m_backlog; }
    unsigned int getDaemonPortNumber() const { return m_portNumber; }
    long getReadTimeout() const;
    long getWriteTimeout() const;

    bool isReadTimeout() const;
    bool isWriteTimeout() const;

    // True if the user forced the ip string.
    // Otherwise, false.
    bool isUserForcedIp() const;

    gtString getUserForcedIpString() const;

private:
    dmnServerThreadConfig& operator=(const dmnServerThreadConfig&);
    const unsigned int m_backlog;
    const unsigned int m_portNumber;
    const long m_readTimeout;
    const long m_writeTimeout;
    const gtString m_userForcedIpStr;
};

#endif // __dmnServerThreadConfig_h