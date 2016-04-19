//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file dmnStringConstants.h
///
//==================================================================================


#ifndef __dmnStringConstants_h
#define __dmnStringConstants_h

// All string constants related to remote agent, and  remote client will be moved to this file.

// Remote Agent Section - Start
// Remote Agent Section - End

// Remote Client Section - Start

#define DMN_STR_ERR_PLATFORM_MISMATCH_A             L"Remote sessions from "
#define DMN_STR_ERR_PLATFORM_MISMATCH_B             L"are currently not supported."
#define DMN_STR_ERR_VERSION_MISMATCH_A              L"Version mismatch: remote agent's version is "
#define DMN_STR_ERR_VERSION_MISMATCH_B              L", while the local client's version is "
#define DMN_STR_SESSION_DISCONNECTION_A             L"*** Disconnection: "
#define DMN_STR_SESSION_DISCONNECTION_B             L"ended a session and disconnected ***"
#define DWM_STR_CODEXL_DIR_NAME                     L"CodeXL"
#define DMN_STR_CODEXL_REMOTE_AGENT                 L"CodeXLRemoteAgent"
#define DMN_STR_CODEXL_REMOTE_AGENT_SPACED_W        L"CodeXL Remote Agent"
#define DMN_STR_CODEXL_REMOTE_AGENT_SPACED_A        "CodeXL Remote Agent"
#define DMN_STR_PRESS_ANY_KEY                       L"Press any key to terminate CodeXL Remote Agent. . ."
#define DMN_STR_USAGE                               L"Usage: CodeXLRemoteAgent [--ip <ip string>]"
#define DMN_STR_ERR_MISSING_SETTINGS                L"Unable to retrieve CodeXL Remote Agent settings."
#define DMN_STR_ERR_CONFIGURATION                   L"Unable to extract CodeXL Remote Agent configurations."
#define DMN_STR_ERR_CONFIG_FILE                     L"Failed to find configuration file (%ls)."
#define DMN_STR_ERR_CORRUPTED_CONFIG_FILE           L"Corrupted configuration file (CodeXLRemoteDaemonConfig.xml)."
#define DMN_STR_ERR_CORRUPTED_CONFIG_FILE_EXCPTION  "Corrupted config file. Exception will be thrown."
#define DMN_STR_ERR_INVALID_CONFIG_FILE             L"Invalid configuration file."
#define DMN_STR_INFO_INFINIE_TIMEOUT                L"Infinite Timeout"
#define DMN_STR_ERR_INVALID_TIMEOUT                 L"Invalid Timeout Value"
#define DMN_STR_ERR_CONNECTION                      L"An error has occurred, CodeXL Remote Agent is unable to listen to incoming connections."
#define DMN_STR_ERR_ADDRESS_NOT_VALID               L"The address %ls is not a valid IPv4 address."

#define DMN_STR_HEADER_LINE                         "********************************************************"
#define DMN_STR_SETTINGS_EXTRACTED_OK               "CodeXL Remote Agent settings were extracted successfully"
#define DMN_STR_SEPARATOR                           "--------------------------------------------------------"
#define DMN_STR_AGENT_VERSION                       "Agent version is: "
#define DMN_STR_AGENT_READ_TIMEOUT                  "Read  timeout is: "
#define DMN_STR_AGENT_WRITE_TIMEOUT                 "Write timeout is: "
#define DMN_STR_AGENT_PORT                          "Agent IP:Port is: "
#define DMN_STR_END_LINE                            DMN_STR_HEADER_LINE
#define DMN_STR_AGENT_LISTENING_ON                  L"CodeXL Remote Agent is listening on "


#define DMN_STR_AMD                                 L"AMD"
#define DMN_STR_CODEXL                              L"CodeXL"

// Frame analysis
// Session frames XML
#define DMN_STR_FA_sessionInfoXMLVersion "1.0"
#define DMN_STR_FA_sessionInfoXMLSessions "Sessions"
#define DMN_STR_FA_sessionInfoXMLSession "Session"
#define DMN_STR_FA_sessionInfoXMLName "name"
#define DMN_STR_FA_sessionInfoXMLFrame "Frame"
#define DMN_STR_FA_sessionInfoXMLIndex "index"
#define DMN_STR_FA_playerName          L"CXLGraphicsServerPlayer"


// Remote Client Section - End

#endif // __dmnStringConstants_h

