//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file dmnConfigManager.h
///
//==================================================================================

#ifndef __dmnConfigManager_h
#define __dmnConfigManager_h

#include <tinyxml.h>
#include <string>
#include <vector>

#include <AMDTRemoteAgent/dmnUtils.h>
#include <AMDTBaseTools/Include/AMDTDefinitions.h>
#include <AMDTBaseTools/Include/gtAssert.h>

class dmnConfigManager
{
public:
    ~dmnConfigManager(void);
    bool Init(std::wstring& errMsgBuffer);
    static dmnConfigManager* Instance();

    unsigned int GetPortNumber() const;

    long GetReadTimeout() const;
    long GetWriteTimeout() const;

    // Output a string representation of timeout as a string.
    bool TimeoutToString(long timeout, std::wstring& buffer) const;

    bool IsInternalVersion() const;

private:
    dmnConfigManager(void);
    void ReadConfigFile(const gtString& configFilePath, gtASCIIString& fileContent) const;
private:
    static dmnConfigManager* m_pInstance;
    unsigned int m_portNumber;
    long m_readTimeout;
    long m_writeTimeout;
    bool m_isInitialized;
    bool m_isPortNumExtracted;
    bool m_isReadTimeoutExtracted;
    bool m_isWriteTimeoutExtracted;



    bool handlePortNumElement(const std::string& elemValue);
    bool handleReadTimeoutElement(const std::string& elemValue);
    bool handleWriteTimeoutElement(const std::string& elemValue);
};

#endif // __dmnConfigManager_h
