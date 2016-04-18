//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file dmnServerThreadConfig.cpp
///
//==================================================================================

#include <AMDTRemoteAgent/dmnServerThreadConfig.h>
#include <AMDTOSWrappers/Include/osChannel.h>

dmnServerThreadConfig::dmnServerThreadConfig(unsigned int backlog, unsigned int daemonPortNumber, long readTimeout, long writeTimeout, const gtString& userForcedIpStr) : m_backlog(backlog),
    m_portNumber(daemonPortNumber),
    m_readTimeout(readTimeout),
    m_writeTimeout(writeTimeout),
    m_userForcedIpStr(userForcedIpStr)
{
}


dmnServerThreadConfig::~dmnServerThreadConfig(void)
{
}

static bool isTimeoutRequired(long timeoutValue)
{
    return (timeoutValue >= DMN_MAX_TIMEOUT_VAL);
}

bool dmnServerThreadConfig::isReadTimeout() const
{
    return isTimeoutRequired(m_readTimeout);
}

bool dmnServerThreadConfig::isWriteTimeout() const
{
    return isTimeoutRequired(m_writeTimeout);
}

static long CalculateTimeout(long timeoutValue)
{
    long ret = timeoutValue;

    if (timeoutValue == DMN_MAX_TIMEOUT_VAL)
    {
        ret = LONG_MAX;
    }
    else if (timeoutValue <= DMN_MAX_TIMEOUT_VAL)
    {
        // Indicates a programmer error.
        dmnUtils::LogMessage(L"Warning: Calculated timeout for negative value.", OS_DEBUG_LOG_ERROR);
    }

    return ret;
}

long dmnServerThreadConfig::getReadTimeout() const
{
    return CalculateTimeout(m_readTimeout);
}

long dmnServerThreadConfig::getWriteTimeout() const
{
    return CalculateTimeout(m_writeTimeout);
}

gtString dmnServerThreadConfig::getUserForcedIpString() const
{
    return m_userForcedIpStr;
}

bool dmnServerThreadConfig::isUserForcedIp() const
{
    return (!m_userForcedIpStr.isEmpty());
}


