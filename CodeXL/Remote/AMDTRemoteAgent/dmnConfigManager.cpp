//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file dmnConfigManager.cpp
///
//==================================================================================

#include <sstream>
#include <exception>
#include <AMDTBaseTools/Include/AMDTDefinitions.h>
#include <AMDTOSWrappers/Include/osProcess.h>
#include <AMDTOSWrappers/Include/osFilePath.h>
#include <AMDTRemoteAgent/dmnConfigManager.h>
#include <AMDTRemoteAgent/Public Include/dmnStringConstants.h>
// Static definitions - BEGIN.

// Daemon config file name.
static const wchar_t* CONFIG_FILE_NAME = L"CodeXLRemoteAgentConfig.xml";

dmnConfigManager* dmnConfigManager::m_pInstance = NULL;

static const char* SETTINGS_ELEM_NAME        = "Settings";
static const char* PORTNUM_ELEM_NAME         = "PortNumber";
static const char* RD_TIMEOUT_ELEM_NAME      = "ReadTimeoutMs";
static const char* WT_TIMEOUT_ELEM_NAME      = "WriteTimeoutMs";

// Static definitions - END.

dmnConfigManager::dmnConfigManager(void) : m_portNumber(0), m_readTimeout(0), m_writeTimeout(0),
    m_isInitialized(false), m_isPortNumExtracted(false),
    m_isReadTimeoutExtracted(false), m_isWriteTimeoutExtracted(false)
{
}


dmnConfigManager::~dmnConfigManager(void)
{
    if (m_pInstance != NULL)
    {
        delete m_pInstance;
    }
}

unsigned int dmnConfigManager::GetPortNumber() const
{
    return m_portNumber;
}

long dmnConfigManager::GetReadTimeout() const
{
    return m_readTimeout;
}

long dmnConfigManager::GetWriteTimeout() const
{
    return m_writeTimeout;
}

static bool IsPortNumElement(const std::string& elemName)
{
    return (elemName.compare(PORTNUM_ELEM_NAME) == 0);
}

static bool IsReadTimeoutElement(const std::string& elemName)
{
    return (elemName.compare(RD_TIMEOUT_ELEM_NAME) == 0);
}

static bool IsWriteTimeoutElement(const std::string& elemName)
{
    return (elemName.compare(WT_TIMEOUT_ELEM_NAME) == 0);
}

static bool HandlePortNumberElement(const std::string& elemValue,
                                    unsigned int& portNumBuffer)
{
    gtString text;
    bool ret = dmnUtils::ToGtString(elemValue, text);
    GT_IF_WITH_ASSERT(ret)
    {
        ret = !text.startsWith(L"-");
        GT_IF_WITH_ASSERT(ret)
        {
            ret = text.toUnsignedIntNumber(portNumBuffer);
            GT_ASSERT(ret);
        }
    }
    return ret;
}

static bool HandleTimeoutElement(const std::string& elemValue,
                                 long& timeoutBuffer)
{
    gtString text;
    bool ret = dmnUtils::ToGtString(elemValue, text);
    GT_IF_WITH_ASSERT(ret)
    {
        ret = text.toLongNumber(timeoutBuffer);
        GT_IF_WITH_ASSERT(ret)
        {
            ret = timeoutBuffer >= DMN_MAX_TIMEOUT_VAL;
            GT_ASSERT(ret);
        }
    }
    return ret;
}

bool dmnConfigManager::handlePortNumElement(const std::string& elemValue)
{
    m_isPortNumExtracted = m_isPortNumExtracted || HandlePortNumberElement(elemValue, m_portNumber);
    return m_isPortNumExtracted;
}

bool dmnConfigManager::handleReadTimeoutElement(const std::string& elemValue)
{
    m_isReadTimeoutExtracted = m_isReadTimeoutExtracted || HandleTimeoutElement(elemValue, m_readTimeout);
    return m_isReadTimeoutExtracted;
}

bool dmnConfigManager::handleWriteTimeoutElement(const std::string& elemValue)
{
    m_isWriteTimeoutExtracted = m_isWriteTimeoutExtracted || HandleTimeoutElement(elemValue, m_writeTimeout);
    return m_isWriteTimeoutExtracted;
}

static void FillConfigFileNotFoundErrorMessage(std::wstring& buffer)
{
    gtString errMessage;
    errMessage.appendFormattedString(DMN_STR_ERR_CONFIG_FILE, CONFIG_FILE_NAME);
    buffer = errMessage.asCharArray();
}

static void FillConfigFileCorruptedErrorMessage(std::wstring& buffer)
{
    buffer = std::wstring(DMN_STR_ERR_CORRUPTED_CONFIG_FILE);
}

static void HandleConfigFileCorruptedScenario(std::wstring& errMessageBuffer)
{
    FillConfigFileCorruptedErrorMessage(errMessageBuffer);
    const std::string msg = DMN_STR_ERR_CORRUPTED_CONFIG_FILE_EXCPTION;
    dmnUtils::LogMessage(msg, OS_DEBUG_LOG_ERROR);
}

bool dmnConfigManager::Init(std::wstring& errMsgBuffer)
{
    // Do this only once.
    if (m_isInitialized) { return true; }

    // Load settings from config file.
    bool isFailure = false;

    // Open config file.
    gtString configFilePath = L"";
    isFailure = !dmnUtils::GetCurrentDirectory(configFilePath);
    GT_ASSERT_EX(!isFailure, L"DMN: Failed extracting current directory to find config file.");

    if (!isFailure)
    {
        gtASCIIString fileContent;
        ReadConfigFile(configFilePath, fileContent);

        TiXmlDocument doc;
        doc.Parse(fileContent.asCharArray());
        m_isInitialized = (false == doc.Error());

        GT_IF_WITH_ASSERT(m_isInitialized)
        {
            TiXmlElement* pElement = doc.FirstChildElement();
            GT_IF_WITH_ASSERT(pElement != NULL)
            {
                string val = pElement->Value();
                m_isInitialized = (val.compare(SETTINGS_ELEM_NAME) == 0);
                pElement = pElement->FirstChildElement();
                m_isInitialized = pElement != NULL;
                GT_IF_WITH_ASSERT(m_isInitialized)
                {
                    for (; (pElement != NULL); pElement = pElement->NextSiblingElement())
                    {
                        GT_IF_WITH_ASSERT(pElement != NULL)
                        {
                            const char* elemNameTxt = pElement->Value();
                            isFailure = (elemNameTxt == NULL);

                            const char* elemValueTxt = pElement->GetText();
                            isFailure = isFailure || (elemValueTxt == NULL);

                            if (!isFailure)
                            {
                                const std::string elemName = pElement->Value();
                                const std::string elemValue = pElement->GetText();

                                // Handle the element.
                                isFailure = (IsPortNumElement(elemName) && !handlePortNumElement(elemValue))    ||
                                            (IsReadTimeoutElement(elemName) && !handleReadTimeoutElement(elemValue))    ||
                                            (IsWriteTimeoutElement(elemName) && !handleWriteTimeoutElement(elemValue));

                                // Break in case that the processing failed.
                                if (isFailure)
                                {
                                    std::stringstream errMsg;
                                    errMsg << "Failed to extract CodeXL Daemon configuration: <key, value> = <" <<
                                           elemName << ", " << elemValue << ">. Exception will be thrown.";

                                    dmnUtils::LogMessage(errMsg.str().c_str(), OS_DEBUG_LOG_ERROR);
                                    FillConfigFileCorruptedErrorMessage(errMsgBuffer);
                                    throw (std::exception());
                                }
                            }
                            else
                            {
                                HandleConfigFileCorruptedScenario(errMsgBuffer);
                                throw (std::exception());
                            }
                        }
                        else
                        {
                            HandleConfigFileCorruptedScenario(errMsgBuffer);
                            throw (std::exception());
                        }
                    }
                }
            }
        }
        else
        {
            FillConfigFileNotFoundErrorMessage(errMsgBuffer);
            throw (std::exception());
        }
    }
    else
    {
        FillConfigFileNotFoundErrorMessage(errMsgBuffer);
        throw (std::exception());
    }

    m_isInitialized = (!isFailure)    &&
                      m_isPortNumExtracted          &&
                      m_isReadTimeoutExtracted      &&
                      m_isWriteTimeoutExtracted;

    if (!m_isInitialized)
    {
        // If we made it up to here, some configuration parameter missing.
        std::stringstream errMsg;
        errMsg << "Configuration parameter missing in " <<
               CONFIG_FILE_NAME << ". Exception will be thrown.";
        dmnUtils::LogMessage(errMsg.str().c_str(), OS_DEBUG_LOG_ERROR);
        errMsgBuffer = DMN_STR_ERR_INVALID_CONFIG_FILE;
        throw (std::exception());
    }

    return m_isInitialized;
}

void dmnConfigManager::ReadConfigFile(const gtString& configFilePath, gtASCIIString& fileContent) const
{
    //Bug fix - Tiny xml can't open files with Unicode filePath, so we read the file into string
    osFilePath configPath;
    configPath.setFileDirectory(configFilePath);
    configPath.setFileName(CONFIG_FILE_NAME);
    osFile configFile(configPath);

    bool isFileOpenOk = configFile.open(configPath, osChannel::OS_ASCII_TEXT_CHANNEL, osFile::OS_OPEN_TO_READ);
    GT_ASSERT(isFileOpenOk);

    configFile.readIntoString(fileContent);
    GT_ASSERT(fileContent.length() > 0);
}

dmnConfigManager* dmnConfigManager::Instance()
{
    if (m_pInstance == NULL)
    {
        m_pInstance = new(std::nothrow)dmnConfigManager();
    }

    return m_pInstance;
}

bool dmnConfigManager::TimeoutToString(long timeout, std::wstring& buffer) const
{
    bool ret = true;
    wstringstream stream;

    try
    {
        if (timeout > 0)
        {
            stream << timeout;
        }
        else if (timeout == DMN_MAX_TIMEOUT_VAL)
        {
            stream << DMN_STR_INFO_INFINIE_TIMEOUT;
        }
        else
        {
            stream << DMN_STR_ERR_INVALID_TIMEOUT;
        }
    }
    catch (...)
    {
        ret = false;
    }

    buffer = std::wstring(stream.str());
    return ret;
}

bool dmnConfigManager::IsInternalVersion() const
{
#if AMDT_BUILD_ACCESS == AMDT_INTERNAL_ACCESS
    return  true;
#else
    return false;
#endif
}

