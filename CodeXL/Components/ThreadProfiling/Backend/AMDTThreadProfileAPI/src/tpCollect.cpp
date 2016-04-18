//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file tpCollect.cpp
///
//==================================================================================

#include <AMDTBaseTools/Include/AMDTDefinitions.h>

#include <wchar.h>
#include <string>
#include <list>

// Project headers
#include <AMDTThreadProfileApi.h>
#include <tpInternalDataTypes.h>
#include <AMDTOSWrappers/Include/osProcess.h>
#include <tpCollect.h>

#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
    #include <Windows/tpCollectImpl.h>
#elif AMDT_BUILD_TARGET == AMDT_LINUX_OS
    #include <Linux/tpCollectImpl.h>
#else
    #error Unknown build configuration!
#endif


//
//  Public Memeber functions
//

tpCollect::tpCollect()
{
    m_pImpl = new tpCollectImpl();
}


tpCollect::~tpCollect()
{
    if (NULL != m_pImpl)
    {
        delete m_pImpl;
        m_pImpl = NULL;
    }
}


AMDTResult tpCollect::tpClear()
{
    AMDTResult retVal = AMDT_ERROR_NOTIMPL;

    if (NULL != m_pImpl)
    {
        retVal = m_pImpl->tpClear();
    }

    return retVal;
} // tpClearTPSessionData


AMDTResult tpCollect::tpInitialize()
{
    AMDTResult retVal = AMDT_ERROR_NOTIMPL;

    if (NULL != m_pImpl)
    {
        retVal = m_pImpl->tpInitialize();
    }

    return retVal;
} // tpInitialize


AMDTResult tpCollect::tpSetThreadProfileConfiguration(AMDTUInt32 flags, const char* pFilePath)
{
    AMDTResult retVal = AMDT_ERROR_NOTIMPL;

    if (NULL != m_pImpl)
    {
        retVal = m_pImpl->tpSetThreadProfileConfiguration(flags, pFilePath);
    }

    return retVal;
} // tpSetThreadProfileConfiguration


AMDTResult tpCollect::tpStartThreadProfile()
{
    AMDTResult retVal = AMDT_ERROR_NOTIMPL;

    if (NULL != m_pImpl)
    {
        retVal = m_pImpl->tpStartThreadProfile();
    }

    return retVal;
} // tpStartThreadProfile


AMDTResult tpCollect::tpStopThreadProfile()
{
    AMDTResult retVal = AMDT_ERROR_NOTIMPL;

    if (NULL != m_pImpl)
    {
        retVal = m_pImpl->tpStopThreadProfile();
    }

    return retVal;
} // tpStopThreadProfile
